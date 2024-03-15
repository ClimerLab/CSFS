#include "RelaxationSolver.h"
#include <cassert>

//------------------------------------------------------------------------------
//    Constructor
//------------------------------------------------------------------------------
RelaxationSolver::RelaxationSolver(const CSFS_Data &_data) : data(&_data),
                                                            cutSet(data->numStates),
                                                            objValue(0),
                                                            markVals(data->numStates),
                                                            indivVals(data->numIndiv),
                                                            env(IloEnv()),
                                                            cplex(IloCplex(env)),
                                                            model(IloModel(env)),
                                                            mark(IloNumVarArray(env, data->numStates, 0, 1, ILOFLOAT)),
                                                            indiv(IloNumVarArray(env, data->numIndiv, 0, 1, ILOFLOAT)),
                                                            markCopy(IloNumArray(env, data->numStates)),
                                                            indivCopy(IloNumArray(env, data->numIndiv)),
                                                            obj(IloExpr(env)),
                                                            baseConstraints(IloConstraintArray(env)),
                                                            cutConstraints(IloConstraintArray(env)),
                                                            fixedMarkConstraints(IloConstraintArray(env)),
                                                            fixedIndivConstraints(IloConstraintArray(env)),
                                                            indivEqualityConstraints(IloConstraintArray(env))
{
  mark.setNames("m");
  indiv.setNames("i");

  buildModel();

  cplex.extract(model);
  cplex.setParam(IloCplex::Param::Threads, 1);
  cplex.setParam(IloCplex::Param::RandomSeed, data->CPLEX_SEED);
  if (!data->PRINT_CPLEX_OUTPUT)
    cplex.setOut(env.getNullStream());
}


//------------------------------------------------------------------------------
// Adds the cut to cutConstraints
//------------------------------------------------------------------------------
void RelaxationSolver::add(const Cut &cut)
{
  cutSet.add(cut);

  IloExpr cutExpr(env);
  std::size_t x = 0, i = 0;
  while (x < cut.size())
  {
    if (cut[i])
    {
      cutExpr += mark[i];
      ++x;
    }
    ++i;
  }

  IloConstraint cutConstraint(cutExpr <= static_cast<IloInt>(data->setSize - 1));
  cutConstraint.setName("Cut");
  cutConstraints.add(cutConstraint);
  cutExpr.end();
}


//------------------------------------------------------------------------------
// Adds the objective function and constraints to the model
//------------------------------------------------------------------------------
void RelaxationSolver::buildModel()
{
  // *
  // * Add objective function
  // *
  for (std::size_t j = data->grpOneStart; j <= data->grpOneEnd; ++j)
    obj += indiv[j] / static_cast<IloNum>(data->numGrpOne);
  for (std::size_t j = data->grpTwoStart; j <= data->grpTwoEnd; ++j)
    obj -= indiv[j] / static_cast<IloNum>(data->numGrpTwo);


  // *
  // * The sum of the markers in the pattern must equal setSize.
  // *
  IloExpr markSummationExpr(env);
  for (std::size_t i = 0; i < data->numStates; ++i)
    markSummationExpr += mark[i];
  IloConstraint markSummation(markSummationExpr == data->setSize);
  markSummation.setName("MarkSummation");

  baseConstraints.add(markSummation);
  markSummationExpr.end();


  // *
  // * An individual can only be 1 if they carry the full pattern.
  // * An individual cannot be zero if they carry the full pattern.
  // *
  for (std::size_t j = data->grpOneStart; j <= data->grpOneEnd; ++j)
  {
    IloExpr gij_marki(env);
    for (std::size_t i = 0; i < data->numStates; ++i)
      gij_marki += mark[i] * data->exprs[i][j];
    IloConstraint indivUpper(indiv[j] <= gij_marki / data->setSize);
    std::string indivUpperName = "Indiv_" + std::to_string(j) + "_Upper";
    indivUpper.setName(indivUpperName.c_str());
    baseConstraints.add(indivUpper);
    gij_marki.end();
  }
  for (std::size_t j = data->grpTwoStart; j <= data->grpTwoEnd; ++j)
  {
    IloExpr gij_marki(env);
    for (std::size_t i = 0; i < data->numStates; ++i)
      gij_marki += mark[i] * data->exprs[i][j];
    IloConstraint indivLower(indiv[j] >= gij_marki - data->setSize + 1);
    std::string indivLowerName = "Indiv_" + std::to_string(j) + "_Lower";
    indivLower.setName(indivLowerName.c_str());
    baseConstraints.add(indivLower);
    gij_marki.end();
  }
 

  model.add( IloMaximize(env, obj, "Objective") );
  model.add(baseConstraints);
}


//------------------------------------------------------------------------------
// Removes all cuts from the model and adds cut from the cut set.
// Reasoning for this function is that the cut set is always cleaning itself up
// internally so that no cut is a subset of any other cut. Cplex does not do
// this, meaning the model could accumulate many redundant cuts.
//------------------------------------------------------------------------------
inline void RelaxationSolver::cleanUp()
{
  cutConstraints.removeFromAll();
  cutConstraints.endElements();
  auto rawCutSet = cutSet.getRawSet();
  for (auto it = std::begin(rawCutSet); it != std::end(rawCutSet); ++it)
  {
    IloExpr cutExpr(env);
    std::size_t x = 0, i = 0;
    while (x < (*it).size())
    {
      if ((*it)[i])
      {
        cutExpr += mark[i];
        ++x;
      }
      ++i;
    }

    IloConstraint cutConstraint(cutExpr <= static_cast<IloInt>(data->setSize - 1));
    cutConstraint.setName("Cut");
    cutConstraints.add(cutConstraint);
    cutExpr.end();
  }

  model.add(cutConstraints);
  //cplex.extract(model);
}


//------------------------------------------------------------------------------
// Returns the CPU time needed to solve the last relaxation
//------------------------------------------------------------------------------
double RelaxationSolver::getCpuTimeToSolve() const
{
  return timer.elapsed_cpu_time();
}


//------------------------------------------------------------------------------
// Returns the individual values from the most recently solved relaxation
//------------------------------------------------------------------------------
std::vector<std::pair<std::size_t, double> > RelaxationSolver::getIndivVals() const
{
  return indivVals;
}


Solution RelaxationSolver::getIntegralSolution() const
{
  assert(integral());

  // *
  // * Get the solution
  // *
  std::vector<std::size_t> vec;
  vec.reserve(data->setSize);
  for (std::size_t i = 0; vec.size() < data->setSize; ++i)
  {
    if (markVals[i].second > data->TOL)
      vec.push_back(markVals[i].first);
  }
  return Solution(vec, objValue);
}


//------------------------------------------------------------------------------
// Returns the mark values from the most recently solved relaxation
//------------------------------------------------------------------------------
std::vector<std::pair<std::size_t, double> > RelaxationSolver::getMarkVals() const
{
  return markVals;
}


//------------------------------------------------------------------------------
// Returns objValue
//------------------------------------------------------------------------------
double RelaxationSolver::getObjValue() const
{
  return objValue;
}


//------------------------------------------------------------------------------
// Returns a string of the marker and individual values from the last solved
// relaxation.
//------------------------------------------------------------------------------
std::string RelaxationSolver::getStringOfRelaxationValues() const
{
  std::ostringstream oss;

  {
    auto it = std::begin(markVals);
    std::size_t i = 0;
    oss << "\nRelaxation Mark Values:\n";
    for (i = 0; it != std::end(markVals); ++it, ++i)
    {
      oss << std::setw(7) << std::setprecision(5) << CSFSUtils::pround(it->second, 5) << " ";
      if (i % 10 == 9 || i == markVals.size() - 1)
        oss << "\n";
    }
  }

  {
    auto it = std::begin(indivVals);
    std::size_t i = 0;
    oss << "\nRelaxation Indiv Values:\n";
    for (i = 0; it != std::end(indivVals); ++it, ++i)
    {
      oss << std::setw(7) << std::setprecision(5) << CSFSUtils::pround(it->second, 5) << " ";
      if (i % 10 == 9 || i == indivVals.size() - 1)
        oss << "\n";
    }
  }

  return oss.str();
}


//------------------------------------------------------------------------------
// Returns the wall clock time needed to solve the last relaxation
//------------------------------------------------------------------------------
double RelaxationSolver::getWallTimeToSolve() const
{
  return timer.elapsed_wall_time();
}


//------------------------------------------------------------------------------
// Returns true if the the relaxation gave an integral solution
//------------------------------------------------------------------------------
bool RelaxationSolver::integral() const
{
  for (std::size_t j = 0; j < data->numIndiv; ++j)
  {
    if (indivCopy[j] < 1 - data->TOL || indivCopy[j] > data->TOL)
      return false;
  }
  for (std::size_t i = 0; i < data->numStates; ++i)
  {
    if (markCopy[i] < 1 - data->TOL || markCopy[i] > data->TOL)
      return false;
  }
  return true;
}


//------------------------------------------------------------------------------
// Rounds values within data->TOL of 0 to zero, and values within TOL of 1 to 1.
//------------------------------------------------------------------------------
inline void RelaxationSolver::roundExtremeValues(std::vector<std::pair<std::size_t, double> > *vec)
{
  for (std::size_t i = 0; i < vec->size(); ++i)
  {
    if ((*vec)[i].second <= data->TOL)
      (*vec)[i].second = 0;
    else if ((*vec)[i].second >= 1-data->TOL)
      (*vec)[i].second = 1;
  }
}


//------------------------------------------------------------------------------
// Adds the constraint that indiv[indivNumber] must equal val
//------------------------------------------------------------------------------
void RelaxationSolver::setIndiv(const std::size_t indivNumber, const bool val)
{
  IloConstraint fixedIndivConstraint(indiv[indivNumber] == static_cast<IloInt>(val));
  fixedIndivConstraint.setName("FixedIndiv");
  fixedIndivConstraints.add(fixedIndivConstraint);
}


//------------------------------------------------------------------------------
// Adds the constraint that mark[markNumber] must equal val
//------------------------------------------------------------------------------
void RelaxationSolver::setMark(const std::size_t markNumber, const bool val)
{
  IloConstraint fixedMarkConstraint(mark[markNumber] == static_cast<IloInt>(val));
  fixedMarkConstraint.setName("FixedMark");
  fixedMarkConstraints.add(fixedMarkConstraint);
}


//------------------------------------------------------------------------------
// Adds the constraint that individuals x and y must be equal to one another
//------------------------------------------------------------------------------
void RelaxationSolver::setIndivEquality(const std::size_t x, const std::size_t y)
{
  IloConstraint indivEqualityConstraint(indiv[x] == indiv[y]);
  indivEqualityConstraint.setName("IndivEquality");
  indivEqualityConstraints.add(indivEqualityConstraint);
}


//------------------------------------------------------------------------------
// Update the model, solve the relaxation, and get the objective and variable
// values
//------------------------------------------------------------------------------
void RelaxationSolver::solve()
{
  // *
  // * Update the model
  // *
  if (static_cast<std::size_t>(cutConstraints.getSize()) < 2 * cutSet.numCuts())
    model.add(cutConstraints);
  else
    cleanUp();

  model.add(fixedMarkConstraints);
  model.add(fixedIndivConstraints);
  model.add(indivEqualityConstraints);

  cplex.extract(model);
  cplex.setParam(IloCplex::Param::RandomSeed, data->CPLEX_SEED);

  // *
  // * Solve the relaxation
  // *
  timer.restart();
  cplex.solve();
  timer.stop();

  // *
  // * Get the objective value and variable values
  // *
  if (cplex.getStatus() == IloAlgorithm::Infeasible)
  {
    objValue = 0;
    for (std::size_t i = 0; i < data->numStates; ++i)
    {
      markCopy[i] = 0;
      markVals[i] = std::make_pair(i, 0);
    }
    for (std::size_t j = 0; j < data->numIndiv; ++j)
    {
      indivCopy[j] = 0;
      indivVals[j] = std::make_pair(j, 0);
    }
  }
  else if (cplex.getStatus() == IloAlgorithm::Optimal)
  {
    objValue = cplex.getObjValue();
    cplex.getValues(mark, markCopy);
    cplex.getValues(indiv, indivCopy);
    for (std::size_t i = 0; i < data->numStates; ++i)
      markVals[i] = std::make_pair(i, markCopy[i]);
    for (std::size_t j = 0; j < data->numIndiv; ++j)
      indivVals[j] = std::make_pair(j, indivCopy[j]);
    roundExtremeValues(&markVals);
    roundExtremeValues(&indivVals);
  }
  else
  {
    throw std::logic_error("Relaxation was not optimal");
  }
}


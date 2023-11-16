#include "CoverLpSolver.h"

//------------------------------------------------------------------------------
//    Constructor
//------------------------------------------------------------------------------
CoverLpSolver::CoverLpSolver(const CSFS_Data &_data) : data(&_data),
                                                      env(IloEnv()),
                                                      mark(IloNumVarArray(env, data->numStates, 0, 1, ILOFLOAT)),
                                                      markCopy(IloNumArray(env, data->numStates)),
                                                      cutConstraints(IloConstraintArray(env)),
                                                      fixedMarkConstraints(IloConstraintArray(env)),
                                                      fixedIndivConstraints(IloConstraintArray(env)),
                                                      cutConstraintsUpdated(false),
                                                      fixedMarkConstraintsUpdated(false),
                                                      fixedIndivConstraintsUpdated(false)
{
  if (!data->USE_COVER_LPS)
    return;

  // *
  // * Build the models
  // *
  model.reserve(data->numIndiv);
  for (std::size_t j = 0; j < data->numIndiv; ++j)
    model.emplace_back(env);
  buildModels();

  // *
  // * Set up CPLEX objects
  // *
  cplex.reserve(data->numIndiv);
  for (std::size_t j = 0; j < data->numIndiv; ++j)
  {
    cplex.emplace_back(model[j]);
    cplex[j].setParam(IloCplex::Param::Threads, 1);
    if (!data->PRINT_CPLEX_OUTPUT)
      cplex[j].setOut(env.getNullStream());
  }

  mark.setNames("m");

  // *
  // * Set up the vector of Individuals
  // *
  individuals.reserve(data->numIndiv);
  for (std::size_t j = 0; j < data->numIndiv; ++j)
  {
    // Count the number of nonzero marker states for this individual
    std::size_t numNonzeroStates = 0;
    for (std::size_t i = 0; i < data->numStates; ++i)
    {
      if (data->exprs[i][j] == 1)
        ++numNonzeroStates;
    }

    // Determine the group number
    short group = (j >= data->grpOneStart && j <= data->grpOneEnd) ? 1 : 2;

    individuals.emplace_back(Individual(j, group, numNonzeroStates));
  }

  for (std::size_t j = data->grpTwoStart; j <= data->grpTwoEnd; ++j)
    individuals[j].setObjValue(0);
}


//------------------------------------------------------------------------------
// Add the cut to the cutConstraints array
//------------------------------------------------------------------------------
void CoverLpSolver::add(const Cut &cut)
{
  IloExpr cutExpr(env);

  for (std::size_t x = 0, i = 0; x < cut.size(); ++i)
  {
    if (cut[i])
    {
      cutExpr += mark[i];
      ++x;
    }
  }

  IloConstraint cutConstraint(cutExpr <= static_cast<IloInt>(data->setSize - 1));
  cutConstraint.setName("Cut");
  cutConstraints.add(cutConstraint);
  cutExpr.end();
  cutConstraintsUpdated = true;
}


//------------------------------------------------------------------------------
// Build the models
//------------------------------------------------------------------------------
void CoverLpSolver::buildModels()
{
  // *
  // * Add objective function
  // *
  for (std::size_t j = data->grpOneStart; j <= data->grpOneEnd; ++j)
  {
    IloExpr obj(env);
    for (std::size_t i = 0; i < data->numStates; ++i)
      obj += mark[i] * data->exprs[i][j];
    model[j].add( IloMaximize(env, obj, "Objective") );
    obj.end();
  }
  for (std::size_t j = data->grpTwoStart; j <= data->grpTwoEnd; ++j)
  {
    IloExpr obj(env);
    for (std::size_t i = 0; i < data->numStates; ++i)
      obj += mark[i] * data->exprs[i][j];
    model[j].add( IloMinimize(env, obj, "Objective") );
    obj.end();
  }

  // *
  // * For group two, the sum of all the markers must equal setSize
  // *
  IloExpr markSummationExpr(env);
  for (std::size_t i = 0; i < data->numStates; ++i)
    markSummationExpr += mark[i];
  IloConstraint markSummation(markSummationExpr == data->setSize);
  markSummation.setName("MarkSummation");
  for (std::size_t j = data->grpTwoStart; j <= data->grpTwoEnd; ++j)
    model[j].add(markSummation);
  markSummationExpr.end();


  // *
  // * Must add these explicitly because each individual only has half the
  // * total number of possible states. If we didn't have this, then calling
  // * getValues would give an error.
  // *
  for (std::size_t j = data->grpOneStart; j <= data->grpOneEnd; ++j)
    model[j].add(mark);


  // *
  // * For each SNP, a pattern is not allowed to have homozygote for the first
  // * allele and carrier of the first allele add up to greater than 1. Same for
  // * the second allele.
  // *
  for (std::size_t i = 0; i < data->numStates; i += 4)
  {
    IloConstraint firstAndSecondState(mark[i] + mark[i+1] <= 1);
    IloConstraint thirdAndFourthState(mark[i+2] + mark[i+3] <= 1);

    std::ostringstream firstAndSecondStateName;
    std::ostringstream thirdAndFourthStateName;
    firstAndSecondStateName << "SNP_" << i/4 << "_States_1_2";
    thirdAndFourthStateName << "SNP_" << i/4 << "_States_3_4";
    firstAndSecondState.setName(firstAndSecondStateName.str().c_str());
    thirdAndFourthState.setName(thirdAndFourthStateName.str().c_str());

    for (std::size_t j = 0; j < data->numIndiv; ++j)
    {
      model[j].add(firstAndSecondState);
      model[j].add(thirdAndFourthState);
    }
  }
}


//------------------------------------------------------------------------------
// Get the CPU time it took to solve the last set of cover LPs
//------------------------------------------------------------------------------
double CoverLpSolver::getCpuTimeToSolve() const
{
  return timer.elapsed_cpu_time();
}


//------------------------------------------------------------------------------
// Returns the marker values for the individual
//------------------------------------------------------------------------------
std::vector<std::pair<std::size_t, double> > CoverLpSolver::getMarkVals(const std::size_t j) const
{
  return individuals[j].getMarkVals();
}


//------------------------------------------------------------------------------
// Returns the number of nonzero markers for the individual
//------------------------------------------------------------------------------
std::size_t CoverLpSolver::getNumNonzeroMarkers(const std::size_t j) const
{
  return individuals[j].getNumNonzeroMarkers();
}


//------------------------------------------------------------------------------
// Returns the objective value for the individual
//------------------------------------------------------------------------------
double CoverLpSolver::getObjValue(const std::size_t j) const
{
  return individuals[j].getObjValue();
}


//------------------------------------------------------------------------------
// Get the wall clock time it took to solve the last set of cover LPs
//------------------------------------------------------------------------------
double CoverLpSolver::getWallTimeToSolve() const
{
  return timer.elapsed_wall_time();
}


//------------------------------------------------------------------------------
// Sets the indiviudal to 0 or 1
//------------------------------------------------------------------------------
void CoverLpSolver::setIndiv(const std::size_t j, const bool val)
{
  // *
  // * Return if an indiviudal has already been set to the value given
  // *
  if (val == 0 && individuals[j].isZero())
    return;
  else if (val == 1 && individuals[j].isOne())
    return;

  // *
  // * Set the individual to val
  // *
  individuals[j].set(val);

  // *
  // * Sum up the markers the individual carries
  // *
  IloExpr expr(env);
  for (std::size_t i = 0; i < data->numStates; ++i)
    expr += mark[i] * data->exprs[i][j];

  // *
  // * Add a constraint to fixedIndivConstraints
  // *
  if (val == 0)
  {
    std::ostringstream constraintName;
    constraintName << "Indiv_" << j << "==0";
    IloConstraint fixedIndivConstraint(expr <= static_cast<IloInt>(data->setSize - 1));
    fixedIndivConstraint.setName(constraintName.str().c_str());
    fixedIndivConstraints.add(fixedIndivConstraint);
  }
  else
  {
    std::ostringstream constraintName;
    constraintName << "Indiv_" << j << "==1";
    IloConstraint fixedIndivConstraint(expr == data->setSize);
    fixedIndivConstraint.setName(constraintName.str().c_str());
    fixedIndivConstraints.add(fixedIndivConstraint);
  }

  fixedIndivConstraintsUpdated = true;
  expr.end();

  // *
  // * Update the individual's information
  // *
  if (val == 0)
  {
    std::vector<std::pair<std::size_t, double> > markVals(data->numStates);
    for (std::size_t i = 0; i < data->numStates; ++i)
      markVals[i] = std::make_pair(i, 0);

    individuals[j].setObjValue(0);
    individuals[j].setNumNonzeroMarkers(0);
    individuals[j].setMarkVals(markVals);
  }
  else
  {
    std::vector<std::pair<std::size_t, double> > markVals(data->numStates);
    for (std::size_t i = 0; i < data->numStates; ++i)
      markVals[i] = std::make_pair(i, static_cast<double>(data->setSize) / (data->numStates / 2));

    individuals[j].setObjValue(data->setSize);
    individuals[j].setNumNonzeroMarkers(data->numStates / 2);
    individuals[j].setMarkVals(markVals);
  }
}


//------------------------------------------------------------------------------
// Sets the marker value to 0 or 1
//------------------------------------------------------------------------------
void CoverLpSolver::setMark(const std::size_t markNumber, const bool val)
{
  IloConstraint fixedMarkConstraint(mark[markNumber] == static_cast<IloInt>(val));
  fixedMarkConstraint.setName("FixedMark");
  fixedMarkConstraints.add(fixedMarkConstraint);
  fixedMarkConstraintsUpdated = true;
}


//------------------------------------------------------------------------------
// For each unset individual, the model is updated, solved, and the individual's
// data is updated.
//------------------------------------------------------------------------------
void CoverLpSolver::solve()
{
  timer.restart();

  // *
  // * Update the models
  // *
  for (std::size_t j = 0; j < data->numIndiv; ++j)
  {
    if (individuals[j].isSet())
      continue;

    if (cutConstraintsUpdated)
      model[j].add(cutConstraints);
    if (fixedMarkConstraintsUpdated)
      model[j].add(fixedMarkConstraints);
    if (fixedIndivConstraintsUpdated)
      model[j].add(fixedIndivConstraints);
  }

  // *
  // * Solve the models
  // *
  for (std::size_t j = 0; j < data->numIndiv; ++j)
  {
    if (individuals[j].isSet())
      continue;

    cplex[j].solve();

    // *
    // * Update the individual
    // *
    std::vector<std::pair<std::size_t, double> > markVals(data->numStates);
    std::size_t numNonzeroMarkers = 0;

    individuals[j].setObjValue(cplex[j].getObjValue());
    cplex[j].getValues(mark, markCopy);

    for (std::size_t i = 0; i < data->numStates; ++i)
    {
      markVals[i].first = i;
      if (markCopy[i] <= data->TOL)
      {
        markVals[i].second = 0;
      }
      else
      {
        ++numNonzeroMarkers;
        if (markCopy[i] < 1-data->TOL)
          markVals[i].second = markCopy[i];
        else
          markVals[i].second = 1;
      }
    }
    individuals[j].setMarkVals(markVals);
    individuals[j].setNumNonzeroMarkers(numNonzeroMarkers);
  }

  cutConstraintsUpdated = false;
  fixedMarkConstraintsUpdated = false;
  fixedIndivConstraintsUpdated = false;
  timer.stop();
}


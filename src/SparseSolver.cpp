#include "SparseSolver.h"
#include "CSFS_Utils.h"

//------------------------------------------------------------------------------
//    Constructor
//------------------------------------------------------------------------------
SparseSolver::SparseSolver(const CSFS_Data &_data) : data(&_data),
                                                    cutSet(0),
                                                    cutToSolve(0),
                                                    markVals(data->numStates),
                                                    indVals(data->numIndiv),
                                                    indivEquals(data->numIndiv),
                                                    numMarkersInCutToSolve(data->numIndiv),
                                                    numGrpOneCarrying(data->numStates, 0),
                                                    lpMarkVals(data->numStates),
                                                    numGrpTwoFullCutToSolve(0),
                                                    objValue(0),
                                                    solutionPool(0),
                                                    pattern(data->numStates),
                                                    threshold(0)
{}

//------------------------------------------------------------------------------
//    Updates the cut to solce
//------------------------------------------------------------------------------
void SparseSolver::setCutToSolve(const Cut &cut)
{
  cutToSolve = cut;
}

//------------------------------------------------------------------------------
//   Adds the cut to the local cut set
//------------------------------------------------------------------------------
void SparseSolver::addToCutSet(const Cut &cut)
{
  cutSet.push_back(cut);
}

//------------------------------------------------------------------------------
//   Updates the local marker values container
//   val == 0: marker should be forced to 0 in model
//   val == 1: marker should be forced to 1 in model
//   val == 2: marker value is determined by model
//------------------------------------------------------------------------------
void SparseSolver::setMark(const std::size_t index, const std::size_t val)
{
  assert(index < data->numStates);

  markVals[index] = val;
}

//------------------------------------------------------------------------------
//   Updates the local individual values container
//   val == 0: individual should be forced to 0 in model
//   val == 1: individual should be forced to 1 in model
//   val == 2: individual value is determined by model
//------------------------------------------------------------------------------
void SparseSolver::setIndiv(const std::size_t index, const std::size_t val)
{
  assert(index < data->numIndiv);

  indVals[index] = val;
}

//------------------------------------------------------------------------------
//   Updates the local threshold
//------------------------------------------------------------------------------
void SparseSolver::setThreshold(const double val)
{
  threshold = val;
}


//------------------------------------------------------------------------------
//   Counts the number of non-zero marker states each individual has that 
//	 matches the cut to solve
//------------------------------------------------------------------------------
void SparseSolver::countNumberOfMarkersInCutToSOlve()
{
  std::vector<std::size_t> cut_elements = cutToSolve.getTrueElements();
  
  for(std::size_t j = 0; j < data->numIndiv; ++j)
  {
    numMarkersInCutToSolve[j] = 0;
	
	  for(auto it = std::begin(cut_elements); it != std::end(cut_elements); ++it)
	  {
		  if(data->exprs[*it][j] && markVals[*it]!= 0)
			  ++numMarkersInCutToSolve[j];
	  }
  }
}

//------------------------------------------------------------------------------
//   Iterates through all the individuals to see if any can be set to 0 or 1
//	 based on the cut to solve
//------------------------------------------------------------------------------
bool SparseSolver::setIndividualsToZeroOrOne()
{
  numGrpTwoFullCutToSolve = 0;
  std::size_t num_inds_set = 0;
	
  for (size_t j = data->grpOneStart; j <= data->grpOneEnd; ++j)
  {
    if (indVals[j] == 2)
    {
      if (numMarkersInCutToSolve[j] < data->setSize)
      {
        setIndiv(j, 0);
        ++num_inds_set;
      }
      else if (numMarkersInCutToSolve[j] == cutToSolve.size())
      {
        setIndiv(j, 1);
        ++num_inds_set;
      }
    }
  }

  for (size_t j = data->grpTwoStart; j <= data->grpTwoEnd; ++j)
  {
    if (indVals[j] == 2)
    {
      if (numMarkersInCutToSolve[j] < data->setSize)
      {
        setIndiv(j, 0);
        ++num_inds_set;
      }
      else if (numMarkersInCutToSolve[j] == cutToSolve.size())
      {
        setIndiv(j, 1);
        ++numGrpTwoFullCutToSolve;
        ++num_inds_set;
      }
    }
  }

  //std::cout << "Set " << num_inds_set << " individuals in sparse problem\n";
  
  return (num_inds_set > 0);
}


bool SparseSolver::setMarkersToZero()
{
  std::size_t num_markers_set = 0;
  
  std::vector<std::size_t> cut_elements = cutToSolve.getTrueElements();
  
  for(auto it = std::begin(cut_elements); it != std::end(cut_elements); ++it) // Loop through all markers in cut
  {
    if(markVals[*it] == 2) // Check that marker is not already set
    {
      numGrpOneCarrying[*it] = 0; // Reset counter
      
      for (size_t j = data->grpOneStart; j <= data->grpOneEnd; ++j) // Loop through all group 1
      {
        if(data->exprs[*it][j] && indVals[j] != 0)
          ++numGrpOneCarrying[*it];
      }
      
      double upperLimit = numGrpOneCarrying[*it]/static_cast<double>(data->numGrpOne) - numGrpTwoFullCutToSolve / static_cast<double>(data->numGrpTwo);
      
      if(upperLimit < threshold)
      {
        setMark(*it, 0);
        ++num_markers_set;
      }
    }
  }
  
  // std::cout << "Set " << num_markers_set << " markers in sparese problem" << std::endl;
  
  return (num_markers_set > 0);
}


//------------------------------------------------------------------------------
// Creates equalities between individuals with identical marker states in the
// Cut To Solve (of those marker states that haven't been set to zero).
// Individuals already set to zero or one are exlcuded. Returns true if at least
// 1 equality was set, false otherwise.
//------------------------------------------------------------------------------
bool SparseSolver::setIndividualEqualityConstraints()
{
  std::size_t num_eqaulities_set = 0;
  std::vector<std::size_t> cut_elements = cutToSolve.getTrueElements();
  std::vector<std::pair<std::size_t, std::size_t>> individuals_copy;
  std::vector<std::pair<std::size_t, std::size_t> > groups;
  
  for(std::size_t j = 0; j < data->numIndiv; ++j)
  {
    if(indVals[j] == 2)
      individuals_copy.emplace_back(std::make_pair(j, numMarkersInCutToSolve[j]));
  }
  std::sort(std::begin(individuals_copy), std::end(individuals_copy), CSFSUtils::SortPairBySecondItemIncreasing());
    
  // *
  // * Find the start and end indexes for all groups of individuals that have
  // * the same number of remaining markers
  // *
  for (std::size_t i = 0; i < individuals_copy.size() - 1;)
  {
    std::size_t j = i + 1;
    while (j < individuals_copy.size())
    {
      if (individuals_copy[i].second != individuals_copy[j].second)
      {
        if (j-1 != i)
          groups.emplace_back(std::make_pair(i, j-1));
        break;
      }
      else if (j == individuals_copy.size() - 1)
      {
        groups.emplace_back(std::make_pair(i, j));
      }
      ++j;
    }
    i = j;
  }
  
  for (auto it = std::begin(groups); it != std::end(groups); ++it)
  {
    const std::size_t start = it->first;
    const std::size_t end = it->second;

    for (std::size_t index1 = start; index1 <= end - 1; ++index1)
    {
      std::size_t x = individuals_copy[index1].first;
      for (std::size_t index2 = index1+1; index2 <= end; ++index2)
      {
        std::size_t y = individuals_copy[index2].first;

        // *
        // * Check if individuals x or y already have an
        // * equality constraint
        // *
        if ((indVals[x] != 2) || (indVals[y] != 2))
          continue;

        // *
        // * Check if individual x and y's remaining markers are equal
        // *
        {
          bool match = true;
          for(auto it2 = std::begin(cut_elements); it2 != std::end(cut_elements); ++it2)
          {
            if((markVals[*it2] != 0) && (data->exprs[*it2][x] != data->exprs[*it2][y]))
            {
              match = false;
              break;
            }
          }
           
          // *
          // * If individual x and y's remaining markers are equal
          // *
          if (match)
          {
            indVals[y] = 3;
            indivEquals[y] = x;
            ++num_eqaulities_set;

            #ifndef NDEBUG
              std::cout << "Forcing individual_" << y << " to equal individual_" << x << std::endl;
            #endif
          }
        }
      }
    }
  }
  
  //std::cout << "Setting " << num_eqaulities_set << " individual equalities in sparse problem" << std::endl;
  return (num_eqaulities_set > 0);
}


void SparseSolver::solve()
{
  std::vector<Solution>().swap( solutionPool ); // Reset container
  objValue = 0;
    
  if(data->USE_SPARSE_CONTRAINTS)
  {
    countNumberOfMarkersInCutToSOlve();
    setIndividualsToZeroOrOne();
  }
  solveMIP(cutToSolve); 
   
  std::vector<Cut>().swap( cutSet ); // Reset container  
}

//------------------------------------------------------------------------------
// Update the model, solve the relaxation, and get the objective and variable
// values
//------------------------------------------------------------------------------
void SparseSolver::solveMIP(const Cut& mipCutToSolve)
{  
  // Get number of marks and individuals for sparse problem
  const std::size_t numMarks = cutToSolve.size();
    
  IloEnv env;
  try {
    // DEBUG
    if (data->VERBOSE)
      std::cout << "Sparse Solver solv() Starting" << std::endl;
  
    IloModel model(env);
    IloCplex cplex(env);
    IloNumVarArray mark(env, numMarks, 0, 1, ILOINT);
    IloNumArray markCopy(env, numMarks, 0, 1);
    
    IloNumVarArray indiv(env, data->numIndiv, 0, 1, ILOINT);
  
    IloExpr obj(env);
    IloConstraintArray userConstraints(env);  

    mark.setNames("m");
    indiv.setNames("i");
  
    if (data->VERBOSE)
      std::cout << "Declared CPLEX variables" << std::endl;


    std::vector<std::size_t> origToSparse(data->numStates, data->numStates);
    std::vector<std::size_t> sparseToOrig = cutToSolve.getTrueElements();
    for(std::size_t i = 0; i < sparseToOrig.size(); ++i)
      origToSparse[sparseToOrig[i]] = i;

    // *
    // * Add objective function
    // *
    for (std::size_t j = data->grpOneStart; j <= data->grpOneEnd; ++j)
    {
      obj += indiv[j] / static_cast<IloNum>(data->numGrpOne);
    }
      
    for (std::size_t j = data->grpTwoStart; j <= data->grpTwoEnd; ++j)
    {
      obj -= indiv[j] / static_cast<IloNum>(data->numGrpTwo);
    }      
    
    if (data->VERBOSE)    
      std::cout << "Created Objective Function" << std::endl;
  
    model.add(IloMaximize(env, obj, "Objective"));
  
    if (data->VERBOSE)
      std::cout << "Added Objective Function" << std::endl;
     
    IloExpr markSummationExpr(env);
    for (std::size_t i = 0; i < numMarks; ++i)
      markSummationExpr += mark[i];    
    IloConstraint markSummation(markSummationExpr == data->setSize);
    markSummation.setName("MarkSummation");
    userConstraints.add(markSummation);
    markSummationExpr.end();
    if (data->VERBOSE)
    std::cout << "Added Pattern Size Constraint" << std::endl;


    // *
    // * Add constraints for each individual
    // *
    for (size_t j = data->grpOneStart; j <= data->grpOneEnd; ++j)
    {
      if(indVals[j] == 0) {
        // DEBUG        
        IloConstraint indivConst(indiv[j] == 0);
        userConstraints.add(indivConst);
      } else if (indVals[j] == 1) {
        IloConstraint indivConst(indiv[j] == 1);
        userConstraints.add(indivConst);
      } else if (indVals[j] == 3) { // Indivudal set to another individual
        IloConstraint indivConst(indiv[j] == indiv[indivEquals[j]]);
        userConstraints.add(indivConst);
      }
      else if (indVals[j] == 2) // Default
      {
        IloExpr gij_marki(env);
        for(std::size_t i = 0; i < data->numStates; ++i)
        {
          if (cutToSolve[i])
            gij_marki += mark[origToSparse[i]] * data->exprs[i][j];
        }
        IloConstraint indivConst(indiv[j] <= gij_marki / data->setSize);
        userConstraints.add(indivConst);
        gij_marki.end();
      }
    }
    for (size_t j = data->grpTwoStart; j <= data->grpTwoEnd; ++j)
    {
      if (indVals[j] == 0)
      {
        IloConstraint indivConst(indiv[j] == 0);
        userConstraints.add(indivConst);
      }
      else if (indVals[j] == 1)
      {
        IloConstraint indivConst(indiv[j] == 1);
        userConstraints.add(indivConst);
      }
      else if (indVals[j] == 3)
      {
        IloConstraint indivConst(indiv[j] == indiv[indivEquals[j]]);
        userConstraints.add(indivConst);
      }
      else if (indVals[j] == 2)
      {
        IloExpr gij_marki(env);
        for(std::size_t i = 0; i < data->numStates; ++i)
        {
          if(cutToSolve[i])
            gij_marki += mark[origToSparse[i]] * data->exprs[i][j];
        }
        IloConstraint indivConst(indiv[j] >= gij_marki - data->setSize + 1);
        userConstraints.add(indivConst);
        gij_marki.end();
      }
    }
    if (data->VERBOSE)
      std::cout << "Added Individual Contraints" << std::endl;
  

    // *
    // * Add constraints for forced makers
    // *
    for(std::size_t i = 0; i < data->numStates; ++i)
    {
      // Check if marker is forced
      if((markVals[i] != 2) && (cutToSolve[i]))
      {
        if(origToSparse[i] == numMarks)
          printf("ERROR: Indexing mark out of bounds\n");
        IloConstraint fixedMarkConstraint(mark[origToSparse[i]] == static_cast<IloInt>(markVals[i]));
        fixedMarkConstraint.setName("FixedMark");
        userConstraints.add(fixedMarkConstraint);      
      }   
    }
    if (data->VERBOSE)
      std::cout << "Added Fixed Marker Contraints" << std::endl;

    // *
    // * Add constraints from cut set
    // *
    for(std::size_t i_cut = 0; i_cut < cutSet.size(); ++i_cut)
    {
      IloExpr cutExpr(env);
      std::size_t numMarksInBothCuts = 0;
      for(std::size_t i = 0; i < data->numStates; ++i)
      {
        if(cutSet[i_cut][i] && cutToSolve[i])
        {
          if(origToSparse[i] == numMarks)
            printf("ERROR: Indexing mark out of bounds\n");

          cutExpr += mark[origToSparse[i]];
          ++numMarksInBothCuts;
        }
      }

      if (numMarksInBothCuts > data->setSize-1)
      {
        IloConstraint cutConstraint(cutExpr <= static_cast<IloInt>(data->setSize-1));
        cutConstraint.setName("Cut");
        userConstraints.add(cutConstraint);
      }
      cutExpr.end();
    }

    if (data->VERBOSE)
      std::cout << "Added Cuts Contraints" << std::endl;
  
    model.add(userConstraints);

    if (!data->PRINT_CPLEX_OUTPUT)
      cplex.setOut(env.getNullStream());

    if(data->USE_SOLUTION_POOL_THRESHOLD)
    {
      model.add(IloConstraint(obj >= data->SOLUTION_POOL_THRESHOLD));
      cplex.extract(model);
      cplex.setParam(IloCplex::Param::Threads, 1);
      cplex.setParam(IloCplex::Param::RandomSeed, data->CPLEX_SEED);
      cplex.setParam(IloCplex::Param::MIP::Pool::Replace, CPX_SOLNPOOL_DIV);
      cplex.setParam(IloCplex::Param::MIP::Pool::Intensity, 4);
      cplex.setParam(IloCplex::Param::MIP::Limits::Populate, 1000);
            
      // *
      // * Enumerate all solutions
      // *
      timer.restart();
      cplex.populate();
      timer.stop();
    }
    else
    {
      cplex.extract(model);
      cplex.setParam(IloCplex::Param::Threads, 1);
      cplex.setParam(IloCplex::Param::RandomSeed, data->CPLEX_SEED);

      if(data->USE_LOWER_CUTOFF && threshold > 0)
        cplex.setParam(IloCplex::Param::MIP::Tolerances::LowerCutoff, threshold);
      
      // *
      // * Solve the sparse problem
      // *
      timer.restart();
      cplex.solve();
      timer.stop();
    }
    
    
    // DEBUG
    if (data->VERBOSE)
      std::cout << "CPLEX Solved" << std::endl;
  
    // *
    // * Get the objective value and variable values
    // *
    if (cplex.getStatus() == IloAlgorithm::Infeasible)
    {
      // DEBUG
      if (data->VERBOSE)
        std::cout << "Infeasible Solution" << std::endl;
      objValue = 0;

      for (std::size_t i = 0; i < numMarks; ++i)
        markCopy[i] = 0;
      for (std::size_t i = 0; i < data->numStates; ++i)
        pattern[i] = 0;
    }
    else if (cplex.getStatus() == IloAlgorithm::Optimal)
    {
      int numSol = cplex.getSolnPoolNsolns();
            
      for (int i_sol = 0; i_sol < numSol; ++i_sol)
      {
        objValue = cplex.getObjValue(i_sol);
        
        if (objValue >= threshold && objValue > 0)
        {
          cplex.getValues(mark, markCopy, i_sol);
          
          for(std::size_t i = 0; i < data->numStates; ++i)
            pattern[i] = 0;            
          for(std::size_t i = 0; i < numMarks; ++i)
            pattern[sparseToOrig[i]] = markCopy[i];

          roundExtremeValues(&pattern);
          if (data->VERBOSE)
            std::cout << "Coppied Pattern" << std::endl;
        
          std::vector<std::size_t> sol_vect = getSolution();

          // DEBUG
          if (data->VERBOSE)
            std::cout << "Output Model" << std::endl;
      
          // Check if solution is duplicate
          bool dup = false;
          for (auto sol : solutionPool) {
            for (std::size_t i = 0; i < sol.markerStates.size(); ++i) {
              if (sol.markerStates[i] != sol_vect[i]) {
                break;
              }
              dup = true;
            }
            if (dup) {
              break;
            }
          }
          if (!dup) {
            solutionPool.push_back(Solution(sol_vect, objValue));
          }
        }       
      }
      
      std::cout << "Found " << solutionPool.size() << " soutions above threshold." << std::endl;
    }
    else
    {
      throw std::logic_error("Relaxation was not optimal");
    }
  
    obj.end();
    markSummation.end();
    model.end();
    cplex.end();
  
    // DEBUG
    if (data->VERBOSE)
      std::cout << "Finished Sparse Solver" << std::endl;
  }  
  catch (IloException &e) {
    std::cout << "Concert exception caught: " << e << std::endl;
  }
  catch (...) {
    std::cout << "Unknown exception caught" << std::endl;
  }
  
  env.end();

}
    
//------------------------------------------------------------------------------
//    Returns the solution pool
//------------------------------------------------------------------------------
std::vector<Solution>  SparseSolver::getSolutionPool() const
{
  return solutionPool;
}

//------------------------------------------------------------------------------
//    Returns the objective value 
//------------------------------------------------------------------------------
double SparseSolver::getObjValue() const
{
  return objValue;
}

//------------------------------------------------------------------------------
//    Returns the CPU time needed to solve the last sparse problem
//------------------------------------------------------------------------------
double SparseSolver::getCpuTimeToSolve() const
{
  return timer.elapsed_cpu_time();
}

//------------------------------------------------------------------------------
//    Returns the marker locations in pattern from CPLEX
//------------------------------------------------------------------------------
std::vector<std::size_t> SparseSolver::getSolution() const {
  std::vector<std::size_t> solution;

  for(std::size_t i = 0; i < data->numStates; ++i) {
    if(pattern[i] == 1) {
      solution.push_back(i);      
    }
  }

  return solution;
}

inline void SparseSolver::roundExtremeValues(std::vector<double> *vec)
{
  for(std::size_t i = 0; i < vec->size(); ++i)
  {
    if((*vec)[i] <= data->TOL)
      (*vec)[i] = 0;
    else if ((*vec)[i] >= 1 - data->TOL)
      (*vec)[i] = 1;
  }  
}
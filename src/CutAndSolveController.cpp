#include "CutAndSolveController.h"

//------------------------------------------------------------------------------
//    Constructor
//------------------------------------------------------------------------------
CutAndSolveController::CutAndSolveController(const CSFS_Data &_data) : data(&_data),
                                                              cc(_data),
                                                              rs(_data),
                                                              cutSet(data->numStates),
                                                              world_size(Parallel::getWorldSize()),
                                                              iter(0),
                                                              lb(data->STARTING_LOWER_BOUND),
                                                              ub(data->STARTING_UPPER_BOUND),
                                                              totalSparseTime(0) {
  if (data->USE_SOLUTION_POOL_THRESHOLD && data->SOLUTION_POOL_THRESHOLD > lb) {
    lb = data->SOLUTION_POOL_THRESHOLD;
  }

  for (std::size_t i = world_size - 1; i > 0; --i) {
    availableWorkers.push(i);
  }

  logfile.open(data->logfileName.c_str());
  if (!logfile.is_open()) {
    throw std::runtime_error("Logfile could not be opened");
  }

  // *
  // * Set up the vector of Markers
  // *
  markers.reserve(data->numStates);
  for (std::size_t i = 0; i < data->numStates; ++i) {
    std::size_t numGrpOneCarrying = 0;
    std::size_t numGrpTwoCarrying = 0;

    for (std::size_t j = data->grpOneStart; j <= data->grpOneEnd; ++j) {
      if (data->exprs[i][j]) {
        ++numGrpOneCarrying;
      }
    }
    for (std::size_t j = data->grpTwoStart; j <= data->grpTwoEnd; ++j) {
      if (data->exprs[i][j]) {
        ++numGrpTwoCarrying;
      }
    }
    markers.emplace_back(i, numGrpOneCarrying, numGrpTwoCarrying);
  }

  // *
  // * Set up the vector of Individuals
  // *
  individuals.reserve(data->numIndiv);
  for (std::size_t j = 0; j < data->numIndiv; ++j) {
    // Count the number of nonzero marker states for this individual
    std::size_t numNonzeroStates = 0;
    for (std::size_t i = 0; i < data->numStates; ++i) {
      if (data->exprs[i][j] == 1) {
        ++numNonzeroStates;
      }
    }

    // Determine the group number
    const short group = (j >= data->grpOneStart && j <= data->grpOneEnd) ? 1 : 2;

    individuals.emplace_back(Individual(j, group, numNonzeroStates));
  }
  
  setMarkersToZero();
  setIndividualsToZero();
  setIndividualEqualityConstraints();
}

//------------------------------------------------------------------------------
// Returns true if the upper and lower bounds have crossed.
//
// If USE_SOLUTION_POOL_THRESHOLD was set to true in the config file, then
// this will only return true if the upper bound is less than or equal to the
// SOLUTION_POOL_THRESHOLD value set in the config file.
//------------------------------------------------------------------------------
bool CutAndSolveController::converged() const {
  static bool trueConvergence = false;
  if (data->USE_SOLUTION_POOL_THRESHOLD) {
    if (!trueConvergence && ub <= lb && ub > data->SOLUTION_POOL_THRESHOLD) {
      trueConvergence = true;
      std::cout << "True convergence occurred (optimal solution = " << lb
                << ") but will continue to search for patterns with objective"
                << " values greater than " << data->SOLUTION_POOL_THRESHOLD
                << std::endl;
    }
    return ub <= data->SOLUTION_POOL_THRESHOLD;
  }
  return ub <= lb;
}


//------------------------------------------------------------------------------
// Returns the lower bound
//------------------------------------------------------------------------------
double CutAndSolveController::getLb() const {
  return lb;
}

//------------------------------------------------------------------------------
// Returns a string of the ranks of workers that are currently working
//------------------------------------------------------------------------------
std::string CutAndSolveController::getStringOfUnavailableWorkers() const {
  std::ostringstream oss;
  for (auto it = std::begin(unavailableWorkers); it != std::end(unavailableWorkers); ++it) {
    oss << *it << " ";
  }

  return oss.str();
}


//------------------------------------------------------------------------------
// Returns the upper bound
//------------------------------------------------------------------------------
double CutAndSolveController::getUb() const {
  return ub;
}


//------------------------------------------------------------------------------
// Returns the number of workers currently working on a sparse problem
//------------------------------------------------------------------------------
std::size_t CutAndSolveController::numWorkersWorking() const {
  return unavailableWorkers.size();
}


//------------------------------------------------------------------------------
// Receives a completed sparse problem from a worker
//------------------------------------------------------------------------------
inline void CutAndSolveController::receiveCompletion() {
  assert(availableWorkers.size() < world_size - 1); // Cannot receive problem when no workers are working

  MPI_Status status;
  double sparseRunTime;
  double bestObjValue = 0;
  std::size_t sparseNumSol;
  std::vector<std::vector<std::size_t> > solutionPool;
  std::vector<double> objValues;

  // *
  // * Receive the solution
  // *

  MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
  #ifndef NDEBUG
    
    std::cout << "Controller about to receive completion from rank_" << status.MPI_SOURCE << std::endl;
  #endif

  MPI_Recv(&sparseNumSol, 1, CUSTOM_SIZE_T, status.MPI_SOURCE, Parallel::SPARSE_TAG, MPI_COMM_WORLD, &status);
  
  #ifndef NDEBUG
    if (sparseNumSol > 0)
      std::cout << "Controller about to receive " << sparseNumSol << " solution vectors from rank_" << status.MPI_SOURCE << std::endl;
  #endif
  
  solutionPool.resize(sparseNumSol);
  objValues.resize(sparseNumSol);
  for (std::size_t i = 0; i < solutionPool.size(); ++i) {
    MPI_Recv(&objValues[i], 1, MPI_DOUBLE, MPI_ANY_SOURCE, Parallel::SPARSE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    
    #ifndef NDEBUG
      std::cout << "Controller received objective value from rank_" << status.MPI_SOURCE << std::endl;
    #endif
    if (i == 0) {
      bestObjValue = objValues[i];
    } else if (objValues[i] > bestObjValue) {
      bestObjValue = objValues[i];
    }
    
    solutionPool[i].resize(data->setSize);
    MPI_Recv(&solutionPool[i][0], solutionPool[i].size(), CUSTOM_SIZE_T, status.MPI_SOURCE, Parallel::SPARSE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
   }

  #ifndef NDEBUG
    if (sparseNumSol > 0)
      std::cout << "Controller received all solution vectors from rank_" << status.MPI_SOURCE << std::endl;
  #endif

  MPI_Recv(&sparseRunTime, 1, MPI_DOUBLE, status.MPI_SOURCE, Parallel::SPARSE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  
  #ifndef NDEBUG
    std::cout << "Controller received the run time from rank_"
              << status.MPI_SOURCE << "\nController received all info from rank_"
              << status.MPI_SOURCE << std::endl;
  #endif

  // *
  // * Update lower bound and statistics
  // *
  if (!data->USE_SOLUTION_POOL_THRESHOLD) // Don't update bound if using solutions pool
    lb = std::max(bestObjValue, lb);
  totalSparseTime += sparseRunTime;  
  
  for (std::size_t i = 0; i < solutionPool.size(); ++i) {
    if (!data->QUIET)
      std::cout << "\nSolution " << i + 1 << " of " << solutionPool.size()
                << " from rank_" << status.MPI_SOURCE << ":\n";
    CSFS::printSolution(solutionPool[i], &logfile, data);
  }

  if (!data->QUIET)
    std::cout << std::endl;

  checkIn.insert(static_cast<std::size_t>(status.MPI_SOURCE));

  // *
  // * Make the worker available again
  // *
  availableWorkers.push(status.MPI_SOURCE);
  unavailableWorkers.erase(status.MPI_SOURCE);
}

//------------------------------------------------------------------------------
// Sends a problem to a worker
//------------------------------------------------------------------------------
inline void CutAndSolveController::sendProblem(const Cut &cut)
{
  assert(!availableWorkers.empty()); // Cannot send problem with no available workers

  // *
  // * Convert the cuts into transferable vectors
  // *
  const std::vector<char> convertedCut = cut.getCharVector();
  const std::vector<std::vector<char> > convertedCuts = cutSet.get2dCharVector();

  // *
  // * Convert the individuals' fixed statuses
  // * If indiviudals[i] is fixed to 0, convertedIndiv[i] = 0
  // * If indiviudals[i] is fixed to 1, convertedIndiv[i] = 1
  // * If indiviudals[i] is not fixed to a value, convertedIndiv[i] = 2
  // *
  std::vector<char> convertedIndiv(data->numIndiv);
  for (std::size_t j = 0; j < individuals.size(); ++j)
  {
    if (!individuals[j].isSet())
      convertedIndiv[j] = 2;
    else
      convertedIndiv[j] = individuals[j].isZero() ? 0 : 1;
  }

  // *
  // * Convert the markers' fixed statuses
  // * If markers[i] is fixed to 0, then convertedMark[i] = 0
  // * If markers[i] is fixed to 1, then convertedMark[i] = 1
  // * If markers[i] is not fixed to a value, then convertedMark[i] = 2
  // *
  std::vector<char> convertedMark(data->numStates);
  for (std::size_t i = 0; i < markers.size(); ++i)
  {
    if (!markers[i].isSet())
      convertedMark[i] = 2;
    else
      convertedMark[i] = markers[i].isZero() ? 0 : 1;
  }

  const int worker = availableWorkers.top();
  const std::size_t numCuts = convertedCuts.size();

  // *
  // * Send the problem
  // *
  MPI_Send(&lb, 1, MPI_DOUBLE, worker, Parallel::SPARSE_TAG, MPI_COMM_WORLD);

  #ifndef NDEBUG
    std::cout << "Controller sent the lower bound to rank_" << worker << std::endl;
  #endif

  MPI_Send(&convertedCut[0], convertedCut.size(), MPI_CHAR, worker, Parallel::SPARSE_TAG, MPI_COMM_WORLD);

  #ifndef NDEBUG
    std::cout << "Controller sent the cut to rank_" << worker << std::endl;
  #endif

  MPI_Send(&numCuts, 1, CUSTOM_SIZE_T, worker, Parallel::SPARSE_TAG, MPI_COMM_WORLD);

  #ifndef NDEBUG
    std::cout << "Controller sent the size of the cut set to rank_" << worker << std::endl;
  #endif

  for (std::size_t i = 0; i < convertedCuts.size(); ++i)
    MPI_Send(&convertedCuts[i][0], convertedCuts[i].size(), MPI_CHAR, worker, Parallel::SPARSE_TAG, MPI_COMM_WORLD);

  #ifndef NDEBUG
    std::cout << "Controller sent the cut set to rank_" << worker << std::endl;
  #endif

  MPI_Send(&convertedMark[0], convertedMark.size(), MPI_CHAR, worker, Parallel::SPARSE_TAG, MPI_COMM_WORLD);

  #ifndef NDEBUG
    std::cout << "Controller sent the vector of markers to rank_" << worker << std::endl;
  #endif

  MPI_Send(&convertedIndiv[0], convertedIndiv.size(), MPI_CHAR, worker, Parallel::SPARSE_TAG, MPI_COMM_WORLD);

  #ifndef NDEBUG
    std::cout << "Controller sent the vector of individuals to rank_" << worker
              << "\nController sent all info to rank_" << worker <<  std::endl;
  #endif

  // *
  // * Make the worker unavailable
  // *
  availableWorkers.pop();
  unavailableWorkers.insert(worker);
}


//------------------------------------------------------------------------------
// Splits up a single sparse problem into multiple sparse problems and sends
// them to workers. If all workers are working, then problems are sent as workers
// finish their current problems.
//------------------------------------------------------------------------------
inline void CutAndSolveController::sendProblems(Cut cut)
{
  // *
  // * Removes all marker states from the cut that are shared by all cuts
  // * (meaning they've been removed from the problem)
  std::set<std::size_t> markersInAllCuts = cutSet.getMarkersKeptInAllCuts();
  for (auto it = std::begin(markersInAllCuts); it != std::end(markersInAllCuts); ++it)
    cut.remove(*it);

  if (availableWorkers.empty()) // wait for a free worker
    receiveCompletion();
  
  if (!data->QUIET)
    std::cout << "\nSending cut to rank_" << availableWorkers.top() << "\n" << cut.getMarkerNumberString() << std::endl;

  sendProblem(cut);  
}


//------------------------------------------------------------------------------
// Sets an individual to 0 or 1
//------------------------------------------------------------------------------
inline bool CutAndSolveController::setIndiv(const std::size_t j, const bool val)
{
  assert(!(val == 1 && individuals[j].isZero()));
  assert(!(val == 0 && individuals[j].isOne()));

  if (!individuals[j].set(val))
    return false;

  #ifndef NDEBUG
    std::cout << "Forcing indiv_" << j << " to equal " << val << std::endl;
  #endif

  rs.setIndiv(j, val);
  
  if (val == 0)
  {
    if (individuals[j].inGrpOne())
    {
      for (std::size_t i = 0; i < data->numStates; ++i)
      {
        if (data->exprs[i][j])
          markers[i].decrementNumGrpOneCarrying();
      }
    }
    else
    {
      for (std::size_t i = 0; i < data->numStates; ++i)
      {
        if (data->exprs[i][j])
          markers[i].decrementNumGrpTwoCarrying();
      }
    }
  }
  else
  {
    for (std::size_t i = 0; i < data->numStates; ++i)
    {
      if (!data->exprs[i][j] && !markers[i].isSet())
        setMark(i, 0);
    }
  }

  return true;
}


//------------------------------------------------------------------------------
// Creates equalities between individuals with identical marker states (of those
// marker states that haven't been set to zero). Returns true if at least 1
// equality was set, false otherwise.
//------------------------------------------------------------------------------
inline bool CutAndSolveController::setIndividualEqualityConstraints()
{
  bool equalityWasSet = false;
  std::vector<Individual> individuals_copy = individuals;
  std::sort(std::begin(individuals_copy),
            std::end(individuals_copy),
            Individual::sortByNumRemainingMarkers);
  std::vector<std::pair<std::size_t, std::size_t> > groups;

  // *
  // * Find the start and end indexes for all groups of individuals that have
  // * the same number of remaining markers
  // *
  for (std::size_t i = 0; i < data->numIndiv - 1;)
  {
    std::size_t j = i + 1;
    while (j < data->numIndiv)
    {
      if (individuals_copy[i].getNumRemainingMarkers() != individuals_copy[j].getNumRemainingMarkers())
      {
        if (j-1 != i)
          groups.emplace_back(std::make_pair(i, j-1));
        break;
      }
      else if (j == data->numIndiv - 1)
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
      std::size_t x = individuals_copy[index1].getId();
      for (std::size_t index2 = index1+1; index2 <= end; ++index2)
      {
        std::size_t y = individuals_copy[index2].getId();

        // *
        // * Check if there's already an equality constraint between
        // * individuals x and y
        // *
        if (individualEqualities.exists(x, y))
          continue;

        // *
        // * Check if individual x and y's remaining markers are equal
        // *
        {
          std::size_t markNum = 0;
          while (markNum < data->numStates && (markers[markNum].isZero()
             ||  data->exprs[markNum][x] == data->exprs[markNum][y]))
          {
            ++markNum;
          }

          // *
          // * If individual x and y's remaining markers are equal
          // *
          if (markNum == data->numStates)
          {
            individualEqualities.add(x, y);
            rs.setIndivEquality(x, y);
            equalityWasSet = true;

            #ifndef NDEBUG
              std::cout << "Forcing individual_" << x << " and individual_"
                        << y << " to equal each other" << std::endl;
            #endif
          }
        }
      }
    }
  }

  return equalityWasSet;
}


//------------------------------------------------------------------------------
// Iterates through all the individuals to see if any can be set to 0 based on
// the number of remaining nonzero markers
//------------------------------------------------------------------------------
inline bool CutAndSolveController::setIndividualsToZero()
{
  bool individualWasSet = false;

  for(std::size_t j = 0; j < data->numIndiv; ++j)
  {
    if(!individuals[j].isSet()
    &&  individuals[j].getNumRemainingMarkers() < data->setSize)
    {
      setIndiv(j, 0);
      individualWasSet = true;
    }
  }

  return individualWasSet;
}


//------------------------------------------------------------------------------
// Iterates through all the individuals to see if any can be set to 0 or 1
//------------------------------------------------------------------------------
inline bool CutAndSolveController::setIndividualsToZeroOrOne()
{
  bool individualWasSet = false;

  for (std::size_t j = data->grpOneStart; j <= data->grpOneEnd; ++j)
  {
    if (!individuals[j].isSet()
    &&  individuals[j].getObjValue() < data->setSize - data->TOL)
    {
      setIndiv(j, 0);
      individualWasSet = true;
    }
  }
  for (std::size_t j = data->grpTwoStart; j <= data->grpTwoEnd; ++j)
  {
    if (!individuals[j].isSet()
    &&  individuals[j].getObjValue() > data->setSize - 1 + data->TOL)
    {
      setIndiv(j, 1);
      individualWasSet = true;
    }
  }

  return individualWasSet;
}


//------------------------------------------------------------------------------
// Sets a marker to 0 or 1
//------------------------------------------------------------------------------
inline bool CutAndSolveController::setMark(const std::size_t i, const bool val)
{
  assert(!(val == 1 && markers[i].isZero()));
  assert(!(val == 0 && markers[i].isOne()));

  if (!markers[i].set(val))
    return false;

  #ifndef NDEBUG
    std::cout << "Forcing mark_" << i << " to equal " << val << std::endl;
  #endif

  rs.setMark(i, val);
  
  if (val == 0)
  {
    cutSet.keepMarkerInAllCuts(i);
    for (std::size_t j = 0; j < data->numIndiv; ++j)
    {
      if (data->exprs[i][j])
        individuals[j].decrementNumRemainingMarkers();
    }
  }
  else
  {
    for (std::size_t j = 0; j < data->numIndiv; ++j)
    {
      if (!data->exprs[i][j])
        setIndiv(j, 0);
    }
  }

  return true;
}


//------------------------------------------------------------------------------
// Iterates through all the markers to see if any can be set to 0
//------------------------------------------------------------------------------
inline bool CutAndSolveController::setMarkersToZero()
{
  bool markerWasSet = false;
  const double minRatio = data->USE_SOLUTION_POOL_THRESHOLD ? data->SOLUTION_POOL_THRESHOLD : std::max(lb, data->TOL);

  for (std::size_t i = 0; i < markers.size(); ++i)
  {
    if (!markers[i].isSet()
    &&  markers[i].getNumGrpOneCarrying() / static_cast<double>(data->numGrpOne) < minRatio)
    {
      setMark(i, 0);
      markerWasSet = true;
    }
  }

  return markerWasSet;
}


//------------------------------------------------------------------------------
// Sends a signal to workers to termiante after solving their current sparse
// problem.
//------------------------------------------------------------------------------
void CutAndSolveController::signalWorkersToEnd()
{
  char signal = 0;
  for (std::size_t i = 1; i < world_size; ++i)
    MPI_Send(&signal, 1, MPI_CHAR, i, Parallel::CONVERGE_TAG, MPI_COMM_WORLD);
}


//------------------------------------------------------------------------------
// Returns true if at least 1 worker is still working
//------------------------------------------------------------------------------
bool CutAndSolveController::workersStillWorking() const
{
  return (availableWorkers.size() < (world_size - 1));
}


//------------------------------------------------------------------------------
// Prints a string of the ranks of the working workers, and calls
// receiveCompletion()
//------------------------------------------------------------------------------
void CutAndSolveController::waitForWorkers()
{
  const double prevUb = ub;
  const double prevLb = lb;

  if (!data->QUIET)
    std::cout << "Waiting on " << numWorkersWorking() << " workers to finish" << std::endl;

  #ifndef NDEBUG
    std::cout << "Working workers: " << getStringOfUnavailableWorkers() << std::endl;
  #endif

  receiveCompletion();

  if (!data->QUIET && (prevUb != ub || prevLb != lb))
    std::cout << "\nBounds updated\nUpper bound: " << ub
              << "\nLower bound: " << lb << "\n" << std::endl;
}


//------------------------------------------------------------------------------
// The main function for cut and solve
//------------------------------------------------------------------------------
void CutAndSolveController::work() {
  static bool first = true;
  Cut cut;
  int cutCreatedFrom;
  std::size_t indivCutWasBasedOn;

  std::vector<std::pair<std::size_t, bool> > markersFixedAfterCut;
  std::vector<std::pair<std::size_t, bool> > individualsFixedAfterCut;
  VariableEqualities individualEqualitiesAfterCut;

  if (first && !data->QUIET) {
    std::cout << "---------------------------\n"
              << "   Initialization"
              << "\n---------------------------\n"
              << "Took " << data->elapsed_cpu_time()
              << " seconds to read data and set up the problem\n\n"
              << CSFS::getStringOfEndOfIterInfo(ub, lb, data->elapsed_cpu_time())
              << "\n" << std::endl;

    first = false;
  }  

  if (!data->QUIET)
    std::cout << "---------------------------\n"
              << "   Iteration " << iter
              << "\n---------------------------\n";


  // *
  // * Get the next cut to solve
  // *
  if (!data->QUIET)
    std::cout << "Solving relaxation..." << std::endl;

    
  // *
  // * Solve a relaxation
  // *
  rs.solve();
  ub = std::min(ub, rs.getObjValue());


  // *
  // * Print the relaxation values
  // *
  if (!data->QUIET)
    std::cout << "Relaxation found objective value of " << ub
              << "\nRelaxation took " << rs.getCpuTimeToSolve()
              << " seconds" << std::endl;
  if (data->VERBOSE)
    std::cout << rs.getStringOfRelaxationValues() << std::endl;


  // *
  // * Check for integrality
  // *
  if (rs.integral()) {
    ub = rs.getObjValue();
    lb = rs.getObjValue();
    CSFS::printSolution(rs.getIntegralSolution(), &logfile, data);
  }

  // *
  // * Return if convergence occurred
  // *
  if (converged()) {
    std::cout << "Convergence occurred.\n";
    return;
  }

  // *
  // * Create a cut to solve
  // *
  cut = cc.createCut(cutSet,
                     markers,
                     individuals,
                     rs.getMarkVals(),
                     data->maxNumCuts(lb),
                     &cutCreatedFrom,
                     &indivCutWasBasedOn);

  if (!data->QUIET) {
    if (cutCreatedFrom == cc.RELAXATION)
      std::cout << "Cut was created from relaxed values" << std::endl;
    else if (cutCreatedFrom == cc.MERGE)
      std::cout << "Cut was created from merging two previous cuts" << std::endl;
    else if (cutCreatedFrom == cc.INDIVIDUAL)
      std::cout << "Cut was created from indiv_" << indivCutWasBasedOn
                << "'s marker states" << std::endl;
  }
  
  const double prevLb = lb;

  // *
  // * Send sparse problems based on the cut to workers
  // *
  sendProblems(cut);


  // *
  // * Update markers, individuals, individualEqualities, and bounds if the cut
  // * we just sent was read from the cutfile
  // *
  bool individualWasSet = false;

  // *
  // * If the cut was based on an individual, set that individual to 0
  // *
  if (cutCreatedFrom == cc.INDIVIDUAL) {
    setIndiv(indivCutWasBasedOn, 0);

    if (individualEqualities.exists(indivCutWasBasedOn)) {
      for (std::size_t j = 0; j < data->numIndiv; ++j) {
        if (individualEqualities.exists(indivCutWasBasedOn, j))
          setIndiv(j, 0);
      }
    }

    individualWasSet = true;
  }


  // *
  // * Update the model
  // *
  cutSet.add(cut);
  rs.add(cut);

  // *
  // * Try to set markers to zero or one and individuals equal to each other
  // *
  if (std::abs(lb - prevLb) > data->TOL || individualWasSet) {
    if (setMarkersToZero()) {
      setIndividualsToZero();
      setIndividualEqualityConstraints();
    }
  }

  ++iter;

  #ifndef NDEBUG
    std::cout << "Working workers: " << getStringOfUnavailableWorkers() << std::endl;
  #endif

  if (!data->QUIET)
  std::cout << "\n" << CSFS::getStringOfEndOfIterInfo(ub, lb, data->elapsed_cpu_time())
            << "\n" << std::endl;
}
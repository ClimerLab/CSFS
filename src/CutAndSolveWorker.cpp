#include "CutAndSolveWorker.h"

//------------------------------------------------------------------------------
//    Constructor
//------------------------------------------------------------------------------
CutAndSolveWorker::CutAndSolveWorker(const CSFS_Data &_data) : data(&_data),
                                                            world_rank(Parallel::getWorldRank()),
                                                            ss(_data),
                                                            cutToSolve(data->numStates),
                                                            lb(data->STARTING_LOWER_BOUND),
                                                            end_(false)
{

}


//------------------------------------------------------------------------------
// Returns whether or not the worker has received a signal to end
//------------------------------------------------------------------------------
bool CutAndSolveWorker::end() const
{
  return end_;
}


//------------------------------------------------------------------------------
// Receives the sparse problem to solve
//------------------------------------------------------------------------------
inline void CutAndSolveWorker::receiveProblem()
{
  std::size_t numCuts;
  std::vector<char> cutCharVec(data->numStates);
  std::vector<char> temp(data->numStates);
  MPI_Status status;

  // *
  // * Check if received a signal to end
  // *
  MPI_Probe(0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
  if (status.MPI_TAG == Parallel::CONVERGE_TAG)
  {
    char signal;
    MPI_Recv(&signal, 1, MPI_CHAR, 0, Parallel::CONVERGE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    end_ = true;

    #ifndef NDEBUG
      std::cout << "Rank_" << world_rank << " received signal to end" << std::endl;
    #endif

    return;
  }

  // *
  // * Receive the problem
  // *
  #ifndef NDEBUG
    std::cout << "Rank_" << world_rank << " about to receive sparse problem" << std::endl;
  #endif

  MPI_Recv(&lb, 1, MPI_DOUBLE, 0, Parallel::SPARSE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

  //ss.setThreshold(data->USE_SOLUTION_POOL_THRESHOLD? std::min(lb, data->SOLUTION_POOL_THRESHOLD): lb);
  ss.setThreshold(lb);
  //ss.setThreshold(0);
  #ifndef NDEBUG
    std::cout << "Rank_" << world_rank << " received lower bound of " << lb << std::endl;
  #endif

  MPI_Recv(&cutCharVec[0], cutCharVec.size(), MPI_CHAR, 0, Parallel::SPARSE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  cutToSolve.set(cutCharVec);
  ss.setCutToSolve(cutToSolve);
  
  #ifndef NDEBUG
    std::cout << "Rank_" << world_rank << " received the cut" << std::endl;
  #endif

  MPI_Recv(&numCuts, 1, CUSTOM_SIZE_T, 0, Parallel::SPARSE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

  #ifndef NDEBUG
    if (numCuts > 0)
      std::cout << "Rank_" << world_rank << " about to receive the cut set" << std::endl;
  #endif

  for (std::size_t i = 0; i < numCuts; ++i)
  {
    MPI_Recv(&temp[0], temp.size(), MPI_CHAR, 0, Parallel::SPARSE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    ss.addToCutSet(Cut(temp));
  }

  #ifndef NDEBUG
    if (numCuts > 0)
      std::cout << "Rank_" << world_rank << " received the cut set " << std::endl;
  #endif

  MPI_Recv(&temp[0], temp.size(), MPI_CHAR, 0, Parallel::SPARSE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

  #ifndef NDEBUG
    std::cout << "Rank_" << world_rank << " received the vector of markers" << std::endl;
  #endif

  for (std::size_t i = 0; i < temp.size(); ++i)
  {
    assert(temp[i] == 0 || temp[i] == 1 || temp[i]==2);
    ss.setMark(i, temp[i]);
    
  }

  temp.resize(data->numIndiv);
  MPI_Recv(&temp[0], temp.size(), MPI_CHAR, 0, Parallel::SPARSE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

  #ifndef NDEBUG
    std::cout << "Rank_" << world_rank << " received the vector of individuals\n"
              << "Rank_" << world_rank << " received all info from controller" << std::endl;
  #endif

  for (std::size_t i = 0; i < temp.size(); ++i)
  {
    assert(temp[i] == 0 || temp[i] == 1 || temp[i] == 2);
    ss.setIndiv(i, temp[i]);    
  }
}


//------------------------------------------------------------------------------
// Sends the solution to the sparse problem back to the controller
//------------------------------------------------------------------------------
inline void CutAndSolveWorker::sendBackSolution()
{
  // *
  // * Get the data
  // *
  const std::vector<Solution> solutionPool = ss.getSolutionPool();
  const std::size_t numSol = solutionPool.size();    
  const double runTime = ss.getCpuTimeToSolve();

  // *
  // * Send back the data
  // *
  #ifndef NDEBUG
    std::cout << "Rank_" << world_rank << " about to send back sparse problem" << std::endl;
  #endif

  #ifndef NDEBUG
    std::cout << "Rank_" << world_rank << " about to send back " << numSol << " solution vectors" << std::endl;
  #endif
  
  MPI_Send(&numSol, 1, CUSTOM_SIZE_T, 0, Parallel::SPARSE_TAG, MPI_COMM_WORLD);
  
  for (std::size_t i = 0; i < numSol; ++i)
  {
    MPI_Send(&solutionPool[i].objValue, 1, MPI_DOUBLE, 0, Parallel::SPARSE_TAG, MPI_COMM_WORLD);

    #ifndef NDEBUG
      std::cout << "Rank_" << world_rank << " sent back objective value of " << solutionPool[i].objValue << std::endl;
    #endif

    MPI_Send(&solutionPool[i].markerStates[0], data->setSize, CUSTOM_SIZE_T, 0, Parallel::SPARSE_TAG, MPI_COMM_WORLD);
  }
  
  #ifndef NDEBUG
    std::cout << "Rank_" << world_rank << " sent back the solution vectors" << std::endl;
  #endif

  MPI_Send(&runTime, 1, MPI_DOUBLE, 0, Parallel::SPARSE_TAG, MPI_COMM_WORLD);

  #ifndef NDEBUG
    std::cout << "Rank_" << world_rank << " sent back the run time" << std::endl;
    std::cout << "Rank_" << world_rank << " sent back all info to the controller" << std::endl;
  #endif
}


//------------------------------------------------------------------------------
// Receives a problem (or a signal to end), solves the sparse problem, and sends
// back the solution
//------------------------------------------------------------------------------
void CutAndSolveWorker::work()
{
  #ifndef NDEBUG
    std::cout << "Rank_" << world_rank << " about to receive sparse problem" << std::endl;
  #endif

  receiveProblem();
  if (end_)
    return;

  #ifndef NDEBUG
    std::cout << "Rank_" << world_rank << " about to solve sparse problem" << std::endl;
  #endif

  ss.solve();

  #ifndef NDEBUG
    std::cout << "Rank_" << world_rank << " finished sparse problem and about to send back solution" << std::endl;
  #endif

  sendBackSolution();

  #ifndef NDEBUG
    std::cout << "Rank_" << world_rank << " sent back solution" << std::endl;
  #endif
}


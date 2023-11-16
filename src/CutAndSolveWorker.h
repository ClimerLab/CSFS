#ifndef CNS_WORKER_H
#define CNS_WORKER_H

#include "Parallel.h"
#include "SparseSolver.h"

class CutAndSolveWorker
{
  private:
    const CSFS_Data *data;
    std::size_t world_rank;
    SparseSolver ss;

    Cut cutToSolve;

    double lb;
    bool end_;

    void receiveProblem();
    void sendBackSolution();

  public:
    CutAndSolveWorker(const CSFS_Data &);
    bool end() const;
    void work();
};

#endif


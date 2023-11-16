#ifndef SPARSE_SOLVER_H
#define SPARSE_SOLVER_H

#include <ilcplex/ilocplex.h>
#include <ilconcert/ilomodel.h>
#include "CutSet.h"
#include "CSFS.h"
#include "Timer.h"

class SparseSolver
{
  private:
    const CSFS_Data *data;
    std::vector<Cut> cutSet;
    Cut cutToSolve;
    Cut newCut;

    std::vector<std::size_t> markVals;
    std::vector<std::size_t> indVals;
    std::vector<std::size_t> indivEquals;
    std::vector<std::size_t> numMarkersInCutToSolve;
    std::vector<std::size_t> numGrpOneCarrying;
    std::vector<double> lpMarkVals;
    std::size_t numGrpTwoFullCutToSolve;
    
    double objValue;
    std::vector<Solution> solutionPool;
    std::vector<double> pattern;

    double threshold;

    Timer timer;

    std::vector<std::size_t> getSolution() const;
    
    void countNumberOfMarkersInCutToSOlve();
    bool setIndividualsToZeroOrOne();
    bool setMarkersToZero();
    bool setIndividualEqualityConstraints();
    double solveLP();
    void solveMIP(const Cut&);
    
  public:
    SparseSolver(const CSFS_Data &);
    void setCutToSolve(const Cut &);
    void addToCutSet(const Cut&);
    void setMark(const std::size_t, const std::size_t);
    void setIndiv(const std::size_t, const std::size_t);
    void setThreshold(const double);
    void solve();
    std::vector<Solution> getSolutionPool() const;
    double getObjValue() const;
    double getCpuTimeToSolve() const;
    void roundExtremeValues(std::vector<double> *vec);
};

#endif
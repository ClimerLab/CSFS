#ifndef COVER_LP_SOLVER_H
#define COVER_LP_SOLVER_H

#include <ilcplex/ilocplex.h>
#include <ilconcert/ilomodel.h>

#include "Cut.h"
#include "Individual.h"
#include "CSFS_Data.h"
#include "Timer.h"

class CoverLpSolver
{
  private:
    const CSFS_Data *data;

    // Cplex items
    IloEnv env;
    std::vector<IloCplex> cplex;
    std::vector<IloModel> model;
    IloNumVarArray mark;
    IloNumArray markCopy;
    IloConstraintArray cutConstraints;
    IloConstraintArray fixedMarkConstraints;
    IloConstraintArray fixedIndivConstraints;

    bool cutConstraintsUpdated;
    bool fixedMarkConstraintsUpdated;
    bool fixedIndivConstraintsUpdated;

    std::vector<Individual> individuals;
    Timer timer;

    void buildModels();

  public:
    CoverLpSolver();
    CoverLpSolver(const CSFS_Data &);
    void add(const Cut &);
    double getCpuTimeToSolve() const;
    std::vector<std::pair<std::size_t, double> > getMarkVals(const std::size_t) const;
    std::size_t getNumNonzeroMarkers(const std::size_t) const;
    double getObjValue(const std::size_t) const;
    double getWallTimeToSolve() const;
    void setIndiv(const std::size_t, const bool);
    void setMark(const std::size_t, const bool);
    void solve();
};

#endif


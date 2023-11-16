#ifndef RELAXATION_SOLVER_H
#define RELAXATION_SOLVER_H

#include <ilcplex/ilocplex.h>
#include <ilconcert/ilomodel.h>
#include <iterator>
#include "CutSet.h"
#include "CSFS_Data.h"
#include "Solution.h"
#include "Timer.h"

class RelaxationSolver
{
  private:
    const CSFS_Data *data;
    CutSet cutSet;

    double objValue;

    std::vector<std::pair<std::size_t, double> > markVals;
    std::vector<std::pair<std::size_t, double> > indivVals;

    // Cplex items
    IloEnv env;
    IloCplex cplex;
    IloModel model;
    IloNumVarArray mark;
    IloNumVarArray indiv;
    IloNumArray markCopy;
    IloNumArray indivCopy;
    IloExpr obj;
    IloConstraintArray baseConstraints;
    IloConstraintArray cutConstraints;
    IloConstraintArray fixedMarkConstraints;
    IloConstraintArray fixedIndivConstraints;
    IloConstraintArray indivEqualityConstraints;

    Timer timer;

    void buildModel();
    void cleanUp();
    void roundExtremeValues(std::vector<std::pair<std::size_t, double> > *);

  public:
    RelaxationSolver(const CSFS_Data &);
    void add(const Cut &);
    double getCpuTimeToSolve() const;
    std::vector<std::pair<std::size_t, double> > getIndivVals() const;
    Solution getIntegralSolution() const;
    std::vector<std::pair<std::size_t, double> > getMarkVals() const;
    double getObjValue() const;
    std::string getStringOfRelaxationValues() const;
    double getWallTimeToSolve() const;
    bool integral() const;
    void setMark(const std::size_t, const bool);
    void setIndiv(const std::size_t, const bool);
    void setIndivEquality(const std::size_t, const std::size_t);
    void solve();
};

#endif


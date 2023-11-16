#ifndef CNS_CONTROLLER_H
#define CNS_CONTROLLER_H

#include <boost/multiprecision/cpp_dec_float.hpp>

#include "CutCreator.h"
#include "CutfileReader.h"
#include "CSFS.h"
#include "Parallel.h"
#include "RelaxationSolver.h"
#include "Solution.h"
#include "VariableEqualities.h"

class CutAndSolveController
{
  private:
    const CSFS_Data *data;
    CutCreator cc;
    RelaxationSolver rs;
    CutSet cutSet;
    CutfileReader cutfileReader;
    
    std::size_t world_size;
    std::size_t iter;

    double lb;
    double ub;

    std::stack<int> availableWorkers;
    std::set<int> unavailableWorkers;

    std::vector<Marker> markers;
    std::vector<Individual> individuals;

    VariableEqualities individualEqualities;

    std::ofstream logfile;
    
    double totalSparseTime;
    std::set<std::size_t> checkIn;
    
    void readCompletedCutsFromCutfile();
    void receiveCompletion();
    void sendProblem(const Cut &);
    void sendProblems(Cut);
    bool setIndiv(const std::size_t, const bool);
    bool setIndividualEqualityConstraints();
    bool setIndividualsToZero();
    bool setIndividualsToZeroOrOne();
    bool setMark(const std::size_t, const bool);
    bool setMarkersToZero();

  public:
    CutAndSolveController(const CSFS_Data &);
    bool converged() const;
    double getLb() const;
    std::string getStringOfUnavailableWorkers() const;
    double getUb() const;
    std::size_t numWorkersWorking() const;
    void signalWorkersToEnd();
    bool workersStillWorking() const;
    void waitForWorkers();
    void work();

};

#endif


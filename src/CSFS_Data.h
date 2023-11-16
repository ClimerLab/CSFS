#ifndef CSFS_DATA_H
#define CSFS_DATA_H

#include <string>
#include <vector>

#include "ConfigParser.h"
#include "Timer.h"
#include "CSFS_Utils.h"
const std::size_t STRSIZE = 1024;

class CSFS_Data
{
private:

	Timer timer;

	std::string determineLogfileName() const;
	std::string determineOutputCutfileName() const;
	std::size_t getIdColNum() const;
	void readInput();

public:
	const tm* startTime;
	const ConfigParser parser;
	const std::string configFilename;

	// Parameters
	const double STARTING_LOWER_BOUND;
	const double STARTING_UPPER_BOUND;
	const bool USE_SOLUTION_POOL_THRESHOLD;
	const double SOLUTION_POOL_THRESHOLD;
	const bool RISK;
	const bool QUIET;
	const bool VERBOSE;
	const bool PRINT_CPLEX_OUTPUT;
	const double TOL;	
	std::string ID_PREFIX;
	std::string MISSING_SYMBOL;
  const std::size_t CPLEX_SEED;
  const bool USE_LOWER_CUTOFF;
  const bool USE_SPARSE_CONTRAINTS;	

	const std::string inputFilename;
	const std::size_t numActualExprs;
	const std::size_t setSize;
	const std::size_t numCase;
	const std::size_t numCtrl;

	const std::size_t numHeadRows;
	const std::size_t numHeadCols;

	const std::string logfileName;
	const std::string inputCutfileName;
	const bool inputCutfileProvided;
	const std::string outputCutfileName;	

	const std::size_t numBins;
  const std::size_t numStates;
	const std::size_t numIndiv;

	const std::size_t numGrpOne; // Cases if looking for risk pattern, controls otherwise
	const std::size_t numGrpTwo; // Controls if looking for risk pattern, cases otherwise
	const std::size_t grpOneStart;
	const std::size_t grpOneEnd;
	const std::size_t grpTwoStart;
	const std::size_t grpTwoEnd;

	const std::size_t idColNum;
 
 	const bool USE_HIGH;
	const bool USE_NORM;
	const bool USE_LOW;
	const bool USE_NOT_LOW;
	const bool USE_NOT_HIGH;
	const bool SET_NA_TUE;

	const double HIGH_VALUE;
	const double NORM_VALUE;
	const double LOW_VALUE;
	const double NOT_LOW_VALUE;
	const double NOT_HIGH_VALUE;

	std::vector<std::vector<std::string>> exprsInfo;
	std::vector<std::vector<bool>> exprs;
	std::vector<std::vector<double>> boundaries;

	CSFS_Data(const std::string &);

	void checkParameters() const;
	double elapsed_cpu_time() const;
	double elapsed_wall_time() const;
	std::string exprMatrixString() const;
	std::size_t maxNumCuts(const double) const;
	std::string exprInfoString() const;

	std::size_t getHighIndex() const;
	std::size_t getNormIndex() const;
	std::size_t getLowIndex() const;
 	std::size_t getNotLowIndex() const;
	std::size_t getNotHighIndex() const;
};

#endif // !CSFS_DATA_H




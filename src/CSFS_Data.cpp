#include "CSFS_Data.h"
#include <limits>

CSFS_Data::CSFS_Data(const std::string &configFile) :	timer(true),
																										startTime(timer.current_time()),
																										parser(configFile),
																										configFilename(configFile),
																										STARTING_LOWER_BOUND(parser.getDouble("STARTING_LOWER_BOUND")),
																										STARTING_UPPER_BOUND(parser.getDouble("STARTING_UPPER_BOUND")),
																										USE_SOLUTION_POOL_THRESHOLD(parser.getBool("USE_SOLUTION_POOL_THRESHOLD")),
																										SOLUTION_POOL_THRESHOLD(parser.getDouble("SOLUTION_POOL_THRESHOLD")),
																										RISK(parser.getBool("RISK")),
																										QUIET(parser.getBool("QUIET")),
																										VERBOSE(parser.getBool("VERBOSE")),
																										PRINT_CPLEX_OUTPUT(parser.getBool("PRINT_CPLEX_OUTPUT")),
																										TOL(parser.getDouble("TOL")),
																										ID_PREFIX(parser.getString("ID_PREFIX")),
																										MISSING_SYMBOL(parser.getString("MISSING_SYMBOL")),
                          													CPLEX_SEED(parser.getSizeT("CPLEX_SEED")),
                          													USE_LOWER_CUTOFF(parser.getBool("USE_LOWER_CUTOFF")),
                          													USE_SPARSE_CONTRAINTS(parser.getBool("USE_SPARSE_CONTRAINTS")),
																										inputFilename(parser.getString("DATA_FILE")),
																										numActualExprs(parser.getSizeT("NUM_EXPRS")),
																										setSize(parser.getSizeT("PATTERN_SIZE") >= 1 && parser.getSizeT("PATTERN_SIZE") <= 2 * numActualExprs ? parser.getSizeT("PATTERN_SIZE") : 1),
																										numCase(parser.getSizeT("NUM_CASES")),
																										numCtrl(parser.getSizeT("NUM_CTRLS")),
																										numHeadRows(parser.getSizeT("NUM_HEAD_ROWS")),
																										numHeadCols(parser.getSizeT("NUM_HEAD_COLS")),
																										logfileName(determineLogfileName()),
														           							numBins(parser.getSizeT("NUM_BINS")),
																										numStates(numBins * numActualExprs),
																										numIndiv(numCase + numCtrl),
																										numGrpOne(RISK ? numCase : numCtrl),
																										numGrpTwo(RISK ? numCtrl : numCase),
																										grpOneStart(RISK ? 0 : numCase),
																										grpOneEnd(RISK ? numCase - 1 : numCase + numCtrl - 1),
																										grpTwoStart(RISK ? numCase : 0),
																										grpTwoEnd(RISK ? numCase + numCtrl - 1 : numCase - 1),
																										idColNum(getIdColNum()),
																										USE_HIGH(parser.getBool("USE_HIGH")),
																										USE_NORM(parser.getBool("USE_NORM")),
																										USE_LOW(parser.getBool("USE_LOW")),
																										USE_NOT_LOW(parser.getBool("USE_NOT_LOW")),
																										USE_NOT_HIGH(parser.getBool("USE_NOT_HIGH")),
																										SET_NA_TUE(parser.getBool("SET_NA_TRUE")),
																										HIGH_VALUE(parser.getDouble("HIGH_VALUE")),
																										NORM_VALUE(parser.getDouble("NORM_VALUE")),
																										LOW_VALUE(parser.getDouble("LOW_VALUE")),
                          													NOT_LOW_VALUE(parser.getDouble("NOT_LOW_VALUE")),
																										NOT_HIGH_VALUE(parser.getDouble("NOT_HIGH_VALUE")) {
	// Set up the expression matrix
	std::vector<bool> expressionRow(numIndiv, 0);
	for (std::size_t i = 0; i < numStates; ++i) {
		exprs.push_back(expressionRow);
	}

	// Set up the expression info matrix
	std::vector<std::string> exprInfoRow(numHeadCols, "");
	for (std::size_t i = 0; i < numActualExprs + 1; ++i) {
		exprsInfo.push_back(exprInfoRow);
	}

	std::vector<double> boundariesRow(2, 0.0);
	for (size_t i = 0; i < numStates; ++i) {
		boundaries.push_back(boundariesRow);
	}

	// Read the input data
	readInput(); 
}

//------------------------------------------------------------------------------
// Checks the validity of parameters in the config file
//------------------------------------------------------------------------------
void CSFS_Data::checkParameters() const
{
	if (setSize < 1 || setSize > 2 * numActualExprs)
		throw std::runtime_error("PATTERN_SIZE must be greater than or equal to 1, and less than or equal to twice the number of Expressions.");

	if (ID_PREFIX.empty())
		throw std::runtime_error("A value must be given for ID_PREFIX.");

	if (STARTING_UPPER_BOUND <= STARTING_LOWER_BOUND)
		throw std::runtime_error("STARTING_LOWER_BOUND must be less than STARTING_UPPER_BOUND.");
	if (STARTING_LOWER_BOUND < 0 || STARTING_LOWER_BOUND >= 1)
		throw std::runtime_error("STARTING_LOWER_BOUND must be on the interval [0,1).");
	if (STARTING_UPPER_BOUND > 1 || STARTING_UPPER_BOUND <= 0)
		throw std::runtime_error("STARTING_UPPER_BOUND must be on the interval (0, 1].");


	if (USE_SOLUTION_POOL_THRESHOLD && SOLUTION_POOL_THRESHOLD < 0)
		throw std::runtime_error("SOLUTION_POOL_THRESHOLD must be positive.");
	if (USE_SOLUTION_POOL_THRESHOLD && SOLUTION_POOL_THRESHOLD < 0.1)
		CSFSUtils::warning("SOLUTION_POOL_THRESHOLD may be too low.");
	if (USE_SOLUTION_POOL_THRESHOLD && SOLUTION_POOL_THRESHOLD > 0.6)
		CSFSUtils::warning("SOLUTION_POOL_THRESHOLD may be too high");

	if(USE_SOLUTION_POOL_THRESHOLD && USE_LOWER_CUTOFF)
		throw std::runtime_error("USE_SOLUTION_POOL_THRESHOLD and USE_LOWER_CUTOFF cannot both be true.");

	if (QUIET && VERBOSE)
		throw std::runtime_error("QUIET and VERBOSE cannot both be true.");

	if (TOL <= 0)
		throw std::runtime_error("TOL must be a positive number.");
	if (TOL >= 1e-2)
		CSFSUtils::warning("TOL may be too large.");

	std::size_t num_bins_act = 0;
	if (USE_HIGH)
		num_bins_act++;
	if (USE_NORM)
		num_bins_act++;
	if (USE_LOW)
		num_bins_act++;
	if (USE_NOT_LOW)
		num_bins_act++;
	if (USE_NOT_HIGH)
		num_bins_act++;
	if (numBins != num_bins_act)
		throw std::runtime_error("The number of USE_* flags set true must equal NUM_BINS");
}


//------------------------------------------------------------------------------
// Determines and returns the logfile name
//------------------------------------------------------------------------------
inline std::string CSFS_Data::determineLogfileName() const
{
	#ifndef NDEBUG
		static bool first = true;
		if (first)
			first = false;
		else
			throw std::logic_error("CSFS_Data::determineLogfileName() may only be called once.");
	#endif
	

	// Get base file name
	const std::size_t positionOfLastForwardSlash = inputFilename.find_last_of('/');
	std::string base = inputFilename.substr(positionOfLastForwardSlash + 1);
	const std::size_t positionOfLastDot = base.find_last_of('.');
	base = base.substr(0, positionOfLastDot);

	
	// Get date portion of file name
	char dateString[19];
	sprintf(dateString, "%d-%02d-%02d--%02d%02d%02d", startTime->tm_year + 1900,
		startTime->tm_mon + 1,
		startTime->tm_mday,
		startTime->tm_hour,
		startTime->tm_min,
		startTime->tm_sec);
	dateString[18] = '\0';
	

	// Put the pieces together
	std::ostringstream name;
	name << base << "_pattSize" << setSize;
	if (RISK)
		name << "_risk_";
	else
		name << "_prot_";
	name << dateString << ".log";
	//name << "test.log";

	return name.str();
}

//------------------------------------------------------------------------------
// Determines and returns the output cutfile name
//------------------------------------------------------------------------------
inline std::string CSFS_Data::determineOutputCutfileName() const
{
#ifndef NDEBUG
	static bool first = true;
	if (first)
		first = false;
	else
		throw std::logic_error("CSFS_Data::determineOutputCutfileName() may only be called once.");
#endif

	// Get base file name
	const std::size_t positionOfLastForwardSlash = inputFilename.find_last_of('/');
	std::string base = inputFilename.substr(positionOfLastForwardSlash + 1);
	const std::size_t positionOfLastDot = base.find_last_of('.');
	base = base.substr(0, positionOfLastDot);

	
	// Get date portion of file name
	char dateString[19];
	sprintf(dateString, "%d-%02d-%02d--%02d%02d%02d", startTime->tm_year + 1900,
		startTime->tm_mon + 1,
		startTime->tm_mday,
		startTime->tm_hour,
		startTime->tm_min,
		startTime->tm_sec);
	dateString[18] = '\0';
	
	// Put the pieces together
	std::ostringstream name;
	name << base << "_pattSize" << setSize;
	if (RISK)
		name << "_risk_";
	else
		name << "_prot_";
	name << dateString << ".cuts";
	//name << "test.cuts";

	return name.str();
}


//------------------------------------------------------------------------------
// Returns the number of cpu seconds since the CSFS_Data object was created
//------------------------------------------------------------------------------
double CSFS_Data::elapsed_cpu_time() const
{
	return timer.elapsed_cpu_time();
}


//------------------------------------------------------------------------------
// Returns the number of wall seconds since the CSFS_Data object was created
//------------------------------------------------------------------------------
double CSFS_Data::elapsed_wall_time() const
{
	return timer.elapsed_cpu_time();
}

//------------------------------------------------------------------------------
// Returns the expression matrix as a string
//------------------------------------------------------------------------------
std::string CSFS_Data::exprMatrixString() const
{
	  std::ostringstream oss;
  for (std::size_t i = 0; i < exprs.size(); ++i)
  {
    oss << "State_" << i << ":";
    for (std::size_t j = 0; j < exprs[i].size(); ++j)
      oss << " " << static_cast<bool>(exprs[i][j]);
    oss << "\n";
  }
  return oss.str();
}


//------------------------------------------------------------------------------
// Returns the maximum number of cuts that should ever be in the cut set before
// cuts are created based on individuals. This is a minimum of 2 since that is
// the minimum needed to merge.
//------------------------------------------------------------------------------
std::size_t CSFS_Data::maxNumCuts(const double lb) const
{
  const double thresh = USE_SOLUTION_POOL_THRESHOLD ? SOLUTION_POOL_THRESHOLD : lb;
  return std::max(static_cast<std::size_t>(2), static_cast<std::size_t>(std::round(thresh * numGrpOne)));
}

std::string CSFS_Data::exprInfoString() const
{
  std::ostringstream oss;
  for (std::size_t i = 1; i < exprsInfo.size(); ++i)
  {
    oss << "EXPR_" << i - 1 << ": ";
    for (std::size_t j = 0; j < exprsInfo[i].size(); ++j)
      oss << " " << exprsInfo[i][j];
    oss << "\n";
  }
  return oss.str();
}


std::size_t CSFS_Data::getIdColNum() const
{
	return std::size_t();
}

void CSFS_Data::readInput()
{
	FILE* input;
	double exprs_data;

	if ((input = fopen(inputFilename.c_str(), "r")) == NULL)
		throw std::runtime_error("Input file could not be opened.");

	char strng[STRSIZE]; // temporary string storage

	// read in header rows
	// read in first header row and record expression header info
	for (std::size_t j = 0; j < numHeadCols; ++j)
	{
		fscanf(input, "%s", strng);
		exprsInfo[0][j] = strng;
	}

	for (std::size_t j = numHeadCols; j < numHeadCols + numIndiv; ++j)
		fscanf(input, "%s", strng); // disregard these

	// read in the rest of the header rows and disregard
	for (std::size_t i = 1; i < numHeadRows; ++i)
	{
		for (std::size_t j = 0; j < numHeadCols + numIndiv; ++j)
			fscanf(input, "%s", strng);
	}

	std::size_t statePtr = 0; // point to current state
	std::size_t highIndex = getHighIndex();
	std::size_t normIndex = getNormIndex();
	std::size_t lowIndex = getLowIndex();
	std::size_t notHighIndex = getNotHighIndex();
	std::size_t notLowIndex = getNotLowIndex();

	// read in data
	for (std::size_t i = 0; i < numActualExprs; ++i)
	{
		for (std::size_t j = 0; j < numHeadCols; ++j)
		{
			fscanf(input, "%s", strng); // read in and record header columns
			exprsInfo[i + 1][j] = strng;
		}
	
		for (std::size_t j = 0; j < numIndiv; ++j)
		{
			if (feof(input))
				throw std::runtime_error("Input file is missing data");

			fscanf(input, "%s", strng);

			// Check if string represents missing data
			if ((MISSING_SYMBOL.compare(strng) == 0) && (SET_NA_TUE == true))
			{
				if (USE_HIGH)
					exprs[statePtr + highIndex][j] = true;
				if (USE_NORM)
					exprs[statePtr + normIndex][j] = true;
				if (USE_LOW)
					exprs[statePtr + lowIndex][j] = true;
				if (USE_NOT_LOW)
					exprs[statePtr + notHighIndex][j] = true;
				if (USE_NOT_HIGH)
					exprs[statePtr + notLowIndex][j] = true;
			}
			else
			{
				exprs_data = atof(strng);
					
			  // Set bin values depending on percentile
			  if (USE_HIGH && (exprs_data == HIGH_VALUE))
				  exprs[statePtr + highIndex][j] = true;
        
			  if (USE_NORM && (exprs_data == NORM_VALUE))
				  exprs[statePtr + normIndex][j] = true;
        
			  if (USE_LOW && (exprs_data == LOW_VALUE))
				  exprs[statePtr + lowIndex][j] = true;
        
			  if (USE_NOT_LOW && (exprs_data == NOT_LOW_VALUE))
			    exprs[statePtr + notLowIndex][j] = true;
        
			  if (USE_NOT_HIGH && (exprs_data == NOT_HIGH_VALUE))
				  exprs[statePtr + notHighIndex][j] = true;
		  }
    }
    
		statePtr += numBins;
	}
}

std::size_t CSFS_Data::getHighIndex() const
{
	return 0;
}

std::size_t CSFS_Data::getNormIndex() const
{
	std::size_t index = 0;

	if (USE_NORM)
	{
		if (USE_HIGH)
			++index;		
	}

	return index;
}

std::size_t CSFS_Data::getLowIndex() const
{
	std::size_t index = 0;

	if (USE_LOW)
	{
		if (USE_HIGH)
			++index;

		if (USE_NORM)
			++index;
	}

	return index;
}

std::size_t CSFS_Data::getNotLowIndex() const
{
	std::size_t index = 0;

	if (USE_NOT_LOW)
	{
		if (USE_HIGH)
			++index;

		if (USE_NORM)
			++index;

		if (USE_LOW)
			++index;
	}

	return index;
}

std::size_t CSFS_Data::getNotHighIndex() const
{
	std::size_t index = 0;

	if (USE_NOT_HIGH)
	{
		if (USE_HIGH)
			++index;

		if (USE_NORM)
			++index;

		if (USE_LOW)
			++index;      

		if (USE_NOT_LOW)
			++index;
	}

	return index;
}



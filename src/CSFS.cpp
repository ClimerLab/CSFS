#include "CSFS.h"
#include <iomanip>
#include <iostream>

//------------------------------------------------------------------------------
// Returns the next coordinates by reference.
// coords is a vector of unsigned integers such as [0, 1, 2, 3].
// If enumerationNumStates is 6, then the next coordinates would be:
// [0, 1, 2, 4], [0, 1, 2, 5], [0, 1, 3, 4], [0, 1, 3, 5], [0, 1, 4, 5],
// [0, 2, 3, 4], etc.
//------------------------------------------------------------------------------
bool CSFS::getNextEnumerationCoords(std::vector<std::size_t> *coords,
                                    const std::size_t enumerationNumStates)
{
  if ((*coords)[0] == enumerationNumStates - coords->size())
  {
    // Edge case for when coords->size() == enumerationNumStates
    if (coords->size() > 1
    &&  coords->back() == (*coords)[coords->size() - 2])
    {
      ++(coords->back());
      return true;
    }
    else
    {
      return false;
    }
  }

  bool changeAllToTheRight = false;
  std::size_t i = coords->size() - 1;

  for (std::size_t j = 1;; --i, ++j)
  {
    if ((*coords)[i] < enumerationNumStates - j)
    {
      ++(*coords)[i];
      break;
    }
    else
    {
      changeAllToTheRight = true;
    }
  }

  if (changeAllToTheRight)
  {
    ++i;
    for (; i < coords->size(); ++i)
      (*coords)[i] = (*coords)[i - 1] + 1;
  }

  return true;
}


//------------------------------------------------------------------------------
// Makes sure coords is before endCoords, then gets the next coordinates
//------------------------------------------------------------------------------
bool CSFS::getNextEnumerationCoords(std::vector<std::size_t> *coords,
                                    const std::size_t enumerationNumStates,
                                    const std::vector<std::size_t> &endCoords)
{
  assert(coords->size() == endCoords.size());

  for (std::size_t i = 0; i < coords->size(); ++i)
  {
    if ((*coords)[i] < endCoords[i])
      return getNextEnumerationCoords(coords, enumerationNumStates);
    else if ((*coords)[i] > endCoords[i])
      return false;
  }

  return false;
}


//------------------------------------------------------------------------------
// Calculates and returns the objective value
//------------------------------------------------------------------------------
double CSFS::getObjectiveValue(const std::size_t numGrpOneWithPattern,
                               const std::size_t numGrpTwoWithPattern,
                               const CSFS_Data *data)
{
  const double grpOneRatio = static_cast<double>(numGrpOneWithPattern) / data->numGrpOne;
  const double grpTwoRatio = static_cast<double>(numGrpTwoWithPattern) / data->numGrpTwo;
  return grpOneRatio - grpTwoRatio;
}


//------------------------------------------------------------------------------
// Returns a string of info to be printed at the end of each iteration
//------------------------------------------------------------------------------
std::string CSFS::getStringOfEndOfIterInfo(const double upperBound,
                                           const double lowerBound,
                                           const double elapsedTime)
{
  std::ostringstream oss;
  oss << "Upper bound: " << upperBound
      << "\nLower bound: " << lowerBound
      << "\nElapsed CPU time: " << elapsedTime
      << " seconds";
  return oss.str();
}


//------------------------------------------------------------------------------
// Prints the solution to the console and the logfile
//------------------------------------------------------------------------------
void CSFS::printSolution(const Solution &solution,
                         std::ofstream *logfile,
                         const CSFS_Data *data)
{
  CSFS::printSolution(solution.markerStates, logfile, data);
}


//------------------------------------------------------------------------------
// Prints the solution to the console and the logfile
//------------------------------------------------------------------------------
void CSFS::printSolution(const std::vector<std::size_t> &solution,
                         std::ofstream *logfile,
                         const CSFS_Data *data)
{
  // *
  // * Count the number of individuals in each group with the pattern
  // *
  std::size_t numGrpOneWithPattern = data->numGrpOne;
  for (std::size_t j = data->grpOneStart; j <= data->grpOneEnd; ++j)
  {
    for (std::size_t i = 0; i < data->setSize; ++i)
    {
      if (data->exprs[solution[i]][j] != 1)
      {
        --numGrpOneWithPattern;
        break;
      }
    }
  }

  std::size_t numGrpTwoWithPattern = data->numGrpTwo;
  for (std::size_t j = data->grpTwoStart; j <= data->grpTwoEnd; ++j)
  {
    for (std::size_t i = 0; i < data->setSize; ++i)
    {
      if (data->exprs[solution[i]][j] != 1)
      {
        --numGrpTwoWithPattern;
        break;
      }
    }
  }

  // *
  // * Calculate ratios and objective value
  // *
  const double grpOneRatio = numGrpOneWithPattern / static_cast<double>(data->numGrpOne);
  const double grpTwoRatio = numGrpTwoWithPattern / static_cast<double>(data->numGrpTwo);
  const double objValue = grpOneRatio - grpTwoRatio;

  // *
  // * Build the output
  // *
  std::ostringstream consoleOutput;
  std::ostringstream logfileOutput;

  consoleOutput << std::setprecision(5)
                << "Pattern possessed by:\n\t"
                << numGrpOneWithPattern << " (" << grpOneRatio << ") "
                << (data->RISK ? "cases" : "controls")
                << "\n\t" << numGrpTwoWithPattern << " (" << grpTwoRatio << ") "
                << (data->RISK ? "controls" : "cases")
                << "\n\tobjective value: " << std::setprecision(9)
                << objValue << "\n\nExpressionData\tState\tUpperBound\tLowerBound\t";

  for (std::size_t i = 0; i < data->numHeadCols; ++i) // header row
    consoleOutput << data->exprsInfo[0][i] << "\t";

  std::size_t dummyCount = 0;

  std::size_t highIndex = data->getHighIndex();
  std::size_t normIndex = data->getNormIndex();
  std::size_t lowIndex = data->getLowIndex();
  std::size_t notLowIndex = data->getNotLowIndex();
  std::size_t notHighIndex = data->getNotHighIndex();

  for (std::size_t i = 0; i < data->setSize; ++i)
  {
    std::size_t exprsNumber = solution[i] / data->numBins; // number of bins based on config file

    if (data->exprsInfo[exprsNumber + 1][data->idColNum] == "dummy")
    {
      ++dummyCount;
    }
    else
    {
      consoleOutput << "\n" << exprsNumber + 1 << "\t";//"(" <<solution[i] << ")" << "\t";
      logfileOutput << data->exprsInfo[exprsNumber + 1][data->idColNum] << "\t";

      // Print out associated bounds
      const std::size_t exprsState = solution[i] % data->numBins; // number of bins based on config file

      // Check if state is for HIGH
      if ((data->USE_HIGH) && (exprsState == highIndex))
      {
          consoleOutput << "HIGH\t";
          consoleOutput << data->HIGH_VALUE << "\t";
          logfileOutput << "HIGH\t";
          logfileOutput << data->HIGH_VALUE << "\t";
      }
      // Check if state is for NORM
      if ((data->USE_NORM) && (exprsState == normIndex))
      {
          consoleOutput << "NORM\t";
          consoleOutput << data->NORM_VALUE << "\t";
          logfileOutput << "NORM\t";
          logfileOutput << data->NORM_VALUE << "\t";
      }
      // Check if state is for LOW
      if ((data->USE_LOW) && (exprsState == lowIndex))
      {
          consoleOutput << "LOW\t";
          consoleOutput << data->LOW_VALUE << "\t";
          logfileOutput << "LOW\t";
          logfileOutput << data->LOW_VALUE << "\t";
      }
      // Check if state is for NOT_LOW
      if ((data->USE_NOT_LOW) && (exprsState == notLowIndex))
      {
          consoleOutput << "NOT_LOW\t";
          consoleOutput << data->NOT_LOW_VALUE << "\t";
          logfileOutput << "NOT_LOW\t";
          logfileOutput << data->NOT_LOW_VALUE << "\t";
      }
      // Check if state is for NOT_HIGH
      if ((data->USE_NOT_HIGH) && (exprsState == notHighIndex))
      {
          consoleOutput << "NOT_HIGH\t";
          consoleOutput << data->NOT_HIGH_VALUE << "\t";
          logfileOutput << "NOT_HIGH\t";
          logfileOutput << data->NOT_HIGH_VALUE << "\t";
      }

      for (std::size_t j = 0; j < data->numHeadCols; ++j) // print out expression information
        consoleOutput << data->exprsInfo[exprsNumber + 1][j] << "\t";
    }
  }

  for (std::size_t i = 0; i < dummyCount; ++i)
    logfileOutput << "dummy\n";

  // *
  // * Print to the console
  // *
  if (!data->QUIET)
    std::cout << consoleOutput.str() << std::endl;

  // *
  // * Print to the logfile
  // *
  if (logfile->is_open())
  {
    *logfile << logfileOutput.str() << "\n";
    logfile->flush();
  }
}

#include "CutfileReader.h"

//------------------------------------------------------------------------------
//    Constructors
//------------------------------------------------------------------------------
// If there is an input cut file name stored in the CSFS_Data object, then that
// file will be opened
//------------------------------------------------------------------------------
CutfileReader::CutfileReader(const CSFS_Data &_data) : data(&_data),
                                                      fileIsOpen(false),
                                                      phase_(PENDING),
                                                      lastCutRead(0)
{
  if (!(data->inputCutfileName).empty())
    open(data->inputCutfileName);
}


//------------------------------------------------------------------------------
// Open a cutfile other than the one named in the CSFS_Data object
//------------------------------------------------------------------------------
CutfileReader::CutfileReader(const CSFS_Data &_data,
                             const char filename[]) : data(&_data),
                                                      fileIsOpen(false),
                                                      phase_(PENDING),
                                                      lastCutRead(0)
{
  open(filename);
}


//------------------------------------------------------------------------------
// Open a cutfile other than the one named in the CSFS_Data object
//------------------------------------------------------------------------------
CutfileReader::CutfileReader(const CSFS_Data &_data,
                             const std::string &filename) : data(&_data),
                                                            fileIsOpen(false),
                                                            phase_(PENDING),
                                                            lastCutRead(0)
{
  open(filename.c_str());
}


//------------------------------------------------------------------------------
// Closes the cutfile
//------------------------------------------------------------------------------
void CutfileReader::close()
{
  assert(fileIsOpen);

  cutfile.close();
  fileIsOpen = false;
}


//------------------------------------------------------------------------------
// Returns a code for the section the the line designates
//------------------------------------------------------------------------------
inline unsigned char CutfileReader::determineSection(const std::string &str) const
{
  if (str.substr(0, 4) == "Cut_")
    return CUT;
  else if (str.substr(0, 12) == "Markers_Set:")
    return MARKERS;
  else if (str.substr(0, 16) == "Individuals_Set:")
    return INDIVIDUALS;
  else if (str.substr(0, 12) == "Upper_Bound:")
    return UPPER_BOUND;
  else if (str.substr(0, 12) == "Lower_Bound:")
    return LOWER_BOUND;
  else if (str.substr(0, 21) == "START_COMPLETED_CUTS:")
    return START_COMPLETED_CUTS;
  else if (str.substr(0, 23) == "START_UNCOMPLETED_CUTS:")
    return START_UNCOMPLETED_CUTS;
  else
    return UNKNOWN;
}


//------------------------------------------------------------------------------
// Extracts the first number from a string.
// Examples:
//   extractFirstNumber("abc123def456") returns 123
//   extractFirstNumber("123abc456") returns 123
//   extractFirstNumber("123") returns 123
//   extractFirstNumber("") throws an error
//   extractFirstNumber("abc") throws an error
//------------------------------------------------------------------------------
inline std::size_t CutfileReader::extractFirstNumber(const std::string &str) const
{
  const std::size_t begIndex = str.find_first_of("1234567890");
  const std::size_t length = str.find_first_not_of("1234567890") - begIndex;

  std::stringstream sstream(str.substr(begIndex, length));
  std::size_t result;
  sstream >> result;
  return result;
}


//------------------------------------------------------------------------------
// Returns fileIsOpen
//------------------------------------------------------------------------------
bool CutfileReader::fileOpen() const
{
  return fileIsOpen;
}


//------------------------------------------------------------------------------
// Expects a string of a cut in the form: m(2) + m(3) + m(14) + ... m(20) <= x
// where x is 1 less than the pattern size (though this function only looks for
// the m values and doesn't care about the x).
//
// This function splits (by whitespace) the string into a vector, then interates
// over the vector. If the first two characters of an element are "m(", then it
// extracts the number bewteen the parantheses and adds it to the cut. The cut
// is returned.
//------------------------------------------------------------------------------
inline Cut CutfileReader::getCutFromString(const std::string &str) const
{
  Cut cut(data->numStates);
  std::vector<std::string> vec = splitString(str);
  for (std::size_t i = 0; i < vec.size(); ++i)
  {
    if (vec[i].substr(0, 2) == "m(")
      cut.add(extractFirstNumber(vec[i]));
  }
  return cut;
}


//------------------------------------------------------------------------------
// Returns the number of the last cut read
//------------------------------------------------------------------------------
std::size_t CutfileReader::getNumberOfLastCutRead() const
{
  return lastCutRead;
}


//------------------------------------------------------------------------------
// Returns true is the string contains only whitespace
//------------------------------------------------------------------------------
bool CutfileReader::isWhitespace(const std::string &str) const
{
  for (std::size_t i = 0; i < str.length(); ++i)
  {
    if (str[i] != ' '
    &&  str[i] != '\n'
    &&  str[i] != '\t'
    &&  str[i] != '\r'
    &&  str[i] != '\f'
    &&  str[i] != '\v')
    {
      return false;
    }
  }
  return true;
}


//------------------------------------------------------------------------------
// Loads the first line of the cut file ot process
//------------------------------------------------------------------------------
void CutfileReader::loadFirstLineToProcess()
{
  while (cutfile.good())
  {
    getline(cutfile, nextLineToProcess);
    const unsigned char section = determineSection(nextLineToProcess);

    if (section == START_COMPLETED_CUTS
    ||  section == START_UNCOMPLETED_CUTS)
    {
      updatePhase(section);
    }
    else if (!isWhitespace(nextLineToProcess)
         &&  nextLineToProcess.substr(0, 4) == "Cut_")
    {
      break;
    }
  }

  if (nextLineToProcess.substr(0, 4) != "Cut_")
    nextLineToProcess.clear();
}


//------------------------------------------------------------------------------
// Returns true if a cut was read, false otherwise.
//
// If a cut was read, it returns by reference:
// - the next cut from the cut file
// - any markers that were set as a result of adding the cut to the model
// - any individuals that were set as a result of adding the cut to the model
// - any individual equalities that were created as a result of adding the cut
//   to the model
// - the lower and upper bound if they were specified (otherwise default of 0
//   1 are set back respectively)
//------------------------------------------------------------------------------
bool CutfileReader::nextCut(Cut *cut,
                            std::vector<std::pair<std::size_t, bool> > *markers,
                            std::vector<std::pair<std::size_t, bool> > *individuals,
                            VariableEqualities *individualEqualities,
                            double *lb,
                            double *ub)
{
  cut->clear();
  markers->clear();
  individuals->clear();
  individualEqualities->clear();
  *lb = 0;
  *ub = 1;

  if (!nextCutAvail())
    return false;

  while (cutfile.good() || !nextLineToProcess.empty())
  {
    unsigned char section = determineSection(nextLineToProcess);
    if (section == CUT)
    {
      if (cut->empty())
      {
        *cut = getCutFromString(nextLineToProcess);
        lastCutRead = extractFirstNumber(CSFSUtils::getNthWord(nextLineToProcess, 1));
        nextLineToProcess.clear();
        getline(cutfile, nextLineToProcess);
      }
      else
      {
        break;
      }
    }
    else if (section == MARKERS)
    {
      processMarkers(markers);
    }
    else if (section == INDIVIDUALS)
    {
      processIndividuals(individuals, individualEqualities);
    }
    else if (section == UPPER_BOUND)
    {
      *ub = std::stod(CSFSUtils::getNthWord(nextLineToProcess, 2));
      getline(cutfile, nextLineToProcess);
    }
    else if (section == LOWER_BOUND)
    {
      *lb = std::stod(CSFSUtils::getNthWord(nextLineToProcess, 2));
      getline(cutfile, nextLineToProcess);
    }
    else if (section == START_COMPLETED_CUTS)
    {
      updatePhase(section);
      getline(cutfile, nextLineToProcess);
    }
    else if (section == START_UNCOMPLETED_CUTS)
    {
      updatePhase(section);
      getline(cutfile, nextLineToProcess);
      getline(cutfile, nextLineToProcess);
      break;
    }
    else
    {
      nextLineToProcess.clear();
      getline(cutfile, nextLineToProcess);
    }
  }

  if (!cut->empty())
    return true;
  else
    return false;
}


//------------------------------------------------------------------------------
// Returns true if the cutfile has another cut available, false otherwise.
//------------------------------------------------------------------------------
bool CutfileReader::nextCutAvail() const
{
  return (fileIsOpen && !nextLineToProcess.empty());
}


//------------------------------------------------------------------------------
// Opens the cutfile
//------------------------------------------------------------------------------
void CutfileReader::open(const char filename[])
{
  if (!fileIsOpen)
  {
    cutfile.open(filename);
    if (!cutfile.is_open())
      throw std::runtime_error("Input cutfile could not be opened");
    fileIsOpen = true;
    loadFirstLineToProcess();
  }
}


//------------------------------------------------------------------------------
// Opens the cutfile
//------------------------------------------------------------------------------
void CutfileReader::open(const std::string &filename)
{
  open(filename.c_str());
}


//------------------------------------------------------------------------------
// Reads and processes individual equality constraints, or constraints that fix
// individuals to be 0 or 1
//------------------------------------------------------------------------------
inline void CutfileReader::processIndividuals(std::vector<std::pair<std::size_t, bool> > *individuals,
                                       VariableEqualities *individualEqualities)
{
  if (CSFSUtils::getNthWord(nextLineToProcess, 4).substr(0, 2) == "i(")
  {
    const std::size_t indivId = extractFirstNumber(CSFSUtils::getNthWord(nextLineToProcess, 2));
    const std::size_t indivId2 = extractFirstNumber(CSFSUtils::getNthWord(nextLineToProcess, 4));
    individualEqualities->add(indivId, indivId2);
  }
  else
  {
    const std::size_t indivId = extractFirstNumber(CSFSUtils::getNthWord(nextLineToProcess, 2));
    const bool val = std::stod(CSFSUtils::getNthWord(nextLineToProcess, 4));
    individuals->emplace_back(std::make_pair(indivId, val));
  }

  nextLineToProcess.clear();
  getline(cutfile, nextLineToProcess);

  while (CSFSUtils::getNthWord(nextLineToProcess, 1).substr(0, 2) == "i(")
  {
    const std::size_t indivId = extractFirstNumber(CSFSUtils::getNthWord(nextLineToProcess, 1));
    if (CSFSUtils::getNthWord(nextLineToProcess, 3).substr(0, 2) == "i(")
    {
      const std::size_t indivId2 = extractFirstNumber(CSFSUtils::getNthWord(nextLineToProcess, 3));
      individualEqualities->add(indivId, indivId2);
    }
    else
    {
      const bool val = std::stod(CSFSUtils::getNthWord(nextLineToProcess, 3));
      individuals->emplace_back(std::make_pair(indivId, val));
    }
    nextLineToProcess.clear();
    getline(cutfile, nextLineToProcess);
  }
}


//------------------------------------------------------------------------------
// Reads and processes lines that fix markers to 0 or 1
//------------------------------------------------------------------------------
inline void CutfileReader::processMarkers(std::vector<std::pair<std::size_t, bool> > *markers)
{
  {
    const std::size_t markerId = extractFirstNumber(CSFSUtils::getNthWord(nextLineToProcess, 2));
    const bool val = std::stod(CSFSUtils::getNthWord(nextLineToProcess, 4));
    markers->emplace_back(std::make_pair(markerId, val));
  }

  nextLineToProcess.clear();
  getline(cutfile, nextLineToProcess);

  while (CSFSUtils::getNthWord(nextLineToProcess, 1).substr(0, 2) == "m(")
  {
    const std::size_t markerId = extractFirstNumber(CSFSUtils::getNthWord(nextLineToProcess, 1));
    const bool val = std::stod(CSFSUtils::getNthWord(nextLineToProcess, 3));
    markers->emplace_back(std::make_pair(markerId, val));
    nextLineToProcess.clear();
    getline(cutfile, nextLineToProcess);
  }
}


//------------------------------------------------------------------------------
// Split the string into a vector of strings
//------------------------------------------------------------------------------
inline std::vector<std::string> CutfileReader::splitString(const std::string &str) const
{
  std::stringstream sstream(str);
  std::istream_iterator<std::string> begin(sstream);
  std::istream_iterator<std::string> end;
  std::vector<std::string> vec(begin, end);
  return vec;
}


//------------------------------------------------------------------------------
// Updates the phase
//------------------------------------------------------------------------------
inline void CutfileReader::updatePhase(const unsigned char upcomingSection)
{
  assert(upcomingSection == START_COMPLETED_CUTS || upcomingSection == START_UNCOMPLETED_CUTS);
  assert(phase_ != READING_UNCOMPLETED_CUTS);

  if (upcomingSection == START_UNCOMPLETED_CUTS)
  {
    assert(phase_ == READING_COMPLETED_CUTS);
    phase_ = READING_UNCOMPLETED_CUTS;
  }
  else if (upcomingSection == START_COMPLETED_CUTS)
  {
    assert(phase_ == PENDING);
    phase_ = READING_COMPLETED_CUTS;
  }
}


//------------------------------------------------------------------------------
// Returns the current phase
//------------------------------------------------------------------------------
unsigned char CutfileReader::phase() const
{
  return phase_;
}


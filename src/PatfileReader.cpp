#include "PatfileReader.h"
#include "Cut.h"

//------------------------------------------------------------------------------
//    Constructors
//------------------------------------------------------------------------------
// If there is an input pattern file name stored in the CSFS_Data object, then
// that file will be opened
//------------------------------------------------------------------------------
PatternReader::PatternReader(const CSFS_Data &_data) :   data(&_data),
                                                        fileIsOpen(false),
                                                        lastPatRead(0)
{
  if (!(data->inputPatfileName).empty())
    open(data->inputPatfileName);
}

//------------------------------------------------------------------------------
// Returns true if the patfile has another pattern available, false otherwise.
//------------------------------------------------------------------------------
bool PatternReader::nextPatAvail() const
{
  return (fileIsOpen && !nextLineToProcess.empty());
}

//------------------------------------------------------------------------------
// Loads the first line of the pattern file to process
//------------------------------------------------------------------------------
void PatternReader::loadFirstLineToProcess()
{
    while (patfile.good())
    {
        getline(patfile, nextLineToProcess);
        
        if (!isWhitespace(nextLineToProcess))
        {
            break;
        }
    }  
}

Cut PatternReader::getPatFromString(const std::string &str) const
{
    Cut cut(data->numStates);
    std::vector<std::string> vec = splitString(str);
    std::size_t expr_num;

    // Check that pattern mathces the set size for CSFS
    if(vec.size() == NUM_ITEMS_PER_PAT_EL*data->setSize)
    {
      for (std::size_t i = 0; i < vec.size(); i+=3)
      {
        // Find index of expression data
        bool found_expr = false;
        // Note: The exprsInfo also contains hearer info, so need to add 1 to index
        for(expr_num = 0; expr_num < data->numActualExprs; ++expr_num)
        {
          if(data->exprsInfo[expr_num+1][data->idColNum] == vec[i])
          {
            found_expr = true;
            break;            
          }
        }

        if (found_expr)
        {
          if(atof(vec[i+2].c_str()) == data->HIGH_VALUE)
            cut.add(expr_num*data->numBins + data->getHighIndex());
          else if(atof(vec[i+2].c_str()) == data->NORM_VALUE)
            cut.add(expr_num*data->numBins + data->getNormIndex());
          else if(atof(vec[i+2].c_str()) == data->LOW_VALUE)
            cut.add(expr_num*data->numBins + data->getLowIndex());
          else if(atof(vec[i+2].c_str()) == data->NOT_LOW_VALUE)
            cut.add(expr_num*data->numBins + data->getNotLowIndex());
          else if(atof(vec[i+2].c_str()) == data->NOT_HIGH_VALUE)
            cut.add(expr_num*data->numBins + data->getNotHighIndex());
          else
          {
            printf("Warning - Could not find expression index for pattern\n");
            cut.clear();
            return cut;
          }
        }
        else
        {
          printf("WARNING - Could not find expression name for pattrn.\n");
          cut.clear();
          return cut;
        }
              
      }
    }
    return cut;
}

//------------------------------------------------------------------------------
// Split the string into a vector of strings
//------------------------------------------------------------------------------
inline std::vector<std::string> PatternReader::splitString(const std::string &str) const
{
  std::stringstream sstream(str);
  std::istream_iterator<std::string> begin(sstream);
  std::istream_iterator<std::string> end;
  std::vector<std::string> vec(begin, end);
  return vec;
}

//------------------------------------------------------------------------------
// Returns true is the string contains only whitespace
//------------------------------------------------------------------------------
bool PatternReader::isWhitespace(const std::string &str) const
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
// Opens the patfile
//------------------------------------------------------------------------------
void PatternReader::open(const char filename[])
{
  if (!fileIsOpen)
  {
    patfile.open(filename);
    if (!patfile.is_open())
      throw std::runtime_error("Input patfile could not be opened");
    fileIsOpen = true;
    loadFirstLineToProcess();
  }
}

//------------------------------------------------------------------------------
// Opens the patfile
//------------------------------------------------------------------------------
void PatternReader::open(const std::string &filename)
{
  open(filename.c_str());
}


//------------------------------------------------------------------------------
// Closes the patfile
//------------------------------------------------------------------------------
void PatternReader::close()
{
  assert(fileIsOpen);

  patfile.close();
  fileIsOpen = false;
}

//-----------------------------------------------------------------------------
// Returns the number of the last pattern read
//------------------------------------------------------------------------------
std::size_t PatternReader::getNumberOfLastPatRead() const
{
  return lastPatRead;
}

//------------------------------------------------------------------------------
// Returns true if a pattern was read, false otherwise.
//
// If a pattern was read, it returns by reference (as a cut):
// - the next cut from the cut file
bool PatternReader::nextPat(Cut *cut)
{
    cut->clear();

    if (!nextPatAvail())
        return false;

    while (patfile.good() || !nextLineToProcess.empty())
    {
        if (cut->empty())
        {
            *cut = getPatFromString(nextLineToProcess);
            ++lastPatRead;
            nextLineToProcess.clear();
            getline(patfile, nextLineToProcess);

            if(nextLineToProcess.empty())
              close();
        }
        else
        {
            break;
        }
    }

    if (!cut->empty())
        return true;
    else
        return false;
}

#include "Cut.h"
#include <cassert>

//------------------------------------------------------------------------------
//    Constructors
//------------------------------------------------------------------------------
// Default constructor
//------------------------------------------------------------------------------
Cut::Cut() : numMarkersInCut(0)
{}


//------------------------------------------------------------------------------
// Creates a cut of given size and sets all elements to the given initial state.
// If no initial state is provided, all items are set to false (i.e., not in the
// cut).
//------------------------------------------------------------------------------
Cut::Cut(const std::size_t _numElements, const bool initial) : cut(_numElements, initial),
                                                               numMarkersInCut(_numElements * initial)
{}


//------------------------------------------------------------------------------
// Creates a cut based on the given vector.
//------------------------------------------------------------------------------
Cut::Cut(const std::vector<bool> &vec) : cut(vec.size()),
                                         numMarkersInCut(0)
{
  for (std::size_t i = 0; i < cut.size(); ++i)
  {
    cut[i] = vec[i];
    if (vec[i])
      ++numMarkersInCut;
  }
}


//------------------------------------------------------------------------------
// Creates a cut based on the given vector.
//------------------------------------------------------------------------------
Cut::Cut(const std::vector<char> &vec) : cut(vec.size()),
                                         numMarkersInCut(0)
{
  for (std::size_t i = 0; i < cut.size(); ++i)
  {
    cut[i] = vec[i];
    if (vec[i])
      ++numMarkersInCut;
  }
}


//------------------------------------------------------------------------------
//    Copy constructor
//------------------------------------------------------------------------------
Cut::Cut(const Cut &other) : cut(other.getBoolVector()),
                             numMarkersInCut(other.size())
{}


//------------------------------------------------------------------------------
//    Copy assignment operator
//------------------------------------------------------------------------------
Cut & Cut::operator=(const Cut &other)
{
  cut = other.getBoolVector();
  numMarkersInCut = other.size();

  return *this;
}


//------------------------------------------------------------------------------
// Adds the marker at the given index to the cut. Does nothing if the marker was
// already in the cut. Returns true if the marker was added, false otherwise.
//------------------------------------------------------------------------------
bool Cut::add(const std::size_t i)
{
  if (!cut[i])
  {
    cut[i] = true;
    ++numMarkersInCut;
    return true;
  }

  return false;
}


//------------------------------------------------------------------------------
// Returns the cardinality of the intersection of the two cuts
//------------------------------------------------------------------------------
std::size_t Cut::cardinalityOfIntersection(const Cut &other) const
{
  assert(cut.size() == other.numElements());

  std::size_t cardinality = 0;

  if (numMarkersInCut < other.size())
  {
    for (std::size_t i = 0; i < cut.size(); ++i)
    {
      if (cut[i] && other[i])
        ++cardinality;
    }
  }
  else
  {
    for (std::size_t i = 0; i < cut.size(); ++i)
    {
      if (other[i] && cut[i])
        ++cardinality;
    }
  }

  return cardinality;
}


//------------------------------------------------------------------------------
// Removes all markers from the cut, but keeps the size of the vector the same.
//------------------------------------------------------------------------------
void Cut::clear()
{
  numMarkersInCut = 0;
  for (std::size_t i = 0; i < cut.size(); ++i)
    cut[i] = false;
}


//------------------------------------------------------------------------------
// Returns the 'distance' or similarity metric between two cuts. Increment the
// distance for each marker state that differs between the two cuts.
//------------------------------------------------------------------------------
std::size_t Cut::distance(const Cut &other) const
{
  assert(cut.size() == other.numElements());

  std::size_t x = 0;
  for (std::size_t i = 0; i < cut.size(); ++i)
  {
    if (cut[i] != other[i])
      ++x;
  }
  return x;
}


//------------------------------------------------------------------------------
// Returns whether or not the cut is empty.
//------------------------------------------------------------------------------
bool Cut::empty() const
{
  return (numMarkersInCut == 0);
}


//------------------------------------------------------------------------------
// Returns a string of 0's and 1's, whether or not each marker is in the cut.
//------------------------------------------------------------------------------
std::string Cut::getBinaryString() const
{
  std::ostringstream oss;
  for (std::size_t i = 0; i < cut.size(); ++i)
    oss << cut[i] << " ";
  return oss.str();
}

//------------------------------------------------------------------------------
// Returns the cut as a std::vector<bool>
//------------------------------------------------------------------------------
std::vector<bool> Cut::getBoolVector() const
{
  return cut;
}


//------------------------------------------------------------------------------
// Returns the cut as a vector of chars.
//------------------------------------------------------------------------------
std::vector<char> Cut::getCharVector() const
{
  std::vector<char> vec(cut.size());
  for (std::size_t i = 0; i < cut.size(); ++i)
    vec[i] = cut[i];
  return vec;
}


//------------------------------------------------------------------------------
// Returns the location of the true elements as vector
//------------------------------------------------------------------------------
std::vector<std::size_t> Cut::getTrueElements() const
{
   std::vector<std::size_t>	vec;
   for (std::size_t i = 0; i < cut.size(); ++i)
   {
      if(cut[i])
      {
		    vec.push_back(i);
      }
   }
  return vec;
}


//------------------------------------------------------------------------------
// Returns a string of the marker states that are in the cut.
//------------------------------------------------------------------------------
std::string Cut::getMarkerNumberString() const
{
  std::ostringstream oss;
  for (std::size_t i = 0; i < cut.size(); ++i)
  {
    if (cut[i])
      oss << i << " ";
  }
  return oss.str();
}


//------------------------------------------------------------------------------
// Returns the total number of elements in the cut array.
//------------------------------------------------------------------------------
std::size_t Cut::numElements() const
{
  return cut.size();
}


//------------------------------------------------------------------------------
// Removes the marker at the given index from the cut. Does nothing if the
// marker already wasn't in the cut. Returns true if the marker was removed,
// false otherwise.
//------------------------------------------------------------------------------
bool Cut::remove(const std::size_t i)
{
  assert(i < cut.size());

  if (cut[i])
  {
    cut[i] = false;
    --numMarkersInCut;
    return true;
  }

  return false;
}


//------------------------------------------------------------------------------
// Sets the element at the given index to the given state. Returns true if set,
// false otherwise.
//------------------------------------------------------------------------------
bool Cut::set(const std::size_t i, const bool state)
{
  assert(i < cut.size());

  if (cut[i] == 0 && state == 1)
  {
    cut[i] = 1;
    ++numMarkersInCut;
    return true;
  }
  else if (cut[i] == 1 && state == 0)
  {
    cut[i] = 0;
    --numMarkersInCut;
    return true;
  }

  return false;
}


//------------------------------------------------------------------------------
// Sets the elements of the cut to the elements of the given vector.
//------------------------------------------------------------------------------
void Cut::set(const std::vector<bool> &vec)
{
  assert(cut.size() == vec.size());

  numMarkersInCut = 0;
  for (std::size_t i = 0; i < cut.size(); ++i)
  {
    cut[i] = vec[i];
    if (cut[i])
      ++numMarkersInCut;
  }
}


//------------------------------------------------------------------------------
// Sets the elements of the cut to the elements of the given vector.
//------------------------------------------------------------------------------
void Cut::set(const std::vector<char> &vec)
{
  assert(cut.size() == vec.size());

  numMarkersInCut = 0;
  for (std::size_t i = 0; i < cut.size(); ++i)
  {
    cut[i] = vec[i];
    if (cut[i])
      ++numMarkersInCut;
  }
}


//------------------------------------------------------------------------------
// Changes the number of total elements to i
//------------------------------------------------------------------------------
void Cut::setNumElements(const std::size_t i)
{
  cut.resize(i, false);
}


//------------------------------------------------------------------------------
// Returns the number of markers in the cut (i.e., the number of elements that
// are true).
//------------------------------------------------------------------------------
std::size_t Cut::size() const
{
  return numMarkersInCut;
}


//------------------------------------------------------------------------------
// Overload ==
// If all the data members have the same value, the two are equal.
//------------------------------------------------------------------------------
bool Cut::operator==(const Cut &rhs) const
{
  if (numMarkersInCut != rhs.size()
  ||  cut.size() != rhs.numElements())
  {
    return false;
  }

  for (std::size_t i = 0; i < cut.size(); ++i)
  {
    if (cut[i] != rhs[i])
      return false;
  }

  return true;
}


//------------------------------------------------------------------------------
// Overload <
// Return true if the size of the cut is smaller than the right hand side.
//------------------------------------------------------------------------------
bool Cut::operator<(const Cut &rhs) const
{
  assert(cut.size() == rhs.numElements());

  if (numMarkersInCut == rhs.size())
  {
    for (std::size_t i = 0; i < cut.size(); ++i)
    {
      if (cut[i] != rhs[i])
        return cut[i] < rhs[i];
    }
  }
  else
  {
    return numMarkersInCut < rhs.size();
  }
  return false;
}


//------------------------------------------------------------------------------
// Overload >
// Return true if the size of the cut is larger than the right hand side.
//------------------------------------------------------------------------------
bool Cut::operator>(const Cut &rhs) const
{
  assert(cut.size() == rhs.numElements());

  if (numMarkersInCut == rhs.size())
  {
    for (std::size_t i = 0; i < cut.size(); ++i)
    {
      if (cut[i] != rhs[i])
        return cut[i] > rhs[i];
    }
  }
  else
  {
    return numMarkersInCut > rhs.size();
  }
  return false;
}


//------------------------------------------------------------------------------
// Returns whether or not the given element is in the cut.
//------------------------------------------------------------------------------
bool Cut::operator[](const std::size_t i) const
{
  assert(i < cut.size());

  return cut[i];
}


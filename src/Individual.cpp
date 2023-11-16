#include "Individual.h"

//------------------------------------------------------------------------------
//    Constructors
//------------------------------------------------------------------------------
Individual::Individual(const std::size_t _id,
                       const short _group,
                       const std::size_t _numRemainingMarkers) : id(_id),
                                                                 group(_group),
                                                                 one(false),
                                                                 zero(false),
                                                                 numRemainingMarkers(_numRemainingMarkers),
                                                                 numNonzeroMarkers(0),
                                                                 objValue(-1)
{
  assert(group == 1 || group == 2);
}

Individual::Individual(const std::size_t _id,
                       const short _group,
                       const std::size_t _numRemainingMarkers,
                       const bool _val)                        : id(_id),
                                                                 group(_group),
                                                                 one(_val),
                                                                 zero(!_val),
                                                                 numRemainingMarkers(_numRemainingMarkers),
                                                                 numNonzeroMarkers(0),
                                                                 objValue(-1)
{
  assert(group == 1 || group == 2);
}


//------------------------------------------------------------------------------
// Decrements numRemainingMarkers
//------------------------------------------------------------------------------
void Individual::decrementNumRemainingMarkers()
{
  assert(numRemainingMarkers > 0);
  --numRemainingMarkers;
}


//------------------------------------------------------------------------------
// Returns id
//------------------------------------------------------------------------------
std::size_t Individual::getId() const
{
  return id;
}


//------------------------------------------------------------------------------
// Returns markVals
//------------------------------------------------------------------------------
std::vector<std::pair<std::size_t, double> > Individual::getMarkVals() const
{
  return markVals;
}


//------------------------------------------------------------------------------
// Returns numNonzeroMarkers
//------------------------------------------------------------------------------
std::size_t Individual::getNumNonzeroMarkers() const
{
  return numNonzeroMarkers;
}


//------------------------------------------------------------------------------
// Returns numRemainingMarkers
//------------------------------------------------------------------------------
std::size_t Individual::getNumRemainingMarkers() const
{
  return numRemainingMarkers;
}


//------------------------------------------------------------------------------
// Returns the objective value.
//
// For group one individuals, this is the maximum value of the sum of their
// marker states, given the current constraints.
//
// For group two individuals, this is the minimum value of the sum of their
// marker states, given the current constraints.
//------------------------------------------------------------------------------
double Individual::getObjValue() const
{
  return objValue;
}


//------------------------------------------------------------------------------
// Increments numRemainingMarkers
//------------------------------------------------------------------------------
void Individual::incrementNumRemainingMarkers()
{
  ++numRemainingMarkers;
}


//------------------------------------------------------------------------------
// Returns whether or not the individual is in group one
//------------------------------------------------------------------------------
bool Individual::inGrpOne() const
{
  return (group == 1);
}


//------------------------------------------------------------------------------
// Returns whether or not the individual is in group two
//------------------------------------------------------------------------------
bool Individual::inGrpTwo() const
{
  return (group == 2);
}


//------------------------------------------------------------------------------
// Returns whether or not the individual has been set to 1
//------------------------------------------------------------------------------
bool Individual::isOne() const
{
  return one;
}


//------------------------------------------------------------------------------
// Returns whether or not the individual has been set to 0 or 1
//------------------------------------------------------------------------------
bool Individual::isSet() const
{
  return (zero || one);
}


//------------------------------------------------------------------------------
// Returns whether or not the individual has been set to 0
//------------------------------------------------------------------------------
bool Individual::isZero() const
{
  return zero;
}


//------------------------------------------------------------------------------
// Sets the individual to the given value.
//------------------------------------------------------------------------------
bool Individual::set(const bool val)
{
  assert(!(val == 1 && zero));
  assert(!(val == 0 && one));

  if (isSet())
    return false;

  if (val == 0)
    zero = true;
  else
    one = true;

  return true;
}


//------------------------------------------------------------------------------
// Sets markVals
//------------------------------------------------------------------------------
void Individual::setMarkVals(const std::vector<std::pair<std::size_t, double> > &_markVals)
{
  markVals = _markVals;
}


//------------------------------------------------------------------------------
// Sets numNonzeroMarkers
//------------------------------------------------------------------------------
void Individual::setNumNonzeroMarkers(const std::size_t _numNonzeroMarkers)
{
  numNonzeroMarkers = _numNonzeroMarkers;
}


//------------------------------------------------------------------------------
// Sets numRemainingMarkers
//------------------------------------------------------------------------------
void Individual::setNumRemainingMarkers(const std::size_t _numRemainingMarkers)
{
  numRemainingMarkers = _numRemainingMarkers;
}


//------------------------------------------------------------------------------
// Sets the individual's objective value.
//------------------------------------------------------------------------------
void Individual::setObjValue(const double val)
{
  objValue = val;
}


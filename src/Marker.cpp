#include "Marker.h"
#include <cassert>

//------------------------------------------------------------------------------
//    Constructors
//------------------------------------------------------------------------------
Marker::Marker(const std::size_t _id) : id(_id),
                                        numGrpOneCarrying(0),
                                        numGrpTwoCarrying(0),
                                        one(false),
                                        zero(false)
{}

Marker::Marker(const std::size_t _id, const bool _val) : id(_id),
                                                         numGrpOneCarrying(0),
                                                         numGrpTwoCarrying(0),
                                                         one(_val),
                                                         zero(!_val)
{}

Marker::Marker(const std::size_t _id,
               const std::size_t _numGrpOneCarrying,
               const std::size_t _numGrpTwoCarrying) : id(_id),
                                                       numGrpOneCarrying(_numGrpOneCarrying),
                                                       numGrpTwoCarrying(_numGrpTwoCarrying),
                                                       one(false),
                                                       zero(false)
{}

Marker::Marker(const std::size_t _id,
               const std::size_t _numGrpOneCarrying,
               const std::size_t _numGrpTwoCarrying,
               const bool _val)                      : id(_id),
                                                       numGrpOneCarrying(_numGrpOneCarrying),
                                                       numGrpTwoCarrying(_numGrpTwoCarrying),
                                                       one(_val),
                                                       zero(!_val)
{}


//------------------------------------------------------------------------------
// Decrements numGrpOneCarrying
//------------------------------------------------------------------------------
void Marker::decrementNumGrpOneCarrying()
{
  assert(numGrpOneCarrying > 0);
  --numGrpOneCarrying;
}


//------------------------------------------------------------------------------
// Decrements numGrpTwoCarrying
//------------------------------------------------------------------------------
void Marker::decrementNumGrpTwoCarrying()
{
  assert(numGrpTwoCarrying > 0);
  --numGrpTwoCarrying;
}


//------------------------------------------------------------------------------
// Increments numGrpOneCarrying
//------------------------------------------------------------------------------
void Marker::incrementNumGrpOneCarrying()
{
  ++numGrpOneCarrying;
}


//------------------------------------------------------------------------------
// Increments numGrpTwoCarrying
//------------------------------------------------------------------------------
void Marker::incrementNumGrpTwoCarrying()
{
  ++numGrpTwoCarrying;
}


//------------------------------------------------------------------------------
// Returns id
//------------------------------------------------------------------------------
std::size_t Marker::getId() const
{
  return id;
}


//------------------------------------------------------------------------------
// Returns numGrpOneCarrying
//------------------------------------------------------------------------------
std::size_t Marker::getNumGrpOneCarrying() const
{
  return numGrpOneCarrying;
}


//------------------------------------------------------------------------------
// Returns numGrpTwoCarrying
//------------------------------------------------------------------------------
std::size_t Marker::getNumGrpTwoCarrying() const
{
  return numGrpTwoCarrying;
}


//------------------------------------------------------------------------------
// Returns whether or not the marker has been fixed to one
//------------------------------------------------------------------------------
bool Marker::isOne() const
{
  return one;
}


//------------------------------------------------------------------------------
// Returns whether or not the marker has been set to a value
//------------------------------------------------------------------------------
bool Marker::isSet() const
{
  return (zero || one);
}


//------------------------------------------------------------------------------
// Returns whether or not the marker has been fixed to zero
//------------------------------------------------------------------------------
bool Marker::isZero() const
{
  return zero;
}


//------------------------------------------------------------------------------
// Sets the marker to the given value.
//------------------------------------------------------------------------------
bool Marker::set(const bool val)
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
// Sets numGrpOneCarrying
//------------------------------------------------------------------------------
void Marker::setNumGrpOneCarrying(const std::size_t num)
{
  numGrpOneCarrying = num;
}


//------------------------------------------------------------------------------
// Sets numGrpTwoCarrying
//------------------------------------------------------------------------------
void Marker::setNumGrpTwoCarrying(const std::size_t num)
{
  numGrpTwoCarrying = num;
}


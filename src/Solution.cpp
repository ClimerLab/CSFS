#include "Solution.h"

Solution::Solution() : objValue(0)
{}

Solution::Solution(const std::vector<std::size_t> &_markerStates,
                   const double _objValue)
                                                                  : markerStates(_markerStates),
                                                                    objValue(_objValue)
{}

bool Solution::operator<(const Solution &rhs) const
{
  return (objValue < rhs.objValue);
}

bool Solution::operator<(const double rhs) const
{
  return objValue < rhs;
}


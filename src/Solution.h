// *
// * Plain old data structure to hold a vector of marker states and a double
// * together, with some overridden < operators for sorting.
// *

#ifndef SOLUTION_H
#define SOLUTION_H

#include <vector>

class Solution
{
  public:
    std::vector<std::size_t> markerStates; // The marker states that make up the solution
    double objValue;                       // The objective value of the solution

    Solution();
    Solution(const std::vector<std::size_t> &, const double);
    bool operator<(const Solution &) const;
    bool operator<(const double) const;
};

#endif


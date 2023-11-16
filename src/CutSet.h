#ifndef CUT_SET_H
#define CUT_SET_H

#include <cmath>
#include <cstdlib>
#include <set>
#include <stack>

#include "Cut.h"

class CutSet
{
  private:
    std::set<Cut> cuts;
    std::set<std::size_t> markersInAllCuts;
    std::size_t maxSize_;
    std::size_t numElementsPerCut;

    std::size_t absoluteDifference(const std::size_t, const std::size_t) const;

  public:
    CutSet(const std::size_t);
    bool add(Cut);
    bool exists(const Cut &) const;
    bool exists(const std::vector<std::size_t> &) const;
    std::vector<std::vector<char> > get2dCharVector() const;
    std::set<std::size_t> getMarkersKeptInAllCuts() const;
    Cut getMergedCutContainingSmallest() const;
    Cut getMergedCutOfClosestPair() const;
    std::set<Cut> getRawSet() const;
    std::size_t greatestCardinalityOfIntersection(const Cut &) const;
    bool keepMarkerInAllCuts(const std::size_t);
    std::string matrixString() const;
    std::size_t maxSize() const;
    std::size_t numCuts() const;
};

#endif


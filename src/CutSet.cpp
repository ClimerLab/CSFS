#include "CutSet.h"

//------------------------------------------------------------------------------
//    Constructor
//------------------------------------------------------------------------------
CutSet::CutSet(const std::size_t _numElementsPerCut) : maxSize_(0),
                                                       numElementsPerCut(_numElementsPerCut)
{}


//------------------------------------------------------------------------------
// Utility function to find the absolute difference between two std::size_t
//------------------------------------------------------------------------------
inline std::size_t CutSet::absoluteDifference(const std::size_t a,
                                              const std::size_t b) const
{
  if (a > b)
    return a - b;
  else
    return b - a;
}


//------------------------------------------------------------------------------
// Adds the given cut to the set. Returns true if the cut was added, returns
// false if the given cut is a subset of another cut already in the set.
// If a cut already in the set is a subset of the given cut, the cut in the set
// is erased prior to adding the new cut.
//------------------------------------------------------------------------------
bool CutSet::add(Cut cut)
{
  // *
  // * Check if the given cut is a subset of any other cut in the set
  // *
  for (auto it = cuts.rbegin(); it != cuts.rend(); ++it)
  {
    if (cut.size() > it->size())
      break; // break because cuts are in order from largest to smallest

    for (std::size_t i = 0; i < numElementsPerCut; ++i)
    {
      if (cut[i] && !(*it)[i]) //!it->inCut(i)
        goto NEXT_CUT_A;
    }
    return false;
    NEXT_CUT_A:
    ;
  }

  // *
  // * Add markers in markersInAllCuts
  // *
  for (auto it = std::begin(markersInAllCuts); it != std::end(markersInAllCuts); ++it)
    cut.add(*it);

  // *
  // * Check if any cut in the set is a subset of the given cut
  // *
  {
    auto it = std::begin(cuts);
    while (it != std::end(cuts))
    {
      bool advance = true;
      if (it->size() >= cut.size())
        break; // break because cuts are in order from smallest to largest
      for (std::size_t i = 0; i < numElementsPerCut; ++i)
      {
        if ((*it)[i] && !cut[i])
          goto NEXT_CUT_B;
      }

      // *it is a subset of the given cut
      if (it != std::begin(cuts))
      {
        const auto prev = std::prev(it);
        cuts.erase(it);
        it = prev;
      }
      else
      {
        cuts.erase(it);
        it = std::begin(cuts);
        advance = false;
      }

      NEXT_CUT_B:
      if (advance)
        ++it;
    }
  }

  cuts.insert(cut); // Add the cut to the set
  maxSize_ = std::max(maxSize_, cut.size()); // update maxSize_
  return true;
}


//------------------------------------------------------------------------------
// Returns true if the given cut already exists in some form in the set
//------------------------------------------------------------------------------
bool CutSet::exists(const Cut &cut) const
{
  for (auto it = cuts.rbegin(); it != cuts.rend() && cut.size() <= it->size(); ++it)
  {
    for (std::size_t i = 0; i < numElementsPerCut; ++i)
    {
      if (cut[i] && !(*it)[i])
        goto NEXT_CUT;
    }

    return true;

    NEXT_CUT:
    ;
  }

  return false;
}


//------------------------------------------------------------------------------
// Returns true if the given cut already exists in some form in the set
//------------------------------------------------------------------------------
bool CutSet::exists(const std::vector<std::size_t> &vec) const
{
  for (auto it = cuts.rbegin(); it != cuts.rend(); ++it)
  {
    for (std::size_t i = 0; i < vec.size(); ++i)
    {
      if ( !((*it)[vec[i]]) )
        goto NEXT_CUT;
    }

    return true;

    NEXT_CUT:
    ;
  }

  return false;
}


//------------------------------------------------------------------------------
// Returns a 2d vector of chars of the cuts in the set. Each row is a cut. In a
// row, if an element is 0, that marker state is not in the cut. If an element
// is 1, that marker state is in the cut.
//------------------------------------------------------------------------------
std::vector<std::vector<char> > CutSet::get2dCharVector() const
{
  std::vector<std::vector<char> > vec(numCuts());
  auto it = std::begin(cuts);

  for (std::size_t i = 0; i < cuts.size(); ++i, ++it)
  {
    vec[i].resize(numElementsPerCut);
    for (std::size_t j = 0; j < numElementsPerCut; ++j)
      vec[i][j] = (*it)[j];
  }

  return vec;
}


//------------------------------------------------------------------------------
// Returns the set of markers shared by all cuts.
//------------------------------------------------------------------------------
std::set<std::size_t> CutSet::getMarkersKeptInAllCuts() const
{
  return markersInAllCuts;
}


//------------------------------------------------------------------------------
// Finds the smallest cut and its closest cut and returns their merged cut.
//------------------------------------------------------------------------------
Cut CutSet::getMergedCutContainingSmallest() const
{
  assert(cuts.size() >= 2);

  std::vector<Cut> smallestCuts; // Stores the smallest cuts in the cut set.
                                 // This will only contain a single cut if there
                                 // is only one cut in the set with the fewest
                                 // number of marker states.

  // *
  // * Find the smallest cuts
  // *
  {
    std::size_t smallestSize = 0;
    auto it = std::begin(cuts);

    smallestCuts.push_back(*it);
    smallestSize = it->size();
    ++it;

    for (; it != std::end(cuts); ++it)
    {
      assert(it->size() >= smallestSize);

      if (it->size() == smallestSize)
        smallestCuts.push_back(*it);
      else
        break;
    }
  }


  // *
  // * Find the two cuts that have the smallest distance between them, where
  // * at least one of these cuts must be part of the smallestCuts vector
  // *
  auto it = std::begin(cuts);
  Cut cut1 = smallestCuts[0];

  while (!it->distance(cut1))
    ++it;

  Cut cut2 = *it;
  ++it;

  std::size_t shortestDistance = cut1.distance(cut2);

  for (std::size_t i = 0; i < smallestCuts.size(); ++i)
  {
    for (; it != std::end(cuts); ++it)
    {
      const std::size_t distance = it->distance(smallestCuts[i]);
      if (distance == 0)
      {
        continue;
      }
      else if (distance < shortestDistance)
      {
        cut1 = smallestCuts[i];
        cut2 = *it;
        shortestDistance = distance;
      }
      else if (distance == shortestDistance && it->size() < cut2.size())
      {
        cut2 = *it;
      }
    }
  }

  // *
  // * Create the merged cut
  // *
  Cut mergedCut(numElementsPerCut);
  for (std::size_t i = 0; i < numElementsPerCut; ++i)
  {
    if (cut1[i] || cut2[i])
      mergedCut.add(i);
  }
  return mergedCut;
}


//------------------------------------------------------------------------------
// Finds the two closest cuts and returns the merged cut.
//------------------------------------------------------------------------------
Cut CutSet::getMergedCutOfClosestPair() const
{
  assert(cuts.size() >= 2);

  auto it = std::begin(cuts);
  auto it2 = std::next(it, 1);
  std::size_t distance = it->distance(*it2);
  Cut cut1 = *it;
  Cut cut2 = *it2;
  std::size_t sizeDiff = absoluteDifference(cut1.size(), cut2.size());

  std::advance(it2, 1);
  for (; it != std::end(cuts); ++it)
  {
    for (; it2 != std::end(cuts); ++it2)
    {
      const std::size_t temp = it->distance(*it2);

      if ((temp < distance)
      ||  (temp == distance && absoluteDifference(it->size(), it2->size()) < sizeDiff))
      {
        distance = temp;
        cut1 = *it;
        cut2 = *it2;
        sizeDiff = absoluteDifference(cut1.size(), cut2.size());
      }
    }
  }

  Cut mergedCut(numElementsPerCut);
  for (std::size_t i = 0; i < numElementsPerCut; ++i)
  {
    if (cut1[i] || cut2[i])
      mergedCut.add(i);
  }
  return mergedCut;
}


//------------------------------------------------------------------------------
// Returns a copy of the original std::set<Cut>
//------------------------------------------------------------------------------
std::set<Cut> CutSet::getRawSet() const
{
  return cuts;
}


//------------------------------------------------------------------------------
// Returns the greatest cardinality of intersection between the given cut and
// the CutSet. No comparisons are done between members in the CutSet.
//------------------------------------------------------------------------------
std::size_t CutSet::greatestCardinalityOfIntersection(const Cut &other) const
{
  std::size_t greatestCardinality = 0;

  for (auto it = std::begin(cuts); it != std::end(cuts); ++it)
    greatestCardinality = std::max(greatestCardinality, other.cardinalityOfIntersection(*it));

  return greatestCardinality;
}


//------------------------------------------------------------------------------
// Adds the marker to all the cuts and keeps the marker number in the
// markersInAllCuts set. Whenever a new cut is added, all markers in that set
// are automatically added to the new cut.
//------------------------------------------------------------------------------
bool CutSet::keepMarkerInAllCuts(const std::size_t markNum)
{
  const auto pair = markersInAllCuts.insert(markNum);
  const bool markerAlreadyInAllCuts = !pair.second;

  if (markerAlreadyInAllCuts)
    return false;

  std::stack<Cut> cutsToAddBack;

  {
    auto it = std::begin(cuts);
    while (it != std::end(cuts))
    {
      bool advance = true;

      if (!(*it)[markNum])
      {
        Cut cut(*it);
        cut.add(markNum);
        cutsToAddBack.push(cut);

        if (it != std::begin(cuts))
        {
          const auto prev = std::prev(it);
          cuts.erase(it);
          it = prev;
        }
        else
        {
          cuts.erase(it);
          it = std::begin(cuts);
          advance = false;
        }
      }

      if (cuts.empty())
        break;
      else if (advance)
        ++it;
    }
  }

  while ( !cutsToAddBack.empty() )
  {
    add(cutsToAddBack.top());
    cutsToAddBack.pop();
  }

  return true;
}


//------------------------------------------------------------------------------
// Returns the cut set matrix as a string. Each row is a cut, and each column
// is a marker state.
//------------------------------------------------------------------------------
std::string CutSet::matrixString() const
{
  std::ostringstream oss;
  for (auto it = std::begin(cuts); it != std::end(cuts); ++it)
  {
    if (it != std::begin(cuts))
      oss << "\n";
    oss << it->getBinaryString();
  }
  return oss.str();
}


//------------------------------------------------------------------------------
// Returns the size of the largest cut in the set.
//------------------------------------------------------------------------------
std::size_t CutSet::maxSize() const
{
  return maxSize_;
}


//------------------------------------------------------------------------------
// Returns the number of cuts in the set
//------------------------------------------------------------------------------
std::size_t CutSet::numCuts() const
{
  return cuts.size();
}


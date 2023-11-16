#include "CutCreator.h"

//------------------------------------------------------------------------------
//    Constructor
//------------------------------------------------------------------------------
CutCreator::CutCreator(const CSFS_Data &_data) : data(&_data)
{}


//------------------------------------------------------------------------------
// Creates and returns a cut to be solved. Returns by reference a boolean of
// whether or not the cut was created by an individual, and if it was, the
// number of the individual.
//------------------------------------------------------------------------------
Cut CutCreator::createCut(const CutSet &cutSet,
                          const std::vector<Marker> &markers,
                          const std::vector<Individual> &individuals,
                          const std::vector<std::pair<std::size_t, double> > &markVals,
                          const std::size_t maxNumCuts,
                          int *cutCreatedFrom,
                          std::size_t *indivCutWasBasedOn)
{
  static bool useIndivs = false;
  Cut cut(data->numStates);
  *indivCutWasBasedOn = data->numIndiv; // initially set to an invalid individual

  if (useIndivs || switchToIndivs(cutSet, individuals, &useIndivs))
  {
    cut = createCutFromIndividuals(markers, individuals, indivCutWasBasedOn);
    *cutCreatedFrom = INDIVIDUAL;
  }
  else
  {
    if (cutSet.numCuts() >= maxNumCuts)
    {
      cut = createCutFromMerging(cutSet);
      *cutCreatedFrom = MERGE;
    }
    else
    {
      cut = createCutFromRelaxation(cutSet, markers, markVals);
      *cutCreatedFrom = RELAXATION;
    }

    if (cut.size() >= minCutSizeToSwitch(individuals))
    {
      cut = createCutFromIndividuals(markers, individuals, indivCutWasBasedOn);
      *cutCreatedFrom = INDIVIDUAL;
      useIndivs = true;
    }
  }

  return cut;
}


//------------------------------------------------------------------------------
// Creates and returns a cut based on an individual. Also returns be reference
// the number of the individual the cut was based on.
//------------------------------------------------------------------------------
inline Cut CutCreator::createCutFromIndividuals(const std::vector<Marker> &markers,
                                                const std::vector<Individual> &individuals,
                                                std::size_t *indivCutWasBasedOn)
{
  Cut cut(data->numStates);

  // *
  // * Find the individual to base the cut on
  // *
  std::size_t ind = data->numIndiv;
  std::size_t min = data->numStates;

  for (std::size_t j = data->grpOneStart; j <= data->grpOneEnd; ++j)
  {
    if (!individuals[j].isZero()
    &&  individuals[j].getNumRemainingMarkers() < min)
    {
      min = individuals[j].getNumRemainingMarkers();
      ind = j;
    }
  }

  assert(ind != data->numIndiv);

  for (std::size_t i = 0; i < data->numStates; ++i)
  {
    if (data->exprs[i][ind] && !markers[i].isZero())
      cut.add(i);
  }

  *indivCutWasBasedOn = ind;
  return cut;
}


//------------------------------------------------------------------------------
// Creates a cut by merging the smallest cut with its closest cut
//------------------------------------------------------------------------------
inline Cut CutCreator::createCutFromMerging(const CutSet &cutSet) const
{
  return cutSet.getMergedCutOfClosestPair();
}


//------------------------------------------------------------------------------
// Creates a cut from the result of a relaxation. All nonzero markers from the
// relaxation are made into a cut. If this is a duplicate of a previous cut,
// then add random markers until it's no longer a duplicate.
//------------------------------------------------------------------------------
inline Cut CutCreator::createCutFromRelaxation(const CutSet &cutSet,
                                               const std::vector<Marker> &markers,
                                               std::vector<std::pair<std::size_t, double> > markVals) const
{
  // *
  // * Sort the relaxed marker values (highest to lowest)
  // *
  std::sort(std::begin(markVals),
            std::end(markVals),
            CSFSUtils::SortPairBySecondItemDecreasing());

  // *
  // * Create the cut
  // *
  Cut cut(data->numStates);

  auto it = std::begin(markVals);
  for (; it != std::end(markVals); ++it) {
    if (!markers[it->first].isZero())
      cut.add(it->first);

    if (!std::next(it, 1)->second)
      break;
  }

  // *
  // * If this cut is a duplicate, add random marker states until it's no longer
  // * a duplicate
  // *
  if (cutSet.exists(cut))
  {
    ++it;
    std::shuffle(it, std::end(markVals), *CSFSUtils::rng());

    for (; it != std::end(markVals); ++it)
    {
      std::size_t i = it->first;
      if (!markers[i].isZero())
      {
        cut.add(i);
        if (!cutSet.exists(cut))
          break;
      }
    }
  }

  return cut;
}


//------------------------------------------------------------------------------
// Looks through all the group one individuals for the individual who has the
// fewest remaining markers. The number of marker states this individual carries
// is divided by 2. This number is the new minCutSizeToSwitch.
//
// Divison by 2 occurs because we want to solve small cuts to get better
// incumbent solutions. If a cut is going to be large, then we may as well
// solve a cut based on an individual instead.
//
// Outside of this function:
// When the cut size is greater than or equal to minCutSizeToSwitch, the cut
// creator switches to basing the cuts on individuals.
//------------------------------------------------------------------------------
inline std::size_t CutCreator::minCutSizeToSwitch(const std::vector<Individual> &individuals) const
{
  std::size_t minCutSize = data->numStates;

  for (std::size_t j = data->grpOneStart; j <= data->grpOneEnd; ++j)
    minCutSize = std::min(minCutSize, individuals[j].getNumRemainingMarkers());

  minCutSize /= 2;
  return minCutSize;
}


//------------------------------------------------------------------------------
// Returns true if the cut creator should switch to basing cuts on individuals.
//------------------------------------------------------------------------------
inline bool CutCreator::switchToIndivs(const CutSet &cutSet,
                                       const std::vector<Individual> &individuals,
                                       bool *useIndivs)
{
  for (std::size_t j = data->grpOneStart; j <= data->grpOneEnd; ++j)
  {
    if (!individuals[j].isZero()
    &&  individuals[j].getNumRemainingMarkers() <= cutSet.maxSize())
    {
      *useIndivs = true;
      return true;
    }
  }

  *useIndivs = false;
  return false;
}


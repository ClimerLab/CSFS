#ifndef CUT_CREATOR_H
#define CUT_CREATOR_H

#include "CutSet.h"
#include "Individual.h"
#include "Marker.h"
#include "CSFS_Data.h"

class CutCreator
{
  private:
    const CSFS_Data *data;

    Cut createCutFromIndividuals(const std::vector<Marker> &,
                                 const std::vector<Individual> &,
                                 std::size_t *);
    Cut createCutFromMerging(const CutSet &) const;
    Cut createCutFromRelaxation(const CutSet &,
                                const std::vector<Marker> &,
                                std::vector<std::pair<std::size_t, double> >) const;
    std::size_t minCutSizeToSwitch(const std::vector<Individual> &) const;
    bool switchToIndivs(const CutSet &,
                        const std::vector<Individual> &,
                        bool *);

  public:
    const int RELAXATION = 1;
    const int MERGE = 2;
    const int INDIVIDUAL = 3;

    CutCreator(const CSFS_Data &);
    Cut createCut(const CutSet &,
                  const std::vector<Marker> &,
                  const std::vector<Individual> &,
                  const std::vector<std::pair<std::size_t, double> > &,
                  const std::size_t,
                  int *,
                  std::size_t *);
};

#endif


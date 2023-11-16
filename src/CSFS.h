#ifndef CSFS_H
#define CSFS_H

#include <sstream>
#include "CSFS_Data.h"
#include "Solution.h"

namespace CSFS
{
  bool getNextEnumerationCoords(std::vector<std::size_t> *, const std::size_t);
  bool getNextEnumerationCoords(std::vector<std::size_t> *,
                                const std::size_t,
                                const std::vector<std::size_t> &);
  double getObjectiveValue(const std::size_t,
                           const std::size_t,
                           const CSFS_Data *);
  std::string getStringOfEndOfIterInfo(const double,
                                       const double,
                                       const double);
  void printSolution(const Solution &, std::ofstream *, const CSFS_Data *);
  void printSolution(const std::vector<std::size_t> &,
                     std::ofstream *,
                     const CSFS_Data *);
}

#endif


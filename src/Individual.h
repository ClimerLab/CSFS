#ifndef INDIVIDUAL_H
#define INDIVIDUAL_H

#include <vector>

class Individual
{
  private:
    std::size_t id;
    short group; // group number (either 1 or 2)
    bool one;    // true if Individual has been set to 1
    bool zero;   // true if Individual has been set to 0
    std::size_t numRemainingMarkers; // starts as the total number of markers the
                                     // individual carries. As markers get set to zero,
                                     // this number decreases

    std::size_t numNonzeroMarkers; // the number of nonzero markers from cover LP
    double objValue; // the objective value from cover LP
    std::vector<std::pair<std::size_t, double> > markVals; // marker values from cover LP

  public:
    Individual(const std::size_t, const short, const std::size_t);
    Individual(const std::size_t, const short, const std::size_t, const bool);
    void decrementNumRemainingMarkers();
    std::vector<std::pair<std::size_t, double> > getMarkVals() const;
    std::size_t getId() const;
    std::size_t getNumNonzeroMarkers() const;
    std::size_t getNumRemainingMarkers() const;
    double getObjValue() const;
    void incrementNumRemainingMarkers();
    bool inGrpOne() const;
    bool inGrpTwo() const;
    bool isOne() const;
    bool isSet() const;
    bool isZero() const;
    bool set(const bool);
    void setMarkVals(const std::vector<std::pair<std::size_t, double> > &);
    void setNumNonzeroMarkers(const std::size_t);
    void setNumRemainingMarkers(const std::size_t);
    void setObjValue(const double);

    static bool sortById(const Individual &lhs, const Individual &rhs)
    {
      return lhs.getId() < rhs.getId();
    }

    static bool sortByNumRemainingMarkers(const Individual &lhs, const Individual &rhs)
    {
      if (lhs.getNumRemainingMarkers() != rhs.getNumRemainingMarkers())
        return lhs.getNumRemainingMarkers() < rhs.getNumRemainingMarkers();
      else
        return lhs.getId() < rhs.getId();
    }

    static bool sortByObjValue(const Individual &lhs, const Individual &rhs)
    {
      if (lhs.getObjValue() != rhs.getObjValue())
        return lhs.getObjValue() < rhs.getObjValue();
      else if (lhs.getNumNonzeroMarkers() != rhs.getNumNonzeroMarkers())
        return lhs.getNumNonzeroMarkers() < rhs.getNumNonzeroMarkers();
      else
        return lhs.getId() < rhs.getId();
    }

    static bool sortByNumNonzeroMarkers(const Individual &lhs, const Individual &rhs)
    {
      if (lhs.getNumNonzeroMarkers() != rhs.getNumNonzeroMarkers())
        return lhs.getNumNonzeroMarkers() < rhs.getNumNonzeroMarkers();
      else if (lhs.getObjValue() != rhs.getObjValue())
        return lhs.getObjValue() < rhs.getObjValue();
      else
        return lhs.getId() < rhs.getId();
    }
};

#endif


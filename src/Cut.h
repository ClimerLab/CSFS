#ifndef CUT_H
#define CUT_H

#include <sstream>
#include <vector>

class Cut {
  private:
    std::vector<bool> cut;
    std::size_t numMarkersInCut; // the number of elements in the array set to true

  public:
    Cut();
    Cut(const std::size_t, const bool = 0);
    Cut(const std::vector<bool> &);
    Cut(const std::vector<char> &);
    Cut(const Cut &);
    Cut & operator=(const Cut &);

    bool add(const std::size_t);
    std::size_t cardinalityOfIntersection(const Cut &) const;
    void clear();
    std::size_t distance(const Cut &) const;
    bool empty() const;
    std::string getBinaryString() const;
    std::vector<bool> getBoolVector() const;
    std::vector<char> getCharVector() const;
    std::vector<std::size_t> getTrueElements() const;
    std::string getMarkerNumberString() const;
    std::size_t numElements() const;
    bool remove(const std::size_t);
    bool set(const std::size_t, const bool);
    void set(const std::vector<bool> &);
    void set(const std::vector<char> &);
    void setNumElements(const std::size_t);
    std::size_t size() const;

    bool operator==(const Cut &) const;
    bool operator<(const Cut &) const;
    bool operator>(const Cut &) const;
    bool operator[](const std::size_t) const;
};


struct SortCutsBySizeDecreasing
{
  bool operator()(const Cut &lhs, const Cut &rhs) const
  {
    return lhs.size() > rhs.size();
  }
};


struct SortCutsBySizeIncreasing
{
  bool operator()(const Cut &lhs, const Cut &rhs) const
  {
    return lhs.size() < rhs.size();
  }
};

#endif


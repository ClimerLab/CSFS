#ifndef CSFS_UTILS_H
#define CSFS_UTILS_H

//#include <boost/multiprecision/cpp_int.hpp>
#include <random>

namespace CSFSUtils
{
  //boost::multiprecision::cpp_int C(const std::size_t, std::size_t);
  std::mt19937 createRng();
  std::size_t genRand();
  std::string getNthWord(const std::string &, std::size_t);
  double pround(const double, const std::size_t);
  unsigned long long rdtsc();
  std::mt19937* rng();
  void warning(const std::string &);
  void warning(const char[]);

  struct SortPairByFirstItemDecreasing
  {
    template<typename T, typename U>
    bool operator()(const std::pair<T, U> &lhs, const std::pair<T, U> &rhs) const
    {
      return lhs.first > rhs.first;
    }
  };

  struct SortPairByFirstItemIncreasing
  {
    template<typename T, typename U>
    bool operator()(const std::pair<T, U> &lhs, const std::pair<T, U> &rhs) const
    {
      return lhs.first < rhs.first;
    }
  };

  struct SortPairBySecondItemDecreasing
  {
    template<typename T, typename U>
    bool operator()(const std::pair<T, U> &lhs, const std::pair<T, U> &rhs) const
    {
      return lhs.second > rhs.second;
    }
  };

  struct SortPairBySecondItemIncreasing
  {
    template<typename T, typename U>
    bool operator()(const std::pair<T, U> &lhs, const std::pair<T, U> &rhs) const
    {
      return lhs.second < rhs.second;
    }
  };
}


#endif


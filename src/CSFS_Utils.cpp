#include "CSFS_Utils.h"
#include <iostream>


//------------------------------------------------------------------------------
// Binomial coefficient. "n choose k"
// Adapted from:
// https://www.geeksforgeeks.org/space-and-time-efficient-binomial-coefficient/
//------------------------------------------------------------------------------
boost::multiprecision::cpp_int CSFSUtils::C(const std::size_t n, std::size_t k)
{
  if (k > n)
    return 0;

  if (k > n - k) // C(n, k) = C(n, n-k)
    k = n - k;

  boost::multiprecision::cpp_int result = 1;

  for (std::size_t i = 0; i < k; ++i)
  {
    result *= (n - i);
    result /= (i + 1);
  }

  return result;
}


//------------------------------------------------------------------------------
// Creates and returns a random number generator
//------------------------------------------------------------------------------
std::mt19937 CSFSUtils::createRng()
{
  std::mt19937 rng(rdtsc());
  rng.discard(700000); // https://codereview.stackexchange.com/a/109518
  return rng;
}


//------------------------------------------------------------------------------
// Generates a random number
//------------------------------------------------------------------------------
std::size_t CSFSUtils::genRand()
{
  return (*rng())();
}


//------------------------------------------------------------------------------
// Gets the nth word of a string.
// Example:
//   getNthWord("This     extracts   the nth   word", 2) returns extracts
//------------------------------------------------------------------------------
std::string CSFSUtils::getNthWord(const std::string &str, std::size_t n)
{
  std::string word = str;
  std::istringstream iss(word);
  while (n-- > 0 && (iss >> word));
  return word;
}


//------------------------------------------------------------------------------
// Precision round. Round num to an integer number of decimal points.
// From https://stackoverflow.com/a/50404037
//------------------------------------------------------------------------------
double CSFSUtils::pround(const double num, const std::size_t precision)
{
  const std::size_t x = pow(10, precision);
  return std::round(num * x) / x;
}


//------------------------------------------------------------------------------
// Timestamp counter.
// From: https://stackoverflow.com/a/8661396
//------------------------------------------------------------------------------
unsigned long long CSFSUtils::rdtsc()
{
  unsigned lo, hi;
  __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
  return ((unsigned long long)hi << 32) | lo;
}


//------------------------------------------------------------------------------
// Returns a pointer to the random number generator
//------------------------------------------------------------------------------
std::mt19937* CSFSUtils::rng()
{
  static std::mt19937 rng = createRng();
  return &rng;
}


//------------------------------------------------------------------------------
// Prints a warning
//------------------------------------------------------------------------------
void CSFSUtils::warning(const std::string &msg)
{
  std::cout << "Warning: " << msg << std::endl;
}


//------------------------------------------------------------------------------
// Prints a warning
//------------------------------------------------------------------------------
void CSFSUtils::warning(const char msg[])
{
  std::cout << "Warning: " << msg << std::endl;
}


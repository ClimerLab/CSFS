// *
// * Even though the class is called "Marker", it actually represents a marker
// * state. Currently the only marker that Orca uses is a SNP. Each SNP is
// * separated into four states:
// *     1) Homozygous in the first allele
// *     2) Carrier of the first allele
// *     3) Carrier of the second allele
// *     4) Homozygous in the second allele
// *

#ifndef MARKER_H
#define MARKER_H

#include <cstddef>

class Marker
{
  private:
    const std::size_t id;
    std::size_t numGrpOneCarrying;
    std::size_t numGrpTwoCarrying;
    bool one;
    bool zero;

  public:
    Marker(const std::size_t);
    Marker(const std::size_t, const bool);
    Marker(const std::size_t, const std::size_t, const std::size_t);
    Marker(const std::size_t, const std::size_t, const std::size_t, const bool);
    void decrementNumGrpOneCarrying();
    void decrementNumGrpTwoCarrying();
    void incrementNumGrpOneCarrying();
    void incrementNumGrpTwoCarrying();
    std::size_t getId() const;
    std::size_t getNumGrpOneCarrying() const;
    std::size_t getNumGrpTwoCarrying() const;
    bool isOne() const;
    bool isSet() const;
    bool isZero() const;
    bool set(const bool);
    void setNumGrpOneCarrying(const std::size_t);
    void setNumGrpTwoCarrying(const std::size_t);
};

#endif


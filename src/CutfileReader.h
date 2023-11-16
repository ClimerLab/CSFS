#ifndef CUTFILE_READER_H
#define CUTFILE_READER_H

#include "Cut.h"
#include "Individual.h"
#include "Marker.h"
#include "CSFS_Data.h"
#include "VariableEqualities.h"

class CutfileReader
{
  public:
    // phases
    const unsigned char PENDING = 0;
    const unsigned char READING_COMPLETED_CUTS = 1;
    const unsigned char READING_UNCOMPLETED_CUTS = 2;

  private:
    // sections
    const unsigned char UNKNOWN = 0;
    const unsigned char CUT = 1;
    const unsigned char MARKERS = 2;
    const unsigned char INDIVIDUALS = 3;
    const unsigned char UPPER_BOUND = 4;
    const unsigned char LOWER_BOUND = 5;
    const unsigned char START_COMPLETED_CUTS = 6;
    const unsigned char START_UNCOMPLETED_CUTS = 7;

    const CSFS_Data *data;
    std::ifstream cutfile;
    bool fileIsOpen;
    unsigned char phase_;
    std::string nextLineToProcess;
    std::size_t lastCutRead;

    unsigned char determineSection(const std::string &) const;
    std::size_t extractFirstNumber(const std::string &str) const;
    Cut getCutFromString(const std::string &) const;
    bool isWhitespace(const std::string &) const;
    void loadFirstLineToProcess();
    void processIndividuals(std::vector<std::pair<std::size_t, bool> > *,
                            VariableEqualities *);
    void processMarkers(std::vector<std::pair<std::size_t, bool> > *);
    std::vector<std::string> splitString(const std::string &) const;
    void updatePhase(const unsigned char);

  public:
    CutfileReader(const CSFS_Data &);
    CutfileReader(const CSFS_Data &, const char[]);
    CutfileReader(const CSFS_Data &, const std::string &);
    void close();
    bool fileOpen() const;
    std::size_t getNumberOfLastCutRead() const;
    bool nextCut(Cut *,
                 std::vector<std::pair<std::size_t, bool> > *,
                 std::vector<std::pair<std::size_t, bool> > *,
                 VariableEqualities *,
                 double *,
                 double *);
    bool nextCutAvail() const;
    void open(const char[]);
    void open(const std::string &);
    unsigned char phase() const;
};

#endif


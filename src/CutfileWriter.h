#ifndef CUTFILE_WRITER_H
#define CUTFILE_WRITER_H

#include "CutSet.h"
#include "Individual.h"
#include "Marker.h"
#include "CSFS_Data.h"
#include "VariableEqualities.h"

class CutfileWriter
{
  public:
    const unsigned char PENDING = 0;
    const unsigned char WRITING_COMPLETED_CUTS = 1;
    const unsigned char WRITING_UNCOMPLETED_CUTS = 2;

  private:
    const CSFS_Data *data;
    std::ofstream cutfile;

    std::size_t cutNumber;
    double lastLbWritten;
    double lastUbWritten;

    bool fileIsOpen;
    unsigned char phase_;

    void writeCut(const Cut &);
    void writeIndividuals(const std::vector<Individual> &,
                          const VariableEqualities &);
    void writeLb(const double);
    void writeMarkers(const std::vector<Marker> &);
    void writeUb(const double);

  public:
    CutfileWriter(const CSFS_Data &);
    CutfileWriter(const CSFS_Data &, const char[]);
    CutfileWriter(const CSFS_Data &, const std::string &);
    void close();
    bool fileOpen() const;
    void open(const char[]);
    void open(const std::string &);
    unsigned char phase() const;
    void startCompletedCuts();
    void startUncompletedCuts();
    void write(const Cut &,
               const std::vector<Marker> &,
               const std::vector<Individual> &,
               const VariableEqualities &,
               const double = 0,
               const double = 1);
    void write(const CutSet &,
               const std::vector<Marker> &,
               const std::vector<Individual> &,
               const VariableEqualities &,
               const double = 0,
               const double = 1);
};

#endif


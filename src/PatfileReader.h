#ifndef PATFILE_READER_H
#define PATFILE_READER_H

#include "CSFS_Data.h"

const std::size_t NUM_ITEMS_PER_PAT_EL = 3;
class Cut;

class PatternReader
{
    private:
        const CSFS_Data *data;
        std::ifstream patfile;
        bool fileIsOpen;
        std::string nextLineToProcess;
        std::size_t lastPatRead;

        void loadFirstLineToProcess();
        Cut getPatFromString(const std::string &) const;
        std::vector<std::string> splitString(const std::string &) const;
        bool isWhitespace(const std::string &) const;

    public:
        PatternReader(const CSFS_Data &);
        bool nextPatAvail() const;
        void open(const char[]);
        void open(const std::string &);
        void close();
        std::size_t getNumberOfLastPatRead() const;

        bool nextPat(Cut *);
};

#endif
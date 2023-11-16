#include "CutfileWriter.h"

//------------------------------------------------------------------------------
//    Constructors
//------------------------------------------------------------------------------
CutfileWriter::CutfileWriter(const CSFS_Data &_data) : data(&_data),
                                                      cutNumber(0),
                                                      lastLbWritten(0),
                                                      lastUbWritten(1),
                                                      fileIsOpen(false),
                                                      phase_(PENDING)
{}

CutfileWriter::CutfileWriter(const CSFS_Data &_data,
                             const char cutfileName[]) : data(&_data),
                                                         cutNumber(0),
                                                         lastLbWritten(0),
                                                         lastUbWritten(1),
                                                         fileIsOpen(false),
                                                         phase_(PENDING)
{
  open(cutfileName);
}

CutfileWriter::CutfileWriter(const CSFS_Data &_data,
                             const std::string &cutfileName) : data(&_data),
                                                               cutNumber(0),
                                                               lastLbWritten(0),
                                                               lastUbWritten(1),
                                                               fileIsOpen(false),
                                                               phase_(PENDING)
{
  open(cutfileName.c_str());
}


//------------------------------------------------------------------------------
// Closes the cutfile
//------------------------------------------------------------------------------
void CutfileWriter::close()
{
  assert(fileIsOpen);

  cutfile.close();
  fileIsOpen = false;
}


//------------------------------------------------------------------------------
// Returns fileIsOpen
//------------------------------------------------------------------------------
bool CutfileWriter::fileOpen() const
{
  return fileIsOpen;
}


//------------------------------------------------------------------------------
// Opens a cutfile with the given name
//------------------------------------------------------------------------------
void CutfileWriter::open(const char cutfileName[])
{
  assert(!fileIsOpen);

  cutfile.open(cutfileName);
  if (!cutfile.is_open())
    throw std::runtime_error("Output cutfile could not be opened");
  else {
    cutNumber = 0;
    phase_ = PENDING;
    fileIsOpen = true;
  }
}


//------------------------------------------------------------------------------
// Opens a cutfile with the given name
//------------------------------------------------------------------------------
void CutfileWriter::open(const std::string &cutfileName)
{
  open(cutfileName.c_str());
}


//------------------------------------------------------------------------------
// Returns the current phase
//------------------------------------------------------------------------------
unsigned char CutfileWriter::phase() const
{
  return phase_;
}


//------------------------------------------------------------------------------
// Prints "START_COMPLETED_CUTS:" to the cutfile. This can only be called once
// per cutfile and cannot be called after startUncompletedCuts() has been called.
//------------------------------------------------------------------------------
void CutfileWriter::startCompletedCuts()
{
  assert(phase_ == PENDING);

  cutfile << "START_COMPLETED_CUTS:\n\n";
  cutfile.flush();
  phase_ = WRITING_COMPLETED_CUTS;
}


//------------------------------------------------------------------------------
// Prints "START_COMPLETED_CUTS:" to the cutfile. This can only be called once
// per cutfile.
//------------------------------------------------------------------------------
void CutfileWriter::startUncompletedCuts()
{
  assert(phase_ != WRITING_UNCOMPLETED_CUTS);

  cutfile << "START_UNCOMPLETED_CUTS:\n\n";
  cutfile.flush();
  phase_ = WRITING_UNCOMPLETED_CUTS;
}


//------------------------------------------------------------------------------
// Writes the cut and resultant information to the cutfile
//------------------------------------------------------------------------------
void CutfileWriter::write(const Cut &cut,
                          const std::vector<Marker> &markers,
                          const std::vector<Individual> &individuals,
                          const VariableEqualities &individualEqualities,
                          const double lb,
                          const double ub)
{
  assert(fileIsOpen);
  assert(phase_ != PENDING);

  writeCut(cut);
  if (!individuals.empty() || !individualEqualities.empty())
    writeIndividuals(individuals, individualEqualities);
  if (!markers.empty())
    writeMarkers(markers);
  if (ub < lastUbWritten)
    writeUb(ub);
  if (lb > lastLbWritten)
    writeLb(lb);
  cutfile << "\n";
  cutfile.flush();
}


//------------------------------------------------------------------------------
// Writes all the cuts in the cutset and resultant information to the cutfile
//------------------------------------------------------------------------------
void CutfileWriter::write(const CutSet &cutSet,
                          const std::vector<Marker> &markers,
                          const std::vector<Individual> &individuals,
                          const VariableEqualities &individualEqualities,
                          const double lb,
                          const double ub)
{
  const std::set<Cut> cuts = cutSet.getRawSet();
  const std::vector<Marker> emptyMarkers;
  const std::vector<Individual> emptyIndividuals;
  const VariableEqualities emptyIndivEqualities;

  auto it = std::begin(cuts);
  for (; std::next(it) != cuts.end(); ++it)
    write(*it, emptyMarkers, emptyIndividuals, emptyIndivEqualities);
  write(*it, markers, individuals, individualEqualities, lb, ub);
}


//------------------------------------------------------------------------------
// Writes the cut to the cutfile
//------------------------------------------------------------------------------
void CutfileWriter::writeCut(const Cut &cut)
{
  bool first = true;

  cutfile << "Cut_" << cutNumber << ":";

  if (!cut.empty())
  {
    for (std::size_t i = 0; i < cut.numElements(); ++i)
    {
      if (cut[i])
      {
        if (!first)
          cutfile << " +";
        else
          first = false;
        cutfile << " m(" << i << ")";
      }
    }
    cutfile << " <= " << data->setSize - 1;
  }

  cutfile << "\n";

  ++cutNumber;
}


//------------------------------------------------------------------------------
// Writes the individual information to the cutfile
//------------------------------------------------------------------------------
void CutfileWriter::writeIndividuals(const std::vector<Individual> &individuals,
                                     const VariableEqualities &individualEqualities)
{
  bool first = true;

  cutfile << "Individuals_Set:";
  for (auto it = std::begin(individuals); it != std::end(individuals); ++it)
  {
    if (!first)
      cutfile << "\n                ";
    else
      first = false;
    cutfile << " i(" << it->getId() << ") = ";
    if (it->isZero())
      cutfile << "0";
    else
      cutfile << "1";
  }

  const std::vector<std::vector<std::size_t> > equalities = individualEqualities.get2dVector();

  for (std::size_t i = 0; i < equalities.size(); ++i)
  {
    for (std::size_t j = 1; j < equalities[i].size(); ++j)
    {
      if (!first)
        cutfile << "\n                ";
      else
        first = false;
      cutfile << " i(" << equalities[i][0] << ") = i(" << equalities[i][j] << ")";
    }
  }

  cutfile << "\n";
}


//------------------------------------------------------------------------------
// Write the lower bound to the cutfile
//------------------------------------------------------------------------------
void CutfileWriter::writeLb(const double lb)
{
  cutfile << "Lower_Bound: " << lb << "\n";
  lastLbWritten = lb;
}


//------------------------------------------------------------------------------
// Writes the marker information to the cutfile
//------------------------------------------------------------------------------
void CutfileWriter::writeMarkers(const std::vector<Marker> &markers)
{
  bool first = true;

  cutfile << "Markers_Set:";
  for (auto it = std::begin(markers); it != std::end(markers); ++it)
  {
    if (!first)
      cutfile << "\n            ";
    else
      first = false;

    cutfile << " m(" << it->getId() << ") = ";

    if (it->isZero())
      cutfile << "0";
    else
      cutfile << "1";
  }

  cutfile << "\n";
}


//------------------------------------------------------------------------------
// Write the upper bound to the cutfile
//------------------------------------------------------------------------------
void CutfileWriter::writeUb(const double ub)
{
  cutfile << "Upper_Bound: " << ub << "\n";
  lastUbWritten = ub;
}


SYSTEM     = x86-64_linux
# SYSTEM     = x86-64_osx
LIBFORMAT  = static_pic

#---------------------------------------------------------------------------------------------------
#
# Set CPLEXDIR and CONCERTDIR to the directories where CPLEX and CONCERT are installed.
#
#---------------------------------------------------------------------------------------------------

CPLEXDIR      = /opt/ibm/ILOG/CPLEX_Studio221/cplex
CONCERTDIR    = /opt/ibm/ILOG/CPLEX_Studio221/concert
#CPLEXDIR      = /cluster/software/cplex-studio/cplex-studio-12.7.0/cplex
#CONCERTDIR    = /cluster/software/cplex-studio/cplex-studio-12.7.0/concert
# CPLEXDIR      = /Applications/CPLEX_Studio128/cplex
# CONCERTDIR    = /Applications/CPLEX_Studio128/concert

#---------------------------------------------------------------------------------------------------
# Compiler selection
#---------------------------------------------------------------------------------------------------

MPICXX = mpicxx
CXX = g++

#---------------------------------------------------------------------------------------------------
# Directories
#---------------------------------------------------------------------------------------------------

OBJDIR = build
SRCDIR = src

#---------------------------------------------------------------------------------------------------
# Executables
#---------------------------------------------------------------------------------------------------

EXE = csfs

#---------------------------------------------------------------------------------------------------
# Object files
#---------------------------------------------------------------------------------------------------

_COMMONOBJ = ConfigParser.o Cut.o CutCreator.o CutSet.o Individual.o Marker.o \
             CSFS.o CSFS_Data.o CSFS_Utils.o RelaxationSolver.o SparseSolver.o Solution.o Timer.o \
             VariableEqualities.o
CSFSOBJ    = main.o CutAndSolveController.o CutAndSolveWorker.o CutfileReader.o Parallel.o $(_COMMONOBJ)


#---------------------------------------------------------------------------------------------------
# Compiler options
#---------------------------------------------------------------------------------------------------

CXXFLAGS = -O3 -Wall -fPIC -fexceptions -DIL_STD -std=c++11 -fno-strict-aliasing

#---------------------------------------------------------------------------------------------------
# Link options and libraries
#---------------------------------------------------------------------------------------------------

CPLEXLIBDIR    = $(CPLEXDIR)/lib/$(SYSTEM)/$(LIBFORMAT)
CONCERTLIBDIR  = $(CONCERTDIR)/lib/$(SYSTEM)/$(LIBFORMAT)
OPENMPIT	   = /usr/lib/lam/lib

CXXLNDIRS      = -L$(CPLEXLIBDIR) -L$(CONCERTLIBDIR) -L$(OPENMPIT)
CXXLNFLAGS     = -lconcert -lilocplex -lcplex -lm -lpthread -ldl

#---------------------------------------------------------------------------------------------------
# Includes
#---------------------------------------------------------------------------------------------------

CONCERTINCDIR = $(CONCERTDIR)/include
CPLEXINCDIR   = $(CPLEXDIR)/include

INCLUDES = -I$(CPLEXINCDIR) -I$(CONCERTINCDIR)

#---------------------------------------------------------------------------------------------------

all: CXXFLAGS += -DNDEBUG
all: $(EXE)

debug: CXXFLAGS += -g
debug: $(EXE)

csfs: $(OBJDIR)/main.o
	$(MPICXX) $(CXXLNDIRS) -o $@ $(addprefix $(OBJDIR)/, $(CSFSOBJ)) $(CXXLNFLAGS)

$(OBJDIR)/main.o: $(addprefix $(SRCDIR)/, main.cpp main.h) \
                  $(addprefix $(OBJDIR)/, CutAndSolveController.o CutAndSolveWorker.o)
	$(MPICXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<

$(OBJDIR)/ConfigParser.o: $(addprefix $(SRCDIR)/, ConfigParser.cpp ConfigParser.h) \
                          $(addprefix $(OBJDIR)/, CSFS_Utils.o)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJDIR)/Cut.o: $(addprefix $(SRCDIR)/, Cut.cpp Cut.h)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJDIR)/CutAndSolveController.o:	$(addprefix $(SRCDIR)/, CutAndSolveController.cpp CutAndSolveController.h) \
                               			$(addprefix $(OBJDIR)/, CutCreator.o CutfileReader.o \
													   															 	CSFS.o Parallel.o \
                                                       			RelaxationSolver.o Solution.o \
                                                       			VariableEqualities.o)
	$(MPICXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<

$(OBJDIR)/CutAndSolveWorker.o: $(addprefix $(SRCDIR)/, CutAndSolveWorker.cpp CutAndSolveWorker.h) \
                              $(addprefix $(OBJDIR)/, Parallel.o SparseSolver.o)
	$(MPICXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<

$(OBJDIR)/CutCreator.o: $(addprefix $(SRCDIR)/, CutCreator.cpp CutCreator.h) \
                        $(addprefix $(OBJDIR)/, CutSet.o Individual.o Marker.o CSFS_Data.o)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJDIR)/CutSet.o: $(addprefix $(SRCDIR)/, CutSet.cpp CutSet.h) \
                    $(addprefix $(OBJDIR)/, Cut.o)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJDIR)/CutfileReader.o: $(addprefix $(SRCDIR)/, CutfileReader.cpp CutfileReader.h) \
                           $(addprefix $(OBJDIR)/, Cut.o Individual.o Marker.o CSFS_Data.o \
                                                   VariableEqualities.o)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJDIR)/CutfileWriter.o: $(addprefix $(SRCDIR)/, CutfileWriter.cpp CutfileWriter.h) \
                           $(addprefix $(OBJDIR)/, CutSet.o Individual.o Marker.o CSFS_Data.o \
                                                   VariableEqualities.o)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJDIR)/Individual.o: $(addprefix $(SRCDIR)/, Individual.cpp Individual.h)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJDIR)/Marker.o: $(addprefix $(SRCDIR)/, Marker.cpp Marker.h)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJDIR)/CSFS.o: $(addprefix $(SRCDIR)/, CSFS.cpp CSFS.h) \
                  $(addprefix $(OBJDIR)/, CSFS_Data.o Solution.o)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<

$(OBJDIR)/CSFS_Data.o: $(addprefix $(SRCDIR)/, CSFS_Data.cpp CSFS_Data.h) \
                      $(addprefix $(OBJDIR)/, ConfigParser.o CSFS_Utils.o Timer.o)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJDIR)/CSFS_Utils.o: $(addprefix $(SRCDIR)/, CSFS_Utils.cpp CSFS_Utils.h)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJDIR)/Parallel.o: $(addprefix $(SRCDIR)/, Parallel.cpp Parallel.h)
	$(MPICXX) $(CXXFLAGS) -c -o $@ $<

$(OBJDIR)/SparseSolver.o: $(addprefix $(SRCDIR)/, SparseSolver.cpp SparseSolver.h) \
                          $(addprefix $(OBJDIR)/, CutSet.o CSFS.o Solution.o Timer.o)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<

$(OBJDIR)/RelaxationSolver.o: $(addprefix $(SRCDIR)/, RelaxationSolver.cpp RelaxationSolver.h) \
                              $(addprefix $(OBJDIR)/, CutSet.o CSFS_Data.o Solution.o Timer.o)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<

$(OBJDIR)/Solution.o: $(addprefix $(SRCDIR)/, Solution.cpp Solution.h)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJDIR)/Timer.o: $(addprefix $(SRCDIR)/, Timer.cpp Timer.h)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJDIR)/VariableEqualities.o: $(addprefix $(SRCDIR)/, VariableEqualities.cpp VariableEqualities.h)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

#---------------------------------------------------------------------------------------------------
.PHONY: clean cleanest
clean:
	/bin/rm -f $(OBJDIR)/*.o

cleanest:
	/bin/rm -f $(OBJDIR)/*.o *.log *.cuts *.lp $(EXE)
#---------------------------------------------------------------------------------------------------

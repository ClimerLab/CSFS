#include "main.h"

int main(int argc, char **argv) {
  //*
  //* MPI init
  //*
  MPI_Init(NULL, NULL);  

  const int world_rank = Parallel::getWorldRank();
  const int world_size = Parallel::getWorldSize();
  
  try {
    // *
    // * Check for correct command line arguments
    // *
    if (world_rank == 0) {
      std::ostringstream oss;
      if (argc != 2)
        oss << "Usage:\n   " << argv[0] << " <config file>";
      else if (world_size < 2)
        oss << "world_size must be greater than 1.";

      if ( !oss.str().empty() ) // exit if an above condition was met
      {
        std::cerr << oss.str() << std::endl;
        MPI_Abort(MPI_COMM_WORLD, 1);
      }
    }


    // *
    // * Set up the data
    // *
    const CSFS_Data data(argv[1]);
    if (world_rank == 0)
    {
      data.checkParameters();
      printInitialMessages(data);
    }


    // *
    // * Cut and solve
    // *
    switch (world_rank)
    {

      case 0:
      {
        CutAndSolveController controller(data);

        while ( !controller.converged() )
          controller.work();

        controller.signalWorkersToEnd();

        while ( controller.workersStillWorking() )
          controller.waitForWorkers();

        std::cout << "\nDone.\n"
                  << "\nUpper bound: " << controller.getUb()
                  << "\nLower bound: " << controller.getLb()
                  << "\n\nTotal execution time"
                  << "\nCPU seconds: " << data.elapsed_cpu_time()
                  << "\nWall clock seconds: " << data.elapsed_wall_time() << std::endl;

        break;
      }

      default:
      {
        CutAndSolveWorker worker(data);
        while ( !worker.end() )
          worker.work();

        break;
      }

    }
  }
  catch (std::exception &e)
  {
    std::cout << "  *** Fatal error reported by rank_"
              << world_rank << ": " << e.what() << " ***" << std::endl;
    MPI_Abort(MPI_COMM_WORLD, 1);
  }


  MPI_Finalize();
}


//------------------------------------------------------------------------------
// Prints some initial information
//------------------------------------------------------------------------------
void printInitialMessages(const CSFS_Data &data)
{
  std::ostringstream consoleOutput;
  consoleOutput << "\nCommand line arguments:\n"
                << "  csfs "  << data.configFilename << "\n\n"
                << "***********************************IMPORTANT************************************\n\n"
                << "  " << asctime(data.startTime) << "\n"
                << "  Reading data from:\n    " << data.inputFilename << "\n\n";

  consoleOutput << "  Solutions will be written to:\n    " << data.logfileName << "\n\n";

  consoleOutput << "  Assumed the first " << data.numHeadCols
                << " columns are header columns, the next " << data.numCase
                << " columns\n  represent " << data.numCase
                << " cases, and the final " << data.numCtrl << " columns"
                << " represent " << data.numCtrl << " controls.\n\n"
                << "  Assumed the first " << data.numHeadRows
                << " rows are header rows and the remaining " << data.numActualExprs
                << " rows represent\n  expression data.\n\n";

  {
    const int world_size = Parallel::getWorldSize();
    consoleOutput << "  Running " << world_size << " processes (1 controller and "
                  << world_size - 1 << " workers).\n\n";
  }

  if (data.RISK)
    consoleOutput << "  Risk patterns of size 1 through "
                  << data.setSize << " will be identified.\n\n";
  else
    consoleOutput << "  Protective patterns of size 1 through "
                  << data.setSize << " will be identified.\n\n";

  if (data.USE_SOLUTION_POOL_THRESHOLD)
    consoleOutput << "  All patterns with objective value >= " << data.SOLUTION_POOL_THRESHOLD << " will be saved.\n\n";

  consoleOutput << "  Starting upper bound: " << data.STARTING_UPPER_BOUND << "\n"
                << "  Starting lower bound: " << data.STARTING_LOWER_BOUND << "\n\n";

  consoleOutput << "********************************************************************************\n";

  std::cout << consoleOutput.str() << std::endl;

  // *
  // * Print out data read
  // *
  if (data.VERBOSE)
    std::cout << data.exprInfoString() << "\n"
              << data.exprMatrixString() << std::endl;
}


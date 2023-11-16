# CSFS
A cut-and-solve based feature selection algorithm for continous data.

## To Use
Configure the Makefile with the locaion of the IBL ILOG CPLEX and open mpi libraries and binary

Compile with the Makefile by navigating to the root directory and entering: make

Update configuration file

Run the program. For an example enter: mpirun -np 4 ./csfs <cfg_file>

## Configuration
DATA_FILE - Tab seperated file where the first NUM_CASES columns are cases and the next NUM_CTRLS columns are controls. The row indicate features.

RISK - Boolean that indicates if risk patterns (true) or protective patterns (false) should be found.

NUM_CASES - The number of cases in DATA_FILE.

NUM_CTRLS - The number of controls in DATA_FILE.

NUM_EXPRS - The number of features in DATA_FILE.

NUM_HEAD_ROWS - The number of header rows in DATA_FILE.

NUM_HEAD_COLS - The number of header columns in DATA_FILE.

PATTERN_SIZE - The number of marker states in the pattern(s) to be found.

USE_LOWER_CUTOFF - A boolean indicates if the STARTING_LOWER_BOUND is used. This lower bound can be updated during the search. USE_LOWER_CUTOFF and USE_SOLUTION_POOL_THRESHOLD cannot both be set to true at the same time.

USE_SOLUTION_POOL_THRESHOLD - A boolean indicating of the SOLUTION_POOL_THRESHOLD is to be used. When using the SOLUTION_POOL_THRESHOLD, all patterns with a beter objective value will be retained. USE_LOWER_CUTOFF and USE_SOLUTION_POOL_THRESHOLD cannot both be set to true at the same time.

SOLUTION_POOL_THRESHOLD - Threshold used to retaine solutions in the pool. Used if USE_SOLUTION_POOL_THRESHOLD is true.

STARTING_LOWER_BOUND - Starting lower bound when 0 USE_LOWER_CUTOFF is true.

STARTING_UPPER_BOUND - Starting upper bound when 0 USE_LOWER_CUTOFF is true.

QUIET - Boolean that limits the output to only the most important items when true. QUIET and VERBOSE cannot both be set to true at the same time.

VERBOSE - Boolean that controls if all outputs are dispayed. QUIET and VERBOSE cannot both be set to true at the same time.

PRINT_CPLEX_OUTPUT - Boolean controlling if the CPLEX output is displayed.

TOL - Tolerance value for used for rounding decimals to integers in CPLEX.

ID_PREFIX - Prefix of ID column in DATA_FILE.

MISSING_SYMBOL - String used to indicate missing data in DATA_FILE.

CPLEX_SEED - Seed provide to CPLEX.

USE_SPARSE_CONTRAINTS - Boolean that indicates if additional contraints for the sparse problem are used.

NUM_BINS - The number of bins, from HIGH, NORM, LOW, NOT_HIGH, and NOT_LOW to be used.

USE_HIGH - Set to true if HIGH variable will be used in pattern.

USE_NORM - Set to true if NORM variable will be used in pattern.

USE_LOW - Set to true if LOW variable will be used in pattern.

USE_NOT_HIGH - Set to true if NOT_HIGH variable will be used in pattern.

USE_NOT_LOW - Set to true if NOT_LOW variable will be used in pattern.

HIGH_VALUE - Value in DATA_FILE that indicates high expression.

NORM_VALUE - Value in DATA_FILE that indicates normal expression.

LOW_VALUE - Value in DATA_FILE that indicates low expression.

NOT_LOW_VALUE - Value in DATA_FILE that indicates not low expression.

NOT_HIGH_VALUE - Value in DATA_FILE that indicates not high expression.

SET_NA_TRUE - Boolean used to indicate if missing data is treated as both high and low.

## Output
*.log - File containing the collection of patterns

## Notes
Requires Open MPI and IBM ILOG CPLEX

DATA_FILE should be tab seperate, the columns represent individuals and the rows represent features

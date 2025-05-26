In both the static and dynamic solutions, process 0 (the master) does not perform the computation itself (except for summing the results). Therefore, to run with 3 workers, the program should be executed with 4 processes total (i.e., 3 workers and 1 master).
Additionally, to enable the built-in mathematical operations used in the code, the -lm flag must be added during compilation and execution.

Example of compiling and running the static solution with 3 workers:
mpicc static.c -o st -lm
mpiexec -np 4 ./st -lm

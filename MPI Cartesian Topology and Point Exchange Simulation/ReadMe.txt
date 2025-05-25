This project uses MPI to simulate a group of processes arranged in a 2D Cartesian grid.
Each process starts with one point (with x, y coordinates and ID), and in every iteration, it exchanges this point with its four neighbors (top, bottom, left, right).

Each process picks the closest point received, and if at least 3 processes have almost no change (distance < MinDist), the simulation stops.
Otherwise, it continues until reaching the MaxIterations limit

When running the code, specify the number of processes to be used – this number should match the number of points in the text file (which must be located in the same directory as the .c file).
Additionally, in order to enable the built-in mathematical functions used in the code, the -lm flag must be added during compilation and execution.
Example of compiling and running the solution with 9 processes:
mpicc exe2.c -o e -lm
mpiexec -np 9 ./e -lm

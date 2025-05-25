This project compares three implementations of a heavy computation task:
1. Sequential (CPU)
2. Parallel with OpenMP
3. Parallel with CUDA (GPU)

The program runs a computationally expensive function over an array of randomly generated numbers.
We measure execution time under various conditions (size, loopSize) and compare performance across the three methods.


To simplify the evaluation process, a Makefile is included with predefined compilation commands for all solution files:
    gcc seq.c -o s -lm → for the sequential version
    gcc -fopenmp openMP.c -o omp -lm → for the OpenMP version
    nvcc -gencode arch=compute_61,code=sm_61 cuda.cu -o cu -lm → for the CUDA version

Therefore, can simply run:
make
This will compile all solution files.

Example execution (with size = 100 and loopSize = 10):
./s 100 10      # Sequential
./omp 100 10    # OpenMP
./cu 100 10     # CUDA

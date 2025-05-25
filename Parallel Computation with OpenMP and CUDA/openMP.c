#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <omp.h>


// This function performs heavy computations
// No Changes to this function are allowed
 double heavy(double data, int loopSize) {
    double sum = 0;       
    for (int i = 0; i < loopSize; i++)
        sum += cos(exp(sin(data * (i % 11))))/ loopSize;

     return sum;             
}
 

// OpenMP code to be parallelized
int main(int argc, char *argv[]) {
    int i;
    int size = atoi(argv[1]);
    int loopSize = atoi(argv[2]);
    double start_time, end_time;

    double* arr = (double*)malloc(size * sizeof(double));
    if (!arr) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    
    for (i = 0; i < size; i++)
        arr[i] = (double) rand() / RAND_MAX;

    double answer = 0;

    start_time = omp_get_wtime();
    #pragma omp parallel for reduction (+: answer)
        for (i = 0; i < size; i++)
            answer += heavy(arr[i], loopSize);

    end_time = omp_get_wtime();

    printf("answer = %e\n", answer); 
    printf("Time: %f seconds\n", end_time - start_time);

    free(arr);
}

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <cuda_runtime.h>

// Function to check CUDA errors
void cudaError(cudaError_t err, const char* message) {
    if (err != cudaSuccess) {
        fprintf(stderr, "Error: %s (code %s)\n", message, cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }
}

// This function performs heavy computations
// No Changes to this function are allowed
__device__ double heavy(double data, int loopSize) {
    double sum = 0;       
    for (int i = 0; i < loopSize; i++)
        sum += cos(exp(sin(data * (i % 11))))/ loopSize;

     return sum;             
}

__global__ void kernel_heavy(int* size, int* loopSize, double* arr, double* answer) {
    int i = blockDim.x * blockIdx.x + threadIdx.x;

    if (i < *size) {
        double calc = heavy(arr[i], *loopSize);
        atomicAdd(answer, calc);
    }
}
 

// CUDA code to be parallelized
int main(int argc, char *argv[]) {
    // Error code to check return values for CUDA calls
    cudaError_t err = cudaSuccess;
    
    int i;
    int size = atoi(argv[1]);
    int loopSize = atoi(argv[2]);
    float time = 0;
    cudaEvent_t start_time, end_time;

    cudaEventCreate(&start_time);
    cudaEventCreate(&end_time);

    double* arr = (double*)malloc(size * sizeof(double));
    
    if (!arr) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < size; i++)
        arr[i] = (double) rand() / RAND_MAX;
    
    // Allocate the device input size
    int *d_size = NULL;
    err = cudaMalloc((void **)&d_size, sizeof(int));

    cudaError(err, "allocate device size");

    // Allocate the device input loop size
    int *d_loopSize = NULL;
    err = cudaMalloc((void **)&d_loopSize, sizeof(int));

    cudaError(err, "allocate device loop size");

    // Allocate the device input vector arr
    double *d_arr = NULL;
    err = cudaMalloc((void **)&d_arr, size * sizeof(double));

    cudaError(err, "allocate device vector arr");

    // Allocate the device output answer
    double *d_answer = NULL;
    err = cudaMalloc((void **)&d_answer, sizeof(double));

    cudaError(err, "allocate device answer");

    cudaMemset(d_answer, 0, sizeof(double));

    cudaError(err, "set asnwer to 0");

    err = cudaMemcpy(d_size, &size, sizeof(int), cudaMemcpyHostToDevice);

    cudaError(err, "copy size from host to device");

    err = cudaMemcpy(d_loopSize, &loopSize, sizeof(int), cudaMemcpyHostToDevice);

    cudaError(err, "copy loop size from host to device");

    err = cudaMemcpy(d_arr, arr, size * sizeof(double), cudaMemcpyHostToDevice);

    cudaError(err, "copy vector arr from host to device");

    // Launch CUDA Kernel
    int threadsPerBlock = 256;
    int blocksPerGrid = (size + threadsPerBlock - 1) / threadsPerBlock;
    
    double answer = 0;
    cudaEventRecord(start_time);
    kernel_heavy<<<blocksPerGrid, threadsPerBlock>>>(d_size, d_loopSize, d_arr, d_answer);
    cudaEventRecord(end_time);
    cudaEventSynchronize(end_time);
    err = cudaGetLastError();

    cudaError(err, "launch kernel");
    
    // Copy the device result in device memory to the host result in host memory.
    err = cudaMemcpy(&answer, d_answer, sizeof(double), cudaMemcpyDeviceToHost);

    cudaError(err, "copy answer from device to host");

    printf("answer = %e\n", answer);
    cudaEventElapsedTime(&time, start_time, end_time);   
    printf("Time: %f seconds\n", time/1000.0);

    // Free device global memory
    err = cudaFree(d_size);

    cudaError(err, "allocate device size");
    if (err != cudaSuccess) {
        fprintf(stderr, "Failed to free device size (error code %s)!\n",
                cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }

    err = cudaFree(d_loopSize);

    cudaError(err, "free device loop size");

    err = cudaFree(d_arr);

    cudaError(err, "free device vector arr");

    err = cudaFree(d_answer);

    cudaError(err, "free device answer");

    // Free host memory
    free(arr);

    // Free events
    cudaEventDestroy(start_time);
    cudaEventDestroy(end_time);
}
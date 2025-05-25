build:
	gcc seq.c -o s -lm
	gcc -fopenmp openMP.c -o omp -lm
	nvcc  -gencode arch=compute_61,code=sm_61  cuda.cu -o cu -lm

clean:
	rm -f *.o ./cu

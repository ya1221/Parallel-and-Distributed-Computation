#include <stdio.h>
#include <math.h>
#include <mpi.h>
#define HEAVY  10000
#define SIZE   40
#define ARR_POINT_LEN 2
#define TERMINATION_TAG 1

// This function performs heavy computations, 
// its run time depends on x and y values
double heavy(int x, int y) {
	int i, loop;
	double sum = 0;
	if (x > y)
		loop = 10;
	else
		loop = 1;
	for (i = 1; i < loop * HEAVY; i++)
		sum += sin(exp(cos((double)i / HEAVY)));
	return  sum;
}

int getNextTaskFromTaskPool(int* arr_point, int* last_point) {
	if(*last_point >= SIZE*SIZE)
		return 0;
	arr_point[0] = *last_point%SIZE;
	arr_point[1] = *last_point/SIZE;
	(*last_point)++;
	
	return 1;
}

// Dynamic code to be parallelized
int main(int argc, char* argv[]) {	
	int myid, numprocs, last_point = 0, count = 0;
	MPI_Status status;
	MPI_Init(&argc,&argv);
	MPI_Comm_size(MPI_COMM_WORLD,&numprocs);
	MPI_Comm_rank(MPI_COMM_WORLD,&myid);
	numprocs--;
	
	int arr_point[ARR_POINT_LEN] = {0,0};
	double result = 0, answer = 0;
	
	if (myid == 0) {
		double start_time = MPI_Wtime();
		for(int i=1; i<=numprocs; i++){
			getNextTaskFromTaskPool(arr_point, &last_point);
			MPI_Send(arr_point, ARR_POINT_LEN, MPI_INT, i, 0, MPI_COMM_WORLD);
		}
		do{
			MPI_Recv(&result, 1, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			answer+=result;
			if(getNextTaskFromTaskPool(arr_point, &last_point))
				MPI_Send(arr_point, ARR_POINT_LEN, MPI_INT, status.MPI_SOURCE, 0, MPI_COMM_WORLD);
			else{
				MPI_Send(arr_point, ARR_POINT_LEN, MPI_INT, status.MPI_SOURCE, TERMINATION_TAG, MPI_COMM_WORLD);
				count++;	
			}
		}while(count < numprocs);
		double end_time = MPI_Wtime();
		printf("answer = %e, time = %e\n", answer, end_time - start_time);
	}
	else {
		while(1){
			MPI_Recv(arr_point, 2, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			// x = arr_point[0], y = arr_point[1]
			if (status.MPI_TAG == TERMINATION_TAG)
				break;
			answer = heavy(arr_point[0], arr_point[1]);
			MPI_Send(&answer, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
		}
	}
	MPI_Finalize();
}

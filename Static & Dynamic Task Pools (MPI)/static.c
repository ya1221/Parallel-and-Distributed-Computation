#include <stdio.h>
#include <math.h>
#include <mpi.h>
#define HEAVY  10000
#define SIZE 40
#define ARR_POINT_LEN 4

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

double calculationSlave(int* arr_point) {
	int x,y;
	double answer = 0;
	for (y = arr_point[1]; y < SIZE; y++)
	{
		for (x = arr_point[0]; x < SIZE && (x!=arr_point[2] || y!=arr_point[3]); x++)
		{
			arr_point[0] = 0;
			answer += heavy(x, y);
		}
		if(x==arr_point[2] && y== arr_point[3])
			break;
	}
	return answer;
}

// Static code to be parallelized
int main(int argc, char* argv[]) {
	
	int myid, numprocs, last_point = 0;
	MPI_Status status;
	MPI_Init(&argc,&argv);
	MPI_Comm_size(MPI_COMM_WORLD,&numprocs);
	MPI_Comm_rank(MPI_COMM_WORLD,&myid);
	numprocs--;
	
	int arr_point[ARR_POINT_LEN] = {0,0,0,0};
	double result = 0, answer = 0;
	
	if (myid == 0) {
		int range = SIZE*SIZE/numprocs; // Basic count points - for each processes
		int extra = SIZE*SIZE%numprocs; // One more point - for process number one to process number extra
		double start_time = MPI_Wtime();
		int i;
		for(i=1;i<=numprocs; i++){
			last_point+=(range+(extra>0));
			extra--;
			arr_point[2] = last_point%SIZE;
			arr_point[3] = last_point/SIZE;
			MPI_Send(arr_point, ARR_POINT_LEN, MPI_INT, i, 0, MPI_COMM_WORLD);
			arr_point[0] = arr_point[2];
			arr_point[1] = arr_point[3];
		}
		for(i=1;i<=numprocs; i++){
			MPI_Recv(&result, 1, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			answer+=result;
		}
		double end_time = MPI_Wtime();
		printf("answer = %e, time = %e\n", answer, end_time - start_time);
	}
	else {
		MPI_Recv(arr_point, ARR_POINT_LEN, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		// first_x = arr_point[0], first_y = arr_point[1], last_x = arr_point[2], last_y = arr_point[3]
		answer = calculationSlave(arr_point);
		MPI_Send(&answer, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
	}
	MPI_Finalize();
}

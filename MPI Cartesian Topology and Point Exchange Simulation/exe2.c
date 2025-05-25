#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

struct Point
{
	int id;
    float x;
    float y;
};

FILE* open_file(){
    FILE *file;
    file = fopen("input.txt", "r");
    if (!file) {
        perror("Error opening file");
        return 0;
    }
    return file;
}

int main(int argc, char *argv[]) {
    int myrank, size;
    MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

    int k, maxIterations, num_proc, values[2]={0};
    float minDist;
    FILE *file;
    char line[256];  // Buffer to hold each line

    if (myrank == 0) {
        file = open_file();
        
        if (fgets(line, sizeof(line), file) != NULL) {
            if (sscanf(line, "%d %e %d", &k, &minDist, &maxIterations) != 3) {
                perror("k, minDist, maxIterations incorrect");
                return 0;
            }
        }

        num_proc = k*k;
        if (size != num_proc) {
            printf("The number of processes and the number of points are different.\n");fflush(stdout);
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        values[0] = k;
        values[1] = maxIterations;
        MPI_Bcast(values, 2, MPI_INT, 0, MPI_COMM_WORLD);
    } else {
        MPI_Bcast(values, 2, MPI_INT, 0, MPI_COMM_WORLD);
        k = values[0];
        maxIterations = values[1];
    }
    
    struct Point current_point;
    MPI_Status status;
    MPI_Datatype PointMPIType;
    MPI_Datatype type[3] = { MPI_INT, MPI_FLOAT, MPI_FLOAT };
    int blocklen[3] = { 1, 1, 1 };
    MPI_Aint disp[3];
	 
	// Create MPI user data type for partical
    disp[0] = (char *) &current_point.id - (char *) &current_point;
    disp[1] = (char *) &current_point.x - (char *) &current_point;
    disp[2] = (char *) &current_point.y - (char *) &current_point;
    MPI_Type_create_struct(3, blocklen, disp, type, &PointMPIType);
    MPI_Type_commit(&PointMPIType);
  
    struct Point points[num_proc];
    // Send and recieve one struct of the PointMPIType type
    if (myrank == 0) {
        // Read the file line by line
        for (int i = 0; i<num_proc; i++) {
            if ((fgets(line, sizeof(line), file) == NULL) || (sscanf(line, "%d %e %e", &points[i].id, &points[i].x, &points[i].y) != 3)) {// Parse the numbers from the line
                perror("id, x, y incorrect");
                return 0;
            }
        }
        // Close the file
        fclose(file);
        MPI_Scatter(points, 1, PointMPIType, &current_point, 1, PointMPIType, 0, MPI_COMM_WORLD);
    } else
        MPI_Scatter(NULL, 0, PointMPIType, &current_point, 1, PointMPIType, 0, MPI_COMM_WORLD);
        


    int source, dest[4], count;
    int dim[2] = {k, k}, period[2] = {1,1}, reorder = 0;
    MPI_Comm comm;
    MPI_Cart_create(MPI_COMM_WORLD, 2, dim, period, reorder, &comm);
    double min, dist;
    struct Point pointN, pointCh;

    for (int j=0; j<maxIterations; j++) {
        min = -1;
        dist = 0;
        pointCh = current_point;
        for (int i=0; i<4; i++) { // send point to neighbors
            MPI_Cart_shift(comm, i/2, (int)pow(-1,i), &source, &dest[i]);
            MPI_Send(&current_point, 1, PointMPIType, dest[i], 0, MPI_COMM_WORLD);
        }
        for(int i=0; i<4;i++){ //get point from neighbors
            MPI_Recv(&pointN, 1, PointMPIType, dest[i], 0, MPI_COMM_WORLD, &status);
            dist = sqrt(pow(pointN.x - current_point.x, 2) + pow(pointN.y - current_point.y, 2));
            if(min == -1 || min > dist){
                min = dist;
                pointCh = pointN;
            }
        }

        current_point = pointCh;
        count = 0;
        if (myrank != 0) {
            MPI_Gather(&min, 1, MPI_INT, NULL, 0, MPI_INT, 0, MPI_COMM_WORLD);
            MPI_Bcast(&count, 1, MPI_INT, 0, MPI_COMM_WORLD);
            if(count == 3){
                MPI_Gather(&current_point, 1, PointMPIType, NULL, 0, MPI_INT, 0, MPI_COMM_WORLD);
                break;
            }
        } else {
            int minAll[num_proc];
            MPI_Gather(&min, 1, MPI_INT, minAll, 1, MPI_INT, 0, MPI_COMM_WORLD);
            for (int i=0;i<num_proc && count != 3; i++){
                if(minAll[i] < minDist){
                    count++;
                }
            }
            MPI_Bcast(&count, 1, MPI_INT, 0, MPI_COMM_WORLD);
            if(count==3){
                MPI_Gather(&current_point, 1, PointMPIType, points, 1, PointMPIType, 0, MPI_COMM_WORLD);
                int rank, coords[2];
                for(int r=0; r<k;r++){
                    for(int c=0; c<k; c++){
                        coords[0] = r;
                        coords[1] = c;
                        MPI_Cart_rank(comm, coords, &rank);
                        printf("Coordinate: (%d,%d) - id: %d - point: (%e,%e) - rank: %d\n", r,c,points[rank].id, points[rank].x, points[rank].y, rank); fflush(stdout);
                    }
                }
                break;
            }
        }
    }

    if(myrank == 0 && count!=3){
        printf("No proximity was detected\n"); fflush(stdout);
    }

    MPI_Type_free(&PointMPIType);
    MPI_Finalize();
    return 0;
}
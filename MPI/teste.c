#include <mpi.h>
#include <stdio.h>
#include <string.h>

int main( int argc, char *argv[]) {
    int rank, msg, N;

    /*MPI_Status status;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    MPI_Comm_size(MPI_COMM_WORLD, &N);

    /* Process 0 sends and Process 1 receives 
    if (rank == 0) {
        msg = 123456;
        MPI_Send( &msg, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
    } else if (rank == N-1) {
        MPI_Recv( &msg, 1, MPI_INT, N-2, 0, MPI_COMM_WORLD, &status );
        printf( "Received on process %d %d\n", N-1, msg);
    } else {
        MPI_Recv( &msg, 1, MPI_INT, rank-1, 0, MPI_COMM_WORLD, &status );
        printf( "Received on process %d %d\n", rank, msg);
        MPI_Send( &msg, 1, MPI_INT, rank+1, 0, MPI_COMM_WORLD);
    }


    MPI_Finalize();*/

    int array[100];
    for(int i=0; i<100; i++){
        array[i]=1;
    }

    int arrayb[10];
    memcpy(&array[10],arrayb,10*sizeof(int));

    for(int i=0; i<100; i++){
        printf("%d ", array[i]);
    }
    return 0;
}
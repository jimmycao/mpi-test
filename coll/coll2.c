/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  (C) 2001 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */
#include "mpi.h"
#include <stdio.h>
#include "mpitest.h"

#define MAX_PROCESSES 10

int main( int argc, char **argv )
{
    int              rank, size, i,j;
    int              table[MAX_PROCESSES][MAX_PROCESSES];
    int              errors=0;
    int              participants;

    MTest_Init( &argc, &argv );
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    MPI_Comm_size( MPI_COMM_WORLD, &size );

    /* A maximum of MAX_PROCESSES processes can participate */
    if ( size > MAX_PROCESSES ) participants = MAX_PROCESSES;
    else              participants = size;

    if (MAX_PROCESSES % participants) {
	fprintf( stderr, "Number of processors must divide %d\n",
		MAX_PROCESSES );
	MPI_Abort( MPI_COMM_WORLD, 1 );
	}
    if ( (rank < participants) ) {

      /* Determine what rows are my responsibility */
      int block_size = MAX_PROCESSES / participants;
      int begin_row  = rank * block_size;
      int end_row    = (rank+1) * block_size;
      int send_count = block_size * MAX_PROCESSES;
      int recv_count = send_count;

      /* Paint my rows my color */
      for (i=begin_row; i<end_row ;i++)
	for (j=0; j<MAX_PROCESSES; j++)
	  table[i][j] = rank + 10;

      /* Gather everybody's result together - sort of like an */
      /* inefficient allgather */
      for (i=0; i<participants; i++) {
        void *sendbuf = (i == rank ? MPI_IN_PLACE : &table[begin_row][0]);
	MPI_Gather(sendbuf,              send_count, MPI_INT,
		   &table[0][0],         recv_count, MPI_INT, i, 
		   MPI_COMM_WORLD );
      }

      /* Everybody should have the same table now,  */
      /* This test does not in any way guarantee there are no errors */
      /* Print out a table or devise a smart test to make sure it's correct */
      for (i=0; i<MAX_PROCESSES;i++) {
	if ( (table[i][0] - table[i][MAX_PROCESSES-1] !=0) ) 
	  errors++;
      }
    } 

    MTest_Finalize( errors );
    MPI_Finalize();
    return MTestReturnValue( errors );
}

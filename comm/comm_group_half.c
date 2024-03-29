/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  (C) 2003 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include <stdio.h>
#include <mpi.h>
/* USE_STRICT_MPI may be defined in mpitestconf.h */
#include "mpitestconf.h"

int main(int argc, char **argv)
{
    int rank, size, i;
    MPI_Group full_group, half_group;
    int range[1][3];
    MPI_Comm comm;

    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    MPI_Comm_group(MPI_COMM_WORLD, &full_group);
    range[0][0] = 0;
    range[0][1] = size / 2;
    range[0][2] = 1;
    MPI_Group_range_incl(full_group, 1, range, &half_group);

#if !defined(USE_STRICT_MPI) && defined(MPICH2)
    if (rank <= size / 2) {
        MPIX_Comm_create_group(MPI_COMM_WORLD, half_group, 0, &comm);
        MPI_Barrier(comm);
        MPI_Comm_free(&comm);
    }
#endif /* USE_STRICT_MPI */

    MPI_Group_free(&half_group);
    MPI_Group_free(&full_group);

    if (rank == 0)
        printf(" No Errors\n");

    MPI_Finalize();

    return 0;
}

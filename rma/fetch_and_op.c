/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *
 *  (C) 2012 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <mpi.h>
#include "mpitest.h"

/* MPI-3 is not yet standardized -- allow MPI-3 routines to be switched off.
 */

#if !defined(USE_STRICT_MPI) && defined(MPICH2)
#  define TEST_MPI3_ROUTINES 1
#endif

static const int SQ_LIMIT = 10;
static       int SQ_COUNT = 0;

#define SQUELCH(X)                      \
  do {                                  \
    if (SQ_COUNT < SQ_LIMIT || verbose) { \
      SQ_COUNT++;                       \
      X                                 \
    }                                   \
  } while (0)

#define ITER 100

const int verbose = 0;

#if defined (FOP_TYPE_CHAR)
#  define TYPE_C   char
#  define TYPE_MPI MPI_CHAR
#  define TYPE_FMT "%d"
#elif defined (FOP_TYPE_SHORT)
#  define TYPE_C   short
#  define TYPE_MPI MPI_SHORT
#  define TYPE_FMT "%d"
#elif defined (FOP_TYPE_LONG)
#  define TYPE_C   long
#  define TYPE_MPI MPI_LONG
#  define TYPE_FMT "%ld"
#elif defined (FOP_TYPE_DOUBLE)
#  define TYPE_C   double
#  define TYPE_MPI MPI_DOUBLE
#  define TYPE_FMT "%f"
#elif defined (FOP_TYPE_LONG_DOUBLE)
#  define TYPE_C   long double
#  define TYPE_MPI MPI_LONG_DOUBLE
#  define TYPE_FMT "%Lf"
#else
#  define TYPE_C   int
#  define TYPE_MPI MPI_INT
#  define TYPE_FMT "%d"
#endif

#define CMP(x, y) ((x - ((TYPE_C) (y))) > 1.0e-9)

void reset_vars(TYPE_C *val_ptr, TYPE_C *res_ptr, MPI_Win win) {
    int i, rank, nproc;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nproc);

    MPI_Win_lock(MPI_LOCK_EXCLUSIVE, rank, 0, win);
    for (i = 0; i < nproc; i++) {
        val_ptr[i] = 0;
        res_ptr[i] = -1;
    }
    MPI_Win_unlock(rank, win);

    MPI_Barrier(MPI_COMM_WORLD);
}

int main(int argc, char **argv) {
    int       i, rank, nproc, mpi_type_size;
    int       errors = 0, all_errors = 0;
    TYPE_C   *val_ptr, *res_ptr;
    MPI_Win   win;

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nproc);
 
    MPI_Type_size(TYPE_MPI, &mpi_type_size);
    assert(mpi_type_size == sizeof(TYPE_C));

    val_ptr = malloc(sizeof(TYPE_C)*nproc);
    res_ptr = malloc(sizeof(TYPE_C)*nproc);

    MPI_Win_create(val_ptr, sizeof(TYPE_C)*nproc, sizeof(TYPE_C), MPI_INFO_NULL, MPI_COMM_WORLD, &win);

#ifdef TEST_MPI3_ROUTINES

    /* Test self communication */

    reset_vars(val_ptr, res_ptr, win);

    for (i = 0; i < ITER; i++) {
        TYPE_C one = 1, result = -1;
        MPI_Win_lock(MPI_LOCK_EXCLUSIVE, rank, 0, win);
        MPIX_Fetch_and_op(&one, &result, TYPE_MPI, rank, 0, MPI_SUM, win);
        MPI_Win_unlock(rank, win);
    }

    MPI_Win_lock(MPI_LOCK_EXCLUSIVE, rank, 0, win);
    if ( CMP(val_ptr[0], ITER) ) {
        SQUELCH( printf("%d->%d -- SELF: expected "TYPE_FMT", got "TYPE_FMT"\n", rank, rank, (TYPE_C) ITER, val_ptr[0]); );
        errors++;
    }
    MPI_Win_unlock(rank, win);

    /* Test neighbor communication */

    reset_vars(val_ptr, res_ptr, win);

    for (i = 0; i < ITER; i++) {
        TYPE_C one = 1, result = -1;
        MPI_Win_lock(MPI_LOCK_EXCLUSIVE, (rank+1)%nproc, 0, win);
        MPIX_Fetch_and_op(&one, &result, TYPE_MPI, (rank+1)%nproc, 0, MPI_SUM, win);
        MPI_Win_unlock((rank+1)%nproc, win);
        if ( CMP(result, i) ) {
            SQUELCH( printf("%d->%d -- NEIGHBOR[%d]: expected result "TYPE_FMT", got "TYPE_FMT"\n", (rank+1)%nproc, rank, i, (TYPE_C) i, result); );
            errors++;
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);

    MPI_Win_lock(MPI_LOCK_EXCLUSIVE, rank, 0, win);
    if ( CMP(val_ptr[0], ITER) ) {
        SQUELCH( printf("%d->%d -- NEIGHBOR: expected "TYPE_FMT", got "TYPE_FMT"\n", (rank+1)%nproc, rank, (TYPE_C) ITER, val_ptr[0]); );
        errors++;
    }
    MPI_Win_unlock(rank, win);

    /* Test contention */

    reset_vars(val_ptr, res_ptr, win);

    if (rank != 0) {
        for (i = 0; i < ITER; i++) {
            TYPE_C one = 1, result;
            MPI_Win_lock(MPI_LOCK_EXCLUSIVE, 0, 0, win);
            MPIX_Fetch_and_op(&one, &result, TYPE_MPI, 0, 0, MPI_SUM, win);
            MPI_Win_unlock(0, win);
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);

    MPI_Win_lock(MPI_LOCK_EXCLUSIVE, rank, 0, win);
    if (rank == 0 && nproc > 1) {
        if ( CMP(val_ptr[0], ITER*(nproc-1)) ) {
            SQUELCH( printf("*->%d - CONTENTION: expected="TYPE_FMT" val="TYPE_FMT"\n", rank, (TYPE_C) ITER*(nproc-1), val_ptr[0]); );
            errors++;
        }
    }
    MPI_Win_unlock(rank, win);

    /* Test all-to-all communication */

    reset_vars(val_ptr, res_ptr, win);

    for (i = 0; i < ITER; i++) {
        int j;

        MPI_Win_fence(MPI_MODE_NOPRECEDE, win);
        for (j = 0; j < nproc; j++) {
            TYPE_C rank_cnv = (TYPE_C) rank;
            MPIX_Fetch_and_op(&rank_cnv, &res_ptr[j], TYPE_MPI, j, rank, MPI_SUM, win);
            res_ptr[j] = i*rank;
        }
        MPI_Win_fence(MPI_MODE_NOSUCCEED, win);
        MPI_Barrier(MPI_COMM_WORLD);

        for (j = 0; j < nproc; j++) {
            if ( CMP(res_ptr[j], i*rank) ) {
                SQUELCH( printf("%d->%d -- ALL-TO-ALL[%d]: expected result "TYPE_FMT", got "TYPE_FMT"\n", rank, j, i, (TYPE_C) i*rank, res_ptr[j]); );
                errors++;
            }
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Win_lock(MPI_LOCK_EXCLUSIVE, rank, 0, win);
    for (i = 0; i < nproc; i++) {
        if ( CMP(val_ptr[i], ITER*i) ) {
            SQUELCH( printf("%d->%d -- ALL-TO-ALL: expected "TYPE_FMT", got "TYPE_FMT"\n", i, rank, (TYPE_C) ITER*i, val_ptr[i]); );
            errors++;
        }
    }
    MPI_Win_unlock(rank, win);

#endif /* TEST_MPI3_ROUTINES */

    MPI_Win_free(&win);

    MPI_Reduce(&errors, &all_errors, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0 && all_errors == 0)
        printf(" No Errors\n");

    free(val_ptr);
    free(res_ptr);
    MPI_Finalize();

    return 0;
}

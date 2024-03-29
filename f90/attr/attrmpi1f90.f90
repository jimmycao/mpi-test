! This file created from test/mpi/f77/attr/attrmpi1f.f with f77tof90
! -*- Mode: Fortran; -*- 
!
!  (C) 2003 by Argonne National Laboratory.
!      See COPYRIGHT in top-level directory.
!
      program main
      use mpi
      integer value, wsize, wrank, extra, mykey
      integer rvalue, ncomm
      logical flag
      integer ierr, errs
!
      errs = 0
      call mtest_init( ierr )
      call mpi_comm_size( MPI_COMM_WORLD, wsize, ierr )
      call mpi_comm_rank( MPI_COMM_WORLD, wrank, ierr )
!
!     Simple attribute put and get
!
      call mpi_keyval_create( MPI_NULL_COPY_FN, MPI_NULL_DELETE_FN, &
      &     mykey, extra,ierr ) 
      call mpi_attr_get( MPI_COMM_WORLD, mykey, value, flag, ierr )
      if (flag) then
         errs = errs + 1
         print *, &
      &       "Did not get flag==.false. for attribute that was not set"
      endif
!
      value = 1234567
      call mpi_attr_put( MPI_COMM_WORLD, mykey, value, ierr )
      call mpi_attr_get( MPI_COMM_WORLD, mykey, rvalue, flag, ierr )
      if (.not. flag) then
         errs = errs + 1
         print *, "Did not find attribute after set"
      else
         if (rvalue .ne. value) then
            errs = errs + 1
            print *, "Attribute value ", rvalue, " should be ", value
         endif
      endif
      value = -123456
      call mpi_attr_put( MPI_COMM_WORLD, mykey, value, ierr )
      call mpi_attr_get( MPI_COMM_WORLD, mykey, rvalue, flag, ierr )
      if (.not. flag) then
         errs = errs + 1
         print *, "Did not find attribute after set (neg)"
      else
         if (rvalue .ne. value) then
            errs = errs + 1
            print *, "Neg Attribute value ", rvalue, " should be ",value
         endif
      endif
!      
      call mpi_keyval_free( mykey, ierr )
      call mtest_finalize( errs )
      call mpi_finalize( ierr )
      end

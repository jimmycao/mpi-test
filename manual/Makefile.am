## -*- Mode: Makefile; -*-
## vim: set ft=automake :
##
## (C) 2011 by Argonne National Laboratory.
##     See COPYRIGHT in top-level directory.
##

include $(top_srcdir)/Makefile.mtest

noinst_PROGRAMS = singjoin testconnect testconnectserial dimsbalanced \
                  spawntest_master spawntest_child segfault \
                  mpit_test mpit_test2

# testconnectserial would like MPICHLIBSTR to be defined as the installation
# directory of the MPI library.  This definition is not required.
noinst_HEADERS = connectstuff.h
testconnectserial_SOURCES = testconnectserial.c tchandlers.c tcutil.c 
testconnectserial_LDADD   = $(LDADD) -lm

CLEANFILES += test-port


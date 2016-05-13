AUTOMAKE_OPTIONS = 1.4 foreign

WARNS =		-W -Wall -Wshadow -Wpointer-arith 
IFLAGS =
FLAGS =		-g -O3 -funroll-loops -D_THREAD_SAFE -D_GNU_SOURCE -march=native
VERBOSE	=	-DVERBOSE
AM_CFLAGS =	$(IFLAGS) $(VERBOSE) $(WARNS) $(FLAGS) $(ICU_CFLAGS)
AM_CXXFLAGS =	$(IFLAGS) $(VERBOSE) $(WARNS) $(FLAGS) -std=c++0x $(ICU_CFLAGS)
AM_LDFLAGS =	-g -licuuc $(ICU_LIBS)

noinst_HEADERS = \
	common_impl.h wg_mempool.h

libtextcat_includedir = $(includedir)/libtextcat

libtextcat_include_HEADERS = \
	common.h constants.h fingerprint.h textcat.h utf8misc.h

lib_LTLIBRARIES =	libtextcat.la libtextcat++.la
libtextcat_la_SOURCES = \
	common.c fingerprint.c textcat.c wg_mempool.c utf8misc.c

libtextcat___includedir = $(includedir)/libtextcat
libtextcat___include_HEADERS= textcat++.hxx
libtextcat___la_SOURCES= textcat++.cxx

bin_PROGRAMS =		createfp
createfp_SOURCES =	createfp.c
createfp_LDADD =	libtextcat.la

noinst_PROGRAMS =	testtextcat my_createfp
testtextcat_SOURCES =	testtextcat.c
testtextcat_LDADD =	libtextcat.la

my_createfp_SOURCES =	my_createfp.cxx
my_createfp_LDADD =	libtextcat++.la

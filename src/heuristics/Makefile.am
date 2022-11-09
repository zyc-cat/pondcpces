lib_LTLIBRARIES = libheuristics.la
libheuristics_la_SOURCES = lug.cc  lug.h  relaxedPlan.cc  relaxedPlan.h  statistical.cc  statistical.h

INCLUDES =  -I. -I@top_srcdir@/src/ -I@top_srcdir@/src/cudd -I@top_srcdir@/src/ppddl \
		-I@top_srcdir@/src/heuristics -I@top_srcdir@/src/lug -I@top_srcdir@/src/search




libheuristics_la_LDFLAGS = -no-undefined



LIBTOOL_DEPS = @LIBTOOL_DEPS@
libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status --recheck

libheuristics_ladir = .


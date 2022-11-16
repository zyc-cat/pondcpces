lib_LTLIBRARIES = libcounterexample.la
libcounterexample_la_SOURCES = samplegen.h samplegen.cc

INCLUDES =  -I. -I@top_srcdir@/src/ -I@top_srcdir@/src/cudd -I@top_srcdir@/src/ppddl \
		-I@top_srcdir@/src/heuristics -I@top_srcdir@/src/lug -I@top_srcdir@/src/search \
		-I@top_srcdir@/src/counterexample



#libsearch_la_LDFLAGS = -L../cudd -lcudd -lcudd_util -ldddmp -lepd -lst -lmtr -lm -lfl 

#libsearch_la_LIBADD = ../globals.lo





LIBTOOL_DEPS = @LIBTOOL_DEPS@
libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status --recheck

libcounterexample_ladir = .


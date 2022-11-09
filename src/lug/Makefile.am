lib_LTLIBRARIES = liblug.la
liblug_la_SOURCES = graph_wrapper.cc      output.cc \
graph_wrapper.h         output.h \
build_graph.cc      ipp.h               randomGenerator.cc \
build_graph.h       kGraphInfo.h        randomGenerator.h \
correlation.cc    labelledElement.cc  sample_size.cc \
correlation.h       labelledElement.h   sample_size.h \
exclusions.cc     memory.cc           utilities.cc \
exclusions.h       memory.h            utilities.h 

INCLUDES =  -I. -I@top_srcdir@/src/ -I@top_srcdir@/src/cudd -I@top_srcdir@/src/ppddl \
		-I@top_srcdir@/src/heuristics -I@top_srcdir@/src/lug -I@top_srcdir@/src/search


liblug_la_LDFLAGS = -no-undefined



LIBTOOL_DEPS = @LIBTOOL_DEPS@
libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status --recheck

liblug_ladir = .


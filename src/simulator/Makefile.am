lib_LTLIBRARIES = libsimulator.la
libsimulator_la_SOURCES = client.cc  client.h  monitor.cc  monitor.h  server.cc  server.h


INCLUDES =  -I. -I@top_srcdir@/src/ -I@top_srcdir@/src/cudd -I@top_srcdir@/src/ppddl \
		-I@top_srcdir@/src/heuristics -I@top_srcdir@/src/lug -I@top_srcdir@/src/search 


#libsimulator_la_LDFLAGS = -L../cudd -lcudd -lcudd_util -ldddmp -lepd -lst -lmtr -lm -lfl





LIBTOOL_DEPS = @LIBTOOL_DEPS@
libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status --recheck

libsimulator_ladir = .


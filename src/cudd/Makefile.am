lib_LTLIBRARIES = libdd.la
libdd_la_SOURCES = dd.cc dd.h cudd.h

#INCLUDES = -I@top_srcdir@/src -I. -I.. -I../lug
INCLUDES =  -I. -I@top_srcdir@/src/ -I@top_srcdir@/src/cudd -I@top_srcdir@/src/ppddl \
		-I@top_srcdir@/src/heuristics -I@top_srcdir@/src/lug -I@top_srcdir@/src/search


libdd_la_LDFLAGS = -L. 
#-lcudd -lutil -ldddmp -lepd -lst -lmtr -lm -lfl



LIBTOOL_DEPS = @LIBTOOL_DEPS@
libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status --recheck

libdd_ladir = .


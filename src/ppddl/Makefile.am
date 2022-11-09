lib_LTLIBRARIES = libppddl.la
libppddl_la_SOURCES =  client.cc client.h strxml.cc strxml.h \
		    hashing.h exceptions.cc exceptions.h requirements.cc \
		    requirements.h rational.cc rational.h types.cc types.h \
		    terms.cc terms.h predicates.cc predicates.h functions.cc \
                    functions.h expressions.cc expressions.h formulas.cc \
                    formulas.h effects.cc effects.h actions.cc actions.h \
                    domains.cc domains.h problems.cc problems.h states.cc \
                    states.h tokenizer.ll mtbdd.cc mtbdd.h parser.yy observations.cc observations.h


INCLUDES =  -I. -I@top_srcdir@/src/ -I@top_srcdir@/src/cudd -I@top_srcdir@/src/ppddl \
		-I@top_srcdir@/src/heuristics -I@top_srcdir@/src/lug -I@top_srcdir@/src/search


libppddl_la_LDFLAGS = -no-undefined

#libppddl_la_LIBADD = ../globals.lo 

LIBTOOL_DEPS = @LIBTOOL_DEPS@
libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status --recheck

libppddl_ladir = .

MAINTAINERCLEANFILES = parser.cc tokenizer.cc config.h.in~

AM_YFLAGS = -d

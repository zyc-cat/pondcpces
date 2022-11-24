lib_LTLIBRARIES = libvalidate.la
libvalidate_la_SOURCES = planval.h planval.cc

INCLUDES =  -I. -I@top_srcdir@/src/ -I@top_srcdir@/src/validate -I@top_srcdir@/src/cudd \
		-I@top_srcdir@/src/ppddl -I@top_srcdir@/src/heuristics -I@top_srcdir@/src/lug \
		-I@top_srcdir@/src/search 





LIBTOOL_DEPS = @LIBTOOL_DEPS@
libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status --recheck

libvalidate_ladir = .
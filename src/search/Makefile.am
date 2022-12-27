lib_LTLIBRARIES = libsearch.la
libsearch_la_SOURCES = alao_star.cc alao_star.h anytime_search.cc anytime_search.h \
			astar.cc astar.h aostar.cc aostar.h astar_advance.cc astar_advance.h backup.cc backup.h \
			ehc_dominance.cc ehc.h exph.cc exph.h graph.cc graph.h hop.cc hop.h \
			hop_advance.cc hop_advance.h lao.cc lao.h lao_wrapper.cc lao_wrapper.h \
			NSPolicy.cc NSPolicy.h online_search.cc online_search.h pchop.cc pchop.h \
			pchop_advance.cc pchop_advance.h replan.cc replan.h \
			sampled_search.cc sampled_search.h search.cc search.h solve.cc \
			solve.h step_search.cc step_search.h vi.cc vi.h rtdp.h rtdp.cc molao.cc molao.h \
			mobackup.cc mobackup.h movalfn.cc movalfn.h dbn.cc dbn.h \
			moastar.cc moastar.h moastar_heuristics.cc moastar_heuristics.h planvalidate.cc planvalidate.h

INCLUDES =  -I. -I@top_srcdir@/src/ -I@top_srcdir@/src/cudd -I@top_srcdir@/src/ppddl \
		-I@top_srcdir@/src/heuristics -I@top_srcdir@/src/lug -I@top_srcdir@/src/search


#libsearch_la_LDFLAGS = -L../cudd -lcudd -lcudd_util -ldddmp -lepd -lst -lmtr -lm -lfl 

#libsearch_la_LIBADD = ../globals.lo





LIBTOOL_DEPS = @LIBTOOL_DEPS@
libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status --recheck

libsearch_ladir = .


## Process this file with automake to produce Makefile.in
SUBDIRS = ppddl search heuristics lug cudd simulator rddlclient
# create three executable program
bin_PROGRAMS = pond ppddl2st lug2txt pondbeta counter

#--------------------------------------------------------------
# The first one target

pond_SOURCES = main.cc globals.cc globals.h
#pond_sim_SOURCES = sim_main.cc $SOURCES

# do not inlude POMDP, rddlcilent,
INCLUDES =  -I. -I@top_srcdir@/src/ -I@top_srcdir@/src/cudd -I@top_srcdir@/src/ppddl \
		-I@top_srcdir@/src/heuristics -I@top_srcdir@/src/lug -I@top_srcdir@/src/search -I/home/labrea6/bryce/include

# define the dynamic link
pond_LDADD = ppddl/libppddl.la search/libsearch.la heuristics/libheuristics.la lug/liblug.la \
             cudd/libdd.la simulator/libsimulator.la ${CUDD_HOME}/cudd/libcudd.a \
             ${CUDD_HOME}/dddmp/libdddmp.a ${CUDD_HOME}/epd/libepd.a ${CUDD_HOME}/mtr/libmtr.a \
             ${CUDD_HOME}/st/libst.a ${CUDD_HOME}/util/libutil.a 
             #-L${CUDD_HOME}/cudd \
             #-L${CUDD_HOME}/dddmp -L${CUDD_HOME}/epd -L${CUDD_HOME}/mtr \
             #-L${CUDD_HOME}/st -L${CUDD_HOME}/util
 
pond_LDFLAGS = -no-undefined --disable-shared -L/usr/local/lib 
#-lcudd -lutil -ldddmp -lepd -lst -lmtr -lm -lfl


#---------------------------------------------------------------
# The second one target
lug2txt_SOURCES = lug2txt_main.cc globals.cc globals.h
lug2txt_LDADD = ppddl/libppddl.la search/libsearch.la heuristics/libheuristics.la lug/liblug.la \
             cudd/libdd.la simulator/libsimulator.la ${CUDD_HOME}/cudd/libcudd.a \
             ${CUDD_HOME}/dddmp/libdddmp.a ${CUDD_HOME}/epd/libepd.a ${CUDD_HOME}/mtr/libmtr.a \
             ${CUDD_HOME}/st/libst.a ${CUDD_HOME}/util/libutil.a -L${CUDD_HOME}/cudd \
             -L${CUDD_HOME}/dddmp -L${CUDD_HOME}/epd -L${CUDD_HOME}/mtr \
             -L${CUDD_HOME}/st -L${CUDD_HOME}/util
             
lug2txt_LDFLAGS = -no-undefined --disable-shared -L/usr/local/lib
#		  -lutil -ldddmp -lepd -lst -lmtr -lm -lfl 



#----------------------------------------------------------------
# The last one target

ppddl2st_SOURCES = ppddl2st.cc globals.cc globals.h
ppddl2st_LDADD = ppddl/libppddl.la search/libsearch.la heuristics/libheuristics.la lug/liblug.la \
             cudd/libdd.la simulator/libsimulator.la ${CUDD_HOME}/cudd/libcudd.a \
             ${CUDD_HOME}/dddmp/libdddmp.a ${CUDD_HOME}/epd/libepd.a ${CUDD_HOME}/mtr/libmtr.a \
             ${CUDD_HOME}/st/libst.a ${CUDD_HOME}/util/libutil.a -L${CUDD_HOME}/cudd \
             -L${CUDD_HOME}/dddmp -L${CUDD_HOME}/epd -L${CUDD_HOME}/mtr \
             -L${CUDD_HOME}/st -L${CUDD_HOME}/util
ppddl2st_LDFLAGS = -no-undefined --disable-shared -L/usr/local/lib 
#-lcudd -lutil -ldddmp -lepd -lst -lmtr -lm -lfl

#-----------------------------------------------------------------
# The test target

pondbeta_SOURCES = main_beta.cc globals.cc globals.h

INCLUDES =  -I. -I@top_srcdir@/src/ -I@top_srcdir@/src/cudd -I@top_srcdir@/src/ppddl \
		-I@top_srcdir@/src/heuristics -I@top_srcdir@/src/lug -I@top_srcdir@/src/search -I/home/labrea6/bryce/include

# define the dynamic link
pondbeta_LDADD = ppddl/libppddl.la search/libsearch.la heuristics/libheuristics.la lug/liblug.la \
             cudd/libdd.la simulator/libsimulator.la ${CUDD_HOME}/cudd/libcudd.a \
             ${CUDD_HOME}/dddmp/libdddmp.a ${CUDD_HOME}/epd/libepd.a ${CUDD_HOME}/mtr/libmtr.a \
             ${CUDD_HOME}/st/libst.a ${CUDD_HOME}/util/libutil.a 
             #-L${CUDD_HOME}/cudd \
             #-L${CUDD_HOME}/dddmp -L${CUDD_HOME}/epd -L${CUDD_HOME}/mtr \
             #-L${CUDD_HOME}/st -L${CUDD_HOME}/util
 
pondbeta_LDFLAGS = -no-undefined --disable-shared -L/usr/local/lib 

#--------------------------------------------------------------
# The counter target

counter_SOURCES = main_counter.cc globals.cc globals.h

INCLUDES =  -I. -I@top_srcdir@/src/ -I@top_srcdir@/src/cudd  -I@top_srcdir@/src/ppddl \
             -I@top_srcdir@/src/heuristics -I@top_srcdir@/src/lug -I@top_srcdir@/src/search -I/home/labrea6/bryce/include

# define the dynamic link
counter_LDADD = ppddl/libppddl.la search/libsearch.la heuristics/libheuristics.la \
             lug/liblug.la cudd/libdd.la simulator/libsimulator.la ${CUDD_HOME}/cudd/libcudd.a \
             ${CUDD_HOME}/dddmp/libdddmp.a ${CUDD_HOME}/epd/libepd.a ${CUDD_HOME}/mtr/libmtr.a \
             ${CUDD_HOME}/st/libst.a ${CUDD_HOME}/util/libutil.a
 
counter_LDFLAGS = -no-undefined --disable-shared -L/usr/local/lib 
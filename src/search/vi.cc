/* vi.c */
#include "stdio.h"
#include <time.h>
#include "solve.h"
#include "track.h"
#include "graph.h"
#include "vi.h"
#include "backup.h"
#include "lao_wrapper.h"
#include "globals.h"
#include "lug/ipp.h"
#ifdef PPDDL_PARSER
#else
#include "actions/actions.h"
#endif
//#include "parser/ptree.h"

#include "cudd/dd.h"

using namespace __gnu_cxx;

/* Create list of all non-goal states */
StateList *CreateAllStateList(){
  int state;
  StateList *list;

  list = new StateList;
  list->push_back(Start);
  for(StateHash::iterator i = StateIndex->begin(); i != StateIndex->end(); i++)
    list->push_back((*i).second);
  return(list);
}

/* Create list of all non-goal states */
StateList *CreateAllStateListWithoutGoal(){
  int state;
  StateList *list;

  list = new StateList;
  list->push_back(Start);
  for(StateHash::iterator i = StateIndex->begin(); i != StateIndex->end(); i++)
    if((*i).second->Terminal != 1)
      list->push_back((*i).second);
  return(list);
}

void ValueIteration(StateList *list, int MaxIter){
  int Iter;
  double diff, maxdiff;         // Bellman residual
  double error;
  StateList::iterator node;
  
  for(Iter = 0; Iter < MaxIter; Iter++){
    maxdiff = 0.0;

    for(node = list->begin(); node != list->end(); node++){
      if((*node)->Terminal != 1){
        diff = Backup(*node, -1 /* shouldnt use horizon --dan*/);
        if(diff > maxdiff)
          maxdiff = diff;
      }
    }

    if(gMethod == vi){
      if(gDiscount < 1.0)
        error = (maxdiff * gDiscount) / (1.0 - gDiscount);
      else
        error = (maxdiff * Start->meanFirstPassage);

      printf("\n%3d ( %f secs.)  f: %f  Error bound: %f", Iter, (float)(clock()-gStartTime)/CLOCKS_PER_SEC, Start->f, error);

      if(error < gEpsilon)
        return;
    }
  }
}


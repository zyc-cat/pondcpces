#include "rtdp.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include "track.h"
#include "graph.h"
#include "backup.h"
#include "solve.h"
#include "rtdp.h"
#include "lao.h"
#include "lao_wrapper.h"
#include <iostream>

using namespace std;

RTDP::RTDP(int nTrial)
: Search(RTDP_SEARCH){
  this->nTrial = nTrial;
  trial = 0;
  stateExpanded = 0;
  stateExpandedTrial = 0;
  Iteration = 0;
  NumExpandedStates = 0;
}

void RTDP::ExpandNode(struct StateNode *node){
  if(node->NextActions == NULL || node->Expanded == -1){
     node->expand();
     if(node->NextActions == NULL)
       node->Terminal = 1;
  }
  node->Expanded = Iteration;
  NumExpandedStates++;
}

/* Real Time Dynamic Programming always starts at the Start state */
void RTDP::search()
{
  struct StateNode *state;
  double diff, maxdiff, val;

  stateExpanded = 0;
  Iteration = 0;

  for(trial = 0; trial < nTrial; trial++){
    Iteration++;
    printf("\n%5d ( %f secs.)  f: %f", trial, (float)(clock() - gStartTime) / CLOCKS_PER_SEC, Start->f);
    stateExpandedTrial = 0;
    state = Start;
    cout << endl;
    while(true){
      cout << state->StateNo << " " << flush;
      if(state->Terminal == 1 || state->Expanded == Iteration)
        break;
      ExpandNode(state);
      Backup(state, 0);
      state = SimulateNextState(state);
      printStateList();
    }
  }

  printf("\nRTDP done. \n");
}

struct StateNode* RTDP::SimulateNextState(struct StateNode *currentState){
  double r, p;
  struct ActionNode* greedyAction = currentState->BestAction;
  struct StateDistribution* nextState;

  if(greedyAction == NULL)
    return currentState;

  nextState = greedyAction->NextState;

  while(true){
    p = 0.0;
    r = drand48();
    for(nextState = greedyAction->NextState; nextState != NULL; nextState = nextState->Next){
      p += nextState->Prob;
      if(r <= p)
        return nextState->State;
    }
  }

  /* shall always return in the above loop, otherwise something is wrong */
  printf("\nSomething is wrong in SimulateNextState!");
  printf("\np=%f, r=%f\n", p, r);
  exit(0);
}

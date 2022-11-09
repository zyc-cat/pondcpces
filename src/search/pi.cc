/* pi.c */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "solve.h"
#include "graph.h"
#include "math.h"

static void PolicyEvaluation(StateList *list)
/* Compute value function for current policy */
{
  float **a, **b;
  int stateNum, i, j, NumSolutionStates;
  StateList     *stateListNode;
  struct StateDistribution *nextState;

  /* Determine policy size */
  NumSolutionStates = 0;
  for (stateListNode = list; 
       stateListNode; 
       stateListNode = stateListNode->Next)
    if (stateListNode->Node->Terminal != 1) {
      NumSolutionStates++;
      /* Number solution states using update field */
      stateListNode->Node->Update = NumSolutionStates;
    }
      
  /* allocate matrices and initialize coefficient matrix to zeros 
     (wish there were way to avoid double loop for initialization) */
  a=matrix(1,NumSolutionStates,1,NumSolutionStates);
  b=matrix(1,NumSolutionStates,1,1);
  for (i=1;i<=NumSolutionStates;i++) {
    for (j=1;j<=NumSolutionStates;j++) {
      a[i][j] = 0.0;
    }
    b[i][1] = 0.0;
  }
  
  /* load equations into matrices */
  /* a is square matrix of coefficients */
  /* b is vector of constants */
  stateNum = 1;
  for (stateListNode = list; 
       stateListNode; 
       stateListNode = stateListNode->Next)
    if (stateListNode->Node->Terminal != 1) {
      for (nextState = stateListNode->Node->BestAction->NextState;
	   nextState;
	   nextState = nextState->Next)
	a[stateNum][nextState->State->Update] = - gDiscount * nextState->Prob;
      a[stateNum][stateNum] += 1.0;
      b[stateNum][1] = stateListNode->Node->BestAction->Cost;
      stateNum++;
    }

  printf("\nEnter gaussj with %d solution states", stateNum);
  /* gaussj returns unknown value function in vector b */
  gaussj(a, NumSolutionStates, b, 1);
  printf("\nExit gaussj"); fflush(0);

  /* Copy updated value function to state array */
  stateNum = 1;
  for (stateListNode = list; 
       stateListNode; 
       stateListNode = stateListNode->Next)
    if (stateListNode->Node->Terminal != 1) {
      stateListNode->Node->f = b[stateNum][1];
      stateNum++;
    }

  free_matrix(a,1,NumSolutionStates,1,NumSolutionStates);
  free_matrix(b,1,NumSolutionStates,1,1);
}

void PolicyIteration(StateList *list)
{
  int    Iter, stateNum, improveFlag;
  double diff;
  StateList   *stateListNode;
  
  do {
    PolicyEvaluation( list );
    printf(".");

    improveFlag = 0;
    for (stateListNode = list; 
	 stateListNode; 
	 stateListNode = stateListNode->Next)
      if (stateListNode->Node->Terminal != 1) { 
	diff = Backup(stateListNode->Node);
	if (diff > gEpsilon)
	  improveFlag = 1;
      }
  } while (improveFlag == 0);
}

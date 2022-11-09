#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "track.h"
#include "graph.h"
#include "lao.h"
#include "./molao.h"
#include "./mobackup.h"
#include "solve.h"
#include "../globals.h"
#include "lao_wrapper.h"
#include "vi.h"
#include <iostream>
#include <float.h>
#include <math.h>
#include <list>

using namespace __gnu_cxx;
using namespace std;

MOLAOStar::MOLAOStar()
: Search(MOLAO){
  NumExpandedStates = 0;
  Iteration = 0;
  NumExpandedStatesIter = 0;
  NumAncestorStatesIter = 0;
  NumSolutionStates = 0;
  ExpandedStateList = NULL;
  Residual = 0.0;
  ExitFlag = 0;

  lastBackup = NULL;
}

void MOLAOStar::search()
{
  StateList *AncestorList;
  struct StateNode *node;
  lastBackup = Start;

  for(Iteration = 1; ; Iteration++){
    /* Expand best partial solution */
    NumExpandedStatesIter = 0;
    ExpandedStateList = new StateList;

    expandSolution(lastBackup, 0, 0, 1.0);  

    /* Call convergence test if solution graph has no unexpanded tip states */
    Start->moValueFunction->print(cout);

    if(Start->moValueFunction->solutionNoLessThan(goal_threshold) != NULL){
      cout << "FoundSolution" << endl;
      Start->moValueFunction->print(cout);
      printPolicy(Start->moValueFunction->solutionNoLessThan(goal_threshold));
      cout << "Num Efficient Solutions = " << Start->moValueFunction->numSolutions() << endl;
      exit(0);
    }

    /* Dynamic programming step */
    /* (Skip if backups were performed in expansion step) */
    if(gBackupMethod == separate){
      /* Update state costs and best partial solution */
      NumAncestorStatesIter = 0;
      /* the following condition is necessary in case no nodes are
       expanded but the convergence test fails */
      if(NumExpandedStatesIter > 0){
        AncestorList = findAncestors(ExpandedStateList);
        /* note that expanded nodes are included as ancestors */
        ValueIteration(AncestorList, 3);
        delete AncestorList;
      }
    }

    delete ExpandedStateList;

    /* NumSolutionStates includes GOAL and WALL */
    printf("  ExpandedStates: %d", NumExpandedStatesIter); 
  }
}

void MOLAOStar::expandSolution(struct StateNode *node, int currHorizon, int dfs_depth, double path_pr){
/* Returns list of expanded states */
/* if num expanded nodes exceeds limit, break */
  struct StateDistribution *successor;

  if(node->Terminal == 1)
    return;

  double nextCost = 0.0;

  if(!node->dd){
    cout << "no dd for node " << node->StateNo <<endl;
    return;
  }

  if(node->Expanded == -1 || (node->Expanded == 0 && (!(OBSERVABILITY==OBS_FULL && Start->BestAction) || node != Start))){
    expandNode(node, currHorizon);

    NumExpandedStatesIter++;
    if(DO_INCREMENTAL_HEURISTIC && node->hIncrement <= MAX_H_INCREMENT)
      return;

    LeafStates->remove(node);

    bool removedOrig;
    if(gBackupMethod == combine) 
      MOBackup(node, currHorizon);

    if(node->NextActions == NULL || node->BestAction == NULL){
      cout << "NO ACTION " <<  node->StateNo <<endl;
      return;
    }

    if(!ENUMERATE_SPACE)
     return;
  }
  else
    node->Expanded = Iteration;

  /* Assume successor states are sorted in order of probability */
  // expand default action
  for(successor = node->moValueFunction->actionDefault->NextState; 0 && successor; successor = successor->Next) {    
    /* Stop if already visited this iteration or goal state */
    if((successor->State->Expanded < Iteration)) {
      /* If already expanded, just mark as visited this iteration */
      if (successor->State->Expanded > 0)
        successor->State->Expanded = Iteration;        
      expandSolution(successor->State, currHorizon+1, dfs_depth, path_pr*successor->Prob);
    }
  }

  double sn1 = ((double)randomGen->myrand()/((double)(RAND_MAX)+(double)(1)))*node->moValueFunction->points.size(); 
  double cumm1 = 0.0;

  //expand value function points
  for(MOValueFunctionHash::iterator sol = node->moValueFunction->points.begin(); sol != node->moValueFunction->points.end(); sol++){
    if(!(*sol) || (*sol)->bestAction == NULL || (*sol)->solution)
      continue;

    cumm1++;

    for(int randIters = 0; (randIters < 1 ); randIters++){
      double sn = ((double)randomGen->myrand()/((double)(RAND_MAX)+(double)(1))); 
      double cumm = 0.0;
      for(successor = (*sol)->bestAction->NextState; successor != NULL; successor = successor->Next){
        cumm += successor->Prob;

        /* Stop if already visited this iteration or goal state */
        if((successor->State->Expanded < Iteration)){
          if(cumm < sn && MOLAORand)
            continue;
      
        if(successor->State->goalSatisfaction == 1.0)
          continue;
        
        /* If already expanded, just mark as visited this iteration */
        if(successor->State->Expanded > 0)
          successor->State->Expanded = Iteration;

        expandSolution(successor->State, currHorizon+1, dfs_depth, path_pr*successor->Prob);
        if(MOLAORand)
          break;
        }
      }
      if(!MOLAORand)
        break; 
    }
  }

  /* Possibly perform backup when backtrack to this node */

  double back;
  bool removedOrig;

  if(gBackupMethod == combine)
    back = MOBackup(node, currHorizon);
}

void MOLAOStar::expandNode(struct StateNode *node, int currHorizon){
  if(DO_INCREMENTAL_HEURISTIC && node->hIncrement < MAX_H_INCREMENT){
    increment_heuristic(node);
    return;
  }

  if(max_horizon != -1 && currHorizon > max_horizon)
    return; 

  if(node->NextActions == NULL || node->Expanded == -1){
    node->horizon = currHorizon;
    node->expand();
  }

  node->Expanded = Iteration;

  /* set heuristic value */
  if(gBackupMethod == separate)
    ExpandedStateList->push_back(node);
  NumExpandedStates++;
}

StateList* MOLAOStar::findAncestors(StateList *ExpandedStateList){
  StateList AddList, NewAddList;
  StateList *AncestorList;
  StateList::iterator node;
  ActionNodeList::iterator prev;

  AncestorList = new StateList;

  /* Initial AddList consists of all expanded states */
  for(node = ExpandedStateList->begin(); node != ExpandedStateList->end(); node++){
    (*node)->Update = Iteration;
    AddList.push_back(*node);
  }

  /* Find ancestor states that need to be updated */
  while(!AddList.empty()){  /* check for empty list */
    NewAddList.clear();
    /* For each state added to Z ... */
    for(node = AddList.begin(); node != AddList.end(); node++){
      /* ... and for each parent of that state ... */
      for(prev = (*node)->PrevActions->begin(); prev != (*node)->PrevActions->end(); prev++){
        ActionNode* prevAct = *prev;
        /* only add a parent along a marked action arc */
        /* also, parent must be expanded */
        if(prevAct == prevAct->PrevState->BestAction && prevAct->PrevState->Expanded > 0){
          /* don't add parent that is already in ancestor list */
          if(prevAct->PrevState->Update < Iteration){
            NewAddList.push_back(prevAct->PrevState);
            prevAct->PrevState->Update = Iteration;
          }
        }
      }

      AncestorList->push_back(*node);
      NumAncestorStatesIter++;
    }

    AddList.swap(NewAddList);
  }

  return AncestorList;
}

bool MOLAOStar::convergenceTest(){
/* From start state, perform depth-first search of all states 
   visited by every best solution. Mark each state when it is visited
   so that it is visited only once each pass. For each state:
   -- perform backup
   -- update Bellman residual
   If visit unexpanded state, exit with FALSE. 
   If error bound <= epsilon, exit with TRUE.
   Otherwise repeat.
*/

  double error, lastError;
  cout << "Enter Converge Test" << endl;

  ExitFlag = 0;
  error = DBL_MAX;

  do{
    NumSolutionStates = 0;
    Residual = 0.0;

    Iteration++;
    convergenceTestRecursive(Start, 0, 1.0, false);

    if(gDiscount < 1.0)
      error = (gDiscount * Residual) / (1.0 - gDiscount);
    else{
      double maxStartMFP = 0.0;
      for(MOValueFunctionHash::iterator pt = Start->moValueFunction->points.begin(); pt != Start->moValueFunction->points.end(); pt++){
        if(!(*pt)->solution)
          continue;

        if((*pt)->meanFirstPassage > maxStartMFP)
          maxStartMFP = (*pt)->meanFirstPassage;
      }

      printf("Max meanFirstPassage = %f\n", maxStartMFP);
      printf("Residual = %f\n", Residual);
      lastError = error;
      error = (maxStartMFP - 1.0) * Residual;

      if(error > lastError){
        cout << "Error increased " << lastError << " " << error << endl;
      }
    }

    printf("Exit flag = %d\n", ExitFlag); 
    printf("  Error bound: %f\n", error); 
  }
  while(!ExitFlag && (error > gEpsilon));

  cout <<"done"<<endl;

  if(ExitFlag || error > gEpsilon)
    return false;
  else
    return true;
}

bool MOLAOStar::convergenceTestRecursive(struct StateNode* stateNode, int currHorizon,  double path_pr, bool checkingNonSolutionPt){
  int oldAct, newAct;
  struct StateDistribution *successor;
  double Diff;

  /* Count this node as part of best solution */
  NumSolutionStates++; 
  stateNode->Update = Iteration;

  bool solutionPoint = false;

  for(MOValueFunctionHash::iterator pt = stateNode->moValueFunction->points.begin(); pt != stateNode->moValueFunction->points.end(); pt++){
    solutionPoint = (*pt)->solution;
    if(!solutionPoint && stateNode == Start)
      checkingNonSolutionPt = true;
    else if(stateNode == Start)
      checkingNonSolutionPt = false;

    if((*pt)->solution && (*pt)->bestAction == NULL)
      continue;

    /* Exit convergence test if best solution has unexpanded node */
    if((!(*pt)->bestAction  && checkingNonSolutionPt && !solutionPoint) || (checkingNonSolutionPt && !(*pt)->stateNode->Expanded)) {
      ExitFlag = 1;
      return false;
    }
    else if((*pt)->bestAction == NULL)
      return true;

    /* Recursively consider unvisited successor nodes */
    for(successor = (*pt)->bestAction->NextState; successor; successor = successor->Next){
      if(successor->State->Update != Iteration){
        successor->State->Update = Iteration;
        convergenceTestRecursive(successor->State, currHorizon + 1, 0, (!solutionPoint && checkingNonSolutionPt));     
      }  
    }
  }

  /* Do backup to improve value of node */
  Diff = MOBackup(stateNode, currHorizon);
  if(Diff > Residual)
    Residual = Diff;

  return false;
}

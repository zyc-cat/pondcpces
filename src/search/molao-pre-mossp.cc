/* lao.c */

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
//#include "lao_wrapper.h"        // expandState
#include "graph.h"              // StateNode::expand
#include "vi.h"
#include <iostream>
#include <float.h>
#include <math.h>

using namespace __gnu_cxx;
using namespace std;

/* Static variables */
extern list<StateNode*> terminals;
extern StateNode* lastBackup;

void MOLAOStar()
{
  StateList *AncestorList;
  struct StateNode *node;
  lastBackup = Start;

  dfs_limit=Start->f;
  for (Iteration = 1; ;Iteration++) {

    /* Expand best partial solution */
    NumExpandedStatesIter = 0;
    ExpandedStateList = new StateList;

    MOExpandSolution(this, lastBackup, 0, 0, 1.0);  


         printf("Expanded Solution\n");
    /* Call convergence test if solution graph has no unexpanded tip states */

    Start->moValueFunction->print(cout);
    MOValueFunctionPoint * pt = Start->moValueFunction->solutionNoLessThan(goal_threshold);
    if(pt &&
       MOConvergenceTest() &&
       (pt = Start->moValueFunction->solutionNoLessThan(goal_threshold))){
      cout << "FoundSolution" << endl;
      printPolicy(pt);
      cout << "Num Efficient Solutions = " << Start->moValueFunction->numSolutions() << endl;
      exit(0);
    }

/*     if(Start->Solved == 1){ */
    if ( (
    //#ifndef PPDDL_PARSER
    (NumExpandedStatesIter == 0)  &&  
    //#endif
   0 &&  MOConvergenceTest() )
   ||  (allowed_time > -1 && (clock()-gStartTime)*1000/CLOCKS_PER_SEC > allowed_time) //allowing for lcp's
   ) {
      //            cout << "HOOO " << Start->ExtendedGoalSatisfaction << endl;
      
      if(Start->moValueFunction->solutionNoLessThan(goal_threshold) != NULL &&
   //Start->ExtendedGoalSatisfaction >= goal_threshold &&
   goal_threshold > 0.0){

  break; //prevents anytime algo


//   printf("\n**********************\nFound P(G) = %f\n", Start->ExtendedGoalSatisfaction);
//   printf("%3d ( %f secs.)",  
//          Iteration, (float)(clock()-gStartTime)/CLOCKS_PER_SEC); 
//   printf("  f: %f, p: %f\n**********************\n", Start->f,
//          Start->ExtendedGoalSatisfaction); fflush(stdout);
  //exit(0);
 
  double t = ((1.0 - Start->ExtendedGoalSatisfaction)/10.0) + 
    Start->ExtendedGoalSatisfaction;

  if(t < 1.0){
    goal_threshold = t;
    //    cout << "THRESHOLD = " << goal_threshold << endl;
    //update_leaf_h_values();
  }
  else
    break;//return;
  //   backup_graph();
  cout << (float)(clock()-gStartTime)/CLOCKS_PER_SEC 
       << " " << Start->f
       << " " << Start->ExtendedGoalSatisfaction 
       << " " << goal_threshold
       << endl;
      }  
      else if(goal_threshold == 0.0)
  break;
      else if(0 && Start->Solved){
  Start->ExtendedGoalSatisfaction = 1.0;
  break;
      }
      
      //taken out for real-time search
      ///break;

      if(Start->ExtendedGoalSatisfaction == 1.0)
  break;


      }
      /*  }*/

    /* Dynamic programming step */
    /* (Skip if backups were performed in expansion step) */
    if( gBackupMethod == separate)
      {
  /* Update state costs and best partial solution */
  NumAncestorStatesIter = 0;
  /* the following condition is necessary in case no nodes are
     expanded but the convergence test fails */
  if (NumExpandedStatesIter > 0) { 
    AncestorList = findAncestors(ExpandedStateList);
    /* note that expanded nodes are included as ancestors */
/*     printf("  ancestors: %d", NumAncestorStatesIter); */
/*     printf("  explicit graph: %d", NumExpandedStates); */
    ValueIteration(AncestorList, 3);
    delete AncestorList;
  }
      }

    delete ExpandedStateList;
    /* NumSolutionStates includes GOAL and WALL */
/*     printf("  ExpandedStates: %d", NumExpandedStatesIter); */
  }
  /* write solution to file? */

  //        DisplaySolution();  
  //  exit(0);
  if(Start->ExtendedGoalSatisfaction >= goal_threshold){
    printf("\nf: %f", Start->f);
    printf("  NumSolutionStates: %d  NumExpandedStates: %d\n",
     NumSolutionStates, NumExpandedStates);
    
  }
  //  printf(
}




void MOExpandSolution(struct StateNode *node, int currHorizon, int dfs_depth, double path_pr)
/* Returns list of expanded states */
/* if num expanded nodes exceeds limit, break */

{
  struct StateDistribution *successor;

   /* If not yet expanded, expand here.
     Then consider successor nodes for expansion. */

  //  printf("Expand Solution, for state = %d, horizon = %d \n",  node->StateNo, currHorizon);  
  //  node->moValueFunction->print(cout);

  /*     printBDD(node->dd); */
  //cout << node->StateNo << "|" <<flush;


  if( node->Terminal == 1 ){
    //  cout << "terminal state = " << node->StateNo << endl;
    return;
  }

  double nextCost = 0.0;

  if(!node->dd){
    cout << "no dd for node " << node->StateNo <<endl;
    return;
  }

  if (node->Expanded == -1 ||
      (node->Expanded == 0 && 
      (!(OBSERVABILITY==OBS_FULL && Start->BestAction) ||
       node != Start))) {
    MOExpandNode(node, currHorizon);

    //    lastBackup = Start;
    NumExpandedStatesIter++;
    if(DO_INCREMENTAL_HEURISTIC && node->hIncrement <= MAX_H_INCREMENT)
      return;

    LeafStates->remove(node);

    //    cout << "#" << endl << flush;
    bool removedOrig;
    if(gBackupMethod == combine) 
      MOBackup(node, currHorizon, &removedOrig);
/*          if(node->f == 99999999.9 )  */
//    cout << "#" << flush;

//#ifndef PPDDL_PARSER
    if((!node->NextActions || !node->BestAction) ){
      //      cout << "NO ACTION " <<  node->StateNo <<endl;
      return;
    }

    //printAction(node->BestAction);

    if(0){//&& dfs_depth < dfs_limit       ){//dfs-ao* w/gradient check
      dfs_depth+=1;
     for (successor = node->BestAction->NextState;
       successor;
       successor = successor->Next) {  
       nextCost += successor->Prob * successor->State->f;
     }
     //nextCost += node->BestAction->Cost;
     if(nextCost > node->f){
       cout << "Gradient is no good " << nextCost << " " << node->h <<endl;
       return;
     }
    }
    else if(1){//ao*
//       if(rand()%(int)node->BestAction->NextState->State->h >
//    node->BestAction->NextState->State->h/2)

//      cout << "exit node gen = " << node->StateNo <<endl;
      if(!ENUMERATE_SPACE)
       return;
    }
    else if(0){//dfs
    }
    //#else
    //   return;
    //#endif
    }
  else{
    node->Expanded = Iteration;
  }

 
  if(node->BestAction){ 



      /* Assume successor states are sorted in order of probability */

      //expand default action
      for (successor = node->moValueFunction->actionDefault->NextState;
     0 && successor; successor = successor->Next) {  
  /* Stop if already visited this iteration or goal state */
  if ((successor->State->Expanded < Iteration)) {
    /* If already expanded, just mark as visited this iteration */
    if (successor->State->Expanded > 0)
      successor->State->Expanded = Iteration;      
    MOExpandSolution(successor->State, currHorizon+1, dfs_depth, path_pr*successor->Prob);
  }
      }

      double sn1 = ((double)randomGen->myrand()/((double)(RAND_MAX)+(double)(1)))*node->moValueFunction->points.size(); 
      double cumm1 = 0.0;

      //node->moValueFunction->print(cout);

      //expand value function points
      for(__gnu_cxx::MOValueFunctionHash::iterator sol = node->moValueFunction->points.begin();
    sol != node->moValueFunction->points.end();
    sol++){


  //  cout << "HO" << endl;
  if(!(*sol) || (*sol)->bestAction == NULL || (*sol)->solution)
    continue;

  //cout << "IHO" << endl;

  
  
  
  cumm1 += 1.0;
  if(cumm1 < sn1){
    //continue;
  }

  //cout << currHorizon << " "; printAction((*sol)->bestAction); 

  for(int randIters = 0; (randIters < 1 ); randIters++){
  double sn = ((double)randomGen->myrand()/((double)(RAND_MAX)+(double)(1))); 
  double cumm = 0.0;
  for (successor = (*sol)->bestAction->NextState;
       successor;
       successor = successor->Next) {  



    cumm += successor->Prob;

    /* Stop if already visited this iteration or goal state */
    if ((successor->State->Expanded < Iteration)  ) {

      if(cumm < sn && MOLAORand){      
        continue;
      }
    
      if(successor->State->goalSatisfaction == 1.0)
        continue;
      
      /* If already expanded, just mark as visited this iteration */
      if (successor->State->Expanded > 0){
        successor->State->Expanded = Iteration;
        //        cout << successor->State->StateNo << " " << successor->State->Expanded << endl;
      }

      MOExpandSolution(successor->State, currHorizon+1, dfs_depth, path_pr*successor->Prob);
      if(MOLAORand)
      break;
    }
    else{
      //      cout << "successor already expanded state = " << successor->State->StateNo << endl;
    }
  }
  if(!MOLAORand){
    break; 
  }
  }
  //break;
  //cout << "Done expand successors of vfp of " << node->StateNo << endl;
    }
      
  }  
  else{
    //    cout << "not expanded, node: " << node->StateNo <<endl;   
    return;
  }

  //  cout << "about to backup, state = " << node->StateNo << endl;
  /* Possibly perform backup when backtrack to this node */

  double back;
  bool removedOrig;
  //  ActionNode* bestA = node->BestAction;
  if( gBackupMethod == combine )
    back = MOBackup(node, currHorizon, &removedOrig);

//   if(back == 0.0)// || node->BestAction == bestA)
//     lastBackup = node;
  
//   if(node->ExtendedGoalSatisfaction >= goal_threshold)
//     lastBackup = Start;
//  cout << "exit node = " << node->StateNo <<endl;
      
}




  



static void MOExpandNode(struct StateNode *node, int currHorizon)
{
  if(DO_INCREMENTAL_HEURISTIC && node->hIncrement < MAX_H_INCREMENT){
    increment_heuristic(node);
    return;
  }

  if(max_horizon != -1 && currHorizon > max_horizon)
    return; 

  if(!node->NextActions  || node->Expanded == -1){
    node->horizon = currHorizon;
    node->expand();
  }

  node->Expanded = Iteration;

  /* set heuristic value */
  if(gBackupMethod == separate)
    ExpandedStateList->push_back(node);
  NumExpandedStates++;
}

StateList* findAncestors(StateList *ExpandedStateList){
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

bool gotDom;
int MOConvergenceTest()
/* From start state, perform depth-first search of all states 
   visited by best solution. Mark each state when it is visited
   so that it is visited only once each pass. For each state:
   -- perform backup
   -- update Bellman residual
   If visit unexpanded state, exit with FALSE. 
   If error bound <= epsilon, exit with TRUE.
   Otherwise repeat.
*/
{
  double error;
  // printf("Enter Converge Test \n"); 

  /* if(Start->Solved != 1) */
/*     return FALSE; */


  ExitFlag = 0;

  do {
 
    NumSolutionStates = 0;
    Residual = 0.0;
    bool removedOrig;
    /*DB    ExitFlag =  1-*/

    
    MOValueFunctionPoint *pt;
     do {
           cout << "Convergence Check";
      Iteration++;
      gotDom = false; 
      pt = Start->moValueFunction->solutionNoLessThan(goal_threshold);
      removedOrig = MOConvergenceTestRecursive( pt, 0, 1.0);
       pt = Start->moValueFunction->solutionNoLessThan(goal_threshold);
      Start->moValueFunction->print(cout);
     }while(0 && pt && removedOrig);
     if(!pt)
       ExitFlag = 1;

//    Iteration++;
//    for(__gnu_cxx::MOValueFunctionHash::iterator i = Start->moValueFunction->points.begin();
//   i != Start->moValueFunction->points.end(); ){
//       removedOrig = false;
//       MOValueFunctionPoint *mvf = *i;
//       if((*i)->solution){
//   removedOrig = MOConvergenceTestRecursive( *i, 0, 1.0);
//   Start->moValueFunction->print(cout);
//       }
//       if(removedOrig){
//   cout << "Restart Convergence Check" << endl;
//   i = Start->moValueFunction->points.begin();
//   Iteration++;
//       }
//       else if (mvf == Start->moValueFunction->solutionNoLessThan(goal_threshold)){
//   //cout << "GOT IT" << endl;
//   return 1;
//       }
//       else
//   i++;
//     }
    if ( gDiscount < 1.0 )
      error = -10.0;//(gDiscount * Residual)/(1.0 - gDiscount);
    else
      error = -10.0;//Start->meanFirstPassage * Residual;
//     printf("Exit flag = %d\n", ExitFlag); 
//     printf("  Error bound: %f\n", error); 
    /*     printf("\n%3d ( %f secs.)",  */
    /*      Iteration, (float)(clock()-gStartTime)/CLOCKS_PER_SEC); */
    /*     printf("  f: %f", Start->f); */
    //     cout << (error > gEpsilon) <<endl;
  } while (gotDom && !ExitFlag && (error > gEpsilon));
  //  cout <<"done"<<endl;


  if (ExitFlag || (error > gEpsilon) )
    return( 0 );
  else
    return( 1 );
}



static bool  MOConvergenceTestRecursive( MOValueFunctionPoint *pt,
              int currHorizon,
              double path_pr)
{
  int oldAct, newAct;
  struct StateDistribution *successor;
  double Diff;
  struct StateNode* stateNode = pt->stateNode;
  bool removedOriginal = false;

  //cout << "Convergence Test Recursive for state = " << pt->stateNode->StateNo << " " <<  pt << endl; 
    /*   printBDD(node->dd); /*

  /* Count this node as part of best solution */
  NumSolutionStates++; 

  pt->stateNode->Update = Iteration;
  /* Terminate recursion at goal node */
  if (!pt->bestAction){
    //   cout << "hit terminal"<<endl;
    //ExitFlag = 0;
    return false;
  }

  if (pt->dominated || pt->bestPoints.size() == 0) {
    //ExitFlag = 1;
    gotDom = true;
    //   stateNode->moValueFunction->print(cout);
    //      cout << "dominated " << pt->stateNode->StateNo <<endl;
 
//       Diff = MOBackup(nxt->State, currHorizon);

    Diff = MOBackup(pt->stateNode, currHorizon, &removedOriginal); 
//    stateNode->moValueFunction->print(cout);
//     for(struct StateDistribution* nxt = pt->bestAction->NextState;
//    nxt; nxt = nxt->Next){
//        cout << nxt->Prob << " " << nxt->State->StateNo << endl;
//        nxt->State->moValueFunction->print(cout);
//      }
    // for(__gnu_cxx::MOValueFunctionHash::iterator i = pt->stateNode->moValueFunction->points.begin();
//   i != pt->stateNode->moValueFunction->points.end(); i++){
//       if(!(*i)->solution){
//   (*i)->h+=1;
//   (*i)->f=(*i)->g+gWeight(*i)->h;
//       }
//      }
    //stateNode->moValueFunction->print(cout);
    return (removedOriginal);
  }

  /* Exit convergence test if best solution has unexpanded node */
  if (!pt->stateNode->Expanded) {
    ExitFlag = 1;
    //cout << "unexpanded"<<endl;
     return false;
  }

  /* Recursively consider unvisited successor nodes */
  for(successor = pt->bestAction->NextState; successor; successor = successor->Next){
    

    MOValueFunctionPoint *spt = NULL;
    for (list<MOValueFunctionPoint*>::iterator sp = pt->bestPoints.begin();
       sp != pt->bestPoints.end();
       sp++){
      if((*sp)->stateNode == successor->State){
  spt = *sp;
  break;
      }
    }
    
    if(!spt){//point became dominated, choose another point
      gotDom = true;
      //spt = pt->stateNode->moValueFunction->solutionNoLessThan(0.0);
    }

    //   printf("State %d, update = %d, iter = %d \n", successor->State->StateNo, successor->State->Update, Iteration); 
//     if(pt->dominated){
//       // ExitFlag = 1;
//       gotDom = true;
//       break;
//     }

    if (spt && (spt)->stateNode->Update != Iteration) {

      (spt)->stateNode->Update = Iteration;
     
      MOConvergenceTestRecursive(spt,currHorizon+1, 0);     
    }  
   //  else
//       printf("State again %d, update = %d, iter = %d \n", spt->stateNode->StateNo, spt->stateNode->Update, Iteration); 
  }
  
  /* Do backup to improve value of node */
  //oldAct = pt->bestAction->ActionNo;

  // printf("Convergence Test Backup for state = %d\n",pt->stateNode->StateNo);    
  Diff = MOBackup(pt->stateNode, currHorizon, &removedOriginal);
 
  if ( Diff > Residual ) Residual = Diff;
  //   printf("Residual = %f\n", Residual); 
  //newAct = pt->bestAction->ActionNo;
  
  //#ifndef PPDDL_PARSER
//   if(newAct != oldAct)// && my_problem->domain().requirements.non_deterministic)
//      ExitFlag = 1;
  //#endif

/*BD   return node->Solved; */
  return  (removedOriginal);

}

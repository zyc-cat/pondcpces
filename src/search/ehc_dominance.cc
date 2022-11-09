#include "ehc.h"

#include "globals.h"
#include "astar.h"
#include "lao_wrapper.h"        // printAction
#include "graph.h"              // StateNode::expand
#include "lug.h"
#include "math.h"
#include "backup.h"
#include "statistical.h"
#include <float.h>
#include <algorithm>

using namespace __gnu_cxx;
using namespace std;

extern double gWeight;

EHC::EHC()
: Search(EHC_SEARCH){
  reEvaluateChildren = false;
  INITLUG = false;
}

bool EHC::f_dominance(StateNode* lhs, StateNode* rhs){
  // used to heapify, where best nodes have highest value
  int s = SPRT(lhs, rhs);
  return lhs == rhs
      || (s == 1)
      || (s == 0
      && (lhs->hValues[lhs->hIncrement - 1] > rhs->hValues[rhs->hIncrement - 1]
      || (lhs->hValues[lhs->hIncrement - 1] == rhs->hValues[rhs->hIncrement - 1]
      &&  lhs->goalSatisfaction <= rhs->goalSatisfaction)))
      || (s != 0 && s != -1);
}

bool EHC::card_less_than(StateNode* a, StateNode* b){
  if(a == Goal || a->goalSatisfaction >= goal_threshold)
    return true;
  else
    return Cudd_CountMinterm(manager, a->dd, num_alt_facts) < Cudd_CountMinterm(manager, b->dd, num_alt_facts);
}

void EHC::printPlan(StateNode* tail){
  if(tail == Start){
    cout << "START" << endl;
    return;
  }
  else
    printPlan(tail->PrevActions->front()->PrevState);

  if(!tail->PrevActions->empty())
    printAction(tail->PrevActions->front());
  else
    cout << "NO BEST" << endl;
}

void EHC::setBestAct(StateNode* s){
  if(s == Start || s->Update == 1)
    return;

  s->Update = 1;
  for(ActionNodeList::iterator act = s->PrevActions->begin(); act != s->PrevActions->end(); act++){
    (*act)->PrevState->BestAction = (*act);
    setBestAct((*act)->PrevState);
  }
}

void EHC::backupSet(StateHash* nodes){
  bool changed = true;
  while(changed){
    changed = false;
    for(StateHash::iterator i = nodes->begin(); i != nodes->end(); i++){
      double of = (*i).second->f;
      Backup((*i).second, (int)(*i).second->g);
      if(of != (*i).second->f)
        changed = true;
    }
  }
}

StateNode* EHC::findBetterNode(StateNode* root){
  std::deque<StateNode*> StateQ;
  std::list<StateNode*>  tmpList;
  StateQ.push_back(root);

  StateNode* bestSuccessor = root;
  double oldSuccessorH, oldRootH = root->h;
  StateHash Nodes;


  while(!StateQ.empty()){
    StateNode* front = StateQ.front();
    StateQ.pop_front();

    if(front->NextActions == NULL){
      cout << "." << flush;
      front->expand(); 
      ExpandedStates++; 
    }
    else if(front->BestAction != NULL && front != root) {
      cout << "CYCLE " << front->StateNo << endl;
      bestSuccessor = NULL;
      break;
    }

    if(front->NextActions == NULL){
      cout << "DEAD END " <<  endl;
      front->h = DBL_MAX;
      front->g = 0;
      front->Terminal = 1;
      bestSuccessor = NULL;
      break;
    }
    
    for(ActionNodeList::iterator act = front->NextActions->begin(); act != front->NextActions->end(); act++){
      // assuming each act has one successor because otherwise we would need AO*
      StateNode* successor = (*act)->NextState->State;

      if(!Nodes.contains(successor)){    //new node
        NodesExpanded.add(successor);
        Nodes.add(successor);
        StateQ.push_back(successor);

        if((DO_INCREMENTAL_HEURISTIC
        && (bestSuccessor == root || !f_dominance(successor, bestSuccessor))
        && !f_dominance(successor, root))
        || (!DO_INCREMENTAL_HEURISTIC && (successor->h < root->h))) {
          bestSuccessor = successor;
        }
      }

      if(successor == Goal || successor->goalSatisfaction >= goal_threshold){
        successor->Terminal = 1;
        bestSuccessor = successor;
        cout << "GOAL" << endl;
        break;
      }      
    }

    if(bestSuccessor != NULL && bestSuccessor != root && (bestSuccessor == Goal || bestSuccessor->goalSatisfaction >= goal_threshold)){
      cout << "GOAL" << endl;
      bestSuccessor->h = 0;
      bestSuccessor->g = 0;      
      break;
    }
    else if(bestSuccessor != NULL && bestSuccessor != root)
      break;
  }
  bestSuccessor = recursive(bestSuccessor);

  if(bestSuccessor != Goal && bestSuccessor->goalSatisfaction < goal_threshold)
    return NULL;
  else
    return bestSuccessor;
}

StateNode* EHC::recursive(StateNode* root){
  deque<StateNode*> tmpList;
  list<StateNode*> tmpList1;

  StateNode *rnode;
  ActionNode* bestAct;

  if(root == NULL){
    cout << "NULL" << endl;
    return NULL;
  }

  // needed if nodes are already in graph
  if(root->Expanded == -1){
    tmpList1.push_back(root);
    getHeuristic(&tmpList1, root, 0);
    tmpList1.clear();
    root->Expanded = 0;
  }

  if(root->Terminal == 1){
    cout << "Hit terminal " << root->StateNo  
         << (root == Goal || root->goalSatisfaction >= goal_threshold ? " GOAL" : " NO GOAL")
         << (root->Solved ? " Solved" : " Not Solved")
         << endl;

    if(root != Goal && root->goalSatisfaction < goal_threshold){
      root->h = DBL_MAX;
      root->g = 0;
      root->Solved = 0;
      root = NULL;
    }
    return root;
  }

  if(root->h == DBL_MAX){
    cout << "BAD SUFFIX " << root->StateNo << endl;
    return NULL;
  }

  if(root->BestAction != NULL && root->Expanded != -1)
    rnode = recursive(root->BestAction->NextState->State);
  else{
    cout << endl << "[" << root->h << ", " << root->hIncrement
	     << ", " << root->goalSatisfaction << "]" << flush;

    StateNode* bestNode = NULL;
    if(root == Goal || root->goalSatisfaction >= goal_threshold){
      Start->ExtendedGoalSatisfaction = root->goalSatisfaction;
      root->Terminal = 1;
      root->h = 0;
      root->g = 0;
      rnode = root;
    }
  
    if(!root->BestAction && root->Terminal != 1){
      root->expand();
      ExpandedStates++;

      // needed if nodes are already in graph
      if(root->Expanded == -1){
        for(ActionNodeList::iterator act = root->NextActions->begin(); act != root->NextActions->end(); act++){
          for(StateDistribution* successor = (*act)->NextState; successor != NULL; successor = successor->Next){
            if(successor->State->Expanded == -1){
              successor->State->Expanded = 0;
              tmpList1.push_back(successor->State);
              getHeuristic(&tmpList1, root, 0);
              tmpList1.clear();
            }
          }
        }
      }

      if(root->NextActions == NULL){
        cout << "DEAD_END " << root->StateNo << endl;
        root->h = DBL_MAX;
        root->g = 0;
        root->Terminal = 1;
        root->Solved = 0;
        return NULL;	
      }      
    }

    for(ActionNodeList::iterator act_it = root->NextActions->begin(); act_it != root->NextActions->end(); act_it++){
      ActionNode* action = *act_it;

      // assuming each act has one successor because otherwise we would need AO*
      StateNode* successor = action->NextState->State;

      if(!NodesExpanded.contains(successor)){   // new node
        NodesExpanded.add(successor);
        tmpList.push_back(successor);
    }


      if(successor == Goal || successor->goalSatisfaction >= goal_threshold){
        bestNode = successor;
        bestAct = action;
        cout << "GOAL" << endl;
        break;
      }

      if((!DO_INCREMENTAL_HEURISTIC && successor->h < root->h)
      || ( DO_INCREMENTAL_HEURISTIC && (!bestNode || !f_dominance(successor, bestNode)))
      && !f_dominance(successor, root)){
        bestAct = action;
        bestNode = successor;
      }
    }

    if(bestNode != NULL && (bestNode == Goal || bestNode->goalSatisfaction >= goal_threshold)){
      cout << "GOT GOAL"<<endl;
      bestNode->h = 0;
      bestNode->g = 0;
      bestNode->Terminal = 1;
      root->Solved;
      root->BestAction = bestAct;
      return bestNode;
    }

    if(bestNode != NULL)
      rnode = recursive(bestNode);
    else
      rnode = findBetterNode(root);
  }

  return rnode;
}

void EHC::search(){
  StateNode *end;

  ExpandedStates = 0;
  Start->ExtendedGoalSatisfaction = 1.0;    // hack
  Start->f *= gWeight;

  while(Start->Solved == 0 && Start->h != DBL_MAX){
    cout << "**New EHC Trial**" << endl;
    end = recursive(Start);
    if(end->Terminal == 1){
      Start->Solved = 1;
      setBestAct(end);
    }
  }

  if(Start->h == DBL_MAX){
    cout << "No Solution" << endl;
    return;
  }

  if(end == NULL){
    Search *search = new AStar();
    search->search();
    delete search;
  }
  else{
    cout << "ending EHC " << end->StateNo <<endl;
    cout << Start->ExtendedGoalSatisfaction << endl;
  }

  cout << "Expanded States = " << ExpandedStates << endl;
}

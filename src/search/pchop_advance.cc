#include "pchop_advance.h"

#include "NSPolicy.h"
#include "globals.h"        // verbosity
#include <iostream>         // cout

using namespace std;

PCHOPAdvance::PCHOPAdvance(int numSamples, int maxDepth){
  this->numSamples = numSamples;
  PolicyNode::maxDepth = maxDepth;
}

// TODO: Sometimes returning state or NULL as the "goal" isn't necessarily true
// (i.e. while we haven't reached a "goal", we haven't hit a dead-end either)
ActionNode* PCHOPAdvance::findBestAction(StateNode* state){
  PolicyState root(state, numSamples, state->prReached);

  set<PolicyState*> open;
  open.insert(&root);

  do{
    for(set<PolicyState*>::iterator state_it = open.begin(); state_it != open.end(); state_it++)
      (*state_it)->expand();

    renewFringe(open, root);
  }
  while(!open.empty());

  if(root.getF() >= PolicyState::maxDepth){
    if(verbosity >= 1)
      cout << "XX-- NO SOLUTION --XX\n";
    return NULL;
  }

  if(verbosity >= 1){
    if(root.getBestAction() == NULL)
      cout << "*** NO BEST ACTION ***\n";
    else
      cout << "*** Found best action (action " << root.getBestAction()->ActionNo << ", distance to goal = " << root.getF() << ") ***\n";
  }

  return root.getBestAction();
}

void PCHOPAdvance::renewFringe(set<PolicyState*> &fringe, PolicyState& root){
  fringe.clear();

  set<PolicyState*> open;
  open.insert(&root);

  PolicyState* front = NULL;

  while(!open.empty()){
    front = *open.begin();
    open.erase(open.begin());

    if(front->getBestPolicyAction() != NULL){
      vector<PolicyState*> &states = front->getBestPolicyAction()->getChildren();
      for(vector<PolicyState*>::iterator state_it = states.begin(); state_it != states.end(); state_it++){
        open.insert(*state_it);
      }
    }
    else{
      if(front->isGoal()){
//        front->setBestAct();
//        front->freeze();
      }
      else if(!front->isDeadEnd())
        fringe.insert(front);
    }
  }
}

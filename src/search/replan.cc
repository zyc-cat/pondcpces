#include "replan.h"

#include <set>

using namespace std;

Replan::Replan(bool determinized)
: AnytimeSearch(REPLAN), AStarAdvance(determinized){
}

Replan::~Replan(){
}

bool Replan::solve(StateNode* start){

  StateNode* state = AStarAdvance::advance(start);
  //printStateList();
  if(state != NULL){
		setBestAct(state);
		return true;
	}

  return false;
}


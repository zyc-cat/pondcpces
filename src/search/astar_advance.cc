#include "astar_advance.h"

#include "globals.h"            // PointerHasher
#include "graph.h"              // StateNode, StateComparator
#include "search.h"
#include <set>
#include <deque>
#include <ext/hash_set>

using namespace __gnu_cxx;
using namespace std;

extern double gWeight;
extern void printBDD(DdNode*);

AStarAdvance::AStarAdvance(bool determinized){
	this->determinized = determinized;

	iteration = 0;
}

StateNode* AStarAdvance::advance(StateNode* start){
	set<StateNode*> states;
	states.insert(start);// 传入开始状态
	return advance(states);// 调用的搜索实现函数
}

StateNode* AStarAdvance::advance(set<StateNode*>& states){
	iteration++;// 迭代次数增加

  	open.clear();
	open.key_comp().init(StateComparator::F_VAL);//比较f

	StateNode *front = NULL;
	// 遍历每个节点
	for(set<StateNode*>::iterator state_it = states.begin(); state_it != states.end(); state_it++){
		front = *state_it;
		assert(front->Solved <= 0);// 需要每个均未solved
		front->BestPrevAction = NULL;// 初始化到达该state最佳action为NULL
		open.insert(front);// 加入open中
	}

	while(!open.empty()){
	  //	  printStateList();
		front = *open.begin();

		//printBDD(front->dd);
		// 判断是否到达目标或以处理
		if(front->isGoal() || front->Solved > 0){
			if(verbosity >= 1)
				cout << "\t" << front->StateNo << "\t" << front->g << "\t" << front->h << "\t"
					<< front->goalSatisfaction << "\t" << front->prReached << endl;
			return front;
		}
		// 移除open
		open.erase(open.begin());
		// 如果没办法拓展，设置为0，最佳action为空，因为没有后继状态
		if(front->Terminal > 0){
			front->Solved = 0;
			front->BestAction = NULL;
			continue;
		}
		// 如果还为拓展，进行拓展，最佳action为空
		if(front->Expanded <= 0){
			front->expand();
			front->BestAction = NULL;
		}
		// 设置是第几次迭代进行的expand
		front->Expanded = iteration;
		// 考虑每个满足条件的action
		for(ActionNodeList::iterator act_it = front->NextActions->begin(); act_it != front->NextActions->end(); act_it++){
			ActionNode* action = *act_it;

			double new_g = front->g + action->Cost;// 计算花费g

			double p = (double)rand() / (double)RAND_MAX;
			double cum_p = 0.0;
			// 考虑其状态分布
			for(StateDistribution* dist = action->NextState; dist != NULL; dist = dist->Next){
				StateNode* successor = dist->State;// 获得后继状态
				// cout << dist->Prob << endl;
				if(dist->Prob <= 0.0)// 概率为0，不可达
					continue;

				if(determinized){
					cum_p += dist->Prob;
					if(cum_p <= p)// 概率小于随即概率，忽略该分布 
						continue;
				}
				// 否则，查找该state
				set<StateNode *, StateComparator>::iterator successor_it = open.find(successor);
				// 不是第一次遇到
				bool inFringe = (successor_it != open.end());
				// 是否以及拓展了
				bool closed = (successor->Expanded == iteration);
				// 到达该节点的g小于原先的g
				bool cheaper = (new_g < successor->g);
				// 是否满足个更新条件，还未closed，第一次遇到\不是第一次遇到更便宜\拓展了但是更便宜
				if((!closed && (!inFringe || (inFringe && cheaper))) || (closed && cheaper)){
					action->Determinization = dist;
					successor->BestPrevAction = action;// 更便宜因此，更新该state的最佳action

					if(inFringe)// 存在open了移除，后续再添加
						open.erase(successor_it);

					if(max_horizon <= 0 && successor->h <= 0 && successor->goalSatisfaction < search_goal_threshold)
						successor->h = IPP_MAX_PLAN;
					// 更新cost
					successor->g = new_g;
					// 更新其 f = g + h
					successor->f = successor->g + gWeight * successor->h;
					
					double oldPrReached = successor->prReached;
					// 更新概率
					successor->prReached = front->prReached * dist->Prob;
					// 加入了，重新加入open
					if(inFringe)
						open.insert(successor_it, successor);
					else if(!closed)// 第一次加入
						open.insert(successor);
					// closed了，利用该节点更新后继状态
          			if(closed)
            			updateSuccessors(successor, new_g - successor->g, oldPrReached, successor->prReached);
				}// end if
				// 确定性只有一个后继state，利用前面赋值determinized为true，结束别的后继状态
				if(determinized)
					break;
			}// end-for StateDistribution
		} // end-for ActionNodeList
	}

	return NULL;
}

void AStarAdvance::
	
updateSuccessors(StateNode* start, double delta_g, double oldPrReached, double newPrReached){
  hash_set<StateNode *, PointerHasher> closed_up;
  set<StateNode *> open_up;
  open_up.insert(start);
  StateNode *front = NULL;

  while(!open_up.empty()){
    front = *open_up.begin();
    open_up.erase(open_up.begin());
    closed_up.insert(front);

    for(ActionNodeList::iterator act_it = front->NextActions->begin(); act_it != front->NextActions->end(); act_it++){
      ActionNode* action = *act_it;

      for(StateDistribution* dist = action->NextState; dist != NULL; dist = dist->Next){
        StateNode* successor = dist->State;

        if((determinized && action->Determinization != dist) || closed_up.count(successor) > 0 || dist->Prob <= 0.0)
          continue;

        set<StateNode *, StateComparator>::iterator successor_it = open.find(successor);
        bool inFringe = (successor_it != open.end());
        bool closed = (successor->Expanded == iteration);

        if(closed || inFringe){
          if(inFringe)
            open.erase(successor_it);

          successor->g += delta_g;
          successor->f = successor->g + gWeight * successor->h;
          successor->prReached = (successor->prReached / oldPrReached) * newPrReached;

          if(inFringe)
            open.insert(successor_it, successor);
          else// if(closed)
            open_up.insert(successor);
        }
      }
    }
  }
}
/**
 * 更新设置最佳action
 */
void AStarAdvance::setBestAct(StateNode* state){
	assert(state != NULL && (state->isGoal() || state->Solved > 0));

  if(state->Solved <= 0){// 第一次调用，实现初始化
		state->BestAction = NULL;
		state->Solved = 1;
	}
	
	ActionNode* prevAction = NULL;
	StateNode* prevState = NULL;
	// 从state开始传递更新其最佳action
	while((prevAction = state->BestPrevAction) != NULL && (prevState = prevAction->PrevState) != NULL && prevState->Solved <= 0){
		prevState->BestAction = prevAction;
		prevState->Solved = 1;
		state = prevState;
	}
}

#include "graph.h"

#include "globals.h"				// StateHash, PF_LUG, NUMBER_OF_MGS
#include "cudd/dd.h"				// DdNode
#include "lug.h"					// getHeuristic, increment_heuristic
#include "movalfn.h"				// MOValueFunction, MOValueFunctionPoint, getMOLAOHeuristics
#include "solve.h"				  // gWeight, gNumActions
#include "sample_size.h"			// random_walk_sample_size
#include "lao_wrapper.h"			// everything else (ex. computeGoalSatisfaction and other functions)
#include "track.h"
#include "cudd.h"
#include "relaxedPlan.h"
#include "search.h"
#include "NSPolicy.h"

#include <cmath>										// fabs
#include <stdio.h>
#include <stdlib.h>
#include <list>
#include <queue>
#include <vector>
#include <float.h>
#include <iomanip>
#include <fstream>

using namespace __gnu_cxx;
using namespace std;

extern double gWeight;
extern int num_lugs;

extern void computeSuccessors(pair<const Action* const, DdNode*>* action, 
		struct StateNode* parent, list<DdNode*>* successors,
		list<DdNode*>* reasons, list<set<const pEffect*>*>* creasons,
		list<double>* probabilities, list<double>* rewards,
		list<double>* klds, int reps);

hash_map<DdNode*, StateNode*, __gnu_cxx::hash<DdNode*>, eqstr> StateNode::generated;
int StateNode::expandedStates = 0;
int ActionNode::actionCount = 0;

/********************* StateNode functions **********************/

StateNode::StateNode(){
	dd = NULL;
	backup_dd = NULL;
	rp_states = NULL;

	Description = NULL;

	TerminalAction = NULL;
	NextActions = NULL;
	BestAction = NULL;
	PrevActions = NULL;

	Terminal = 0;
	meanFirstPassage = 0.0;
	Expanded = 0;
	expH = h = 0;
	g = 0;
	f = DBL_MAX;
	expDiscRew = 0;
	Solved = 0;
	bestActF = -1;
	ExtendedGoalSatisfaction = 0.0;
	num_supporters = 0;
	prReached = 0.0;
	ha = NULL;
	Backups = 0;
	horizon = 0;
	StateNo = -1;
	Update = 0;
	hIncrement = 0;
	numHvalues = 0;
	usedSamples = NULL;
	hValues = (double *)calloc(MAX_H_INCREMENT, sizeof(double));
	rpIncrement = NULL;
	currentRP = NULL;
	goalSamples = 0;
	kld = 1;
	moValueFunction = NULL;//new MOValueFunction();  // For MOLAOStar and MOAStar
	randPriority = rand();

//	PrevActions = new ActionNodeList();
//	NextActions = new ActionNodeList();		// Messes up StateNode::expand
	BestPrevAction = NULL;
	m_status = UNEXPANDED;
}

StateNode::~StateNode(){
/*
	if(dd != NULL)
		Cudd_Deref(dd);
	if(backup_dd != NULL && backup_dd != dd)
		Cudd_Deref(backup_dd);
	if(rp_states != NULL)
		Cudd_Deref(rp_states);
*/
	if(Description != NULL)
		free(Description);
	if(rpIncrement != NULL)
		delete rpIncrement;
	if(currentRP != NULL)
		delete currentRP;
/*
	if(usedSamples != NULL)
		Cudd_Deref(usedSamples);
*/
	if(ha != NULL)
		free(ha);
	if(hValues != NULL)
		free(hValues);
	if(moValueFunction != NULL)
		delete moValueFunction;
	if(NextActions != NULL)
		delete NextActions;
	if(PrevActions != NULL)
		delete PrevActions;
}

/**
 * momo007 2022.06.01
 * waring!!!, need rewrite
 */
bool StateNode::isGoal(){
	return bdd_is_one(manager,bdd_imply(manager, this->dd, Goal->dd));
}

// used for observational outcomes to check looping and change search graph successors
int StateNode::processSuccessors(list<StateNode*>* states, list<StateNode*>* fh_states){
	StateNode* parent = this;
	int currHorizon = parent->horizon + 1;
	StateDistribution* sdp;
	StateNode* tmp;
	int posLoops, negLoops;
	int loop_condition;
	int ok;
	list<StateNode*> h_states;

	for(list<StateNode*>::iterator j = states->begin(); j!= states->end(); j++){

		ok = TRUE;

		pair<StateHash::iterator,
		StateHash::iterator> q = StateIndex->equal_range((*j)->dd);
		for(StateHash::iterator i = q.first; i != q.second; i++){   // find which one it is
			if((*i).second->Terminal)
				continue;

			loop_condition = check_loop_conditions((*i).second, parent, parent->dd, currHorizon);

			if(loop_condition == 0){//make connection

				if((*i).second->Expanded == -1){
					h_states.push_back((*i).second);
				}

				if((*j)->PrevActions != NULL){
					for(sdp = (*j)->PrevActions->front()->NextState; sdp; sdp = sdp->Next){

						if(sdp->State->dd == (*j)->dd && sdp->State->StateNo == -1){
							if((*j)->PrevActions != (*i).second->PrevActions){
								if((*i).second->PrevActions != NULL)
									(*j)->PrevActions->insert((*j)->PrevActions->end(), (*i).second->PrevActions->begin(), (*i).second->PrevActions->end());
								(*i).second->PrevActions = (*j)->PrevActions;
							}

							sdp->State = (*i).second;
							break;
						}
					}

					(*j)->PrevActions = NULL;

				}
				(*j)->h = -5;
			}
			else if(loop_condition == 1){//don't make connection, delete node

				for(list<StateNode*>::iterator k = states->begin();
						k != states->end(); k++){
					if(*k){
						delete *k;
					}
				}
				return FALSE;
			}
			else if(loop_condition == 2){ // don't make connection, add as new node
				//	cout << "dok" << endl;
			}

		}
	}

	// add new nodes to graph
	for(list<StateNode*>::iterator j = states->begin();
			j!= states->end(); j++){

		if((*j)->h != -5){  // add to graph


			(*j)->StateNo = state_count++;
			StateIndex->add(*j);
			if(LUG_FOR != FRONTIER && LUG_FOR != INCREMENTAL)
				h_states.push_back(*j);
			else
				fh_states->push_back(*j);

			LeafStates->add(*j);
		}
		else{   // delete
			Cudd_RecursiveDeref(manager, (*j)->dd);
			(*j)->dd = NULL;

			delete *j;
		}
	}

	if(LUG_FOR != FRONTIER && LUG_FOR != INCREMENTAL && h_states.size() > 0){
		if(Search::type == MOLAO)
			getMOLAOHeuristics(&h_states, parent);
		else{
			if(DO_INCREMENTAL_HEURISTIC){
				for(list<StateNode*>::iterator hs = h_states.begin(); hs != h_states.end(); hs++)
					increment_heuristic(*hs);
			}
			else if (!DEFERRED_EVALUATION){
				list<StateNode*> m_states;
				for(list<StateNode*>::iterator hs = h_states.begin(); hs != h_states.end(); hs++){
					m_states.push_back(*hs);
					getHeuristic(&m_states, parent, currHorizon);
					m_states.clear();
				}
			}
		}
	}

	return TRUE;
}

void StateNode::expand(){
	struct StateNode* parent = this;
	int currHorizon = parent->horizon;
	struct StateNode* stateNode;// 创建新的state结点
	struct ActionNode* actionNode;// 创建新的action结点
	struct StateDistribution* stateDist, *stateDist1;
	list<DdNode*> successors, reasons;
	list<set<const pEffect*>*> creasons;
	list<double> probabilities, rewards, rwval, klds;

	int obs_count = 0;
	int found_repeat;
	DdNode* tmp,*obsOut, *fr, *fr1;
	char description[15];

	int num_relevant_obs =0;
	int loop_condition; /* 0 == ok, 1 == bad, 2 == don't know yet */
	int posLoops, negLoops;
	int obsRangeS, obsRangeE;
	int startNewNode;
	int reasons_size =0;
	list<StateNode*> states, h_states;
	DdNode* allrw,* tmprw;

	double goalSatisfaction; // probability of goal



	if(parent->NextActions && parent->Expanded > -1)
		return;
	else if(parent->NextActions && parent->Expanded == -1 && !DEFERRED_EVALUATION){
		list<StateNode*> hlist;
		for(ActionNodeList::iterator pa = parent->NextActions->begin(); pa != parent->NextActions->end(); pa++){
			hlist.push_back((*pa)->NextState->State);
			getHeuristic(&hlist, parent, 0);
			hlist.clear();
		}
		return;
	}

	if(max_horizon == -1 /* indefinite horizon */ || currHorizon < max_horizon){

		for(map<const Action*, DdNode*>::iterator a = action_preconds.begin(); a != action_preconds.end(); a++){
			const int MAX_MACRO_REPS = 1;

			if(strcmp((*a).first->name().c_str(), "noop") != 0 && first_act_noop && parent->StateNo == 1)
			  continue;

			if(strcmp((*a).first->name().c_str(), "noop_action") == 0)
				continue;

			for(int reps = 1; reps <= MAX_MACRO_REPS; reps++){

				if(HELPFUL_ACTS && parent->ha && !get_bit(parent->ha, gop_vector_length, (*a).first->id()))
					continue;
				//			std::cout << "[" << std::flush;
//																Cudd_CheckKeys(manager);
//																std::cout << "|" << std::endl;

				computeSuccessors(&(*a), parent, &successors, &reasons, &creasons, &probabilities, &rewards, &klds, reps);

//				Cudd_CheckKeys(manager);
//				std::cout << "]" << std::flush;

				actionNode = NULL;
				reasons_size = 0;
				num_relevant_obs = 0;

				if(successors.empty())
					continue;

				// put outcomes into search space
				list<DdNode*>::iterator r;
				list<set<const pEffect*>*>::iterator cr;
				if(OBS_TYPE == OBS_CPT){
					cr = creasons.begin();
				}
				else{
					r = reasons.begin();
				}

				list<double>::iterator v = probabilities.begin();
				list<double>::iterator rw = rewards.begin();
				list<double>::iterator kl = klds.begin();

				for(list<DdNode*>::iterator o = successors.begin(); o != successors.end(); o++){

					if(
							(my_problem->domain().requirements.probabilistic && Cudd_ReadZero(manager) != *o)
							|| (my_problem->domain().requirements.non_deterministic && Cudd_ReadLogicZero(manager) != *o)
					){

						num_relevant_obs++;

						goalSatisfaction = computeGoalSatisfaction(*o);

						if(generated.count(*o) == 0) //Don't add duplicate dd'
						{

							stateNode = new StateNode();
							states.push_back(stateNode);
							stateNode->dd = *o;
//							Cudd_Ref(stateNode->dd);

							stateNode->horizon = parent->horizon+1;//currHorizon;
							stateNode->kld = *kl;
							stateNode->goalSatisfaction = goalSatisfaction;
							stateNode->ExtendedGoalSatisfaction = stateNode->goalSatisfaction;
							stateNode->prReached = parent->prReached * (*v);

							if(!OPTIMIZE_REWARDS && OPTIMIZE_PROBABILITY)//probability
								stateNode->g = -1.0 * (stateNode->goalSatisfaction - parent->goalSatisfaction);

							if(Search::type == MOLAO){
								stateNode->moValueFunction->points.insert(new MOValueFunctionPoint(0.0, 0.0, 0.0,
										stateNode->goalSatisfaction,
										stateNode->goalSatisfaction,
										1, 0.0,
										NULL, true, stateNode));
							}
							generated[*o] = stateNode;
						}
						else
							stateNode = generated[*o];


						stateDist = CreateStateDistribution();
						stateDist->State = stateNode;
						if(OBS_TYPE==OBS_CPT)
							stateDist->creason = *cr;
						else
							stateDist->reason = *r;

						if(OBSERVABILITY!=OBS_FULL && OBS_TYPE!=OBS_CPT)
							Cudd_Ref(stateDist->reason);

						stateDist->Prob = *v;

						if(actionNode == NULL){
							actionNode = new ActionNode();
							actionNode->NextState = NULL;
						}

						stateDist->Next = actionNode->NextState;
						actionNode->NextState = stateDist;

						/* Also add pointers from next states back to this action. */
						if(stateNode->PrevActions == NULL)
							stateNode->PrevActions = new ActionNodeList;
						stateNode->PrevActions->push_front(actionNode);

						stateNode->horizon=parent->horizon+1;
						actionNode->act = (*a).first;
						actionNode->ActionNo = actionNode->act->id();

						if(my_problem->domain().requirements.rewards){
							stateNode->expDiscRew = (*rw);
							actionNode->Cost = 1.0;
							actionNode->expDiscRew += (*v)* stateNode->expDiscRew;
						}
						else
							actionNode->Cost = 1.0 * reps;


					}

					//Cudd_RecursiveDeref(manager,*o);

					//					if(OBSERVABILITY != OBS_FULL && OBS_TYPE!=OBS_CPT)
					//						Cudd_RecursiveDeref(manager,*r);

					if(OBS_TYPE==OBS_CPT)
						cr++;
					else
						r++;
					v++;
					kl++;
				}
				// sensor action
				if(successors.size() > 1 &&  num_relevant_obs >= 2 || successors.size() == 1 && num_relevant_obs == 1){
					if(!processSuccessors(&states, &h_states)){
						for(stateDist = actionNode->NextState; stateDist != NULL; ){
							stateDist1 = stateDist;
							stateDist = stateDist->Next;
							delete stateDist1;
						}
						delete actionNode;
					}
					else{
						if(parent->NextActions == NULL)
							parent->NextActions = new ActionNodeList;

						parent->NextActions->push_front(actionNode);

						/* create best action if it doesn't already exist */
						if(parent->BestAction == NULL)
							parent->BestAction = actionNode;

						/* pointer to state in which action is taken */
						actionNode->PrevState = parent;
					}
				}


				states.clear();
				successors.clear();
				reasons.clear();
				creasons.clear();
				probabilities.clear();
				rewards.clear();
				klds.clear();
			}
			//>>>>>>> .r207

			if((LUG_FOR == INCREMENTAL || LUG_FOR == FRONTIER) && !DEFERRED_EVALUATION && state_count > startNewNode && h_states.size() > 0)
				getHeuristic(&h_states, parent, currHorizon);
		}
	}

	parent->expH = h;
	parent->BestAction = NULL;

	// Add terminal state, so we can quit if we want to
	stateNode = new StateNode();
	StateIndex->add(stateNode);
	stateNode->StateNo = state_count++;
	stateNode->Terminal = 1;
	stateNode->dd = parent->dd;
	Cudd_Ref(stateNode->dd);
	stateNode->horizon = parent->horizon;

	if(my_problem->domain().requirements.rewards){
		stateNode->expDiscRew = 0;//pow(gDiscount, parent->horizon)*transform_reward_to_probability(0)/(1-gDiscount);
		//stateNode->applyDiscountedGoalProbability();
		stateNode->ExtendedGoalSatisfaction = computeGoalSatisfaction(stateNode->dd);
		stateNode->goalSatisfaction = stateNode->ExtendedGoalSatisfaction;
	}
	else{
		stateNode->expDiscRew = 0;//parent->expDiscRew;
		stateNode->ExtendedGoalSatisfaction = parent->ExtendedGoalSatisfaction;
		stateNode->goalSatisfaction = parent->goalSatisfaction;
	}

	if(stateNode->goalSatisfaction < goal_threshold){
		// How costly is it that you are not exceeding the goal threshold
		if(OBSERVABILITY == OBS_NONE)
			stateNode->g = DBL_MAX;
		else
			stateNode->h = 0;
	}

	stateNode->g = 0.0;

	stateNode->h = 0;
	stateNode->f = stateNode->g + stateNode->h;

	stateNode->BestAction = NULL;
	stateNode->NextActions = NULL;
	stateNode->PrevActions = new ActionNodeList;

	// set up terminal action
	parent->TerminalAction = actionNode = new ActionNode();
	if(parent->BestAction == NULL)
		parent->BestAction = actionNode;

	stateNode->PrevActions->push_back(actionNode);
	if(parent->NextActions == NULL)
		parent->NextActions = new ActionNodeList;
	parent->NextActions->push_front(actionNode);
	actionNode->Cost = 0.0;
	actionNode->ActionNo = -1;
	actionNode->act = terminalAction;
	actionNode->NextState = CreateStateDistribution();
	actionNode->NextState->State = stateNode;
	actionNode->NextState->Prob = 1.0;
	actionNode->NextState->Reward = 0.0;
	actionNode->NextState->reason = parent->dd;
	actionNode->NextState->creason = new set<const pEffect*>();
	Cudd_Ref(actionNode->NextState->reason);
	actionNode->PrevState = parent;

	parent->Expanded = 1;
	if(verbosity >= 2){
		cout << expandedStates << ":\t" << parent->StateNo << "\t" << parent->g << "\t" << parent->h << "\t"
				 << parent->goalSatisfaction << "\t" << parent->prReached;
		if(parent->Terminal > 0)
			cout << " *";
		cout << endl;
	}
	expandedStates++;
}

void StateNode::applyDiscountedGoalProbability(){
	//	 cout << "apply terminal reward, State " << StateNo << endl;
	//	 printBDD(dd);


	DdNode *goal_state = Cudd_bddAnd(manager, Cudd_bddIthVar(manager, 2*(num_alt_facts-2)), Cudd_Not(Cudd_bddIthVar(manager, 2*(num_alt_facts-1))));
	Cudd_Ref(goal_state);
	DdNode *goal_state_add = Cudd_BddToAdd(manager, goal_state);
	Cudd_Ref(goal_state_add);	   


	DdNode *ndd1 = Cudd_addApply(manager, Cudd_addTimes, dd, goal_state_add);
	Cudd_Ref(ndd1);

	double value = (pow(gDiscount, horizon)*transform_reward_to_probability(0)/(1-gDiscount))/Cudd_CountMinterm(manager, ndd1, num_alt_facts);
	//	cout << horizon << " " << (pow(gDiscount, horizon)) << " " << endl;
	//	 cout << "rew: " << transform_reward_to_probability(0) << " val = " << value << endl;
	//	 cout << Cudd_CountMinterm(manager, ndd1, num_alt_facts) << endl;
	DdNode *v = Cudd_addConst(manager, value);
	Cudd_Ref(v);

	DdNode *ndd2 = Cudd_addBddStrictThreshold(manager, ndd1,0.0);
	Cudd_Ref(ndd2);
	DdNode *ndd3 = Cudd_BddToAdd(manager, ndd2);
	Cudd_Ref(ndd3);
	DdNode *v1 = Cudd_addApply(manager, Cudd_addTimes, v,ndd3);
	Cudd_Ref(v1);	
	//		 printBDD(ndd1);
	//		 printBDD(v1);

	DdNode *ndd = Cudd_addApply(manager, Cudd_addPlus, v1, dd);
	Cudd_Ref(ndd);
	Cudd_RecursiveDeref(manager, dd);
	dd = ndd;
	Cudd_Ref(dd);
	Cudd_RecursiveDeref(manager, ndd);
	Cudd_RecursiveDeref(manager, v);
	Cudd_RecursiveDeref(manager, v1);
	//	printBDD(dd);
}

void StateNode::addAllActions(){
	bool wasNotExpanded = (Expanded == 0);
	for(map<const Action*, DdNode*>::iterator a = action_preconds.begin(); a != action_preconds.end(); a++){
		if((*a).first->name().compare("noop_action") == 0)
			continue;
		ActionNode *action = addAction((*a).first->name(), false, false);
		assert(action != NULL);
	}
	if(wasNotExpanded)
		cout << expandedStates << ":";
	cout << "\t" << StateNo << "\t" << g << "\t" << h << "\t" << goalSatisfaction << "\t" << prReached << endl;
}

ActionNode* StateNode::getAction(const string name, bool addIfNeeded){
	if(NextActions == NULL)
		NextActions = new ActionNodeList();		// Here (not in constructor) because it messes up StateNode::expand
	
	for(ActionNodeList::iterator act_it = NextActions->begin(); act_it != NextActions->end(); act_it++){
		ActionNode* action = *act_it;
		if(action->act->name().compare(name) == 0){
			return action;
		}
	}

	if(addIfNeeded)
		return addAction(name, false);

	return NULL;
}

ActionNode* StateNode::addAction(const string name, bool reuseIfExists, bool displayState){
	if(name.compare("noop_action") == 0){
		std::cout << "noop_action\n";
		return NULL;
	}
		
	ActionNode* action = NULL;
	if(reuseIfExists){// 该动作已经记录过
		action = getAction(name, false);// 这里根据名字查找会出现重复
		if(action != NULL){
			if (displayState)
				cout << "\t" << StateNo << "\t" << g << "\t" << h << "\t" << goalSatisfaction << "\t" << prReached << endl;
			return action;
		}
	}
	cout << "add new action\n";
	
	map<const Action*, DdNode*>::iterator a;
	for(a = action_preconds.begin(); a != action_preconds.end() && (*a).first->name().compare(name) != 0; a++);
	if(a == action_preconds.end())
		return NULL;
	// 创建action结点
	action = new ActionNode();
	// 设置动作结点的前置状态和动作
	action->PrevState = this;
	action->act = (*a).first;

	if(NextActions == NULL)
		NextActions = new ActionNodeList();
	NextActions->push_back(action);
	if (Expanded == 0)
		expandedStates++;
	displayState = true;
	if (displayState)
	{
		if(Expanded == 0)
			cout << expandedStates << ":";
		cout << "\t" << StateNo << "\t" << g << "\t" << h << "\t" << goalSatisfaction << "\t" << prReached << endl;

	}
	Expanded++;

	return action;
}
/**
 * momo007 not use comment
 */
/*
void StateNode::getAncestery(vector<StateNode*>& ancestery){
	vector<StateNode*> states;
	states.push_back(this);
	return getAncestery(states, ancestery);
}

void StateNode::getAncestery(vector<StateNode*>& states, vector<StateNode*>& ancestery){
	hash_set<StateNode*, PointerHasher> closed;

	queue<StateNode*> open;
	for(vector<StateNode*>::iterator state_it = states.begin(); state_it != states.end(); state_it++)
		open.push(*state_it);

	StateNode* front = NULL;

	while(!open.empty()){
		front = open.front();
		open.pop();
		closed.insert(front);

		if(front->Expanded > 0 && front->Terminal <= 0)
			ancestery.push_back(front);

		if(front != Start && front->PrevActions != NULL){
			for(ActionNodeList::iterator act_it = front->PrevActions->begin(); act_it != front->PrevActions->end(); act_it++){
				ActionNode* action = *act_it;
				StateNode* prevState = action->PrevState;
				if(prevState->BestAction == action && closed.count(prevState) <= 0)
					open.push(prevState);
			}
		}
	}
}
*/

/**
 * 计算更新后继状态的probability Reached和value
 */
double StateNode::forwardUpdate(){
	double biggestChange = 0;

	if(this == Start){ // 开始节点，到达概率为1
		prReached = 1.0;
		g = 0;
	}
	else{
		assert(PrevActions != NULL);

		double newGValue = 0;
		double newPrReached = 0;
		// 迭代每一个previous action
		for(ActionNodeList::iterator act_it = PrevActions->begin(); act_it != PrevActions->end(); act_it++){
			ActionNode* action = *act_it;
			StateNode* state = action->PrevState;// 根据action得到state
			// 该state的best action不是到达当前状态，忽略
			if(state->BestAction != action) //skip parents that are not set to move to this state.
				continue;
			// 否则，迭代该action的后继状态分布
			for(StateDistribution *dist = action->NextState; dist != NULL; dist = dist->Next){
				if(dist->State == this){// 到达该action的情况
					newPrReached += state->prReached * dist->Prob;// 更新到达概率
					// 更新value
					newGValue += state->prReached * dist->Prob * (state->g + action->Cost);
					break;
				}
			}
		}

		newGValue /= newPrReached;

		biggestChange = max(biggestChange, fabs(newGValue - g));
		g = newGValue;

		biggestChange = max(biggestChange, fabs(newPrReached - prReached));
		prReached = newPrReached;
	}

	f = g + gWeight * h;

	return biggestChange;

}

double StateNode::backwardUpdate(bool setBestAct){
	double biggestChange = 0;
	// 当前节点还未拓展
	if(Expanded <= 0){// 初始化
		BestAction = NULL;
		if(max_horizon > 0 || h > 0.0 || goalSatisfaction >= search_goal_threshold)
			expH = h;
		else
			expH = IPP_MAX_PLAN;
	}
	else{
		double bestExpH = expH;

		if(setBestAct && Solved <= 0){
			BestAction = NULL;

			for(ActionNodeList::iterator act_it = NextActions->begin(); act_it != NextActions->end(); act_it++){
				ActionNode* action = *act_it;
				if(server_socket > 0 && action == TerminalAction)	// For online search
					continue;

				double curExpH = action->Cost;

				for(StateDistribution *dist = action->NextState; dist != NULL; dist = dist->Next){
					StateNode* state = dist->State;
					curExpH += dist->Prob * state->expH;
				}

				if(BestAction == NULL || bestExpH > curExpH || (bestExpH == curExpH && rand() % 2 == 0)){
					BestAction = action;
					bestExpH = curExpH;
				}
			}
		}
		else if(BestAction != NULL){
			bestExpH = BestAction->Cost;
			for(StateDistribution *dist = BestAction->NextState; dist != NULL; dist = dist->Next)
				bestExpH += dist->Prob * dist->State->expH;
		}

		biggestChange = max(biggestChange, fabs(bestExpH - expH));
		expH = bestExpH;
	}

	if(BestAction == NULL){
		meanFirstPassage = 0;
		ExtendedGoalSatisfaction = goalSatisfaction;
	}
	else{
		double newExtendedGoalSatisfaction = 0.0;
		double newMeanFirstPassage = BestAction->Cost;				// (Terminal action cost = 0.0)
		double newExpDiscRew = 0;

		for(StateDistribution *dist = BestAction->NextState; dist != NULL; dist = dist->Next){
			StateNode* state = dist->State;

			newExtendedGoalSatisfaction += dist->Prob * state->ExtendedGoalSatisfaction;
			newMeanFirstPassage += dist->Prob * state->meanFirstPassage;
			newExpDiscRew += (dist->Prob * gDiscount * state->expDiscRew);
		}
		BestAction->expDiscRew = newExpDiscRew;

		biggestChange = max(biggestChange, fabs(newExtendedGoalSatisfaction - ExtendedGoalSatisfaction));
		ExtendedGoalSatisfaction = newExtendedGoalSatisfaction;

		biggestChange = max(biggestChange, fabs(newMeanFirstPassage - meanFirstPassage));
		meanFirstPassage = newMeanFirstPassage;

		biggestChange = max(biggestChange, fabs(newExpDiscRew - expDiscRew));
		expDiscRew = newExpDiscRew;
	}

	return biggestChange;
}

// momo007 2022.05.25
/*
double StateNode::valueUpdate(bool setBestAct){
	double biggestChange = backwardUpdate(setBestAct);// 后向更新寻找最大change
	biggestChange = max(biggestChange, forwardUpdate());// 前向更新寻找最大change

	return biggestChange;
}
*/

void StateNode::valueIteration(){
	vector<StateNode*> states;
	states.push_back(this);
	valueIteration(states);
}

void StateNode::valueIteration(vector<StateNode*>& states){
	assert(false && "StateNode::valueIteration is temporarily out of order");
	/*
	vector<StateNode*> ancestery;
	double diff, maxDiff;
	double prevGoalSat;
	StateNode *state = NULL;

	getAncestery(states, ancestery);

	do{
		maxDiff = 0.0;

		for(vector<StateNode *>::iterator state_it = ancestery.begin(); state_it < ancestery.end(); state_it++){
			state = *state_it;

			prevGoalSat = state->ExtendedGoalSatisfaction;
			state->valueUpdate();
			diff = fabs(state->ExtendedGoalSatisfaction - prevGoalSat);

			if(maxDiff < diff)
				maxDiff = diff;
		}
	} while(maxDiff >= gEpsilon);
	 */
}

void StateNode::terminateLeaves(){
	terminateLeaves(this);
}

void StateNode::terminateLeaves(StateNode* start){
	hash_set<StateNode *, PointerHasher> closed;

	queue<StateNode *> open;
	open.push(start);

	StateNode* front = NULL;

	while(!open.empty()){
		front = open.front();
		open.pop();
		closed.insert(front);

		if(front->Terminal <= 0){
			if(front->BestAction == NULL)
				front->BestAction = front->TerminalAction;
			else{
				for(StateDistribution *dist = front->BestAction->NextState; dist != NULL; dist = dist->Next){
					StateNode* obs = dist->State;
					if(closed.count(obs) <= 0)
						open.push(obs);
				}
			}
		}
	}
}

/********************* ActionNode functions *********************/

ActionNode::ActionNode(){
	ActionNo = actionCount++;
	Solved = 0;
	Cost = 1;
	act = NULL;
	NextState = NULL;
	PrevState = NULL;
	penalty = 0;
	Determinization = NULL;
}

ActionNode::~ActionNode(){
	while(NextState != NULL){
		StateDistribution* nextDist = NextState->Next;
		if(NextState->creason != NULL)
			delete NextState->creason;
		delete NextState;
		NextState = nextDist;
	}
}

StateNode* ActionNode::getState(const set<string>& trueObs, bool addIfNeeded){
	for(StateDistribution* dist = NextState; dist != NULL; dist = dist->Next){
		bool match = true;

		assert(dist->creason != NULL);
		for(set<const pEffect*>::iterator cr_it = dist->creason->begin(); cr_it != dist->creason->end(); cr_it++){
			const SimpleEffect *simple = dynamic_cast<const SimpleEffect*>(*cr_it);
			assert(simple != NULL);

			const string& prop = my_problem->domain().predicates().name(simple->atom().predicate());
			const AddEffect *add = dynamic_cast<const AddEffect*>(simple);
			const DeleteEffect *del = dynamic_cast<const DeleteEffect*>(simple);

			if((add && !trueObs.count(prop)) || (del && trueObs.count(prop))){
				match = false;
				break;
			}
		}

		if(match)
			return dist->State;
	}

	if(addIfNeeded)
		return addState(trueObs, false);

	return NULL;
}

StateNode* ActionNode::addState(const set<string>& trueObs, bool reuseIfExists){
	StateNode* state = NULL;
	if(reuseIfExists){
		state = getState(trueObs, false);
		if(state != NULL)
			return state;
	}

  map<const Action*, list<map<const pEffect*, DdNode*>*>* >::iterator ob = action_observations_cpt.find(act);
  if(ob != action_observations_cpt.end() && (*ob).second != NULL){
		list<map<const pEffect*, DdNode*>*> observations;
		for(list<map<const pEffect*, DdNode*>*>::iterator map_it = ob->second->begin(); map_it != ob->second->end(); map_it++){
			for(map<const pEffect*, DdNode*>::iterator pair_it = (*map_it)->begin(); pair_it != (*map_it)->end(); pair_it++){
				const pEffect* effect = pair_it->first;
				DdNode* dd = pair_it->second;

				const SimpleEffect *simple = dynamic_cast<const SimpleEffect*>(effect);
				assert(simple != NULL);

				const string& prop = my_problem->domain().predicates().name(simple->atom().predicate());
				const AddEffect *add = dynamic_cast<const AddEffect*>(simple);
				const DeleteEffect *del = dynamic_cast<const DeleteEffect*>(simple);
				assert(add != NULL || del != NULL);

				if((add && trueObs.count(prop)) || (del && !trueObs.count(prop))){
					map<const pEffect*, DdNode*>* newMap = new map<const pEffect*, DdNode*>();
					(*newMap)[effect] = dd;
					observations.push_back(newMap);
					//cout << "YES: ";
				}
				//else
				//	cout << "NO:  ";
				//if(del)
				//	cout << "(not ";
				//cout << "(" << prop << ")";
				//if(del)
				//	cout << ")";
				//cout << endl;
			}
		}

		state = newSample(&observations);

		for(list<map<const pEffect*, DdNode*>*>::iterator ob_it = observations.begin(); ob_it != observations.end(); ob_it++)
			delete *ob_it;
		observations.clear();
	}

	return state;
}

StateNode* ActionNode::newSample(){
	return newSample(NULL);
}
/**
 * momo007 2022.07.06
 * This method now is without using in the A star algorithm
 */
StateNode* ActionNode::newSample(list<map<const pEffect*, DdNode*>*>* observations){
	/**
	 * momo007 not used dbn node
	 */
	// if(observations == NULL){
	// 	std::cout << "action dbd new Sample[warring]\n";
	// 	assert(0);
	// 	action_dbn(*act);
	// 	map<const Action *, list<map<const pEffect *, DdNode *> *> *>::iterator ob = action_observations_cpt.find(act);
	// 	if (ob == action_observations_cpt.end() || (observations = ob->second) == NULL)
	// 		return NULL;
	// }
	// 查找动作判断其满足情况
	map<const Action*, DdNode*>::iterator fr = action_preconds.find(act);
	assert(fr != action_preconds.end());
	std::cout << "before add_bdd()\n" << std::flush;
	/**
	 * momo007 2022.06.09 add using error, not the state only contain bdd
	 */
	// if (!add_bdd_entailed(manager, PrevState->dd, fr->second))
	if(bdd_isnot_one(manager,bdd_imply(manager,PrevState->dd, fr->second)))
	{
		std::cout << "not entailed()\n" << std::flush;
		return NULL;
	}
	pair<const Action *const, DdNode *> act_pair(act, fr->second);// 动作及其前提条件pair
	DdNode *causativeSuccessor = progress(PrevState->dd, &act_pair);
	Cudd_Ref(causativeSuccessor);

	int num_successors = 0;

	list<DdNode*> successors;										// state set distributions
	list<set<const pEffect*>*> creasons;				// observations that gave state sets
	list<double> probabilities;									// probabilities of making observations

	if(causativeSuccessor == Cudd_ReadZero(manager))
	{
		std::cout << "split the zero bdd\n";
		num_successors = split(observations, PrevState->dd, &successors, &creasons, &probabilities, 1);
	}
	successors.push_back(causativeSuccessor);
	// else{
	// 	std::cout << "split the bdd\n";
	// 	// 分别传入总的后继状态，successor(obs分割后的)，creason（reason），每个分支的概率,sample数量
	// 	num_successors = split(observations, causativeSuccessor, &successors, &creasons, &probabilities, 1);
	// }

//	Cudd_RecursiveDeref(manager, causativeSuccessor);		// Keep or Comment??

	// assert(num_successors == 1);// 这里为什么一定要保证为1?
	// assert(Cudd_ReadZero(manager) != *(successors.begin()));

	list<DdNode*>::iterator o = successors.begin();// 考虑后继状态
	// list<set<const pEffect*>*>::iterator cr = creasons.begin();// 考虑observation
	// list<double>::iterator v = probabilities.begin();// 考虑observation的prob

	StateNode* state = NULL;
//	cout << "A" << ActionNo;

	if(Cudd_ReadZero(manager) != *o){
		bool isNew = (StateNode::generated.count(*o) == 0);
		if(isNew) //Don't add duplicate dd'
		{
			state = new StateNode();
			state->StateNo = state_count++;
			state->dd = *o;

			state->horizon = PrevState->horizon + 1;
			state->kld = 1.0;
			state->goalSatisfaction = computeGoalSatisfaction(*o);
			state->ExtendedGoalSatisfaction = state->goalSatisfaction;
			state->prReached = PrevState->prReached * 1.0;         // HACK
			state->g = PrevState->g + Cost;
			state->BestAction = NULL;
			state->horizon = PrevState->horizon + 1;

			state->PrevActions = new ActionNodeList();
			state->NextActions = new ActionNodeList();		// Here (not in constructor) because it messes up StateNode::expand

			list<StateNode*> m_states;
			m_states.push_back(state);
			getHeuristic(&m_states, PrevState, PrevState->horizon + 1);
			//state->h = 100;
			m_states.clear();

			StateNode::generated[*o] = state;
		}
		else
			state = StateNode::generated[*o];

		bool linked = false;
		// 判断是否添加过
		for(StateDistribution* dist = NextState; dist != NULL; dist = dist->Next){
			if(dist->State == state){
				linked = true;
				break;
			}
		}
		// 第一次添加，链表头插法
		if(!linked){
			StateDistribution* stateDist = CreateStateDistribution();
			stateDist->State = state;
			// stateDist->creason = *cr;
			stateDist->Prob = 1.0;		// HACK
			stateDist->Next = NextState;
			NextState = stateDist;

			/* Also add pointers from next states back to this action. */
			if(state->PrevActions == NULL)
				state->PrevActions = new ActionNodeList;
			state->PrevActions->push_front(this);

			// HACK
			double total_prob = 0.0;
			for(StateDistribution* dist = NextState; dist != NULL; dist = dist->Next)
				total_prob += dist->Prob;
			for(StateDistribution* dist = NextState; dist != NULL; dist = dist->Next)
				dist->Prob /= total_prob;
		}
/*
		if(isNew)
			cout << "->";
		else if(linked)
			cout << "=>";
		else
			cout << "~>";
		cout << "S" << state->StateNo << "\t" << state->g << "\t" << state->h << "\t" << state->goalSatisfaction << "\t" << state->prReached;
*/
  }
//	else
//		cout << "-|";
//	cout << endl;

	return state;
}

/******************** StateDistribution functions ***************/

struct StateDistribution *CreateStateDistribution( void )
{
	struct StateDistribution *x;

	x = (struct StateDistribution*)malloc(sizeof(struct StateDistribution));
	x->reason = NULL;
	x->State = NULL;
	x->Prob = 0.0;
	x->Reward = 0.0;//Cudd_ReadZero(manager);
	x->Next = NULL;
	/* set default values */
	return( x );
}

/******************* StateList functions ********************/

void DisplayStateList(StateList *list)
{
	StateList::iterator node;

	for(node = list->begin(); node != list->end(); node++) {
		printf("\nindex %d State %s f %f",
				(*node)->StateNo,
				(*node)->Description,
				(*node)->f);
		if((*node)->Terminal != 1)
			printf(" best action %d", (*node)->BestAction->ActionNo);
		printf(" meanFP %f", (*node)->meanFirstPassage);
	}
	printf("\nPress return"); getchar();
}

/******************* StateComparator functions ********************/

StateComparator::StateComparator(){
	mode = HEUR;
}

void StateComparator::init(CompareMode mode){
	this->mode = mode;
}

/**
 * 状态比较函数
 * 根据comparator的类型进行比较
 */
bool StateComparator::operator() (StateNode *lhs, StateNode *rhs) const{
	if(mode == HEUR && lhs->h != rhs->h)// 启发式值比较h
		return (lhs->h < rhs->h);

	if(mode == F_VAL && lhs->f != rhs->f)// F-value比较f
		return (lhs->f < rhs->f);

	// if(mode == PR_REACH && lhs->prReached != rhs->prReached)// PR_REACH比较达到该节点的概率
	// 	return (lhs->prReached > rhs->prReached);

	// if(mode == EXP_EGS)// EXP_EGS比较拓展到达目标的期望值
	// {
	// 	double lhs_exp_ps = lhs->prReached * (pow(0.99, lhs->h));
	// 	double rhs_exp_ps = rhs->prReached * (pow(0.99, rhs->h));
	// 	if(lhs_exp_ps != rhs_exp_ps)
	// 		return (lhs_exp_ps > rhs_exp_ps);
	// 	if(lhs->f != rhs->f)
	// 		return (lhs->f < rhs->f);
	// }
	if(lhs->goalSatisfaction != rhs->goalSatisfaction)
		return (lhs->goalSatisfaction > rhs->goalSatisfaction);

	return lhs->StateNo < rhs->StateNo;
	// if (lhs->randPriority != rhs->randPriority)
	// 	return (lhs->randPriority < rhs->randPriority);

	// return lhs < rhs;	// In case this comparator is used for a set
}

bool StateComparator::operator() (PolicyState *lhs_ps, PolicyState *rhs_ps) const{
	StateNode* lhs = lhs_ps->getState();
	StateNode* rhs = rhs_ps->getState();
	double lhs_f = lhs_ps->getG() + gWeight * lhs->h;
	double rhs_f = rhs_ps->getG() + gWeight * rhs->h;

	bool lhs_goal = (server_socket < 0 && (lhs->isGoal() || lhs->Solved > 0));
	bool rhs_goal = (server_socket < 0 && (rhs->isGoal() || rhs->Solved > 0));

	if(lhs_goal != rhs_goal)
		return (lhs_goal && !rhs_goal);
	else if(lhs_goal){// implies "&& rhs_goal"
		double lhs_PlanSuccess = lhs_ps->getPrReached() * lhs->goalSatisfaction;
		double rhs_PlanSuccess = rhs_ps->getPrReached() * rhs->goalSatisfaction;

		if(lhs_PlanSuccess != rhs_PlanSuccess)
			return (lhs_PlanSuccess > rhs_PlanSuccess);
	}

	if(mode == PR_REACH && lhs_ps->getPrReached() != rhs_ps->getPrReached())
		return (lhs_ps->getPrReached() > rhs_ps->getPrReached());

	if(mode == EXP_EGS)
	{
		double lhs_exp_ps = lhs_ps->getPrReached() * (pow(0.5, lhs->h));
		double rhs_exp_ps = rhs_ps->getPrReached() * (pow(0.5, rhs->h));
		if(lhs_exp_ps != rhs_exp_ps)
			return (lhs_exp_ps > rhs_exp_ps);
		if(lhs_f != rhs_f)
			return (lhs_f < rhs_f);
	}

	if(mode == HEUR && lhs->h != rhs->h)
		return (lhs->h < rhs->h);

	if(mode == F_VAL && lhs_f != rhs_f)
		return (lhs_f < rhs_f);

	if(lhs->goalSatisfaction != rhs->goalSatisfaction)
		return (lhs->goalSatisfaction > rhs->goalSatisfaction);

	if(lhs->randPriority != rhs->randPriority)
		return lhs->randPriority < rhs->randPriority;

	return lhs < rhs;	// In case this comparator is used for a set
}

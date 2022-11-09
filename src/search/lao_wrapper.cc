#include <iostream>
#include <sstream>
#include <fstream>

#include "globals.h"
#include "search.h"
#include <math.h>
#include <stdio.h>

#include "mtbdd.h"
#include "cudd/dd.h"
#include "sample_size.h"

#include <list>

#ifdef PPDDL_PARSER
#else
#include "actions/actions.h"
#endif

#include "dbn.h"
#include "lug/graph_wrapper.h"
#include "lug/labelledElement.h"
#include "heuristics/lug.h"
#include "graph.h"
#include "lao_wrapper.h"
#include "backup.h"
#include "simulator/monitor.h"  // for BDDtoITEString
#include "float.h"
#include "solve.h"
#include <typeinfo>

using namespace __gnu_cxx;
using namespace std;

void free_bit_vector(BitVector*);

struct StateDistribution *CreateStateDistribution();
void printAction(struct ActionNode* a);
void CreateHeuristic();
void DisplayStateList(StateList *list);
StateList *CreateStateList();
BitVector *new_bit_vector(int length);
int get_bit(BitVector*, int, int);
DdNode** extractDNFfromBDD(DdNode* node);
DdNode** extractTermsFromMinterm(DdNode* node);
void set_bit(BitVector* b, int index);

int dummy = 0;
extern int num_lugs;
/**
 * 更新叶子节点的启发值
 */
void update_leaf_h_values(){
	list<StateNode*> nodes;
	for(StateHash::iterator i = LeafStates->begin(); i != LeafStates->end(); i++){
		nodes.push_back((*i).second);
		getHeuristic(&nodes, Start, 0);
		nodes.clear();
	}
}

double VI(){
	double r = Start->f;

	for(StateHash::iterator i = StateIndex->begin(); i != StateIndex->end(); i++)
		if((*i).second->Terminal == 0)
			Backup((*i).second, 0);

	return fabs(r - Start->f);
}

void backup_graph(){
	double residual;
	int i = 0;
	do{
		residual = VI();
	}
	while(residual >= gEpsilon);
}
/**
 * 判断初始和目标状态两者之间是否形成loop，如果目标存在出边到达初始状态则形成
 */
int not_loop(StateNode* dest, BitVector* visited, StateNode* srcNo){
	// want to know if connecting source to dest will create a loop
	// it will create a loop if following any of dest's outgoing edges
	// eventually leads to source.

	ActionNodeList::iterator acts;
	StateDistribution* succ;

	if(srcNo->StateNo != dest->StateNo && get_bit(visited, gNumStates, srcNo->StateNo))
		return TRUE;        // don't recurse on loops you've seen before that are not involving destNo
	else if(srcNo->StateNo == dest->StateNo)
		return FALSE;       // found loop to destNo

	set_bit(visited, srcNo->StateNo);
	for(acts = srcNo->NextActions->begin(); acts != srcNo->NextActions->end(); acts++)
		for(succ = (*acts)->NextState; succ != NULL; succ = succ->Next)
			if(!not_loop(dest, visited, succ->State))
				return FALSE;   // found loop via an ancestor

	return TRUE;          // found no loop via an ancestor
}

int in_same_possible_solution(StateNode* node1, StateNode* node2, StateNode* curr, BitVector* visited);


/**
 * momo007 2022.05.12
 * ignore, not used in project
 */
/*int goal_reachable(){
	return 0;
}*/

int in_same_possible_solution(StateNode* node1, StateNode* node2, StateNode* curr, BitVector* visited){
	// is node1 possibly in the same solution as node2
	// i.e. does an execution that contains node2, contain node1

	// got == 0 - neither 1 or 2
	//     == 1 - just 1
	//     == 2 - just 2
	//     == 3 - one of them, but not both
	//     == 4 - both

	StateNode *cS = curr;
	int gotOutcome, gotState, gotAct, returnVal;

	ActionNodeList::iterator acts;
	StateDistribution *states;

	if(get_bit(visited, gNumStates, curr->StateNo)){
		if(curr == node1)
			return 1;
		else if(curr == node2)
			return 2;
		else
			return 0;
	}
	else
		set_bit(visited, curr->StateNo);

	if(curr == node1)
		gotState = 1;
	else if(curr == node2)
		gotState = 2;
	else
		gotState = 0;

	returnVal = gotState;
	for(acts = cS->NextActions->begin(); acts != cS->NextActions->end(); acts++){
		gotAct = gotState;
		for(states = (*acts)->NextState; states; states = states->Next){
			gotOutcome = in_same_possible_solution(node1, node2, states->State, visited);

			if(gotOutcome == 0)
				continue;
			else if(gotOutcome == 1){
				if(gotAct == 2 || gotAct == 3)
					return 4;
				else
					gotAct = 1;
			}
			else if(gotOutcome == 2){
				if(gotAct == 1 || gotAct == 3)
					return 4;
				else
					gotAct = 2;
			}
			else if(gotOutcome == 3){
				if(gotAct == 1 || gotAct == 2)
					return 4;
				else
					gotAct = 3;
			}
			else if(gotOutcome == 4)
				return 4;
		}

		if(gotAct == 0)
			continue;
		else if(gotAct == 1){
			if(returnVal == 2)
				returnVal = 3;
			else if(returnVal == 3)
				return 4;
			else
				returnVal = 1;
		}
		else if(gotAct == 2){
			if(returnVal == 1)
				returnVal = 3;
			else if(returnVal == 3)
				return 4;
			else
				returnVal = 2;
		}
		else if(gotAct == 3){
			if(returnVal == 1 || returnVal == 2)
				return 4;
			else
				returnVal = 3;
		}
	}

	return returnVal;
}

/**
 * 判断是否只有一个后继状态
 */
bool single_successor(StateNode *src, StateNode *dest){
	// checks paths of length 1 only, where an action with
	// a single outcome leads back to its parent.

	if(src == NULL || dest == NULL || src->NextActions == NULL)
		return false;

	for(ActionNodeList::iterator a = src->NextActions->begin(); a != src->NextActions->end(); a++){
		ActionNode *action = *a;
		if((src == dest ||// 当前到达dest
			(action != NULL &&
			action->NextState != NULL &&
			action->NextState->State == dest))  &&// 下一个状态到达dest
			action != NULL &&
			action->NextState != NULL &&
			action->NextState->Next == NULL){// 只有一个后继状态
				return true;
		}
	}

	return false;
}

/**
 * 
 */
int check_loop_conditions(StateNode* src, StateNode* dest, DdNode* state, int currHorizon){
	int reach_goal; /* 0 == reach, 1 == no reach, 2 == don't know yet */

	BitVector* loop_visited = NULL;
	loop_visited = new_bit_vector(((int)(state_count)/(gcword_size))+1);

	if(
		// finite horizon, full obs, or not loop okay to connect same nodes
		((max_horizon == -1 &&
		my_problem->domain().requirements.probabilistic &&
		((OBSERVABILITY==OBS_PART && !single_successor(src, dest)) 
		||
		(OBSERVABILITY==OBS_FULL && !single_successor(src, dest))
		||
		(OBSERVABILITY==OBS_NONE && not_loop(dest, loop_visited, src))))
		||
		(max_horizon == -1 &&
		my_problem->domain().requirements.non_deterministic &&
		(OBSERVABILITY==OBS_FULL || not_loop(dest, loop_visited, src))))
		){
			// ok make connnection
			free_bit_vector(loop_visited);
			return 0;
	}
	else if(max_horizon != -1 &&  currHorizon != src->horizon){
		// don't make loop, add as new node
		free_bit_vector(loop_visited);
		return 2;
	}

	// is loop need to make sure its okay
	free_bit_vector(loop_visited);
	return 1;
}


DdNode* normalize(DdNode* dd){
	double total = get_sum(dd);
	DdNode* fr = Cudd_addConst(manager, total);
	Cudd_Ref(fr);
	DdNode *fr1 = Cudd_addApply(manager, Cudd_addDivide, dd, fr);
	Cudd_Ref(fr1);
	Cudd_RecursiveDeref(manager, fr);
	return fr1;
}
/**
 * momo007 2022.05.12 Compute Successors
 * the core code of update the belief state
 * 
 */
void computeSuccessors(pair<const Action* const, DdNode*>* action,
	struct StateNode* parent,
	list<DdNode*>* successors,   // state set distributions
	list<DdNode*>* reasons,      // observations that gave state sets
	list<set<const pEffect*>*>* creasons,      // observations that gave state sets
	list<double>* probabilities, // probabilities of making observations
	list<double>* rewards,       // rewards associated with state sets
	list<double>* klds,          // kl distances
	int reps                     // how many times to repeat action, for macros
	){
		DdNode *tmpdd, *tmpdd1, *tmpdd2, *fr;
		int num_successors;
		// get the precond fr
		if(action && action->first && &(action->first->precondition()))
			fr = action_preconds[action->first];
		else
			fr = Cudd_ReadLogicZero(manager);

		if(!&(*action))
			return;

		//	(*action).first->print(cout, (*my_problem).terms()); cout << endl;

		//  printBDD(fr);
		//  printBDD(parent->dd);
		
		// check precondition
		if((my_problem->domain().requirements.probabilistic &&
			!add_bdd_entailed(manager, parent->dd, fr)) ||// parent |= fr
			(my_problem->domain().requirements.non_deterministic &&
			!bdd_entailed(manager, parent->dd, fr))){ // parent |= fr
				return;
		}

		//	std::cout << "<" << std::endl;
		//				Cudd_CheckKeys(manager);
		//				std::cout << "|" << std::endl;

		// apply image operator to parent (an add), get an add
		//DdNode* tmp_successor = parent->dd;
		//Cudd_Ref(tmp_successor);

		DdNode* causativeSuccessor = NULL;

		// use the action to progres the parent(current state bdd)
		//	for(int i = 0; i < reps; i++){
		causativeSuccessor = progress(&(*action),
			//&(*tmp_successor)
			parent->dd
			);

		Cudd_Ref(causativeSuccessor);

		//		Cudd_RecursiveDeref(manager, tmp_successor);
		//		tmp_successor = causativeSuccessor;
		//		Cudd_Ref(tmp_successor);
		//
		//		Cudd_RecursiveDeref(manager, causativeSuccessor);
		//		if(tmp_successor == Cudd_ReadZero(manager))
		//			break;
		//	}


		//	causativeSuccessor = tmp_successor;
		//	Cudd_Ref(causativeSuccessor);
		//Cudd_RecursiveDeref(manager, tmp_successor);

		if(verbosity >= 3){
			(*action).first->print(cout, (*my_problem).terms()); cout << endl;
			cout << "act:"<<endl;
			printBDD((&(*action))->second);
			cout << "Causative Successor: " << endl;
			printBDD(causativeSuccessor);
			cout << "parent:"<<endl;
			printBDD(parent->dd);

		}

		//	Cudd_CheckKeys(manager);
		//	std::cout << ">" << std::endl;

		// if action doesn't change belief state or give knowledge,
		// then return empty successors
		if((((my_problem->domain().requirements.probabilistic &&
			causativeSuccessor == Cudd_ReadZero(manager)) ||
			(my_problem->domain().requirements.non_deterministic &&
			causativeSuccessor == Cudd_ReadLogicZero(manager)))
			&&
			max_horizon == -1
			&&
			!&(((Action*)(*action).first)->observation())) ||
			(OBSERVABILITY==OBS_FULL &&
			parent->dd == causativeSuccessor &&
			!&(((Action*)(*action).first)->observation()))){
				if(verbosity >= 3)
					cout << "Non useful successor" <<endl;
				Cudd_RecursiveDeref(manager, causativeSuccessor);
				return;
		}

		// handle observations
		if(OBSERVABILITY == OBS_PART || OBSERVABILITY == OBS_NONE){
			if(OBS_TYPE==OBS_CPT){
				//      map<const Action*, map<DdNode*, set<const Atom*>* >* >* >
				std::map<const Action*, std::list<std::map<const pEffect*, DdNode*>*>* >::iterator ob = action_observations_cpt.find(action->first);
				// if the action has observations, several successors
				if(USESENSORS &&
					ob != action_observations_cpt.end() && &(*ob)){
						if(((my_problem->domain().requirements.probabilistic &&
							causativeSuccessor == Cudd_ReadZero(manager)) ||
							(my_problem->domain().requirements.non_deterministic &&
							causativeSuccessor == Cudd_ReadLogicZero(manager)))){
								num_successors = split((*ob).second,
									parent->dd,
									successors,
									creasons, probabilities);
						}
						else{
							num_successors = split((*ob).second, causativeSuccessor,
								successors, creasons, probabilities);
						}
				}
			}
			else{
				map<const Action*, list<DdNode*>* >::iterator ob = action_observations.find(action->first);
				// if the action has observations, several successors
				if(USESENSORS &&
					ob != action_observations.end() && &(*ob)){
						if(((my_problem->domain().requirements.probabilistic &&
							causativeSuccessor == Cudd_ReadZero(manager)) ||
							(my_problem->domain().requirements.non_deterministic &&
							causativeSuccessor == Cudd_ReadLogicZero(manager)))){
								num_successors = split((*ob).second, parent->dd,
									successors, reasons);
						}
						else{
							num_successors = split((*ob).second, causativeSuccessor,
								successors, reasons);
						}
				}
			}
		}
		else {  // no observations
			num_successors = 1;
			Cudd_Ref(causativeSuccessor);
			Cudd_Ref(causativeSuccessor);
			successors->push_back(causativeSuccessor);
			reasons->push_back(causativeSuccessor);
		}


		// remove empty successors
		list<DdNode*> keepers;
		for(list<DdNode*>::iterator successor = successors->begin();
			successor != successors->end(); successor++){
				if((my_problem->domain().requirements.non_deterministic &&
					Cudd_ReadLogicZero(manager) != *successor) ||
					(my_problem->domain().requirements.probabilistic &&
					Cudd_ReadZero(manager) != *successor )){
						keepers.push_back(*successor);
						klds->push_back(1.0);
				}
		}
		if(OBS_TYPE==OBS_CPT){
			list<set<const pEffect*>*> keeper_reasons;
			for(list<set<const pEffect*>*>::iterator reason = creasons->begin();
				reason != creasons->end(); reason++){
					if(*reason){
						keeper_reasons.push_back(*reason);
					}
			}

			num_successors = keepers.size();
			successors->clear();
			successors->merge(keepers);
			successors->unique();

			//reasons->clear();
			//reasons->merge(keeper_reasons);
			//reasons->unique();


			// if sensing gave no knowledge, and causation gave no new states return with empty successors
			if(num_successors == 1 && successors->front() == parent->dd && max_horizon == -1){
				if(verbosity >= 3)
					cout << "Non useful successor" <<endl;
				Cudd_RecursiveDeref(manager, causativeSuccessor);
				Cudd_RecursiveDeref(manager, successors->front());
				//      Cudd_RecursiveDeref(manager, reasons->front());
				successors->clear();
				reasons->clear();
				return;
			}
		}
		else{
			list<DdNode*> keeper_reasons;
			for(list<DdNode*>::iterator reason = reasons->begin();
				reason != reasons->end(); reason++){
					if((my_problem->domain().requirements.non_deterministic &&
						Cudd_ReadLogicZero(manager) != *reason) ||
						(my_problem->domain().requirements.probabilistic &&
						Cudd_ReadZero(manager) != *reason )){
							keeper_reasons.push_back(*reason);
					}
			}

			num_successors = keepers.size();
			successors->clear();
			successors->merge(keepers);
			successors->unique();

			reasons->clear();
			reasons->merge(keeper_reasons);
			reasons->unique();


			// if sensing gave no knowledge, and causation gave no new states return with empty successors
			if(num_successors == 1 && successors->front() == parent->dd && max_horizon == -1){
				if(verbosity >= 3)
					cout << "Non useful successor" <<endl;
				Cudd_RecursiveDeref(manager, causativeSuccessor);
				Cudd_RecursiveDeref(manager, successors->front());
				Cudd_RecursiveDeref(manager, reasons->front());
				successors->clear();
				reasons->clear();
				return;
			}


			else if(OBSERVABILITY == OBS_FULL){
				get_states_from_add(causativeSuccessor, successors, probabilities);
				// store reasons as bdds, and successors as adds
				for(list<DdNode*>::iterator i = successors->begin();
					i != successors->end(); i++){
						reasons->push_back(*i);
						*i = Cudd_BddToAdd(manager, *i);
						Cudd_Ref(*i);

				}
			}
		}
		// process each successor
		// - find probability of getting successor
		// - find reward
		// - normalize state distribution

		list<DdNode*> tmp;
		list<double> tmpprs;
		list<double>::iterator tmpprs1;
		double totalprs = 0.0, tmppr;

		tmpprs1 = probabilities->begin();
		for(list<DdNode*>::iterator successor = successors->begin();
			successor != successors->end(); successor++, tmpprs1++){

				// rewards->push_back(computeReward(action, *successor, parent->dd));
				if(my_problem->domain().requirements.non_deterministic){
					tmppr = 1.0/(double)successors->size();
					tmpdd=*successor;
					Cudd_Ref(tmpdd);
				}
				else if(my_problem->domain().requirements.probabilistic && OBSERVABILITY != OBS_FULL ){
					tmppr = *tmpprs1;//get_sum(*successor);
					tmpdd = *successor;//normalize(*successor);
					Cudd_Ref(tmpdd);
					//printBDD(*successor);
				}
				else if(my_problem->domain().requirements.probabilistic && OBSERVABILITY == OBS_FULL){
					DdNode *pr = Cudd_addApply(manager, Cudd_addTimes, *successor, causativeSuccessor);
					Cudd_Ref(pr);
					double mpr = get_sum(pr);
					tmppr = mpr;
					*tmpprs1=mpr;     // code above in this block fixes bug with getting the right pr on outcome
					tmpdd= *successor;
				}


				tmpprs.push_back(tmppr);
				totalprs += tmppr;
				tmp.push_back(tmpdd);
		}


		if(totalprs == 0){
			probabilities->clear();
			successors->clear();
			reasons->clear();
			creasons->clear();
			klds->clear();
			num_successors =0;

		}

		probabilities->clear();
		for(list<double>::iterator pr = tmpprs.begin();
			pr != tmpprs.end(); pr++){
				//std::cout << *pr << " " << totalprs << " " << (*pr / totalprs) << std::endl;
				probabilities->push_back(*pr / totalprs);
		}
		if(OBSERVABILITY!=OBS_FULL){
			for(list<DdNode*>::iterator successor = successors->begin();
				successor != successors->end(); successor++){
					Cudd_RecursiveDeref(manager, *successor);
			}
		}
		successors->clear();
		successors->insert(successors->end(), (tmp.begin()), tmp.end());

		//				Cudd_CheckKeys(manager);
		//				std::cout << "]" << std::endl;

}

double computeGoalSatisfaction(DdNode* dd){
	double retVal;
	//std::cout << "Goal Sat" <<std::endl;
	//	std::cout << "[" << std::endl;
	//				Cudd_CheckKeys(manager);
	//				std::cout << "|" << std::endl;
	// not used
	if(my_problem->domain().requirements.probabilistic){
		std::cout << "assert probability in goal sat[assert]\n";
		assert(0);
		//	printBDD(dd);
		DdNode *tmp1 = Cudd_BddToAdd(manager, Goal->dd);
		Cudd_Ref(tmp1);
		DdNode *tmp = Cudd_addApply(manager, Cudd_addTimes,
			dd, tmp1);
		Cudd_Ref(tmp);
		//printBDD(tmp);


		if(RBPF_PROGRESSION){
			//sum out samples, combine weights
			//		printBDD(RBPF_SAMPLE_CUBE);
			//			printBDD(tmp);

			DdNode *tmp3 = Cudd_addExistAbstract(manager, tmp, RBPF_SAMPLE_CUBE);
			Cudd_Ref(tmp3);
			Cudd_RecursiveDeref(manager, tmp);

			//printBDD(tmp3);

			//			DdNode* tmp2 = normalize(tmp3);
			//				Cudd_Ref(tmp2);
			//				Cudd_RecursiveDeref(manager, tmp3);


			//normalize distribution
			tmp = tmp3; //normalize(tmp2);
			Cudd_Ref(tmp);
			//			Cudd_RecursiveDeref(manager, tmp2);
			Cudd_RecursiveDeref(manager, tmp3);

			//			std::cout << "Goal DD" << std::endl;
			//			printBDD(tmp);

		}


		retVal = get_sum(tmp)/RBPF_SAMPLES;

		//std::cout << "retVal = " << retVal << std::endl;
		Cudd_RecursiveDeref(manager, tmp);
		Cudd_RecursiveDeref(manager, tmp1);
	}
	// 判断当前状态是否leq目标命题
	else if(my_problem->domain().requirements.non_deterministic){
		if(bdd_entailed(manager, dd, Goal->dd))
			retVal = 1.0;
		else
			retVal = 0.0;
	}
	//	Cudd_CheckKeys(manager);
	//				std::cout << "]" << std::endl;

	return retVal;
}


int split(list<DdNode*>* a,
					DdNode* parent, list<DdNode*>* result,
					list<DdNode*>* reasons){
						int obs = 0;
						DdNode* tmp, *tmp1, *pr, *tmp2;


						for(list<DdNode*>::iterator i = a->begin(); i != a->end(); i++){

							if(my_problem->domain().requirements.probabilistic){
								tmp = Cudd_addApply(manager, Cudd_addTimes, *i, parent);
								Cudd_Ref(tmp);
							}
							else{
								tmp = Cudd_bddAnd(manager, *i, parent);
								Cudd_Ref(tmp);
							}
							result->push_back(tmp);
							tmp1 = *i;
							Cudd_Ref(tmp1);
							reasons->push_back(tmp1);
							obs++;
						}

						return obs;
}
int split(list<map<const pEffect*, DdNode*>* >* a,
					DdNode* parent, list<DdNode*>* result,
					list<set<const pEffect*>*>* reasons,
					list<double>* probabilities,
					int numSamples){
						int obs = 0;
						DdNode* tmp, *tmp1, *pr, *tmp2;
						double pr1;
						//cout << "split" << endl;
						//printBDD(parent);

						//	std::cout << "[" << std::endl;
						//				Cudd_CheckKeys(manager);
						//				std::cout << "|" << std::endl;

						map<set<const pEffect*>*, DdNode*> outcomes;

						if(numSamples < 0){//compute all outcomes

							map<set<const pEffect*>*, DdNode*> new_outcomes;
							set<const pEffect*>* outcome = new set<const pEffect*>();
							outcomes[outcome] = parent;
							Cudd_Ref(parent);

							for(list<map<const pEffect*, DdNode*>*>::iterator i = a->begin(); i != a->end(); i++){
								for(map<set<const pEffect*>*, DdNode*>::iterator k = outcomes.begin(); k != outcomes.end(); k++){
									set<const pEffect*> *observables = (*k).first;
									DdNode *state = (*k).second;
									for(map<const pEffect*, DdNode*>::iterator j = (*i)->begin(); j != (*i)->end(); j++){
										set<const pEffect*> *new_observables = new set<const pEffect*>();


										DdNode *new_state = Cudd_addApply(manager, Cudd_addTimes, (*j).second, state);
										Cudd_Ref(new_state);

										//printBDD((*j).second);
										//printBDD(new_state);

										if(new_state == Cudd_ReadZero(manager))
											continue;

										new_observables->insert(observables->begin(), observables->end());
										new_observables->insert((*j).first);

										new_outcomes[new_observables] = new_state;
										break;
									}
									delete observables;
									Cudd_RecursiveDeref(manager, state);
								}
								outcomes.clear();
								outcomes.insert(new_outcomes.begin(), new_outcomes.end());
								new_outcomes.clear();
							}
						}
						else{//sample k outcomes
							for(int s = 0; s < numSamples; s++){
								DdNode* state = parent;
								Cudd_Ref(state);
								set<const pEffect*>* outcome = new set<const pEffect*>();

								//sample each observable
								//cout << "SAMPLING OBSERVATIONS:\n";
								for(list<map<const pEffect*, DdNode*>*>::iterator i = a->begin(); i != a->end(); i++){
									//get observable pr
									map<const pEffect*, double> prs;
									map<const pEffect*, DdNode*> states;
									double norm = 0;
									for(map<const pEffect*, DdNode*>::iterator j = (*i)->begin(); j != (*i)->end(); j++){
										DdNode *new_state = Cudd_addApply(manager, Cudd_addTimes, (*j).second, state);		
										Cudd_Ref(new_state);	      
										states[(*j).first] = new_state;
										double pr = get_sum(new_state);
										//if(pr <= 0){
										//	printBDD((*j).second);
										//	printBDD(state);
										//	printBDD(new_state);
										//}
										prs[(*j).first] = pr;
										norm += pr;
									}		


									double pr = ((double)randomGen->myrand()/((double)(RAND_MAX)+(double)(1)));
									double v = 0;
									for(map<const pEffect*, DdNode*>::iterator j = (*i)->begin(); j != (*i)->end(); j++){
										//cout << prs[(*j).first] << "/" << norm << " ... ";
										if(norm != 0)
											v += (prs[(*j).first]/norm);


										//const SimpleEffect *simple = dynamic_cast<const SimpleEffect*>((*j).first);
										//assert(simple != NULL);

										//const string& prop = my_problem->domain().predicates().name(simple->atom().predicate());
										//const AddEffect *add = dynamic_cast<const AddEffect*>(simple);
										//const DeleteEffect *del = dynamic_cast<const DeleteEffect*>(simple);
										//if(del)
										//	cout << "(not ";
										//cout << "(" << prop << ")";
										//if(del)
										//	cout << ")";
										//cout << " " << pr << "/" << v << " ";

										if(norm == 0 || pr < v){
											outcome->insert((*j).first);
											Cudd_RecursiveDeref(manager, state);
											state = states[(*j).first];

											//cout << "+\n";

											Cudd_Ref(state);
											//printBDD(state);
											break;
										}
										//cout << "-\n";
									}
									for(map<const pEffect*, DdNode*>::iterator j = (*i)->begin(); j != (*i)->end(); j++){
										Cudd_RecursiveDeref(manager, states[(*j).first]);
									}
								}
								outcomes[outcome]=state;
								//printBDD(state);
							}
						}

						for(map<set<const pEffect*>*, DdNode*>::iterator i = outcomes.begin(); i != outcomes.end(); i++){
							//cout << "split " << obs<<endl;
							if(my_problem->domain().requirements.probabilistic){
								if(RBPF_PROGRESSION){
									//weight and resample
									//std::cout << "Weighting " << std::endl;
									//printBDD(parent);
									//printBDD((*i).second);

									//weight samples
									tmp = (*i).second; //Cudd_addApply(manager, Cudd_addTimes, (*i).second, parent);
									Cudd_Ref(tmp);
									//printBDD(tmp);

									pr1 = get_sum(tmp);
									//cout << "pr = " << pr1 << endl;

									if(pr1 == 0){
										Cudd_RecursiveDeref(manager, tmp);
										continue;
									}

									if(1){ //RB particles

										//normalize each sample
										DdNode* res = Cudd_ReadZero(manager);
										Cudd_Ref(res);
										for(int k = 0; k < RBPF_SAMPLES; k++){
											DdNode* index = rbpf_sample_map[k];
											//printBDD(index);
											DdNode *aindex = Cudd_BddToAdd(manager, index);
											Cudd_Ref(aindex);
											DdNode* sample = Cudd_addApply(manager, Cudd_addTimes, aindex, tmp);
											Cudd_Ref(sample);
											//printBDD(sample);
											DdNode* norm = normalize(sample);
											Cudd_Ref(norm);
											//printBDD(norm);
											DdNode *t = Cudd_addApply(manager, Cudd_addMaximum, res, norm);
											Cudd_Ref(t);
											//printBDD(t);
											Cudd_RecursiveDeref(manager, res);
											res = t;
											Cudd_Ref(res);
											Cudd_RecursiveDeref(manager,t);
											Cudd_RecursiveDeref(manager,sample);
											Cudd_RecursiveDeref(manager,norm);
											Cudd_RecursiveDeref(manager,aindex);

										}


										//1 get weight of each particle (sum out state)
										DdNode *tmp1 = Cudd_addExistAbstract(manager, res, current_state_cube);
										Cudd_Ref(tmp1);
										//Cudd_RecursiveDeref(manager, tmp);
										//printBDD(tmp1);


										//1.5 map particle ids into current state
										DdNode *tmp2 = Cudd_addPermute(manager, tmp1, rbpf_index_map);
										Cudd_Ref(tmp2);
										//printBDD(tmp2);

										//2 sample each particle and map to new sample index
										set<int> vars;
										for(int k = 0; k < rbpf_bits; k++){
											vars.insert(2*num_alt_facts+max_num_aux_vars+rbpf_bits+k);
										}
										DdNode *tmp3 = draw_cpt_samples(tmp2, RBPF_SAMPLES, &vars);
										Cudd_Ref(tmp3);
										//printBDD(tmp3);

										//swap old indexes back into place
										DdNode *tmp4 = Cudd_addPermute(manager, tmp3, rbpf_index_map);
										Cudd_Ref(tmp4);
										//printBDD(tmp4);

										//apply states to indexes
										DdNode* tmp5 = Cudd_addApply(manager, Cudd_addTimes, res, tmp4);
										Cudd_Ref(tmp5);
										//printBDD(tmp5);

										//remove old indexes
										DdNode *tmp6 = Cudd_addExistAbstract(manager, tmp5, RBPF_SAMPLE_CUBE);
										Cudd_Ref(tmp6);
										//printBDD(tmp6);

										//move new sample indexes into place
										DdNode *tmp7 = Cudd_addPermute(manager, tmp6, rbpf_index_map);
										Cudd_Ref(tmp7);
										//printBDD(tmp7);


										Cudd_RecursiveDeref(manager, tmp1);
										Cudd_RecursiveDeref(manager, tmp2);
										Cudd_RecursiveDeref(manager, tmp3);
										Cudd_RecursiveDeref(manager, tmp4);
										Cudd_RecursiveDeref(manager, tmp5);
										Cudd_RecursiveDeref(manager, tmp6);
										Cudd_RecursiveDeref(manager, res);
										Cudd_RecursiveDeref(manager, tmp);
										tmp = tmp7;
										Cudd_Ref(tmp);
										Cudd_RecursiveDeref(manager, tmp7);
										//3 normalize each particle

									}
									else { //non-RB particles
										//sum out samples, combine weights
										DdNode *tmp1 = Cudd_addExistAbstract(manager, tmp, RBPF_SAMPLE_CUBE);
										Cudd_Ref(tmp1);
										Cudd_RecursiveDeref(manager, tmp);


										//normalize distribution
										DdNode* tmp2 = normalize(tmp1);
										Cudd_Ref(tmp2);
										Cudd_RecursiveDeref(manager, tmp1);
										//printBDD(tmp2);

										//Resample
										DdNode* tmp3 = draw_samples(tmp2, RBPF_SAMPLES);
										Cudd_Ref(tmp3);
										Cudd_RecursiveDeref(manager, tmp2);

										tmp = Cudd_BddToAdd(manager, tmp3);
										Cudd_Ref(tmp);
										Cudd_RecursiveDeref(manager, tmp3);

										//printBDD(tmp);

									}
									//printBDD(RBPF_SAMPLE_CUBE);

									//				//sum out samples, combine weights
									//				tmp1 = Cudd_addExistAbstract(manager, tmp2, RBPF_SAMPLE_CUBE);
									//				Cudd_Ref(tmp1);
									//				Cudd_RecursiveDeref(manager, tmp2);
									//
									//				 tmp = normalize(tmp1);
									//				Cudd_Ref(tmp);
									//				Cudd_RecursiveDeref(manager, tmp1);
									//				//printBDD(tmp);



								}
								else{
									tmp = Cudd_addApply(manager, Cudd_addTimes, (*i).second, parent);
									Cudd_Ref(tmp);
								}
							}
							else{
								tmp = Cudd_bddAnd(manager, (*i).second, parent);
								Cudd_Ref(tmp);
							}
							//		printBDD((*i).second);
							//printBDD(tmp);
							result->push_back(tmp);
							reasons->push_back((*i).first);
							probabilities->push_back(pr1);
							obs++;
						}


						//				Cudd_CheckKeys(manager);
						//				std::cout << "]" << std::endl;


						//cout << "end split" << endl;
						return obs;
}


DdNode* stateForward(DdManager *m,DdNode *f)
{
	DdNode *equiv,*op1,*op2,*op3;
	int i=0;
	equiv = Cudd_ReadLogicZero(m);
	Cudd_Ref(equiv);
	for(;i<num_alt_facts;i++)
	{
		op1 = Cudd_ReadVars(m,i);
		op2 = Cudd_ReadVars(m,Cudd_bddReadPairIndex(m,i));
		op3 = Cudd_Not(op2);
		op1 = Cudd_bddAnd(m,equiv,Cudd_bddIte(m,op1,op2,op3));
		Cudd_Ref(op1);
		Cudd_RecursiveDeref(manager,equiv);
		equiv = op1;

	}
	op1 = Cudd_bddAndAbstract(m,equiv,f,current_state_cube);
	Cudd_Ref(op1);
	Cudd_RecursiveDeref(manager,equiv);
	return op1;
}

DdNode* stateBackward(DdManager *m,DdNode *f)
{
	DdNode *equiv,*op1,*op2,*op3;
	int i=0;
	equiv = Cudd_ReadLogicZero(m);
	Cudd_Ref(equiv);
	for(;i<num_alt_facts;i++)
	{
		op1 = Cudd_ReadVars(m,i);
		op2 = Cudd_ReadVars(m,Cudd_bddReadPairIndex(m,i));
		op3 = Cudd_Not(op2);
		op1 = Cudd_bddAnd(m,equiv,Cudd_bddIte(m,op1,op2,op3));
		Cudd_Ref(op1);
		Cudd_RecursiveDeref(manager,equiv);
		equiv = op1;

	}
	op1 = Cudd_bddAndAbstract(m,equiv,f,next_state_cube);
	Cudd_Ref(op1);
	Cudd_RecursiveDeref(manager,equiv);
	return op1;
}

DdNode* or_merge(list<DdNode*> states)
{
	DdNode *res = Cudd_ReadLogicZero(manager);
	DdNode *temp = NULL;
	for (list<DdNode *>::iterator ite = states.begin(); ite != states.end(); ++ite)
	{
		temp = Cudd_bddOr(manager, res, *ite);
		Cudd_Ref(temp);
		Cudd_RecursiveDeref(manager, res);
		res = temp;
	}
	return res;
}
DdNode* update(DdNode* state, list<DdNode*> eff)
{
	DdNode *temp;
	for (list<DdNode *>::iterator ite = eff.begin(); ite != eff.end(); ++ite)
	{
		if(bdd_entailed(manager, state, *ite))
		{
			continue;
		}
		else if(bdd_entailed(manager, state, Cudd_Not(*ite)))
		{
			state = Cudd_bddRestrict(manager, state, Cudd_Not(*ite));
			// printBDD(state);
			temp = Cudd_bddAnd(manager, state, *ite);
			Cudd_Ref(temp);
			Cudd_RecursiveDeref(manager, state);
			state = temp;
			// printBDD(state);
		}
		else
		{
			temp = Cudd_bddAnd(manager, state, *ite);
			Cudd_Ref(temp);
			Cudd_RecursiveDeref(manager, state);
			state = temp;
		}
	}
	return state;
}

map<DdNode *, set<DdNode *> > SEmap;
void partition(DdNode *ps, const pEffect& effect)
{
	const SimpleEffect *se = dynamic_cast<const SimpleEffect *>(&effect);
	if(se != NULL)
	{
		if(SEmap.find(ps) == SEmap.end())
		{
			SEmap[ps] = set<DdNode *>();
		}
		DdNode *effBDD = formula_bdd(se->atom(), false);
		bool is_true = typeid(*se) == typeid(AddEffect);
		if(! is_true)
		{
			effBDD = Cudd_Not(effBDD);
			Cudd_Ref(effBDD);
		}
		// printBDD(effBDD);
		if (SEmap[ps].find(effBDD) == SEmap[ps].end())
		{
			SEmap[ps].insert(effBDD);
		}
		return;
	}
	const ConjunctiveEffect *ce = dynamic_cast<const ConjunctiveEffect *>(&effect);
	if(ce != NULL)
	{
		size_t n = ce->size();
		for (size_t i = 0; i < n;++i)
		{
			partition(ps, ce->conjunct(i));
		}
		return;
	}
	assert(0);
}
void partition(list<DdNode*> &ps,const pEffect& effect)
{
	// 1. simple efffect
	// all ps add the eff
	const SimpleEffect *se = dynamic_cast<const SimpleEffect *>(&effect);
	if(se != NULL)
	{
		for (list<DdNode *>::iterator ite = ps.begin(); ite != ps.end();++ite)
		{
			if(SEmap.find(*ite) == SEmap.end())
			{
				SEmap[*ite] = set<DdNode *>();
			}
			DdNode *effBDD = formula_bdd(se->atom(),false);
			bool is_true = typeid(*se) == typeid(AddEffect);
			if (!is_true)
			{
				effBDD = Cudd_Not(effBDD);
				Cudd_Ref(effBDD);
			}
			if (SEmap[*ite].find(effBDD) == SEmap[*ite].end())
			{
				SEmap[*ite].insert(effBDD);
			}
		}
		return;
	}
	// 2. conditional effect
	const ConditionalEffect *cde = dynamic_cast<const ConditionalEffect *>(&effect);
	if(cde != NULL)
	{
		list<DdNode *> temp = ps;// create a temp
		DdNode *preBDD = formula_bdd(cde->condition(), false);
		bool flag = false;
		// 考虑每个state
		for (list<DdNode *>::iterator ite = ps.begin(); ite != ps.end(); ++ite)
		{
			// sat, only consider current state
			if(bdd_entailed(manager, *ite, preBDD))
			{
				partition(*ite, cde->effect());
			}
			// unsat, pass this state
			else if(bdd_entailed(manager, *ite, Cudd_Not(preBDD)))
			{
				continue;
			}
			// unknown, split the state
			else
			{
				flag = true;
				DdNode *t1 = Cudd_bddAnd(manager, *ite, preBDD);
				Cudd_Ref(t1);
				Cudd_RecursiveDeref(manager, *ite);
				DdNode *t2 = Cudd_bddAnd(manager,*ite, Cudd_Not(preBDD));
				Cudd_Ref(t2);
				Cudd_RecursiveDeref(manager, *ite);
				temp.remove(*ite);
				temp.push_back(t1);
				temp.push_back(t2);
				SEmap[t1] = SEmap[*ite];
				SEmap[t2] = SEmap[*ite];
				SEmap.erase(*ite);// do not consider this state
				partition(t1, cde->effect());// only add to the entiable one
			}
		}
		if(flag)
			ps = temp;
		return;
	}
	// 3. conjuntion effect
	//   do for each conjunct
	const ConjunctiveEffect *ce = dynamic_cast<const ConjunctiveEffect *>(&effect);
	if(ce != NULL)
	{
		size_t n = ce->size();
		for (size_t i = 0; i < n; ++i)
		{
			// for each effect, may change the set ps (split the state),
			// and next time the new set ps will pass to the partition
			partition(ps, ce->conjunct(i));
		}
		return;
	}
	assert(0);
}
/**
 * momo007 2022.10.24 BDD-based progress
 */
DdNode* progress(DdNode* parent, const Action* a)
{
	SEmap.clear();
	list<DdNode *> ps; // the belief state after partition
	ps.push_back(parent);
	// TO.DO partition
	// 1. consider each precondtion
	// 2. partion the state and update te Effect set.
	// 3. update the Belief state respectively.
	partition(ps, a->effect());
	for (list<DdNode *>::iterator ite = ps.begin(); ite != ps.end(); ++ite)
	{
		// printf("before update:\n");
		// printBDD(*ite);
		// for (set<DdNode *>::iterator i = SEmap[*ite].begin(); i != SEmap[*ite].end(); i++)
		// {
		// 	printBDD(*i);
		// }
		*ite = update(*ite, list<DdNode *>(SEmap[*ite].begin(), SEmap[*ite].end()));
		// printf("done after\n");
		// printBDD(*ite);
	}
	DdNode *res = or_merge(ps);
	// printBDD(res);
	return res;
}
/**
 * momo007 2022.05.11 动作节点进行更新
 */
DdNode* progress(pair<const Action* const, DdNode*>* a, DdNode *parent){
	DdNode* result,*fr = NULL, *fr1= NULL;
	int i,j;

	// case1: 深度信念网络更新（原始代码DBN_PROGRESSION置为true）
	if(DBN_PROGRESSION){
		std::cout << "progress mode: dbn[waring!!!]\n";
		//		std::cout << "(" << std::endl;
		//		Cudd_CheckKeys(manager);
		//		std::cout << "|" << std::endl;
		dbn* d = action_dbn(*(a->first));
		//		Cudd_CheckKeys(manager);
		//				std::cout << ")" << std::endl;

		result = progress(d, parent);
		return result;
	}
	// case2:
	else{
		std::cout << "progress mode: bdd\n";
		// 获取action的BDD，核心实现，涉及到axioms
		DdNode *t = groundActionDD(*(a->first));
		Cudd_Ref(t);
		if (verbosity > 2)
		{
			std::cout << "print action BDD\n";
			printBDD(t);
			if((a->first)->hasObservation())
			{
				int i = 0;
				for (std::list<DdNode *>::iterator ite = action_observations[a->first]->begin(); ite != action_observations[a->first]->end(); ++ite)
				{
					printf("obs: %d\n", i++);
					printBDD(*ite);
				}
			}
		}

		// 动作的BDD为空
		if(t == Cudd_ReadZero(manager) || t == Cudd_ReadLogicZero(manager)){
			std::cout << "action " << a->first->name() << "BDD is zero\n";
			if (my_problem->domain().requirements.probabilistic)
				return Cudd_ReadZero(manager);
			else if(my_problem->domain().requirements.non_deterministic)
				return Cudd_ReadLogicZero(manager);
		}
		if(verbosity>2)
		{
			std::cout << "before perform the action state bdd\n";
			printBDD(parent);	
		}
		// 使用action的BDD进行遗忘更新
		result = progress(t, parent);
		Cudd_RecursiveDeref(manager,t);
		if(verbosity)
		{
			std::cout << "After perform the action\n";
			printBDD(result);	
		}
		std::cout << "@@@@@@@@@@@@@@@@@@@@@@\n";
		// 定义了event，进一步更新
		// DdNode *ex_result;
		// if(event_preconds.size() > 0){
		// 	ex_result = apply_events(result);
		// 	Cudd_Ref(ex_result);
		// 	return ex_result;
		// }
		// else{
		// 	return result;
		// }
		return result;
	}
}
/** momo007
DdNode * apply_events(DdNode *belief){
	list<DdNode*> b;
	DdNode *out = Cudd_ReadOne(manager), *disj= Cudd_ReadLogicZero(manager);
	Cudd_Ref(out);
	Cudd_Ref(disj);

	int iter = 0;
	double diff = 1.0, epsilon = 0.001;
	while(diff > epsilon){  // not reached a fix point
		if(disj != Cudd_ReadLogicZero(manager)){
			Cudd_RecursiveDeref(manager, belief);
			belief = disj;
			Cudd_Ref(belief);
			Cudd_RecursiveDeref(manager, disj);
			out = Cudd_ReadOne(manager);
			Cudd_Ref(disj);
			Cudd_RecursiveDeref(manager, disj);
			disj = Cudd_ReadLogicZero(manager);
			Cudd_Ref(disj);
		}
		for(map<const Action*, DdNode*>::iterator a = event_preconds.begin();
			a != event_preconds.end(); a++){
				if(add_bdd_entailed(manager, belief, (*a).second)){
					DdNode *e = groundEventDD(*((*a).first));
					DdNode *result = progress(e, belief);
					Cudd_Ref(result);
					b.push_back(result);

					DdNode *t = Cudd_addBddStrictThreshold(manager,result,0.0);
					Cudd_Ref(t);
					DdNode *t1 = Cudd_bddOr(manager, t, disj);
					Cudd_Ref(t1);
					Cudd_RecursiveDeref(manager, t);
					Cudd_RecursiveDeref(manager, disj);
					disj = t1;
					Cudd_Ref(disj);
					Cudd_RecursiveDeref(manager, t1);
				}
		}

		DdNode *t = Cudd_BddToAdd(manager, disj);
		Cudd_Ref(t);
		Cudd_RecursiveDeref(manager, disj);
		disj = t;
		Cudd_Ref(disj);
		Cudd_RecursiveDeref(manager, t);

		for(list<DdNode*>::iterator i = b.begin(); i != b.end(); i++){
			DdNode *t = Cudd_addApply(manager, Cudd_addTimes, *i, disj);
			Cudd_Ref(t);
			Cudd_RecursiveDeref(manager, disj);
			disj = t;
			Cudd_Ref(disj);
			Cudd_RecursiveDeref(manager, t);
			Cudd_RecursiveDeref(manager, *i);
		}
		t = normalize(disj);
		Cudd_Ref(t);
		Cudd_RecursiveDeref(manager, disj);
		disj = t;
		Cudd_Ref(disj);
		Cudd_RecursiveDeref(manager, t);

		b.clear();

		DdNode *d = Cudd_addApply(manager, Cudd_addMinus, disj, belief);
		Cudd_Ref(d);
		DdNode *dd = Cudd_addApply(manager, Cudd_addTimes, d, d);
		Cudd_Ref(dd);
		diff = get_sum(dd);
		Cudd_RecursiveDeref(manager, d);
		Cudd_RecursiveDeref(manager, dd);


	}

	return disj;
}
*/


//DdNode * sample_transitions(dbn* a, DdNode* parent){
//	DdNode* result = parent;
//	Cudd_Ref(result);
//
//	int NUM_PF_SAMPLES = 2;
//
//
//
//
//
//	//sample each aux var with stochastic universal sampling
////	for(int i = 0 ; i < a->num_aux_vars; i++){
////		DdNode * t = Cudd_addApply(manager, Cudd_addTimes,
////					result, draw_samples(a->vars[2*num_alt_facts+i]->cpt, NUM_PF_SAMPLES));
////		Cudd_Ref(t);
////		Cudd_RecursiveDeref(manager, result);
////		result = t;
////		Cudd_RecursiveDeref(manager, t);
////	}
//
//	//apply each of the next state cpts to sampled result
//	//sum out previous states (use variable ordering heuristic, skip aux var applies)
//
//}

DdNode * progress(dbn* a, DdNode* parent){

	//std::map<int, DdNode*> *dds  = a->get_dds();

	//	if(RBPF_PROGRESSION){
	//		//sample state transitions for particle filter
	//		return sample_transitions(a, parent);
	//	}


	DdNode* result = parent;
	//cout << "PROGRESS: parent\n";
	//  	printBDD(parent);
	//	std::cout << "{" << std::endl;
	//					Cudd_CheckKeys(manager);
	//					std::cout << "|" << std::endl;

	Cudd_Ref(result);
	//	Cudd_Ref(result);
	//    Cudd_CheckKeys(manager);
	//  std::cout << "{" << std::flush;
	std::list<std::pair<int, bool>*>* elimination_order =a->get_variable_elimination_order();
	//  std::cout << "}" << std::flush;  

	//	std::cout << *a << std::endl;

	//std::list<DdNode*>* cubes = a->get_abstraction_order_cubes();
	int *a_varmap = a->get_varmap();
	//	std::cout << "{" << std::endl;

	std::map<int, DdNode*>* var_cubes = a->get_var_cubes();
	//std::list<DdNode*>::iterator c = cubes->begin();
	//std::cout << "["  << std::flush;
	//Cudd_CheckKeys(manager); std::cout << " | " << std::flush;
	std::set<int> irrelevantNodes;

	std::set<int> processed_parents;

	for(std::list<std::pair<int, bool>*>::iterator i = elimination_order->begin();
		i != elimination_order->end(); i++){
			bool noop = (a_varmap[(*i)->first] == (*i)->first && (*i)->first < 2*num_alt_facts);
			///    std::cout << a_varmap[(*(*i)->begin())] << " " <<(*(*i)->begin()) << std::endl;
			if(!noop){
				//      std::cout << (*i)->first << " " << (*i)->second << std::endl;
				if((*i)->second == true
					//&& (*i)->first < 2*num_alt_facts
					){//eliminate

						if(irrelevantNodes.find((*i)->first) != irrelevantNodes.end()){
							//	  	  cout << "skip irrel" << endl;
							continue;
						}
						if(processed_parents.find((*i)->first) != processed_parents.end()){
							//	  	  cout << "skip pp" << endl;
							continue;
						}


						DdNode *c = (*var_cubes)[(*i)->first];
						//	std::cout << "S" << std::flush;
						//	cout << "c\n";
						//	printBDD(c);

						DdNode *tmp = Cudd_addExistAbstract(manager, result, c);
						Cudd_Ref(tmp);
						Cudd_RecursiveDeref(manager, result);
						result = tmp;
						Cudd_Ref(result);
						Cudd_RecursiveDeref(manager, tmp);

						if((*i)->first == 2*(num_alt_facts-2)+1){
							DdNode *c = (*var_cubes)[2*(num_alt_facts-1)+1];
							//std::cout << "S" << std::flush;
							//		cout << "c (if)\n";
							//	  printBDD(c);

							DdNode *tmp = Cudd_addExistAbstract(manager, result, c);
							Cudd_Ref(tmp);
							Cudd_RecursiveDeref(manager, result);
							result = tmp;
							Cudd_Ref(result);
							Cudd_RecursiveDeref(manager, tmp);
						}

				}
				else if((*i)->second == false){//apply
					if(processed_parents.find((*i)->first) != processed_parents.end()){
						//	  	  cout << "skip ppa" << endl;
						continue;
					}

					// 	//check if irrelevant
					// 	if(0 && (*i)->first >= num_alt_facts*2){
					// 	  DdNode* resultSupport = Cudd_Support(manager, result);
					// 	  Cudd_Ref(resultSupport);
					// 	  set<int>* bits = a->vars[(*i)->first]->dd_bits;
					// 	  bool irrelevant = true;
					// 	  for(set<int>::iterator b = bits->begin(); b != bits->end(); b++){
					// 	    if(Cudd_bddIsVarEssential(manager, resultSupport, *b, 1)){
					// 			  irrelevant = false;
					// 			  break;
					// 	    }
					// 	  }
					// 	  if(irrelevant) {
					// 	    //cout << "IRRELEVANT" << endl;
					// 	    irrelevantNodes.insert((*i)->first);
					// 	    continue;
					// 	  }
					// 	}

					list<DdNode*> *fcpts = &a->vars[(*i)->first]->factored_cpts;
					//	cout << "size = " << fcpts->size() << endl;
					if(fcpts->size()>0 || a->vars[(*i)->first]->cpt == Cudd_ReadZero(manager)){
						DdNode *tres = result;
						Cudd_Ref(tres);
						DdNode *unusedTransitions = tres;// = Cudd_addBddThreshold(manager,tres, 1.0);
						Cudd_Ref(unusedTransitions);
						Cudd_RecursiveDeref(manager, result);
						//	  std::cout << "apply " << (*i)->first << std::endl;


						// if((*i)->first == (2*(num_alt_facts-2))+1){
						result = Cudd_ReadZero(manager);
						// 			    }
						// 			    else if((*i)->first == (2*(num_alt_facts-1))+1){
						// 			      result = tres;
						// 			    }
						Cudd_Ref(result);
						for(list<DdNode*>::iterator f = fcpts->begin(); f!=fcpts->end(); f++){
							DdNode *tmp, *tmp1; 
							//cout << "f\n";
							//printBDD(*f);
							tmp = Cudd_addApply(manager, Cudd_addTimes, *f, tres);
							Cudd_Ref(tmp);	    
							//cout << "tmp\n";
							//printBDD(tmp);
							map<DdNode* const, int>::iterator parent = a->vars[(*i)->first]->probabilistic_parents.find(*f);
							if(tmp == Cudd_ReadZero(manager) && parent != a->vars[(*i)->first]->probabilistic_parents.end() && (*i)->first < 2*(num_alt_facts-2)){
								//probabilistic parent is irrelevant
								irrelevantNodes.insert((*parent).second);
								processed_parents.insert((*parent).second);
								//	      	      cout << "IRRELEVANT" << endl;
							}
							else if(tmp != Cudd_ReadZero(manager)){
								if( parent !=  a->vars[(*i)->first]->probabilistic_parents.end()){

									//apply sink and reward together because have same probabilsitic parent
									if((*i)->first == 2*(num_alt_facts-2)+1){
										for(list<DdNode*>::iterator f1 = a->vars[2*(num_alt_facts-1)+1]->factored_cpts.begin(); f1!=a->vars[2*(num_alt_facts-1)+1]->factored_cpts.end();f1++){
											map<DdNode* const, int>::iterator parent1 = a->vars[2*(num_alt_facts-1)+1]->probabilistic_parents.find(*f1);
											if((*parent1).second == (*parent).second){

												//DdNode** variables = new DdNode*[max_num_aux_vars+1];
												DdNode** variables =  new DdNode*[1];
												int k = 0;
												variables[k++] = Cudd_addIthVar(manager, 2*(num_alt_facts-2)+1);
												DdNode* cube = Cudd_addComputeCube(manager, variables, NULL, 1);
												Cudd_Ref(cube);
												//					cout << "cube\n";
												//		      printBDD(cube);
												delete [] variables;

												DdNode* rBDD = Cudd_addExistAbstract(manager, tmp, cube);
												Cudd_Ref(rBDD);
												//					cout << "rBDD\n";
												//		      printBDD(rBDD);
												DdNode* rExBDD = Cudd_addApply(manager, Cudd_addMinus, tres,rBDD);
												Cudd_Ref(rExBDD);			

												DdNode* tmp2 = rExBDD;//Cudd_addExistAbstract(manager, rExADD, cube);
												// Cudd_bddAnd(manager, unusedTransitions, rExADD);
												Cudd_Ref(tmp2);
												//		      cout << "tmp2 (unused)" << endl;
												//		      printBDD(tmp2);

												dbn_node *n = new dbn_node((*i)->first);
												n->make_noop();
												DdNode *withNoop = Cudd_addApply(manager, Cudd_addTimes, tmp2, n->cpt);
												Cudd_Ref(withNoop);
												delete n;

												//					cout << "f1\n";
												//		      printBDD(*f1);
												tmp1 = Cudd_addApply(manager, Cudd_addTimes, withNoop, *f1);
												Cudd_Ref(tmp1);
												Cudd_RecursiveDeref(manager, tmp2);
												//					cout << "tmp1\n";
												//		       printBDD(tmp1);


												//add neg sink to tmp
												DdNode* tmp4 = Cudd_BddToAdd(manager, Cudd_Not(Cudd_bddIthVar(manager,  2*(num_alt_facts-1)+1)));
												Cudd_Ref(tmp4);
												DdNode* tmp3 = Cudd_addApply(manager, Cudd_addTimes, tmp, tmp4);
												Cudd_Ref(tmp3);
												Cudd_RecursiveDeref(manager, tmp);
												tmp = tmp3;
												Cudd_Ref(tmp);
												Cudd_RecursiveDeref(manager, tmp3);
												Cudd_RecursiveDeref(manager, tmp4);

												tmp2 =Cudd_addApply(manager, Cudd_addPlus, tmp, tmp1);
												Cudd_Ref(tmp2);

												//					cout << "tmp2\n";
												//		      printBDD(tmp2);
												Cudd_RecursiveDeref(manager, tmp);
												tmp = tmp2;
												Cudd_Ref(tmp);
												Cudd_RecursiveDeref(manager, tmp1);
												Cudd_RecursiveDeref(manager, tmp2);
												break;
											}
										}
									}

									//cout << "tmp\n";
									//printBDD(tmp);
									//std::cout << "PP " <<  (*parent).second << std::flush;
									DdNode * cpt = a->vars[(*parent).second]->sample_cpt();

									//		cout << "cpt\n";
									//		printBDD(cpt);
									///DdNode *tmp;

									tmp1 = Cudd_addApply(manager, Cudd_addTimes, cpt, tmp);
									Cudd_Ref(tmp1);
									Cudd_RecursiveDeref(manager, tmp);
									tmp = tmp1;
									Cudd_Ref(tmp);
									Cudd_RecursiveDeref(manager, tmp1);



									DdNode *c = (*var_cubes)[(*parent).second];
									//std::cout << "PS" << std::flush;
									//		cout << "c\n";
									//		printBDD(c);

									tmp1 = Cudd_addExistAbstract(manager, tmp, c);
									Cudd_Ref(tmp1);
									Cudd_RecursiveDeref(manager, tmp);
									tmp = tmp1;
									Cudd_Ref(tmp);
									Cudd_RecursiveDeref(manager, tmp1);
									processed_parents.insert((*parent).second);
									//cout << "pp " << (*parent).second << endl;

									//		cout << "tmp\n";
									//		printBDD(tmp);			   



								}

								//DdNode** variables = new DdNode*[max_num_aux_vars+1];
								DdNode** variables = ((*i)->first == 2*(num_alt_facts-2)+1 ? new DdNode*[2] : new DdNode*[1]);
								int k = 0;
								variables[k++] = Cudd_addIthVar(manager, (*i)->first);
								if((*i)->first == 2*(num_alt_facts-2)+1){
									variables[k++] = Cudd_addIthVar(manager, 2*(num_alt_facts-1)+1);
								}
								//		      for(;k<max_num_aux_vars+1; ){
								//variables[k++]=Cudd_addIthVar(manager, 2*num_alt_facts+k-1);
								//}

								//DdNode* cube = Cudd_addComputeCube(manager, variables, NULL, max_num_aux_vars+1);
								DdNode* cube = Cudd_addComputeCube(manager, variables, NULL, ((*i)->first == 2*(num_alt_facts-2)+1 ? 2: 1));
								Cudd_Ref(cube);
								//				cout << "cube\n";
								//	      printBDD(cube);
								delete [] variables;

								///abstract cube, and negate result
								//		      DdNode* fp = Cudd_addApply(manager, Cudd_addTimes, *f, cpt);
								//		      Cudd_Ref(fp);
								DdNode* rBDD = Cudd_addExistAbstract(manager,tmp, cube);
								Cudd_Ref(rBDD);
								//				cout << "rBDD\n";
								//	      printBDD(rBDD);
								DdNode* rExBDD = Cudd_addApply(manager, Cudd_addMinus, unusedTransitions,rBDD);
								Cudd_Ref(rExBDD);			
								//DdNode* rExADD = Cudd_addApply(manager, Cudd_addMaximum, Cudd_ReadZero(manager), rExBDD);
								//Cudd_Ref(rExADD);			


								DdNode* tmp2 = rExBDD;//Cudd_addExistAbstract(manager, rExADD, cube);
								// Cudd_bddAnd(manager, unusedTransitions, rExADD);
								Cudd_Ref(tmp2);
								Cudd_RecursiveDeref(manager, unusedTransitions);
								unusedTransitions = tmp2;
								Cudd_Ref(unusedTransitions);
								Cudd_RecursiveDeref(manager,tmp2);
								//	      	      cout << "unusedTransitions" << endl;
								//	      printBDD(unusedTransitions);

								//

								if((*i)->first == (2*(num_alt_facts-2))+1){
									//		cout << "tres\n";
									//		printBDD(tres);
									tmp1 = Cudd_addApply(manager, Cudd_addPlus, tmp, result);
									Cudd_Ref(tmp1); 			       
								}
								else {//if((*i)->first == (2*(num_alt_facts-1))+1){
									//		cout << "result\n";
									//		printBDD(result);
									tmp1 = Cudd_addApply(manager, Cudd_addMaximum, tmp, result);
									Cudd_Ref(tmp1); 			       
								}

								Cudd_RecursiveDeref(manager, result);
								result = tmp1;
								Cudd_Ref(result);
								Cudd_RecursiveDeref(manager, tmp);
								Cudd_RecursiveDeref(manager, tmp1);


								//				cout << "result (again)\n";
								//	      printBDD(result);
							}
							else{
								map<DdNode* const, int>::iterator parent = a->vars[(*i)->first]->probabilistic_parents.find(*f);
								if( parent !=  a->vars[(*i)->first]->probabilistic_parents.end()){
									processed_parents.insert((*parent).second);
									//		cout << "ppp " << (*parent).second << endl;
								}

							}
						}
						//			    printBDD(result);
						//add noops
						///build cube of all non parents
						//if((*i)->first != (2*(num_alt_facts-2))+1 && (*i)->first != (2*(num_alt_facts-1))+1){
						//	  std::cout << "noop" << std::endl;
						// 		      DdNode* support = Cudd_Support(manager, result);
						// 		      Cudd_Ref(support);
						// 		      set<int> v;
						// 		      for(int p = 0; p < (2*num_alt_facts)+max_num_aux_vars; p++){
						// 			if(//p != (*i)->first && //p != (*i)->first-1 &&
						// 			   Cudd_bddIsVarEssential(manager, support, p, 1)
						// 			   ){
						// 			  v.insert(p);
						// 			}
						// 		      }
						// 		      Cudd_RecursiveDeref(manager, support);
						// 		      for(list<DdNode*>::iterator f = fcpts->begin(); f!=fcpts->end(); f++){
						// 			support = Cudd_Support(manager, *f);
						// 			Cudd_Ref(support);
						// 			for(int p = 0; p < (2*num_alt_facts)+max_num_aux_vars; p++){
						// 			  if(p != (*i)->first && //p != var-1 &&
						// 			     Cudd_bddIsVarEssential(manager, support, p, 1) && (p >= 2*num_alt_facts || p%2==0)
						// 			     ){
						// 			    v.erase(p);
						// 			  }
						// 			}
						// 			Cudd_RecursiveDeref(manager, support);
						// 		      }
						// 		      DdNode** variables = new DdNode*[v.size()];
						// 		      int k = 0;
						// 		      for(set<int>::iterator p = v.begin(); p != v.end(); p++){
						// 			variables[k++] = Cudd_bddIthVar(manager, *p);
						// 		      }			    
						// 		      DdNode* cube = Cudd_bddComputeCube(manager, variables, NULL, v.size());
						// 		      Cudd_Ref(cube);
						// 		      //printBDD(cube);
						// 		      delete [] variables;

						// 		      ///abstract cube, and negate result
						// 		      DdNode* resBDD = Cudd_addBddThreshold(manager,result, 1.0);
						// 		      Cudd_Ref(resBDD);
						// 		      DdNode* resExBDD = Cudd_bddExistAbstract(manager, resBDD, cube);
						// 		      Cudd_Ref(resExBDD);
						// 		      //printBDD(resExBDD);
						// 		      DdNode* negResExBDD = Cudd_Not(resExBDD);
						// 		      Cudd_Ref(negResExBDD);
						// 		      //printBDD(negResExBDD);
						//			DdNode* negResExADD = Cudd_BddToAdd(manager, unusedTransitions);//negResExBDD);
						//	      Cudd_Ref(negResExADD);

						///apply noop
						dbn_node *n = new dbn_node((*i)->first);
						n->make_noop();
						DdNode *withNoop = Cudd_addApply(manager, Cudd_addTimes, unusedTransitions, n->cpt);
						Cudd_Ref(withNoop);
						delete n;
						if((*i)->first == 2*(num_alt_facts-2)+1){
							n = new dbn_node(2*(num_alt_facts-1)+1);
							n->make_noop();
							DdNode *tmp = Cudd_addApply(manager, Cudd_addTimes, withNoop, n->cpt);
							Cudd_Ref(tmp);
							Cudd_RecursiveDeref(manager, withNoop);
							withNoop = tmp;
							Cudd_Ref(withNoop);
							Cudd_RecursiveDeref(manager, tmp);
							delete n;
						}


						//cout << "with NOOP" << endl;
						//printBDD(withNoop);

						///apply to belief
						DdNode* noopBelief = withNoop;// Cudd_addApply(manager, Cudd_addTimes, tres, withNoop);
						Cudd_Ref(noopBelief);
						//cout << "noopBelief\n";
						//printBDD(noopBelief);

						DdNode* t1 = Cudd_addApply(manager, Cudd_addPlus, noopBelief, result);
						Cudd_Ref(t1);
						Cudd_RecursiveDeref(manager, result);
						result = t1;
						Cudd_Ref(result);
						Cudd_RecursiveDeref(manager, t1);
						Cudd_RecursiveDeref(manager, noopBelief);
						Cudd_RecursiveDeref(manager, withNoop);
						//printBDD(result);
						//Cudd_RecursiveDeref(manager, negResExADD);
						//		      Cudd_RecursiveDeref(manager, negResExBDD);
						//		      Cudd_RecursiveDeref(manager, resExBDD);
						//		      Cudd_RecursiveDeref(manager, resBDD);
						//		      Cudd_RecursiveDeref(manager, cube);

						//}



						// 			    for(list<DdNode*>::iterator f = fcpts->begin(); f!=fcpts->end(); f++){
						// 			      DdNode *tmp, *tmp1; 
						// 			      printBDD(*f);
						// 			      //abstract away variables that are not parents of fcpts
						// 			      //negate remaining and multiply by noop, then apply result to state


						// 			      if((*i)->first == (2*(num_alt_facts-2))+1){
						// 				//printBDD(tres);
						// 				tmp = Cudd_addApply(manager, Cudd_addTimes, *f, tres);
						// 				Cudd_Ref(tmp);

						// 				tmp1 = Cudd_addApply(manager, Cudd_addPlus, tmp, result);
						// 				Cudd_Ref(tmp1); 			       
						// 			      }
						// 			      else if((*i)->first == (2*(num_alt_facts-1))+1){
						// 				printBDD(result);
						// 				tmp1 = Cudd_addApply(manager, Cudd_addTimes, *f, result);
						// 				Cudd_Ref(tmp1); 				
						// 				tmp = tmp1;
						// 				Cudd_Ref(tmp);
						// 			      }
						// 			      Cudd_RecursiveDeref(manager, result);
						// 			      result = tmp1;
						// 			      Cudd_Ref(result);
						// 			      Cudd_RecursiveDeref(manager, tmp);
						// 			      Cudd_RecursiveDeref(manager, tmp1);
						// 			        printBDD(result);
						// 			    }
						Cudd_RecursiveDeref(manager, tres);
					}
					else if((*i)->first < 2*num_alt_facts) {
						DdNode * cpt = a->vars[(*i)->first]->sample_cpt();
						DdNode *tmp;

						if(cpt == Cudd_ReadOne(manager)){//noop
							dbn_node *n = new dbn_node((*i)->first);
							n->make_noop();
							Cudd_RecursiveDeref(manager, cpt);
							cpt = n->cpt;
							Cudd_Ref(cpt);
							delete n;
						}
						//std::cout << "P" << std::flush;
						//		cout << "cpt\n";
						//	  printBDD(cpt);


						tmp = Cudd_addApply(manager, Cudd_addTimes, cpt, result);
						Cudd_Ref(tmp);
						Cudd_RecursiveDeref(manager, result);
						result = tmp;
						Cudd_Ref(result);
						Cudd_RecursiveDeref(manager, tmp);

						if((*i)->first == 2*(num_alt_facts-2)+1){
							cpt = a->vars[2*(num_alt_facts-1)+1]->sample_cpt();
							tmp = Cudd_addApply(manager, Cudd_addTimes, cpt, result);
							Cudd_Ref(tmp);
							Cudd_RecursiveDeref(manager, result);
							result = tmp;
							Cudd_Ref(result);
							Cudd_RecursiveDeref(manager, tmp);
						}
					}
					// printBDD((*dds)[*j]);
				}
				//printBDD(result);
				//			if(result == Cudd_ReadZero(manager)){
				//				std::cout << "zero" << std::endl;
				//				exit(0);
				//			}
			}
	}




	//   for(std::list<DdNode*>::iterator dd = dds->begin(); dd != dds->end(); dd++){
	//     DdNode *tmp = Cudd_addApply(manager, Cudd_addTimes, *dd, result);
	//     Cudd_Ref(tmp);
	//     Cudd_RecursiveDeref(manager, result);
	//     result = tmp;
	//     Cudd_Ref(result);
	//     Cudd_RecursiveDeref(manager, tmp);
	//         printBDD(*dd);
	//      printBDD(result);
	//   }
	//   DdNode *tmp = Cudd_addExistAbstract(manager, result, current_state_cube);
	//   Cudd_Ref(tmp);
	//   Cudd_RecursiveDeref(manager, result);
	//   result = tmp;
	//   Cudd_Ref(result);
	//   Cudd_RecursiveDeref(manager, tmp);

	//   //   printBDD(tmp);

	//   tmp = Cudd_addExistAbstract(manager, result, aux_var_cube);
	//   Cudd_Ref(tmp);
	//   Cudd_RecursiveDeref(manager, result);
	//   result = tmp;
	//   Cudd_Ref(result);
	//   Cudd_RecursiveDeref(manager, tmp);
	//  //  printBDD(aux_var_cube);
	//
	//	std::cout << "M" << std::flush;
	//	   printBDD(result);


	//	   int num = (2*num_alt_facts)+max_num_aux_vars+(int)ceil(log2(RBPF_SAMPLES));
	//	   std::cout << "num1: " << num << " " << max_num_aux_vars<< std::endl;
	//	   for(int b = 0 ; b < num; b++)
	//		   std::cout<< "[" << b << ", " << a_varmap[b] << "] " << std::flush;
	//	   std::cout << std::endl;

	DdNode* tmp =  Cudd_addPermute(manager, result, a_varmap);
	Cudd_Ref(tmp);

	Cudd_RecursiveDeref(manager, result);
	result = tmp;
	Cudd_Ref(result);
	Cudd_RecursiveDeref(manager, tmp);

	// 	std::cout << "result (Before normalize)" << std::endl;
	//   	printBDD(result);


	//    result = normalize(result);
	//    Cudd_Ref(result);


	//split
	//		std::cout << "End Result" << std::endl;
	//	   printBDD(result);

	return result;
}

// DdNode * progress(dbn* a, DdNode* parent){

//   map<int, DdNode*> *dds  = a->get_dds();

//   DdNode* result = parent;
//   Cudd_Ref(result);

//   list<set<int>*>* order = a->get_abstraction_order();
//   list<DdNode*>* cubes = a->get_abstraction_order_cubes();
//   list<DdNode*>::iterator c = cubes->begin(); 
//   for(list<set<int>*>::iterator i = order->begin(); (c != cubes->end() && i != order->end()); c++, i++){
//     for(set<int>::iterator j = (*i)->begin(); j != (*i)->end(); j++){
//       DdNode *tmp = Cudd_addApply(manager, Cudd_addTimes, (*dds)[*j], result);
//       Cudd_Ref(tmp);

//       Cudd_RecursiveDeref(manager, result);
//       result = tmp;
//       Cudd_Ref(result);
//       Cudd_RecursiveDeref(manager, tmp);
//     }

//     DdNode *tmp = Cudd_addExistAbstract(manager, result, *c);
//     Cudd_Ref(tmp);
//     Cudd_RecursiveDeref(manager, result);
//     result = tmp;
//     Cudd_Ref(result);
//     Cudd_RecursiveDeref(manager, tmp);
//   }

//   DdNode* tmp =  Cudd_addPermute(manager, result, varmap);
//     Cudd_Ref(tmp);

//   Cudd_RecursiveDeref(manager, result);
//   result = tmp;
//   Cudd_Ref(result);
//   Cudd_RecursiveDeref(manager, tmp);

//   result = normalize(result);
//   Cudd_Ref(result);

//   return result;
// }

DdNode * progress(DdNode *image,DdNode *parent){
	DdNode *tmp1,*tmp2,*result;
	// 基于概率的progress
	if(my_problem->domain().requirements.probabilistic){
		std::cout << "probabilitis progres[assert]\n";
		assert(0);
		DdNode *ddp = Cudd_addApply(manager, Cudd_addTimes, parent, image);
		Cudd_Ref(ddp);

		tmp1 = Cudd_addExistAbstract(manager, ddp, current_state_cube);
		Cudd_Ref(tmp1);
		Cudd_RecursiveDeref(manager,ddp);

		result =  Cudd_addPermute(manager, tmp1, varmap);
		Cudd_Ref(result);
		Cudd_RecursiveDeref(manager,tmp1);
	}
	// 非确定性更新
	else if(my_problem->domain().requirements.non_deterministic){
		std::cout << "bdd progress\n";
		// 计算 parent /\ image，随后对 current state variable进行遗忘
		tmp1 = Cudd_bddAndAbstract(manager,parent, image,current_state_cube);
		Cudd_Ref(tmp1);
		// 将后继状态变量替换为当前状态变量，使用Cudd_SetVarMap的映射（set_cube进行了封装）进行替换
		result = Cudd_bddVarMap(manager,tmp1);
		Cudd_Ref(result);
		Cudd_RecursiveDeref(manager,tmp1);
	}
	return result;
}



const char* getAction(struct ActionNode* a){
	ostringstream os (ostringstream::out);
	if(!a || !a->act || !my_problem || !&(*my_problem).terms())
		return "BOGUS--FIXME";
	a->act->print(os, (*my_problem).terms());
	return os.str().c_str();
}

void printAction(struct ActionNode* a){
	ostringstream os (ostringstream::out);
	if(!a || !a->act || !my_problem || !&(*my_problem).terms())
		return;
	a->act->print(os, (*my_problem).terms());
	cout << os.str() <<endl;
}

/**
 *  从goal出发，点为每个节点创建启发值
 */
void CreateHeuristic()
/****** This doesn't yet work correctly for discounted MDPs ******/
{
	StateList *CurrentList, *NewList;
	StateList::iterator node;
	ActionNodeList::iterator prev;

	CurrentList = new StateList;

	/* Initial CurrentList contains GOAL */
	Goal->h = 0.0;
	Goal->g = 0.0;
	Goal->f = 0.0;
	Goal->fWeight = 0.0;
	CurrentList->push_back(Goal);

	/* Add unvisited states that can be reached by one backward step */
	while(!CurrentList->empty()){   /* check for empty list */
		NewList = new StateList;
		/* For each state added in previous step ... */
		for(node = CurrentList->begin(); node != CurrentList->end(); node++){
			/* ... and for each parent of that state ... */
			for(prev = (*node)->PrevActions->begin(); prev != (*node)->PrevActions->end(); prev++){
				/* If parent has not already been updated, update heuristic and add to list */
				ActionNode* prevAct = *prev;
				if(prevAct->PrevState->h == 0.0){//设置启发值
					cout << "Updating State " << prevAct->PrevState->StateNo << endl;
					printBDD(prevAct->PrevState->dd);
					prevAct->PrevState->f =
						prevAct->PrevState->h =
						gDiscount * (*node)->h + prevAct->Cost;
					prevAct->PrevState->g = 0.0;
					prevAct->PrevState->fWeight = gWeight * prevAct->PrevState->f;
					NewList->push_back(prevAct->PrevState);
				}
			}
		}

		delete CurrentList;
		CurrentList = NewList;
		NewList = NULL;
	}

	delete CurrentList;
}

ofstream pfout;
extern int num_lugs;
static int actNum = 1;
/**
 * momo007 2025.05.25 bug
 * 输出plan这块有bug，没办法正确输出
 */
void outputPlan(){

	gEndTime = clock();
	/**
	int min, max, numPlans;
	min = 10000;
	max = -1;
	numPlans = 0;
	BitVector*   solved_visited;
	pfout.open (out_file, ofstream::out );

	list<double> costs;
	list<double> plan_prs;

	double expcost = 0;
	double exppr = 0;

	gNumStates = state_count;
	pfout << "(define (plan " << dname << ")" <<endl;
	pfout << "\t(:problem " << pname << ")" <<endl;
	pfout << "(" << endl;

	cout << "NumStatesGenerated = " << state_count << endl;
	//printStateList();

	if( Start->ExtendedGoalSatisfaction < plan_success_threshold){// 失败
		cout << "$$$$$$$$$$$$$$$No Plan :( $$$$$$$$$$$$$$$$$$$$$$$$"<< endl;
		cout << "P(G) = " << Start->ExtendedGoalSatisfaction << endl;
	}
	else{// 成功
		cout << "$$$$$$$$$$$$$$$GOT PLAN$$$$$$$$$$$$$$$$$$$$$$$$"<<endl;
		outputPlanR(Start, 0, 1, max, min, numPlans, solved_visited, 0, &costs, 1.0, &plan_prs);
		cout << "$$$$$$$$$$$$$$$GOT PLAN$$$$$$$$$$$$$$$$$$$$$$$$"<<endl;
		pfout << ")" << endl;
		pfout << ")" <<endl;
		pfout.close();
		cout << "Num LUGS = " << num_lugs << endl;
		for(list<double>::iterator i = costs.begin(); i != costs.end(); i++){
			expcost += *i;
		}
		for(list<double>::iterator i = plan_prs.begin(); i != plan_prs.end(); i++){
			exppr += *i;
		}
		if(  my_problem->domain().requirements.rewards && DBN_PROGRESSION){
			cout << "E[Reward] = " << Start->expDiscRew << endl;
		}

		cout << "p(Plan Success) = "<< Start->ExtendedGoalSatisfaction <<endl;

	}
	*/
	gNumStates = state_count;
	pfout << "(define (plan " << dname << ")" <<endl;
	pfout << "\t(:problem " << pname << ")" <<endl;
	pfout << "(" << endl;

	cout << "NumStatesGenerated = " << state_count << endl;
	cout << "ExpandNode = " << expandedNodes << endl;
	cout << "Heuristic type is:" << HEURISTYPE << endl;
	printf("Total User Time = %f secs\n", (float)((gEndTime - gStartTime) / (float)CLOCKS_PER_SEC));
}

void outputPlanR(StateNode* s, int indent, int step, int &max, int &min,
		int &numPlans, BitVector* solved_visited,
		double cost, list<double>* costs, double path_pr,
		list<double>* plan_prs ){

			if (!s)    {
				numPlans++;
				if(step < min)
					min = step;
				if(step > max)
					max = step;
				pfout << "l" << actNum<<" (done " << actNum << " " <<-1<<")"<<endl;
				cout << "l" << actNum<< " (done " << actNum << " inf)" <<endl;
				return;
			}



			for(int i = 0; i < indent; i++){
				cout << " ";
				pfout << " ";
			}

			if(s->Terminal){
				costs->push_back(cost);
				numPlans++;
				if(step < min)
					min = step;
				if(step > max)
					max = step;
				double d = (path_pr*s->goalSatisfaction);
				plan_prs->push_back(d);
				pfout << "(done)" << endl;
				cout <<"(done) P(G) = " << s->goalSatisfaction
					<< " P(branch) = " << s->prReached //path_pr
					<<  endl;

				return;
			}


			//printBDD(s->dd);
			//cout << "pr reach = " << s->prReached << endl;
			//    cout << "State = " << s->StateNo << " h = " << s->h <<endl;
			// cout << "E[R] " << s->expDiscRew << endl;
			//   cout << "E[Pr] " << s->ExtendedGoalSatisfaction << endl;
			//   if(s->BestAction)
			//     cout << "E[Ra] " << s->BestAction->expDiscRew << endl;

			if(s->BestAction &&
				s->Update == -10 &&
				s->BestAction->Solved != -1 ){
					pfout <<"(goto l"<< s->BestAction->Solved << ")"<< endl;
					cout << "(goto l" << s->BestAction->Solved << ")"<< endl;
					return;
			}
			else  if(s->BestAction){
				pfout << "l" << actNum << " " << getAction(s->BestAction) << " " << endl;
				cout << "l" << actNum << " ";
				printAction(s->BestAction);
				cout << flush;

				s->Update = -10;
				s->BestAction->Solved = actNum++;
			}

			else {
				numPlans++;
				if(step < min)
					min = step;
				if(step > max)
					max = step;

				pfout << "l" << actNum<<" (done " << s->g << " " <<s->h<<")"<<endl;
				cout << "l" << actNum<< " (done " << s->g << " " <<s->h<<") "<<endl;
				return;
			}


			struct StateDistribution* nextStates = s->BestAction->NextState;

			if (!nextStates)
			{
				outputPlanR(0,indent,step+1,max,min,numPlans,solved_visited,
					cost+s->BestAction->Cost,
					costs, path_pr,plan_prs);
				return;
			}

			if(nextStates->Next)
			{
				// then we have more than one next state distinguished by reasons
				for(int i =0; i < indent; i++){
					cout << " ";
					pfout << " ";
				}
				pfout << "(case ";
				cout << "(case ";
				indent += 5;

				while(nextStates){

					pfout << endl;
					cout << endl;
					for(int i = 0; i < indent-1; i++){
						cout << " ";
						pfout <<  " ";
					}
					cout << "(" << nextStates->Prob ;
					pfout << "(";
					//    printBDD(nextStates->reason);

					if(OBS_TYPE==OBS_CPT){
						for(set<const pEffect*>::iterator i = nextStates->creason->begin();
							i !=  nextStates->creason->end(); i++){
								(*i)->print(cout, my_problem->domain().predicates(), my_problem->domain().functions(), my_problem->terms());
								cout << " " << flush;
						}
						cout << endl;
					}
					else{
						pfout << " " << BDDToITEString(nextStates->reason);
						cout << " " << BDDToITEString(nextStates->reason);

						pfout << endl;
						cout << endl;
					}

					outputPlanR(nextStates->State, indent, step+1, max, min,
						numPlans, solved_visited, cost+s->BestAction->Cost,
						costs, path_pr*nextStates->Prob,plan_prs);


					for(int i =0;  i < indent-1; i++){
						cout << " ";
						pfout << " ";
					}
					pfout << ")" << endl;
					cout << ")" << endl;

					nextStates = nextStates->Next;

				}
				indent -= 5;
				for(int i =0; i < indent; i++){
					pfout << " ";
					cout << " ";
				}

				pfout << ")" << endl;
				cout << ")" << endl;
			}
			else
				outputPlanR(nextStates->State,indent,step+1,max,min,numPlans,solved_visited, cost+s->BestAction->Cost, costs, path_pr,plan_prs);
}

#include "lug.h"
#include <stdio.h>
#include "graph.h"
#include "graph_wrapper.h"
#include "labelledElement.h"
#include "dd.h"
#include "memory.h"
#include "output.h"
#include "lao_wrapper.h"
#include "build_graph.h"
#include "statistical.h"

#include <iostream>
#include <list>
#include <set>
#include <algorithm>
#include <limits.h>
//#include "actions/actions.h"
#include <stdlib.h>
#include <float.h>
#include <math.h>
#include "correlation.h"
#include "dd.h"
#include <typeinfo>
using namespace std;

RelaxedPlan* my_rp;
extern int pos_fact_at_min_level[MAX_RELEVANT_FACTS];
extern int neg_fact_at_min_level[MAX_RELEVANT_FACTS];
extern DdNode* initialBDD;
extern int NDLcounter;
extern  DdNode** extractDNFfromBDD(DdNode*);
extern  DdNode** extractTermsFromMinterm(DdNode* node);
extern  void free_graph_and_search_info();
extern  void free_graph_info(int);
extern  int build_graph(int*,int,int,int);
extern  BitVector* new_bit_vector(int);
extern  void printMemInfo();

extern int num_alt_facts;
extern int num_alt_acts;
extern int num_alt_effs;
extern  void cost_prop_ops(int);
extern  void cost_prop_efs(int);
extern  void cost_prop_facts(int);
extern  int getFactWorldCost(int fact, DdNode* worlds);
extern  int get_bit( BitVector *, int , int );

extern  int getActWorldCost(int act, DdNode* worlds);
extern  int getEffectWorldCost(int effect, DdNode* worlds);
extern  void setFactWorldCost(int fact, int cost, DdNode* worlds);
extern  void setActWorldCost(int act, int cost, DdNode* worlds);
extern  void setEffectWorldCost(int effect, int cost, DdNode* worlds);
extern  int getFactCover(FtNode* f, DdNode* worlds, int time);
//extern  void unionElementLabels(std::set<LabelledElement*>* );
extern int getlevel(int index, int polarity);// momo007 add support for getMinSumHueristicValue
std::list<std::list<LabelledElement*>*> ** actWorldCost;
std::list<std::list<LabelledElement*>*> ** effectWorldCost;
std::list<std::list<LabelledElement*>*> ** factWorldCost;
double expectedReward[IPP_MAX_PLAN]; //stores reward reached, at each level of LUG

//std::list<list<LabelledElement*>*> ** sactWorldCost;
//std::list<list<LabelledElement*>*> ** seffectWorldCost;
//std::list<list<LabelledElement*>*> ** sfactWorldCost;

extern int graph_levels;

int num_lugs = 0;// 创建lug graph


std::list<std::list<int>* >* level_vars;

//Integer** integers;


// 未使用
void increment_heuristic(StateNode* node){

	cout << "increment " <<  node->StateNo<< " " << node->hIncrement << endl;


	double oldh = node->h;
	std::list<StateNode*> states;
	states.push_back(node);

	int oldp = NUMBER_OF_MGS;

	NUMBER_OF_MGS *= node->hIncrement+1;

	if(node->currentRP){
		delete node->currentRP;
		node->currentRP = NULL;
	}
	getHeuristic(&states, node, 0);
	//  cout << "got h " <<endl;
	NUMBER_OF_MGS=oldp;

	if(!node->currentRP){
		// cout << "no rp"<< endl;
		if(USE_GLOBAL_RP){
			if(0){//incremental, otherwise h is correct
				node->h += oldh;
			}
		}
		else{
			node->h = 0;//DBL_MAX;
			// cout << "NO RP" <<endl;
			//    exit(0);
		}
	}
	else{
		///     cout << " rp"<< endl;
		if(0){//incremetnal no recomp
			if(!node->rpIncrement)
				node->rpIncrement = new RelaxedPlanLite();
			node->rpIncrement->unionRP(node->currentRP);
			node->h = node->rpIncrement->h_value();
		}
		else{
			node->h = node->currentRP->getRelaxedConformantCost();
			delete node->currentRP;
			node->currentRP = NULL;
		}


	}

	node->hValues[node->hIncrement] = node->h;

	if(fabs(oldh - node->h) <= INCREMENT_MIN_SLOPE){
		//    cout << "CAP \t\t\t\t\t\t\t\t\t\t" << node->hIncrement <<endl;
		for(int i = node->hIncrement+1; i < MAX_H_INCREMENT; i++)
			node->hValues[node->hIncrement] = node->h;
		node->hIncrement = MAX_H_INCREMENT;
	}
	else
		node->hIncrement++;
	//   cout << "Increment " << endl;//node->StateNo << " h = " << node->h << endl;


}


#ifdef PPDDL_PARSER
void initLUG(std::map<const Action*, DdNode*>* acts , DdNode* goal){

#else
	extern action_list available_acts;
	void initLUG(action_list acts, DdNode* goal){
		action_list_node* a = acts;
#endif //ppddlparser

		// 考虑fact需要多少个int进行存储
		gft_vector_length =  ( ( int )( num_alt_facts) / (gcword_size) )+1;
		printf("gft_vector_length = %d\n", gft_vector_length);
		//     cout << "INSTANTIATE LUG ACTIONS" <<endl;
#ifdef PPDDL_PARSER
		// make list to keep track of label vars at levels of LUG
		level_vars = new list<list<int>* >();
		// 创建action的BitOperator
		for(std::map<const Action*, DdNode*>::iterator a = acts->begin();
				a != acts->end(); a++){
			// 007 here still need to check
			if(!a->first->hasObservation())
				generate_BitOperators((*a).first);
		}
#else
		pos = Cudd_BddToAdd(manager, p);
		while (a) {

			if(a->act->is_original() && !a->act->get_effs()->obs)
				generate_BitOperators(a);
			a = a->next;
		}
#endif //ppddlparser


		gef_vector_length =  ( ( int ) gnum_cond_effects_pre / gcword_size )+1+2*gft_vector_length;
		gop_vector_length = ((int) gnum_bit_operators /gcword_size)+1;

		 cout << "gef_Vector_length = " << gnum_cond_effects_pre << " "<<gef_vector_length <<endl;
		 cout << "gop_Vector_length = " << gnum_bit_operators << " "<<gop_vector_length <<endl;
		// cout << "cost sizezes = " << (num_alt_acts+(num_alt_facts*2)) << " " << (num_alt_effs+num_alt_facts*2) << " " << (num_alt_facts*2)<<endl;
		//cout << "num_alt_effs " << num_alt_effs << " " << num_alt_facts << endl;
		if(COMPUTE_LABELS &&  RP_EFFECT_SELECTION != RP_E_S_COVERAGE){// RP_EFFECT_SELECTION default is RP_E_S_COVERAGE
			actWorldCost = new std::list<list<LabelledElement*>*>*[num_alt_acts+(num_alt_facts*2)];
			effectWorldCost = new std::list<list<LabelledElement*>*>*[num_alt_effs+(num_alt_facts*2)];
			factWorldCost = new std::list<list<LabelledElement*>*>*[num_alt_facts*2];
			for(int i = 0; i < (num_alt_acts+num_alt_facts*2); i++){
				actWorldCost[i] = new std::list<list<LabelledElement*>*>();
			}
			for(int i = 0; i < (num_alt_effs+num_alt_facts*2); i++){
				effectWorldCost[i] = new std::list<list<LabelledElement*>*>();
			}
			for(int i = 0; i < 2*num_alt_facts; i++){
				factWorldCost[i] = new list<list<LabelledElement*>*>();
			}
		}
		//   if(SENSORS && USESENSORS){
		//     sactWorldCost = new (std::list<LabelledElement*>*)[num_alt_acts+(num_alt_facts*2)];
		//     seffectWorldCost = new (std::list<LabelledElement*>*)[num_alt_effs+num_alt_facts*2];
		//     sfactWorldCost = new (std::list<LabelledElement*>*)[num_alt_facts*2];
		//   }

		//   integers = new Integer*[1000000];
		//   for(int i = 0; i < 1000000; i++){
		//   integers[i] = new Integer(i);
		//}



		//  cout << "INSTANTIATE LUG GOAL" <<endl;
		FactInfo *tpos = new_FactInfo();
		FactInfo *tneg = new_FactInfo();

		DdNode* support;

		support = Cudd_Support(manager, goal);
		Cudd_Ref(support);
#ifdef PPDDL_PARSER
		/**
		 * Momo007 2022.09.16
		 * 下面这块分配的空间，在MG模式下，调用build_planning_graph会重复，
		 * 同时造成gbit_goal_state的内存没有释放
		 */
		for(int i = 0; i < num_alt_facts; i++){
			if(bdd_entailed(manager, support, Cudd_bddIthVar(manager, 2*i))){
				if(Cudd_bddIsVarEssential(manager, goal, 2*i, 1)){
					make_entry_in_FactInfo( &tpos, i);
				}
				else if(Cudd_bddIsVarEssential(manager, goal, 2*i, 0)){
					make_entry_in_FactInfo( &tneg, i);
				}
				else{
					make_entry_in_FactInfo( &tpos, i);
					make_entry_in_FactInfo( &tneg, i);
				}
			}
#else
	for(int i = 0; i < num_alt_facts; i++){
		if(bdd_entailed(manager, support, Cudd_bddIthVar(manager, i))){
			if(Cudd_bddIsVarEssential(manager, goal, i, 0)){
				make_entry_in_FactInfo( &tpos, i);
			}
			else if(Cudd_bddIsVarEssential(manager, goal, i, 1)){
				make_entry_in_FactInfo( &tneg, i);
			}
			else{
				make_entry_in_FactInfo( &tpos, i);
				make_entry_in_FactInfo( &tneg, i);
			}
		}
#endif

	}
	Cudd_RecursiveDeref(manager,support);
	// if(!(dg = dynamic_cast<disj_goal*>(goal->getClauses()))){
	//     if(!(cg = dynamic_cast<conj_goal*>(goal->getClauses()))){
	//       if((ng = dynamic_cast<neg_goal*>(goal->getClauses()))){
	//   if((sg = dynamic_cast<simple_goal*>(ng->getGoal())))
	//     make_entry_in_FactInfo( &tneg, sg->getProp()->ground->getnum());
	//       } else if((sg = dynamic_cast<simple_goal*>(goal->getClauses()))) {
	//   make_entry_in_FactInfo( &tpos,  sg->getProp()->ground->getnum() );
	//       }
	//     } else {
	//       goal_list::iterator k = cg->getGoals()->begin();
	//       for (; k != cg->getGoals()->end();++k ) {
	//   if((ng = dynamic_cast<neg_goal*>(*k))){
	//     if((sg = dynamic_cast<simple_goal*>(ng->getGoal())))
	//       make_entry_in_FactInfo( &tneg, sg->getProp()->ground->getnum());

	//   } else if((sg = dynamic_cast<simple_goal*>(*k))){
	//     make_entry_in_FactInfo( &tpos,  sg->getProp()->ground->getnum() );
	//   }
	//       }
	//     }
	//   } else{}

	//  print_BitVector(tpos->vector, gft_vector_length);
	//  print_BitVector(tneg->vector, gft_vector_length);

	gbit_goal_state = new_fact_info_pair( tpos, tneg );
	my_gbit_goal_state = gbit_goal_state;
	// cout << "init lug" <<endl;

		}


		void addCostLevel(int w){

			switch (w){
			case 0: { //actions
				for(int i = 0; i < num_alt_acts+(2*num_alt_facts); i++){
					actWorldCost[i]->push_back(new list<LabelledElement*>());
				}
				break;
			}
			case 1: { //effects
				for(int i = 0; i < num_alt_effs+(2*num_alt_facts); i++){
					effectWorldCost[i]->push_back(new list<LabelledElement*>());
				}
				break;
			}
			case 2: { //facts
				for(int i = 0; i < (2*num_alt_facts); i++){
					factWorldCost[i]->push_back(new list<LabelledElement*>());
				}
				break;
			}
			};
		}


		extern double gDiscount;
		int costLevelOff(int time){
			list<LabelledElement*>* tmpList, *tmpList1;
			list<LabelledElement*>::iterator tmp, tmp1;
			//   cout << "level off costs " << time
			//        << " goals reached = " << GOALS_REACHED
			//        <<endl;

			if(time - COST_K_LOOKAHEAD >= GOALS_REACHED)
				return TRUE;


			if(goal_threshold > 0){//do costs of individual literals change??
				if(time > 0){
					for(int i = 0; i < 2*num_alt_facts; i++){
						tmpList = getCostsAt(time, i, 2);
						tmpList1 = getCostsAt(time-1, i, 2);

						if(tmpList->size() != tmpList1->size())
							return FALSE;

						for(tmp = tmpList->begin(); tmp != tmpList->end(); tmp++){
							for(tmp1 = tmpList1->begin(); tmp1 != tmpList1->end(); tmp1++){
								if((*tmp)->label == (*tmp1)->label && (*tmp)->cost != (*tmp1)->cost){
									//       cout << "YO " << i << " "
											//      << ((*tmp1)->cost-(*tmp)->cost) <<endl;

									//     if((*tmp1)->cost < (*tmp)->cost){
									//       cout << "cost increased!!"<<endl;
									//       printBDD((*tmp)->label);
									//       exit(0);
									//     }

									return FALSE;
								}
								//else cout << "NO"<<endl;
							}
						}
					}
					return TRUE;
				}
			}
			else{//return false if E[cost(goal) - reward(goal)] does not decrease
				double pr_goal;
				double goalCost;
				double goalReward;
				if(goal_samples != Cudd_ReadLogicZero(manager)){
					goalCost = costOfGoal(time, goal_samples);
					pr_goal = (double)countParticles(goal_samples)/(double)NUMBER_OF_MGS;
					goalReward = pr_goal*total_goal_reward;
					expectedReward[time] = (goalCost-goalReward);//*pow(gDiscount, time);
				}
				else{
					pr_goal =  0.0;
					goalCost = 0.0;
					goalReward = 0.0;
					expectedReward[time] = DBL_MAX;
				}

				//      cout << "goalCost = " << goalCost
				//     << " goalReward = " << goalReward
				//     << " exp = " << expectedReward[time] << endl;

				if(time > 0){
					return expectedReward[time] - expectedReward[time-1] > 0;
				}
				else
					return FALSE;
			}

		}

		void printCosts(int w, int time){
			list<list<LabelledElement*>*>::iterator j;
			list<LabelledElement*>::iterator l;
			int max;

			switch (w){
			case 0: { //actions
				max = num_alt_acts+2*num_alt_facts;
				break;
			}
			case 1: { //effects
				max = num_alt_effs+2*num_alt_facts;
				break;
			}
			case 2: { //facts
				max = 2*num_alt_facts;
				break;
			}

			};


			int i = 0;
			while(i < max){
				switch (w){
				case 0: { //actions
					cout << "act " << i << endl;
					j = actWorldCost[i]->begin();
					break;
				}
				case 1: { //effects
					cout << "effect " << i << endl;
					j = effectWorldCost[i]->begin();
					break;
				}
				case 2: { //facts
					cout << "fact " << i << " ";
					(i>=num_alt_facts ? printFact(i-num_alt_facts): printFact(i)); cout << endl;
					j = factWorldCost[i]->begin();
					break;
				}

				};

				for(int k =0; k < time; k++) j++;
				if((*j)->size() >1){
					cout << "COSTS at " << time <<endl;
					for(l = (*j)->begin(); l !=(*j)->end(); l++){
						//  cout << "cost = " << ((Integer*)(*l)->elt)->value << endl;
						cout << "cost = " << (*l)->cost << endl;
						printBDD((*l)->label);
					}
				}
				i++;
			}
			//  cout << "Exit"<<endl;
		}

		void copyCostLevelUp(int w, int i){
			list<list<LabelledElement*>*>::iterator j, k;
			list<LabelledElement*>::iterator l;

			switch (w){
			case 0: { //actions
				j = actWorldCost[i]->end();
				k = actWorldCost[i]->end();
				break;
			}
			case 1: { //effects
				j = effectWorldCost[i]->end();
				k = effectWorldCost[i]->end();
				break;
			}
			case 2: { //facts
				j = factWorldCost[i]->end();
				k = factWorldCost[i]->end();
				break;
			}
			};
			j--;
			k--;k--;

			for(l = (*k)->begin(); l !=(*k)->end(); l++)
				(*j)->push_back(new LabelledElement(*l));


		}


		void setFactWorldCost(int time, int fact, int cost, DdNode* worlds){
			//   if(fact == 0){
			//     cout << "set " << fact << " cost = " << cost <<  " time = " << time << " size = "<<factWorldCost[fact]->size() << endl;
			//     printBDD(worlds);
			//   }
			list<list<LabelledElement*>*>::iterator i;


			i = factWorldCost[fact]->begin();
			for(int j = 0; j < time; j++) i++;
			// (*i)->push_back(new LabelledElement((int*)integers[cost], worlds));
			(*i)->push_back(new LabelledElement(worlds, cost));
			//cout << "Ok"<<endl;
		}
		void setActWorldCost(int time, int act, int cost, DdNode* worlds){
			//  cout << "set " << act << " cost = " << cost << endl;
			list<list<LabelledElement*>*>::iterator i;

			i = actWorldCost[act]->begin();
			for(int j = 0; j < time; j++) i++;
			//  (*i)->push_back(new LabelledElement((int*)integers[cost], worlds));
			(*i)->push_back(new LabelledElement(worlds, cost));

		}
		void setEffectWorldCost(int time, int effect, int cost, DdNode* worlds){
			//cout << "set " << effect << " cost = " << cost << endl;



			list<list<LabelledElement*>*>::iterator i;

			i = effectWorldCost[effect]->begin();
			for(int j = 0; j < time; j++) i++;
			(*i)->push_back(new LabelledElement(worlds, cost));
		}

		list<LabelledElement*>* getCostsAt(int time, int elt, int type){
			list<list<LabelledElement*>*>::iterator j;

			switch (type){
			case 0: { //actions
				j = actWorldCost[elt]->begin();
				break;
			}
			case 1: { //effects
				j = effectWorldCost[elt]->begin();
				break;
			}
			case 2: { //facts
				j = factWorldCost[elt]->begin();
				break;
			}
			};


			for(int i =0 ; i < time; i++) j++;
			//  cout << "siex -= " << (*j)->size() <<endl;
			return (*j);

		}

		// struct weight_cover_compare
		// {
		//   int operator()(LabelledElement* p, LabelledElement* q) const
		//   {
		//     double cost_p = (double)bdd_size(manager, p->label)/(double)((Integer*)p->elt)->value;
		//     double cost_q = (double)bdd_size(manager, q->label)/(double)((Integer*)q->elt)->value;

		//     if(cost_p > cost_q)
		//       return -1;
		//     else
		//       return 1;

		//   }
		// };

		double sqr(double a){
			return a;
		}
		// DdNode* tmpWs;
		// int op_less_than(LabelledElement* p, LabelledElement* q){
		//   //double cost_p = (double)Cudd_CountMinterm(manager, p->label, num_alt_facts)/(double)((Integer*)p->elt)->value;
		//   //double cost_q = (double)Cudd_CountMinterm(manager, q->label, num_alt_facts)/(double)((Integer*)q->elt)->value;
		//   double cost_p;// = ((Integer*)p->elt)->value;
		//   double cost_q;// = ((Integer*)q->elt)->value;
		//   DdNode* tmpW;
		//     if( RP_EFFECT_SELECTION == RP_E_S_RATIO){
		//       tmpW = bdd_and(manager, b_initial_state, tmpWs);
		//        cost_p = p->cost*
		//    sqr((double)Cudd_CountMinterm(manager,tmpW, num_alt_facts)/(double)Cudd_CountMinterm(manager, p->label, num_alt_facts));
		//        cost_q = q->cost*
		//    sqr((double)Cudd_CountMinterm(manager,tmpW, num_alt_facts)/(double)Cudd_CountMinterm(manager, q->label, num_alt_facts));
		//     }
		//     else if( RP_EFFECT_SELECTION == RP_E_S_COST){
		//       cost_p = p->cost;
		//       cost_q = q->cost;
		//     }
		//     else if( RP_EFFECT_SELECTION == RP_E_S_COVERAGE){
		//       tmpW = bdd_and(manager, b_initial_state, tmpWs);
		//       cost_p = sqr((double)Cudd_CountMinterm(manager,tmpW, num_alt_facts)/(double)Cudd_CountMinterm(manager, p->label, num_alt_facts));
		//       cost_q = sqr((double)Cudd_CountMinterm(manager,tmpW, num_alt_facts)/(double)Cudd_CountMinterm(manager, q->label, num_alt_facts));
		//     }
		//     if(cost_p < cost_q)
		//       return 1;
		//     else
		//       return 0;
		// }


		double elt_cost(LabelledElement *p, DdNode* worlds){
			if( RP_EFFECT_SELECTION == RP_E_S_RATIO && PF_LUG){
				//     cout << "cost = " << p->cost
				//    << " pr = " << (double)countParticles(p->label)/(double)NUMBER_OF_MGS
				//    << endl;
				return  p->cost/(double)countParticles(p->label);
			}
			else if( RP_EFFECT_SELECTION == RP_E_S_RATIO){
				DdNode *covered = Cudd_bddAnd(manager, worlds, p->label);

				return  p->cost*
				(double)Cudd_CountMinterm(manager, worlds, num_alt_facts)/
				(double)Cudd_CountMinterm(manager, p->label, num_alt_facts);
			}
			else if( RP_EFFECT_SELECTION == RP_E_S_COST){
				return  p->cost;
			}
			else if( RP_EFFECT_SELECTION == RP_E_S_COVERAGE){
				return  (double)Cudd_CountMinterm(manager, worlds, num_alt_facts)/(double)Cudd_CountMinterm(manager, p->label, num_alt_facts);
			}
		}

		int opnode_comp (OpNode* a, OpNode* b){return (a->alt_index == b->alt_index);}

		// int contains(list<OpNode*> mylist, OpNode* op){
		//   list<OpNode*>::iterator i = mylist.begin();
		//   for(;i!= mylist.end(); i++)
		//     if((*i)->alt_index == op->alt_index)
		//       return 1;
		//   return 0;
		// }
		int contains(list<EfNode*> mylist, EfNode* op){
			list<EfNode*>::iterator i = mylist.begin();
			for(;i!= mylist.end(); i++)
				if((*i)->alt_index == op->alt_index)
					return 1;
			return 0;
		}

		//need to find min cover of worlds with sets in my list
		int costOfCover(std::list<LabelledElement*>* my_list, DdNode* worlds){

			std::list<LabelledElement*> candidates;// = new std::list<LabelledElement*>();
			list<EfNode*> effs;//list<OpNode*> acts;
			EfNode* last_eff;//OpNode *last_act;
			std::list<LabelledElement*>::iterator j;
			std::list<LabelledElement*>::iterator k;
			std::list<LabelledElement*>::iterator l;
			std::list<LabelledElement*>::iterator i = my_list->begin();
			int returnVal;


			DdNode* all_stored = Cudd_ReadLogicZero(manager), *fr, *chosenWorlds;
			LabelledElement* tmpElt, *tmpElt1, *minCan;
			double minCost, cost;

			//   printf("before my cover\n");fflush(stdout);
			//    Cudd_CheckKeys(manager);fflush(stdout);


			Cudd_Ref(all_stored);

			// cout << "cost of cover, worlds = " <<endl;
			//     cout << "list size = " << my_list->size()<<endl;
			//      printBDD(worlds);

			// cout << "my list size = " << my_list->size()<<endl;
			for(;i != my_list->end(); i++){
				//cout << "i = " << ((Integer*)(*i)->elt)->value << endl;
				// printBDD((*i)->label);
				chosenWorlds = Cudd_bddAnd(manager, (*i)->label, worlds);
				Cudd_Ref(chosenWorlds);
				if(chosenWorlds != Cudd_ReadLogicZero(manager)){
					//cout << "cost = " << (*i)->cost <<endl;
					if((*i)->cost >= 0){
						//Cudd_Ref(chosenWorlds);
						((((LabelledEffect*)(*i)) &&
								(*i)->elt != NULL &&
								((LabelledEffect*)(*i))->elt != NULL) ?
										candidates.push_back(new LabelledEffect(((LabelledEffect*)(*i))->elt,
												chosenWorlds,
												(*i)->cost))
												:
												candidates.push_back(new LabelledElement(chosenWorlds,
														(*i)->cost)));

						fr = Cudd_bddOr(manager, all_stored, chosenWorlds);
						Cudd_Ref(fr);
						Cudd_RecursiveDeref(manager, all_stored);
						all_stored = fr;
						Cudd_Ref(all_stored);
						Cudd_RecursiveDeref(manager, fr);
						Cudd_RecursiveDeref(manager, chosenWorlds);
						//Cudd_Ref(all_stored);

					}
				}
				//Cudd_RecursiveDeref(manager, chosenWorlds);


			}
			//   cout << "GOT:"<<endl;
			//        printBDD(all_stored);
			//  cout << "can size = " << candidates.size()<<endl;
			//     printf("after my cover\n");fflush(stdout);
			//    Cudd_CheckKeys(manager);fflush(stdout);



			if(worlds != all_stored || //!bdd_entailed(manager, worlds, all_stored) ||
					(candidates.size() == 0) || worlds == Cudd_ReadLogicZero(manager)){
				Cudd_RecursiveDeref(manager, all_stored);

				cout<< "CANT COVER WORLDS" <<endl;
				printBDD(worlds);
				cout << "with" << endl;
				printBDD(all_stored);
				exit(0);
				//    cout << "INT_MAX" <<endl;
				return INT_MAX; //can't support worlds
			}
			Cudd_RecursiveDeref(manager, all_stored);

			if(COST_PROP_WORLD==SUM)
				returnVal = 0;
			else if(COST_PROP_WORLD==MAXN){
				returnVal = -1;
			}
			// cout << "covering"<<endl;
			chosenWorlds = Cudd_ReadLogicZero(manager);
			Cudd_Ref(chosenWorlds);
			while(chosenWorlds != worlds){
				//cout << "|candidates| = " << candidates.size() <<endl;

				//     cout << "worlds:" <<endl;
				//     printBDD(worlds);
				//     cout << "chosenworlds:" <<endl;
				//     printBDD(chosenWorlds);

				//find minCandidate
				minCost = DBL_MAX;
				minCan = NULL;



				for(j = candidates.begin(); j != candidates.end(); j++){
					cost = elt_cost(*j, worlds);
					//cout << "can cost = " << cost << endl;
					if(cost < minCost){
						minCan = *j;
						minCost = cost;
						//  cout << "set min " << cost <<endl;
					}

				}


				//cout << "min cost = " << minCost << endl;
				candidates.remove(minCan);

				fr = Cudd_bddOr(manager, chosenWorlds, minCan->label);
				Cudd_Ref(fr);
				Cudd_RecursiveDeref(manager, chosenWorlds);
				chosenWorlds = fr;
				Cudd_Ref(chosenWorlds);
				Cudd_RecursiveDeref(manager, fr);

				if(COST_PROP_WORLD==SUM)
					returnVal += //minCan->cost;//
							minCost;
				else if(COST_PROP_WORLD==MAXN){
					if(minCost > returnVal)
						returnVal = //minCan->cost;//
								minCost;
				}



				if(((LabelledEffect*)(minCan)) && minCan->elt != NULL &&
						((LabelledEffect*)(minCan))->elt != NULL){
					last_eff = ((LabelledEffect*)minCan)->elt;//last_act = ((LabelledEffect*)minCan)->elt->op;
					effs.push_back(last_eff);//acts.push_back(last_act);
				}

				for(k = candidates.begin();  k != candidates.end(); ){
					//if(!bdd_is_one(manager,Cudd_Not((*k)->label))){
					fr = Cudd_bddAnd(manager, (*k)->label, Cudd_Not(minCan->label));
					Cudd_Ref(fr);
					Cudd_RecursiveDeref(manager, (*k)->label);
					(*k)->label = fr;
					Cudd_Ref((*k)->label);
					Cudd_RecursiveDeref(manager, fr);
					//}
				if((*k)->label == Cudd_ReadLogicZero(manager)){
					//  cout << "A"<<endl;
					candidates.remove((*k));
					delete *k;
					k=candidates.begin();
				}
				else if((((LabelledEffect*)(*k)) &&
						(*k)->elt != NULL &&
						((LabelledEffect*)(*k))->elt != NULL)){
					//  cout << "B"<<endl;
					//if we are covering a fact with effects, reduce cost of other
					//effects of same action should we choose them
					if(!contains(effs, ((EfNode*)(*k)->elt)) && //!contains(acts, ((EfNode*)(*k)->elt)->op) &&
							last_eff->alt_index == ((EfNode*)(*k)->elt)->alt_index &&//last_act->alt_index == ((EfNode*)(*k)->elt)->op->alt_index &&
							last_eff->alt_index < num_alt_effs//last_act->alt_index < num_alt_acts
					){
						// cout << "reducing the cost of " << ((EfNode*)(*k)->elt)->op->alt_index << " "  <<  (*k)->cost << " " << alt_act_costs[((EfNode*)(*k)->elt)->op->alt_index] <<endl;
						(*k)->cost -= -1*((EfNode*)(*k)->elt)->effect->reward;//alt_act_costs[((EfNode*)(*k)->elt)->op->alt_index];
						//(*k)->cost = 0;
					}
					k++;
				}
				else{
					//  cout << "C"<<endl;
					++k;
				}
				}
				delete minCan;
			}
			if(returnVal < 0) {
				cout << endl << "COVER ERROR for: "<< returnVal << endl;
				//printBDD(worlds); printBDD(chosenWorlds);
				cout << "eq: " << (worlds == chosenWorlds)<<endl;
				exit(0);
			}// returnVal = 0;
			for(k = candidates.begin(); k != candidates.end(); k++){
				delete (*k);
			}
			candidates.clear();
			Cudd_RecursiveDeref(manager, chosenWorlds);

			//      cout << "return = " << returnVal << endl;
			return returnVal;

		}

		DdNode* unionNDLabels(EfLevelInfo_pointer);


		int getFactCover(FtNode* fact, DdNode* worlds, int time){
			std::list<LabelledElement*> my_list;// = new std::list<LabelledElement*>();
			std::list<LabelledElement*>::iterator k;
			DdNode* tmp, *fr, *all_worlds;
			double cost;
			EfEdge* add = fact->adders;


			if(time == 0){
				cout << "can't cover facts at 0"<<endl;
				exit(0);
			}

			//   all_worlds = Cudd_ReadLogicZero(manager);
			//   Cudd_Ref(all_worlds);

			//cout << "factCover" <<endl;
			//  printBDD(fact->info_at[time]->label->label);
			while(add){
				if(add->ef->info_at[time-1]  // && !add->ef->info_at[time-1]->is_dummy
				){
					//cout << "adder = " << add->ef->op->name << " " << add->ef->alt_index <<endl;
					tmp = Cudd_bddAnd(manager, worlds, add->ef->info_at[time-1]->label->label);
					Cudd_Ref(tmp);

					//       fr = Cudd_bddOr(manager, tmp, all_worlds);
					//       Cudd_Ref(fr);
					//       Cudd_RecursiveDeref(manager, all_worlds);
					//       all_worlds = fr;
					//       Cudd_Ref(all_worlds);
					//       Cudd_RecursiveDeref(manager, fr);


					if(tmp != Cudd_ReadLogicZero(manager)){
						//printBDD(tmp);
						//   if(add->ef->op->alt_index < num_alt_acts){

						//     cost =  getEffectWorldCost(time-1, add->ef->alt_index, tmp);//+
						//     //alt_act_costs[add->ef->op->alt_index];
						//   }
						//   else{
						cost = getEffectWorldCost(time-1, add->ef->alt_index, tmp);
						//  }

						// cout << "cost = " << cost <<endl;
						if(cost != INT_MAX && cost != -INT_MAX// && cost != DBL_MAX && cost != -DBL_MAX
						){
							Cudd_Ref(tmp);
							my_list.push_back(new LabelledEffect(add->ef, tmp, cost));
						}
					}
					Cudd_RecursiveDeref(manager, tmp);

				}
				add = add->next;
			}


			//   if(all_worlds != worlds){
			//     cout<< "CANT fact COVER WORLDS" <<endl;
			//     printBDD(worlds);
			//     cout << "with" << endl;
			//     printBDD(all_worlds);
			//     exit(0);

			//   }


			//   printf("before cover\n");fflush(stdout);
			//    Cudd_CheckKeys(manager);fflush(stdout);


			cost = costOfCover(&my_list, worlds);
			//   printf("after cover\n");fflush(stdout);
			//    Cudd_CheckKeys(manager);fflush(stdout);

			for(k = my_list.begin(); k != my_list.end(); k++){
				delete (*k);
			}
			//my_list.clear();



			return cost;

		}

		void checkCover(list<LabelledElement*> *mlist, DdNode *worlds,
				int type, int elt, int time){
			DdNode *tmp, *fr, *all_worlds;
			all_worlds = Cudd_ReadLogicZero(manager);
			Cudd_Ref(all_worlds);
			for(list<LabelledElement*>::iterator i = mlist->begin();
					i != mlist->end(); i++){
				tmp = Cudd_bddAnd(manager, worlds, (*i)->label);
				Cudd_Ref(tmp);

				fr = Cudd_bddOr(manager, tmp, all_worlds);
				Cudd_Ref(fr);
				Cudd_RecursiveDeref(manager, all_worlds);
				all_worlds = fr;
				Cudd_Ref(all_worlds);
				Cudd_RecursiveDeref(manager, fr);
				Cudd_RecursiveDeref(manager, tmp);
			}

			if(all_worlds != worlds){
				cout<< "CANT get cost COVER WORLDS "
						<< type << " "<< elt
						<< " " << time
						<<endl;
				printBDD(worlds);
				cout << "with" << endl;
				printBDD(all_worlds);
				exit(0);

			}
		}


		int getFactWorldCost(int time, int fact, DdNode* worlds){
			//cout << "cost of " << fact<<endl;
			//    (fact > num_alt_facts ? printFact(fact-num_alt_facts) : printFact(fact));
			list<list<LabelledElement*>*>::iterator i = factWorldCost[fact]->begin();
			for(int j =0; j < time  ; j++) i++;

			// checkCover(*i, worlds, 0, fact, time);

			return costOfCover(*i, worlds);
		}
		int getActWorldCost(int time, int act, DdNode* worlds){
			// cout << "act cost " << endl;
			list<list<LabelledElement*>*>::iterator i = actWorldCost[act]->begin();
			for(int j =0; j< time; j++) i++;
			//checkCover(*i, worlds, 1, act, time);
			return costOfCover(*i, worlds);
		}
		int getEffectWorldCost(int time, int effect, DdNode* worlds){
			list<list<LabelledElement*>*>::iterator i = effectWorldCost[effect]->begin();

			for(int j =0; j< time; j++) i++;
			// checkCover(*i, worlds, 2, effect, time);
			return costOfCover(*i, worlds);
		}

		void setInitCostVector(DdNode* init){
			DdNode* tmp;
			list<list<LabelledElement*>*>::iterator j;
			//  cout << "set init cost vect" <<endl;
			for(int i = 0; i < num_alt_facts; i++){
				// cout << "i = " << i << endl;
				factWorldCost[i]->push_back(new list<LabelledElement*>());
				j = factWorldCost[i]->begin();
				tmp = bdd_and(manager, init, init_pos_labels[i]);
				if(!bdd_is_one(manager, Cudd_Not(tmp)))
					(*j)->push_back(new LabelledElement(tmp, 0));
				factWorldCost[i+num_alt_facts]->push_back(new list<LabelledElement*>());
				j = factWorldCost[i+num_alt_facts]->begin();
				tmp = Cudd_bddAnd(manager, init, init_neg_labels[i]);
				Cudd_Ref(tmp);
				if(!bdd_is_one(manager, Cudd_Not(tmp)))
					(*j)->push_back(new LabelledElement(tmp, 0));
			}
		}

		void clearEltVector(list<list<LabelledElement*>*>* mlist){
			list<list<LabelledElement*>*>::iterator i;
			for(i = mlist->begin(); i != mlist->end(); i++){
				delete(*i);
			}
			mlist->clear();
		}

		void clearCostVectors(){
			std::list<list<LabelledElement*>*>::iterator k;
			for(int i = 0; i < num_alt_acts+(2*num_alt_facts); i++){
				clearEltVector(actWorldCost[i]);
			}
			for(int i = 0; i < num_alt_effs+(2*num_alt_facts); i++){
				clearEltVector(effectWorldCost[i]);
			}
			for(int i = 0; i < num_alt_facts*2; i++){
				clearEltVector(factWorldCost[i]);
			}
		}

/**
 * 创建初始Fact layer和初始状态的label BDD
 */
#ifdef PPDDL_PARSER
void createInitLayer(DdNode* init){
	FactInfo *tpos = new_FactInfo();
	FactInfo *tneg = new_FactInfo();
	DdNode *b_sg, *b_ng,* tmp, *tmp1;
	int j = 0;
	DdNode *c;

	GOALS_REACHED = IPP_MAX_PLAN;

	if(COMPUTE_LABELS){// LUG enter
		if(PF_LUG && my_problem->domain().requirements.probabilistic){

			tmp = init; //tmp holds the mapping from states to particles
			Cudd_Ref(tmp);

			//Cudd_CheckKeys(manager);  fflush(stdout);  cout <<"]";    fflush(stdout);
			if(LUG_FOR != SPACE){
				c = Cudd_addBddStrictThreshold(manager, current_state_cube, 0.0);
				Cudd_Ref(c);
				tmp1 = Cudd_bddExistAbstract(manager, init, c);
				Cudd_Ref(tmp1);
				Cudd_RecursiveDeref(manager, init);
				init = tmp1;
				Cudd_Ref(init);
				Cudd_RecursiveDeref(manager, tmp1);
			}

			if(new_sampleDD)
				Cudd_RecursiveDeref(manager, new_sampleDD);

			new_sampleDD = init;
			Cudd_Ref(new_sampleDD);
		}
		else if(PF_LUG && my_problem->domain().requirements.non_deterministic){
			tmp = init;
			Cudd_Ref(tmp);
			c = current_state_cube;
			Cudd_Ref(c);

			tmp1 = Cudd_bddExistAbstract(manager, init, c);
			Cudd_Ref(tmp1);
			Cudd_RecursiveDeref(manager, init);
			init = tmp1;
			Cudd_Ref(init);
			Cudd_RecursiveDeref(manager, tmp1);

			if(new_sampleDD)
				Cudd_RecursiveDeref(manager, new_sampleDD);

			new_sampleDD = init;
			Cudd_Ref(new_sampleDD);
//			      cout << "new_sampleDD:"<<endl;
//			       printBDD(new_sampleDD);
		}
		else  if(!PF_LUG && my_problem->domain().requirements.probabilistic){
			probabilisticLabels = init;
			Cudd_Ref(probabilisticLabels);
			tmp = Cudd_addBddStrictThreshold(manager, init, 0.0);
			Cudd_Ref(tmp);
		}
		// conformant planning
		else if(!PF_LUG && my_problem->domain().requirements.non_deterministic){
			tmp = init;
			Cudd_Ref(tmp);
			probabilisticLabels = Cudd_ReadOne(manager);
			Cudd_Ref(probabilisticLabels);
		}

	}
	// SG enter
	else{
		tmp = init;
		Cudd_Ref(tmp);
		Cudd_Ref(tmp);
		Cudd_Ref(tmp);
	}
	b_initial_state = init;
	initialBDD = init;
	// printf("tmp bdd is\n");
	// printBDD(tmp);
	// Cudd_Ref(b_initial_state);
	//Cudd_Ref(initialBDD);


	// cout << "SET INIT FACTS"<<endl;
	/**
	 * momo007 2022.10.10
	 * 这里无论是SG还是LUG都使用创建label。(如果SGRP这里取消label会大大增加expand的个数,原因未知)
	 */
	for(int i = 0; i < num_alt_facts; i++){

		//  cout << "[" << endl; Cudd_CheckKeys(manager); cout <<"|" << endl;

		// 记录每个fact为true和false的情况下，创建label（BDD即满足该fact的状态集合）
		//    if(COMPUTE_LABELS){
		b_sg = Cudd_bddIthVar(manager,2*i);
		//Cudd_Ref(b_sg);
		init_pos_labels[i] = Cudd_bddAnd(manager, b_sg, tmp);
		Cudd_Ref(init_pos_labels[i]);
		// printBDD(init_pos_labels[i]);
		b_ng = Cudd_Not(b_sg);//Cudd_bddIthVar(manager,2*i));
		//Cudd_Ref(b_ng);
		init_neg_labels[i] = Cudd_bddAnd(manager, b_ng, tmp);
		Cudd_Ref(init_neg_labels[i]);
		//}

		// 如果合取某个fact的BDD不为0，该变量将该位置为1
		if(//!COMPUTE_LABELS ||
				Cudd_ReadLogicZero(manager) !=  init_pos_labels[i]){
			make_entry_in_FactInfo( &tpos, i);
		}


		if(//!COMPUTE_LABELS ||
				Cudd_ReadLogicZero(manager) != init_neg_labels[i]){
			make_entry_in_FactInfo( &tneg, i);
		}
		// default do not enter
		if(PF_LUG && LUG_FOR != SPACE){//abstract out state variables to leave sample variables

			DdNode *tmp1 = Cudd_bddExistAbstract(manager, init_pos_labels[i], c);
			Cudd_Ref(tmp1);
			Cudd_RecursiveDeref(manager, init_pos_labels[i]);
			init_pos_labels[i] = tmp1;
			Cudd_Ref(init_pos_labels[i]);
			Cudd_RecursiveDeref(manager, tmp1);
			tmp1 = Cudd_bddExistAbstract(manager, init_neg_labels[i], c);
			Cudd_Ref(tmp1);
			Cudd_RecursiveDeref(manager, init_neg_labels[i]);
			init_neg_labels[i] = tmp1;
			Cudd_Ref(init_neg_labels[i]);
			Cudd_RecursiveDeref(manager, tmp1);
		}
	}

	// default do not enter
	if(!COST_REPROP && COMPUTE_LABELS && RP_EFFECT_SELECTION != RP_E_S_COVERAGE)
		setInitCostVector(init);

	Cudd_RecursiveDeref(manager, tmp);
	// FactInfoPair存储正负命题的List
	gbit_initial_state = new_fact_info_pair( tpos, tneg );
	//   print_fact_info(tpos, gft_vector_length);
	//   print_fact_info(tneg, gft_vector_length);
	 //cout << "DONE" <<endl;

	if(PF_LUG && LUG_FOR != SPACE)
		Cudd_RecursiveDeref(manager, c);
	// print_BitVector(gbit_initial_state->positive->vector,gft_vector_length);
	// print_BitVector(gbit_initial_state->negative->vector,gft_vector_length);
	// print_BitVector(gbit_goal_state->positive->vector,gft_vector_length);
	// print_BitVector(gbit_goal_state->negative->vector,gft_vector_length);

}

int updateInitLayer(DdNode* newSamples){
	DdNode *tmp, *tmp1, *tmp2, *c, *b_sg, *b_ng;
	FtNode *ft;

	int updated = 0;

	//refine initial (pg) state bdd
	tmp = newSamples;
	Cudd_Ref(tmp);

	//     printBDD(tmp);

	if(PF_LUG){
		c = Cudd_addBddStrictThreshold(manager, current_state_cube, 0.0);
		Cudd_Ref(c);
		tmp1 = Cudd_bddExistAbstract(manager, newSamples, c);
		Cudd_Ref(tmp1);
		Cudd_RecursiveDeref(manager, newSamples);
		newSamples = tmp1;
		Cudd_Ref(newSamples);
		Cudd_RecursiveDeref(manager, tmp1);


		if(new_sampleDD)
			Cudd_RecursiveDeref(manager, new_sampleDD);

		new_sampleDD = newSamples;
		Cudd_Ref(new_sampleDD);
	}

	if(bdd_entailed(manager, newSamples, initialBDD))
		return 0;


	if(1){
		tmp1 = Cudd_bddOr(manager, newSamples, initialBDD);
	}
	else{
		tmp1 = Cudd_overApproxOr(manager, newSamples, initialBDD);
		if(tmp1 == initialBDD)//bdd_entailed(manager, newSamples, initialBDD))
			return 0;
	}
	Cudd_Ref(tmp1);




	Cudd_RecursiveDeref(manager, initialBDD);
	initialBDD = tmp1;
	Cudd_Ref(initialBDD);
	Cudd_RecursiveDeref(manager, tmp1);

	Cudd_RecursiveDeref(manager, b_initial_state);
	b_initial_state = initialBDD;
	Cudd_Ref(b_initial_state);


	//   cout << "WORLDS in LUG ARE:"<<endl;
	//   printBDD(b_initial_state);

	//   printBDD(overApprox(manager, b_initial_state));



	gprev_level_fts_pointer = gall_fts_pointer;
	for(int i = 0; i < num_alt_facts; i++){
		b_sg = Cudd_bddIthVar(manager,2*i);
		Cudd_Ref(b_sg);
		b_ng = Cudd_Not(Cudd_bddIthVar(manager,2*i));
		Cudd_Ref(b_ng);


		//update positive fluent
		tmp1 = Cudd_bddAnd(manager, b_sg, tmp);
		Cudd_Ref(tmp1);

		tmp2 = Cudd_bddOr(manager, tmp1, init_pos_labels[i]);
		Cudd_Ref(tmp2);
		Cudd_RecursiveDeref(manager, tmp1);
		Cudd_RecursiveDeref(manager, init_pos_labels[i]);
		init_pos_labels[i] = tmp2;
		Cudd_Ref(init_pos_labels[i]);
		Cudd_RecursiveDeref(manager, tmp2);




		if(PF_LUG){//abstract out state variables to leave sample variables
			tmp1 = Cudd_bddExistAbstract(manager, init_pos_labels[i], c);
			Cudd_Ref(tmp1);
			Cudd_RecursiveDeref(manager, init_pos_labels[i]);
			init_pos_labels[i] = tmp1;
			Cudd_Ref(init_pos_labels[i]);
			Cudd_RecursiveDeref(manager, tmp1);
		}

		if(init_pos_labels[i] != Cudd_ReadLogicZero(manager)){
			if(!gft_table[i]){//no fact node
				ft = new_ft_node(0, i, TRUE, FALSE, new_Label(init_pos_labels[i], 0));
				gft_table[i] = ft;

				//    cout << "ADDED POS FACT" << endl;
				//    printFact(i);

				ft->next = gall_fts_pointer;
				gall_fts_pointer = ft;
				(gpos_facts_vector_at[0])[ft->uid_block] |= ft->uid_mask;
				ft->info_at[0]->updated = 1;
				updated = 1;
			}
			else if(!gft_table[i]->info_at[0]){ //exists but not at this level
				//   cout << "new pos level info" <<endl;
				//    printFact(i);
				ft = gft_table[i];
				ft->info_at[0] = new_ft_level_info(gft_table[i]);
				ft->info_at[0]->updated = 1;
				updated = 1;
				(gpos_facts_vector_at[0])[ft->uid_block] |= ft->uid_mask;
				ft->info_at[0]->label = new_Label(init_pos_labels[i], 0);
				//  printBDD(init_pos_labels[i]);
			}
			else {//fact node exists
				ft = gft_table[i];

				if(!ft->info_at[0]->label){
					//    cout << "pos no label yet"<<endl;
					ft->info_at[0]->label = new_Label(init_pos_labels[i], 0);
					ft->info_at[0]->updated = 1;
					updated = 1;
				}
				else if(ft->info_at[0]->label->label !=
						init_pos_labels[i]){
					//    cout << "pos new label"<<endl;
					//    printBDD(ft->info_at[0]->label->label);
					ft->info_at[0]->updated = 1;
					Cudd_RecursiveDeref(manager, ft->info_at[0]->label->label);
					ft->info_at[0]->label->label = init_pos_labels[i];
					Cudd_Ref(ft->info_at[0]->label->label);
					updated = 1;

				}
			}
		}


		//update  negative fluent
		tmp1 = Cudd_bddAnd(manager, b_ng, tmp);
		Cudd_Ref(tmp1);
		tmp2 = Cudd_bddOr(manager, tmp1, init_neg_labels[i]);
		Cudd_Ref(tmp2);
		Cudd_RecursiveDeref(manager, tmp1);
		Cudd_RecursiveDeref(manager, init_neg_labels[i]);
		init_neg_labels[i] = tmp2;
		Cudd_Ref(init_neg_labels[i]);
		Cudd_RecursiveDeref(manager, tmp2);



		if(PF_LUG){//abstract out state variables to leave sample variables
			tmp1 = Cudd_bddExistAbstract(manager, init_neg_labels[i], c);
			Cudd_Ref(tmp1);
			Cudd_RecursiveDeref(manager, init_neg_labels[i]);
			init_neg_labels[i] = tmp1;
			Cudd_Ref(init_neg_labels[i]);
			Cudd_RecursiveDeref(manager, tmp1);
		}
		if(init_neg_labels[i] != Cudd_ReadLogicZero(manager)){
			if(!gft_table[NEG_ADR(i)]){  //no fact node
				//   cout << "ADDED NEG FACT" << endl;
				//    printFact(i);

				ft = new_ft_node(0, i, FALSE, FALSE, new_Label(init_neg_labels[i], 0));
				gft_table[NEG_ADR(i)] = ft;
				ft->next = gall_fts_pointer;
				gall_fts_pointer = ft;
				(gneg_facts_vector_at[0])[ft->uid_block] |= ft->uid_mask;
				ft->info_at[0]->updated = 1;
				updated = 1;
			}
			else if(!gft_table[NEG_ADR(i)]->info_at[0]){
				//    cout << "new neg level info" <<endl;
				//    printFact(i);
				ft = gft_table[NEG_ADR(i)];
				ft->info_at[0] = new_ft_level_info(ft);
				ft->info_at[0]->updated = 1;
				updated = 1;
				(gneg_facts_vector_at[0])[ft->uid_block] |= ft->uid_mask;
				ft->info_at[0]->label = new_Label(init_neg_labels[i], 0);
			}
			else { //fact node exists at time
				ft = gft_table[NEG_ADR(i)];

				if(!ft->info_at[0]->label){
					ft->info_at[0]->label = new_Label(init_neg_labels[i], 0);
					ft->info_at[0]->updated = 1;
					updated = 1;
				}
				else if(ft->info_at[0]->label->label != init_neg_labels[i]){
					ft->info_at[0]->updated = 1;
					updated = 1;
					Cudd_RecursiveDeref(manager, ft->info_at[0]->label->label);
					ft->info_at[0]->label->label = init_neg_labels[i];
					Cudd_Ref(ft->info_at[0]->label->label);
				}
			}

		}

		Cudd_RecursiveDeref(manager, b_sg);
		Cudd_RecursiveDeref(manager, b_ng);
	}

	Cudd_RecursiveDeref(manager, tmp);
	if(PF_LUG)
		Cudd_RecursiveDeref(manager, c);


	return updated;
}

#else
void createInitLayer(DdNode* init){
	FactInfo *tpos = new_FactInfo();
	FactInfo *tneg = new_FactInfo();
	DdNode *b_sg, *b_ng,* tmp;
	int j = 0;

	b_initial_state = init;
	initialBDD = init;



	//   cout << "Setting LUG initial Layer ="<<endl;
	//   printBDD(init);

	//counter for number of new vars used for effect labels
	NDLcounter = 0;

	//    cout << "SET INIT FACTS"<<endl;
	for(int i = 0; i < num_alt_facts; i++){
		b_sg = Cudd_bddIthVar(manager,i);
		b_ng = Cudd_Not(Cudd_bddIthVar(manager,i));

		init_pos_labels[i] = Cudd_bddAnd(manager, b_sg, init);
		Cudd_Ref(init_pos_labels[i]);

		if(!bdd_is_zero(manager, init_pos_labels[i])){
			make_entry_in_FactInfo( &tpos, i);
		}
		init_neg_labels[i] = Cudd_bddAnd(manager, b_ng, init);
		Cudd_Ref(init_neg_labels[i]);

		if(!bdd_is_zero(manager, init_neg_labels[i])){
			make_entry_in_FactInfo( &tneg, i);
		}

	}

	if(!COST_REPROP && COMPUTE_LABELS && RP_EFFECT_SELECTION != RP_E_S_COVERAGE)
		setInitCostVector(init);

	gbit_initial_state = new_fact_info_pair( tpos, tneg );
	//    print_fact_info(tpos, gft_vector_length);
	//     print_fact_info(tneg, gft_vector_length);
	// cout << "DONE" <<endl;


}
#endif
DdNode* substitute_label(DdNode* dd, int k){
	DdNode* returnDD = Cudd_ReadOne(manager);
	int got_one = false;
	DdNode *fr;

	for(int i = 0; i < num_alt_facts; i++){
		if(gft_table[i] &&
				gft_table[i]->info_at[k] &&
				!gft_table[i]->info_at[k]->is_dummy &&
				gft_table[i]->info_at[k]->label &&
#ifdef PPDDL_PARSER
bdd_entailed(manager,
		dd,
		Cudd_bddIthVar(manager, i))
#else
		bdd_entailed(manager,
				dd,
				Cudd_bddIthVar(manager, 2*i))
#endif
		){
			//      cout << "subbed +"<< i<<endl;
			got_one = true;
			returnDD = bdd_and(manager, fr=returnDD,
					gft_table[i]->info_at[k]->label->label);
			Cudd_RecursiveDeref(manager,fr);
		}
		else if(gft_table[NEG_ADR(i)] &&
				gft_table[NEG_ADR(i)]->info_at[k] &&
				!gft_table[NEG_ADR(i)]->info_at[k]->is_dummy &&
				gft_table[NEG_ADR(i)]->info_at[k]->label &&
#ifdef PPDDL_PARSER
				bdd_entailed(manager,
						dd,
						Cudd_Not(Cudd_bddIthVar(manager, i)))
#else
		bdd_entailed(manager,
				dd,
				Cudd_Not(Cudd_bddIthVar(manager, 2*i)))
#endif
		){
			//   cout << "subbed -"<< i<<endl;
			got_one = true;
			returnDD = bdd_and(manager, fr=returnDD,
					gft_table[NEG_ADR(i)]->info_at[k]->label->label);
			Cudd_RecursiveDeref(manager,fr);
		}
#ifdef PPDDL_PARSER
		else if(bdd_entailed(manager, dd, Cudd_bddIthVar(manager, 2*i)) ||
				bdd_entailed(manager, dd, Cudd_Not(Cudd_bddIthVar(manager, 2*i)))) 
#else
			else if(bdd_entailed(manager, dd, Cudd_bddIthVar(manager, i)) ||
					bdd_entailed(manager, dd, Cudd_Not(Cudd_bddIthVar(manager, i))))
#endif
			{
				Cudd_RecursiveDeref(manager,returnDD);
				return Cudd_ReadLogicZero(manager);
			}


	}
	//printBDD(returnDD);

	if(got_one)
		return returnDD;
	else
	{
		Cudd_RecursiveDeref(manager,returnDD);
		return Cudd_ReadLogicZero(manager);
	}

}




RelaxedPlan* addSupport(// RelaxedPlan* rp,
		DdNode* facts, DdNode* worlds, int k){
	//TREATING FACTS as a conjunction -- needs generalization

	if(k == 0)
		return 0;

	//    cout << "Adding support for:"<<endl;
	//    printBDD(facts);
	//    cout << "In worlds: " <<endl;
	//    printBDD(worlds);

	//    clausalState* state = new clausalState();

	//   //make state out of sensor preconds
	//    for(int i = 0; i < num_alt_facts; i++){
	//      if(bdd_entailed(manager, facts, Cudd_bddIthVar(manager, i)))
	//        state->addClause(new simple_goal(new proposition(alt_facts[i]), E_POS));
	//      else if(bdd_entailed(manager, facts, Cudd_Not(Cudd_bddIthVar(manager, i))))
	//        state->addClause(new neg_goal(new simple_goal(new proposition(alt_facts[i]), E_NEG)));
	//    }

	//support state
	//   state->display();
	RelaxedPlan* tmp_rp = getLabelRelaxedPlan(facts, k, worlds);
	// tmp_rp->display();


	//   //insert into rp
	//    for(int i = 0; i < tmp_rp->plan_length; i++){
	//      std::set_union(rp->action_levels[i]->begin(),
	//          rp->action_levels[i]->end(),
	//          tmp_rp->action_levels[i]->begin(),
	//          tmp_rp->action_levels[i]->end(),
	//          inserter(*(rp->action_levels[i]),
	//       rp->action_levels[i]->begin()));
	//       unionElementLabels(rp->action_levels[i]);


	//    }
	//   delete state;
	return tmp_rp;

}



int supportCost(int time, DdNode* f, DdNode *w){
	double cost = 0.0;
	DdNode* tmpdd;
	//   cout << "enter support cost "<<endl;

	if(RP_EFFECT_SELECTION == RP_E_S_COVERAGE)
		return cost;

	for(int i = 0; i < num_alt_facts; i++){
#ifdef PPDDL_PARSER
		tmpdd = Cudd_bddIthVar(manager, 2*i);
#else
		tmpdd = Cudd_bddIthVar(manager, i);
#endif
		if(bdd_entailed(manager, f, tmpdd))
			cost += getFactWorldCost(time, i, w );
		else if(bdd_entailed(manager, f, Cudd_Not(tmpdd)))
			cost += getFactWorldCost(time, i+num_alt_facts, w );
	}
	//cout << "support cost = "<<cost<<endl;
	return cost;
}

double getPartCost(RelaxedPlan* rp,
		std::list<LabelledFormula*>* parts,
		std::list<LabelledFormula*>::iterator part,
		DdNode* world, int k){
	//what will cost of parts be if world is added to part?
	//measured as expected cost of parts with the part containing world
	//  cout << "enter part cost"<<endl;
	DdNode* hyp_part = bdd_or(manager, (*part)->label, world), *tmp_part;
	double returnVal = 0.0;
	double partVal = 0.0;

	Cudd_Ref(hyp_part);
	for(std::list<LabelledFormula*>::iterator p = parts->begin();
			p!= parts->end();
			p++){
		partVal = 0.0;
		if((*p)->operator==(*part)){
			tmp_part = hyp_part;
		}
		else{
			tmp_part = (*p)->label;
		}
		Cudd_Ref(tmp_part);
		for(std::set<LabelledAction*>::iterator i = rp->action_levels[k]->begin();
				i != rp->action_levels[k]->end();
				i++){
			if(((OpNode*)(*i)->elt)->alt_index < num_alt_acts &&
					!bdd_is_one(manager, Cudd_Not(bdd_and(manager, tmp_part, (*i)->label)))){
				//  cout << "act = " << ((OpNode*)(*i)->elt)->alt_index << endl;
				partVal+=alt_act_costs[((OpNode*)(*i)->elt)->alt_index];
			}
		}
		//   cout << "pv = " << partVal << " size = " << parts->size()<<endl;
		returnVal += partVal/parts->size();
	}
	// cout << "hyp_part cost = "<<returnVal<<endl;
	return returnVal;
}
int reducesCost(RelaxedPlan* tmp_rp,
		RelaxedPlan* rp,
		list<DdNode*>* tmp_parts,
		int k){
	double increase, decrease, rpcost;
	list<DdNode*>::iterator j;

	list<DdNode*>* oldPart = rp->partitions[k];
	//   cout << "in Reduce?"<<endl;



	for(int i = k+1; i < rp->plan_length; i++)
		rp->partitions[i] = rp->partitions[k];

	rpcost = rp->getExpectedCost();


	//get increase in support cost through adding tmp_rp to rp
	//requires finding the cost of tmp_rp wrt the partitions in rp
	for(int i = 0; i < k; i++){
		if(rp->partitions[i]){
			tmp_rp->partitions[i] = rp->partitions[i];
		}
	}
	increase = tmp_rp->getExpectedCost();


	//get decrease in support cost through adding tmp_parts to rp at k

	rp->partitions[k] = new list<DdNode*>();
	if(oldPart){
		for(j = oldPart->begin(); j != oldPart->end(); j++){
			//Cudd_Ref(*j);
			rp->partitions[k]->push_back(*j);
		}
	}
	for(j = tmp_parts->begin(); j != tmp_parts->end(); j++){
		//Cudd_Ref(*j);
		rp->partitions[k]->push_back(*j);
	}
	for(int i = k+1; i < rp->plan_length; i++)
		rp->partitions[i] = rp->partitions[k];
	// Cudd_CheckKeys(manager);


	decrease = rpcost - rp->getExpectedCost();
	for(int i = k+1; i < rp->plan_length;i++)
		rp->partitions[i] = NULL;

	//return if rp cost is greater than increase + decrease + rp cost
	if(rpcost > (increase-decrease+rpcost)){
		//         cout << "REDUCED"<<endl;
		return 1;
	}
	else{
		//  cout << "INCREASED"<<endl;

		delete rp->partitions[k];
		rp->partitions[k] = oldPart;
		return 0;
	}

}

#ifdef PPDDL_PARSER
double getSensoryLabelRPHeuristic(int max_lev){
	return 0.0;
}
#else
void dumb_partition(std::list<alt_action*>*sensors,
		std::list<std::list<LabelledFormula*>*>* partitions,
		RelaxedPlan* rp,
		int k){

	//find subset of sensors and partitions for my_rp at level k

	std::list<DdNode*> new_parts, parted_parts, supported_parts, unparted_parts;// = new std::list<DdNode*>();
	std::list<DdNode*>* tmp_parts;

	std::list<LabelledFormula*> *divided_parts;
	std::list<DdNode*>::iterator p, p1, tsp;
	std::list<LabelledElement*>::iterator  tp1;
	std::list<LabelledFormula*>::iterator tp,dp;
	std::list<alt_action*>::iterator s;
	std::list<std::list<LabelledFormula*>*>::iterator sp;
	DdNode* tmpDD, *sensorLab;
	DdNode** tmpDNF, **tmpTerm;
	int ip, itp, in_list, newPartFound;
	double part_cost, min_part_cost;
	LabelledFormula* min_part=NULL;
	DdNode* intersecting;
	RelaxedPlan* tmp_rp, *tmp_rp1;

	//cout << "ENTER PART, time = " << k << "|sensors|= " << sensors->size() <<endl;

	tmp_parts = new std::list<DdNode*>();
	//intialize new_parts
	if(k == 0){
		//initial partition is initialDD
		new_parts.push_back(b_initial_state);//initialBDD);
	}
	else{
		//  cout <<"starting parting"<<endl;
		for(p = rp->partitions[k-1]->begin(); p != rp->partitions[k-1]->end(); p++){
			//    printBDD(*p);
			new_parts.push_back(*p);
		}
	}

	//try each sensor in given order
	sp = partitions->begin();
	for(s = sensors->begin(); s != sensors->end(); s++){

		//cout << "trying to partition with "<< (*s)->get_name() <<endl;
		for(p = new_parts.begin(); p != new_parts.end(); p++){

			//cout << "partitioning:" <<endl;
			//printBDD(*p);
			if(Cudd_CountMinterm(manager, *p, num_alt_facts) < 2){
				unparted_parts.push_back(*p);
				continue;
			}
			//      cout << "partitioning:" <<endl;
			//printBDD(*p);

			sensorLab = Cudd_ReadLogicZero(manager);

			//choose worlds to support sensor outcomes -- to make part
			//break out non-intersecting worlds of partitions

			intersecting = *p;//Cudd_ReadOne(manager);

			for(tp = (*sp)->begin(); tp != (*sp)->end(); tp++){
				intersecting = bdd_and(manager, intersecting, (*tp)->label);
			}
			Cudd_Ref(intersecting);
			//  cout << "outcomes intersetc:"<<endl;
			//       printBDD(intersecting);

			divided_parts = new std::list<LabelledFormula*>();
			for(tp = (*sp)->begin(); tp != (*sp)->end(); tp++){
				divided_parts->push_back(new LabelledFormula(((DdNode*)(*tp)->elt), bdd_and(manager, bdd_and(manager, *p, (*tp)->label), Cudd_Not(intersecting)), 0));
			}

			//get all possible worlds a part can take from intersecting
			if(!bdd_is_one(manager, Cudd_Not(intersecting))){
				for(tp = (*sp)->begin(); tp != (*sp)->end(); tp++){
					tmpDNF = extractDNFfromBDD(intersecting);
					ip = 0;
					while(tmpDNF[ip]){
						itp = 0;
						tmpTerm = extractTermsFromMinterm(tmpDNF[ip]);
						while(tmpTerm[itp]){
							//         cout << "adding to supported parts:"<<endl;
							//        printBDD(tmpTerm[itp]);
							//Cudd_Ref(tmpTerm[itp]);
							supported_parts.push_back(tmpTerm[itp]);
							itp++;
						}
						ip++;
						delete tmpTerm;
					}
					delete tmpDNF;
				}
			}

			//add worlds from supported parts to tmp_parts in order of least cost
			//cost is dependent on what actions will be introduced into the
			//partition if we add this world to it.
			for(tsp = supported_parts.begin();
					supported_parts.size() > 0 && tsp != supported_parts.end();
					tsp = supported_parts.begin()){
				//add to correct parttiiton
				//    cout << "picking side for world" <<endl;
				min_part_cost = INT_MAX;
				for(dp = divided_parts->begin(); dp != divided_parts->end(); dp++){
					part_cost = getPartCost(rp, divided_parts, dp, (*tsp), k) +
							supportCost(k, ((DdNode*)(*dp)->elt), bdd_or(manager, *tsp, (*dp)->label));
					//  cout << "YO " << part_cost<<endl;
					if(part_cost < min_part_cost){
						//      cout << "Set min part"<<endl;
						min_part_cost = part_cost;
						min_part = *dp;
						//printBDD((DdNode*)(min_part)->elt);
					}
				}

				(min_part)->label = bdd_or(manager, min_part->label, (*tsp));
				//    cout << "HO"<<endl;

				for(dp = divided_parts->begin(); dp != divided_parts->end(); dp++){
					if(!(*dp)->operator==(min_part)){
						(*dp)->label = bdd_and(manager, (*dp)->label, Cudd_Not(*tsp));
						Cudd_Ref((*dp)->label);
					}
				}
				supported_parts.remove(*tsp);
			}

			for(tp = divided_parts->begin(); tp != divided_parts->end(); tp++){
				if(!bdd_is_one(manager, Cudd_Not((*tp)->label))){
					tmp_parts->push_back((*tp)->label);
					sensorLab = bdd_or(manager, sensorLab, (*tp)->label);
					Cudd_Ref(sensorLab);
				}
			}


			tmp_parts->unique();
			// cout << "Got parts = , size = " << tmp_parts->size() << "//" << new_parts.size() << endl;

			//        for(p1 = tmp_parts->begin(); p1 != tmp_parts->end(); p1++){
			//     printBDD(*p1);
			//         }


			if(tmp_parts->size() > 1 ){//made a useful partition
				//    cout << "trying to add a part"<<endl;
				tmp_rp = new RelaxedPlan(k+1, 0);

				//  tmp_rp->action_levels[k]->insert(new LabelledAction((*s), sensorLab, 0));
				OpNode *tmp_op = new_op_node(k,(char*)(*s)->get_name(),FALSE,NULL,NULL,NULL,(*s)->index);
				//cout << tmp_op->name << " " << tmp_op->alt_index <<endl;
				tmp_rp->action_levels[k]->insert(new LabelledAction(tmp_op, sensorLab, 0));

				tmp_rp1=addSupport((*s)->get_effs()->b_pre, sensorLab, k);

				for(int i = 0; i < k && tmp_rp1; i++){
					//  tmp_rp1->display();
					//       std::set_union(tmp_rp->action_levels[i]->begin(),
					//        tmp_rp->action_levels[i]->end(),
					//          tmp_rp1->action_levels[i]->begin(),
					//          tmp_rp1->action_levels[i]->end(),
					//          inserter(*(tmp_rp->action_levels[i]),
					//             tmp_rp->action_levels[i]->begin()));
					set<LabelledAction*>::iterator rpit, rpit1;
					int found;
					for(rpit = tmp_rp1->action_levels[i]->begin();
							rpit != tmp_rp1->action_levels[i]->end(); ){
						found = FALSE;
						for(rpit1= tmp_rp->action_levels[i]->begin();
								rpit1 != tmp_rp->action_levels[i]->end(); rpit1++){
							if((*rpit) == (*rpit1)){
								found = TRUE;
								break;
							}
						}
						if(!found){
							tmp_rp1->action_levels[i]->erase(*rpit);
							tmp_rp->action_levels[i]->insert(*rpit);
							rpit = tmp_rp1->action_levels[i]->begin();
						}
						else
							rpit++;

					}
					unionElementLabels((set<LabelledAction*>*)tmp_rp->action_levels[i]);
				}
				//  tmp_rp->display();

				if(tmp_rp1)
					delete tmp_rp1;
				for(tp = divided_parts->begin(); tp != divided_parts->end(); tp++){

					tmp_rp1=addSupport(((DdNode*)(*tp)->elt), (*tp)->label , k);
					for(int i = 0; i < k && tmp_rp1; i++){
						//  tmp_rp1->display();
						//         std::set_union(tmp_rp->action_levels[i]->begin(),
						//            tmp_rp->action_levels[i]->end(),
						//            tmp_rp1->action_levels[i]->begin(),
						//            tmp_rp1->action_levels[i]->end(),
						//            inserter(*(tmp_rp->action_levels[i]),
						//               tmp_rp->action_levels[i]->begin()));
						set<LabelledAction*>::iterator rpit, rpit1;
						int found;
						for(rpit = tmp_rp1->action_levels[i]->begin();
								rpit != tmp_rp1->action_levels[i]->end(); ){
							found = FALSE;
							for(rpit1= tmp_rp->action_levels[i]->begin();
									rpit1 != tmp_rp->action_levels[i]->end(); rpit1++){
								if((*rpit) == (*rpit1)){
									found = TRUE;
									break;
								}
							}
							if(!found){
								tmp_rp1->action_levels[i]->erase(*rpit);
								tmp_rp->action_levels[i]->insert(*rpit);
								rpit = tmp_rp1->action_levels[i]->begin();
							}
							else
								rpit++;

						}
						unionElementLabels((set<LabelledAction*>*)tmp_rp->action_levels[i]);
					}
					if(tmp_rp1)
						delete tmp_rp1;

				}

				//    cout << "about to check reduce"<<endl;
				//insert sensor and its support if it reduces cost of rp
				//     Cudd_CheckKeys(manager);
				//   cout << flush;


				if((SRP_INSERT_METHOD == SRP_GREEDY_INSERT) ||

						(SRP_INSERT_METHOD == SRP_REDUCE_INSERT &&
								reducesCost(tmp_rp, rp, tmp_parts, k)) ||

								(SRP_INSERT_METHOD == SRP_MUTEX_INSERT &&
										rp->partitionBreaksMutex(tmp_parts,k)) ||

										(SRP_INSERT_METHOD == SRP_REDUCE_MUTEX_INSERT &&
												(rp->partitionBreaksMutex(tmp_parts,k) ||
														reducesCost(tmp_rp, rp, tmp_parts, k)))

				){

					//cout << endl<<"ADDED"<<endl<<flush;
					for(p1 = tmp_parts->begin(); p1 != tmp_parts->end(); p1++){

						// printBDD(*p1);
						parted_parts.push_back(*p1);
					}

					//    tmp_rp->display();
					//     rp->display();
					parted_parts.unique();
					for(int i = 0; i < tmp_rp->plan_length && tmp_rp; i++){
						//       std::set_union(rp->action_levels[i]->begin(),
						//          rp->action_levels[i]->end(),
						//          tmp_rp->action_levels[i]->begin(),
						//          tmp_rp->action_levels[i]->end(),
						//          inserter(*(rp->action_levels[i]),
						//             rp->action_levels[i]->begin()));
						set<LabelledAction*>::iterator rpit, rpit1;
						int found;
						for(rpit = tmp_rp->action_levels[i]->begin();
								rpit != tmp_rp->action_levels[i]->end(); ){
							found = FALSE;
							for(rpit1= rp->action_levels[i]->begin();
									rpit1 != rp->action_levels[i]->end(); rpit1++){
								if((*rpit) == *rpit1){
									found = TRUE;
									break;
								}
							}
							if(!found){
								tmp_rp->action_levels[i]->erase(*rpit);
								rp->action_levels[i]->insert(*rpit);
								rpit = tmp_rp->action_levels[i]->begin();
							}
							else
								rpit++;

						}


						unionElementLabels((set<LabelledAction*>*)rp->action_levels[i]);
					}


				}
				else{
					unparted_parts.push_back(*p);
					//cout << "NOT ADDED"<<endl;
				}


				if(tmp_rp){

					delete tmp_rp;
				}
			}
			else{
				unparted_parts.push_back(*p);
				//cout << "NOT ADDED"<<endl;
			}


			//       Cudd_CheckKeys(manager);
			//    cout << flush;

			tmp_parts->clear();

			delete divided_parts;

		}

		// cout << "finished sensor"<<endl<<flush;
		//    if(parted_parts.size()>0)
		//        cout << "ADDING PARTED PARTS TO NEW PARTS " << parted_parts.size() <<endl;

		new_parts.clear();
		for(p1 = parted_parts.begin(); p1 != parted_parts.end(); p1++){
			//       cout << "new part added"<<endl<<flush;
			//       printBDD(*p1);
			new_parts.push_back(*p1);
		}
		for(p1 = unparted_parts.begin(); p1 != unparted_parts.end(); p1++){
			//       cout << "old part added"<<endl;
			//       printBDD(*p1);
			new_parts.push_back(*p1);
		}
		new_parts.unique();
		parted_parts.clear();
		unparted_parts.clear();
		//}
		sp++;
	}
	// tmp_parts->clear();
	delete tmp_parts;

	//parted_parts.unique();

	if(rp->partitions[k])
		delete rp->partitions[k];
	rp->partitions[k] = new list<DdNode*>();
	rp->partitions[k]->merge(new_parts);
	//  cout << "Parts at " << k << " = " << rp->partitions[k]->size() <<endl<<flush;
	//    for(p = rp->partitions[k]->begin(); p != rp->partitions[k]->end(); p++){
	//      printBDD(*p);
	//    }
	// Cudd_CheckKeys(manager);
}


void insertSensors(action_list acts, RelaxedPlan* rp, int start_lev, int end_lev){

	//find mutexes in rp

	//find sensors to break mutexes

	//-----or-------

	//find min cost partition of rp

	DdNode* tmpPart, *tmpDD;
	std::set<LabelledElement*>::iterator j;
	std::list<LabelledFormula*>* tmpPartitions;
	std::list<LabelledElement*>::iterator p;
	std::list<std::list<LabelledFormula*>*>::iterator k;
	std::list<std::list<LabelledFormula*>*>* partitions = new std::list<std::list<LabelledFormula*>*>();
	std::list<alt_action*>* sensors = new std::list<alt_action*>();
	int min;

	if(end_lev < rp->plan_length)
		min = end_lev;
	else
		min =  rp->plan_length;


	rp->computeStaticMutexes();

	for(int i = start_lev; i < min; i++){
		if(!rp->action_levels[i])
			continue;




		//      cout << "Want to partition: at: " << i << endl;
		//      j = my_rp->action_levels[i]->begin();
		//          for(;j != my_rp->action_levels[i]->end(); ++j){
		//        cout << ((OpNode*)(*j)->elt)->name << endl;
		//        printBDD((*j)->label);
		//          }
		//     cout << "Can use:"<<endl;

		//find useable sensors
		for(action_list a = acts; a; a = a->next){

			if(a->act->get_effs()->obs){

				tmpPartitions = new std::list<LabelledFormula*>();

				for(alt_effect* ef = a->act->get_effs(); ef; ef=ef->next){

					tmpDD = labelBDD(b_initial_state, ef->b_pre, i); //tmpDD = bdd_and(manager, substitute_label(ef->b_pre, i), b_initial_state );

					//Cudd_Ref(tmpDD);
					//cout << a->act->get_name() << " " << ef->obs << endl;
					//    cout << "sensor's pre worlds:"<<endl;
					//    printBDD(tmpDD);
					if(ef->obs && (bdd_is_one(manager, ef->b_pre) || !bdd_is_one(manager, Cudd_Not(tmpDD)))){
						//cout << a->act->get_name() << endl;
						tmpPart =  labelBDD(b_initial_state, ef->b_eff, i);//bdd_and(manager, substitute_label(ef->b_eff, i), b_initial_state );
						//      Cudd_Ref(tmpPart);
						if(!bdd_is_one(manager, Cudd_Not(tmpPart))){
							tmpPartitions->push_back(new LabelledFormula(ef->b_eff , tmpPart, 0));
						}
					}
					Cudd_RecursiveDeref(manager, tmpDD);
				}
				if(tmpPartitions->size() > 1){
					partitions->push_back(tmpPartitions);
					sensors->push_back(a->act);
					//        cout << a->act->get_name() << endl;
					//        for(p = tmpPartitions->begin(); p != tmpPartitions->end(); p++){
					//          printBDD(((DdNode*)(*p)->elt));
					//     printBDD((*p)->label);
					//         }
				}
				else{
					delete tmpPartitions;
					//tmpPartitions->clear();
				}
			}
		}

		//determine partitions
		dumb_partition(sensors, partitions, rp, i);

		sensors->clear();

		for(k = partitions->begin(); k != partitions->end(); k++)
			delete *k;

		partitions->clear();

	}
	delete partitions;
	delete sensors;
}


// void updateLUGCosts(RelaxedPlan *rp){
//   DdNode* tmp;
//   for(int i = 0; i < rp->max_lev; i++){

//     //set act costs
//     for(std::set<LabelledElement*>::iterator j = rp->action_levels[i]->begin();
//   j != rp->action_levels[i]->end(); j++){
//       for(std::list<LabelledElement*>::iterator k = rp->partitions[i]->begin();
//     k !=  rp->partitions[i]->end(); k++){
//   tmp = bdd_and(manager, (*k)->label, (*j)->label);
//   Cudd_Ref(tmp);
//   if(!bdd_is_one(manger, Cudd_Not(tmp))){//action is in partition
//     sactWorldCost[((OpNode*)(*j)->elt)->index]->push_back(new LabelledElement(tmp, 1.0/(double)rp->partitions[i]->size()));
//     for(int s = 0; s < num_alt_acts; s++){
//       if(get_bit(((BitVector*)(*k)->elt), (int)num_alt_acts/gcword_size, s))
//         set_bit(((OpNode*)(*j)->elt)->info_at[i]->sensor_dependencies, s);
//     }
//   }
//       }
//     }

//     //set fact costs

//   }
// }



double getSensoryLabelRPHeuristic(int max_lev){
	// cout << "getting rp"<<endl;
	my_rp = getLabelRelaxedPlan(b_goal_state, max_lev, b_initial_state);
	if(!my_rp)
		return IPP_MAX_PLAN;
	double return_val = 0.0;
	double conf_val =  my_rp->getRelaxedConformantNumActions();
	std::list<DdNode*>::iterator j;
//	   my_rp->display();
	int extend_levels = //
			FALSE;//    TRUE;

	//   cout << "inserting sensors" <<endl;
	if(extend_levels){

		RelaxedPlan* tmp_rp = new RelaxedPlan(2*max_lev, 0);
		for(int i = 0; i < max_lev; i++){
			tmp_rp->partitions[i] = new std::list<DdNode*>();
			tmp_rp->partitions[i]->push_back(b_initial_state);
			tmp_rp->action_levels[i+max_lev] = my_rp->action_levels[i];
			tmp_rp->b_goal_levels[i+max_lev] = my_rp->b_goal_levels[i];
		}
		insertSensors(available_acts, tmp_rp, max_lev-1, max_lev);
		for(int i = max_lev; i < 2*max_lev; i++){
			tmp_rp->partitions[i] = new std::list<DdNode*>();
			for(j = tmp_rp->partitions[max_lev-1]->begin(); j != tmp_rp->partitions[max_lev-1]->end(); j++)
				tmp_rp->partitions[i]->push_back(*j);
		}
		// tmp_rp->display();
		return_val = tmp_rp->getExpectedCost();
		// return_val =  my_rp->getRelaxedConformantNumActions();
	}
	else{
		insertSensors(available_acts, my_rp, 0, graph_levels);//max_lev);
//		 my_rp->display();
		return_val = my_rp->getExpectedCost();
		// return_val =  my_rp->getRelaxedConformantNumActions();
	}


//	 cout << "GOT RP = "  << return_val <<endl << "*********************"<<endl;
//	printBDD(b_initial_state);
//	 my_rp->display();


	//  updateLUGCosts(my_rp);

	if(TAKEMINRP)
		return_val = min(conf_val, return_val);


	if(return_val == 0)
		return_val = IPP_MAX_PLAN;


	delete my_rp;


	return return_val;
}
#endif

int maximum(int a, int b){
	if(a > b)
		return a;
	else
		return b;
}

double getLabelRPHeuristic(StateNode* s, int max_lev, list<DdNode*>* worldsAtLevels){
	DdNode* rpWorlds, *tmp;
	//cout << "State " << s->StateNo << endl;
	//printBDD(s->dd);
	// abstract away state information so that only get particles
	if(PF_LUG && LUG_FOR != SPACE){// conformant do not enter
		if(RP_EFFECT_SELECTION == RP_E_S_COVERAGE || goal_threshold == 0.0){
			DdNode* c;
			if(my_problem->domain().requirements.probabilistic)
				c = Cudd_addBddStrictThreshold(manager, current_state_cube, 0.0);
			else
				c = current_state_cube;

			Cudd_Ref(c);
			DdNode *t = Cudd_bddExistAbstract(manager, s->dd, c);
			Cudd_Ref(t);
			Cudd_RecursiveDeref(manager, c);
			rpWorlds = t;
			Cudd_Ref(rpWorlds);
			Cudd_RecursiveDeref(manager, t);
		}
		else if(RP_EFFECT_SELECTION != RP_E_S_COVERAGE){
//			// find a subset of the particles in samples that reach
//			// goal that will support with enough probability and do
//			// so with minimal cost
//			// rpWorlds = removeWorldsToImproveCost(goal_threshold, graph_levels);
//			rpWorlds = addWorldsUntilCostIncreases(goal_threshold, graph_levels);
//			Cudd_Ref(rpWorlds);
		}
	}
	// conformant planning
	else{
		if(LUG_LEVEL_WORLDS == 0 || (LUG_LEVEL_WORLDS != 0 && max_lev <= LUG_LEVEL_WORLDS)){  // get all worlds
			rpWorlds = s->dd;
			Cudd_Ref(rpWorlds);
		}
		else{ // get for worlds that reach goals at last LUG_LEVEL_WORLDS levels
			int i = 0;
			int start_lev = max_lev - LUG_LEVEL_WORLDS + 1;
			rpWorlds = Cudd_ReadLogicZero(manager);
			Cudd_Ref(rpWorlds);

			for(list<DdNode*>::iterator w = worldsAtLevels->begin(); w != worldsAtLevels->end(); w++){
				if(i >= start_lev){
					tmp = Cudd_bddOr(manager, rpWorlds, *w);
					Cudd_Ref(tmp);
					Cudd_RecursiveDeref(manager, rpWorlds);
					rpWorlds = tmp;
					Cudd_Ref(rpWorlds);
					Cudd_RecursiveDeref(manager, tmp);
				}
				i++;
			}
		}
	}

	currentNode = s;
	my_rp = getLabelRelaxedPlan(b_goal_state, max_lev,  rpWorlds);
	Cudd_RecursiveDeref(manager, rpWorlds);



	  //cout << "State " << s->StateNo << endl;
	if(!my_rp){
		//cout << "NO RELAXED PLAN!\n";
		return IPP_MAX_PLAN;
	}
	//else{
	//  cout << "State " << s->StateNo << endl;
	//  printBDD(s->dd); 
	//  my_rp->display();
	//	cout << "SUCCESS -- Found a relaxed plan!\n";
	//}
	double return_val;

	return_val = my_rp->getRelaxedConformantCost();
	//	cout << return_val << endl;
	my_rp->unsetEffectRPFlags();

	if(my_rp && !HELPFUL_ACTS && !DO_INCREMENTAL_HEURISTIC)
		delete my_rp;

	if(DO_INCREMENTAL_HEURISTIC)
		s->currentRP = my_rp;

	return return_val;
}

int getRPHeuristic(int max_lev){
//	   cout << "Getting RP"<<endl;
	my_rp = getRelaxedPlan(max_lev, b_goal_state, FALSE, Cudd_ReadOne(manager));
	if(!my_rp)
		return IPP_MAX_PLAN;
	int return_val =  my_rp->getRelaxedConformantNumActions();
	if(return_val == 0)
		return_val = IPP_MAX_PLAN;
//	    cout << "GOT RP = "  << return_val <<endl;
//	my_rp->display();
	my_rp->unsetEffectRPFlags();
	return return_val;
}


void free_my_info(int j ){

	//  cout << "free_my_info"<<endl;
	//    Cudd_CheckKeys(manager);
	//    cout << "|"<<endl;
	//     cout << "num acts = " << num_alt_acts<<endl;

	//   cout << "[" <<endl;
	//   Cudd_CheckKeys(manager);
	//   cout << "|" <<endl;



	if(COMPUTE_LABELS){
		std::list<list<LabelledElement*>*>::iterator k;
		list<LabelledElement*>::iterator p;
		if(RP_EFFECT_SELECTION != RP_E_S_COVERAGE){
			for(int i = 0; i < (num_alt_acts+(num_alt_facts*2)); i++){
				//      cout << "i = " << i << endl;
				if(!COST_REPROP){
					for(k = actWorldCost[i]->begin(); k != actWorldCost[i]->end(); k++){
						for(p = (*k)->begin(); p != (*k)->end(); p++){
							delete *p;
						}
						(*k)->clear();
						delete (*k);
					}
					actWorldCost[i]->clear();
				}
			}
			//  cout <<"HO"<<endl;
			for(int i = 0; i < (num_alt_effs+(num_alt_facts*2)); i++){
				if(!COST_REPROP){
					for(k = effectWorldCost[i]->begin(); k != effectWorldCost[i]->end(); k++){
						for(p = (*k)->begin(); p != (*k)->end(); p++)
							delete *p;
						(*k)->clear();
						delete (*k);
					}
					effectWorldCost[i]->clear();
				}

			}
		}
		for(int i = 0; i < num_alt_facts; i++){
			if(!COST_REPROP && RP_EFFECT_SELECTION != RP_E_S_COVERAGE){
				for(k = factWorldCost[i]->begin(); k != factWorldCost[i]->end(); k++){
					for(p = (*k)->begin(); p != (*k)->end(); p++)
						delete *p;
					(*k)->clear();
					delete (*k);
				}
				factWorldCost[i]->clear();
				for(k = factWorldCost[i+num_alt_facts]->begin(); k != factWorldCost[i+num_alt_facts]->end(); k++){
					for(p = (*k)->begin(); p != (*k)->end(); p++)
						delete *p;
					(*k)->clear();
					delete (*k);
				}
				factWorldCost[i+num_alt_facts]->clear();
			}



			//delete factWorldCost[i+num_alt_facts];
			Cudd_RecursiveDeref(manager, init_pos_labels[i]);
			init_pos_labels[i] = NULL;
			Cudd_RecursiveDeref(manager, init_neg_labels[i]);
			init_neg_labels[i] = NULL;

		}


	}
	else if(HEURISTYPE==CORRRP){
		for(int i = 0; i < num_alt_facts; i++){
			Cudd_RecursiveDeref(manager, init_pos_labels[i]);
			init_pos_labels[i] = NULL;
			Cudd_RecursiveDeref(manager, init_neg_labels[i]);
			init_neg_labels[i] = NULL;
		}
	}
	goal_sample_costs.clear();

	NDLcounter = 0;

#ifdef PPDDL_PARSER

	//this should be derefed but causes problems if it is
	//Cudd_RecursiveDeref(manager, probabilisticLabels);

	list<list<int>* >::iterator v = level_vars->begin();
	for(;v != level_vars->end(); v++)
		delete *v;
	//delete level_vars;
	level_vars->clear();
#endif

	//       Cudd_CheckKeys(manager);


	if(HEURISTYPE==CORRRP){
		freeCorrelation();
	}
	//      cout << "["<<endl;

	// free_graph_and_search_info();
	free_graph_info(j);
	new_plan = TRUE;
	//      cout << "]"<<endl;
	//  cuddGarbageCollect(manager, 0);


	// cout << "EXIT FREE" <<endl;

	//   Cudd_CheckKeys(manager);
	//   cout << "]" <<endl;
}
// direct return false, if contain bug.
bool findGraphForStates(StateNode* parent, DdNode* init){
	return false;
}

double build_forward_get_h(DdNode* init,
		list<StateNode*>* states,
		StateNode* parent){
	//             cout << "INTO Heuristics"<<endl;
	//cout << "" <<flush;
	//  cout << "Hit Enter for next LUG" <<endl;
	//   string c;
	//   cin >> c;
	//     Cudd_CheckKeys(manager);
	//       cout << "asdfHI" << endl;

	BitOperator *tmp;
	Effect* tmpe, *tmpe1;
	int okay;
	int num_ops = 0;
	BitVector* helpful_acts;
	DdNode* lastWorlds;
	list<DdNode*> worldsAtLevels;
	int j = 0;
	double h;
	int reached_goals = 0;// if reach the goal

	//     printPlan(parent);
	//     if(states && !states->empty() && states->front()->PrevAction)
	//       printAction(states->front()->PrevAction->Node);
	// findGraphForState current directly return false
	if(LUG_FOR != SPACE || !states || (LUG_FOR == AHREACHABLE && !findGraphForStates(parent, init)))
	{
		if(!init)
			return IPP_MAX_PLAN;
		//printBDD(init);
		num_lugs++;
		//first incremental lug or not doing incremental lug
		if((LUG_FOR == INCREMENTAL && num_lugs == 1) || LUG_FOR != INCREMENTAL)
		{
			if(LUG_FOR == INCREMENTAL) cout << "FIRST INCREMENTAL LUG"<<endl;

			createInitLayer(init);// 创建了FactInfoPair
			// cout << "RESTORING OPERATORS" <<endl;

			while(used_gbit_operators){
				tmp = used_gbit_operators;
				used_gbit_operators = used_gbit_operators->next;
				tmpe = tmp->conditionals;
				while( tmp->activated_conditionals){
					tmpe = tmp->activated_conditionals;
					//      cout << "tmpe rp = " << tmpe->in_rp <<endl;
					tmp->activated_conditionals = tmp->activated_conditionals->next;
					okay = TRUE;
					tmpe1 = tmp->conditionals;
					while(tmpe1){
						if(tmpe1->index == tmpe->index){
							okay = FALSE;
						}
						tmpe1 = tmpe1->next;
					}
					if(okay){
						tmpe->next = tmp->conditionals;
						tmp->conditionals = tmpe;
					}
				}
				tmp->next = gbit_operators;
				gbit_operators = tmp;
			}// end-outer while
			// compute the num_ops
			tmp = gbit_operators;
			while(tmp){
				tmpe = tmp->conditionals;
				while(tmpe){
					tmpe=tmpe->next;
				}
				tmp = tmp->next;
				num_ops++;
			}
			// set the current gbit_goal_state from initLUG
			gbit_goal_state = my_gbit_goal_state;
			

			if(HEURISTYPE==CORRRP){
				Cudd_RecursiveDeref(manager, initialBDD);
				initialBDD = states->front()->dd;
				Cudd_Ref(initialBDD);
			}

			reached_goals = build_graph(&j, num_alt_facts, ALLOW_LEVEL_OFF, MUTEX_SCHEME );
			
			// display the planning graph info
			// cout << "YO HERES THE GRAPH:" << reached_goals <<endl;
			// for(int i = 0; i <= j; i++)
			// {
			// 	cout << "LEVEL " << i << endl;
			// 	print_BitVector(gpos_facts_vector_at[i],gft_vector_length);
			// 	print_BitVector(gneg_facts_vector_at[i],gft_vector_length);
			// 	cout << endl;
			// 	for(OpNode* k = gall_ops_pointer; (k) ; k=k->next)
			// 		if(k->info_at[i] && k->name && !k->is_noop )cout << k->name << endl;
			// 			cout << endl<< "-------------------------------------------------" <<endl;
			// }// end-for
			//    cout << "got " <<  j << endl;
			graph_levels = j;
			for(int i=0; i< num_alt_facts; ++i)
			{
				pos_fact_at_min_level[i] = getlevel(i, TRUE);
				neg_fact_at_min_level[i] = getlevel(i, FALSE);
			}

			if(LUG_FOR == SPACE)
			{
				cout << "Global LUG has " << graph_levels << " layers"<<endl;
				fflush(stdout);
				// Cudd_CheckKeys(manager);

				// printBDD(init);
				if(USE_GLOBAL_RP)
					globalRP = getLabelRelaxedPlan(b_goal_state, graph_levels-1, init);
				//cout << "HI" << endl;
				//globalRP->display();
			}

		}//end-if
		else //later incremental lug
		{ 
			cout << "LATER INCREMENTAL LUG"<<endl;

			//int LOOKAHEAD_FOR_STATES = 0;

			if(LOOKAHEAD_FOR_STATES >= 0)
			{
				// cout << "parent->dd:"<<endl;
				// printBDD(parent->dd);
				DdNode* dan;
				dan = statesAtLevel( parent->dd, (LOOKAHEAD_FOR_STATES < graph_levels ? LOOKAHEAD_FOR_STATES+1 :  graph_levels));
				Cudd_Ref(dan);
				// cout << "dan = " << endl;
				// printBDD(dan);


				// cout << "build lug for additional states:"<<endl;
				DdNode *dan1 = Cudd_bddOr(manager, dan, init);
				Cudd_Ref(dan1);
				Cudd_RecursiveDeref(manager, dan);
				init = dan1;
				Cudd_Ref(init);
				Cudd_RecursiveDeref(manager, dan1);
			}
			graph_levels = add_samples(init);
		}

		// Cudd_ReduceHeap(manager, CUDD_REORDER_WINDOW2, num_alt_facts);
		// if(LUG_FOR == NODE && !reached_goals )
		// {
		//     cout << "Goals not reached in PG"<<endl;
		//     my_rp = NULL;
		//     free_my_info(j);
		//     h = 999999999.9;
		//     return h;
		// }
	}

	//   cout << "HPO"<<endl;

	//  b_initial_state = 0;  // added, -Will

	if(!states){
		return 0.0;
	}
	for(list<StateNode*>::iterator i = states->begin();
			i != states->end(); i++){
		// cout << "get h for " << (*i)->StateNo<<endl;
		// Cudd_CheckKeys(manager);
		// cout << "|"<<endl;
		// if(b_initial_state)
		Cudd_RecursiveDeref(manager, b_initial_state);
		b_initial_state = (*i)->dd;
		Cudd_Ref(b_initial_state);

		// cout << "get rp for " <<  endl;
		// printBDD(b_initial_state);

		// printf("reached_goals = %d\n",reached_goals);
		if(!reached_goals && LUG_FOR == NODE){
			assert(0);
			cout << "goals not reached" <<endl;
			(*i)->h = DBL_MAX;
			(*i)->currentRP = NULL;
			continue;
		}


		//   cout << ",|" << flush;
		if((*i)->h == 999999999.9){
			//  cout <<"inf"<<endl;
			continue;
		}


		if(COMPUTE_LABELS){// LUG graph

			if(HEURISTYPE == SLUGRP){

				j = getLabelLevel((*i)->dd, &worldsAtLevels, parent);
				//  cout << "LEVEL = " << j << endl;
				h = getSensoryLabelRPHeuristic(j);
			}
			else if(HEURISTYPE == LUGRP){// LUG + RP
				if(USE_GLOBAL_RP){//global rp
					//     if((*i)->PrevAction){
					// //       cout << "PREV: "
					// //      << (*i)->PrevAction->Node->ActionNo
					// //      << " " << (*i)->StateNo
					// //      <<endl;
					// //      printAction((*i)->PrevAction->Node);
					//     }
					//     else
					// cout << "PREV is null" << endl;
					h = globalRP->restrictRP((*i)->dd);
				}
				else{
					//cout << "get level"<<endl;
					if(LUG_FOR == NODE )
						j = graph_levels;
					else if(RBPF_LUG) {
					  if(parent)
					    j = min(max_horizon - parent->horizon+1, graph_levels);
					  else
					    j = min(max_horizon, graph_levels);
					}
					else{
						j = getLabelLevel((*i)->dd, &worldsAtLevels, parent);
					}
					if(COST_REPROP){
						setInitCostVector((*i)->dd);
						for(int i = 0; i < j; i++){
							cost_prop_ops(i);
							cost_prop_efs(i);
							cost_prop_facts(i+1);
						}
					}
					//					cout << "LEVEL = " << j << "/" << graph_levels << endl << flush;
					if(j > 0 && j <= graph_levels){
						h = getLabelRPHeuristic(*i, j, &worldsAtLevels);
					}
					else  if (j == 0)
						h = 0;
					else{ //if(j == DBL_MAX)
						h = 99999999;//DBL_MAX;
						printBDD((*i)->dd);
						cout << "level bogus " << j  << endl;
						exit(0);
					}
				}
			}
			else if(HEURISTYPE == LUGLEVEL){// LUG + Level
				h = getLabelLevel((*i)->dd, &worldsAtLevels, parent);

			}
			else if(HEURISTYPE == LUGSUM){// LUG + Sum
				h = getLabelSum();
			}
			else if(HEURISTYPE == LUGMAX){// LUG + MAX
				h = getLabelMax();
			}
			// cout << "H = " << h << endl;
			(*i)->h = h;
		}
		else{// Single graph

			if(HEURISTYPE == LUGRP){// SG + RP, 抽取action的个数

				// printBDD((*i)->dd);
				if(LUG_FOR == NODE)
					j = graph_levels;
				else
					j = getLabelLevel((*i)->dd, &worldsAtLevels, parent);

				// cout << "lev = " << j <<endl;
				if(j > 0)
					h =getLabelRPHeuristic((*i), j, &worldsAtLevels);// getRPHeuristic(j);
				else
					h = 0;

			}
			// 这里默认了没有mutex才可以这样处理
			else if(HEURISTYPE == LUGLEVEL){// SG + Level, 计算miniterm的最小不冲突level，不存在mutex时等于max情况。
				h = j;
			}
			else if(HEURISTYPE == LUGSUM){// SG + Sum, 计算CNF clause的距离之和
				h = getMinSumHueristicValue();// momo007 fix the bug of clause
			}
			else if(HEURISTYPE == LUGMAX){// SG + Max, 计算CNF clause的最大距离(目前只有一个clause,直接等于level)
				h = j;
			}
			else if(HEURISTYPE == CORRRP){
				cout << "GETTING CORRP HEURISTIC" << endl;
				h = relaxedPlanHeuristic();//graph_levels;
			}
			(*i)->h = h;
		}
		//cout << "#" << flush;

		for(list<DdNode*>::iterator w = worldsAtLevels.begin();
				w != worldsAtLevels.end(); w++)
			Cudd_RecursiveDeref(manager, *w);
		worldsAtLevels.clear();

		if(HELPFUL_ACTS && my_rp){
			helpful_acts = getHelpfulActs( my_rp);
			//      print_BitVector(helpful_acts, gop_vector_length);
			(*i)->ha = helpful_acts;
			if(my_rp && !DO_INCREMENTAL_HEURISTIC){
				//     cout << "DEL RP"<<endl;
				delete my_rp;
				my_rp = NULL;
				//  cout << "DDEL RP"<<endl;
			}
		}

		//    cout << "#" << flush;

		if(COST_REPROP)
			clearCostVectors();

		//cout << "|" << flush;
	}
	Cudd_RecursiveDeref(manager, b_initial_state);
	b_initial_state = initialBDD;
	Cudd_Ref(b_initial_state);

	// cout << "@" << flush;

	//after first lug, prune out actions not used
	if(num_lugs == 1){
		for(BitOperator* tmp = gbit_operators; tmp; tmp=tmp->next){
			if(tmp->valid == 0)
				continue;
			for(OpNode *tops = gall_ops_pointer; tops; tops=tops->next){
				if(tops->alt_index == tmp->alt_index &&
						!tops->info_at[graph_levels]){
					tmp->valid = 0;
					if(action_transitions.count(tmp->action) == 0){
						action_transitions.insert(std::make_pair(tmp->action, Cudd_ReadZero(manager)));
						// action_rewards.insert(std::make_pair(tmp->action, Cudd_ReadZero(manager)));
					}
				}
			}
		}
	}


	if(LUG_FOR != SPACE && LUG_FOR != INCREMENTAL)
		free_my_info(j);




//	if(PF_LUG){// && LUG_FOR != SPACE){
//		//      cout << "|all_samples| = " << all_samples.size() << endl
//		//     << "|samples| = " << samples.size()
//		//     << "|used_samples| = " << used_samples.size()
//		//    << endl;
//		all_samples.insert(all_samples.begin(), used_samples.begin(), used_samples.end());
//		used_samples.clear();
//		all_samples.insert(all_samples.begin(), samples.begin(), samples.end());
//		samples.clear();
//
//		/// all_samples.insert(all_samples.begin(), new_samples.begin(), new_//samples.end());
//		//    all_samples.push_front(samples.front());
//		//    all_samples.merge(samples);
//		//     cout << "|all_samples| = " << all_samples.size() << endl;
//	}

	//cout << "@" << flush;

	//    if(b_initial_state)
	//      Cudd_RecursiveDeref(manager, b_initial_state);

	// cout << "Mem After Free" <<endl;
	//   printMemInfo();


	cout << (*states->begin())->h  << " " << (*states->begin())->g << endl;
	return h;

}
/** 
 * momo007 该函数实现给定CNF，创建vector，存储每个clause的fact的pos/neg情况
 * 目前每个vector<pair<int,bool>> 对应一个clause
 */
void getCnfIndex(const StateFormula& formula, vector<vector<pair<int,bool> > > &vec, int clauseNum)
{
	if(typeid(formula) == typeid(Constant))
	{
		return;
	}
	const Atom *af = dynamic_cast<const Atom *>(&formula);
	if(af != NULL)
	{
		if(vec.size() < clauseNum)
		{
			vec.push_back(vector<pair<int, bool> >());
		}
		if(dynamic_atoms.find(state_variables[af]) != dynamic_atoms.end())
		{
			vec[clauseNum - 1].push_back(make_pair(state_variables[af], true));
		}
	}
	const Negation *nf = dynamic_cast<const Negation *>(&formula);
	if(nf != NULL)
	{
		if(vec.size() < clauseNum)
		{
			vec.push_back(vector<pair<int, bool> >());
		}
		getCnfIndex(nf->negand(), vec, clauseNum);
		return;
	}
	const Conjunction *cf = dynamic_cast<const Conjunction *>(&formula);
	if(cf != NULL)
	{
		for (size_t i = 0; i < cf->size(); i++){
			getCnfIndex(cf->conjunct(i), vec, i + 1);
		}
		return;
	}
	const Disjunction *df = dynamic_cast<const Disjunction *>(&formula);
	if(df != NULL)
	{
		for (size_t i = 0; i <  df->size();++i)
		{
			getCnfIndex(df->disjunct(i), vec, clauseNum);
		}
	}
}
/**
 * momo007 该函数实现打印cnf公式
 */
void printGoalCnf(const vector<vector<pair<int, bool> > > &index){

	for (int i = 0; i < index.size();++i)
	{
		if(i!=0)
			cout << "/\\";
		std::cout << "(";
		for (int j = 0; j < index[i].size(); ++j)
		{
			if(j != 0)
				std::cout << "\\/";
			if(index[i][j].second == false)
			{
				std::cout << "!";
			}
			
			dynamic_atoms[index[i][j].first]->print(std::cout,
													my_problem->domain().predicates(),
													my_problem->domain().functions(),
													my_problem->terms());
			
			std::cout << "\n";
		}
		std::cout << ")";
	}
}
/**
 * momo007 实现计算获取所有clause的最小总价值。
 */
int getMinSumHueristicValue() {

	int return_val = 0, cur_val;
	vector<vector<std::pair<int, bool> > > index;
	getCnfIndex(my_problem->goal(), index, 1);// 第一个clause
	printGoalCnf(index);
	for (int i = 0; i < index.size();++i)
	{
		cur_val = IPP_MAX_PLAN;// 初始化最大层次，计算clause的最小层
		for (int j = 0; j < index[i].size(); ++j)
		{
			if(index[i][j].second == true)
			{
				cur_val = min(cur_val, pos_fact_at_min_level[index[i][j].first]);
			}
			else
			{
				cur_val = min(cur_val, neg_fact_at_min_level[index[i][j].first]);
			}
		}
		if(cur_val == IPP_MAX_PLAN)
			return IPP_MAX_PLAN;
		return_val += cur_val;
	}
	printf("%d\n", return_val);
	return return_val;

	//  conj_goal* cg;
	//   simple_goal* sg;
	//   neg_goal* ng;
	//   disj_goal* dg;
	// // cout << "Getting PG Heur for " << endl;
	// // this->display();

	// // only one fact
	// if(sg = dynamic_cast<simple_goal*>(goal_state->getClauses()))
	// {
	// 	return_val = pos_fact_at_min_level[sg->getProp()->ground->getnum()]; //getlevel(sg->getProp()->ground, TRUE);
	// }
	// // only one neg fact
	// else if (ng = dynamic_cast<neg_goal *>(goal_state->getClauses()))
	// {
	// 	if(sg = dynamic_cast<simple_goal*>(ng)) {
	// 		return_val = neg_fact_at_min_level[sg->getProp()->ground->getnum()]; //getlevel(sg->getProp()->ground, FALSE);
	// 	}
	// }
	// // CNF
	// else if (cg = dynamic_cast<conj_goal*>(goal_state->getClauses()))
	// {
	// 	// get one clasise
	// 	pc_list<goal*>::iterator i = cg->getGoals()->begin();
	// 	// consider each clause
	// 	for(;i!=cg->getGoals()->end();++i)
	// 	{
	// 		//add up clauses
	//         // cout << "CHECKING " << endl;
	//     	// (*i)->display(1);
	//     	if(sg = dynamic_cast<simple_goal*>(*i))
	// 		{
	// 			return_val += pos_fact_at_min_level[sg->getProp()->ground->getnum()];//getlevel(sg->getProp()->ground, TRUE);
	// 		}
	//     	else if(ng = dynamic_cast<neg_goal*>(*i))
	// 		{
	//     		if(sg = dynamic_cast<simple_goal*>(ng))
	// 				return_val += neg_fact_at_min_level[sg->getProp()->ground->getnum()];//getlevel(sg->getProp()->ground, FALSE);
	//     	}
	// 		// several fact
	//     	else if(dg = dynamic_cast<disj_goal*>(*i))
	// 		{
	//     		int min_val = -1;
	//     		int tmp_lev;
	// 			// compute the min fact
	//     		pc_list<goal*>::iterator j = dg->getGoals()->begin();
	//     		for(;j!=dg->getGoals()->end();++j)
	// 			{
	// 				if(sg = dynamic_cast<simple_goal*>(*j))
	// 				{
	// 					tmp_lev = pos_fact_at_min_level[sg->getProp()->ground->getnum()];//getlevel(sg->getProp()->ground, TRUE);
	// 				if(min_val == -1 || tmp_lev < min_val)
	// 					min_val = tmp_lev;
	//    				}
	// 				else if (ng = dynamic_cast<neg_goal*>(*j))
	// 				{
	// 					if(sg = dynamic_cast<simple_goal*>(ng))
	// 					{
	// 						tmp_lev = neg_fact_at_min_level[sg->getProp()->ground->getnum()];//getlevel(sg->getProp()->ground, FALSE);
	// 						if(min_val == -1 || tmp_lev < min_val)
	// 							min_val = tmp_lev;
	// 					}
	// 				}
	// 			}
	// 			// add the min val for CNF
	// 			return_val += min_val;
	// 		}
	//    }
	//  }

	// cout << "Val = " << return_val << endl;

	return return_val;
}

double costOfGS(DdNode* s, int time){
	double cost, c1, worldCost;
	DdNode** minterms = extractDNFfromBDD(b_goal_state);
	int j = 0;
	cost = 99999999;


	while(minterms[j] != NULL){
		if(COST_PROP_LIT==SUM)
			worldCost = 0;
		else if(COST_PROP_LIT==MAXN){
			worldCost = -1;
		}

		for(int i = 0; i< num_alt_facts;i++){
#ifdef PPDDL_PARSER
if(bdd_entailed(manager, minterms[j], Cudd_bddIthVar(manager, 2*i))){
	c1 = getFactWorldCost(time, i, s);
}
else if(bdd_entailed(manager, minterms[j], Cudd_Not(Cudd_bddIthVar(manager, 2*i)))){
	c1 =  getFactWorldCost(time, i+num_alt_facts, s);
}
#else
	if(bdd_entailed(manager, minterms[j], Cudd_bddIthVar(manager, i))){
		c1 = getFactWorldCost(time, i, s);
	}
	else if(bdd_entailed(manager, minterms[j], Cudd_Not(Cudd_bddIthVar(manager, i)))){
		c1 =  getFactWorldCost(time, i+num_alt_facts, s);
	}
#endif
if(COST_PROP_LIT==SUM)
	worldCost += c1;
else if(COST_PROP_LIT==MAXN){
	if(c1 > worldCost)
		worldCost = c1;
}
		}
		if(worldCost < cost)
			cost = worldCost;
		j++;
	}
	delete minterms;
	return cost;
}

//extern DdNode* substituteLabels(int, clausalState*);

double getLabelLevel(DdNode* s, list<DdNode*>* worldsAtLevels, StateNode* parent ){
	// cout << "get level"<<endl;
	//   Cudd_CheckKeys(manager);
	//   cout << "]" << endl;

	if(0 && DO_INCREMENTAL_HEURISTIC)
		return graph_levels;



	DdNode*  currStates = s;
	Cudd_Ref(currStates);

	double max_pr;
	double first_acheived = getLabelProven(currStates, b_goal_state, worldsAtLevels, &max_pr);
	// cout << "first = " << first_acheived << endl;
	return min(first_acheived, (double)graph_levels);

	double best_cost_level = DBL_MAX;
	double best_cost, cost;
	best_cost = 100000;
	Cudd_RecursiveDeref(manager, currStates);

#ifdef PPDDL_PARSER

	if(my_problem->domain().requirements.probabilistic){
		//  cout << "max pr = " << max_pr <<  " thresh = " << goal_threshold<<endl;
		if(OPTIMIZE_REWARDS){//cost

			if(max_pr <= -1*goal_threshold)
				best_cost_level = first_acheived;
			else
				best_cost_level = DBL_MAX;
		}
		else if(!OPTIMIZE_REWARDS && OPTIMIZE_PROBABILITY){//probability
			best_cost_level = max_pr;
		}
	}
	else if(my_problem->domain().requirements.non_deterministic){
		best_cost_level = first_acheived;
	}

#else
	best_cost_level = first_acheived;
#endif


	for(int i =  first_acheived;
			(i < graph_levels && COMPUTE_LABELS &&
					RP_EFFECT_SELECTION != RP_E_S_COVERAGE); i++){
		cost = costOfGS(s,i);
		cout << "cost at " << i << " = " << cost <<endl;
		if(cost < best_cost){
			best_cost_level = i;
			best_cost = cost;
		}
	}
	//    cout << "first = " << first_acheived << "  best = " << best_cost_level << endl;

	return best_cost_level;

}

int clauseCost(){

}

int getLabelSum(){
	cout << "need to implement labelsum"<<endl;
	exit(0);
	//get clauses
	//get

	//   goal_list* gl_list = goal_state->getClauseList();
	//   goal_list::iterator i = gl_list->begin();
	//   int sum = 0;
	//   clausalState* tmpS;

	//   for(;i!= gl_list->end(); ++i){
	//     tmpS = new clausalState(*i);
	//     //   tmpS->display();
	//     sum +=  getLabelProven(tmpS);
	//   }

	//   return sum;

	return 0;
}
int getLabelMax(){
	cout << "need to implement labelmax"<<endl;
	exit(0);
	//  goal_list* gl_list = goal_state->getClauseList();
	//   goal_list::iterator i = gl_list->begin();
	//   int max = 0;
	//   clausalState* tmpS;
	//   int tmp;

	//   for(;i!= gl_list->end(); ++i){
	//     tmpS = new clausalState(*i);
	//     tmp = getLabelProven(tmpS);
	//     if(tmp > max)
	//       max = tmp;
	//   }

	//   return max;
	return 0;
}

pair<DdNode*, DdNode*>* extractAffects(pair<const Action* const, DdNode*> *a){
	DdNode* tmp, *fr;

	map<const Action*, pair<DdNode*, DdNode*>* >::iterator aff =
			action_affects.find(a->first);

	if(aff != action_affects.end()){
		return (*aff).second;
	}
	else if(a->second == Cudd_ReadZero(manager)){
		return new pair<DdNode*, DdNode*>(Cudd_ReadLogicZero(manager), Cudd_ReadLogicZero(manager));
	}
	else{
		DdNode* mypos = Cudd_ReadOne(manager);
		DdNode* myneg = Cudd_ReadOne(manager);
		list<DdNode*> cubes;
		list<double> values;
		DdNode *nextSt;

		if(my_problem->domain().requirements.probabilistic){
			nextSt = Cudd_addExistAbstract(manager, a->second,
					current_state_cube);
		}
		else if(my_problem->domain().requirements.non_deterministic){
			nextSt = Cudd_bddExistAbstract(manager, (a)->second,
					current_state_cube);
		}
		Cudd_Ref(nextSt);
		get_cubes(nextSt, &cubes, &values);

		Cudd_RecursiveDeref(manager,nextSt);

		for(list<DdNode*>::iterator c = cubes.begin();
				c != cubes.end(); c++){
			DdNode *b = *c;//Cudd_addBddStrictThreshold(manager, *c, 0.0);
			Cudd_Ref(b);
			for(int k = 0 ; k < num_alt_facts; k++){
				if(Cudd_bddIsVarEssential(manager, b, k*2+1,1)){
					fr = Cudd_bddIthVar(manager, k*2+1);
					Cudd_Ref(fr);
					tmp = Cudd_bddAnd(manager, mypos, fr);
					Cudd_RecursiveDeref(manager,fr);

					Cudd_Ref(tmp);
					Cudd_RecursiveDeref(manager, mypos);
					mypos = tmp;
					Cudd_Ref(mypos);
					Cudd_RecursiveDeref(manager, tmp);
				}
				else if(Cudd_bddIsVarEssential(manager, b, k*2+1,0)){
					fr = Cudd_Not(Cudd_bddIthVar(manager, k*2+1));
					Cudd_Ref(fr);
					tmp = Cudd_bddAnd(manager, myneg, fr);
					Cudd_RecursiveDeref(manager,fr);
					Cudd_Ref(tmp);
					Cudd_RecursiveDeref(manager, myneg);
					myneg = tmp;
					Cudd_Ref(myneg);
					Cudd_RecursiveDeref(manager, tmp);
				}
			}
			Cudd_RecursiveDeref(manager,b);
			Cudd_RecursiveDeref(manager,*c);
		}
		pair<DdNode*, DdNode*> *b = new pair<DdNode*, DdNode*>(mypos, myneg);
		action_affects.insert(make_pair((a)->first ,b));
		return b;
	}
}


BitVector*  getHelpfulActs(RelaxedPlan* t_rp){

	//   //  BitVector* returnBV = NULL;

	bool sameEffects = //
			true;//    false;

	bool samePos = true;
	bool sameNeg = true;

	std::set<LabelledAction*>::iterator j;
	//   int uid_block;
	//   unsigned int uid_mask;
	//   action_list_node* acts = available_acts;
	//   OpNode* op;
	//   alt_action* act;
	//   FtNode* facts = gall_fts_pointer;
	//   EfEdge* tmp_adder = 0;

	BitVector* returnBV = new_bit_vector(gop_vector_length);
	DdNode* posVars = Cudd_ReadOne(manager);
	Cudd_Ref(posVars);
	DdNode* negVars = Cudd_ReadOne(manager);
	Cudd_Ref(negVars);

	//  cout << "Getting HA's"<<endl;
	//  t_rp->display();
	//   // returnBV = (BitVector*)new_bit_vector(gop_vector_length);
	//    if(  Cudd_DebugCheck(manager)){  cout << "DEBUG PROBLEM " << Cudd_ReadDead(manager)  <<endl;     }

	for(int i = 0; i < 1; i++){
		//    cout << "i = " << i << endl;

		DdNode* tmp, *fr;
		for(j = t_rp->action_levels[i]->begin();
				j != t_rp->action_levels[i]->end();++j){

			if(((OpNode*)(*j)->elt)->alt_index <= num_alt_acts){//action
				// cout << ((OpNode*)(*j)->elt)->alt_index <<endl;



				std::map<const Action*, DdNode*>::iterator a = action_transitions.begin();
				for(;a != action_transitions.end(); a++){
					if((*a).first->id() == ((OpNode*)(*j)->elt)->alt_index){
						//     (*a).first->print(cout, (*my_problem).terms()); cout << endl;


						if(sameEffects){
							//DdNode* mypos, *myneg;
							pair<DdNode*, DdNode*>* affects = extractAffects(&(*a));

							if(affects->first == Cudd_ReadLogicZero(manager) ||
									affects->second == Cudd_ReadLogicZero(manager)){
								delete affects;
								continue;
							}

							if(samePos){
								fr = Cudd_bddAnd(manager, posVars, affects->first);
								Cudd_Ref(fr);
								Cudd_RecursiveDeref(manager,posVars);
								posVars = fr;
								Cudd_Ref(posVars);
								Cudd_RecursiveDeref(manager, fr);
							}
							if(sameNeg){
								fr = Cudd_bddAnd(manager, negVars, affects->second);
								Cudd_Ref(fr);
								Cudd_RecursiveDeref(manager,negVars);
								negVars = fr;
								Cudd_Ref(negVars);
								Cudd_RecursiveDeref(manager, fr);
							}
						}
						else{//use actions in rp only
							set_bit(returnBV, (*a).first->id());
						}
					}
				}

			}
			else {//noop

			}

		}

	}

	//find actions that give pos or neg vars
	//    printBDD(posVars);
	//    printBDD(negVars);
	std::map<const Action*, DdNode*>::iterator a = action_transitions.begin();
	for(;a != action_transitions.end() && sameEffects; a++){
		DdNode *nextSt;

		pair<DdNode*, DdNode*>* affects1 = extractAffects(&(*a));
		int in = 0;
		if(affects1->first == Cudd_ReadLogicZero(manager) || !affects1->first ||
				affects1->second == Cudd_ReadLogicZero(manager) || !affects1->second){
			delete affects1;
			continue;
		}

		for(int k = 0 ; !in && k < num_alt_facts; k++){
			if((samePos &&
					Cudd_bddIsVarEssential(manager, affects1->first, k*2+1,1) &&
					Cudd_bddIsVarEssential(manager, posVars, k*2+1,1)) ||
					(sameNeg &&
							Cudd_bddIsVarEssential(manager, affects1->second, k*2+1,0) &&
							Cudd_bddIsVarEssential(manager, negVars, k*2+1,1))){
				in = 1;
			}
		}
		//       Cudd_RecursiveDeref(manager,b);
		//       Cudd_RecursiveDeref(manager,*c);
		//     }

	//        cout << "in = " << in << endl;

		if(in)
			set_bit(returnBV, (*a).first->id());

	}

	Cudd_RecursiveDeref(manager,posVars);
	Cudd_RecursiveDeref(manager,negVars);


	//        print_BitVector(returnBV, gop_vector_length);

	//        cout << endl;
	return returnBV;
	//  return NULL;
}


bool useCuddCard = true;
double getCardinality(DdNode* d){
	DdNode* t;

	double return_val =0.0;

	if(useCuddCard){
		t = Cudd_bddAnd(manager, d, next_state_cube);
		Cudd_Ref(t);
		if(pow(2.0, 2*num_alt_facts) < DBL_MAX){
			return_val = Cudd_CountMinterm(manager, t, 2*num_alt_facts);
		}
		else{
			assert(0);
			EpDouble *epd = EpdAlloc();
			Cudd_EpdCountMinterm(manager, d, 2*num_alt_facts , epd);
			double v;
			int e;
			EpdGetValueAndDecimalExponent(epd, &v, &e);
			return_val =((double)e)*log((double)10.0)+(double)log(v);
			EpdFree(epd);
		}
		Cudd_RecursiveDeref(manager, t);
	}
	if(0 && (return_val > DBL_MAX || !useCuddCard)){
		//useCuddCard = false;
		t = Cudd_BddToAdd(manager, d);
		Cudd_Ref(t);
		return_val = get_sum(t);
		Cudd_RecursiveDeref(manager, t);
	}

	if(CUDD_OUT_OF_MEM == return_val){
		cout << "OOM" <<endl;
		exit(0);
	}
	//  cout << "Card is = " << return_val << " for: " <<endl;





	return min(return_val, DBL_MAX);

}

DdNode* getInvariants(){
	int graph_levels=0;
	DdNode* invar;// = Cudd_ReadOne(manager);

	if(PF_LUG && LUG_FOR == SPACE){
		DdNode* invar1, *invar2;

		invar = Cudd_ReadLogicZero(manager);
		Cudd_Ref(invar);

		PF_LUG = FALSE;
		createInitLayer(Start->dd);

		gbit_goal_state = my_gbit_goal_state;
//		cout << "BG"<<endl;
//		printBDD(Start->dd);
		build_graph(&graph_levels, num_alt_facts, ALLOW_LEVEL_OFF, MUTEX_SCHEME );
//		cout << "BG"<<endl;
//				printBDD(Start->dd);

		invar2 = statesAtLevel(//Cudd_ReadOne(manager),
				Start->dd,
				graph_levels);
		Cudd_Ref(invar2);
//		cout << "BG"<<endl;
//				printBDD(Start->dd);

		free_graph_info(graph_levels);
		COMPUTE_LABELS=TRUE;
		PF_LUG = TRUE;

		for(int i = 0; i < NUMBER_OF_MGS; i++){

			invar1 = Cudd_ReadOne(manager);
			Cudd_Ref(invar1);
			DdNode *nd = make_sample_index(i);//all_samples.front();
			Cudd_Ref(nd);
			//      cout << "n d = " << endl;
			//      printBDD(nd);
//			all_samples.remove(nd);
//			new_samples.push_back(nd);
//			samples.push_back(nd);
			DdNode *tmp = Cudd_bddAnd(manager,
					nd,
					invar1);
			Cudd_Ref(tmp);
			Cudd_RecursiveDeref(manager, nd);
			Cudd_RecursiveDeref(manager, invar1);
			invar1 = tmp;
			Cudd_Ref(invar1);
			Cudd_RecursiveDeref(manager, tmp);

			tmp = Cudd_bddOr(manager, invar, invar1);
			Cudd_Ref(tmp);
			Cudd_RecursiveDeref(manager, invar);
			invar = tmp;
			Cudd_Ref(invar);
			Cudd_RecursiveDeref(manager, tmp);
			Cudd_RecursiveDeref(manager, invar1);
		}
		//    printBDD(invar);
		//  printBDD(invar2);

		DdNode *invar3 = Cudd_bddAnd(manager, invar, invar2);
		Cudd_Ref(invar3);
		Cudd_RecursiveDeref(manager, invar2);
		Cudd_RecursiveDeref(manager, invar);

		return invar3;
	}

	//COMPUTE_LABELS=FALSE;
	createInitLayer(Start->dd);
	gbit_goal_state = my_gbit_goal_state;
	cout << "BG"<<endl;
	  printBDD(Start->dd);
	build_graph(&graph_levels, num_alt_facts, ALLOW_LEVEL_OFF, MUTEX_SCHEME );

	invar = statesAtLevel(//Cudd_ReadOne(manager),
			Start->dd,
			graph_levels);
	Cudd_Ref(invar);

	if(invar == Cudd_ReadLogicZero(manager)){
		Cudd_RecursiveDeref(manager, invar);
		invar = Cudd_ReadOne(manager);
		Cudd_Ref(invar);
	}

	//   cout << "invar:"<<endl;
	//printBDD(invar);
	//   cout << "free"<<endl;
	free_graph_info(graph_levels);
	COMPUTE_LABELS=TRUE;
	//  printBDD(invar);

	return invar;

}


//returns a normalized ADD that is a probability distribution over num_samples
//orig is an ADD
DdNode* sampleKWorlds(DdNode *orig, int num_samples, DdNode **usedSamples, int *num_goals, int numUsedSamples){
	list<DdNode*> cubes;
	list<double> values;
	list<DdNode*>::iterator c;
	DdNode* nd, *tmp, *tmp1;
	list<double>::iterator v;
	DdNode *sample = Cudd_ReadLogicZero(manager);
	Cudd_Ref(sample);
	double cumm;
	int max_goal_states;
	//  if(DO_INCREMENTAL_HEURISTIC)
	//    max_goal_states = num_samples;
	//  else{
	int a = floor(goal_threshold*(num_samples+numUsedSamples));
	max_goal_states = (a < 0 ? 0 : a);
	//  }
	int num_goal_states = *num_goals;

	//cout << "get sample" << endl;

	//devel
	if(1){
		Cudd_RecursiveDeref(manager, sample);


		if(RBPF_PROGRESSION){
		  DdNode *t = Cudd_addExistAbstract(manager, orig, RBPF_SAMPLE_CUBE);
		  Cudd_Ref(t);
		  t = normalize(t);
		  //printBDD(t);
		  sample = draw_samples(t, num_samples);
		  //printBDD(sample);
		  Cudd_RecursiveDeref(manager, t);
		  
		}
		else{
		//    DdNode *tmp = Cudd_BddToAdd(manager, b_goal_state);
		//      Cudd_Ref(tmp);
		//      DdNode* tmp1 = Cudd_addApply(manager, Cudd_addMinus, tmp, Cudd_addConst(manager, 1));
		//      Cudd_Ref(tmp1);
		//      DdNode* tmp2 = Cudd_addApply(manager, Cudd_addTimes, tmp1, Cudd_addConst(manager, -1));
		//      Cudd_Ref(tmp2);
		//      DdNode* tmp3 = Cudd_addApply(manager, Cudd_addTimes, tmp2, orig);
		//      Cudd_Ref(tmp3);
		//      tmp3= normalize(tmp3);

		//     printBDD(tmp1);
		//     printBDD(tmp2);
		//     printBDD(orig);
		//     printBDD(tmp3);
		 sample = draw_samples(orig, num_samples);
		}
		//DdNode * sample = draw_samples(tmp3, num_samples);


		//      Cudd_RecursiveDeref(manager, tmp);
		//      Cudd_RecursiveDeref(manager, tmp1);
		//      Cudd_RecursiveDeref(manager, tmp2);
		//      Cudd_RecursiveDeref(manager, tmp3);
		//      std::cout << "sample = " << std::endl;
		//      printBDD(sample);
		return sample;

	}
//	else{
//		if(my_problem->domain().requirements.non_deterministic){
//			tmp1 = Cudd_bddAnd(manager, orig, Cudd_Not(b_goal_state));
//			Cudd_Ref(tmp1);
//			tmp = Cudd_BddToAdd(manager, tmp1);
//			Cudd_Ref(tmp);
//			Cudd_RecursiveDeref(manager, tmp1);
//			tmp1 = normalize(tmp);
//			Cudd_Ref(tmp1);
//			get_cubes(tmp1, &cubes, &values);
//			Cudd_RecursiveDeref(manager, tmp);
//			Cudd_RecursiveDeref(manager, tmp1);
//		}
//		else{
//			if(0){
//				tmp = Cudd_addBddStrictThreshold(manager, orig, 0.0);
//				Cudd_Ref(tmp);
//				tmp1 = Cudd_bddAnd(manager, tmp, Cudd_Not(b_goal_state));
//				Cudd_Ref(tmp1);
//				Cudd_RecursiveDeref(manager, tmp);
//				tmp = Cudd_BddToAdd(manager, tmp1);
//				Cudd_Ref(tmp);
//				Cudd_RecursiveDeref(manager, tmp1);
//				tmp1 = Cudd_addApply(manager, Cudd_addTimes, tmp, orig);
//				Cudd_Ref(tmp1);
//				Cudd_RecursiveDeref(manager, tmp);
//				tmp = normalize(tmp1);
//				Cudd_Ref(tmp);
//				get_cubes(tmp, &cubes, &values);
//				Cudd_RecursiveDeref(manager, tmp1);
//				Cudd_RecursiveDeref(manager, tmp);
//			}
//			else
//				get_cubes(orig, &cubes, &values);
//		}
//
//
//		//reuse previously drawn samples
//		if(DO_INCREMENTAL_HEURISTIC && LUG_FOR==SPACE){
//			//Cudd_RecursiveDeref(manager, sample);
//			if(0){//incremntal no recomp
//			}
//			else{
//				sample = *usedSamples;
//				Cudd_Ref(sample);
//			}
//		}
//
//
//		//draw sample and conjoin with nd_cube and or into mapping
//
//
//		/// cout << "Drawing " << num_samples << " allowing " << max_goal_states << " goals"  <<endl;
//		//    printBDD(orig);
////		new_samples.clear();
//		for(int i = 0; i < num_samples; i++ ){//nd = all_samples.begin(); nd != all_samples.end(); nd++){
//
//			if(all_samples.empty()){
//				cout << "ERROR: RAN OUT OF SAMPLES" << endl;
//				exit(0);
//			}
//
//
//
//			cout << "sample " << i << endl;
//
//			nd = all_samples.front();
//			Cudd_Ref(nd);
//
//
//			if(DO_INCREMENTAL_HEURISTIC && LUG_FOR==SPACE){
//				DdNode *t = Cudd_bddIntersect(manager, *usedSamples, nd);
//				Cudd_Ref(t);
//				if(t != Cudd_ReadLogicZero(manager)){
//					all_samples.remove(nd);
//					used_samples.push_back(nd);
//					//cout << "HO" << endl;
//					//exit(0);
//					//     i--;
//					Cudd_RecursiveDeref(manager, t);
//					continue;
//				}
//				Cudd_RecursiveDeref(manager, t);
//			}
//
//
//
//			all_samples.remove(nd);
//			samples.push_back(nd);
//			new_samples.push_back(nd);
//
//			//    cout << "nd = " << endl;
//			//printBDD(nd);
//
//			//     for(c = cubes.begin(); c != cubes.end();c++){
//			//   cout << "start try cube = "<<endl;
//			//   printBDD(*c);
//			//       }
//
//			bool accept = false;
//
//			while(!accept){
//
//				cumm = 0.0;
//				double s = ((double)randomGen->myrand()/((double)(RAND_MAX)+(double)(1)));
//				cout << "Sample " << i << " = " << s <<endl;
//				for(c = cubes.begin(), v = values.begin();
//						c != cubes.end() && v != values.end();
//						c++, v++){
//					cout << "try cube = "<<endl;
//					printBDD(*c);
//
//
//					cumm += (*v);
//
//					if(cumm >= s){
//						cout << "cumm = " << cumm << " s = " << s
//								<< "numg = " <<  num_goal_states
//								<< "maxg = " << max_goal_states <<endl;
//						int is_goal = bdd_entailed(manager, *c, b_goal_state);
//
//						if(num_goal_states == max_goal_states && is_goal)
//							break;
//						else if(is_goal){
//							num_goal_states++;
//							(*num_goals)++;
//						}
//						accept = true;
//						//    cout << "cube = "<<endl;
//						//    printBDD(*c);
//						//   //  printBDD(nd);
//						tmp = Cudd_bddAnd(manager, nd, *c);
//						//   cout << "cube&nd = "<<endl;
//						Cudd_Ref(tmp);
//
//
//						//   printBDD(tmp);
//						tmp1 = Cudd_bddOr(manager, tmp, sample);
//						Cudd_Ref(tmp1);
//						//printBDD(tmp1);
//						Cudd_RecursiveDeref(manager, sample);
//						Cudd_RecursiveDeref(manager, tmp);
//						sample = tmp1;
//						Cudd_Ref(sample);
//						Cudd_RecursiveDeref(manager, tmp1);
//						printBDD(sample);
//						Cudd_RecursiveDeref(manager, nd);
//						break;
//					}
//				}
//			}
//		}
//		for(c = cubes.begin(); c != cubes.end(); c++){
//			Cudd_RecursiveDeref(manager, *c);
//		}
//
//		//remember samples used
//		if(DO_INCREMENTAL_HEURISTIC && LUG_FOR==SPACE){
//			//    tmp1 = Cudd_bddOr(manager, *usedSamples, sample);
//			//    Cudd_Ref(tmp1);
//			if(0){//no recomp
//			}
//			else{
//				Cudd_RecursiveDeref(manager, *usedSamples);
//				*usedSamples = sample;
//				Cudd_Ref(*usedSamples);
//			}
//			//    Cudd_RecursiveDeref(manager, tmp1);
//		}
//	}

	cout << "got " << num_goal_states << " goals" << endl;

	//  cout << "]" <<endl;

	//   sample = normalize(sample);
	//   Cudd_RecursiveDeref(manager, sample);

	//  cout << endl<< "got sample " << samples.size() << endl;
	//   printBDD(sample);

	//    pf_mappings[0] = sample;
	//    Cudd_Ref(pf_mappings[0]);
	//    Cudd_RecursiveDeref(manager, sample);

	//  cout << "DIFF = " << endl;
	//  DdNode *t = Cudd_addApply(manager, Cudd_addMinus, orig, sample);
	//  printBDD(t);

	//   cout << "used" << endl;
	//   printBDD(*usedSamples);
	//   cout << "sample" << endl;
	//   printBDD(sample);
	//   cout << "used_samples = " << used_samples.size() << endl;

	return sample;


}

void getStateFormulasToEvaluate(list<list<DdNode*>* >* state_formulas,
		int level, int singles){
	//  int singles = 1;
	//  int doubles = 0;

	list<DdNode*>* tmpl;

	if(singles){
		for(int i = 0; i < num_alt_facts; i++){
			tmpl = new list<DdNode*>();
			if(gft_table[i] && gft_table[i]->info_at[level] &&
					!gft_table[i]->info_at[level]->is_dummy){
				tmpl->push_back(Cudd_bddIthVar(manager, 2*i));
			}
			if(gft_table[NEG_ADR(i)] && gft_table[NEG_ADR(i)]->info_at[level] &&
					!gft_table[NEG_ADR(i)]->info_at[level]->is_dummy){
				tmpl->push_back(Cudd_Not(Cudd_bddIthVar(manager, 2*i)));
			}
			state_formulas->push_back(tmpl);
		}
	}

	else {//if(doubles){
		DdNode *tmp;
		int pos1, neg1, pos2, neg2;
		for(int i = 0; i < num_alt_facts; i++){
			for(int j = i+1; (j < num_alt_facts); j++){
				tmpl = new list<DdNode*>();
				if(gft_table[i] && gft_table[i]->info_at[level] &&
						!gft_table[i]->info_at[level]->is_dummy)
					pos1 = 1;
				else
					pos1 = 0;

				if(gft_table[NEG_ADR(i)] && gft_table[NEG_ADR(i)]->info_at[level] &&
						!gft_table[NEG_ADR(i)]->info_at[level]->is_dummy)
					neg1 = 1;
				else
					neg1 = 0;

				if(!pos1 && !neg1){
					delete tmpl;
					continue;
				}

				if(gft_table[j] && gft_table[j]->info_at[level] &&
						!gft_table[j]->info_at[level]->is_dummy)
					pos2 = 1;
				else
					pos2 = 0;

				if(gft_table[NEG_ADR(j)] && gft_table[NEG_ADR(j)]->info_at[level] &&
						!gft_table[NEG_ADR(j)]->info_at[level]->is_dummy)
					neg2 = 1;
				else
					neg2 = 0;

				if(!pos2 && !neg2){
					delete tmpl;
					continue;
				}


				if(pos1 && pos2){
					tmp = Cudd_bddAnd(manager,
							Cudd_bddIthVar(manager, 2*i),
							Cudd_bddIthVar(manager, 2*j));
					Cudd_Ref(tmp);
					tmpl->push_back(tmp);
					//Cudd_RecursiveDeref(manager, tmp);
				}
				else if(pos1 && neg2){
					tmp = Cudd_bddAnd(manager,
							Cudd_bddIthVar(manager, 2*i),
							Cudd_Not(Cudd_bddIthVar(manager, 2*j)));
					Cudd_Ref(tmp);
					tmpl->push_back(tmp);
					//Cudd_RecursiveDeref(manager, tmp);
				}
				else if(neg1 && pos2){
					tmp = Cudd_bddAnd(manager,
							Cudd_Not(Cudd_bddIthVar(manager, 2*i)),
							Cudd_bddIthVar(manager, 2*j));
					Cudd_Ref(tmp);
					tmpl->push_back(tmp);
					//Cudd_RecursiveDeref(manager, tmp);
				}
				else if(neg1 && neg2){
					tmp = Cudd_bddAnd(manager,
							Cudd_Not(Cudd_bddIthVar(manager, 2*i)),
							Cudd_Not(Cudd_bddIthVar(manager, 2*j)));
					Cudd_Ref(tmp);
					tmpl->push_back(tmp);
					//Cudd_RecursiveDeref(manager, tmp);
				}

				state_formulas->push_back(tmpl);
			}
		}
	}
}

//return the set of states reachable at level,
//from states in source
DdNode* statesAtLevel(DdNode* source, int level){
	list<list<DdNode*>* > state_formulas;
	DdNode *tmp, *tmp1, *tres, *result = Cudd_ReadOne(manager);

//	cout << "states at level = " << level << endl;
//	printBDD(source);
	getStateFormulasToEvaluate(&state_formulas, level, 1);

	//	DdNode *src = Cudd_addExistAbstract(manager, source, RBPF_SAMPLE_CUBE);
	//	Cudd_Ref(src);


	for(list<list<DdNode*>* >::iterator i = state_formulas.begin();
			i != state_formulas.end(); i++){
		tres = Cudd_ReadLogicZero(manager);
		Cudd_Ref(tres);
		for(list<DdNode*>::iterator j = (*i)->begin();
				j != (*i)->end(); j++){
			//printBDD(*j);


			tmp = labelBDD(source,
					//initialBDD,
					*j, level);
			Cudd_Ref(tmp);
			//			    printBDD(tmp);
			//     printBDD(*j);
					    //printBDD(source);
			if(tmp != Cudd_ReadLogicZero(manager)){
				//there exists some state in source that reaches
				//every state described by *j
				tmp1 = Cudd_bddOr(manager, tres, *j);
				Cudd_Ref(tmp1);
				Cudd_RecursiveDeref(manager, tres);
				tres = tmp1;
				Cudd_Ref(tres);
				Cudd_RecursiveDeref(manager, tmp1);
			}
			Cudd_RecursiveDeref(manager, tmp);
		}
		tmp1 = Cudd_bddAnd(manager, result, tres);
		Cudd_Ref(tmp1);
		Cudd_RecursiveDeref(manager, result);
		result = tmp1;
		Cudd_Ref(result);
		Cudd_RecursiveDeref(manager, tmp1);
		Cudd_RecursiveDeref(manager, tres);
		delete *i;
	}

	//Cudd_RecursiveDeref(manager, src);
	/* SOME WHERE BELOW HERE, THERE IS A SEG FAULT */

	//   state_formulas.clear();
	//   getStateFormulasToEvaluate(&state_formulas, level, 0);


	//   for(list<list<DdNode*>* >::iterator i = state_formulas.begin();
	//       i != state_formulas.end(); i++){
	//     for(list<DdNode*>::iterator j = (*i)->begin();
	//   j != (*i)->end(); j++){
	//       tmp = labelBDD(source, *j, level);
	//       Cudd_Ref(tmp);

	//       if(tmp == Cudd_ReadLogicZero(manager)){
	//   tmp1 = Cudd_bddAnd(manager, result, Cudd_Not(*j));
	//   Cudd_Ref(tmp1);
	//   Cudd_RecursiveDeref(manager, result);
	//   result = tmp1;
	//   Cudd_Ref(result);
	//   Cudd_RecursiveDeref(manager, tmp1);
	//       }
	//       Cudd_RecursiveDeref(manager, tmp);
	//     }
	//     delete *i;
	//   }





	return result;
}


bool first_sample = true;

/**
 * 计算启发式值， states为children。
 */
void getHeuristic(list<StateNode*>* states,
		StateNode* parent,//double parentGoalSatisfaction,
		int currHorizon){
	BitVector* helpful_acts;
	DdNode* lugBase = Cudd_ReadLogicZero(manager);// initial state BDD for LUG/SG
	DdNode* invars, *fr, *labelFnHolder;

	//if building a subset of worlds in the LUG,
	//we replace the bdds in states with a random subset of worlds
	//and pass it to compute the heuristics
	//this is the list of the originals we restore before exiting
	list<DdNode*> originals;


	int gotAppl;
	int lugHeur = 0;
	// if(states)
	//        cout << "|states|= " << states->size() <<endl;
	// else
	//        cout << "|states|=  NULL"<<endl;
	// cout << "NUM = " << NUMBER_OF_MGS <<endl;

	if(LUG_FOR == SPACE)// set the global config
		cout << "ppHI" <<endl;

	/**
	 * NONE，根据最小的reward最为目标的h
	 */
	if (HEURISTYPE == NONE && states){
		// cout << "|states| = " << states->size()<<endl;
		for(list<StateNode*>::iterator i = states->begin();
				i != states->end(); i++){
			(*i)->h = min(total_goal_reward, 0.0);//(*i)->h = 0.0;
			cout << "NONE" <<endl;
			cout << (*i)->h << endl;
		}
	}
	/**
	 * CARD，根据每个dd的cardinality作为h
	 */
	else if (HEURISTYPE == CARD && states){
		//cout << "|states| = " << states->size()<<endl;
		for(list<StateNode*>::iterator i = states->begin();
				i != states->end(); i++){
				// 注释掉输出
				/*
			     cout << "CARD" <<endl;
			(*i)->h = getCardinality((*i)->dd);*/

			cout << (*i)->h << endl;
		}
	}
	/*************** Multiple graph Heuristics **************/
	
	else if((HEURISTYPE == HRPSUM ||
			HEURISTYPE == HRPMAX ||
			HEURISTYPE == HRPUNION) && states){
		// 考虑每个state, now only one belief state
		for(list<StateNode*>::iterator i = states->begin();
				i != states->end(); i++){
			COMPUTE_LABELS=FALSE;// MG Heuristic不使用label
			ALLOW_LEVEL_OFF = FALSE;
			totalgp = build_k_planning_graphs((*i)->dd,
					b_goal_state,  num_k_graphs );

			(*i)->h = getRelaxedPlanHeuristic();// 该接口目前没有问题
			if((*i)->h == IPP_MAX_PLAN)
				(*i)->h = 999999999.9999;
			       cout << "i = " << (*i)->StateNo << " H = " << (*i)->h << endl;
			free_k_graphs();
			cout << "done free_k_graphs()"<<endl;
		}

	}

	/*************** End Multiple graph Heuristics **************/


	/*************** LUG Heuristics **************/

	else{

		/*------- All but construct initial SAG  --------*/
		// LUG_FOR default is NODE
		// NUMBER_OF_MGS default is 0.0 means to pick all initial states

		if((LUG_FOR != INCREMENTAL && LUG_FOR != SPACE) || states){


			//build lug for all states in list of states
			// cout << "|states| = " << states->size() << endl;
			for(list<StateNode*>::iterator i = states->begin();
					i != states->end(); i++){

				/*------- Pick a subset of states in i to use in heuristic  --------*/
				
				if(LUG_FOR!=AHREACHABLE && NUMBER_OF_MGS != 0.0){ //then pick a subset of worlds for lug

					lugHeur = 1;
					//store original
					//originals.push_back((*i)->dd);
					(*i)->backup_dd = (*i)->dd;
					Cudd_Ref((*i)->backup_dd);

					int num_cubes;
					if(!PF_LUG){
						if(RANDOM_SUBSTRATE == RANDOM_STATES ?
								num_cubes = get_sum((*i)->dd) :
								num_cubes = Cudd_CountMinterm(manager,
										(*i)->dd,
										Cudd_SupportSize(manager, (*i)->dd)));
					}
					int num_graphs = (NUMBER_OF_MGS < 1.0 ?
							(NUMBER_OF_MGS == 0.0 ?
									num_cubes : //all graphs
					(int)ceil(num_cubes*NUMBER_OF_MGS)) : //build proporation
					NUMBER_OF_MGS);  //build this number


					if(!PF_LUG){
						//turn dd into bdd for pickK
						DdNode *tmp1 = Cudd_addBddStrictThreshold(manager, (*i)->dd, 0.0);
						Cudd_Ref(tmp1);
						//    printBDD(tmp1);
						DdNode* tmp = pickKRandomWorlds(tmp1, num_graphs);
						Cudd_RecursiveDeref(manager, tmp1);
						tmp1 = Cudd_BddToAdd(manager, tmp);
						Cudd_Ref(tmp1);
						Cudd_RecursiveDeref(manager, tmp);
						tmp = Cudd_addApply(manager, Cudd_addTimes, (*i)->dd, tmp1);
						Cudd_Ref(tmp);
						(*i)->dd = tmp;

						// printBDD(tmp);
					}
					else{//sample particles from distribution
						//cout << "sampling from belief" << endl;
						//      double alpha = 0.01, beta = 0.1, delta = 0.1;
						//       if(first_sample){
						//         num_graphs = guess_num_samples(alpha, beta, goal_threshold, delta);
						//         first_sample = false;
						//         NUMBER_OF_MGS = num_graphs;
						//       }

						//expected_sample_size = (sample_threshold < 1 ? 1 : sample_threshold);

						if(0){
							expected_sample_size = expected_samples(total_sample_size,
									sample_threshold,
									beta, alpha,
									(*i)->goalSatisfaction,
									goal_threshold, delta);

							expected_sample_size = (expected_sample_size < 1 ? 1 : expected_sample_size);

							//        cout << "expected_sample_size = " << expected_sample_size << endl;

							num_graphs = expected_sample_size;
						}
						else
							num_graphs = NUMBER_OF_MGS;

						//       cout << "num = " << num_graphs <<endl;

						DdNode* tmp;
						//if(my_problem->domain().requirements.probabilistic)
						if(!(*i)->usedSamples)
							(*i)->usedSamples = Cudd_ReadLogicZero(manager);
						Cudd_Ref((*i)->usedSamples);
						//						cout << "Before: " <<endl;
						//printBDD((*i)->dd);
						//printBDD((*i)->usedSamples);
						DdNode *used = (*i)->usedSamples;
						Cudd_Ref(used);
						//						printBDD((*i)->dd);
						int num_goals = (DO_INCREMENTAL_HEURISTIC ? (*i)->goalSamples : 0);
						tmp = sampleKWorlds((*i)->dd, num_graphs, &used, &num_goals, (*i)->hIncrement*NUMBER_OF_MGS);
						//printBDD(tmp);
						(*i)->goalSamples = num_goals;
						Cudd_RecursiveDeref(manager, (*i)->usedSamples);
						(*i)->usedSamples = used;
						Cudd_Ref((*i)->usedSamples);
						Cudd_RecursiveDeref(manager, used);
						//      cout << "After: " <<endl;
						//printBDD((*i)->usedSamples);



						//else{
						//  tmp =  pickKRandomWorlds((*i)->dd, num_graphs);
						//}

						//       printBDD((*i)->dd);
						//       printBDD(tmp);

						Cudd_Ref(tmp);
						Cudd_RecursiveDeref(manager, (*i)->dd);
						(*i)->dd = tmp;
						Cudd_Ref((*i)->dd);
						//printBDD(tmp);
						lugBase = tmp;
						Cudd_Ref(lugBase);
						Cudd_RecursiveDeref(manager, tmp);
					}


				}
				/*------- End Pick a subset of states in i to use in heuristic  --------*/


				//if there is more than one state in states, then union all dds to create a child sag
				if(my_problem->domain().requirements.non_deterministic){
					DdNode *tmp = Cudd_bddOr(manager, lugBase, (*i)->dd);
					Cudd_Ref(tmp);
					Cudd_RecursiveDeref(manager, lugBase);
					lugBase = tmp;
					Cudd_Ref(lugBase);
					Cudd_RecursiveDeref(manager, tmp);

				}
				// do not use in conformant planning
				if(HEURISTYPE == CORRRP){
					DdNode *dd = Cudd_addBddStrictThreshold(manager, (*i)->dd, 0.0);
					Cudd_Ref(dd);
					DdNode *tmp = Cudd_bddOr(manager, lugBase, dd);
					Cudd_Ref(tmp);
					Cudd_RecursiveDeref(manager, lugBase);
					lugBase = tmp;
					Cudd_Ref(lugBase);
					Cudd_RecursiveDeref(manager, tmp);
					Cudd_RecursiveDeref(manager, dd);
				}


				//   if(LUG_FOR == SPACE && my_problem->domain().requirements.probabilistic){
				//     //set label distribution to match belief
				//     labelFnHolder = probabilisticLabels;
				//     Cudd_Ref(labelFnHolder);

				//     fr = Cudd_addApply(manager, Cudd_addTimes, probabilisticLabels, lugBase);
				//     Cudd_Ref(fr);
				//     Cudd_RecursiveDeref(manager, probabilisticLabels);
				//     probabilisticLabels = fr;
				//     Cudd_Ref(probabilisticLabels);
				//     Cudd_RecursiveDeref(manager, fr);
				//   }

			}
		}

		/*------- Determine worlds for reachable SAG  --------*/

		else if (LUG_FOR == SPACE){

			//       if(my_problem->domain().requirements.probabilistic){

			//   //set base to uniform distribution
			//   fr = Cudd_addConst(manager, (1.0/(double)pow(2.0,num_alt_facts)));
			//   Cudd_Ref(fr);
			//   lugBase = Cudd_addApply(manager, Cudd_addTimes,
			//         fr, Cudd_ReadOne(manager));
			//   Cudd_Ref(lugBase);
			//   Cudd_RecursiveDeref(manager, fr);
			//       }
			//       else if(my_problem->domain().requirements.non_deterministic){

			int old = NUMBER_OF_MGS;
			if(DO_INCREMENTAL_HEURISTIC)
				NUMBER_OF_MGS *= MAX_H_INCREMENT;
			//  NUMBER_OF_MGS = MAX_H_INCREMENT;//*(MAX_H_INCREMENT+1)/2;//pow(2.0, MAX_H_INCREMENT);
			///cout << "get invars" <<endl;



			//invars = getInvariants();
			//Cudd_RecursiveDeref(manager, invars);
			invars = Cudd_ReadOne(manager);
			//Cudd_Ref(invars);
			lugBase = invars;
			//cout << "get invars" <<endl;
			//			printBDD(invars);

			NUMBER_OF_MGS = old;
			Cudd_ReduceHeap(manager, CUDD_REORDER_SIFT, 0);


			//      }

			//printBDD(lugBase);
		}
		/*------- End Determine worlds for reachable SAG  --------*/

		/*------- Determine worldsfor incremental SAG  --------*/

		else if (LUG_FOR == INCREMENTAL){
			if(my_problem->domain().requirements.probabilistic){
				cout << "incremental prabailistic lug NOT SUPPORTED"<<endl;
				exit(0);
			}
			else if(my_problem->domain().requirements.non_deterministic){
				//start incremental lug with just the initial state
				lugBase = b_initial_state;

				//   cout << "orig base = " <<endl;
				//   printBDD(lugBase);

				//   DdNode *dan = Cudd_SupersetShortPaths(manager, lugBase, Cudd_ReadSize(manager),num_alt_facts/2, 1);
				//     //Cudd_SupersetHeavyBranch(manager,lugBase, Cudd_ReadSize(manager), num_alt_facts);

				//   Cudd_Ref(dan);
				//   Cudd_RecursiveDeref(manager, lugBase);
				//   lugBase = dan;
				//   Cudd_Ref(lugBase);
				//   Cudd_RecursiveDeref(manager, dan);

				//   cout << "super base = " <<endl;
				//   printBDD(lugBase);

				//   if(lugBase == b_initial_state){
				//     cout << "EQ"<<endl;
				//     exit(0);
				//   }

			}
		}
		/*------- End Determine worlds for incremental SAG  --------*/







		build_forward_get_h(lugBase, states, parent);
		//cout << "[" <<endl;

//        cout << "BASE = " <<endl;
//		        printBDD(lugBase);


		Cudd_RecursiveDeref(manager,lugBase);

	}
	//  cout << "]"<<flush;
 	
	 // 释放前面分配的DD内存，从backup_dd中恢复原始的state
	if(NUMBER_OF_MGS != 0.0 && lugHeur){

		for(list<StateNode*>::iterator i = states->begin();
				i != states->end(); i++){

			Cudd_RecursiveDeref(manager, (*i)->dd);
			(*i)->dd = (*i)->backup_dd;
			Cudd_Ref((*i)->dd);
			Cudd_RecursiveDeref(manager, (*i)->backup_dd);
		}
	}

	/*************** END LUG Heuristics **************/

	//set f-values
	if(states ){
		for(list<StateNode*>::iterator i = states->begin();
				i != states->end(); i++){
			(*i)->f = (*i)->g + (*i)->h;
			// 注释掉输出
			// cout << "f = " << (*i)->f << "(g=" << (*i)->g << ",h=" << (*i)->h << ")\n";
		}
	}



}

// void getHeuristic(list<StateNode*>* states, StateNode* parent,//double parentGoalSatisfaction,
//       int currHorizon){
//   BitVector* helpful_acts;
//   DdNode* lugBase = Cudd_ReadLogicZero(manager);
//   DdNode* invars, *fr, *labelFnHolder;

//   //if building a subset of worlds in the LUG,
//   //we replace the bdds in states with a random subset of worlds
//   //and pass it to compute the heuristics
//   //this is the list of the originals we restore before exiting
//   list<DdNode*> originals;


//   int gotAppl;
//   int lugHeur = 0;
// //  if(states)
// //       cout << "|states|= " << states->size() <<endl;
// //     else
// //       cout << "|states|=  NULL"<<endl;
//   // cout << "NUM = " << NUMBER_OF_MGS <<endl;




// #ifdef PPDDL_PARSER
// //   if(currHorizon+1 == max_horizon){
// //     for(list<StateNode*>::iterator i = states->begin();
// //   i != states->end(); i++){
// //       (*i)->h = 0.0;
// //     }

// //     return;
// //   }


// #else

//   if(states ){
//     for(list<StateNode*>::iterator i = states->begin();
//   i != states->end(); i++){
//       gotAppl = FALSE;
//       for(action_list a = available_acts; a; a = a->next){
//   if(bdd_entailed(manager, (*i)->dd, a->act->get_effs()->b_pre)){
//     gotAppl = TRUE;
//     break;
//   }
//       }
//       if(!gotAppl){
//   (*i)->h = 999999999.9;
//   //cout << "Skipping dead state"<<endl;
//       }
//     }
//   }
// #endif

//   if(HEURISTYPE == CARD && states!=NULL){


// //    cout << "CARD" <<endl << flush;
//     for(list<StateNode*>::iterator i = states->begin();
//   i != states->end(); i++){
//       (*i)->h =  getCardinality((*i)->dd);
//       //       cout << "Card = " << (*i)->h<<endl;
//       //       printBDD((*i)->dd);
//     }
//   }
//   else if (HEURISTYPE == NONE && states){
//     //cout << "|states| = " << states->size()<<endl;
//      for(list<StateNode*>::iterator i = states->begin();
//   i != states->end(); i++)
// #ifdef PPDDL_PARSER
//        if(OPTIMIZE_REWARDS && OPTIMIZE_PROBABILITY){//cost
//    if(my_problem->domain().requirements.rewards){ //cost - reward
//      //right now there is only a single goal, so its reward is a single number
//      (*i)->h = min(total_goal_reward, 0);
//    }
//    else{ //cost
//      (*i)->h = 0;
//    }
//        }
//        else if(!OPTIMIZE_REWARDS && OPTIMIZE_PROBABILITY){//probability
//    (*i)->h = (*i)->goalSatisfaction - 1.0;
//        }
// #else
//      (*i)->h = 0.0;
// #endif


//      // cout << "NONE" <<endl;
//   }
//   else if((HEURISTYPE == HRPSUM ||
//     HEURISTYPE == HRPMAX ||
//     HEURISTYPE == HRPUNION) && states){

//     for(list<StateNode*>::iterator i = states->begin();
//   i != states->end(); i++){
//       COMPUTE_LABELS=FALSE;
//       ALLOW_LEVEL_OFF = FALSE;

// #ifdef PPDDL_PARSER
//       totalgp = build_k_planning_graphs((*i)->dd,
//           b_goal_state,  num_k_graphs );
// #else
//       totalgp = build_k_planning_graphs((*i)->dd,
//           b_goal_state,
//           available_acts, num_k_graphs );
// #endif
//       (*i)->h = getRelaxedPlanHeuristic();
//       if((*i)->h == IPP_MAX_PLAN)
//   (*i)->h = 999999999.9999;
//       //        cout << "i = " << (*i)->StateNo << " H = " << (*i)->h << endl;
//       free_k_graphs();
//       //        cout << "freed"<<endl;
//     }

//   }
//   else{
//     if(LUG_FOR != SPACE || states){



//       //build lug for all states in list of states
//       for(list<StateNode*>::iterator i = states->begin();
//     i != states->end(); i++){

//   if(NUMBER_OF_MGS != 0.0){ //then pick a subset of worlds for lug

//     lugHeur = 1;
//     //store original
//     //originals.push_back((*i)->dd);
//     (*i)->backup_dd = (*i)->dd;
//     Cudd_Ref((*i)->backup_dd);

//     int num_cubes;
//     if(!PF_LUG){
//       if(RANDOM_SUBSTRATE == RANDOM_STATES ?
//          num_cubes = get_sum((*i)->dd) :
//          num_cubes = Cudd_CountMinterm(manager,
//                (*i)->dd,
//                Cudd_SupportSize(manager, (*i)->dd)));
//     }
//     int num_graphs = (NUMBER_OF_MGS < 1.0 ?
//          (NUMBER_OF_MGS == 0.0 ?
//           num_cubes : //all graphs
//           (int)ceil(num_cubes*NUMBER_OF_MGS)) : //build proporation
//          NUMBER_OF_MGS);  //build this number

//     // cout << "num = " << num_graphs <<endl;

// #ifdef PPDDL_PARSER

//     if(!PF_LUG){
//       //turn dd into bdd for pickK
//       DdNode *tmp1 = Cudd_addBddStrictThreshold(manager, (*i)->dd, 0.0);
//       Cudd_Ref(tmp1);
//       //    printBDD(tmp1);
//       DdNode* tmp =   pickKRandomWorlds(tmp1, num_graphs);
//       Cudd_RecursiveDeref(manager, tmp1);
//       tmp1 = Cudd_BddToAdd(manager, tmp);
//       Cudd_Ref(tmp1);
//       Cudd_RecursiveDeref(manager, tmp);
//       tmp = Cudd_addApply(manager, Cudd_addTimes, (*i)->dd, tmp1);
//       Cudd_Ref(tmp);
//       (*i)->dd = tmp;

//       // printBDD(tmp);
//     }
//     else{//sample particles from distribution
//       DdNode* tmp = sampleKWorlds((*i)->dd, num_graphs);
//       Cudd_Ref(tmp);
//       Cudd_RecursiveDeref(manager, (*i)->dd);
//       (*i)->dd = tmp;
//       Cudd_Ref((*i)->dd);
//       //      printBDD(tmp);
//     }
// #else
//     //pick subset of worlds
//     DdNode* tmp =   pickKRandomWorlds((*i)->dd, num_graphs);
//     //printBDD(tmp);
//     (*i)->dd = tmp;
// #endif



//   }

// #ifdef PPDDL_PARSER
//   lugBase = (*i)->dd;
//   Cudd_Ref(lugBase);
//   if(LUG_FOR == SPACE && my_problem->domain().requirements.probabilistic){
//     //set label distribution to match belief
//     labelFnHolder = probabilisticLabels;
//     Cudd_Ref(labelFnHolder);

//     fr = Cudd_addApply(manager, Cudd_addTimes, probabilisticLabels, lugBase);
//     Cudd_Ref(fr);
//     Cudd_RecursiveDeref(manager, probabilisticLabels);
//     probabilisticLabels = fr;
//     Cudd_Ref(probabilisticLabels);
//     Cudd_RecursiveDeref(manager, fr);
//   }
// #else
//   lugBase = Cudd_bddOr(manager, fr=lugBase, (*i)->dd);
//   Cudd_RecursiveDeref(manager, fr);
//   Cudd_Ref(lugBase);
// #endif
//       }
//     }
//     else{
// #ifdef PPDDL_PARSER
//       if(my_problem->domain().requirements.probabilistic){
//   //set base to uniform distribution
//   fr = Cudd_addConst(manager, (1.0/(double)pow(2.0,num_alt_facts)));
//   Cudd_Ref(fr);
//   lugBase = Cudd_addApply(manager, Cudd_addTimes,
//         fr, Cudd_ReadOne(manager));
//   Cudd_Ref(lugBase);
//   Cudd_RecursiveDeref(manager, fr);
//       }
//       else if(my_problem->domain().requirements.non_deterministic){
//    invars = getInvariants();
//    lugBase = invars;
//       }
// #else
//       invars = getInvariants();
//       lugBase = invars;
//       Cudd_Ref(lugBase);
// #endif

//     }
// //     cout << "BASE = " <<endl;
// //      printBDD(lugBase);

//     build_forward_get_h(lugBase, states, parent);

// #ifdef PPDDL_PARSER
//     if(LUG_FOR == SPACE && states &&
//        my_problem->domain().requirements.probabilistic){
//       //remove distribution from labels
//       Cudd_RecursiveDeref(manager, probabilisticLabels);
//       probabilisticLabels = labelFnHolder;
//       Cudd_Ref(probabilisticLabels);
//       Cudd_RecursiveDeref(manager, labelFnHolder);
// //       fr = Cudd_addApply(manager, Cudd_addDivide, probabilisticLabels, lugBase);
// //       Cudd_Ref(fr);
// //       Cudd_RecursiveDeref(manager, probabilisticLabels);
// //       probabilisticLabels = fr;
// //       Cudd_Ref(probabilisticLabels);
// //       Cudd_RecursiveDeref(manager, fr);

//    //    cout << "Labels = " << exAbstractAllLabels(probabilisticLabels, graph_levels-1) <<endl;
// //     printBDD(probabilisticLabels);
//     }
// #endif
//     Cudd_RecursiveDeref(manager,lugBase);

//   }
//   ///  cout << "]"<<flush;

//   if(NUMBER_OF_MGS != 0.0 && lugHeur){ //restore real states on top of subsets

// //     list<DdNode*>::iterator o = originals.begin();
//     for(list<StateNode*>::iterator i = states->begin();
//   i != states->end(); i++){
//       //  cout << "Reinstating: " << (*i)->StateNo <<endl;

//       Cudd_RecursiveDeref(manager, (*i)->dd);
//       (*i)->dd = (*i)->backup_dd;//*o;
//       Cudd_Ref((*i)->dd);
//       Cudd_RecursiveDeref(manager, (*i)->backup_dd);
//       //      printBDD((*i)->dd);
//       //   o++;
//     }
//   }

// //   if(  Cudd_DebugCheck(manager)){
// //     cout << "DEBUG PROBLEM " << Cudd_ReadDead(manager)  <<endl;
// //     exit(0);
// //   }

//   //set f-values
//   if(states ){
//     for(list<StateNode*>::iterator i = states->begin();
//   i != states->end(); i++){
//       (*i)->f = (*i)->g + (*i)->h;
//       //cout << "f = " << (*i)->f << endl;
//     }
//   }
// }




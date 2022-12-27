#include <math.h>
#include <stdio.h>
#include <fstream>
#include <sstream>

#include <stdexcept>
#include <typeinfo>
#include <exception>
#include <assert.h>


#include "globals.h"
#include "graph_wrapper.h"
#include "solve.h"
#include <iomanip>
#include <iostream>
#include <limits.h>           // INT_MAX
#include <stdlib.h>
#include "cudd.h"
#include "cudd/dd.h"

#include <float.h>
#include <fstream>
#include <algorithm>
#include <string>

#include "exclusions.h"

#ifdef PPDDL_PARSER
#else
#include "parser/ptree.h"
#include "actions/actions.h"
#include "parser/facts.h"
#endif

#include "ipp.h"
#include "memory.h"
#include "output.h"
#include "build_graph.h"
#include "kGraphInfo.h"

#include "labelledElement.h"
#include "lug.h"

#include "lao_wrapper.h"
using namespace std;

int get_bit( BitVector *, int , int );

extern void print_fact_info(FactInfo*,int);
extern DdNode** extractDNFfromBDD(DdNode* node);
extern DdNode** extractTermsFromMinterm(DdNode* minterm);
extern void printFact(int f);
extern char* getFactName(int f);
extern void printLabel(goal* g);// it is undefined.
extern void free_fact_info_pair( FactInfoPair *p );
extern Effect *new_effect();
extern int are_there_non_exclusive( int time, FactInfo *pos, FactInfo *neg );
extern void addInitialAxiomForNonDeterEffect(Label* labels, int type, int);
extern DdNode* generateUniqueNonDeterLabel(int time, char* opname, int effnum,  int ndnum);
extern BitVector *copy_bit_vector( BitVector*, int );
extern void free_graph_and_search_info();
extern  void  reset_original_ipp_information();
extern  void free_graph_info( void );
extern  void print_BitOperator( BitOperator *o );
extern  void free_effect(Effect * );
extern  void free_BitOperator( BitOperator* );
extern int num_alt_facts;
extern  int build_graph( int* ,int, int, int);
extern  FactInfo *new_FactInfo( void );
extern  FactInfoPair *new_fact_info_pair(FactInfo*, FactInfo*);
extern  void print_FactInfo( FactInfo*);
extern  BitOperator *new_BitOperator( char * );
extern  Effect *new_Effect();
extern  Consequent *new_Consequent();
extern  Integers *new_integers(int );
extern  void print_BitVector( BitVector* , int  );
extern  void build_graph_evolution_step( void );
extern  void print_vector( BitVector *, int  );
extern  BitVector *new_bit_vector(int length);
extern kGraphInfo** k_graphs;
extern hash_entry* alt_facts[];
extern int getlevel(int index, int polarity);// momo007 2022.09.16
// extern  goal* get_init_labels(int index, int polarity);
// extern  goal* and_labels(goal*,FtEdge* conditions, int time);
// extern  goal* or_labels(FtNode* a, int time);
extern  DdNode* get_init_labels(int index, int polarity);
extern  DdNode* and_labels(DdNode*,FtEdge* conditions, int time);
extern  DdNode* or_labels(FtNode* a, int time);
extern  void set_bit(BitVector* b, int index);
extern  int goal_non_mutex(int);
//extern  CodeNode *new_CodeNode( Connective c );
//extern  void print_CodeNode( CodeNode *node, int indent );
//extern  void cnf ( CodeNode * codenode );
//extern  void dnf ( CodeNode * codenode );
extern  int goals_proven(int);
extern  void printBDD(DdNode*);
extern  void copy_contents_of_FactInfo(FactInfo**, FactInfo*);
int labels_same(int time);

// extern clausalState* goal_state;
// extern clausalState* initial_state;


//int glCompare(goal* i, goal* j);
//goal* extract_init_label(int, clausalState*,int);
// extern goal* reduceClause(goal* g, int);
// extern goal* liteResolution(goal* g);
// extern int isSubset(goal*,goal*);
// extern goal* reduceClause(goal*, int);
// extern int listCompare(goal*, goal*);
//extern goal* inClauseListHelper(char*, goal*, int&, int, goal_list*);
extern  int getEffectWorldCost(int effect, DdNode* worlds);
extern DdManager* manager;
extern DdNode* NDUniqueMask;
extern DdNode* b_initial_state;
extern DdNode* b_goal_state;
extern  int isOneOf(FtNode*, FtNode*);
//FactInfoPair *gbit_goal_state = NULL;
//FactInfoPair *gbit_initial_state = NULL;



int factsFirstPresent(FactInfoPair*);
//goal* init_pos_labels[MAX_RELEVANT_FACTS];
//goal* init_neg_labels[MAX_RELEVANT_FACTS];
DdNode* init_pos_labels[MAX_RELEVANT_FACTS];
DdNode* init_neg_labels[MAX_RELEVANT_FACTS];

goal* init_label;
int gop_vector_length;
BitVector* initial_model;
DdNode* initialBDD;

std::list< FactInfoPair* > * oneofBitVecs;

int graph_levels = 0;
int num_graphs = 0;

int NDLMax = 0;
int NDLcounter = 0;

int pos_fact_at_min_level[MAX_RELEVANT_FACTS];
int neg_fact_at_min_level[MAX_RELEVANT_FACTS];
unsigned int max_k_pos_fact_at_min_level[MAX_RELEVANT_FACTS];
unsigned int max_k_neg_fact_at_min_level[MAX_RELEVANT_FACTS];

void setKGraphTo(int k);
//int getlevelNonMutex(clausalState*, clausalState*&);
//RelaxedPlan* getRelaxedPlan(int levels, clausalState* cs, int support);

DdNode*** regFactAct; //each entry(for facts) is a BDD array(for acts)
extern int num_alt_acts;
extern std::list<LabelledElement*>* actWorldCost[];
extern std::list<LabelledElement*>* effectWorldCost[];
extern std::list<LabelledElement*>* factWorldCost[];

void free_k_graphs(){
	//cout << "FREEING " << num_graphs << " GRAPHS" <<endl;
	for(int i = 0; i < num_graphs; i++) {
		setKGraphTo(i);
		//  cout << "NUM_LEVS = " << k_graphs[i]->num_levels<<endl;
		free_graph_info(k_graphs[i]->num_levels-1);

		//   printMemInfo();
		//   delete [] k_graphs[i]->pos_fact_at_min_level;
		// delete [] k_graphs[i]->neg_fact_at_min_level;
		//      for (int j = 0; (j < IPP_MAX_PLAN &&  j < k_graphs[i]->num_levels ) ; j++ ) {
		//  	free_it_vector(k_graphs[i]->pos_facts_vector_at[j]);
		//  	free_bit_vector(k_graphs[i]->neg_facts_vector_at[j]);
		//        }
		//        free_ft_pair( k_graphs[i]->ft_mutex_pairs );
		//	 free_ft_node(*(k_graphs[i]->graph));
		//       delete []  k_graphs[i]->pos_facts_vector_at;
		//       delete [] k_graphs[i]->neg_facts_vector_at;
		//      if(k_graphs[i]->relaxed_plan)
		//delete k_graphs[i]->relaxed_plan;
		// cout << "HO"<<endl;
		// Cudd_RecursiveDeref(manager, b_initial_state);
		delete k_graphs[i];

		new_plan = TRUE;
	}
	// cout << "done free"<<endl;
}

// DdNode* regressFact(int factNum, alt_action* act){
//   alt_effect* tmp_eff = NULL;
//   DdNode *ants1, *ants2, *b_sg, *b_ng, *tmp, *ants3, *ants4;
//   simple_goal* sg;
//   neg_goal* ng;

//   if(!regFactAct){
//     regFactAct = new DdNode**[num_alt_facts];
//     for(int i = 0; i < num_alt_facts; i++){
//       regFactAct[i] = new DdNode*[num_alt_acts];

//       for(int j = 0; j < num_alt_acts; j++){
// 	regFactAct[i][j] = NULL;
//       }
//     }
//   }

//   if(!regFactAct[factNum][act->index]){
//     //    cout << "Generating regression for fact " << factNum << " and act " << act->get_name() <<endl;
// //     sg = new simple_goal(new proposition(alt_facts[factNum]), E_POS);
// //     ng = new neg_goal(new simple_goal(new proposition(alt_facts[factNum]), E_POS));
// //     b_sg = goalToBDD(sg);
// //     b_ng = goalToBDD(ng);
//     b_sg = Cudd_bddIthVar(manager,factNum);
//     b_ng = Cudd_Not(Cudd_bddIthVar(manager,factNum));

//     //  cout << "Looking for support for " <<endl;
//     //printFact(factNum); cout <<endl;
//     ants1 = Cudd_ReadLogicZero(manager);
//     ants2 = Cudd_ReadOne(manager);
//     ants3 = Cudd_ReadOne(manager);
//     ants4 = Cudd_ReadLogicZero(manager);
//     Cudd_Ref(ants1);
//     Cudd_Ref(ants2);
//     Cudd_Ref(ants3);
//     Cudd_Ref(ants4);

//     tmp_eff = act->get_effs();
//     while(tmp_eff){
//       if(tmp_eff->b_eff){
// 	//cout << "eff = " <<endl;
// 	//printBDD(tmp_eff->b_eff);
// 	///printBDD(b_sg);
// 	//  1- Or all antecdents whose consequent entails f
// 	//      cout << "CHECK 1"<<endl;
// 	if(bdd_entailed(manager, tmp_eff->b_eff, b_sg)){
// 	  //	if(bdd_entailed(manager, b_sg, tmp_eff->b_eff)){

// 	  //	  cout << "Effect entails postive f" <<endl;
// 	  tmp = bdd_or(manager, tmp_eff->b_pre, ants1);
// 	  Cudd_Ref(tmp);
// 	  Cudd_RecursiveDeref(manager, ants1);
// 	  ants1 = tmp;
// 	  Cudd_Ref(ants1);
// 	}
// 	//  2- And all antecedents whose consequent entail !f
// 	//cout << "CHECK 2"<<endl;
// 	else if(bdd_entailed(manager, tmp_eff->b_eff, b_ng)){
// 	  //if(bdd_entailed(manager, b_ng, tmp_eff->b_eff)){
// 	  //	  cout << "Effect entails negative f" <<endl;
// 	  tmp = bdd_and(manager, tmp_eff->b_pre, ants2);
// 	  Cudd_Ref(tmp);
// 	  Cudd_RecursiveDeref(manager, ants2);
// 	  ants2 = tmp;
// 	  Cudd_Ref(ants2);
// 	}
//       }
//       tmp_eff = tmp_eff->next;
//     }
//     //  3- And f and !2

//     if(!bdd_is_one(manager, ants2)){

//     ants3 = bdd_and(manager, b_sg, Cudd_Not(ants2));
//     //Cudd_Ref(tmp);
//     //Cudd_RecursiveDeref(manager, ants3);
//     //ants3 = tmp;
//     //Cudd_Ref(ants3);

//     //    cout << "ants3" <<endl;
//     //    printBDD(ants3);
//     }
//     else{
//       ants3 = b_sg;
//       Cudd_Ref(ants3);
//     }

//     //  4- Or 3 and 1
//     //    cout << "ants1" << endl;
//     //    printBDD(ants1);
//     if(!bdd_is_one(manager, Cudd_Not(ants1))){
//       //cout << "3 or 1" <<endl;
//       tmp = bdd_or(manager, ants3, ants1);
//       Cudd_Ref(tmp);
//       Cudd_RecursiveDeref(manager, ants4);
//       ants4 = tmp;
//       Cudd_Ref(ants4);

//       //      cout << "ants4" <<endl;
//       //      printBDD(ants4);
//     }
//     else {
//       ants4 = ants3;
//       Cudd_Ref(ants4);
//     }
//     regFactAct[factNum][act->index] = ants4;
//     Cudd_Ref(regFactAct[factNum][act->index] );
//   }

//   //  cout << "Reg fact returns = " <<endl;
//   //  printBDD(regFactAct[factNum][act->index]);

//   return regFactAct[factNum][act->index];
// }


// DdNode* regressBDD(DdNode* src, alt_action* act){
//     cout << "Regressing: " << act->get_name() << " over: " << endl;
//   //  printBDD(src);
//   //  printBDD(act->get_effs()->next->b_eff);

//   alt_effect* tmp_eff = NULL;
//   DdNode *b_sg, *b_ng, *result, *tmp_world, *tmp;
//   simple_goal* sg;
//   neg_goal* ng;

//   result = Cudd_ReadOne(manager);


//   DdNode** minterms = extractDNFfromBDD(src);
//   result = Cudd_ReadLogicZero(manager);
//   Cudd_Ref(result);
//   int k = 0;
//   while(minterms[k] != NULL){
//     //    cout << "k = " << k <<endl;
//     //    printBDD(minterms[k]);
//     tmp_world = Cudd_ReadOne(manager);
//     Cudd_Ref(tmp_world);
//     //for all facts f in src (this could be a pre-process
//     //step where each effect and fact has a bdd for the regression)
//     for(int i = 0; i < num_alt_facts; i++){
// //       sg = new simple_goal(new proposition(alt_facts[i]), E_POS);
// //       ng = new neg_goal(new simple_goal(new proposition(alt_facts[i]), E_POS));
// //       b_sg = goalToBDD(sg);
// //       b_ng = goalToBDD(ng);

//       b_sg = Cudd_bddIthVar(manager,i);
//       b_ng = Cudd_Not(Cudd_bddIthVar(manager,i));
// //  //      Cudd_Ref(b_sg);
// // //       Cudd_Ref(b_ng);


//       //      cout << "i = " << i <<endl;

//       if(bdd_entailed(manager, minterms[k], b_sg)){
// 	//	cout << "Looking for support for " <<endl;
// 	//	printFact(i); cout <<endl;
// 	tmp = regressFact(i, act);
// 	Cudd_Ref(tmp);
// 	//	cout << "got from regressFact, tmp = " <<endl;
// 	//	printBDD(tmp);
// 	//	cout <<   "tmp_world = " << endl;

// 	if(bdd_isnot_one(manager, Cudd_Not(tmp))){

// 	  //printBDD(tmp_world);
// 	  tmp = bdd_and(manager, tmp, tmp_world);
// 	  //Cudd_Ref(tmp);
// 	  Cudd_RecursiveDeref(manager, tmp_world);
// 	  tmp_world = tmp;
// 	  //Cudd_Ref(tmp_world);


// 	  //cout <<   "tmp_world = " << endl;
// 	  //printBDD(tmp_world);
// 	}

// 	//	tmp = bdd_or(manager, result, tmp_world);
// 	//	Cudd_Ref(tmp);
// 	//	Cudd_RecursiveDeref(manager, tmp_world);
// 	//	result = tmp;
// 	//	Cudd_Ref(result);

// 	//	cout << "tmp, Result = " << endl;
// 	//	printBDD(result);


//       }
//      if(bdd_entailed(manager, minterms[k], b_ng)){
//        //	cout << "Looking for support for NEG" <<endl;
//        //        printFact(i); cout <<endl;
//         tmp = Cudd_Not(regressFact(i, act));
// 	//Cudd_Ref(tmp);
// 	//	cout << "got from regressFact, tmp = " <<endl;
// 	//        printBDD(tmp);


// 	if(bdd_isnot_one(manager, Cudd_Not(tmp))){
// 	  tmp = bdd_and(manager, tmp, tmp_world);
// 	  //Cudd_Ref(tmp);
// 	  Cudd_RecursiveDeref(manager, tmp_world);
// 	  tmp_world = tmp;
// 	  //Cudd_Ref(tmp_world);
// 	}
//       }
//     }
//     //and in enabling  precond
//     if(act->get_effs()->b_pre){
//       tmp = bdd_and(manager, tmp_world, act->get_effs()->b_pre);
//       Cudd_Ref(tmp);
//       Cudd_RecursiveDeref(manager, tmp_world);
//       tmp_world = tmp;
//       Cudd_Ref(tmp_world);

//     }

//     tmp = bdd_or(manager, result, tmp_world);
//     Cudd_Ref(tmp);
//     Cudd_RecursiveDeref(manager, result);
//     result = tmp;
//     Cudd_Ref(result);

//     //    cout << "tmp, Result = " << endl;
//     //    printBDD(result);



// 	k++;
// 	Cudd_RecursiveDeref(manager, tmp_world);

//   }

//     cout << "Result = " << endl;
//   printBDD(result);

//   return result;



//   //Insert all bdds from 4 for all facts f in src
//   //for each minterm of src
//   //  for each fact of minterm
//   //    if pos -> And 4(f)
//   //    if neg -> And !4(f)
//   //    else -> noop
//   //  Or minterm





//   // return Cudd_ReadOne(manager);
// }






int isOneOf(FtNode* a, FtNode* b){
	//     cout << "IsOneOF: " << a->index  << " " << a->positive << " " << b->index << " " << b->positive << endl;

	if(!oneofBitVecs)
		return FALSE;

	std::list<FactInfoPair*>::iterator i = oneofBitVecs->begin();

	for(;i != oneofBitVecs->end(); ++i){
		//     print_BitVector((*i)->positive->vector, gft_vector_length);
		//     print_BitVector((*i)->negative->vector, gft_vector_length);
		//     cout << endl;


		if(a->positive && b->positive){
			if(!(get_bit((*i)->positive->vector, gft_vector_length, a->index) &&
					get_bit((*i)->positive->vector, gft_vector_length, b->index)) ) {
				continue;
			}
			else{
				// 	cout << get_bit((*i)->positive->vector, gft_vector_length, a->index) << endl;
				// 	cout << get_bit((*i)->positive->vector, gft_vector_length, b->index) << endl;

				return TRUE;
			}
		}
		else if(a->positive && !b->positive){
			if(get_bit((*i)->positive->vector, gft_vector_length, a->index) &&
					get_bit((*i)->negative->vector, gft_vector_length, b->index)) {
				return TRUE;
			}
			else
				continue;
		}
		else if(!a->positive && b->positive){
			if(get_bit((*i)->negative->vector, gft_vector_length, a->index) &&
					get_bit((*i)->positive->vector, gft_vector_length, b->index)) {
				return TRUE;
			}
			else
				continue;
		}
		else if(!a->positive && !b->positive){
			if(get_bit((*i)->negative->vector, gft_vector_length, a->index) &&
					get_bit((*i)->negative->vector, gft_vector_length, b->index)) {
				return TRUE;
			}
			else
				continue;
		}
		else
			continue;
	}
	//    cout << "NO" <<endl;
	return FALSE;

}


//action_list_node* rifo(action_list_node* acts, clausalState* goal_state) {

//   clausalState* cs = 0;
//   RelaxedPlan* support = 0;
//   action_list_node* tmp_to_remove = 0;;
//   action_list_node* tmp_prev = 0;
//   action_list_node* return_acts = acts;
//   RelaxedPlan* tmpRPs = 0;


//   int tmp_plan_length;
//   int act_ok = FALSE;
//   std::set<OpNode*>::iterator k;

//   for(int i = 0; i < num_graphs; i++) {
//     if(num_graphs > 1)
//       setKGraphTo(i);

//     tmp_plan_length = getlevelNonMutex(goal_state, cs);
//     if(num_graphs > 1){
//       // tmpRP = getRelaxedPlan(tmp_plan_length, cs, TRUE, tmpRPs);
//       k_graphs[i]->relaxed_plan = getRelaxedPlan(tmp_plan_length, cs, TRUE);
//       //cs->display();
//       //k_graphs[i]->relaxed_plan->display();
//     }
//     else
//       support = getRelaxedPlan(tmp_plan_length, cs, TRUE);
//   }

//   while(acts) {

//     //cout << "Checking action: " << acts->act->get_name() << endl;

//     for(int i = 0; i < num_graphs; i++) {
//       if(num_graphs > 1) {
// 	support = /*tmpRPs;*/k_graphs[i]->relaxed_plan;
// 	//tmpRPs = tmpRPs->next;

//       }

//       //cout << "RP length = " << support->plan_length << endl;
//       for(int j = 0; j < support->plan_length; j++){
// 	//cout << "Checking level " << j <<endl;
// 	k = support->action_levels[j]->begin();
// 	//cout << "Size of level = " <<support->action_levels[j]->size() << endl;
// 	for(;k != support->action_levels[j]->end();++k) {
// 	  //cout << "Against: " << (*k)->name << " in RP" <<endl;
// 	  if(strcmp((*k)->name, acts->act->get_name()) == 0){
// 	    act_ok = TRUE;
// 	    //cout <<  "Okay to use" <<endl;
// 	    break;
// 	  }
// 	  else{
// 	    //	    cout << acts->act->get_name()<< "Not in RP" <<endl;
// 	  }
// 	}
// 	if(act_ok)
// 	  break;
//       }

//       //if action not present in graph
//       //or
//       //if one of the actions facts is not present in graph
//       // then remove =true;

//       if(act_ok)
// 	break;


//     }


//     //remove action
//     if(!act_ok) {
//       //cout << acts->act->get_name() << " Not in RP" <<endl;
//       /////////////////cout << "removed" <<endl;
//       tmp_to_remove = acts;

//       if(!tmp_prev) { //first in list
// 	//cout << "HEY" <<endl;
// 	acts = acts->next;
// 	return_acts = acts;
// 	tmp_prev = NULL;

//       }
//       else { //not first in list
//  	acts = acts->next;
// 	tmp_prev->next = acts;
//       }
//       delete tmp_to_remove;
//     }
//     else { //keep action

//       tmp_prev = acts;
//       acts = acts->next;
//     }

//     act_ok = FALSE;
//   }


//   if(num_graphs > 1) {
//     for(int i = 0; i < num_graphs; i++) {
//       if( k_graphs[i]->relaxed_plan != NULL) {
// 	delete k_graphs[i]->relaxed_plan ;
// 	k_graphs[i]->relaxed_plan = NULL;
//       }
//     }
//   }

//  //  cout << "RIFO-ed acts: " << endl;
// //   acts = return_acts;
// //   while(acts){
// //     cout << acts->act->get_name() << endl;
// //     acts=acts->next;
// //   }



//   return return_acts;

//}

// int containsFacts(clausalState* st, FactInfoPair* fip) {

//   goal_list::iterator k;
//   conj_goal* cg;
//   clausalState* tmpSt;
//   goal_list* tmp = new goal_list();
//   Integers *i;

//   //  cout << "CONTAINS FACTS?"<< endl;


//   for ( i = fip->positive->indices; i; i = i->next ) {
//     tmp->push_back(new simple_goal(new proposition(alt_facts[i->index]), E_POS));
//   }
//   for ( i = fip->negative->indices; i; i = i->next ) {
//     tmp->push_back(new neg_goal(new simple_goal(new proposition(alt_facts[i->index]), E_POS)));
//   }
//   if(tmp->size() > 1)
//     tmpSt = new clausalState(new conj_goal(tmp));
//   else
//     tmpSt = new clausalState(tmp->front());

//   // tmpSt->sortClauses(TRUE);
//  //  tmpSt->display();
// //   st->display();


//   //if(isSubset(st->getClauses(), tmpSt->getClauses())){
//    if(tmpSt->satisfiedBy(st)) {
// //     cout << "YES" <<endl;
//   return TRUE;

//   }
//   else {
//     //    cout <<"NO"<< endl;
//     return FALSE;
//   }

// }

// clausalState*  pickInitStatesDeterHelper(clausalState* in, int count, int inSize, FactInfoPair* init_matches) {
//   //cout << "PICK" << endl;

//   clausalState* plast = in;
//   clausalState* pthis = in;
//   clausalState* return_list = NULL;


//   if(count > 0 && in != NULL) {
//     int rnd = 1;
//     int i = 1;
//     while(i != rnd) {
//       plast=pthis;
//       pthis=pthis->next;
//       i++;
//     }
//     if(rnd == 1)
//       in = in->next;
//     plast->next = pthis->next;
//     pthis->next = NULL;
//     return_list = pthis;


//     if(containsFacts(return_list, init_matches)){
//       return_list = pthis;
//       return_list->next = pickInitStatesDeterHelper(in, count-1, inSize-1, init_matches);
//     }
//     else
//       return_list = pickInitStatesDeterHelper(in, count, inSize-1, init_matches);


//   }
//  //  else if(count == 0 && in != NULL) {
// //     clausalState* tmp = in;
// //     while(in) {
// //       tmp = in;
// //       in = in->next;
// //       //     delete tmp;
// //     }
// //   }ccccccccccccccccccc

// //   cout << "RETURNING " <<endl;
// //   if(return_list)
// //     return_list->display();
// //   else
// //     cout << "NULL"<< endl;
//   return return_list;

// }

// clausalState* pickInitStatesDeter(clausalState* in, int count, int inSize, clausalState* initial_state, clausalState* goal_state, action_list acts) {


//   //build single flat graph (all props) until level off
//   //regress relaxed plan from goals to come in from last action layer's effects
//   //take props in init layer that support the relaxed plan
//   //build count number of graphs, only for states that contain the props
//   int levs = 0;
//   int totalgp = 0;
//   clausalState* return_list = NULL;
//     RelaxedPlan* tmp_plan = NULL;

//   cout << "PICK DETER" <<endl;

//   totalgp = build_planning_graph(initial_state, goal_state, acts, levs);

//   cout << "GOT GRAPH" <<endl;

//   //tmp_plan = new RelaxedPlan(IPP_MAX_PLAN, NULL);
//   tmp_plan = getRelaxedPlan(totalgp, goal_state, FALSE);

//    print_FactInfo(tmp_plan->goal_levels[0]->front()->positive);
//    print_FactInfo(tmp_plan->goal_levels[0]->front()->negative);

//    return_list = pickInitStatesDeterHelper(in, count, inSize, tmp_plan->goal_levels[0]->front());


//    //   cout << "RETURNING " <<endl;
//    if(return_list)
//      return_list->display();


//    // exit(0);


//   //when done with graph do this
//    gops_with_unactivated_effects_pointer = NULL;
//    gsame_as_prev_flag = FALSE;
//     new_plan = TRUE;
//     gbit_operators = NULL;
//     gall_ops_pointer = NULL;
//     gprev_level_ops_pointer = NULL;
//     gops_exclusions_count = 0;
//     gop_mutex_pairs = NULL;
//     gall_fts_pointer = NULL;
//     gprev_level_fts_pointer = NULL;
//     gft_mutex_pairs = NULL;


//   return return_list;
// }


// clausalState* pickInitStatesNondeter(clausalState* in, int count, int inSize) {
//   //cout << "PICK" << endl;

//   clausalState* plast = in;
//   clausalState* pthis = in;
//   clausalState* return_list = NULL;


//   if(count > 0 && in != NULL) {
//     int rnd = 1+(int) (inSize*rand()/(RAND_MAX+1.0));
//     int i = 1;
//     while(i != rnd) {
//       plast=pthis;
//       pthis=pthis->next;
//       i++;
//     }
//     if(rnd == 1)
//       in = in->next;
//     plast->next = pthis->next;
//     pthis->next = NULL;
//     return_list = pthis;


//     return_list->next = pickInitStatesNondeter(in, count-1, inSize-1);

//   }
//  //  else if(count == 0 && in != NULL) {
// //     clausalState* tmp = in;
// //     while(in) {
// //       tmp = in;
// //       in = in->next;
// //       //     delete tmp;
// //     }
// //   }ccccccccccccccccccc

// //   cout << "RETURNING " <<endl;
// //   if(return_list)
// //     return_list->display();
// //   else
// //     cout << "NULL"<< endl;
//   return return_list;

// }


// clausalState* DDtoCS(DdNode* dd){
//   //conjunctive clausal states only right now
//   // cout <<"YO"<<endl;
//   clausalState* returnState;// = new clausalState();
//   goal_list* tmpList = new goal_list();

//   //    printBDD(dd);
//   for(int i = 0; i < num_alt_facts; ++i) {
//     if(bdd_is_one(manager, Cudd_Not(bdd_and(manager, Cudd_Not(Cudd_ReadVars(manager, i)), dd)))){

//       tmpList->push_back(new simple_goal(new proposition(alt_facts[i]), E_POS));

//     }
//     else if(bdd_is_one(manager, Cudd_Not(bdd_and(manager, Cudd_ReadVars(manager, i), dd)))){
//       tmpList->push_back(new neg_goal(new simple_goal(new proposition(alt_facts[i]), E_POS)));
//     }
//     else{

//       //    initial_state->display();
//          if(initial_state->inClauseList(alt_facts[i]->get_name(), TRUE)){
// 	   //	   cout << "HO"<<endl;
// 	   tmpList->push_back(new simple_goal(new proposition(alt_facts[i]), E_POS));
// 	 }
//          else if(initial_state->inClauseList(alt_facts[i]->get_name(), FALSE))
//             tmpList->push_back(new neg_goal(new simple_goal(new proposition(alt_facts[i]), E_POS)));
// 	 //   cout<< "BYE"<<endl;
//     }
//     //    if(bdd_is_one(manager, bdd_and(manager, Cudd_Not(Cudd_bddIthVar(manager, i)), dd)))
//       //      tmpList->push_back(new neg_goal(new simple_goal(new proposition(alt_facts[i]), E_POS)));
//   }
//   returnState = new clausalState(new conj_goal(tmpList));
//   // returnState->display();
//   return returnState;

// }

// clausalState* DDDNFtoCSs(DdNode** initDDDNF){
//   clausalState* returnList = NULL;
//   clausalState* tmpList= NULL;

//   //  cout << "DDDNFStoCSs" <<endl;

//   DdNode* tmp = initDDDNF[0];
//   int i = 0;

//   while(tmp) {
//     //   cout << "MAKING STATE " << i<< endl;
//     tmpList = returnList;
//     returnList = DDtoCS(tmp);
//     returnList->next = tmpList;
//     tmp = initDDDNF[++i];
//   }
//   return returnList;
// }



void pickKRandomWorlds(DdNode* dd, int k, list<DdNode*>* worlds){
	//  cout << "picking " << k << " random worlds" << endl;
	list<DdNode*> cubes;
	list<DdNode*>::iterator c;
	DdNode *cube;

	(RANDOM_SUBSTRATE == RANDOM_STATES ?
			get_states_from_bdd(dd, &cubes) :
			get_bdd_cubes(dd, &cubes));

	int num_cubes = cubes.size();
	int build_num = k;
	int i;

	for(int p = 0; p < num_cubes && p < k; p++){
		i = rand()%cubes.size();

		c = cubes.begin();
		for(int j = 0; j < i; j++)
			c++;
		// printBDD(*c);
		worlds->push_back(*c);
		cubes.remove(*c);
	}

	for(c = cubes.begin(); c != cubes.end(); c++)
		Cudd_RecursiveDeref(manager, *c);

	//  cout << "done picking worlds."<<endl;

}


DdNode* pickKRandomWorlds(DdNode* dd, int k){
	list<DdNode*> my_list;
	DdNode* tmp;
	pickKRandomWorlds(dd, k, &my_list);
	//  cout << "size = " << my_list.size() << endl;
	DdNode* worlds = Cudd_ReadLogicZero(manager); //won't work for adds
	for(list<DdNode*>::iterator i = my_list.begin();
			i != my_list.end(); i++){
		//    printBDD(*i);
		tmp = Cudd_bddOr(manager, worlds, *i);
		Cudd_Ref(tmp);
		Cudd_RecursiveDeref(manager, worlds);
		Cudd_RecursiveDeref(manager, *i);
		worlds = tmp;
		Cudd_Ref(worlds);
		Cudd_RecursiveDeref(manager, tmp);

	}
	return worlds;

}


#ifdef PPDDL_PARSER
int build_k_planning_graphs(DdNode* init, DdNode* goal, int& k) {
	int size = 0;
	int k_gp_time = 0;
	int gp_time = 0;
	int levs = 0;
	int i = 0;
	int j;


	list<DdNode*> cubes;
	DdNode *cube;
	int num_cubes;
	// 抽样选取初始状态
	if(RANDOM_SUBSTRATE == RANDOM_STATES ?
			num_cubes = get_sum(init) :
			num_cubes = Cudd_CountMinterm(manager,
					init,
					Cudd_SupportSize(manager, init)));
	std::cout << "num_cubes: " << num_cubes << endl;

	int build_num = (NUMBER_OF_MGS < 1.0 ?
			(NUMBER_OF_MGS == 0.0 ?
					num_cubes : //all graphs
	(int)ceil(num_cubes*NUMBER_OF_MGS)) : //build proporation
	NUMBER_OF_MGS);  //build this number

	// std::cout << "build_num: " << build_num << endl;

	pickKRandomWorlds(init, build_num, &cubes);

	gft_vector_length =  ( ( int ) num_alt_facts / gcword_size )+1;


	//  cout << "|cubes| = " << cubes.size() <<endl;

	num_graphs = 0;
	// 考虑为这些初始状态计算planning graph
	while((NUMBER_OF_MGS >= 1.0 &&
			0 < cubes.size() &&
			num_graphs < build_num) ||
			(NUMBER_OF_MGS < 1.0 &&
					num_graphs < build_num)){

		cube = cubes.front();
		cubes.remove(cube);


		// cout << "build mg " << num_graphs << " of " << build_num <<endl;
		// printBDD(cube);

		levs = 0;
		// MoMo007 2022.09.05 修复build_planning_graph
		// 根据当个状态, goal,构建graph，返回最终层次levs
		gp_time = build_planning_graph(cube, goal, levs);
		k_gp_time += gp_time;
		// 完成build_planning_graph更新相应的全局变量
		// 并存储到k_graph进行后续处理
		k_graphs[num_graphs] = new kGraphInfo(gft_table, levs,
												gft_mutex_pairs,
												gall_fts_pointer,
												gall_ops_pointer,
												gall_efs_pointer,
												cube);

		// 考虑graph每一层，配置op,pos/neg facts
		for( j = 0; j < levs; j++){
			k_graphs[num_graphs]->op_vector_length_at[j] = gop_vector_length_at[j];
			k_graphs[num_graphs]->pos_facts_vector_at[j] =
					copy_bit_vector(gpos_facts_vector_at[j],gft_vector_length  );
			k_graphs[num_graphs]->neg_facts_vector_at[j] =
					copy_bit_vector(gneg_facts_vector_at[j], gft_vector_length);
		}
		// std::cout << "######outside#############\n";
		// 修复getlevel
		// 考虑每一个fact，计算该fact所在的最小层数。还差一个接口,getlevel，获取命题为true或false的最小层。
		for(int l = 0; l < num_alt_facts; l++) {
			k_graphs[num_graphs]->pos_fact_at_min_level[l] =
					pos_fact_at_min_level[l];
		    // cout << "POS " << l << "at" << k_graphs[num_graphs]->pos_fact_at_min_level[l] <<endl;
			k_graphs[num_graphs]->neg_fact_at_min_level[l] =
					neg_fact_at_min_level[l];
		    // cout << "NEG " << l << "at" << k_graphs[num_graphs]->neg_fact_at_min_level[l] <<endl;
		}
		// 将全局变量制空
		gops_with_unactivated_effects_pointer = NULL;
		gsame_as_prev_flag = FALSE;
		new_plan = TRUE;
		gall_ops_pointer = NULL;
		gprev_level_ops_pointer = NULL;
		gops_exclusions_count = 0;
		gop_mutex_pairs = NULL;
		gall_fts_pointer = NULL;
		gprev_level_fts_pointer = NULL;
		gft_mutex_pairs = NULL;
		gft_table = NULL;
		num_graphs++;  //number of graphs built
		Cudd_RecursiveDeref(manager, cube);
	}

	//set max_k_levs
	// 统计kgraph结构的数据
	int z = 0;
	for(int f = 0; f<num_alt_facts; f++){// 从所有图中计算得到最大的min_level
		max_k_pos_fact_at_min_level[f] = -1;
		max_k_neg_fact_at_min_level[f] = -1;
		for(z = 0; z<num_graphs; z++) {
			if(max_k_pos_fact_at_min_level[f] == -1 ||
					max_k_pos_fact_at_min_level[f] <
					k_graphs[z]->pos_fact_at_min_level[f])
				max_k_pos_fact_at_min_level[f] = k_graphs[z]->pos_fact_at_min_level[f];
		}
		for(z = 0; z<num_graphs; z++) {
			if(max_k_neg_fact_at_min_level[f] == -1 ||
					max_k_neg_fact_at_min_level[f] <
					k_graphs[z]->neg_fact_at_min_level[f])
				max_k_neg_fact_at_min_level[f] = k_graphs[z]->neg_fact_at_min_level[f];
		}
	}
	return k_gp_time;

}


int build_planning_graph(DdNode* init, DdNode* goal, int& levs) {
	BitOperator *tmp;
	Effect* tmpe, *tmpe1;
	int okay;
	int num_ops = 0;

	b_initial_state = init;

	int j = 0;

	FactInfo *tpos = new_FactInfo();// 创建一个bit_vector，并表头Integer置为空。
	FactInfo *tneg = new_FactInfo();
	DdNode* support;


	support = Cudd_Support(manager, init);// 返回 BDD的变量的合取式
	Cudd_Ref(support);
	// 考虑每个命题
	// printBDD(support);
	printf("%d\n",dynamic_atoms.size());
	for(int i = 0; i < num_alt_facts; i++){
		// 对命题的正负进行编码到二进制FactInfo
		// dynamic_atoms[i]->print(cout, my_problem->domain().predicates(),my_problem->domain().functions(), my_problem->terms());
		// printBDD(Cudd_bddIthVar(manager, i*2));
		if(bdd_entailed(manager, support, Cudd_bddIthVar(manager, i*2))){
			if(Cudd_bddIsVarEssential(manager, init, i*2, 1)){
				// std::cout << "is pos\n";
				make_entry_in_FactInfo(&tpos, i); //连接到FactInfo List中
			}
			else if(Cudd_bddIsVarEssential(manager, init, i*2, 0)){
				// std::cout << "is neg\n";
				make_entry_in_FactInfo( &tneg, i);
			}
			else{// uncertain
				// std::cout << "is uncertain\n";
				make_entry_in_FactInfo( &tpos, i);
				make_entry_in_FactInfo( &tneg, i);
			}
		}
		else{
			// cout << "not in\n";
		}
			
	}
	Cudd_RecursiveDeref(manager,support);
	// 那到初始状态所有命题的真假情况
	gbit_initial_state = new_fact_info_pair( tpos, tneg );

	tpos = new_FactInfo();
	tneg = new_FactInfo();

	// 类是操作对Goal进行
	support = Cudd_Support(manager, goal);
	Cudd_Ref(support);
	// printBDD(goal);
	for(int i = 0; i < num_alt_facts; i++){
		// dynamic_atoms[i]->print(cout, my_problem->domain().predicates(),my_problem->domain().functions(), my_problem->terms());
		// printBDD(Cudd_bddIthVar(manager, i*2));
		if(bdd_entailed(manager, support, Cudd_bddIthVar(manager, i*2))){
			if(Cudd_bddIsVarEssential(manager, goal, i*2, 1)){
				make_entry_in_FactInfo( &tpos, i);
				// std::cout << "is pos\n";
			}
			else if(Cudd_bddIsVarEssential(manager, goal, i*2, 0)){
				make_entry_in_FactInfo( &tneg, i);
				// std::cout << "is neg\n";
			}
			else{
				// std::cout << "is uncertain\n";
				make_entry_in_FactInfo( &tpos, i);
				make_entry_in_FactInfo( &tneg, i);
			}
		}
		else{
			// cout << "not in\n";
		}
	}
	Cudd_RecursiveDeref(manager,support);
	// 拿到目标状态所有命题的真假情况,这里和InitLUG是否重复？
	gbit_goal_state = new_fact_info_pair( tpos, tneg );
	my_gbit_goal_state = gbit_goal_state;
	// 预处理 operators，处理后的operator加入到 gbit_operators中
	while(used_gbit_operators){// 第一次不会进入
		tmp = used_gbit_operators;
		used_gbit_operators = used_gbit_operators->next;
		tmpe = tmp->conditionals;// 获取condtion
		// 下面将activated condtion不重复的加入到condtion中
		while( tmp->activated_conditionals){// 存在 activated condition
			tmpe = tmp->activated_conditionals;// 取出activated condition
			tmp->activated_conditionals = tmp->activated_conditionals->next;
			okay = TRUE;
			tmpe1 = tmp->conditionals;
			while(tmpe1){// 检测 activated condition是否在condition中
				if(tmpe1->index == tmpe->index){
					okay = FALSE;// 标记
				}
				tmpe1 = tmpe1->next;
			}
			if(okay){// activated condtion未包含在condtion中
				tmpe->next = tmp->conditionals;// activate前插法加入condtion
				tmp->conditionals = tmpe; // 成为新的condtion
			}
		}
		// 加该动作加入到 gbit_operators list中
		tmp->next = gbit_operators;
		gbit_operators = tmp;
	}
	// 遍历gbit_operators
	tmp = gbit_operators;
	while(tmp){
		tmpe = tmp->conditionals;
		while(tmpe){// 这块没统计前提条件
			tmpe=tmpe->next;
		}
		tmp = tmp->next;
		num_ops++;// op计数
	}
	string strMutex;
	switch (MUTEX_SCHEME)
	{
	case MS_CROSS:
		strMutex = "MS_CROSS\n";
		break;
	case MS_REGULAR:
		strMutex = "MS_REGULAR\n";
		break;
	case MS_NONE:
		strMutex = "MS_NONE\n";
		break;
	case MS_STATIC:
		strMutex = "MS_STATIC\n";
		break;
	}
	// cout << "MUX SCh = " << strMutex;
	// cout << "allow level off:" << ALLOW_LEVEL_OFF << endl;
	// 根据 init, goal, opt的 BitVector进行构造graph
	// 此时BitOpVector还有点bug，关于effect的编码
	int reached_goals =  build_graph(&j, num_alt_facts, ALLOW_LEVEL_OFF, MUTEX_SCHEME );

	graph_levels = j;
	levs = j;
	// cout <<"LEVELS = " << j << endl;
	//  cout << "YO HERES THE GRAPH" <<endl;
	//     for(int i = 0; i <= j; i++){
	// 		cout << "LEVEL " << i << endl;
	//     	print_BitVector(gpos_facts_vector_at[i],gft_vector_length);
	//     	print_BitVector(gneg_facts_vector_at[i],gft_vector_length);
	// 		cout << endl;
	// 	for(OpNode* k = gall_ops_pointer; (k) ; k=k->next)
	//  		if(k->info_at[i] && k->name && !k->is_noop )cout << k->name << endl;
	//       		cout << endl<< "-------------------------------------------------" <<endl;
	//     }

	if(!reached_goals) {// 没有到达目标
		cout << "Goals not reached in PG"<<endl;
	}

	for(int n = 0; n<num_alt_facts; n++){
		pos_fact_at_min_level[n] = getlevel(n, TRUE);
		neg_fact_at_min_level[n] = getlevel(n, FALSE);
	}
	return j;

}
#else
int build_k_planning_graphs(DdNode* init, DdNode* goal, action_list acts, int& k) {

	int size = 0;
	int k_gp_time = 0;
	int gp_time = 0;
	int levs = 0;
	int i = 0;
	int j;


	list<DdNode*> cubes;
	DdNode *cube;
	int num_cubes;
	//   cout << "["<<endl;
	//     Cudd_CheckKeys(manager);
	//     cout << "|" <<endl;
	//     Cudd_CheckKeys(manager);
	//       cout << "]"<<endl;

	if(RANDOM_SUBSTRATE == RANDOM_STATES ?
			num_cubes = get_sum(init) :
			num_cubes = Cudd_CountMinterm(manager,
					init,
					Cudd_SupportSize(manager, init)));

	int build_num = (NUMBER_OF_MGS < 1.0 ?
			(NUMBER_OF_MGS == 0.0 ?
					num_cubes : //all graphs
	(int)ceil(num_cubes*NUMBER_OF_MGS)) : //build proporation
	NUMBER_OF_MGS);  //build this number

	pickKRandomWorlds(init, build_num, &cubes);

	gft_vector_length =  ( ( int ) num_alt_facts / gcword_size )+1;


	//  cout << "|cubes| = " << cubes.size() <<endl;

	num_graphs = 0;
	while((NUMBER_OF_MGS >= 1.0 &&
			0 < cubes.size() &&
			num_graphs < build_num) ||
			(NUMBER_OF_MGS < 1.0 &&
					num_graphs < build_num)){

		cube = cubes.front();
		cubes.remove(cube);


		//     cout << "build mg " << num_graphs << " of " << build_num <<endl;
		//     printBDD(cube);

		levs = 0;
		gp_time = build_planning_graph(cube, goal, acts, levs);
		k_gp_time += gp_time;
		k_graphs[num_graphs] = new kGraphInfo(gft_table, levs,
				gft_mutex_pairs,
				gall_fts_pointer,
				gall_ops_pointer,
				gall_efs_pointer,
				cube);

		for( j = 0; j < levs; j++){
			k_graphs[num_graphs]->op_vector_length_at[j] = gop_vector_length_at[j];
			k_graphs[num_graphs]->pos_facts_vector_at[j] =
					copy_bit_vector(gpos_facts_vector_at[j],gft_vector_length  );
			k_graphs[num_graphs]->neg_facts_vector_at[j] =
					copy_bit_vector(gneg_facts_vector_at[j], gft_vector_length);
		}
		for(int l = 0; l < num_alt_facts; l++) {
			k_graphs[num_graphs]->pos_fact_at_min_level[l] =
					getlevel(alt_facts[l] ,TRUE);
			k_graphs[num_graphs]->neg_fact_at_min_level[l] =
					getlevel(alt_facts[l] ,FALSE);
		}

		gops_with_unactivated_effects_pointer = NULL;
		gsame_as_prev_flag = FALSE;
		new_plan = TRUE;
		gall_ops_pointer = NULL;
		gprev_level_ops_pointer = NULL;
		gops_exclusions_count = 0;
		gop_mutex_pairs = NULL;
		gall_fts_pointer = NULL;
		gprev_level_fts_pointer = NULL;
		gft_mutex_pairs = NULL;
		gft_table = NULL;
		//Cudd_RecursiveDeref(manager, cube);
		num_graphs++;  //number of graphs built
		Cudd_RecursiveDeref(manager, cube);
	}

	//set max_k_levs
	int z = 0;
	for(int f = 0; f<num_alt_facts; f++){
		max_k_pos_fact_at_min_level[f] = -1;
		max_k_neg_fact_at_min_level[f] = -1;
		for(z = 0; z<num_graphs; z++) {
			if(max_k_pos_fact_at_min_level[f] == -1 ||
					max_k_pos_fact_at_min_level[f] <
					k_graphs[z]->pos_fact_at_min_level[f])
				max_k_pos_fact_at_min_level[f] = k_graphs[z]->pos_fact_at_min_level[f];
		}
		for(z = 0; z<num_graphs; z++) {
			if(max_k_neg_fact_at_min_level[f] == -1 ||
					max_k_neg_fact_at_min_level[f] <
					k_graphs[z]->neg_fact_at_min_level[f])
				max_k_neg_fact_at_min_level[f] = k_graphs[z]->neg_fact_at_min_level[f];
		}
	}



	return k_gp_time;
}
int build_planning_graph(DdNode* init, DdNode* goal, action_list acts, int& levs) {
	BitOperator *tmp;
	Effect* tmpe, *tmpe1;
	int okay;
	int num_ops = 0;

	b_initial_state = init;
	//b_goal_state = goal;
	//generate_bitmap_representation(init, goal, acts);
	// generate_ini_goal_bitmap_representation(init, goal);

	  //cout << "INIT" << endl ;
	  // print_vector(gbit_initial_state->positive->vector,gft_vector_length );
	  // print_vector(gbit_initial_state->negative->vector,gft_vector_length );
	  // cout <<endl <<endl<< "GOAL";
	  // print_vector(gbit_goal_state->positive->vector, gft_vector_length);
	  // print_vector(gbit_goal_state->negative->vector, gft_vector_length);
	  //cout << endl<< "-------------------------------------------------" <<endl;

	int j = 0;
	//free_graph_info();
	// gsame_as_prev_flag = FALSE;
	//	  reset_original_ipp_information();
	// cout << "RESTORING OPERATORS" <<endl;


	FactInfo *tpos = new_FactInfo();
	FactInfo *tneg = new_FactInfo();
	simple_goal* sg;
	conj_goal* cg;
	disj_goal* dg;
	neg_goal* ng;
	DdNode* support;


	support = Cudd_Support(manager, init);
	Cudd_Ref(support);
	for(int i = 0; i < num_alt_facts; i++){
		if(bdd_entailed(manager, support, Cudd_bddIthVar(manager, i))){
			if(Cudd_bddIsVarEssential(manager, init, i, 1)){
				make_entry_in_FactInfo( &tpos, i);
			}
			else if(Cudd_bddIsVarEssential(manager, init, i, 0)){
				make_entry_in_FactInfo( &tneg, i);
			}
			else{
				make_entry_in_FactInfo( &tpos, i);
				make_entry_in_FactInfo( &tneg, i);
			}
		}
	}
	Cudd_RecursiveDeref(manager,support);

	gbit_initial_state = new_fact_info_pair( tpos, tneg );

	tpos = new_FactInfo();
	tneg = new_FactInfo();


	support = Cudd_Support(manager, goal);
	Cudd_Ref(support);
	for(int i = 0; i < num_alt_facts; i++){
		if(bdd_entailed(manager, support, Cudd_bddIthVar(manager, i))){
			if(Cudd_bddIsVarEssential(manager, goal, i, 1)){
				make_entry_in_FactInfo( &tpos, i);
			}
			else if(Cudd_bddIsVarEssential(manager, goal, i, 0)){
				make_entry_in_FactInfo( &tneg, i);
			}
			else{
				make_entry_in_FactInfo( &tpos, i);
				make_entry_in_FactInfo( &tneg, i);
			}
		}
	}
	Cudd_RecursiveDeref(manager,support);

	gbit_goal_state = new_fact_info_pair( tpos, tneg );
	my_gbit_goal_state = gbit_goal_state;

	while(used_gbit_operators){
		tmp = used_gbit_operators;
		used_gbit_operators = used_gbit_operators->next;
		tmpe = tmp->conditionals;
		while( tmp->activated_conditionals){
			tmpe = tmp->activated_conditionals;
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
	}

	tmp = gbit_operators;
	while(tmp){
		tmpe = tmp->conditionals;
		while(tmpe){
			tmpe=tmpe->next;
		}
		tmp = tmp->next;
		num_ops++;
	}

	//	  new_plan = TRUE;

	//  action_list_node* a = acts;


	//    while (a) {

	//     if(a->act->is_original() && !a->act->get_effs()->obs)
	//       generate_BitOperators(a);
	//     a = a->next;
	//   }


	//    cout << "MUX SCh = "<< MUTEX_SCHEME <<endl;
	int reached_goals =  build_graph(&j, num_alt_facts, ALLOW_LEVEL_OFF, MUTEX_SCHEME );

	graph_levels = j;
	levs = j;
	//num_graphs = 1;
	 //   cout <<"LEVELS = " << j << endl;

	 //cout << "YO HERES THE GRAPH" <<endl;
	 //    for(int i = 0; i <= j; i++){
	 //      cout << "LEVEL " << i << endl;
	 //      print_BitVector(gpos_facts_vector_at[i],gft_vector_length);
	 //      print_BitVector(gneg_facts_vector_at[i],gft_vector_length);
	 //      cout << endl;
	 //      for(OpNode* k = gall_ops_pointer; (k) ; k=k->next)
	 //	if(k->info_at[i] && k->name && !k->is_noop )
	 //	  cout << k->name << endl;


	 //      cout << endl<< "-------------------------------------------------" <<endl;
	 //    }

	if(!reached_goals) {
		cout << "Goals not reached in PG"<<endl;
		/// j++;
		//reached_goals =  build_graph(&j, num_alt_facts, ALLOW_LEVEL_OFF );
		//  exit(0);
	}



	for(int n = 0; n<num_alt_facts; n++){

//		    cout <<"FACT AT LEV " << alt_facts[n]->get_name() << " = " << endl;

		pos_fact_at_min_level[n] = getlevel(alt_facts[n] ,TRUE);
//		    cout << "POS " << pos_fact_at_min_level[n] <<endl;
		neg_fact_at_min_level[n] = getlevel(alt_facts[n] ,FALSE);
//		    cout << "NEG " << neg_fact_at_min_level[n] <<endl;
	}

	return j;


}
#endif

int get_bit( BitVector *vec, int vec_len, int pos )

{

	return (vec[pos / gcword_size] & (1 << (pos % gcword_size)));

}

void generate_Factored_BitOperators(action_list);

//goal*
DdNode* get_init_labels(int index, int polarity) {
	//  cout << "get init lablel for " << index << endl;
	if(polarity)
		return init_pos_labels[index];
	else
		return init_neg_labels[index];

}


void printFact(int f) {
#ifdef PPDDL_PARSER
	cout << f << " " << getFactName(f) << endl;
#else
	cout << alt_facts[f]->get_name();
#endif
}
char* getFactName(int f) {
#ifdef PPDDL_PARSER
	ostringstream os (ostringstream::out);
	(*dynamic_atoms.find(f)).second->print(os, my_problem->domain().predicates(),
			my_problem->domain().functions(), my_problem->terms());
	//(*action).first->print(os, (*my_problem).terms());
	//cout << os.str().c_str() << " : " <<endl;
	return (char*)os.str().c_str();
	//return (char*)my_problem->domain().predicates().name((*dynamic_atoms.find(f)).second->predicate()).c_str();
#else
	return alt_facts[f]->get_name();
#endif
}



// CodeNode* buildCodeNode(goal* g) {
//  CodeNode* cn = NULL;

//  CodeNode* tmp = NULL;

//   neg_goal* ng;
//   simple_goal* sg;
//   conj_goal* cg;
//   disj_goal* dg;


//   if((sg = dynamic_cast<simple_goal*>(g))){
//     cn = new_CodeNode(ATOM);
//     cn->predicate = sg->getProp()->ground->getnum();
//   }
//   else if((ng = dynamic_cast<neg_goal*>(g))){
//     cn = new_CodeNode(NOT);
//     cn->sons = buildCodeNode(ng->getGoal());
//   }
//   else if((dg = dynamic_cast<disj_goal*>(g))){
//     cn = new_CodeNode(OR);
//     cn->sons = NULL;
//     // cout << "CREATED OR CODENOED\n";
// //     exit(0);
//     goal_list::iterator k = dg->getGoals()->begin();
//     for (; k != dg->getGoals()->end();++k ) {
//       tmp = cn->sons;
//       cn->sons = buildCodeNode(*k);
//       cn->sons->next = tmp;
//     }
//   }
//   else if((cg = dynamic_cast<conj_goal*>(g))){
//         cn = new_CodeNode(AND);
//     cn->sons = NULL;
//     goal_list::iterator k = cg->getGoals()->begin();
//     for (; k != cg->getGoals()->end();++k ) {
//       tmp = cn->sons;
//       cn->sons = buildCodeNode(*k);
//       cn->sons->next = tmp;
//     }

//   }
//   return cn;

// }

// goal* buildGoal(CodeNode* cn){

//   goal* g = NULL;
//   goal_list* tmp_list = NULL;
//   CodeNode *i_son;

//   // cout << "Building GOAl" <<endl;


//   switch ( cn->connective ) {
//   case NOT:
//     //    cout << "NOT" << endl;
//     g = new neg_goal(buildGoal(cn->sons));
//     break;
//   case ATOM:
//     //cout << "ATOM" << endl;
//     g = new simple_goal(new proposition(alt_facts[cn->predicate]) , E_POS);
//     break;
//   case AND:
//     //cout << "AND" << endl;
//    tmp_list = new goal_list();
//     for (i_son = cn->sons; i_son!=NULL; i_son = i_son->next) {
//       tmp_list->push_back(buildGoal(i_son));
//     }
//     g = new conj_goal(tmp_list);
//     break;
//    case OR:
//      //cout << "OR" << endl;
//      tmp_list = new goal_list();
//      for (i_son = cn->sons; i_son!=NULL; i_son = i_son->next) {
//        tmp_list->push_back(buildGoal(i_son));
//      }
//      g = new disj_goal(tmp_list);
//      break;
//   }
//   return g;
// }
// goal* removeRepeats(goal* g);

// //check if a goal is already in the cnf form because the cnf function tends to do funky stuff if it is.
// int cnfIzed(goal* g) {
//   //not all cases, just the tricky ones
//   simple_goal* sg;
//   neg_goal* ng;
//   conj_goal* cg;
//     disj_goal* dg;

//   if(dg = dynamic_cast<disj_goal*>(g)) {
//     return FALSE;
//   }
//   else
//     return TRUE;

// }


//goal*

#ifdef PPDDL_PARSER
#else
DdNode* getLabelAt(simple_goal* g, int time, int polarity) {
	FtNode *i1;
	//       cout << "Getting label for" << endl;
	//    g->display(0);

	if(time == 0){

		for(int i = 0; i < gnum_relevant_facts; i++) {
			if( alt_facts[i] == g->getProp()->ground) {
				if(polarity)
					return init_pos_labels[i];
				else
					return init_neg_labels[i];
			}

		}
	}
	else { //cout << "gnum = " << gnum_relevant_facts<<endl;
		for (int k=0; k<2*gnum_relevant_facts; k++ ) {
			//      cout << " k = " << k << endl;
			if(k < gnum_relevant_facts)
				i1 = gft_table[k];
			else
				i1 = gft_table[NEG_ADR(k-gnum_relevant_facts)];

			//      if(i1)
			//             cout << "OKAY"<<endl;
			//       else
			//       cout << "NOKAY"<<endl;

			//      printFact(i1->index);
			if(i1 &&
					//alt_facts[i1->index] && printf("HO\n") &&
					// g->getProp()->ground && printf("HO\n") &&
					alt_facts[i1->index] == g->getProp()->ground &&
					i1->info_at[time] &&
					!i1->info_at[time]->is_dummy &&
					i1->info_at[time]->label &&
					(polarity == i1->positive) ) {
				// 		cout << "Label gotten" <<endl;
				//printBDD(i1->info_at[time]->label->label);
				return i1->info_at[time]->label->label;
			}
		}
	}
	//   cout << "FAlse label" << endl;

	return Cudd_ReadLogicZero(manager);//NULL;
}
#endif

/*
int are_there_labels_proven(int time, goal* g) {

  conj_goal* cg;
  disj_goal* dg;
   disj_goal* dg1;
 simple_goal* sg;
  neg_goal* ng;
  int ok = FALSE;
  goal_list::iterator k;
  goal_list::iterator k1;


  //  cout << "Enter are there lables proven" << endl;

  if((cg = dynamic_cast<conj_goal*>(g))){
    // cout << "Cg = " <<endl;
    //cg->display(0);
    k1 = cg->getGoals()->begin();
    for(; k1 != cg->getGoals()->end();++k1 ) {

      //      cout << "checking " << endl;
      //(*k1)->display(0);
      if ((sg = dynamic_cast<simple_goal*>(*k1)) && !(getLabelAt(sg,time))) {}
      else if ((ng = dynamic_cast<neg_goal*>(*k1)) && (sg = dynamic_cast<simple_goal*>(ng->getGoal())) && !(getLabelAt(sg,time))) {}
      //  else if ((sg = dynamic_cast<simple_goal*>(*k1)) && initial_state->inClauseList(sg->getProp()->getGrounding(), TRUE)) {
      else if ((sg = dynamic_cast<simple_goal*>(*k1)) && initial_state->inClauseList(sg->getProp()->getGrounding(), TRUE)) {	//	cout <<"OKAY, in INIT" <<endl;


      }
      else if ((ng = dynamic_cast<neg_goal*>(*k1)) && (sg = dynamic_cast<simple_goal*>(ng->getGoal())) && initial_state->inClauseList(sg->getProp()->getGrounding(), FALSE)) {
	//cout <<"OKAY, in INIT" <<endl;


      }
      else if((sg = dynamic_cast<simple_goal*>(*k1)) &&
	      !(initial_state->inClauseList(sg->getProp()->getGrounding(), TRUE) ) &&
	      //      (getLabelAt(sg,time))->operator==(init_label)
	      ddEqualVal(getLabelAt(sg,time), init_label, 0)  ) {
	//	cout << "MATCHED" <<endl;

      }
      else if((ng = dynamic_cast<neg_goal*>(*k1))) {
	if((sg = dynamic_cast<simple_goal*>(ng->getGoal())) &&
	   !(initial_state->inClauseList(sg->getProp()->getGrounding(), FALSE) ) &&
	   //(getLabelAt(sg,time))->operator==(init_label)
	   ddEqualVal(getLabelAt(sg,time), init_label, 0) ) {}
	  //cout << "MATCHED" <<endl;
      }
      else {
	//	cout << "BAD MATCH" <<endl;
	return FALSE;
      }
    }
    return TRUE;
  }
  else if((sg = dynamic_cast<simple_goal*>(g))){
    //    if((getLabelAt(sg,time))->operator==(init_label) || !(getLabelAt(sg,time)) )
    if(isSubset(getLabelAt(sg,time),init_label) || !(getLabelAt(sg,time)) )
    return TRUE;
  }
  else {
    cout << "NEED new case" <<endl;
    exit(0);
  }


  return FALSE;



}
 */

// void removeSatClauses(goal* g, goal_list* disj_clauses) {

//   simple_goal* sg;
//   neg_goal* ng;
//   disj_goal* dg;

//   goal_list::iterator i = disj_clauses->begin();
//   goal_list::iterator j;

//   //cout << "Removing Clauses with: " <<endl;
//   // g->display(0);

//   // cout << "From Clause list: " << endl;
//   //disj_clauses->display(0);


//   if (ng = dynamic_cast<neg_goal*>(g)) {
//     for(;i!=disj_clauses->end();++i) {
//       if(dg = dynamic_cast<disj_goal*>(*i)){
// 	j = dg->getGoals()->begin();
// 	for(;j!= dg->getGoals()->end();++j){
// 	  if(ng->operator==(*j)){
// 	    disj_clauses->remove(*i);
// 	     i = disj_clauses->begin();
// 	    break;
// 	  }
// 	}
//       }
//     }
//   }
//   else if (sg = dynamic_cast<simple_goal*>(g)) {
//     for(;i!=disj_clauses->end();++i) {
//       if(dg = dynamic_cast<disj_goal*>(*i)){
// 	j = dg->getGoals()->begin();
// 	for(;j!= dg->getGoals()->end();++j){
// 	  if(sg->operator==(*j)){
// 	    disj_clauses->remove(*i);
// 	    i = disj_clauses->begin();
// 	    break;
// 	  }
// 	}
//       }
//     }
//   }
//   //return disj_clauses;

//   //  cout << "ENDED WITH: " <<endl;
//   //disj_clauses->display(0);
// }

// clausalState* chooseDisjunctsForConjunctiveState(goal_list* unit_clauses, goal_list* disj_clauses) {

//   simple_goal* sg;
//   neg_goal* ng;
//   disj_goal* dg;
//   int SET_UNIT = FALSE;
//   clausalState* cs = NULL;
//   clausalState* cs1 = NULL;

//   //      cout << "HO" <<endl;
// //       disj_clauses->display(1);


//   if(disj_clauses->size() == 0) {
//     return new clausalState(new conj_goal(unit_clauses));

//   }
//   else{
//     if(dg = dynamic_cast<disj_goal*>(disj_clauses->back())) {
//       disj_clauses->pop_back();
//       //disj_clauses->pop_back();
//       goal_list::iterator i = dg->getGoals()->end();
//       for(;i!=dg->getGoals()->begin();--i){
// 	if(sg = dynamic_cast<simple_goal*>(*i)) {
// 	  //dg->getGoals()->pop_front();
// 	  unit_clauses->push_back(sg);
// 	  cs = new clausalState(new conj_goal(unit_clauses));
// 	  if(getlevelNonMutex(cs, cs1) < IPP_MAX_PLAN) {
// 	    //cout << "got level" <<endl;
// 	    if(disj_clauses->size() > 0)
// 	      removeSatClauses(sg, disj_clauses);

// 	    SET_UNIT=TRUE;
// 	    break;
// 	  }
// 	  else
// 	    unit_clauses->remove(sg);

// 	  //break;
// 	}
// 	else if (ng = dynamic_cast<neg_goal*>(*i)) {
// 	  //dg->getGoals()->pop_front();
// 	  if (sg = dynamic_cast<simple_goal*>(ng->getGoal())) {
// 	    unit_clauses->push_back(ng);
// 	    cs = new clausalState(new conj_goal(unit_clauses));
// 	    if(getlevelNonMutex(cs, cs1) < IPP_MAX_PLAN) {
// 	      //cout << "got level" <<endl;
// 	      if(disj_clauses->size() > 0)
// 		removeSatClauses(ng, disj_clauses);
// 	      SET_UNIT=TRUE;
// 	      break;
// 	    }
// 	    else
// 	      unit_clauses->remove(ng);

// 	    //break;
// 	  }
// 	}
//       }
//     }
//     if(SET_UNIT)
//       return chooseDisjunctsForConjunctiveState(unit_clauses, disj_clauses);
//     else{
//       //   cout << "THERE IS  A PROBLEM"<<endl;
//       return NULL;
//       //      exit(0);
//     }
//   }


// }

DdNode* generateUniqueNonDeterLabel(int time, char* opname, int effnum,  int ndnum){

	//generate a new label for a non deter effect and
	//add the new variable to the Dd manager.
	DdNode *tmp, *newDD;




	if(NDLcounter < NDLMax){

		newDD = Cudd_bddIthVar(manager, NDLcounter+(2*num_alt_facts)+1);
	}
	else{
		//   cout << "NEW VAR" <<endl;
		//     printBDD(initialBDD);
		newDD = Cudd_bddNewVar(manager);
		//         printBDD(newDD);

		NDLMax++;

	}
	NDLcounter++;
	Cudd_Ref(newDD);
	return newDD;
}

// void addInitialAxiomForNonDeterEffect(Label* labels, int type, int ndnum) {
//   //after assigning labels to the non deter effects
//   //add an axiom to init state saying they are 'or'
//   //or 'xor' effects based on type.

//   DdNode* newAxiom = Cudd_ReadLogicZero(manager);
//   DdNode *tmp1, *b_sg, *b_sg1;
//   Label* tmp, *tmpl;
//   int ndnum1;

//      printf("add init ax, ndnum = %d\n", ndnum);
//    printBDD(b_initial_state);
//   Cudd_Ref(newAxiom);

//   if(type == 0) {//or
//     tmp = labels;
//     while(tmp) {

//       newAxiom = bdd_or(manager, newAxiom, tmp->label);
//       tmp=tmp->next;
//     }
//     b_initial_state=bdd_and(manager, b_initial_state, newAxiom);
//     Cudd_Ref(b_initial_state);
//   }
//   else if(type==1) {//xor
//     tmp = labels;
//     // ndnum1 = (NDLcounter)+num_alt_facts;
//     while(tmp && ndnum > 0) {
//       //  tmpl = labels;
//       //tmp1 = Cudd_ReadLogicZero(manager);
//       //      cout << "ndnum = " << ndnum <<endl;
//       //       for(int k =((NDLcounter-ndnum)+num_alt_facts); k <(NDLcounter+num_alt_facts); k++){
//       // 	//	cout << "k = " << k << endl;
//       //  	if(k != ndnum1-1)
//       //  	  tmp1 = bdd_and(manager, tmp1, Cudd_Not(Cudd_bddIthVar(manager, k)));
//       // 	else
//       //  	  tmp1 = bdd_and(manager, tmp1, Cudd_bddIthVar(manager, k));

//       //  	tmpl = tmpl->next;
// //       }

//       //      printBDD(tmp1);
//       //printBDD( tmp->label);
//        ndnum--;
//       newAxiom = bdd_or(manager, tmp->label, newAxiom);
//       Cudd_Ref(newAxiom);
//       // printBDD(newAxiom);
//       tmp=tmp->next;
//     }
// //     tmp1 = Cudd_Support(manager, newAxiom);
// //     Cudd_Ref(tmp1);
//     //printBDD(tmp1);
//   //   for(int i = 0; (i < num_alt_facts-1); i++){
// //       for(int j = i+1; (j < num_alt_facts); j++){
// // 	b_sg = Cudd_bddIthVar(manager, i);
// // 	b_sg1 = Cudd_bddIthVar(manager, j);
// // 	if(bdd_entailed(manager, tmp1, b_sg) &&
// // 	   bdd_entailed(manager, tmp1, b_sg1) ){
// // 	  newAxiom = bdd_and(manager, newAxiom, Cudd_Not(bdd_and(manager, b_sg, b_sg1)));
// // 	}
// //       }
// //     }



//      printBDD(newAxiom);
//     //     printBDD(b_initial_state);
//     b_initial_state=bdd_and(manager, b_initial_state, newAxiom);

//    Cudd_Ref(b_initial_state);
//     Cudd_RecursiveDeref(manager, newAxiom);
//   }

//     cout << "adding to intiial bDD"<<endl;
//     printBDD(b_initial_state);

// }


// DdNode* substituteLabels(int time, clausalState* cs) {


//   conj_goal* cg;
//   disj_goal* dg;
//   neg_goal* ng;
//   simple_goal* sg;
//   //goal* g;


//   conj_goal* cg1;
//   goal_list* tmp_list;
//   goal_list* tmp_list1;


//   DdNode *final_goal, *g, *var, *tmp;


//   final_goal = Cudd_ReadOne(manager);
//   Cudd_Ref(final_goal);
//    //      cout << "SUBBING Labels for: " << endl;
// //          cs->display();
// //       cout << "at time = " << time <<endl;

//   if(cg = dynamic_cast<conj_goal*>(cs->getClauses())) {
//     goal_list::iterator i = cg->getGoals()->begin();
//     // tmp_list = new goal_list();
//     for(;i!=cg->getGoals()->end(); ++i) {
//       //   cout << "LIST = " <<endl;
// //       tmp_list->display(0);
//       //  g = NULL;
//       if((sg = dynamic_cast<simple_goal*>(*i)) && (g = getLabelAt(sg,time, TRUE))) {
// 	//	tmp_list->push_back(g);
// 	Cudd_Ref(g);
// 	//	if(!bdd_is_one(manager, final_goal)){
// 	// tmp = Cudd_bddAnd(manager, g, final_goal);
// 	  tmp = bdd_and(manager, g, final_goal);
// 	  Cudd_Ref(tmp);
// 	  Cudd_RecursiveDeref(manager, final_goal);
// 	  final_goal = tmp;
// 	  Cudd_Ref(final_goal);
// 	  //}
// 	  //else
// 	  //  final_goal = g;
//       }
//       else if(ng = dynamic_cast<neg_goal*>(*i)) {
// 	if((sg = dynamic_cast<simple_goal*>(ng->getGoal())) && (g = getLabelAt(sg,time, FALSE)) ) {
// 	  //tmp_list->push_back(g);
// 	  // tmp = Cudd_bddAnd(manager, g, final_goal);
// 	Cudd_Ref(g);
// 	//	if(!bdd_is_one(manager, final_goal)){
// 		  //   tmp = Cudd_bddAnd(manager, g, final_goal);
// 	   tmp = bdd_and(manager, g, final_goal);
// 	  Cudd_Ref(tmp);
// 	  Cudd_RecursiveDeref(manager, final_goal);
// 	  final_goal = tmp;

// 	  Cudd_Ref(final_goal);

// 	  // }
// 	  // else
// 	  //final_goal = g;
// 	}
//       }
//       else if(dg = dynamic_cast<disj_goal*>(*i)) {
// 	var = Cudd_ReadOne(manager);
// 	Cudd_Ref(var);
// 	goal_list::iterator j = dg->getGoals()->begin();
// 	//tmp_list1 = new goal_list();
// 	for(;j!=dg->getGoals()->end(); ++j) {
// 	  //g = NULL;
// 	  if((sg = dynamic_cast<simple_goal*>(*j)) && (g = getLabelAt(sg,time, TRUE))) {
// 	    //tmp_list1->push_back(g);
// 	    //  tmp = Cudd_bddOr(manager, g, var);
// 	    Cudd_Ref(g);
// 	    	    if(!bdd_is_one(manager, var)){
// 	      tmp = bdd_or(manager, g, var);
// 	      Cudd_Ref(tmp);
// 	      Cudd_RecursiveDeref(manager, var);
// 	      var = tmp;
// 	      Cudd_Ref(var);
// 	      }
// 	      else
// 	      var = g;
// 	  }
// 	  else if(ng = dynamic_cast<neg_goal*>(*j)) {
// 	    if((sg = dynamic_cast<simple_goal*>(ng->getGoal())) && (g = getLabelAt(sg,time, FALSE)) ) {
// 	      //tmp_list1->push_back(g);
// 	      // tmp = Cudd_bddOr(manager, g, var);
// 	      Cudd_Ref(g);
// 	     if(!bdd_is_one(manager, var)){
// 		tmp = bdd_or(manager, g, var);
// 		Cudd_Ref(tmp);
// 		Cudd_RecursiveDeref(manager, var);
// 		var = tmp;
// 		Cudd_Ref(var);
// 		 }
// 		else
// 			var = g;
// 	    }
// 	  }

// 	}
// 	// tmp = Cudd_bddAnd(manager, var, final_goal);
// 	//	if(!bdd_is_one(manager, final_goal)){
// 	  tmp = bdd_and(manager, var, final_goal);
// 	  Cudd_Ref(tmp);
// 	  Cudd_RecursiveDeref(manager, final_goal);
// 	  final_goal = tmp;
// 	  Cudd_Ref(final_goal);

// 	  //	}
// 	  //else
// 	  //final_goal = var;


// 	//	if(tmp_list1->size() > 1)
// 	//	  tmp_list->push_back(removeRepeats(new disj_goal(tmp_list1)));
// 	//	else if(tmp_list1->size() == 1)
// 	//	  tmp_list->push_back(tmp_list1->front());

//       }
//     }
//     //   if(tmp_list->size() > 1)
//     //     return_goal = removeRepeats(new conj_goal(tmp_list));
//     //  else if(tmp_list->size() == 1)
//     //  return_goal = tmp_list->front();
//     // else
//     //return_goal = NULL;
//     // cout << "OK"<<endl;
//   }
//   else if((sg = dynamic_cast<simple_goal*>(cs->getClauses())) && (g = getLabelAt(sg,time, TRUE))) {
//     //return_goal = g;
//     Cudd_Ref(g);
//     final_goal = g;
//   }
//   else if(ng = dynamic_cast<neg_goal*>(cs->getClauses())) {
//     if((sg = dynamic_cast<simple_goal*>(ng->getGoal())) && (g = getLabelAt(sg,time, FALSE))) {
//       //return_goal = g;
//       Cudd_Ref(g);
//       final_goal = g;
//     }
//   }
//   else if(dg = dynamic_cast<disj_goal*>(cs->getClauses())) {
//     goal_list::iterator j = dg->getGoals()->begin();
//     //tmp_list1 = new goal_list();
//     for(;j!=dg->getGoals()->end(); ++j) {
//       if((sg = dynamic_cast<simple_goal*>(*j)) && (g = getLabelAt(sg,time, TRUE))) {
// 	//tmp_list1->push_back(g);
// 	//tmp = Cudd_bddOr(manager, g, final_goal);
// 	Cudd_Ref(g);
// 	if(!bdd_is_one(manager, final_goal)){
// 	  tmp = bdd_or(manager, g, final_goal);
// 	  Cudd_Ref(tmp);
// 	  Cudd_RecursiveDeref(manager, final_goal);
// 	  final_goal = tmp;
// 	  Cudd_Ref(final_goal);

// 	}
// 	else{
// 	  final_goal = g;
// 	  Cudd_Ref(final_goal);
// 	}
//       }
//       else if(ng = dynamic_cast<neg_goal*>(*j)) {
// 	if((sg = dynamic_cast<simple_goal*>(ng->getGoal()))&& (g = getLabelAt(sg,time, FALSE))) {
// 	  //tmp_list1->push_back(g);
// 	  // tmp = Cudd_bddOr(manager, g, final_goal);
// 	  Cudd_Ref(g);
// 	  if(!bdd_is_one(manager, final_goal)){
// 	  tmp = bdd_or(manager, g, final_goal);
// 	  Cudd_Ref(tmp);
// 	  Cudd_RecursiveDeref(manager, final_goal);
// 	  final_goal = tmp;
// 	  Cudd_Ref(final_goal);
//  	}
// 	else
// 	  final_goal = g;
// 	  Cudd_Ref(final_goal);

// 	}
//       }
//     }
//     // if(tmp_list1->size() > 1)
//     // return_goal = removeRepeats(new disj_goal(tmp_list1));
//     //else if(tmp_list1->size() == 1)
//     // return_goal = tmp_list1->front();
//     //else
//     //return_goal = NULL;

//   }

//   //  if(return_goal && ((cg = dynamic_cast<conj_goal*>(return_goal)) && cg->getGoals()->size() < 1) ||
//   // ((dg = dynamic_cast<disj_goal*>(return_goal)) && dg->getGoals()->size() < 1))
//   //return_goal = NULL;


//   //     cout << "SUBBED RETURNS: " <<endl;
// //       printBDD(final_goal);
// //     if(return_goal)
// //       return_goal->display(1);
// //     else
// //       cout << "TRUE" <<endl;

//   return final_goal;
// }
// char * st = "state";
// char * st1 = "state1";
// char * st2 = "state2";


// int entailmentCheck(goal* g1, goal* g2) {


//   if(g1){
//     build_model(g1,st1);
//     build_model(g2,st2);
//     system("cat ./smv/tmp/state1.out > ./smv/tmp/dan1.smv");
//     system("cat ./smv/tmp/state2.out >> ./smv/tmp/dan1.smv");
//     system("cat ./smv/tmp/entail1.smv >> ./smv/tmp/dan1.smv");
//     //    cout << "ABOUT TO MODEL CHECK" <<endl;
//     system("./smv/run1 > ./smv/tmp/result1.out");
//     //  system("cat ./smv/tmp/dan.smv");
//     if(!system("grep 'false' ./smv/tmp/result1.out > /dev/null ")){
//       return FALSE;
//     }
//     else return TRUE;
//   }
//   else return TRUE;
// }

// int initEntailmentCheck(goal* s) {
//   //find sets of models for init and s
//   //return true if init set is subset of s set



//   /// char * st = "state";

//   //cout << "IS ENTAILED?: " <<endl;
//   if(s) {
//     // s->display(0);
//   }
//   else {
//     //cout << "GOT A NULL"<<endl;
//     return TRUE;
//   }

//   build_model(s,st);


//   system("cat ./smv/tmp/initial.out > ./smv/tmp/dan.smv");
//   system("cat ./smv/tmp/state.out >> ./smv/tmp/dan.smv");
//   system("cat ./smv/tmp/entail.smv >> ./smv/tmp/dan.smv");

//   //    cout << "ABOUT TO MODEL CHECK" <<endl;
//   system("./smv/run > ./smv/tmp/result.out");
//   //  system("cat ./smv/tmp/dan.smv");
//   if(!system("grep 'false' ./smv/tmp/result.out > /dev/null ")){

//     return FALSE;

//   }
//   else return TRUE;


// }

int labelEntailed(int lev, DdNode* cs);

#ifdef PPDDL_PARSER
/**
 * 涉及reward时 getRelaxedConformantCost计算价值用到
 */
int countParticles(DdNode* d){
	int count = 0;

	//     cout << "|all_samples| = " << all_samples.size() << endl
	//    << "|samples| = " << samples.size()
	//    << endl;

	if(d == Cudd_ReadLogicZero(manager))
		return 0;

	for(int i = 0; i < NUMBER_OF_MGS; i++){
		//			list<DdNode*>::iterator i = samples.begin();
		//		i != samples.end(); i++){
		DdNode* s = make_sample_index(i);
		DdNode *p = Cudd_bddIntersect(manager, s, d);
		Cudd_Ref(p);
		if(p != Cudd_ReadLogicZero(manager))
			count++;
		Cudd_RecursiveDeref(manager, p);
		//	}
		//	for(list<DdNode*>::iterator i = used_samples.begin();
		//			i != used_samples.end(); i++){
		//		DdNode *p = Cudd_bddIntersect(manager, *i, d);
		//		Cudd_Ref(p);
		//		if(p != Cudd_ReadLogicZero(manager))
		//			count++;
		//		Cudd_RecursiveDeref(manager, p);
	}
	/*  for(list<DdNode*>::iterator i = new_samples.begin();
       i != new_samples.end(); i++){
     DdNode *p = Cudd_bddIntersect(manager, *i, d);
     Cudd_Ref(p);
     if(p != Cudd_ReadLogicZero(manager))
       count++;
     Cudd_RecursiveDeref(manager, p);
   }
	 */

	//cout << count << endl;
	return count;
}


//is P(G) > tau
int goals_proven(int lev) {
	//   cout << "proven at " << lev << endl;
	//   Cudd_CheckKeys(manager);
	//   cout << "|" << endl;
	double pr;
	if(!PF_LUG && my_problem->domain().requirements.probabilistic){
		//    cout << "HI" <<endl;
		// printBDD(b_initial_state);

		//DdNode *bddInit = Cudd_addBddStrictThreshold(manager, b_initial_state, 0.0);
		//Cudd_Ref(bddInit);

		// DdNode* gLab = labelBDD(bddInit, b_goal_state, lev);
		DdNode* gLab = labelBDD(b_initial_state, b_goal_state, lev);
		Cudd_Ref(gLab);
		//Cudd_RecursiveDeref(manager, bddInit);

		pr = exAbstractAllLabels(gLab,lev-1);
		Cudd_RecursiveDeref(manager, gLab);
	}
	else if(PF_LUG){
		//DdNode* gLab
		//    printBDD(initialBDD);
		// printBDD(b_goal_state);
		Cudd_RecursiveDeref(manager, goal_samples);
		if( LUG_FOR == SPACE){
			goal_samples = labelBDD(initialBDD,
					b_goal_state, lev);
		}
		else
			goal_samples = labelBDD(Cudd_ReadOne(manager),
					b_goal_state, lev);
		Cudd_Ref(goal_samples);
		//          cout << "goal_samples:"<<endl;
		//         printBDD(goal_samples);

		if(LUG_FOR == SPACE){
			//count those particles where every state reaches goals

			//want bdd of all particles that all states reach the goal
			//I(s, p) --> G(s, p)

			DdNode *gs = Cudd_ReadLogicZero(manager);
			//			for(list<DdNode*>::iterator i = samples.begin();
			//					i != samples.end(); i++){
			for(int i = 0; i < NUMBER_OF_MGS; i++){
				DdNode* s = make_sample_index(i);
				DdNode *tmp = Cudd_bddAnd(manager, s, goal_samples);
				Cudd_Ref(tmp);
				DdNode *tmp1 = Cudd_bddAnd(manager, s, initialBDD);
				Cudd_Ref(tmp1);
				//	cout << "HI" << endl;
				//printBDD(tmp1);
				//	printBDD(tmp);

				if(bdd_entailed(manager, tmp1, tmp)){
					DdNode *tmp2 = Cudd_bddOr(manager, gs, s);
					Cudd_Ref(tmp2);
					gs=tmp2;
					Cudd_Ref(gs);
					Cudd_RecursiveDeref(manager, tmp2);

					///	  printBDD(gs);
				}
				Cudd_RecursiveDeref(manager, tmp);
				Cudd_RecursiveDeref(manager, tmp1);
			}

			Cudd_RecursiveDeref(manager, goal_samples);
			goal_samples=gs;
			Cudd_Ref(goal_samples);
			Cudd_RecursiveDeref(manager, gs);
		}
		//printBDD(goal_samples);
		//    printBDD(b_goal_state);
		if(goal_samples == Cudd_ReadOne(manager))
			pr = 1.0;
		//	else if(goal_samples == Cudd_ReadLogicZero(manager))
		//	  pr = -1.0;
		else if (goal_samples == Cudd_ReadLogicZero(manager))
			pr = 0.0;
		else
			pr = (double)countParticles(goal_samples)/(double)NUMBER_OF_MGS;

		//       cout << "pr = " << pr << " " << samples.size() << endl;

		//		if(PF_LUG){//update costs of particles to reach goal
		//			double new_cost;
		//			for(list<DdNode*>::iterator s = samples.begin();
		//					s != samples.end(); s++){
		//				if(bdd_entailed(manager, *s, goal_samples)){
		//					if(RP_EFFECT_SELECTION != RP_E_S_COVERAGE){
		//						new_cost = costOfGoal(lev, *s);
		//					}
		//					else{
		//						new_cost = lev;
		//					}
		//					if(new_cost < goal_sample_costs[*s])
		//						goal_sample_costs[*s] = new_cost;
		//				}
		//				else
		//					goal_sample_costs[*s] = DBL_MAX;
		//			}
		//		}

		//Cudd_RecursiveDeref(manager, gLab);
		//cout << "PLEASE FIX goals_proven for PF!!"<<endl;
		//return 0;
	}
	else if(!PF_LUG && my_problem->domain().requirements.non_deterministic){
		if(goal_samples)
			Cudd_RecursiveDeref(manager, goal_samples);
		goal_samples = labelBDD(initialBDD,//b_initial_state,
				b_goal_state, lev);
		Cudd_Ref(goal_samples);

		if(MUTEX_SCHEME != MS_NONE){
			DdNode* tmp2 = removeMutexWorlds(goal_samples, b_goal_state, lev);
			Cudd_Ref(tmp2);
			Cudd_RecursiveDeref(manager, goal_samples);
			goal_samples = tmp2;
			Cudd_Ref(goal_samples);
			Cudd_RecursiveDeref(manager, tmp2);
		}


		//    cout << "+++++++LABEL RETURNED++++++++" <<endl;
		//    printBDD(goal_samples);
		//    printBDD(b_initial_state);//
		//      printBDD(initialBDD);

		if(bdd_entailed(manager, //b_initial_state,
				initialBDD,
				goal_samples)){
			cout << "YES" <<endl;
			if(GOALS_REACHED == IPP_MAX_PLAN)
				GOALS_REACHED = lev;
			return 1;
		}
		else{
			cout << "NO" <<endl;
			return 0;
		}
	}
	//cout << "goal_threshold " << goal_threshold << endl;
	if(pr >= goal_threshold){

		if(GOALS_REACHED == IPP_MAX_PLAN)
			GOALS_REACHED = lev;
		return 1;
	}
	else
		return 0;
}
#else
int goals_proven(int lev) {
	// cout << "CHECKING GOALS PROVEN at " << lev<<endl;
	return labelEntailed(lev, b_goal_state);
}
#endif

#ifdef PPDDL_PARSER
DdNode* maskNDLabels(DdNode* lab){


	DdNode* mask = lab, *fr, *fr1, *fr2;
	Cudd_Ref(mask);
	//   cout << "Masking label " << num_alt_facts << "  " << NDLcounter <<endl;
	//   printBDD(lab);

	//for abstracting non-init events
	//      int num_iter = num_alt_facts + maxNDL;
	//      DdNode** cube = new (DdNode*)[maxNDL];


	//for abstracting init events
	int num_iter = 2*num_alt_facts;
	DdNode** cube = new DdNode*[num_iter];//num_alt_facts];




	int j = 0;
	for(int i = 0; i < num_iter; i++){
		//if(((i+1)%2 == 0 || i > 2*num_alt_facts)){  //for abstracting non-init events
		if(((i)%2 == 0 && i < 2*num_alt_facts)){ //for abstracting init events
			cube[j++] =  Cudd_addIthVar(manager,i);
			Cudd_Ref(cube[j-1]);
		}
	}

	//for abstracting non-init events
	//DdNode *cubedd = Cudd_addComputeCube(manager, cube, 0, maxNDL-1);


	//for abstracting init events
	DdNode *cubedd = Cudd_addComputeCube(manager, cube, 0, num_alt_facts);

	// printBDD(lab);
	// printBDD(cubedd);
	Cudd_Ref(cubedd);
	delete [] cube;

	//fr = Cudd_addExistAbstract(manager, mask, cubedd);

	fr = mask;
	Cudd_Ref(fr);
	Cudd_RecursiveDeref(manager, mask);
	Cudd_RecursiveDeref(manager, cubedd);
	mask = fr;
	Cudd_Ref(mask);
	Cudd_RecursiveDeref(manager, fr);

	//fr = normalize(mask);
	//Cudd_RecursiveDeref(manager, mask);
	//mask = fr;
	//Cudd_Ref(mask);
	//Cudd_RecursiveDeref(manager, fr);





	// printBDD(mask);

	return mask;
}
#else
DdNode* maskNDLabels(DdNode* lab){
	DdNode* mask = lab, *fr, *fr1;

	//  cout << "Masking label " << num_alt_facts << "  " << NDLcounter <<endl;
	//  printBDD(lab);
	for(int i = 2*num_alt_facts; i < NDLcounter; i++){
		mask = bdd_or(manager,
				mask,
				fr1=bdd_and(manager,
						fr= mask,
						Cudd_Not(Cudd_bddIthVar(manager, i))));
		//Cudd_Ref(mask);
		Cudd_RecursiveDeref(manager, fr1);
		Cudd_RecursiveDeref(manager, fr);
		mask = bdd_or(manager,
				mask,
				fr1 = bdd_and(manager,
						fr=mask,
						Cudd_bddIthVar(manager, i)));
		//Cudd_Ref(mask);
		Cudd_RecursiveDeref(manager, fr);
		Cudd_RecursiveDeref(manager, fr1);
	}
	//printBDD(mask);
	return mask;
}
#endif

//take add dd and existentially abstract all variables to get
//a single number, if it is a probability distribution for a label,
//then it is the total probability represented in the label
double exAbstractAllLabels(DdNode* dd, int time){
	return exAbstractAllLabels(dd, -1, time);
}

//abstract labels for events from start to finish
double exAbstractAllLabels(DdNode* dd, int start, int finish){
	//cout << start << " " << finish <<endl;
	int num_vars = 0;// num_alt_facts+NDLcounter;


	if(dd == Cudd_ReadZero(manager))
		return 0.0;


	std::list<std::list<int>* >::iterator v = level_vars->begin();
	for(int i = 0; i < start; i++)
		v++;

	for(int i = start; i <= finish; i++){
		if(i == -1)
			num_vars += num_alt_facts;
		else{
			num_vars += (*v)->size();
			v++;
		}
	}
	//cout << "Num = " << num_vars << endl;

	DdNode** cube = new DdNode*[num_vars];//num_alt_facts];
	int k = 0;
	if(start == -1){
		for(int i = 0; i < num_alt_facts; i++){
			cube[i] =  Cudd_addIthVar(manager,2*i);
			Cudd_Ref(cube[i]);
			k++;
		}
	}

	v = level_vars->begin();
	int p = 0;
	for(int i = 0; i < start; i++){
		v++;
		p++;
	}

	for(;v != level_vars->end()  && p <= finish
	;v++, p++){
		// cout << "p = " << p << endl;
		for(list<int>::iterator j = (*v)->begin(); j != (*v)->end();j++){
			//    cout << *j << endl;
			cube[k++] =  Cudd_addIthVar(manager,*j);
			Cudd_Ref(cube[k-1]);
		}

	}


	DdNode *cubedd = Cudd_addComputeCube(manager, cube, NULL, num_vars);

	Cudd_Ref(cubedd);


	//     printBDD(cubedd);

	for(int i = 0; i < num_vars; i++){
		Cudd_RecursiveDeref(manager, cube[i]);
	}
	delete [] cube;



	DdNode* fr = Cudd_addExistAbstract(manager, dd, cubedd);

	//printBDD(fr);
	Cudd_Ref(fr);
	Cudd_RecursiveDeref(manager, cubedd);

	int* x = new int[num_vars];
	CUDD_VALUE_TYPE value;
	DdGen* gen = Cudd_FirstCube(manager, fr, &x, &value);
	DdNode* tmp_cube = Cudd_CubeArrayToBdd(manager, x);
	Cudd_Ref(tmp_cube);
	//   printBDD(tmp_cube);
	//   Cudd_RecursiveDeref(manager, tmp_cube);
	Cudd_RecursiveDeref(manager, fr);


	//  while(Cudd_NextCube(gen, &x, &value)){
	//     tmp_cube = Cudd_CubeArrayToBdd(manager, x);
	//   }
	delete [] x;
	if(gen){
		//    cout << "Got gen" <<endl;
		//Cudd_GenFree(gen);
	}
	else{
		// cout << "no gen" <<endl;
	}
	// cout << value<< endl;
	return value;
}

double LEV_OFF_EPSILON = 0.001;
//are labels of facts at time same as those at time-1
int labels_same(int time){
	FtNode* tmpF = gall_fts_pointer;
	FtNode* tmpFnxt;
	//    cout << "Are labels same at " << time<< endl;



	// 对所有的factNode进行比较，通过则返回true
	if(time > 0){
		while(tmpF){

			//       printBDD(tmpF->info_at[time]->label->label);
			//       printBDD(probabilisticLabels);
#ifdef PPDDL_PARSER
			//       DdNode *diff = Cudd_addApply(manager, Cudd_addMinus,
			// 				   maskNDLabels(tmpF->info_at[time]->label->label),
			// 				   maskNDLabels(tmpF->info_at[time-1]->label->label));

			//       for(EfEdge* e = tmpF->adders; e; e = e->next){
			// 	cout << "added by " << e->ef->op->name << endl;
			// 	printBDD(e->ef->info_at[time-1]->label->label);
			//       }
			//   cout << "[" <<endl;
			//   Cudd_CheckKeys(manager);
			//   cout << "| ls"<<endl;

			DdNode *tmp, *tmp1;//diff;
			double norm, norm1;


			// 当前fact没有label，考虑下一个fact
			if(!tmpF->info_at[time] ||
					!tmpF->info_at[time]->label ||
					!tmpF->info_at[time]->label->label){
				tmpF=tmpF->next;
				continue;
			}
			// 该fact的上一层没有label，当前层有，label不同返回0
			if((tmpF->info_at[time] &&
					(!tmpF->info_at[time-1] ||
							tmpF->info_at[time-1]->is_dummy ||
							!tmpF->info_at[time-1]->label))){
				return 0;
			}


			if(PF_LUG){// in PF_LUG, new_samle = init state BDD
			  if(RBPF_LUG && tmpF->index == (num_alt_facts-2)){
			    tmp1 =  tmpF->info_at[time-1]->label->label;
			    Cudd_Ref(tmp1);
			    tmp =  tmpF->info_at[time]->label->label;
			    Cudd_Ref(tmp);
			  }
			  else{// 两层都有label
			    tmp1 = Cudd_bddAnd(manager, tmpF->info_at[time-1]->label->label, new_sampleDD);
			    Cudd_Ref(tmp1);
			    tmp = Cudd_bddAnd(manager, tmpF->info_at[time]->label->label, new_sampleDD);
			    Cudd_Ref(tmp);
			  }
				if(tmp1 != tmp){// 两个label不同
					Cudd_RecursiveDeref(manager, tmp);
					Cudd_RecursiveDeref(manager, tmp1);
					return 0;
				}
				Cudd_RecursiveDeref(manager, tmp);
				Cudd_RecursiveDeref(manager, tmp1);
			}
			else if(!PF_LUG && my_problem->domain().requirements.probabilistic){
				tmp1 = Cudd_BddToAdd(manager, tmpF->info_at[time]->label->label);
				Cudd_Ref(tmp1);
				tmp = Cudd_addApply(manager, Cudd_addTimes, tmp1, probabilisticLabels);
				Cudd_Ref(tmp);
				Cudd_RecursiveDeref(manager, tmp1);
				norm1 = exAbstractAllLabels(tmp, time-1);
				Cudd_RecursiveDeref(manager, tmp);
				//         cout << "t: " << norm1 <<endl;
				tmp1 = Cudd_BddToAdd(manager, tmpF->info_at[time-1]->label->label);
				Cudd_Ref(tmp1);
				tmp = Cudd_addApply(manager, Cudd_addTimes, tmp1, probabilisticLabels);
				Cudd_Ref(tmp);
				Cudd_RecursiveDeref(manager, tmp1);
				norm = exAbstractAllLabels(tmp, time-1);
				Cudd_RecursiveDeref(manager, tmp);
				if(norm1 -norm > LEV_OFF_EPSILON){ //not stable, return no lev off
					//	cout << "YES"<<endl;
					return FALSE;
				}
			}
			// conformant planning enter
			else if (!PF_LUG && my_problem->domain().requirements.non_deterministic){
				if(tmpF->info_at[time]->label->label !=
						tmpF->info_at[time-1]->label->label){// directly compare two label
					//	  cout<< "Not"<<endl;
					return FALSE;
				}

				if(MUTEX_SCHEME != MS_NONE){// open mutex, compare the mutex


					tmpFnxt = tmpF->next;// 判断其他FactNode,其在两层的exclusive mutex情况是否
					while(tmpFnxt){
						if((tmpFnxt->positive &&// positive fact
								tmpF->info_at[time]->exclusives->p_exlabel[tmpFnxt->index] !=
										tmpF->info_at[time-1]->exclusives->p_exlabel[tmpFnxt->index]) ||
										(!tmpFnxt->positive &&// negative fact
												tmpF->info_at[time]->exclusives->n_exlabel[tmpFnxt->index] !=
														tmpF->info_at[time-1]->exclusives->n_exlabel[tmpFnxt->index]))
							return FALSE;

						tmpFnxt = tmpFnxt->next;
					}
				}
			}



			//          cout << "NORM = " << (norm1 - norm) <<endl;

#else
			if(maskNDLabels(tmpF->info_at[time]->label->label) !=
					maskNDLabels(tmpF->info_at[time-1]->label->label)){
				//	cout << "No"<<endl;
				return FALSE;
			}
#endif
			tmpF = tmpF->next;
		}// end-while
	}
	// at layer 0, directly return false
	else
		return FALSE;

	//  cout << "YES"<<endl;


	return TRUE;
}


int labelEntailed(int lev, DdNode* cs) {
	//construct a new formula by substituting all labels of literals in cs at lev
	//see if initial state entails it
	//    cout << "CHECKING at " << lev << " for LABEL ENTAILED FOR: " <<endl;
	//      cs->display();

	DdNode* g = substitute_label(cs, lev);
	DdNode* tmp;

	//  printBDD(g);

	Cudd_Ref(g);
	int ent = FALSE;
	//  printBDD(g);

	//      cout << "DOES " <<endl;
	//  	printBDD(//initialBDD);//
	//  		 b_initial_state);
	//  	cout << "ENTAIL " <<endl;
	//  	printBDD(g);

	ent =  bdd_entailed(manager, b_initial_state, g);
	Cudd_RecursiveDeref(manager, g);
	//      cout << " entailed = " << ent <<endl;
	return ent;

}

int no_dummys(int lev, FactInfo* pos, FactInfo* neg) {

	FtNode* tmp_table = gall_fts_pointer;
	EfEdge* tmp_adders;

	while(tmp_table) {
		if((get_bit(pos->vector,gop_vector_length_at[0] , tmp_table->index)||
				get_bit(neg->vector,gop_vector_length_at[0] , tmp_table->index))
				&&
				(tmp_table->info_at[lev]->is_dummy)){
			return FALSE;
		}

		tmp_adders = tmp_table->adders;
		while (tmp_adders) {
			if(tmp_adders->ef->info_at[lev] && tmp_adders->ef->info_at[lev]->is_dummy)
				return FALSE;
			tmp_adders = tmp_adders->next;

		}

		tmp_table = tmp_table->next;
	}
	return TRUE;
}


// int is_ClausalState_consistent(int time, BitVector* pos, BitVector* neg ) {
//   //none of the literals are mutex across worlds entailed by the intiial states
//     int r,s;
//   FtNode* tmpFt;// = gall_fts_pointer;
//   ExclusionLabelPair *tmpelp;

//   //  printf("goals non mutex?\n");

//   for(s=0;s<2*gnum_relevant_facts;s++){// tmpFt!=gprev_level_fts_pointer; tmpFt=tmpFt->next){

//     tmpFt=gft_table[s];
//     if(!tmpFt)
//       continue;
//     //    printf("Checking cs mutexes for:%d \n",s);
//     //    printFact(tmpFt->index);
//     if(tmpFt->positive && get_bit(pos, gft_vector_length, tmpFt->index)){
//       //      printf("checking exclusions for fact: %d\n", tmpFt->index);
//       //      printFact(tmpFt->index);
//       //      printf("\n");
//       //      print_BitVector(tmpFt->info_at[time]->exclusives->pos_exclusives, gft_vector_length);
//       //       print_BitVector(pos, gft_vector_length);
//       for(r=0; r< gnum_relevant_facts;r++){
// // 	printf("%d\n",	get_bit(tmpFt->info_at[time]->exclusives->pos_exclusives,gft_vector_length ,r));
// // 	printf("%d\n",	get_bit(pos, gft_vector_length,r));
// // 	printf("r = %d\n", r);
// 	if((get_bit(tmpFt->info_at[time]->exclusives->pos_exclusives,gft_vector_length ,r) &&
// 	    get_bit(pos, gft_vector_length,r))){
// //	if((tmpFt->info_at[time]->exclusives->pos_exclusives[r] & gbit_goal_state->positive->vector[r])){
// // 	  printf("Is mutex with: %d\n", r);
// // 	  printFact(r);
// //        print_BitVector(pos, gft_vector_length);
// //       printf("\n");

// 	  tmpelp = tmpFt->info_at[time]->exclusives->p_exlabels[r];
// 	  while(tmpelp){
// 	    //	    printf("loop\n");
// 	    if(!((bdd_entailed(manager,
// 			       initialBDD,
// 			       Cudd_Not(tmpelp->elp1) ))||
// 		 (bdd_entailed(manager,
// 			       initialBDD,
// 			       Cudd_Not(tmpelp->elp2)))))
// 	      return FALSE;
// 	  }
// 	}
// 	if((get_bit(tmpFt->info_at[time]->exclusives->neg_exclusives,gft_vector_length ,r) &&
// 	    get_bit(neg, gft_vector_length,r))){
// /* 	if((tmpFt->info_at[time]->exclusives->neg_exclusives[r] & gbit_goal_state->negative->vector[r])){ */
// 	  tmpelp = tmpFt->info_at[time]->exclusives->n_exlabels[r];
// 	  while(tmpelp){
// 	    if(!((bdd_entailed(manager,
// 			       initialBDD,
// 			       Cudd_Not(tmpelp->elp1) ))||
// 		 (bdd_entailed(manager,
// 			       initialBDD,
// 			       Cudd_Not(tmpelp->elp2)))))
// 	      return FALSE;
// 	  }
// 	}

//       }
//     }

//     else if(!tmpFt->positive && get_bit(neg, gft_vector_length, tmpFt->index)){
//       for(r=0; r< gft_vector_length;r++){
// 	if((get_bit(tmpFt->info_at[time]->exclusives->neg_exclusives,gft_vector_length ,r) &&
// 	    get_bit(neg, gft_vector_length,r))){
// /* 	if((tmpFt->info_at[time]->exclusives->pos_exclusives[r] & gbit_goal_state->positive->vector[r])){ */
// 	  tmpelp = tmpFt->info_at[time]->exclusives->p_exlabels[r];
// 	  while(tmpelp){
// 	    if(!((bdd_entailed(manager,
// 			       initialBDD,
// 			       Cudd_Not(tmpelp->elp1) ))||
// 		 (bdd_entailed(manager,
// 			       initialBDD,
// 			       Cudd_Not(tmpelp->elp2)))))
// 	      return FALSE;
// 	  }
// 	}
// 	if((get_bit(tmpFt->info_at[time]->exclusives->neg_exclusives,gft_vector_length ,r) &&
// 	    get_bit(neg, gft_vector_length,r))){
// /* 	if((tmpFt->info_at[time]->exclusives->neg_exclusives[r] & gbit_goal_state->negative->vector[r])){ */
// 	  tmpelp = tmpFt->info_at[time]->exclusives->n_exlabels[r];
// 	  while(tmpelp){
// 	    if(!((bdd_entailed(manager,
// 			       initialBDD,
// 			       Cudd_Not(tmpelp->elp1) ))||
// 		 (bdd_entailed(manager,
// 			       initialBDD,
// 			       Cudd_Not(tmpelp->elp2)))))
// 	      return FALSE;
// 	  }
// 	}
//       }
//     }
//   }
//   return TRUE;

// }


// int maxFactLevel(DdNode* b){
//   simple_goal* sg1;
//   neg_goal* ng1;
//   DdNode  *b_sg1, *b_ng1;
//   int max = -1;
//   int min = graph_levels;
//   DdNode** minterms = extractDNFfromBDD(b);
//   int k = 0;
//   while(minterms[k] != NULL){
//     for(int i = 0; i < num_alt_facts; i++){
// //       sg1 = new simple_goal(new proposition(alt_facts[i]), E_POS);
// //       ng1 = new neg_goal(new simple_goal(new proposition(alt_facts[i]), E_POS));
// //       b_sg1 = goalToBDD(sg1);
// //       b_ng1 = goalToBDD(ng1);
//       b_sg1 = Cudd_bddIthVar(manager,i);
//       b_ng1 = Cudd_Not(Cudd_bddIthVar(manager,i));
//       if(bdd_entailed(manager, minterms[k], b_sg1)){
// 	if(pos_fact_at_min_level[i] > max)
// 	  max = pos_fact_at_min_level[i];
//       }
//       else if(bdd_entailed(manager, minterms[k], b_ng1)){
// 	if(neg_fact_at_min_level[i] > max)
// 	  max = neg_fact_at_min_level[i];
//       }
//     }
//     if(max < min && max != -1){
//       min = max;
//       max = -1;
//     }
//     k++;
//   }
//   return min;
// }

// DdNode* removeMutexMinterms(DdNode* b, int k){

//   if(MUTEX_SCHEME==MS_NONE)
//     return b;

//   DdNode** minterms = extractDNFfromBDD(b);
//   DdNode *min_lab,  *b_sg1, *b_ng1, *b_sg2, *b_ng2, *return_b, *tmp;
//   simple_goal* sg1, *sg2;
//   neg_goal* ng1, *ng2;
//   int l = 0;

//   return_b = b;
//   Cudd_Ref(return_b);

//   while(minterms[l] != NULL){
//     min_lab = labelBDD(b, k);
//     for(int i = 0; i < num_alt_facts; i++){
// //       sg1 = new simple_goal(new proposition(alt_facts[i]), E_POS);
// //       ng1 = new neg_goal(new simple_goal(new proposition(alt_facts[i]), E_POS));
// //       b_sg1 = goalToBDD(sg1);
// //       b_ng1 = goalToBDD(ng1);
//       b_sg1 = Cudd_bddIthVar(manager,i);
//       b_ng1 = Cudd_Not(Cudd_bddIthVar(manager,i));
// // //       Cudd_Ref(b_sg1);
// // //       Cudd_Ref(b_ng1);
//       for(int j = 0; j < num_alt_facts; j++){
// // 	sg2 = new simple_goal(new proposition(alt_facts[j]), E_POS);
// // 	ng2 = new neg_goal(new simple_goal(new proposition(alt_facts[j]), E_POS));
// // 	b_sg2 = goalToBDD(sg2);
// // 	b_ng2 = goalToBDD(ng2);
//       b_sg2 = Cudd_bddIthVar(manager,j);
//       b_ng2 = Cudd_Not(Cudd_bddIthVar(manager,j));
// // // 	Cudd_Ref(b_sg2);
// // // 	Cudd_Ref(b_ng2);
// 	if((bdd_entailed(manager, minterms[l], b_sg1) &&
// 	    bdd_entailed(manager, minterms[l], b_sg2) &&
// 	    !labelGraphLiteralConsistent(sg1, sg2, k)) ||
// 	   (bdd_entailed(manager, minterms[l], b_sg1) &&
// 	    bdd_entailed(manager, minterms[l], b_ng2) &&
// 	    !labelGraphLiteralConsistent(sg1, ng2, k)) ||
// 	   (bdd_entailed(manager, minterms[l], b_ng1) &&
// 	    bdd_entailed(manager, minterms[l], b_sg2) &&
// 	    !labelGraphLiteralConsistent(ng1, sg2, k)) ||
// 	   (bdd_entailed(manager, minterms[l], b_ng1) &&
// 	    bdd_entailed(manager, minterms[l], b_ng2) &&
// 	    !labelGraphLiteralConsistent(ng1, ng2, k))) {
// 	  tmp = bdd_and(manager, return_b, Cudd_Not(minterms[l]));
// 	  Cudd_Ref(tmp);
// 	  Cudd_RecursiveDeref(manager, return_b);
// 	  return_b = tmp;
// 	  Cudd_Ref(return_b);
// 	}
//       }
//     }
//     l++;
//   }

//   return return_b;
// }

DdNode* labelBDD(DdNode* s, DdNode* d, int k){


	//replace current state vars in b with next state vars
	//replace next state vars with correponding label (in terms of this state vars)

	DdNode* next, *result=NULL;



	FtNode* ft;

	DdNode *tmp, *tmp1, *tmp2;


	int deep;
	//	if(maxNDL + num_alt_facts > 2*num_alt_facts)
	//		deep = maxNDL + num_alt_facts;
	//	else
	//		deep = 2*num_alt_facts;

	deep = (2*num_alt_facts)+max_num_aux_vars+(2*rbpf_bits);

	DdNode** pvector = new DdNode*[deep];
	DdNode** nvector = new DdNode*[deep];


	if(//LUG_FOR != SPACE &&
			!PF_LUG &&
			my_problem->domain().requirements.probabilistic){
		tmp2 = Cudd_addBddStrictThreshold(manager, s,0.0);
		Cudd_Ref(tmp2);
	}
	else{// if (LUG_FOR == SPACE || my_problem->domain().requirements.non_deterministic){

		tmp2 = s;
		Cudd_Ref(tmp2);

	}
	//	printBDD(s);




	for(int i = num_alt_facts*2; i < deep; i++){
		pvector[i] = Cudd_bddIthVar(manager, i);
		Cudd_Ref(pvector[i]);

		nvector[i] = Cudd_Not(pvector[i]);
		Cudd_Ref(nvector[i]);

		//      printBDD(pvector[i]);
		//      printBDD(nvector[i]);
	}

	for(int i = 0; i < num_alt_facts; i++){
		//std::cout << "i = " << 2*i << std::endl;

		pvector[2*i+1] = Cudd_ReadOne(manager);
		Cudd_Ref(pvector[2*i+1]);
		nvector[2*i+1] = Cudd_ReadOne(manager);
		Cudd_Ref(nvector[2*i+1]);


		DdNode* p = Cudd_bddIthVar(manager, 2*i);
		Cudd_Ref(p);


		DdNode* n = Cudd_Not(p);
		Cudd_Ref(n);

		ft = gft_table[i];

		if(ft && ft->info_at[k] && !ft->info_at[k]->is_dummy ){
			if(COMPUTE_LABELS){

				if(!ft->info_at[k]->label)
					cout << "no label" <<endl;

				//  printBDD(ft->info_at[k]->label->label);
				//  printBDD(tmp2);

				DdNode* tmpd = Cudd_bddIntersect(manager, d, p);
				Cudd_Ref(tmpd);



				if(tmpd != Cudd_ReadLogicZero(manager)){//1 || Cudd_bddIsVarEssential(manager, d, 2*i, 1)){
					//cout << "HI" << endl;
					if(1)
						tmp1 = Cudd_bddAnd(manager, ft->info_at[k]->label->label, tmp2);
					else
						tmp1 = Cudd_overApproxAnd(manager, ft->info_at[k]->label->label, tmp2);
					Cudd_Ref(tmp1);
					pvector[2*i] = tmp1;
					Cudd_Ref(pvector[2*i]);
					Cudd_RecursiveDeref(manager, tmp1);

				}
				else{
					pvector[2*i] = Cudd_ReadLogicZero(manager);//Cudd_addIthVar(manager, 2*i);
					Cudd_Ref(pvector[2*i]);
				}
				Cudd_RecursiveDeref(manager, tmpd);
			}
			else{
				pvector[2*i] = Cudd_ReadOne(manager);
				Cudd_Ref(pvector[2*i]);
			}
		}
		else{
			//pvector[2*i] = Cudd_ReadLogicZero(manager);
			pvector[2*i] = Cudd_ReadLogicZero(manager);
			Cudd_Ref(pvector[2*i]);
		}


		ft = gft_table[NEG_ADR(i)];
		if(ft  && ft->info_at[k] && !ft->info_at[k]->is_dummy ){
			if(COMPUTE_LABELS){

				if(!ft->info_at[k]->label)
					cout << "no label" <<endl;

				DdNode* tmpd = Cudd_bddIntersect(manager, d, n);
				Cudd_Ref(tmpd);

				if(tmpd != Cudd_ReadLogicZero(manager)){//1 || Cudd_bddIsVarEssential(manager, d, 2*i, 0)){

					if(1)
						tmp1 = Cudd_bddAnd(manager, ft->info_at[k]->label->label, tmp2);
					else
						tmp1 = Cudd_overApproxAnd(manager, ft->info_at[k]->label->label, tmp2);
					Cudd_Ref(tmp1);
					nvector[2*i] = tmp1;
					Cudd_Ref(nvector[2*i]);
					Cudd_RecursiveDeref(manager, tmp1);
				}
				else{
					nvector[2*i] = Cudd_ReadLogicZero(manager);//Cudd_Not(Cudd_addIthVar(manager, 2*i));
					Cudd_Ref(nvector[2*i]);
				}
				Cudd_RecursiveDeref(manager, tmpd);

			}
			else{
				nvector[2*i] = Cudd_ReadOne(manager);
				Cudd_Ref(nvector[2*i]);
			}

		}
		else{
			nvector[2*i] = Cudd_ReadLogicZero(manager);
			Cudd_Ref(nvector[2*i]);
			//nvector[2*i] = Cudd_ReadLogicZero(manager);
		}
		//		if(i == 1 ){
		//		              cout << (2*i) << endl;
		//		      printBDD(pvector[2*i]);
		//		      printBDD(nvector[2*i]);
		//		}
		//       printBDD(pvector[2*i+1]);
		//       printBDD(nvector[2*i+1]);

	}
	//	printBDD(tmp2);
	//	printBDD(s);




	//    cout << "in compose" <<endl;
	//Cudd_RecursiveDeref(manager, tmp2);

//	for(int i = 0; i < deep; i++){
//
//		cout << i << endl;
//		printBDD(pvector[i]);
//		printBDD(nvector[i]);
//
//	}

	Cudd_Ref(s);
	tmp1 = Cudd_bddVectorCompose1(manager,
			d,
			pvector,
			nvector,
			deep //(2*num_alt_facts)//+max_num_aux_vars+(2*rbpf_bits)
	);
	Cudd_Ref(tmp1);
	//	printBDD(s);

	//printBDD(b_goal_state);

	if(!PF_LUG && my_problem->domain().requirements.probabilistic){
		tmp = Cudd_BddToAdd(manager, tmp1);
		Cudd_RecursiveDeref(manager, tmp1);
		Cudd_Ref(tmp);
		result = Cudd_addApply(manager, Cudd_addTimes, tmp, probabilisticLabels);
		Cudd_Ref(result);
		Cudd_RecursiveDeref(manager, tmp);
	}
	else {
		result = tmp1;
		Cudd_Ref(result);
		Cudd_RecursiveDeref(manager, tmp1);
	}

	//printBDD(result);
	//cout << "out compose" <<endl;
	//	printBDD(s);


	for(int i = 0; i < deep; i++){
		Cudd_RecursiveDeref(manager, pvector[i]);
		Cudd_RecursiveDeref(manager, nvector[i]);
	}
	//}



	//	printBDD(s);


	//	    cout << "GOT " <<endl <<flush;
	    Cudd_Ref(d);
	    //	    printBDD(d);
	    //	    printBDD(result);

	//  cout << "exit"<<endl;
	//   Cudd_CheckKeys(manager);  fflush(stdout);
	delete [] pvector;
	delete [] nvector;

	return result;
}

int getLabelProven(DdNode* s, DdNode* b, list<DdNode*>* worldsAtLevels,
		double *max_pr) {
	DdNode* tmp,* tmp1;

	//      cout << "get lab proven for " <<endl;
	//      printBDD(s);
	//  	printBDD(b);
	//int k = maxFactLevel(b);
	//  cout << "maxFactLevel = " << k << endl;
	//cout << "graph levels = " << graph_levels <<endl;

	// printBDD(initialBDD);
	//  cout << "DOES state entail graph dd" << endl;
	//   if(bdd_entailed(manager, s, initialBDD))
	//     cout << "YES" <<endl;
	//   else
	//     cout << "NO" <<endl;



	double max_pr_lev = graph_levels;
	*max_pr = 0.0;
	double pr;
	for(int k =0 ; (k <= graph_levels); k++){
		// cout << "k = " << k <<endl;
		//    Cudd_CheckKeys(manager);  fflush(stdout);


		if(COMPUTE_LABELS){

			tmp1 = labelBDD(s, b, k);
			//printBDD(tmp1);
			Cudd_Ref(tmp1);

			if(MUTEX_SCHEME != MS_NONE){
				DdNode* tmp2 = removeMutexWorlds(tmp1, b, k);
				Cudd_Ref(tmp2);
				Cudd_RecursiveDeref(manager, tmp1);
				tmp1 = tmp2;
				Cudd_Ref(tmp1);
				Cudd_RecursiveDeref(manager, tmp2);
			}


			if(worldsAtLevels){

				worldsAtLevels->push_back(tmp1);
			}
			// printBDD(tmp1);
#ifdef PPDDL_PARSER
			if(PF_LUG && my_problem->domain().requirements.probabilistic){
				//pr = exAbstractAllLabels(tmp1, k-1);//get_sum(tmp1);
				if(tmp1 != Cudd_ReadLogicZero(manager))
					pr = (double)countParticles(tmp1)/(double)NUMBER_OF_MGS;
				else
					pr = 0.0;
				//cout << "PR = " << pr << " k = " << k << endl;

				if(pr > *max_pr){
					//cout << "max = " << k <<endl;
					max_pr_lev = k;
					*max_pr = pr;

					if(*max_pr >= goal_threshold){
						*max_pr *= -1.0;
						return max_pr_lev;
					}
				}
			}
			else if(my_problem->domain().requirements.non_deterministic){
				//DdNode *dan = Cudd_bddAnd(manager, Cudd_Not(tmp1), s);
				// 	printBDD(dan);

				if(bdd_entailed(manager, s, tmp1)){
					//cout << "conf reach at k = " << k << endl;
					Cudd_RecursiveDeref(manager, tmp1);
					*max_pr = -1.0;
					return k;
				}
			}
#else
			if(bdd_entailed(manager, s, tmp1)){
				cout << "conf reach at k = " << k << endl;
				Cudd_RecursiveDeref(manager, tmp1);
				return k;
			}
#endif
			//     Cudd_CheckKeys(manager);  fflush(stdout);
			//     cout << "]" <<endl;
		}
		else{
			tmp1 = labelBDD(Cudd_ReadOne(manager), b, k);
			Cudd_Ref(tmp1);
			//printBDD(tmp1);
			if(tmp1 == Cudd_ReadOne(manager)){
				//	cout << "conf reach at k = " << k << endl;
				Cudd_RecursiveDeref(manager, tmp1);
				return k;
			}
		}
		Cudd_RecursiveDeref(manager, tmp1);
	}

#ifdef PPDDL_PARSER
	//  cout << "HO"<<endl;
	return INT_MAX;//max_pr_lev;
#else
	return IPP_MAX_PLAN;//graph_levels;
#endif
}

// int getLabelProven(clausalState* cs) {


//   //look for first level where all literals appear
//   //substitute labels into cs to get csls
//   //check if CS_I entails csls
//   //  clausalState* cs1 = cs->guestimateEasiestState();
//   // FactInfoPair* csliterals = cs->getBitVectors(TRUE);
//   int excl = TRUE;
//   int p = 0;//factsFirstPresent(csliterals);
//   FactInfoPair* csliterals1 = cs->getBitVectors(FALSE);

//   int IMPROVE_COST;

//  //   cout << "Enter GetLabelProven " << p <<endl;
// //   cs->display();

//   for(; (p <= graph_levels && p > -1); p++){
//     //  cout << "Looking at level " << p  << " Of " << graph_levels << endl;
// 	 //	 if(no_dummys(p, csliterals->positive, csliterals->negative)){
//     //	   cout << "NO DUMMYS"<<endl;


//      if((MUTEX_SCHEME == MS_NONE) ||
//        ((MUTEX_SCHEME!=MS_CROSS)  &&
//      	are_there_non_exclusive(p, csliterals1->positive, csliterals1->negative)
// 	) ||
//        (MUTEX_SCHEME!=MS_NONE  && labelGraphClausalStateConsistent(cs, p)
// 	)){

//       excl = FALSE;
//       // cout << "NON EXCLUSIVE"<<endl;
//       if(labelEntailed(p, cs)){
// // 	cout << "Found at level " << p <<endl;
// 	return  p;
//       }
//     }
//   }
//   free_fact_info_pair(csliterals1);

//   //  cout << "DUH " << excl <<endl;
//   //return p;
//   if(excl){
//     //       cout << "NOT FOUND"<<endl;
//        //       exit(0);
//    return IPP_MAX_PLAN;
//   }
//   else
//     return graph_levels;
// }





// goal_list* getGoalClauses(goal* g) {
//   //get a list of clauses
//   conj_goal* cg;
//   goal_list* tmp;
//   if(cg = dynamic_cast<conj_goal*>(g))
//     return cg->getGoals();
//   else{
//     tmp = new goal_list();
//     tmp->push_back(g);
//     return tmp;
//   }
// }

// goal_list* getClauseLiterals(goal* g) {
//   //get a list of clauses
//   disj_goal* dg;
//   conj_goal* cg;
//   goal_list* tmp;
//   if(dg = dynamic_cast<disj_goal*>(g))
//     return dg->getGoals();
//   else if(cg = dynamic_cast<conj_goal*>(g)){
//     return NULL;
//   }
//   else{
//     tmp = new goal_list();
//     tmp->push_back(g);
//     return tmp;
//   }

// }

int isNonMutexCrossWorld(BitVector* a_excl,
		ExclusionLabelPair* a_exlabel,
		int a,
		DdNode* a_label,
		BitVector* b_excl,
		ExclusionLabelPair* b_exlabel,
		int b,
		DdNode* b_label,
		int time) {
	DdNode** dda;
	DdNode** ddb;
	int sizea, sizeb;
	ExclusionLabelPair* tmp;





	//    cout << "Is Non mutex cross world a = " << a << " b = " << b <<endl;

	// print_BitVector(a_excl, gft_vector_length);
	// print_BitVector(b_excl, gft_vector_length);
	// cout << endl;

	//return true if there is a pair of worlds where a and b are non mutex
	if(get_bit(a_excl,gft_vector_length ,b) || get_bit(b_excl,gft_vector_length ,a)) {
		//they are mutex -- need to see if its in all worlds
		//        cout << "Are mutex"<<endl;
		//if either side has a true label then -- all mutex
		if((b_exlabel && bdd_is_one(manager, b_exlabel->elp1) && bdd_is_one(manager, b_exlabel->elp2)) ||
				(a_exlabel && bdd_is_one(manager, a_exlabel->elp1) && bdd_is_one(manager, a_exlabel->elp2)))
			return FALSE;
		else {    //else need to check the subsets fo the worlds
			//if literals are mutex for all cross products of worlds where they are su/pported then return FALSE
			//	cout << "Labels are not TRUE\n";

			//		printBDD(a_label);
			//	printBDD(b_label);


			//get DNF for labels
			dda =  extractDNFfromBDD(a_label);
			//		cout <<"OKAY"<<endl;
			ddb =  extractDNFfromBDD(a_label);
			//cout << "OKAY" <<endl;


			sizea = 0;
			while(dda[sizea++]){
				sizeb = 0;
				while(ddb[sizeb++]){
					tmp = a_exlabel;

					//cout << "YOU" <<endl;
					while(tmp){
						//	    cout << "PARTA\n";
						// printBDD(tmp->elp1);
						//printBDD(dda[sizea-1]);
						//printBDD(tmp->elp2);
						//printBDD(ddb[sizeb-1]);


						if((bdd_entailed(manager, dda[sizea-1], tmp->elp1) &&
								bdd_entailed(manager ,ddb[sizeb-1], tmp->elp2)))
							return FALSE;
						tmp=tmp->next;

					}

					tmp = b_exlabel;
					while(tmp){
						//  printBDD(tmp->elp1);
						// printBDD(dda[sizea-1]);
						// printBDD(tmp->elp2);
						// printBDD(ddb[sizeb-1]);

						//cout << "PARTB\n";

						if((bdd_entailed(manager, dda[sizea-1], tmp->elp1) &&
								bdd_entailed(manager ,ddb[sizeb-1], tmp->elp2)))
							return FALSE;
						tmp=tmp->next;
					}
					//	    cout << "LOSAT"<<endl;
				}
			}

			return TRUE;
		}
	}
	else {// they aren't mutex in any worlds
		//         cout << "Non mutex"<<endl;
		return TRUE;
	}


}

// int labelGraphLiteralConsistent(goal* a, goal* b, int time){
//   //there is no mutex that is entailed by the initial state
//   simple_goal* sga;
//   simple_goal* sgb;
//   neg_goal* nga;
//   neg_goal* ngb;
//   FtNode* fta;
//   FtNode* ftb;
//   BitVector* a_excl, *b_excl;
//   ExclusionLabelPair* a_exlab, *b_exlab;


//   int bex, aex;


//   //   cout << "IS literals consistent" << endl;
//   //a->display(0);
//   // b->display(0);

//   if(nga = dynamic_cast<neg_goal*>(a)) {
//     if(sga = dynamic_cast<simple_goal*>(nga->getGoal())) {
//       fta = gft_table[NEG_ADR(sga->getProp()->ground->getnum())];
//       bex = FALSE;
//     }
//   }
//   else if (sga = dynamic_cast<simple_goal*>(a)) {
//       fta = gft_table[sga->getProp()->ground->getnum()];
//       bex = TRUE;
//   }

//   if(ngb = dynamic_cast<neg_goal*>(b)) {
//     if(sgb = dynamic_cast<simple_goal*>(ngb->getGoal())) {
//       ftb = gft_table[NEG_ADR(sgb->getProp()->ground->getnum())];
//       aex = FALSE;
//     }
//   }
//   else if (sgb = dynamic_cast<simple_goal*>(b)) {
//     ftb = gft_table[sgb->getProp()->ground->getnum()];
//     aex = TRUE;
//   }

//   if(aex){
//     b_excl = ftb->info_at[time]->exclusives->pos_exclusives;
//     b_exlab = ftb->info_at[time]->exclusives->p_exlabels[fta->index];
//  }
//   else{
//     b_excl = ftb->info_at[time]->exclusives->neg_exclusives;
//     b_exlab = ftb->info_at[time]->exclusives->n_exlabels[fta->index];
//   }

//   if(bex){
//     a_excl = fta->info_at[time]->exclusives->pos_exclusives;
//     a_exlab = fta->info_at[time]->exclusives->p_exlabels[ftb->index];
//   }
//   else{
//     a_excl = fta->info_at[time]->exclusives->neg_exclusives;
//     a_exlab = fta->info_at[time]->exclusives->n_exlabels[ftb->index];
//   }




//   if(isNonMutexCrossWorld(a_excl, a_exlab, fta->index, fta->info_at[time]->label->label, b_excl, b_exlab, ftb->index, ftb->info_at[time]->label->label, time))//non mutex for at least one pair of worlds
//     return TRUE; //can't show that a mutex exists
//   else
//     return FALSE;

// }


// int labelGraphClauseConsistent(goal* a, goal* b, int time){
//     //one pair of literals is non mutex


//   goal_list* a_literals = getClauseLiterals(a);
//   goal_list* b_literals = getClauseLiterals(b);
//   goal_list::iterator i = a_literals->begin();
//   goal_list::iterator j;

//   //  cout << "IS clauses consistent" << endl;
//   //a->display(0);
//   // b->display(0);

//   for(;i != a_literals->end();++i) {
//     j = b_literals->begin();
//     for(;j!= b_literals->end(); ++j) {
//       if(labelGraphLiteralConsistent(*i, *j, time))
// 	return TRUE;
//     }
//   }
//   return FALSE;

// }

// int labelGraphClausalStateConsistent(clausalState* cs, int time){
//   //all clauses are non mutex

//   goal_list* clauses = getGoalClauses(cs->getClauses());
//   goal_list::iterator i = clauses->begin();
//   goal_list::iterator j;

//   //    cout << "Is cs consistent at time " << time << endl;
//   //  cs->display();

//   for(;i != clauses->end();++i) {
//     j = i;
//     for(;j!= clauses->end(); ++j) {
//       if(!labelGraphClauseConsistent(*i, *j, time))
// 	return FALSE;
//     }
//   }
//   return TRUE;
// }


// int goal_non_mutex(int time){
//     cout << "IS GOAL NON MUTEX"<<endl;
//   return labelGraphClausalStateConsistent(goal_state, time);
// }

int factsFirstPresent(FactInfoPair* lits) {
	int return_val = -1;



	for(int n = 0; n<num_alt_facts; n++){
		//  cout << "CHECKING " << alt_facts[n]->get_name() << endl;

		if(get_bit(lits->positive->vector,gft_vector_length , n)) {
			if(return_val == -1 || pos_fact_at_min_level[n] > return_val){
				return_val = pos_fact_at_min_level[n];
				//     cout << "POS"<<endl;
			}
		}
		if(get_bit(lits->negative->vector,gft_vector_length , n))
			if(return_val == -1 || neg_fact_at_min_level[n] > return_val){
				return_val = neg_fact_at_min_level[n];
				//    cout << "NEG" <<endl;
			}
		//cout << "RETURN = " << return_val << endl;
	}
	return return_val;


}

RelaxedPlan* getLabelRelaxedPlan(DdNode* cs, int lev, DdNode* worlds){

	RelaxedPlan* tmp_plan = NULL;


	int num;

	   //cout << "LABEL RP"<<endl;
	   //printBDD(worlds);
	   //printBDD(cs);

	//  lev = getLabelProven(cs); //getLabeLevel(cs, best);

	//    cout << "GOT label proven"<<endl;

	//if(lev < IPP_MAX_PLAN) {
	//    if(!tmp_plan)
	// tmp_plan = new RelaxedPlan(IPP_MAX_PLAN, NULL);
	//    cs = cs->guestimateEasiestState();
	tmp_plan = getRelaxedPlan(lev, cs, FALSE, worlds);

	//    if(RP_COUNT == RPEFFS)
	//      num = tmp_plan->getRelaxedNumEffects();
	//    else if(RP_COUNT == RPACTS)
	//      num = tmp_plan->getRelaxedNumActions();
	//           cout << "NUM = " <<  num<< endl;




	return tmp_plan;
	//    return num;


	//  }
	//  else
	//    return NULL;

}
// goal* reduceToCNF(goal* g) {

//   if(g && !cnfIzed(g)) {
//     cout << "CNFize " << endl;
//     g->display(0);
//   CodeNode* cn = buildCodeNode(g);
//   //  cout << "Built CODE nODE"<<endl;
//   //  print_CodeNode(cn, 5);
//   cnf(cn);
//   //cout << "got CNF COIDE NOIDE"<<endl;
//   goal* return_goal = buildGoal(cn);
//   //  cout << "got CNF GOAL"<<endl;

//     return_goal = removeRepeats(return_goal);

//     //return_goal->display(0);
//     cout << "CNFized " << endl;
//     return_goal->display(0);

//   return return_goal;
//   }
//   else if(g)
//     return g;
//   else
//     return NULL;
// }

//goal* and_labels(goal* prevLabel, FtEdge* conditions, int time){
DdNode* and_labels(DdNode* prevLabel, FtEdge* conditions, int time){
  FtEdge* e1;
  DdNode *final_label, *tmp, *fr, *fr1;
  
  //    cout << "enter and lables" <<endl;
  

  if(prevLabel)
    final_label = prevLabel;
  else
    final_label = Cudd_ReadOne(manager);
  Cudd_Ref(final_label);

  //   if(!conditions)
	//     cout << "no conditions"<<endl;
  // 考虑每个前提条件BDD，进行前提条件label合取，即所有条件同时满足的label
  for(e1 = conditions;
      e1 &&
	// #ifdef PPDDL_PARSER
	// 	Cudd_ReadZero(manager) != final_label
	// #else
	Cudd_ReadLogicZero(manager) != final_label
	//#endif
	; e1=e1->next) {
    //            cout << "supported by:" << endl;
    //            printFact(e1->ft->index);
    //            printBDD(e1->ft->info_at[time]->label->label);
    if(e1->ft && e1->ft->info_at[time] && e1->ft->info_at[time]->label) {
      // #ifdef PPDDL_PARSER
      //       //if(final_label == Cudd_ReadOne(manager))

      //       fr = Cudd_addBddStrictThreshold(manager, final_label,0.0);
      //       Cudd_Ref(fr);
      //       fr1 = Cudd_addBddStrictThreshold(manager, e1->ft->info_at[time]->label->label, 0.0);
      //       Cudd_Ref(fr1);
      
      //       tmp = Cudd_bddAnd(manager, fr, fr1);
      //       Cudd_Ref(tmp);
      //       Cudd_RecursiveDeref(manager, fr);
      //       Cudd_RecursiveDeref(manager, fr1);
      //       fr1 = Cudd_BddToAdd(manager, tmp);
      //       Cudd_Ref(fr1);
      //       Cudd_RecursiveDeref(manager, tmp);
      
      //       if(final_label != Cudd_ReadOne(manager))
      // 	fr = Cudd_addApply(manager, Cudd_addMaximum, final_label, e1->ft->info_at[time]->label->label);
      //       else
      // 	fr = e1->ft->info_at[time]->label->label;
      //       Cudd_Ref(fr);
      
      //       printBDD(fr);
      //       printBDD(e1->ft->info_at[time]->label->label);
      
      
      //       tmp = Cudd_addApply(manager, Cudd_addTimes, fr, fr1);
      //       Cudd_Ref(tmp);
      //       Cudd_RecursiveDeref(manager, fr);
      //       Cudd_RecursiveDeref(manager, fr1);
      
      
      // 	//else
      // 	//tmp = Cudd_addApply(manager, Cudd_addMaximum, final_label, e1->ft->info_at[time]->label->label);
      // #else
      
      if(1)
	tmp = Cudd_bddAnd(manager, final_label, e1->ft->info_at[time]->label->label);
      //tmp = Cudd_bddIntersect(manager, final_label, e1->ft->info_at[time]->label->label);
      else
	tmp = Cudd_overApproxAnd(manager, final_label, e1->ft->info_at[time]->label->label);
      Cudd_Ref(tmp);
      //#endif
      
      Cudd_RecursiveDeref(manager, final_label);
      final_label = tmp;
      Cudd_Ref(final_label);
      Cudd_RecursiveDeref(manager, tmp);
//ao         break;
    }
	// 存在fact没有label，则该operator的label返回0
    else{
      Cudd_RecursiveDeref(manager, final_label);
      // #ifdef PPDDL_PARSER
      //       final_label = Cudd_ReadZero(manager);
      // #else
      final_label = Cudd_ReadLogicZero(manager);
      //#endif
      Cudd_Ref(final_label);
    }
  }
  //  printBDD(final_label);
  //  cout << "exit and lables" <<endl;
  return final_label;
}

DdNode*  analytical_label(FtNode* fact, int time){
  //if rbpf analytical node, then combine contributions of one action and then combine differetn action contributions,
  //multiply contributions by probability that noop does not contribute in same world
  //add contributions to noop contribution.

  std::map<int, DdNode*> op_labels;
  DdNode* noop_label;
  EfEdge* e1;
  Consequent *tmpCons;
  Label *tmpLab;


  //  std::cout << "compute goal pr " << time << std::endl;

  if(fact->noop && time > 0 && fact->info_at[time] != NULL){
    noop_label = fact->info_at[time]->label->label;
  }
  else{
    noop_label = Cudd_ReadLogicZero(manager);
  }

    for(e1 = fact->adders; e1; e1=e1->next) {
      if(e1->ef &&
	 e1->ef->info_at[time] &&
	 !e1->ef->info_at[time]->is_dummy &&
	 !e1->ef->op->is_noop
	 ){
// 	cout << "Supported by: " << e1->ef->op->name << " " << e1->ef->op->index << endl;
// 	printBDD(e1->ef->info_at[time]->label->label);
	tmpCons = e1->ef->effect->cons;
	tmpLab = e1->ef->info_at[time]->label;
	while(tmpCons && tmpLab){
	  if((fact->positive && get_bit(tmpCons->p_effects->vector, gft_vector_length, fact->index)) ||
	     (!fact->positive && get_bit(tmpCons->n_effects->vector, gft_vector_length, fact->index))){
	    DdNode* op_label = op_labels[e1->ef->op->index];
	    if(op_label == NULL){
	      op_label = Cudd_ReadZero(manager);
	    }
	    Cudd_Ref(op_label);
	    //	    std::cout << "Start reward to op: " << e1->ef->op->index << " " << time << std::endl;
	    DdNode* tmp = Cudd_BddToAdd(manager, tmpLab->label);
	    Cudd_Ref(tmp);

	    //	    printBDD(tmp);
	    std::set<int> *parents = &e1->ef->effect->node->parents;
	    double pr = 1.0;
	    
	    dbn* dbop = action_dbn(*(e1->ef->op->action));
		assert(e1->ef->effect->row != NULL);
		DdNode *rowDD = Cudd_addBddPattern(manager, e1->ef->effect->row);
		Cudd_Ref(rowDD);
		printBDD(e1->ef->effect->row);
	    for(std::set<int>::iterator p = parents->begin(); p != parents->end(); p++){
	      if(*p < 2*num_alt_facts) continue;
	      std::set<int> *dd_bits = dbop->vars[*p]->dd_bits;
	      for(std::set<int>::iterator d = dd_bits->begin();d != dd_bits->end(); d++){
		//		std::cout << "d = " << *d << std::endl;
		if(Cudd_bddIsVarEssential(manager, rowDD, *d, 0)){
		  pr *= dbop->vars[*p]->prs[0];
		  //std::cout << "pr = " << pr << std::endl;
		  break;
		}
	      }
	    }

	    pr*=pow(gDiscount, time);
	    DdNode* tmp2 = Cudd_addConst(manager, pr);
	    Cudd_Ref(tmp2);
	    DdNode* tmp3 = Cudd_addApply(manager, Cudd_addTimes, tmp, tmp2);
	    Cudd_Ref(tmp3);
	    //	    printBDD(tmp3);
	    //std::cout << "Add reward to op: " << e1->ef->op->index << std::endl;


	    DdNode* tmp1 = Cudd_addApply(manager, Cudd_addPlus, op_label, tmp3);
	    Cudd_Ref(tmp1);
	    //	    	    printBDD(tmp1);
	    //std::cout << "Done reward to op: " << e1->ef->op->index << std::endl;

	    Cudd_RecursiveDeref(manager, rowDD);
	    Cudd_RecursiveDeref(manager, tmp);
	    Cudd_RecursiveDeref(manager, tmp2);
	    Cudd_RecursiveDeref(manager, tmp3);
	    Cudd_RecursiveDeref(manager, op_label);
	    op_label = tmp1;
	    op_labels[e1->ef->op->index] = op_label;
					
	    
	  }
	  tmpCons = tmpCons->next;
	  tmpLab = tmpLab->next;
	}
      }
    }  


		//compute max worlds for each op
		DdNode *running_max = Cudd_ReadZero(manager);
		Cudd_Ref(running_max);
		for(std::map<int, DdNode*>::iterator i = op_labels.begin(); i != op_labels.end(); i++){
			DdNode *max_worlds = (*i).second;
			Cudd_Ref(max_worlds);
			//printBDD(max_worlds);
			//			std::cout << "checking act i=" << (*i).first << std::endl;
			for(std::map<int, DdNode*>::iterator j = op_labels.begin(); j != op_labels.end(); j++){
				if(j == i) continue;
// 				std::cout << "checking act j=" << (*j).first << std::endl;
// 				printBDD(max_worlds);
// 				printBDD((*j).second);

				DdNode* new_max = Cudd_addApply(manager, Cudd_addThreshold, max_worlds, (*j).second);
				//				printBDD(new_max);
				Cudd_Ref(new_max);
				Cudd_RecursiveDeref(manager, max_worlds);
				max_worlds = new_max;//Cudd_addApply(manager, Cudd_addTimes, new_max, max_worlds);
				//Cudd_Ref(max_worlds);
				//printBDD(max_worlds);
				if(max_worlds == Cudd_ReadZero(manager))
					break;
			
			}			
// 			std::cout << "act i=" << (*i).first << " is max in " << std::endl;
// 			printBDD(max_worlds);

			//need to remove from max_worlds cases that are equal to running_max
				DdNode* tmp = Cudd_addApply(manager, Cudd_addStrictThreshold, max_worlds, running_max);
				Cudd_Ref(tmp);
				Cudd_RecursiveDeref(manager, max_worlds);
				max_worlds = tmp;

// 			std::cout << "running act i=" << (*i).first << " is max in " << std::endl;
// 			printBDD(max_worlds);
			
			 tmp = Cudd_addApply(manager, Cudd_addMaximum, max_worlds, running_max);
			Cudd_Ref(tmp);
			Cudd_RecursiveDeref(manager, running_max);
			running_max = tmp;
			for(OpNode *o = gall_ops_pointer; o != NULL; o = o->next){
				if(o->index == (*i).first){
					DdNode* max_bdd = Cudd_addBddPattern(manager, max_worlds);
					Cudd_Ref(max_bdd);
					o->info_at[time]->max_worlds = max_bdd;
					Cudd_RecursiveDeref(manager, max_worlds);
					//std::cout << "o = " << o->index << " " << o << std::endl;
					//printBDD(max_bdd);
					break;
				}
			}
		}

    for(e1 = fact->adders; e1; e1=e1->next) {
      if(e1->ef &&
	 e1->ef->info_at[time] &&
	 !e1->ef->info_at[time]->is_dummy &&
	 !e1->ef->op->is_noop
	 ){
	//	std::cout << "o1 = " << e1->ef->op->index << " " << e1->ef->op << std::endl;
	//printBDD( e1->ef->op->info_at[time]->max_worlds);
	DdNode *tmp = Cudd_bddAnd(manager, e1->ef->info_at[time]->label->label, e1->ef->op->info_at[time]->max_worlds);
	Cudd_Ref(tmp);
	//		printBDD(tmp);
	Cudd_RecursiveDeref(manager, e1->ef->info_at[time]->label->label);
	e1->ef->info_at[time]->label->label = tmp;
      }
    }


    DdNode* maxPr = Cudd_ReadZero(manager);
    Cudd_Ref(maxPr);
    for(std::map<int, DdNode*>::iterator i = op_labels.begin(); i != op_labels.end(); i++){
      //      std::cout << "Max reward to op: " << (*i).first << std::endl;
      //printBDD((*i).second);
      DdNode* tmp = Cudd_addApply(manager, Cudd_addMaximum, maxPr, (*i).second);
      Cudd_Ref(tmp);
      Cudd_RecursiveDeref(manager, maxPr);
      maxPr = tmp;
    }

    DdNode* tmp = Cudd_addBddPattern(manager, maxPr);
    Cudd_Ref(tmp);
    Cudd_RecursiveDeref(manager, maxPr);
    maxPr = tmp;


    tmp = Cudd_bddOr(manager, maxPr, noop_label);
    Cudd_Ref(tmp);
    Cudd_RecursiveDeref(manager, maxPr);
    maxPr = tmp;



    Cudd_RecursiveDeref(manager, noop_label);
    for(std::map<int, DdNode*>::iterator i = op_labels.begin(); i != op_labels.end(); i++){
      Cudd_RecursiveDeref(manager, (*i).second);
    }

    //    printBDD(maxPr);

    //  std::cout << "done compute goal pr" << std::endl;
    return maxPr;
}

DdNode* or_labels(FtNode* fact, int time){
  EfEdge* e1;
  DdNode *final_label, *tmp, *fr;
  final_label = Cudd_ReadLogicZero(manager);

  Cudd_Ref(final_label);
  
  Consequent *tmpCons;
  Label *tmpLab;



  if(RBPF_LUG && fact->index == num_alt_facts-2 && fact->positive){
    return analytical_label(fact, time);
  }

  if(fact->adders){
    
    for(e1 = fact->adders; e1; e1=e1->next) {
      if(e1->ef &&
	 e1->ef->info_at[time] &&
	 !e1->ef->info_at[time]->is_dummy
	 ){
	// 		cout << "Supported by: " << e1->ef->op->name << endl;
	// 		printBDD(e1->ef->info_at[time]->label->label);
	tmpCons = e1->ef->effect->cons;
	tmpLab = e1->ef->info_at[time]->label;
	while(tmpCons && tmpLab){
	  if((fact->positive && get_bit(tmpCons->p_effects->vector, gft_vector_length, fact->index)) ||
	     (!fact->positive && get_bit(tmpCons->n_effects->vector, gft_vector_length, fact->index))){
	    // #ifdef PPDDL_PARSER
	    // // 	    cout << "max " <<endl;
	    // // 	    printBDD(tmpLab->label);
	    // // 	    printBDD(final_label);
	    
	    // 	    tmp = Cudd_addApply(manager, Cudd_addMaximum, tmpLab->label, final_label);
	    // 	    //	    printBDD(tmp);
	    // #else
	    if(1)
	      tmp = Cudd_bddOr(manager, tmpLab->label, final_label);
	    else
	      tmp = Cudd_overApproxOr(manager, tmpLab->label, final_label);
	    
	    //#endif
	    Cudd_Ref(tmp);
	    Cudd_RecursiveDeref(manager, final_label);
	    final_label = tmp;
	    Cudd_Ref(final_label);
	    Cudd_RecursiveDeref(manager, tmp);
	    // printBDD(final_label);
	  }
	  tmpCons = tmpCons->next;
	  tmpLab = tmpLab->next;
	}
      }
    }
  }

	//  if(time > 0 &&
	//      fact->info_at[time] &&
	//      !fact->info_at[time]->is_dummy &&
	//      fact->info_at[time]->label != NULL &&
	// #ifdef PPDDL_PARSER
	//     Cudd_ReadZero(manager) != fact->info_at[time]->label->label
	// #else
	//     Cudd_ReadLogicZero(manager) != fact->info_at[time]->label->label
	// #endif
	// ){
	// #ifdef PPDDL_PARSER
	//    tmp = Cudd_addApply(manager, Cudd_addMaximum, fact->info_at[time]->label->label, final_label);
	// #else
	//    tmp = Cudd_bddOr(manager, fact->info_at[time]->label->label, final_label);
	// #endif
	//     //    printBDD(tmp);
	//     Cudd_Ref(tmp);
	//     Cudd_RecursiveDeref(manager, final_label);
	//     final_label = tmp;
	//     Cudd_Ref(final_label);
	//     Cudd_RecursiveDeref(manager, tmp);

	//  }
	//  else if( fact->noop &&
	// 	  fact->noop->unconditional &&
	// 	  fact->noop->unconditional->info_at[time] &&
	// 	  !fact->noop->unconditional->info_at[time]->is_dummy &&
	// 	  fact->noop->unconditional->info_at[time]->label){
	// #ifdef PPDDL_PARSER
	//    tmp = Cudd_addApply(manager, Cudd_addMaximum, final_label, fact->noop->unconditional->info_at[time]->label->label);
	// #else
	//    tmp = Cudd_bddOr(manager, final_label, fact->noop->unconditional->info_at[time]->label->label);
	// #endif

	//    Cudd_Ref(tmp);
	//    Cudd_RecursiveDeref(manager, final_label);
	//    final_label = tmp;
	//    Cudd_Ref(final_label);
	//    Cudd_RecursiveDeref(manager, tmp);
	//   }
	//  else if(time == 0 &&    fact->positive &&    init_pos_labels[fact->index]){
	// #ifdef PPDDL_PARSER
	//    final_label = Cudd_addApply(manager, Cudd_addMaximum, init_pos_labels[fact->index], fr=final_label);
	// #else
	//    final_label = Cudd_bddOr(manager, init_pos_labels[fact->index], fr=final_label);
	// #endif
	//    Cudd_Ref(final_label);
	//    Cudd_RecursiveDeref(manager, fr);
	//  }
	//  else if(time == 0 && !fact->positive && init_neg_labels[fact->index]){
	// #ifdef PPDDL_PARSER
	//    final_label = Cudd_addApply(manager, Cudd_addMaximum, init_neg_labels[fact->index], fr=final_label);
	// #else
	//    final_label = Cudd_bddOr(manager, init_neg_labels[fact->index], fr=final_label);
	// #endif
	//    Cudd_Ref(final_label);
	//    Cudd_RecursiveDeref(manager, fr);
	//  }
	//  cout << "exit or lables" <<endl;
	//      Cudd_CheckKeys(manager);fflush(stdout);
	return final_label;
}


#ifdef PPDDL_PARSER
#else

DdNode* goalToBDD(goal* cs) {
	DdNode *acc, *op1, *op2;
	simple_goal* sg;
	conj_goal* cg;
	disj_goal* dg;
	neg_goal* ng;
	goal_list::iterator i;
	int var;

	if(sg = dynamic_cast<simple_goal*>(cs)) {
		//    cout << "LITERLA"<<endl;
		var = sg->getProp()->getHashEntry();
		acc = Cudd_bddIthVar(manager,var);
		if(sg->getPolarity()==E_NEG)  {
			acc = Cudd_Not(acc);
		}
		Cudd_Ref(acc);
		//printBDD(acc);
		return acc;
	}

	if(ng = dynamic_cast<neg_goal*>(cs)) {
		//cout << "Negate" << endl;
		acc = Cudd_Not(op1=goalToBDD(ng->getGoal()));
		Cudd_Ref(acc);
		Cudd_RecursiveDeref(manager,op1);
		return acc;
	}

	if(cg = dynamic_cast<conj_goal*>(cs)) {
		//cout << "Conjoin" << endl;
		acc = Cudd_ReadOne(manager);
		Cudd_Ref(acc);

		i = cg->getGoals()->begin();
		for(;i!=cg->getGoals()->end();++i) {
			acc = Cudd_bddAnd(manager,op1=acc,op2=goalToBDD(*i));
			Cudd_Ref(acc);
			Cudd_RecursiveDeref(manager,op1);
			Cudd_RecursiveDeref(manager,op2);
		}
		return acc;
	}

	if(dg = dynamic_cast<disj_goal*>(cs)) {
		//  cout << "Disjoin " << dg->type << endl;
		//    dg->display(0);
		acc=Cudd_ReadLogicZero(manager);
		Cudd_Ref(acc);
		switch(dg->type)
		{
		case 0:
			i = dg->getGoals()->begin();
			for(;i!=dg->getGoals()->end();++i) {
				//  (*i)->display(0);
				acc = Cudd_bddOr(manager,op1=acc,op2=goalToBDD(*i));
				Cudd_Ref(acc);
				Cudd_RecursiveDeref(manager,op1);
				Cudd_RecursiveDeref(manager,op2);
			}
			break;
		case 1:
			i = dg->getGoals()->begin();
			op1 = goalToBDD(*i);
			acc = bdd_or(manager,op1,Cudd_Not(op1));
			Cudd_RecursiveDeref(manager,op1);
			break;
		case 2:
			assert("Not Imlemented"==0);
			// see groundOnOf
			break;
		default:
			assert("Not Implemented"==0);
			;
		}
		//    printBDD(acc);

		return acc;
	}

	assert("Not implemented"==0);
	//  acc = Cudd_ReadLogicZero(manager);
	acc = Cudd_ReadOne(manager);
	Cudd_Ref(acc);
	return acc;
}


DdNode* goalToBDD(goal* cs, int withoutLabels) {

	DdNode *f, *g, *var, *tmp;
	simple_goal* sg;
	conj_goal* cg, *cg1;
	disj_goal* dg, *dg1;
	neg_goal* ng;
	goal_list::iterator i;
	goal_list::iterator j;

	// cout << "Enter goalToBDD, cs = "<<endl;
	//     cout << "WITHOUT LABELS = " << withoutLabels << endl;
	//      if(cs)
	//    cs->display(0);
	//      else
	//        cout << "NULL"<<endl;
	//    if(manager)
	//        cout << "manager ok"<<endl;

	//f = NULL;

	f = Cudd_ReadOne(manager);
	Cudd_Ref(f);


	if(cg = dynamic_cast<conj_goal*>(cs)) {
		i = cg->getGoals()->begin();
		for(;i!=cg->getGoals()->end();++i) {
			//    cout << "Its a cg"<<endl;
			if(dg = dynamic_cast<disj_goal*>(*i)) {
				j = dg->getGoals()->begin();
				//cout << "Its a dg"<<endl;
				var = Cudd_ReadOne(manager);
				for(;j!=dg->getGoals()->end();++j) {
					if(cg1 = dynamic_cast<conj_goal*>(*j)){
						if(!bdd_is_one(manager, var)) {
							tmp = bdd_or(manager, g=goalToBDD(cg1, TRUE), var);
							Cudd_RecursiveDeref(manager, var);
							Cudd_RecursiveDeref(manager, g);
							var = tmp;
							//Cudd_Ref(var);
						}
						else {
							var = goalToBDD(cg1, TRUE);
							//Cudd_Ref(var);
						}	  }
					else if(dg1 = dynamic_cast<disj_goal*>(*j)){
						if(!bdd_is_one(manager, var)) {
							tmp = bdd_or(manager, g=goalToBDD(dg1, TRUE), var);
							Cudd_RecursiveDeref(manager, var);
							Cudd_RecursiveDeref(manager, g);
							var = tmp;
							//Cudd_Ref(var);
						}
						else {
							var = goalToBDD(dg1, TRUE);
							//Cudd_Ref(var);
						}
					}
					else if(sg = dynamic_cast<simple_goal*>(*j)) {
						//  cout << "Its a dg sg"<<endl;
						g = Cudd_bddIthVar(manager, sg->getProp()->getHashEntry());
						//Cudd_Ref(g);
						//tmp = Cudd_bddOr(manager, g, var);
						// Bug? !bdd_is_zero is what it should be? - Will
						if(!bdd_is_one(manager, var)) {
							tmp = bdd_or(manager, g, var);
							Cudd_RecursiveDeref(manager, var);
							var = tmp;
							//Cudd_Ref(var);
						}
						else {
							var = g;
							Cudd_Ref(var);
						}
					}
					else if(ng = dynamic_cast<neg_goal*>(*j)) {
						if(sg = dynamic_cast<simple_goal*>(ng->getGoal())) {
							//cout << "Its a dg ng"<<endl;
							g = Cudd_Not(Cudd_bddIthVar(manager, sg->getProp()->getHashEntry()));
							Cudd_Ref(g);
							// tmp = Cudd_bddOr(manager, Cudd_Not(g), var);
							if(!bdd_is_one(manager, var)) {
								tmp = bdd_or(manager, g, var);
								Cudd_RecursiveDeref(manager, var);
								Cudd_RecursiveDeref(manager, g);
								var = tmp;
								//Cudd_Ref(var);
							}
							else{
								var = g;
								//Cudd_Ref(var);
							}
						}
					}
				}
				//	tmp = Cudd_bddAnd(manager, var, f);
				if(f) {
					tmp = bdd_and(manager, var, f);
					Cudd_RecursiveDeref(manager, f);
					Cudd_RecursiveDeref(manager, var);
					f = tmp;
					//Cudd_Ref(f);
				}
				else{
					// 	  cout << "YO"<<endl;
					f = var;
					if(bdd_is_one(manager, f))
						Cudd_Ref(f);
					//Cudd_Ref(f);
					//Cudd_RecursiveDeref(manager, var);
					//Cudd_Ref(f);
				}
			}
			else if(sg = dynamic_cast<simple_goal*>(*i)) {
				//	cout << "Its a sg"<<endl;
				if(!withoutLabels) {
					var = Cudd_ReadOne(manager);
				}
				else {
					var = Cudd_bddIthVar(manager, sg->getProp()->getHashEntry());
					//Cudd_Ref(var);
					//	tmp = Cudd_bddAnd(manager, var, f);
					//	if(!bdd_is_one(manager, f)) {
					tmp = bdd_and(manager, var, f);
					Cudd_RecursiveDeref(manager, f);
					f = tmp;
					//Cudd_Ref(f);
					//}
					//else
					//f = var;
				}

			}
			else if(ng = dynamic_cast<neg_goal*>(*i)) {
				if(sg = dynamic_cast<simple_goal*>(ng->getGoal())) {
					// 	cout << "Its a ng"<<endl;
					if(!withoutLabels) {
						var = Cudd_ReadOne(manager);

					}
					else {
						var = Cudd_Not(Cudd_bddIthVar(manager, sg->getProp()->getHashEntry()));
						Cudd_Ref(var);
						//	  tmp = Cudd_bddAnd(manager, Cudd_Not(var), f);
						//	if(!bdd_is_one(manager, f)) {
						tmp = bdd_and(manager, var, f);
						Cudd_RecursiveDeref(manager, f);
						Cudd_RecursiveDeref(manager, var);
						f = tmp;
						// Cudd_Ref(f);
						//	}
						//else
						//f = var;
					}
				}
			}
		}
	}
	else if(dg = dynamic_cast<disj_goal*>(cs)) {
		var = Cudd_ReadOne(manager);
		j = dg->getGoals()->begin();
		for(;j!=dg->getGoals()->end();++j) {
			if(cg1 = dynamic_cast<conj_goal*>(*j)){

				if(!bdd_is_one(manager, var)) {
					tmp = bdd_or(manager, g=goalToBDD(cg1, TRUE), var);
					Cudd_RecursiveDeref(manager, var);
					Cudd_RecursiveDeref(manager, g);
					var = tmp;
					//Cudd_Ref(var);
				}
				else {
					var = goalToBDD(cg1, TRUE);
					//Cudd_Ref(var);
				}
			}
			else if(sg = dynamic_cast<simple_goal*>(*j)) {
				var = Cudd_bddIthVar(manager, sg->getProp()->getHashEntry());
				//	tmp = Cudd_bddAnd(manager, var, f);

				// f not set here?

				if(!bdd_is_one(manager, f)) {
					tmp = bdd_or(manager, var, f);
					Cudd_RecursiveDeref(manager, f);
					f = tmp;
					//Cudd_Ref(f);
				}

				else{
					f = var;
					Cudd_Ref(f);
				}
			}
			else if(ng = dynamic_cast<neg_goal*>(*j)) {
				if(sg = dynamic_cast<simple_goal*>(ng->getGoal())) {
					var = Cudd_Not(Cudd_bddIthVar(manager, sg->getProp()->getHashEntry()));
					Cudd_Ref(var);

					//	tmp = Cudd_bddAnd(manager, Cudd_Not(var), f);
					if(!bdd_is_one(manager, f)) {
						tmp = bdd_or(manager, var, f);
						Cudd_RecursiveDeref(manager, var);
						Cudd_RecursiveDeref(manager, f);
						f = tmp;
						//Cudd_Ref(f);

					}
					else{
						f = var;
					}
				}
			}
		}
	}
	else if(sg = dynamic_cast<simple_goal*>(cs)) {
		//cout << "It's a Simple (Positive) Goal" << endl;
		f = Cudd_bddIthVar(manager, sg->getProp()->getHashEntry());
		//  f = Cudd_ReadOne(manager);
		Cudd_Ref(f);
	}
	else if(ng = dynamic_cast<neg_goal*>(cs)) {
		if(sg = dynamic_cast<simple_goal*>(ng->getGoal())) {
			//cout << "It's a Negation of a Simple (Positive) Goal" << endl;
			assert(sg);
			assert(sg->getProp());
			//sg->display(0);
			f = Cudd_Not(Cudd_bddIthVar(manager, sg->getProp()->getHashEntry()));
			Cudd_Ref(f);
		}
		//     f = Cudd_ReadOne(manager);
		//     Cudd_Ref(f);

	}
	else if(!cs) {
		f = Cudd_ReadOne(manager);
		Cudd_Ref(f);

	}

	//   if(bdd_is_one(manager, f))
	//cout << "BDD IS ONE" <<endl;
	//   cout << "goalToBDD returns = " <<endl;
	//   printBDD(f);
	return f;
}
#endif

void printBDD(DdNode* bdd) {

	if(!bdd)
		cout << "BDD is null pointer"<<endl;

	else  if(bdd == Cudd_ReadLogicZero(manager)){
		cout << "BDD is FALSE"<<endl;
		//     Cudd_PrintDebug(manager,bdd , bdd_size(manager, bdd), 4);
	}
	else  if(bdd == Cudd_ReadZero(manager)){
		cout << "BDD is ZERO"<<endl;
		//     Cudd_PrintDebug(manager,bdd , bdd_size(manager, bdd), 4);
	}
	else if(bdd == Cudd_ReadOne(manager)){
		cout << "BDD is TRUE" <<endl;
		//     Cudd_PrintDebug(manager,bdd , bdd_size(manager, bdd), 4);
	}
	else {
		cout << "BDD is Formula, ref = " << bdd->ref <<endl;
		//Cudd_PrintDebug(manager,bdd , bdd_size(manager, bdd), 4);
		cout << "the Minterm of BDD is: \n";
		Cudd_PrintMinterm(manager, bdd);

		//        cout << "# PATHS = " << Cudd_CountPath(bdd) << endl;
		//        cout << "Supported by = " <<endl;
		//    Cudd_PrintDebug(manager, Cudd_Support(manager, bdd), 100, 3);

	}

	cout << flush;
}

// //already know its in init, just checking if its in a disjunctive clause
// int uncertainInInit(goal* g) {
// //   cout << "SEEing if uncertain = "<<endl;
// //   g->display(1);

//   simple_goal* sg;
//   neg_goal* ng;
//   disj_goal* dg;
//   goal* tmp = NULL;
//   int polarity = TRUE;
//   goal_list* matches = new goal_list();

//  if(sg = dynamic_cast<simple_goal*>(g)) {
//    polarity = TRUE;
//    tmp =  (goal*) inClauseListHelper(sg->getProp()->getGrounding(), initial_state->getClauses(), polarity, FALSE, matches);
//    for(goal_list::iterator i = matches->begin(); i != matches->end(); ++i) {
//      if(dg = dynamic_cast<disj_goal*>(*i))
//        return TRUE;
//    }
//  }
//  else if(ng = dynamic_cast<neg_goal*>(g)) {
//    if(sg = dynamic_cast<simple_goal*>(ng->getGoal())) {

//      polarity = FALSE;
//      tmp = (goal*) inClauseListHelper(sg->getProp()->getGrounding(), initial_state->getClauses(), polarity, FALSE, matches);
//      for(goal_list::iterator i = matches->begin(); i != matches->end(); ++i) {
//        if(dg = dynamic_cast<disj_goal*>(*i))
// 	 return TRUE;
//      }
//    }
//  }
//  // cout << "NO"<<endl;
//  return FALSE;
// }



// void generate_ini_goal_bitmap_representation(clausalState* init, clausalState* goal)

// {

//   simple_goal* sg;
//   conj_goal* cg;
//   disj_goal* dg;
//   neg_goal* ng;



//   FactInfo *tpos = new_FactInfo();
//   FactInfo *tneg = new_FactInfo();
//   //  CodeNode *i, *l;
//   //int j;
//   //  RelevantFact *tmp;
//   //BitOperator *tmpop;

//   //  cout << "NUM COND EFFS = " << gnum_cond_effects << endl;
//   //    cout << "NUM FACTS = "  << num_alt_facts << endl;
//   gft_vector_length =  ( ( int ) num_alt_facts / gcword_size )+1;
//   gop_vector_length = ((int) gnum_cond_effects_pre /gcword_size)+1;

//   //  cout << "gop_Vector_length = " << gop_vector_length <<endl;

//   /* goal state is non triv //  clausalState* tmp = new clausalState(final_label);
// //    tmp->sortClauses(TRUE);
// //    final_label = removeRepeats(reduceClause(tmp->getClauses(), TRUE));
// //    tmp->sortClauses(TRUE);ial, otherwise we would have stopped already.
//    */
//   //DNF-ize goal
//   ////  dnf( gcode_goal_state );
//   //if not disj goal
//   ////if ( gcode_goal_state->connective != OR ) {
//   if(!(dg = dynamic_cast<disj_goal*>(goal->getClauses()))){
//     //if not conj goal
//     ////if ( gcode_goal_state->connective != AND ) {
//     if(!(cg = dynamic_cast<conj_goal*>(goal->getClauses()))){
//       //if neg goal
//       ////if ( gcode_goal_state->connective == NOT ) {
//       if((ng = dynamic_cast<neg_goal*>(goal->getClauses()))){
//        	//add neg atom
// 	if((sg = dynamic_cast<simple_goal*>(ng->getGoal())))
// 	  make_entry_in_FactInfo( &tneg, sg->getProp()->ground->getnum());
//       } else if((sg = dynamic_cast<simple_goal*>(goal->getClauses()))) {
// 	//add pos atom
// 	make_entry_in_FactInfo( &tpos,  sg->getProp()->ground->getnum() );
//       }
//     } else { //is a conj goal
//       //for each conjunct
//       goal_list::iterator k = cg->getGoals()->begin();
//       for (; k != cg->getGoals()->end();++k ) {
// 	//if neg goal
// 	if((ng = dynamic_cast<neg_goal*>(*k))){
// 	  //add neg atom
// 	  if((sg = dynamic_cast<simple_goal*>(ng->getGoal())))
// 	    make_entry_in_FactInfo( &tneg, sg->getProp()->ground->getnum());

// 	} else if((sg = dynamic_cast<simple_goal*>(*k))){

// 	  //add pos atom
// 	  make_entry_in_FactInfo( &tpos,  sg->getProp()->ground->getnum() );
// 	}
//       }
//     }
//   } else{}


//   //add atom to goal rep
//   gbit_goal_state = new_fact_info_pair( tpos, tneg );
//   //    cout << "BIT GOAL" << endl;


//   tpos = new_FactInfo();
//   tneg = new_FactInfo();



//   if((sg = dynamic_cast<simple_goal*>(init->getClauses()))){
// 	    if ( !get_bit( tpos->vector, gft_vector_length, sg->getProp()->ground->getnum() ))
//     make_entry_in_FactInfo( &tpos, sg->getProp()->ground->getnum() );
//   }
//   else if((cg = dynamic_cast<conj_goal*>(init->getClauses()))){
//     goal_list::iterator k = cg->getGoals()->begin();
//     for (; k != cg->getGoals()->end();++k ) {
//       //if neg goal
//       if((ng = dynamic_cast<neg_goal*>(*k))){
// 	//add neg atom
// 	if((sg = dynamic_cast<simple_goal*>(ng->getGoal())))
// 	      if ( !get_bit( tneg->vector, gft_vector_length, sg->getProp()->ground->getnum() ))
// 	  make_entry_in_FactInfo( &tneg, sg->getProp()->ground->getnum());

//       } else if((sg = dynamic_cast<simple_goal*>(*k))){

// 	//add pos atom
// 	    if ( !get_bit( tpos->vector, gft_vector_length, sg->getProp()->ground->getnum() ))
// 	make_entry_in_FactInfo( &tpos,  sg->getProp()->ground->getnum() );
//       } else if((dg = dynamic_cast<disj_goal*>(*k))) {
// 	goal_list::iterator l = dg->getGoals()->begin();
// 	for (; l != dg->getGoals()->end();++l ) {
// 	  //if neg goal
// 	  if((ng = dynamic_cast<neg_goal*>(*l))){
// 	      //add neg atom
// 	    if((sg = dynamic_cast<simple_goal*>(ng->getGoal()))) {
// 	      if ( !get_bit( tneg->vector, gft_vector_length, sg->getProp()->ground->getnum() ))
// 		make_entry_in_FactInfo( &tneg, sg->getProp()->ground->getnum());
// 	      //  if(COMPUTE_LABELS){
// //  		//dg->display(0);
// // 		 	init_neg_labels[sg->getProp()->ground->getnum()] = extract_label(ng, dg);
// //  	      }

// 	    }

// 	  } else if((sg = dynamic_cast<simple_goal*>(*l))){

// 	    //add pos atom
// 	    if ( !get_bit( tpos->vector, gft_vector_length, sg->getProp()->ground->getnum() ))
// 	      make_entry_in_FactInfo( &tpos,  sg->getProp()->ground->getnum() );
// 	     //  if(COMPUTE_LABELS){
// //  		//dg->display(0);
// // 			init_pos_labels[sg->getProp()->ground->getnum()] = extract_label(sg, dg);
// //  	      }
// 	  }
// 	}
//       }
//     }
//   }
//   else if((dg = dynamic_cast<disj_goal*>(init->getClauses()))){
//     goal_list::iterator l = dg->getGoals()->begin();
//     for (; l != dg->getGoals()->end();++l ) {
//       //if neg goal
//       if((ng = dynamic_cast<neg_goal*>(*l))){
// 	//add neg atom
// 	if((sg = dynamic_cast<simple_goal*>(ng->getGoal())))
// 	  if ( !get_bit( tneg->vector, gft_vector_length, sg->getProp()->ground->getnum() ))
// 	    make_entry_in_FactInfo( &tneg, sg->getProp()->ground->getnum());

//       } else if((sg = dynamic_cast<simple_goal*>(*l))){
// 	//add pos atom
// 	if ( !get_bit( tpos->vector, gft_vector_length, sg->getProp()->ground->getnum() ))
// 	  make_entry_in_FactInfo( &tpos,  sg->getProp()->ground->getnum() );
//       }
//     }
//   }


//   //get initial state's label
//   if(COMPUTE_LABELS) {
//     //  cout << "Initing bdd manager"<<endl;

//     //   manager = Cudd_Init(num_alt_facts, 0, CUDD_UNIQUE_SLOTS, CUDD_CACHE_SLOTS, 0);

//     //get labels for all initial literals
//     for(int i = 0; i < num_alt_facts; i++) {
//       for(int j = 0; j < 2; j++) {
// 	//	cout << "GOT INIT LABEL for :" << alt_facts[i]->get_name() << " " << j<< endl;
// 	if(j == 0) { //try to get neg label
// 	  //init_neg_labels[i] = reduceClause(extract_init_label(i, init, FALSE), TRUE);
// 	  if(init->inClauseList(alt_facts[i]->get_name(), FALSE)) {
// 	    //  cout << "In Init"<<endl;
// 	    if(LABEL_TYPE==STANDARD_LABELS)
// 	      init_neg_labels[i] = goalToBDD(reduceClause(extract_init_label(i, init, FALSE), TRUE), FALSE);
// 	    else if (LABEL_TYPE==ATMS_LABELS) {
// 	      ng = new neg_goal(new simple_goal(new proposition(alt_facts[i]), E_POS));
// 	      if(uncertainInInit(ng)){
// 		init_neg_labels[i] = goalToBDD(ng, FALSE);
// 		//	cout << "UNcertaint" << endl;
// 	      }
// 	      else {
// 		init_neg_labels[i] = Cudd_ReadOne(manager);
// 	      }
// 	    }
// 	     Cudd_Ref( init_neg_labels[i]);
// 	  }
// 	  else{
// 	    init_neg_labels[i]= Cudd_ReadLogicZero(manager);
// 	    Cudd_Ref( init_neg_labels[i]);
// 	  }
// 	  //  printBDD( init_neg_labels[i]);

// // 	   if(init_neg_labels[i])
// // 	       init_neg_labels[i]->display(0);
// 	}
// 	else if(j == 1) {//try to get pos label
// 	  /////init_pos_labels[i] = reduceClause(extract_init_label(i, init, TRUE), TRUE);
// 	  if(init->inClauseList(alt_facts[i]->get_name(), TRUE)) {
// 	    //  cout << "In Init"<<endl;
// 	    if(LABEL_TYPE==STANDARD_LABELS)
// 	      init_pos_labels[i] = goalToBDD(reduceClause(extract_init_label(i, init, TRUE), TRUE), FALSE);
// 	    else if(LABEL_TYPE==ATMS_LABELS) {
// 	      sg = new simple_goal(new proposition(alt_facts[i]), E_POS);
// 	      if(uncertainInInit(sg)){
// 		//		cout << "UNcertaint" << endl;
// 	init_pos_labels[i] = goalToBDD(sg, FALSE);
// 	      }
// 	      else{
// 		init_pos_labels[i] = Cudd_ReadOne(manager);
// 	      }
// 	    }

// 	    Cudd_Ref(init_pos_labels[i]);
// 	  }
// 	  else{
// 	    init_pos_labels[i] = Cudd_ReadLogicZero(manager);
// 	    Cudd_Ref(init_pos_labels[i]);
// 	  }
// 	  //	  printBDD( init_pos_labels[i]);

// 	  // if(init_pos_labels[i])
// 	  //   init_pos_labels[i]->display(0);
// 	}
//       }
//     }




//     //for every non-unit clause, insert clause into model checker
//     //  cout << "BUILDING init model"<<endl;


//     //output init state to model checker


//     //    cout << "About to make init bdd"<<endl;
//      initialBDD =  goalToBDD(init->getClauses(), FALSE);

//      //    cout << "Made intial bdd"<<endl;

//        // cout << "Background = " <<endl;
//        Cudd_SetBackground(manager, Cudd_ReadLogicZero(manager));

//        //     printBDD(Cudd_ReadBackground(manager));
//        if(!initialBDD){
// 	 initialBDD = Cudd_ReadOne(manager);
//        }
//        else{
// 	 //     printBDD(initialBDD);
//        //       cout << "# PATHS = " << Cudd_CountPath(initialBDD) << endl;
// 	 //     extractDNFfromBDD(initialBDD);
//        }

//  //       printBDD(bdd_pick_one_minterm_rand(manager,
// // 				 initialBDD,
// // 				  Cudd_ReadVar(manager),
// // 				  dd_get_size(manager)));
// //        printBDD(bdd_pick_one_minterm_rand(manager,
// // 				 initialBDD,
// // 				  Cudd_ReadVar(manager),
// // 				  dd_get_size(manager)));



// //        DdNode **result = Cudd_bddPickArbitraryMinterms( manager,
// // 							initialBDD,
// // 							supports,
// // 							sizeofSupports,
// // 							sizeofSupports);
// //  cout << "YO"<<endl;
// //  for(int l = 0 ; l < sizeofSupports; l++){

// //    if( result[l])
// //       printBDD(result[l]);
// //  }
// //   // printBDD(result[1]);
// //  cout << "Y1O"<<endl;






//        //   printBDD(Cudd_Support(manager, initialBDD));
//      //          Cudd_PrintDebug(manager, initialBDD, 100, 3);
// // 	  cout << "Here is the minterms"<<endl;
// // 	  cout << dd_printminterm(manager, initialBDD) <<endl;
// // 	  cout << "SIZE = " <<endl;
// // 	  cout << bdd_count_minterm(manager, initialBDD, bdd_size(manager, initialBDD)) <<endl;


// 	  //     Cudd_PrintLinear(manager);


// //     char* ini = "initial";
// //     build_model(init->getClauses(), ini);
// //     cout << "Done BUILDING init model" << endl;
// //     build_init_entailer();
// //     build_entailer();
// //   cout << "Done BUILDING entailer" << endl;






//   }


//   //add all of the neg preds to satisfy CWA in init
// //   for (int j = 0; j<num_alt_facts; j++ ) {
// //      if ( !get_bit( tpos->vector, gft_vector_length, j )
// // 	  && !get_bit( tneg->vector, gft_vector_length, j )) {
// //        //    cout << "CWA for " << alt_facts[j]->get_name() << endl;
// //        make_entry_in_FactInfo( &tneg, j );
// //        init_pos_labels[j] = Cudd_ReadLogicZero(manager);
// //    	Cudd_Ref(init_pos_labels[j]);
// // 	init_neg_labels[j] = Cudd_ReadOne(manager);
// //      	Cudd_Ref(init_neg_labels[j]);
// //   }
// //    }

//   gbit_initial_state = new_fact_info_pair( tpos, tneg );
//   //   cout << "BIT INIT" <<endl;
//   //   if ( gcmd_line.display_info == 103 ) {
//   //     printf("\nbit coded initial state reads:");
//   //     printf("\npositive");
//   //     print_FactInfo( gbit_initial_state->positive );
//   //     printf("\nnegative");
//   //     print_FactInfo( gbit_initial_state->negative );
//   //     printf("\nbit coded goal state reads:");
//   //     printf("\npositive");
//   //     print_FactInfo( gbit_goal_state->positive );
//   //     printf("\nnegative");
//   //     print_FactInfo( gbit_goal_state->negative );
//   //   }

// }


// minterm即能使得公式为true的虽有命题的product的。所有sum of minterm即DNF
DdNode** extractTermsFromMinterm(DdNode* minterm){

	//   cout << "getting terms for "<< endl;
	//   printBDD(minterm);
	//   cout << "num_alt_facts = " << num_alt_facts << " " << Cudd_SupportSize(manager, minterm) <<endl;
	DdNode** vars = new DdNode*[num_alt_facts];
	int num_uncertain = num_alt_facts - Cudd_SupportSize(manager, minterm);
	//     cout << "num_uncetain = " << num_uncertain << " " << pow(2, num_uncertain)<<endl;
	DdNode** result = new DdNode*[(2 << num_uncertain)+1];


	// 每增加一个uncertain, minterm个数*2
	for(int j = 0; j < (2 << num_uncertain)+1; j++)
		result[j] = NULL;


	for(int i = 0; i < num_alt_facts; i++){
		vars[i] = Cudd_bddIthVar(manager, i);
	}



	Cudd_PickAllTerms(manager, minterm, vars, num_alt_facts, result, (2 << num_uncertain) + 1);



	return result;

}


/**
 * momo007 2022.09.20
 * 用于 sglevel，首先获取每个terms，随后利用terms计算level，
 * 最后用所有terms的最小level作为最终结果
 */
DdNode** extractDNFfromBDD(DdNode* node) {
	//   cout << "Extracting DNF from " <<endl;
	//     if(node)
	//       printBDD(node);
	//     else
	//       cout << "BLAH"<<endl;
	int sizeofSupport=0;

	DdNode **result,**support;
	int *sup;

	if(bdd_is_one(manager, node)){
		result = new DdNode*[1];
		result[0] = node;
		return result;
	}

	sizeofSupport = Cudd_SupportSize(manager, node);
	support = new DdNode*[sizeofSupport];
	sup = Cudd_SupportIndex(manager,node);// 每个变量是否存在该公式
	int i=0,j=0;
	for (; i < Cudd_ReadSize(manager) && j<sizeofSupport; i++)
	{
		if (!sup[i]) continue;
		//		cout << "var = " << i << " " << j << endl;
		support[j++] = Cudd_ReadVars(manager,i);// 记录该变量
	}

	//  cout << "supports = " << sizeofSupport << endl;
	//   for(int i = 0; i < sizeofSupport; i++){
	//              printBDD(support[i]);
	//   }

	//  while(supports[sizeofSupport]){
	//            printBDD(supports[sizeofSupport]);
	//  sizeofSupports++;
	//  }

	// 计数DNF的term的个数
	int sizeofResult = Cudd_CountMinterm(manager, node, sizeofSupport);
	result = new DdNode*[sizeofResult+1];



	bdd_pick_all_terms( manager,
			node,
			// 		    Cudd_ReadVar(manager),
			// 		    dd_get_size(manager),
			support,
			sizeofSupport,
			result,
			sizeofResult);


	//  while(result[sizeofResult])
	//sizeofResult++;

	result[sizeofResult] = NULL;
	//   cout << "Got these DNFS" <<endl;
	// for(int l = 0 ; l < sizeofResult; l++){
	//   if( result[l])
	//    printBDD(result[l]);
	// }
	//  for(int i = 0; i< sizeofResult; i++){
	//     cout << "YO"<<endl;
	//     printBDD(result[i]);
	//     result[i] = bdd_and(manager, result[i], node);
	//     Cudd_Ref(result[i]);
	//     printBDD(result[i]);

	//   }

	return result;

}

// goal* extract_init_label(int i, clausalState* cs, int polarity) {
//   //DdNode* extract_init_label(int i, clausalState* cs, int polarity) {
//   goal_list::iterator j;
//   goal_list::iterator k;
//   goal_list* label_list = new goal_list();
//   simple_goal* sg;
//   neg_goal* ng;
//   disj_goal* dg;
//   disj_goal* dg1;
//   conj_goal* cg;
//   goal* return_goal;
//   //DdNode* return_node;


//   //  cout << "Extracting label for fact: " << polarity << alt_facts[i]->get_name() << " " << polarity <<endl;

//   if(cg = dynamic_cast<conj_goal*>(cs->getClauses())) {
//     j = cg->getGoals()->begin();
//     for(;j!=cg->getGoals()->end(); ++j) {
//       if((dg = dynamic_cast<disj_goal*>(*j))) {
// 	if(polarity) {
// 	  sg = new simple_goal(new proposition(alt_facts[i]), E_POS);
// 	  //  sg->display(0);
// 	  if(isSubset(sg,dg)) {
// 	    //cout << "NON TRUE LABEL";
// 	    dg1 = new disj_goal(dg->getGoals());
// 	     k = dg1->getGoals()->begin();
// 	    for(;k!= dg1->getGoals()->end(); ++k){
// 	      if(sg->operator==(*k)) {
// 		dg1->getGoals()->remove(*k);
// 		break;
// 	      }
// 	    }
// 	    if(dg1->getGoals()->size() > 1)
// 	      label_list->push_back(dg1);
// 	    else
// 	      label_list->push_back(dg1->getGoals()->front());
// 	  }
// 	}
// 	else {
// 	  ng = new neg_goal(new simple_goal(new proposition(alt_facts[i]), E_POS));
// 	  // ng->display(0);
// 	  if(isSubset(ng, dg)){
// 	    //   cout << "NON TRUE LABEL";

// 	    dg1 = new disj_goal(dg->getGoals());
// 	    k = dg1->getGoals()->begin();
// 	    for(;k!= dg1->getGoals()->end(); ++k){
// 	      if(ng->operator==(*k)) {
// 		dg1->getGoals()->remove(*k);
// 		break;
// 	      }
// 	    }



// 	  //   cout <<"DG = " <<endl;
// // 	    dg->display(0);
// // 	    cout <<"DG1 = " <<endl;
// // 	    dg1->display(0);
// 	    if(dg1->getGoals()->size() > 1)
// 	      label_list->push_back(dg1);
// 	    else
// 	      label_list->push_back(dg1->getGoals()->front());

// 	  }
// 	}
//       }
//     }
//   }
//   else if(dg = dynamic_cast<disj_goal*>(cs->getClauses())) {
//     if(polarity) {
//       sg = new simple_goal(new proposition(alt_facts[i]), E_POS);
//       if(isSubset(sg,dg)) {
// 	dg1 = new disj_goal(dg->getGoals());

// 	k = dg1->getGoals()->begin();
// 	    for(;k!= dg1->getGoals()->end(); ++k){
// 	      if(sg->operator==(*k)) {
// 		dg1->getGoals()->remove(*k);
// 		break;
// 	      }
// 	    }
// 	if(dg1->getGoals()->size() > 1)
// 	  label_list->push_back(dg1);
// 	else
// 	  label_list->push_back(dg1->getGoals()->front());
//       }
//     }
//     else {
//       ng = new neg_goal(new simple_goal(new proposition(alt_facts[i]), E_POS));
//       if(isSubset(ng, dg)){
// 	dg1 = new disj_goal(dg->getGoals());
// 	k = dg1->getGoals()->begin();
// 	for(;k!= dg1->getGoals()->end(); ++k){
// 	  if(ng->operator==(*k)) {
// 	    dg1->getGoals()->remove(*k);
// 	    break;
// 	  }
// 	}

// 	if(dg1->getGoals()->size() > 1)
// 	  label_list->push_back(dg1);
// 	else
// 	  label_list->push_back(dg1->getGoals()->front());
//       }
//     }
//   }

//   if(label_list->size() > 1){
//     // cout << "CONJ"<<endl;
//     return_goal = new neg_goal(new conj_goal(label_list));
//   }
//   else if(label_list->size() == 1) {
//     //cout << "SINGLE"<<endl;
//     return_goal = new neg_goal(label_list->front());
//   }
//   else
//     return_goal = NULL;

// //    cout << "RETURNING: " <<endl;
//    if(return_goal){
// //     return_goal->display(0);

//   return return_goal;
//   }
//   else {
//   //     cout << "NULL"<<endl;
//  return NULL;
//   }
// }

void build_entailer(){
	ofstream outfile;

	char* filename = "./smv/tmp/entail1.smv";

	cout << "FILENAME: " << filename<<endl;
	//  system(strcat("rm ", filename));
	outfile.open(filename,ios::trunc|ios::out);

	if(!outfile) {
		cerr << "Can't open smv.out for results!\n";
		return;
	}

	outfile << "MODULE main\nVAR\n";
	for(int i  = 0; i < num_alt_facts; i++){
		outfile << "x" << i << " : boolean;\n";
	}
	outfile << "st1 : process state1(";
	for(int i  = 0; i < num_alt_facts; i++){
		outfile << "x" << i;
		if(i < num_alt_facts - 1)
			outfile << ", ";
		else
			outfile <<");\n";
	}
	outfile << "st2 : process state2(";
	for(int i  = 0; i < num_alt_facts; i++){
		outfile << "x" << i;
		if(i < num_alt_facts - 1)
			outfile << ", ";
		else
			outfile <<");\n";
	}
	outfile << "LTLSPEC O ( (st1.val -> st2.val ) )";
	outfile.close();
}

void build_init_entailer(){
	ofstream outfile;

	char* filename = "./smv/tmp/entail.smv";

	cout << "FILENAME: " << filename<<endl;
	//  system(strcat("rm ", filename));
	outfile.open(filename,ios::trunc|ios::out);

	if(!outfile) {
		cerr << "Can't open smv.out for results!\n";
		return;
	}

	outfile << "MODULE main\nVAR\n";
	for(int i  = 0; i < num_alt_facts; i++){
		outfile << "x" << i << " : boolean;\n";
	}
	outfile << "ist : process initial";
	for(int i  = 0; i < num_alt_facts; i++){
		outfile << "x" << i;
		if(i < num_alt_facts - 1)
			outfile << ", ";
		else
			outfile <<");\n";
	}
	outfile << "cst : process state(";
	for(int i  = 0; i < num_alt_facts; i++){
		outfile << "x" << i;
		if(i < num_alt_facts - 1)
			outfile << ", ";
		else
			outfile <<");\n";
	}
	outfile << "LTLSPEC O ( (ist.val -> cst.val ) )";
	outfile.close();
}


// void build_model_helper(goal* cs, ofstream* outfile) {
//  simple_goal* sg;
//   conj_goal* cg;
//    conj_goal* cg1;
//  disj_goal* dg;
//   disj_goal* dg1;
//  neg_goal* ng;
//   goal_list::iterator i;
//   goal_list::iterator j;


//   if((cg = dynamic_cast<conj_goal*>(cs)) && cg->getGoals()->size() > 0) {
//     i = cg->getGoals()->begin();
//     *outfile <<"(";
//     for(;i !=cg->getGoals()->end(); ++i) {
//       if(cg1 = dynamic_cast<conj_goal*>(*i)) {
// 	build_model_helper(cg1, outfile);
//       }
//       else if(dg = dynamic_cast<disj_goal*>(*i)) {
// 	build_model_helper(dg, outfile);
// 	// 	outfile <<"(";

// 	// 	j = dg->getGoals()->begin();
// 	// 	for(;j != dg->getGoals()->end(); ++j) {
// 	// 	  if (ng = dynamic_cast<neg_goal*>(*j)) {
// 	// 	    if (sg = dynamic_cast<simple_goal*>(ng->getGoal())) {
// 	// 	      outfile << "!x" << sg->getProp()->getHashEntry();
// 	// 	    }
// 	// 	  }
// 	// 	  else if (sg = dynamic_cast<simple_goal*>(*j)) {
// 	// 	    outfile << "x" << sg->getProp()->getHashEntry();
// 	// 	  }
// 	// 	  if((*j) !=dg->getGoals()->back()) {
// 	// 	   outfile << " | ";
// 	// 	   //j--;
// 	// 	  }
// 	//   	}
// 	// 	outfile <<  ")";
//       }
//       else if (ng = dynamic_cast<neg_goal*>(*i)) {
// 	*outfile <<  "!";
// 	build_model_helper(ng->getGoal(), outfile);

// // 	if (sg = dynamic_cast<simple_goal*>(ng->getGoal())) {
// // 	  outfile << "!x" << sg->getProp()->getHashEntry();
// // 	}
//       }
//       else if (sg = dynamic_cast<simple_goal*>(*i)) {
// 	*outfile << "x" << sg->getProp()->getHashEntry();
//       }
//       else if (cg1 = dynamic_cast<conj_goal*>(*i)) {
//   	build_model_helper(cg1, outfile);
//       }
//       else
// 	*outfile << "TRUE";
//        if(*i != cg->getGoals()->back()) {
// 	 *outfile << " & ";
// 	 //--i;
//        }
//     }
//     *outfile << ")";
//   }
//   else if((dg = dynamic_cast<disj_goal*>(cs)) && dg->getGoals()->size() > 0) {
//     *outfile <<  "(";
//     j = dg->getGoals()->begin();
//     for(;j != dg->getGoals()->end(); ++j) {
//       if(cg = dynamic_cast<conj_goal*>(*j)) {
// 	build_model_helper(cg, outfile);
//       }
//       else if (ng = dynamic_cast<neg_goal*>(*j)) {
// 	*outfile <<  "!";
// 	build_model_helper(ng->getGoal(), outfile);
// // 	if (sg = dynamic_cast<simple_goal*>(ng->getGoal())) {
// // 	  outfile << "!x" << sg->getProp()->getHashEntry();
// // 	}
//       }
//       else if (sg = dynamic_cast<simple_goal*>(*j)) {
// 	*outfile << "x" << sg->getProp()->getHashEntry();
//       }
//       else if (dg1 = dynamic_cast<disj_goal*>(*j)) {
// 	build_model_helper(dg1, outfile);

//       }
//       if((*j) != dg->getGoals()->back()) {
// 	*outfile <<" | ";
// 	//j--;
//       }
//     }
//     *outfile << ")";
//   }
//   else if(ng = dynamic_cast<neg_goal*>(cs)) {
//     *outfile <<  "!";
//     build_model_helper(ng->getGoal(), outfile);
//     //     if (sg = dynamic_cast<simple_goal*>(ng->getGoal())) {
//     //       outfile << "!x" << sg->getProp()->getHashEntry();
//     //     }
//   }
//   else if (sg = dynamic_cast<simple_goal*>(cs)) {
//     *outfile << "x" << sg->getProp()->getHashEntry();
//   }
//   else
//     *outfile << "TRUE";

// }


// void build_model(goal* cs, char* name) {
//   ofstream outfile;


//   char* filename = new char[50];
//   strcat(filename, "./smv/tmp/");
//    strcat(filename, name);
//    strcat(filename, ".out");

//   //   cout << "BUilding model for " << endl;
// //     cs->display(0);


//    //   cout << "FILENAME: " << filename<<endl;

//   outfile.open(filename,ios::trunc|ios::out);
//   if(!outfile) {
//       cerr << "Can't open smv.out for results!\n";
//     return;
//   }

//     outfile << "MODULE "<< name << "(";
//     //cout << "NUM_FACTSA = " << num_alt_facts<<endl;
//   for(int i  = 0; i < num_alt_facts; i++){

//     outfile << "x" << i;

//     if(i < num_alt_facts - 1)
//       outfile << ", ";
//     else
//       outfile <<")\n";
//   }

//   outfile <<"VAR\n val: boolean;\n ASSIGN\ninit(val) := ";
//   ofstream* out = &outfile;

//   build_model_helper(cs, out);

//   outfile << "; " <<endl<<endl;
//   outfile.close();



// }


// BitVector* build_model(clausalState* cs) {
//   double model_size = pow(2.0,(double)num_alt_facts);
//   cout << "NUM FACTS  = " << num_alt_facts<< endl;

//   cout << "MODEL SIZE  = " << model_size<< endl;




//  BitVector* model = new_bit_vector(model_size);

//   for(int i = 0; i < model_size; i++) {
//     if(TRUE)
//       set_bit(initial_model, i);
//   }
//   //print_BitVector(model , model_size);
//   return model;
// }



void setKGraphTo(int k) {
	for(int i = 0; i < IPP_MAX_PLAN; i++){
		gpos_facts_vector_at[i] = k_graphs[k]->pos_facts_vector_at[i];
		gneg_facts_vector_at[i] = k_graphs[k]->neg_facts_vector_at[i];
		gop_vector_length_at[i] = k_graphs[k]->op_vector_length_at[i];
		pos_fact_at_min_level[i] = k_graphs[k]->pos_fact_at_min_level[i];
		neg_fact_at_min_level[i] = k_graphs[k]->neg_fact_at_min_level[i];
	}
	gall_ops_pointer = k_graphs[k]->gall_ops;
	gall_efs_pointer = k_graphs[k]->gall_efs;

	graph_levels = k_graphs[k]->num_levels;// 层级数目
	gall_fts_pointer = k_graphs[k]->all_fts_pointer;
	gft_mutex_pairs = k_graphs[k]->ft_mutex_pairs;// 互斥对
	gft_table = k_graphs[k]->graph;
	b_initial_state = k_graphs[k]->initial;// 初始状态
}

#ifdef PPDDL_PARSER
#else
int getlevelK(hash_entry* he, int polarity, int k){
	setKGraphTo(k);
	return getlevel(he, polarity);
}
#endif


// int getMaxOfKMinlevelNonMutex(clausalState* cs) {
//   int tmp;
//   clausalState* cs1;
//   int max = -1;

//   // cout << "Enter get max" <<endl;

//   for(int k = 0; k < num_graphs; k++){
//     setKGraphTo(k);
//     tmp = getlevelNonMutex(cs, cs1);
//     // cout << "Got " << tmp << " for graph " << k << endl;
//     if(tmp > max && tmp != IPP_MAX_PLAN)
//       max = tmp;
//   }
//   if(max == -1)
//     max = IPP_MAX_PLAN;

//   //cout << "Exit get max" << endl;
//   return max;
// }

// int fullySupported(EfEdge* adders /*supporing acts*/,
// 		       FtNode* prop /*prop to support*/,
// 		       int lev /*level*/,
// 		       RelaxedPlan* &return_plan /*ds for RP*/) {
//   //  cout << "Checking FUlly Supported for" << prop->index <<endl;
//   int ok = FALSE;
//   EfEdge* tmp_adders = adders;

//   //  cout << "Trying to compose label: " <<endl;
// //   if(prop->info_at[lev]->label == NULL)
// //     cout << "TRUE" <<endl;
// //   else
// //     prop->info_at[lev]->label->display(1);


//   //find subset of actions in tmp_adder whose labels combine to make the label of the prop
//   //actions are directly inserted into the RP in this function, instead of passing it back to findRP()

//   //first try to find a true label
//   //then if fail try to compose a set of actions that combine the label of the prop

//   //if trying to support a true label, look for a single support with a true label
//   if(prop->info_at[lev] && prop->info_at[lev]->label == NULL){
//     //   cout << "A"<<endl;
//     while(tmp_adders) {
//       //     cout << "Checking " << tmp_adders->ef->op->name  << "at level " << (lev) << endl;
//       if(tmp_adders->ef->info_at[lev-1] && tmp_adders->ef->info_at[lev-1]->label == NULL ){

// 	return_plan->action_levels[lev-1]->insert(tmp_adders->ef->op);
// 	return TRUE;
//       }
//       tmp_adders = tmp_adders->next;
//     }
//   }
//   //else tring to support a complex label, combinational problem, finding the smallest support set to compose the label, most of the time it will be all of the adders --- need to do a recursive procedure --- going to do it reverse style by first trying all the adders at once, then if that doesn't work, I'll start removing singles, pairs, triplets, etc.
//   //but first I'll try how the estimate is with putting in all of the adders
//   else if (prop->info_at[lev]){
//     //    cout << "B"<<endl;
//     // exit(0);
//      while(tmp_adders) {
//        //       cout << "Checking " << tmp_adders->ef->op->name  << "at level " << (lev) << endl;
//        if(tmp_adders->ef->info_at[lev-1] ){
// 	 //	 cout << "YO" <<endl;
// 	 if( tmp_adders->ef->info_at[lev-1]->label != NULL){

// 	   return_plan->action_levels[lev-1]->insert(tmp_adders->ef->op);
// 	   ok = TRUE;
// 	   //  exit(0);
// 	 }
//        }

//        tmp_adders = tmp_adders->next;
//      }
//      if(ok)
//        return TRUE;
//      else
//        return FALSE;
//   }
//   //  cout << "C" << endl;
//   return FALSE;








// }

//goal*
// DdNode* conjoinActionLabels(std::set<LabelledElement*>* return_ops, int lev) {
//   std::set<LabelledElement*>::iterator i = NULL;//EfNode
//   std::set<LabelledElement*>::iterator j = NULL;//FtNode
//   //  disj_goal* return_goal = new disj_goal(new goal_list());
//   //   cout << "Enter conjoinActionLabels" <<endl;
//   DdNode *f,  *var, *tmp;
//   Consequent *cons;
//   Label* lab;

//   f = Cudd_ReadLogicZero(manager);
//   Cudd_Ref(f);
//      if( return_ops) {
//     //        cout << "conjoin ops, size = "<<  return_ops->size() <<endl;
//     i = return_ops->begin();
//     for(;i!=return_ops->end();++i) {
//       //      if(!(*i)->op->is_noop)
//       //      	cout << (*i)->op->name << endl;
//       //      else
//       //	cout << (*i)->op->name << endl;

//       //      printBDD((*i)->info_at[lev]->label->label);
//       //   if(((EfNode*)(*i)->elt)->info_at[lev] && ((EfNode*)(*i)->elt)->info_at[lev]->label && !((EfNode*)(*i)->elt)->info_at[lev]->is_dummy){
// // 	cout <<"Label is " << endl;
// // 	printBDD((*i)->info_at[lev]->label->label);
// 	//	return_goal->getGoals()->push_back((*i)->info_at[lev]->label);
// 	//       tmp = Cudd_bddOr(manager, (*i)->info_at[lev]->label, f);
// 	//  cout << "ORING in = " <<endl;

// 	//    if(bdd_is_zero(manager,(*i)->info_at[lev]->label )){ cout << "BDD is FALSE"<<endl; }
// 	//    else if(bdd_is_one(manager,(*i)->info_at[lev]->label )){ cout << "BDD is TRUE" <<endl; }
// 	//    else{  cout << "BDD is Formula"<<endl; Cudd_PrintDebug(manager, (*i)->info_at[lev]->label, 100, 3); }
//    	//if(!bdd_is_one(manager, f)) {
// 	cons = ((EfNode*)(*i)->elt)->effect->cons;
// 	lab = ((EfNode*)(*i)->elt)->info_at[lev]->label;
// 	while(cons){
// 	  tmp = bdd_or(manager, bdd_and(manager, lab->label, (*i)->label), f);
// 	  Cudd_Ref(tmp);
// 	  Cudd_RecursiveDeref(manager, f);
// 	  f = tmp;
// 	  Cudd_Ref(f);

// 	  cons = cons->next;
// 	  lab = lab->next;
// 	}

// 	  //}
// 	  //else{
// 	  //f = (*i)->info_at[lev]->label->label;
// 	  //Cudd_Ref(f);
// 	  //}

// 	// }
//     }
//       }




//  //      cout << "Exit conjoinActionLabels" <<endl;

//  //    cout << "RETURNING = " << endl;
// //     printBDD(f);

//   return f;//return_goal;
// }



// static int my_time = 0;
// DdNode * efWorlds;
// struct effect_compare
// {
//   int operator()(LabelledElement* p, LabelledElement* q) const
//   {

//     double sum_p = getEffectWorldCost(((EfNode*)(p)->elt)->alt_index,efWorlds );
//     double sum_q =getEffectWorldCost(((EfNode*)(q)->elt)->alt_index,efWorlds );
// //     cout << "p = " << p->op->name << ", sum_p = " << sum_p<<endl;
// //     cout << "q = " << q->op->name << ", sum_q = " << sum_q<<endl;

//     if(sum_p > sum_q)
//       return -1;
//     else
//       return 1;

//     //return weighted sum of effect criteria







//   }
// };

// extern int num_alt_effs;
// extern  double sqr(double a);
// bool ef_less_than(LabelledElement* p, LabelledElement* q){


//     double cost_p, cost_q;
//     DdNode *tmpW;

//     if( RP_EFFECT_SELECTION == RP_E_S_RATIO){
//       tmpW = bdd_and(manager, efWorlds, b_initial_state);
//       cost_p = (double)getEffectWorldCost(my_time, ((EfNode*)(p)->elt)->alt_index,p->label )*
// 	sqr((double)Cudd_CountMinterm(manager, tmpW, num_alt_facts)/(double)Cudd_CountMinterm(manager, p->label, num_alt_facts));
//       cost_q = (double)getEffectWorldCost(my_time, ((EfNode*)(q)->elt)->alt_index,q->label )*
// 	sqr((double)Cudd_CountMinterm(manager, tmpW, num_alt_facts)/(double)Cudd_CountMinterm(manager, q->label, num_alt_facts));
//     }
//     else if( RP_EFFECT_SELECTION == RP_E_S_COST){
//       cost_p = (double)getEffectWorldCost(my_time, ((EfNode*)(p)->elt)->alt_index,p->label );
//       cost_q = (double)getEffectWorldCost(my_time, ((EfNode*)(q)->elt)->alt_index,q->label );
//     }
//     else if( RP_EFFECT_SELECTION == RP_E_S_COVERAGE){
//       tmpW = bdd_and(manager, efWorlds, b_initial_state);
//       cost_p = sqr((double)Cudd_CountMinterm(manager,tmpW, num_alt_facts)/(double)Cudd_CountMinterm(manager, p->label, num_alt_facts));
//       cost_q = sqr( (double)Cudd_CountMinterm(manager,tmpW, num_alt_facts)/(double)Cudd_CountMinterm(manager, q->label, num_alt_facts));
//     }
//  //      cout << "p = " << cost_p <<endl;
// //       cout << "q = " << cost_q <<endl;
// //       cout << "return = " << (cost_p <= cost_q) <<endl;

// //     if(((EfNode*)(p)->elt)->in_rp && !((EfNode*)(q)->elt)->in_rp)
// // 	return true;
// //       else if(((EfNode*)(q)->elt)->in_rp && !((EfNode*)(p)->elt)->in_rp)
// // 	return false;
// //     else{

//     if(((EfNode*)(p)->elt)->op->is_noop && !((EfNode*)(q)->elt)->op->is_noop)
//       return true;
//     else if(!((EfNode*)(p)->elt)->op->is_noop && ((EfNode*)(q)->elt)->op->is_noop)
//       return false;


//     if(cost_p < cost_q )
//       return true;
//  //    else if(cost_p == cost_q){
// //       if(((EfNode*)(q)->elt)->alt_index < num_alt_effs && !((EfNode*)(p)->elt)->alt_index >= num_alt_effs)
// // 	return true;
// //       else if(((EfNode*)(p)->elt)->alt_index < num_alt_effs && !((EfNode*)(q)->elt)->alt_index >= num_alt_effs)
// // 	return false;
// //       else
// // 	return true;
// //     }
//     else
//       return false;
//     //    }
// }

// DdNode* factInfoPairToBDD(FactInfoPair* fip){
//   DdNode //* returnDD = Cudd_ReadLogicZero(manager),
// 	*tmpDD=Cudd_ReadLogicZero(manager);


//   for(int i = 0; i < num_alt_facts; i++){
//     if(get_bit(fip->positive->vector , gft_vector_length, i)){
//       tmpDD = Cudd_bddOr(manager, tmpDD, Cudd_bddIthVar(manager, i));
//     }
//     if(get_bit(fip->negative->vector , gft_vector_length, i)){
//       tmpDD = Cudd_bddOr(manager, tmpDD, Cudd_bddIthVar(manager, i));
//     }
//   }
//   Cudd_Ref(tmpDD);
//   return tmpDD;
// }


// set<LabelledElement*>* getSupportForClause(FactInfoPair* clause,
// 					   FtNode* facts,
// 					   int lev,
// 					   DdNode* clgls,
// 					   std::set<LabelledElement*>* &return_efs,
// 					  set<LabelledElement*>* rp_acts ){
//   EfEdge* tmp_adder = 0;
//   // supp_ops = tmp_ops;
//   conj_goal* my_label = new conj_goal(new goal_list());
//   my_time = lev-1;
//   efWorlds = clgls;
//   std::list<LabelledElement*> tmp_efs;// = new std::list<LabelledElement*>();
//   //set<LabelledElement*>* tmp_efs = new std::set<LabelledElement*>();//EfNode

//   std::set<LabelledElement*> tmp_noops;// = new std::set<LabelledElement*>();//FtNode
//   //std::set<LabelledElement*> return_efs;// = new std::set<LabelledElement*>();//EfNode
//   //  std::set<FtNode*>*

//   std::set<LabelledElement*>* return_ops = new std::set<LabelledElement*>();//OpNode
//   LabelledElement* last;
//   std::list<LabelledElement*>::iterator i = NULL;//EfNode
//   std::list<LabelledElement*>::iterator j = NULL;//EfNode
//   std::set<LabelledElement*>::iterator k = NULL;//FtNode
//   Label* tmpLabel;
//   Consequent* tmpCons;
//   DdNode* tmpDD;
//   DdNode *clauseDD=factInfoPairToBDD(clause);
//   DdNode *fr;
//   list<LabelledElement*>* wlist;


//   int ok;
//   //      cout << "Enter getSupportForClause, time = " << lev <<endl;
//       //  printBDD(clgls);

//   //get all supporting actions for the clause
//   while(facts) {
//     if(((facts->positive &&  get_bit(clause->positive->vector , gft_vector_length, facts->index ) ) ||
// 	(!facts->positive && get_bit(clause->negative->vector , gft_vector_length, facts->index)))){
//       if (facts->info_at[lev]){
// 	tmp_adder = facts->adders;
// 	while(tmp_adder) {  //adding actions that support
// 	  if(tmp_adder->ef->info_at[lev-1] &&
// 	     !tmp_adder->ef->info_at[lev-1]->is_dummy){
// 	    // 	    if(tmp_adder->ef->op->is_noop)
// 	    // 	      cout << "Supported by noop"<<endl;
// 	    // 	    else
// 	    //	     	      cout << "Supported by = " << tmp_adder->ef->op->name <<endl;

// 	    tmpLabel = tmp_adder->ef->info_at[lev-1]->label;

// 	    tmpDD = Cudd_ReadLogicZero(manager);
// 	    tmpCons = tmp_adder->ef->effect->cons;
// 	    while(tmpLabel){
// 	      //	printBDD(tmpLabel->label);
// 	      if(Cudd_ReadOne(manager)!=tmpCons->b || Cudd_ReadOne(manager)!=clauseDD)		{

// 		tmpDD = Cudd_bddOr(manager, tmpLabel->label, fr=tmpDD);
// 		Cudd_Ref(tmpDD);
// 		Cudd_RecursiveDeref(manager,fr);
// 	      }
// 	      tmpLabel = tmpLabel->next;
// 	      tmpCons = tmpCons->next;
// 	    }
// 	    tmpDD = Cudd_bddAnd(manager, clgls, fr=tmpDD);
// 	    Cudd_Ref(tmpDD);
// 	    Cudd_RecursiveDeref(manager,fr);
// 	    //	    printBDD(tmpDD);
// 	    //don't add if a noop gives same worlds
// 	    //  ok = TRUE;
// 	    if(tmpDD != Cudd_ReadLogicZero(manager)){
// 	      // if(tmp_adder->ef->op->name)
// 	      //	      cout << "Really Supported by = " << tmp_adder->ef->op->name <<endl;
// 	      //	      printBDD(tmpDD);
// 	      if(!bdd_is_zero(manager, tmpDD)){
// 		if( RP_EFFECT_SELECTION == RP_E_S_COVERAGE
// 		   ){
// 		  tmp_efs.push_back(new LabelledElement((int*)tmp_adder->ef, tmpDD, 0));
// 		}
// 		else{
// 		  wlist = getCostsAt(lev-1, tmp_adder->ef->alt_index, 1);
// 		  //	    cout << "|wlist| = " << wlist->size()<<endl;
// 		  for(j = wlist->begin(); j != wlist->end(); j++){
// 		    //   printBDD((*j)->label);
// 		    fr = bdd_and(manager, (*j)->label, tmpDD);
// 		    ///printBDD(fr);
// 		    if(!bdd_is_one(manager, Cudd_Not(fr)))
// 		      tmp_efs.push_back(new LabelledElement((int*)tmp_adder->ef, fr, 0));

// 		  }
// 		}
// 	      }
// 	    }
// 	  }
// 	  tmp_adder=tmp_adder->next;
// 	}



//       }

//     }
//     facts=facts->next;
//   }

//   //reduce costs of effects whose actions are already in the relaxed plan
//   for(j = tmp_efs.begin(); j!=tmp_efs.end();++j) {
//     for(k = rp_acts->begin(); k!= rp_acts->end(); k++){
//       if(((OpNode*)(*k)->elt)->alt_index ==
// 	 ((EfNode*)(*j)->elt)->op->alt_index &&
// 	 ((OpNode*)(*k)->elt)->alt_index < num_alt_acts){
// 	(*j)->cost -= alt_act_costs[((EfNode*)(*j)->elt)->op->alt_index];
//       }
//     }
//   }




// //     i = tmp_efs.begin();
//   //      cout <<"ADDING ACTS"<<endl;

// //   for(;i!=tmp_efs.end();i++) {
// //     cout << "before sort = " << ((EfNode*)(*i)->elt)->op->name << " id = " << ((EfNode*)(*i)->elt)->op->alt_index << " ";
// //     if( RP_EFFECT_SELECTION == RP_E_S_RATIO){
// //       cout << " cost = " << (double)getEffectWorldCost(lev-1, ((EfNode*)(*i)->elt)->alt_index,(*i)->label )*sqr((double)Cudd_CountMinterm(manager,b_initial_state, num_alt_facts)/(double)Cudd_CountMinterm(manager, (*i)->label, num_alt_facts));
// //       cout << endl;
// //     }
// //     else if( RP_EFFECT_SELECTION == RP_E_S_COST){
// //         cout << " cost = " <<  (double)getEffectWorldCost(lev-1, ((EfNode*)(*i)->elt)->alt_index,(*i)->label );
// //       cout << endl;
// //     }
// //     else if( RP_EFFECT_SELECTION == RP_E_S_COVERAGE){
// //         cout << " cost = " <<   sqr( (double)Cudd_CountMinterm(manager,b_initial_state, num_alt_facts)/(double)Cudd_CountMinterm(manager, (*i)->label, num_alt_facts));
// //       cout << endl;
// //     }
// //    }


//   tmp_efs.sort(ef_less_than);



//  //  i = tmp_efs.begin();
// //   for(;i!=tmp_efs.end();i++) {
// //     cout << "after sort = " << ((EfNode*)(*i)->elt)->op->name;
// //     if( RP_EFFECT_SELECTION == RP_E_S_RATIO){
// //       cout << " cost = " << (double)getEffectWorldCost(lev-1, ((EfNode*)(*i)->elt)->alt_index,(*i)->label )*sqr((double)Cudd_CountMinterm(manager,b_initial_state, num_alt_facts)/(double)Cudd_CountMinterm(manager, (*i)->label, num_alt_facts));
// //       cout << endl;
// //     }
// //     else if( RP_EFFECT_SELECTION == RP_E_S_COST){
// //         cout << " cost = " <<  (double)getEffectWorldCost(lev-1, ((EfNode*)(*i)->elt)->alt_index,(*i)->label );
// //       cout << endl;
// //     }
// //     else if( RP_EFFECT_SELECTION == RP_E_S_COVERAGE){
// //         cout << " cost = " <<   sqr( (double)Cudd_CountMinterm(manager,b_initial_state, num_alt_facts)/(double)Cudd_CountMinterm(manager, (*i)->label, num_alt_facts));
// //       cout << endl;
// //     }
// //   }


//   for(i = tmp_efs.begin();i!=tmp_efs.end();i = tmp_efs.begin()//++i
//       ) {
//     //  cout << "Inserting = " << ((EfNode*)(*i)->elt)->op->name << "AT time = " << lev-1 << endl;
//     // printBDD((*i)->label);
//     return_efs->insert((*i));
//     tmp_efs.remove(*i);
//     last = new LabelledElement((int*)((EfNode*)(*i)->elt)->op, (*i)->label, 0);
//     return_ops->insert(last);

//     ((EfNode*)(*i)->elt)->in_rp = TRUE;
//     efWorlds = Cudd_bddAnd(manager, fr=efWorlds, Cudd_Not((*i)->label));
//     Cudd_Ref(efWorlds);
//     Cudd_RecursiveDeref(manager,fr);
//     tmpDD = (*i)->label;
//     ok = TRUE;
//     j = tmp_efs.begin();//i;
//     //j++;
//     for(; j!=tmp_efs.end();++j) {

//       (*j)->label = Cudd_bddAnd(manager, fr=(*j)->label, Cudd_Not(tmpDD));
//       Cudd_Ref((*j)->label);
//       Cudd_RecursiveDeref(manager,fr);
//       if(bdd_is_zero(manager, (*j)->label)){
// 	//	 cout << "removign"<<endl;
// 	tmp_efs.remove(*j);
// 	//delete *j;
// 	j--;
//       }
//       else if(((OpNode*)last->elt)->alt_index == ((EfNode*)(*j)->elt)->op->alt_index &&
// 	      ((OpNode*)last->elt)->alt_index < num_alt_acts){
//       //if we are covering a fact with effects, reduce cost of other
//       //effects of same action should we choose them
// 	(*j)->cost -= alt_act_costs[((EfNode*)(*j)->elt)->op->alt_index];
//       }
//     }
//     if(bdd_entailed(manager, efWorlds, conjoinActionLabels(return_efs, lev-1))) {
//       //       //      cout << "Got support at acts"<<endl;
//       //       k = return_efs->begin();
//       //       for(;k!=return_efs->end();++k){
//       // 	//		 	 cout << "With: " << ((EfNode*)(*k)->elt)->op->name << endl;
//       // 	return_ops->insert(new LabelledElement((int*)((EfNode*)(*k)->elt)->op, (*k)->label, 0));
//       //       }
//       //       //  if(cs)
//       //       //    cs->setHelpfulActs((set<EfNode*>*)tmp_efs); //helpful efs are all efs supporting goals
//       //       //            cout << "return"<<endl;
//       //       return return_ops;
//       //     }
//       //     //    else
//       //     //         cout << "Not enough Support"<< endl;
//       break;
//     }
//     tmp_efs.sort(ef_less_than);
//   }
//   //cout<< "exit"<<endl;
//   return return_ops;
// }


// // clausalState* getStateFromBitVector(list<LabelledElement*>* fips, FtNode* facts){

// //   goal_list* tmp_list = new goal_list();
// //   goal_list* tmp_list1 = NULL;
// //   clausalState* return_state = NULL;
// //   std::list<LabelledElement*>::iterator i = fips->begin();
// //   FtNode* tmp_ptr = facts;



// //   for(;i!=fips->end(); ++i) {
// //     //  print_BitVector(((FactInfoPair*)(*i)->elt)->positive->vector, gft_vector_length);
// //     tmp_ptr = facts;
// //     tmp_list1 = new goal_list();
// //     for(int j = 0; j < num_alt_facts; j++){
// //       //while(tmp_ptr) {
// //       //   cout << tmp_ptr->index <<endl;
// //       // printFact(tmp_ptr->index); cout << endl;
// //       if(//tmp_ptr->positive &&
// // 	 get_bit(((FactInfoPair*)(*i)->elt)->positive->vector , gft_vector_length, j//tmp_ptr->index
// // 		 )) {
// // 	//	        printFact(tmp_ptr->index);cout << " + " << endl;
// // 	tmp_list1->push_back(new simple_goal(new proposition(alt_facts[j]), E_POS));
// //       }
// //       else if(//!tmp_ptr->positive &&
// // 	      get_bit(((FactInfoPair*)(*i)->elt)->negative->vector , gft_vector_length, j//tmp_ptr->index
// // 		      )){
// // 	//  printFact(tmp_ptr->index);cout << " - " <<endl;
// // 	tmp_list1->push_back(new neg_goal(new simple_goal(new proposition(alt_facts[j]), E_POS)));
// //       }
// //       // tmp_ptr=tmp_ptr->next;
// //     }
// //     //}
// //    //   tmp_list1->unique(listCompare);


// //    if(tmp_list1->size() > 1)
// //      tmp_list->push_back(new disj_goal(tmp_list1));
// //    else if(tmp_list1->size()>0)
// //      tmp_list->push_back(tmp_list1->front());

// //    // tmp_list->display(1);

// //   }


// //   if(tmp_list->size() > 1)
// //     return_state = new clausalState(new conj_goal(tmp_list));
// //   else if(tmp_list->size() == 1)
// //     return_state = new clausalState(tmp_list->front());
// //   else if(tmp_list->size() < 1)
// //     return_state = new clausalState();//NULL;

// //   // cout << "ret="<<endl;
// //   //  return_state->display();
// //   return return_state;
// // }

// LabelledElement* setFIPFromNoops(set<LabelledElement*>* noops) {
//   FactInfoPair* returnFIP = new_fact_info_pair(new_FactInfo(), new_FactInfo());
//   DdNode* returnLabel = Cudd_ReadLogicZero(manager);
//   std::set<LabelledElement*>::iterator i = noops->begin();

//   for(;i!=noops->end(); ++i) {
// //     cout << ((FtNode*)(*i)->elt)->index <<endl;
// //    cout << "OK"<<endl;
//    if(((FtNode*)(*i)->elt)->positive)
//       make_entry_in_FactInfo(&(returnFIP->positive) , ((FtNode*)(*i)->elt)->index);
//     else
//       make_entry_in_FactInfo(&(returnFIP->negative) , ((FtNode*)(*i)->elt)->index);
//     returnLabel = bdd_or(manager, returnLabel, (*i)->label);
//   }

//   return new LabelledElement((int*)returnFIP, returnLabel,0);//returnFIP;
// }



// // int clauseCompare(FactInfoPair* c1, FactInfoPair* c2){
// //   clausalState* cs1, *cs2;
// //   std::list<LabelledElement*>* tmp_facts = new std::list<LabelledElement*>();
// //   FtNode* tmp_table = gall_fts_pointer;

// //   tmp_facts->push_back(new LabelledElement((int*)c1, Cudd_ReadOne(manager), 0));
// //   cs1 = getStateFromBitVector(tmp_facts, tmp_table);
// //   tmp_facts->empty();
// //   tmp_facts->push_back(new LabelledElement((int*)c2, Cudd_ReadOne(manager), 0));
// //   cs2 = getStateFromBitVector(tmp_facts, tmp_table);

// //   return compareGoals(cs1->getClauses(), cs2->getClauses());
// // }

void unionElementLabels(list<LabelledFormula*>* my_list){
	//for every element that is the same, union the labels of the element
	std::list<LabelledFormula*>::iterator i;
	std::list<LabelledFormula*>::iterator j,k;
	DdNode* fr;
	for(i = my_list->begin(); i != my_list->end(); i++){
		k = i;
		k++;
		// 和后面的Formula进行比较
		for(j = k ; j != my_list->end(); ){
			if(*i && *j && (*j)->elt == (*i)->elt){// 如果Fact BDD相同
				fr = Cudd_bddOr(manager, (*i)->label, (*j)->label);// 合并Label
				Cudd_Ref(fr);
				Cudd_RecursiveDeref(manager, (*i)->label);
				(*i)->label = fr;
				Cudd_Ref((*i)->label);
				Cudd_RecursiveDeref(manager, fr);

				my_list->remove(*j);
				if(*j){
					//	  cout << "del"<<endl;
					//delete *j;
					//	  cout << "ddel"<<endl;
				}
				i = my_list->begin();// 删除后，需要重置ite
				k = i;
				k++;
				j = k;
				//j--;
				break;
			}
			else
				j++;
		}
	}
}
void unionElementLabels(list<LabelledEffect*>* my_list){
	//for every element that is the same, union the labels of the element
	std::list<LabelledEffect*>::iterator i;
	std::list<LabelledEffect*>::iterator j,k;
	DdNode* fr;
	for(i = my_list->begin(); i != my_list->end(); i++){
		k = i;
		k++;
		for(j = k ; j != my_list->end(); ){
			if(*i && *j && (*j)->elt == (*i)->elt){
				(*i)->label = bdd_or(manager,fr= (*i)->label, (*j)->label);
				Cudd_RecursiveDeref(manager, fr);
				Cudd_Ref((*i)->label);
				my_list->remove(*j);
				delete *j;
				k = i;
				k++;
				j = k;
				//j--;
				//break;
			}
			else
				j++;
		}
	}
}
void unionElementLabels(set<LabelledAction*>* my_list){
	//for every element that is the same, union the labels of the element
	std::set<LabelledAction*>::iterator i, j, k;
	//  malloc(1);
	// cout << "unioning element labels, size = " << my_list->size() <<endl;
	DdNode* fr;

	for(i = my_list->begin(); i != my_list->end(); i++){
		k = i;
		k++;
		for(j = k; j != my_list->end(); ){
			//       cout << "Does " << ((*j)->elt)->alt_index  << " = " << ((*i)->elt)->alt_index << endl;
			if((*j)->elt == (*i)->elt){
				//	cout << "YES "<<endl<<flush;
				(*i)->label = bdd_or(manager,fr= (*i)->label, (*j)->label);
				Cudd_RecursiveDeref(manager, fr);
				Cudd_Ref((*i)->label);
				my_list->erase(*j);
				delete *j;
				k = i;
				k++;
				j = k;
				//j--;
				//break;
			}
			else{
				j++;
				//	cout << "NO "<<flush;
			}
		}
	}
	// cout << "done unioning element labels, size = " << my_list->size() <<endl;

}



double costOfClause(int time, DdNode *clause, DdNode* worlds){
	list<LabelledElement*> costs, *litcosts;
	double return_cost;

	for(int i = 0; i < num_alt_facts; i++){
		litcosts = NULL;
		if(Cudd_bddIsVarEssential(manager, clause, 2*i, 1)){
			litcosts = getCostsAt(time, i, 2);
		}
		else if(Cudd_bddIsVarEssential(manager, clause, 2*i, 0)){
			litcosts = getCostsAt(time, i+num_alt_facts, 2);
		}
		if(litcosts){
			for(list<LabelledElement*>::iterator c = litcosts->begin();
					c != litcosts->end(); c++){
				costs.push_back(*c);
			}
		}
	}
	return_cost = costOfCover(&costs, worlds);
	costs.clear();
	return return_cost;
}

double costOfGoal(int time, DdNode *worlds){
	double cost, c;
	if(COST_PROP_LIT==SUM)
		cost = 0;
	else if(COST_PROP_LIT==MAXN){
		cost = -1;
	}

	if((*my_problem).goal_cnf()){
		for(list<DdNode*>::iterator g = goal_cnf.begin();
				g != goal_cnf.end(); g++){
			c = costOfClause(time, *g, worlds);

			if(COST_PROP_LIT==SUM)
				cost += c;
			else if(COST_PROP_LIT==MAXN){
				if(c > cost)
					cost = c;
			}
		}
	}
	else {
		DdTlcInfo * ddclauses;
		DdHalfWord var1, var2;
		int phase1, phase2, match;
		ddclauses = Cudd_FindTwoLiteralClauses(manager, b_goal_state);
		int c = 0;
		while( Cudd_ReadIthClause(ddclauses, c, &var1, &var2, &phase1, &phase2)){
			DdNode* tmp_clause = Cudd_bddOr(manager,
					(// (var1 == CUDD_MAXINDEX || !(var1)) ?
							// 			       Cudd_ReadLogicZero(manager) :
							(!(phase1) ?
									Cudd_bddIthVar(manager, (int)var1) :
									Cudd_Not(Cudd_bddIthVar(manager, (int)var1)))),
									((var2 == CUDD_MAXINDEX || !(var2)) ?
											Cudd_ReadLogicZero(manager) :
											(!(phase2) ?
													Cudd_bddIthVar(manager, (int)var2) :
													Cudd_Not(Cudd_bddIthVar(manager, (int)var2)))));
			Cudd_Ref(tmp_clause);

			c = costOfClause(time, tmp_clause, worlds);

			if(COST_PROP_LIT==SUM)
				cost += c;
			else if(COST_PROP_LIT==MAXN){
				if(c > cost)
					cost = c;
			}
			Cudd_RecursiveDeref(manager, tmp_clause);
			c++;
		}
		Cudd_tlcInfoFree(ddclauses);
	}
	return cost;
}


double expectedSamplingUsage(DdNode *label, DdNode *belief){
	//compute the expectation that the belief will use samples in the label

	double usage = 0.0;

	//  for(list<DdNode*>::iterator s = samples.begin();
	//  s != samples.end(); s++){
	DdNode *as = Cudd_BddToAdd(manager, label);
	Cudd_Ref(as);

	DdNode *p = Cudd_addApply(manager, Cudd_addTimes, as, belief);
	Cudd_Ref(p);

	DdNode *exp = Cudd_addExistAbstract(manager, p, current_state_cube);
	Cudd_Ref(exp);

	DdNode *exp1 = Cudd_addExistAbstract(manager, exp, particle_cube);
	Cudd_Ref(exp1);



	int* cube = new int[2*num_alt_facts];
	// int cube [2*num_alt_facts];
	CUDD_VALUE_TYPE value;
	DdGen* gen = Cudd_FirstCube(manager, exp1, &cube, &value);


	usage = value;
	//  delete [] cube;
	Cudd_GenFree(gen);

	// printBDD(p);
	//printBDD(exp);
	//printBDD(exp1);
	// cout << usage << endl;

	Cudd_RecursiveDeref(manager, as);
	Cudd_RecursiveDeref(manager, p);
	Cudd_RecursiveDeref(manager, exp);
	Cudd_RecursiveDeref(manager, exp1);



	return usage;
}

double supportCost(int time,
		DdNode* worlds,
		LabelledEffect* eff,
		set<LabelledEffect*>* effects,
		LabelledFormula *clause){
	double cost = DBL_MAX;

	//cout << "cost of " << eff->elt->op->name << " " << eff->elt->op->alt_index<< " = ";

	if(COMPUTE_LABELS && RP_EFFECT_SELECTION != RP_E_S_COVERAGE){
		DdNode* supportWorlds = Cudd_bddAnd(manager, worlds, eff->label);
		Cudd_Ref(supportWorlds);

		if(supportWorlds != Cudd_ReadLogicZero(manager)){


			cost = getEffectWorldCost(time-1,
					((EfNode*)eff->elt)->alt_index,
					supportWorlds);



			//if same effect already chosen, then don't pay cost
			for(set<LabelledEffect*>::iterator e = effects->begin();
					e != effects->end(); e++){
				if(((EfNode*)eff->elt)->alt_index ==
						((EfNode*)(*e)->elt)->alt_index){
					cost = 0;//-= -1*((EfNode*)eff->elt)->effect->reward;//alt_act_costs[((EfNode*)eff->elt)->op->alt_index];
					break;
				}
			}

			if(RP_EFFECT_SELECTION == RP_E_S_RATIO && PF_LUG)
				cost *= (double)NUMBER_OF_MGS/(double)countParticles(eff->label);
		}

		Cudd_RecursiveDeref(manager, supportWorlds);
	}
	else if(RP_EFFECT_SELECTION == RP_E_S_COVERAGE || !COMPUTE_LABELS){

#ifdef PPDDL_PARSER
		if(!eff->elt->op->is_noop ){

			if(!COMPUTE_LABELS){
				cost = 1;
			}
			else{

				if(PF_LUG){
					// 	  if(eff->elt->effect->original->in_rp)
					// 	    cost = 0;
					// 	  else
					//	  cout << (double)NUMBER_OF_MGS << " " << (double)countParticles(eff->label) << endl;
					if(DO_INCREMENTAL_HEURISTIC){
						//cout << "" << endl;
						if(currentNode &&
								currentNode->rpIncrement &&

								currentNode->rpIncrement->actions[time-1]->find(eff->elt->op->alt_index) !=
										currentNode->rpIncrement->actions[time-1]->end())
							cost = 0.0;
						else
							cost =  1.0/(double)Cudd_CountMinterm(manager, eff->label, 2*num_alt_facts);
						//1.0/expectedSamplingUsage(eff->label, currentNode->backup_dd);
						//(double)NUMBER_OF_MGS/(double)countParticles(eff->label);
					}
					else{
					  // cost = (double)NUMBER_OF_MGS/(double)countParticles(eff->label);
					  cost =1;
					}
					//	  cout << "cost = " << cost << endl;
// 					for(set<LabelledEffect*>::iterator e = effects->begin();
// 							e != effects->end(); e++){
// 						if(((EfNode*)eff->elt)->alt_index ==
// 								((EfNode*)(*e)->elt)->alt_index){
// 							cost = 0;//-= -1*((EfNode*)eff->elt)->effect->reward;//alt_act_costs[((EfNode*)eff->elt)->op->alt_index];
// 							break;
// 						}
// 					}



				}
				else {
					if(my_problem->domain().requirements.probabilistic){

						//cost = 1; //need to change to probability of support
						DdNode *tmp = Cudd_BddToAdd(manager, eff->label);
						Cudd_Ref(tmp);
						DdNode *prob = Cudd_addApply(manager, Cudd_addTimes, tmp, probabilisticLabels);
						Cudd_Ref(prob);
						Cudd_RecursiveDeref(manager, tmp);
						cost = exAbstractAllLabels(prob, graph_levels-1); //1/probability is cost
						//          cout << "pr = " << cost <<endl;
						if(cost == 0)
							cost = DBL_MAX-1;
						else
							cost = 1.0/cost;
						Cudd_RecursiveDeref(manager, prob);
					}
					else if(my_problem->domain().requirements.non_deterministic){
						if(eff->elt->op->alt_index-1 < num_alt_acts){
							cost = Cudd_CountMinterm(manager, eff->label, num_alt_facts);
							for(set<LabelledEffect*>::iterator e = effects->begin();
									e != effects->end(); e++){
								if(((EfNode*)eff->elt)->alt_index ==
										((EfNode*)(*e)->elt)->alt_index){
									cost = 0;//-= -1*((EfNode*)eff->elt)->effect->reward;//alt_act_costs[((EfNode*)eff->elt)->op->alt_index];
									break;
								}
							}
						}
						else{
							cost = 0;
						}
					}
				}
			}
		}
		else{
			cost = -1;
		}

#else
		if(eff->elt->op->alt_index < num_alt_acts){
			cost = Cudd_CountMinterm(manager, eff->label, num_alt_facts);
		}
		else{
			cost = 0;
		}
#endif

	}
	else
		cost = 0;
	//    cout << "cost = "  << cost <<endl;
	return cost;
}
/**
 * momo007 2022.09.22
 */
bool supportClause(int time,
		   LabelledFormula* clause,
		   // list<LabelledEffect*>* supporters,
		   set<LabelledEffect*>* effects){
	DdNode* supportingWorlds = Cudd_ReadLogicZero(manager),
		*tmp, *tmp1, *tmpDD, *fr, *tmp_worlds, *support;
	LabelledEffect* minSupporter, *le;
	double minSupport, supportC;
	EfNode* ef;
	FtNode* tmp_fact;
	EfEdge* tmp_adder;
	// cout << "supporting clause"<<endl;
	//  list<LabelledEffect*> my_supporters;
	int ok = FALSE;
	list<LabelledEffect*>::iterator s, t;
	list<LabelledEffect*> supporters;
	list<LabelledElement*> *wlist;
	Cudd_Ref(supportingWorlds);
	Cudd_Ref(supportingWorlds);
	bool reward_var = false;
  
	// Cudd_RecursiveDeref(manager, support);
  
	// get supporters -- all effects that give a literal in a clause and worlds intersect

	/*
    cout << endl<< "Supporting at time: " <<time   <<endl;
        	printBDD(clause->elt);
        	cout << "in worlds: " <<endl;
        	printBDD(clause->label);*/
	support = Cudd_Support(manager, clause->elt);// 获取clause BDD
	Cudd_Ref(support);
	// printBDD(support);
	for(int i = 0; i < num_alt_facts; i++)// 考虑每个命题
	{
		// for(list<LabelledFormula*>::iterator s = clauses.begin(); //s != clauses.end(); s++){
		tmp_fact = NULL;
		// 获取fact在table的 FactNode
		if(Cudd_bddIsVarEssential(manager, support, 2*i, 1))
		{
			fr = Cudd_bddAnd(manager, clause->elt, Cudd_bddIthVar(manager, 2*i));
			Cudd_Ref(fr);
			// printBDD(fr);
			// cout << "i = " << i <<endl;
			tmp_fact = fr != Cudd_ReadLogicZero(manager)? gft_table[i]:gft_table[NEG_ADR(i)];
      		Cudd_RecursiveDeref(manager,fr);
		}

		if(tmp_fact){
			if(tmp_fact->index == num_alt_facts-2 && RBPF_LUG){
				// std::cout << "ft: " << tmp_fact->index << std::endl;
				reward_var = true;
			}
			// if(tmp_fact->index < 2*num_alt_facts-2)
			// printFact(tmp_fact->index); cout << tmp_fact->positive << " " << tmp_fact->index <<endl;
			tmp_adder = tmp_fact->adders;
			while(tmp_adder) {  //adding actions that support
				//	cout << tmp_adder->ef->op->name <<endl;
				if(tmp_adder->ef->info_at[time-1] && !tmp_adder->ef->info_at[time-1]->is_dummy && COMPUTE_LABELS)
				{
					// #ifdef PPDDL_PARSER
					// 	  fr=unionNDLabels(tmp_adder->ef->info_at[time-1]);
					// 	  tmp = Cudd_addBddStrictThreshold(manager, fr, 0.0);
					// 	  Cudd_Ref(tmp);
					// 	  Cudd_RecursiveDeref(manager,fr);
					// 	  fr = tmp;
					// 	  Cudd_Ref(fr);
					// 	  Cudd_RecursiveDeref(manager,tmp);
					// #else
					fr=tmp_adder->ef->info_at[time-1]->label->label;//unionNDLabels(tmp_adder->ef->info_at[time-1]);
	  				Cudd_Ref(fr);
					//#endif
					tmpDD = Cudd_bddIntersect(manager, fr, clause->label);//  Cudd_bddAnd(manager, (*j)->label, tmpDD);
	    			//Cudd_bddAnd(manager, fr, clause->label);
					//Cudd_bddAnd
	  				Cudd_Ref(tmpDD);
					// printBDD(fr);
					// printBDD(tmpDD);
					// Cudd_RecursiveDeref(manager,fr);
					// Cudd_RecursiveDeref(manager,tmp_worlds);
					if(tmpDD != Cudd_ReadLogicZero(manager) && tmpDD != Cudd_ReadZero(manager))
					{
						if(!DO_INCREMENTAL_HEURISTIC)
						{
							if(1)	fr = Cudd_bddAnd(manager, clause->label,
										tmp_adder->ef->info_at[time-1]->label->label);
							//tmpDD = Cudd_bddIntersect(manager, clause->label, fr);
	      					else	fr = Cudd_underApproxAnd(manager, clause->label,
							  			tmp_adder->ef->info_at[time-1]->label->label);
	    				}
						else
						{
							fr = tmp_adder->ef->info_at[time-1]->label->label;
						}
						// printBDD(tmp_adder->ef->op->info_at[time-1]->label->label);
						// printBDD(fr);//tmpDD);
						// Cudd_RecursiveDeref(manager, tmp_worlds);
						Cudd_Ref(fr);
	    				
						// if(tmp_adder->ef->op->is_noop)
						// {
						// 	cout << "NOOP: " << flush; //printFact(tmp_adder->ef->op->preconds->ft->index);	
						// }
						// else
						// 	cout << "OP: " << tmp_adder->ef->op->name <<endl;	
						// printBDD(fr);
						// printBDD(tmp_adder->ef->info_at[time-1]->label->label);
						if(RP_EFFECT_SELECTION == RP_E_S_COVERAGE )
						{
							supporters.push_back(new LabelledEffect(tmp_adder->ef, fr, 0));
							//cout << "A";
						}
						else
						{
							wlist = getCostsAt(time-1, tmp_adder->ef->alt_index, 1);
							for(list<LabelledElement*>::iterator j = wlist->begin(); 
									j != wlist->end(); j++){
								Cudd_RecursiveDeref(manager, tmpDD);
								tmpDD = Cudd_bddIntersect(manager, fr, (*j)->label);//Cudd_bddAnd(manager, (*j)->label, tmpDD);
								Cudd_Ref(tmpDD);
								
								if(tmpDD != Cudd_ReadLogicZero(manager))
								{//!bdd_is_one(manager, Cudd_Not(fr))){
									Cudd_RecursiveDeref(manager, tmpDD);
									if(1) tmpDD = Cudd_bddAnd(manager, fr, (*j)->label);
									else tmpDD = Cudd_underApproxAnd(manager, fr, (*j)->label);
									Cudd_Ref(tmpDD);
									supporters.push_back(new LabelledEffect(tmp_adder->ef, tmpDD, 0));
									//  cout << "B";
								}
								//Cudd_RecursiveDeref(manager, tmpDD);
							}// end-for
						}//end-else
					Cudd_RecursiveDeref(manager, fr);
					}//end-if
					else if(fr != Cudd_ReadLogicZero(manager) && fr != Cudd_ReadZero(manager))
					{
						// cout << "NO INTERSECTION WITH:\n";
						// printBDD(fr);
						// printBDD(clause->label);
						// printBDD(tmpDD);
					}
	  				Cudd_RecursiveDeref(manager, tmpDD);
				}//end-if
				// 存在action支持该fact，将该action的Effect加入到supports中
				else if(!COMPUTE_LABELS && tmp_adder->ef->info_at[time-1] && !tmp_adder->ef->info_at[time-1]->is_dummy)
				{
					//cout << "REALLY " << tmp_adder->ef->op->name <<endl;
					supporters.push_back(new LabelledEffect(tmp_adder->ef, Cudd_ReadOne(manager), 0));
					// cout << "C";
				}
				tmp_adder=tmp_adder->next;
			}// end action support loop
		}// end fact-if
	}// end fact loop
	// cout << endl;
	// Cudd_CheckKeys(manager);
    // cout <<"|";    fflush(stdout);
	Cudd_RecursiveDeref(manager, support);

	if(supporters.size() == 0 //&& !RBPF_LUG
	){
    	// std::cout << "No supporters" << std::endl;
    	return false;
	}
  
    // cout << "got |supporters| = " << supporters.size()<<endl;
  	// 该clause是析取，至少有一个fact满足即可
	while(supporters.size() > 0 && (supportingWorlds != clause->label || reward_var))
	{
		//find min supporter
		minSupport = DBL_MAX;
		minSupporter = NULL;
		// 获取min support Effect
		for(s = supporters.begin(); s != supporters.end(); s++)
		{
			supportC = supportCost(time, clause->label, *s, effects, clause);// MG is return 1

			// if((*s)->elt->op->is_noop)
			// {
			// 	cout << "cost of: " <<endl;
			// 	printFact((*s)->elt->op->preconds->ft->index);
			// 	cout << " = " << supportC << endl;
            // }
            // else
			// {
			// 	cout << "cost of: " << ((EfNode*)(*s)->elt)->op->name << " "
			// 						<< ((EfNode*)(*s)->elt)->effect->outcome
			// 						<< " = " << supportC << endl;
			// 	printBDD((*s)->label);
            // }
      		if(minSupport > supportC || !minSupporter)
			{
				  minSupporter = *s;
				  minSupport = supportC;
				if(minSupporter->elt->op->is_noop)
				{// && minSupport == 0){
					break;
				}
			}
		}// end-for
		// cout << "HI"<<endl;

    	//add supporter
    	tmp1 = Cudd_bddAnd(manager, minSupporter->label, clause->label);
    	Cudd_Ref(tmp1);

		// printBDD(tmp1);

    	if(1) tmp = Cudd_bddOr(manager, supportingWorlds, tmp1);
    	else tmp = Cudd_overApproxOr(manager, supportingWorlds, tmp1);
    
		Cudd_Ref(tmp);
		Cudd_RecursiveDeref(manager, supportingWorlds); //over-derefed
		supportingWorlds = tmp;
		Cudd_Ref(supportingWorlds);
		Cudd_RecursiveDeref(manager, tmp);

		ef = ((EfNode*)minSupporter->elt);
		ef->effect->original->in_rp++;// = TRUE;
		supporters.remove(minSupporter);
		// Cudd_Ref(minSupporter->label);
		delete minSupporter;
		minSupporter=NULL;
		// Cudd_Ref(tmp1);
		le = new LabelledEffect(ef, tmp1,0);
		// Cudd_RecursiveDeref(manager,tmp1);

		effects->insert(le);// 将该Effect加入到上一层
    
    	// if(ef->op->is_noop){
    	//   cout << "added: noop " << endl; //printFact(ef->op->preconds->ft->index);
    	// }
    	// else{
    	//   cout << "added: " << ef->op->name << " " << minSupport<< endl;
    	// }
    
    	// printBDD(supportingWorlds);
    
    	// printBDD(ef->op->b_pre);
        // printBDD(ef->effect->ant->b);
		//reduce labels of rest of supporters so no two support in same world
		list<LabelledEffect*> toDel;
    	if(!reward_var)
		{
    		for(s = supporters.begin(); s != supporters.end(); s++)
			{
				// 其他的support effect删除目前min support的label状态
				tmp = Cudd_bddAnd(manager, (*s)->label, Cudd_Not(le->label));
				Cudd_Ref(tmp);
				Cudd_RecursiveDeref(manager, (*s)->label);
				(*s)->label = tmp;
				Cudd_Ref((*s)->label);
				Cudd_RecursiveDeref(manager, tmp);
      			
				DdNode *inter = Cudd_bddIntersect(manager, (*s)->label, clause->label);
				Cudd_Ref(inter);
				// 没有其他状态可达，可以删除
				if(inter == Cudd_ReadLogicZero(manager))
					toDel.push_back(*s);
				Cudd_RecursiveDeref(manager, inter);
			}//end-for
		}//end-if
		//remove elts with empty labels
		for(s = toDel.begin(); s != toDel.end(); s++)
		{
			// cout << "HOOOv"<<endl;
			if(*s)
			{
				supporters.remove(*s);
				Cudd_Ref((*s)->label);
				delete *s;
			}
			//cout << "HOOO"<<endl;
		}
		//cout << "HOOOq"<<endl;
	}//end-while support
  
	// cout << "after (*^(*^"<<endl;
	// Cudd_CheckKeys(manager);  fflush(stdout);
	// cout << "after (*^(*^"<<endl;
	for(s = supporters.begin(); s != supporters.end(); s++)
	{
		if(*s)
		{
			Cudd_Ref((*s)->label);
			delete *s;
		}
	}
	supporters.clear();
	Cudd_RecursiveDeref(manager, supportingWorlds);
	// cout << "done supporting clause"<<endl;
    // cout << "before (*^(*^"<<endl;
	return true;
}


bool getSupport(int time,
		set<LabelledAction*>* actions,
		set<LabelledEffect*>* effects,
		list<LabelledFormula*>* subgoals){
	DdNode* subgoal, *worlds, *tmp_clause, *tmp_worlds, *tmpDD, *fr;
	Label* cons_label;
	//list<LabelledEffect*> supporters;
	//list<LabelledElement*> *wlist;
	list<LabelledFormula*> clauses;
	//  LabelledEffect* le_supporter;
	LabelledFormula *max_cost_clause;
	//EfNode* ef_supporter;
	DdTlcInfo * ddclauses;
	DdHalfWord var1, var2;
	int phase1, phase2, match;
	//
	//  EfEdge* tmp_adder;
	double max_clause_cost;

	//form cover elements as world-clause pairs
	//  cout << "|subgoals| = " << subgoals->size() << endl;
	//   cout << "["<<endl;  Cudd_CheckKeys(manager);cout <<"|" <<endl;
	// 创待待查找support operator的目标集合
	for(list<LabelledFormula*>::iterator s = subgoals->begin();
			s != subgoals->end(); s++){
		subgoal = ((DdNode*)(*s)->elt);// 目标fact的BDD
		Cudd_Ref(subgoal);
		worlds = (*s)->label;// 获取可达states
		Cudd_Ref(worlds);

		//      DdNode* approx = underApprox(manager, subgoal);
		//      Cudd_Ref(approx);
		//      Cudd_RecursiveDeref(manager, subgoal);
		//      subgoal = approx;
		//      Cudd_Ref(subgoal);
		//      Cudd_RecursiveDeref(manager, approx);

		//     cout << "need to support:" <<endl;
		//     printBDD(subgoal);

		if((*my_problem).goal_cnf()){
			clauses.push_back(new LabelledFormula(subgoal, worlds, 0));
			//Cudd_RecursiveDeref(manager, subgoal);
			//Cudd_RecursiveDeref(manager, worlds);
		}
		else{
			ddclauses = Cudd_FindTwoLiteralClauses(manager, subgoal);
			int c = 0;
			while( Cudd_ReadIthClause(ddclauses, c, &var1, &var2, &phase1, &phase2)){
				// if()
				tmp_clause = Cudd_bddOr(manager,
						(// (var1 == CUDD_MAXINDEX || !(var1)) ?
								// 			       Cudd_ReadLogicZero(manager) :
								(!(phase1) ?
										Cudd_bddIthVar(manager, (int)var1) :
										Cudd_Not(Cudd_bddIthVar(manager, (int)var1)))),
										((var2 == CUDD_MAXINDEX || !(var2)) ?
												Cudd_ReadLogicZero(manager) :
												(!(phase2) ?
														Cudd_bddIthVar(manager, (int)var2) :
														Cudd_Not(Cudd_bddIthVar(manager, (int)var2)))));
				Cudd_Ref(tmp_clause);
//				       printBDD(tmp_clause);
				clauses.push_back(new LabelledFormula(tmp_clause, worlds, 0));
				Cudd_RecursiveDeref(manager, tmp_clause);
				c++;
			}
			Cudd_tlcInfoFree(ddclauses);
			Cudd_RecursiveDeref(manager, subgoal);
			Cudd_RecursiveDeref(manager, worlds);
		}
	}
	//Cudd_CheckKeys(manager);  cout << "]"<<endl;

	 //cout << "done getting clauses"<<endl;

	//unionElementLabels(&clauses);
	//   cout << "done getting clauses"<<endl;

	//do cover
	//cout << "|clauses| = " << clauses.size() <<endl;

	/**
	 * 直到处理完成全部clause
	 */
	while(clauses.size() > 0){
		//   cout << "|clauses| = " << clauses.size() <<endl;
		//      cout << "|effects| = " << effects->size() <<endl;

		//find max cost clause to support first
		max_cost_clause = NULL;
		max_clause_cost = -1;
		// 优先处理max cost clause
		for(list<LabelledFormula*>::iterator c = clauses.begin();
				c != clauses.end(); c++){
			//      if((*c)->cost > max_clause_cost){
			max_cost_clause = *c;
			max_clause_cost = (*c)->cost;
			// }
			break;
		}

		//support clause
		clauses.remove(max_cost_clause);

		//      cout << "["<<flush;
		//    Cudd_CheckKeys(manager);cout <<"|";    fflush(stdout);
		// 根据effect层获取判断clause是否support
		if(!supportClause(time, max_cost_clause, effects)){
//cout << "can't support clause\n";
			return false;
}
		//   Cudd_CheckKeys(manager);    fflush(stdout);
		//     cout << "]"<<flush << endl;
		if(max_cost_clause)
			delete max_cost_clause;


	}


	//insert associated actions for effects
	for(set<LabelledEffect*>::iterator e = effects->begin();
			e != effects->end(); e++){
		//    if(!(*e)->elt->op->is_noop)
		//   cout << "adding op: " << (*e)->elt->op->name << endl;
		/// else
		//   cout << "adding noop"<<endl;
		match = FALSE;
		// 判断该effect是否有action匹配
		for(set<LabelledAction*>::iterator a = actions->begin();
				a != actions->end(); a++){
			if(((OpNode*)(*a)->elt)->alt_index == (*e)->elt->op->alt_index){
				if(COMPUTE_LABELS){//更新该action Node的 label
					fr = Cudd_bddOr(manager, (*a)->label, (*e)->label);
					Cudd_Ref(fr);
					Cudd_RecursiveDeref(manager, (*a)->label);
					(*a)->label = fr;
					Cudd_Ref((*a)->label);
					Cudd_RecursiveDeref(manager, fr);
				}
				match = TRUE;
				break;
			}
		}
		if(!match){// 没有合适的operator，创建noop action
			if(COMPUTE_LABELS)
				actions->insert(new LabelledAction((*e)->elt->op, (*e)->label, 0));
			else
				actions->insert(new LabelledAction((*e)->elt->op, Cudd_ReadOne(manager), 0));
		}

	}
	//cout << "DONE SUBs" << endl;
	return true;
}

void insertSubgoals(int time,
		set<LabelledAction*>* acts,
		set<LabelledEffect*>* efs,
		list<LabelledFormula*>* subgoals){
#ifdef PPDDL_PARSER

	//   int* permute = new int[2*num_alt_facts];
	//  for(int i = 0; i < 2*num_alt_facts; i++){
	//    if(i%2 == 0)
	//      permute[i] = i/2;
	//    else
	//      permute[i] = num_alt_facts+1;
	//  }
	DdNode* tmp1;
	//cout << "inserting subgoals for acts"<<endl;
	for(set<LabelledAction*>::iterator a = acts->begin();
			a != acts->end(); a++){
		tmp1 = (*a)->elt->b_pre;//Cudd_bddPermute(manager, (*a)->elt->b_pre, permute);
		Cudd_Ref(tmp1);
		//     printBDD((*a)->elt->b_pre);
		//     printBDD(tmp1);

		for(int i = 0 ; i < num_alt_facts; i++){
			if(Cudd_bddIsVarEssential(manager, tmp1, 2*i, 1)){
				subgoals->push_back(new LabelledFormula(Cudd_bddIthVar(manager, 2*i),
						(*a)->label,
						0));
			}
			else if(Cudd_bddIsVarEssential(manager, tmp1, 2*i, 0)){
				subgoals->push_back(new LabelledFormula(Cudd_Not(Cudd_bddIthVar(manager, 2*i)),
						(*a)->label,
						0));
			}
		}
		Cudd_RecursiveDeref(manager, tmp1);
	}
	//cout << "inserting subgoals for effs"<<endl;
	for(set<LabelledEffect*>::iterator e = efs->begin();
			e != efs->end(); e++){
		tmp1 = (*e)->elt->effect->ant->b;//Cudd_bddPermute(manager, (*e)->elt->effect->ant->b, permute);
		Cudd_Ref(tmp1);
		//printBDD(tmp1);
		for(int i = 0 ; i < num_alt_facts; i++){
			if(Cudd_bddIsVarEssential(manager, tmp1, 2*i, 1)){
				subgoals->push_back(new LabelledFormula(Cudd_bddIthVar(manager, 2*i),
						(*e)->label,
						0));
			}
			else if(Cudd_bddIsVarEssential(manager, tmp1, 2*i, 0)){
				subgoals->push_back(new LabelledFormula(Cudd_Not(Cudd_bddIthVar(manager, 2*i)),
						(*e)->label,
						0));
			}
		}
		//     subgoals->push_back(new LabelledFormula(tmp1,
		// 					    (*e)->label,
		// 					    0));
		Cudd_RecursiveDeref(manager, tmp1);
	}
	//    cout << "done inserting subgoals"<<endl;
	//delete [] permute;
#else

	//  cout << "inserting subgoals for acts"<<endl;
	for(set<LabelledAction*>::iterator a = acts->begin();
			a != acts->end(); a++){
		//    printBDD((*a)->elt->b_pre);
		//  Cudd_Ref((*a)->elt->b_pre);
		// Cudd_Ref((*a)->label);
		subgoals->push_back(new LabelledFormula((*a)->elt->b_pre,
				(*a)->label,
				0));
	}
	//   cout << "inserting subgoals for effs"<<endl;
	for(set<LabelledEffect*>::iterator e = efs->begin();
			e != efs->end(); e++){
		//    printBDD((*e)->elt->effect->ant->b);
		//  Cudd_Ref((*e)->elt->effect->ant->b);
		//  Cudd_Ref((*e)->label);
		subgoals->push_back(new LabelledFormula((*e)->elt->effect->ant->b,
				(*e)->label,
				0));
	}
	//    cout << "done inserting subgoals"<<endl;
#endif

	// cout << "union" << endl;
	unionElementLabels(subgoals);
	// cout << "done union" << endl;
}


typedef std::set<OpNode*> Opset;
RelaxedPlan*  getRelaxedPlan(int levels, DdNode* cs, int support, DdNode* worlds) {
	//	  Cudd_DisableGarbageCollection( manager );
	//cout << "[" <<endl;  Cudd_CheckKeys(manager);    fflush(stdout);    cout << "|" << endl;
  //	     cout << "\nFinding RP for: levs = " << levels << " of " << graph_levels << endl;
	     //       printBDD(cs);
	     //	     printBDD(worlds);

	RelaxedPlan* return_plan = new RelaxedPlan(levels, 0);
	// Cudd_Ref(cs);
	// Cudd_Ref(worlds);

#ifdef PPDDL_PARSER
	DdNode* tmp;
	DdNode* tmp1;
	// 忽略
	if(!PF_LUG && my_problem->domain().requirements.probabilistic){
		std::cout << "Error heuristic\n";
		assert(0);
		tmp = Cudd_addBddStrictThreshold(manager, worlds, 0.0);
		Cudd_Ref(tmp);
	}
	else{
		tmp = worlds;// initial set to one
		Cudd_Ref(tmp);
	}
	//    int* permute = new int[2*num_alt_facts];
	//    for(int i = 0; i < 2*num_alt_facts; i++){
	//      if(i%2 == 0)
	//        permute[i] = i/2;
	//      else
	//        permute[i] = num_alt_facts+1;
	//    }
	tmp1 = cs;//Cudd_bddPermute(manager, cs, permute);
	Cudd_Ref(tmp1);
	//    delete [] permute;

	//   printBDD(tmp);
	//   printBDD(worlds);
	// 在最后一层添加上目标的标记公式
	if((*my_problem).goal_cnf() && !(*my_problem).domain().requirements.rewards){
		// 考虑目标每个命题
		for(list<DdNode*>::iterator g = goal_cnf.begin();
				g != goal_cnf.end(); g++){
			Cudd_Ref(*g);
			//cout << "got goal cnf" << endl;
			return_plan->b_goal_levels[return_plan->plan_length-1]->push_back(new LabelledFormula(*g, tmp, 0));
		}
	}
	else{
		(COMPUTE_LABELS ?
				return_plan->b_goal_levels[return_plan->plan_length-1]->push_back(new LabelledFormula(tmp1, tmp, 0))
				: return_plan->b_goal_levels[return_plan->plan_length-1]->push_back(new LabelledFormula(tmp1, Cudd_ReadOne(manager), 0)) );
	}
	Cudd_RecursiveDeref(manager, tmp);
	Cudd_RecursiveDeref(manager, tmp1);

#else
	(COMPUTE_LABELS ?
			return_plan->b_goal_levels[return_plan->plan_length-1]->push_back(new LabelledFormula(cs, worlds, 0))
			: return_plan->b_goal_levels[return_plan->plan_length-1]->push_back(new LabelledFormula(cs, Cudd_ReadOne(manager), 0)) );
#endif


	// 从最后一层开始，抽取plan
	for(int j =  return_plan->plan_length; j>0; j--) {
		if(LUG_FOR == SPACE){// 空的？
			//     cout << "." << flush;
		}

		   //cout << "Starting backwards from " << j << endl;
		//find actions for level
		  //cout << "before get support"<<endl;
		// 计算上一层的support operator
		if(!getSupport(j, //time to support
				return_plan->action_levels[j-1], //ops returned
				return_plan->effect_levels[j-1], //efs returned
				return_plan->b_goal_levels[j-1] //formulas to support
		)){
//			cout << "Can't get support at " << j << endl;
			//return_plan->display();
			delete return_plan;
			return_plan = NULL;
			break;
		}

		//cout << "after get support"<<endl;
		//    Cudd_ReduceHeap(manager,
		//    		   CUDD_REORDER_SYMM_SIFT,
		//    		   num_alt_facts);
		// return_plan->display();
		//           Cudd_CheckKeys(manager);    fflush(stdout);
		//    Cudd_CheckKeys(manager);    fflush(stdout);
		//      cout << "]"<<flush << endl;
		// 还为到达第一层，更新子目标
		if(j>1){
			//insert preconditions of actions and effects
			insertSubgoals(j-1, //time to insert subgoals
					return_plan->action_levels[j-1], //chosen actions
					return_plan->effect_levels[j-1], //chosen effects
					return_plan->b_goal_levels[j-2]
			);
		}

		  //cout << "after subgoals"<<endl;
	}
	numRPs++;// relaxed plan heuristic调研次数
	//   Cudd_CheckKeys(manager);    fflush(stdout);

	//cout << "done with rp"<<endl;
	//    Cudd_ReduceHeap(manager,
	//   		   CUDD_REORDER_SYMM_SIFT_CONV,
	//   		   num_alt_facts); //return_plan->display();
	//   Cudd_CheckKeys(manager);  fflush(stdout);

	return return_plan;
}


// int existsSupportedMinterm(DdNode** minterms, std::set<EfNode*>* efs){
//   //for all minterms
//   // for all facts in minterm
//   //   for all efs
//   //     check if ef entails fact
//   //       yes -> go next fact
//   //       no -> go to next ef
//   //   if can't support fact go to next minterm
//   //if can't support a minterm return false
//   //  cout << "existsSupportedMinterm?" <<endl;

//   DdNode *b_sg, *b_ng, *g;
//   simple_goal* sg;
//   neg_goal* ng;
//   int k = 0;
//   int support_fact;
//   std::set<EfNode*>::iterator j;
//   while(minterms[k] != NULL){
//     //    printBDD(minterms[k]);
//     for(int i = 0; i < num_alt_facts; i++){

// //       sg = new simple_goal(new proposition(alt_facts[i]), E_POS);
// //       ng = new neg_goal(new simple_goal(new proposition(alt_facts[i]), E_POS));
// //       b_sg = goalToBDD(sg);
// //       b_ng = goalToBDD(ng);
//       b_sg = Cudd_bddIthVar(manager,i);
//       b_ng = Cudd_Not(Cudd_bddIthVar(manager,i));
// // //       Cudd_Ref(b_sg);
// // //       Cudd_Ref(b_ng);
//       g = NULL;

//       if(bdd_entailed(manager, minterms[k], b_sg)){
// 	g = b_sg;
// 	Cudd_Ref(g);
//       }
//       else if(bdd_entailed(manager, minterms[k], b_ng)){
// 	g = b_ng;
// 	Cudd_Ref(g);
//       }

//       if(g){
// 	//	cout << "Finding ef to support" <<endl;
// 	//	printFact(i);cout << endl;
// 	//	printBDD(g);
// 	support_fact = FALSE;
// 	j = efs->begin();
// 	for(; j!= efs->end(); ++j){
// 	  if(bdd_entailed(manager, (*j)->effect->cons->b, g)){
// 	    //	    cout << "Found support with " <<  (*j)->op->name <<  endl;
// 	    //printBDD(g);
// 	    g = NULL;
// 	    support_fact = TRUE;
// 	    break;
// 	  }
// 	}
// 	if(!support_fact)
// 	  break;
//       }
//       if(support_fact && (i == num_alt_facts-1))
// 	return TRUE;

//     }
//     k++;
//   }
//   return FALSE;

// }


// set<EfNode*>* supportMinterm(DdNode* b, std::set<EfNode*>* efs, int k){
//   //  cout << "enter support minterm" <<endl;
//   //determine if a subset of efs can cover all the facts in b
//   //if so, return the subset
//   //overlapping support is okay

//   std::set<EfNode*>* return_efs = new std::set<EfNode*>();
//   DdNode *b_sg, *b_ng, *g;
//   simple_goal* sg;
//   neg_goal* ng;

//   int support_fact;
//   std::set<EfNode*>::iterator j;
//   for(int i = 0; i < num_alt_facts; i++){
//     //    cout << "i = " << i << endl;
// //     sg = new simple_goal(new proposition(alt_facts[i]), E_POS);
// //     ng = new neg_goal(new simple_goal(new proposition(alt_facts[i]), E_POS));
// //     b_sg = goalToBDD(sg);
// //     b_ng = goalToBDD(ng);
//     b_sg = Cudd_bddIthVar(manager,i);
//     b_ng = Cudd_Not(Cudd_bddIthVar(manager,i));
// // //     Cudd_Ref(b_sg);
// // //     Cudd_Ref(b_ng);
//     g = NULL;

//     if(bdd_entailed(manager, b, b_sg)){
//       g = b_sg;
//       Cudd_Ref(g);
//     }
//     else if(bdd_entailed(manager, b, b_ng)){
//       g = b_ng;
//       Cudd_Ref(g);
//     }

//     if(g){
//       cout << "Finding ef to support" <<endl;
//       printFact(i);cout << endl;
//       //printBDD(g);
//       support_fact = FALSE;
//       j = efs->begin();
//       for(; j!= efs->end(); ++j){
// 	if(bdd_entailed(manager, (*j)->effect->cons->b, g)
// 	   //  &&	   (!support_fact || !bdd_is_one(manager, (*j)->info_at[k-1]->label->label))
// 	   ){
// 	    cout << "Found support with " <<  (*j)->op->name <<  endl;
// 	  //printBDD(g);
// 	  support_fact = TRUE;

// 	  return_efs->insert(*j);
// 	}
//       }
//       if(!support_fact){
// 	cout << "No support" <<endl;
//       	return NULL;
//       }
//     }
//   }
//   return return_efs;
// }

// DdNode* labelEfSet(set<DdNode*>* states, std::set<EfNode*>* efs, int k){//need to change to construct label of ef set based on the minterms
//   cout << "labelefSet" << endl;
//   DdNode* return_dd, * tmp, *tmp_world, *tmp_ftef;
//   return_dd = Cudd_ReadLogicZero(manager);

//   std::set<EfNode*>::iterator i;
//   int got_one = FALSE;
//   std::set<DdNode*>::iterator j = states->begin();
//     DdNode *b_sg, *b_ng, *g;
//   simple_goal* sg;
//   neg_goal* ng;


//   for(;j != states->end(); ++j){

//     tmp_world = Cudd_ReadOne(manager);


//     for(int l = 0; l < num_alt_facts; l++){
//       //    cout << "i = " << i << endl;
// //       sg = new simple_goal(new proposition(alt_facts[l]), E_POS);
// //       ng = new neg_goal(new simple_goal(new proposition(alt_facts[l]), E_POS));
// //       b_sg = goalToBDD(sg);
// //       b_ng = goalToBDD(ng);
//       b_sg = Cudd_bddIthVar(manager,l);
//       b_ng = Cudd_Not(Cudd_bddIthVar(manager,l));
// // //       Cudd_Ref(b_sg);
// // //       Cudd_Ref(b_ng);
//       g = NULL;

//       tmp_ftef = Cudd_ReadLogicZero(manager);
//       i = efs->begin();
//       got_one = FALSE;
//       for(;i != efs->end();++i){
//   	if((bdd_entailed(manager, (*j), b_sg) &&
// 	    bdd_entailed(manager, (*i)->effect->cons->b, b_sg)) ||
// 	   (bdd_entailed(manager, (*j), b_ng) &&
// 	    bdd_entailed(manager, (*i)->effect->cons->b, b_ng))){
// 	  cout << "Or-ing effects for a fact " << (*i)->op->name << endl;
// 	  got_one = TRUE;
// 	  tmp = bdd_or(manager, tmp_ftef, (*i)->info_at[k]->label->label);
// 	  Cudd_Ref(tmp);
// 	  Cudd_RecursiveDeref(manager, tmp_ftef);
// 	  tmp_ftef = tmp;
// 	  Cudd_Ref(tmp_ftef);
// 	  printBDD(tmp_ftef);
// 	}
//       }

//       if(got_one || !bdd_is_one(manager, Cudd_Not(tmp_ftef))){
// 	cout << "And-ing facts of a world " << endl;

// 	tmp = bdd_and(manager, tmp_world, tmp_ftef);
// 	Cudd_Ref(tmp);
// 	Cudd_RecursiveDeref(manager, tmp_world);
// 	tmp_world = tmp;
// 	Cudd_Ref(tmp_world);
// 	printBDD(tmp_world);
//       }
//     }
//     cout << "Or-ing worlds " << endl;
//     printBDD(tmp_world);
//     tmp = bdd_or(manager, return_dd, tmp_world);
//     Cudd_Ref(tmp);
//     Cudd_RecursiveDeref(manager, return_dd);
//     return_dd = tmp;
//     Cudd_Ref(return_dd);
//     printBDD(return_dd);

//   }
//   //   if(!got_one)
//     //     return_dd = Cudd_ReadLogicZero(manager);

//   return return_dd;
// }

// int fullySupported(set<EfNode*>* efs, DdNode* b, int k){
//   //  cout << "enter fully supported" <<endl;
//   //full support is when
//   // all initial states are mapped into b
//   //   if disjunction of effect labels for supported states are entailed by b
//   //     1 - check if state has all its facts supported
//   //     2 - add set of effects for state in 1 to ef set
//   //     3 - if efset label entails b -> done

//   std::set<DdNode*>* supportedStates = new std::set<DdNode*>();
//   std::set<EfNode*>* supportingEfs = new std::set<EfNode*>();
//   std::set<EfNode*>* tmpEfs = new std::set<EfNode*>();
//   DdNode* tmp_bdd, *tmp_bdd1;

//   DdNode** minterms = extractDNFfromBDD(b);
//   int i = 0;
//   tmp_bdd1 = labelBDD(b, k);
//   while(minterms[i] != NULL){
//     cout << "i = " << i << endl;
//     printBDD(minterms[i]);
//     tmpEfs = supportMinterm(minterms[i], efs, k);

//     if(tmpEfs){
//       set_union(supportingEfs->begin(),
// 		supportingEfs->end(),
// 		tmpEfs->begin(),
// 		tmpEfs->end(),
// 		inserter(*(supportingEfs),
// 			 supportingEfs->begin()));

//       supportedStates->insert(minterms[i]);
//     }

//     if(supportingEfs->size()>0){

//       tmp_bdd = labelEfSet(supportedStates, supportingEfs, k-1);
//       cout << "does " <<endl;
//       printBDD(tmp_bdd1);
//       cout << "entail" <<endl;
//       printBDD(tmp_bdd);


//       if(//bdd_is_one(manager, tmp_bdd) ||
// 	 bdd_entailed(manager, tmp_bdd1, tmp_bdd))
// 	return TRUE;
//   }

//     i++;
//   }

//   return FALSE;
// }



// std::set<EfNode*>* getSupportForBDD(DdNode* b, int k)// {
//   //support b at level k with efs from k-1
//   my_time = k-1;
//   //  cout << "Getting support for = " <<endl;
//   //  printBDD(b);
//   std::set<EfNode*,effect_compare>* all_supporting_efs = new std::set<EfNode*,effect_compare>();

//   //set<EfNode*>* all_supporting_efs = new std::set<EfNode*>();
//     std::set<EfNode*>* return_supporting_efs = new std::set<EfNode*>();
//   std::set<EfNode*>::iterator i;
//   DdNode** minterms = extractDNFfromBDD(b);
//   EfNode* tmp_ef = gall_efs_pointer;
//   DdNode* b_label = labelBDD(b, k);
//   DdNode* ef_label = Cudd_ReadLogicZero(manager);
//   DdNode *b_sg, *b_ng,* tmp;
//    simple_goal* sg;
//   neg_goal* ng;

//   if(bdd_is_one(manager, b_label))
//      return return_supporting_efs;

//   //find all supporting efs

//   //for all minterms
//   //  for all effect consequents that are entailed by minterm
//   //    add effect to all
//   int j = 0;
//   while(minterms[j] != NULL){
//     //    cout << "j = " << j << endl;
//     //    printBDD(minterms[j]);
//     tmp_ef = gall_efs_pointer;
//     while(tmp_ef){
//     for(int i = 0; i < num_alt_facts; i++){
//       sg = new simple_goal(new proposition(alt_facts[i]), E_POS);
//       ng = new neg_goal(new simple_goal(new proposition(alt_facts[i]), E_POS));
//       b_sg = goalToBDD(sg);
//       b_ng = goalToBDD(ng);
//       Cudd_Ref(b_sg);
//       Cudd_Ref(b_ng);
//       // cout << tmp_ef->op->name <<endl;
//       //printBDD( tmp_ef->effect->cons->b);
//       if(tmp_ef->info_at[k-1] && //printf("A") &&
// 	 !tmp_ef->info_at[k-1]->is_dummy && //printf("B") &&
// 	 tmp_ef->effect->cons->b && //printf("C") &&
// 	 (( bdd_entailed(manager, tmp_ef->effect->cons->b, b_sg) &&
// 	    bdd_entailed(manager,minterms[j], b_sg))
// 	  ||
// 	  (bdd_entailed(manager, tmp_ef->effect->cons->b, b_ng) &&
// 	   bdd_entailed(manager,minterms[j], b_ng)))){
// 	//cout << "inserted" <<endl;
// 	//	printBDD( tmp_ef->effect->cons->b);
// 	//	cout << "ef = " << tmp_ef->op->name << endl;
// 	all_supporting_efs->insert(tmp_ef);
// 	tmp_ef->in_rp = TRUE;
//       }
//     }
//     tmp_ef = tmp_ef->all_next;
//     }
//     j++;
//   }



//   //add efs to return supporting efs until label(b, k) entails the disjunctionof efs

//   i = all_supporting_efs->begin();
//   for(; i != all_supporting_efs->end(); ++i){
//     if(
//        fullySupported(return_supporting_efs, b, k)
//        //bdd_entailed(manager, b_label, ef_label) &&
//        //       existsSupportedMinterm(minterms, return_supporting_efs)
//        ){
//       cout << "Fully supported"<<endl;
//       return return_supporting_efs;
//     }
//     else{
//       cout << "Need more support" << endl;
//       cout << "Adding ef = " << (*i)->op->name << endl;
//       return_supporting_efs->insert(*i);
// //       tmp = bdd_or(manager, ef_label, (*i)->info_at[k-1]->label->label);
// //       Cudd_Ref(tmp);
// //       Cudd_RecursiveDeref(manager, ef_label);
// //       ef_label = tmp;
// //       Cudd_Ref(ef_label);
//     }

//   }

//   if(fullySupported(return_supporting_efs, b, k)){

//     return return_supporting_efs;
//   }
//   else{

//     return NULL;
//   }
// }


// RelaxedPlan*  getRelaxedPlan(int levels, DdNode* b, int support) {
// //  cout << "Getting RP with " << levels << " levels " <<endl;

//   RelaxedPlan* return_plan = new RelaxedPlan(levels, 0);
//   std::set<EfNode*>* supporting_efs = NULL;

//   for(int k = levels; k > 0; k--){
//     cout << "Supporting level k = " << k << endl;
//     supporting_efs = getSupportForBDD(b, k);

//     return_plan->insertEfs(supporting_efs, k-1); //separate acts and noops


//     if(k > 1){
//       b = return_plan->b_goal_levels[k-1];
//       cout << "Need to support = " <<endl;
//       printBDD(b);
//    }
//   }

//   return return_plan;
// }

int getRelaxedPlanHeuristic(){

	int tmp_plan_length;
	int return_val = 0;
	int sum_val = 0;
	int tmp_num_acts = 0;
	RelaxedPlan* merged_plan;
	RelaxedPlan* tmp_plan;
	//cout << "\nNum graphs = " << num_graphs<<endl;
	int set_merged = FALSE;

	std::set<LabelledAction*>::iterator end1;//OpNode
	std::set<LabelledAction*>::iterator end2;//OpNode
	int a;
	int match_found = FALSE;

	gettimeofday(&rp_tstart,0);


	if( HEURISTYPE==HRPUNION )// 每个动作仅取一次
		merged_plan = new RelaxedPlan(IPP_MAX_PLAN, 0);
	cout << "|graphs| = " << num_graphs << endl;
	for (int i = 0; i < num_graphs; i++)// 考虑每张图
	{
		if(num_graphs > 1 || HEURISTYPE==HRPUNION || HEURISTYPE==HRPMAX|| HEURISTYPE==HRPSUM)
			setKGraphTo(i);// 将第i张图数据放到全局变量随后处理
		// cout << "num levs = " << graph_levels <<endl;
		tmp_plan_length = graph_levels;//getLabelLevel(b_initial_state, NULL);

		if(tmp_plan_length == IPP_MAX_PLAN){// 无法到达目标
			//cout << "HO"<<endl;
			return IPP_MAX_PLAN;
		}
		else if(tmp_plan_length == 0)// 初始状态即目标，忽略
			continue;
		else {
			//tmp_plan = new RelaxedPlan(IPP_MAX_PLAN, 0);

			tmp_plan = getRelaxedPlan(tmp_plan_length, b_goal_state, FALSE, Cudd_ReadOne(manager));
			// cout << "got rp"<<endl;
		}
		// 多个graph，合并relaxed plan
		if(num_graphs > 1 || HEURISTYPE==HRPUNION || HEURISTYPE==HRPMAX || HEURISTYPE==HRPSUM) {
			if(tmp_plan) {// 存在relaxed plan，合并
				if(HEURISTYPE==HRPUNION){
					a = 0;
					// 考虑每一层relax plan
					for(int j = tmp_plan->plan_length - 1;j >=0; j--){
						set_merged = TRUE;
						//如果存在action levels
						if(tmp_plan && tmp_plan->action_levels[j]){
							//loop through tmp level and add to merged
							//level if not in merged level
							for(end1 = tmp_plan->action_levels[j]->begin();
									end1 != tmp_plan->action_levels[j]->end(); ++end1 ) {
								match_found = FALSE;// 该action是否添加过
								for(end2 = merged_plan->action_levels[a]->begin();
										end2 != merged_plan->action_levels[a]->end(); ++end2 ) {
									if(((OpNode*)(*end1)->elt)->alt_index ==
											((OpNode*)(*end2)->elt)->alt_index) {
										match_found = TRUE;
										break;
									}
								}
								// 之前没添加，增加
								if(!match_found)
									merged_plan->action_levels[a]->insert(*end1);
							}
						}
						else
							break;
						a++;// 此次合并的层数
					}
				}
			}
			// 释放内存
			if(k_graphs[i]->relaxed_plan) {
				delete k_graphs[i]->relaxed_plan;
				k_graphs[i]->relaxed_plan = NULL;
			}
			k_graphs[i]->relaxed_plan = tmp_plan;
		}// end muti-graph
		// 单个graph,直接赋值
		else{
			set_merged = TRUE;
			merged_plan = tmp_plan;
		}
	}

	if(HEURISTYPE==HRPUNION /*&& merged_plan && set_merged*/) {
		return_val = 0;
		return_val = merged_plan->getRelaxedConformantNumActions();
		std::cout << "getRelaxedConformantNumActions() =" << return_val << std::endl;
		//    cout << "num acts = " << return_val<<endl;
		delete merged_plan;
	}
	else if (HEURISTYPE==HRP /*&& merged_plan && set_merged*/) {
		//sum non-unioned levels, has repeats between levels
		for(int k = 0; k < merged_plan->plan_length; k++) {
			return_val += merged_plan->action_levels[k]->size();
		}
	}
	// Heuristic Relaxed planning max and sum
	else if(HEURISTYPE==HRPMAX ||HEURISTYPE==HRPSUM ){
		return_val = -1;
		int tmp = 0;
		for(int i = 0; i < num_graphs; i++) {
			if(!(k_graphs[i]->relaxed_plan)) {// 存在一个状态不可达，置为max
				return_val = IPP_MAX_PLAN;
				break;
			}
			if(RP_COUNT == RPEFFS)// 计算effect
				tmp = k_graphs[i]->relaxed_plan->getRelaxedNumEffects();
			else if(RP_COUNT == RPACTS)// 计算act
				tmp = k_graphs[i]->relaxed_plan->getRelaxedConformantNumActions();

			delete k_graphs[i]->relaxed_plan;

			// cout << "Num acts " << tmp << endl;
			if(tmp > return_val )// 更新最大值
				return_val = tmp;

			if(HEURISTYPE==HRPSUM)// 累加
				sum_val += tmp;

		}
		if(return_val == -1)
			return_val = IPP_MAX_PLAN;
		if(HEURISTYPE==HRPSUM)
			return_val = sum_val;

	}
	else
		return_val = IPP_MAX_PLAN;


	gettimeofday(&rp_tend,0);
	rp_total += (rp_tend.tv_sec - rp_tstart.tv_sec) * 1000 + (rp_tend.tv_usec-rp_tstart.tv_usec)/1000;

	//cout << "Exit RPS"<<endl;

	return return_val;

	//calculate interactions


}

#ifdef PPDDL_PARSER
int getlevel(int index, int polarity){
	// graph_levels在build_planning_graph中进行了赋值
	if(polarity)
	{
		for (int i = 0; i < graph_levels && gft_vector_length > 0;++i)
		{
			if(get_bit(gpos_facts_vector_at[i], gft_vector_length, index))
				return i;
		}
	}
	else
	{
		for (int i = 0; i < graph_levels && gft_vector_length > 0;++i)
		{
			if(get_bit(gneg_facts_vector_at[i], gft_vector_length,index))
				return i;
		}
	}
	return IPP_MAX_PLAN;
}
#else
int getlevel(hash_entry* he, int polarity) {
	//int ok = FALSE;
	int i = 0;
	// cout << "In Get level for: "  << he->getnum() <<endl;
	if(polarity) {
		while(i <= graph_levels && gft_vector_length > 0 ) {
			//      print_BitVector(gpos_facts_vector_at[i],gft_vector_length);cout <<endl;
			if(get_bit(gpos_facts_vector_at[i] ,gft_vector_length , he->getnum())) {
				//ok = TRUE;
				//		cout << "Found " << he->get_name()<<" at lev: " << i<<endl;

				return i;
			}
			i++;
		}
	}
	else{
		while(i <= graph_levels && gft_vector_length > 0 ) {
			//     print_BitVector(gneg_facts_vector_at[i],gft_vector_length); cout <<endl;
			if(get_bit(gneg_facts_vector_at[i] , gft_vector_length, he->getnum())) {
				//ok = TRUE;
				//	cout << "Found " << he->get_name()<<" at lev: " << i<<endl;
				return i;
			}
			i++;
		}
	}

	//  cout << "Exit Get level "  <<endl;

	//  if(!ok)
	return IPP_MAX_PLAN;
}
#endif

int num_states(DdNode* d){
	return num_states_in_minterm(d);
}

double num_states_in_minterm(DdNode* dd){
	//assuming dd is only in terms of current state vars and is a minterm
	double num = 0;

	//  DdNode *t = Cudd_bddAnd(manager, next_state_cube, dd);
	//  Cudd_Ref(t);
	//  num = Cudd_CountMinterm(manager, t, 2*num_alt_facts);
	//  Cudd_RecursiveDeref(manager, t);
	//  return num;

	for(int i = 0; i < num_alt_facts; i++){
		//cout << "i = " << i<<endl;
#ifdef PPDDL_PARSER
		if(!(Cudd_bddIsVarEssential(manager, dd, i*2, 0) ||
				Cudd_bddIsVarEssential(manager, dd, i*2, 1))){
#else
			if(!(Cudd_bddIsVarEssential(manager, dd, i, 0) ||
					Cudd_bddIsVarEssential(manager, dd, i, 1))){
#endif

				num++;
				// cout << "i = " << i << " num = " << num << endl;
			}
		}
		//Cudd_RecursiveDeref(manager, dd);
		num = pow(2, num);
		return num;
	}

	void get_states_from_minterm(DdNode* cube,
			list<DdNode*>* cube_states,
			int var){
		if(var == num_alt_facts){
			cube_states->push_back(cube);

			return;
		}

		DdNode* tdd, *fdd, *fr;

		if(!(Cudd_bddIsVarEssential(manager, cube, var*2, 0) ||
				Cudd_bddIsVarEssential(manager, cube, var*2, 1))){
			fr = Cudd_bddIthVar(manager, 2*var);
			Cudd_Ref(fr);
			tdd = Cudd_bddAnd(manager, cube, fr);
			Cudd_Ref(tdd);
			Cudd_RecursiveDeref(manager, fr);

			get_states_from_minterm(tdd, cube_states, var+1);
			fr = Cudd_Not(Cudd_bddIthVar(manager, 2*var));
			Cudd_Ref(fr);
			fdd = Cudd_bddAnd(manager, cube, fr);
			Cudd_Ref(fdd);
			Cudd_RecursiveDeref(manager, fr);
			get_states_from_minterm(fdd, cube_states, var+1);

			//clean up
			Cudd_RecursiveDeref(manager, cube);
		}
		else
			get_states_from_cube(cube, cube_states, var+1);

	}

	void get_states_from_cube(DdNode* cube,
			list<DdNode*>* cube_states,
			int var){
		if(var == num_alt_facts){
			cube_states->push_back(cube);
			return;
		}

		DdNode* tdd, *fdd, *fr;

		if(!(Cudd_bddIsVarEssential(manager, cube, var*2, 0) ||
				Cudd_bddIsVarEssential(manager, cube, var*2, 1))){
			fr = Cudd_bddIthVar(manager, 2*var);
			Cudd_Ref(fr);
			tdd = Cudd_bddAnd(manager, cube, fr);
			Cudd_RecursiveDeref(manager, fr);
			Cudd_Ref(tdd);
			get_states_from_cube(tdd, cube_states, var+1);
			fr = Cudd_Not(Cudd_bddIthVar(manager, 2*var));
			Cudd_Ref(fr);
			fdd = Cudd_bddAnd(manager, cube, fr);
			Cudd_RecursiveDeref(manager, fr);   Cudd_Ref(fdd);
			get_states_from_cube(fdd, cube_states, var+1);

			//clean up
			Cudd_RecursiveDeref(manager, cube);
		}
		else
			get_states_from_minterm(cube, cube_states, var+1);

	}

	void get_even_cubes_from_cube(DdNode* cube,
			list<DdNode*>* cube_states,
			DdNode *support,
			int var){
		if(var == num_alt_facts){
			cube_states->push_back(cube);

			return;
		}

		DdNode* tdd, *fdd, *fr;
		//printBDD(Cudd_FindEssential(manager, cube));
		//printBDD(Cudd_bddIthVar(manager, var*2));



		if(Cudd_bddIntersect(manager, cube,
				Cudd_bddIthVar(manager, var*2)) != Cudd_ReadLogicZero(manager) &&
				Cudd_bddIntersect(manager, cube,
						Cudd_Not(Cudd_bddIthVar(manager, var*2))) != Cudd_ReadLogicZero(manager) &&
						Cudd_bddIntersect(manager, support,
								Cudd_bddIthVar(manager, var*2)) != Cudd_ReadLogicZero(manager) &&
								Cudd_bddIntersect(manager, support,
										Cudd_Not(Cudd_bddIthVar(manager, var*2))) == Cudd_ReadLogicZero(manager)
		){
			//    cout << (2*var) << endl;
			//printBDD(cube);
			fr = Cudd_bddIthVar(manager, var*2);
			Cudd_Ref(fr);
			tdd = Cudd_bddAnd(manager, cube, fr);
			Cudd_Ref(tdd);
			Cudd_RecursiveDeref(manager, fr);
			//    printBDD(tdd);
			get_even_cubes_from_cube(tdd, cube_states, support, var+1);
			fr = Cudd_Not(Cudd_bddIthVar(manager, var*2));
			Cudd_Ref(fr);
			fdd = Cudd_bddAnd(manager, cube, fr);
			Cudd_Ref(fdd);
			Cudd_RecursiveDeref(manager, fr);
			get_even_cubes_from_cube(fdd, cube_states, support, var+1);

			//clean up
			//Cudd_RecursiveDeref(manager, cube);
			//    Cudd_RecursiveDeref(manager, fdd);
			//    Cudd_RecursiveDeref(manager, tdd);
		}
		else{
			//cout << "p" << (2*var) << endl;
			get_even_cubes_from_cube(cube, cube_states, support, var+1);
		}
	}

	void get_states_from_add(DdNode* dd,
			list<DdNode*>* states,
			list<double>* values){

		list<double> cubes_values;
		list<DdNode*> cubes;
		//  list<double> cube_values;
		list<DdNode*> cube_states;

		if(dd != Cudd_ReadZero(manager)){
			get_cubes(dd, &cubes, &cubes_values);
			list<DdNode*>::iterator c = cubes.begin();
			for(list<double>::iterator r = cubes_values.begin();
					r != cubes_values.end() && c!= cubes.end(); r++,c++){
				get_states_from_minterm(*c, &cube_states, 0);
				for(list<DdNode*>::iterator j = cube_states.begin();
						j != cube_states.end(); j++){
					values->push_back(*r);
				}

				states->merge(cube_states);

				//clean up
				//Cudd_RecursiveDeref(manager, *c);
			}
		}
	}

	void get_states_from_bdd(DdNode* dd,
			list<DdNode*>* states){
		list<DdNode*> cubes;
		list<DdNode*> cube_states;

		if(dd != Cudd_ReadZero(manager)){
			get_cubes(dd, &cubes);
			for(list<DdNode*>::iterator c = cubes.begin();
					c != cubes.end(); c++){
				get_states_from_cube(*c, &cube_states, 0);
				states->merge(cube_states);

				//clean up
				Cudd_RecursiveDeref(manager, *c);
			}
		}
	}

	void get_even_cubes_from_bdd(DdNode* dd,
			list<DdNode*>* states){
		list<DdNode*> cubes;
		list<DdNode*> cube_states;

		if(dd != Cudd_ReadZero(manager)){
			get_bdd_cubes(dd, &cubes);
			DdNode * support = Cudd_Support(manager, dd);
			Cudd_Ref(support);

			for(list<DdNode*>::iterator c = cubes.begin();
					c != cubes.end(); c++){


				get_even_cubes_from_cube(*c, &cube_states, support, 0);
				states->merge(cube_states);

				//clean up
				//      Cudd_RecursiveDeref(manager, *c);
			}
			Cudd_RecursiveDeref(manager, support);
		}
	}

	double get_sum(DdNode* dd){
		list<double> values;
		list<DdNode*> cubes;
		double rew = 0.0;
		if(dd != Cudd_ReadZero(manager)){
			get_cubes(dd, &cubes, &values);
			list<DdNode*>::iterator c = cubes.begin();
			for(list<double>::iterator r = values.begin();
					r != values.end() && c!= cubes.end(); r++,c++){
				rew += *r * num_states_in_minterm(*c);
				//printBDD(*c);
				//cout << "num_states = " << num_states_in_minterm(*c)<<endl;
				//      cout << "*" << flush;
				Cudd_RecursiveDeref(manager, *c);
			}
			//    cout <<endl;
		}



		return rew;
	}
	double get_min(DdNode* dd){
		list<double> values;
		list<DdNode*> cubes;
		double rew = DBL_MAX;
		if(dd != Cudd_ReadZero(manager)){
			get_cubes(dd, &cubes, &values);
			list<DdNode*>::iterator c = cubes.begin();
			for(list<double>::iterator r = values.begin();
					r != values.end() && c!= cubes.end(); r++,c++){
				if(*r < rew)
					rew = *r;
				//printBDD(*c);
				//  cout << "num_states = " << num_states_in_minterm(*c)<<endl;
				Cudd_RecursiveDeref(manager, *c);
			}
		}
		return rew;
	}
	void get_bdd_cubes(DdNode* dd, list<DdNode*>* cubes){
		//    cout << "|" <<endl;
		int* cube = new int[2*num_alt_facts];
		CUDD_VALUE_TYPE value;
		DdNode* tmp_cube;


		DdGen* gen = Cudd_FirstCube(manager, dd, &cube, &value);
		tmp_cube = Cudd_CubeArrayToBdd(manager, cube);
		Cudd_Ref(tmp_cube);
		//printBDD(tmp_cube);
		if(cubes)
			cubes->push_back(tmp_cube);

		while(Cudd_NextCube(gen, &cube, &value)){
			tmp_cube = Cudd_CubeArrayToBdd(manager, cube);
			Cudd_Ref(tmp_cube);
			if(cubes)
				cubes->push_back(tmp_cube);
		}
		Cudd_GenFree(gen);
	}

	void get_cubes(DdNode* dd, list<DdNode*>* cubes){
		//    cout << "|" <<endl;
		int* cube = new int[2*num_alt_facts];
		CUDD_VALUE_TYPE value;
		DdNode* tmp_cube;


		DdGen* gen = Cudd_FirstCube(manager, dd, &cube, &value);
		tmp_cube =  Cudd_addApply(manager,
				Cudd_addTimes,
				Cudd_addConst(manager, value),
				Cudd_BddToAdd(manager,
						Cudd_CubeArrayToBdd(manager,
								cube)));
		Cudd_Ref(tmp_cube);
		//printBDD(tmp_cube);
		if(cubes)
			cubes->push_back(tmp_cube);

		while(Cudd_NextCube(gen, &cube, &value)){
			tmp_cube =  Cudd_addApply(manager,
					Cudd_addTimes,
					Cudd_addConst(manager, value),
					Cudd_BddToAdd(manager,
							Cudd_CubeArrayToBdd(manager,
									cube)));
			//      printBDD(tmp_cube);
			Cudd_Ref(tmp_cube);
			if(cubes)
				cubes->push_back(tmp_cube);

		}
		Cudd_GenFree(gen);
		//delete [] cube;
	}
	void get_cubes(DdNode* dd, list<DdNode*>* cubes, list<double>* values){
		//    cout << "|" <<endl;
		int* cube = new int[2*num_alt_facts];
		CUDD_VALUE_TYPE value;
		DdNode* tmp_cube;

		//NEED TO EXPAND CUBE iNTO STATES
		DdGen* gen = Cudd_FirstCube(manager, dd, &cube, &value);
		tmp_cube =//  Cudd_addApply(manager,
				// 			       Cudd_addTimes,
				// 			       Cudd_addConst(manager, value),
				// 			       Cudd_BddToAdd(manager,
				Cudd_CubeArrayToBdd(manager,
						cube);//));
		Cudd_Ref(tmp_cube);
		//printBDD(tmp_cube);
		if(cubes)
			cubes->push_back(tmp_cube);
		if(values)
			values->push_back((double)value);
		while(Cudd_NextCube(gen, &cube, &value)){
			tmp_cube = // Cudd_addApply(manager,
					// 				 Cudd_addTimes,
					// 				 Cudd_addConst(manager, value),
					// 				 Cudd_BddToAdd(manager,
					Cudd_CubeArrayToBdd(manager,
							cube);//));
			//      printBDD(tmp_cube);
			Cudd_Ref(tmp_cube);
			if(cubes)
				cubes->push_back(tmp_cube);
			if(values)
				values->push_back((double)value);
		}
		Cudd_GenFree(gen);
	}
#ifdef PPDDL_PARSER

	void generate_BitOperatorEffects(const Action* op);
	void generate_BitOperatorEffectsFromDBN(const Action* op);

	// void generate_RewardNoopBitOperator( ){
	//   BitOperator *op = new_BitOperator("noop");
	//   t
	// }

	void generate_BitOperators(const Action *op ){
		cout << "gen bit op for " << endl;
		op->print(cout, my_problem->terms());
		cout << endl;
		cout << flush;
		DdNode *b_pre = action_preconds[op];
		Cudd_Ref(b_pre);
		list<DdNode*> p_cubes;
		list<double> p_values;
		BitOperator *tmp;
		//  Effect *tef;
		//  OutcomeSet* outcomes = action_outcomes[op];
		// get each state in belief state in p_cubes
		get_cubes(b_pre, &p_cubes, &p_values);
		int counter = 0;
		// consider each pre condtion
		for(list<DdNode*>::iterator i = p_cubes.begin();
				i != p_cubes.end(); i++){
			cout << counter++ << endl;
			ostringstream os(ostringstream::out);
			op->print(os, (*my_problem).terms());
			const char* name = (new string(os.str()))->c_str();
			tmp = new_BitOperator(name);
			//	tmp->nonDeter =
			tmp->alt_index = op->id();
			tmp->action = op;

			//set precondition;
			tmp->b_pre = *i;
			Cudd_Ref(tmp->b_pre);
			if(*i != Cudd_ReadOne(manager)){// the precondtion is not empty
				for(int k = 0; k < num_alt_facts; k++){
					if(Cudd_bddIsVarEssential(manager,*i , k*2, 1)){
						make_entry_in_FactInfo( &(tmp->p_preconds), k);
					}
					else if(Cudd_bddIsVarEssential(manager, *i, k*2, 0)){
						make_entry_in_FactInfo( &(tmp->n_preconds), k);
					}
				}
			}

			tmp->unconditional = NULL;
			tmp->conditionals = NULL;
			// insert in to the gbit_operator list
			tmp->next = gbit_operators;
			gbit_operators = tmp;
			gnum_bit_operators++;
			Cudd_RecursiveDeref(manager, *i);
			// momo007 2022.09.16
			// default selection is converage will not enter in
			// in conformant/contingent will generate effect in bulid_graph -> apply_operator
			if(RP_EFFECT_SELECTION != RP_E_S_COVERAGE ||
					HEURISTYPE==CORRRP){
				if(DBN_PROGRESSION){
					generate_BitOperatorEffectsFromDBN(op);
				}
				else{
					generate_BitOperatorEffects(op);
				}
			}
		}
		p_values.clear();
		p_cubes.clear();

	}



	void generate_BitOperatorEffectsFromDBN(const Action* op){
		//   std::cout << "gen bit op eff for " << std::endl;
		//op->print(std::cout, my_problem->terms());
		//std::cout << std::endl;
		//  list<DdNode*> c_cubes;
		//list<double> c_values;
		BitOperator *tmp;
		Effect *tef;

		//DdNode* ddop;

		//make sure we have the actions' effects set up

		//ddop = groundActionDD(*op);
		//    std::cout << "["<<std::endl;
		//    Cudd_CheckKeys(manager);
		//    std::cout << "|"<<std::endl;
		dbn* dbnop = action_dbn(*op);
		//    Cudd_CheckKeys(manager);
		//    std::cout << "]"<<std::endl;

		for(tmp = gbit_operators; tmp; tmp = tmp->next){
			//cout << tmp->name;
			if(tmp->action != op)
				continue;

			if(tmp->unconditional){
				//      std::cout << "already did" << std::endl;
				break;
			}



			//not using unconditional in here! so set to empty effect
			tmp->unconditional = new_Effect(gnum_cond_effects_pre);
			tmp->unconditional->op = tmp;
			//tmp->unconditional->index = uncond_eff->index;
			tmp->unconditional->cons->b = Cudd_ReadLogicZero(manager);//uncond_eff->b_eff;
			Cudd_Ref(tmp->unconditional->cons->b);
			tmp->unconditional->ant->b = Cudd_ReadLogicZero(manager);//uncond_eff->b_pre;
			Cudd_Ref(tmp->unconditional->ant->b);
			///    make_effect_entries( &(tmp->unconditional), tmp->unconditional->cons->b);
			tmp->unconditional->index = gnum_cond_effects_pre++;
			if(!LUGTOTEXT)
				num_alt_effs++;

			DdNode *i = tmp->b_pre;
			if(i != Cudd_ReadOne(manager)){
				for(int k = 0; k < num_alt_facts; k++){
					if(Cudd_bddIsVarEssential(manager,i , k*2, 1)){
						make_entry_in_FactInfo( &(tmp->unconditional->ant->p_conds), k);
					}
					else if(Cudd_bddIsVarEssential(manager, i, k*2, 0)){
						make_entry_in_FactInfo( &(tmp->unconditional->ant->n_conds), k);
					}
				}
			}

			//set conditional effects
			size_t n = dbnop->num_aux_vars;//outcomes->probabilities.size();
			tmp->num_outcomes = 1;//pow(2,n);
			//cout << "gen eff: " << endl;
			if(n > 0)
				NDACTIONS=TRUE;

			//    DdNode* aux_cube = dbnop->get_aux_var_cube();
			//    Cudd_Ref(aux_cube);
			//    printBDD(aux_cube);
			for (size_t k = 0; k < num_alt_facts-1; k++) {




				//       DdNode *cpt = dbnop->vars[2*k+1]->cpt;
				//       Cudd_Ref(cpt);
				//       list<DdNode*> p_cubes;
				//       list<double> p_values;



				if(dbn::isNoop(dbnop->vars[2*k+1])){
					//cout << "noop" << endl;
					continue;
				}

				//       DdNode *cptNoAux = Cudd_addExistAbstract(manager, cpt, aux_cube);
				//       Cudd_Ref(cptNoAux);

				//std::cout << "CPT for: " << (2*k+1) << std::endl;
				//printBDD(dbnop->vars[2*k+1]->cpt);

				std::list<DdNode*> *efs = (&dbnop->vars[2*k+1]->effects);

				//       printBDD(cptNoAux);




				//      get_cubes(cptNoAux, &p_cubes, &p_values);
				set<DdNode*> used_efs;
				
				for(list<DdNode*>::iterator i = efs->begin();
						i != efs->end(); i++){
				  DdNode *effbdd = Cudd_ReadOne(manager);
				  Cudd_Ref(effbdd);
					DdNode *be = Cudd_addBddThreshold(manager, *i, 1.0);
					Cudd_Ref(be);

					//if noop for value, continue
					if((Cudd_bddIsVarEssential(manager, be, 2*k+1, 1) &&
							Cudd_bddIsVarEssential(manager, be, 2*k, 1) ) ||
							(Cudd_bddIsVarEssential(manager, be, 2*k+1, 0) &&
							 Cudd_bddIsVarEssential(manager, be, 2*k, 0) )){
						continue;
					}

					//	cout << "cube" << endl;
					//	printBDD(be);


					//      for(dbn_cpt::iterator row = dbnop->vars[2*k+1]->cpt.begin();
					//  row != dbnop->vars[2*k+1]->cpt.end(); row++){
					tef = new_Effect(gnum_cond_effects_pre);				
					tef->op = tmp;
					  tef->node = dbnop->vars[2*k+1];

					tef->row = *i;
					Cudd_Ref(*i);

					//  if(tef->node == NULL || tef->row == NULL){
					//std::cout << "Null node " << k << std::endl;
					//exit(0);
					//	}


					//	if(my_problem->domain().requirements.rewards)
					//tef->reward = dbnop->get_matching_reward((dbn_row*)&(*row));
					//else
					//tef->reward = -1;
					tef->index = gnum_cond_effects_pre++;
					//	for(int j = 0; j  < num_alt_facts; j++){

					//for(std::map<double,double>::iterator p = row->second->begin();
					//  p != row->second->end(); p++){
					DdNode* t = effbdd;
					if(Cudd_bddIsVarEssential(manager, be, 2*k+1, 1)){
						//	    cout << "pos" <<endl;
						make_entry_in_FactInfo( &((tef)->cons->p_effects), k);
						t = Cudd_bddAnd(manager, effbdd, Cudd_bddIthVar(manager, 2*k+1));						
					}
					else if (Cudd_bddIsVarEssential(manager, be, 2*k+1, 0)){
						//cout << "neg" <<endl;
						make_entry_in_FactInfo( &((tef)->cons->n_effects), k);
						t = Cudd_bddAnd(manager, effbdd, Cudd_Not(Cudd_bddIthVar(manager, 2*k+1)));
					}
					//}
					Cudd_Ref(t);
					Cudd_RecursiveDeref(manager, effbdd);
					effbdd = t;


					DdNode* b_pre = Cudd_ReadOne(manager);
					Cudd_Ref(b_pre);


					//for(std::map<int,int>::iterator p = row->first->begin();
					//p != row->first->end(); p++){
					int num_pres =0;
					for(int j = 0; j  < num_alt_facts; j++){
						//cout << (*p).first << " " << 2*num_alt_facts << std::endl;
						DdNode* n_pre;

						if(Cudd_bddIsVarEssential(manager, be, 2*j, 1)){
							//	      cout << "ppos " << (2*j) <<endl;
							make_entry_in_FactInfo( &(tef->ant->p_conds), j);
							n_pre = Cudd_bddIthVar(manager,  2*j);
							
						}
						else if(Cudd_bddIsVarEssential(manager, be, 2*j, 0)){
							//cout << "pneg " << (2*j) <<endl;
							make_entry_in_FactInfo( &(tef->ant->n_conds), j);
							n_pre = Cudd_Not(Cudd_bddIthVar(manager,  2*j));
						}
						else{
							continue;
						}
						//cout << "HI" << endl;
						Cudd_Ref(n_pre);
						DdNode* t = Cudd_bddAnd(manager, b_pre, n_pre);
						Cudd_Ref(t);
						Cudd_RecursiveDeref(manager, b_pre);
						b_pre = t;
						Cudd_Ref(b_pre);
						Cudd_RecursiveDeref(manager, t);
						Cudd_RecursiveDeref(manager, n_pre);

						t = Cudd_bddAnd(manager, effbdd, n_pre);
						Cudd_Ref(t);
						Cudd_RecursiveDeref(manager, effbdd);
						effbdd = t;

						set<DdNode*>::iterator e = used_efs.find(effbdd);
						if(e == used_efs.end()){
						  used_efs.insert(effbdd);
						}
						else{
						  //cout << "skipped"<<endl;											  
						  delete tef;
						  tef = NULL;
						  break;
						}
						
						num_pres++;
						//db	
						if(num_pres>MAX_PRES-1)
						  break;
					}
					if(tef == NULL){
					  //cout << "skipped"<<endl;											  
					  continue;
					}
					tef->ant->b = b_pre;
					//printBDD(b_pre);

					//cout << "conds bv" << endl;
					//print_BitVector(tef->ant->p_conds->vector, gft_vector_length);
					//print_BitVector(tef->ant->n_conds->vector, gft_vector_length);

					tef->next = tmp->conditionals;
					tmp->conditionals = tef;
					//gnum_cond_effects_pre++;
					if(!LUGTOTEXT)
						num_alt_effs++;
				}
			}
		}

		//std::cout << "Done " << num_alt_effs <<std::endl;
	}

	void generate_BitOperatorEffects(const Action* op){
		// std::cout << "gen bit op eff for " << std::endl;
				// op->print(cout, my_problem->terms());
				// cout << endl;
		list<DdNode*> c_cubes;
		list<double> c_values;
		BitOperator *tmp;
		Effect *tef;

		DdNode* ddop;

		//make sure we have the actions' effects set up

		ddop = groundActionDD(*op);

		// 这里是readZero说明原先需要groundActionDD返回ADD
		/**
		 * momo007 2022.09.27
		 * following may contains a bug, always not enter, because ddop is BDD not ADD
		 */
		if (ddop == Cudd_ReadZero(manager) && !LUGTOTEXT)
		{
			// 后续需要apply的action有相同的，忽略，并设置invalid
			for(tmp = gbit_operators; tmp; tmp = tmp->next)
			{
				if(tmp->action == op)
				{
					tmp->valid = 0;
					return;
				}
			}
		}

		// find current action BitOperator
		for(tmp = gbit_operators; tmp; tmp = tmp->next){
			//cout << tmp->name;
			if(tmp->action != op)
				continue;
			// we have created, pass it
			if(tmp->unconditional){
				break;
			}

			//not using unconditional in here! so set to empty effect
			tmp->unconditional = new_Effect(gnum_cond_effects_pre);
			tmp->unconditional->op = tmp;
			//tmp->unconditional->index = uncond_eff->index;
			tmp->unconditional->cons->b = Cudd_ReadLogicZero(manager);//uncond_eff->b_eff;
			Cudd_Ref(tmp->unconditional->cons->b);
			tmp->unconditional->ant->b = Cudd_ReadLogicZero(manager);//uncond_eff->b_pre;
			Cudd_Ref(tmp->unconditional->ant->b);
			///    make_effect_entries( &(tmp->unconditional), tmp->unconditional->cons->b);
			gnum_cond_effects_pre++;// in new_EFfect had set the index
			if(!LUGTOTEXT)
				num_alt_effs++;

			// set the precondition(a) to the unconditional effect
			DdNode *i = tmp->b_pre;
			if(i != Cudd_ReadOne(manager)){
				for(int k = 0; k < num_alt_facts; k++){
					if(Cudd_bddIsVarEssential(manager,i , k*2, 1)){
						make_entry_in_FactInfo( &(tmp->unconditional->ant->p_conds), k);
					}
					else if(Cudd_bddIsVarEssential(manager, i, k*2, 0)){
						make_entry_in_FactInfo( &(tmp->unconditional->ant->n_conds), k);
					}
				}
			}
			// get the outcome set
			if(action_outcomes.find(op) == action_outcomes.end())
				return;
			OutcomeSet* outcomes = action_outcomes[op];


			//set conditional effects
			size_t n = outcomes->probabilities.size();
			tmp->num_outcomes = n;
			// cout << "gen eff: " << endl;
			if(n > 1)
				NDACTIONS=TRUE;

			for (size_t k = 0; k < n; k++) {
				      cout << "outcome " << k << ": " << outcomes->probabilities[k] << std::endl;
				//        if(Cudd_DebugCheck(manager)){
				// 	 cout << "DEBUG PROBLEM " << Cudd_ReadDead(manager)  <<endl;
				// 	 exit(0);
				//
				// transitionSet save the condtion <-> effect pair
				for (TransitionSetList::iterator ti =
						outcomes->transitions[k].begin();
						ti != outcomes->transitions[k].end(); ti++) {

					c_cubes.clear();
					c_values.clear();
					// get the condition state
					get_cubes((*ti)->condition_bdd(), &c_cubes, &c_values);
					// consider each condition state
					for(list<DdNode*>::iterator j = c_cubes.begin();
							j != c_cubes.end(); j++){

						DdNode *effbdd = Cudd_bddPermute(manager, (*ti)->effect_bdd(), varmap);
						Cudd_Ref(effbdd);
						int got_repeat = 0;
						//  cout << "mk ef"<<endl;
						//check to see if effect already exists
						for(tef = tmp->conditionals; tef; tef=tef->next){
							if(tef->ant->b == *j && tef->cons->b == effbdd){
								//add to outcomes of eff
								Cudd_RecursiveDeref(manager, effbdd);
								//set_bit(tef->outcome, k);
								tef->outcome->insert(k);// 该行代码可以省略？
								tef->probability_sum +=  outcomes->probabilities[k].double_value();
								(*tef->probability)[k] = outcomes->probabilities[k].double_value();
								//cout << "got repeat"<<endl;
								got_repeat = 1;
								break;
							}
						}
						// 之前已经处理过该effect
						if(got_repeat)
							continue;

						// 第一次，需要创建effect
						tef = new_Effect(gnum_cond_effects_pre);
						tef->op = tmp;
						if(my_problem->domain().requirements.rewards)
							tef->reward = (*ti)->reward().double_value();
						else
							tef->reward = -1;
						tef->index = gnum_cond_effects_pre++;
						((TransitionSet*)(*ti))->set_index(tef->index);

						//cout << tef->index << " " << (*ti)->index() << endl;
						// 设定第k个outcome的概率，累计概率
						(*tef->probability)[k] = outcomes->probabilities[k].double_value();
						tef->probability_sum +=  outcomes->probabilities[k].double_value();
						//	  tef->outcome = new_bit_vector(((int)n/gcword_size)+1);
						//set_bit(tef->outcome, k);
						tef->outcome->insert(k);
						tef->ant->b = *j;// 设置该effect的前提BDD
						Cudd_Ref(tef->ant->b);
						//printBDD(tef->ant->b);
						//printBDD((*ti)->effect_bdd());

						tef->cons->b = effbdd;// 设置该effect的结果BDD
						Cudd_Ref(tef->cons->b);
						Cudd_RecursiveDeref(manager, effbdd);
						// printBDD(tef->cons->b);
						//if(tef->cons->b != Cudd_ReadOne(manager)){
						// 设置effect结果FactInfo
						make_effect_entries( &(tef), tef->cons->b);
						// 设置effect前提FactInfo
						if(*j != Cudd_ReadOne(manager)){
							for(int p = 0; p < num_alt_facts; p++){
								if(Cudd_bddIsVarEssential(manager,*j, p*2, 1)){
									make_entry_in_FactInfo( &(tef->ant->p_conds), p);
								}
								else if(Cudd_bddIsVarEssential(manager, *j, p*2, 0)){
									make_entry_in_FactInfo( &(tef->ant->n_conds), p);
								}
							}
						}
						//}
						//   cout << "conds bv" << endl;
						//  	  print_BitVector(tef->ant->p_conds->vector, gft_vector_length);
						//  	  print_BitVector(tef->ant->n_conds->vector, gft_vector_length);
						// 	   	  cout << endl <<  "conds dd" << endl;
						// 	  printBDD(tef->ant->b);
						// insert into conditional effects
						tef->next = tmp->conditionals;
						tmp->conditionals = tef;
						//gnum_cond_effects_pre++;
						if(!LUGTOTEXT)
							num_alt_effs++;
						Cudd_RecursiveDeref(manager, *j);
					}
				}
			}
		}

		//  std::cout << "Done"<<std::endl;
	}


#else
	void generate_BitOperators(action_list_node *op )
	{
		//     cout << "gen bit op for " << op->act->get_name()<<endl;

		alt_effect* eff, *uncond_eff;
		eff = op->act->get_effs();
		uncond_eff = eff;
		DdNode** minterms = extractDNFfromBDD(uncond_eff->b_pre),** minterms1;

		int i, j;//, mm;
		BitOperator *tmp;//, *tmp2;
		//CodeNode *n, *j, *k, *l;
		Effect *tef;//, *ttt;

		i = 0;
		while(minterms[i]!=NULL){
			tmp = new_BitOperator( op->act->get_name() );
			tmp->nonDeter = op->act->isNonDeter();
			//   cout << "BitOp: " << tmp->name <<endl;
			//   cout << "pre:"<<endl;
			//  printBDD(minterms[i]);
			tmp->num_vars = op->act->get_num_vars();
			for (int k=0; k<MAX_VARS; k++ ) {
				tmp->inst_table[k] = op->act->inst_table[k];
			}
			tmp->alt_index = op->act->index;

			tmp->unconditional = new_Effect(gnum_cond_effects_pre);
			tmp->unconditional->op = tmp;
			tmp->unconditional->index = uncond_eff->index;
			tmp->unconditional->cons->b = uncond_eff->b_eff;
			Cudd_Ref(tmp->unconditional->cons->b);
			tmp->unconditional->ant->b = uncond_eff->b_pre;
			Cudd_Ref(tmp->unconditional->ant->b);
			tmp->b_pre = minterms[i];
			Cudd_Ref(tmp->b_pre);
			//  printBDD(tmp->b_pre);
			make_effect_entries( &(tmp->unconditional), uncond_eff);
			gnum_cond_effects_pre++;
			if(minterms[i] && minterms[i] != Cudd_ReadOne(manager)){
				for(int k = 0; k < num_alt_facts; k++){
					if(Cudd_bddIsVarEssential(manager,minterms[i] , k, 1)){
						make_entry_in_FactInfo( &(tmp->p_preconds), k);
						make_entry_in_FactInfo( &(tmp->unconditional->ant->p_conds), k);
					}
					else if(Cudd_bddIsVarEssential(manager,minterms[i], k, 0)){
						make_entry_in_FactInfo( &(tmp->n_preconds), k);
						make_entry_in_FactInfo( &(tmp->unconditional->ant->n_conds), k);
					}
				}
			}
			//       cout << "Uncond: " << endl <<"PRE: " << endl;
			//       print_BitVector(tmp->p_preconds->vector, gft_vector_length);
			//       print_BitVector(tmp->n_preconds->vector, gft_vector_length);
			//      printBDD(tmp->b_pre);
			//      cout << "EFF: " <<endl;
			//      printBDD(tmp->unconditional->cons->b);

			eff = op->act->get_effs();
			eff = eff->next;
			while (eff) {
				minterms1 = extractDNFfromBDD(eff->b_pre);
				j=0;
				while(minterms1[j]){
					//	cout << "cpre:"<<endl;
					//	printBDD(minterms1[j]);
					tef = new_Effect(gnum_cond_effects_pre);
					tef->op = tmp;
					tef->index = eff->index;
					tef->ant->b = eff->b_pre;
					Cudd_Ref(tef->ant->b);
					tef->cons->b = eff->b_eff;
					Cudd_Ref(tef->cons->b);
					make_effect_entries( &(tef), eff);
					if(minterms1[j] && minterms1[j] != Cudd_ReadOne(manager)){
						for(int i = 0; i < num_alt_facts; i++){
							if(Cudd_bddIsVarEssential(manager,minterms1[j], i, 1)){
								make_entry_in_FactInfo( &(tef->ant->p_conds), i);
							}
							else if(Cudd_bddIsVarEssential(manager,minterms1[j], i, 0)){
								make_entry_in_FactInfo( &(tef->ant->n_conds), i);
							}
						}
					}

					tef->next = tmp->conditionals;
					tmp->conditionals = tef;
					gnum_cond_effects_pre++;
					if(minterms1[j] == Cudd_ReadOne(manager)){
						Cudd_RecursiveDeref(manager, minterms1[j]);
						j++;
						break;
					}
					Cudd_RecursiveDeref(manager, minterms1[j]);
					j++;
					// 	cout << "Cond: " << endl <<"PRE: " << endl;
					// 	printBDD(tef->ant->b);
					// 	cout << "EFF: " <<endl;
					// 	printBDD(tef->cons->b);

				}
				delete [] minterms1;
				eff=eff->next;
			}

			tmp->next = gbit_operators;
			gbit_operators = tmp;
			gnum_bit_operators++;

			if(minterms[i] == Cudd_ReadOne(manager)){
				Cudd_RecursiveDeref(manager, minterms[i]);
				i++;
				break;
			}
			Cudd_RecursiveDeref(manager, minterms[i]);
			i++;
		}
		delete [] minterms;
	}
#endif;

	// void generate_Factored_BitOperators(action_list_node *op )

	// {
	//   simple_goal* sg;
	//   conj_goal* cg;
	//   disj_goal* dg;
	//   neg_goal* ng;


	//   alt_effect* uncond_eff;
	//   int i;
	//   BitOperator *tmp;


	//   //   cout << "gen bit op " << op->act->get_name()<<endl;

	//   // action_list factored_acts = op->act->getFactored();



	//   while(factored_acts) {

	//     tmp = new_BitOperator( factored_acts->act->get_name() );
	//     tmp->num_vars = factored_acts->act->get_num_vars();
	//     for ( i=0; i<tmp->num_vars; i++ ) {
	//       tmp->inst_table[i] = factored_acts->act->inst_table[i];
	//       // cout << "inst_table[" << i << "] = " <<tmp->inst_table[i]<<endl;
	//     }
	//     // tmp->p_preconds = 0;
	//     //tmp->n_preconds = 0;

	//     tmp->unconditional = new_effect();
	//     uncond_eff = factored_acts->act->get_effs();
	//     make_effect_entries( &(tmp->unconditional), uncond_eff);




	//   //assign preconds
	//     //  cout <<endl<< "UNcond pres" <<endl;
	//   if(!(dg = dynamic_cast<disj_goal*>(uncond_eff->pre))){
	//     if(!(cg = dynamic_cast<conj_goal*>(uncond_eff->pre))){
	//       if((ng = dynamic_cast<neg_goal*>(uncond_eff->pre))){
	// 	if((sg = dynamic_cast<simple_goal*>(ng->getGoal())))
	// 	     make_entry_in_FactInfo( &(tmp->n_preconds), sg->getProp()->ground->getnum());
	// 	//make_entry_in_FactInfo( &(tmp->unconditional->n_conds), sg->getProp()->ground->getnum());

	//       } else if ((sg = dynamic_cast<simple_goal*>(uncond_eff->pre))) {
	// 	make_entry_in_FactInfo( &(tmp->p_preconds),  sg->getProp()->ground->getnum() );
	// 	//make_entry_in_FactInfo( &(tmp->unconditional->p_conds),  sg->getProp()->ground->getnum() );
	//       }
	//     }
	//     else {
	//       goal_list::iterator k = cg->getGoals()->begin();
	//       for (; k != cg->getGoals()->end();++k ) {
	// 	if((ng = dynamic_cast<neg_goal*>(*k))){
	// 	  if((sg = dynamic_cast<simple_goal*>(ng->getGoal())))
	// 	    make_entry_in_FactInfo(&(tmp->n_preconds) , sg->getProp()->ground->getnum());
	// 	    //make_entry_in_FactInfo(&(tmp->unconditional->n_conds) , sg->getProp()->ground->getnum());

	// 	} else if((sg = dynamic_cast<simple_goal*>(*k))){
	// 	  make_entry_in_FactInfo( &(tmp->p_preconds),  sg->getProp()->ground->getnum() );
	// 	  //make_entry_in_FactInfo( &(tmp->unconditional->p_conds),  sg->getProp()->ground->getnum() );
	// 	}
	//       }
	//     }
	//   } else{
	//     cout << "Disj preconds not allowed in IPP graph";
	//     exit(0);
	//   }

	//    tmp->next = gbit_operators;
	//    gbit_operators = tmp;
	//    gnum_bit_operators++;

	//    //   print_BitOperator(gbit_operators);

	//    factored_acts = factored_acts->next;
	//   }


	// }

	void set_bit(BitVector* b, int index) {
		int uid_block;
		unsigned int uid_mask;
		if ( index < 0 ) {
			/* can happen in effects: facts that are deleted, but not contained
			 * in the inidtial state and never added.
			 */
			return;
		}

		uid_block = index / gcword_size;
		uid_mask = 1 << ( index % gcword_size );

		//cout <<setbase(2);
		// if(  !((*f)->vector[uid_block]))
		//   cout  << "UIDBLCOK " << uid_block << " "<< endl << "UIDMASK " << uid_mask <<endl;
		//cout << "Vector value = " <<  (*f)->vector[uid_block] << endl;
		b[uid_block] |= uid_mask;


	}
	void unset_bit(BitVector* b, int index) {
		int uid_block;
		unsigned int uid_mask;
		if ( index < 0 ) {
			/* can happen in effects: facts that are deleted, but not contained
			 * in the initial state and never added.
			 */
			return;
		}

		uid_block = index / gcword_size;
		for(int i = 0; i < gcword_size; i++){
			if(i != index)
				uid_mask |= 1 << i;
			//    else
			//      uid_mask |= 0 << i;
		}
		//uid_mask = 0xfffe << ( index % gcword_size );

		//cout <<setbase(2);
		// if(  !((*f)->vector[uid_block]))
		//   cout  << "UIDBLCOK " << uid_block << " "<< endl << "UIDMASK " << uid_mask <<endl;
		//cout << "Vector value = " <<  (*f)->vector[uid_block] << endl;
		b[uid_block] &= uid_mask;


	}
	void make_entry_in_FactInfo( FactInfo **f, int index )

	{
		//  cout << "Entering " << index <<endl;
		int uid_block;
		unsigned int uid_mask;
		Integers *i;



		if ( index < 0 ) {
			/* can happen in effects: facts that are deleted, but not contained
			 * in the initial state and never added.
			 */
			return;
		}

		uid_block = index / gcword_size;// 所属链表的块索引
		uid_mask = 1 << ( index % gcword_size ); // 所在块的index掩码
		i = new_integers( index );//创建integer，前插法
		//cout <<setbase(2);
		// if(  !((*f)->vector[uid_block]))
		//   cout  << "UIDBLCOK " << uid_block << " "<< endl << "UIDMASK " << uid_mask <<endl;
		if(!(*f)){// 未调用newFactInfo进行初始化
			cout<<"NULL FACTINFO\n";
			exit(0);
			//      (*f) = new_FactInfo();
		}
		//cout << "Vector value = " <<  (*f)->vector[uid_block] << endl;
		(*f)->vector[uid_block] |= uid_mask;// 标记该掩码为1
		// 前插法
		i->next = (*f)->indices;// 新的integer的next指向表头的integer
		(*f)->indices = i;// 更新表头的integer

	}


#ifdef PPDDL_PARSER
	void make_effect_entries( Effect **e, DdNode* eff)

	{

		//  cout << "Making effect entry" <<endl;
		int first_disj = TRUE;
		Consequent* tmp;
		int j;
		DdNode* support, *fr, *fr1, *fr2, *fr3;

		if(eff != Cudd_ReadOne(manager)) {
			(*e)->cons->b = eff;
			Cudd_Ref((*e)->cons->b);
			support = Cudd_Support(manager, (*e)->cons->b);
			Cudd_Ref(support);
			// 考虑结果的每个fact
			for(int i = 0; (i < num_alt_facts); i++){
				// 仅考虑和support一致的fact
				if(Cudd_bddIsVarEssential(manager, support, 2*i, 1)){
					fr1 = Cudd_bddIthVar(manager, 2*i);
					Cudd_Ref(fr1);
					fr = Cudd_bddAnd(manager,(*e)->cons->b, fr1);// 计算所有满足的合取
					Cudd_Ref(fr);
					Cudd_RecursiveDeref(manager, fr1);

					fr2 = Cudd_Not(Cudd_bddIthVar(manager, 2*i));
					Cudd_Ref(fr2);
					fr3 = Cudd_bddAnd(manager,(*e)->cons->b, fr2);// 计算所有满足的否定合取
					Cudd_Ref(fr3);
					Cudd_RecursiveDeref(manager, fr2);

					/** 
					 * momo007 通过计算该fact为true或false的可达状态(同label) 不为空，
					 * 设置该fact
					 */
					if(fr != Cudd_ReadLogicZero(manager)){
						make_entry_in_FactInfo( &((*e)->cons->p_effects), i);
					}
					else if(fr3 != Cudd_ReadLogicZero(manager)){
						make_entry_in_FactInfo( &((*e)->cons->n_effects), i);
					}
					Cudd_RecursiveDeref(manager, fr);
					Cudd_RecursiveDeref(manager, fr3);

				}
			}
			Cudd_RecursiveDeref(manager, support);



		}
		else{// uncodition effect, only set the consequence
			(*e)->cons->b = eff;
			Cudd_Ref((*e)->cons->b);
		}
		//     cout << "eff bv"<<endl;
		//     print_fact_info((*e)->cons->p_effects, gft_vector_length);
		//     print_fact_info((*e)->cons->n_effects, gft_vector_length);
		//     cout << "eff dd" <<endl;
		//     printBDD((*e)->cons->b);
	}
#else
	void make_effect_entries( Effect **e, alt_effect* eff)

	{

		// cout << "Making effect entry" <<endl;

		//token_list adds,dels = 0;
		//    if(!(*e)->p_effects)
		//       (*e)->p_effects = new FactInfo();
		//     if(!(*e)->n_effects)
		//       (*e)->n_effects = new FactInfo();

		//   for(adds = eff->add_list; adds>0; adds=adds->next) {
		//       make_entry_in_FactInfo( &((*e)->p_effects), adds->info->getnum() );

		//     }

		//     for(dels = eff->del_list; dels>0; dels=dels->next) {
		//       make_entry_in_FactInfo( &((*e)->n_effects), dels->info->getnum() );

		//     }
		//  cout<<"Make entries"<<endl;


		int first_disj = TRUE;
		Consequent* tmp;
		int j;
		DdNode* support, *fr;

		if(eff->b_eff && !bdd_is_one(manager,eff->b_eff)) {
			//cout << "about to do DNF from BDD\n";
			//cout.flush();
			DdNode** minterms = extractDNFfromBDD(eff->b_eff);
			j = 0;
			//printBDD(eff->b_eff);
			while(minterms[j] != NULL){
				//           cout << "j = " << j << endl;
				//	   printBDD(minterms[j]);
				if(j > 0 && !eff->obs){
					(*e)->is_nondeter=TRUE;
					tmp = (*e)->cons;
					(*e)->cons = new_Consequent();
					(*e)->cons->next = tmp;
				}
				(*e)->cons->b = minterms[j];
				Cudd_Ref((*e)->cons->b);
				support =  Cudd_Support(manager, minterms[j]);
				Cudd_Ref(support);
				for(int i = 0; (i < num_alt_facts); i++){
					if(Cudd_bddIsVarEssential(manager, support, i, 1)){
						if(Cudd_bddAnd(manager, minterms[j], Cudd_bddIthVar(manager, i)) != Cudd_ReadLogicZero(manager)){
							make_entry_in_FactInfo( &((*e)->cons->p_effects), i);
						}
						else{
							make_entry_in_FactInfo( &((*e)->cons->n_effects), i);
						}
					}
				}

				//  print_fact_info((*e)->cons->p_effects, gft_vector_length);
				//    print_fact_info((*e)->cons->n_effects, gft_vector_length);
				Cudd_RecursiveDeref(manager, support);
				//Cudd_RecursiveDeref(manager, minterms[j]);
				j++;
			}
			delete [] minterms;
		}
		else{
			//	cout << "Doing else clause\n";
			//	printBDD(eff->b_eff);
			(*e)->cons->b = eff->b_eff;
			Cudd_Ref((*e)->cons->b);
		}

		//   cout << "DONE" <<endl;
		//     if((sg = dynamic_cast<simple_goal*>(eff->eff->getClauses()))) {
		//       make_entry_in_FactInfo( &((*e)->cons->p_effects), sg->getProp()->getHashEntry() );
		//   //                 cout << "ADDING to EFF" <<endl;
		// //        sg->display(0);
		//     }
		//     else if(ng = dynamic_cast<neg_goal*>(eff->eff->getClauses())){
		//       if(sg = dynamic_cast<simple_goal*>(ng->getGoal())) {

		// 	make_entry_in_FactInfo( &((*e)->cons->n_effects), sg->getProp()->getHashEntry() );
		//  //  	cout << "ADDING to EFF" <<endl;
		// //   	 ng->display(0);
		//       }
		//     }
		//     else if(cg = dynamic_cast<conj_goal*>(eff->eff->getClauses())){
		//       goal_list::iterator i = cg->getGoals()->begin();
		//       for(;i!=cg->getGoals()->end();++i) {
		// 	if(sg = dynamic_cast<simple_goal*>(*i)){
		// 	  make_entry_in_FactInfo( &((*e)->cons->p_effects), sg->getProp()->getHashEntry() );
		//  // 	  cout << "ADDING to EFF" <<endl;
		// //   	  sg->display(0);
		// 	}
		// 	else if(ng = dynamic_cast<neg_goal*>(*i)){
		// 	  if(sg = dynamic_cast<simple_goal*>(ng->getGoal())){
		// 	    make_entry_in_FactInfo( &((*e)->cons->n_effects), sg->getProp()->getHashEntry() );
		//  	//     cout << "ADDING to EFF" <<endl;
		// //   	    ng->display(0);
		// 	  }
		// 	}
		// 	else if(dg = dynamic_cast<disj_goal*>(*i)){
		// 	  //	  	  cout << "GOT DG IN CG EFF" <<endl;
		// 	  goal_list::iterator j = dg->getGoals()->begin();
		// 	  for(;j!=dg->getGoals()->end();++j) {
		// 	    if(sg = dynamic_cast<simple_goal*>(*j)){
		// 	      make_entry_in_FactInfo( &((*e)->cons->p_effects), sg->getProp()->getHashEntry() );
		// 	      // 	  cout << "ADDING to EFF" <<endl;
		// 	      //   	  sg->display(0);
		// 	    }
		// 	    else if(ng = dynamic_cast<neg_goal*>(*j)){
		// 	      if(sg = dynamic_cast<simple_goal*>(ng->getGoal())){
		// 		make_entry_in_FactInfo( &((*e)->cons->n_effects), sg->getProp()->getHashEntry() );
		// 		//     cout << "ADDING to EFF" <<endl;
		// 		//   	    ng->display(0);
		// 	      }
		// 	    }
		// 	  }
		// 	}

		//       }
		//     }
		//     else if(dg = dynamic_cast<disj_goal*>(eff->eff->getClauses())){

		//       i = dg->getGoals()->begin();
		//       	 //  cout << "GOT DG EFF" <<endl;
		// // 	  dg->display(0);
		//       if(!eff->obs)
		// 	(*e)->is_nondeter=TRUE;
		//      for(;i!= dg->getGoals()->end(); ++i) {
		// 	if(!first_disj){
		// 	  tmp = (*e)->cons;
		// 	  (*e)->cons = new_Consequent();
		// 	  (*e)->cons->next = tmp;
		// 	}
		// 	if(sg = dynamic_cast<simple_goal*>(*i)){
		// 	  make_entry_in_FactInfo( &((*e)->cons->p_effects), sg->getProp()->getHashEntry() );
		// 	}
		// 	else if(ng = dynamic_cast<neg_goal*>(*i)){
		// 	  if(sg = dynamic_cast<simple_goal*>(ng->getGoal())){
		// 	     make_entry_in_FactInfo( &((*e)->cons->n_effects), sg->getProp()->getHashEntry() );
		// 	  }
		// 	}
		// 	first_disj = FALSE;
		// 	//(*i)->display(0);
		//       }


		//     }
		//   }



		//  //  CodeNode *j;

		// //   if ( n->connective != AND ) {
		// //     if ( n->connective == NOT ) {
		// //       make_entry_in_FactInfo( &((*e)->n_effects), n->sons->var );
		// //     } else {
		// //       make_entry_in_FactInfo( &((*e)->p_effects), n->var );
		// //     }
		// //   } else {
		// //     for ( j=n->sons; j; j = j->next ) {
		// //       if ( j->connective == NOT ) {
		// // 	make_entry_in_FactInfo( &((*e)->n_effects), j->sons->var );
		// //       } else {
		// // 	make_entry_in_FactInfo( &((*e)->p_effects), j->var );
		// //       }
		// //     }
		// //   }


	}
#endif

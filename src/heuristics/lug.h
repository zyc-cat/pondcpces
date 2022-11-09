#ifndef __LUG_H
#define __LUG_H


#include "ipp.h"
#include "graph_wrapper.h"
#include "graph.h"
#include "stdio.h"
#include "cudd.h"
//#include <stl.h>

class RelaxedPlan;
class LabelledElement;

extern void getHeuristic(std::list<StateNode*>*, StateNode*, int);
extern DdNode* sampleKWorlds(DdNode *orig, int num_samples, DdNode *usedSamples, int * num_goals, int numUsedSamples);
void increment_heuristic(StateNode* node);
#ifdef PPDDL_PARSER 
extern std::list<std::list<int>* >* level_vars;
extern void initLUG(std::map<const Action*, DdNode*>* , DdNode* );
void createInitLayer(DdNode*);
void free_my_info(int);
#else 
#include "actions/actions.h"
extern hash_entry* alt_facts[];
extern void initLUG(action_list , DdNode* );
#endif


//#include "cudd/cuddObj.hh"

class Integer{
public:
  int value;
  Integer(int v) : value(v) {};
};


//
extern DdNode* statesAtLevel(DdNode*, int level);

extern int updateInitLayer(DdNode*);
extern int graph_levels;
extern std::list<LabelledElement*>* getCostsAt(int time, int elt, int type);
extern BitOperator *gbit_operators;
extern BitOperator *used_gbit_operators;
extern BitOperator *my_gbit_operators;
extern FactInfoPair *gbit_goal_state;
extern FactInfoPair *my_gbit_goal_state;
extern int gft_vector_length;
extern int gop_vector_length;
extern int gef_vector_length;
extern DdNode* init_pos_labels[];
extern DdNode* init_neg_labels[];
extern DdNode* b_goal_state;
//extern clausalState* goal_state;
extern DdNode* b_initial_state;

extern double getLabelLevel(DdNode*,std::list<DdNode*>*, StateNode*);
extern int getLabelSum();
extern int getLabelMax();
extern int getMinSumHueristicValue();
extern BitVector* getHelpfulActs(RelaxedPlan*);
extern double build_forward_get_h(DdNode*, BitVector* helpful_acts );
extern double getCardinality(DdNode*); 

extern void printCosts(int w, int time);
extern int costLevelOff(int time);


extern int getFactWorldCost(int time, int fact, DdNode* worlds); 
extern int getActWorldCost(int time, int act, DdNode* worlds); 
extern int getEffectWorldCost(int time, int effect, DdNode* worlds); 
extern void setFactWorldCost(int time, int fact, int cost, DdNode* worlds); 
extern void setActWorldCost(int time, int act, int cost, DdNode* worlds); 
extern void setEffectWorldCost(int time, int effect, int cost, DdNode* worlds); 

extern  int getFactCover(FtNode* f, DdNode* worlds, int time);

extern void addCostLevel(int w);
extern  DdNode** extractDNFfromBDD(DdNode*);
extern  DdNode** extractTermsFromMinterm(DdNode* node);
extern  void free_graph_and_search_info();
extern  void free_graph_info(int);
extern  int build_graph(int*,int,int,int);
extern  void printMemInfo();
extern  void cost_prop_ops(int);
extern  void cost_prop_efs(int);
extern  void cost_prop_facts(int);
extern  int get_bit( BitVector *, int , int );
extern  BitVector* new_bit_vector(int);
extern void copyCostLevelUp(int w, int i);
extern DdNode* substitute_label(DdNode* dd, int k);

extern int costOfCover(std::list<LabelledElement*>* my_list, DdNode* worlds);

#endif









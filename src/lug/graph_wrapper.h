


#ifndef GRAPH_WRAPPER_H
#define GRAPH_WRAPPER_H

#include "ipp.h"
#include "heuristics/relaxedPlan.h"
#include "globals.h"
extern DdNode* initialBDD;


extern double exAbstractAllLabels(DdNode* dd, int time);
extern double exAbstractAllLabels(DdNode* dd, int start, int finish);
extern void printFact(int f);
extern void printBDD(DdNode*);
extern void make_entry_in_FactInfo( FactInfo **f, int index );
RelaxedPlan* getLabelRelaxedPlan(DdNode* cs, int lev, DdNode* worlds);
extern RelaxedPlan* getRelaxedPlan(int, DdNode*, int, DdNode*);
extern int getLabelProven(DdNode*,DdNode*, std::list<DdNode*>* cs, double* pr);
DdNode* pickKRandomWorlds(DdNode* dd, int k);
void pickKRandomWorlds(DdNode* dd, int k, std::list<DdNode*>* worlds);
void generate_BitOperatorEffects(const Action* op);
void generate_BitOperatorEffectsFromDBN(const Action* op);
double costOfGoal(int time, DdNode *worlds); 
extern  int labels_same(int time);
extern int goals_proven(int);
extern  DdNode* get_init_labels(int index, int polarity);
extern int are_there_non_exclusive( int time, FactInfo *pos, FactInfo *neg );
extern  DdNode* and_labels(DdNode*,FtEdge* conditions, int time);
extern  DdNode* or_labels(FtNode* a, int time);
extern DdNode* generateUniqueNonDeterLabel(int time, char* opname, int effnum,  int ndnum);
extern char* getFactName(int f);
extern void set_bit(BitVector* b, int index);
extern int get_bit( BitVector *, int , int );
extern DdNode** extractTermsFromMinterm(DdNode* minterm);
extern DdNode** extractDNFfromBDD(DdNode* node);
void free_k_graphs();
int getRelaxedPlanHeuristic();
int num_states(DdNode*);
double num_states_in_minterm(DdNode*);
void get_even_cubes_from_bdd(DdNode* dd, std::list<DdNode*>* states);  
void get_cubes(DdNode* dd, std::list<DdNode*>* cubes, std::list<double>* values);
void get_states_from_add(DdNode* dd, std::list<DdNode*>* cubes, std::list<double>* values);
void get_cubes(DdNode* dd, std::list<DdNode*>* cubes);
void get_bdd_cubes(DdNode* dd, std::list<DdNode*>* cubes);
double get_sum(DdNode* dd);
double get_min(DdNode* dd);
void get_states_from_bdd(DdNode* dd, std::list<DdNode*>* cubes);
void get_states_from_cube(DdNode* dd, std::list<DdNode*>* cubes, int);
extern DdNode* labelBDD(DdNode*, DdNode*, int);
extern int countParticles(DdNode*);

#ifdef PPDDL_PARSER
extern double exAbstractAllLabels(DdNode* dd, int time);
extern double exAbstractAllLabels(DdNode* dd, int time, int);
extern void generate_BitOperators(const Action*);

extern int build_planning_graph(DdNode*, DdNode*,  int&);
extern int build_k_planning_graphs(DdNode* , DdNode* ,  int&) ;
extern void make_effect_entries( Effect** , DdNode* );
#else
#include "parser/ptree.h"
extern int build_planning_graph(DdNode*, DdNode*, action_list, int&);
extern int build_k_planning_graphs(DdNode* , DdNode* , action_list , int&) ;
extern DdNode* goalToBDD(goal* cs); // a simpler (better?) version of the following
extern DdNode* goalToBDD(goal* cs, int withLabels);
extern void printLabel(goal* g);
extern int getlevel(hash_entry*, int);
extern int getlevelK(hash_entry* , int , int );
extern hash_entry* alt_facts[];
class alt_effect;
extern void make_effect_entries( Effect** , alt_effect* );


//#include "clausalState.h"
//#include "actions/actions.h"

class alt_effect;
class alt_action;


/* typedef struct _k_graph_info { */
/*   // unsigned int* pos_facts_vector_at[IPP_MAX_PLAN]; */
/*   /// unsigned int* neg_facts_vector_at[IPP_MAX_PLAN]; */
/*   //unsigned int op_vector_length_at[IPP_MAX_PLAN]; */
extern   int pos_fact_at_min_level[MAX_RELEVANT_FACTS]; 
extern   int neg_fact_at_min_level[MAX_RELEVANT_FACTS]; 
extern unsigned int max_k_pos_fact_at_min_level[MAX_RELEVANT_FACTS];
extern unsigned int max_k_neg_fact_at_min_level[MAX_RELEVANT_FACTS];
/*   int num_levels; */
/*   gft_table* graph; */

/*   RelaxedPlan* relaxed_plan; */
/* } k_graph_info; */
//DdNode** extractDNFfromBDD(DdNode* node);
//int NDLcounter;

extern void unionElementLabels(std::set<LabelledElement*>* my_list);
extern void unionElementLabels(std::set<LabelledAction*>* my_list);
extern void unionElementLabels(std::list<LabelledElement*>* my_list);
//extern int my_entails(goal* a, goal *b);
extern int getAdjustedSum2M(DdNode*);
//extern int labelGraphLiteralConsistent(goal* a, goal* b, int time);

//extern int getAdjustedSum2M(clausalState*);
//extern action_list_node* rifo(action_list_node*, clausalState*);
//extern int getMaxOfKMinlevelNonMutex(clausalState* );
//extern int getRelaxedPlanHeuristic(clausalState* );

//extern void generate_bitmap_representation(clausalState*, clausalState*,action_list_node*);
//extern void generate_ini_goal_bitmap_representation(clausalState*, clausalState*);
extern void generate_BitOperators(action_list_node*);

//extern int build_planning_graph(clausalState*, clausalState*, action_list, int&);
//extern int build_k_planning_graphs(clausalState* , clausalState* , action_list , int&) ;
//extern clausalState* chooseDisjunctsForConjunctiveState(goal_list*, goal_list*);
//BitVector* build_model(clausalState* cs);
//void set_bit(BitVector* b, int index);
//extern void build_model(goal*, char*);
//extern void build_entailer();
//extern void build_init_entailer();
//extern int entailmentCheck(goal* g1, goal* g2);

//void printBDD(DdNode* bdd);
//int goal_non_mutex(time);
//extern int labelGraphClausalStateConsistent(clausalState*, int);
//extern RelaxedPlan* getRelaxedPlan(int, clausalState*, int, DdNode*);
//extern RelaxedPlan* getLabelRelaxedPlan(clausalState*, int, DdNode*);
extern RelaxedPlan* getLabelRelaxedPlan(DdNode*);

extern int gop_vector_length;



extern int getAdjustedSum2M(DdNode*);

//int getLabelRelaxedPlanHeuristic(clausalState*);
extern DdManager* manager;

extern int COMPUTE_LABELS;

extern int ALLOW_LEVEL_OFF;
extern int goal_non_mutex(int);
extern int isOneOf(FtNode*, FtNode*);

extern int num_alt_facts;


extern void print_fact_info(FactInfo*,int);
extern DdNode** extractDNFfromBDD(DdNode* node);
extern DdNode** extractTermsFromMinterm(DdNode* minterm);

extern void free_fact_info_pair( FactInfoPair *p );
extern Effect *new_effect();
extern void addInitialAxiomForNonDeterEffect(Label* labels, int type, int);
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
extern  Effect *new_Effect(int);
extern  Consequent *new_Consequent();
extern  Integers *new_integers(int );
extern  void print_BitVector( BitVector* , int  );
extern  void build_graph_evolution_step( void );
extern  void print_vector( BitVector *, int  );
extern  BitVector *new_bit_vector(int length);


//extern  goal* get_init_labels(int index, int polarity);
//extern  goal* and_labels(goal*,FtEdge* conditions, int time);
//extern  goal* or_labels(FtNode* a, int time);

extern  void set_bit(BitVector* b, int index);
extern  int goal_non_mutex(int);
//extern  CodeNode *new_CodeNode( Connective c );
//extern  void print_CodeNode( CodeNode *node, int indent );
//extern  void cnf ( CodeNode * codenode ); 
//extern  void dnf ( CodeNode * codenode ); 
extern  int goals_proven(int);
extern  void printBDD(DdNode*);
extern  void copy_contents_of_FactInfo(FactInfo**, FactInfo*);

//extern clausalState* initial_state;
//extern clausalState* goal_state;
//extern int glCompare(goal* i, goal* j);
//extern goal* extract_init_label(int, clausalState*,int);

//extern goal* reduceClause(goal* g, int);
//extern goal* liteResolution(goal* g);
//extern int isSubset(goal*,goal*);
//extern goal* reduceClause(goal*, int);
//extern int listCompare(goal*, goal*);
extern int factsFirstPresent(FactInfoPair*);
extern DdNode* init_pos_labels[MAX_RELEVANT_FACTS];
//extern goal* inClauseListHelper(char*, goal*, int&, int, goal_list*);
extern DdNode* init_neg_labels[MAX_RELEVANT_FACTS];      
//extern goal* init_label;
extern int gop_vector_length;
//extern BitVector* initial_model;

extern  int getEffectWorldCost(int effect, DdNode* worlds);

extern DdManager* manager;
extern DdNode* initialBDD;
extern DdNode* NDUniqueMask;
extern DdNode* b_initial_state;
extern DdNode* b_goal_state;

extern  int isOneOf(FtNode*, FtNode*);


extern void setKGraphTo(int k);
//extern int getlevelNonMutex(clausalState*, clausalState*&);
//extern RelaxedPlan* getRelaxedPlan(int levels, clausalState* cs, int support);
#endif



#endif
 


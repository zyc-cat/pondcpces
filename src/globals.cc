/* 
* $Revision: 1.10 $
* $Date: 2007/07/17 00:08:06 $


globals.cc -- All the main global definitions. */


#include "globals.h"
#include "float.h"

int allowed_time =-1;//10000000;
int debugCnt = 0;
// using node to represente the state variable
DdNode ** current_state_vars;// 当前状态变量列表
DdNode ** next_state_vars;// 后继状态变量列表
DdNode ** aux_state_vars;// 辅助状态变量
DdNode ** particle_vars;
DdNode * current_state_cube;// 当前状态变量的cube，用于forget选取那些变量进行遗忘
DdNode * aux_var_cube;
DdNode * next_state_cube;// 后继状态变量的cube
DdNode * particle_cube;
DdNode * all_but_sample_cube = NULL;


//DdManager *manager;

randomGenerator* randomGen;
int PF_LUG = FALSE;//default is FALSE
int RANDOM_SUBSTRATE = RANDOM_CUBES;
DdNode* new_sampleDD = NULL; //joined new_samples
//std::list<DdNode*>  samples; //currently in use
//std::list<DdNode*>  new_samples; //newly in current use
//std::list<DdNode*>  all_samples; //pool of sample ids
//std::list<DdNode*>  used_samples; //samples not to use
DdNode *goal_samples; //samples where goal is reached
std::map<DdNode*, double> goal_sample_costs;
int LOOKAHEAD_FOR_STATES = -1; //levels to lookahead for inc lug

int LUGTOTEXT = 0;

bool USE_GLOBAL_RP = false;
bool USE_CARD_GRP = false;
int LUG_FOR = NODE;
int HEURISTYPE = NONE;

int MAX_PRES=100;
int NUM_LUG_SAMPLES=3;
int LEVELS_PAST_LEVEL_OFF = 0;
std::map<const Action*, std::map<int, std::map<int, DdNode*>*>*> lug_samples;
bool RBPF_LUG = false;// momo007 2022.09.20 change the default value to false

bool ENUMERATE_SPACE = false;


/* DBN parameters */
/* DBN_PROGRESSION值为false，关闭所有DBN prgession的功能*/
bool DBN_PROGRESSION = false;//true;
/* Rao-Blacckwellised Particle Filtering，默认关系，需要用参数开启*/
bool RBPF_PROGRESSION = false;
int RBPF_SAMPLES=0;
DdNode* RBPF_SAMPLE_CUBE = NULL;
std::map<int, DdNode*> rbpf_sample_map;
int* rbpf_index_map;
int rbpf_bits =0;
double min_reward = DBL_MAX;
double max_reward = -1*DBL_MAX;
/* DBN parameters */
bool DEFERRED_EVALUATION = false;
int OBS_TYPE = OBS_CPT;
bool first_act_noop = false;

/* Dominance parameters */
int MAX_H_INCREMENT = 40;
double INCREMENT_MIN_SLOPE = -1;
bool DO_INCREMENTAL_HEURISTIC = false;//true;
StateNode *currentNode = NULL;

double modom_epsilon = 0.0;
double modom_anc_epsilon = 0.0;
double goal_pr_in = 0.0;

/* Statistical sampling parameters */
double alpha = 0.1;
double beta = 0.1;
double delta = 0.1;
int total_sample_size;
int sample_threshold;
int expected_sample_size;
/* Statistical sampling parameters */


// #ifdef CANDSTATS
// int lowestdepth = MAX_PLAN;
// int parenttotal;
// int Currentbest;
// #define newlowest(x) lowestdepth = lowestdepth<x?lowestdepth:x
// heritage * parent;
// #endif

// #ifdef HASHDEBUG
// int counthits = 0;
// #endif
DdNode* probabilisticLabels; //add stores distribution over effect labels, so lug can be built with bdds
RelaxedPlan* globalRP;
int max_horizon = -1;
int maxNDL = 0;
#ifdef PPDDL_PARSER
#include <FlexLexer.h>
yyFlexLexer *yfl;

bool OPTIMIZE_REWARDS = true;
bool OPTIMIZE_PROBABILITY = true;
const Action* terminalAction;
double goal_threshold = 0.0001;// 在main中根据domain类型设置
double search_goal_threshold = 1.0;
double plan_success_threshold = 1.0;
const Problem* my_problem;
std::list<DdNode*> goal_cnf;
int* varmap;
int warning_level = 1;
int verbosity = 3;// default 2
DdNode* goal_reward;
double total_goal_reward = DBL_MAX;
int beam_search_size = 0;
int server_socket = -1;
/* The reward function. */
Function reward_function;
/* Whether the current domain defines a reward function. */
bool valid_reward_function;
/* State variables for the current problem. */
std::map<const Atom*, int> state_variables;
/* A mapping from variable indices to atoms. */
std::map<int, const Atom*> dynamic_atoms;
/* BDDs representing identity between the `current state' and `next
   state' versions of a variable. */
std::vector<DdNode*> identity_bdds;
/* BDD representing identity between the `current state' and `next
   state' versions of all state variables. */
DdNode* identity_bdd;
/*preconditions of actions*/
std::map<const Action*, DdNode*> action_preconds;
/* MTBDDs representing transition probability matrices for actions. */
std::map<const Action*, DdNode*> action_transitions;
std::map<const Action*, dbn*> action_dbns;
std::map<const Action*, dbn*> action_obs_dbns;
/* MTBDDs representing observations for actions. */
//std::map<const Action*, std::map<const StateFormula*, DdNode*> > action_observations;
//std::map<const Action*, std::map<DdNode*, DdNode*> > action_observations;
std::map<const Action*, std::list<DdNode*>* > action_observations;
//std::map<const Action*, std::map<std::set<const pEffect*>*, DdNode*>* > action_observations_cpt;
std::map<const Action*, std::list<std::map<const pEffect*, DdNode*>*>* > action_observations_cpt;
//for each action keeps a pair of positive and negative affected variables
std::map<const Action*, std::pair<DdNode*, DdNode*>* > action_affects;
/* MTBDDs representing reward vector for actions. */
// std::map<const Action*, DdNode*> action_rewards;
std::map<const Action*, OutcomeSet*> action_outcomes;
plan* the_plan;
 const StateFormula* the_observation;

/*preconditions of events*/
std::map<const Action*, DdNode*> event_preconds;
/* MTBDDs representing transition probability matrices for events. */
std::map<const Action*, DdNode*> event_transitions;
std::map<const Action*, std::pair<DdNode*, DdNode*>* > event_affects;
/* MTBDDs representing reward vector for events. */
std::map<const Action*, DdNode*> event_rewards;
/* exhaustive conditional effects, ala SGP -- dan */
std::map<const Action*, OutcomeSet*> event_outcomes;
/*conditional definability, euqiv BDD*/
ProgMode progMode = FORGETTING; // use the forgetting based progression
std::map<int, std::vector<DdNode*>* > equivBDD;
std::map<int,DdNode *> unit_cube;
std::map<const Action *, std::vector<int> > act_ndefp; // 每个动作不可定义的命题情况
std::map<const Action *, std::map<int, DdNode*> > ndef_literalT;//literal组合的T情况。
std::vector<int> good(4,0);
std::map<int,int> g1ndefTeqMT;
std::map<int,int> g1ndefTneqMT;
int total_act = 0;
int zero_act = 0;
#endif

//proportion of graphs to compute, if 1 then compute all
double NUMBER_OF_MGS = 0.0;
//levels to extract LUG RPs, default all worlds
int LUG_LEVEL_WORLDS = 0;


std::vector<const Action*> candidateplan;
DdNode* counterexample;
DdNode *init_states;

DdNode* b_initial_state=0;
DdNode* b_goal_state=0; // the bdd of the goal formula or goal rewawrd 


DdManager* manager;
int num_k_graphs;
int num_alt_facts = 0;// 被赋值为状态变量的个数，即abcd，不包括后即状态变量个数
int max_num_aux_vars = 0;
 int num_alt_effs = 0;
int num_alt_acts = 0;
hash_entry* alt_facts[MAX_RELEVANT_FACTS];
int initFacts;


// lao stuff
int state_count=0;
/* vector of size gNumStates holds pointers to each state node */
//struct StateNode**  StateIndex=0;



__gnu_cxx::StateHash* StateIndex;
__gnu_cxx::StateHash* LeafStates;

/* start and goal states */
struct StateNode*   Start=0;
struct StateNode*   Goal=0;
/* Conformant planning类型*/
int OBSERVABILITY = OBS_NONE;//PART;


clausalState* goal_state;
clausalState* initial_state;
kGraphInfo** k_graphs;
int K_GRAPH_MAX;
char *HOST="localhost";
int PORT = 1500;
double* alt_act_costs;
/* 采用BDD表示信念状态*/
int STATE_TYPE = BDD_BELIEF;
bool MOLAORand = false;
int RP_EFFECT_SELECTION = RP_E_S_COVERAGE;//RP_E_S_RATIO;
int CWA = TRUE;
#ifdef FINALSTATS
int final_level_count;
int initial_length;
#endif
DdNode* balt_facts[MAX_RELEVANT_FACTS];
int initDONE=FALSE;
/* 在mtbdd.c中如果创建action有observation action则会置为true*/
int SENSORS=FALSE;
/* 在graph_wrapper.cc的generate_BitOperatorEffects中判断outcome的个数是否>1 */
int NDACTIONS=FALSE;
// int EXEFS = FALSE;
int RP_COUNT=RPACTS;// momo007 2022.09.16 change to PRACTS.
// int CARD_TYPE=NONE;
// int REACH_TYPE=NONE;
int SRP_INSERT_METHOD = SRP_REDUCE_MUTEX_INSERT;

int COST_PR_INDIV = 0;
int COST_PROP_LIT=//MAX;//
SUM; //cost of a conjunction
int COST_PROP_WORLD=//MAX;//
SUM; //cost of a cover
int COST_PROP=//MAX;//
SUM; //not used
int GOALS_REACHED = IPP_MAX_PLAN;
int COST_K_LOOKAHEAD = 5;

int GRAPH_SUBSET_SELECTION_STRATEGY = SELECT_GRAPH_LINEARLY;

int COMPUTE_LABELS = TRUE;
int MAX_GRAPHS=-1;
int SUM_STATES_IMPL_CARD=FALSE;
/*RS: Added by RS*/
int CHILDCOMBO = CCLAO;
int COST_PROP_TECH = CPT_FIXED_GROUPING_NEW_WORLDS;//CPT_NEW_WORLDS_ONLY

/*These are the different available options for ALtALt*/

int COST_REPROP=FALSE;
int TAKEMINRP=FALSE;
// int graphbuilt=0;
// int level_out = 0;
// int WHICHSEARCH=STANBASED;

// int SERIAL=FALSE;
// int PACTION=FALSE;
// int NOLEVOFF=TRUE;
// int REVCHECK=FALSE;
int HELPFUL_ACTS=FALSE;
// int PARALLEL=FALSE; //Parallel: Global decision variable
// int PARALLEL1=FALSE;
// int PARALLEL2=FALSE;
// int EXPAND_STATES = TRUE;
int USESENSORS = TRUE;
int GWEIGHT=5;

// int GLEVEL=0; 
// long cutoff_time=1200;
// int PRINT_PLAN_TRACE = FALSE;
int REGRESS_DIRECT_ADJUST = RDA_SET_DIFF_INIT;
int ALLOW_LEVEL_OFF = TRUE;
int LABEL_TYPE = ATMS_LABELS;//STANDARD_LABELS;
int BRANCH_SCHEME = BRANCH_ON_ACTS;
int KACMBP_OUT = FALSE;
// int FILTER_TRANSLATION= FALSE;
int DO_INDUCED = INDUCED;
struct timeval pg_tstart, pg_tend, hsp_tstart, hsp_tend, h_start, h_end, act_tstart, act_tend, rp_tstart, rp_tend, level_tstart, level_tend, dnf_tstart, dnf_tend;
long totalgp = 0, totalhsp = 0, h_total = 0, rp_total = 0, level_total = 0, dnf_total = 0;
int numRPs = 0;
// int numDNFs = 0;
int expandedNodes=0;
// int generatedNodes=0;
// int numberActions=0;
// int numberSteps=0;
char* dname,*pname;
//char* out_file = "my_plan.out";
char* out_file = "/dev/null";
int MUTEX_SCHEME = MS_NONE;//MS_CROSS;
int SORTBYHASH=TRUE;
int WORLD_CHECK = WC_ALL;

/* RS End */


// int exvecsizea;
// int exvecsizef;
// int no_facts;
// int no_acts;
// int hits;
// int oldhits;

// int * fixedreslim;
// int * fixedres;
// int * apprate;
// int * appres;
// char * * resnms;
// int frCount;
// int rCount;
// int rStart;

// char buff[20];

///alt_action * finalAcTable[MAX_ACTIONS];
// int finacts_at[MAX_PLAN];

//int facts_at[MAX_PLAN];
//int acts_at[MAX_PLAN];
//fact * fact_table[MAX_FACTS];
//alt_action * action_table[MAX_ACTIONS];
//action_list available_acts;
//int mutex_at[MAX_PLAN];
//token_list goals_at[MAX_PLAN];
//action_list committed_acts_at[MAX_PLAN];

// #ifdef BADSTATS
// int calls_at[MAX_PLAN];
// int fail2s_at[MAX_PLAN];
// #endif

// #ifdef CHECKSTATS
// int checks[MAX_PLAN][MAX_ACTIONS];
// #endif

// int acnt;

// int stcs = 100;

// #ifdef COUNTCANDS
// int numCs = 0;
// #endif

// int top_level;

// int checked = 0;

//#ifdef BADSTATS
//int badstats[MAX_PLAN];
//#endif

// fact * oGoals[MAX_GOALS];
// int gdval[MAX_GOALS];
// int chlen[MAX_GOALS];
// int num_goals;
// int gcs[EXVECSIZE];

// #ifdef SYMMETRY
// class object;
// int * brokenSym;
// int numAllSymObs;
// object ** symDomobs;
// int symSizes[MAX_ACTIONS];
// #endif



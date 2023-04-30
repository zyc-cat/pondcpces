/*
* $Revision: 1.8 $
* $Date: 2007/07/17 00:08:06 $


globals.h -- All the main global definitions and stuff. */

#ifndef __GLOBALS
#define __GLOBALS

#define PPDDL_PARSER 1
#define IMAGE_TRANSITION 1
#define IPC 1
#define SIZEOF_LONG 8
#define SIZEOF_VOID_P 8
#include <list>
#include <ext/hash_map>
#include <map>

#include <sys/time.h>
#include <sys/times.h>
#include <assert.h>
#include <stdio.h>

//DAN
#include "ipp.h"
#include "graph.h"
#include "lug/randomGenerator.h"

class dbn;
class dbn_node;
// LUG采样的状态个数
extern int NUM_LUG_SAMPLES;
extern std::map<const Action*, std::map<int, std::map<int, DdNode*>*>*> lug_samples;
extern bool RBPF_LUG;
extern int maxNDL;
extern int expandedNodes;


extern int MAX_PRES;
#define ALTALT_CONF
extern int LUGTOTEXT;
//启发式类型
extern int HEURISTYPE;
// CUDD中节点的类型
struct DdNode;
// The unique tables and some auxiliary data structures make up the DdManager (manager for short). 
struct DdManager;
extern DdManager* manager;
// 目标阈值
extern double goal_threshold;
// 搜索阈值
extern double search_goal_threshold;
// 成功阈值
extern double plan_success_threshold;
#define RANDOM_CUBES 0 //cubes
#define RANDOM_STATES 1 //states
// 策略策略
extern int RANDOM_SUBSTRATE;
extern int LUG_LEVEL_WORLDS;
extern int max_horizon;
// 随机数生存器
extern randomGenerator* randomGen;
extern int LOOKAHEAD_FOR_STATES; //levels to lookahead for inc lug
// 目标的CNF对应BDD的节点
extern std::list<DdNode*> goal_cnf;
extern bool USE_GLOBAL_RP;
extern bool USE_CARD_GRP;
extern int beam_search_size;
extern int server_socket;
// no-op的action优先
extern bool first_act_noop;
extern int OBS_TYPE;
#define OBS_CPT 1
#define OBS_TPFP 2

/* Dominance parameters */
extern int MAX_H_INCREMENT;
extern double INCREMENT_MIN_SLOPE;
extern bool DO_INCREMENTAL_HEURISTIC;
extern StateNode *currentNode;

extern double modom_epsilon;
extern double modom_anc_epsilon;
extern double goal_pr_in;
extern double gEpsilon;

/* DBN parameters */
extern bool DBN_PROGRESSION;
/* DBN parameters */
extern bool RBPF_PROGRESSION;
extern int RBPF_SAMPLES;
extern DdNode* RBPF_SAMPLE_CUBE;
extern std::map<int, DdNode*> rbpf_sample_map;
extern int* rbpf_index_map;
extern int rbpf_bits;
extern bool DEFERRED_EVALUATION;
extern    double min_reward;
extern    double max_reward;
/*===========================================================*/
/* Statistical sampling parameters */
extern double alpha;
extern double beta;
extern double delta;
extern int total_sample_size;
extern int sample_threshold;
extern int expected_sample_size;
/* Statistical sampling parameters */

extern bool ENUMERATE_SPACE;

#ifdef PPDDL_PARSER
extern int PF_LUG;
//extern std::list<DdNode*> samples;
//extern std::list<DdNode*>  new_samples; //newly in current use
//extern std::list<DdNode*> all_samples;
//extern std::list<DdNode*> used_samples;
extern DdNode* new_sampleDD; //joined new_samples
extern DdNode* probabilisticLabels;
extern DdNode* goal_samples;
extern std::map<DdNode*, double> goal_sample_costs;
/*----------------------------------------------------------------------------
  Plan
 ----------------------------------------------------------------------------*/
#include "simulator/monitor.h"

// plan的一部操作，包含action及时间相关的信息
class plan_step //: public parse_category
{
public:
    Action* op_sym;
    //const_symbol_list* params;

    bool start_time_given;
    bool duration_given;
    float start_time;
    float duration;

    plan_step(Action* o) :
      op_sym(o)//,
      //params(p)
	{};

    virtual ~plan_step()
	{
	  //	    delete params;
	};

/*     virtual void display(int ind) const; */
/*     virtual void write(std::ostream & o) const; */
};



/* 一个plan包含的相关信息,
  plan内使用了Instruction而不是plan_step */
class plan //: public parse_category
{
public:
	char * name;
	char * problem_name;
  // could contain an initial state
  // and goal state for indexing into a plan library
  Instruction *begin;
  plan() {};
  ~plan();
};
// Label
class label_symbol
{

 public:
  const std::string symbol;
  Instruction *instr;
  label_symbol(const std::string& s) : symbol(s) {instr = 0;}
  virtual ~label_symbol() {};

};

#include "ppddl/allheaders.h"
extern int debugCnt;
extern bool OPTIMIZE_REWARDS;
extern bool OPTIMIZE_PROBABILITY;
extern const Action* terminalAction;
//#define  FRACT_RATIONALS //use num/denom to represent rationals instead of doubles
extern int verbosity;
extern DdNode* solve_problem(const Problem&, double, double);
extern const Problem* my_problem;
/* /\* The reward function. *\/ */
/*  extern Function reward_function;  */
/* Whether the current domain defines a reward function. */
extern DdNode* goal_reward;
extern double total_goal_reward;
extern bool valid_reward_function;


/*==========================================================
	后续这部分是重点，一些关键的定义
==========================================================*/

// state variable和 命题之间的映射
/* State variables for the current problem. */
extern std::map<const Atom*, int> state_variables;
/* A mapping from variable indices to atoms. */
extern std::map<int, const Atom*> dynamic_atoms;
extern int* varmap;


/* 当前状态和后继状态等价的BDD，即 x0=x0‘,x1=x1',...*/
/* BDDs representing identity between the `current state' and `next
   state' versions of a variable. */
extern std::vector<DdNode*> identity_bdds;
/* 当前状态和后继状态等价的BDD，即 X=X‘ */
/* BDD representing identity between the `current state' and `next
   state' versions of all state variables. */
extern DdNode* identity_bdd;
/* 动作相关的BDD定义 */
/*preconditions of actions*/
extern std::map<const Action*, DdNode*> action_preconds;
/* MTBDDs representing transition probability matrices for actions. */
/* 表示action的Effect+Frame axioms BDD */
extern std::map<const Action*, DdNode*> action_transitions;
/* 深度信念网络DBN相关的结构 */
extern std::map<const Action*, dbn*> action_dbns;
extern std::map<const Action*, dbn*> action_obs_dbns;
/* 动作的effect，pair的first表示action，pair的second存储变换前后的atom的BDD */
extern std::map<const Action*, std::pair<DdNode*, DdNode*>* > action_affects;
/* MTBDDs representing reward vector for actions. */
// extern std::map<const Action*, DdNode*> action_rewards;
/* 根据formula计算得到BDD，prime=true代表后继状态，false为当前状态 */
extern DdNode* formula_bdd(const StateFormula& formula, bool primed);
 /* exhaustive conditional effects, ala SGP -- dan */
 /* 每个action有一个outcomeSet,每个outcome代表一种更新结果 */
extern std::map<const Action*, OutcomeSet*> action_outcomes;
/* observation的formula */
extern const StateFormula* the_observation;
/**
 * 这里注释的observation结构更像是conformant planning使用的
 */
//extern std::map<const Action*, std::map<const StateFormula*, DdNode*> > action_observations;
//extern std::map<const Action*, std::map<DdNode*, DdNode*> > action_observations;
/**
 * 下面使用的observation结构更像是DBN中使用的结构
 */
extern std::map<const Action*, std::list<DdNode*>* > action_observations;
//extern std::map<const Action*, std::map<std::set<const pEffect*>*, DdNode*>* > action_observations_cpt;
extern std::map<const Action*, std::list<std::map<const pEffect*, DdNode*>*>* > action_observations_cpt;
 extern plan* the_plan;

/**
 * 下面设计event的结构，这部分在conformant中可忽略
 */
/*preconditions of events*/
extern std::map<const Action*, DdNode*> event_preconds;
/* MTBDDs representing transition probability matrices for events. */
extern std::map<const Action*, DdNode*> event_transitions;
extern std::map<const Action*, std::pair<DdNode*, DdNode*>* > event_affects;
/* MTBDDs representing reward vector for events. */
extern std::map<const Action*, DdNode*> event_rewards;
 /* exhaustive conditional effects, ala SGP -- dan */
extern std::map<const Action*, OutcomeSet*> event_outcomes;
/*conditional definability, euqiv BDD*/
enum ProgMode
{
   FORGETTING = 1,
   PARTITION_MERGE = 2,
   DEFINABILITY = 3,
};
extern ProgMode progMode;
extern std::map<int, std::vector<DdNode*>* > equivBDD;
extern std::map<int,DdNode *> unit_cube;
extern std::map<const Action *, std::vector<int> > act_ndefp; // 每个动作不可定义的命题情况
extern std::map<const Action *, std::map<int, DdNode *> > ndef_literalT;//literal组合的T情况。
extern std::vector<int> good;
extern std::map<int,int> g1ndefTeqMT;
extern std::map<int,int> g1ndefTneqMT;
extern int total_act;
extern int zero_act;
/**
 * A set of state transitions.
 * 一个transitionSet包括了con(e)的BDD和修改的atom
 */
// 存储多个状态转换的集合
struct TransitionSetList;
// 记录转换关系，即前提条件和相应的相关
struct TransitionSet {
  /* Fills the provided list with the conjunction of the given
     transition sets. */
  static void conjunction(TransitionSetList& transitions,
			  const TransitionSet& t1, const TransitionSet& t2);

  /* Constructs a null transition set. */
  TransitionSet();

  /* Constructs a copy of the give transition set. */
  TransitionSet(const TransitionSet& t);

  /* Constructs a transition set for a simple effect. */
  TransitionSet(DdNode* condition_bdd, const Atom& atom, bool is_true);

  /* Constructs a transition set for a reward effect. */
  TransitionSet(DdNode* condition_bdd, const Rational& reward);

  /* Constructs a transition set. */
  TransitionSet(DdNode* condition_bdd, DdNode* effect_bdd,
		const Rational& reward, const std::set<int>& touched_variables)
    : condition_bdd_(condition_bdd), effect_bdd_(effect_bdd),
      reward_(reward), touched_variables_(touched_variables) {}

  /* Deletes this transition set. */
  ~TransitionSet();
  // 重点查看，BDD转换函数相关的实现
  /* Returns the BDD representing the condition of this transition set. */
  DdNode* condition_bdd() const { return condition_bdd_; }

  /* Returns the BDD representing the effect of this transition set. */
  DdNode* effect_bdd() const { return effect_bdd_; }

  /* Returns the reward associated with this transition set. */
  const Rational& reward() const { return reward_; }

  /** Returns the variables touched by the effect of this
   * transition set. 
   */
  std::set<int>& touched_variables() { return touched_variables_; }

  /* Returns the variables touched by the effect of this
     transition set. */
  const std::set<int>& touched_variables() const { return touched_variables_; }

  /* Tests if this is a set of self-transitions with zero reward. */
  bool is_null() const {
    return effect_bdd() == Cudd_ReadOne(manager) && reward() == 0.0;
  }
  int index() const {return index_;}
  void set_index(int i){index_ = i;}

private:
  /* BDD representing the condition of this transition set. */
  DdNode* condition_bdd_;
  /* BDD representing the effect of this transition set. */
  DdNode* effect_bdd_;
  /* Reward associated with this transition set. */
  Rational reward_;
  /** Variables touched by the effect of this transition set. 
   * 这里存储int使用前面定义的dynamic_atoms结构可以知道是修改那些atom
   */
  std::set<int> touched_variables_;
  int index_;
};

// 转换集合的列表
struct TransitionSetList : public std::vector<const TransitionSet*> {
};
// 输出集合，每个输出集合是一组转换关系
struct OutcomeSet {
  OutcomeSet(){}
  std::vector<Rational> probabilities;// 每个outcome即transitionSet的概率
  std::vector<TransitionSetList> transitions;
};
#endif

/*
========================================================================
========================================================================
*/
class RelaxedPlan;
extern RelaxedPlan* globalRP;

/* Globals having to do with DD */
#define MAX_RELEVANT_FACTS 10000/* i think this is VERY generous... */
extern int allowed_time;
extern int initFacts;
extern int num_k_graphs;
extern int num_alt_facts;
extern int max_num_aux_vars;
extern int num_alt_effs;
extern int num_alt_acts;
class hash_entry;
extern hash_entry* alt_facts[MAX_RELEVANT_FACTS];
extern DdNode* balt_facts[MAX_RELEVANT_FACTS];
struct Rational;
extern int LEVELS_PAST_LEVEL_OFF;
extern double* alt_act_costs;



namespace __gnu_cxx {

// 定义一个BDD节点的哈希实现，返回指针的无符号类型。
template<> struct hash<DdNode*>{
  size_t const operator()(DdNode* p) const {return (size_t)p;}
};
// 继承封装得到一个state的哈希表（1个key可对应多个数据元素）
struct StateHash : public hash_multimap<DdNode*, StateNode*, hash<DdNode*>, eqstr>{
  // 添加state到哈希表
  void add(StateNode* p){
    this->insert(std::pair<DdNode*,StateNode*>(p->dd, p));
  }
  // 哈希表中移除一个state
  void remove(StateNode* p){
    std::pair<StateHash::iterator, StateHash::iterator> q = this->equal_range(p->dd);
    for(StateHash::iterator i = q.first; i != q.second; i++){
      // find which one it is
      if((*i).second->StateNo == (p)->StateNo){
	    this->erase(i);
	    break;
      }
    }
  }
  // 哈希表中是否包含该state
  bool contains(StateNode* p){
    pair<StateHash::iterator, StateHash::iterator> q = this->equal_range(p->dd);
    return (q.first != q.second);
  }
};

}

class PointerHasher {
public:
    size_t operator()(const void *key) const {
        return reinterpret_cast<unsigned long>(key);
    }
};

extern __gnu_cxx::StateHash* StateIndex;
extern __gnu_cxx::StateHash* LeafStates;

extern std::vector<const Action*> candidateplan;
extern DdNode* counterexample;
extern DdNode *init_states;

extern  DdNode* b_initial_state;
extern  DdNode* b_goal_state;
extern char *HOST;
extern int PORT;
/**
 * caltclat中采用的CNF来保存状态，但这里的clausalState没有使用到，可忽略
 */
class clausalState;
extern clausalState* goal_state;
extern clausalState* initial_state;
/**
 * 选择observability的类型full完全可观察，part部分可观察(Contingent)，none不可观察(conformant)
 */
extern int OBSERVABILITY;
#define OBS_FULL 0
#define OBS_PART 1
#define OBS_NONE 2
/**
 * 这块功能不清楚
 */
#define CPT_NEW_WORLDS_ONLY 0
#define CPT_FIXED_GROUPING_NEW_WORLDS 1
#define CPT_ALL_SUPPORTERS 2
extern int GWEIGHT;
extern int GOALS_REACHED;
extern int COST_K_LOOKAHEAD;
extern int COST_PROP_TECH;
extern int COST_PROP;
extern int COST_PROP_WORLD;
extern int COST_PROP_LIT;
extern int COST_PR_INDIV;
extern int SRP_INSERT_METHOD;
#define SRP_GREEDY_INSERT 0
#define SRP_REDUCE_INSERT 1
#define SRP_MUTEX_INSERT 2
#define SRP_REDUCE_MUTEX_INSERT 3
extern int state_count;
class kGraphInfo;
extern int K_GRAPH_MAX;
extern kGraphInfo** k_graphs;
extern DdNode ** current_state_vars;
extern DdNode ** next_state_vars;
extern DdNode ** aux_state_vars;
extern DdNode ** particle_vars;
extern DdNode * current_state_cube;
extern DdNode * aux_var_cube;
extern DdNode * next_state_cube;
extern DdNode * particle_cube;
extern DdNode * all_but_sample_cube;

#define RP_E_S_COST 0
#define RP_E_S_COVERAGE 1
#define RP_E_S_RATIO 2
extern int RP_EFFECT_SELECTION;
extern int initFacts;
extern int COST_PROP_LIT;
extern int COST_PROP_WORLD;
extern int NDLcounter;
#define SUM 1
#define MAXN 2
extern int CHILDCOMBO;
#define CCMAX 1
#define CCLAO 2
extern int initDONE;
extern int COST_REPROP;
extern int TAKEMINRP;
extern int SENSORS;
extern int NDACTIONS;
// extern int EXEFS;
extern int STATE_TYPE;
extern int num_alt_facts;
extern int MAX_GRAPHS;
// extern int CARD_TYPE;
// extern int REACH_TYPE;
extern int GRAPH_SUBSET_SELECTION_STRATEGY;
// extern int PRINT_PLAN_TRACE;
extern int REGRESS_DIRECT_ADJUST;
extern int SUM_STATES_IMPL_CARD;

#define MAX_VARS 15

extern int HELPFUL_ACTS;

/* typedef struct _k_graph_info { */
/*   BitVector_pointer pos_facts_vector_at[IPP_MAX_PLAN]; */
/*   BitVector_pointer neg_facts_vector_at[IPP_MAX_PLAN]; */
/*   unsigned int* op_vector_length_at[IPP_MAX_PLAN]; */
/*   int num_levels; */
/* } k_graph_info; */

/* k_graph_info k_graphs[K_GRAPH_MAX]; */
extern int COMPUTE_LABELS;
extern int LUG_FOR;
#define SPACE 0
#define NODE 1
#define FRONTIER 2
#define INCREMENTAL 3
#define AHREACHABLE 4

//NAD



/**
 * problem name, domain name, outfile name
 */
extern  char * pname,*dname, *out_file;



/**
 * some configuration about the problem 
 */
#define MAX_HASH 1024 //from 50 Increase to 100 //DAN 1024 from 100
#define MAX_FACTS 10244 //from 5122 to 10244 For some domains this constant needs to be increased
#define MAX_GOALS 200
#define MAX_ACTIONS 10244 //from 5122 10244
#define MAX_PLAN 1024
#define EXVECSIZE 360 //from 160 Increase to 320
#define BIGPRIME 8000977
#define TRUE 1
#define FALSE 0
#define MAX_ACTIONS_LAYER 1000



/**
 * planner使用CNF还是BDD类型进行存储
 */
#define CLAUSAL_BELIEF 100
#define BDD_BELIEF 101

/**
 * 启发式函数的类型
 */
/* RS Added */
#define MAX_PROPS 100 //It was 50 //It should be more...
#define HSPBASED 100
#define STANBASED 200
#define HMAX 100
#define HSUM 101
#define HLEVEL 102
#define HRP 103
#define HSUMK 104
#define HLEVELK 105
#define HRPMAX 106
#define HRPUNION 107
#define HCARD 108
#define HSUML 109
#define HRPL 110
#define HCOMBO 111
#define HLEVELL 112
#define HRPSUM 113

#define LUGRP 114
#define SLUGRP 115
#define CARD 116
#define NONE 117
#define LUGLEVEL 118
#define LUGMAX 119
#define LUGSUM 120
#define CORRRP 121

extern int DO_INDUCED;
#define INDUCED 99
#define NINDUCED 98

extern int CWA;

/**
 * LUG中标签的类型
 */
#define ATMS_LABELS 400
#define STANDARD_LABELS 401

extern bool MOLAORand;

//proportion of graphs to compute, if 1 then compute all
extern double NUMBER_OF_MGS;

// extern long cutoff_time;
/**
 * 用于统计各种计算的时间
 */
extern struct timeval pg_tstart, pg_tend, hsp_tstart, hsp_tend, h_start, h_end,
act_tstart, act_tend, rp_tstart, rp_tend, level_tstart, level_tend, dnf_tstart,
dnf_tend;
extern long totalgp, totalhsp, h_total, rp_total, level_total, dnf_total;
extern int numRPs;


#define CARD_DNF_EXPAND 200
#define CARD_STRENGTH 201
#define CARD_CONSISTENT 202

#define RDA_SET_DIFF_INIT 250
#define RDA_FOOTPRINT 251

#define SELECT_GRAPH_LINEARLY 300
#define SELECT_GRAPH_NONDETER 301
#define SELECT_GRAPH_DETER 302

#define MS_NONE 352
#define MS_CROSS 350
#define MS_REGULAR 351

#define RPACTS 375
#define RPEFFS 376

extern int RP_COUNT;

extern int MUTEX_SCHEME;

extern int ALLOW_LEVEL_OFF;
extern int LABEL_TYPE;

extern int SORTBYHASH;

#define BRANCH_ON_CONDS 400
#define BRANCH_ON_ACTS 401

extern int BRANCH_SCHEME;
extern int KACMBP_OUT;
// extern int FILTER_TRANSLATION;
extern int USESENSORS;



extern int WORLD_CHECK;

#define WC_ALL 500
#define WC_SAME 501
#define WC_INTERSECT 502




extern int CHILDCOMBO;
extern int MUTEX_SCHEME;
#define MS_CROSS 350  // 不同的mutex策略
#define MS_REGULAR 351
#define MS_NONE 352
#define MS_STATIC 353

extern int ALLOW_LEVEL_OFF;
#define ATMS_LABELS 400

extern int LABEL_TYPE;
extern int HELPFUL_ACTS;



#endif

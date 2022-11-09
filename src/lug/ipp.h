/*********************************************************************
 * File: ipp.h
 * Description: Types and structures for the PP planner.
 *
 * Author: Joerg Hoffmann / Frank Rittinger / Andreas Schoen
 * Contact: hoffmann@informatik.uni-freiburg.de
 *
 *********************************************************************/ 
/*********************************************************************
 * (C) Copyright 1998 Albert Ludwigs University Freiburg
 *     Institute of Computer Science
 *
 * All rights reserved. Use of this software is permitted for 
 * non-commercial research purposes, and it may be copied only 
 * for that use.  All copies must include this copyright message.
 * This software is made available AS IS, and neither the authors
 * nor the  Albert Ludwigs University Freiburg make any warranty
 * about the software or its performance. 
*********************************************************************/

#ifndef __IPP_H
#define __IPP_H

struct DdNode;

#include "globals.h"
#include "dbn.h"

#ifdef PPDDL_PARSER
class Action;
#else
#include "parser/ptree.h"
#endif

#include <set>
#include <map>

/*
 *  ------------------------------------ DEFINES ----------------------------
 */

/***********************
 * MEANINGLESS HELPERS *
 ***********************/

/* if this is defined, memory consumption gets recorded
 */
#define MEMORY_INFO

/* fprintf s are parametrized with OUT 
 */
#define ERR stderr
#define OUT stdout

/* 
 * The following constants are for exit_codes for shellscripts
 */
#ifndef ERROR_CODES
#define ERROR_CODES
#define EXIT_FAILURE              1 /* for not yet classified errors */
#define NOT_SUPPORTED_ERROR_CODE  2
#define BADDOMAIN_ERROR_CODE      3
#define FCT_PARSE_ERROR_CODE      4
#define FCT_MISSING_ERROR_CODE    5
#define OPS_PARSE_ERROR_CODE      6
#define OPS_MISSING_ERROR_CODE    7
#define PARSE_ERROR_CODE          8
#define USAGE_ERROR_CODE          9
#define OTHER_ERROR_CODE         10 
#define INTERNAL_ERROR_CODE      11 /* these errors 
                                    should never happen finally */
#define TRAFO_INTERNAL_ERROR_CODE 12

#endif

/* strcmp returns 0 if two strings are equal, which is not nice */
#define SAME 0


/****************
 * PARSING ETC. *
 ****************/

/* type of quantifier, used for parsing */
#define NO_QUANT '-'
#define ALL_QUANT 'A'
#define EX_QUANT 'E'
#define ANY_PRED 0
#define EQ 1
#define EQ_STR "eq"
#define NOT_EQ 2
#define NOT_PRED '!'
#define NOT_EQ_PRED "!eq"
#define NO_SOLUTION "NO SOLUTION\n"

/* 
 * The following constants are for pl1 expression preprocessing.
 */
#define LIT_CONST      1
#define AND_CONST      2
#define OR_CONST       3
#define IMPLY_CONST    4
#define NOT_CONST      5
#define EXISTS_CONST   6 
#define FORALL_CONST   7
#define LBRACK_CONST   8
#define RBRACK_CONST   9
#define VAR_CONST     10
#define ENDNOT_CONST  11
#define HIDDEN_STR "#"
#define AXIOM_STR "AXIOM"
#define NAME_STR "name\0"
#define VARIABLE_STR "variable\0"
#define STANDARD_TYPE "OBJECT\0"
#define GOAL_OP_STR "#REACHGOAL"
#define GOAL_REACHED "#GOALREACHED"
#define EITHER_STR "EITHER"


/***************************
 * SOME ARBITRARY SETTINGS *
 ***************************/

/* maximal string length */
#define MAX_LENGTH 256 

/* marks border between connected items */
#define CONNECTOR "~"

/* der blanke wahnsinn */
#define NOOP "noop"


/************************
 * INSTANTIATION LIMITS *
 ************************/

#define MAX_CONSTANTS_TABLE 500
#define MAX_PREDICATES_TABLE 50
#define MAX_TYPES_TABLE 200
#define MAX_ARITY 6
#define MAX_VARS 15

#define MAX_RELEVANT_FACTS 10000/* i think this is VERY generous... */


/******************************
 * GRAPH AND SEARCHING LIMITS *
 ******************************/

#define IPP_MAX_PLAN 100
#define ARRAY_SIZE 50
#define MEMO_HASHSIZE 4
#define MEMO_HASH 3

/****************
 * CODE DEFINES *
 ****************/

/* define boolean types if not allready defined
 */
#ifndef Bool
typedef unsigned char Bool;
#ifndef TRUE /* we assume that FALSE is also not defined */
#define TRUE 1
#define FALSE 0
#endif /* TRUE */
#endif /* Bool */

/* Check allocated memory
 */
#define CHECK_PTR(p) if (NULL == (p)){printf("NULL---EXITING\n");exit(1);}

/* add elapsed time from main local time vars to specified val
 */
#define TIME( val ) val += ( float ) ( ( end.tms_utime - start.tms_utime + \
					 end.tms_stime - start.tms_stime  ) / 100.0 )

/* compute the adress of negative fact in fact array
 */
#define NEG_ADR( index ) gnum_relevant_facts + index


/*
 *  ------------------------------ DATA STRUCTURES ----------------------------
 */


/*******************
 * GENERAL HELPERS *
 *******************/

/* This holds all command line switches
 */
struct _command_line{

  char path[MAX_LENGTH];
  char ops_file_name[MAX_LENGTH];
  char fct_file_name[MAX_LENGTH];
  
  int display_info;
  
  int write_graph;
  char *save_name;

  int do_subset;
  int min_time;

  int rifo_on;
  int rifo_meta_on;
  int rifo_union;
  int rifo_filter;

  /* for various debugging informations
   */
  int debug;

};

/* different reasons for program termination 
 */
typedef enum{MEMORY, SYNTAX_ERR , IO_ERROR} TermReason;

typedef unsigned int BitVector, *BitVector_pointer;

typedef struct _Integers{

  int index;
  struct _Integers *next;

} Integers, *Integers_ptr;

typedef char *String;

typedef struct _StringIntegers{

  char *name;
  Integers *integers;

} StringIntegers;

typedef int *int_pointer;

typedef int IntArray[ARRAY_SIZE];

typedef char *Token;


/***********
 * PARSING *
 ***********/

/* A list of strings
 */
typedef struct _TokenList{

  char *item;
  struct _TokenList *next;

} TokenList;

/* list of string lists
 */
typedef struct _FactList{

  TokenList *item;
  struct _FactList *next;

} FactList;

/* structure to store  typed-list-of <name>/<variable>,
 * as they are declared in PDDL files
 */
typedef struct _TypedList{

  char *name;

  /* each item in this list is the name of a type which
   * our type is the union of (EITHER - types ...)
   *
   * usually, this will default to a single-item TokenList.
   */
  TokenList *type;
  /* after first sweep, this will contain the number in type table
   */
  int n;

  struct _TypedList *next;

} TypedList;

/* only needed to parse in the predicates and their arg
 * definitions
 */
typedef struct _TypedListList{

  char *predicate;

  TypedList *args;

  struct _TypedListList *next;

} TypedListList;

/* the type_tree structure is used to deal with types and subclasses
 *  of types
 */
typedef struct TYPETREE_LIST *type_tree_list, type_tree_list_elt;

typedef struct TYPETREE{
  
  char *name;  /* an object type */
  type_tree_list sub_types;

} *type_tree, type_tree_elt;

struct TYPETREE_LIST{

  type_tree item;
  struct TYPETREE_LIST *next;

};


/***************** 
 * INSTANTIATION *
 *****************/

typedef int ArgArray[MAX_ARITY];

/* actually, this structure is not needed.
 * just for debugging and info:
 * stores (in grelevant_facts) the relevant fact to it's index
 */
typedef struct _Relevant_Fact{

  short int predicate;
  ArgArray arguments;

} RelevantFact, *RelevantFact_pointer;


/****************************
 * BITVECTOR REPRESENTATION *
 ****************************/
// 用于构造op和effect的前提和结果的基础元素
typedef struct _FactInfo{

  BitVector *vector;// 一个Bitvector记录各个pos的是否存在
  Integers *indices;// 一个link list存储所有的pos的值

} FactInfo;

typedef struct _FactInfoPair{

  FactInfo *positive;
  FactInfo *negative;

} FactInfoPair;
// 用于构造effect的前提
typedef struct _Antecedent{

  FactInfo *p_conds;
  FactInfo *n_conds;

  DdNode* b;
  struct _Antecedent *next;

} Antecedent;
// 用于构造effect的结果
typedef struct _Consequent{

  FactInfo *p_effects;
  FactInfo *n_effects;
  DdNode* b;

  struct _Consequent *next;

} Consequent;
// 标记到当前结点的状态信息
typedef struct _Label{

  DdNode* label;// use the label recode the info
  double CSD; /*Cummulative Support Density*/
  struct _Label *next;

} Label;

typedef struct _BitOperator BitOperator;
// 主要包括该effect所在的op和effect的前提和结果
typedef struct _Effect{
  int index; /*matches alt_action index*/
  int is_nondeter;
  //  int outcome; //index of nd outcome that it part of
  std::set<int>* outcome;//BitVector* outcome; //every outcome effect is part of
  std::map<int, double>* probability;//double probability; //probability of outcome;
  dbn_node* node;
  DdNode* row;
  double probability_sum;
  int in_rp;
  BitOperator *op;
  Antecedent *ant;// 前提FactInfo + BDD
  Consequent *cons;// 结果FactInfo + BDD
  double reward;// effect的reward
  struct _Effect* original;
  struct _Effect *next;

} Effect;
// 该动作的precondtion，和多种类型的effect,相应的Action的BDD
struct _BitOperator{

  char *name;// 动作名
  short int num_vars, inst_table[MAX_VARS];
  int alt_index;
  int nonDeter;// 是否确定
  int valid;
#ifdef PPDDL_PARSER
  const Action* action;// PPDDL中定义的动作
#endif
  int num_outcomes;// 输出个数
  FactInfo *p_preconds;// 前提positive-Fact
  FactInfo *n_preconds;// 前提negative-Fact
  DdNode* b_pre; // BDD precodition
  Effect *unconditional;// IPP的无条件Effect list
  Effect *conditionals;// IPP的条加Effect list
  Effect *activated_conditionals;// activated condtional effect(成功使用的effect)
  struct _BitOperator *next;

};


/**********************
 * BUILDING THE GRAPH *
 **********************/

/* 
 * Literature: e.g. Technical report 88 
 *         "Extending Planning Graphs to an ADL Subset"
 *
 * more detailed description of current implementation 
 * forthcoming, see IPP - Homepage
 *
 * ... or have a look into 
 * "The efficient Implementation of the Planning Graph in STAN",
 * D.Long and M.Fox, JAIR, 10, 1999
 */

/*
 * ...some preliminary definitions, used in the graph nodes
 */
typedef struct _OpNode OpNode, *OpNode_pointer;
typedef struct _EfNode EfNode;
typedef struct _FtNode FtNode, *FtNode_pointer;
typedef struct _OpEdge OpEdge;
typedef struct _EfEdge EfEdge;
typedef struct _FtEdge FtEdge;
typedef struct _OpLevelInfo OpLevelInfo, *OpLevelInfo_pointer;
typedef struct _EfLevelInfo EfLevelInfo, *EfLevelInfo_pointer;
typedef struct _FtLevelInfo FtLevelInfo, *FtLevelInfo_pointer;

typedef FtNode_pointer FtArray[ARRAY_SIZE];
typedef OpNode_pointer OpArray[ARRAY_SIZE];

typedef struct _OpExclusion OpExclusion;
typedef struct _FtExclusion FtExclusion;
typedef struct _EfExclusion EfExclusion;

/* nodes in the memoization tree UBTree Data Structure
 */
typedef struct _MemoNode MemoNode, *MemoNode_pointer;
typedef MemoNode_pointer MemoNode_table[MEMO_HASHSIZE];

/*
 * operator representation
 */
struct _OpNode{

  const char *name;
  short int num_vars, inst_table[MAX_VARS];


  int alt_index;
  int index;
  int uid_block;
  unsigned int uid_mask;

  int nonDeter;
  FtEdge *preconds;/* a list of pointers to the nodes for its preconds, set by insert_ft_edge() */
  EfNode *unconditional;
  EfNode *conditionals;
  DdNode *b_pre;// set in new_op_node(), if noop, set condition as effect fact

  int is_noop;/* is it a noop ? (used in printing out the plan) */

  OpLevelInfo_pointer info_at[IPP_MAX_PLAN];/* level-dependent info */

  BitVector *pos_precond_vector;
  BitVector *neg_precond_vector;

#ifdef PPDDL_PARSER
  const Action* action;
#endif

  struct _OpNode *next;/* ops are globally stored as a list */

  Effect *unactivated_effects;
  struct _OpNode *thread;/* use to link the list: gops_with_unactivated_effects_pointer*/

};

/*
 * effects...
 */
struct _EfNode{

  OpNode *op;/* the op it belongs to */
  unsigned int first_occurence;/* ...of this effect in the graph */
  int is_nondeter;
  int in_rp;// if current effect in relaxed plan

  int index;
  int alt_index;
  int uid_block;
  unsigned int uid_mask;

  FtEdge *conditions;// 前提fact
  FtEdge *effects;// 效果fact

  Effect* effect;// 

  EfLevelInfo_pointer info_at[IPP_MAX_PLAN];

  struct _EfNode *all_next;
  struct _EfNode *next;

};

/*
 * ...and facts.
 */
struct _FtNode{

  int index;/* index in fact hierarchy */

  int uid_block;
  unsigned int uid_mask;

  int positive;/* is it the positive occurence ? */

  OpNode *noop;/* to make noops - first strategy explizit */
  EfEdge *adders;/* the effects that add this fact */
  OpEdge *preconds;

  FtLevelInfo_pointer info_at[IPP_MAX_PLAN+1];/* level dependent information */

  struct _FtNode *next;

};

/* simply a list element for op-edge-lists.
 */
struct _OpEdge{

  OpNode *op;

  struct _OpEdge *next;

};

/* simply a list element for effect-edge-lists.
 */
struct _EfEdge{

  EfNode *ef;

  struct _EfEdge *next;

};

/* same for facts
 */
struct _FtEdge{

  FtNode *ft;

  struct _FtEdge *next;

};

/*
 * info for an op that changes during evolution of graph
 */
struct _OpLevelInfo{

  int is_used;
  Label* label;
  int updated; //incremental version updated node
  BitVector *sensor_dependencies;
  double probability; //used in corrrp

  DdNode* max_worlds;

  /*  BitVector *exclusives;*/
  OpExclusion* exclusives;
};

struct _EfLevelInfo{

  int is_dummy;
  Label* label;
  int updated; //incremental version updated node  
  EfExclusion* exclusives;
  std::map<int, double>* probability;
  double probability_sum;
  DdNode* max_worlds;
};

/*
 * level dependent info for facts
 */
struct _FtLevelInfo{
  Label* label;

  EfEdge *adders_pointer;// 实现该fact的effect Node

  int is_dummy;
  
  int is_goal;
  int is_true;
  int updated; //incremental version updated node
  double probability; //used in correlation graph
  OpArray is_goal_for;

  FtExclusion* exclusives;// 记录的Fact mutex

  EfEdge *relaxedPlanEdges;// plan由effect的fact形成，efNode中存储了op

  BitVector *adders;
  EfExclusion* adders_exclusives;// 和实现该Fact的Effect互斥的Effect

  MemoNode *memo_start;

};

typedef struct _OpPair{

  OpNode *o1;
  OpNode *o2;

  struct _OpPair *next;

} OpPair;

typedef struct _EfPair{

  EfNode *e1;
  EfNode *e2;

  struct _EfPair *next;

} EfPair;

typedef struct _FtPair{

  FtNode *f1;
  FtNode *f2;

  struct _FtPair *next;

} FtPair;

typedef struct _ExclusionLabelPair{
  
  int last;
  DdNode* elp1;
  DdNode* elp2;

  struct _ExclusionLabelPair *next;

} ExclusionLabelPair;
// 构成互斥的fact
struct _FtExclusion{
  
  BitVector *pos_exclusives;
  BitVector *neg_exclusives;
 
  DdNode** p_exlabel;
  DdNode** n_exlabel;
 
};/* // FtExclusion; */
// 构成互斥的effect
struct _EfExclusion{
  int set;
  BitVector *exclusives;
  BitVector *pos_exclusives;
  BitVector *neg_exclusives;
  DdNode** exlabel;// 存储effect和其他每个Effect的mutex的BDD。
  DdNode** p_exlabel;
  DdNode** n_exlabel;
 
} ;/* //EfExclusion; */
// 构成互斥的op
struct _OpExclusion{
 
  BitVector *exclusives;
  DdNode** exlabel;
 
} ;/* //OpExclusion; */


/************************
 * SEARCHING STRUCTURES *
 ************************/

/* the UBTree node structure
 *
 * ( see Technical Report 108, "A new Method to index and query Sets" )
 */
struct _MemoNode{

  int double_index;

  MemoNode_table *sons;
  
  int min_way;

  struct _MemoNode *next;

};

/* a candidate node in the wave front
 *
 * ( Fox / Long: "Fast implementation of the planning graph in STAN" )
 */
typedef struct _Candidate{

  FtArray fts;

  OpArray ops;

  struct _Candidate *father;
  int depth;

  struct _Candidate *prev; 
  struct _Candidate *next;

} Candidate;


/*
 *  -------------------------------- MAIN FN HEADERS ----------------------------
 */

void ipp_usage( void );

void print_official_result();


/*
 *  ----------------------------- GLOBAL VARIABLES ----------------------------
 */


/*******************
 * GENERAL HELPERS *
 *******************/

/* used to time the different stages of the planner
 */
extern struct tms gstart, gend;
extern float gtotal_time, gexcl_time;

/* the command line inputs
 */
extern struct _command_line gcmd_line;

/* simple help: store names of connectives
 */
extern const char *gconnectives[];

/* word size of the used machine
 */
extern const int gcword_size;

/* record memory consumption
 */
extern int gmemory, rifo_memory, ggraph_memory, gexcl_memory, gmemo_memory, gwave_memory;

/* default graph save name
 */
extern char gdef_save_name[MAX_LENGTH];


/***********
 * PARSING *
 ***********/

/* used for pddl parsing, flex only allows global variables
 */
extern int gbracket_count;
extern char *gproblem_name;

/* The current input line number
 */

extern int lineno;

/* The current input filename
 */
extern char *gact_filename;


/* The pddl domain name
 */
extern char *gdomain_name;

/* the types, as defined in the domain file
 */
extern TypedList *gparse_types;

/* the constants, as defined in domain file
 */
extern TypedList *gparse_constants;

/* the predicates and their arg types, as defined in the domain file
 */
extern TypedListList *gparse_predicates;

/* the objects, declared in the problem file
 */
extern TypedList *gparse_objects;

/* connection to instantiation ( except ops, goal, initial )
 */

/* all typed objects 
 */
extern FactList *gorig_constant_list;

/* the predicates and their types
 */
extern FactList *gpredicates_and_types;

/* type hierarchy (PDDL) 
 */
extern type_tree_list gglobal_type_tree_list;

/* helper for types parsing
 */
extern FactList *gtypes;

/* switching between AIPS-2000 Competion Style
   and original IPP format
 */
extern int gofficial_output_style;
extern int gnum_plan_ops;


/*****************
 * INSTANTIATING *
 *****************/

/* global arrays of constant names,
 *               type names (with their constants),
 *               predicate names,
 *               predicate aritys,
 *               defined types of predicate args
 */
extern String gconstants_table[MAX_CONSTANTS_TABLE];
extern int gconstants_table_size;
extern StringIntegers gtypes_table[MAX_TYPES_TABLE];
extern int gtype_size[MAX_TYPES_TABLE];
extern int gtypes_table_size;
extern String gpredicates_table[MAX_PREDICATES_TABLE];
extern int garity[MAX_PREDICATES_TABLE];
extern int gpredicates_args_type[MAX_PREDICATES_TABLE][MAX_ARITY];
extern int gpredicates_table_size;

/* helper in solving the Atomic Instantiation problem: 
 *                                    the implicit tuple tables
 *
 * one table size is the size of one implicit table
 * (there are 2^{arity(predicate)} such tables for each predicate)
 *
 * ( see Technical Report 122, "Handling of Inertia in a Planning System" )
 */
extern int_pointer gtuples[MAX_PREDICATES_TABLE];
extern int gone_table_size[MAX_PREDICATES_TABLE];

/* stores inertia - information: is any occurence of the predicate
 * added / deleted in the uninstantiated ops ?
 *
 * ( see TR 122 )
 */
extern int gis_added[MAX_PREDICATES_TABLE];
extern int gis_deleted[MAX_PREDICATES_TABLE];

/* store the final "relevant facts", see TR 122
 */
extern RelevantFact_pointer grelevant_facts[MAX_RELEVANT_FACTS];
extern int gnum_relevant_facts;

/* standard name for inferred types ( unary inertia, see TR 122 )
 */
extern char gnew_types_name[MAX_LENGTH];

/* standard name for GOAL-REACHED fact, as needed for disjunctive goals
 */
extern char ggoal_reached_name[MAX_LENGTH];


/*************************
 * BITMAP REPRESENTATION *
 *************************/

/* the bitvector length for relevant facts
 */
extern int gft_vector_length;

extern int gef_vector_length;

/* final representation of ops,
 *                         initial state,
 *                         goal state
 */
extern BitOperator *gbit_operators;
extern int gnum_bit_operators;
extern int gnum_cond_effects;
extern int gnum_cond_effects_pre;
extern int gnum_cond_effects_at[IPP_MAX_PLAN];
extern FactInfoPair *gbit_initial_state;// 初始状态正负命题的情况
extern FactInfoPair *gbit_goal_state;// 目标状态正负命题的情况


/**********************
 * RIFO               *
 **********************/

/* which metastrategy is to be used to remove irrelevants before 
   building the graph */
extern int rifo_meta;
extern int actions_threshold;
extern int objects_threshold;

/* used to make function build_graph_evolution_step() start 
   with a new plan when replanning due to rifo failure */
extern int new_plan;

/**********************
 * BUILDING THE GRAPH *
 **********************/

/* 
 * dis is da graph!
 *
 * will later be allocated as an array of pointers to fact nodes
 */
extern FtNode_pointer *gft_table;

/* points to the first element of a global operator (ft) -node-list;
 */ 
extern OpNode *gall_ops_pointer;
extern OpNode *gprev_level_ops_pointer;
extern FtNode *gall_fts_pointer;
extern FtNode *gprev_level_fts_pointer;
extern EfNode *gall_efs_pointer;
extern EfNode *gprev_level_efs_pointer;

extern OpNode *gops_with_unactivated_effects_pointer;

/* current mutexes: exclusives speedup
 */
extern OpPair *gop_mutex_pairs;
extern EfPair *gef_mutex_pairs;
extern FtPair *gft_mutex_pairs;

/* information about current state of graph, needed for level off test
 */
extern unsigned int gfacts_count, gexclusions_count;
extern unsigned int gops_count, gops_exclusions_count;
extern unsigned int gefs_count, gefs_exclusions_count;

/* for comparison: mutex number between positives
 */
extern int gprint_ftnum, gprint_exnum;

/* the pres_nt facts ordered by levels as bitvectors
 */
extern BitVector_pointer gpos_facts_vector_at[IPP_MAX_PLAN];
extern BitVector_pointer gneg_facts_vector_at[IPP_MAX_PLAN];

/* the bitvector length for ops at each graph level
 */
extern unsigned int gop_vector_length_at[IPP_MAX_PLAN];
extern unsigned int gef_vector_length_at[IPP_MAX_PLAN];

/* is TRUE iff graph has levelled off.
 */
extern int gsame_as_prev_flag;

/* store the time step at which graph has levelled off.
 */
extern int gfirst_full_time;

extern int gwf_found_plan;


/*************
 * SEARCHING *
 *************/

/* current state of search: goals at levels,
 *                          same as bitvectors,
 *                          selectde ops
 */
extern FtArray *ggoals_at;
extern int *gnum_goals_at;
extern BitVector_pointer gpos_goals_vector_at[IPP_MAX_PLAN];
extern BitVector_pointer gneg_goals_vector_at[IPP_MAX_PLAN];
extern OpArray *gops_at;
extern int *gnum_ops_at;

/* //extern OpArray gplan_ops; */
extern int gnum_plan_ops;

/* the wave front, currently implemented as a doubly
 * connected linear list
 */
extern Candidate *gwave_front_head;
extern Candidate *gwave_front_tail;


/* to avoid memory leak: keep a pointer on the list of
 * candidates that have been expanded and removed from
 * the wave front already
 */
extern Candidate *gwave_front_trash;

/* search space information: actions, noops tried,
 *                           memoization (UBTree) hits
 */
extern int gnum_of_actions_tried, gnum_of_noops_tried;
extern int gsimple_hits, gpartial_hits, gsubset_hits;

/* only for communication from wave front to save graph:
 * to find out, which ops are used in the plan, we need
 * to search the list of candidates (connected by ->father)
 * that starts with the one Candidate that finally led to 
 * a plan.
 *
 * not really good implementation style, but who does really
 * care about this ?
 */
extern Candidate *gplan_start;

#endif 

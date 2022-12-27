
/*********************************************************************
 * File: build_graph.c
 * Description: main routines for building the graph
 *
 * Author: Joerg Hoffmann 1998
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
/*********************************************************************
 *
 * NOTE: the commentaries in this file, sparse as they are, are all
 *       in German, cause these are thoughts that I had while working
 *       on some really tedious parts of the code (potential effects...)
 *
 *       If you have problems understanding the code (like I do when I have
 *       a look at it now), contact me at:
 *
 *       hoffmann@informatik.uni-freiburg.de
 *
 *       and I'll be happy to answer your questions, if I can...
 *
 **********************************************************************/








#include "ipp.h"

#include "output.h"
#include "utilities.h"
#include "memory.h"
#include <ext/hash_set>
#include <ext/hash_map>

#include "build_graph.h"
#include "exclusions.h"
#include "graph_wrapper.h"
#include "globals.h"
#include "lug.h"
#include <math.h>
#include <sys/times.h>
#include "cudd/dd.h"
#include "correlation.h"

/*****************
 * INSTANTIATING *
 *****************/





extern int MUTEX_SCHEME;
extern std::list<std::list<LabelledElement*>*>** actWorldCost;
extern std::list<std::list<LabelledElement*>*>** factWorldCost;
extern std::list<std::list<LabelledElement*>*>** effectWorldCost;
int reached_goals = FALSE;
int proven_goals = FALSE;

/* global arrays of constant names,
 *               type names (with their constants),
 *               predicate names,
 *               predicate aritys,
 *               defined types of predicate args
 */
String gconstants_table[MAX_CONSTANTS_TABLE];
int gconstants_table_size = 0;
StringIntegers gtypes_table[MAX_TYPES_TABLE];
int gtype_size[MAX_TYPES_TABLE];
int gtypes_table_size = 0;
String gpredicates_table[MAX_PREDICATES_TABLE];
int garity[MAX_PREDICATES_TABLE];
int gpredicates_args_type[MAX_PREDICATES_TABLE][MAX_ARITY];
int gpredicates_table_size = 0;


/* the parsed input structures, translated into CodeNodes
 */
//CodeNode *gcode_initial_state = NULL;
//CodeNode *gcode_goal_state = NULL;
//CodeOperator *gcode_operators = NULL;
int gnum_cond_effects = 0;
int gnum_cond_effects_pre = 0;

int gnum_cond_effects_at[IPP_MAX_PLAN];


/* helper in solving the Atomic Instantiation problem:
 *                                    the implicit tuple tables
 *
 * ( see Technical Report 122, "Handling of Inertia in a Planning System" )
 */
int_pointer gtuples[MAX_PREDICATES_TABLE];
int gone_table_size[MAX_PREDICATES_TABLE];


/* stores inertia - information: is any occurence of the predicate
 * added / deleted in the uninstantiated ops ?
 *
 * ( see TR 122 )
 */


/* store the final "relevant facts", see TR 122
 */
RelevantFact_pointer grelevant_facts[MAX_RELEVANT_FACTS];
int gnum_relevant_facts = 0;
//CodeOperator *ginst_code_operators = NULL;/* helper: first get all inst.ops */


/* standard name for inferred types ( unary inertia, see TR 122 )
 */
char gnew_types_name[MAX_LENGTH] = "INFERRED TYPE";


/* standard name for GOAL-REACHED fact, as needed for disjunctive goals
 */
char ggoal_reached_name[MAX_LENGTH] = "GOAL-REACHED";

/*************
 * SEARCHING *
 *************/








/* current state of search: goals at levels,
 *                          same as bitvectors,
 *                          selected ops
 */
FtArray *ggoals_at;
int *gnum_goals_at;
OpArray *gops_at;
int *gnum_ops_at;

OpArray gplan_ops;
int gnum_plan_ops;

/* the wave front, currently implemented as a doubly
 * connected linear list
 */
Candidate *gwave_front_head;
Candidate *gwave_front_tail;


/* to avoid memory leak: keep a pointer on the list of
 * candidates that have been expanded and removed from
 * the wave front already
 */
Candidate *gwave_front_trash;


/* search space information: actions, noops tried,
 *                           memoization (UBTree) hits
 */
int gnum_of_actions_tried = 0, gnum_of_noops_tried = 0;
int gsimple_hits = 0, gpartial_hits = 0, gsubset_hits = 0;


/* only for communication from wave front to save graph:
 * to find out, which ops are used in the plan, we need
 * to search the list of candidates (connected by ->father)
 * that starts with the one Candidate that finally led to
 * a plan.
 *
 * not really good implementation style, but who does really
 * care about this ?
 */
Candidate *gplan_start;



/*******************
 * GENERAL HELPERS *
 *******************/








/* used to time the different stages of the planner
 */
struct tms gstart, gend, lstart, lend;
float gtotal_time = 0, gexcl_time = 0;

/* the command line inputs
 */
struct _command_line gcmd_line;


/* simple help: store names of connectives
 */
const char *gconnectives[] = {"ATOM", "NOT", "AND", "OR", "ALL", "EX", "WHEN",
		"TRU", "FAL", "EMPTY", "DUMMY", "NEUTRAL"};


/* word size of the used machine
 */
const int gcword_size = sizeof(int) * 8;




/* default graph save name
 */
char gdef_save_name[MAX_LENGTH] = "graph";




//IPP vars from main.c in ipp

int gmemory = 0, rifo_memory = 0, ggraph_memory = 0, gexcl_memory = 0;
int gmemo_memory = 0, gwave_memory = 0;
int new_plan = FALSE;
FtPair *gft_mutex_pairs = NULL;
OpNode *gops_with_unactivated_effects_pointer = NULL;
//Candidate *gwave_front_head;
//Candidate *gwave_front_tail;
//Candidate *gwave_front_trash;
FtNode_pointer *gft_table;
OpNode *gprev_level_ops_pointer = NULL;
EfNode *gprev_level_efs_pointer = NULL;
FtNode *gprev_level_fts_pointer = NULL;
int gsame_as_prev_flag = FALSE;
int gfirst_full_time = 0;
//Candidate *gplan_start;

unsigned int gops_count = 0, gops_exclusions_count = 0;
unsigned int gefs_count = 0, gefs_exclusions_count = 0;
FactInfoPair *gbit_goal_state = NULL;
FactInfoPair *my_gbit_goal_state = NULL;
OpNode *gall_ops_pointer = NULL;//End IPP vars
EfNode *gall_efs_pointer = NULL;//End IPP vars
OpPair *gop_mutex_pairs = NULL;
EfPair *gef_mutex_pairs = NULL;
unsigned int gop_vector_length_at[IPP_MAX_PLAN];
unsigned int gef_vector_length_at[IPP_MAX_PLAN];
int gprint_ftnum = 0, gprint_exnum = 0;
FtNode *gall_fts_pointer = NULL;

int gft_vector_length;
int gef_vector_length;

BitVector_pointer gpos_facts_vector_at[IPP_MAX_PLAN];
BitVector_pointer gneg_facts_vector_at[IPP_MAX_PLAN];
unsigned int gfacts_count = 0, gexclusions_count = 0;
FactInfoPair *gbit_initial_state = NULL;
BitOperator *gbit_operators = NULL;// 全局op编码, initLUG中创建
BitOperator *my_gbit_operators = NULL;
BitOperator *used_gbit_operators = NULL;// 使用的op编码
//struct _command_line gcmd_line;
//int garity[MAX_PREDICATES_TABLE];
/***********
 * PARSING *
 ***********/







/* used for pddl parsing, flex only allows global variables
 */
int gbracket_count = 0;
char *gproblem_name;

/* The current input line number
 */

//int lineno = 1;

/* The current input filename
 */
char *gact_filename;


/* The pddl domain name
 */
char *gdomain_name = NULL;

/* loaded, uninstantiated operators
 */
//PlOperator *gloaded_ops = NULL;

/* stores initials as fact_list
 */
//PlNode *gorig_initial_facts = NULL;

/* not yet preprocessed goal facts
 */
//PlNode *gorig_goal_facts = NULL;

/* axioms as in UCPOP before being changed to ops
 */
//PlOperator *gloaded_axioms = NULL;



/* the types, as defined in the domain file
 */
TypedList *gparse_types = NULL;

/* the constants, as defined in domain file
 */
TypedList *gparse_constants = NULL;

/* the predicates and their arg types, as defined in the domain file
 */
TypedListList *gparse_predicates = NULL;
//unsigned int gef_vector_length_at[IPP_MAX_PLAN];

/* the objects, declared in the problem file
 */
TypedList *gparse_objects = NULL;


/* connection to instantiation ( except ops, goal, initial )
 */

/* all typed objects
 */
FactList *gorig_constant_list = NULL;

/* the predicates and their types
 */
FactList *gpredicates_and_types = NULL;




/* type hierarchy (PDDL)
 */
type_tree_list gglobal_type_tree_list;

/* helper for types parsing
 */
FactList *gtypes;


/* switching between AIPS-2000 Competion Style
   and original IPP format
 */
int gofficial_output_style = FALSE;
//int gnum_plan_ops;

int gis_added[MAX_PREDICATES_TABLE];
int gis_deleted[MAX_PREDICATES_TABLE];

/*************************
 * BITMAP REPRESENTATION *
 *************************/







/* the bitvector length for relevant facts
 */
//int gft_vector_length;




int gnum_bit_operators = 0;

//NAD
;








void getNDLabels(OpNode* i1, DdNode* allEffForm, int time);
int supportsAdder (Consequent *, EfNode *, int);

















/**********************************
 * INITIAL GRAPH BUILDING PROCESS *
 **********************************/



int stop_building_graph(int time) {
	// std::cout << "STOP?" << std::endl;
	//  if(!COMPUTE_LABELS)
	//     return FALSE;

  if(RBPF_LUG && time == max_horizon)
    return true;
  //  else if(RBPF_LUG)
  //return false;

	if(ALLOW_LEVEL_OFF) {// LUG is enter
		if(COMPUTE_LABELS) {// label存在则需要相同
			//  printf("STOP1\n");
		  
		  if(( gsame_as_prev_flag && labels_same(time) &&
		       (RP_EFFECT_SELECTION == RP_E_S_COVERAGE || costLevelOff(time))))// 这里需要考虑下后续Lug是否导致横为0
		    LEVELS_PAST_LEVEL_OFF++;
		  else
		    LEVELS_PAST_LEVEL_OFF=0;

		  return LEVELS_PAST_LEVEL_OFF>2;
		  
			/*  && goals_proven(time) */
			//&& (MUTEX_SCHEME!=MS_CROSS || goal_non_mutex(time));
		}// 没有label,且为CORRPR
		else if(!COMPUTE_LABELS && HEURISTYPE==CORRRP){
			return stopCORRGraph(time);
		}// conformant情况, 没有label
		else{
			//      printf("STOP2\n");
			// 返回标记，是否和上一层相同。
			return gsame_as_prev_flag; //&&(MUTEX_SCHEME==MS_REGULAR || goal_non_mutex(time));
		}
	}
	else {// MG and SG etc enter
		if(COMPUTE_LABELS) {
			//    printf("STOP3\n");

			int goals_reached = goals_proven(time);
			proven_goals = goals_reached;

			int levelled_off = 0;
			if(!goals_reached)
				levelled_off = labels_same(time);

			int costs_level_off = 0;
			if(RP_EFFECT_SELECTION != RP_E_S_COVERAGE)
				costs_level_off = costLevelOff(time);


			//      std::cout << "++++++STOP AT TIME: "<< time  << "+++++++" << std::endl
			// 		<< "Goals Reached: " << goals_reached << std::endl
			// 		<< "Levelled Off: " << levelled_off << std::endl
			// 		<< "Costs Level Off: " << costs_level_off << std::endl
			// 		<< "++++++++++++++++++++++++++++" <<std::endl;



			return (RP_EFFECT_SELECTION == RP_E_S_COVERAGE &&
					(goals_reached || (levelled_off && ALLOW_LEVEL_OFF) || (!goals_reached && levelled_off))) ||
					(RP_EFFECT_SELECTION != RP_E_S_COVERAGE &&
							(goals_reached || (levelled_off && ALLOW_LEVEL_OFF)) &&
							costs_level_off);
			// && (MUTEX_SCHEME==MS_REGULAR || goal_non_mutex(time));
		}
		else if(!COMPUTE_LABELS && HEURISTYPE==CORRRP){
			return stopCORRGraph(time) && reached_goals;
		}
		// MG and SG enter
		else{
			//printf("STOP4\n");
			return gsame_as_prev_flag;
			// && (MUTEX_SCHEME==MS_REGULAR || goal_non_mutex(time));
		}
	}
}






extern void print_BitVector(BitVector*, int);
void build_graph_evolution_step();
int  build_graph( int *min_time,
		int num_facts,
		int ALLOW_LEVEL_OFF,
		int MUTEX_SCHEMES )

{

	int time = 0, j;
	int  first = TRUE;
	Integers *i;
	FtNode *ft;
	DdNode* tmp;
	reached_goals = FALSE;
	proven_goals = FALSE;
	MUTEX_SCHEME=MUTEX_SCHEMES;// config the mutex

	// printMemInfo();

	// printf("MUX = %d, LEVOFF = %d\n", MUTEX_SCHEME, ALLOW_LEVEL_OFF);

	// std::cout <<"\n\nenter build graph\n" << std::flush;
	// printBDD(b_initial_state);
	// clear the layers
	for(j=0;j<IPP_MAX_PLAN;j++){
		gpos_facts_vector_at[j]=NULL;
		gneg_facts_vector_at[j]=NULL;
		gnum_cond_effects_at[j]=0;
	}
	// printf("gnum_relevant_facts = %d \n", num_facts);
	gnum_relevant_facts = num_facts;
	// printf("Num releveant facts = %d \n" ,gnum_relevant_facts );
	// 创建正负FactNode的table
	gft_table = ( FtNode_pointer * ) calloc( gnum_relevant_facts * 2, sizeof( FtNode_pointer ) );

	CHECK_PTR(gft_table);
#ifdef MEMORY_INFOi
	gmemory += gnum_relevant_facts * 2 * sizeof( FtNode_pointer );
#endif
	// 将数组的指针置空
	for ( j = 0; j<2*gnum_relevant_facts; j++ ) gft_table[j] = NULL;
	// 创建目标pos,neg at initial layer
	gpos_facts_vector_at[0] = new_bit_vector( gft_vector_length );
	gneg_facts_vector_at[0] = new_bit_vector( gft_vector_length );
	/**
	 * 创建初始状态的FactNode
	 * 这里所有俄positive和negative可以indices拿到index
	 * 该index决定了他在gpos_facts_vector_at和gneg_facts_vector_at的情况，从而可以拿到min fact的值
	 */
	// std::cout << "positive:\n";
	for ( i = gbit_initial_state->positive->indices; i; i = i->next ) {
		if(COMPUTE_LABELS){// 如果需要label
			tmp = get_init_labels(i->index, TRUE);
			Cudd_Ref(tmp);
			ft = new_ft_node( 0, i->index, TRUE, FALSE, new_Label(tmp, 0));// 创建Fact Node同时配置该层的level info
			ft->info_at[0]->updated =1;// 设置当前层fact更新
			Cudd_RecursiveDeref(manager, tmp);
		}
		else
			ft = new_ft_node( 0, i->index, TRUE, FALSE, NULL);// 直接创建FactNode，不用配置Label

		gft_table[i->index] = ft;// factNode存储到table中
		ft->next = gall_fts_pointer;// factNode前插法加入全局FactNode链表中
		gall_fts_pointer = ft;
		(gpos_facts_vector_at[0])[ft->uid_block] |= ft->uid_mask;// 该层记录该命题
		// dynamic_atoms[i->index]->print(std::cout, my_problem->domain().predicates(), my_problem->domain().functions(), my_problem->terms());
		// std::cout << std::endl;
	}
	// std::cout << "negative:\n";
	for ( i = gbit_initial_state->negative->indices; i; i = i->next ) {
		if(COMPUTE_LABELS){
			tmp = get_init_labels(i->index, FALSE);
			Cudd_Ref(tmp);
			ft = new_ft_node( 0, i->index, FALSE, FALSE, new_Label(tmp, 0));
			ft->info_at[0]->updated =1;
			Cudd_RecursiveDeref(manager, tmp);
		}
		else
			ft = new_ft_node( 0, i->index, FALSE, FALSE, NULL);

		gft_table[NEG_ADR( i->index )] = ft;
		ft->next = gall_fts_pointer;
		gall_fts_pointer = ft;
		(gneg_facts_vector_at[0])[ft->uid_block] |= ft->uid_mask;
		// dynamic_atoms[i->index]->print(std::cout, my_problem->domain().predicates(), my_problem->domain().functions(), my_problem->terms());
		// std::cout << std::endl;
	}

	free_fact_info_pair( gbit_initial_state );
	// printf("The initial fact vector is:\n");
	// print_BitVector(gpos_facts_vector_at[0], gft_vector_length);
	// print_BitVector( gneg_facts_vector_at[0], gft_vector_length); 
	// done initial fact layer construct.

	if(HEURISTYPE == CORRRP){// this option does not call in our conformant planning
		instantiateCorrelation();
		initialStateProbability(initialBDD, b_initial_state);
		if(USE_CORRELATION)
			initialStateCorrelation(initialBDD, b_initial_state);
	}

	// printf("SET INIT IN GRAPH\n");  fflush(stdout);
	if ( gcmd_line.display_info ) {
		fprintf( OUT, "time: %3d, %5d facts and %7d exclusive pairs (%5d, %7d positives)\n",
				time, gfacts_count, gexclusions_count, gprint_ftnum, gprint_exnum );
	}
	// printf("time= %d, min_time = %d\n", time, min_time);
	// 下面这块代码没有进入，min_time = time = 0
	for (; time < *min_time; time++)
	{
		printf("call in to min_time loop\n");
		assert(0);
		if ( first ) {// 第一次执行
			reached_goals = are_there_non_exclusive( time, gbit_goal_state->positive, gbit_goal_state->negative );

			if ( reached_goals) {
				//if ( gcmd_line.display_info ) {
				//fprintf( OUT, "\ngoals first reachable in %d time steps\n\n", time);
				//printf("\ngoals first reachable in %d time steps\n\n", time);
				first = FALSE;
				//}
			}
		}
		// 满足停止条件
		// 1.最大层数, 2. stop_building, 3. 不计算label,falg无变化
		if (time == IPP_MAX_PLAN-1 ||
				(stop_building_graph(time))  ||
				(!COMPUTE_LABELS && gsame_as_prev_flag && HEURISTYPE != CORRRP) /*&& (!COMPUTE_LABELS || goals_proven(time)) */)
				break;

		build_graph_evolution_step();
	}

	for( ; time < IPP_MAX_PLAN; time++ ) {
		// std::cout << "time = " << time << std::endl;
		reached_goals = are_there_non_exclusive( time, gbit_goal_state->positive, gbit_goal_state->negative );// 当前目标当当前层满足，同时没有mutex

		if ( reached_goals ) {
			if ( gcmd_line.display_info && time > 0 && first ) {
				fprintf( OUT, "\ngoals first reachable in %d time steps\n\n", time);
				printf( "\ngoals first reachable in %d time steps\n\n", time);
			}
		}
		///else
		//   printf("DUHHHH\n");
		// 检测是否需要停止
		if (time == IPP_MAX_PLAN-1 ||
				(!COMPUTE_LABELS && gsame_as_prev_flag && HEURISTYPE != CORRRP)||
				(stop_building_graph(time)))
				break;
		// 继续迭代生成图
		build_graph_evolution_step();
		//   un_update_actions_effects_facts(time);

	}

	if(time < IPP_MAX_PLAN-1)
		reached_goals = TRUE;

	*min_time = time;// 记录MG中的min_time，最小需要level
	// clear the update for next call
	un_update_actions_effects_facts(time);


	/**
	 * momo007 2022.09.30
	 * Following code contain bug.
	 * But now use this one is ok.
	 */
	if(COMPUTE_LABELS){
		//    if(time == 0)
		//std::cout << "did reach goals?"<<std::endl;
		//  std::cout << "REACHED GOALS " << proven_goals << " " << reached_goals <<std::endl;
		// return proven_goals;//goals_proven(time);
		return reached_goals;
	}
	else{
		return reached_goals;
	}
}



//currently works only for pflug
int add_samples(DdNode* newSamples){
	int time=0;

	//    std::cout << "Adding samples" << std::endl;
	//    printBDD(newSamples);

	//add labels for new samples to init layer
	if(updateInitLayer(newSamples)){
		std::cout << "---------------ADD-------------" <<std::endl;

		//  std::cout << "updated init"<<std::endl;

		for(; (time < IPP_MAX_PLAN &&
				!stop_building_graph(time)); time++ ) {
			std::cout << "update at time " << time << std::endl;

			update_structure(time);
			update_actions_effects_facts(time);
			mark_operator_labels(time);
			mark_effect_lables(time);
			mark_fact_labels(time+1);

			//     update_effects(time);
			//     update_facts(time+1);

		}
		un_update_actions_effects_facts(time);
	}
	else{
		std::cout << "**************NO ADD************" <<std::endl;
		time = graph_levels;
		//   exit(0);
	}

	return time;
}

void update_actions_effects_facts(int time){
	//  std::cout << "propagating updates at time: " << time << std::endl;
	for( OpNode *i1 = gall_ops_pointer; i1; i1=i1->next ){
		//if a precondition is updated, then action is updated
		if(i1->info_at[time]){
			//std::cout << "op"<<std::endl;
			if(i1->info_at[time]->updated == 0){
				for(FtEdge *ft = i1->preconds; ft; ft=ft->next){
					if(ft->ft->info_at[time] &&
							ft->ft->info_at[time]->updated){
						i1->info_at[time]->updated =1;
						if(i1->unconditional && i1->unconditional->info_at[time])
							i1->unconditional->info_at[time]->updated=1;
						break;
					}
				}
				if(!i1->preconds){
					i1->info_at[time]->updated =1;
					if(i1->unconditional && i1->unconditional->info_at[time])
						i1->unconditional->info_at[time]->updated=1;
				}

			}
			//std::cout << "conds"<<std::endl;
			//if effect condition is updated, then effect is updated
			for(EfNode *ef = i1->conditionals; ef; ef=ef->next){
				for(FtEdge *ft = ef->conditions;
						(ft && ef->info_at[time]); ft=ft->next){
					if(ft->ft->info_at[time] &&
							ft->ft->info_at[time]->updated){
						ef->info_at[time]->updated =1;
						break;
					}
				}
				if(!ef->conditions && ef->info_at[time])
					ef->info_at[time]->updated =1;
			}

		}
	}
	//std::cout << "facts"<<std::endl;
	//if a facts adder is updated, then it is
	for(FtNode *ftn1 = gall_fts_pointer; ftn1; ftn1 =ftn1->next){
		if(ftn1->info_at[time+1] &&
				ftn1->info_at[time+1]->updated == 0){
			for(EfEdge *efe = ftn1->adders; efe; efe=efe->next){
				if(efe->ef->info_at[time] && efe->ef->info_at[time]->updated){
					ftn1->info_at[time+1]->updated =1;
					break;
				}
			}
		}
	}
	//  std::cout << "done prop"<<std::endl;
}


void un_update_actions_effects_facts(int t){
	// std::cout << "undo updates at time: " << time << std::endl;
	for(int time = 0; t >= time; time++){
		for( OpNode *i1 = gall_ops_pointer; i1; i1=i1->next ){

			if(i1->info_at[time]){
				i1->info_at[time]->updated = 0;
				if(i1->unconditional && i1->unconditional->info_at[time])
					i1->unconditional->info_at[time]->updated=0;

			}

			for(EfNode *ef = i1->conditionals; ef; ef=ef->next){
				if(ef->info_at[time])
					ef->info_at[time]->updated =0;
			}
		}

		for(FtNode *ftn1 = gall_fts_pointer; ftn1; ftn1 =ftn1->next){
			if(ftn1->info_at[time])
				ftn1->info_at[time]->updated = 0;
		}
	}
}

void make_noop(FtNode* ft, int time){
	FtNode *f1;
	BitVector *a, *b;
	OpNode *noop;
	EfNode *tmp;
	Effect* tmp_eff;

	char* name;

	tmp_eff = new_Effect(num_alt_effs + ft->index + abs(ft->positive-1)*num_alt_facts);
	tmp_eff->outcome->insert(0);
	(*tmp_eff->probability)[0] = 1.0;
	a = new_bit_vector( gft_vector_length );
	b = new_bit_vector( gft_vector_length );
	if ( ft->positive ) {
		a[ft->uid_block] |= ft->uid_mask;
		tmp_eff->cons->p_effects->vector[ft->uid_block] |= ft->uid_mask;
		tmp_eff->cons->p_effects->indices = new_integers(ft->index);
		tmp_eff->ant->p_conds->vector[ft->uid_block] |= ft->uid_mask;
		tmp_eff->ant->p_conds->indices = new_integers(ft->index);

		tmp_eff->cons->b = Cudd_bddIthVar(manager, 2*ft->index );
		Cudd_Ref(tmp_eff->cons->b);
		tmp_eff->ant->b = Cudd_bddIthVar(manager, 2*ft->index );
		Cudd_Ref(tmp_eff->ant->b);
	} else {
		b[ft->uid_block] |= ft->uid_mask;
		tmp_eff->cons->n_effects->vector[ft->uid_block] |= ft->uid_mask;
		tmp_eff->cons->n_effects->indices = new_integers(ft->index);
		tmp_eff->ant->n_conds->vector[ft->uid_block] |= ft->uid_mask;
		tmp_eff->ant->n_conds->indices = new_integers(ft->index);
		tmp_eff->cons->b = Cudd_Not(Cudd_bddIthVar(manager, 2*ft->index ));
		Cudd_Ref(tmp_eff->cons->b);
		tmp_eff->ant->b = Cudd_Not(Cudd_bddIthVar(manager, 2*ft->index ));
		Cudd_Ref(tmp_eff->ant->b);
	}


	/*        printf("About to make noop an op node\n"); */
	noop = new_op_node( time,name /* NULL */, TRUE,
			a, b, tmp_eff->ant->b,
			num_alt_acts + ft->index +
			abs(ft->positive-1)*num_alt_facts );
	noop->info_at[time]->updated = 1;
	noop->action = NULL;
	free_bit_vector(a);
	free_bit_vector(b);
	insert_ft_edge( &(noop->preconds), ft );


	/*  tmp_cons->p_effects->vector = a; */
	/*     tmp_cons->n_effects->vector = b; */

	/*        printf("about to make noop an ef node\n"); */
	tmp = new_ef_node( time, noop, tmp_eff, num_alt_effs + ft->index + abs(ft->positive-1)*num_alt_facts/* a, b */ );

	tmp->effect->probability_sum = 1.0;

	free_Effect(tmp_eff);
	if(COMPUTE_LABELS){


		/*   if(ft->info_at[time]->label) { */
		/*          printf("A\n"); */
		/* 	 //  ft->info_at[time+1]->label = ft->info_at[time]->label; */
		/*            printf("A1\n"); */
		/*    } */

		/*  if(ft->info_at[time]->label)  */
		/*        tmp->info_at[time]->label = ft->info_at[time]->label; */
		/*      else */
		/*        tmp->info_at[time]->label = NULL; */
		tmp->info_at[time]->label = new_Label(ft->info_at[time]->label->label, 0);
	}
	if(MUTEX_SCHEME!=MS_NONE){
		for(int r = 0; r < gft_vector_length; r++){
			tmp->info_at[time]->exclusives->pos_exclusives[r] |=
					ft->info_at[time]->exclusives->pos_exclusives[r];
			tmp->info_at[time]->exclusives->neg_exclusives[r] |=
					ft->info_at[time]->exclusives->neg_exclusives[r];
		}
		for(int r = 0; r < num_alt_facts; r++){
			add_worlds_to_mutex(tmp->info_at[time]->exclusives->p_exlabel[r],
					ft->info_at[time]->exclusives->p_exlabel[r]);
			add_worlds_to_mutex(tmp->info_at[time]->exclusives->n_exlabel[r],
					ft->info_at[time]->exclusives->n_exlabel[r]);
		}
	}
	// }
	tmp->all_next = gall_efs_pointer;
	gall_efs_pointer = tmp;
	noop->unconditional = tmp;
	noop->next = gall_ops_pointer;
	gall_ops_pointer = noop;
	noop->is_noop = TRUE;
	//need to assign adder
	insert_ef_edge( &(ft->adders), tmp );
	ft->noop = noop;



}

void update_structure(int time){
	FtNode *ft;
	FtEdge* fte;
	EfNode* ef, *j;
	OpNode *op, *op1, *prev_op, *tmp_op, *i;
	BitOperator *o, *prev, *tmp;
	EfNode *ef_node;
	FtNode *ft_node;
	Consequent* conseq;
	Integers *is;

	//  std::cout << "update acts at time " << time << std::endl;

	//persist facts
	if(!gpos_facts_vector_at[time+1])
		gpos_facts_vector_at[time+1] = new_bit_vector( gft_vector_length );
	if(!gneg_facts_vector_at[time+1])
		gneg_facts_vector_at[time+1] = new_bit_vector( gft_vector_length );

	for(int i = 0; i < gft_vector_length; i++){
		(gpos_facts_vector_at[time+1])[i] |= (gpos_facts_vector_at[time])[i];
		(gneg_facts_vector_at[time+1])[i] |= (gneg_facts_vector_at[time])[i];

	}

	//persist ops and effects already in graph
	for ( i = gall_ops_pointer; (i && time > 0); i = i->next ) {

		if(i->info_at[time-1] && i->info_at[time-1]->updated
		){

			if(!i->info_at[time]){
				//	std::cout << "new op:" << op->name <<std::endl;
				i->info_at[time] = new_op_level_info();
				i->info_at[time]->updated = 1;
			}

			if(i->unconditional &&
					i->unconditional->info_at[time-1] &&
					i->unconditional->info_at[time-1]->updated &&
					!i->unconditional->info_at[time]){

				i->unconditional->info_at[time] = new_ef_level_info(time);
				i->unconditional->info_at[time]->updated = 1;
			}

			for ( j = i->conditionals; j; j = j->next ) {
				if(j->info_at[time-1] &&
						j->info_at[time-1]->updated &&
						!j->info_at[time]){

					j->info_at[time] = new_ef_level_info(time);
					j->info_at[time]->updated = 1;
					// 	  if ( j->info_at[time-1] &&
					// 	       j->info_at[time-1]->is_dummy &&
					// 	       j->info_at[time] ) {
					// 	    /* effekte bleiben erstmal dummy...
					// 	     * spaeter dann nachschauen, ob sie doch eingezogen werden koennen.
					// 	     *
					// 	     * ( kann sein, dass effekt erst drei runden spaeter nicht-dummy wird )
					// 	     */
					// 	    j->info_at[time]->is_dummy = TRUE;
					// 	  }
				}
			}
		}
	}

	// std::cout << "update facts" <<std::endl;

	//update ft level info's
	for (int k=0;(k<2*gnum_relevant_facts); k++ ) {

		//do not persist nodes that do not exist
		if ( (ft_node = gft_table[k]) == NULL  ||
				!ft_node->info_at[time] ||
				!ft_node->info_at[time]->updated ||
				(ft_node->info_at[time] &&
						ft_node->info_at[time]->label->label == Cudd_ReadLogicZero(manager)))
			continue;

		//    std::cout << "persisting: *********** " << std::endl;
		///    printFact(ft_node->index);

		if(!ft_node->info_at[time+1]){ //don't over-write
			ft_node->info_at[time+1] = new_ft_level_info( ft_node );
		}
		ft_node->info_at[time+1]->updated = 1;

		if(!ft_node->noop){
			make_noop(ft_node, time);
			///      std::cout << "made noop" << std::endl;
		}

		//insert noop level infos
		if(ft_node->noop && !ft_node->noop->info_at[time]){
			ft_node->noop->info_at[time] = new_op_level_info();
			ft_node->noop->info_at[time]->updated = 1;
			//std::cout << "updated op " << std::endl;
		}
		if(ft_node->noop && ft_node->noop->unconditional && !ft_node->noop->unconditional->info_at[time]){

			ft_node->noop->unconditional->info_at[time] = new_ef_level_info(time);
			ft_node->noop->unconditional->info_at[time]->updated = 1;
			//std::cout << "updated uncond " << std::endl;
		}
	}

	//  std::cout << "persisted ops" <<std::endl;

	//insert new operators
	//for all new ft nodes, add noops
	gprev_level_ops_pointer = gall_ops_pointer;
	gprev_level_efs_pointer = gall_efs_pointer;
	apply_noops(time);
	gprev_level_fts_pointer = gall_fts_pointer;

	// std::cout << "did noops"<<std::endl;

	//Insert all new operators
	//1. that come in later
	//2. that have not come in yet
	//For 1,2 - insert all effects
	// a. that come in later
	// b. that have not come in yet

	//insert new effects, for existing operators
	//effect is not in graph at all, or at a later level.
	//*********************************************
	//first we do those that are in the graph at later levels

	for(op = gall_ops_pointer; op; op=op->next){
		//(1) insert ops that are already in graph,
		//but not at this level.  It is in graph somewhere if
		//it is in this list
		int update_op = 0;

		if(!op->info_at[time]){
			//std::cout << "try op " << op->name <<std::endl;
			for(fte = op->preconds; fte; fte=fte->next){
				//std::cout << "try pre" <<std::endl;
				if(fte->ft &&
						fte->ft->info_at[time] &&
						fte->ft->info_at[time]->updated
				){
					//std::cout << "update" <<std::endl;
					update_op = 1;
				}
				else if(!fte->ft || !fte->ft->info_at[time]){
					//std::cout << "fail"<<std::endl;
					update_op = 0;
					break;
				}
			}
			if(update_op){
				//std::cout << "<-------- updated op " << op->name
				//    <<" ------->" << std::endl;
				op->info_at[time] = new_op_level_info();
				op->info_at[time]->updated = 1;
			}
		}

		//std::cout << "did op"<<std::endl << std::endl;

		//the unconditional effect should already
		//be in graph if op is, so do conds
		//These are effects of type 1a
		for(ef = op->conditionals; ef; ef=ef->next){
			//      std::cout << "try cond"<<std::endl;
			if(!ef->info_at[time] && //ef->first_occurence > time &&
					op->info_at[time]){
				//std::cout << "new cond"<<std::endl;
				//not in graph yet
				//it can go in now if its preconds are present,
				//they should only be present if they newly came in
				BitVector *cond_pos_exclusives, *cond_neg_exclusives = NULL;
				//ExclusionLabelPair
				DdNode**p_exlabel, **n_exlabel = NULL;
				if(get_them_non_exclusive( time,
						ef->effect->ant->p_conds,
						ef->effect->ant->n_conds,
						&cond_pos_exclusives,
						&cond_neg_exclusives,
						&p_exlabel,
						&n_exlabel )){
					//apply_effect(time, ef->effect, op)){
					//  std::cout << "applied new effect" <<std::endl;
					//	  printBDD(ef->effect->ant->b);
					ef->info_at[time] = new_ef_level_info(time);

					ef->info_at[time]->is_dummy = 0;
					ef->first_occurence = time;
					ef->info_at[time]->updated = 1;

					//take care of added facts
					conseq = ef->effect->cons;
					while(conseq) {
						//std::cout << "cons" <<std::endl;
						for ( is = conseq->p_effects->indices; is; is=is->next ) {
							if ( (ft_node = gft_table[is->index]) == NULL ) {
								//std::cout << "Added POS Fact" << std::endl;
								// 		printFact(is->index);
								ft_node = new_ft_node( time+1, is->index, TRUE, FALSE, NULL );
								ft_node->info_at[time+1]->updated=1;
								ft_node->next = gall_fts_pointer;
								gall_fts_pointer = ft_node;
								gft_table[is->index] = ft_node;
								(gpos_facts_vector_at[time+1])[ft_node->uid_block] |= ft_node->uid_mask;
								ft_node->info_at[time+1]->is_dummy = 0;
								insert_ef_edge( &(ft_node->adders), ef );
								insert_ft_edge( &(ef->effects), ft_node );
							}
							else if(!ft_node->info_at[time+1]){
								// 		std::cout << "new pos level info" <<std::endl;
								// 		printFact(is->index);

								ft_node->info_at[time+1] = new_ft_level_info(ft_node);
								ft_node->info_at[time+1]->updated = 1;
								ft_node->info_at[time+1]->is_dummy = 0;
								(gpos_facts_vector_at[time+1])[ft_node->uid_block] |= ft_node->uid_mask;
							}
						}
						// std::cout << "neg cons" <<std::endl;
						for ( is = conseq->n_effects->indices; is; is=is->next ) {
							if ( (ft_node = gft_table[NEG_ADR( is->index )]) == NULL ) {
								//std::cout << "Added NEG Fact" << std::endl;
								// 		printFact(is->index);
								ft_node = new_ft_node( time+1, is->index, FALSE, FALSE, NULL );
								ft_node->info_at[time+1]->updated=1;
								ft_node->next = gall_fts_pointer;
								gall_fts_pointer = ft_node;
								gft_table[NEG_ADR( is->index )] = ft_node;
								(gneg_facts_vector_at[time+1])[ft_node->uid_block] |= ft_node->uid_mask;
								ft_node->info_at[time+1]->is_dummy = 0;
								insert_ef_edge( &(ft_node->adders), ef );
								insert_ft_edge( &(ef->effects), ft_node );
							}
							else if(!ft_node->info_at[time+1]){
								// 		std::cout << "new neg level info" <<std::endl;
								// 		printFact(is->index);

								ft_node->info_at[time+1] = new_ft_level_info(ft_node);
								ft_node->info_at[time+1]->updated = 1;
								ft_node->info_at[time+1]->is_dummy = 0;
								(gneg_facts_vector_at[time+1])[ft_node->uid_block] |= ft_node->uid_mask;
							}
						}
						conseq = conseq->next;
					}
				}
			}
		}
	}


	//now for effects of type 1b
	op = gops_with_unactivated_effects_pointer;
	while ( op && apply_all_effects( time, op ) ) {
		//    std::cout << "applied all 1"<<std::endl;
		tmp_op = op;
		op = op->thread;
		tmp_op->thread = NULL;
	}

	gops_with_unactivated_effects_pointer = op;
	prev_op = op;
	if ( op ) op = op->thread;
	while ( op ) {
		if ( apply_all_effects( time, op ) ) {
			//      std::cout << "applied all 2"<<std::endl;
			prev_op->thread = op->thread;
			tmp_op = op;
			op = op->thread;
			tmp_op->thread = NULL;
		} else {
			prev_op = prev_op->thread;
			op = op->thread;
		}
	}



	//*********************************************

	//insert new operators(2)
	o = gbit_operators;
	while ( o && apply_operator( time, o ) ) {
		//     std::cout << "applied new bitop 1" <<std::endl;
		tmp = o;
		o = o->next;
		tmp->next = used_gbit_operators;
		used_gbit_operators = tmp;
	}

	gbit_operators = o;
	prev = o;
	if ( o ) o = o->next;
	while ( o ) {
		if ( apply_operator( time, o ) ) {
			//     std::cout << "applied new bitop 2" <<std::endl;
			prev->next = o->next;
			tmp = o;
			o = o->next;
			tmp->next = used_gbit_operators;
			used_gbit_operators = tmp;
		} else {
			prev = prev->next;
			o = o->next;
		}
	}

	gop_vector_length_at[time] = ( ( int ) gops_count / gcword_size );
	if ( ( gops_count % gcword_size ) > 0 ) gop_vector_length_at[time]++;

	//   gef_vector_length_at[time] = ( ( int ) gnum_cond_effects_pre / gcword_size );
	//   if ( ( gnum_cond_effects_pre % gcword_size ) > 0 ) gef_vector_length_at[time]++;

}





/*******************************************************
 * ADDED BY DAN FOR LABEL GRAPH
 *******************************************************/

double getActCost(OpNode* i1, DdNode* worlds, int time){
	FtEdge* pres;
	double cost, c1;

	if(COST_PROP_LIT==SUM)
		cost = 0;
	else if(COST_PROP_LIT==MAXN){
		cost = -1;
	}

	//  std::cout << "act cost" <<std::endl;

	for(pres = i1->preconds; pres; pres=pres->next){
		//      printf("%d %d\n", pres->ft->positive, pres->ft->index);
		if(pres->ft->positive){
			if(COST_PROP_LIT==SUM)
				cost += //(time > 0 ? getFactCover(pres->ft, worlds, time) :
						getFactWorldCost(time, pres->ft->index, worlds);//);
			else if(COST_PROP_LIT==MAXN){
				c1 = //(time > 0 ? getFactCover(pres->ft, worlds, time) :
						getFactWorldCost(time, pres->ft->index, worlds);//);
				if(c1 > cost)
					cost = c1;
			}
		}
		else{
			if(COST_PROP_LIT==SUM)
				cost += //(time > 0 ? getFactCover(pres->ft, worlds, time):
						getFactWorldCost(time, pres->ft->index + num_alt_facts, worlds);//);
			else if(COST_PROP_LIT==MAXN){
				c1 = //(time > 0 ? getFactCover(pres->ft, worlds, time) :
						getFactWorldCost(time, pres->ft->index + num_alt_facts, worlds);//);
				if(c1 > cost)
					cost = c1;
			}
		}
	}

	if(!i1->preconds)
		cost = 0;

	//   std::cout << "done"<<std::endl;
	return cost;
}

void cost_prop_ops(int time){
	OpNode *i1;
	double cost;
	DdNode* tmp;
	std::list<LabelledElement*>::iterator i;
	std::list<std::list<LabelledElement*>*>::iterator j;


	//     printf("cost pr ops, time = %d\n", time);
	addCostLevel(0);

	for ( i1= gall_ops_pointer; i1; i1=i1->next ) {
		//   printf("op = %s, %d\n", i1->name, i1->alt_index);

		if(i1 && i1->info_at[time] && !i1->info_at[time]->updated){
			copyCostLevelUp(0, i1->index);
		}

		if(i1->info_at[time] && i1->info_at[time]->updated) {
			if(time > 0 && i1->info_at[time-1]){

				if(COST_PROP_TECH == CPT_FIXED_GROUPING_NEW_WORLDS){
					j = actWorldCost[i1->alt_index-1]->begin();
					//	    std::cout << "asize = " << actWorldCost[i1->alt_index]->size() << std::endl;
					for(int l = 0; l < time-1; l++) j++;
					//track new cost for each i
					//  std::cout << "size = " << (*j)->size() << std::endl;
					for(i = (*j)->begin(); i != (*j)->end(); ++i){
						//	    std::cout << "HI"<<std::endl;
						//	       	    printBDD((*i)->label);
						// 	    if(getActWorldCost(time-1, i1->alt_index, (*i)->label) == 0)
						// 	      cost = 0;
						// 	    else
						//	std::cout << "old"<< std::endl;
						cost = getActCost(i1, (*i)->label, time);
						//	    	    std::cout << "cost = " << cost <<std::endl;
						setActWorldCost(time, i1->alt_index-1, cost, (*i)->label);
					}

				}
				tmp = Cudd_bddAnd(manager, i1->info_at[time]->label->label, Cudd_Not(i1->info_at[time-1]->label->label));
			}
			else{
				tmp = i1->info_at[time]->label->label;
			}
			Cudd_Ref(tmp);
			//      tmp = bdd_and(manager, tmp, b_initial_state);
			//   printBDD(tmp);
			if(COST_PROP_TECH == CPT_NEW_WORLDS_ONLY && time > 0){
				copyCostLevelUp(0, i1->alt_index-1);
			}
			if(Cudd_ReadLogicZero(manager) != tmp){
				//	std::cout << "new"<< std::endl;
				if(COST_PR_INDIV && PF_LUG){
					for(int s = 0; s < NUMBER_OF_MGS; s++){
						//std::list<DdNode*>::iterator s = new_samples.begin();
							//s != new_samples.end(); s++){
						DdNode *s1 = make_sample_index(s);
						if(bdd_entailed(manager, s1, tmp)){
							cost = getActCost(i1, s1, time);
							//if(cost >= 0){
							// 	  	  printBDD(tmp);
							// 	 	  	  printf("cost = %d\n", cost);
							setActWorldCost(time, i1->alt_index-1, cost, s1);
							//	printf("done\n");
							//}
						}
					}
				}
				else{
					cost = getActCost(i1, tmp, time);
					//if(cost >= 0){
					// 	  	  printBDD(tmp);
					// 	 	  	  printf("cost = %d\n", cost);
					setActWorldCost(time, i1->alt_index-1, cost, tmp);
					//	printf("done\n");
					//}
				}
			}
		}
	}

	//  printf("doen oops\n");
}

void mark_operator_labels(int time) {
	OpNode *i1;
	DdNode* tmp, *tmp1;
	FtEdge* pres;
	int cost = 0, c1;
	//  printf("-------Marking Op Labels at time: %d -------------------\n", time);
	//  Cudd_CheckKeys(manager);fflush(stdout);

	//for all opNodes
	for ( i1= gall_ops_pointer; i1; i1=i1->next ) {
		if(i1->info_at[time]  ) {




			/*    if(time > 0 && i1->info_at[time-1] && i1->info_at[time-1]->label && bdd_entailed(manager, b_initial_state,i1->info_at[time-1]->label->label)){ */
			/* 	//	printf("full support\n"); */
			/* 	//	printBDD(i1->info_at[time-1]->label->label); */
			/* 		i1->info_at[time]->label = new_Label(i1->info_at[time-1]->label->label ,0); */
			/* 	Cudd_Ref(i1->info_at[time]->label->label); */
			/* 	//	printf("OKAY\n"); */
			/* 	continue; */
			/*       }  */


			// use the precond label to config the OpNode label
			if(i1->preconds && i1->info_at[time]->updated)
			{

				// 	if(i1->is_noop){
				// 	  std::cout << (i1->preconds->ft->positive? "pos": "neg" ) << " noop ";
				// 	  printFact(i1->preconds->ft->index);
				// 	  std::cout << " is used at " <<  time << std::endl;
				// 	}
				// 	else{
				// 	  printf("%s is used at %d\n", i1->name, time);   fflush(stdout);
				//  	}
				tmp =  and_labels(NULL, i1->preconds, time);
				Cudd_Ref(tmp);
				if(0 && PF_LUG && !i1->is_noop){
					tmp1 = Cudd_bddAnd(manager, tmp,
							Cudd_Not(goal_samples));
					Cudd_Ref(tmp1);
					Cudd_RecursiveDeref(manager, tmp);
					tmp = tmp1;
					Cudd_Ref(tmp);
					Cudd_RecursiveDeref(manager, tmp1);
				}
				//	printBDD(tmp);
				// 当前OpNode没有label，利用BDD创建label
				if(!i1->info_at[time]->label)
					i1->info_at[time]->label = new_Label(tmp, 0);
				// 存在label且BDD相同，说明该OpNode的label没有变化, 重置为0
				else if(i1->info_at[time]->label->label == tmp){
					i1->info_at[time]->updated = 0;
				}
				// 存在label但BDD不同，更新
				else{
					i1->info_at[time]->label->label = tmp;
					Cudd_Ref(i1->info_at[time]->label->label);
				}

				/*  	printf("Got Operator labels of %s at time: %d \n", i1->name, time); */
				// 	   	printBDD(i1->info_at[time]->label->label);
				Cudd_RecursiveDeref(manager, tmp);
			}
			/**
			 * momo007 2022.09.26 maybe a bug
			 * 下面两种情况分类有点问题?
			 * if失败包括: empty precond or precond is false
			 */
			else
			{
				if(PF_LUG || my_problem->domain().requirements.non_deterministic){
					// 先创建没有成立的label为所有初始状态（相当于empty precond）
					if(!i1->info_at[time]->label)
						i1->info_at[time]->label = new_Label(initialBDD,
								//b_initial_state,
								0);
					// 有label，且前提成立，还是empty precond?
					else if(i1->info_at[time]->updated){
						Cudd_RecursiveDeref(manager, i1->info_at[time]->label->label);
						i1->info_at[time]->label->label=initialBDD;
						Cudd_Ref(i1->info_at[time]->label->label);
					}
				}

				else if(!PF_LUG && my_problem->domain().requirements.probabilistic)
					if(!i1->info_at[time]->label)
						i1->info_at[time]->label = new_Label(Cudd_addBddStrictThreshold(manager,
								//b_initial_state
								initialBDD, 0.0), 0);
				//tmp = Cudd_addBddStrictThreshold(manager, b_initial_state, 0.0);
				//	printBDD(i1->info_at[time]->label->label);
			}
		}
	}
	//  std::cout << "done with facts" <<std::endl;
}


double getEffectCost(EfNode* e1, DdNode* worlds, int time){
	int cost, c1;
	FtEdge* pres;

	//   std::cout << "get ef cost" <<std::endl;

	if(COST_PROP_LIT==SUM || !e1 || !e1->conditions)
		cost = 0;
	else if(COST_PROP_LIT==MAXN){
		cost = -1;
	}

	//   if(!e1->conditions || !e1 )
	//     return cost;



	for(pres = e1->conditions; pres; pres=pres->next){
		//printFact(pres->ft->index);
		if(pres->ft->positive){
			if(COST_PROP_LIT==SUM)
				cost += //(time > 0 ? getFactCover(pres->ft, worlds, time) :
						getFactWorldCost(time, pres->ft->index, worlds);//);
			else if(COST_PROP_LIT==MAXN){
				c1 = //(time > 0 ? getFactCover(pres->ft, worlds, time) :
						getFactWorldCost(time, pres->ft->index, worlds);//);
				if(c1 > cost)
					cost = c1;
			}
		}
		else{
			if(COST_PROP_LIT==SUM)
				cost += //(time > 0 ? getFactCover(pres->ft, worlds, time) :
						getFactWorldCost(time, pres->ft->index + num_alt_facts, worlds);//);
			else if(COST_PROP_LIT==MAXN){
				c1 = //(time > 0 ? getFactCover(pres->ft, worlds, time) :
						getFactWorldCost(time, pres->ft->index + num_alt_facts, worlds);//);
				if(c1 > cost)
					cost = c1;
			}
		}
		//    std::cout << "pre cost = " << cost << std::endl;
	}
	if(!e1->conditions)
		cost = 0;

	//std::cout << "Support cost = " << cost << std::endl;
	//    printBDD(worlds);
	// if(COST_PROP==SUM){
	cost+= //getActCost(e1->op, worlds, time);/
			getActWorldCost(time, e1->op->alt_index-1, worlds);
	//  std::cout << "w/ act Support cost = " << cost << std::endl;
	if(e1->op->alt_index-1 < num_alt_acts)
		cost += -1*e1->effect->reward;//alt_act_costs[e1->op->alt_index];

	//}
	//   else if(COST_PROP==MAX){
	//     c1 = getActCost(e1->op, worlds, time);//getActWorldCost(time, e1->op->index, worlds);
	//     if(c1 > cost)
	//       cost = c1;
	//   }
	// std::cout << "Cost = " << cost << std::endl;
	return cost;
}

DdNode* unionNDLabels(EfLevelInfo_pointer ef){
	Label *lab = ef->label;
	DdNode *retDD = Cudd_ReadLogicZero(manager), *tmp;

	while(lab){
		retDD = bdd_or(manager, lab->label, tmp=retDD);
		Cudd_Ref(retDD);
		Cudd_RecursiveDeref(manager, tmp);
		lab = lab->next;
	}
	return retDD;
}

void cost_prop_efs(int time){
	int cost;
	OpNode *i1;
	EfNode *e1;
	DdNode *tmp, *newND, *oldND, *tmp1;
	int ef =0;

	//   std::cout << "[" <<std::endl;
	//   Cudd_CheckKeys(manager);
	//   std::cout << "|" <<std::endl;

	std::list<LabelledElement*>::iterator i;
	std::list<std::list<LabelledElement*>*>::iterator j;

	//   printf("cost pr efs\n");
	addCostLevel(1);

	for ( i1= gall_ops_pointer; i1/* != gprev_level_ops_pointer*/; i1=i1->next ){
		//if(i1->is_noop)
		//  std::cout << "NOOP" <<std::endl;
		//  printf("\nop = %s, time = %d\n", i1->name, time);
		//    printBDD(i1->info_at[time]->label->label);

		if(i1 && i1->info_at[time] && !i1->info_at[time]->updated){
			copyCostLevelUp(0, i1->index);
		}

		if(i1->info_at[time] && i1->info_at[time]->updated) {
			for(ef = 0; ef < 2; ef++){
				if(ef == 0){
					//			std::cout << "Uncond"<<std::endl;
					e1 = i1->unconditional;
				}
				else{
					//			std::cout << "Cond"<<std::endl;
					e1 = i1->conditionals;
				}


				for(; e1; e1=e1->next) {
					//std::cout << "ef" <<std::endl;
					cost = 0;
					if(e1->info_at[time]  && !e1->info_at[time]->is_dummy){
						//printBDD(e1->info_at[time]->label->label);
						if(time > 0 && e1->info_at[time-1]){

							if(COST_PROP_TECH == CPT_FIXED_GROUPING_NEW_WORLDS){
								j = effectWorldCost[e1->alt_index]->begin();
								for(int l = 0; l < time-1; l++) j++;
								//track new cost for each i
								for(i = (*j)->begin(); i!= (*j)->end(); i++){
									DdNode *lab;
									if(PF_LUG){//labels are not monotonic -- remove excess worlds from partitions
										lab = Cudd_bddAnd(manager, (*i)->label, e1->info_at[time-1]->label->label);
										Cudd_Ref(lab);
										if(lab == Cudd_ReadLogicZero(manager)){
											Cudd_RecursiveDeref(manager, lab);
											continue;
										}
									}
									else{
										lab = (*i)->label;
										Cudd_Ref(lab);
									}

									if(getEffectWorldCost(time-1, e1->alt_index, lab) == 0)
										cost = 0;
									else
										cost = getEffectCost(e1, lab, time);
									//std::cout << "prop cost = " << cost<<std::endl;
									//printBDD(lab);
									setEffectWorldCost(time, e1->alt_index, cost, lab);
								}
							}
							//tmp = bdd_and(manager, unionNDLabels(e1->info_at[time]), Cudd_Not(unionNDLabels(e1->info_at[time-1])));
							tmp = Cudd_bddAnd(manager, e1->info_at[time]->label->label, Cudd_Not(e1->info_at[time-1]->label->label));
						}
						else{
							//tmp = unionNDLabels(e1->info_at[time]);
							tmp = e1->info_at[time]->label->label;
						}
						//	    printBDD(tmp);

						//	    tmp1 = Cudd_bddAnd(manager, e1->op->info_at[time]->label->label, tmp);
						Cudd_Ref(tmp);


						if(tmp != Cudd_ReadLogicZero(manager)){
							if(COST_PROP_TECH == CPT_NEW_WORLDS_ONLY && time > 0){
								copyCostLevelUp(1, e1->alt_index);
							}
							if(COST_PR_INDIV && PF_LUG){
//								for(std::list<DdNode*>::iterator s = new_samples.begin();
//										s != new_samples.end(); s++){
//									if(bdd_entailed(manager, *s, tmp)){
//										cost = getEffectCost(e1, *s, time);
//										//if(cost >= 0){
//										//printf("new cost = %d\n", cost);
//										//printBDD(tmp);
//										setEffectWorldCost(time, e1->alt_index, cost, *s);
//										// }
//									}
//								}
							}
							else{
								cost = getEffectCost(e1, tmp, time);
								//if(cost >= 0){
								//printf("new cost = %d\n", cost);
								//printBDD(tmp);
								setEffectWorldCost(time, e1->alt_index, cost, tmp);
								//}
							}
						}
					}
				}
			}
		}
	}
	//   Cudd_CheckKeys(manager);
	//   std::cout << "]" <<std::endl;
	//  std::cout << "DOEN"<<std::endl;
}


extern int NDLMax;
void getNDLabels(OpNode* i1, DdNode* allEffForm, int time){//Label* oldLabels, int ndlow, int ndhigh){

	int numOutcomes = 0;// = Cudd_CountMinterm(manager, allEffForm, num_alt_facts);
	int numVars = 0;
	int extraValues = 0;
	int i, j;
	int lowVar, highVar;
	int effType;
	Label* tmpLab, *tmpLab1;
	DdNode* tmpdd, *tmpndlab, *allLabs;
	Consequent* cons;
	//printBDD(allEffForm);


	EfNode *e1;

	if(i1->info_at[time-1]){
		for(effType = 0; effType < 2; effType++){
			if(!effType)
				e1 = i1->unconditional;
			else
				e1 = i1->conditionals;

			for(; e1; e1=e1->next) {
				if(e1->info_at[time-1]){
					tmpLab = e1->info_at[time-1]->label;
					tmpLab1 = e1->info_at[time]->label;
					while(tmpLab){
						tmpLab1->label = Cudd_bddOr(manager, tmpLab->label, tmpdd=tmpLab1->label);
						Cudd_Ref(tmpLab1->label);
						Cudd_RecursiveDeref(manager, tmpdd);
						tmpLab = tmpLab->next;
						tmpLab1 = tmpLab1->next;
					}
				}
			}
		}
	}
	else{

		DdNode** minterms = extractDNFfromBDD(allEffForm);

		i = 0;
		while(minterms[i]){
			//printBDD(minterms[i]);
			i++;
			numOutcomes++;
		}

		for( i = 1; i <= (numOutcomes); i++){
			// std::cout << "i = " << i  <<std::endl;
			if((2<<(i-1)) >= (numOutcomes)){
				numVars = i;
				break;
			}
		}
		extraValues = (2<< (numVars-1))-numOutcomes;

		// printf("marking nd effects of %s\n", i1->name);
		//    printf("need %d ndvars for %d nd outcomes, with %d extra values\n",numVars, (numOutcomes), extraValues);


		lowVar = NDLcounter+(2*num_alt_facts);
		highVar = lowVar;
		for(i = 0; i < numVars; i++){
			generateUniqueNonDeterLabel(0, 0, 0, 0);
			highVar++;
		}
		//  std::cout <<NDLMax << " ";
		allLabs = Cudd_ReadLogicZero(manager);
		for(effType = 0; effType < 2; effType++){
			if(!effType)
				e1 = i1->unconditional;
			else
				e1 = i1->conditionals;

			for(; e1; e1=e1->next) {
				tmpLab = e1->info_at[time]->label;
				//generate bitstring for each outcome and use bits to set label
				cons = e1->effect->cons;
				while(cons){
					tmpdd = Cudd_ReadLogicZero(manager);
					for(i = 0; i < numOutcomes; i++){
						if(!bdd_is_one(manager, Cudd_Not(bdd_and(manager, minterms[i], cons->b)))){
							//printBDD(tmpLab->label);
							//      print_BitVector((BitVector *)&i, gft_vector_length);
							tmpndlab = Cudd_ReadOne(manager);
							for(j = 0; j < numVars; j++){

								// Super dangerous typecasting

								if(get_bit((BitVector *)&i, gft_vector_length, j))
									tmpndlab = bdd_and(manager, tmpndlab, Cudd_bddIthVar(manager, j+lowVar));
								else
									tmpndlab = bdd_and(manager, tmpndlab, Cudd_Not(Cudd_bddIthVar(manager, j+lowVar)));

							}
							tmpdd = bdd_or(manager, tmpndlab, tmpdd);
						}
					}
					tmpLab->label = bdd_and(manager, tmpdd, tmpLab->label);
					//		printBDD(tmpLab->label);
					allLabs = bdd_or(manager, allLabs, tmpdd);
					//	printBDD(tmpdd);
					tmpLab = tmpLab->next;
					cons = cons->next;
				}


				//take care of extra values
				tmpndlab = Cudd_ReadOne(manager);
				tmpdd= Cudd_ReadLogicZero(manager);
				for(i = numOutcomes; i < numOutcomes+extraValues; i++){
					//	print_BitVector((BitVector *)&i, gft_vector_length);
					for(j = 0; j < numVars; j++){
						if(get_bit((BitVector *)&i, gft_vector_length, j))
							tmpndlab = bdd_and(manager, tmpndlab, Cudd_bddIthVar(manager, j+lowVar));
						else
							tmpndlab = bdd_and(manager, tmpndlab, Cudd_Not(Cudd_bddIthVar(manager, j+lowVar)));

					}
					tmpdd = bdd_or(manager, tmpndlab, tmpdd);
				}
				// printBDD(tmpdd);
				tmpLab = e1->info_at[time]->label;
				while(tmpLab){
					tmpLab->label = bdd_or(manager, tmpdd, tmpLab->label);
					//	printBDD(tmpLab->label);
					allLabs = bdd_or(manager, allLabs, tmpdd);
					tmpLab = tmpLab->next;
				}
			}
		}
		delete [] minterms;
		//   printf("got alllabs = \n");
		//      printBDD(allLabs);


	}


}

#ifdef PPDDL_PARSER
void generate_labels_recurse(DdNode* variables,
		DdNode* model,
		int curr_var,
		int num_new_labels,
		std::list<DdNode*> *nd_cubes){

	//  std::cout << curr_var << " " << num_new_labels <<std::endl;


	//exit recursion because found model of label variables
	if(curr_var == num_new_labels){
		//    printBDD(model);
		Cudd_Ref(model);
		nd_cubes->push_back(model);
		return;
	}


	//find next var to branch over
	int i = 0, j = -1;
	DdNode *tmp, *fr;
	while(1){
		if(Cudd_bddIsVarEssential(manager, variables, i, 1))
			j++;

		if(j == curr_var){
			//positive
			tmp = Cudd_bddIthVar(manager, i);
			Cudd_Ref(tmp);
			fr = Cudd_bddAnd(manager, model, tmp);
			Cudd_Ref(fr);
			Cudd_RecursiveDeref(manager, tmp);
			generate_labels_recurse(variables, fr, curr_var+1, num_new_labels, nd_cubes);
			Cudd_RecursiveDeref(manager, fr);

			//negative
			tmp = Cudd_Not(Cudd_bddIthVar(manager, i));
			Cudd_Ref(tmp);
			fr = Cudd_bddAnd(manager, model, tmp);
			Cudd_Ref(fr);
			Cudd_RecursiveDeref(manager, tmp);
			generate_labels_recurse(variables, fr, curr_var+1, num_new_labels, nd_cubes);
			Cudd_RecursiveDeref(manager, fr);

			break;
		}
		i++;
	}
}

void generate_n_labels(int num_new_labels,
		std::list<DdNode*> *nd_cubes,
		std::list<int>* new_vars){
	DdNode* variables = Cudd_ReadOne(manager), *fr, *var;
	Cudd_Ref(variables);
	//fr = NULL;



	//pick variables to use for labels
	for(int i = 0; i < num_new_labels; i++){

		//  std::cout << "i = " << i << std::endl;
		if(NDLcounter < num_alt_facts){ //use next state vars
			var = Cudd_bddIthVar(manager, NDLcounter*2+1);
			Cudd_Ref(var);
			new_vars->push_back(NDLcounter*2+1);
			//std::cout << NDLcounter*2+1 << std::endl;
		}
		else if(NDLcounter < maxNDL){ //use existing vars
			var = Cudd_bddIthVar(manager, NDLcounter+num_alt_facts);
			Cudd_Ref(var);
			new_vars->push_back(num_alt_facts+NDLcounter);
			//std::cout << num_alt_facts+NDLcounter << std::endl;
		}
		else if(NDLcounter == maxNDL){ //create new vars
			var = //Cudd_bddIthVar(manager, num_alt_facts+NDLcounter);//
					Cudd_bddNewVar(manager);
			Cudd_Ref(var);
			new_vars->push_back(num_alt_facts+NDLcounter);
			//std::cout << num_alt_facts+NDLcounter << std::endl;
		}

		fr = Cudd_bddAnd(manager, variables, var);
		Cudd_Ref(fr);




		Cudd_RecursiveDeref(manager, var);
		Cudd_RecursiveDeref(manager, variables);
		variables = fr;
		Cudd_Ref(variables);
		Cudd_RecursiveDeref(manager, fr);
		// fr = NULL;
		NDLcounter++;
		if(NDLcounter > maxNDL)
			maxNDL = NDLcounter;
	}

	//      std::cout << "Using vars: " <<  std::endl;
	//       printBDD(variables);

	//std::cout << "NDLcounter = " << NDLcounter << "maxNDL = " << maxNDL <<std::endl;
	//generate labels
	var = Cudd_ReadOne(manager);
	Cudd_Ref(var);
	generate_labels_recurse(variables, var, 0, num_new_labels, nd_cubes);
	Cudd_RecursiveDeref(manager, var);


}

int precond_labels_changed(int time, EfNode* e){
	if(time == 0)
		return 1;

	//std::cout << "enter" <<std::endl;

	for(FtNode *f = e->conditions->ft; f ; f=f->next){
		if(f->info_at[time-1] && f->info_at[time] &&
				f->info_at[time]->label->label &&  f->info_at[time-1]->label){
			//      printFact(f->index); std::cout << f->positive <<std::endl;
		}
		else{
			//       std::cout << "OOPS" <<std::endl;
			//       printFact(f->index); std::cout << f->positive <<std::endl;
			return 0;
		}

		if((f->info_at[time] && !f->info_at[time-1]) ||
				f->info_at[time]->label->label != f->info_at[time-1]->label->label){
			//  std::cout << "one-changed "  << e->op->name <<std::endl;
			return 1;
		}
	}

	//  std::cout << "all non-changed "  << e->op->name <<std::endl;
	//  exit(0);
	return 0;
}

void mark_probabilitic_effect_labels(int time){
	//for each set of feasible outcomes, give a label
	//std::cout << std::endl<<"&&&&&&&&&&&&&TIME == " << time << std::endl;
	//number of new worlds is equal to product of number of outcomes for
	//each action, assuming no mutexes
	OpNode *i1 = gall_ops_pointer;
	EfNode *e1;
	DdNode* fr, *fr1, *tmp;
	std::set<int> act_outcomes;
	std::list<int>* new_vars = new std::list<int>();
	int num_new_worlds = 1;
	DdNode* new_level_worlds = Cudd_ReadOne(manager), *new_effect_worlds;
	Cudd_Ref(new_level_worlds);
	__gnu_cxx::hash_set<DdNode*> storedLabels;




	level_vars->push_back(new_vars);

	//   std::cout << "["<<std::endl;
	//       Cudd_CheckKeys(manager);
	//       std::cout <<"|" << std::endl;

	for (; i1; i1=i1->next ) {
		act_outcomes.clear();
		num_new_worlds = 1;

		double num =0;//1.0;
		double num1 = 0;
		for(int t = 1; t < 2; t++){  //only conditionals
			if(t == 0)
				e1=i1->unconditional;
			else
				e1=i1->conditionals;

			if(e1 && e1->op->is_noop)
				continue;

			for(; e1; e1=e1->next) {
				if(e1->info_at[time] &&

						!e1->info_at[time]->is_dummy){
					//std::cout << e1->effect->original->in_rp;
					//num+=e1->effect->original->in_rp;
					//	    printBDD(e1->info_at[time]->label->label);
					//	    printBDD(probabilisticLabels);
					fr = Cudd_BddToAdd(manager, e1->info_at[time]->label->label);
					Cudd_Ref(fr);
					tmp = Cudd_addApply(manager, Cudd_addTimes, probabilisticLabels,
							fr);
					Cudd_Ref(tmp);
					Cudd_RecursiveDeref(manager, fr);
					//	    printBDD(tmp);
					num+=exAbstractAllLabels(tmp, time-1);
					Cudd_RecursiveDeref(manager, tmp);
					num1++;
				}
			}
		}
		num /=num1;



		//        if(num > 0){
		//       	std::cout << i1->name << " " << num << std::endl;
		// 	//exit(0);
		// 	}

		//       double num1 = (rand()%100);//*(1.0/((double)time+1)*((double)time+1));
		//       //      std::cout << "num1 = " << num1 <<std::endl;
		//       double num2 = (100.0);///(double)((double)time+1)*((double)time+1));
		//       //      std::cout << "num2 = " << num2 <<std::endl;


		if(0 &&
				i1->info_at[time]
				            && !i1->is_noop
				            // && num1 > num2*.850
				            //&&  num < .99
				            //&& rand()%(int)(100*num) > (num*95)  //sample from distibution of uses
				            //&& (rand()%100 >= 75) //random sampling
				            //&& !i1->info_at[time-1]  //only label first occurrance of probabilistic actions
		) {
			//      for(int t = 0; t < 2; t++){
			for(int t = 1; t < 2; t++){  //only conditionals
				if(t == 0)
					e1=i1->unconditional;
				else
					e1=i1->conditionals;




				//count outcomes of action
				for(; e1; e1=e1->next) {
					if(e1->info_at[time] &&
							!e1->info_at[time]->is_dummy
							//	     &&	     precond_labels_changed(time, e1)
					){
						//   std::cout << "$";
						//printf("*******Getting label for eff of %s, outcome:  %d, pr: %f \n", i1->name, e1->effect->outcome, e1->effect->probability);
						// 	    printBDD(e1->info_at[time]->label->label);
						for(int n = 0; n < e1->effect->op->num_outcomes; n++)
							if(e1->effect->outcome->find(n) != e1->effect->outcome->end())//get_bit(e1->effect->outcome, ((int)n/gcword_size)+1, n))
								act_outcomes.insert(n);

					}
				}
			}

			//      std::cout << "num_outcomes = " << act_outcomes.size() << std::endl;
			if(act_outcomes.size() > 0)
				num_new_worlds *= act_outcomes.size();

			if(num_new_worlds == 1) //don't have an prob effs
				continue;

			//std::cout << "Num new worlds = " << num_new_worlds <<std::endl;

			int num_new_labels = 0;
			for( int i = 1; i <= (num_new_worlds); i++){
				// std::cout << "i = " << i  <<std::endl;
				if((2<<(i-1)) >= (num_new_worlds)){
					num_new_labels = i;
					break;
				}
			}
			int extraValues = (2<< (num_new_labels-1))-num_new_worlds;

			//        std::cout << "Need " << num_new_labels << " new labels, with "
			//                  << extraValues << " extra values used" <<std::endl;



			//     std::cout << "[" <<std::endl;
			//     Cudd_CheckKeys(manager);
			//     std::cout << "| p"<<std::endl;
			//    std::cout << "[" <<std::endl;
			//     Cudd_CheckKeys(manager);
			//     std::cout << "| p"<<std::endl;
			//get models for labels
			std::list<DdNode*> nd_cubes;
			generate_n_labels(num_new_labels, &nd_cubes, new_vars);

			//    Cudd_CheckKeys(manager);
			//     std::cout << "]"<<std::endl;
			//add models for extra labels to the labels used
			DdNode* unused_labels = Cudd_ReadLogicZero(manager);
			int r = 0;
			std::list<DdNode*>::iterator k = nd_cubes.begin();
			for(;r < extraValues; r++, k = nd_cubes.begin()){
				fr = Cudd_bddOr(manager, *k, unused_labels);
				Cudd_Ref(fr);
				Cudd_RecursiveDeref(manager, unused_labels);
				unused_labels = fr;
				Cudd_Ref(unused_labels);
				Cudd_RecursiveDeref(manager, fr);
				nd_cubes.remove(*k);
			}

			//        std::cout << "unused:" << std::endl;
			//        printBDD(unused_labels);


			for(std::list<DdNode*>::iterator k = nd_cubes.begin();
					unused_labels != Cudd_ReadLogicZero(manager) && k != nd_cubes.end();
					k++){
				fr = Cudd_bddOr(manager, *k, unused_labels);
				Cudd_Ref(fr);
				Cudd_RecursiveDeref(manager, *k);
				*k = fr;
				//printBDD(*k);
				Cudd_Ref(*k);
				Cudd_RecursiveDeref(manager, fr);
			}
			Cudd_RecursiveDeref(manager, unused_labels);



			__gnu_cxx::hash_map<int, DdNode*> outcome_labels;
			k = nd_cubes.begin();


			//assign labels to outcomes
			for(int t = 1; t < 2; t++){ //only conditionals
				if(t == 0)
					e1=i1->unconditional;
				else
					e1=i1->conditionals;
				for(; e1; e1=e1->next) {
					if(e1->info_at[time] && !e1->info_at[time]->is_dummy){

						for(int n = 0; n < e1->effect->op->num_outcomes; n++){
							if(e1->effect->outcome->find(n) != e1->effect->outcome->end() && //get_bit(e1->effect->outcome, ((int)n/gcword_size)+1, n)  &&
									//	     precond_labels_changed(time, e1) &&
									outcome_labels.find(n) == outcome_labels.end()){
								//	     	    std::cout << "outcome: " << e1->effect->outcome << std::endl;
								//  	    printBDD(*k);
								outcome_labels[n] = *k;
								Cudd_Ref(outcome_labels[n]);
								Cudd_RecursiveDeref(manager, *k);
								k++;
							}
						}
					}
				}
			}
			nd_cubes.clear();



			DdNode* j;
			new_effect_worlds = Cudd_ReadZero(manager);
			Cudd_Ref(new_effect_worlds);
			for(int t = 1; t < 2; t++){ //only conditionals
				if(t == 0)
					e1=i1->unconditional;
				else
					e1=i1->conditionals;
				for(; e1; e1=e1->next) {
					if(e1->info_at[time] && !e1->info_at[time]->is_dummy
							//	     &&    precond_labels_changed(time, e1)
					){
						for(int n = 0; n < e1->effect->op->num_outcomes; n++){
							if(e1->effect->outcome->find(n) != e1->effect->outcome->end()){//get_bit(e1->effect->outcome, ((int)n/gcword_size)+1, n)){
								j = outcome_labels[n];
								fr1 = Cudd_bddAnd(manager, e1->info_at[time]->label->label, j);
								Cudd_Ref(fr1);
								Cudd_RecursiveDeref(manager, e1->info_at[time]->label->label);
								e1->info_at[time]->label->label = fr1;
								Cudd_Ref(e1->info_at[time]->label->label);
								Cudd_RecursiveDeref(manager, fr1);

								//set up new prob effect label
								fr = Cudd_addConst(manager, ((*e1->effect->probability)[n]/(extraValues+1)));
								Cudd_Ref(fr);
								fr1 = Cudd_BddToAdd(manager, j);
								Cudd_Ref(fr1);
								tmp = Cudd_addApply(manager, Cudd_addTimes, fr, fr1);
								Cudd_Ref(tmp);
								Cudd_RecursiveDeref(manager, fr1);
								Cudd_RecursiveDeref(manager, fr);

								//storing labels of effects
								__gnu_cxx::hash_set<DdNode*>::iterator p = storedLabels.find(tmp);
								if(p == storedLabels.end()){
									storedLabels.insert(tmp);
									fr = Cudd_addApply(manager, Cudd_addPlus,
											tmp, new_effect_worlds);
									Cudd_Ref(fr);
									Cudd_RecursiveDeref(manager, new_effect_worlds);
									new_effect_worlds = fr;
									Cudd_Ref(new_effect_worlds);
									Cudd_RecursiveDeref(manager, fr);
								}
								else
									Cudd_RecursiveDeref(manager, tmp);
							}
						}
					}
				}
			}

			//std::cout << "new_effect_worlds: " <<std::endl;
			//   	printBDD(new_effect_worlds);
			fr = Cudd_addApply(manager, Cudd_addTimes, new_effect_worlds, new_level_worlds);
			Cudd_Ref(fr);
			Cudd_RecursiveDeref(manager, new_level_worlds);
			new_level_worlds = fr;
			Cudd_Ref(new_level_worlds);
			Cudd_RecursiveDeref(manager, fr);
			Cudd_RecursiveDeref(manager, new_effect_worlds);
			//  std::cout << "new_level_worlds: " <<std::endl;
			//    	printBDD(new_level_worlds);


			__gnu_cxx::hash_set<DdNode*>::iterator p = storedLabels.begin();
			for(; p != storedLabels.end(); p++)
				Cudd_RecursiveDeref(manager, *p);
			storedLabels.clear();

			//    Cudd_CheckKeys(manager);
			//    std::cout << "]"<<std::endl;
			__gnu_cxx::hash_map<int, DdNode*>::iterator hm = outcome_labels.begin();
			for(; hm != outcome_labels.end(); hm++)
				Cudd_RecursiveDeref(manager, (*hm).second);
			outcome_labels.clear();

		}
	}

	//fold marginal over effects at this level into joint over all levels
	fr = Cudd_addApply(manager, Cudd_addTimes, new_level_worlds, probabilisticLabels);
	Cudd_Ref(fr);
	Cudd_RecursiveDeref(manager, probabilisticLabels);
	probabilisticLabels = fr;
	Cudd_Ref(probabilisticLabels);
	Cudd_RecursiveDeref(manager, fr);
	Cudd_RecursiveDeref(manager, new_level_worlds);

	if(verbosity > 3){

		//print probability mass in effects
		for (i1 = gall_ops_pointer; i1; i1=i1->next ) {
			if(i1->info_at[time] ) {
				for(e1=i1->conditionals; e1; e1=e1->next) {
					if(e1->info_at[time] &&
							!e1->info_at[time]->is_dummy){
						//	    printf("*******Getting label for eff of %s, outcome:  %d, pr: %f \n", i1->name, e1->effect->outcome, e1->effect->probability);
						DdNode *addlab = Cudd_BddToAdd(manager, e1->info_at[time]->label->label);
						Cudd_Ref(addlab);
						DdNode *prlab = Cudd_addApply(manager, Cudd_addTimes, addlab, probabilisticLabels);
						Cudd_Ref(prlab);
						Cudd_RecursiveDeref(manager, addlab);

						std::cout << "pr = " << exAbstractAllLabels(prlab, time-1)<< std::endl;
						Cudd_RecursiveDeref(manager, prlab);

						//printBDD(e1->info_at[time]->label->label);
					}
				}
			}
		}
	}
}

void sample_proposal_DBN(int time){
  OpNode *i1;
  EfNode *e1;
	
  if(!NDACTIONS)
    return;
  //  std::cout << "Sample Effects" << std::endl;

	//    std::cout << "[" << std::endl;
	//    Cudd_CheckKeys(manager);
	//    std::cout << "|"<<std::endl;
  if(1){
    
    for ( i1= gall_ops_pointer; i1; i1=i1->next ) {
      if(i1->info_at[time] && !i1->is_noop){
	const Action *op = i1->action;
	dbn* dbop = action_dbn(*op);
	//	std::cout << "op" << stdz::endl;
	if(dbop->num_aux_vars == 0){
	  continue;
	}

	//map aux var values to samples
	std::map<const Action*, std::map<int, std::map<int, DdNode*>*>*>::iterator ls = lug_samples.find(op);
	if(ls == lug_samples.end()){
	  lug_samples[op]= new std::map<int, std::map<int, DdNode*>*>();
	}
	if((*lug_samples[op])[time%NUM_LUG_SAMPLES] == NULL){
	  (*lug_samples[op])[time%NUM_LUG_SAMPLES] =   new std::map<int, DdNode*>();
	  //	  std::cout << "new sample set " << time << " " << i1->index << std::endl;
	}
	 std::map<int, DdNode*>* aux_var_samples = (*lug_samples[op])[time%NUM_LUG_SAMPLES];
	
	e1=i1->conditionals;
	for(; e1; e1=e1->next) {
	  if(!e1->info_at[time]) continue;	
	
	
	
	  if(RBPF_LUG && e1->effect->node->var > 2*(num_alt_facts-2)){
	    //it is an effect for an analytical variable.	    	    
	    continue;
	  }


	  //get relevant aux vars, and sample if needed
	  std::set<int> *parents = &e1->effect->node->parents;
	  DdNode* sample = Cudd_ReadOne(manager);
	  Cudd_Ref(sample);
	  

	  //DdNode *gar = Cudd_addApply(manager, Cudd_addTimes, Cudd_ReadOne(manager), Cudd_ReadOne(manager));

	  for(std::set<int>::iterator p = parents->begin(); p != parents->end(); p++){
	    if(*p < 2*num_alt_facts) continue;
	    std::map<int, DdNode*>::iterator avsi = aux_var_samples->find(*p);
	    //	    std::cout << "parent = " << *p << std::endl;
	    //	    printBDD(dbop->vars[*p]->cpt);
	    if(avsi == aux_var_samples->end()){
	      //gen samples
	  
	      (*aux_var_samples)[*p]=draw_cpt_samples(dbop->vars[*p]->cpt, NUMBER_OF_MGS, dbop->vars[*p]->dd_bits);
// 	      //printBDD(aux_var_samples[*p]);
// 	      //aux_var_samples[*p] = Cudd_ReadOne(manager);
// 	      double sample_offset = ((double)randomGen->myrand()/((double)(RAND_MAX)+(double)(1)));
// 		DdNode* aux_value = Cudd_ReadOne(manager);
// 		Cudd_Ref(aux_value);

// 	      for(int s = 0; s < NUMBER_OF_MGS; s++){
// 		DdNode* sd = make_sample_index(s);
// 		//std::cout << "Using sample" << std::endl;

// 		//sample each var
// 		//std::map<int, double> aux_value;
// 		//	for(int i = 0; i < dbop->num_aux_vars; i++){
// 		  //std::cout << "Sampling aux var " << i << " of " << dbop->num_aux_vars << std::endl;
// 		double pr = ((double)s/NUMBER_OF_MGS)+sample_offset;
// 		//std::cout << "pr = " << pr << " " << dbop->vars[*p]->outcomes.size() << std::endl;
// 		if(pr > 1.0){
// 		  pr -= 1.0;
// 		}
// 		double cumm = 0.0;
// 		  //	  for(std::map<double,double>::iterator j = (*dbop->vars[2*num_alt_facts+i]->cpt.begin()).second->begin(); j != (*dbop->vars[2*num_alt_facts+i]->cpt.begin()).second->end(); j++){

// 		for(std::map<int, DdNode*>::iterator j = dbop->vars[*p]->outcomes.begin();
// 		    j != dbop->vars[*p]->outcomes.end(); j++){
// 		  //	    printBDD((*j).second);
// 		  //	    printBDD(Cudd_addFindMax(manager, (*j).second));
// 		  //	    DdNode *x = Cudd_addApply(manager, Cudd_addTimes, (*j).second, dbop->vars[2*num_alt_facts+i]->cpt);
// 		  //	    Cudd_Ref(x);
// 		  //	    printBDD(x);
		  
// 		  cumm += dbop->vars[*p]->prs[(*j).first];//Cudd_addFindMax(manager, x)->type.value;
// 		  //Cudd_RecursiveDeref(manager, x);
// 		  //	    std::cout << "cumm = " << cumm << " val = " << dbop->vars[2*num_alt_facts+i]->prs[(*j).first] << " indx = " << (*j).first <<std::endl;
		  
// 		  if(cumm >= pr){

// 		    //  std::cout << "Got value " << (*j).first << " " << (*j).second << std::endl;
// 		    //printBDD((*j).second);  
// 		    //aux_value[i+2*num_alt_facts] = (*j).first;
// 		    DdNode* t = Cudd_addApply(manager, Cudd_addTimes, aux_value, (*j).second);
// 		    Cudd_Ref(t);
// 		    Cudd_RecursiveDeref(manager, aux_value);
// 		    aux_value = t;
// 		    Cudd_Ref(aux_value);
// 		    Cudd_RecursiveDeref(manager, t);
// 		    //printBDD(aux_value);
// 		    break;
// 		  }
// 		}
// 	      }
// 	      //  }	
	      
//	      aux_var_samples[*p] = aux_value;
	    }
	    DdNode *aux_sample = (*aux_var_samples)[*p];//(*avsi).second;
	    //	    DdNode *gar = Cudd_addApply(manager, Cudd_addTimes, aux_sample, aux_sample);
	    //Cudd_Ref(aux_sample);
	    //	     printBDD(aux_sample);
	    //printBDD(sample);
	    // Cudd_DisableGarbageCollection(manager);


	    DdNode* t = Cudd_addApply(manager, Cudd_addTimes, aux_sample, sample);
	    Cudd_Ref(t);
	    Cudd_Ref(t);
	    //Cudd_EnableGarbageCollection(manager);
	    Cudd_RecursiveDeref(manager, sample);
	    sample = t;
	    Cudd_Ref(sample);
	    Cudd_RecursiveDeref(manager, t);

	  }
	  //	  printBDD(sample);
	  //printBDD(e1->effect->row);
	  //Cudd_DisableGarbageCollection(manager);
	  //remove unsampled outcomes from label
	  DdNode * t1 = Cudd_addApply(manager, Cudd_addTimes, sample, e1->effect->row);
	  Cudd_Ref(t1);
	  //	  Cudd_Ref(t1);
	  //printBDD(t1);
	  //Cudd_EnableGarbageCollection(manager);
	  DdNode* t3 = Cudd_addBddThreshold(manager, t1, 1.0);
	  Cudd_Ref(t3);
	  Cudd_RecursiveDeref(manager, t1);





	  if(!all_but_sample_cube){
	  DdNode** vars = new DdNode*[2*num_alt_facts+max_num_aux_vars];
	  for(int i = 0; i < 2*num_alt_facts+max_num_aux_vars; i++){
	    vars[i] = Cudd_bddIthVar(manager, i);
	    Cudd_Ref(vars[i]);
	  }	    
	  all_but_sample_cube = Cudd_bddComputeCube(manager, vars, 0, 2*num_alt_facts+max_num_aux_vars);
	  Cudd_Ref(all_but_sample_cube);
	  //	  Cudd_Ref(cube);
	  for(int i = 0; i < 2*num_alt_facts+max_num_aux_vars; i++){
	    Cudd_RecursiveDeref(manager, vars[i]);
	  }
	  delete [] vars;
	  }

	  //	  printBDD(t1);


	  DdNode *t2 = Cudd_bddExistAbstract(manager, t3, all_but_sample_cube);
	  Cudd_Ref(t2);

	  Cudd_RecursiveDeref(manager, t3);

	  //Cudd_RecursiveDeref(manager, cube);
	  //printBDD(t2);

	 

	  DdNode* tmp = Cudd_bddAnd(manager, e1->info_at[time]->label->label, t2);
	  Cudd_Ref(tmp);
	  Cudd_RecursiveDeref(manager, t2);
	  //	  printBDD(tmp);
	  Cudd_RecursiveDeref(manager, e1->info_at[time]->label->label);
	  e1->info_at[time]->label->label = tmp;
	  Cudd_Ref(e1->info_at[time]->label->label);
	  Cudd_RecursiveDeref(manager, tmp);
	  
	 
	  

	}	
// 	for(std::map<int, DdNode*>::iterator i = aux_var_samples.begin(); i != aux_var_samples.end();i++){
// 	  Cudd_RecursiveDeref(manager, (*i).second);
// 	}
// 	aux_var_samples.clear();
      }
    }
  }
	else{
	for ( i1= gall_ops_pointer; i1; i1=i1->next ) {

		if(i1->info_at[time] &&
				!i1->is_noop){


			const Action *op = i1->action;
			dbn* dbop = action_dbn(*op);

			if(dbop->num_aux_vars == 0){
				continue;
			}

			//        std::cout << "Sampling Effects of " << std::endl;
			//        op->print(std::cout, my_problem->terms());
			//        std::cout << std::endl;






			double sample_offset[dbop->num_aux_vars];
			for(int i = 0; i < dbop->num_aux_vars; i++){
				sample_offset[i] = ((double)randomGen->myrand()/((double)(RAND_MAX)+(double)(1)));
			}

			// int sample_index = 0;
			//    for(std::list<DdNode*>::iterator s = new_samples.begin(); s != new_samples.end(); s++, sample_index++){
			for(int s = 0; s < NUMBER_OF_MGS; s++){

				DdNode* sd = make_sample_index(s);
				DdNode *tp = Cudd_bddIntersect(manager, sd, i1->info_at[time]->label->label);
				Cudd_Ref(tp);
				if(tp == Cudd_ReadLogicZero(manager)){
					//  std::cout << "Skipping sample" << std::endl;
					Cudd_RecursiveDeref(manager, tp);
					continue;
				}
				Cudd_RecursiveDeref(manager, tp);

				//std::cout << "Using sample" << std::endl;

				//sample each var
				//std::map<int, double> aux_value;
				DdNode* aux_value = Cudd_ReadOne(manager);
				Cudd_Ref(aux_value);
				for(int i = 0; i < dbop->num_aux_vars; i++){
					//std::cout << "Sampling aux var " << i << " of " << dbop->num_aux_vars << std::endl;
					double pr = ((double)s/NUMBER_OF_MGS)+sample_offset[i];
					//std::cout << "pr = " << pr << std::endl;
					if(pr > 1.0){
						pr -= 1.0;
					}
					double cumm = 0.0;
					//	  for(std::map<double,double>::iterator j = (*dbop->vars[2*num_alt_facts+i]->cpt.begin()).second->begin(); j != (*dbop->vars[2*num_alt_facts+i]->cpt.begin()).second->end(); j++){

					for(std::map<int, DdNode*>::iterator j = dbop->vars[2*num_alt_facts+i]->outcomes.begin();
							j != dbop->vars[2*num_alt_facts+i]->outcomes.end(); j++){
						//	    printBDD((*j).second);
						//	    printBDD(Cudd_addFindMax(manager, (*j).second));
						//	    DdNode *x = Cudd_addApply(manager, Cudd_addTimes, (*j).second, dbop->vars[2*num_alt_facts+i]->cpt);
						//	    Cudd_Ref(x);
						//	    printBDD(x);

						cumm += dbop->vars[2*num_alt_facts+i]->prs[(*j).first];//Cudd_addFindMax(manager, x)->type.value;
						//Cudd_RecursiveDeref(manager, x);
						//	    std::cout << "cumm = " << cumm << " val = " << dbop->vars[2*num_alt_facts+i]->prs[(*j).first] << " indx = " << (*j).first <<std::endl;

						if(cumm >= pr){
							//std::cout << "Got value " << (*j).first << " " << (*j).second << std::endl;

							//aux_value[i+2*num_alt_facts] = (*j).first;
							DdNode* t = Cudd_addApply(manager, Cudd_addTimes, aux_value, (*j).second);
							Cudd_Ref(t);
							Cudd_RecursiveDeref(manager, aux_value);
							aux_value = t;
							Cudd_Ref(aux_value);
							Cudd_RecursiveDeref(manager, t);
							//  printBDD(aux_value);
							break;
						}
					}
				}

				//mark effects that are sampled
				e1=i1->conditionals;
				for(; e1; e1=e1->next) {

					//printBDD(e1->effect->row);
					//printBDD(aux_value);
					DdNode * t1 = Cudd_addApply(manager, Cudd_addTimes,
							aux_value,
							e1->effect->row);
					Cudd_Ref(t1);
					//printBDD(t1);

					if(!e1->info_at[time] ||
							//!e1->effect->node ||
							t1 != Cudd_ReadZero(manager)){
						//e1->effect->node->row_matches_aux(e1->effect->row, &aux_value)){
						//   std::cout << "skip effect " << e1->index << std::endl;
						Cudd_RecursiveDeref(manager, t1);
						continue;
					}
					Cudd_RecursiveDeref(manager, t1);
					//printBDD(*s);
					//printBDD(e1->info_at[time]->label->label);
					DdNode *tp1 = Cudd_bddIntersect(manager, sd, e1->info_at[time]->label->label);
					Cudd_Ref(tp1);

					// 	  std::cout << "Checking intersect with conditional " << e1->index << std::endl;
					// 	  printBDD(*s);
					// 	  printBDD(e1->info_at[time]->label->label);
					// 	  printBDD(tp1);

					if(tp1 != Cudd_ReadLogicZero(manager)){
						// std::cout << "Marking not sample of effect " << e1->index << std::endl;
						//	    printBDD(*s);
						//effect is not enabled for sample, so remove sample from
						//its label
						DdNode* f = Cudd_Not(sd);
						Cudd_Ref(f);
						DdNode* tmp = Cudd_bddAnd(manager,
								e1->info_at[time]->label->label,
								f);
						Cudd_Ref(tmp);
						Cudd_RecursiveDeref(manager, f);
						Cudd_RecursiveDeref(manager, e1->info_at[time]->label->label);
						e1->info_at[time]->label->label = tmp;
						Cudd_Ref(e1->info_at[time]->label->label);
						Cudd_RecursiveDeref(manager, tmp);
					}
					else{
						// std::cout << "NOT Marking not sample of effect " << e1->index << std::endl;
					}
					Cudd_RecursiveDeref(manager, tp1);
				}
				Cudd_RecursiveDeref(manager, aux_value);
			}

		}
	}
	}
	//    Cudd_CheckKeys(manager);
	//    std::cout << "]"<<std::endl;
}

void sample_proposal(int time){
	OpNode *i1;
	EfNode *e1;

	if(!NDACTIONS)
		return;

	//    return;

	//  std::cout << "Hi " << std::endl;


	//for each sample world from previous layer
	for(int s = 0; s < NUMBER_OF_MGS; s++){
		DdNode *s1 = make_sample_index(s);
			//std::list<DdNode*>::iterator s = new_samples.begin(); s != new_samples.end(); s++){
		// std::cout << "Transitioning sample:" <<std::endl;
		// printBDD(*s);

		if(0 && bdd_entailed(manager, s1, goal_samples))
			continue;


		for ( i1= gall_ops_pointer; i1; i1=i1->next ) {
			DdNode *tp = Cudd_bddIntersect(manager, s1, i1->info_at[time]->label->label);
			Cudd_Ref(tp);
			if(i1->info_at[time] &&
					!i1->is_noop &&
					tp != Cudd_ReadLogicZero(manager)){
				const Action *op = i1->action;
				OutcomeSet* outcomes = action_outcomes[op];
				int num_outcomes = outcomes->probabilities.size();
				if(num_outcomes < 2)
					continue;



				// sample outcome of each action applicable in sampled world
				//double sn = ((double)rand()/((double)(RAND_MAX)+(double)(1)));
				double sn = ((double)randomGen->myrand()/((double)(RAND_MAX)+(double)(1)));

				//std::cout << "Sampling outcomes of " << i1->name  << " sample: " << sn << std::endl;
				double cumm = 0.0;
				for(int i = 0; i < num_outcomes; i++){
					cumm += outcomes->probabilities[i].double_value();
					if(cumm >= sn){
						// 	    std::cout << "got outcome " << i  << " pr = "
						//  		 << outcomes->probabilities[i].double_value() <<std::endl;

						//label each effect in outcome
						e1=i1->conditionals;
						for(; e1; e1=e1->next) {
							DdNode *tp1 = Cudd_bddIntersect(manager, s1, e1->info_at[time]->label->label);
							Cudd_Ref(tp1);
							if(e1->info_at[time] &&
									tp1 != Cudd_ReadLogicZero(manager)){
								// 		std::cout << "outcome " << e1->effect->outcome <<std::endl;
								if(e1->effect->outcome->find(i) != e1->effect->outcome->end()){//get_bit(e1->effect->outcome, ((int)num_outcomes/gcword_size)+1, i)){
									//effect is enabled for sample already, keep it
								}
								else{
									//effect is not enabled for sample, so remove sample from
									//its label

									DdNode* tmp = Cudd_bddAnd(manager,
											e1->info_at[time]->label->label,
											Cudd_Not(s1));
									Cudd_Ref(tmp);
									Cudd_RecursiveDeref(manager, e1->info_at[time]->label->label);
									e1->info_at[time]->label->label = tmp;
									Cudd_Ref(e1->info_at[time]->label->label);
									Cudd_RecursiveDeref(manager, tmp);
								}
								//printBDD(e1->info_at[time]->label->label);
							}
							Cudd_RecursiveDeref(manager, tp1);
						}


						break;
					}
				}


			}
			Cudd_RecursiveDeref(manager, tp);
		}
	}


}

// DdNode* weight_samples(DdNode* samples, int time){

// }

// DdNode* resample(DdNode* samples, DdNode* weights){
// }

//select which worlds to continue propagating
//algo 3 from Arulampalam tutorial
void particle_filter_layer(int time){

	//  std::cout << "Particle Filtering" <<std::endl;

	//construct and sample proposal distribution
	//DdNode* samples =

	if(DBN_PROGRESSION){
		sample_proposal_DBN(time);
	}
	else{
		sample_proposal(time);
	}
	//Cudd_Ref(samples);

	//   //weight particles, incl. normalization
	//   DdNode* weights = weight_samples(samples, time);
	//   Cudd_Ref(weights);

	//   //resample if necessary
	//   DdNode* resamples = resample(samples, weights);
	//   Cudd_Ref(resamples);

}

#endif
/**
 * momo007 2022.09.27 this maybe contains bugs about the NDLabel
 */
void mark_effect_lables(int time){
	OpNode *i1;
	EfNode *e1;
	FtNode *f1;
	FtEdge *pres;
	int ndnum, ndnum1, ndnum2, k, ndlow, ndhigh;
	int condnum, effType, ok;
	Label* tmp = NULL;
	Label* tmp1 = NULL;
	DdNode* tmpD, *tmpD1, *allEffForm, *condEffForm, *fr, *oldLabel;
	Consequent* conseq, *conseq1;
	Label* newNonDeter;
	Label* tmpnewNonDeter;
	Label* precond_labels;
	int DONDLABELS;	
	i1 = gall_ops_pointer;



	//Need to loop through effects and collect new labels
	//created by non-deterministic effects


	ndnum1 = 0;
	//for all ops
	for (; i1; i1=i1->next ) {
		if(i1->info_at[time] && i1->info_at[time]->updated) {
      		precond_labels = i1->info_at[time]->label;// 获取OpNode的前提
    		for(int t = 0; t < 2; t++){// 利用循环处理uncon和cond eff
				if(t == 0)
	  				e1=i1->unconditional;
				else
	  				e1=i1->conditionals;
				for(; e1; e1=e1->next) {
	  				if(e1->info_at[time]){
	    	      		// printf("Getting label for eff of %s, outcome:  %d \n", i1->name, e1->effect->outcome);
						if(e1->info_at[time]->is_dummy){
	      					// printf("uncond is dummy\n");
// #ifdef PPDDL_PARSER
	      					// e1->info_at[time]->label = new_Label(Cudd_ReadZero(manager), 0);
// #else
	      					e1->info_at[time]->label = new_Label(Cudd_ReadLogicZero(manager), 0);

//#endif
	    				}
	    				else if(e1->effect){
	      					conseq = e1->effect->cons;
	      					ndnum = 0;
	    					tmp1 = NULL;
	    					tmp = NULL;
	      					while(conseq){
								if(e1->info_at[time]->label){
		  							oldLabel = e1->info_at[time]->label->label;
								}
								else
		  							oldLabel = Cudd_ReadLogicZero(manager);
								Cudd_Ref(oldLabel);


								if(t == 0) {//uncond
		  		    				// std::cout << "uncond" <<std::endl;
		  							tmp = new_Label(fr=precond_labels->label, 0);//new_Label(and_labels(precond_labels->label, e1->conditions, time));
		  							// Cudd_Ref(fr);
								}
								else{ //cond
									fr = and_labels(precond_labels->label, e1->conditions, time);
									Cudd_Ref(fr);
									// printBDD(fr);
									tmp = new_Label(fr, 0);
									Cudd_RecursiveDeref(manager, fr);
								}
								if(e1->is_nondeter) {
		  							ndnum1++;
								}
								Cudd_Ref(tmp->label);
		
								if(ndnum == 0){// 确定性effect
		  						//for some reason this first case needs
		  						//to always be entered if not doing incrm
		  						//sag -- shoudl be fixed.
									if(LUG_FOR != INCREMENTAL || !e1->info_at[time]->label){
										e1->info_at[time]->label = tmp;
										tmp1=tmp;
										tmp->next = NULL;
									}
									else if(e1->info_at[time]->label->label == tmp->label){
										e1->info_at[time]->updated = 0;
									}
		  							else{
		    							if(PF_LUG){//make sure that don't add samples back
		    								tmpD = Cudd_bddOr(manager, oldLabel, new_sampleDD);
		    								Cudd_Ref(tmpD);
		    								tmpD1 = Cudd_bddAnd(manager, tmp->label, tmpD);
		    								Cudd_Ref(tmpD1);
		    								Cudd_RecursiveDeref(manager, tmp->label);
		    								Cudd_RecursiveDeref(manager, tmpD);
		    								tmp->label = tmpD1;
		    								Cudd_Ref(tmp->label);
		      								Cudd_RecursiveDeref(manager, tmpD1);
										}
		    
										e1->info_at[time]->label->label = tmp->label;
										Cudd_Ref(e1->info_at[time]->label->label);
										delete tmp;
									}
		  
		  
		  //  		    e1->info_at[time]->label = tmp;
		  //  		    tmp1=tmp;
		  //  		    tmp->next = NULL;
								}
								else {// 非确定性effect
									tmp1->next = tmp;
									tmp1 = tmp;
									tmp->next = NULL;
								}
								if(!e1->is_nondeter) break;
								conseq=conseq->next;// 非确定effect，继续遍历conseq
								ndnum++;
							}// end-while
						}
					}
				}
			}
		}
	}
    // std::cout << "HI"<<std::endl;
  	// process Non-deterministic labels
#ifdef PPDDL_PARSER
	if(my_problem->domain().requirements.probabilistic_effects){
    	if(PF_LUG){
      		particle_filter_layer(time);
    	}
    	else{
      		mark_probabilitic_effect_labels(time);
    	}
	}
#else
  
  DONDLABELS = //TRUE;//
    FALSE;
  if(COMPUTE_LABELS && DONDLABELS){
    ndnum = (2*num_alt_facts)+(NDLcounter-ndnum1);
    
    for ( i1=gall_ops_pointer; i1; i1=i1->next ) {
      if(!i1->nonDeter)
	continue;

      //  printf("Getting labels for nds of %s \n", i1->name);
      
      allEffForm = Cudd_ReadLogicZero(manager);
      Cudd_Ref(allEffForm);
      ok = FALSE;
      
      // for(effType = 0; effType < 2; effType++){
      //  if(!effType)
      e1 = i1->unconditional;
      
      if(e1->is_nondeter)
	ok = TRUE;
      
      for(; e1; e1=e1->next) {
	if(!e1->info_at[time]->is_dummy){
	  conseq = e1->effect->cons;
	  while(conseq){
	    allEffForm = Cudd_bddOr(manager, fr=allEffForm, conseq->b);
	    Cudd_Ref(allEffForm);
	    Cudd_RecursiveDeref(manager, fr);
	    // printBDD(allEffForm);
	    // printBDD(conseq->b);
	    conseq = conseq->next;
	  }
	}
      }
      
      
      //   else
      e1 = i1->conditionals;
      condEffForm = Cudd_ReadLogicZero(manager);
      Cudd_Ref(condEffForm);
      if(e1 && e1->is_nondeter)
	ok = TRUE;
      
      for(; e1; e1=e1->next) {
	if(!e1->info_at[time]->is_dummy){
	  conseq = e1->effect->cons;
	  while(conseq){
	    //	  printBDD(conseq->b);
	    condEffForm = Cudd_bddOr(manager, fr=condEffForm, conseq->b);
	    Cudd_Ref(condEffForm);
	    Cudd_RecursiveDeref(manager, fr);
	    conseq = conseq->next;
	  }
	}
      }
      
      allEffForm = Cudd_bddOr(manager, fr=allEffForm, condEffForm);
      Cudd_Ref(allEffForm);
      Cudd_RecursiveDeref(manager, fr);
			

      // printf("ok = %d\n", ok);

      //printBDD(allEffForm);
      
      if(ok)
	getNDLabels(i1, allEffForm, time);
      
    }
  }
#endif
	// printf("----------DONE w/ effect lables-------------\n");

}

void cost_prop_facts(int time){
	int k;
	int j;
	FtNode *i1;
	int cost;
	DdNode* tmp;
	std::list<LabelledElement*>::iterator i;
	std::list<std::list<LabelledElement*>*>::iterator p;

	//  printf("COSTPR facts, time = %d\n", time);fflush(stdout);
	//   Cudd_CheckKeys(manager);fflush(stdout);
	//   for(j = 0; j <num_alt_facts;j++)
	//     printFact(gft_table[j]->index);

	addCostLevel(2);
	for ( k=0; k < num_alt_facts; k++ ) {
		//printFact(k); std::cout << " k = " << k << std::endl;
		for(j=0; j < 2; j++){
			if(j == 1)
				i1 = gft_table[k];
			else
				i1 = gft_table[NEG_ADR(k)];
			//std::cout << " j = " << j << std::endl;

			if(i1 && i1->info_at[time] && !i1->info_at[time]->updated){
				if(i1->positive)
					copyCostLevelUp(2, i1->index);
				else
					copyCostLevelUp(2, i1->index+num_alt_facts);
			}

			if(i1 && i1->info_at[time] && i1->info_at[time]->updated){
				if(time > 0 && i1->info_at[time-1] && !i1->info_at[time-1]->is_dummy){
					if(COST_PROP_TECH == CPT_FIXED_GROUPING_NEW_WORLDS){
						if(j == 1)
							p = factWorldCost[i1->index]->begin();
						else
							p = factWorldCost[i1->index+num_alt_facts]->begin();
						for(int l = 0; l < time-1; l++) p++;
						//track new cost for each i
						for(i = (*p)->begin(); i!= (*p)->end(); i++){
							double old_cost = (j == 1 ?
									getFactWorldCost(time-1, i1->index, (*i)->label)
									:
									getFactWorldCost(time-1, i1->index+num_alt_facts, (*i)->label));
							if(old_cost == 0)
								cost = 0;
							else
								cost = getFactCover(i1, (*i)->label, time);


							// 	      if(old_cost < cost){
							// 		std::cout << "cost increase " << cost-old_cost << std::endl;
							// 		exit(0);
							// 	      }



							// 	      int noop =0;
							// 	      for(EfEdge* add = i1->adders; add; add=add->next){
							// 		if(add->ef->op->is_noop)
							// 		  noop = 1;
							// 	      }
							// 	      if(!noop){
							// 		std::cout << "No noop"<<std::endl;
							// 		exit(0);
							// 	      }


							//	      std::cout << "fact cost = " << cost << std::endl;
							// 	      printBDD((*i)->label);
							if(j == 1)
								setFactWorldCost(time, i1->index, cost, (*i)->label);
							else
								setFactWorldCost(time, i1->index+num_alt_facts, cost, (*i)->label);
						}
					}
					tmp = Cudd_bddAnd(manager,
							Cudd_Not(i1->info_at[time-1]->label->label),
							i1->info_at[time]->label->label);
				}
				else{
					tmp = i1->info_at[time]->label->label;
				}
				Cudd_Ref(tmp);
				//	tmp = bdd_and(manager, tmp, b_initial_state);
				if(i1->positive){
					if(COST_PROP_TECH == CPT_NEW_WORLDS_ONLY){
						copyCostLevelUp(2, i1->index);
					}
				}
				else{
					if(COST_PROP_TECH == CPT_NEW_WORLDS_ONLY){
						copyCostLevelUp(2, i1->index+num_alt_facts);
					}
				}
				if(tmp != Cudd_ReadLogicZero(manager)){
					// printBDD(tmp);
					if(COST_PR_INDIV && PF_LUG){
//						for(std::list<DdNode*>::iterator s = new_samples.begin();
//								s != new_samples.end(); s++){
//							if(bdd_entailed(manager, *s, tmp)){
//								cost = getFactCover(i1, *s, time);
//								if(i1->positive){
//									setFactWorldCost(time, i1->index, cost, *s);
//								}
//								else{
//									setFactWorldCost(time, i1->index+num_alt_facts, cost, *s);
//								}
//							}
//						}
					}
					else{
						cost = getFactCover(i1, tmp, time);
						//if(cost >= 0){
						// printf("cost = %d \n", cost);
						if(i1->positive){
							setFactWorldCost(time, i1->index, cost, tmp);
						}
						else{
							setFactWorldCost(time, i1->index+num_alt_facts, cost, tmp);
						}
						//}
					}
				}
			}
		}
		// Cudd_RecursiveDeref(manager, tmp);
	}
	//   printf("exit COSTPR facts\n");fflush(stdout);
	//   Cudd_CheckKeys(manager);fflush(stdout);
	//   std::cout << "HI"<<std::endl << flush;
}

void mark_fact_labels(int time) {
	int k;
	int j;
	FtNode *i1;
	EfEdge *e1;
	EfEdge* tmp_adder;
	DdNode* tmp, *tmp1,*fr;
	int once = TRUE;
	Consequent* tmpC, *tmpC1;
	Label *lab, *lab1;
	// printf("-------Marking Fact Labels at time: %d -------------------\n", time);

	for ( k=0; k < gnum_relevant_facts; k++ ) {


		for(j=0; j < 2; j++) // 考虑 pos和neg
		{

			if(j == 1)
				i1 = gft_table[k];
			else
				i1 = gft_table[NEG_ADR(k)];
			if(i1 && i1->info_at[time] && i1->info_at[time]->updated)
			{

				if(i1->info_at[time]->is_dummy)
				{
					i1->info_at[time]->label = new_Label(fr=Cudd_ReadLogicZero(manager), 0);
					Cudd_Ref(fr);
				}// end-dummy fact
				else
				{



					tmp = or_labels(i1, time-1);
					Cudd_Ref(tmp);
					//std::cout << i1->index << std::endl;
					//printBDD(tmp);

					if(verbosity >3){
						/////print pr of label////////


						if(!PF_LUG){
							DdNode *addlab = Cudd_BddToAdd(manager, tmp);
							Cudd_Ref(addlab);
							DdNode *prlab = Cudd_addApply(manager,Cudd_addTimes, addlab, probabilisticLabels);
							Cudd_Ref(prlab);
							Cudd_RecursiveDeref(manager, addlab);
							std::cout << "pr = " << exAbstractAllLabels(prlab, time-1) << std::endl;
							Cudd_RecursiveDeref(manager, prlab);
						}
						else{
							printBDD(tmp);
							std::cout << " pr = " << (countParticles(tmp)/NUMBER_OF_MGS) <<std::endl;
						}
						/////print pr of label////////
					}


					if(!i1->info_at[time]->label){
						//std::cout << "got dd" <<std::endl;
						i1->info_at[time]->label = new_Label(tmp,0);
					}
					else if(i1->info_at[time]->label->label == tmp){
						i1->info_at[time]->updated = 0;
					}
					else{
						i1->info_at[time]->label->label = tmp;
						Cudd_Ref(i1->info_at[time]->label->label);
					}

					//	  i1->info_at[time]->label = new_Label(tmp, 0);

					//	  Cudd_RecursiveDeref(manager, tmp);

				}// end not dummy fact
			}// end-if
		}// end-inner for
	}// end-outter for

	//   //  printf("Getting loop\n");
	//   //get loopy labels, i.e. or in all labels of non-deter effect consequnets
	//   //that do not give this fact, but give a fact that supports the execution
	//   //of the effect.
	//   for ( k=0; k < gnum_relevant_facts; k++ ) {
	//     for(j=0; j < 2; j++){
	//       if(j == 1){

	// 	Cudd_Ref(tmp);
	// 	i1 = gft_table[k];
	// 	tmp = Cudd_bddIthVar(manager, k);
	//       }
	//       else{
	// 	Cudd_Ref(tmp);
	// 	i1 = gft_table[NEG_ADR(k)];
	// 	tmp = Cudd_Not(Cudd_bddIthVar(manager, k));
	//       }


	//       if(time > 0 && i1 && i1->adders && i1->info_at[time] && !i1->info_at[time]->is_dummy){
	// 	//	printf("Loopy Marking Fact %d %d ",i1->index, j); printFact(i1->index); printf("\n");
	// 	//	printBDD(i1->info_at[time]->label->label);
	// 	tmp_adder = i1->adders;

	// 	while(tmp_adder){
	// 	  if(tmp_adder->ef && tmp_adder->ef->info_at[time-1] && !tmp_adder->ef->info_at[time-1]->is_dummy){
	// 	    //    printf("check adder, for op = %s\n", tmp_adder->ef->op->name);
	// 	  if(tmp_adder->ef->is_nondeter){
	// 	    //    printf("is nondeter\n");
	// 	    tmpC = tmp_adder->ef->effect->cons;
	// 	    lab = tmp_adder->ef->info_at[time-1]->label;
	// 	    while(tmpC){
	// 	      //  printBDD(tmpC->b);
	// 	      //printBDD(tmp);
	// 	      if(bdd_entailed(manager, tmpC->b, tmp)){
	// 		tmpC1 = tmp_adder->ef->effect->cons;
	// 		lab1 = tmp_adder->ef->info_at[time-1]->label;
	// 		while(tmpC1){
	// 		  if(tmpC1!=tmpC){
	// 		    if(supportsAdder(tmpC1, tmp_adder->ef, time)/*  && */
	// /* 		       canSenseDifference(tmpC1, tmpC, time) */){
	// 		      //		      i1->info_at[time]->label->label = bdd_or(manager,  i1->info_at[time]->label->label, lab1->label);
	// 		      //  printBDD(lab1->label);
	// 		      tmp1 = bdd_or(manager,  i1->info_at[time]->label->label, lab1->label);
	// 		      Cudd_Ref(tmp1);
	// 		      Cudd_RecursiveDeref(manager, i1->info_at[time]->label->label);
	// 		      i1->info_at[time]->label->label = tmp1;
	// 		      Cudd_Ref(i1->info_at[time]->label->label);
	// 		      Cudd_RecursiveDeref(manager,tmp1);
	// 		    }
	// 		  }
	// 		  tmpC1=tmpC1->next;
	// 		  lab1=lab1->next;
	// 		}
	// 	      }
	// 	      tmpC = tmpC->next;
	// 	      lab = lab->next;
	// 	    }
	// 	  }
	// 	  }
	// 	  tmp_adder = tmp_adder->next;
	// 	}
	// 	//	printBDD( i1->info_at[time]->label->label );
	//       }
	//       Cudd_RecursiveDeref(manager, tmp);
	//     }
	//   }


	//  printf("---`-------DONE w/ fact lables-------------\n");
}


//return true if can trace back through add to find all facts in
//con
int supportsAdder(Consequent* con, EfNode* add, int time){
	//DdNode* mySupport = Cudd_ReadLogicZero(manager);
	int i, j, k;
	DdNode* tmp;
	EfNode* tmp_adder;
	Integers *ind1;

	//Cudd_Ref(mySupport);

	//  printf("enter supportsAdder, for op = %s, ef = %d\n", add->op->name, add->index);

	/*   printBDD(con->b); */
	//  need to find a way to support add by using the facts that don't
	//  conflict with con
	//  see if con exists at previous level, and is not mutex with add's
	//  preconds

	//  print_fact_info( con->p_effects, gft_vector_length);
	for(ind1=con->p_effects->indices; ind1; ind1=ind1->next){
		//    printf("ind1 = %d\n", ind1->index);
		if(gft_table[ind1->index] == NULL ||
				!gft_table[ind1->index]->info_at[time-1] ||
				gft_table[ind1->index]->info_at[time-1]->is_dummy
				/*|| mutex with add->pre*/){
			return FALSE;
		}
	}
	//printf("POS OKAY\n");
	for(ind1=con->n_effects->indices; ind1; ind1=ind1->next){
		if(gft_table[ind1->index] == NULL ||
				!gft_table[ind1->index]->info_at[time-1] ||
				gft_table[NEG_ADR(ind1->index)]->info_at[time-1]->is_dummy
				/*|| mutex with add->pre*/)
			return FALSE;
	}
	//printf("OKAY\n");





	return TRUE;
}


/*******************************************************
 * MAIN FUNCTION: ADD AN ADDTIONAL LAYER INFO TO GRAPH *
 *******************************************************/


void print_mutex_ops(int time){

	OpNode* tmp = gall_ops_pointer;
	printf("Op Mutexes for level %d \n", time);
	for(;tmp/*!= gprev_level_ops_pointer*/; tmp = tmp->next){
		printf("\n%s %d %d has these mutexes", tmp->name, tmp->index, tmp->uid_mask);
		//       print_BitVector(tmp->uid_mask, 1);
		print_BitVector(tmp->info_at[time]->exclusives->exclusives, gop_vector_length_at[time]);
		printf("\n");

	}
	printf("Done Mutexes for ops\n");
}
void print_mutex_effects(int time){

	EfNode* tmp = gall_efs_pointer;
	printf("Ef Mutexes for level %d \n", time);
	for(;tmp/*!= gprev_level_efs_pointer*/; tmp = tmp->all_next){
		printf("\n%d:%s, eff %d has these mutexes", tmp->index, tmp->op->name, tmp->index);
		print_BitVector(tmp->info_at[time]->exclusives->exclusives, gef_vector_length);
		printf("\n");

	}
	printf("Done Mutexes for efs\n");

}

void print_mutex_facts(int time){
	FtNode* tmp = gall_fts_pointer;
	printf("Ft Mutexes for level %d \n", time);
	for(;tmp/*!= gprev_level_fts_pointer*/; tmp = tmp->next){
		printFact(tmp->index);
		printf(" %d %d has these mutexes\n",tmp->index, tmp->positive);
		if(tmp->info_at[time])
			printf("HO\n");
		print_BitVector(tmp->info_at[time]->exclusives->pos_exclusives, gft_vector_length);
		if(tmp->info_at[time]->exclusives->neg_exclusives)
			print_BitVector(tmp->info_at[time]->exclusives->neg_exclusives, gft_vector_length);
		printf("\n");
	}
	printf("Done Mutexes for fts\n");

}






void build_graph_evolution_step( void )
{
	// printf(".");fflush(stdout);
	static int time = 0;// 仅第一次调用为0
	int facts_count = gfacts_count, exclusions_count = gexclusions_count;
	BitOperator *o, *prev, *tmp;
	OpNode *op, *prev_op, *tmp_op, *i;
	EfNode *j;
	FtNode *l;
	int k;

	struct tms start, end;
	//if(  Cudd_DebugCheck(manager)){  std::cout << "DEBUG PROBLEM " << Cudd_ReadDead(manager)  <<std::endl;     }

	//  printf("enter build graph evo step \n");
	/* when replanning due to rifo failure, 
		set time back to zero in first function call */
	if ( new_plan )
	{
		// printf("NEW GRAPH\n");
		time = 0;
		new_plan = FALSE;
	}

	// printf("BUILD EVO STEP %d \n", time); fflush(stdout);
	// Cudd_CheckKeys(manager);
	// 赋值t到t+1层次，假设所有的effect没有变化
	gpos_facts_vector_at[time+1] = copy_bit_vector( gpos_facts_vector_at[time],
			gft_vector_length );
	gneg_facts_vector_at[time+1] = copy_bit_vector( gneg_facts_vector_at[time],
			gft_vector_length );

	// momo007 2022.09.16 initially  gall_ops_pointer is null, it always empty?
	for ( i = gall_ops_pointer; i; i = i->next ) {
		i->info_at[time] = new_op_level_info();//创建op level info

		i->unconditional->info_at[time] = new_ef_level_info(time);
		// 考虑每个无条件effect
		for ( j = i->conditionals; j; j = j->next ) {
			// 配置level信息
			j->info_at[time] = new_ef_level_info(time);
			if ( j->info_at[time-1]->is_dummy ) {// 传递dummy信息
				/* effekte bleiben erstmal dummy...
				 * spaeter dann nachschauen, ob sie doch eingezogen werden koennen.
				 *
				 * ( kann sein, dass effekt erst drei runden spaeter nicht-dummy wird )
				 */
				j->info_at[time]->is_dummy = TRUE;
			}
		}
	}
	// 为第2层开始，配置op和effect level info
	for ( i = gall_ops_pointer; (i && time > 0); i = i->next ) {
		// 上一层设置了level info且更新，当前层设置level info和更新
		if(i->info_at[time-1]  && i->info_at[time-1]->updated)
		{
			// 当前层level-info为空	
			if(!i->info_at[time])
			{
				i->info_at[time] = new_op_level_info();
				i->info_at[time]->updated = 1;
			}
			// 当前unconditional的level-info为空
			if(i->unconditional &&
					i->unconditional->info_at[time-1] &&
					i->unconditional->info_at[time-1]->updated &&
					!i->unconditional->info_at[time])
			{

				i->unconditional->info_at[time] = new_ef_level_info(time);
				i->unconditional->info_at[time]->updated = 1;
			}
			// 考虑当前的每个condtion，当前level-info为空
			for ( j = i->conditionals; j; j = j->next )
			{
				if(j->info_at[time-1] &&
						j->info_at[time-1]->updated &&
						!j->info_at[time])
				{
					j->info_at[time] = new_ef_level_info(time);
					j->info_at[time]->updated = 1;
					// 传递dummy
					if ( j->info_at[time-1] &&
							j->info_at[time-1]->is_dummy &&
							j->info_at[time] ) {
						/* effekte bleiben erstmal dummy...
						 * spaeter dann nachschauen, ob sie doch eingezogen werden koennen.
						 *
						 * ( kann sein, dass effekt erst drei runden spaeter nicht-dummy wird )
						 */
						j->info_at[time]->is_dummy = TRUE;
					}
				}
			}
		}
	}// end-for opNode configuration


	/* eigentlich ineffizient; unterscheidung wegen dummys noetig, die beim
	 * ersterzeugen NICHT in die globale liste aufgenommen werden.
	 *
	 * EFFIZIENTER: extra verkettung fuer dummys erstellen.
	 */
	// config the fact node in next level
	for ( k=0; k<2*gnum_relevant_facts; k++ ) {
		if ( (l = gft_table[k]) == NULL ) continue;
		// 创建fact的level info
		l->info_at[time+1] = new_ft_level_info( l );
		l->info_at[time+1]->updated = 1;
		// 传递下一层dummy
		if ( l->info_at[time]->is_dummy ) {
			/* siehe oben...
			 *
			 * facts, die hier SO markiert sind, sind NUR DIEJENIGEN, DIE VON
			 * EINEM DUMMY EFFEKT GEADDET wurden. also einfach markierung wieder
			 * umstellen, sobald der entsprechende dummy effekt richtig einziehbar
			 * wird!
			 */
			l->info_at[time+1]->is_dummy = TRUE;
		}
	}




	// 将当前的op和effec Node作为上一层
	gprev_level_ops_pointer = gall_ops_pointer;
	gprev_level_efs_pointer = gall_efs_pointer;
	//    if(MUTEX_SCHEME!=MS_REGULAR && COMPUTE_LABELS==TRUE){
	//      gprev_level_efs_pointer = gall_efs_pointer;
	//    }
	//     printf("@@@@@@@@@@@@@@@@@@@@APPLYING NOOPS@@@@@@@@@@@@@@@@@@@@@@\n");
	// printf("");
	apply_noops( time );



	gprev_level_fts_pointer = gall_fts_pointer;// gall_fts_pointer一开始在build_graph中初始化了




	//	printf("@@@@@@@@@@@@@@@@@@@@APPLYING OPERATORS@@@@@@@@@@@@@@@@@@@@\n");fflush(stdout);

	// printf("@@@@@@@@@@@@@@@@@@@@APPLYING OPS w/ unact effs@@@@@@@@@@@@@@@@@@@@\n");
	// 先处理含未激活effect的op
	op = gops_with_unactivated_effects_pointer;
	while ( op && apply_all_effects( time, op ) ) {

		tmp_op = op;
		op = op->thread;// 继续处理unactivated ops(在apply_operator中用thread构建了链表)
		tmp_op->thread = NULL;// remove from list
	}

	// printf("@@@@@@@@@@@@@@@@@@@@APPLYING OPS w/ unact effs again@@@@@@@@@@@@@@@@@@@@\n");
	gops_with_unactivated_effects_pointer = op;
	prev_op = op;// 第一个处理失败的op list
	if ( op ) op = op->thread;
	while ( op ) {
		if ( apply_all_effects( time, op ) ) {// 处理成功
			prev_op->thread = op->thread;// 移除该op
			tmp_op = op;
			op = op->thread;// 考虑下一个
			tmp_op->thread = NULL;// 处理成功，取消thread
		} else {// 处理失败
			prev_op = prev_op->thread;// 保留在list中
			op = op->thread;// 考虑下一个unactivated
		}
	}

	// printf("@@@@@@@@@@@@@@@@@@@@APPLYING OPS w/ effs@@@@@@@@@@@@@@@@@@@@\n");

	o = gbit_operators;// 在initLUG中创建了gbit_operators
	while ( o && apply_operator( time, o ) ) {/*momo007 2022.09.16 apply_operator有bug或operator创建本生有bug*/
		tmp = o;
		o = o->next;// gbit_operator中移除加入到 used_gbit_operators
		gbit_operators = o;
		tmp->next = used_gbit_operators;
		used_gbit_operators = tmp;
		//    /*    printf("op1\n"); */
		// /*     exit(0); */

		//     // free_partial_operator( tmp );

	}

	// printf("@@@@@@@@@@@@@@@@@@@@APPLYING OPS w/ effs2@@@@@@@@@@@@@@@@@@@@\n");

	gbit_operators = o;

	prev = o;// 处理失败的BitOperator

	if ( o ) o = o->next;

	while ( o ) {
		if ( apply_operator( time, o ) ) {// 处理成功
			prev->next = o->next;// 移除该op
			tmp = o;
			o = o->next;// 下次处理下一个

			tmp->next = used_gbit_operators;// 处理成功的op加入到used
			used_gbit_operators = tmp;
			/*  printf("op2\n"); */
			/*      exit(0); */
			//free_partial_operator( tmp );
		} else {// 处理失败

			if(!o->valid){// invalid,说明redundant
				prev->next = o->next;// 移除
				o = o->next;
				//free_partial_operator( o );

			}
			else{// 不是重复
				prev = prev->next;// 保留在gbit_operator
				o = o->next;
			}
		}
	}

	update_actions_effects_facts(time);

	// 设置 BitVector的长度,有余数需要考虑+1
	gop_vector_length_at[time] = ( ( int ) gops_count / gcword_size );
	if ( ( gops_count % gcword_size ) > 0 ) gop_vector_length_at[time]++;

	if(MUTEX_SCHEME!=MS_REGULAR && MUTEX_SCHEME!=MS_STATIC){

		gef_vector_length_at[time] = ( ( int ) gnum_cond_effects_pre / gcword_size );
		if ( ( gnum_cond_effects_pre % gcword_size ) > 0 ) gef_vector_length_at[time]++;
	}
	/**
	 * 检查所有适用的ops，判断effect是否是dummy.
	 * 修改effect和添加的fact
	 */
	if(COMPUTE_LABELS) {
	  //	  printf("@@@@@@@@@@@@@@@@@@@@MARKING OPERATOR LABELS@@@@@@@@@@@@@@@@@@@@\n");fflush(stdout);
		mark_operator_labels(time);
		if(!COST_REPROP &&  RP_EFFECT_SELECTION != RP_E_S_COVERAGE){
			cost_prop_ops(time);
			//               printCosts(0, time);
		}
	}

	// printf("@@@@@@@@@@@@@@@@@@@@INTEGRATING EFFECTS@@@@@@@@@@@@@@@@@@@@\n");
	// integrate_potential_effects( time );

	times(&start);



	if(MUTEX_SCHEME!=MS_NONE){
		// printf("@@@@@@@@@@@@@@@@@@@@FINDING MUTEX OPS@@@@@@@@@@@@@@@@@@@@\n");
		find_mutex_ops( time );// 查找所有的mutex ops pair?
	}
	// print_mutex_ops(time);

	/***
	 * momo007 这里先进行label标记，在查找label和下面情况相反
	 */
	if(MUTEX_SCHEME!=MS_REGULAR &&
			MUTEX_SCHEME!=MS_STATIC &&
			COMPUTE_LABELS==TRUE){
		// printf("@@@@@@@@@@@@@@@@@@@@INSERTING EFFECTS@@@@@@@@@@@@@@@@@@@@\n");
		// insert_potential_effects( time );
		if(COMPUTE_LABELS) {
			//dan
		  	// printf("@@@@@@@@@@@@@@@@@@@@MARKING EFFECT LABELS@@@@@@@@@@@@@@@@@@@@\n");fflush(stdout);

			mark_effect_lables(time);
			if(!COST_REPROP &&  RP_EFFECT_SELECTION != RP_E_S_COVERAGE){
				cost_prop_efs(time);
				// printCosts(1, time);
			}
		}

		if(COMPUTE_LABELS) {
			// printf("@@@@@@@@@@@@@@@@@@@@MARKING FACT LABELS@@@@@@@@@@@@@@@@@@@@\n");fflush(stdout);
			mark_fact_labels(time+1);
			//nad
			if(!COST_REPROP &&  RP_EFFECT_SELECTION != RP_E_S_COVERAGE){
				cost_prop_facts(time+1);
				// printCosts(2, time+1);
			}
			//	  printf("Reordering\n");
			//   Cudd_ReduceHeap(
			//  		  manager,
			// 		  CUDD_REORDER_WINDOW2 ,
			// //		  //CUDD_REORDER_GROUP_SIFT,
			// //		  //CUDD_REORDER_SIFT,
			// //		  // CUDD_REORDER_SYMM_SIFT,
			// //		  //CUDD_REORDER_SAME,
			// //		  //  CUDD_REORDER_NONE,
			// //		  // 		  CUDD_REORDER_DYNAMIC,
			//  		  10000000//100*num_alt_facts
			//  		  );
			//	  Cudd_PrintInfo(manager, stdout);


		}

		//     printf("%d\n", MUTEX_SCHEME);
		if(MUTEX_SCHEME!=MS_NONE){
			//       printf("@@@@@@@@@@@@@@@@@@@@FINDING MUTEX EFFECTS at: %d@@@@@@@@\n", time);
			find_mutex_effects1(time);
		}
		// print_mutex_effects(time);

		// printf("%d\n", MUTEX_SCHEME);


		if(MUTEX_SCHEME!=MS_NONE){
			//       printf("@@@@@@@@@@@@@@@@@@@@FINDING MUTEX FACTS at: %d@@@@@@@@\n", time+1);
			find_mutex_facts( time + 1 );
			//       print_mutex_facts(time+1);
		}
		times(&end);
		TIME( gexcl_time );

	}
	else{
		//     printf("@@@@@@@@@@@@@@@@@@@@FINDING MUTEX FACTS at: %d@@@@@@@@\n", time+1);


		if(MUTEX_SCHEME!=MS_NONE){
			//printf("@@@@@@@@@@@@@@@@@FINDING MUTEX EFFECTS at: %d@@@@@@@@\n", time);
			std::cout << std::flush;
			find_mutex_effects1(time);
			//printf("@@@@@@@@@@@@@@@@FINDING MUTEX FACTS at: %d@@@@@@@@\n", time+1);
			std::cout << std::flush;
			find_mutex_facts( time + 1 );
			//      print_mutex_facts(time+1);
		}
		times(&end);
		TIME( gexcl_time );


		if(HEURISTYPE==CORRRP){

			actionAndEffectProbability(time);

			if(USE_CORRELATION)
				//actionAndEffectCorrelation(time);
				//effectProbability(time);

				propositionProbability(time+1);
			if(USE_CORRELATION)
				propositionCorrelation(time+1);

		}


		//insert_potential_effects( time );



		if(COMPUTE_LABELS) {
			//dan
			mark_effect_lables(time);
			mark_fact_labels(time+1);
			//nad
		}
	}

	// fact没有增加，mutex没有增加，设置停止迭代标志
	if ( facts_count == gfacts_count &&
			exclusions_count == gexclusions_count ) {
		gsame_as_prev_flag = TRUE;
		if ( gcmd_line.display_info )
			fprintf( OUT, "\ngraph has leveled off at time step %d\n\n", time+1 );
		gfirst_full_time = time;
	}



	if ( gcmd_line.display_info ) {
		fprintf( OUT, "           %5d ops   and %7d exclusive pairs\n",
				gops_count, gops_exclusions_count );
		fprintf( OUT, "time: %3d, %5d facts and %7d exclusive pairs (%5d, %7d positives)\n",
				time+1, gfacts_count, gexclusions_count, gprint_ftnum, gprint_exnum );
	}

	time++;

	//    printf("Exit build graph evo step\n");fflush(stdout);


}









/********************************************************************
 * GRAPH BUILDING FUNCTIONS; OVERSEEN BY BUILD_GRAPH_EVOLUTION_STEP *
 ********************************************************************/









/**
 * momo007 2022.09.26
 * Following code need refactor, use the make_noop() to remove the redundant code
 */
void apply_noops( int time )

{
	FtNode *f1;

	FtNode *ft;
	BitVector *a, *b;
	OpNode *noop;
	EfNode *tmp;
	Effect* tmp_eff;

	char* name;

	//   if(gall_fts_pointer)
	//     printf("Enter apply noops\n");

	/*   if(time < 1) */
	/*       f1 = gall_fts_pointer; */
	/*     else */
	/*       f1=gprev_level_fts_pointer; */

	for ( ft = /* f1 */ gall_fts_pointer; ft && ft != gprev_level_fts_pointer; ft = ft->next ) {

		//     printf("Applying noop for uid: %d , pos: %d ",
		// 	   ft->uid_mask,
		// 	   ft->positive);
		//     if(!ft->info_at[time]->label && ft->info_at[time]->is_dummy){
		//       printFact(ft->index); std::cout << ft->index <<  " has no label" << std::endl;
		//     }
		//     else{
		//       printFact(ft->index); std::cout << ft->index <<  " has label" << std::endl;
		//     }

		//   printf(" at time %d \n With label \n", time);
		//   if(ft->info_at[time] &&
		//      COMPUTE_LABELS &&
		//      ft->info_at[time]->label &&
		//      Cudd_ReadLogicZero(manager) !=
		//      ft->info_at[time]->label->label){
		// //     printf("HI \n");
		//     //    printf("Applying noop for uid: %d , pos: %d ", ft->uid_mask, ft->positive); printFact(ft->index); printf(" at time %d \n With label \n", time);
		// //        printBDD(ft->info_at[time]->label->label);
		//       name = getFactName(ft->index);
		//       std::cout << name << std::endl;
		//     }
		name = getFactName(ft->index);
		tmp_eff = new_Effect(num_alt_effs + ft->index + abs(ft->positive - 1) * num_alt_facts);
		tmp_eff->outcome->insert(0);// 这里添加一个 outcome，设置概率为1
		(*tmp_eff->probability)[0] = 1.0;
		tmp_eff->probability_sum = 1.0;

		a = new_bit_vector( gft_vector_length );
		b = new_bit_vector( gft_vector_length );
		// 创建 p -> p的effect, 将fact自身作为前提和结果创建effect
		if ( ft->positive ) {
			a[ft->uid_block] |= ft->uid_mask;// 标记当前fact
			tmp_eff->cons->p_effects->vector[ft->uid_block] |= ft->uid_mask;// 添加到pos consequence
			tmp_eff->cons->p_effects->indices = new_integers(ft->index);
			tmp_eff->ant->p_conds->vector[ft->uid_block] |= ft->uid_mask;// 添加到pos antedent
			tmp_eff->ant->p_conds->indices = new_integers(ft->index);

			tmp_eff->cons->b = Cudd_bddIthVar(manager, 2*ft->index );//设置bdd
			Cudd_Ref(tmp_eff->cons->b);
			tmp_eff->ant->b = Cudd_bddIthVar(manager, 2*ft->index );//设置bdd
			Cudd_Ref(tmp_eff->ant->b);


		} else {
			b[ft->uid_block] |= ft->uid_mask;// 标记当前fact
			tmp_eff->cons->n_effects->vector[ft->uid_block] |= ft->uid_mask;// 添加到neg consequence
			tmp_eff->cons->n_effects->indices = new_integers(ft->index);
			tmp_eff->ant->n_conds->vector[ft->uid_block] |= ft->uid_mask;// 添加到neg antedent
			tmp_eff->ant->n_conds->indices = new_integers(ft->index);
			tmp_eff->cons->b = Cudd_Not(Cudd_bddIthVar(manager, 2*ft->index ));// 添加bdd
			Cudd_Ref(tmp_eff->cons->b);
			tmp_eff->ant->b = Cudd_Not(Cudd_bddIthVar(manager, 2*ft->index ));//添加bdd
			Cudd_Ref(tmp_eff->ant->b);
		}
		// 使用name, 正负fact vector, fact BDD,以及index创建 NoopNode
		/*        printf("About to make noop an op node\n"); */
		noop = new_op_node( time,name, TRUE, a, b, tmp_eff->ant->b,
							num_alt_acts + ft->index + abs(ft->positive-1)*num_alt_facts+1 );
		noop->info_at[time]->updated = 1;
		noop->action = NULL;
		free_bit_vector(a);
		free_bit_vector(b);
		insert_ft_edge( &(noop->preconds), ft );// 将该Fact添加到 op的Precondtions中


		/*  tmp_cons->p_effects->vector = a; */
		/*     tmp_cons->n_effects->vector = b; */

		//printf("about to make noop an ef node\n");
		// add the effect Node to the OpNode
		// 里面创建的EffectNode没有设置condition,仅设置了 effect,上一行代码将effect的condition设置到了OpNode上
		tmp = new_ef_node( time, noop, tmp_eff, num_alt_effs + ft->index + abs(ft->positive-1)*num_alt_facts/* a, b */ );
		tmp->info_at[time]->updated = 1;
		free_Effect(tmp_eff);

		if(COMPUTE_LABELS){


			/*   if(ft->info_at[time]->label) { */
			/*          printf("A\n"); */
			/* 	 //  ft->info_at[time+1]->label = ft->info_at[time]->label; */
			/*            printf("A1\n"); */
			/*    } */

			/*  if(ft->info_at[time]->label)  */
			/*        tmp->info_at[time]->label = ft->info_at[time]->label; */
			/*      else */
			/*        tmp->info_at[time]->label = NULL; */
			// 将Fact的label传递给EfNode
			tmp->info_at[time]->label = new_Label(ft->info_at[time]->label->label, 0);
		}
		// 将FtNode的mutex传递给EfNode
		if(MUTEX_SCHEME!=MS_NONE){
			for(int r = 0; r < gft_vector_length; r++){
				tmp->info_at[time]->exclusives->pos_exclusives[r] |=
						ft->info_at[time]->exclusives->pos_exclusives[r];
				tmp->info_at[time]->exclusives->neg_exclusives[r] |=
						ft->info_at[time]->exclusives->neg_exclusives[r];
			}
			for(int r = 0; r < num_alt_facts; r++){
				add_worlds_to_mutex(tmp->info_at[time]->exclusives->p_exlabel[r],
						ft->info_at[time]->exclusives->p_exlabel[r]);
				add_worlds_to_mutex(tmp->info_at[time]->exclusives->n_exlabel[r],
						ft->info_at[time]->exclusives->n_exlabel[r]);
			}
			//     tmp->info_at[time]->exclusives->pos_exclusives = copy_bit_vector(ft->info_at[time]->exclusives->pos_exclusives, gft_vector_length);
			//      tmp->info_at[time]->exclusives->neg_exclusives = copy_bit_vector(ft->info_at[time]->exclusives->neg_exclusives, gft_vector_length);
			//      //  if(MUTEX_SCHEME==MS_CROSS){
			//      tmp->info_at[time]->exclusives->p_exlabel = ft->info_at[time]->exclusives->p_exlabel;
			//      tmp->info_at[time]->exclusives->n_exlabel = ft->info_at[time]->exclusives->n_exlabel;
		}
		// }
		tmp->all_next = gall_efs_pointer;// insert effect node to effectNode list
		gall_efs_pointer = tmp;
		noop->unconditional = tmp;// noop set the unconditional effect(because the con(eff) have move to OpNode)
		noop->next = gall_ops_pointer;// insert opNode to opNode list
		gall_ops_pointer = noop;
		noop->is_noop = TRUE;
		//need to assign adder
		insert_ef_edge( &(ft->adders), tmp );// add this effect did by this fact
		ft->noop = noop;

	}
	//      printf("Exit apply noop\n");
}

//extern DdNode* labelBDD(DdNode*, DdNode*, int);
/**
 * 1. 判断effect是否有mutex
 * 2. 创建BitOperatorEffect
 * 3. 根据BitOPerator+ BitOperatorEffect 创建OpNode
 * 4. Apply all Effect(创建，连接EfNode)
 * 5. Check the failure OpNode, add to gops_with_unactivated_effects_pointer
 */
int apply_operator( int time, BitOperator *op ) {





	OpNode *op_node;
	EfNode *ef_node;
	FtNode *ft_node;
	BitVector *precond_pos_exclusives, *precond_neg_exclusives;
	//  ExclusionLabelPair
	DdNode **p_exlabel, **n_exlabel = NULL;
	Integers *i;
	int j;
	/*   Effect* tmp_eff = new_Effect(); */
	Consequent* conseq;

	//std::cout << "enter apply op " << std::endl;

	//   print_BitOperator(op);
	//printBDD(op->b_pre);
	if ( !op->valid ||
			!get_them_non_exclusive( time,
					op->p_preconds,
					op->n_preconds,
					&precond_pos_exclusives,
					&precond_neg_exclusives,
					&p_exlabel,
					&n_exlabel)   ||
					(my_problem->domain().requirements.non_deterministic &&
							(labelBDD(b_initial_state, op->b_pre, time) == Cudd_ReadLogicZero(manager)))

	) {
		//printf("Exiting apply op -- not ok\n");

		return FALSE;
	}
	// printf("Entering apply_op: %s \n", op->name);

	if(DBN_PROGRESSION){
		generate_BitOperatorEffectsFromDBN(op->action);
	}
	else{// conformant/contingent
		generate_BitOperatorEffects(op->action);
	}
	// this operator is redundant, just ignore
	// generate_BitOperatorEffects(op->action);
	if(!op->valid)
		return FALSE;


	op_node = new_op_node( time, op->name, FALSE,
						   op->p_preconds->vector,
						   op->n_preconds->vector,
						   op->b_pre, op->alt_index);

	op_node->info_at[time]->updated = 1;
	op_node->action = op->action;
	op_node->nonDeter = op->nonDeter;
	op_node->num_vars = op->num_vars;
	for ( j=0; j<MAX_VARS; j++ ) {
		op_node->inst_table[j] = op->inst_table[j];
	}
	op_node->next = gall_ops_pointer;// insert into list
	gall_ops_pointer = op_node; 
	// From BitOperator, we link the fact layer and operator layer
	for ( i=op->p_preconds->indices; i; i=i->next ) {
		insert_ft_edge( &(op_node->preconds), gft_table[i->index] );// use FtNode create the FtEdge, fact->op
		insert_op_edge( &(gft_table[i->index]->preconds), op_node );// use OpNode create the OpEdge, op->fact
		// set_bit(ant->p_conds, i->index);
	}
	/*   copy_contents_of_FactInfo(&(tmp_eff->ant->p_conds), op->p_preconds ); */
	//  tmp_eff->ant->p_conds->vector = copy_bit_vector(op->p_preconds->vector, gft_vector_length);
	for ( i=op->n_preconds->indices; i; i=i->next ) {
		insert_ft_edge( &(op_node->preconds), gft_table[NEG_ADR( i->index )] );
		insert_op_edge( &(gft_table[NEG_ADR( i->index )]->preconds), op_node );
		//    set_bit(tmp_eff->ant->n_conds, i->index);
	}
	/*    copy_contents_of_FactInfo(&(tmp_eff->ant->n_conds), op->n_preconds ); */
	// tmp_eff->ant->n_conds->vector = copy_bit_vector(op->n_preconds->vector, gft_vector_length);
	/**
	 * momo007 2022.09.27
	 * Following code can reduce, op->uncondtional is alway not empty in if structure
	 */
	if(op->unconditional){
		//    printf("about to Make uncond ef\n");
		//  ant->n_conds->vector |= op->unconditional->ant->n_conds->vector;
		Effect *te = NULL;
		ef_node = new_ef_node( time, op_node,
				/* (op->unconditional ? op->unconditional->cons->p_effects->vector : */
				/* 			  new_bit_vector( gft_vector_length ) ), */
				(op->unconditional ?
						op->unconditional :
		te = new_Effect(op->unconditional->index) ),

		op->unconditional->index
		/* (op->unconditional ? op->unconditional->cons->n_effects->vector : */
		/* 			  new_bit_vector( gft_vector_length ) ) */ );
		ef_node->info_at[time]->updated = 1;
		if(te)
			free_Effect(te);
	}

	//  printf("Made uncond ef\n");
	//ef_node->effect->cons->b = op->unconditional->cons->b;
	//   if(MUTEX_SCHEME!=MS_NONE){
	//   ef_node->info_at[time]->exclusives->pos_exclusives = precond_pos_exclusives;
	//   ef_node->info_at[time]->exclusives->neg_exclusives = precond_neg_exclusives;


	//   //if(MUTEX_SCHEME==MS_CROSS){
	//     ef_node->info_at[time]->exclusives->p_exlabel = p_exlabel;
	//     ef_node->info_at[time]->exclusives->n_exlabel = n_exlabel;
	//     //}
	//   }
	/** momo007 2022.09.27
	 * the ef_node must be not empty
	 */
	assert(ef_node != NULL);
	op_node->unconditional = ef_node;
	ef_node->all_next = gall_efs_pointer;
	gall_efs_pointer = ef_node;

	/*   if(!ef_node->effect->cons->n_effects->vector) */
	//  printf("GOT NULLGHERE TOO \n");

	/*   if(!op_node->unconditional->effect->cons->n_effects->vector) */
	/*     printf("GOT NULLGHERE TOO \n"); */

	if ( op->unconditional ) {

		ef_node->is_nondeter = op->unconditional->is_nondeter;
		conseq = //ef_node->effect->cons;//
				op->unconditional->cons;
		while(conseq){
			// 考虑每个结果fact
			for ( i = /* op->unconditional->cons */conseq->p_effects->indices; i; i=i->next )
			{

				if ( (ft_node = gft_table[i->index]) == NULL ) {// first present, insert into layer

					ft_node = new_ft_node( time+1, i->index, TRUE, FALSE, NULL );
					ft_node->info_at[time+1]->updated = 1;
					ft_node->next = gall_fts_pointer;
					gall_fts_pointer = ft_node;
					gft_table[i->index] = ft_node;
					(gpos_facts_vector_at[time+1])[ft_node->uid_block] |= ft_node->uid_mask;
				}
				insert_ef_edge( &(ft_node->adders), ef_node );// double link effect -> fact
				insert_ft_edge( &(ef_node->effects), ft_node );
			}

			for ( i = /* op->unconditional->cons */conseq->n_effects->indices; i; i=i->next ) {
				if ( (ft_node = gft_table[NEG_ADR( i->index )]) == NULL ) {
					ft_node = new_ft_node( time+1, i->index, FALSE, FALSE, NULL );
					ft_node->info_at[time+1]->updated = 1;
					ft_node->next = gall_fts_pointer;
					gall_fts_pointer = ft_node;
					gft_table[NEG_ADR( i->index )] = ft_node;
					(gneg_facts_vector_at[time+1])[ft_node->uid_block] |= ft_node->uid_mask;
				}
				insert_ef_edge( &(ft_node->adders), ef_node );
				insert_ft_edge( &(ef_node->effects), ft_node );
			}
			conseq = conseq->next;

		}
	}
	/*   else */
	/*     printf("NULL\n"); */
	// initial, all the condition effect is unactivated (not apply)
	op_node->unactivated_effects = op->conditionals;

	// 存在effect没有apply成功，使用thread将OpNode加入到unactive effect op list中
	if ( !apply_all_effects( time, op_node ) ) {
		op_node->thread = gops_with_unactivated_effects_pointer;
		gops_with_unactivated_effects_pointer = op_node;
	}

	//         printf("Exiting apply op -- ok %s \n", op->name);

	//  print_BitVector(gpos_facts_vector_at[time+1],gop_vector_length_at[time+1]);
	//print_BitVector(gneg_facts_vector_at[time+1],gop_vector_length_at[time+1]);
	//printf( "\n-------------------------------------------------\n");

	return TRUE;

}


int apply_all_effects( int time, OpNode *op_node )
{

	Effect *j, *tmp, *prev;

	/* ERROR!!
	 *
	 * Apply the effect, and check if can be release.
	 */
	j = op_node->unactivated_effects;
	// apply the unactivated effect and remove and add it (if success) into activated conditional effect
	while ( j && apply_effect( time, j, op_node ) ) {
		tmp = j;
		j = j->next;
		tmp->next = tmp->op->activated_conditionals;
		tmp->op->activated_conditionals = tmp;
	}
	// if all success, j is nullptr, or j is the effect which is exclusive
	op_node->unactivated_effects = j;
	prev = j;// record this effect
	if ( j ) j = j->next;// consider next effect
	while ( j ) {
		if ( apply_effect( time, j, op_node ) ) {
			prev->next = j->next;// update the unactivated effect list
			tmp = j;
			j = j->next;// consider next effect
			tmp->next = tmp->op->activated_conditionals;// add to the activated effect
			tmp->op->activated_conditionals = tmp;

		} else {
			prev = prev->next;
			j = j->next;
		}
	}
	// 所有的effect apply成功
	if ( op_node->unactivated_effects == NULL ) {
		// printf("exiting apply all effs T\n");
		return TRUE;
	} else {// 输出apply失败的effect
		// printf("op_node->unactivated_effects = %d\n", op_node->unactivated_effects->index);
		// printf("exiting apmakeply all effs F\n");
		return FALSE;
	}

}

/**
 * momo007 2022.09.26 待检查
 * ef,使用ef更新graph
 * op_node，该effect所属的OpNode
 */
int apply_effect( int time, Effect *ef, OpNode *op_node )

{

	Integers *i;
	BitVector *cond_pos_exclusives, *cond_neg_exclusives = NULL;
	//  ExclusionLabelPair
	DdNode **p_exlabel, **n_exlabel = NULL;
	BitVector *a, *b;
	EfNode *ef_node;
	FtNode *ft_node;
	Consequent* conseq;
	int r;

	//  printf( "time = %d, enter Applying effect of: %s\n", time, op_node->name);
	//     print_fact_info( ef->cons->p_effects, gft_vector_length);
	//      print_fact_info( ef->cons->n_effects, gft_vector_length);

	//  printf("\n\n");



	//  //make sure a precondition was updated
	//   int got_update = 0;

	//   //try antecedent next
	//   for(i = ef->

	// FtEdge* ft = op->preconds; !got_update && ft; ft=ft->next){
	//   }

	//   if(!got_update)
	//     return FALSE;


	if(//MUTEX_SCHEME!=MS_NONE &&
			!get_them_non_exclusive( time,
					ef->ant->p_conds,
					ef->ant->n_conds,
					&cond_pos_exclusives,
					&cond_neg_exclusives,
					&p_exlabel,
					&n_exlabel ) ) {
		//	 printf( "exit Applying effect -- not ok\n "); fflush(stdout);

		return FALSE;
	}
	else if(MUTEX_SCHEME!=MS_NONE) {
		// 	 cond_pos_exclusives = new_bit_vector(gft_vector_length);
		// 	 cond_neg_exclusives = new_bit_vector(gft_vector_length);


		//      //print_BitVector(cond_pos_exclusives, gft_vector_length);printf("\n");
		//      ///     print_BitVector(cond_neg_exclusives, gft_vector_length);printf("\n");
		// 	 //	 printf("HY\n");fflush(stdout);



		//      a = cond_pos_exclusives;
		//      b = op_node->pos_precond_vector;



		//      for ( r = 0; r < gft_vector_length; r++ ) {

		//        if ( a[r] & b[r] ) {

		// 	 if(0){
		// 	 free( cond_pos_exclusives );
		// 	 free( cond_neg_exclusives );
		// 	 }
		// 	 return FALSE;
		//        }
		//        else{
		// 	 //	  printf("else nok\n");
		//        }
		//      }




		//      //     a = cond_neg_exclusives;
		//      if(!cond_neg_exclusives)
		//        cond_neg_exclusives = new_bit_vector(gft_vector_length);
		//      //     printf("UOP\n");

		//      a = cond_neg_exclusives;
		//      //printf("UOP\n");
		//      b = op_node->neg_precond_vector;

		//      //     printf("UOP\n");
		//      for ( r = 0; r < gft_vector_length; r++ ) {
		//        if ( a[r] & b[r] ) {
		// 	 if(0){

		// 	   free( cond_pos_exclusives );
		// 	   free( cond_neg_exclusives );
		// 	 }
		//       return FALSE;
		//        }
		//      }
	}

	/*   tmp_cons = new_Consequent(); */
	/*   tmp_cons = ef->cons; */
	ef_node = new_ef_node( time, op_node, ef, ef->index
			/* ef->cons->p_effects->vector,
			       ef->cons->n_effects->vector */ );
	ef_node->info_at[time]->updated = 1;
	// printf("Efnode index = %d\n", ef_node->index);
	//     if(ef){
	//        free_Effect(ef, );
	//        ef = ef_node->effect;
	//      }

	//      if(MUTEX_SCHEME!=MS_NONE){
	//      ef_node->info_at[time]->exclusives->pos_exclusives = /* cond_pos_exclusives; */ copy_bit_vector(cond_pos_exclusives, gft_vector_length);
	//      ef_node->info_at[time]->exclusives->neg_exclusives = /* cond_neg_exclusives; */copy_bit_vector(cond_neg_exclusives, gft_vector_length);

	//      if(MUTEX_SCHEME!=MS_NONE){
	//        ef_node->info_at[time]->exclusives->p_exlabel = p_exlabel;
	//        ef_node->info_at[time]->exclusives->n_exlabel = n_exlabel;
	//      }
	//   }

	for ( i=ef->ant->p_conds->indices; i; i=i->next ) {
		insert_ft_edge( &(ef_node->conditions), gft_table[i->index] );
	}
	//MMM copy_contents_of_FactInfo(&(ef_node->effect->ant->p_conds), ef->ant->p_conds );
	//     ef_node->effect->ant->p_conds->vector = copy_bit_vector(ef->ant->p_conds->vector, gft_vector_length);
	for ( i=ef->ant->n_conds->indices; i; i=i->next ) {
		insert_ft_edge( &(ef_node->conditions), gft_table[NEG_ADR( i->index )] );
	}
	//MMM  copy_contents_of_FactInfo(&(ef_node->effect->ant->n_conds), ef->ant->n_conds );
	//  ef_node->effect->ant->n_conds->vector = copy_bit_vector(ef->ant->n_conds->vector, gft_vector_length);

	ef_node->next = op_node->conditionals;
	op_node->conditionals = ef_node;
	ef_node->is_nondeter = ef->is_nondeter;
	ef_node->all_next = gall_efs_pointer;
	gall_efs_pointer = ef_node;
	conseq = ef_node->effect->cons;

	while(conseq) {

		//     exit(0);
		for ( i = conseq->p_effects->indices; i; i=i->next ) {
			//	 	                     printf("CHecing add fact for %d\n", i->index);
			if ( (ft_node = gft_table[i->index]) == NULL ) {
				//	printf("HO\n");
				//std:cout << time <<" "; printFact(i->index);
				ft_node = new_ft_node( time+1, i->index, TRUE, FALSE, NULL );
				//	ft_node->info_at[time+1]->is_dummy = FALSE;
				ft_node->info_at[time+1]->updated=1;
				ft_node->next = gall_fts_pointer;
				gall_fts_pointer = ft_node;
				gft_table[i->index] = ft_node;
				(gpos_facts_vector_at[time+1])[ft_node->uid_block] |= ft_node->uid_mask;
			}
			/*      else */
			/* 		ft_node->info_at[time+1]->is_dummy = FALSE; */

			//if(ft_node->info_at[time+1] && ft_node->info_at[time+1]->is_dummy)
			//	ft_node->info_at[time+1]->is_dummy = FALSE;
			//    printFact(ft_node->index); printf("pos linking to\n");
			insert_ef_edge( &(ft_node->adders), ef_node );
			insert_ft_edge( &(ef_node->effects), ft_node );
			//printf(" IS dummy %d\n", ft_node->info_at[time+1]->is_dummy);
		}

		for ( i = conseq->n_effects->indices; i; i=i->next ) {
			//              printf("CHecing add fact for neg  %d\n", i->index);
			if ( (ft_node = gft_table[NEG_ADR( i->index )]) == NULL ) {
				//	printf("HO\n");
				ft_node = new_ft_node( time+1, i->index, FALSE, FALSE, NULL );
				//	ft_node->info_at[time+1]->is_dummy = FALSE;
				ft_node->info_at[time+1]->updated=1;
				ft_node->next = gall_fts_pointer;
				gall_fts_pointer = ft_node;
				gft_table[NEG_ADR( i->index )] = ft_node;
				(gneg_facts_vector_at[time+1])[ft_node->uid_block] |= ft_node->uid_mask;
			}
			/*   else */
			/*      ft_node->info_at[time+1]->is_dummy = FALSE; */
			// printFact(ft_node->index); printf("neg linking to\n");
			insert_ef_edge( &(ft_node->adders), ef_node );
			insert_ft_edge( &(ef_node->effects), ft_node );
		}
		conseq = conseq->next;
	}
	///                 printf( "exit Applying effect -- ok\n ");

	return TRUE;

}











/***************************************
 * CODE FOR APPLYING POTENTIAL EFFECTS *
 ***************************************/










void insert_potential_effects( int time )

{

	OpNode *op, *tmp_op, *prev_op;
	int hit = TRUE;

	// printf("enter Inserting potential effs\n");


	while ( hit ) {
		hit = FALSE;

		op = gops_with_unactivated_effects_pointer;
		while ( op && apply_potential_effects( time, op, &hit ) ) {
			tmp_op = op;
			op = op->thread;
			tmp_op->thread = NULL;
		}
		gops_with_unactivated_effects_pointer = op;
		prev_op = op;
		if ( op ) op = op->thread;
		while ( op ) {
			if ( apply_potential_effects( time, op, &hit ) ) {
				prev_op->thread = op->thread;
				tmp_op = op;
				op = op->thread;
				tmp_op->thread = NULL;
			} else {
				prev_op = prev_op->thread;
				op = op->thread;
			}
		}

		if ( !hit ) break;
	}
	//       printf("exit Inserting potential effs\n");

}


int apply_potential_effects( int time, OpNode *op_node, int *hit )

{

	Effect *j, *tmp, *prev;
	//printf("enter apply potential effs for %s\n", op_node->name);


	/* FEHLER!!
	 *
	 * ...genau ueberpruefen, was ge freet werden darf!!
	 */
	j = op_node->unactivated_effects;
	while ( j && apply_potential_effect( time, j, op_node ) ) {
		tmp = j;
		j = j->next;
		tmp->next = tmp->op->activated_conditionals;
		tmp->op->activated_conditionals = tmp;


		//    if ( 0 ) free_partial_effect( tmp );
		*hit = TRUE;
	}
	op_node->unactivated_effects = j;
	prev = j;
	if ( j ) j = j->next;
	while ( j ) {
		if ( apply_potential_effect( time, j, op_node ) ) {
			prev->next = j->next;
			tmp = j;
			j = j->next;
			tmp->next = tmp->op->activated_conditionals;
			tmp->op->activated_conditionals = tmp;
			//      if ( 0 ) free_partial_effect( tmp );
			*hit = TRUE;
		} else {
			prev = prev->next;
			j = j->next;
		}
	}

	if ( op_node->unactivated_effects == NULL ) {
		return TRUE;
	} else {
		return FALSE;
	}

}


int apply_potential_effect( int time, Effect *ef, OpNode *op_node )

{

	Integers *i;
	EfNode *ef_node;
	FtNode *ft_node, *ft;
	Consequent *tmpCons;

	printf("enter apply potential eff %d for %s\n", ef->index ,op_node->name);
	exit(0);
	if ( !potential_applicable( time, ef, op_node ) ) {
		return FALSE;
	}

	ef_node = new_ef_node( time, op_node, ef, ef->index
			/* ef->cons->p_effects->vector, ef->cons->n_effects->vector */ );
	ef_node->info_at[time]->is_dummy = TRUE;
	if(MUTEX_SCHEME==MS_CROSS){
		ef_node->all_next = gall_efs_pointer;
		gall_efs_pointer = ef_node;
	}

	/* gesamtexclusives bleiben leer (cond_pos/neg_exclusives);
	 * koennen fuer dummys nicht besonders sinnvoll angegeben werden...
	 * (siehe kommentar dazu in search_plan.c)
	 *
	 * NACHSCHAUEN: KANN DAS ZU PROBLEMEN FUEHREN ?
	 *              antwort: NEIN ! (?): falls dieser vektor nicht da ist,
	 *                       wird er in der suche sowieso berechnet.
	 */
	//   printf("A\n");
	for ( i=ef_node->effect->ant->p_conds->indices; i; i=i->next ) {
		ft = gft_table[i->index];
		//    printFact(ft->index);
		//    printf("\n");
		insert_ft_edge( &(ef_node->conditions), ft );
		if ( !ft->info_at[time] ) {
			ft->info_at[time] = new_ft_level_info( ft );
			/* diese information wird in suche nicht verwendet!!!
			 * ---> aber bei integrate potential effects
			 * ---> koennte man auch anders machen und diese info ganz weglassen.
			 * ...allerdings wohl zum debuggen ganz praktisch
			 * und aufwand vernachlaessigbar, also was solls...
			 *
			 * ANMERKUNG: exclusivitaet von dummys wird nach
			 * integration nachgeregelt... solange sind lediglich
			 * die contradicting facts exclusiv.
			 */
			//      printf("AY\n");
			ft->info_at[time]->is_dummy = TRUE;
		}

	}
	//   printf("B\n");
	for ( i=ef_node->effect->ant->n_conds->indices; i; i=i->next ) {
		ft = gft_table[NEG_ADR( i->index )];
		insert_ft_edge( &(ef_node->conditions), ft );
		if ( !ft->info_at[time] ) {
			//    printf("BY\n");
			ft->info_at[time] = new_ft_level_info( ft );
			ft->info_at[time]->is_dummy = TRUE;
		}
	}
	ef_node->next = op_node->conditionals;
	//ef->next = op_node->conditionals;
	op_node->conditionals = ef_node;
	//  op_node->conditionals = ef;

	//  printf("C\n");


	tmpCons = ef_node->effect->cons;
	while(tmpCons){
		//     printf("CONS\n");

		for ( i = tmpCons->p_effects->indices; i; i=i->next ) {
			//printf("CY\n");
			if ( (ft_node = gft_table[i->index]) == NULL ) {
				//std::cout << "adding: " << i->index << std::endl;
				ft_node = new_ft_node( time+1, i->index, TRUE, TRUE, NULL );
				ft_node->info_at[time+1]->is_dummy = TRUE;
				gft_table[i->index] = ft_node;
			}
			insert_ef_edge( &(ft_node->adders), ef_node );
			insert_ft_edge( &(ef_node->effects), ft_node );
		}
		// printf("D\n");

		for ( i = tmpCons->n_effects->indices; i; i=i->next ) {
			if ( (ft_node = gft_table[NEG_ADR( i->index )]) == NULL ) {
				//             printf("DY\n");
				//std::cout << "adding: " << i->index << std::endl;
				ft_node = new_ft_node( time+1, i->index, FALSE, TRUE, NULL );
				ft_node->info_at[time+1]->is_dummy = TRUE;
				gft_table[NEG_ADR( i->index )] = ft_node;
				/* siehe oben
				 */
			}
			insert_ef_edge( &(ft_node->adders), ef_node );
			insert_ft_edge( &(ef_node->effects), ft_node );
		}
		tmpCons = tmpCons->next;
	}

	return TRUE;

}


int potential_applicable( int time, Effect *ef, OpNode *op_node )

{

	static FtArray all_fts, dum_fts;
	int alln = 0, dumn = 0;

	Integers *in;
	FtEdge *jn;
	FtNode *ft;
	OpNode *op_node2;
	FtLevelInfo *info;
	BitVector *vec;
	EfEdge *i_ef;
	int i, j;

	//  std::cout << "pot app" <<std::endl;

	/*
  if(COMPUTE_LABELS) {
    precond_label = Cudd_ReadOne(manager);
    vec = ef->ant->p_conds;
    for(i=0; i < (gft_vector_length+1)*32; i++) {
      if(get_bit((*vec), gft_vector_length, i)) {
	bdd_and(manager, precond_label, gft_table[i]->info_at[time]->label->label);
	Cudd_Ref(precond_label);
      }
    }
    vec = ef->ant->p_conds;
    for(i=0; i < (gft_vector_length+1)*32; i++) {
      if(get_bit((*vec), gft_vector_length, i)) {
        bdd_and(manager, precond_label, gft_table[NEG_ADR(i)]->info_at[time]->label->label);
        Cudd_Ref(precond_label);
      }
    }
    bdd_and(manager, precond_label, op_node->info_at[time]->label->label);
    if(bdd_is_one(manager, Cudd_Not(precond_label)))
      return FALSE;

      }*/
	for ( in = ef->ant->p_conds->indices; in; in = in->next ) {
		ft = gft_table[in->index];
		if ( !ft ) {
			return FALSE;
		}
		if ( alln == ARRAY_SIZE ) {
			printf( "\n\nipp: increase ARRAY_SIZE( preset value: %d )", ARRAY_SIZE );
			exit( 1 );
		}
		all_fts[alln++] = ft;
		if ( !ft->info_at[time] ||
				ft->info_at[time]->is_dummy ) {
			dum_fts[dumn++] = ft;
		}
	}
	for ( in = ef->ant->n_conds->indices; in; in = in->next ) {
		ft = gft_table[NEG_ADR( in->index )];
		if ( !ft ) {
			return FALSE;
		}
		if ( alln == ARRAY_SIZE ) {
			printf( "\n\nipp: increase ARRAY_SIZE( preset value: %d )", ARRAY_SIZE );
			exit( 1 );
		}
		all_fts[alln++] = ft;
		if ( !ft->info_at[time] ||
				ft->info_at[time]->is_dummy ) {
			dum_fts[dumn++] = ft;
		}
	}

	for ( i=0; i<alln; i++ ) {
		/* uebernehme fuer dummys exclusions von vorkommen auf naechstem level;
		 * das kann entweder contradict sein, falls fact auf time+1 dummy
		 * oder aber volle excl, falls fact auf time+1 echt.
		 * hier greifen wir etwas vor: ist time+1 echt, so werden spaeter
		 * fuer dummy time sowieso die time+1 excl eingetragen.
		 */
		if ( !all_fts[i]->info_at[time] ||
				all_fts[i]->info_at[time]->is_dummy ) {
			info = all_fts[i]->info_at[time+1];
		} else {
			info = all_fts[i]->info_at[time];
		}
		/* info = all_fts[i]->info_at[time] ? all_fts[i]->info_at[time] :
       all_fts[i]->info_at[time+1]; */

		if(MUTEX_SCHEME!=MS_NONE){

			for ( j = i+1; j<alln; j++ ) {
				vec = all_fts[j]->positive ? info->exclusives->pos_exclusives : info->exclusives->neg_exclusives;
				if ( vec[all_fts[j]->uid_block] & all_fts[j]->uid_mask ) {
					return FALSE;
				}
			}

			for ( jn = op_node->preconds; jn; jn = jn->next ) {
				vec = jn->ft->positive ? info->exclusives->pos_exclusives : info->exclusives->neg_exclusives;
				if ( vec[jn->ft->uid_block] & jn->ft->uid_mask ) {
					return FALSE;
				}
			}
		}
	}

	/* gecheckt ist jetzt: - alle conditions sind zumindest im graph
	 *                       (d.h. dummy oder aufm naechsten level)
	 *                     - alle conditions sind zumindest nicht exclusiv
	 *                       voneinander und von preconds
	 *                       ACHTUNG bei facts, die auf time noch nicht da sind:
	 *                               solche sind auf time + 1 jedenfalls da und
	 *                               auf time mindestens so exclusive wie auf time+1
	 *                       (dummys die auf time da sind, haben nur die contradicting
	 *                        exclusivitaet)(genauso wie dummys auf time+1)
	 */


	/* jetzt checken, ob dummy facts zusammen erreicht werden koennen!
	 * ( bei verwendung von op_node )
	 */
	if(MUTEX_SCHEME!=MS_NONE){

		for ( i=0; i<dumn; i++ ) {
			for ( i_ef = dum_fts[i]->adders; i_ef; i_ef = i_ef->next ) {
				op_node2 = i_ef->ef->op;
				if ( ( op_node->info_at[time]->exclusives->exclusives[op_node2->uid_block] &
						op_node2->uid_mask ) == 0 ) {
					break;
				}
			}
			if ( !i_ef ) return FALSE;
		}
	}
	return TRUE;

}





void integrate_potential_effects( int time )

{

	OpNode *op;
	EfNode *ef;
	FtEdge *i, *j;
	Integers *k;
	Consequent *tmpCons;
	FtNode* ft_node;
	EfNode* ef_node;
	/*        printf("ENTER integ pot eff\n"); */

	for ( op = gall_ops_pointer; op; op = op->next ) {
		/*           printf("Integrating %s's effects\n", op->name); */
		for ( ef = op->conditionals; ef; ef = ef->next ) {
			// printf("Integrating cond\n");

			if ( !ef->info_at[time]->is_dummy ) continue;

			//       printf("Ef is dummy\n");

			/* eine etwas ineffektive methode, die nicht integrierten
			 * potentiellen effekte zu finden, besser waere eine
			 * globale liste der entsprechenden effekte
			 * mit dem ueblichen rausstreichverfahren...
			 *
			 * INEFFIZIENT, vielleicht spaeter mal aendern.
			 */

			for ( i = ef->conditions; i; i = i->next ) {


				if ( i->ft->info_at[time]->is_dummy ) break;
			}
			if ( i ) continue;

			//  printf("are any preconds mutex\n");
			if(MUTEX_SCHEME!=MS_NONE){
				for ( i = ef->conditions; i; i = i->next ) {
					for ( j = i->next; j; j = j->next ) {
						if ( ARE_MUTEX_FTS( time, i->ft, j->ft, NULL, NULL ) ) break;
					}
					if ( j ) break;

					/* conditions, die frueher dummy waren, koennen
					 * exclusiv von den preconds sein!
					 *
					 * glaub ich jez eigentlich nicht, aber riskieren wollen
					 * wers mal net. SPAETER MAL TESTEN!
					 */
					for ( j = op->preconds; j; j = j->next ) {
						if ( ARE_MUTEX_FTS( time, i->ft, j->ft, NULL, NULL ) ) break;
					}
					if ( j ) break;
				}
				if ( i ) continue;
			}

			/* effekt kann integriert werden!
			 */
			//         printf("intersting effect facts in next level\n");
			ef->info_at[time]->is_dummy = FALSE;
			// ef_node = ef;
			for ( i = ef->effects; i; i = i->next ) {
				/*    tmpCons = ef->effect->cons; */
				/*       while(tmpCons){ */

				/* 	print_fact_info( tmpCons->p_effects, gft_vector_length); */
				/*        for ( k = /\* op->unconditional->cons *\/tmpCons->p_effects->indices; k; k=k->next ) */
				/* 	 { */
				/* 	   printf("pos ft = "); */
				/* 	   printFact(k->index); */
				/* 	if ( (ft_node = gft_table[k->index]) == NULL ) { */
				/* 	  ft_node = new_ft_node( time+1, k->index, TRUE, FALSE, NULL ); */
				/* 	  ft_node->next = gall_fts_pointer; */
				/* 	  gall_fts_pointer = ft_node; */
				/* 	  gft_table[k->index] = ft_node; */
				/* 	  (gpos_facts_vector_at[time+1])[ft_node->uid_block] |= ft_node->uid_mask; */
				/* 	} */
				/* 	insert_ef_edge( &(ft_node->adders), ef_node ); */
				/* 	insert_ft_edge( &(ef_node->effects), ft_node ); */
				/*       } */
				/* 	print_fact_info( tmpCons->n_effects, gft_vector_length); */

				/*       for ( k = /\* op->unconditional->cons *\/tmpCons->n_effects->indices; k; k=k->next ) { */
				/* 	   printf("neg ft = "); */
				/* 	   printFact(k->index); */
				/* 	if ( (ft_node = gft_table[NEG_ADR( k->index )]) == NULL ) { */
				/* 	ft_node = new_ft_node( time+1, k->index, FALSE, FALSE, NULL ); */
				/* 	ft_node->next = gall_fts_pointer; */
				/* 	gall_fts_pointer = ft_node; */
				/* 	gft_table[NEG_ADR( k->index )] = ft_node; */
				/* 	(gneg_facts_vector_at[time+1])[ft_node->uid_block] |= ft_node->uid_mask; */
				/* 	} */
				/* 	insert_ef_edge( &(ft_node->adders), ef_node ); */
				/* 	insert_ft_edge( &(ef_node->effects), ft_node ); */
				/*       } */
				/*   for ( i = tmpCons->indicies; i; i = i->next ) { */
				/* JETZT ERST WIRD FACT VON get...non_exclusive FUNKTIONEN
				 * ANERKANNT ---> KANN IN PRECONDS VERWENDET WERDEN.
				 */
				if ( i->ft->positive ) {
					(gpos_facts_vector_at[time+1])[i->ft->uid_block] |= i->ft->uid_mask;
				} else {
					(gneg_facts_vector_at[time+1])[i->ft->uid_block] |= i->ft->uid_mask;
				}
				/* jetzt wirds auch von dieser funktion anerkannt...
				 * prinzipiell wuerde eine der beiden informationen schon ausreichen.
				 */
				if ( i->ft->info_at[time+1]->is_dummy ) {
					i->ft->next = gall_fts_pointer;
					gall_fts_pointer = i->ft;
					i->ft->info_at[time+1]->is_dummy = FALSE;
					gfacts_count++;
				}
			}
			/*   tmpCons = tmpCons->next; */
			/*       } */
		}
	}
	//       printf("Exit integ pot eff\n");
}









/********************************************
 * HELPERS HANDLING CONDITIONS AND PRECONDS *
 ********************************************/










/**
 * 判断当前目标在time layer是否满足同时没有出现mutex
 */
int are_there_non_exclusive( int time, FactInfo *pos, FactInfo *neg )

{


	BitVector *a, *b;
	Integers *i;
	int r;
	//   printf("enter are there non exclusive, time = %d \n", time);

	/* !!!!!!!!!!!!!!!!!!!!!!!!
	 * hier lieber einzeln die fakten auslesen!!
	 * genauso eins weiter unten in get_them..
	 */
	a = pos->vector;
	b = gpos_facts_vector_at[time];

	for ( r = 0; r < gft_vector_length; r++ ) {
		// printf("r = %d\n", r);
		// print_BitVector(a, r);
		// print_BitVector(b, r);

		if ( a[r] != (a[r] & b[r]) ) {// 至少满足目标需要的pos facts
			//  printf("exit are there non exclusive -- not ok\n");
			return FALSE;
		}

	}
	// 相同的，我们的否定命题只有b要求为true时，我们该命题才否定
	a = neg->vector;
	b = gneg_facts_vector_at[time];
	
	for ( r = 0; r < gft_vector_length; r++ ) {
		//printf("B\n");

		if ( a[r] != (a[r] & b[r]) ) {// 满足当前目标需要满足的neg facts
			//printf("1exit are there non exclusive -- not ok\n");
			return FALSE;
		}
	}
	// NONE MUTEX SCHEMA
	if(MUTEX_SCHEME!=MS_NONE){
		/* geht das EFFIZIENTER durch zusammen OR-en der exclusives ??
		 */
		// std::cout << "check mutexes " <<std::endl;
		for ( i=pos->indices; i; i=i->next ) {

			// printf("\nYO %d\n", i->index);
			// 获取该Fact结点在该层的mutex命题
			a = gft_table[i->index]->info_at[time]->exclusives->pos_exclusives;
			// print_BitVector(pos->vector,gft_vector_length);
			// print_BitVector(a,gft_vector_length);

			b = gft_table[i->index]->info_at[time]->exclusives->neg_exclusives;

			for ( r = 0; r < gft_vector_length; r++ ) {
				// 当前要求成立正负命题出现在 exlusive中，返回false
				if ( (pos->vector[r] & a[r]) || (neg->vector[r] & b[r]) ) {
					//	printf("2exit are there non exclusive -- not ok\n");
					return FALSE;
				}
			}
		}

		// similarly, check the mutex about the negative fact
		for ( i=neg->indices; i; i=i->next ) {


			a = gft_table[NEG_ADR( i->index )]->info_at[time]->exclusives->pos_exclusives;
			b = gft_table[NEG_ADR( i->index )]->info_at[time]->exclusives->neg_exclusives;
			for ( r = 0; r < gft_vector_length; r++ ) {
				if ( (pos->vector[r] & a[r]) || (neg->vector[r] & b[r]) ) {
					//printf("3exit are there non exclusive -- not ok\n");
					return FALSE;
				}
			}
		}
	}
	//   printf("exit are there non exclusive -- ok\n");

	return TRUE;

}
/**
 * momo007 2022.09.16
 * 目前仅考虑fact和effect的mutex，没有考虑effect的mutex，
 * 如果effect 之间mutex导致没有world可以使用，利用label值为0，能够保证逻辑正确性。
 */
int get_them_non_exclusive( int time,
		FactInfo *pos, FactInfo *neg,
		BitVector **tmp_pos_exclusives,
		BitVector **tmp_neg_exclusives,
		//ExclusionLabelPair
		DdNode*** p_exlabel,
		//ExclusionLabelPair
		DdNode*** n_exlabel ) {

	BitVector *a, *b;//, *tmp_pos_exclusives, *tmp_neg_exclusives ;
	//  ExclusionLabelPair
	DdNode** tmp_p_exlabel, **tmp_n_exlabel;
	int r, s;
	Integers *i;

	//printf("enter get them non exclus\n");

	a = pos->vector; //preconds
	b = gpos_facts_vector_at[time]; //level
	//      printf("pos vector\n");
	//       print_BitVector(a, gft_vector_length);
	//       //  print_fact_info( pos, gft_vector_length);
	//          print_BitVector(b, gft_vector_length);
	// 	      printf("\n");

	//checking pos applicability
	for ( r = 0; r < gft_vector_length; r++ ) {
		if ( a[r] != (a[r] & b[r]) ) {
			//        printf("1exit get them non exclus -- not ok\n");
			return FALSE;
		}
	}

	//checking neg applicability
	//   printf("neg vector\n");
	if(neg) {
		a = neg->vector;
		b = gneg_facts_vector_at[time];


		//     print_BitVector(a, gft_vector_length);
		//          print_BitVector(b, gft_vector_length);
		//          printf("\n");

		for ( r = 0; r < gft_vector_length; r++ ) {

			if ( a[r] != (a[r] & b[r]) ) {
				//			printf("2exit get them non exclus -- not ok\n");
				return FALSE;
			}
		}
	}


	return TRUE;
	/**
	 * 下面的代码将导致effect之间mutex从而无法执行，因此不考虑，即使没有效果可以应用的world，
	 * 目前不修复下面代码，直接允许该effect，通过label将保证逻辑正确，在effect的label会设置为false，表示无法执行。
	 */
	//executing the code below will prevent effect inclusion if there
	//is a world where a mutex among conditions, rather than fix this
	//code the structure allows the effect even if there is no world
	//where the effect is applicable.  The labels will make sure the
	//right worlds indicate where it is executable.  So the effect
	//may be there and labelled false.


	if(MUTEX_SCHEME != MS_NONE){
		*tmp_pos_exclusives = new_excl_bit_vector( gft_vector_length );
		*tmp_neg_exclusives = new_excl_bit_vector( gft_vector_length );

		tmp_p_exlabel = (DdNode**) calloc(num_alt_facts, sizeof(DdNode*));
		tmp_n_exlabel = (DdNode**) calloc(num_alt_facts, sizeof(DdNode*));

		for ( i=pos->indices; i; i=i->next ) {
			// printf("Checking exclusion for pos %d", i->index);fflush(stdout); printFact(i->index); fflush(stdout);
			b = gft_table[i->index]->info_at[time]->exclusives->pos_exclusives;
			//   print_BitVector(b, gft_vector_length);printf("\n");
			//     print_BitVector(tmp_pos_exclusives, gft_vector_length);printf("\n");
			for ( r = 0; r < gft_vector_length; r++ ) {
				//    printf("%d,%d, %d, %d, %d\n", gft_vector_length,r, b[r], (tmp_pos_exclusives)[r],MUTEX_SCHEME);

				(*tmp_pos_exclusives)[r] |= b[r];
				if( MUTEX_SCHEME==MS_CROSS){
					for(s=r*32; s<((r+1)*32); s++)
						if(get_bit((*tmp_pos_exclusives), gft_vector_length, s))
							tmp_p_exlabel[s]=gft_table[i->index]->info_at[time]->exclusives->p_exlabel[s];
				}
				//         print_BitVector(pos->vector, gft_vector_length);printf("\nYO\n");

				if ( pos->vector[r] & (*tmp_pos_exclusives)[r] ) {
					//		printf("Exclusive with %d\n", r);
					//		print_BitVector(tmp_pos_exclusives, gft_vector_length);
					//printf("FRee1\n");fflush(stdout);

					free( *tmp_pos_exclusives );
					free( *tmp_neg_exclusives );

					//		printf("3exit get them non exclus -- not ok\n");
					return FALSE;
				}

			}

			b = gft_table[i->index]->info_at[time]->exclusives->neg_exclusives;
			for ( r = 0; r < gft_vector_length; r++ ) {
				(*tmp_neg_exclusives)[r] |= b[r];
				if( MUTEX_SCHEME==MS_CROSS){

					for(s=r*32; s<((r+1)*32); s++)
						if(get_bit((*tmp_neg_exclusives), gft_vector_length, s))
							tmp_n_exlabel[s]=gft_table[i->index]->info_at[time]->exclusives->n_exlabel[s];
				}

				if ( neg->vector[r] & (*tmp_neg_exclusives)[r] ) {
					//	printf("FRee2\n");fflush(stdout);

					free( *tmp_pos_exclusives );
					free( *tmp_neg_exclusives );

					//	  	  printf("4exit get them non exclus -- not ok\n");
					return FALSE;
				}
			}
			//std::cout << "HI" << std::endl << flush;
		}
		//printf("check neg\n");fflush(stdout);
		for ( i=neg->indices; i; i=i->next ) {
			//           printf("Checking exclusion for neg "); printFact(i->index);printf("\n");
			b = gft_table[NEG_ADR( i->index )]->info_at[time]->exclusives->pos_exclusives;
			for ( r = 0; r < gft_vector_length; r++ ) {
				(*tmp_pos_exclusives)[r] |= b[r];
				//      printf("YO %d %d\n", gft_vector_length, r);
				//         print_BitVector((tmp_pos_exclusives), gft_vector_length);
				//   print_BitVector((tmp_neg_exclusives), gft_vector_length);
				if(MUTEX_SCHEME==MS_CROSS){
					//      printf("YO\n");
					for(s=r*32; (s<((r+1)*32) && s < gnum_relevant_facts); s++)
						if(get_bit((*tmp_pos_exclusives), gft_vector_length, s))
							tmp_p_exlabel[s] = gft_table[NEG_ADR( i->index )]->info_at[time]->exclusives->p_exlabel[s];
				}
				if ( pos->vector[r] & (*tmp_pos_exclusives)[r] ) {
					//	printf("FRee\n");


					free( *tmp_pos_exclusives );
					free( *tmp_neg_exclusives );

					//		printf("5exit get them non exclus -- not ok\n");
					return FALSE;
				}
				else{
					//	printf("Found exclsuive\n");
				}
			}
			//   printf("out\n");
			b = gft_table[NEG_ADR( i->index )]->info_at[time]->exclusives->neg_exclusives;

			for ( r = 0; r < gft_vector_length; r++ ) {
				//    print_BitVector((tmp_neg_exclusives), gft_vector_length);
				//   print_BitVector(b, gft_vector_length);
				//    printf("YO %d %d\n", gft_vector_length, r);
				(*tmp_neg_exclusives)[r] |= b[r];

				if(MUTEX_SCHEME==MS_CROSS){
					//    printf("YO %d %d\n", gft_vector_length, r);
					for(s=r*32; s<((r+1)*32); s++)
						if(get_bit((*tmp_neg_exclusives), gft_vector_length, s))
							tmp_n_exlabel[s]=gft_table[NEG_ADR( i->index )]->info_at[time]->exclusives->n_exlabel[s];
				}
				if ( neg->vector[r] & (*tmp_neg_exclusives)[r] ) {
					//	printf("FRee\n");


					free( *tmp_pos_exclusives );
					free( *tmp_neg_exclusives );

					//	   printf("6exit get them non exclus -- not ok\n");
					return FALSE;
				}
				//  	printf("Found exclsuive\n");
			}
		}
		//  pos_exclusives=&tmp_pos_exclusives;
		//neg_exclusives=&tmp_neg_exclusives;
		p_exlabel=&tmp_p_exlabel;
		n_exlabel=&tmp_n_exlabel;
		/*  print_BitVector(tmp_pos_exclusives, gft_vector_length);printf("\n"); */
		/*   print_BitVector(tmp_neg_exclusives, gft_vector_length);printf("\n"); */
		// printf("exit get them non exclus -- ok\n");
		//  print_BitVector(p_exlabels, gft_vector_length);printf("\n");
		//print_BitVector(n_exlabels, gft_vector_length);printf("\n");
		//  free_bit_vector(*tmp_pos_exclusives);
		//  free_bit_vector(*tmp_neg_exclusives);
	}


	return TRUE;

}









/******************
 * SIMPLE HELPERS *
 ******************/









void insert_op_edge( OpEdge **l, OpNode *op )

{

	OpEdge *new_edge = new_op_edge( op );

	new_edge->next = *l;
	*l = new_edge;

}

void insert_ef_edge( EfEdge **l, EfNode *ef )

{

	EfEdge *new_edge = new_ef_edge( ef );

	new_edge->next = *l;
	*l = new_edge;

}

void insert_ft_edge( FtEdge **l, FtNode *ft )

{

	FtEdge *new_edge = new_ft_edge( ft );
	new_edge->next = *l;
	*l = new_edge;

}









/************************************************************************
 * FOR RIFO META STRATEGY: GRAPH AND MEMOs, WAVE FRONT FREEING FUNCTION *
 ************************************************************************/









void free_graph_and_search_info( void )

{

	int i;

	OpNode* tmp;

	for ( i = 0; i < 2 * gnum_relevant_facts; i++ ) {
		/* memo s are freed as a side effect of freeing facts, as
		 * memo trees start in fact level infos
		 */

		free_ft_node( gft_table[i] );
		gft_table[i] = NULL;
		//std::cout << "i = " << i << std::endl << flush;
	}

	free( gft_table );
	gall_fts_pointer = NULL;
	gprev_level_fts_pointer = NULL;

	/* effects get wiped away with ops: they are connected only to
	 * their corresponding op nodes
	 */

	//  printf("IPP_MAX_PLAN = %d", IPP_MAX_PLAN); fflush(stdout);
	for(tmp = gall_ops_pointer; tmp; tmp = tmp->next){
		for(i = 0; i< IPP_MAX_PLAN; i++){
			//  printf("%d\n",i);
			free_op_level_info(tmp->info_at[i]);
		}
	}

	/*  gbit_operators = NULL; */



	/*   free_op_node( gall_ops_pointer ); */

	gall_ops_pointer = NULL;
	gprev_level_ops_pointer = NULL;
	gops_with_unactivated_effects_pointer = NULL;



	free_op_pair( gop_mutex_pairs );
	free_ef_pair( gef_mutex_pairs );
	free_ft_pair( gft_mutex_pairs );
	gop_mutex_pairs = NULL;
	gft_mutex_pairs = NULL;

	gfacts_count = 0;
	gexclusions_count = 0;
	gops_count = 0;
	gops_exclusions_count = 0;
	gefs_count = 0;
	gefs_exclusions_count = 0;
	gprint_ftnum = 0;
	gprint_exnum = 0;

	for ( i = 0; i < IPP_MAX_PLAN; i++ ) {
		free( gpos_facts_vector_at[i] );
		free( gneg_facts_vector_at[i] );
	}

	gsame_as_prev_flag = FALSE;
	gfirst_full_time = 0;

	/* free_candidate( gwave_front_head );
     free_candidate( gwave_front_trash ); */
	gwave_front_head = NULL;
	gwave_front_tail = NULL;
	gwave_front_trash = NULL;
	gplan_start = NULL;

}

void free_graph_info(int j )

{
	int i;

	//    printMemInfo();
	for ( i = 0; i < 2 * gnum_relevant_facts; i++ ) {
		/* memo s are freed as a side effect of freeing facts, as
		 * memo trees start in fact level infos
		 */


		//        std::cout << "free fts" << i <<std::endl << flush;
		//     if(i < gnum_relevant_facts)
		//       printFact(i);
		//     else
		//       printFact(i-gnum_relevant_facts);
		//     std::cout << flush;
		free_ft_node( gft_table[i] );

		gft_table[i] = NULL;
	}

	free( gft_table );

	gall_fts_pointer = NULL;
	gprev_level_fts_pointer = NULL;


	/* effects get wiped away with ops: they are connected only to
	 * their corresponding op nodes
	 */


	free_op_node( gall_ops_pointer );


	//  std::cout << "done ops"<<std::endl;
	gall_ops_pointer = NULL;
	gprev_level_ops_pointer = NULL;
	gops_with_unactivated_effects_pointer = NULL;
	gall_efs_pointer = NULL;
	gprev_level_efs_pointer = NULL;

	free_op_pair( gop_mutex_pairs );
	free_ft_pair( gft_mutex_pairs );

	gop_mutex_pairs = NULL;
	gft_mutex_pairs = NULL;

	gfacts_count = 0;
	gexclusions_count = 0;
	gops_count = 0;
	gops_exclusions_count = 0;
	gprint_ftnum = 0;
	gprint_exnum = 0;


	for ( i = 0; (i < IPP_MAX_PLAN &&  i < j+1 ) ; i++ ) {
		free_bit_vector( gpos_facts_vector_at[i] );
		free_bit_vector( gneg_facts_vector_at[i] );
	}

	gsame_as_prev_flag = FALSE;
	gfirst_full_time = 0;
	new_plan = TRUE;


	/*   printf("HO\n"); */


	//   printMemInfo();


}


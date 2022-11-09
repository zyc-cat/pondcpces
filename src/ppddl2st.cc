#include <sstream>
#include <string.h>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>

#include "globals.h"
#include "lao_wrapper.h"
#include "astar.h"
#include "ehc.h"

#ifdef PPDDL_PARSER
#include "allheaders.h"
extern void collectInit(const Problem *);
extern void generate_n_labels(int, std::list<DdNode *> *, std::list<int> *);
extern double gDiscount;
void set_cubes();
void IPC_write();
#else
#include "actions.h"
#include "instantiation.h"
#include "parse.h"
#include "actions.h"
#endif

#include "dddmp.h"
#include "statistical.h"
#include "randomGenerator.h"
#include "dd.h"
#include "lug.h"
#include "graph_wrapper.h"
#include "kGraphInfo.h"
#include "correlation.h"
#include "sample_size.h"
#include "lao.h"
using namespace std;

#define OUTPUT_OVERHEAD (10 + 5) /* the number of milliseconds it takes to receive (10) and process (5) the signal */

itimerval timer; // allowed_time

bool respect_time = true;
int num_new_labels = 0;

void ppddl2st(std::ostream *o); // 该函数实现了ppddl2转化为程序存储变量

void end_early(int signal)
{
	//  outputPlan();
	exit(0);
}

extern int gNumStates; // number of states in lao star search

extern clock_t gStartTime;

/* BM: Added by BM */
ofstream my_outfile;
char my_outfile_name[100];
char my_buffer[100000];
/* BM: End */

/* The parse function. */
extern int yyparse();

/* File to parse. */
#ifdef PPDDL_PARSER
extern FILE *yyin;
#else
FILE *yyin;
#endif

/* Name of current file. */
std::string current_file;

/* Parses the given file, and returns true on success. */
static bool read_file(const char *name)
{
	yyin = fopen(name, "r");
	if (yyin == NULL)
		return false;
	else
	{
		current_file = name;
		bool success = (yyparse() == 0);
		fclose(yyin);
		return success;
	}
}

int main(int argc, char *argv[])
{
	try
	{
		gStartTime = clock(); // 开始计时
		srand(time(NULL));
		randomGen = new randomGenerator(100000); // use a million random nums

		if (argc < 3)
		{
			cout << "POND Planner - Version 1.1.1\n"
				 << "Usage: pond <domain-name> <prob-name> [-w <hweight>] [-h <heuristic>]" << endl
				 << "Heuristics:\n\t <default> \t\t  : h = 0 <Breadth First AO*>\n\t 'card' \t\t  : Cardinality\n\t 'sgrp' \t\t  : SG Relaxed Plan\n\t 'mgrpm' \t\t  : MG max relaxed plan\n\t 'mgrps' \t\t  : MG sum relaxed plan\n\t 'mgrpu' \t\t  : MG unioned relaxed plan\n\t 'lugrp [-pg <sag_type>]' : LUG relaxed plan" << endl
				 << "SAG Types: \n\t <default> \t: One LUG per generated node\n\t 'children' \t: One LUG per expanded node\n\t 'global' \t: One LUG per problem" << endl;
			return 0;
		}

#ifdef PPDDL_PARSER
		if ((read_file(argv[1]) && (Problem::begin()) != (Problem::end())) || read_file(argv[2]))
			cout << "done Parsing" << endl;
		else
		{
			cout << "Parse Error" << endl;
			exit(0);
		}

#endif

		if (argc > 3)
		{
			for (int i = 3; i < argc; i++)
			{
				if (strcmp(argv[i], "-w") == 0)
				{
					int param;
					sscanf(argv[++i], "%d", &param);
					GWEIGHT = param;
					cout << "HWEIGHT = " << GWEIGHT << endl;
				}
				if (strcmp(argv[i], "-cwa") == 0)
				{
					CWA = TRUE;
					cout << "USING CWA" << endl;
				}
				if (strcmp(argv[i], "-enum") == 0)
				{
					ENUMERATE_SPACE = TRUE;
					cout << "ENUMERATING SEARCH TREE" << endl;
				}
				if (strcmp(argv[i], "-h") == 0)
				{
					++i;
					if (strcmp(argv[i], "corrrp") == 0)
					{
						cout << "USING Correlation RP" << endl;
						HEURISTYPE = CORRRP;
						COMPUTE_LABELS = FALSE;
						USE_CORRELATION = TRUE;
					}
					if (strcmp(argv[i], "prrp") == 0)
					{
						cout << "USING Correlation RP" << endl;
						HEURISTYPE = CORRRP;
						COMPUTE_LABELS = FALSE;
						USE_CORRELATION = FALSE;
					}
					if (strcmp(argv[i], "slugrp") == 0)
					{
						cout << "USING SLUGRP" << endl;
						HEURISTYPE = SLUGRP;
					}
					if (strcmp(argv[i], "minslugrp") == 0)
					{
						cout << "USING MINSLUGRP" << endl;
						HEURISTYPE = SLUGRP;
						TAKEMINRP = TRUE;
					}
					if (strcmp(argv[i], "lugrp") == 0)
					{
						cout << "USING LUGRP" << endl;
						HEURISTYPE = LUGRP;
					}
					if (strcmp(argv[i], "luglevel") == 0)
					{
						cout << "USING LUGLEVEL" << endl;
						HEURISTYPE = LUGLEVEL;
					}
					if (strcmp(argv[i], "lugsum") == 0)
					{
						cout << "USING LUGSUM" << endl;
						HEURISTYPE = LUGSUM;
					}
					if (strcmp(argv[i], "lugmax") == 0)
					{
						cout << "USING LUGMAX" << endl;
						HEURISTYPE = LUGMAX;
					}
					if (strcmp(argv[i], "card") == 0)
					{
						cout << "USING CARD" << endl;
						HEURISTYPE = CARD;
					}
					if (strcmp(argv[i], "mgrpu") == 0)
					{
						LUG_FOR = NODE;
						ALLOW_LEVEL_OFF = FALSE;
						COMPUTE_LABELS = FALSE;
						HEURISTYPE = HRPUNION;

						cout << "NOT USING LABELS ON PG" << endl;
					}
					if (strcmp(argv[i], "mgrpm") == 0)
					{
						LUG_FOR = NODE;
						ALLOW_LEVEL_OFF = FALSE;
						COMPUTE_LABELS = FALSE;
						HEURISTYPE = HRPMAX;
						cout << "NOT USING LABELS ON PG" << endl;
					}
					if (strcmp(argv[i], "mgrps") == 0)
					{
						LUG_FOR = NODE;
						ALLOW_LEVEL_OFF = FALSE;
						COMPUTE_LABELS = FALSE;
						HEURISTYPE = HRPSUM;
						cout << "NOT USING LABELS ON PG" << endl;
					}
					if (strcmp(argv[i], "sgrp") == 0)
					{
						LUG_FOR = NODE;
						ALLOW_LEVEL_OFF = FALSE;
						COMPUTE_LABELS = FALSE;
						HEURISTYPE = LUGRP;
						cout << "NOT USING LABELS ON PG" << endl;
					}
					if (strcmp(argv[i], "sglevel") == 0)
					{
						COMPUTE_LABELS = FALSE;
						HEURISTYPE = LUGLEVEL;
						cout << "NOT USING LABELS ON PG" << endl;
					}
					if (strcmp(argv[i], "sgsum") == 0)
					{
						COMPUTE_LABELS = FALSE;
						HEURISTYPE = LUGSUM;
						cout << "NOT USING LABELS ON PG" << endl;
					}
					if (strcmp(argv[i], "sgmax") == 0)
					{
						COMPUTE_LABELS = FALSE;
						HEURISTYPE = LUGMAX;
						cout << "NOT USING LABELS ON PG" << endl;
					}
				}
				else if (strcmp(argv[i], "-inc") == 0)
				{
					DO_INCREMENTAL_HEURISTIC = TRUE;
					cout << "Using Incremental Dominance Heuristic" << endl;
					int param;
					sscanf(argv[++i], "%d", &param);
					MAX_H_INCREMENT = param;
				}
				else if (strcmp(argv[i], "-nlevoff") == 0)
				{
					ALLOW_LEVEL_OFF = FALSE;
					cout << "Don't ALLOW LEVEL OFF" << endl;
				}
				else if (strcmp(argv[i], "-pg") == 0)
				{
					++i;
					if (strcmp(argv[i], "node") == 0)
					{
						LUG_FOR = NODE;

						cout << "BUILDING LUG PER NODE" << endl;
					}
					else if (strcmp(argv[i], "children") == 0)
					{
						LUG_FOR = FRONTIER;
						cout << "BUILDING LUG FOR SET OF CHILDREN" << endl;
					}
					else if (strcmp(argv[i], "global") == 0)
					{
						LUG_FOR = SPACE;
						ALLOW_LEVEL_OFF = TRUE;
						cout << "BUILDING LUG FOR REACHABLE SPACE" << endl;
					}
					else if (strcmp(argv[i], "globalrp") == 0)
					{
						LUG_FOR = SPACE;
						ALLOW_LEVEL_OFF = TRUE;
						USE_GLOBAL_RP = true;
						cout << "BUILDING LUG FOR REACHABLE SPACE, using Global RP" << endl;
					}
					else if (strcmp(argv[i], "globalrpcard") == 0)
					{
						LUG_FOR = SPACE;
						ALLOW_LEVEL_OFF = TRUE;
						USE_GLOBAL_RP = true;
						USE_CARD_GRP = true;
						cout << "BUILDING LUG FOR REACHABLE SPACE, using Global RP and Card" << endl;
					}
					else if (strcmp(argv[i], "incremental") == 0)
					{
						++i;
						LOOKAHEAD_FOR_STATES = atoi(argv[i]);
						LUG_FOR = INCREMENTAL;
						ALLOW_LEVEL_OFF = FALSE;
						cout << "BUILDING LUG INCREMENTALLY" << endl;
					}
					else if (strcmp(argv[i], "adhoc-reachable") == 0)
					{
						LUG_FOR = AHREACHABLE;
						ALLOW_LEVEL_OFF = TRUE;
						NUMBER_OF_MGS = 1000;
						cout << "BUILDING LUG ADHOC REACHABLE" << endl;
					}
				}
				else if (strcmp(argv[i], "-c") == 0)
				{
					++i;

					if (strcmp(argv[i], "max") == 0)
					{
						COST_PROP = MAXN;
						cout << "LUG USING MAX PROPAGATION" << endl;
					}
					else if (strcmp(argv[i], "sum") == 0)
					{
						COST_PROP = SUM;
						cout << "LUG USING SUM PROPAGATION" << endl;
					}
				}
				else if (strcmp(argv[i], "-out") == 0)
				{
					++i;
					out_file = argv[i];
				}
				else if (strcmp(argv[i], "-fo") == 0)
				{
					OBSERVABILITY = OBS_FULL;
					cout << "assuming full observability" << endl;
				}
				else if (strcmp(argv[i], "-cp") == 0)
				{
					++i;

					if (strcmp(argv[i], "enter") == 0)
					{
						COST_PROP_TECH = CPT_NEW_WORLDS_ONLY;
						cout << "COST PROP FOR ONLY NEW ENTRIES" << endl;
					}
					else if (strcmp(argv[i], "groups") == 0)
					{
						COST_PROP_TECH = CPT_FIXED_GROUPING_NEW_WORLDS;
						cout << "COST PROP FOR ALL ENTRIES" << endl;
					}
				}
				else if (strcmp(argv[i], "-pmg") == 0)
				{
					++i;
					NUMBER_OF_MGS = atof(argv[i]);
					PF_LUG = TRUE;

					ALLOW_LEVEL_OFF = FALSE;
					cout << "Using McLUG, w/ particle increment: " << NUMBER_OF_MGS << endl;
				}
				else if (strcmp(argv[i], "-llw") == 0)
				{
					++i;
					LUG_LEVEL_WORLDS = atof(argv[i]);
					cout << "Extracting LUGRP for worlds reaching goals in last levels : " << LUG_LEVEL_WORLDS << endl;
				}
				else if (strcmp(argv[i], "-rpcost") == 0)
				{
					++i;

					if (strcmp(argv[i], "cost") == 0)
					{
						RP_EFFECT_SELECTION = RP_E_S_COST;
						cout << "RP USING COST TO SELECT ACTIONS" << endl;
					}
					else if (strcmp(argv[i], "coverage") == 0)
					{
						RP_EFFECT_SELECTION = RP_E_S_COVERAGE;
						cout << "RP USING COVERAGE TO SELECT ACTIONS" << endl;
					}
					else if (strcmp(argv[i], "ratio") == 0)
					{
						RP_EFFECT_SELECTION = RP_E_S_RATIO;
						cout << "RP USING COVERAGE/COST RATIO TO SELECT ACTIONS" << endl;
					}
				}
				else if (strcmp(argv[i], "-r") == 0)
				{
					cout << "REPROP COST FOR EACH RP" << endl;
					COST_REPROP = TRUE;
				}
				else if (strcmp(argv[i], "-ha") == 0)
				{
					HELPFUL_ACTS = TRUE;
					cout << "Using helpful acts" << endl;
				}
				else if (strcmp(argv[i], "-ccmax") == 0)
				{
					cout << "ChildCombo = MAX" << endl;
					CHILDCOMBO = CCMAX;
				}
				else if (strcmp(argv[i], "-mutex") == 0)
				{
					cout << "COMPUTING MUTEXES" << endl;
					MUTEX_SCHEME = MS_REGULAR;
				}
				else if (strcmp(argv[i], "-nonsense") == 0)
				{
					cout << "Looking for Conformant Plan" << endl;
					USESENSORS = FALSE;
				}
				else if (strcmp(argv[i], "-time") == 0)
				{
					++i;
					respect_time = false;
					sscanf(argv[i], "%d", &allowed_time);
				}
			}
		}

#ifdef PPDDL_PARSER
		double old = NUMBER_OF_MGS;
		ALLOW_LEVEL_OFF = FALSE;
		HEURISTYPE = LUGRP;
		try
		{
			clock_t groundingStartTime = clock();

			verbosity = 1;
			my_problem = (*(Problem::begin())).second;

			if (PF_LUG)
			{
				NUMBER_OF_MGS = 8192; // set to a constant to generate this many

				for (int i = 1; i <= (NUMBER_OF_MGS); i++)
				{
					if ((2 << (i - 1)) >= (NUMBER_OF_MGS))
					{
						num_new_labels = i;
						break;
					}
				}
			}
			std::cout << "=====================================\n";
			std::cout << "start to solve the problem.\n";
			solve_problem(*my_problem, 1.0, 0.0);

			cout << "Grounding/Instantiation Time: " << ((float)(clock() - groundingStartTime) / CLOCKS_PER_SEC) << endl;
      		printBDD(b_initial_state);
			
			if ((*my_problem).goal_cnf())
			{
				cout << "goal is cnf:" << endl;

				bdd_goal_cnf(&goal_cnf);
			}

			if (my_problem->domain().requirements.non_deterministic)
			{
				std::cout << "nondeterministic goal threshold = 1.0\n";
				goal_threshold = 1.0;
			}
			else if (my_problem->domain().requirements.probabilistic)
			{
				std::cout << "probabilistic goal threshold = tau\n";
				goal_threshold = (*my_problem).tau();
			}
			std::cout << "start to set up the discount and horizon\n";
			gDiscount = (*my_problem).discount();

			max_horizon = (*my_problem).horizon();

			OPTIMIZE_REWARDS = 1;	  //= my_problem->domain().functions().find_function("reward").second;
			OPTIMIZE_PROBABILITY = 1; // my_problem->domain().functions().find_function("goal-probability").second;

			num_alt_acts = action_preconds.size();

			dname = (char *)(new string(my_problem->domain().name()))->c_str();
			pname = (char *)(new string(my_problem->name()))->c_str();

			if (my_problem->domain().requirements.rewards)
			{
				std::cout << "start to set up the rewards\n";
				// if (my_problem->goal_reward())
				// 	total_goal_reward = -1 * my_problem->goal_reward()->expression().value(my_problem->init_values()).double_value();
				// else
				total_goal_reward = 0;

				cout << "GOAL REWARD = " << total_goal_reward << endl;

				DdNode *fr = Cudd_BddToAdd(manager, b_goal_state), *fr1 = Cudd_addConst(manager, total_goal_reward);
				Cudd_Ref(fr);
				Cudd_Ref(fr1);

				goal_reward = Cudd_addApply(manager, Cudd_addTimes, fr1, fr);
				Cudd_Ref(goal_reward);
				Cudd_RecursiveDeref(manager, fr);
				Cudd_RecursiveDeref(manager, fr1);

				if (verbosity >= 3)
				{
					cout << "goal reward" << endl;
					printBDD(goal_reward);
				}
			}
			else
			{
				std::cout << "goal threshold = 0.0[without goal reward]\n";
				total_goal_reward = 0.0;

			}
		}
		catch (const Exception &e)
		{
			std::cerr << "main: " << e << std::endl;
			exit(0);
		}

		cout << "#ACTIONS = " << num_alt_acts << endl;
		cout << "#EVENTS = " << event_preconds.size() << endl;
		cout << "#PROPOSITIONS = " << num_alt_facts << endl;

		goal_samples = Cudd_ReadLogicZero(manager);
		//    if(PF_LUG){ // generate samples
		//
		//      std::list<int> new_vars;
		//      generate_n_labels(num_new_labels, &all_samples, &new_vars);
		//
		//
		//
		//      NUMBER_OF_MGS=old;    // reset to actual
		//    }

#else
		manager = Cudd_Init(2 * num_alt_facts,
							0,
							CUDD_UNIQUE_SLOTS,
							CUDD_CACHE_SLOTS,
							0);
#ifdef PPDDL_PARSER
		Cudd_SetBackground(manager, Cudd_ReadZero(manager));
#else
		Cudd_SetBackground(manager, Cudd_ReadLogicZero(manager));
#endif

		assert(current_analysis);
		assert(current_analysis->the_problem);
		assert(current_analysis->the_problem->initial_state);

		b_initial_state = groundAnd(0, current_analysis->the_problem->initial_state); // getInitialStateDD(current_analysis->the_problem->initial_state);//goalToBDD(initial_state->getClauses(), TRUE);
		Cudd_Ref(b_initial_state);
		initFacts = num_alt_facts;
		if (CWA)
			initDONE = TRUE;

		goal *g;
		b_goal_state = goalToBDD(g = goal_grounding_helper(NULL, current_analysis->the_problem->the_goal));
		delete g;
		Cudd_Ref(b_goal_state);

		dname = current_analysis->the_problem->domain_name;
		pname = current_analysis->the_problem->name;

		if (respect_time)
			allowed_time = current_analysis->the_problem->allowed_time;
		if (allowed_time >= 0)
		{
			cout << "Given " << (float)allowed_time << "ms to plan" << endl;

			allowed_time -= (clock() - gStartTime) * 1000 / CLOCKS_PER_SEC;
			if (allowed_time <= OUTPUT_OVERHEAD)
			{
				// then using the timer will cause us to be late, so end now
				end_early(SIGALRM);
			}

			// number of seconds remaining
			timer.it_value.tv_sec = allowed_time / 1000;
			// number of milliseconds remaining, minus some insurance
			timer.it_value.tv_usec = (allowed_time % 1000 - OUTPUT_OVERHEAD) * 1000;

			// install the receiver
			signal(SIGALRM, end_early);
			// before the sender
			setitimer(ITIMER_REAL, &timer, 0);
		}

		signal(SIGINT, end_early); // ctrl-c
		signal(SIGQUIT, end_early);
		signal(SIGHUP, end_early); // terminal close

		if (current_analysis->the_domain && current_analysis->the_domain->ops)
		{

			build_all_actions(current_analysis->the_domain->ops, current_analysis->the_problem->objects, b_initial_state);
		}
		else
		{
			cout << "Problem with ops" << endl;
			exit(0);
		}

		alt_act_costs = new double[num_alt_acts];
		int tmp = 0;
		for (action_list a = available_acts; a; a = a->next)
		{
			a->act->index = tmp++;
			alt_act_costs[a->act->index] = a->act->cost;
		}

		cout << "#ACTIONS = " << num_alt_acts << endl;
		cout << "#LITERALS = " << num_alt_facts << endl;
#endif
		if (HEURISTYPE == SLUGRP ||
			HEURISTYPE == LUGRP ||
			HEURISTYPE == LUGLEVEL ||
			HEURISTYPE == LUGSUM ||
			HEURISTYPE == LUGMAX ||
			HEURISTYPE == HRPSUM ||
			HEURISTYPE == HRPMAX ||
			HEURISTYPE == HRPUNION ||
			HEURISTYPE == CORRRP)
		{
			if (HEURISTYPE == HRPSUM ||
				HEURISTYPE == HRPMAX ||
				HEURISTYPE == HRPUNION ||
				LUG_FOR == AHREACHABLE)
			{

				if (NUMBER_OF_MGS > 0.0 && NUMBER_OF_MGS < 1.0)
					K_GRAPH_MAX = (int)ceil(get_sum(b_initial_state) * NUMBER_OF_MGS);
				else if (NUMBER_OF_MGS >= 1.0)
					K_GRAPH_MAX = NUMBER_OF_MGS;
				else
					K_GRAPH_MAX = (int)get_sum(b_initial_state);

				// printBDD(b_initial_state);
				// cout <<"HI " << K_GRAPH_MAX << " " << NUMBER_OF_MGS << endl;
				k_graphs = new kGraphInfo *[K_GRAPH_MAX];
			}
			gnum_cond_effects = 0;
			gnum_cond_effects_pre = 0;
			gbit_operators = NULL;
			used_gbit_operators = NULL;
			gnum_bit_operators = 0;
			gnum_relevant_facts = 0;
		}

		ppddl2st(&(std::cout));

		if (allowed_time > 0)
		{
			// disable the sender
			timer.it_value.tv_sec = 0;
			timer.it_value.tv_usec = 0;
			setitimer(ITIMER_REAL, &timer, 0);
			// before the receiver
			signal(SIGALRM, SIG_DFL);
		}

#ifdef PPDDL_PARSER
		/*
		 * Clean up.
		 */
		state_variables.clear();
		for (std::map<const Action *, DdNode *>::const_iterator ai =
				 action_transitions.begin();
			 ai != action_transitions.end(); ai++)
		{
			Cudd_RecursiveDeref(manager, (*ai).second);
		}
		action_transitions.clear();
		// for (std::map<const Action *, DdNode *>::const_iterator ai =
		// 		 action_rewards.begin();
		// 	 ai != action_rewards.end(); ai++)
		// {
		// 	Cudd_RecursiveDeref(manager, (*ai).second);
		// }
		// action_rewards.clear();
		delete[] varmap;
		delete randomGen;
		for (std::vector<DdNode *>::const_iterator di = identity_bdds.begin();
			 di != identity_bdds.end(); di++)
		{
			Cudd_RecursiveDeref(manager, *di);
		}
		identity_bdds.clear();
		Cudd_RecursiveDeref(manager, identity_bdd);

#else
		outputPlan();
#endif
	}
	catch (const exception &e)
	{
		cout << "caught something: " << e.what() << endl;
	}
}

void set_cubes()
{

	if (my_problem->domain().requirements.non_deterministic)
	{
		if (!current_state_vars)
		{
			current_state_vars = new DdNode *[num_alt_facts];
			for (int i = 0; i < num_alt_facts; i++)
			{
				current_state_vars[i] = Cudd_bddIthVar(manager, 2 * i);
				Cudd_Ref(current_state_vars[i]);
			}
			next_state_vars = new DdNode *[num_alt_facts];
			for (int i = 0; i < num_alt_facts; i++)
			{
				next_state_vars[i] = Cudd_bddIthVar(manager, 2 * i + 1);
				Cudd_Ref(next_state_vars[i]);
				Cudd_bddSetPairIndex(manager, 2 * i, 2 * i + 1);
			}
			current_state_cube = Cudd_bddComputeCube(manager, current_state_vars, 0, num_alt_facts);
			Cudd_Ref(current_state_cube);

			next_state_cube = Cudd_bddComputeCube(manager, next_state_vars, 0, num_alt_facts);
			Cudd_Ref(next_state_cube);
			Cudd_SetVarMap(manager, current_state_vars, next_state_vars, num_alt_facts);
		}
		if (PF_LUG)
		{
			particle_vars = new DdNode *[num_new_labels];
			for (int i = 0; i < num_new_labels; i += 1)
			{
				particle_vars[i] = Cudd_bddIthVar(manager, 2 * i + 1);
				Cudd_Ref(particle_vars[i]);
			}
			particle_cube = Cudd_bddComputeCube(manager, particle_vars, 0, num_new_labels);
			Cudd_Ref(particle_cube);
		}
	}
	else if (my_problem->domain().requirements.probabilistic)
	{
		if (!current_state_vars)
		{
			current_state_vars = new DdNode *[num_alt_facts];
			for (int i = 0; i < num_alt_facts; i += 1)
			{
				current_state_vars[i] = Cudd_addIthVar(manager, 2 * i);
				Cudd_Ref(current_state_vars[i]);
			}
			next_state_vars = new DdNode *[num_alt_facts];
			for (int i = 0; i < num_alt_facts; i += 1)
			{
				next_state_vars[i] = Cudd_addIthVar(manager, 2 * i + 1);
				Cudd_Ref(next_state_vars[i]);
			}
			current_state_cube = Cudd_addComputeCube(manager, current_state_vars, 0, num_alt_facts);
			Cudd_Ref(current_state_cube);
			next_state_cube = Cudd_addComputeCube(manager, next_state_vars, 0, num_alt_facts);
			Cudd_Ref(next_state_cube);
		}
		if (PF_LUG)
		{
			particle_vars = new DdNode *[num_new_labels];
			for (int i = 0; i < num_new_labels; i += 1)
			{
				particle_vars[i] = Cudd_addIthVar(manager, 2 * i + 1);
				Cudd_Ref(particle_vars[i]);
			}
			particle_cube = Cudd_addComputeCube(manager, particle_vars, 0, num_new_labels);
			Cudd_Ref(particle_cube);
		}
	}
}

extern const char *getAction(struct ActionNode *a);
void IPC_write_plan(StateNode *s, std::list<string> *plan)
{
	if (s->Terminal || s->BestAction->act == terminalAction)
		return;

	plan->push_back(getAction(s->BestAction));

	IPC_write_plan(s->BestAction->NextState->State, plan);
}

void IPC_write()
{

	int plan_length, action_length;
	ostringstream plan_string(ostringstream::out);
	ostringstream action_string(ostringstream::out);
	ostringstream mytime(ostringstream::out);

	std::list<string> plan, actions;
	std::list<int> plan_ints;

	time_t ftime = time(NULL);
	tm *now = localtime(&ftime);
	mytime << "__" << now->tm_hour << "_" << now->tm_min << "_" << now->tm_sec;

	string outfile(my_problem->name());
	outfile += mytime.str() + ".out";
	ofstream fout(outfile.c_str());

	IPC_write_plan(Start, &plan);

	plan_length = plan.size();

	for (std::list<string>::iterator i = plan.begin(); i != plan.end(); i++)
	{
		actions.push_back(*i);
	}
	actions.unique();
	action_length = actions.size();

	for (std::list<string>::iterator i = plan.begin(); i != plan.end(); i++)
	{
		int index = 0;
		for (std::list<string>::iterator j = actions.begin(); j != actions.end(); j++)
		{
			if (*i == *j)
			{
				plan_string << " " << index;
				break;
			}
			else
				index++;
		}
	}
	for (std::list<string>::iterator j = actions.begin(); j != actions.end(); j++)
	{
		action_string << " " << *j;
	}

	fout << "0"
		 << "\n%%\n"
		 << action_length << action_string.str()
		 << "\n%%\n"
		 << "linear " << plan_length << plan_string.str() << endl
		 << endl;

	fout.close();
}

/* Translation Assumptions:
   1. Conjunctive Preconditions and goal
   2. Single observable
 */

void ppddlExecPrec2stExecPrec(std::ostream *o)
{
	for (std::map<const Action *, DdNode *>::iterator a = action_preconds.begin();
		 a != action_preconds.end(); a++)
	{
		for (int i = 0; i < num_alt_facts; i++)
		{
			DdNode *p = Cudd_bddIthVar(manager, i * 2);
			Cudd_Ref(p);
			DdNode *n = Cudd_Not(p);
			Cudd_Ref(n);
			if (bdd_entailed(manager, (*a).second, p) ||
				bdd_entailed(manager, (*a).second, n))
			{
				(*o) << "impossible ";
				(*a).first->print((*o), (*my_problem).terms());
				(*o) << " if ";
				if (bdd_entailed(manager, (*a).second, p))
				{
					(*o) << "-";
					dynamic_atoms[i]->print((*o), my_problem->domain().predicates(),
											my_problem->domain().functions(),
											my_problem->terms());
				}
				else if (bdd_entailed(manager, (*a).second, n))
				{
					dynamic_atoms[i]->print((*o), my_problem->domain().predicates(),
											my_problem->domain().functions(),
											my_problem->terms());
				}
				(*o) << endl;
			}
			Cudd_RecursiveDeref(manager, p);
			Cudd_RecursiveDeref(manager, n);
		}
	}
}
void ppddlInvariants2stInvariants(std::ostream *o)
{
}
/**
 * momo007 segment fault location 
 */
void getSTs(DdNode *d, bool actDD, list<pair<pair<DdNode *, int> *, bool> *> *pos, list<pair<pair<DdNode *, int> *, bool> *> *neg)
{
	std::cout << "getSTs\n";
	for (int i = 0; i < num_alt_facts; i++)
	{
		std::cout << i << "\n";
		DdNode *p = Cudd_bddIthVar(manager, (actDD ? 2 * i + 1 : 2 * i));
		Cudd_Ref(p);
		DdNode *n = Cudd_Not(p);
		Cudd_Ref(n);

		DdNode *pa = Cudd_BddToAdd(manager, p);
		Cudd_Ref(pa);
		DdNode *na = Cudd_BddToAdd(manager, n);
		Cudd_Ref(na);
		std::cout << "in getSTs: only support probability[else segment fault]\n";
		DdNode *pd = Cudd_addApply(manager, Cudd_addTimes,
								   pa, d);// 将 pa 和 d进行乘法操作segment error
		Cudd_Ref(pd);

		pair<DdNode *, int> *ppd = new pair<DdNode *, int>(pd, i);
		pair<pair<DdNode *, int> *, bool> *ppd1 =
			new pair<pair<DdNode *, int> *, bool>(ppd, true);
		pos->push_back(ppd1);

		if (pd == Cudd_ReadZero(manager) || actDD)
		{
			DdNode *nd = Cudd_addApply(manager, Cudd_addTimes,
									   na, d);
			Cudd_Ref(nd);

			pair<DdNode *, int> *nnd = new pair<DdNode *, int>(nd, i);
			pair<pair<DdNode *, int> *, bool> *nnd1 =
				new pair<pair<DdNode *, int> *, bool>(nnd, false);
			neg->push_back(nnd1);
		}

		Cudd_RecursiveDeref(manager, p);
		Cudd_RecursiveDeref(manager, n);
		Cudd_RecursiveDeref(manager, pa);
		Cudd_RecursiveDeref(manager, na);
	}
}

void printST(std::ostream *o, char *prefix,
			 pair<pair<DdNode *, int> *, bool> *d,
			 pair<pair<DdNode *, int> *, bool> *dneg,
			 DdNode *origDD)
{

	DdNode *rev_next_state_vars[num_alt_facts];
	for (int i = 0; i < num_alt_facts; i++)
	{
		rev_next_state_vars[i] = Cudd_addIthVar(manager, 2 * (num_alt_facts - i - 1) + 1);
		Cudd_Ref(rev_next_state_vars[i]);
	}

	// don't output if can't make true under any circumstance
	if (d->first->first == Cudd_ReadZero(manager))
		return;

	// don't output if doesn't change value
	DdNode *prev = (d->second ? Cudd_bddIthVar(manager, 2 * (d->first->second)) : Cudd_Not(Cudd_bddIthVar(manager, 2 * (d->first->second))));
	if (0 != strcmp(prefix, "initially") &&
		add_bdd_entailed(manager, d->first->first, prev))
		return;

	DdNode *ddc = Cudd_addComputeCube(manager, next_state_vars, NULL, d->first->second);
	Cudd_Ref(ddc);
	DdNode *ddt = Cudd_addExistAbstract(manager, d->first->first, ddc);
	Cudd_Ref(ddt);

	int *x = new int[2 * num_alt_facts];
	CUDD_VALUE_TYPE value;
	DdNode *stP;

	if (1 || dneg == NULL || d->second)
	{
		stP = ddt;
		Cudd_Ref(stP);
	}
	else
	{
		// d is the neg act eff proposition
		DdNode *dbdd = Cudd_addBddStrictThreshold(manager, ddt, 0.0);
		Cudd_Ref(dbdd);
		DdNode *ddcm = Cudd_addComputeCube(manager, next_state_vars, NULL, d->first->second + 1);
		Cudd_Ref(ddcm);
		DdNode *ddnegt = Cudd_addExistAbstract(manager, dneg->first->first, ddcm); // next_state_cube);
		Cudd_Ref(ddnegt);
		DdNode *dnegbdd = Cudd_addBddStrictThreshold(manager, ddnegt, 0.0);
		Cudd_Ref(dnegbdd);
		DdNode *dinter = Cudd_bddAnd(manager, dbdd, dnegbdd);
		Cudd_Ref(dinter);
		DdNode *dbddM = Cudd_bddAnd(manager, dbdd, Cudd_Not(dinter));
		Cudd_Ref(dbddM);
		DdNode *daddM = Cudd_BddToAdd(manager, dbddM);
		Cudd_Ref(daddM);
		stP = Cudd_addApply(manager, Cudd_addTimes, daddM, ddt);
		Cudd_Ref(stP);
		Cudd_RecursiveDeref(manager, dbdd);
		Cudd_RecursiveDeref(manager, dnegbdd);
		Cudd_RecursiveDeref(manager, dbddM);
		Cudd_RecursiveDeref(manager, daddM);
		Cudd_RecursiveDeref(manager, dinter);
	}

	DdGen *gen = Cudd_FirstCube(manager, stP, &x, &value);

	do
	{

		// skip if value is noop
		if (NULL == strstr(prefix, "obs") &&
			x[2 * d->first->second] == x[2 * d->first->second + 1])
			continue;

		DdNode *tmp_cube = Cudd_CubeArrayToBdd(manager, x);
		Cudd_Ref(tmp_cube);

		// don't print this if its an obs
		if (NULL == strstr(prefix, "obs"))
		{
			(*o) << prefix << (d->second ? " " : " -");
			dynamic_atoms[d->first->second]->print((*o), my_problem->domain().predicates(),
												   my_problem->domain().functions(),
												   my_problem->terms());
		}
		else
		{
			(*o) << prefix;
		}

		bool haveNew = false;

		// is there a previous rule that will fire and give the correlated probability

		if (d->first->second != num_alt_facts - 1)
		{
			DdNode *ddcp2 = Cudd_addComputeCube(manager, next_state_vars, NULL, d->first->second + 1);
			Cudd_Ref(ddcp2);
			DdNode *ddtp2 = Cudd_addExistAbstract(manager, origDD, ddcp2);
			Cudd_Ref(ddtp2);

			DdNode *tmp_cubeA2 = Cudd_BddToAdd(manager, tmp_cube);
			Cudd_Ref(tmp_cubeA2);
			DdNode *valA2 = Cudd_addConst(manager, value);
			Cudd_Ref(valA2);
			DdNode *tmpValA2 = Cudd_addApply(manager, Cudd_addTimes, tmp_cubeA2, valA2);
			Cudd_Ref(tmpValA2);

			DdNode *val2 = Cudd_addApply(manager, Cudd_addDivide, tmpValA2, ddtp2);
			Cudd_Ref(val2);
			// printBDD(val2);
			int *x1 = new int[2 * num_alt_facts];
			CUDD_VALUE_TYPE value1;
			DdGen *gen1 = Cudd_FirstCube(manager, val2, &x1, &value1);
			value = value1;
			haveNew = false;
		}

		for (int i = num_alt_facts - 1; (0 && i > d->first->second && 0 != strcmp(prefix, "initially")); i--)
		{
			DdNode *ddcp = Cudd_addComputeCube(manager, next_state_vars, NULL, i);
			Cudd_Ref(ddcp);
			DdNode *ddtp = Cudd_addExistAbstract(manager, origDD, ddcp);
			Cudd_Ref(ddtp);

			DdNode *tmp_cubeA = Cudd_BddToAdd(manager, tmp_cube);
			Cudd_Ref(tmp_cubeA);
			DdNode *valA = Cudd_addConst(manager, 1.0); // value
			Cudd_Ref(valA);
			DdNode *tmpValA = Cudd_addApply(manager, Cudd_addTimes, tmp_cubeA, valA);
			Cudd_Ref(tmpValA);

			DdNode *app = Cudd_addApply(manager, Cudd_addTimes, ddtp, tmpValA);
			Cudd_Ref(app);

			if (tmpValA != app)
			{

				// This previous rule probabilistically changed value of current var
				// Does this rule further modify the probability of current var?
				// Yes: value = (this rule)(product of pr of all prev rule)
				// No: value = (1.0)(product of pr of all prev rule)

				// ri = vi/(r0*r1*...*ri-1)

				DdNode *ddcp1 = Cudd_addComputeCube(manager, next_state_vars, NULL, num_alt_facts);
				Cudd_Ref(ddcp1);

				DdNode **vars = new DdNode *[num_alt_facts - d->first->second - 1];

				for (int j = num_alt_facts - 1; (j > d->first->second); j--)
				{
					Cudd_RecursiveDeref(manager, ddcp);
					DdNode *ddcp = Cudd_addComputeCube(manager, next_state_vars, NULL, j);
					Cudd_Ref(ddcp);
					int ind = 0;
					for (int k = d->first->second + 1; k < num_alt_facts; k++)
					{
						if (k != j)
						{
							vars[ind++] = Cudd_addIthVar(manager, 2 * k + 1);
						}
					}
					DdNode *abstCube = Cudd_addComputeCube(manager, vars, NULL, ind);
					Cudd_Ref(abstCube);

					DdNode *ddcpja = Cudd_addApply(manager, Cudd_addTimes, ddcp, abstCube);
					Cudd_Ref(ddcpja);
					DdNode *ddtp1 = Cudd_addExistAbstract(manager, origDD, ddcpja);
					Cudd_Ref(ddtp1);

					DdNode *prRule1 = Cudd_addApply(manager, Cudd_addDivide, ddtp1, tmpValA);
					Cudd_Ref(prRule1);

					DdNode *acc = Cudd_addApply(manager, Cudd_addTimes, tmpValA, prRule1); // ddtp1);
					Cudd_Ref(acc);
					Cudd_RecursiveDeref(manager, tmpValA);
					tmpValA = acc;
					Cudd_Ref(tmpValA);
					Cudd_RecursiveDeref(manager, acc);

					for (int k = 0; k < (num_alt_facts - d->first->second - 1); k++)
					{
						vars[k] = NULL;
					}
					Cudd_RecursiveDeref(manager, ddcpja);
					Cudd_RecursiveDeref(manager, ddtp1);
				}
				Cudd_RecursiveDeref(manager, ddcp1);

				DdNode *valueA = Cudd_addConst(manager, value);
				Cudd_Ref(valueA);
				DdNode *tmpValueA = Cudd_addApply(manager, Cudd_addTimes, tmp_cubeA, valueA);
				Cudd_Ref(tmpValueA);

				DdNode *prRule = Cudd_addApply(manager, Cudd_addDivide, tmpValueA, tmpValA);
				Cudd_Ref(prRule);

				int *x1 = new int[2 * num_alt_facts];
				CUDD_VALUE_TYPE value1;
				DdGen *gen1 = Cudd_FirstCube(manager, prRule, &x1, &value1);
				value = value1;
				haveNew = false;
				break;
			}
			Cudd_RecursiveDeref(manager, ddcp);
			Cudd_RecursiveDeref(manager, ddtp);
			Cudd_RecursiveDeref(manager, app);
			Cudd_RecursiveDeref(manager, tmp_cubeA);
			Cudd_RecursiveDeref(manager, valA);
			Cudd_RecursiveDeref(manager, tmpValA);
			if (haveNew)
				break;
		}

		if (0 != strcmp(prefix, "initially") ||
			origDD != d->first->first)
		{
			(*o) << " withp " << (haveNew ? 1.0 : value);

			// there is more than one way to get value
			if (0 != strcmp(prefix, "initially") &&
				(NULL != strstr(prefix, "obs") ||
				 tmp_cube != d->first->first))
			{
				(*o) << " if";

				bool useAnd = false;
				for (int i = 0; i < 2 * num_alt_facts; i++)
				{
					// don't print out new value
					if (i == 2 * (d->first->second) + 1)
						continue;

					// don't print if it is a dont' care
					DdNode *p = (x[i] ? Cudd_bddIthVar(manager, i) : Cudd_Not(Cudd_bddIthVar(manager, i)));
					if (0 && add_bdd_entailed(manager, origDD, p))
						continue;

					// don't print if supposed to be abstracted
					DdNode *vi = Cudd_addIthVar(manager, i);
					Cudd_Ref(vi);
					DdNode *vid = Cudd_addApply(manager, Cudd_addTimes, vi, ddc);
					Cudd_Ref(vid);
					if (vid == ddc)
					{
						Cudd_RecursiveDeref(manager, vi);
						Cudd_RecursiveDeref(manager, vid);
						continue;
					}
					Cudd_RecursiveDeref(manager, vi);
					Cudd_RecursiveDeref(manager, vid);

					// don't print if don't care
					if (x[i] != 0 && x[i] != 1)
						continue;

					if (useAnd)
						(*o) << " and";

					(*o) << (i % 2 == 0 ? " " : " new");
					(*o) << (x[i] == 1 ? " " : " -");
					dynamic_atoms[(i % 2 == 0 ? i / 2 : (i - 1) / 2)]->print((*o), my_problem->domain().predicates(),
																			 my_problem->domain().functions(),
																			 my_problem->terms());

					useAnd = true;
				}
			}
		}

		(*o) << endl;

		Cudd_RecursiveDeref(manager, tmp_cube);
	} while (Cudd_NextCube(gen, &x, &value));
	delete[] x;
}

bool stLessThan(pair<pair<DdNode *, int> *, bool> *a, pair<pair<DdNode *, int> *, bool> *b)
{
	return (a->first->second > b->first->second ||
			(a->first->second == b->first->second &&
			 a->second == true));
}

void ppddlObs2stObs(std::ostream *o)
{

	int obsNum = 0;
	for (std::map<const Action *, std::list<DdNode *> *>::iterator a = action_observations.begin();
		 a != action_observations.end(); a++)
	{
		if ((*a).second)
			(*o) << "obs" << obsNum++ << endl;
	}
}
/**
 * original version only probability problem support 
 */
// void ppddlInit2stInit(std::ostream *o)
// {
// 	list<pair<pair<DdNode *, int> *, bool> *> *posInitSTs = new list<pair<pair<DdNode *, int> *, bool> *>();
// 	list<pair<pair<DdNode *, int> *, bool> *> *negInitSTs = new list<pair<pair<DdNode *, int> *, bool> *>();
// 	getSTs(b_initial_state, false, posInitSTs, negInitSTs);

// 	for (list<pair<pair<DdNode *, int> *, bool> *>::iterator i =
// 			 posInitSTs->begin();
// 		 i != posInitSTs->end(); i++)
// 	{
// 		printST(o, "initially", *i, NULL, b_initial_state);
// 	}
// 	for (list<pair<pair<DdNode *, int> *, bool> *>::iterator i =
// 			 negInitSTs->begin();
// 		 i != negInitSTs->end(); i++)
// 	{
// 		printST(o, "initially", *i, NULL, b_initial_state);
// 	}
// 	(*o) << endl;
// }
/**
 * momo007 2022.05.12 
 * 新增一个输出初始状态命题的情况，原始仅支持概率的初始状态会报错
 */
void ppddlInit2stInit(std::ostream *o)
{
	for (int i = 0; i < num_alt_facts; i++)
	{
		(*o) << i << endl;
		if (bdd_entailed(manager, b_initial_state, Cudd_bddIthVar(manager, 2 * i)))
		{
			(*o) << "initialy ";
			dynamic_atoms[i]->print((*o), my_problem->domain().predicates(),
									my_problem->domain().functions(),
									my_problem->terms());
			(*o) << endl;
		}
		else if (bdd_entailed(manager, b_initial_state, Cudd_Not(Cudd_bddIthVar(manager, 2 * i))))
		{
			(*o) << "initially -";
			dynamic_atoms[i]->print((*o), my_problem->domain().predicates(),
									my_problem->domain().functions(),
									my_problem->terms());
			(*o) << endl;
		}
		else
		{
			 (*o) << "initially unk";
			dynamic_atoms[i]->print((*o), my_problem->domain().predicates(),
									my_problem->domain().functions(),
									my_problem->terms());
			(*o) << endl;
		}
	}
	(*o) << endl;

}
void ppddlGoal2stGoal(std::ostream *o)
{
	// 输出全部的状态变量
	for (int i = 0; i < num_alt_facts; i++)
	{
		// 满足为true
		if (bdd_entailed(manager, b_goal_state, Cudd_bddIthVar(manager, 2 * i)))
		{
			(*o) << "finally ";
			dynamic_atoms[i]->print((*o), my_problem->domain().predicates(),
									my_problem->domain().functions(),
									my_problem->terms());
			(*o) << endl;
		}
		// 满足为false
		if (bdd_entailed(manager, b_goal_state, Cudd_Not(Cudd_bddIthVar(manager, 2 * i))))
		{
			(*o) << "finally -";
			dynamic_atoms[i]->print((*o), my_problem->domain().predicates(),
									my_problem->domain().functions(),
									my_problem->terms());
			(*o) << endl;
		}
	}
	(*o) << endl;
}

/**
 * original version, only DBN version support
 */
void ppddlActs2stActs(std::ostream *o)
{
	int onum = 0;
	for (std::map<const Action *, DdNode *>::iterator a = action_transitions.begin();
		a != action_transitions.end(); a++)
	{
		list<pair<pair<DdNode *, int> *, bool> *> *posActSTs = new list<pair<pair<DdNode *, int> *, bool> *>();
		list<pair<pair<DdNode *, int> *, bool> *> *negActSTs = new list<pair<pair<DdNode *, int> *, bool> *>();
		getSTs((*a).second, true, posActSTs, negActSTs);
		posActSTs->sort(stLessThan);
		negActSTs->sort(stLessThan);

		ostringstream s(ostringstream::out);
		(*a).first->print(s, (*my_problem).terms());
		s << " causes";

		list<pair<pair<DdNode *, int> *, bool> *>::iterator i2 = negActSTs->begin();
		for (list<pair<pair<DdNode *, int> *, bool> *>::iterator i = posActSTs->begin();
			i != posActSTs->end(); i++, i2++)
		{
			printST(o, (char *)(s.str().c_str()), *i, *i2, (*a).second);
			printST(o, (char *)(s.str().c_str()), *i2, *i, (*a).second);
		}

		if (action_observations.find((*a).first) != action_observations.end())
		{
			for (std::list<DdNode *>::iterator ob = action_observations[(*a).first]->begin();
				 ob != action_observations[(*a).first]->end(); ob++)
			{
				ostringstream so(ostringstream::out);
				(*a).first->print(so, (*my_problem).terms());
				so << " causes obs" << onum;
				pair<DdNode *, int> *o1 = new pair<DdNode *, int>(*ob, onum);
				pair<pair<DdNode *, int> *, bool> *o2 = new pair<pair<DdNode *, int> *, bool>(o1, true);
				printST(o, (char *)(so.str().c_str()), o2, NULL, (*ob));
				break;
			}
		}
		(*o) << endl;
		onum++;
	}
}

/**
 *
 */
void ppddl2st(std::ostream *o)
{
	(*o) << "===========================\nstart print the info about the domain\n";

	(*o) << "ACTIONS:" << endl;
	for (std::map<const Action *, DdNode *>::iterator a = action_preconds.begin();
		 a != action_preconds.end(); a++)
	{
		(*a).first->print((*o), (*my_problem).terms());
		(*o) << endl;
		groundActionDD(*((*a).first));
	}
	(*o) << "END" << endl
		 << endl;

	(*o) << "FLUENTS:" << endl;
	for (std::map<int, const Atom *>::const_iterator vi =
			 dynamic_atoms.begin();
		 vi != dynamic_atoms.end(); vi++)
	{
		(*vi).second->print((*o), my_problem->domain().predicates(),
							my_problem->domain().functions(),
							my_problem->terms());
		(*o) << std::endl;
	}
	(*o) << "END" << endl
		 << endl;

	(*o) << "OBSERVATIONS:" << endl;
	ppddlObs2stObs(o);
	(*o) << "END" << endl
		 << endl;

	(*o) << "ADD_DOMAIN_KNOWLEDGE:" << endl;
	ppddlExecPrec2stExecPrec(o);
	ppddlInvariants2stInvariants(o);
	(*o) << "END" << endl
		 << endl;

	(*o) << "ACTION_EFFECTS:" << endl;
	(*o) << "part 1 initial situation[done rewrite]\n";
	ppddlInit2stInit(o);
	(*o) << "part 2 goal situation\n";
	ppddlGoal2stGoal(o);
	(*o) << "part 3 Action transition[need rewrite]\n";
	// ppddlActs2stActs(o);
	(*o) << "END" << endl;
}

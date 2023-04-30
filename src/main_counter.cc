#include <string.h>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>
#include <sstream>
#include <ctime>   // time
#include <cstdlib> // srand, rand
#include <set>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "float.h"
using namespace std;

#include "globals.h"
#include "lao_wrapper.h"
#include "search.h"
#include "online_search.h"
#include "astar.h"
#include "ehc.h"
#include "molao.h"
#include "lao.h"
#include "rtdp.h"
#include "moastar.h"
#include "replan.h"
#include "hop.h"
#include "pchop.h"
#include "alao_star.h"
#include "exph.h"
#include "movalfn.h"

#ifdef PPDDL_PARSER
#include "allheaders.h"
extern void collectInit(const Problem *);
extern void generate_n_labels(int, list<DdNode *> *, list<int> *);
extern double gDiscount;
void set_cubes();
void IPC_write();
#else
#include "actions.h"
#include "instantiation.h"
#include "parse.h"
#include "actions.h"
#endif

#include "statistical.h"
#include "randomGenerator.h"
#include "dd.h"
#include "lug.h"
#include "graph_wrapper.h"
#include "kGraphInfo.h"
#include "correlation.h"
#include "sample_size.h"

/* 反例 */
#include "planvalidate.h"


#define OUTPUT_OVERHEAD (10 + 5) /* the number of milliseconds it takes to receive (10) and process (5) the signal */

itimerval timer; // allowed_time

bool respect_time = true;
int num_new_labels = 0;

void end_early(int signal)
{
	//  outputPlan();
	exit(0);
}

extern int gNumStates; // number of states in LAO star search
extern clock_t gStartTime;
extern bool use_gs;
extern bool use_h;

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
string current_file;

/* Parses the given file, and returns true on success. */
static bool read_file(const char *name)
{
	yyin = fopen(name, "r");
	if (yyin == NULL)
		return false;
	else
	{
		current_file = name;
		// 使用yyparse进行解析
		bool success = (yyparse() == 0);
		fclose(yyin);
		return success;
	}
}

int main(int argc, char *argv[])
{

	try
	{
		Search *search = NULL;
		StepSearch *step_search = NULL;
		int seed = 1;
		bool incremental_search = false;

		if (argc < 3 || argv[1] == "--help")
		{
			cout << "Counter Planner - Version 2.2\n"
				 << "Usage: counter <domain-name> <prob-name> [OPTIONS]" << endl
				 << "OPTIONS:" << endl
				 << "Search Algorithm:\n\t <default> \t\t : astar\n\t -s astar \t\t : A*\n"
				 << "Heuristics:\n\t <default> \t\t : h = 0 <Breadth First Search>\n\t -h card  \t\t : cardinality \n\t -h sgrp \t\t : SG relaxed plan \n\t -h mgrpm \t\t : MG max relaxed plan\n\t -h mgrps \t\t : MG sum relaxed plan \n\t -h mgrpu \t\t : MG unioned relaxed plan \n\t -h lugrp \t\t : LUG relaxed plan" << endl;
			return 0;
		}

		if (argc > 3)
		{
			for (int i = 3; i < argc; i++)
			{
				if (strcmp(argv[i], "term") == 0)
				{
					if (i + 1 < argc && argv[i + 1][0] != '-')
					{
						search_goal_threshold = atof(argv[++i]);
						cout << "Command line [search] P(G) = " << search_goal_threshold << endl;
					}
				}
			} 

			for (int i = 3; i < argc; i++)
			{
				if (strcmp(argv[i], "-s") == 0)
				{
					i++;
					if (strcmp(argv[i], "astar") == 0)
					{
						cout << "A* Search" << endl;
						step_search = new AStar();
					}
				}
				if (strcmp(argv[i], "-p") == 0)
				{
					++i;
					if(strcmp(argv[i], "fog") == 0) // forgetting
					{
						progMode = FORGETTING;
					}
					else if(strcmp(argv[i], "part") == 0) // partition merge
					{
						progMode = PARTITION_MERGE;
					}
					else if(strcmp(argv[i], "def") == 0) // definability progress
					{
						progMode = DEFINABILITY;
					}
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
			}
		}

		if (seed <= 1)
			seed = time(NULL);
		srand(seed);
		cout << "Random Seed = " << seed << endl;
		cout << "Verbosity: ";
		verbosity = 0;
		cout << " (" << verbosity << ")\n";
		// 开始计时
		gStartTime = clock();
		randomGen = new randomGenerator(100000); // use a million random nums

		// 读取domain和instance文件进行解析
		if ((read_file(argv[1]) && (Problem::begin()) != (Problem::end())) || read_file(argv[2]))
			cout << "done Parsing\n==================================" << endl;
		else
		{
			cout << "Parse Error" << endl;
			exit(0);
		}
		clock_t groundingStartTime;
		clock_t groundingEndTime;
		try
		{
			// clock_t groundingStartTime = clock();
			groundingStartTime = clock();
			my_problem = (*(Problem::begin())).second;
			solve_problem(*my_problem, 1.0, 0.0);	
			groundingEndTime = clock();
			// printBDD(b_initial_state);	// 此时的b_initial_state是一个可能的初始状态
			cout << "Grounding/Instantiation Time: " << ((float)(clock() - groundingStartTime) / CLOCKS_PER_SEC) << endl;
			cout << "==================================\n";

			if ((*my_problem).goal_cnf())
				bdd_goal_cnf(&goal_cnf);

			if (my_problem->domain().requirements.non_deterministic)
			{
				std::cout << "nondet goal\n";
				goal_threshold = 1.0;
				if (SENSORS)
				{
					cout << "SWITCHING TO AO* SEARCH" << endl;
					if (search != NULL)
						delete search;
					search = new LAOStar();
				}
			}

			OPTIMIZE_REWARDS = 1;
			OPTIMIZE_PROBABILITY = 1;
			num_alt_acts = action_preconds.size();
			dname = (char *)(new string(my_problem->domain().name()))->c_str();
			pname = (char *)(new string(my_problem->name()))->c_str();
			total_goal_reward = 0.0;
		}
		catch (const Exception &e)
		{
			cerr << "main: " << e << endl;
			exit(0);
		}

		// cout << "#ACTIONS = " << num_alt_acts << endl;
		// cout << "#EVENTS = " << event_preconds.size() << endl;
		// cout << "#PROPOSITIONS = " << num_alt_facts << endl;
		goal_samples = Cudd_ReadLogicZero(manager);

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
					K_GRAPH_MAX = (int)NUMBER_OF_MGS;
				else
					K_GRAPH_MAX = (int)get_sum(b_initial_state);

				k_graphs = new kGraphInfo *[K_GRAPH_MAX];
			}
			gnum_cond_effects = 0;
			gnum_cond_effects_pre = 0;
			gbit_operators = NULL;
			used_gbit_operators = NULL;
			gnum_bit_operators = 0;
			gnum_relevant_facts = 0;

			if (!USE_CARD_GRP)
				initLUG(&action_preconds, b_goal_state);
		}

		if (step_search != NULL)
		{
			if (search == NULL)
				search = step_search;
			else
				delete step_search;
			step_search = NULL;
		}
		if (search == NULL)
		{
			// std::cout << "using EHC() algorithm\n";
			// search = new EHC();
			cout << "A* Search" << endl;
			search = new AStar();
		}

		search->init(num_alt_acts, b_initial_state, b_goal_state); 
		cout << "初始化candidateplan" << endl;
		search->search(); 

		int iteration = 0; // 循环次数
		Planvalidate p;
		init_states = formula_bdd(my_problem->init_formula(),false);
		Cudd_Ref(init_states);
		Cudd_Ref(b_initial_state);
		DdNode *tmp = Cudd_bddAnd(manager, Cudd_Not(b_initial_state), init_states);
		Cudd_RecursiveDeref(manager, b_initial_state);
		Cudd_Ref(tmp);
		Cudd_RecursiveDeref(manager, init_states);
		init_states = tmp;

		clock_t total_validate_time = 0;
		clock_t total_plan_time = 0;
		clock_t start_time = clock();
		for (;;)
		{
			++iteration;
			{
				
				clock_t validate_start_time = clock();

				if (!p.planvalidate(counterexample))
				{
					std::cout << "未找到反例，查找结束" << std::endl;
					// std::cout << "输出规划相关信息" << std::endl;
					// for (int i = 0; i < candidateplan.size(); i++)
					// {
					// 	candidateplan[i]->print(std::cout, my_problem->terms());
					// 	std::cout << "\n";
					// }
					std::cout << "Length = " << candidateplan.size() << endl;
					outputPlan();
					break;
				}
				clock_t validate_end_time = clock();
				total_validate_time += validate_end_time - validate_start_time;
			}

			candidateplan.clear(); 

			Cudd_Ref(b_initial_state);
			DdNode *tmp2 = Cudd_bddOr(manager, counterexample, b_initial_state);
			Cudd_Ref(tmp2);
			Cudd_RecursiveDeref(manager, b_initial_state);
			b_initial_state = tmp2;
			// printBDD(b_initial_state);

			Cudd_Ref(init_states);
			Cudd_Ref(counterexample);
			DdNode *tmp1 = Cudd_bddAnd(manager, Cudd_Not(counterexample), init_states);
			Cudd_RecursiveDeref(manager, counterexample);
			Cudd_Ref(tmp1);
			Cudd_RecursiveDeref(manager, init_states);
			init_states = tmp1;

			// std::cout << "当前样本：" << std::endl;
			// printBDD(b_initial_state);

			// 搜索规划
			clock_t plan_start_time = clock();
			{
				search->init(num_alt_acts, b_initial_state, b_goal_state);
				cout << "starting search" << endl;
				std::cout << "call search()\n";
				search->search();  // 将规划结果传递给candidateplan
				std::cout << "===============本次规划结束=================" << std::endl;
				clock_t plan_end_time = clock();
        		total_plan_time += plan_end_time - plan_start_time;
				if (allowed_time > 0)
				{
					// disable the sender
					timer.it_value.tv_sec = 0;
					timer.it_value.tv_usec = 0;
					setitimer(ITIMER_REAL, &timer, 0);

					// before the receiver
					signal(SIGALRM, SIG_DFL);
				}

				if (candidateplan.empty())
				{
					cout << "The Problem is Unsolvable Or Some other error" << endl;
					return 0;
				}
			}
			
		}
		// std::cout << "初始状态集合|S| = " << Cudd_DagSize(init_states)<< std::endl;
		
		std::cout << "反例 = " << ((float)total_validate_time / CLOCKS_PER_SEC) << " sec" << std::endl;
		std::cout << "规划 = " << ((float)total_plan_time / CLOCKS_PER_SEC) << " sec" << std::endl;
		std::cout << "实例化 = " << ((float)(groundingEndTime - groundingStartTime) / CLOCKS_PER_SEC) << " sec" << endl;
		std::cout << "样本大小 = " << Cudd_DagSize(b_initial_state)<< std::endl;
		std::cout << "迭代次数 = " << iteration << std::endl;
	}
	catch (const exception &e)
	{
		cout << "caught something: " << e.what() << endl;
	}
}


void set_cubes()
{
	// 非确定
	if (my_problem->domain().requirements.non_deterministic)
	{
		if (!current_state_vars)
		{
			// 当前状态变量，偶数位置
			current_state_vars = new DdNode *[num_alt_facts];
			for (int i = 0; i < num_alt_facts; i++)
			{
				current_state_vars[i] = Cudd_bddIthVar(manager, 2 * i);
				Cudd_Ref(current_state_vars[i]);
			}
			// 下一状态变量，奇数位置
			next_state_vars = new DdNode *[num_alt_facts];
			for (int i = 0; i < num_alt_facts; i++)
			{
				next_state_vars[i] = Cudd_bddIthVar(manager, 2 * i + 1);
				Cudd_Ref(next_state_vars[i]);
				Cudd_bddSetPairIndex(manager, 2 * i, 2 * i + 1);
			}
			// 分别为当前状态变量和后继状态变量创建Cude
			current_state_cube = Cudd_bddComputeCube(manager, current_state_vars, 0, num_alt_facts);
			Cudd_Ref(current_state_cube);

			next_state_cube = Cudd_bddComputeCube(manager, next_state_vars, 0, num_alt_facts);
			Cudd_Ref(next_state_cube);
			// 创建状态变量对应关系
			Cudd_SetVarMap(manager, current_state_vars, next_state_vars, num_alt_facts);
		}

		// 使用了LUG,创建Label的BDD
		if (PF_LUG)
		{
			particle_vars = new DdNode *[num_new_labels];
			for (int i = 0; i < num_new_labels; i++)
			{
				particle_vars[i] = Cudd_bddIthVar(manager, 2 * i + 1);
				Cudd_Ref(particle_vars[i]);
			}
			particle_cube = Cudd_bddComputeCube(manager, particle_vars, 0, num_new_labels);
			Cudd_Ref(particle_cube);
		}
	}
	
}
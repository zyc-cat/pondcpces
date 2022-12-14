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
			cout << "PONDbeta Planner - Version 2.2\n";
			return 0;
		}

		if (argc > 3)
		{
			for (int i = 3; i < argc; i++)
			{
				if (strcmp(argv[i], "term") == 0)
				{ // 查找term的参数
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
					
					else if (strcmp(argv[i], "aostar") == 0)
					{
						cout << "AO* Search" << endl;
						search = new LAOStar();
					}
				}

				if (strcmp(argv[i], "-w") == 0)
				{
					GWEIGHT = atoi(argv[++i]);
					cout << "HWEIGHT = " << GWEIGHT << endl;
				}
				if (strcmp(argv[i], "-i") == 0)
				{
					incremental_search = true;
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

		try
		{
			clock_t groundingStartTime = clock();
			// 获取problem
			my_problem = (*(Problem::begin())).second;
			// zyc11.14: 此时的my_problem传递给solve_problem的应该是一个可能的初始状态(collectInit)
			solve_problem(*my_problem, 1.0, 0.0);  
			printBDD(b_initial_state);   // 当前初始状态是一个可能的初始状态
			cout << "Grounding/Instantiation Time: " << ((float)(clock() - groundingStartTime) / CLOCKS_PER_SEC) << endl;
			cout << "==================================\n";
			// 	
			if ((*my_problem).goal_cnf())
				bdd_goal_cnf(&goal_cnf);
			// 非确定性
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

		cout << "#ACTIONS = " << num_alt_acts << endl;
		cout << "#EVENTS = " << event_preconds.size() << endl;
		cout << "#PROPOSITIONS = " << num_alt_facts << endl;
		goal_samples = Cudd_ReadLogicZero(manager);
		// 这里的kGraph用于后续的启发式
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
			// MG heuristic
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
			// 初始化bit operator
			gnum_cond_effects = 0;
			gnum_cond_effects_pre = 0;
			gbit_operators = NULL;
			used_gbit_operators = NULL;
			gnum_bit_operators = 0;
			gnum_relevant_facts = 0;

			if (!USE_CARD_GRP)// 默认执行
				initLUG(&action_preconds, b_goal_state);
		}

		if (step_search != NULL)//如果参数使用了step_search的子类
		{
			if (search == NULL)
				search = step_search;
			else
				delete step_search;
			step_search = NULL;
		}
		if (search == NULL)// 没有指定搜索算法，则默认强制爬山算法
		{
			std::cout << "using EHC() algorithm\n";
			search = new EHC();
		}

		search->init(num_alt_acts, b_initial_state, b_goal_state);  // b_initial_state为单个初始状态
		cout << "初始化candidateplan" << endl;
		search->search();  // -> 获取到针对选取单个初始状态的candidateplan

		int iteration = 0; // 循环次数
		Planvalidate p;
		for (;;)
		{  // 查找反例
			++iteration;
			std::cout << "进入规划和查找反例循环" << std::endl;
			{ // 
				std::cout << "开始查找反例" << std::endl;
				if (!p.planvalidate(my_problem, candidateplan, counterexample))
				{
					std::cout << "反例查找结束：" << std::endl;
					std::cout << "输出规划相关信息" << std::endl;
					for (int i = 0; i < candidateplan.size(); i++)
					{
						candidateplan[i]->print(std::cout, my_problem->terms());
						std::cout << "\n";
					}
					std::cout << "最终的规划长度为：" << candidateplan.size() << endl;
					outputPlan();
					break; // 结束for(;;)循环
				}
			}

			candidateplan.clear(); // 将当前候选规划置为空，重新进行规划寻找
			// 但是可能存在bug，比如说无法为当前初始状态找到规划，先试一下

			std::cout << "看是否能正确打印counterexample: " << std::endl;
			printBDD(counterexample);
			// 合并counterexample
			b_initial_state = Cudd_bddOr(manager, counterexample, b_initial_state);
			std::cout << "当前的初始信念状态:" << std::endl;
			printBDD(b_initial_state);
			std::cout << "当前的初始信念状态打印完毕" << std::endl;

			{  // planning
				// 初始化
				std::cout << "开始规划" << std::endl;
				search->init(num_alt_acts, b_initial_state, b_goal_state);
				cout << "starting search" << endl;
				std::cout << "call search()\n";
				search->search();  // 将规划结果传递给candidateplan
				std::cout << "本次规划结束" << std::endl;

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
		std::cout << "循环次数 = " << iteration << std::endl;
	}
	catch (const exception &e)
	{
		cout << "caught something: " << e.what() << endl;
	}
}

// int main(int argc, char *argv[])
// {
// 	try
// 	{
// 		// Search是全部搜索算法的抽象基类，StepSearch基于Searc的抽象基类
// 		Search *search = NULL;
// 		StepSearch *step_search = NULL;
// 		int seed = 1;
// 		bool incremental_search = false;
// 		if (argc < 3 || argv[1] == "--help")
// 		{
// 			cout << "PONDbeta Planner - Version 2.2\n";
// 			return 0;
// 		}
// 		if (argc > 3)
// 		{
// 			// Alan- Temporary backwards compatability
// 			for (int i = 3; i < argc; i++)
// 			{
// 				if (strcmp(argv[i], "term") == 0)
// 				{ // 查找term的参数
// 					if (i + 1 < argc && argv[i + 1][0] != '-')
// 					{
// 						search_goal_threshold = atof(argv[++i]);
// 						cout << "Command line [search] P(G) = " << search_goal_threshold << endl;
// 					}
// 				}
// 			} // end-for
// 			// Once we input some extra parameter, it will planner some special search.
// 			for (int i = 3; i < argc; i++)
// 			{
// 				// 根据传递的参数选择具体的搜索算法类，具体赋值给那个类指针根据继承的类决定
// 				if (strcmp(argv[i], "-s") == 0)
// 				{
// 					i++;
// 					// incluing several search algorithm to judge
// 					if (strcmp(argv[i], "astar") == 0)
// 					{
// 						cout << "A* Search" << endl;
// 						step_search = new AStar();
// 					}				
// 					else if (strcmp(argv[i], "aostar") == 0)
// 					{
// 						cout << "AO* Search" << endl;
// 						search = new LAOStar();
// 					}
// 				}
// 				if (strcmp(argv[i], "-w") == 0)
// 				{
// 					GWEIGHT = atoi(argv[++i]);
// 					cout << "HWEIGHT = " << GWEIGHT << endl;
// 				}
// 				if (strcmp(argv[i], "-i") == 0)
// 				{
// 					incremental_search = true;
// 				}
// 				if (strcmp(argv[i], "-h") == 0)
// 				{
// 					++i;
// 					if (strcmp(argv[i], "corrrp") == 0)
// 					{
// 						cout << "USING Correlation RP" << endl;
// 						HEURISTYPE = CORRRP;
// 						COMPUTE_LABELS = FALSE;
// 						USE_CORRELATION = TRUE;
// 					}
// 					if (strcmp(argv[i], "prrp") == 0)
// 					{
// 						cout << "USING Correlation RP" << endl;
// 						HEURISTYPE = CORRRP;
// 						COMPUTE_LABELS = FALSE;
// 						USE_CORRELATION = FALSE;
// 					}
// 					if (strcmp(argv[i], "slugrp") == 0)
// 					{
// 						cout << "USING SLUGRP" << endl;
// 						HEURISTYPE = SLUGRP;
// 					}
// 					if (strcmp(argv[i], "minslugrp") == 0)
// 					{
// 						cout << "USING MINSLUGRP" << endl;
// 						HEURISTYPE = SLUGRP;
// 						TAKEMINRP = TRUE;
// 					}
// 					if (strcmp(argv[i], "lugrp") == 0)
// 					{
// 						cout << "USING LUGRP" << endl;
// 						HEURISTYPE = LUGRP;
// 					}
// 					if (strcmp(argv[i], "luglevel") == 0)
// 					{
// 						cout << "USING LUGLEVEL" << endl;
// 						HEURISTYPE = LUGLEVEL;
// 					}
// 					if (strcmp(argv[i], "lugsum") == 0)
// 					{
// 						cout << "USING LUGSUM" << endl;
// 						HEURISTYPE = LUGSUM;
// 					}
// 					if (strcmp(argv[i], "lugmax") == 0)
// 					{
// 						cout << "USING LUGMAX" << endl;
// 						HEURISTYPE = LUGMAX;
// 					}
// 					if (strcmp(argv[i], "card") == 0)
// 					{
// 						cout << "USING CARD" << endl;
// 						HEURISTYPE = CARD;
// 					}
// 					if (strcmp(argv[i], "mgrpu") == 0)
// 					{
// 						LUG_FOR = NODE;
// 						ALLOW_LEVEL_OFF = FALSE;
// 						COMPUTE_LABELS = FALSE;
// 						HEURISTYPE = HRPUNION;
// 						cout << "NOT USING LABELS ON PG" << endl;
// 					}
// 					if (strcmp(argv[i], "mgrpm") == 0)
// 					{
// 						LUG_FOR = NODE;
// 						ALLOW_LEVEL_OFF = FALSE;
// 						COMPUTE_LABELS = FALSE;
// 						HEURISTYPE = HRPMAX;
// 						cout << "NOT USING LABELS ON PG" << endl;
// 					}
// 					if (strcmp(argv[i], "mgrps") == 0)
// 					{
// 						LUG_FOR = NODE;
// 						ALLOW_LEVEL_OFF = FALSE;
// 						COMPUTE_LABELS = FALSE;
// 						HEURISTYPE = HRPSUM;
// 						cout << "NOT USING LABELS ON PG" << endl;
// 					}
// 					if (strcmp(argv[i], "sgrp") == 0)
// 					{
// 						LUG_FOR = NODE;
// 						ALLOW_LEVEL_OFF = FALSE;
// 						COMPUTE_LABELS = FALSE;
// 						HEURISTYPE = LUGRP;
// 						cout << "NOT USING LABELS ON PG" << endl;
// 					}
// 					if (strcmp(argv[i], "sglevel") == 0)
// 					{
// 						COMPUTE_LABELS = FALSE;
// 						HEURISTYPE = LUGLEVEL;
// 						cout << "NOT USING LABELS ON PG" << endl;
// 					}
// 					if (strcmp(argv[i], "sgsum") == 0)
// 					{
// 						COMPUTE_LABELS = FALSE;
// 						HEURISTYPE = LUGSUM;
// 						cout << "NOT USING LABELS ON PG" << endl;
// 					}
// 					if (strcmp(argv[i], "sgmax") == 0)
// 					{
// 						COMPUTE_LABELS = FALSE;
// 						HEURISTYPE = LUGMAX;
// 						cout << "NOT USING LABELS ON PG" << endl;
// 					}
// 				}
// 			}
// 		}
// 		if (seed <= 1)
// 			seed = time(NULL);
// 		srand(seed);
// 		cout << "Random Seed = " << seed << endl;
// 		cout << "Verbosity: ";
// 		verbosity = 0;
// 		cout << " (" << verbosity << ")\n";
// 		// 开始计时
// 		gStartTime = clock();
// 		randomGen = new randomGenerator(100000); // use a million random nums
// 		// 读取domain和instance文件进行解析
// 		if ((read_file(argv[1]) && (Problem::begin()) != (Problem::end())) || read_file(argv[2]))
// 			cout << "done Parsing\n==================================" << endl;
// 		else
// 		{
// 			cout << "Parse Error" << endl;
// 			exit(0);
// 		}
// 		try
// 		{
// 			clock_t groundingStartTime = clock();
// 			// 获取problem
// 			my_problem = (*(Problem::begin())).second;
// 			// zyc11.14: 此时的my_problem传递给solve_problem的应该是一个可能的初始状态(collectInit)
// 			solve_problem(*my_problem, 1.0, 0.0);  
// 			printBDD(b_initial_state);
// 			cout << "Grounding/Instantiation Time: " << ((float)(clock() - groundingStartTime) / CLOCKS_PER_SEC) << endl;
// 			cout << "==================================\n";		
// 			if ((*my_problem).goal_cnf())
// 				bdd_goal_cnf(&goal_cnf);
// 			// 非确定性
// 			if (my_problem->domain().requirements.non_deterministic)
// 			{
// 				std::cout << "nondet goal\n";
// 				goal_threshold = 1.0;
// 				if (SENSORS)
// 				{
// 					cout << "SWITCHING TO AO* SEARCH" << endl;
// 					if (search != NULL)
// 						delete search;
// 					search = new LAOStar();
// 				}
// 			}
// 			OPTIMIZE_REWARDS = 1;
// 			OPTIMIZE_PROBABILITY = 1;
// 			num_alt_acts = action_preconds.size();
// 			dname = (char *)(new string(my_problem->domain().name()))->c_str();
// 			pname = (char *)(new string(my_problem->name()))->c_str();
// 			total_goal_reward = 0.0;
// 		}
// 		catch (const Exception &e)
// 		{
// 			cerr << "main: " << e << endl;
// 			exit(0);
// 		}
// 		cout << "#ACTIONS = " << num_alt_acts << endl;
// 		cout << "#EVENTS = " << event_preconds.size() << endl;
// 		cout << "#PROPOSITIONS = " << num_alt_facts << endl;
// 		goal_samples = Cudd_ReadLogicZero(manager);
// 		// 这里的kGraph用于后续的启发式
// 		if (HEURISTYPE == SLUGRP ||
// 			HEURISTYPE == LUGRP ||
// 			HEURISTYPE == LUGLEVEL ||
// 			HEURISTYPE == LUGSUM ||
// 			HEURISTYPE == LUGMAX ||
// 			HEURISTYPE == HRPSUM ||
// 			HEURISTYPE == HRPMAX ||
// 			HEURISTYPE == HRPUNION ||
// 			HEURISTYPE == CORRRP)
// 		{
// 			// MG heuristic
// 			if (HEURISTYPE == HRPSUM ||
// 				HEURISTYPE == HRPMAX ||
// 				HEURISTYPE == HRPUNION ||
// 				LUG_FOR == AHREACHABLE)
// 			{
// 				if (NUMBER_OF_MGS > 0.0 && NUMBER_OF_MGS < 1.0)
// 					K_GRAPH_MAX = (int)ceil(get_sum(b_initial_state) * NUMBER_OF_MGS);
// 				else if (NUMBER_OF_MGS >= 1.0)
// 					K_GRAPH_MAX = (int)NUMBER_OF_MGS;
// 				else
// 					K_GRAPH_MAX = (int)get_sum(b_initial_state);
// 				k_graphs = new kGraphInfo *[K_GRAPH_MAX];
// 			}
// 			// 初始化bit operator
// 			gnum_cond_effects = 0;
// 			gnum_cond_effects_pre = 0;
// 			gbit_operators = NULL;
// 			used_gbit_operators = NULL;
// 			gnum_bit_operators = 0;
// 			gnum_relevant_facts = 0;
// 			if (!USE_CARD_GRP)// 默认执行
// 				initLUG(&action_preconds, b_goal_state);
// 		}
// 		if (step_search != NULL)//如果参数使用了step_search的子类
// 		{
// 			if (search == NULL)
// 				search = step_search;
// 			else
// 				delete step_search;
// 			step_search = NULL;
// 		}
// 		// 没有指定搜索算法，则默认强制爬山算法
// 		if (search == NULL)
// 		{
// 			std::cout << "using EHC() algorithm\n";
// 			search = new EHC();
// 		}
// 		/**
// 		 * --------------------2022.11.14------------------------*
// 		 * 1. 定义并计算候选规划(用solve_problem初始化的单个初始状态)
// 		 * 2. 进入无限循环for(;;)
// 		 * 3. 采样：调用samplegenerator中的computeSingleCPCESCounterExample(candidateplan),其结果赋给反例
// 		 * 4. 判断反例是否为空 
// 		 * 5. if(反例 == NULL) -> 直接打印当前candidateplan相关信息outputPlan()，结束循环
// 		 * 6. 否则，将该反例转化为DdNode*,再与当前初始状态合并(Cudd_bddOr)，再继续执行规划部分
// 		 * 7. 规划：，重新Search-> init(动作数量，当前初始状态，目标状态), 调用Search()，其规划结果赋给candidateplan
// 		 * 6. if(candidateplan== NULL)输出问题不可解决，否则又开始循环
// 		 */
// 		/**
// 		 * (1) candidateplan(候选规划)： std::vector<const Action*>
// 		 * (2) counterexample(反例)： StateFormula*  --> 验证规划可行性改为DdNode*
// 		 * (3) 当前初始状态：
// 		 * 			合并反例与当前初始状态 -->DdNode*合并(Cudd_bddOr)
// 		 * 
// 		*/
// 		search->init(num_alt_acts, b_initial_state, b_goal_state);  // 单个初始状态
// 		cout << "初始化candidateplan" << endl;
// 		search->search();  // -> 获取到针对单个初始状态的candidateplan
// 		std::cout << "初始化candidateplan结束" << std::endl;
// 		int iteration = 0; // 循环次数
// 		Planvalidate p;
// 		for (;;)
// 		{
// 			// ++iteration;
// 			// std::cout << "进入规划和采样循环" << std::endl;
// 			//  // sampling
// 			// std::cout << "开始采样" << std::endl;
// 			// if (p.planvalidate(my_problem, candidateplan, counterexample))
// 			// {
// 			// 	// 此段内表示有counterexample
// 			// 	std::cout << "采样成功(初始化初始状态)" <<std::endl;
// 			// 	// 将反例counterexample合并到b_initial_state初始状态中
// 			// 	std::cout << "看看如果找到了反例, 是否能正确打印counterexample" << std::endl;
// 			// 	printBDD(&counterexample);  // 打印出来是个空指针 空指针错误Segmentation Fault
// 			// 	b_initial_state = Cudd_bddOr(manager, &counterexample, b_initial_state);
// 			// 	// planning
// 			// 	// 初始化
// 			// 	std::cout << "开始规划" << std::endl;
// 			// 	search->init(num_alt_acts, b_initial_state, b_goal_state);
// 			// 	cout << "starting search" << endl;
// 			// 	std::cout << "call search()\n";
// 			// 	search->search();  // 将规划结果传递给candidateplan
// 			// 	std::cout << "本次规划结束" << std::endl;	
// 			// 	if (allowed_time > 0)
// 			// 	{
// 			// 		// disable the sender
// 			// 		timer.it_value.tv_sec = 0;
// 			// 		timer.it_value.tv_usec = 0;
// 			// 		setitimer(ITIMER_REAL, &timer, 0);
// 			// 		// before the receiver
// 			// 		signal(SIGALRM, SIG_DFL);
// 			// 	}
// 			// }
// 			// if (candidateplan.empty())
// 			// {
// 			// 	cout << "The Problem is Unsolvable Or Some other error" << endl;
// 			// 	return 0;
// 			// }	
// 			// // 否则，输出输出当前规划相关信息
// 			// std::cout << "输出规划相关信息" << std::endl;
// 			// outputPlan();
// 			// break; // 结束for(;;)循环
// 			//  * 1. 将反例StateFormula *转化为Ddnode *
// 			//  * 2. Cudd_bddOr(manager, DdNode*, DdNode*)合并反例和当前初始状态
// 			// DdNode *b_counterexample = formula_bdd(*counterexample,false);
// 			// b_initial_state = Cudd_bddOr(manager, b_counterexample, b_initial_state);
// /*=============================================================================================================*/	
// 			++iteration;
// 			std::cout << "进入规划和采样循环" << std::endl;
// 			{ // sampling
// 				std::cout << "开始采样" << std::endl;
// 				if (!p.planvalidate(my_problem, candidateplan, counterexample))
// 				{
// 					// counterexample在这里本来就是空的
// 					std::cout << "输出规划相关信息" << std::endl;
// 					for (int i = 0; i < candidateplan.size(); i++)
// 					{
// 						candidateplan[i]->print(std::cout, my_problem->terms());
// 						std::cout << "\n";
// 					}
// 					outputPlan();
// 					break; // 结束for(;;)循环
// 				}
// 			}
// 			// 否则将反例counterexample合并到b_initial_state初始状态中
// 			// counterexample重定义为DdNode*，直接合并
// 			std::cout << "看看如果找到了反例, 是否能正确打印counterexample" << std::endl;
// 			printBDD(counterexample);		
// 			// std::cout << "flag:是否正确得到了counterexample" << std::endl;
// 			// std::cout << "打印一下candidateplan和b_initial_state,"
// 			// 			<< "判断一下candidateplan和b_initial_state是否有问题 " << std::endl;
// 			// printBDD(b_initial_state);
// 			// for (int i = 0; i < candidateplan.size(); i++)
// 			// {
// 			// 	candidateplan[i]->print(std::cout, my_problem->terms());
// 			// 	std::cout << "\n";
// 			// }
// 			// std::cout << "打印完毕，两个都没问题" << std::endl;
// 			b_initial_state = Cudd_bddOr(manager, counterexample, b_initial_state);
// 			std::cout << "当前的初始信念状态:" << std::endl;
// 			printBDD(b_initial_state);
// 			std::cout << "当前的初始信念状态打印完毕" << std::endl;
// 			// DdNode *b_counterexample = formula_bdd(*counterexample,false);
// 			// b_initial_state = Cudd_bddOr(manager, b_counterexample, b_initial_state);
// 			{  // planning
// 				// 初始化
// 				std::cout << "开始规划" << std::endl;
// 				search->init(num_alt_acts, b_initial_state, b_goal_state);
// 				cout << "starting search" << endl;
// 				std::cout << "call search()\n";
// 				search->search();  // 将规划结果传递给candidateplan
// 				std::cout << "本次规划结束" << std::endl;			
// 				if (allowed_time > 0)
// 				{
// 					// disable the sender
// 					timer.it_value.tv_sec = 0;
// 					timer.it_value.tv_usec = 0;
// 					setitimer(ITIMER_REAL, &timer, 0);
// 					// before the receiver
// 					signal(SIGALRM, SIG_DFL);
// 				}
// 				if (candidateplan.empty())
// 				{
// 					cout << "The Problem is Unsolvable Or Some other error" << endl;
// 					return 0;
// 				}				
// 			}
// 			// std::cout << "循环次数 = " << iteration << std::endl;
// 		}
// 		std::cout << "循环次数 = " << iteration << std::endl;
// 		/*从下面这一部分开始修改代码*/
// 		// 初始化动作个数，状态和目标状态
// 		// search->init(num_alt_acts, b_initial_state, b_goal_state);
// 		// cout << "starting search" << endl;
// 		// if (incremental_search && server_socket < 0) // Incremental AND offline?
// 		// {
// 		// 	std::cout << "call incremental_search()\n";
// 		// 	search->incremental_search();
// 		// }
// 		// else
// 		// {
// 		// 	std::cout << 'call search()\n';
// 		// 	search->search();
// 		// }
// 		// if (allowed_time > 0)
// 		// {
// 		// 	// disable the sender
// 		// 	timer.it_value.tv_sec = 0;
// 		// 	timer.it_value.tv_usec = 0;
// 		// 	setitimer(ITIMER_REAL, &timer, 0);
// 		// 	// before the receiver
// 		// 	signal(SIGALRM, SIG_DFL);
// 		// }
// 		// outputPlan();
// 		// if (search != NULL)
// 		// 	delete search;
// 	}
// 	catch (const exception &e)
// 	{
// 		cout << "caught something: " << e.what() << endl;
// 	}
// }

// 创建当前和后继状态变量的Cube，设置映射关系
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

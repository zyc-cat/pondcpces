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
#include "aostar.h"

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
void preprocessSNCWSC();

int main(int argc, char *argv[])
{

	try
	{
		// Search是全部搜索算法的抽象基类，StepSearch基于Searc的抽象基类
		Search *search = NULL;
		StepSearch *step_search = NULL;
		int seed = 1;
		bool incremental_search = false;

		if (argc < 3 || argv[1] == "--help")
		{
			cout << "POND Planner - Version 2.2\n"
				 << "Usage: pond <domain-name> <prob-name> [OPTIONS]" << endl
				 << "OPTIONS:" << endl
				 << "Search Algorithm:\n\t <default> \t\t : Enforced Hill Climbing\n\t -s ehc \t\t : Enforced Hill Climbing\n\t -s astar \t\t : A*\n\t -s aostar \t\t : AO* (ND Conditional)\n\t -s molao \t\t : Multi-Objective LAO* (Pr Conditional)\n\t -s replan \t\t : POND-replan\n\t -s hop \t\t : Hindsight Optimization\n\t -s schop \t\t : Sequential Correlated HOP\n\t -s pchop \t\t : Parallel Correlated HOP\n"
				 << "Heuristics:\n\t <default> \t\t : h = 0 <Breadth First Search>\n\t -h lugrp [-pg <sag_type>] [-pmg <# particles (McLUG)>]\n\t\t\t\t : (Mc)LUG relaxed plan" << endl
				 << "Particles: \n\t 1.0 <= pmg \t\t : #particles = pmg\n\t 0.0 < pmg < 1.0 \t : #particles = auto selected using pmg factor\n\t\t\t\t   (0.1 recommended)" << endl
				 << "SAG Types: \n\t <default> \t\t : One LUG per generated node\n\t 'node' \t\t : One LUG per generated node\n\t 'global' \t\t : One LUG per problem\n\t 'globalrp' \t\t : One relaxed plan per problem" << endl
				 << "Helpful Actions:\n\t <default> \t\t : off\n\t -ha \t\t\t : on " << endl
				 << "Verbosity:\n\t <default> \t\t : 1\n\t -v 0 \t\t\t : Minimal/No search information\n\t -v 1 \t\t\t : Print when goal(s) reached\n\t -v 2 \t\t\t : Print all nodes expanded" << endl;
			return 0;
		}

		if (argc > 3)
		{
			// Alan- Temporary backwards compatability
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
			} // end-for

			// Once we input some extra parameter, it will planner some special search.
			for (int i = 3; i < argc; i++)
			{
				// 根据传递的参数选择具体的搜索算法类，具体赋值给那个类指针根据继承的类决定
				if (strcmp(argv[i], "-s") == 0)
				{
					i++;
					// incluing several search algorithm to judge
					if (strcmp(argv[i], "astar") == 0)
					{
						cout << "A* Search" << endl;
						step_search = new AStar();
					}
					else if (strcmp(argv[i], "ehc") == 0)
					{
						cout << "Enforced Hill Climbing Search" << endl;
						search = new EHC();
					}
					else if (strcmp(argv[i], "aostar") == 0)
					{
						cout << "AO* Search" << endl;
						// search = new LAOStar();
						search = new AOStar();
					}
					else if (strcmp(argv[i], "molao") == 0)
					{
						cout << "Multi-Objective LAO Star Search" << endl;
						search = new MOLAOStar();
						MOLAORand = false;
					}
					else if (strcmp(argv[i], "moastar") == 0)
					{
						cout << "Multi-Objective A Star Search" << endl;
						search = new MOAStar();
					}
					else if (strcmp(argv[i], "rtdp") == 0)
					{
						cout << "RTDP Search" << endl;
						search = new RTDP(50);
					}
					else if (strcmp(argv[i], "molaor") == 0)
					{
						cout << "Randomized Multi-Objective LAO Star Search" << endl;
						search = new MOLAOStar();
						MOLAORand = true;
					}
					else if (strcmp(argv[i], "replan") == 0)
					{
						cout << "Replan Search" << endl;

						bool determinized = false;

						while (i + 1 < argc && argv[i + 1][0] != '-')
						{ // Check for replan arguments
							i++;
							if (strcmp(argv[i], "det") == 0)
							{
								cout << "Determinized\n";
								determinized = true;
							}
						}
						search = new Replan(determinized);
					}

					// three hop search algorithm
					if (strcmp(argv[i], "hop") == 0)
					{
						cout << "\"HOP\" Search" << endl;
						use_gs = true;
						use_h = false;

						int numSamples = 1;
						while (i + 1 < argc && argv[i + 1][0] != '-')
						{ // Check for [SC]HOP arguments
							i++;
							if (strcmp(argv[i], "samples") == 0)
							{
								if (i + 1 < argc && argv[i + 1][0] != '-')
								{
									numSamples = atoi(argv[++i]);
									cout << "HOP's number of samples = " << numSamples << endl;
								}
							}
							else if (strcmp(argv[i], "!use_gs") == 0)
							{
								cout << "NOT using goalSatisfaction for quality\n";
								use_gs = false;
							}
							else if (strcmp(argv[i], "use_gs") == 0)
							{
								cout << "Using goalSatisfaction for quality\n";
								use_gs = true;
							}
							if (strcmp(argv[i], "!use_h") == 0)
							{
								cout << "NOT using heuristic for quality\n";
								use_h = false;
							}
							if (strcmp(argv[i], "use_h") == 0)
							{
								cout << "Using heuristic for quality\n";
								use_h = true;
							}
						}

						step_search = new HOP(numSamples);
					}
					else if (strcmp(argv[i], "pchop") == 0)
					{
						cout << "Parallel Correlated HOP Search" << endl;

						int maxDepth = INT_MAX;
						int numSamples = 10;

						while (i + 1 < argc && argv[i + 1][0] != '-')
						{ // Check for PCHOP arguments
							i++;
							if (strcmp(argv[i], "depth") == 0)
							{
								if (i + 1 < argc && argv[i + 1][0] != '-')
								{
									maxDepth = atoi(argv[++i]);
									cout << "P-CHOP sample's maximum horizon depth = " << maxDepth << endl;
								}
							}
							else if (strcmp(argv[i], "samples") == 0)
							{
								if (i + 1 < argc && argv[i + 1][0] != '-')
								{
									numSamples = atoi(argv[++i]);
									cout << "P-CHOP's number of samples = " << numSamples << endl;
								}
							}
						}

						search = new PCHOP(numSamples, maxDepth);
					}
					else if (strcmp(argv[i], "lao") == 0)
					{
						cout << "LAO* Search" << endl;
						step_search = new ALAOStar();
					}
					else if (strcmp(argv[i], "exph") == 0)
					{
						cout << "Expected-Heuristic Search" << endl;
						step_search = new ExpH();
					}
				}

				if (strcmp(argv[i], "-R") == 0)
				{
					seed = atoi(argv[++i]);
				}
				if (strcmp(argv[i], "-beam") == 0)
				{
					beam_search_size = atoi(argv[++i]);
					cout << "Beam search size = " << beam_search_size << endl;
				}
				if (strcmp(argv[i], "-w") == 0)
				{
					GWEIGHT = atoi(argv[++i]);
					cout << "HWEIGHT = " << GWEIGHT << endl;
				}
				if (strcmp(argv[i], "-c") == 0)
				{
					MAXCONTINGENCIES = atoi(argv[++i]);
					cout << "MAX CONTINGENCIES = " << MAXCONTINGENCIES << endl;
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
				if (strcmp(argv[i], "-eps") == 0)
				{
					modom_epsilon = atof(argv[++i]);
					modom_anc_epsilon = atof(argv[++i]);
					gEpsilon = atof(argv[++i]);
					cout << "MOLAO epsilon = "
						 << modom_epsilon << " "
						 << modom_anc_epsilon << " "
						 << gEpsilon << endl;
				}
				if (strcmp(argv[i], "-gpr") == 0)
				{
					goal_pr_in = atof(argv[++i]);
					cout << "Command line [heuristic] P(G) = " << goal_pr_in << endl;
				}
				if (strcmp(argv[i], "-gt") == 0)
				{
					search_goal_threshold = max(0.0, min(1.0, atof(argv[++i])));
					cout << "Command line [search] P(G) = " << search_goal_threshold << endl;
				}
				if (strcmp(argv[i], "-pst") == 0 || strcmp(argv[i], "-pspr") == 0)
				{ // Alan- "pspr" is a temporary backwards compatability
					plan_success_threshold = max(0.0, min(1.0, atof(argv[++i])));
					cout << "Command line P(PlanSuccess) = " << plan_success_threshold << endl;
				}
				if (strcmp(argv[i], "-i") == 0)
				{
					incremental_search = true;
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
					if (strcmp(argv[i], "slugrp") == 0)// sensor LUG relaxed planning graph
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
					if (strcmp(argv[i], "sglevel") == 0)//最小term费用
					{
						COMPUTE_LABELS = FALSE;
						HEURISTYPE = LUGLEVEL;
						cout << "NOT USING LABELS ON PG" << endl;
					}
					if (strcmp(argv[i], "sgsum") == 0)// 每个clasue的费用和
					{
						COMPUTE_LABELS = FALSE;
						HEURISTYPE = LUGSUM;
						cout << "NOT USING LABELS ON PG" << endl;
					}
					if (strcmp(argv[i], "sgmax") == 0)// 最大clause费用
					{
						COMPUTE_LABELS = FALSE;
						HEURISTYPE = LUGMAX;
						cout << "NOT USING LABELS ON PG" << endl;
					}
				}
				else if (strcmp(argv[i], "-inc") == 0)
				{
					cout << "Using Incremental Dominance Heuristic" << endl;
					DO_INCREMENTAL_HEURISTIC = TRUE;
					MAX_H_INCREMENT = atoi(argv[++i]);
				}
				else if (strcmp(argv[i], "-nlevoff") == 0)
				{
					ALLOW_LEVEL_OFF = FALSE;
					cout << "Don't ALLOW LEVEL OFF" << endl;
				}
				else if (strcmp(argv[i], "-pg") == 0)
				{
					i++;
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
						LOOKAHEAD_FOR_STATES = atoi(argv[++i]);
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
					i++;
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
					out_file = argv[++i];
				}
				else if (strcmp(argv[i], "-fo") == 0)
				{
					OBSERVABILITY = OBS_FULL;
					cout << "assuming full observability" << endl;
				}
				else if (strcmp(argv[i], "-cp") == 0)
				{
					i++;
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
					NUMBER_OF_MGS = atof(argv[++i]);
					PF_LUG = TRUE;
					ALLOW_LEVEL_OFF = FALSE;
					cout << "Using McLUG, w/ particle increment: " << NUMBER_OF_MGS << endl;
				}
				else if (strcmp(argv[i], "-llw") == 0)
				{
					LUG_LEVEL_WORLDS = atoi(argv[++i]);
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
				else if (strcmp(argv[i], "-hzn") == 0)
				{
					max_horizon = atoi(argv[++i]);
				}
				else if (strcmp(argv[i], "-mp") == 0)
				{
					MAX_PRES = atoi(argv[++i]);
				}
				else if (strcmp(argv[i], "-rbpf") == 0)
				{
					RBPF_PROGRESSION = TRUE;
					RBPF_SAMPLES = atoi(argv[++i]);
					rbpf_bits = (int)ceil(log2(RBPF_SAMPLES));
				}
				else if (strcmp(argv[i], "-grw") == 0)
				{
					total_goal_reward = atof(argv[++i]);
					total_goal_reward *= -1; // make it a cost
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
					respect_time = false;
					allowed_time = atoi(argv[++i]);
				}
				else if (strcmp(argv[i], "-noop") == 0)
				{
					first_act_noop = true;
				}
				else if (strcmp(argv[i], "-v") == 0)
				{
					verbosity = atoi(argv[++i]);
				}
			}
		}

		if (seed <= 1)
			seed = time(NULL);
		srand(seed);
		cout << "Random Seed = " << seed << endl;

		cout << "Verbosity: ";

		switch (verbosity)
		{
		default:
			cout << verbosity << " is not a valid option -- changing to ";
			verbosity = 0;
		case 0:
			cout << "Minimal/No search information";
			break;
		case 1:
			cout << "Print when goal(s) reached";
			break;
		case 2:
			cout << "Print all nodes expanded";
			break;
		}
		verbosity = 0;
		cout << " (" << verbosity << ")\n";

		// 开始计时
		gStartTime = clock();
		randomGen = new randomGenerator(100000); // use a million random nums

		//    double old = NUMBER_OF_MGS;

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

			/**
			 * momo007 2022.07.06
			 * this code may use in LUG construct
			 */
			     if(PF_LUG){
			       NUMBER_OF_MGS = 8192;   // set to a constant to generate this many, why?
			
			       for(int i = 1; i <= NUMBER_OF_MGS; i++){
			         if((2 << (i-1)) >= NUMBER_OF_MGS){
			           num_new_labels = i;
			           break;
			         }
			       }
			     }
			// 初始化规划问题，分配BDD操作
			solve_problem(*my_problem, 1.0, 0.0);
			/**
			 * momo 007 2022.05.26
			 * not used */
			// if (RBPF_PROGRESSION)
			// {
			// 	// int num = (int)ceil(log2(RBPF_SAMPLES));
			// 	std::cout << "RBPF_PROGRESSION\n";
			// 	DdNode **variables = new DdNode *[rbpf_bits];
			// 	rbpf_index_map = new int[(2 * num_alt_facts) + max_num_aux_vars + 2 * rbpf_bits];
			// 	for (int k = 0; k < rbpf_bits; k++)
			// 	{
			// 		variables[k] = Cudd_addIthVar(manager, (2 * num_alt_facts) + max_num_aux_vars + k);
			// 		Cudd_Ref(variables[k]);
			// 		rbpf_index_map[(2 * num_alt_facts) + max_num_aux_vars + k] = (2 * num_alt_facts) + max_num_aux_vars + k + rbpf_bits;
			// 		rbpf_index_map[(2 * num_alt_facts) + max_num_aux_vars + k + rbpf_bits] = (2 * num_alt_facts) + max_num_aux_vars + k;
			// 	}
			// 	for (int k = 0; k < (2 * num_alt_facts) + max_num_aux_vars; k++)
			// 	{
			// 		rbpf_index_map[k] = k;
			// 	}
			// 	RBPF_SAMPLE_CUBE = Cudd_addComputeCube(manager, variables, NULL, rbpf_bits);
			// 	Cudd_Ref(RBPF_SAMPLE_CUBE);
			// 	// printBDD(RBPF_SAMPLE_CUBE);
			// 	delete[] variables;
			// }

			// printBDD(b_initial_state);
			cout << "Grounding/Instantiation Time: " << ((float)(clock() - groundingStartTime) / CLOCKS_PER_SEC) << endl;
			cout << "==================================\n";
			
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
					search = new AOStar();
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

			if (HEURISTYPE == HRPSUM ||
				HEURISTYPE == HRPMAX ||
				HEURISTYPE == HRPUNION ||
				LUG_FOR == AHREACHABLE)
			{

				if (NUMBER_OF_MGS > 0.0 && NUMBER_OF_MGS < 1.0)// 按比例
					K_GRAPH_MAX = (int)ceil(get_sum(b_initial_state) * NUMBER_OF_MGS);
				else if (NUMBER_OF_MGS >= 1.0)// 按指定数目
					K_GRAPH_MAX = (int)NUMBER_OF_MGS;
				else// 全部状态
					K_GRAPH_MAX = (int)get_sum(b_initial_state);
				// 创建Graph
				k_graphs = new kGraphInfo *[K_GRAPH_MAX];
			}

			gnum_cond_effects = 0;
			gnum_cond_effects_pre = 0;
			gbit_operators = NULL;
			used_gbit_operators = NULL;
			gnum_bit_operators = 0;
			gnum_relevant_facts = 0;

			if (!USE_CARD_GRP)// config the gbit_goal_state for sgrp, sglevel, sgsum, sgmax
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
		/*}*/
		
		// 没有指定搜索算法，则默认强制爬山算法
		if (search == NULL)
		{
			std::cout << "using EHC() algorithm\n";
			search = new EHC();
		}
		if(progMode == DEFINABILITY){
			preprocessSNCWSC();
		}
		// preprocessCubeUnit();
		// 初始化动作个数，状态和目标状态
		search->init(num_alt_acts, b_initial_state, b_goal_state);
		cout << "SNC success\n";
		// return 0;
		cout << "starting search" << endl;
		if (incremental_search && server_socket < 0) // Incremental AND offline?
		{
			std::cout << "call incremental_search()\n";
			search->incremental_search();
		}
		else
		{
			std::cout << "call search()\n";
			search->search();
		}
			
		
		if (allowed_time > 0)
		{
			// disable the sender
			timer.it_value.tv_sec = 0;
			timer.it_value.tv_usec = 0;
			setitimer(ITIMER_REAL, &timer, 0);

			// before the receiver
			signal(SIGALRM, SIG_DFL);
		}
		/**
		 * momo007 2022.05.12 这里取消了垃圾回收应该是为了节约时间
		 */
		if (false)
		{
			/** clean up **/
			state_variables.clear();

			set<DdNode *> nodes_to_deref;

			for (map<const Action *, DdNode *>::const_iterator ai = action_transitions.begin(); ai != action_transitions.end(); ai++)
				nodes_to_deref.insert((*ai).second);
			action_transitions.clear();

			// for (map<const Action *, DdNode *>::const_iterator ai = action_rewards.begin(); ai != action_rewards.end(); ai++)
			// 	nodes_to_deref.insert((*ai).second);
			// 		action_rewards.clear();

			delete[] varmap;
			delete randomGen;

			for (vector<DdNode *>::const_iterator di = identity_bdds.begin(); di != identity_bdds.end(); di++)
				nodes_to_deref.insert(*di);
			identity_bdds.clear();

			for (set<DdNode *>::iterator it = nodes_to_deref.begin(); it != nodes_to_deref.end(); it++)
			{
				cout << "it = " << *it << endl;
				Cudd_RecursiveDeref(manager, *it);
			}

			Cudd_RecursiveDeref(manager, identity_bdd);
		}
		outputPlan();

		if (search != NULL)
			delete search;
	}
	catch (const exception &e)
	{
		cout << "caught something: " << e.what() << endl;
	}
}
/**
 * pre compute the SNC and WSC for actions
 */
void preprocessSNCWSC(){
	preprocessCubeUnit();
	// for (ActionList::const_iterator ai = my_problem->actions().begin();
	// 			ai != my_problem->actions().end(); ai++)
	// {
	// 	std::cout << (*ai)->id() << std::endl;
	// 	int ret = definability_extract(*ai);
	// 	if ( ret == 1) // (p and SNC) or (!p and !WSC) = T
	// 	{
	// 		if(act_ndefp.find(*ai) == act_ndefp.end())// def-case
	// 		{
	// 			g1ndefTeqMT[0]++;
	// 		}
	// 		else // n-def case
	// 		{
	// 			g1ndefTeqMT[act_ndefp[*ai].size()]++;
	// 		}
	// 	}
	// 	else if ( ret == -1) // zero T
	// 	{
	// 		zero_act++;
	// 	}
	// 	else
	// 	{
	// 		if(act_ndefp.find(*ai) == act_ndefp.end())
	// 		{
	// 			g1ndefTneqMT[0]++;
	// 		}
	// 		else
	// 		{
	// 			g1ndefTneqMT[act_ndefp[*ai].size()]++;
	// 		}
	// 	}
	// 	total_act++;
	// }
}
/**
 * 创建当前和后继状态变量的Cube，设置映射关系
 */
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

		// 使用了百分比状态抽取，这块的作用应该是求解reward
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

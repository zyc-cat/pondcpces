#include <sstream>
#include <string.h>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>

#include "globals.h"
//#include "writeDomain.h"
#include "exclusions.h"
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
//#include <mcheck.h>

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

#define OUTPUT_OVERHEAD 10 + 5 // the number of milliseconds it takes to receive (10) and process (5) the signal

itimerval timer; // allowed_time
// sigaction act; // end_early

bool respect_time = true;
int num_new_labels = 0;

int cnf_mode = 0;
int noHeader = 0;
int idInitial = -1;
int edgeInTh = 0;
int pathLength = 0;

void lug2txt(int levels, std::ostream *o);

// extern void DisplaySolution();

void end_early(int signal)
{
	//  outputPlan();
	exit(0);
}

////////////////LUG2TXT///////////////
bool persist = false; // write only persistent mutexes
bool do_acts = true;
bool do_props = true;
bool do_effs = true;
/////////////////////////////////////

extern int gNumStates; // number of states in lao star search

// //NAD
extern clock_t gStartTime;

// /* BM: Added by BM */
ofstream my_outfile;
char my_outfile_name[100];
char my_buffer[100000];
// /* BM: End */

// extern int yydebug;
/* The parse function. */
extern int yyparse();
#ifdef PPDDL_PARSER
/* File to parse. */
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
	{

		return false;
	}
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
	//  mtrace();

	try
	{

		gStartTime = clock();
		LUGTOTEXT = 1;
		ALLOW_LEVEL_OFF = TRUE;
		DO_INDUCED = NINDUCED;
		srand(time(NULL));
		randomGen = new randomGenerator(100000); // use a million random nums

		// yydebug = 1;

		if (argc < 3)
		{
			//      cout << "POND Planner - Version 1.0\n"<<
			//        "Usage: pond <domain-name> <prob-name> [-h <heuristic> [-ha]] [-levoff] [-nonsense] [-w <hweight>] \n"<<endl;
			//      cout << "Heuristics:\n\t <default> \t: h = 0 <BFS>\n\t 'card' \t: Cardinality\n\t 'lugmax' \t: LUG max\n\t 'lugsum' \t: LUG sum\n\t 'luglevel' \t: LUG level\n\t 'lugrp' \t: LUG relaxed plan"<<endl <<endl;

			//      cout << "Other:\n \t'-ha'\t\t: helpful actions -- only works with lugrp"<<endl;
			//      cout << "\t'-levoff'\t: compute planning graph to level off,\n\t\t\t  otherwise stop when goals supported"<<endl;
			//      cout << "\t'-nonsense'\t: ignore sensors, try to find conformant plan"<<endl;
			//      cout << "\t'-w <hweight>'\t: set h value weight -- default w = 5"<<endl;
			//      cout <<"\t'-nlug'\t\t: use node LUG, default: global LUG" << endl;
			//      cout <<"\t'-cwa'\t\t: use closed world assumption, \n\t\t\t  default: open world assumption" << endl;
			//      cout <<"\t'-rpcost <cost|coverage|ratio>'\t\t: effect selection technique for relaxed plans, default = ratio" << endl;
			//      cout <<"\t'-out <plan output file>" << endl;
			//      cout <<"\t'-time <override time>" << endl;

			cout << "POND Planner - Version 1.1.1\n"
				 << "Usage: pond <domain-name> <prob-name> [-w <hweight>] [-h <heuristic>]" << endl
				 << "Heuristics:\n\t <default> \t\t  : h = 0 <Breadth First AO*>\n\t 'card' \t\t  : Cardinality\n\t 'sgrp' \t\t  : SG Relaxed Plan\n\t 'mgrpm' \t\t  : MG max relaxed plan\n\t 'mgrps' \t\t  : MG sum relaxed plan\n\t 'mgrpu' \t\t  : MG unioned relaxed plan\n\t 'lugrp [-pg <sag_type>]' : LUG relaxed plan" << endl
				 << "SAG Types: \n\t <default> \t: One LUG per generated node\n\t 'children' \t: One LUG per expanded node\n\t 'global' \t: One LUG per problem" << endl;
			// cout << "Heuristics:\n\t <default> \t: h = 0 <BFS>\n\t 'card' \t: Cardinality\n\t 'lugmax' \t: LUG max\n\t 'lugsum' \t: LUG sum\n\t 'luglevel' \t: LUG level\n\t 'lugrp' \t: LUG relaxed plan"<<endl <<endl;

			// cout << "Other:\n \t'-ha'\t\t: helpful actions -- only works with lugrp"<<endl;
			// cout << "\t'-levoff'\t: compute planning graph to level off,\n\t\t\t  otherwise stop when goals supported"<<endl;
			// cout << "\t'-nonsense'\t: ignore sensors, try to find conformant plan"<<endl;
			// cout << "\t'-w <hweight>'\t: set h value weight -- default w = 5"<<endl;
			// cout <<"\t'-nlug'\t\t: use node LUG, default: global LUG" << endl;
			//   cout <<"\t'-cwa'\t\t: use closed world assumption, \n\t\t\t  default: open world assumption" << endl;
			// cout <<"\t'-rpcost <cost|coverage|ratio>'\t\t: effect selection technique for relaxed plans, default = ratio" << endl;
			// cout <<"\t'-out <plan output file>" << endl;
			//      cout <<"\t'-time <override time>" << endl;

			return 0;
		}

#ifdef PPDDL_PARSER
		if ((read_file(argv[1]) && (Problem::begin()) != (Problem::end())) || //(*(Problem::begin())).second) ||
			read_file(argv[2]))
		{
			//!(read_file(argv[1]) && read_file(argv[2]))){
			cout << "done Parsing" << endl;
		}
		else
		{
			cout << "Parse Error" << endl;
			exit(0);
		}

		//   read_file(argv[2]);
#else
		// get parse tree information
		//	cout << "Start Parsing" <<endl;
		current_analysis = parseInput(argc, (char **)argv);
		//	cout << "Done Parsing" <<endl;
// 	//Fill up altalt DSs from parse tree
//         //fillStructures(current_analysis);
#endif

		// gNumStates=40000;

		if (argc > 3)
		{
			for (int i = 3; i < argc; i++)
			{
				if (strcmp(argv[i], "-w") == 0)
				{
					int param;
					sscanf(argv[++i], "%d", &param);
					// if(param>0&&param<6)
					GWEIGHT = param;
					cout << "HWEIGHT = " << GWEIGHT << endl;
				}
				if (strcmp(argv[i], "-cwa") == 0)
				{
					CWA = TRUE;
					cout << "USING CWA" << endl;
				}
				if (strcmp(argv[i], "-persist") == 0)
				{
					persist = TRUE;
				}
				if (strcmp(argv[i], "-persist") == 0)
				{
					persist = TRUE;
				}
				if (strcmp(argv[i], "-no-acts") == 0)
				{
					do_acts = FALSE;
				}
				if (strcmp(argv[i], "-no-effs") == 0)
				{
					do_effs = FALSE;
				}
				if (strcmp(argv[i], "-no-props") == 0)
				{
					do_props = FALSE;
				}
				if (strcmp(argv[i], "-induced") == 0)
				{
					DO_INDUCED = INDUCED;
					cout << "USING INDUCED MUTEXES" << endl;
				}
				if (strcmp(argv[i], "-enum") == 0)
				{
					ENUMERATE_SPACE = TRUE;
					cout << "ENUMERATING SEARCH TREE" << endl;
				}
				if (strcmp(argv[i], "-cnf_out") == 0)
				{
					sscanf(argv[++i], "%d", &cnf_mode);
					sscanf(argv[++i], "%d", &noHeader);
					sscanf(argv[++i], "%d", &idInitial);
					sscanf(argv[++i], "%d", &edgeInTh);
					sscanf(argv[++i], "%d", &pathLength);
				}

				if (strcmp(argv[i], "-h") == 0)
				{
					++i;
					if (strcmp(argv[i], "corrrp") == 0)
					{
						//	   cout << "Heuristic Search" <<endl;
						cout << "USING Correlation RP" << endl;
						HEURISTYPE = CORRRP;
						COMPUTE_LABELS = FALSE;
						USE_CORRELATION = TRUE;
					}
					if (strcmp(argv[i], "prrp") == 0)
					{
						//	   cout << "Heuristic Search" <<endl;
						cout << "USING Correlation RP" << endl;
						HEURISTYPE = CORRRP;
						COMPUTE_LABELS = FALSE;
						USE_CORRELATION = FALSE;
					}
					if (strcmp(argv[i], "slugrp") == 0)
					{
						//	   cout << "Heuristic Search" <<endl;
						cout << "USING SLUGRP" << endl;
						HEURISTYPE = SLUGRP;
					}
					if (strcmp(argv[i], "minslugrp") == 0)
					{
						//	   cout << "Heuristic Search" <<endl;
						cout << "USING MINSLUGRP" << endl;
						HEURISTYPE = SLUGRP;
						TAKEMINRP = TRUE;
					}
					if (strcmp(argv[i], "lugrp") == 0)
					{
						//	   cout << "Heuristic Search" <<endl;
						cout << "USING LUGRP" << endl;
						HEURISTYPE = LUGRP;
					}
					if (strcmp(argv[i], "luglevel") == 0)
					{
						//	   cout << "Heuristic Search" <<endl;
						cout << "USING LUGLEVEL" << endl;
						HEURISTYPE = LUGLEVEL;
					}
					if (strcmp(argv[i], "lugsum") == 0)
					{
						//	   cout << "Heuristic Search" <<endl;
						cout << "USING LUGSUM" << endl;
						HEURISTYPE = LUGSUM;
					}
					if (strcmp(argv[i], "lugmax") == 0)
					{
						//	   cout << "Heuristic Search" <<endl;
						cout << "USING LUGMAX" << endl;
						HEURISTYPE = LUGMAX;
					}
					if (strcmp(argv[i], "card") == 0)
					{
						//	   cout << "Heuristic Search" <<endl;
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
					// if(param>0&&param<6)
					MAX_H_INCREMENT = param;
				}
				else if (strcmp(argv[i], "-nlevoff") == 0)
				{
					ALLOW_LEVEL_OFF = FALSE;
					cout << "Don't ALLOW LEVEL OFF" << endl;
				}
				// 	    else if(strcmp(argv[i],"-l")==0) {
				// 	      ++i;

				// 	      if(strcmp(argv[i],"node")==0 ||
				// 		 COMPUTE_LABELS == FALSE){
				// 		LUG_FOR=NODE;
				// 		cout << "BUILDING LUG FOR EACH INDIVIDUAL NODE" <<endl;
				// 	      }
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
					MUTEX_SCHEME = MS_CROSS;
					WORLD_CHECK = WC_SAME;
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
		// ALLOW_LEVEL_OFF=FALSE;
		HEURISTYPE = LUGRP;
		try
		{
			// cout << "Start Grounding"<<endl;
			clock_t groundingStartTime = clock();

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

			solve_problem(*my_problem, 1.0, 0.0);

			cout << "Grounding/Instantiation Time: " << ((float)(clock() - groundingStartTime) / CLOCKS_PER_SEC) << endl;
			printBDD(b_initial_state);
			printBDD(b_goal_state);
			// NUMBER_OF_MGS = mclugSampleSize(b_initial_state, 0.1, 0.01);
			// cout << "RESET PARTICLES TO = " << NUMBER_OF_MGS << endl;
			if ((*my_problem).goal_cnf())
			{
				cout << "goal is cnf" << endl;

				bdd_goal_cnf(&goal_cnf);
				// 	    for(list<DdNode*>::iterator g = goal_cnf.begin();
				// 		g != goal_cnf.end(); g++){
				// 	      printBDD(*g);
				// 	    }
			}

			if (my_problem->domain().requirements.non_deterministic)
			{
				goal_threshold = 1.0;
			}
			else if (my_problem->domain().requirements.probabilistic)
			{
				goal_threshold = (*my_problem).tau();
			}

			gDiscount = (*my_problem).discount();
			//	  cout << "DISCOUNT = " << gDiscount << endl;

			max_horizon = (*my_problem).horizon();
			//	  cout << "GOAL THRESHOLD = " << goal_threshold << endl;
			// 	  if(max_horizon > -1)
			// 	    cout << "horizon = " << max_horizon <<endl;
			// 	  else
			// 	    cout << "horizon = Indefinite" <<endl;

			OPTIMIZE_REWARDS = 1;	  //= my_problem->domain().functions().find_function("reward").second;
			OPTIMIZE_PROBABILITY = 1; // my_problem->domain().functions().find_function("goal-probability").second;

			// 	  cout << "Optimizing: ";
			// 	  if(OPTIMIZE_REWARDS)
			// 	    cout << "Rewards & ";
			// 	  if(OPTIMIZE_PROBABILITY)
			// 	    cout << "Probability";
			// 	  cout << endl;

			num_alt_acts = action_preconds.size();
			// 	  alt_act_costs = new double[num_alt_acts];
			// 	    for(std::map<const Action*, DdNode*>::iterator i = action_preconds.begin();
			// 		i != action_preconds.end(); i++){
			// 	      alt_act_costs[(*i).first->id()-1] = 1.0;
			// 	    }

			dname = (char *)(new string(my_problem->domain().name()))->c_str();
			pname = (char *)(new string(my_problem->name()))->c_str();
			// if (my_problem->domain().requirements.rewards)
			// {
			// 	if (my_problem->goal_reward())
			// 		total_goal_reward = -1 * my_problem->goal_reward()->expression().value(my_problem->init_values()).double_value();
			// 	else
			// 		total_goal_reward = 0;

			// 	cout << "GOAL REWARD = " << total_goal_reward << endl;

			// 	DdNode *fr = Cudd_BddToAdd(manager, b_goal_state), *fr1 = Cudd_addConst(manager, total_goal_reward);
			// 	Cudd_Ref(fr);
			// 	Cudd_Ref(fr1);

			// 	goal_reward = Cudd_addApply(manager, Cudd_addTimes, fr1, fr);
			// 	Cudd_Ref(goal_reward);
			// 	Cudd_RecursiveDeref(manager, fr);
			// 	Cudd_RecursiveDeref(manager, fr1);

			// 	if (verbosity >= 3)
			// 	{
			// 		cout << "goal reward" << endl;
			// 		printBDD(goal_reward);
			// 	}
			// }
			// else
			// {
			// 	total_goal_reward = 0.0;
			// }
			total_goal_reward = 0.0;
		}
		catch (const Exception &e)
		{
			std::cerr << "main: " << e << std::endl;
			exit(0);
		}

		//  	 	Cudd_AutodynEnable(manager,
		// // 	 			   //CUDD_REORDER_SAME
		// //  			   //CUDD_REORDER_LINEAR
		// //  			   CUDD_REORDER_SIFT
		// 				   CUDD_REORDER_WINDOW2
		// //  			   //CUDD_REORDER_WINDOW3
		// //  			   //CUDD_REORDER_ANNEALING
		// //  			   //CUDD_REORDER_GROUP_SIFT
		//   			   );

		cout << "#ACTIONS = " << num_alt_acts << endl;
		cout << "#effects = " << num_alt_effs << endl;
		cout << "#EVENTS = " << event_preconds.size() << endl;
		cout << "#PROPOSITIONS = " << num_alt_facts << endl;

		goal_samples = Cudd_ReadLogicZero(manager);
		//	if(PF_LUG){//generate samples
		//	  //	  cout << "# particles = " << NUMBER_OF_MGS << endl;
		//
		//// 	  if(goal_threshold + delta >= 1.0)
		//// 	    delta = 0.999 - goal_threshold;
		//
		//	  if(0){
		//	    pair<int, int>* sample_plan = guess_num_samples(alpha, beta, goal_threshold, delta);
		//	    total_sample_size = sample_plan->first; //n
		//	    sample_threshold = sample_plan->second; //c
		//	    delete sample_plan;
		//	    expected_sample_size = (sample_threshold < 1 ? 1 : sample_threshold);
		//	  }
		//
		//
		//	  // expected_samples(total_sample_size,
		//	  // 						  sample_threshold,
		//	  // 						  beta, alpha,
		//	  // 						  1, goal_threshold, delta);
		//
		//	  std::list<int> new_vars;
		//	  generate_n_labels(num_new_labels, &all_samples, &new_vars);
		//
		//
		//
		//	  NUMBER_OF_MGS=old; //reset to actual
		//// 	  for(int i = 0; i < NUMBER_OF_MGS; i++){
		//// 	    samples.push_back(all_samples.front());
		//// 	    all_samples.remove(all_samples.front());NUMBER_OF_MGS
		//// 	  }
		//	}

#else

		//	cout << "Finished Processing Switches" << flush << endl;

		manager = Cudd_Init(2 * num_alt_facts,
							0,
							CUDD_UNIQUE_SLOTS,
							CUDD_CACHE_SLOTS,
							0);

		//	cout << "Initialized DD manager with numfacts: " <<
		//		num_alt_facts << flush <<  endl;

#ifdef PPDDL_PARSER
		Cudd_SetBackground(manager, Cudd_ReadZero(manager));
#else
		Cudd_SetBackground(manager, Cudd_ReadLogicZero(manager));
#endif

		// 	  Cudd_AutodynEnable(manager,
		//   			     //CUDD_REORDER_SAME
		//   // 			     			     CUDD_REORDER_LINEAR
		//   			        CUDD_REORDER_SIFT
		//   // 			     //    //CUDD_REORDER_WINDOW2
		//   // 			     //    //     CUDD_REORDER_WINDOW3
		//   // 			     //  //	   CUDD_REORDER_ANNEALING
		//    			     //	     CUDD_REORDER_GROUP_SIFT
		//    			     );

		// 	    if(STATE_TYPE != CLAUSAL_BELIEF){
		// cout << "Getting init"<<endl;

		assert(current_analysis);
		assert(current_analysis->the_problem);
		assert(current_analysis->the_problem->initial_state);

		//	pred_decl_list* statics = removeStaticPreds(current_analysis);

		b_initial_state = groundAnd(0, current_analysis->the_problem->initial_state); // getInitialStateDD(current_analysis->the_problem->initial_state);//goalToBDD(initial_state->getClauses(), TRUE);
		Cudd_Ref(b_initial_state);
		initFacts = num_alt_facts;
		if (CWA)
			init DONE = TRUE;

		// printBDD(b_initial_state);

		//	goal_state = new clausalState(current_analysis->the_problem->the_goal);
		// goal_state = 0;
		goal *g;
		b_goal_state = goalToBDD(g = goal_grounding_helper(NULL, current_analysis->the_problem->the_goal));
		delete g;
		Cudd_Ref(b_goal_state);

		//	printBDD(b_goal_state);

		//  cout << BDDToDNFString(b_initial_state)<<endl;

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
		//	atexit(outputPlan);

		if (current_analysis->the_domain && current_analysis->the_domain->ops)
		{
			//	cout << "About to build all acts, num_alt_facts = " << num_alt_facts <<endl;

			build_all_actions(current_analysis->the_domain->ops, current_analysis->the_problem->objects, b_initial_state);
		}
		else
		{
			cout << "Problem with ops" << endl;
			exit(0);
		}
		//        printBDD(b_initial_state);

		// 	initLUG(available_acts, b_goal_state);
		//  		  cout << "About to Prune Stuff"<<endl;
		//  	pruneStuff();

		// 	Cudd_ReduceHeap(manager,
		// 			CUDD_REORDER_SYMM_SIFT_CONV,
		// 			num_alt_facts);

		alt_act_costs = new double[num_alt_acts];
		int tmp = 0;
		for (action_list a = available_acts; a; a = a->next)
		{
			a->act->index = tmp++;
			alt_act_costs[a->act->index] = a->act->cost;
			//  cout << a->act->get_name() << endl;
		}

		//	          printBDD(b_initial_state);
		// cout << "#OACTIONS = " << obs_actions<< endl;
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

				//	      printBDD(b_initial_state);
				//	      cout <<"HI " << K_GRAPH_MAX << " " << NUMBER_OF_MGS <<endl;
				k_graphs = new kGraphInfo *[K_GRAPH_MAX];
			}
			gnum_cond_effects = 0;
			gnum_cond_effects_pre = 0;
			gbit_operators = NULL;
			used_gbit_operators = NULL;
			gnum_bit_operators = 0;
			gnum_relevant_facts = 0;
#ifdef PPDDL_PARSER
			if (!USE_CARD_GRP)
				initLUG(&action_preconds, b_goal_state);
#else
			initLUG(available_acts, b_goal_state);
#endif
			//  	    cout << "inited lug"<<endl;
		}

#ifdef PPDDL_PARSER
#else
// 	int i,j;

// 	if (!current_state_vars)
// 	{
// //		cout << "Initializing Current State Vars" << endl;
// 		current_state_vars = new (DdNode*)[num_alt_facts];
// 		for (i=0,j=0; i<num_alt_facts; i++,j++)
// 		{
// //			cout << "Var " << i << endl;
// 			current_state_vars[j] = Cudd_bddIthVar(manager,i);
// 			Cudd_Ref(current_state_vars[i]);
// 		}

// //		cout << "Initializing Next State Vars" << endl;
// 		next_state_vars = new (DdNode*)[num_alt_facts];
// 		for (i=num_alt_facts,j=0; i<2*num_alt_facts; i++,j++)
// 		{
// //			cout << "Size " << Cudd_ReadSize(manager) << " Var " << i <<
// //				" Past Var " << i - num_alt_facts << endl;
// 			next_state_vars[j] = Cudd_bddIthVar(manager,i);
// 			Cudd_Ref(next_state_vars[j]);
// 			Cudd_bddSetPairIndex(manager,i-num_alt_facts,i);
// 		}

// // 		cout << "Permuting Variables" << endl;

// 		int *perm = new int[num_alt_facts*2];
// 		for (i=0;i<num_alt_facts;i++)
// 		{
// 			perm[2*i] = i;
// 			perm[2*i+1] = Cudd_bddReadPairIndex(manager,i);
// 		}

// 		Cudd_ShuffleHeap(manager,perm);

// 		delete perm;

// //		cout << "Initializing Current State Cube" << flush<< endl;

// 		current_state_cube = Cudd_bddComputeCube(manager,current_state_vars,0,num_alt_facts);
// 		Cudd_Ref(current_state_cube);

// //		cout << "Initializing Next State Cube" << endl;
// 		next_state_cube = Cudd_bddComputeCube(manager,next_state_vars,0,num_alt_facts);
// 		Cudd_Ref(next_state_cube);

// //		cout << "Initializing Current State and Next State Variable Mapping" << endl;
// 		Cudd_SetVarMap(manager,current_state_vars,next_state_vars,num_alt_facts);

// //		cout << "$" ;
// //		cout.flush();
// 	}

// 	action_list new_acts = NULL;
// 	action_list prev = NULL;
// 	//turn actions into schemas
// 	DdNode *fr;
// 	for(action_list a = available_acts; a ; ){
// 	  //cout << "union " << a->act->get_name() << endl;
// 	  available_acts = available_acts->next;
// 	  a->act->set_image();
// 	  a->next = new_acts;
// 	  new_acts = a;
// 	  prev = NULL;
// 	  for(action_list a1 = available_acts; a1 ; ){
// 	    if(a1->act->schema_index() == a->act->schema_index()){
// 	     //cout << "with " <<  a1->act->get_name()<<endl;
// 	      //union actions
// 	      a->act->unionImage(a1->act->get_image());
// 	      a->act->get_effs()->b_pre = Cudd_bddOr(manager,
// 						a1->act->get_effs()->b_pre,
// 						fr=a->act->get_effs()->b_pre);
// 	      Cudd_Ref(a->act->get_effs()->b_pre);
// 	      Cudd_RecursiveDeref(manager, fr);
// 	      //fix lists
// 	      if(prev){
// 		prev->next = a1->next;
// 		//delete a1;
// 		delete a1->act;
// 		delete a1;
// 		a1 = prev->next;
// 		//	cout << "A"<<endl;
// 	      }
// 	      else{
// 		prev = a1;
// 		a1 = a1->next;
// 		available_acts=a1;
// 		//delete prev;
// 		delete prev->act;
// 		delete prev;
// 		prev = NULL;
// 		//	cout << "B"<<endl;
// 	      }
// 	    }
// 	    else{
// 	      // cout <<"Not " << a1->act->get_name()<<endl;
// 	      prev = a1;
// 	      a1= a1->next;
// 	      //	cout << "C"<<endl;
// 	    }

// 	  }
// 	  a = available_acts;
// 	}
// 	available_acts = new_acts;
#endif
		/** 1. Construct the Relaxed Plannning Graphy
		 */
		createInitLayer(b_initial_state);
		int j;
		int reached = build_graph(&j,
								  num_alt_facts,
								  ALLOW_LEVEL_OFF,
								  MUTEX_SCHEME);
		cout << "#levels = " << j << endl;
		lug2txt(j, &(std::cout));
		// free_my_info(j);

		ofstream mapping("mapping.out");

		mapping << "#timesteps = " << j << endl;
		mapping << "#propositions = " << num_alt_facts << endl;
		for (std::map<int, const Atom *>::const_iterator vi =
				 dynamic_atoms.begin();
			 vi != dynamic_atoms.end(); vi++)
		{
			mapping << (*vi).first << " " << flush;
			(*vi).second->print(mapping, my_problem->domain().predicates(),
								my_problem->domain().functions(), my_problem->terms());
			mapping << std::endl;
		}

		mapping << "#actions = " << num_alt_acts << endl;
		mapping << "#effects = " << num_alt_effs << endl;
		for (ActionList::const_iterator ai = my_problem->actions().begin();
			 ai != my_problem->actions().end(); ai++)
		{

			if (action_outcomes.find(*ai) == action_outcomes.end() ||
				action_outcomes[*ai]->transitions.size() == 0 ||
				action_outcomes[*ai]->transitions[0].size() == 0)
				continue;

			mapping << ((*ai)->id() - 1) << " " << flush;
			(*ai)->print(mapping, (*my_problem).terms());
			mapping << endl;
			OutcomeSet *outcomes = action_outcomes[(*ai)];

			// assumes there is a single outcome
			for (TransitionSetList::const_iterator ti =
					 outcomes->transitions[0].begin();
				 ti != outcomes->transitions[0].end(); ti++)
			{
				DdNode *effbdd = Cudd_bddPermute(manager, (*ti)->effect_bdd(), varmap);
				Cudd_Ref(effbdd);
				if (effbdd == Cudd_ReadLogicZero(manager))
				{
					mapping << "INCONSISTENT" << endl;
					continue;
				}

				mapping << (*ti)->index() << " (when" << flush;

				// PRINT PRECONDS
				list<DdNode *> c_cubes;
				list<double> c_values;
				c_cubes.clear();
				c_values.clear();
				get_cubes((*ti)->condition_bdd(), &c_cubes, &c_values);
				if (c_cubes.size() > 1)
					mapping << " (or" << flush;
				for (list<DdNode *>::iterator j = c_cubes.begin();
					 j != c_cubes.end(); j++)
				{
					mapping << " (and" << flush;
					if (*j != Cudd_ReadOne(manager))
					{
						for (int p = 0; p < num_alt_facts; p++)
						{
							if (Cudd_bddIsVarEssential(manager, *j, p * 2, 1))
							{
								mapping << " " << flush;
								dynamic_atoms[p]->print(mapping, my_problem->domain().predicates(),
														my_problem->domain().functions(), my_problem->terms());
							}
							else if (Cudd_bddIsVarEssential(manager, *j, p * 2, 0))
							{
								mapping << " (not " << flush;
								dynamic_atoms[p]->print(mapping, my_problem->domain().predicates(),
														my_problem->domain().functions(), my_problem->terms());
								mapping << ")" << flush;
							}
						}
					}
					mapping << ")" << flush;
				}
				if (c_cubes.size() > 1)
					mapping << ")" << flush;

				// PRINT EFFS (should be conjunctive)

				if (effbdd != Cudd_ReadOne(manager))
				{
					mapping << " (and" << flush;
					for (int i = 0; (i < num_alt_facts); i++)
					{
						if (Cudd_bddIsVarEssential(manager, effbdd, 2 * i, 1))
						{
							mapping << " " << flush;
							dynamic_atoms[i]->print(mapping, my_problem->domain().predicates(),
													my_problem->domain().functions(), my_problem->terms());
						}
						else if (Cudd_bddIsVarEssential(manager, effbdd, 2 * i, 0))
						{
							mapping << " (not " << flush;
							dynamic_atoms[i]->print(mapping, my_problem->domain().predicates(),
													my_problem->domain().functions(), my_problem->terms());
							mapping << ")" << flush;
						}
					}
					mapping << ")" << flush; // end eff
				}
				Cudd_RecursiveDeref(manager, effbdd);

				mapping << ")" << endl; // end condeff
			}
			mapping << endl; // end act
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

#ifdef PPDDL_PARSER
		// IPC_write();
		/*
		 * Clean up.
		 */
		state_variables.clear();
		// Cudd_RecursiveDeref(manager, col_cube);
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

		//    DisplaySolution();
		//  outputPlan();

#else
		outputPlan();
#endif

		// return 0;
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
				// printBDD(particle_vars[i]);
			}
			particle_cube = Cudd_addComputeCube(manager, particle_vars, 0, num_new_labels);
			Cudd_Ref(particle_cube);
			//      cout << "Particle cube="<<endl;
			//      printBDD(particle_cube);
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

	fout //<< "==========================" << endl
		<< "0"
		<< "\n%%\n"
		<< action_length << action_string.str()
		<< "\n%%\n"
		<< "linear " << plan_length << plan_string.str() << endl
		//<< "=========================="
		<< endl;

	fout.close();
}

int p, a, e, prop_lev_size, act_lev_size, eff_lev_size, lev_size, size;

// int pos_offset(int i) { return i*(i-1)/2; }
// int neg_offset(int i) { return (i+p)*(i+p-1)/2; }

DdNode *computeLabelPair(DdNode *la, DdNode *lb, DdNode *mutex)
{
	DdNode *label;

	if (1 || MUTEX_SCHEME != MS_CROSS)
	{
		DdNode *sameWorlds = Cudd_bddAnd(manager, la, lb);
		Cudd_Ref(sameWorlds);

		DdNode *pmutex = Cudd_bddVarMap(manager, mutex);
		Cudd_Ref(pmutex);

		DdNode *smutex = Cudd_bddAnd(manager, mutex, pmutex);
		Cudd_Ref(smutex);

		DdNode *amutex = Cudd_bddExistAbstract(manager, smutex, next_state_cube);
		Cudd_Ref(amutex);

		//    printBDD(mutex);
		//     printBDD(pmutex);
		//     printBDD(smutex);
		//     printBDD(amutex);

		label = Cudd_bddAnd(manager, sameWorlds, Cudd_Not(amutex));
		Cudd_Ref(label);
	}
	else if (MUTEX_SCHEME == MS_CROSS)
	{
		cout << "Implement computeLabelPair for cross mutex" << endl;
		exit(0);
	}

	return label;
}

int propDDIndex(int i, bool ipol, int j, int jpol, int level)
{
	int offset = (level)*lev_size + i;

	if (ipol && jpol)
	{
		offset += (j * (j + 1) / 2);
	}
	else if (!ipol && jpol)
	{
		cout << "BUG" << endl;
		exit(0);
	}
	else if (ipol && !jpol)
	{
		offset += ((j + p) * (j + p + 1) / 2);
	}
	else if (!ipol && !jpol)
	{
		offset += p + ((j + p) * (j + p + 1) / 2);
	}

	if (offset > size)
	{
		cout << "illegal offset " << offset << endl;
	}
	//  cout << "prop = " << offset << endl;
	return offset;
}

void setPropositions(int level, DdNode **dds)
{

	//  std::cout << "time = " << level << std::endl;

	for (int i = 0; i < num_alt_facts; i++)
	{
		for (int j = i; j < num_alt_facts; j++)
		{
			if (MUTEX_SCHEME == MS_NONE && j != i)
				continue;

			// std::cout <<  i << " " << j << std::endl;

			int index = propDDIndex(i, true, j, true, level);
			if (gft_table[i] && gft_table[j] &&
				gft_table[i]->info_at[level] && gft_table[j]->info_at[level])
			{

				dds[index] = computeLabelPair(gft_table[i]->info_at[level]->label->label,
											  gft_table[j]->info_at[level]->label->label,
											  ARE_MUTEX_FTS(level, gft_table[i], gft_table[j],
															gft_table[i]->info_at[level]->label->label,
															gft_table[j]->info_at[level]->label->label));
				// 	std::cout << "+" << i << " +" << j << std::endl;
				// 	printBDD(Cudd_bddAnd(manager,
				// 			     gft_table[i]->info_at[level]->label->label,
				// 			     gft_table[j]->info_at[level]->label->label));
				// 	printBDD(dds[index]);
			}
			else
				dds[index] = Cudd_ReadLogicZero(manager);

			index = propDDIndex(i, false, j, false, level);
			if (gft_table[NEG_ADR(i)] && gft_table[NEG_ADR(j)] &&
				gft_table[NEG_ADR(i)]->info_at[level] && gft_table[NEG_ADR(j)]->info_at[level])
			{
				dds[index] = computeLabelPair(gft_table[NEG_ADR(i)]->info_at[level]->label->label,
											  gft_table[NEG_ADR(j)]->info_at[level]->label->label,
											  ARE_MUTEX_FTS(level, gft_table[NEG_ADR(i)], gft_table[NEG_ADR(j)],
															gft_table[NEG_ADR(i)]->info_at[level]->label->label,
															gft_table[NEG_ADR(j)]->info_at[level]->label->label));

				// 	std::cout << "-" << i << " -" << j << std::endl;
				// 	printBDD(Cudd_bddAnd(manager,
				// 			     gft_table[NEG_ADR(i)]->info_at[level]->label->label,
				// 			     gft_table[NEG_ADR(j)]->info_at[level]->label->label));
				// 	printBDD(dds[index]);
			}
			else
				dds[index] = Cudd_ReadLogicZero(manager);

			index = propDDIndex(i, true, j, false, level);
			if (gft_table[i] && gft_table[NEG_ADR(j)] &&
				gft_table[i]->info_at[level] && gft_table[NEG_ADR(j)]->info_at[level])
			{
				dds[index] = computeLabelPair(gft_table[i]->info_at[level]->label->label,
											  gft_table[NEG_ADR(j)]->info_at[level]->label->label,
											  ARE_MUTEX_FTS(level, gft_table[i], gft_table[NEG_ADR(j)],
															gft_table[i]->info_at[level]->label->label,
															gft_table[NEG_ADR(j)]->info_at[level]->label->label));
				// 	std::cout << "+" << i << " -" << j << " " << index << std::endl;
				// 	printBDD(Cudd_bddAnd(manager,
				// 			     gft_table[i]->info_at[level]->label->label,
				// 			     gft_table[NEG_ADR(j)]->info_at[level]->label->label));
				// 	printBDD(dds[index]);
			}
			else
				dds[index] = Cudd_ReadLogicZero(manager);

			index = propDDIndex(j, true, i, false, level);
			if (gft_table[NEG_ADR(i)] && gft_table[j] &&
				gft_table[NEG_ADR(i)]->info_at[level] && gft_table[j]->info_at[level])
			{
				dds[index] = computeLabelPair(gft_table[NEG_ADR(i)]->info_at[level]->label->label,
											  gft_table[j]->info_at[level]->label->label,
											  ARE_MUTEX_FTS(level, gft_table[NEG_ADR(i)], gft_table[j],
															gft_table[NEG_ADR(i)]->info_at[level]->label->label,
															gft_table[j]->info_at[level]->label->label));
				// 	std::cout << "-" << i << " +" << j << " " << index << std::endl;
				// 	printBDD(Cudd_bddAnd(manager,
				// 			     gft_table[NEG_ADR(i)]->info_at[level]->label->label,
				// 			     gft_table[j]->info_at[level]->label->label));
				// 	printBDD(dds[index]);
			}
			else
				dds[index] = Cudd_ReadLogicZero(manager);
		}
	}
}

int actDDindex(int a1, int a2, int level)
{
	int offset = level * lev_size + prop_lev_size + a1 + (a2 * (a2 + 1) / 2);
	if (offset > size)
	{
		cout << "illegal offset " << offset << endl;
	}
	//  cout << "act = " << offset << endl;
	return offset;
}

void setActions(int level, DdNode **dds)
{
	OpNode *opa, *opb;
	for (OpNode *op = gall_ops_pointer; op; op = op->next)
	{
		if (op->is_noop)
			continue;
		for (OpNode *op1 = op; op1; op1 = op1->next)
		{
			if (op1->is_noop)
				continue;

			if (op->alt_index - 1 <= op1->alt_index - 1)
			{
				opa = op;
				opb = op1;
			}
			else
			{
				opa = op1;
				opb = op;
			}

			if (MUTEX_SCHEME == MS_NONE && opa != opb)
				continue;

			int index = actDDindex(opa->alt_index - 1, opb->alt_index - 1, level);

			if (opa->info_at[level] && opb->info_at[level])
			{
				dds[index] = computeLabelPair(opa->info_at[level]->label->label,
											  opb->info_at[level]->label->label,
											  ARE_MUTEX_OPS(level, opa, opb,
															opa->info_at[level]->label->label,
															opb->info_at[level]->label->label));
			}
			else
				dds[index] = Cudd_ReadLogicZero(manager);
		}
	}
}

int effDDindex(int ef1, int ef2, int level)
{
	int offset = level * lev_size + prop_lev_size + act_lev_size + ef1 + (ef2 * (ef2 + 1) / 2);
	if (offset > size)
	{
		cout << "illegal offset " << offset << endl;
	}
	//  cout << "eff = " << offset << endl;

	return offset;
}

void setEffects(int level, DdNode **dds)
{
	EfNode *efa, *efb;
	for (OpNode *op = gall_ops_pointer; op; op = op->next)
	{
		if (op->is_noop)
			continue;
		for (OpNode *op1 = op; op1; op1 = op1->next)
		{
			if (op1->is_noop)
				continue;

			if (MUTEX_SCHEME == MS_NONE && op != op1)
				continue;

			for (EfNode *ef = op->conditionals; ef; ef = ef->next)
			{
				for (EfNode *ef1 = (op == op1 ? ef : op1->conditionals); ef1; ef1 = ef1->next)
				{

					if (ef->alt_index <= ef1->alt_index)
					{
						efa = ef;
						efb = ef1;
					}
					else
					{
						efa = ef1;
						efb = ef;
					}

					if (MUTEX_SCHEME == MS_NONE && efa != efb)
						continue;

					int index = effDDindex(efa->alt_index, efb->alt_index, level);

					if (efa->info_at[level] && efb->info_at[level])
					{
						dds[index] = computeLabelPair(efa->info_at[level]->label->label,
													  efb->info_at[level]->label->label,
													  ARE_MUTEX_EFS(level, efa, efb,
																	efa->info_at[level]->label->label,
																	efb->info_at[level]->label->label));
					}
					else
						dds[index] = Cudd_ReadLogicZero(manager);
				}
			}
		}
	}
}

void lug2txt(int lev, std::ostream *o)
{

	/* BDD array format is as follows:
	   lev = lug level index
	   p = #propositions
	   a = #actions
	   e = #effects

	   prop_lev_size = (2p(2p+1))/2
	   act_lev_size = (a(a+1))/2
	   eff_lev_size = (e(e+1))/2

	   lev_size =  prop_lev_size + act_lev_size + eff_lev_size

	   size = (lev - 1) * lev_size + prop_lev_size

	   label of (+p_i, +p_j, lev), assuming i <= j
	   lev * lev_size + i + (j(j+1)/2)

	   label of (+p_i, -p_j, lev)
	   lev * lev_size + i + ((j+p)(j+p+1)/2)

	   label of (-p_i, -p_j, lev), assuming i <= j
	   lev * lev_size + i + p + ((j+p)(j+p+1)/2)

	   label of (a_i, a_j, lev), assuming i <= j =
		lev * lev_size  + prop_lev_size + i + (j(j+1)/2)

	   label of (e_i, e_j, lev), assuming i <= j =
		lev * lev_size  + prop_lev_size + act_lev_size + i + (j(j+1)/2)

	 */

	// int
	// Dddmp_cuddBddArrayStore(
	//   DdManager *       dd,              manager
	//   char *            ddname,          dd name (or NULL)
	//   int               nroots,          number of output BDD roots to be stored
	//   DdNode **         f,               array of BDD roots to be stored
	//   char **           rootnames,       array of root names (or NULL)
	//   char **           varnames,        array of variable names (or NULL)
	//   int *             auxids,          array of converted var IDs
	//   int               mode,            storing mode selector
	//   Dddmp_VarInfoType varinfo,         extra info for variables in text mode
	//   char *            fname,           file name
	//   FILE *            fp               pointer to the store file
	// )
	//   Dumps the argument array of BDDs to file. Dumping is either in text or
	//   binary form. BDDs are stored to the fp (already open) file if not NULL.
	//   Otherwise the file whose name is fname is opened in write mode. The header
	//   has the same format for both textual and binary dump. Names are allowed for
	//   input variables (vnames) and for represented functions (rnames). For sake of
	//   generality and because of dynamic variable ordering both variable IDs and
	//   permuted IDs are included. New IDs are also supported (auxids). Variables
	//   are identified with incremental numbers. according with their positiom in
	//   the support set. In text mode, an extra info may be added, chosen among the
	//   following options: name, ID, PermID, or an auxiliary id. Since conversion
	//   from DD pointers to integers is required, DD nodes are temporarily removed
	//   from the unique hash table. This allows the use of the next field to store
	//   node IDs.

	//   Side Effects: Nodes are temporarily removed from the unique hash table. They
	//   are re-linked after the store operation in a modified order.

	p = num_alt_facts;
	a = num_alt_acts;
	e = num_alt_effs;

	prop_lev_size = (2 * p * (2 * p + 1)) / 2;
	act_lev_size = (a * (a + 1)) / 2;
	eff_lev_size = (e * (e + 1)) / 2;
	lev_size = prop_lev_size + act_lev_size + eff_lev_size;
	size = (lev - 1) * lev_size + prop_lev_size;

	//   cout << "p = " << num_alt_facts << endl
	//        << "a = " << num_alt_acts << endl
	//        << "e = " << num_alt_effs << endl
	//        << "pls = " << prop_lev_size << endl
	//        << "als = " << act_lev_size << endl
	//        << "els = " << eff_lev_size << endl
	//        << "size = " << size << endl;

	char **varNames = new char *[2 * p];
	for (int i = 0; i < p; i++)
	{
		ostringstream po(ostringstream::out);
		dynamic_atoms[i]->print(po, my_problem->domain().predicates(),
								my_problem->domain().functions(),
								my_problem->terms());
		// cout << po.str() << endl;
		string *pos = new string(po.str());
		varNames[2 * i] = (char *)pos->c_str();
		po << "'";
		// cout << po.str() << endl;
		string *posp = new string(po.str());
		varNames[2 * i + 1] = (char *)posp->c_str();
	}

	DdNode **dds = new DdNode *[size];
	for (int i = 0; i < size; i++)
		dds[i] = Cudd_ReadLogicZero(manager); // NULL;

	for (int i = 0; i < lev - 1; i++)
	{
		if (!persist && do_props)
			setPropositions(i, dds);
		if ((!persist || i == lev - 2) &&
			do_acts)
			setActions(i, dds);
		if ((!persist || i == lev - 2) &&
			do_effs)
			setEffects(i, dds);
	}
	if (do_props)
		setPropositions(lev - 1, dds);

	int *auxids = new int[2 * p];
	for (int i = 0; i < 2 * p; i++)
		auxids[i] = i;

	Dddmp_VarInfoType info;

	FILE *outfile;

	// DD out
	if ((outfile = fopen("dd.out", "w")) == NULL)
		fprintf(stderr, "Cannot open %s\n", "dd.out");

	Dddmp_cuddBddArrayStore(manager, pname, size, dds, NULL, varNames, auxids, DDDMP_MODE_TEXT, DDDMP_VARIDS, (char *)"dd.out", outfile);
	fclose(outfile);

	// CNF out
	if ((outfile = fopen("cnf.out", "w")) == NULL)
		fprintf(stderr, "Cannot open %s\n", "cnf.out");
	int clauseNPtr, varNewNPtr;
	int *bddIds = new int[2 * p];
	int *cnfIds = new int[2 * p];

	for (int i = 0; i < 2 * p; i++)
	{
		bddIds[i] = i;
		cnfIds[i] = i + 1;
	}

	Dddmp_DecompCnfStoreType mode;
	if (cnf_mode == 0)
		mode = DDDMP_CNF_MODE_NODE;
	else if (cnf_mode == 1)
		mode = DDDMP_CNF_MODE_MAXTERM;
	else if (cnf_mode == 2)
		mode = DDDMP_CNF_MODE_BEST;

	// input: cnf_mode noHeader idInitial edgeInTh

	cout << "cnf_mode = " << cnf_mode << endl
		 << "noHeader = " << noHeader << endl
		 << "idInitial = " << idInitial << endl
		 << "edgeInTh = " << edgeInTh << endl
		 << "pathLength = " << edgeInTh << endl;

	Dddmp_cuddBddArrayStoreCnf(manager,		/* IN: DD Manager */
							   dds,			/* IN: array of BDD roots to be stored */
							   size,		/* IN: # output BDD roots to be stored */
							   mode,		/* IN: format selection */
							   noHeader,	/* IN: do not store header iff 1 */
							   varNames,	/* IN: array of variable names (or NULL) */
							   bddIds,		/* IN: array of converted var IDs */
							   auxids,		/* IN: array of BDD node Auxiliary Ids */
							   cnfIds,		// bddIds,     /* IN: array of converted var IDs */
							   idInitial,	/* IN: starting id for cutting variables */
							   edgeInTh,	/* IN: Max # Incoming Edges */
							   pathLength,	/* IN: Max Path Length */
							   "cnf.out",	/* IN: file name */
							   outfile,		/* IN: pointer to the store file */
							   &clauseNPtr, /* OUT: number of clause stored */
							   &varNewNPtr	/* OUT: number of new variable created */
	);

	fclose(outfile);

	delete[] varNames;

	delete[] bddIds;
	delete[] cnfIds;

	delete[] dds;
	delete[] auxids;
}

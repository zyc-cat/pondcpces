/* fpont 12/99 */
/* pont.net    */
/* tcpClient.c */

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h> /* close */
#include <stdlib.h>
#include <string.h>

// #include Lexer.h>
// extern yyFlexLexer* yfl;
#include "globals.h"
#ifdef PPDDL_PARSER

#include "allheaders.h"
extern void collectInit(const Problem*);
//const StateFormula* the_observation;
//plan* the_plan;
//* The parse function. */
extern int yyparse();
/* File to parse. */
extern FILE* yyin;
/* Name of current file. */
std::string current_file;
/* Parses the given file, and returns true on success. */
static bool read_file(const char* name) {
//    if(!yfl)
//      yfl= new yyFlexLexer;
  yyin = fopen(name, "r");
  if (yyin == NULL) {
 
    return false;
  } else {
    current_file = name;
    bool success = (yyparse() == 0);
    fclose(yyin);
    return success;
  }
}
#else
#include "instantiation.h"
#include "actions.h"
#include "parser/parse.h"
#endif



#include "lao_wrapper.h"
#include "dd.h"
#include "lug.h"
#include "graph_wrapper.h"
#include "lao.h"

#include "monitor.h"



using namespace std;


int main (int argc, char *argv[]) {

  string opt=" ";

  if(argc < 3)	  {
    cout << "POND Monitor - Version 1.0\n"<<
      "Usage: mon <domain-name> <prob-name> [-host <host>] [-port <port>] [<Planner Options>] [-- <Planner Options>]"<<endl<<endl;

    //cout << "Compiled with PLANNER=" << PLANNER << endl;
    exit(1);
  }


  int i;
  for(i=3;i<argc;i++)
    {
      if (strcmp(argv[i],"-host")==0)
	{
	  HOST = argv[++i];
	}
      else if (strcmp(argv[i],"-port")==0)
	{
	  PORT = atoi(argv[++i]);
	}
      else if (strcmp(argv[i],"--")==0)
	break;

      else
	{
	  opt += argv[i];
	  opt += " ";
	}
    }

  // escape to planner options
  for(;i<argc;i++)
    {
      opt += argv[i];
      opt += " ";
    }  
  
  //get parse tree information
  //	cout << "Start Parsing" <<endl;

#ifdef PPDDL_PARSER
  try {	
    if(!read_file(argv[1]))
      exit(0);
    else
      cout << "done Parsing"<<endl;
    my_problem = (*(Problem::begin())).second;
    solve_problem(*my_problem, 1.0, 0.0);	 	 
    collectInit(my_problem);
   num_alt_acts = action_transitions.size();
   
    alt_act_costs = new double[num_alt_acts];	 
    for(std::map<const Action*, DdNode*>::iterator i = action_transitions.begin();
	i != action_transitions.end(); i++){
      
      alt_act_costs[(*i).first->id()-1] = 1.0;
 
    }
    dname = (char*)(new string(my_problem->domain().name()))->c_str();
    pname = (char*)(new string(my_problem->name()))->c_str();
  }  catch (const Exception& e) {
    std::cerr << "main: " << e << std::endl;
    exit(0);
  }
  
#else
  current_analysis = parseInput(argc, (char **)argv);
  allowed_time = current_analysis->the_problem->allowed_time;


  cout << "Given " << allowed_time << "ms to plan"<<endl;
  //	cout << "Done Parsing" <<endl;
  // 	//Fill up altalt DSs from parse tree
  //         //fillStructures(current_analysis);
  
  
  
  manager = Cudd_Init(num_alt_facts, 0, CUDD_UNIQUE_SLOTS, CUDD_CACHE_SLOTS, 0);
  
  //	cout << "Initialized DD manager with numfacts: " << 
  //		num_alt_facts << flush <<  endl;

  
  // 	  Cudd_SetBackground(manager, Cudd_ReadLogicZero(manager));
//   	  Cudd_AutodynEnable(manager,
//   			     CUDD_REORDER_SAME
//   // 			     			     CUDD_REORDER_LINEAR
//   			     //   CUDD_REORDER_SIFT
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
  //b_initial_state = getInitialStateDD(current_analysis->the_problem->initial_state);
  b_initial_state = groundAnd(0,current_analysis->the_problem->initial_state);

  Cudd_Ref(b_initial_state);
  initFacts = num_alt_facts;
  if(CWA)
    initDONE = TRUE;
  
  
  //printBDD(b_initial_state);
  
  goal_state = 0;
  b_goal_state = goalToBDD(current_analysis->the_problem->the_goal);
  Cudd_Ref(b_goal_state);
  //printBDD(b_goal_state);

  // 	    }
  
	  
  dname = current_analysis->the_problem->domain_name;
  pname = current_analysis->the_problem->name;
  if(current_analysis->the_domain && current_analysis->the_domain->ops){
    //	cout << "About to build all acts, num_alt_facts = " << num_alt_facts <<endl;
    
    build_all_actions(current_analysis->the_domain->ops, 
		      current_analysis->the_problem->objects, 
		      b_initial_state);
  }
  else {
    cout << "Problem with ops"<<endl;
    exit(0);
  }
  //       printBDD(b_initial_state);
#endif


  Monitor *mon=new Monitor(HOST,PORT,argv[1],argv[2],opt);
 

  mon->start();
  delete mon;
  return 0;
  
}

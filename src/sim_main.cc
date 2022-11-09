#include "globals.h"
#include <iostream>
#include <fstream>

#ifdef PPDDL_PARSER
#include "ppddl/allheaders.h"
#include <FlexLexer.h>
extern void collectInit(const Problem*);
//const StateFormula* the_observation;
//plan* the_plan;
extern yyFlexLexer* yfl;
#else
#include "instantiation.h"
#include "actions.h"
#include "parser/parse.h"
#endif

#include "lao/lao_wrapper.h"
#include "cudd/dd.h"
#include "heuristics/lug.h"
#include "lug/graph_wrapper.h"
//#include "lug/kGraphInfo.h"
#include "lao/lao.h"
#include "simulator/server.h"


extern int gNumStates; // number of states in lao star search

// /* BM: Added by BM */
 ofstream my_outfile;
 char my_outfile_name[100];
 char my_buffer[100000];
// /* BM: End */


//* The parse function. */
extern int yyparse();
/* File to parse. */
extern FILE* yyin;
/* Name of current file. */
std::string current_file;
/* Parses the given file, and returns true on success. */
static bool read_file(const char* name) {
 //  if(!yfl)
//     yfl= new yyFlexLexer;
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


int main(int argc,char * argv[])  {


   if(argc < 3)	  {
     cout << "POND Simulator - Version 1.0\n"<<
       "Usage: pond <domain-name> <prob-name> [-port <port>] [-fo]" << endl;
    
   }

	  for(int i=3;i<argc;i++) {
	  
	    if(strcmp(argv[i],"-port")==0){
	      i++;
	      PORT=atoi(argv[i]);
	      cout << "USING PORT = " << PORT <<endl;
	    }
	    else if(strcmp(argv[i],"-fo")==0) {
	      OBSERVABILITY=OBS_FULL;	
	      cout << "assuming full observability"<<endl;
	    }
	
	  }

 	//get parse tree information
   //	cout << "Start Parsing" <<endl;
#ifdef PPDDL_PARSER
	  try{
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
	    dname = (char*)my_problem->domain().name().c_str();
	    pname = (char*)my_problem->name().c_str();
	  }  catch (const Exception& e) {
	    std::cerr << "main: " << e << std::endl;
	    exit(0);
	  }

#else
	  current_analysis = parseInput(argc, (char **)argv);
	  assert(current_analysis);
	  assert(current_analysis->the_problem);
	  assert(current_analysis->the_domain);

//	cout << "Done Parsing" <<endl;
// 	//Fill up altalt DSs from parse tree
//         //fillStructures(current_analysis);
	





	  
	  manager = Cudd_Init(0, 0, CUDD_UNIQUE_SLOTS, CUDD_CACHE_SLOTS, 0);
	


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

	  b_initial_state = groundAnd(0,current_analysis->the_problem->initial_state); 

	  Cudd_Ref(b_initial_state);
	  initFacts = num_alt_facts;
	  if(CWA)
	    initDONE = TRUE;
	  
	  //printBDD(b_initial_state);

  	  //goal_state = new clausalState(current_analysis->the_problem->the_goal);
	  goal_state = 0;
	  b_goal_state = goalToBDD(current_analysis->the_problem->the_goal);
	  Cudd_Ref(b_goal_state);
          //printBDD(b_goal_state);


  // 	    }

 

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

	      //	          printBDD(b_initial_state);	
	    cout << "#LITERALS = " << num_alt_facts<<endl;
#endif	    
	    server* my_server = new server(PORT);
	    my_server->start();
	    delete my_server;
	    return 0;

 }

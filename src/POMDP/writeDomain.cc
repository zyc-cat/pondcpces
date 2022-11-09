#include "writeDomain.h"
#include <sstream>
#include "math.h"
#include "globals.h"
#include "dd.h"

void writePOMDPHeader(ostringstream* pomdp){
  *pomdp << "discount: 1.0" << endl
	<< "values: cost" << endl
	<< "states: " << pow(2, num_alt_facts) << endl
	<< "actions: " << num_alt_acts << endl
	<< "observations: 1" <<endl << endl;
}

extern double get_min(DdNode*);
extern void printBDD(DdNode*);

static int Num_States = 0;
static int Num_NextStates = 0;
void writeMatrixRecurse(ostringstream* pomdp, DdNode* matrix, 
			DdNode* states, int variable_to_fix, int act,
			DdNode* preconds){
  DdNode *p, *n, *pos, *neg, *states_pos, *states_neg;

  //  cout << " " << variable_to_fix << flush;

  if(variable_to_fix == 2*num_alt_facts-1){ //have s and s'   
    Num_NextStates++;
    //       cout << "REached transition" <<endl;

    *pomdp <<  "T: " << act << " : " << Num_States
	     << Num_NextStates << " : " << get_min(states) << flush;
    return;
  }
 
  p = Cudd_bddIthVar(manager, variable_to_fix);
  Cudd_Ref(p);
  pos = Cudd_BddToAdd(manager, p);
  Cudd_Ref(pos);
  n = Cudd_Not(p);
  Cudd_Ref(n);
  neg = Cudd_BddToAdd(manager, n);
  Cudd_Ref(neg);  
  
  Cudd_RecursiveDeref(manager, p);
  Cudd_RecursiveDeref(manager, n);
   

  states_pos = Cudd_addApply(manager, Cudd_addTimes,
			     pos, states);
  Cudd_Ref(states_pos);
  states_neg = Cudd_addApply(manager, Cudd_addTimes,
			     neg, states);
  Cudd_Ref(states_neg);
  
  if(variable_to_fix == 2*(num_alt_facts-1)){//have s now
    DdNode *bstate = Cudd_addBddStrictThreshold(manager, states, 0.0);
    Cudd_Ref(bstate);
    
    Num_States++;
    Num_NextStates = 0;

    cout << " " << Num_States << flush;

    //if is a goal state, then put self loop and exit recursion
    if(bdd_entailed(manager, bstate, b_goal_state)){
      *pomdp << "T: " << act << " : " << Num_States
	     << Num_States << " : 1.0"<< endl;

      Cudd_RecursiveDeref(manager, bstate);
      Cudd_RecursiveDeref(manager, pos);
      Cudd_RecursiveDeref(manager, neg);
      Cudd_RecursiveDeref(manager, states_pos);
      Cudd_RecursiveDeref(manager, states_neg);
      cout << "(G)" << flush;
      return;
    }
    else{ //continue recursion
      Cudd_RecursiveDeref(manager, bstate);
      variable_to_fix = 1;
    }
  }
  else {//working on s or s'
    variable_to_fix += 2;
  }


//   cout << "POS"<<endl;
//   printBDD(states_pos);
  DdNode *bpstate = Cudd_addApply(manager, Cudd_addTimes, states_pos, matrix);
  Cudd_Ref(bpstate);
  if(bpstate != Cudd_ReadZero(manager) && 
     add_bdd_entailed(manager, states_pos, preconds))
    writeMatrixRecurse(pomdp, matrix, states_pos, variable_to_fix, act, preconds);
  else if(variable_to_fix%2 == 0){
    Num_States += pow(2, (2*num_alt_facts-variable_to_fix)/2 );
      cout << "(S)" <<flush;
  }
  else{
     Num_NextStates += pow(2, (2*num_alt_facts-1-variable_to_fix)/2 );   
     //    cout << "(S)" <<flush;
  }
  Cudd_RecursiveDeref(manager, bpstate);

//    cout << "NEG"<<endl;
//    printBDD(states_neg);
  DdNode *bnstate = Cudd_addApply(manager, Cudd_addTimes, states_neg, matrix);
  Cudd_Ref(bnstate);
  if(bnstate != Cudd_ReadZero(manager) &&
     add_bdd_entailed(manager, states_neg, preconds))
    writeMatrixRecurse(pomdp, matrix, states_neg, variable_to_fix, act, preconds);
  else if(variable_to_fix%2 == 0){
    Num_States += pow(2, (2*num_alt_facts-variable_to_fix)/2 );
      cout << "(S)" <<flush;
  }
  else{
     Num_NextStates += pow(2, (2*num_alt_facts-1-variable_to_fix)/2 );   
     //    cout << "(S)" <<flush;
  }
  Cudd_RecursiveDeref(manager, bnstate);

 
  Cudd_RecursiveDeref(manager, pos);
  Cudd_RecursiveDeref(manager, neg);
  Cudd_RecursiveDeref(manager, states_pos);
  Cudd_RecursiveDeref(manager, states_neg);
 
}


void writeMatrix(ostringstream* pomdp, DdNode* matrix, int act, DdNode* preconds){
  DdNode *states = Cudd_ReadOne(manager);
  Cudd_Ref(states);
  writeMatrixRecurse(pomdp, matrix, states, 0, act, preconds);
  Cudd_RecursiveDeref(manager, states);

}



void writePOMDPActions(ostringstream* pomdp){
  int action_id = 0;
    for(std::map<const Action*, DdNode*>::iterator a = action_preconds.begin();
      a != action_preconds.end(); a++){
      DdNode* action = groundActionDD(*((*a).first));
      Cudd_Ref(action);
      
      cout << "Act" << endl;

      DdNode* preconds = action_preconds[((*a).first)];
      Cudd_Ref(preconds);

      writeMatrix(pomdp, action, action_id++, preconds);
      Cudd_RecursiveDeref(manager, preconds);
   

    }


}

void writePOMDPObservations(ostringstream* pomdp){
  

}

void writePOMDPRewards(ostringstream* pomdp){
}

void writePOMDPProblem(){
  ostringstream pomdp (ostringstream::out);
  
  cout << "Writing POMDP Header" << endl;


  writePOMDPHeader(&pomdp);
  cout << "Writing POMDP Actions" << endl;
  writePOMDPActions(&pomdp);
  cout << "Writing POMDP Observations" << endl;
  writePOMDPObservations(&pomdp);
  writePOMDPRewards(&pomdp);

  cout << pomdp.str();

}

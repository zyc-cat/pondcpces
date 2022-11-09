#include <iostream>
#include <fstream>

#include "movalfn.h"

#include "globals.h"
#include "lug.h"
#include "solve.h"
#include "lao_wrapper.h"
#include "dd.h"

#include <list>
#include <set>
#include <iostream>
#include <iomanip>
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <limits.h>         // INT_MAX

using namespace std;

extern double gWeight;
extern ofstream pfout;
extern int num_lugs;
static int mactNum = 1;

int MAXCONTINGENCIES = INT_MAX;

extern clock_t gStartTime;

int MOValueFunction::numSolutions(){
  int num = 0;

  for(__gnu_cxx::MOValueFunctionHash::iterator i = points.begin();
      i != points.end(); i++){
    if((*i)->solution)
      num++;
  }

  return num;
}


bool mvfpCmp(MOValueFunctionPoint* s1,
			MOValueFunctionPoint* s2)  {
		return (s1->goalSatisfaction < s2->goalSatisfaction);
	}

void  MOValueFunction::print(ostream &out, int index){
  //  out.setw(8);
  //  out << "***********************" << endl
  //  << "pr(G)\t\tf\tact\tsol" << endl;

  //    printf("%3d ( %f secs.)\n",
  //       Iteration, (float)(clock()-gStartTime)/CLOCKS_PER_SEC);

  double time = ((float)(clock()-gStartTime)/CLOCKS_PER_SEC);

  if(index > -1)
    time = (double)index;

  out << endl;
//   out //<< (*i)->g <<"\t"
//      //<< (*i)->h << "\t"
//      //<< setprecision(20)
//      << "T" << "\t"
//      << "1-Pr(G)" << "\t"
//      << "E[f]" << "\t"
//      << "a"   << "\t"
//      << "sol" << "\t"
//      << "br" << "\t"
//      << "ptr" << "\t"
//      << "state" << "\t"
//      //      << (*i)->bestPoints.size() << " "
//      << "dom" << endl;

   list<MOValueFunctionPoint*> pts;
   pts.insert(pts.begin(),points.begin(), points.end());
   pts.sort(mvfpCmp);


   for(list<MOValueFunctionPoint*>::iterator i = pts.begin();
        i != pts.end(); i++){
//	   for(__gnu_cxx::MOValueFunctionHash::iterator i = points.begin();
//	        i != points.end(); i++){
//
		   if(0 && (*i)->bestAction == NULL && (*i)->goalSatisfaction == 0.0)
       continue;

    out //<< (*i)->g <<"\t"
      //<< (*i)->h << "\t"
      //<< setprecision(20)
      << time << "\t"
      << 1.0-(*i)->goalSatisfaction << "\t"
      << (*i)->f << "\t"
      << (*i)->g << "\t"
      << (*i)->h << "\t"
      //<< (((*i)->bestAction != NULL) ? ((*i)->bestAction->ActionNo) : -1)   << "\t"
      << ((*i)->solution ? "1" : "0") << "\t"
      //<< (*i)->contingencies << "\t"
      //<< *i << " "
      //<< (*i)->stateNode->StateNo << "\t"
      //      << (*i)->bestPoints.size() << " "
       //   << (*i)->dominated << " "
      ;
//     for(list<MOValueFunctionPoint*>::iterator ns = (*i)->bestPoints.begin();
// 	ns != (*i)->bestPoints.end(); ns++){
//        out << *ns << " ";
//      }
     out << endl;
  }



//   out //<<setw(8)
//       << time
//       << "\t\t" << gDefault << "\t"
//       << ((actionDefault != NULL) ? (actionDefault->ActionNo)
//  	  : -1)
//       << "\t0"
//       << endl;
}
void  MOValueFunction::print(ostream &out){
  MOValueFunction::print(out, -1);
}

void printPolicyR(MOValueFunctionPoint* p, int indent, int step, int &max, int &min,
		 int &numPlans, BitVector* solved_visited,
		 double cost, list<double>* costs, double path_pr,
		  list<double>* plan_prs );



void markPlan(MOValueFunctionPoint* p)
{

//   if(p->inPlan)
//     cout << "already marked" << p->stateNode->StateNo << endl;

  if (!p || p->inPlan || !p->bestAction){
    return;
  }

  p->inPlan = true;
  // cout << "mark" << p->stateNode->StateNo << p << endl;
  //printAction((p)->bestAction);
  struct StateDistribution* nextStates = p->bestAction->NextState;
  while(nextStates){
    for(list<MOValueFunctionPoint*>::iterator ns = p->bestPoints.begin();
	ns != p->bestPoints.end(); ns++){
      if((*ns)->stateNode->StateNo == nextStates->State->StateNo){
	markPlan((*ns));
	break;
      }

    }
    nextStates = nextStates->Next;
  }



}

double computeDistance(MOValueFunctionPoint *a, MOValueFunctionPoint *b){
  double distance = 0.0;
  //  cout << "distance" << endl;
  distance += pow(a->goalSatisfaction - b->goalSatisfaction, 2.0);
  distance += pow(a->f - b->f, 2.0);
  distance = sqrt(distance);
  return distance;
}

void computeDistanceMatrix(double*** distance, __gnu_cxx::MOValueFunctionHash *pts){
  int ix = 0, jx;
  for(__gnu_cxx::MOValueFunctionHash::iterator i = pts->begin();
      i != pts->end(); i++, ix++){
    jx=0;
    for(__gnu_cxx::MOValueFunctionHash::iterator j = pts->begin();
	j != pts->end(); j++, jx++){
      //  cout << "distance " << ix << " " << jx << endl;
      (*distance)[ix][jx] = computeDistance(*i, *j);
    }
  }
}

enum actionSelection { MOST_DIVERSE, MOST_OPTIONS, RANDOM } actSelection = //
		 MOST_DIVERSE;//
		 //MOST_OPTIONS;//
		 //RANDOM;



ActionNode* pickNextAction(StateNode* node, int actInd){
  //  ActionNode* nodeToReturn;

  __gnu_cxx::MOValueFunctionHash solutions;
  for(__gnu_cxx::MOValueFunctionHash::iterator i = node->moValueFunction->points.begin();
      i != node->moValueFunction->points.end(); i++){
    if((*i)->solution){
      solutions.insert(*i);
      //      cout << "sol" << endl;
    }
  }

  //    cout << "Picking from " << node->moValueFunction->points.size() << " Options" << endl;
  node->moValueFunction->print(cout, actInd);


  if(actSelection == RANDOM){
    double sn = ((double)randomGen->myrand()/((double)(RAND_MAX)+(double)(1)));
    double cumm = 0.0;
    for(__gnu_cxx::MOValueFunctionHash::iterator i = solutions.begin();
	i != solutions.end(); i++){
      cumm += 1.0/(double)solutions.size();

      //cout << "Option: " <<  cumm << " " << sn << endl;
      //      printAction((*i)->bestAction); cout <<flush;
      if(cumm < sn )	continue;
      return (*i)->bestAction;
    }
  }
  else if(actSelection == MOST_OPTIONS){
    int actOpts[num_alt_acts+1];
    for(int j = 0; j < num_alt_acts+1; j++)
      actOpts[j]=0;

    for(__gnu_cxx::MOValueFunctionHash::iterator i = solutions.begin();
	i != solutions.end(); i++){
      if((*i)->bestAction == NULL) //null action, stop
	actOpts[num_alt_acts]++;
      else{
	actOpts[(*i)->bestAction->ActionNo-1]++;
      }
    }
    int max = 0;
    int bestAct = num_alt_acts;
    for(int j = 0; j < num_alt_acts+1; j++){
      if(actOpts[j] > max){
	max = actOpts[j];
	bestAct = j;
      }
    }
    if(bestAct == num_alt_acts)
      return NULL;

    for(__gnu_cxx::MOValueFunctionHash::iterator i = solutions.begin();
	i != solutions.end(); i++){
      if((*i)->bestAction == NULL)
	continue;
      if(((*i)->bestAction->ActionNo-1) == bestAct)
	return (*i)->bestAction;
    }
  }
  else if(actSelection == MOST_DIVERSE){
    int actOpts[num_alt_acts+1];
    int actIndex[solutions.size()];
    double actDiversity[num_alt_acts+1];
    double **distance = new double*[solutions.size()];
    for(int j = 0; j < num_alt_acts+1; j++){
      actOpts[j]=0;
      actDiversity[j] = 0.0;

    }
    int index = 0;
    for(__gnu_cxx::MOValueFunctionHash::iterator i = solutions.begin();
	i != solutions.end(); i++){
      distance[index] = new double[solutions.size()];
      if((*i)->bestAction == NULL){ //null action, stop
	actOpts[num_alt_acts]++;
	actIndex[index++] = -1;
      }
      else{
	actOpts[(*i)->bestAction->ActionNo-1]++;
	actIndex[index++] = (*i)->bestAction->ActionNo-1;
      }
    }
    //    cout << "HI" << endl;
    computeDistanceMatrix(&distance, &solutions);
    index = 0;
    //    cout << "oHI" << endl;
    for(__gnu_cxx::MOValueFunctionHash::iterator i = solutions.begin();
	i != solutions.end(); i++, index++){

      int actId = ((*i)->bestAction ? (*i)->bestAction->ActionNo-1 : num_alt_acts+1);
      int index1 = 0;
      for(__gnu_cxx::MOValueFunctionHash::iterator j = solutions.begin();
	j != solutions.end(); j++, index1++){
      int actId1 = ((*j)->bestAction ? (*j)->bestAction->ActionNo-1 : num_alt_acts+1);
	if(i!=j && actIndex[index] == actIndex[index1]){
	  actDiversity[actId] += (distance)[index][index1];
	}
      }
      actDiversity[actId] /= (double)solutions.size();
    }
    //    cout << "oHIo" << endl;
    double max = 0;
    int bestAct = num_alt_acts;
    for(int j = 0; j < num_alt_acts+1; j++){
      if(actDiversity[j] > max){
	max = actDiversity[j];
	bestAct = j;
      }

    }
    for(int j = 0; j < solutions.size(); j++)
      delete [] distance[j];
    delete [] distance;
    if(bestAct == num_alt_acts)
      return NULL;

    for(__gnu_cxx::MOValueFunctionHash::iterator i = solutions.begin();
	i != solutions.end(); i++){
      if((*i)->bestAction == NULL)
	continue;
      if(((*i)->bestAction->ActionNo-1) == bestAct)
	return (*i)->bestAction;
    }
  }

  // return nodeToReturn;

}

StateNode* simulateAction(ActionNode* act){
  double sn = ((double)randomGen->myrand()/((double)(RAND_MAX)+(double)(1)));
  double cumm = 0.0;
  for (StateDistribution *successor = act->NextState;
       successor;
       successor = successor->Next) {
    cumm += successor->Prob;
    //    cout << "Outcome: " <<  cumm << " " << sn << endl;
    if(cumm < sn){
      continue;
    }
    return successor->State;
  }
}

//use different strategies to execute a multi-option plan
//strategies: 1) Most option action
//            2) Action closest to Obj. Function
//            3) Random Action
//            4) Most diverse option action

void executeOptions(){
  //while not at terminal, choose action, make transition
  StateNode *currentNode = Start;
  int index = 0;
  while(!currentNode->Terminal){
    ActionNode *act = pickNextAction(currentNode, index++);
    if(!act)
      break; //okay, just ended plan
    currentNode = simulateAction(act);
  }

  cout << "Ended: Pr(G) = " << currentNode->goalSatisfaction << endl;
}


void printPolicy(MOValueFunctionPoint *p){
  gEndTime = clock();
  int min, max, numPlans;
  min = 10000;
  max = -1;
  numPlans = 0;
  BitVector*   solved_visited;// = new_bit_vector(((int)(gNumStates)/(gcword_size)));
  pfout.open (out_file, ofstream::out );

  list<double> costs;
  list<double> plan_prs;

  double expcost = 0;
  double exppr = 0;

  gNumStates = state_count;
  pfout << "(define (plan " << dname << ")" <<endl;
  pfout << "\t(:problem " << pname << ")" <<endl;
  pfout << "(" << endl;

  cout << "NumStatesGenerated = " << state_count << endl;
  if(!p->solution){
    cout << "$$$$$$$$$$$$$$$No Plan :( $$$$$$$$$$$$$$$$$$$$$$$$"<< endl;
    cout << "P(G) = " << p->goalSatisfaction << endl;
  }
  else{
    cout << "$$$$$$$$$$$$$$$GOT PLAN$$$$$$$$$$$$$$$$$$$$$$$$"<<endl;
    markPlan(p);
    printPolicyR(p, 0, 1, max, min, numPlans, solved_visited, 0, &costs, 1.0, &plan_prs);
    cout << "$$$$$$$$$$$$$$$GOT PLAN!$$$$$$$$$$$$$$$$$$$$$$$$"<<endl;
    pfout << ")" << endl;
    pfout << ")" <<endl;
    pfout.close();

    cout << "Num LUGS = " << num_lugs << endl;

    for(list<double>::iterator i = costs.begin(); i != costs.end(); i++)
      expcost += *i;

    for(list<double>::iterator i = plan_prs.begin(); i != plan_prs.end(); i++)
      exppr += *i;
    cout << "Num Contingencies = " << p->contingencies << endl;
    cout << "Expected Cost = "<< p->g << endl;
    cout << "p(Plan Success) = "<< p->goalSatisfaction <<endl;
  }

  printf("Total User Time = %f secs\n", (float)((gEndTime-gStartTime)/(float)CLOCKS_PER_SEC));
}

void printPolicyR(MOValueFunctionPoint* p, int indent, int step,
		  int &max, int &min,
		  int &numPlans, BitVector* solved_visited,
		  double cost, list<double>* costs, double path_pr,
		  list<double>* plan_prs ){

//   cout << "Enter outptut R"<<endl;

//	cout << "{" <<cost <<"}" << endl;

  //   printf("State = %d\n", s->StateNo);
  //cout << s->g <<endl;
  // printBDD(p->stateNode->dd);
  //pfout << "\t";

  //cout << s->ExtendedGoalSatisfaction << endl;

  if (!p)    {
     pfout << "l" << mactNum<<" (done " << mactNum << " " <<-1<<")"<<endl;
    cout << "l" << mactNum<< " (done " << mactNum << " inf)" <<endl;
    return;
  }

  for(int i = 0; i < indent; i++){
    cout << " ";
    pfout << " ";
  }
  //cout << "dom: " << (p)->dominated << endl;
//   p->stateNode->moValueFunction->print(cout);

  //cout << s->Terminal << " " << s->StateNo << " " << endl;
  //cout << (long) s->BestAction << endl;

  if(!p->bestAction){

    //printBDD(s->dd);
    pfout << "(done)" << endl;
    cout <<"(done) P(G) = " << p->goalSatisfaction
      //<< " P(branch) = " << s->prReached //path_pr
	 <<  endl;

    if(p->goalSatisfaction > 1){
      p->stateNode->moValueFunction->print(cout);
    }

    return;
  }

  //  printBDD(s->dd);

//   print_BitVector(solved_visited, 1);printf("\n");
//   cout << "my point " << p << endl;
//      for(__gnu_cxx::MOValueFunctionHash::iterator ns = p->stateNode->moValueFunction->points.begin();
// 	 ns != p->stateNode->moValueFunction->points.end(); ns++){
//        if(p == *ns)
// 	 cout << "this" << flush;
//        if((*ns)->inPlan){
// 	 cout << "point " << *ns << flush;
// 	 printAction((*ns)->bestAction);
// 	 cout << flush << endl;
//        }
//        else if(0){
// 	 cout << "no point ";
// 	 printAction((*ns)->bestAction);
// 	 cout << flush << endl;
//        }
//      }

   if( p->bestAction &&
       p->stateNode->Update == -10 && //get_bit(solved_visited, gNumStates, s->StateNo) &&
       p->bestAction->Solved != -1
       ){
     pfout <<"(goto l"<< p->bestAction->Solved << ")"<< endl;
     cout << "(goto l" << p->bestAction->Solved << ")"<< endl;
     return;
   }
   else if(p->bestAction){
 //     bool first = true;
     set<ActionNode*> bestActs;
      for(__gnu_cxx::MOValueFunctionHash::iterator ns = p->stateNode->moValueFunction->points.begin();
	 ns != p->stateNode->moValueFunction->points.end(); ns++){
       if((*ns)->inPlan){
	 bestActs.insert((*ns)->bestAction);
// 	 if(!first){
// 	 for(int i = 0; i < indent; i++){
// 	   cout << " ";
// 	   pfout << " ";
// 	 }
// 	 }
// 	 first = false;
// 	 cout << "l" << mactNum << " ";
// 	 cout << "(" << (*ns)->goalSatisfaction << ", " << (*ns)->f << ")";
// 	 printAction((*ns)->bestAction);

// 	 cout << flush << endl;
        }
     }

  //     //cout << "act = " <<endl;
      //      pfout << "l" << mactNum << " " << getAction(s->BestAction) << " " << endl;
      cout << "l" << mactNum << " " << bestActs.size() << flush;
      //<< getAction(s->BestAction)
      printAction(p->bestAction);
      cout << flush;

//       for(int i = 0; i < MAX_H_INCREMENT; i++)
// 	cout << " " << s->hValues[i];
//       cout << endl;

      //      cout << " " << endl;
      //   cout << mactNum << ": " << s->BestAction->act->get_name() << endl;
      ////      set_bit(solved_visited, s->StateNo);
      p->stateNode->Update = -10;
      p->bestAction->Solved = mactNum++;
  }

  else {
     pfout << "l" << mactNum<<" (done " << p->g << " " <<p->h<<")"<<endl;
    cout << "l" << mactNum<< " (done " << p->g << " " <<p->h<<") "<<endl;
    return;
  }


   struct StateDistribution* nextStates = p->bestAction->NextState;


  //cout << (long)nextStates << endl;

   if (!nextStates)//p->bestPoints.empty())
    {
      printPolicyR(NULL,indent,step+1,max,min,numPlans,solved_visited,
		  cost+p->bestAction->Cost,
		  costs, path_pr,plan_prs);
      return;
    }

   if(//nextStates->Next)
     p->bestPoints.size() > 1)
    {
      // then we have more than one next state distinguished by reasons
      for(int i =0; i < indent; i++){
	cout << " ";
	pfout << " ";
      }
      pfout << "(case ";
      cout << "(case ";
      indent += 5;

      while(nextStates){
	//cout << "LOOPING next states" <<endl;

	pfout << endl;
	cout << endl;
	for(int i = 0; i < indent-1; i++){
	  cout << " ";
	  pfout <<  " ";
	}
	cout << "(" << nextStates->Prob ;
	pfout << "(";


	//pfout << BDDToITEString(nextStates->reason);
	//	cout << BDDToITEString(nextStates->reason);

	pfout << endl;
	cout << endl;

	//	bool found = false;
	for(list<MOValueFunctionPoint*>::iterator ns = p->bestPoints.begin();
	    ns != p->bestPoints.end(); ns++){
//  	  cout << p->bestPoints.size() << " "
//  	       << (*ns)->stateNode->StateNo << " "
//  	       << nextStates->State->StateNo << endl;
	  if((*ns)->stateNode->StateNo == nextStates->State->StateNo){
	    //  found = true;
	    printPolicyR((*ns), indent, step+1, max, min,
			numPlans, solved_visited, cost+p->bestAction->Cost,
			costs, path_pr*nextStates->Prob,plan_prs);
	    break;
	  }
	}
// 	if(!found)
// 	  cout << "NOT FOUND" << endl;



	for(int i =0;  i < indent-1; i++){
	  cout << " ";
	  pfout << " ";
	}
	pfout << ")" << endl;
	cout << ")" << endl;

	nextStates = nextStates->Next;

      }
      indent -= 5;
      for(int i =0; i < indent; i++){
	pfout << " ";
	cout << " ";
      }

      pfout << ")" << endl;
      cout << ")" << endl;
    }
   else if(p->bestPoints.size() > 0)
    {
      //cout << (long) nextStates->State << endl;
      printPolicyR(p->bestPoints.front(),indent,step+1,max,min,
		   numPlans,solved_visited,
		   cost+p->bestAction->Cost,
		   costs, path_pr,plan_prs);
    }
}

void getMOLAOHeuristics(list<StateNode*> *states, StateNode *parent){


  double goalSat = goal_threshold;
  int MGS = NUMBER_OF_MGS;
  double gridPoints = 1;
  double gridSpacing = 1.0/(gridPoints);
  double gridValue = gridSpacing;





  for(;gridValue <= 1.0; gridValue += gridSpacing){
    if(LUG_FOR==SPACE)
      goal_threshold = gridValue;//1.0;
    //NUMBER_OF_MGS = MGS * gridValue;
    getHeuristic(states, parent, 0);

    for(list<StateNode*>::iterator i = states->begin();
	i != states->end(); i++){
      //cout << "Add h for " << (*i)->StateNo << " h = " << (*i)->h << endl;

      //(*i)->h *= (*i)->kld;
      (*i)->moValueFunction->points.insert(new MOValueFunctionPoint((*i)->moValueFunction->gDefault,
								    (*i)->h,
								    (*i)->moValueFunction->gDefault + (gWeight * (*i)->h),
								    gridValue,
								    gridValue,
								    1,
								    0.0,
								    NULL, false, *i));

    }

  }
  //cout << "done"<<endl;

  goal_threshold = goalSat;
  NUMBER_OF_MGS = MGS;
}

bool ancestorOf(MOValueFunctionPoint *p2, MOValueFunctionPoint *p1){
  //is p2 an ancestor of p1

  //cout << "Is ancestorOf " << p2 << " " << p1 << endl;

  if(p1 == p2)
    return true;
//   else if (p1->stateNode == Start)
//     return false;

  for(set<MOValueFunctionPoint*>::iterator i = p1->users.begin();
      i != p1->users.end(); i++){
    if( ancestorOf(p2, *i))
      return true;
  }
  return false;
}

bool MOValueFunction::dominates(MOValueFunctionPoint *p1,
				MOValueFunctionPoint *p2,
				double epsilon){
  //if p1 dominates p2 return true, and false otherwise
  //p1 dominates p2 if:
  //                   1) Pr(G|p1) > Pr(G|p2), and c(p1) - c(p2) < cost-epsilon
  //                   2) c(p1) <= c(p2), and Pr(G|p2) - Pr(G|p1) < pr-epsilon
  //                   3)

  bool ancestor = false;
   if(epsilon != 0.0)
     ancestor = (//ancestorOf(p2, p1) //||
		 ancestorOf(p1, p2)
		 );

   // cout << "Does " << p1 << " dom " << p2 << " anc = " << ancestor << endl;
  if(ancestor) //children cannot dominate ancestors
     return false;






//   cout << (1.0-p1->goalSatisfaction) << " " << p1->f << endl;
//   cout << (epsilon)*(1.0-p1->goalSatisfaction) << " " << (epsilon)*p1->f << endl;

//   cout << (1.0-p2->goalSatisfaction) << " " << p2->f << endl;
//   cout << (epsilon)*(1.0-p2->goalSatisfaction) << " " << (epsilon)*p2->f << endl;

  if(epsilon != 0.0){
    bool dom1 = (//1->solution == p2->solution &&
		 (p1->solution || !p2->solution) &&
		 (((1.0 - p1->goalSatisfaction) <
		  ((epsilon)*(1.0 - p2->goalSatisfaction)) &&
		  (p1->f <= (epsilon * p2->f)))
		  ||
		  ((1.0 - p1->goalSatisfaction) <=
		   ((epsilon)*(1.0 - p2->goalSatisfaction)) &&
		   (p1->f < (epsilon * p2->f))))
		 );
    bool dom2 = (//p1->solution == p2->solution &&
		 (p1->solution || !p2->solution) &&
		 (((1.0 - p2->goalSatisfaction) <
		  ((epsilon)*(1.0 - p1->goalSatisfaction)) &&
		  (p2->f <= (epsilon * p1->f)))
		  ||
		  ((1.0 - p2->goalSatisfaction) <=
		   ((epsilon)*(1.0 - p1->goalSatisfaction)) &&
		   (p2->f < (epsilon * p1->f))))
		 );
    //cout << "dom1 = " << dom1 << " dom2 = " << dom2 << endl;
    if(!(dom1 && dom2)){
      return dom1;
    }
    else{
      bool dom3 = (p1->solution == p2->solution &&
	    //(p1->solution ||  !p2->solution) &&
	    (((1.0 - p1->goalSatisfaction) <=
	      ((1.0 - p2->goalSatisfaction)) &&
	      (p1->f < p2->f))
	     ||
	     ((1.0 - p1->goalSatisfaction) <
	      ((1.0 - p2->goalSatisfaction)) &&
	      (p1->f <=  p2->f))
	     ));
      bool dom4 = (p1->solution == p2->solution &&
	    //(p1->solution ||  !p2->solution) &&
	    (((1.0 - p2->goalSatisfaction) <=
	      ((1.0 - p1->goalSatisfaction)) &&
	      (p2->f < p1->f))
	     ||
	     ((1.0 - p2->goalSatisfaction) <
	      ((1.0 - p1->goalSatisfaction)) &&
	      (p2->f <=  p1->f))
	     ));
          if((dom3 || dom4))
	return dom3;
      else
	return true;


    }


  }
  else{

    return (//p1->solution == p2->solution &&
	    (p1->solution ||  !p2->solution) &&
	    (((1.0 - p1->goalSatisfaction) <=
	      ((1.0 - p2->goalSatisfaction)) &&
	      (p1->f < p2->f))
	     ||
	     ((1.0 - p1->goalSatisfaction) <
	      ((1.0 - p2->goalSatisfaction)) &&
	      (p1->f <=  p2->f))
	     ));
  }
}



// bool MOValueFunction::dominates(MOValueFunctionPoint *p1, MOValueFunctionPoint *p2){

//   //p1 dominates p2 if it has a lower slope and does not satisfy the goal
//   //any less than p2

//   double p1Slope, p2Slope;
//   double mEpsilon = -0.0000000001;
//   double dEpsilon = 0.00000001;
//   if(p1->solution != p2->solution)
//     return false;

//   p1Slope = (p1->goalSatisfaction > 0 ?
// 	     (p1->f / p1->goalSatisfaction) :
// 	     0.0);
//   p2Slope = (p2->goalSatisfaction > 0 ?
// 	     (p2->f / p2->goalSatisfaction) :
// 	     0.0);

//  //  cout << setprecision(20)
//     //<< p1Slope << " "
// 	     //<< p2Slope << " "
//   //   << (double)p1->goalSatisfaction << " "
//   //  << (double)p2->goalSatisfaction << " "
//     //<< (p1Slope <= p2Slope) << " "
//   //	    << (((double)p1->goalSatisfaction - (double)p2->goalSatisfaction) > mEpsilon)  << " "
//   //     << (p1->bestAction ? p1->bestAction->ActionNo : -1) << " "
//   //     << (p2->bestAction ? p2->bestAction->ActionNo : -1)
//   //     << endl;

//   return (p1->solution == p2->solution &&
// 	  //p1Slope <= p2Slope &&
// 	  (p1Slope - p2Slope) <  dEpsilon &&
// 	  ((double)p1->goalSatisfaction - (double)p2->goalSatisfaction) > mEpsilon);


// }


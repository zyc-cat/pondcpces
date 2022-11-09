#include "sample_size.h"
#include "globals.h"
#include "cudd/dd.h"
#include "graph_wrapper.h"
#include "lao_wrapper.h"
#include "lug.h"
#include "float.h"

double normal_quantile(double z){
 if(z == 0.9)
   return 0.8159;
 else if(z == 0.95)
    return 0.8289;
 else if(z == 0.99)
   return 0.8389;
 else{
    std::cout << "No normal quantile for " << z << std::endl;
    exit(0);
  }
}

//determine the number of samples need to approximate d with 
//probability 1-delta that the error is below epsilon
int mclugSampleSize(DdNode *d, double delta, double epsl){
  int k = 0, sample_min = 2, samples = 0, samples_needed = 0;
  std::list<DdNode*> cubes;
  std::list<double> values;
  double old_card = 0, new_card = 0;
    std::list<DdNode*>::iterator c;
    std::list<double>::iterator v;
  DdNode *sampled_states = Cudd_ReadLogicZero(manager);
  Cudd_Ref(sampled_states);

  DdNode *mask = Cudd_addBddStrictThreshold(manager, next_state_cube, 0.0);
  Cudd_Ref(mask);

  //std::cout << "d = " << delta << " e = " << epsl <<std::endl;

  get_cubes(d, &cubes, &values);

  //  samples_needed = sample_min;
  do {
    old_card = new_card;
    double s = ((double)randomGen->myrand()/((double)(RAND_MAX)+(double)(1))); 
    double cumm = 0.0;
    for(c = cubes.begin(), v = values.begin(); 
	c != cubes.end() && v != values.end();
	c++, v++){      
      cumm += (*v);
      if(cumm >= s){
	DdNode *tmp = Cudd_bddOr(manager, sampled_states, *c);
	Cudd_Ref(tmp);
	Cudd_RecursiveDeref(manager, sampled_states);
	sampled_states = tmp;
	Cudd_Ref(sampled_states);
	Cudd_RecursiveDeref(manager, tmp);
	tmp = Cudd_bddAnd(manager, sampled_states, mask);
	Cudd_Ref(tmp);
	Cudd_RecursiveDeref(manager, sampled_states);
	sampled_states = tmp;
	Cudd_Ref(sampled_states);
	Cudd_RecursiveDeref(manager, tmp);
	//printBDD(sampled_states);

	samples++;	
	new_card = Cudd_CountMinterm(manager, sampled_states, 2*num_alt_facts);
	if(new_card > old_card){
	  //std::cout << "new_card = " << new_card << std::endl;
	  
	  k++;
	  if(samples >= sample_min && k > 1){
	    //std::cout << "k = " << k << std::endl;
		 
	    samples_needed = ((k-1)/(2*epsl));


	    double a =  1.0 - (2.0/(9.0*((double)k-1.0)));
	    double b = (sqrt(2.0/(9.0*((double)k-1.0)))*normal_quantile(1.0-delta));

	    samples_needed *= pow(a + b, 3.0);

	  }
	}
	//std::cout << "Samples needed = " << samples_needed << std::endl;
	//std::cout << "Samples = " << samples << std::endl;
	//std::cout << "Samples min = " << sample_min << std::endl;

	break;
      }
     
    }
  } while (samples < samples_needed || samples < sample_min);

  for(c = cubes.begin(); c != cubes.end(); c++){
    Cudd_RecursiveDeref(manager, *c);
  }
  Cudd_RecursiveDeref(manager, sampled_states);
  Cudd_RecursiveDeref(manager, mask);

  //std::cout << "Samples = " << samples << std::endl;

  //exit(0);

  return samples;
}

int pickN(std::set<StateNode*>* beliefs, double epsl){
  int maxN = 0, N;

  for(std::set<StateNode*>::iterator i = beliefs->begin(); 
      i != beliefs->end(); i++){
    N = mclugSampleSize((*i)->dd, 0.01, epsl);
    if(N > maxN)
      maxN = N;
  }
  return maxN;
}

void walk(StateNode *parent, 
	  std::set<StateNode*>* beliefs, 
	  std::set<StateNode*>* generated,
	  double *branch_factor, 
	  int depth,
	  int max_depth){

  beliefs->insert(parent);

  if(depth == max_depth)
    return;

  if(!parent->BestAction && !parent->Terminal)
    parent->expand();

  //std::cout << "Gen: ";
  int children = 0;
  for(ActionNodeList::iterator a = parent->NextActions->begin(); a != parent->NextActions->end(); a++){
    (*a)->NextState->State->Expanded = -1;
    children++;
    generated->insert((*a)->NextState->State);
  }
  *branch_factor = (((*branch_factor) * (beliefs->size()-1)) + children) / beliefs->size();

  int child = rand()/(int)(((unsigned)RAND_MAX + 1) / children);
  for(ActionNodeList::iterator a = parent->NextActions->begin(); a != parent->NextActions->end(); a++){
    if(child == 0)
      walk((*a)->NextState->State, beliefs, generated, branch_factor, depth+1, max_depth);
    child--;   
  }
}

int random_walk_sample_size(StateNode *start, double epsl){
  int heuristic = HEURISTYPE;
  std::set<StateNode*> beliefs, generated;
  double branch_factor = 0;

  HEURISTYPE = LUGLEVEL;
  std::list<StateNode*> tl;
  tl.push_back(start);
  NUMBER_OF_MGS = 4;

  if(LUG_FOR == SPACE)
    getHeuristic(NULL, start, 0);
  getHeuristic(&tl, start, 0);
  int depth = start->h;
  
if(depth < 1 || depth == DBL_MAX)
	depth = 10;

std::cout << "Depth = " << depth << std::endl;

  HEURISTYPE = NONE;
  walk(start, &beliefs, &generated, &branch_factor, 0, depth);
  start->Expanded = -1;

  NUMBER_OF_MGS = log((double)pickN(&beliefs, epsl))*branch_factor;


if(NUMBER_OF_MGS < 1)
 NUMBER_OF_MGS =1;

  std::cout << "Branch factor = " << branch_factor << std::endl;
  std::cout << "Samples are " << (int)NUMBER_OF_MGS << std::endl;	    
  //      exit(0);


  HEURISTYPE = heuristic;


  return NUMBER_OF_MGS;
}

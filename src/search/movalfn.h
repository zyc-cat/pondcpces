#ifndef MOVALFN_H
#define MOVALFN_H

#include <ext/hash_set>
#include <iterator>
#include <iostream>

class MOValueFunction;
#include "graph.h"
#include <list>
#include <set>

struct StateNode;
void getMOLAOHeuristics(std::list<StateNode*> *states,
			StateNode *parent);

class MOValueFunctionPoint;
void printPolicy(MOValueFunctionPoint *p);

bool ancestorOf(MOValueFunctionPoint *p2, MOValueFunctionPoint *p1);
extern int MAXCONTINGENCIES;


class MOValueFunctionPoint {
 public:
  MOValueFunctionPoint(double _g,
		       double _h,
		       double _f,
		       double _gridValue,
		       double _goalSatisfaction,
		       long int _contingencies,
		       double _meanFirstPassage,
		       struct ActionNode *_bestAction,
		       bool _solution,
		       struct StateNode* _stateNode) :
    g(_g), h(_h), f(_f), gridValue(_gridValue),
    goalSatisfaction(_goalSatisfaction),
    contingencies(_contingencies),
    meanFirstPassage(_meanFirstPassage),
    bestAction(_bestAction),
    solution(_solution), stateNode(_stateNode)
    {inPlan = false; dominated = false;
      //      std::cout << "made pt: " << this << std:: endl;
    }
  MOValueFunctionPoint(MOValueFunctionPoint *p) :
    g(p->g), h(p->h), f(p->f), gridValue(p->gridValue),
    goalSatisfaction(p->goalSatisfaction),
    contingencies(p->contingencies),
    meanFirstPassage(p->meanFirstPassage),
    bestAction(p->bestAction),
    solution(p->solution), stateNode(p->stateNode),
    dominated(p->dominated), inPlan(p->inPlan)
    {
      for(std::list<MOValueFunctionPoint*>::iterator i = p->bestPoints.begin();
	  i != p->bestPoints.end(); i++)
	bestPoints.push_back(*i);
      for(std::set<MOValueFunctionPoint*>::iterator i = p->users.begin();
	  i != p->users.end(); i++)
	users.insert(*i);

    }

  ~MOValueFunctionPoint(){
    //for(std::list<MOValueFunctionPoint*>::iterator i = bestPoints.begin();
    //i != bestPoints.end(); i++)
    //delete *i;
  }



  double g, h, f, goalSatisfaction, gridValue, meanFirstPassage;
  long int contingencies;
  struct ActionNode *bestAction;
  std::list<MOValueFunctionPoint*> bestPoints;
  struct StateNode* stateNode;
  bool solution;
  bool dominated;
  std::set<MOValueFunctionPoint*> users;
  bool inPlan; //used to print a specific plan.
};

namespace __gnu_cxx {
  struct eqvfp{
    bool operator()(const MOValueFunctionPoint* s1,
		    const MOValueFunctionPoint* s2) const {
      return (s1->goalSatisfaction == s2->goalSatisfaction &&
	      s1->f == s2->f &&
	      s1->bestAction == s2->bestAction);
    }
  };






template<>  struct hash<MOValueFunctionPoint*>{
    size_t const operator()( MOValueFunctionPoint* p) const {return (size_t)p;}
  };

  struct MOValueFunctionHash : public hash_set<MOValueFunctionPoint*, hash<MOValueFunctionPoint*>, eqvfp > {
    //    void insert(MOValueFunctionPoint *p) { insert(p); }
    //void insert(iterator f, iterator l) { insert(f, l); }

  };
}

class MOValueFunction {
 public:
  MOValueFunction() : gDefault(0.0), actionDefault(NULL) {}
  ~MOValueFunction(){
  /*   for(__gnu_cxx::MOValueFunctionHash::iterator i = points.begin(); */
/* 	i != points.end(); i++) */
/*       delete *i; */
  }


  static bool dominates(MOValueFunctionPoint *p1, MOValueFunctionPoint *p2, double epsilon);

  void getSolutionPts(MOValueFunction *nSolPts){
	   for(__gnu_cxx::MOValueFunctionHash::iterator i = points.begin();
		i != points.end(); i++){
		   if((*i)->solution)
			   nSolPts->points.insert(*i);
	   }
  }

  MOValueFunctionPoint* solutionNoLessThan(double p){
    for(__gnu_cxx::MOValueFunctionHash::iterator i = points.begin();
	i != points.end(); i++){
      if((*i)->goalSatisfaction >= p &&
	 (*i)->solution &&
	 (*i)->contingencies <= MAXCONTINGENCIES
	 )
	return *i;
    }
    return NULL;
  }

  MOValueFunctionPoint* getMaxPrSolPt(bool sol){
    double maxPr = -1.0;
    MOValueFunctionPoint* maxpt = NULL;
    for(__gnu_cxx::MOValueFunctionHash::iterator i = points.begin();
	i != points.end(); i++){
      if((*i)->goalSatisfaction >= maxPr && sol == (*i)->solution){
	maxPr = (*i)->goalSatisfaction;
	maxpt = *i;
      }
    }
    return maxpt;
  }


  int numSolutions();
  void  print(std::ostream &out);
  void  print(std::ostream &out, int);


  __gnu_cxx::MOValueFunctionHash points;
  double gDefault;
  ActionNode *actionDefault;


};






#endif

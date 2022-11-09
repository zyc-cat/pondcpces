#ifndef __MONITOR_H
#define __MONITOR_H

//#include "actions/actions.h"
//#include "cudd/dd.h"
//#include "cudd/cudd.h"


#include <string>
//#include <list>
//#include <stl.h>

struct DdNode;

#ifdef PPDDL_PARSER
#include "globals.h"
class StateFormula;
class plan;
extern const StateFormula* the_observation;
extern plan* the_plan;
extern DdNode* formula_bdd(const StateFormula&, bool);
extern size_t line_number;
Action *findAct(std::string name);
#else
class alt_action;
alt_action *findAct(std::string name);
#endif
class label_symbol;

class Instruction;
typedef std::pair<DdNode *,Instruction *> Guard;
typedef std::list<Guard> Guards;





std::string BDDToCNFString(DdNode* dd,int withAnd);
std::string BDDToDNFString(DdNode* dd);
std::string BDDToITEString(DdNode* dd,int withAnd);
std::string BDDToITEString(DdNode* dd);


// this is how you hardcode a plan

// #ifndef PLANNER
// #define PLANNER "\
// echo %1$s %2$s > /dev/null\n\
// echo \"((stack a b) (stack b c) (done))\" > %3$s\n"
// #endif


#ifndef PLANNER
#define PLANNER "\ echo pond %1$s %2$s -out %3$s %4$s > /dev/null 2>&1\npond %1$s %2$s -out %3$s %4$s\n"
#endif


//#define SUCCESS 1
#define ERROR   -1
#define END_LINE 0x0
#define MAX_MSG 100000
int read_line(int newSd, char *line_to_return);


class Monitor{
  int port;
  char *host;
  int sd;

  char *domain;
  char *problem;
  std::string options;

  DdNode* belief;
  DdNode* goal_belief;
  Instruction *ip;

  int status; 
  // 0, alive
  // 1, done (killed)
  // 2, done (voluntary)
  int episode; // the number of replanning episodes
  int failing; // currently experiencing failure
  int count; // the number of executed instructions
  int threshold; // the threshold on the number of executed
  // actions before replanning occurs regardless

 public: 
  Monitor(char *h,int p,char *dom,char *prob,std::string &opt);
  ~Monitor();
  int start();
  void setFailing(int i);

#ifdef PPDDL_PARSER
  DdNode *simulate(Action *a);
  void exec(Action *a);
#else
  DdNode *simulate(alt_action *a);
  void exec(alt_action *a);
#endif

  void wait();
  int holds(DdNode *condition);

  int observe(DdNode *condition); // 1, ok, 0, inconsistent
  void assume(DdNode *belief);
  void force(DdNode *effect);
  void resetip();
  void invalidate();
  void finish();

  void plan(char *pfile);
  void replan();

};

class Instruction
{
 protected:
  int terminating; 
  // -3 unset
  // -2 setting
  // -1 inf
  // 0...n minimum length
 public:
  Instruction *next;
  Instruction() { next = 0; terminating = -3;}
  virtual ~Instruction() { terminating = -4; Instruction *ptr=next; next=0; 
  if (ptr && ptr->terminating!=-4) 
    { 
      //cout << "Deleting: " << ptr << endl; 
      delete ptr;
    }
  }
  virtual Instruction * exec(Monitor *m) { return next;}
  virtual void load(Instruction *n=0) { if (!next) next=n; else next->load(n);}
  virtual int terminates() 
    { 
      int n;
      switch(terminating) 
	{ 
	case -3: 
	  terminating=-2;
	  n = next ? next->terminates() : -1;
	  terminating = n == -1 ? -1 : n+1;
	  break; 
	case -2: 
	  return -1; 
	  break; 
	} 
      return terminating; 
    }
};

class mAction : public Instruction
{
#ifdef PPDDL_PARSER
  Action* act;
#else
  alt_action *act;
#endif
  void init();
public:
#ifdef PPDDL_PARSER
  void set(Action *a);
  mAction(Action *a);
#else
  void set(alt_action *a);
  mAction(alt_action *a);
#endif
  void reset();
  mAction();
  ~mAction();
  Instruction * exec(Monitor *m);
};

class IfThen : public Instruction
{
  DdNode *pre;
  Instruction *thn; 
  void init();
public:
  void set(DdNode *p,Instruction * t);
  void reset();
  IfThen();
  IfThen(DdNode *p,Instruction* t);
  ~IfThen();
  Instruction * exec(Monitor *m);
  void load(Instruction *n);
  int terminates();
};

class IfThenElse : public Instruction
{
  DdNode *pre;
  Instruction *thn;   Instruction *els; 
  void init();
public:
  void set(DdNode *p,Instruction * t,Instruction *e);
  void reset();
  IfThenElse();
  IfThenElse(DdNode *p,Instruction* t,Instruction*e);
  ~IfThenElse();
  Instruction * exec(Monitor *m);
  void load(Instruction *n);
  int terminates();
};

class Case : public Instruction
{
  Guards *elements;
  void init();
public:
  void reset();
  void set(Guards *e);
  Case(Guards *e);
  Case();
  ~Case();
  Instruction* exec(Monitor *m);
  void load(Instruction *n);
  int terminates();
};

class Goto : public Instruction
{
  label_symbol *label;
  Instruction *unreached;
  void init();
public:
  void set(label_symbol *l);
  void reset();
  Goto(label_symbol * l);
  Goto();
  ~Goto();
  Instruction* exec(Monitor *m);
  void load(Instruction *n);
  int terminates();
};

class Done : public Instruction
{
public:
  Done() : Instruction() {}
  Instruction *exec(Monitor *m);
  int terminates();
};

class Lcp_Done : public Done
{
public:
  double g;
  double h;
  Lcp_Done(double tg, double th) : g(tg), h(th), Done() {}
  Instruction *exec(Monitor *m);
};

#endif

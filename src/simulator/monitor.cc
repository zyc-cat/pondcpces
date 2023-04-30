/* fpont 12/99 */
/* pont.net    */
/* tcpClient.c */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h> /* close */
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

#include "globals.h"
#include "monitor.h"
#include "cudd/dd.h"
// for parsing stuff
#include <FlexLexer.h>
extern int yyparse();
//extern int yyrestart();
extern int yyrestart(FILE*);
extern int yy_scan_string(const char*);
#include "lao_wrapper.h" // for DdNode *progress(alt_action,DdNode*)
#include "graph_wrapper.h" // for goalToBDD
/* File to parse. */
extern FILE* yyin;

#ifdef PPDDL_PARSER
#else
#include "actions/actions.h"
#include "parser/facts.h" // for the BDDTo functions using alt_facts
extern yyFlexLexer* yfl;
#endif






using namespace std;

// neither of the following two functions should be necessary
// resetip -> invalidate
// setFailing() -- failing is an internal flag, the code for handling it
// should be modified directly if different behaivour is required
//    in particular, setFailing(0) is the only reasonable call, so
//    at the very least, it should be restricted to resetFailing
void Monitor::resetip()
{
  ip = 0;
}

void Monitor::setFailing(int i) 
{
  failing = i;
}

#ifdef PPDDL_PARSER
void Monitor::exec(Action *a) {

  failing = 0; // no longer failing constantly

  const char *str = a->name().c_str();

  cout << "Executing: " << str << endl; 

  if(send(sd, str, strlen(str) + 1, 0)<0)
    {
      perror("cannot send data ");
      close(sd);
      exit(1);
    }

  count++;
}
#else
void Monitor::exec(alt_action *a) {

  failing = 0; // no longer failing constantly

  const char *str = a->get_name();

  cout << "Executing: " << str << endl; 

  if(send(sd, str, strlen(str) + 1, 0)<0)
    {
      perror("cannot send data ");
      close(sd);
      exit(1);
    }

  count++;
}
#endif

void Monitor::wait() {
  char line[MAX_MSG];
  DdNode *condition=0;
  //strcpy(line,"(done)");
  read_line(sd, line);
  printf("Received %s\n", line);
  if (strcmp(line,"(done)")==0)
    {
      status=1; // dead
      return;
    }
  if (strcmp(line,"()")!=0)
    {
      
      // build formula from line
      // could hack the parser and use istringstream
      // to make the parser get us a goal*
      
#ifdef PPDDL_PARSER
      line_number = 1;
      //istringstream iss(line);
      //yyin = fopen(iss, "r");
      yyrestart(NULL);

      yy_scan_string(line);
	//      yyparse();
#else
      line_no=1;
      istringstream iss(line);
      assert(yfl);
      yfl->switch_streams(&iss,&cout);
      yyparse();
#endif
      

#ifdef PPDDL_PARSER      
      condition = formula_bdd(*the_observation, false);
      delete the_observation;
      the_observation = NULL;
#else
      condition = goalToBDD(current_analysis->the_observation);
      delete current_analysis->the_observation;
      current_analysis->the_observation = 0;
#endif


      if(!observe(condition))
	force(condition);
    }
}
int Monitor::holds(DdNode *condition)
{ 
  return Cudd_bddLeq(manager,belief,condition);
}

int Monitor::observe(DdNode *condition)
{
  cout << "Attempting observation" << endl;
  DdNode *tmp;
  tmp = Cudd_bddAnd(manager,belief,condition);
  if (tmp == Cudd_ReadLogicZero(manager))
    return 0;
  Cudd_Ref(tmp);
  Cudd_RecursiveDeref(manager,belief);
  Cudd_RecursiveDeref(manager,condition);
  belief=tmp;
  return 1;
}

void Monitor::assume(DdNode *belief)
{
  cout << "Assuming progress" << endl;
  Cudd_RecursiveDeref(manager,this->belief);
  this->belief = belief;
}

#ifdef PPDDL_PARSER
DdNode * Monitor::simulate(Action *a)
{
  cout << "Simulating: " << a->name() << endl;
  return progress(action_transitions[a],belief);
}
#else
DdNode * Monitor::simulate(alt_action *a)
{
  cout << "Simulating: " << a->get_name() << endl;
  return progress(a,belief);
}
#endif

void Monitor::force(DdNode *effect)
{
  cout << "Forcing (by minimal exogenous action) observation" << endl;

  DdNode *op1,*op2,*acc,*frame,*result;

  acc = Cudd_bddVarMap(manager,effect);
  Cudd_Ref(acc);

  int i=0;
  int next;
  for(i=num_alt_facts-1;i>=0;i--)
    {
      if (Cudd_SupportIndex(manager,effect)[i])
	  continue;
      
      op1 = Cudd_ReadVars(manager,i);
      next = Cudd_bddReadPairIndex(manager,i);
      op2 = Cudd_ReadVars(manager,next);
      
      frame = bdd_and(manager,acc,Cudd_bddXnor(manager,op1,op2));
      Cudd_RecursiveDeref(manager,acc);
      acc = frame;
      
    }
#ifdef IMAGE_TRANSITION
  result = progress(acc,belief);
#else
  cout << "Not implmented for monitor" <<endl;
  exit(0);
#endif
  Cudd_RecursiveDeref(manager,acc);
  Cudd_RecursiveDeref(manager,belief);
  belief = result;
}

void Monitor::invalidate()
{
  cout << "Invalidating" << endl;
  ip = 0;

  //cout << current_analysis->the_plan << endl;

#ifdef PPDDL_PARSER
  
#else
  delete current_analysis->the_plan;
  current_analysis->the_plan=0;
#endif
  //failing = 0;
  count = 0;
  threshold = 0;

  // cout << "Done Invalidating" << endl;  
}

void Monitor::finish()
{
  status = 2; // done, voluntarily

  char *str = "(done)";

  cout << "Terminating with: " << str << endl;

  if(send(sd, str, strlen(str) + 1, 0)<0)
    {
      perror("Monitor.cc: Monitor::succeed(): ");
      close(sd);
      exit(1);
    }
}

void Monitor::plan(char *pfile=0)
{
  cout << "Planning" << endl;

  //cout << "Objects: ";
  //cout << current_analysis->the_problem->objects << endl;
  //current_analysis->the_problem->objects->write(cout);

  if (!pfile) pfile = problem;
  assert(pfile);
  char ofile[80];  sprintf(ofile,"%s.plan",pfile);
  char line[1024]; 
  sprintf(line,"rm -f %s",ofile);
  system(line);

  //  sprintf(line,PLANNER,domain,pfile,ofile,options.c_str());
  cout << line << endl;
  system(line);

#ifdef PPDDL_PARSER
      line_number = 1;
#else
      line_no=1;
#endif

  ifstream ifs(ofile);
  if (ifs.bad())
    {
      cout << "No plan generated" << endl;
      finish();
      return;
    }
  //cout << "parsing plan" << endl;



#ifdef PPDDL_PARSER
  yyrestart(fopen(ofile, "r"));
  yyparse();
  ip = the_plan->begin;
#else
   assert(yfl);
  yfl->switch_streams(&ifs,&cout);
  yyparse();
  
  //cout << "parsed plan" << endl;
 ip = current_analysis->the_plan->begin;
#endif

  if (!ip)
    {
      cout << "Invalid plan" << endl;
      finish();
    }
  else
    {
      ip->load();
      threshold = ip->terminates();
//       if (threshold == -1)
// 	threshold = 0;
//       cout << "Threshold: " << threshold << endl;
    }
  //cout << current_analysis->the_plan;
  // old labels can be reused; delete at end of run of program
  //current_analysis->label_tab.clear();
  //cout << endl << goal_belief << endl << endl;
  //cout << BDDToDNFString(goal_belief) << endl;
}

void Monitor::replan()
{
  invalidate(); // clean up memory

  cout << "Forming problem update" << endl;

  if (failing)
    {
      cout << "Not making any progress..." << endl;
      finish();
      return;
    }

  // this should not be commented; the failing flag is meant to stop
  // multiple calls to replan without a call to exec()
  // if you prefer, you could implement a cutoff above, so that
  // it cuts out when there have been n calls to replan without
  // a call to exec
  failing++;

  episode++;
  // write out an initial state
  // extractDNFfromBDD
  // get a plan
  char pfile[80];

  sprintf(pfile,"%s.%i",problem,episode);

  char line[1024];

  ofstream ofs(pfile);
  try {

//     cout << "(define (problem update) (:domain "
// 	 << dname
// 	 << ")" << endl
// 	 << "(:objects";
//     cout << "Objects: ";
//     cout << current_analysis->the_problem->objects << endl;
//     current_analysis->the_problem->objects->write(cout);
//     cout << ")" << endl;
//     cout << "(:plan-time " << allowed_time <<")" << endl;
//     cout << "(:init" << endl;
//     cout << BDDToDNFString(belief); // without the top level and
//     cout << ")" << endl;
//     cout << "(:goal" << endl;
//     cout << BDDToDNFString(goal_belief) << endl;
//     cout << ")\n)" << endl;
//     cout.flush();
    
    ofs << "(define (problem update) (:domain "
	<< dname
	<< ")\n"
	<< "(:objects";
#ifdef PPDDL_PARSER
     for (Object i = (*my_problem).terms().first_object();
	   i <= (*my_problem).terms().last_object(); i++) {
	ofs << std::endl << "  ";
	(*my_problem).terms().print_term(ofs, i);
	ofs << " - ";
	(*my_problem).domain().types().print_type(ofs, (*my_problem).terms().type(i));
     }
#else
    current_analysis->the_problem->objects->write(ofs);
#endif
    ofs << ")" << endl
	<< "(:plan-time " << allowed_time <<")" << endl
	<< "(:init" << endl
	<< BDDToITEString(belief,0) // without the top level and
	<< ")" << endl
	<< "(:goal" << endl
	<< BDDToITEString(goal_belief)
	<< ")\n)" << endl;
    ofs.flush();
    ofs.close();
  }
  catch (...)
    {
      cout << "Exception when writing new problem" << endl;
      finish();
      return;
    }

  // in lieu of defining a new problem...
  //sprintf(line,"cp %s %s\n",problem,pfile);
  //system(line);
  
  plan(pfile);
}


Monitor::Monitor(char *h,int p,char* dom, char*prob,string &opt) : 
host(h),
port(p),
domain(dom),
problem(prob),
options(opt)
{
  belief = Cudd_ReadLogicZero(manager);
  sd = 0;
  ip = 0;
  status = 0;
  episode = 0;
  failing = 0;
  count = 0;
  threshold = 0;
}

Monitor::~Monitor()
{
  close(sd);
}

int Monitor::start() {

  int rc, i;
  struct sockaddr_in localAddr, servAddr;
  struct hostent *h;


  // sets up next state variables 
#ifdef PPDDL_PARSER
cout << "start"<<endl;
   progress(belief, (std::pair<const Action* const, DdNode*>*)0);

#else
  progress((alt_action *)0,belief);
#endif

  h = gethostbyname(host);
  if(h==NULL) {
    printf("Monitor::start: unknown host '%s'\n",host);
    exit(1);
  }

  servAddr.sin_family = h->h_addrtype;
  memcpy((char *) &servAddr.sin_addr.s_addr, h->h_addr_list[0], h->h_length);
  servAddr.sin_port = htons(port);

  /* create socket */
  sd = socket(AF_INET, SOCK_STREAM, 0);
  if(sd<0) {
    perror("Cannot open socket");
    exit(1);
  }

  /* bind any port number */
  localAddr.sin_family = AF_INET;
  localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  localAddr.sin_port = htons(0);
  
  rc = bind(sd, (struct sockaddr *) &localAddr, sizeof(localAddr));
  if(rc<0) {
    printf("Monitor start: cannot bind port TCP %u\n",port);
    perror("Error");
    exit(1);
  }
				
  /* connect to server */
  rc = connect(sd, (struct sockaddr *) &servAddr, sizeof(servAddr));
  if(rc<0) {
    perror("Monitor.cc, Monitor::start(): ");
    exit(1);
  }

  belief = b_initial_state;
  goal_belief = b_goal_state;

  plan();
  
  while(!status)
    {

      if (!ip /*|| count > (threshold==-1?count:threshold)*/)
	{
	  //cout << "Count: " << count << " Threshold: " << threshold << endl;
	  //cout << "calling replan"<<endl;
	  replan();
	  //ip=ip->exec(this);
	}
      else
	{
	  ip = ip->exec(this);
	}
    }
  
  return status;
}





void mAction::init() { act = 0; }

#ifdef PPDDL_PARSER
void mAction::set(Action *a) { act = a; }
mAction::mAction(Action *a) : act(a), Instruction() {}
#else
void mAction::set(alt_action *a) { act = a; }
mAction::mAction(alt_action *a) : act(a), Instruction() {}
#endif
void mAction::reset() 
{ 
  //delete act; // we don't own actions
  init(); 
}
mAction::mAction() : Instruction() { init(); }
mAction::~mAction() { reset(); }
Instruction * mAction::exec(Monitor *m)
{
  // if m->holds(precondition), else replan
  DdNode *result = m->simulate(act);
  if (result!=Cudd_ReadLogicZero(manager))
    {
      m->exec(act);
      m->assume(result);
      m->wait();
      return next;
    }
  else
    {
      Cudd_RecursiveDeref(manager,result);
      //m->invalidate();
      return 0;
    }
}


void IfThen::init() { pre=Cudd_ReadLogicZero(manager);thn=0;}

void IfThen::set(DdNode *p,Instruction * t) {pre=p; thn=t; }
void IfThen::reset() 
{
  Instruction *tptr=thn;
  Cudd_RecursiveDeref(manager,pre);
  init();
  delete tptr; 
}

IfThen::IfThen(): Instruction() {init();}
IfThen::IfThen(DdNode *p,Instruction* t): Instruction() {set(p,t);}
IfThen::~IfThen() { reset(); }
Instruction * IfThen::exec(Monitor *m)
{
  if (m->holds(pre))
    return thn;
  else
      return next; 
}

void IfThen::load(Instruction *n=0)
{
    Instruction::load(n);
    thn->load(next);
}

int IfThen::terminates()
{
  int t,e;
  switch(terminating)
    {
    case -3:
      terminating = -2;
      t=thn ? thn->terminates() : -1;
      e=next ? next->terminates() : -1;
      
      terminating = min(t==-1?e:t,e==-1?t:e);
      break;
    case -2:
      return -1;
      break;
    }
  return terminating;
}


void IfThenElse::init() { pre=Cudd_ReadLogicZero(manager);thn=0;els=0;}

void IfThenElse::set(DdNode *p,Instruction * t,Instruction *e) {pre=p; thn=t; els=e;}
void IfThenElse::reset() 
{
  Instruction *tptr=thn,*eptr=els;
  Cudd_RecursiveDeref(manager,pre);
  init();
  delete tptr; 
  delete eptr; 
}
IfThenElse::IfThenElse():Instruction() {init();}
IfThenElse::IfThenElse(DdNode *p,Instruction* t,Instruction*e):Instruction() {set(p,t,e);}
IfThenElse::~IfThenElse() { reset(); }
Instruction * IfThenElse::exec(Monitor *m)
{
  if (m->holds(pre))
    return thn;
  else
    return els;
}
void IfThenElse::load(Instruction *n=0)
{
  Instruction::load(n);
  thn->load(next);
  els->load(next);
}

int IfThenElse::terminates()
{
  int t,e;
  switch(terminating)
    {
    case -3:
      terminating = -2;
      t=thn ? thn->terminates() : -1;
      e=els ? els->terminates() : -1;
      
      terminating = min(t==-1?e:t,e==-1?t:e);
      break;
    case -2:
      return -1;
      break;
    }
  return terminating;
}



void Case::init() { elements = 0; }

void Case::reset() {
  Guards::iterator c;
  Instruction *ptr;
    for(c=elements->begin();c!=elements->end();c++)
      {
	Cudd_RecursiveDeref(manager,c->first);
	ptr = c->second;
	c->second = 0;
	delete ptr;
      }
    delete elements; 
    init();
}
void Case::set(Guards *e) { elements = e; }
Case::Case(Guards *e):Instruction() {set(e);}
Case::Case():Instruction() {init();}
Case::~Case() { reset();}
Instruction* Case::exec(Monitor *m)
{
  assert(elements);
  Guards::iterator c;
  for (c=elements->begin();c!=elements->end();c++)
    if (m->holds(c->first))
      return c->second;
  return next;
}
void Case::load(Instruction *n) {
  Instruction::load(n);
  
  Guards::iterator c;
  for(c=elements->begin();c!=elements->end();c++)
    {
      c->second->load(next);
    }
}

int Case::terminates()
{
  Guards::iterator c;
  int m;
  int g;
  switch(terminating)
    {
    case -3:
      terminating = -2;
      m=-1;
      for(c=elements->begin();c!=elements->end();c++)
	{
	  if (c->second)
	    {
	      g = c->second->terminates();
	      m = min(m==-1?g:m,g==-1?m:g);
	    }
	}
      terminating = m;
      break;
    case -2:
      return -1;
      break;
    }
  return terminating;
}

void Goto::init() { label = 0; unreached=0; }

void Goto::set(label_symbol *l) {label = l;}
// a bug here with multiple goto's to labels
void Goto::reset() 
{ 
  //delete label; 
  next = unreached;
  init(); 
}

Goto::Goto(label_symbol * l):Instruction() {set(l);}
Goto::Goto():Instruction() { init(); }
Goto::~Goto() { reset(); }
Instruction* Goto::exec(Monitor *m)
{
  if (!next) return 0;
  if (next->terminates()>=0)
      return next;
  else
    return 0;
}
void Goto::load(Instruction *n)
{
  // if next was set here, it isn't reachable
  // unless someone skips this goto
  // if they do, we still want to leave the block
  // correctly
  Instruction::load(n);
  unreached = next;
  // so now next definately points somewhere;
  // (or this goto is at the bottom of the plan)
  // and we reset next to be the jump
  next = label->instr;
  // but we could make some effort to detect if
  // that step is reachable by marking reference
  // counts or some such silly nonsense
  // the main point in cleaning up memory; the above
  // assignment leaks unreachable plan segments
}
int Goto::terminates()
{
  switch(terminating)
    {
    case -3:
      terminating = -2;
      terminating = next ? next->terminates() : -1;
      break;
    case -2:
      return -1;
      break;
    }
  return terminating;
}

Instruction *Done::exec(Monitor *m)
{
  m->finish();
  return 0;
}

int Done::terminates()
{
  return 0;
}

Instruction *Lcp_Done::exec(Monitor *m)
{
  //  m->setFailing(0);
  //  m->replan();//finish();
  //m->invalidate();
  // m->resetip(); // is dominated by invalidate; the old plan needs
  // to be deleted to avoid memory leaks
  return 0;
}





#ifdef PPDDL_PARSER
Action *findAct(string name)
{
  ostringstream os (ostringstream::out);
  for(std::map<const Action*, DdNode*>::iterator a = action_transitions.begin();
      a != action_transitions.end(); a++){  
    (*a).first->print(os, (*my_problem).terms()); 
   if (os.str() == name)
     return (Action*)(*a).first;
  }
}
#else
alt_action *findAct(string name)
{
  action_list_node *p = available_acts;
  
  while (p)
    {
      if (p->act->get_name() == name)
	return p->act;
      p = p->next;
    }
}
#endif











/* WARNING WARNING WARNING WARNING WARNING WARNING WARNING       */
/* this function is experimental.. I don't know yet if it works  */
/* correctly or not. Use Steven's readline() function to have    */
/* something robust.                                             */
/* WARNING WARNING WARNING WARNING WARNING WARNING WARNING       */

/* rcv_line is my function readline(). Data is read from the socket when */
/* needed, but not byte after bytes. All the received data is read.      */
/* This means only one call to recv(), instead of one call for           */
/* each received byte.                                                   */
/* You can set END_CHAR to whatever means endofline for you. (0x0A is \n)*/
/* read_lin returns the number of bytes returned in line_to_return       */
int read_line(int newSd, char *line_to_return) {
  
  static int rcv_ptr=0;
  static char rcv_msg[MAX_MSG];
  static int n;
  int offset;

  offset=0;

  while(1) {
    if(rcv_ptr==0) {
      /* read data from socket */
      memset(rcv_msg,0x0,MAX_MSG); /* init buffer */
      n = recv(newSd, rcv_msg, MAX_MSG, 0); /* wait for data */
      if (n<0) {
	perror("Cannot receive data\n");
	return ERROR;
      } else if (n==0) {
	printf("Connection closed by client\n");
	close(newSd);
	return ERROR;
      }
    }
  
    /* if new data read on socket */
    /* OR */
    /* if another line is still in buffer */

    /* copy line into 'line_to_return' */
    while(*(rcv_msg+rcv_ptr)!=END_LINE && rcv_ptr<n) {
      memcpy(line_to_return+offset,rcv_msg+rcv_ptr,1);
      offset++;
      rcv_ptr++;
    }
    
    /* end of line + end of buffer => return line */
    if(rcv_ptr==n-1) { 
      /* set last byte to END_LINE */
      *(line_to_return+offset)=END_LINE;
      rcv_ptr=0;
      return ++offset;
    } 
    
    /* end of line but still some data in buffer => return line */
    if(rcv_ptr <n-1) {
      /* set last byte to END_LINE */
      *(line_to_return+offset)=END_LINE;
      rcv_ptr++;
      return ++offset;
    }

    /* end of buffer but line is not ended => */
    /*  wait for more data to arrive on socket */
    if(rcv_ptr == n) {
      rcv_ptr = 0;
    } 
    
  } /* while */
}

string BDDToCNFString(DdNode* dd,int withAnd=1){
  ostringstream returnString (ostringstream::out);

  int j = 0;
  DdNode **care=new DdNode*[num_alt_facts];
  int *indices=new int[num_alt_facts];

 if (withAnd)   returnString << "(and " <<endl;
  for(int i = 0; i < num_alt_facts; i++){
    if (!Cudd_SupportIndex(manager,dd)[i])
      {
	continue;
      }

    if(Cudd_bddIsVarEssential(manager,dd,i,1))
      {
	//cout << alt_facts[i]->get_name() << " is true " << endl;
#ifdef PPDDL_PARSER
	dynamic_atoms[i]->print(returnString, 
				(*my_problem).domain().predicates(), 
				(*my_problem).domain().functions(),
				(*my_problem).terms());
	returnString << endl;
#else
	returnString << alt_facts[i]->get_name() <<endl;
#endif
      }
    else if(Cudd_bddIsVarEssential(manager,dd,i,0))
      {
	//cout << alt_facts[i]->get_name() << " is false " << endl;
#ifdef PPDDL_PARSER
	returnString << "(not ";
	dynamic_atoms[i]->print(returnString, 
				(*my_problem).domain().predicates(), 
				(*my_problem).domain().functions(),
				(*my_problem).terms());
	returnString << ")" << endl;
#else
	returnString << "(not " << alt_facts[i]->get_name() << ")" <<endl;
#endif
      }
    else
      {
	//cout << alt_facts[i]->get_name() << " is neither true nor false nor inessential " << endl;
	indices[j]=i;
	care[j] = Cudd_bddIthVar(manager,i);
	j++;
      }
  }
  
  if (j)
    {
      DdNode** minterms;
      int sizeofSupport = Cudd_SupportSize(manager, dd);
      int sizeofResult = Cudd_CountMinterm(manager, dd, sizeofSupport);
      int num = j;
      int size = min(1<<num,sizeofResult);
      minterms = new DdNode*[size+1];
      memset(minterms,0,sizeof(DdNode*)*(size+1));
      Cudd_PickAllTerms(manager,dd,care,num,minterms,size);
      
      //cout << "Escaped from PickAllTerms. " << size << endl;
      
      returnString << "(or " <<endl;
      j = 0;
      while(minterms[j]){                

	returnString << "(and " << endl;
	for(int i = 0; i < num; i++){
	  
	  if(Cudd_bddIsVarEssential(manager,minterms[j],indices[i],1))
	    {
#ifdef PPDDL_PARSER
	      dynamic_atoms[i]->print(returnString, 
				      (*my_problem).domain().predicates(), 
				      (*my_problem).domain().functions(),
				      (*my_problem).terms());
	      returnString << endl;
#else
	      returnString << alt_facts[i]->get_name() <<endl;
#endif
	    }
	  else if(Cudd_bddIsVarEssential(manager,minterms[j],indices[i],0))
	    {
	      //cout << alt_facts[i]->get_name() << " is false " << endl;
#ifdef PPDDL_PARSER
	      returnString << "(not ";
	      dynamic_atoms[i]->print(returnString, 
				      (*my_problem).domain().predicates(), 
				      (*my_problem).domain().functions(),
				      (*my_problem).terms());
	      returnString << ")" << endl;
#else
	      returnString << "(not " << alt_facts[i]->get_name() << ")" <<endl;
#endif
	      
	    }
	}
	returnString << ")" <<endl;    
	j++;
      }
      returnString << ")" <<endl;    

      delete minterms;
    }

  if(withAnd)      returnString << ")" <<endl;

  delete care;
  delete indices;
  
  return returnString.str();
}

string BDDToDNFString(DdNode* dd){
  ostringstream returnString (ostringstream::out);
  assert(dd);

  DdNode** minterms;
  int sizeofSupport = Cudd_SupportSize(manager, dd);
  DdNode** support = new DdNode*[sizeofSupport];
  int j=0;
  for(int i=0; i<num_alt_facts;i++)
    if(Cudd_SupportIndex(manager,dd)[i])
      {
	//cout << i << "th var" << endl;
	support[j] = Cudd_bddIthVar(manager,i);
	j++;
      }
  int size = Cudd_CountMinterm(manager, dd, sizeofSupport);
  
  minterms = new DdNode*[size+1];
  memset(minterms,0,sizeof(DdNode*)*(size+1));

  //cout << "Enter PickAllTerms. " << sizeofSupport << " " << size << " " << endl;

  Cudd_PickAllTerms(manager,dd,support,sizeofSupport,minterms,size);
  
  //cout << "Escaped from PickAllTerms. " << size << endl;
  
  returnString << "(or " <<endl;
  j = 0;
  while(minterms[j]){                
    
    returnString << "(and " << endl;
    for(int i = 0; i < num_alt_facts; i++){
      
      if(Cudd_bddIsVarEssential(manager,minterms[j],i,1))
	{
#ifdef PPDDL_PARSER
	  dynamic_atoms[i]->print(returnString, 
				  (*my_problem).domain().predicates(), 
				  (*my_problem).domain().functions(),
				  (*my_problem).terms());
	  returnString << endl;
#else
	  returnString << alt_facts[i]->get_name() <<endl;
#endif
	}
      else if(Cudd_bddIsVarEssential(manager,minterms[j],i,0))
	{
	  //cout << alt_facts[i]->get_name() << " is false " << endl;
#ifdef PPDDL_PARSER
	  returnString << "(not ";
	  dynamic_atoms[i]->print(returnString, 
				      (*my_problem).domain().predicates(), 
				  (*my_problem).domain().functions(),
				  (*my_problem).terms());
	  returnString << ")" << endl;
#else
	  returnString << "(not " << alt_facts[i]->get_name() << ")" <<endl;
#endif
	}
    }
    returnString << ")" <<endl;    
    j++;
  }
  returnString << ")" <<endl;    
  
  delete minterms;
  
  return returnString.str();
}

// doesn't work right
string BDDToITEStringR(DdNode* dd,string ret,int var,int num);
string BDDToITEString(DdNode* dd){
  if(my_problem->domain().requirements.probabilistic){
    DdNode *tmp = Cudd_addBddStrictThreshold(manager, dd, 0.0);
    Cudd_Ref(tmp);
    string s =  BDDToITEString(tmp,1);    
    Cudd_RecursiveDeref(manager, tmp);
    return s;
  }
  else{
    return BDDToITEString(dd,1);    
  }

}
string BDDToITEString(DdNode* dd,int withAnd=1)
{
  if (withAnd)
    return BDDToITEStringR(dd,"",0,0);
  else
    return BDDToITEStringR(dd,"",0,-num_alt_facts);
  // i.e., even if we have a complete initial state,
  // it won't be enough to earn an "and"
}

string BDDToITEStringR(DdNode* dd,string in,int var,int num){
  string out;



  if (var >= num_alt_facts)
    if (num > 1)
      return "(and "+in+")";
    else
      return in;
  if (!Cudd_SupportIndex(manager,dd)[2*var])
    return BDDToITEStringR(dd,in,var+1,num);
  
#ifdef PPDDL_PARSER
  ostringstream os (ostringstream::out);
 dynamic_atoms[var]->print(os, 
			  (*my_problem).domain().predicates(), 
			  (*my_problem).domain().functions(),
			  (*my_problem).terms()); 
 string name = os.str();
 
#else
  string name = alt_facts[var]->get_name();
#endif
  DdNode *t;
  DdNode *e;
  if(Cudd_bddIsVarEssential(manager,dd,2*var,1))
    {
      //t = bdd_and(manager,dd,Cudd_bddIthVar(manager,var));
      //cout << alt_facts[i]->get_name() << " is true " << endl;
      out= BDDToITEStringR(dd,in + name + " ",var+1,num+1);
      //Cudd_RecursiveDeref(manager,t);
    }
  else if(Cudd_bddIsVarEssential(manager,dd,2*var,0))
    {
      //e = bdd_and(manager,dd,Cudd_Not(Cudd_bddIthVar(manager,var)));
      //cout << alt_facts[i]->get_name() << " is false " << endl;
      out = BDDToITEStringR(dd,in + "(not " + name + ") ",var+1,num+1);
      //Cudd_RecursiveDeref(manager,e);
    }
  else
    {
      t = bdd_and(manager,dd,Cudd_bddIthVar(manager,2*var));
      e = bdd_and(manager,dd,Cudd_Not(Cudd_bddIthVar(manager,2*var)));	

      // resolution immediately simplifies the following

//       out = in+"(or \n"+
// 	BDDToITEStringR(t,name + "\n",var+1,1)+
// 	BDDToITEStringR(e,"(not "+name+")\n",var+1,1)+")\n";
//       if (num>0)
// 	out = "(and \n"+out+")\n";

      out = in+"(or "+
	BDDToITEStringR(t,name + " ",var+1,1)+
	BDDToITEStringR(e,"",var+1,0)+") ";
      if (num>0)
	out = "(and "+out+") ";
      

      Cudd_RecursiveDeref(manager,t);
      Cudd_RecursiveDeref(manager,e);
    }
  return out;
}
  





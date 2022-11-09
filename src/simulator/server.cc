/* fpont 1/00 */
/* pont.net    */
/* tcpServer.c */

#include <sstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h> /* close */
#include "server.h"
#include <stdlib.h>
#include <string.h>
#include "globals.h"
#include "graph_wrapper.h"
#include "lao_wrapper.h"
#include "monitor.h"
#ifdef PPDDL_PARSER
#else
#include "actions/actions.h"
#endif
extern goal* factsFromDD(DdNode* dd);

using namespace std;


int server::start() {
  int sd, newSd, cliLen;
  struct sockaddr_in cliAddr, servAddr;

  /* create socket */
  sd = socket(AF_INET, SOCK_STREAM, 0);
   if(sd<0) {
    perror("cannot open socket ");
    return ERROR;
  }
  
  /* bind server port */
  servAddr.sin_family = AF_INET;
  servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servAddr.sin_port = htons(port);
  
  if(bind(sd, (struct sockaddr *) &servAddr, sizeof(servAddr))<0) {
    perror("cannot bind port ");
    return ERROR;
  }
  
  int ret = -1;
  do {
      listen(sd,5);

      printf(": waiting for data on port TCP %u\n",port);

      cliLen = sizeof(cliAddr);
      newSd = accept(sd, (struct sockaddr *) &cliAddr, (socklen_t*)&cliLen);
      if(newSd<0) {
	perror("cannot accept connection ");
	continue;
      }
      
      if(session(newSd, cliAddr)){
	printf("\n\nSUCCESS\n\n");
	ret = 1;
      }
      else{
	printf("\n\nFAILURE\n\n");
	ret = 0;
      }
    } while (FOREVER);

  return ret;
}

DdNode* server::pickState(DdNode* bs){
//   static int flip=0;
  DdNode* return_state = Cudd_ReadOne(manager);
  Cudd_Ref(return_state);
  DdNode *tmpD,*tmp;

  cout << "Forcing Deterministic Transition" << endl;

  if (bs == Cudd_ReadLogicZero(manager))
    return bs;

  for(int i = 0; i < num_alt_facts; i++){
    if(Cudd_bddIsVarEssential(manager,bs,i,1))
      {
// 	if (flip)
// 	  // flip the state!
// 	  return_state = bdd_and(manager, tmpD=return_state, Cudd_Not(Cudd_bddIthVar(manager, i)));
// 	else
	  return_state = bdd_and(manager, tmpD=return_state, Cudd_bddIthVar(manager, i));
      }
    else if(Cudd_bddIsVarEssential(manager,bs,i,0))
      {
// 	if (flip)
// 	  // flip the state!
// 	  return_state = bdd_and(manager, tmpD=return_state, Cudd_bddIthVar(manager, i));
// 	else
	  return_state = bdd_and(manager, tmpD=return_state, Cudd_Not(Cudd_bddIthVar(manager, i)));
      }
    // not all false; is satisfiable (with positive value)
    else if(!bdd_is_zero(manager, tmpD=bdd_and(manager, bs, tmp = bdd_and(manager, return_state, Cudd_bddIthVar(manager, i))))) //can be random
      {
	Cudd_RecursiveDeref(manager,tmpD);
	tmpD=return_state;
	return_state = tmp;
      }
    // not all false, is satisfiable (with negative value)
    else if(!bdd_is_zero(manager, tmpD=bdd_and(manager, bs, tmp = bdd_and(manager, return_state, Cudd_Not(Cudd_bddIthVar(manager, i))))))
      {
	Cudd_RecursiveDeref(manager,tmpD);
	tmpD=return_state;
	return_state = tmp;
      }
    else
      {
	
	assert("Already Inconsistent?" == 0);
	Cudd_RecursiveDeref(manager,return_state);
	Cudd_RecursiveDeref(manager,tmpD);
	return Cudd_ReadLogicZero(manager);
	
      }
    Cudd_RecursiveDeref(manager,tmpD);
  }
  Cudd_Ref(return_state);
  // flip = 1;

  return return_state;
}

DdNode* server::getState()
{
  return current_state;
}

void server::setState(DdNode* state) 
{ 
  cout << "Setting State" << endl;
  if (current_state)
    Cudd_RecursiveDeref(manager,current_state);
  current_state = state;
}

DdNode* server::updateState(char* line){
  DdNode* return_state, *tmp;
#ifdef PPDDL_PARSER
  Action * act;
#else
  alt_action* act;
 alt_effect* tmpEff; 
#endif
 ostringstream o (ostringstream::out);
 
  const char * tmpc;

  cout << "Updating State" << endl;

  act = findAct(line);

  if (!act)
    {
      cout << "Got Null Action" << endl;
      return Cudd_ReadLogicZero(manager);
    }

  cout << "Got Action " << line;//act->get_name();

  memset(line,0x0,MAX_MSG);
#ifdef PPDDL_PARSER
  /* DONT SUPPORT OBSERVATIONS YET */
//   if(tmpEff->obs){
//         cout << " and it is sensory " << endl;
//     while(tmpEff){
//      tmp = bdd_and(manager, current_state, tmpEff->b_eff);
//      if(!bdd_is_zero(manager, tmp)){
//        Cudd_RecursiveDeref(manager,tmp);
//        factsFromDD(tmpEff->b_eff)->write(o);
//        tmpc = o.str().c_str();
//        memcpy(line, tmpc, strlen(tmpc));      
//        break;
//      }    
//      Cudd_RecursiveDeref(manager,tmp);
//      tmpEff = tmpEff->next;
//     }
//     return_state = current_state;
//     Cudd_Ref(return_state);
//   }
//   else{
    cout << " and it is causative " << endl;
    return_state = pickState(tmp=progress(action_transitions[act], current_state));
    Cudd_RecursiveDeref(manager,tmp);
#else
  tmpEff = act->get_effs();
  if(tmpEff->obs){
        cout << " and it is sensory " << endl;
    while(tmpEff){
     tmp = bdd_and(manager, current_state, tmpEff->b_eff);
     if(!bdd_is_zero(manager, tmp)){
       Cudd_RecursiveDeref(manager,tmp);
       factsFromDD(tmpEff->b_eff)->write(o);
       tmpc = o.str().c_str();
       memcpy(line, tmpc, strlen(tmpc));      
       break;
     }    
     Cudd_RecursiveDeref(manager,tmp);
     tmpEff = tmpEff->next;
    }
    return_state = current_state;
    Cudd_Ref(return_state);
  }
  else{
    cout << " and it is causative " << endl;
    return_state = pickState(tmp=progress(act, current_state));

    //
    //
    // Sending extra observations; ie: failures 
    //
    // Take the BDD for the exo action effect, progress it
    // using the code in Monitor::force (build the image)
    //
    // Store a string representation below
    // 
    //

    //string obs = BDDToCNFString(return_state);
    //memcpy(line,obs.c_str(),obs.length());

    Cudd_RecursiveDeref(manager,tmp);
  }
#endif

  if (line[0] == 0)
    memcpy(line, "()", sizeof("()"));

  // Server uses same progression function
  // so when preconditions aren't satisfied, we go to FALSE
  //
  // this behaivour could be modified to write out the
  // status of the precondition formula using the above remarks
  // about exo actions, and treat failing actions
  // as no-ops, or something more devious
  // (restarts/teleports/backups/more goals...)
  //
  // but the code above for return_state = progression
  // would need to be tweaked as appropriate
  //

  if (return_state == Cudd_ReadLogicZero(manager))
    memcpy(line,"(done)",sizeof("(done)"));

  return return_state;
}

int server::session(int sd, struct sockaddr_in cliAddr){
  char line[MAX_MSG];

  //printBDD(b_initial_state);
  setState(pickState(b_initial_state));
  printBDD(getState());


     /* receive segments */
  memset(line,0x0,MAX_MSG);
    
  while(read_line(sd,line)>0 && strcmp(line, "(done)")){    
      printf("\nReceived from %s:TCP%d : %s\n", 
	     inet_ntoa(cliAddr.sin_addr),
	     ntohs(cliAddr.sin_port), line);
      setState(updateState((char*)&line));
      printBDD(getState());

      // server can also send done to forcibly end business

      cout << "Sending: " << line <<endl;
      send(sd, line, strlen(line)+1,0);

      /* init line */
      memset(line,0x0,MAX_MSG);

  } /* while(read_line) */

  // die
  if (getState() == Cudd_ReadLogicZero(manager))
    return 0;

  // succeed
  if(bdd_entailed(manager, getState(), b_goal_state))
    {
      printBDD(b_goal_state);
      return 1;
    }
  
  // survive
  return 0;
}


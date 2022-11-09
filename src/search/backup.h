/* backup.h */
#ifndef __BACKUP_H
#define __BACKUP_H

//double Backup(struct StateNode *state);
///#define CCMAX 1;
//#define CCLAO 2;

//void checkSolved(int NumExpandedStates);
//int allSolved( struct StateDistribution *nextState);
//int allSuccessorsSolved( struct StateDistribution *nextState, BitVector* solved_visited);
//int checkForUnsolved(struct StateNode* state, BitVector* solved_visited);
double Backup(struct StateNode *state, int currHorizon);


#endif

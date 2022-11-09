/* backup.h */
#ifndef __MOBACKUP_H
#define __MOBACKUP_H

//double Backup(struct StateNode *state);
///#define CCMAX 1;
//#define CCLAO 2;

//void checkSolved(int NumExpandedStates);
//int allSolved( struct StateDistribution *nextState);
//int allSuccessorsSolved( struct StateDistribution *nextState, BitVector* solved_visited);
//int checkForUnsolved(struct StateNode* state, BitVector* solved_visited);
double MOBackup(struct StateNode *state, int currHorizon);
void removeDominatedPoints(struct StateNode *state, MOValueFunction *stateMOValueFunction);
double computeHyperVolume(MOValueFunction *valFn, bool useHPoints);
bool checkDominance(MOValueFunctionPoint* p1,
		MOValueFunction *stateMOValueFunction,
		struct StateNode* state,
		MOValueFunction* dominatedPoints);
#endif




/*********************************************************************
 * File: exclusions.h
 * Description: headers of routines for calculating exclusion relations
 *
 * Author: Joerg Hoffmann 1998
 *
 *********************************************************************/ 
/*********************************************************************
 * (C) Copyright 1998 Albert Ludwigs University Freiburg
 *     Institute of Computer Science
 *
 * All rights reserved. Use of this software is permitted for 
 * non-commercial research purposes, and it may be copied only 
 * for that use.  All copies must include this copyright message.
 * This software is made available AS IS, and neither the authors
 * nor the  Albert Ludwigs University Freiburg make any warranty
 * about the software or its performance. 
 *********************************************************************/




#ifndef _EXCLUSIONS_H
#define _EXCLUSIONS_H







DdNode* ARE_MUTEX_OPS( int time, OpNode *o1, OpNode *o2, DdNode* labela, DdNode* labelb );
DdNode* ARE_MUTEX_FTS( int time, FtNode *f1, FtNode *f2 , DdNode* labela, DdNode* labelb);
DdNode* ARE_MUTEX_EFS( int time, EfNode *f1, EfNode *f2 , DdNode* labela, DdNode* labelb);
void SET_ADDERS( int time, FtNode *ft ) ;


void find_mutex_ops( int time );
void find_mutex_facts( int time );
void find_mutex_effects(int time);
void find_mutex_effects1(int time);
DdNode* add_worlds_to_mutex(DdNode* mutex, DdNode* newMutex);
//int
DdNode* ef_competing_needs( int time, EfNode *o1, EfNode *o2, DdNode* labela, DdNode* labelb );
//int 
DdNode *competing_needs( int time, OpNode *o1, OpNode *o2, DdNode* labela, DdNode* labelb );
int ef_interfere( EfNode *i1, EfNode *i2 );
int interfere( OpNode *i1, OpNode *i2 );
int noop_exclusive( OpNode *i1, OpNode *i2 );
DdNode* facts_are_exclusive( int time, FtNode *f1, FtNode *f2, DdNode* l1, DdNode* l2 );

int are_facts_mutex( FtNode* ft1, FtNode* ft2, DdNode* l1, DdNode* l2,int time);
int are_ops_mutex(OpNode* op1, OpNode* op2, DdNode* l1, DdNode* l2, int time);
int are_efs_mutex(EfNode* op1, EfNode* op2, DdNode* l1, DdNode* l2, int time);

DdNode* removeMutexWorlds(DdNode* origWorlds, DdNode *f, int time);
#endif /* _EXCLUSIONS_H */

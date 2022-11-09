



/*********************************************************************
 * File: memory.h
 * Description: Creation / Deletion functions for all data structures.
 *
 * Author: Joerg Hoffmann / Frank Rittinger
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



#ifndef _MEMORY_H
#define _MEMORY_H



#include "ipp.h"

#ifdef PPDDL_PARSER
#else
#include "../parser/ptree.h"
#endif

void printMemInfo();


char *new_token( int len );
TokenList *new_token_list( void );
FactList *new_fact_list( void );
TypedList *new_TypedList( void );
TypedListList *new_TypedListList( void );
//PlNode *new_pl_node(Connective c);

//CodeNode *new_CodeNode( Connective c ); // for compilation purposes only

//PlOperator *new_pl_operator( const char *name );
//CodeOperator *new_CodeOperator( void );
//PlOperator *new_axiom_op_list( void );
OpNode *new_op_node( int time, const char *name, Bool is_noop,
		     BitVector *pos_precond_vector,
		     BitVector *neg_precond_vector,
		     DdNode* pre, int);
EfNode *new_ef_node( int time, OpNode *op, 
		    /*  BitVector *pos_effect_vector, */
/* 		     BitVector *neg_effect_vector  */ Effect* eff, int ind);


//FtNode *new_ft_node( int time, int index, Bool positive, Bool dummy, goal* label );


FtNode *new_ft_node( int time, int index, Bool positive, Bool dummy, /* DdNode */Label* label );  
OpEdge *new_op_edge( OpNode *op );
EfEdge *new_ef_edge( EfNode *ef );
FtEdge *new_ft_edge( FtNode *ft );
OpLevelInfo *new_op_level_info( void );
EfLevelInfo *new_ef_level_info( int time );
FtLevelInfo *new_ft_level_info( FtNode *ft );
OpPair *new_op_pair( OpNode *o1, OpNode *o2 );
EfPair *new_ef_pair( EfNode *o1, EfNode *o2 );
FtPair *new_ft_pair( FtNode *f1, FtNode *f2 );





MemoNode *new_memo_node( int double_index, int way );
Label * new_Label(DdNode* lab, double csd);
void free_Label(Label* lab);
Consequent * new_Consequent(void);
FtExclusion* new_FtExclusion(void);
EfExclusion* new_EfExclusion(int length) ;
OpExclusion* new_OpExclusion(int length);




MemoNode *new_memo_node( int double_index, int way );
MemoNode_table *new_memo_node_table( void );
Candidate *new_candidate( void );
//RelevantFact *new_RelevantFact( CodeNode *n );

// All, compiling
BitOperator *new_BitOperator( const char *name );
Effect *new_Effect(int );
FactInfo *new_FactInfo( void );
BitVector *new_bit_vector(int length);

BitVector *new_excl_bit_vector(int length);

//All compiling
Effect *new_effect();
Integers *new_integers(int i);

FactInfo *new_fact_info();

FactInfoPair *new_fact_info_pair(FactInfo *pos, FactInfo *neg); // compiling

ExclusionLabelPair* new_ExclusionLabelPair(DdNode* l1, DdNode* l2, int);
FtExclusion* new_FtExclusion(void);
EfExclusion* new_EfExclusion(int);
OpExclusion* new_OpExclusion(int);

EfExclusion* set_Labels_EfExclusion(EfExclusion*,int length); // compiling



void free_token_list(TokenList * tl);
void free_fact_list(FactList * list);
//void free_tree(PlNode * node);

void free_token_list(TokenList *tl);
//void free_tree(PlNode *node);
//void free_CodeNode( CodeNode *node );
//void free_CodeOperator( CodeOperator *arme_sau );
//void free_pl_node(PlNode *node);
void free_fact_list(FactList *list);
void free_complete_token_list(TokenList *source );
void free_complete_fact_list(FactList *source );

//void free_pl_op(PlOperator * op);
//void free_ops(PlOperator *op);
void free_integers( Integers *l, int );
void free_partial_effect( Effect *ef );
void free_partial_operator( BitOperator *op );

void free_fact_info_pair( FactInfoPair *p ); // compiling


void free_bit_vector(BitVector * bvec);

void free_fact_info(FactInfo *info);
//void free_pl_op(PlOperator *op);

void free_effect(Effect * effect); // compiling
void free_BitOperator( BitOperator *op ); // compiling

void free_complete_FactList( FactList *source );
void free_complete_TokenList( TokenList *source );
void free_TypedList( TypedList *t );
void free_TypedListList( TypedListList *t );

void free_op_node( OpNode *op );
void free_Effect(Effect *ef);
void free_Antecedent(Antecedent* ant);
void free_Consequent(Consequent* cons);
void free_ef_node( EfNode *ef );
void free_ft_node( FtNode *ft );
void free_memo_node( MemoNode *m );
void free_op_level_info( OpLevelInfo *i );
void free_ef_level_info( EfLevelInfo *i );
void free_ft_level_info( FtLevelInfo *i );
void free_op_edge( OpEdge *e );
void free_ef_edge( EfEdge *e );
void free_ft_edge( FtEdge *e );
void free_op_pair( OpPair *p );
void free_ef_pair( EfPair *p );
void free_ft_pair( FtPair *p );
void free_candidate( Candidate *c );
void free_op_exclusion(OpExclusion *oe);
void free_ef_exclusion(EfExclusion *oe);
void free_ft_exclusion(FtExclusion *oe);
#endif /* _MEMORY_H */


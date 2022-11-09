


/*********************************************************************
 * File: build_graph.h
 * Description: headers for building the graph
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




#ifndef _BUILD_GRAPH_H
#define _BUILD_GRAPH_H

//DAN
//also changed all Bools to ints
#include "ipp.h"

extern FactInfoPair *gbit_goal_state;
extern FactInfoPair *gbit_initial_state;
extern BitOperator *gbit_operators;



//NAD

int add_samples(DdNode* newSamples);
void mark_operator_labels(int time);
void mark_effect_lables(int time);
void mark_fact_labels(int time);
void update_structure(int time);
void update_actions_effects_facts(int time);
void un_update_actions_effects_facts(int time);



//int build_graph( int *min_time );


//void build_graph_evolution_step( void );

DdNode* unionNDLabels(EfLevelInfo_pointer ef);

void apply_noops( int time );
int apply_operator( int time, BitOperator *op );
int apply_all_effects( int time, OpNode *op_node ); 
int apply_effect( int time, Effect *ef, OpNode *op_node );


void insert_potential_effects( int time );
int apply_potential_effects( int time, OpNode *op_node, int *hit );
int apply_potential_effect( int time, Effect *ef, OpNode *op_node );
int potential_applicable( int time, Effect *ef, OpNode *op_node );
void integrate_potential_effects( int time );


//int are_there_non_exclusive( int time, FactInfo *pos, FactInfo *neg );
int get_them_non_exclusive( int time,
			    FactInfo *pos, 
			    FactInfo *neg,
			    BitVector **pos_exclusives, 
			    BitVector **neg_exclusives,
			    //ExclusionLabelPair 
			    DdNode ***p_exlabels,
			    //ExclusionLabelPair
			    DdNode ***n_exlabels);


void insert_op_edge( OpEdge **l, OpNode *op );
void insert_ft_edge( FtEdge **l, FtNode *ft );
void insert_ef_edge( EfEdge **l, EfNode *ef );



//extern void free_graph_and_search_info( void );
void free_op_node( OpNode *op );
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
void free_ft_pair( FtPair *p );
void free_candidate( Candidate *c );




#endif /* _BUILD_GRAPH_H */


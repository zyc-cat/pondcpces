#ifndef KGRAPHINFO_H
#define KGRAPHINFO_H

#include "ipp.h"
#include "relaxedPlan.h"

class kGraphInfo {

 public:
  int pos_fact_at_min_level[MAX_RELEVANT_FACTS];
  int neg_fact_at_min_level[MAX_RELEVANT_FACTS];
  unsigned int* pos_facts_vector_at[IPP_MAX_PLAN]; 
  unsigned int* neg_facts_vector_at[IPP_MAX_PLAN]; 
  unsigned int op_vector_length_at[IPP_MAX_PLAN]; 
  FtNode *all_fts_pointer;
  int num_levels;
  FtNode_pointer* graph;
  RelaxedPlan* relaxed_plan;
  FtPair* ft_mutex_pairs;
  OpNode* gall_ops;
  EfNode* gall_efs;
  DdNode* initial;
  kGraphInfo(FtNode_pointer* g, 
	     int levs, 
	     FtPair* mux, 
	     FtNode* fts, 
	     OpNode* ops,
	     EfNode* efs,
	     DdNode* init) : 
    graph(g),
    num_levels(levs), 
    relaxed_plan(0),
    ft_mutex_pairs(mux),
    all_fts_pointer(fts),
    gall_ops(ops),
    gall_efs(efs),
    initial(init)
    
    {};
};

#endif

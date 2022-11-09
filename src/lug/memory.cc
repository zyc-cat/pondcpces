



/*********************************************************************
 * File: memory.c
 * Description: Creation functions for all data structures.
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



#include "ipp.h"
#include "utilities.h"
#include "memory.h"
//#include "pddl.h"



#include "globals.h"
#include "graph_wrapper.h"
#include "cudd.h"

extern int num_alt_facts;
#define MS_REGULAR 351
#define MS_NONE 352
extern int MUTEX_SCHEME;
extern int gop_vector_length;
extern int USESENSORS;
extern int SENSORS;
extern int num_alt_acts;

int num_tokens = 0;
int num_token_list = 0;
int num_fact_list = 0;
int num_typedlist= 0;
int num_typedlistlist= 0;
int num_pl_node= 0;
int num_codenode= 0;
int num_pl_operator= 0;
int num_codeoperator= 0;
int num_axiom_op_list= 0;
int num_op_node= 0;
int num_ef_node= 0;
int num_ft_node= 0;
int num_ft_edge= 0;
int num_ef_edge= 0;
int num_op_edge= 0;
int num_op_level_info= 0;
int num_ef_level_info= 0;
int num_ft_level_info= 0;
int num_op_pair= 0;
int num_ft_pair= 0;
int num_memo_node= 0;
int num_memo_node_table= 0;
int num_candidate= 0;
int num_relevantfact= 0;
int num_BitOperator= 0;
int num_label= 0;
int num_antecedent= 0;
int num_effect= 0;
int num_consequent= 0;
int num_factinfo= 0;
int num_bitvector= 0;
int num_excl_bit_vector= 0;
int num_effect1= 0;
int num_integers= 0;
int num_fact_info= 0;
int num_fact_info_pair= 0;
int num_exclusionlabelpair= 0;
int num_ftexclusion= 0;
int num_efexclusion= 0;
int num_opexclusion= 0;

/**********************************************************************
 ********************** CREATION FUNCTIONS ****************************
 *********************************************************************/

/**********************************************************************
 * Allocates memory for a new string with length len.
 *
 * int len: length of string to allocate INCLUDING the '\0'
 * RETURNS a pointer to the allocated memory
 *********************************************************************/
char *
new_token(int len)
{
  char * tok = (char*) calloc(len, sizeof(char));
  num_tokens++;
  CHECK_PTR(tok);
#ifdef MEMORY_INFO
  gmemory += len * sizeof(char);
#endif
  return tok;
}

/**********************************************************************
 * Allocates new memory for a list of strings and initializes
 * all members to NULL.
 *
 * RETURNS a pointer to the allocated memory
 *********************************************************************/
TokenList *
new_token_list(void)
{
  TokenList * result = (TokenList *) malloc(sizeof(TokenList));
  num_token_list++;
  CHECK_PTR(result);
#ifdef MEMORY_INFO
  gmemory += sizeof(TokenList);
#endif
  result->item = NULL; 
  result->next = NULL;
  return result;
}

/**********************************************************************
 * Allocates new memory for a list of lists of strings and 
 * initializes all members to NULL.
 *
 * RETURNS a pointer to the allocated memory
 *********************************************************************/
FactList * 
new_fact_list(void)
{
  FactList * result = (FactList *) malloc(sizeof(FactList));
  num_fact_list++;
  CHECK_PTR(result);
#ifdef MEMORY_INFO
  gmemory += sizeof(FactList);
#endif
  result->item = NULL; 
  result->next = NULL;
  return result;
}



TypedList *new_TypedList( void )

{

  TypedList *result = ( TypedList * ) calloc( 1, sizeof( TypedList ) );
  CHECK_PTR(result);
  num_typedlist++;
  result->name = NULL; 
  result->type = NULL;
  result->n = -1;

  return result;

}



TypedListList *new_TypedListList( void )

{

  TypedListList *result = ( TypedListList * ) calloc( 1, sizeof( TypedListList ) );
  CHECK_PTR(result);
  num_typedlistlist++;

  result->predicate = NULL; 
  result->args = NULL;

  return result;

}



/**********************************************************************
 * Allocates new memory for a PL1 tree node.
 *
 * Connective c: The type of node, see pddl.h for a full description.
 * RETURNS a pointer to the allocated memory
 *********************************************************************/
// PlNode *
// new_pl_node(Connective c)
// {
//   PlNode * result = (PlNode *) malloc(sizeof(PlNode));
//   CHECK_PTR(result);
//   num_pl_node++;
// #ifdef MEMORY_INFO
//   gmemory += sizeof(PlNode);
// #endif
//   result->connective = c;
//   result->atom = NULL;
//   result->sons = NULL;
//   result->next = NULL;
//   return result;
// }

// CodeNode *new_CodeNode( Connective c )

// {

//   CodeNode *result = ( CodeNode * ) calloc( 1, sizeof( CodeNode ) );
//   CHECK_PTR(result);
//   num_codenode++;
// #ifdef MEMORY_INFO
//   gmemory += sizeof( CodeNode );
// #endif

//   result->connective = c;

//   result->var = 0;
//   result->var_type = 0;
//   result->predicate = 0;

//   result->visited = FALSE;

//   result->sons = NULL;
//   result->next = NULL;

//   return result;

// }

// /**********************************************************************
//  * Allocates new memory for a PL1 style operator.
//  *
//  * char * name: The name of the operator, memory will be allocated.
//  * RETURNS a pointer to the allocated memory
//  *********************************************************************/
// PlOperator *
// new_pl_operator(const char * name)
// {
//   PlOperator * result = (PlOperator *) malloc(sizeof(PlOperator));
//   CHECK_PTR(result);
//   num_pl_operator++;
// #ifdef MEMORY_INFO
//   gmemory += sizeof(PlOperator);
// #endif
//   if (NULL != name)
//     {
//       result->name = new_token_list();
//       result->name->item = new_token(strlen(name)+1);
//       CHECK_PTR(result->name->item);
//       strcpy(result->name->item, name);
//     }
//   else
//     {
//       result->name = NULL;
//     }

//   result->params = NULL;
//   result->preconds = NULL;
//   result->effects = NULL;
//   result->number_of_real_params = 0;
//   result->next = NULL;
//   return result;
// }


// CodeOperator *new_CodeOperator( void )

// {

//   int i;

//   CodeOperator * result = ( CodeOperator * ) calloc( 1, sizeof( CodeOperator ) );
//   num_codeoperator++;
//   CHECK_PTR(result);
// #ifdef MEMORY_INFO
//   gmemory += sizeof( CodeOperator );
// #endif

//   result->name = NULL;

//   result->num_vars = 0;
//   /* achtung: var_types und inst_table bleiben uninitialisiert!
//    * --> wird nicht drauf zugegriffen, immer von vorne bis num_vars
//    *
//    * mir egal.
//    */
//   for ( i = 0; i < MAX_VARS; i++ ) {
//     result->inst_table[i] = -1;
//   }

//   result->preconds = NULL;
//   result->conditionals = NULL;

//   result->number_of_real_params = 0;

//   result->next = NULL;

//   return result;

// }


/**********************************************************************
 * Allocates new memory for a PL1 style axiom operator. Creates a
 * unique name and marks it with the hidden string.
 *
 * RETURNS a pointer to the allocated memory
 *********************************************************************/
// PlOperator *
// new_axiom_op_list(void)
// {
//   static int count;
//   char * name;
//   PlOperator * ret;
//   num_axiom_op_list++;
//   /* WARNING: count should not exceed 999 */
//   count++;
//   name = new_token(strlen(HIDDEN_STR)+strlen(AXIOM_STR)+3+1);
//   sprintf(name, "%s%s%d", HIDDEN_STR, AXIOM_STR, count);

//   ret = new_pl_operator(name);
//   free(name);

//   return ret;
// }

/*
 * memory functions for graph stuff...
 */


OpNode *new_op_node( int time, const char *name, Bool is_noop,
		     BitVector *pos_precond_vector,
		     BitVector *neg_precond_vector, 
		     DdNode* b_pre1,int a_index)
		   

{

  OpNode *tmp = ( OpNode * ) calloc (1,  sizeof( OpNode ) );


  CHECK_PTR( tmp );
  num_op_node++;
#ifdef MEMORY_INFO
  ggraph_memory += sizeof( OpNode );
#endif

  if ( name ) {

/*     printf("New opNode for %s\n", tmp->name); */
/*     printf("HI\n"); */
    tmp->name = name;//copy_string( name );

  } else {
    tmp->name = NULL;
  }
  tmp->index = gops_count;

  tmp->alt_index = a_index;

  tmp->uid_block = gops_count / gcword_size;
  tmp->uid_mask = 1 << ( gops_count % gcword_size );
/*     printf("UID= %d, gops = %d \n", tmp->uid_mask, gops_count); */
  gops_count++;

  tmp->preconds = NULL;
  tmp->unconditional = NULL;// 内部没有创建uncondition effect，而是将effect的前提作为OpNode前提
  tmp->conditionals = NULL;
  tmp->b_pre = b_pre1;// 设置前提BDD
  tmp->is_noop = is_noop;
  tmp->unactivated_effects = NULL;
  for(int i = 0; i < IPP_MAX_PLAN+1; i++)
    tmp->info_at[i] = NULL;
  tmp->info_at[time] = new_op_level_info();
  
  //  print_BitVector(pos_precond_vector, gft_vector_length);printf("\n");
  //  print_BitVector(neg_precond_vector, gft_vector_length);printf("\n");

  if(pos_precond_vector)
    tmp->pos_precond_vector = //pos_precond_vector;//
      copy_bit_vector(pos_precond_vector, gft_vector_length);
  if(neg_precond_vector)
  tmp->neg_precond_vector = //neg_precond_vector;//
    copy_bit_vector(neg_precond_vector, gft_vector_length);

  tmp->next = NULL;


  tmp->unactivated_effects = NULL;
  tmp->thread = NULL;

  return tmp;

}
/*use the effect to create the effect Node*/
EfNode *new_ef_node( int time, OpNode *op,
		     /* BitVector *pos_effect_vector, */
/* 		     BitVector *neg_effect_vector */
		     
		     /* Consequent* cons, Antecedent* ant */
		     
		     Effect* eff, int a_index)		     
{
  Consequent *tmpCd, *tmpCs;
  EfNode *tmp = ( EfNode * ) calloc ( 1, sizeof( EfNode ) );
  Effect *tmpPtrs, *tmpPtrd, *tmpHlp;// ptrSource, ptrDestination, helper。将数据 source->dest
  num_ef_node++;
  CHECK_PTR( tmp );
#ifdef MEMORY_INFO
  ggraph_memory += sizeof( EfNode );
#endif
  // printf("New ef node %d %d \n", eff->index, a_index);
  gnum_cond_effects++;
  gnum_cond_effects_at[time]++;
  // printf("New ef node gnum_cond_effects = %d\n",  gnum_cond_effects/* _at[time] */ );

  tmp->in_rp= 0;//FALSE;
  tmp->alt_index = a_index;
  tmp->uid_block = gnum_cond_effects / gcword_size;
  tmp->uid_mask = 1 << ( gnum_cond_effects % gcword_size );
  tmp->index = gnum_cond_effects/* ++ */;
  tmp->op = op;// 设置effect所属的OpNode
  tmp->first_occurence = time;
  tmp->is_nondeter = eff->is_nondeter;//FALSE;
  tmp->conditions = NULL;// here do not set the condition, because the condtion have set to OpNode by insert_ft_edge
  tmp->effects = NULL;

 /*  tmp->pos_effect_vector = pos_effect_vector; */
/*   tmp->neg_effect_vector = neg_effect_vector; */
/*    tmp->effect = eff; */

  tmpPtrs = eff;
  tmpPtrd = new_Effect(a_index);

  
  if(eff->original)// 间接引用eff
    tmpPtrd->original = eff->original;
  else// 直接引用该Effect
    tmpPtrd->original = eff;
  
  //  tmpPtrd->in_rp = eff->in_rp;
  // add the outcome and their probability
  for(std::set<int>::iterator i = eff->outcome->begin(); i != eff->outcome->end(); i++)
    tmpPtrd->outcome->insert(*i);
  for(std::map<int, double>::iterator i = eff->probability->begin(); i != eff->probability->end(); i++)
    tmpPtrd->probability->insert(*i);
  tmpPtrd->probability_sum = eff->probability_sum;
  //cout << "pr size = " << tmpPtrd->probability->size() << endl;
  tmpPtrd->next = NULL;
  tmpPtrd->reward = eff->reward;
  tmpPtrd->node = eff->node;
  tmpPtrd->row = eff->row;
  // set the effect to EffectNode
  tmp->effect = tmpPtrd;
  

  copy_contents_of_FactInfo(&(tmpPtrd->ant->p_conds), tmpPtrs->ant->p_conds );
  copy_contents_of_FactInfo(&(tmpPtrd->ant->n_conds), tmpPtrs->ant->n_conds );

  if(tmpPtrs->ant->b){// the ant may be empty
    tmpPtrd->ant->b = tmpPtrs->ant->b;
    Cudd_Ref(tmpPtrd->ant->b);
  }
  copy_contents_of_FactInfo(&(tmpPtrd->cons->p_effects), tmpPtrs->cons->p_effects );
  copy_contents_of_FactInfo(&(tmpPtrd->cons->n_effects), tmpPtrs->cons->n_effects );
  
  tmpPtrd->cons->b = tmpPtrs->cons->b;

  //    printf("Did first cons\n");
//   tmpCs = tmpPtrs->cons->next;
//   while(tmpCs){
//     tmpCd = new_Consequent();
//     tmpCd->next = tmpPtrd->cons;
//     tmpPtrd->cons = tmpCd;
//     //  printf("Did next cons\n");
//     copy_contents_of_FactInfo(&(tmpCd->p_effects), tmpCs->p_effects );
//     copy_contents_of_FactInfo(&(tmpCd->n_effects), tmpCs->n_effects );
    
//     if( tmpCs->b){
//       tmpCd->b = tmpCs->b;
//       Cudd_Ref(tmpCd->b);
//       //	printBDD(tmpCd->b);
//     }
//     tmpCs= tmpCs->next;
//   }
  
  //printf("OUT\n");
  for(int i = 0; i < IPP_MAX_PLAN+1; i++)
    tmp->info_at[i] = NULL;
  tmp->info_at[time] = new_ef_level_info(time);

  tmp->next = NULL;

  return tmp;

}

//FtNode *new_ft_node( int time, int index, Bool positive, Bool dummy, goal* label0 )
FtNode *new_ft_node( int time, int index, Bool positive, Bool dummy, /* DdNode* */Label* label0 )
{

  int i;

  FtNode *tmp = ( FtNode * ) calloc ( 1, sizeof( FtNode ) );
  num_ft_node++;
  CHECK_PTR( tmp );
#ifdef MEMORY_INFO
  ggraph_memory += sizeof( FtNode );
#endif

//     printf("NEW_FT_NODE %d\n", index);
//   printFact(index); printf(" %d\n", positive);
  if ( !dummy ) { // first create is FALSE
    gfacts_count++;
  }
  if ( positive ) gprint_ftnum++;

  

  tmp->index = index;
  tmp->uid_block = index / gcword_size;
  tmp->uid_mask = 1 << ( index % gcword_size );
  
  tmp->positive = positive;

  tmp->adders = NULL;
  tmp->preconds = NULL;

  tmp->noop = NULL;

  // 清空level info
  for ( i = 0; i < IPP_MAX_PLAN+2; i++ ) {
    tmp->info_at[i] = NULL;
  }
  // 根据FactNode创建level info
  tmp->info_at[time] = new_ft_level_info( tmp );
  if(label0)// 设置Fact的level info the label
    tmp->info_at[time]->label = label0;
  else
    tmp->info_at[time]->label = NULL;

  tmp->next = NULL;

  return tmp;

}



FtEdge *new_ft_edge( FtNode *ft )

{

  FtEdge *tmp = ( FtEdge * ) calloc ( 1, sizeof( FtEdge ) );
  CHECK_PTR( tmp );
  num_ft_edge++;
#ifdef MEMORY_INFO
  ggraph_memory += sizeof( FtEdge );
#endif

  tmp->ft = ft;
  tmp->next = NULL;

  return tmp;

}

EfEdge *new_ef_edge( EfNode *ef )

{

  EfEdge *tmp = ( EfEdge * ) calloc ( 1, sizeof( EfEdge ) );
  CHECK_PTR( tmp );
  num_ef_edge++;
#ifdef MEMORY_INFO
  ggraph_memory += sizeof( EfEdge );
#endif
  tmp->ef = ef;
  tmp->next = NULL;

  return tmp;

}

OpEdge *new_op_edge( OpNode *op )

{

  OpEdge *tmp = ( OpEdge * ) calloc ( 1, sizeof( OpEdge ) );
  CHECK_PTR( tmp );
  num_op_edge++;
#ifdef MEMORY_INFO
  ggraph_memory += sizeof( OpEdge );
#endif

  tmp->op = op;
  tmp->next = NULL;

 return tmp;

}

OpLevelInfo *new_op_level_info( void )

{

  OpLevelInfo *tmp = ( OpLevelInfo * ) calloc ( 1, sizeof( OpLevelInfo ) );
  CHECK_PTR( tmp );
  num_op_level_info++;
#ifdef MEMORY_INFO
  ggraph_memory += sizeof( OpLevelInfo );
#endif
 if(SENSORS && USESENSORS)
   tmp->sensor_dependencies = new_bit_vector(((int)num_alt_acts/gcword_size));

 tmp->updated = 0;
 tmp->is_used = 0;
 if(MUTEX_SCHEME!=MS_NONE){
   tmp->exclusives = new_OpExclusion(num_alt_acts);
 }
 tmp->label = NULL;//new_Label();
   tmp->max_worlds = NULL;
  return tmp;

}

EfLevelInfo *new_ef_level_info(int time)

{

  EfLevelInfo *tmp = ( EfLevelInfo * ) calloc ( 1, sizeof( EfLevelInfo ) );
  CHECK_PTR( tmp );
  num_ef_level_info++;
#ifdef MEMORY_INFO
  ggraph_memory += sizeof( EfLevelInfo );
#endif

  tmp->is_dummy = FALSE;
  tmp->label = NULL;
  tmp->updated = 0;
  tmp->probability_sum = 0;
  tmp->probability = new std::map<int, double>();
  if(MUTEX_SCHEME!=MS_NONE){
    tmp->exclusives = new_EfExclusion(2*num_alt_facts+num_alt_effs);//gnum_cond_effects_pre);
    //tmp->exclusives->pos_exclusives = NULL; 
    //tmp->exclusives->neg_exclusives = NULL; 
  }
  tmp->max_worlds = NULL;
  return tmp;

}

FtLevelInfo *new_ft_level_info( FtNode *ft )

{

  FtLevelInfo *tmp = ( FtLevelInfo * ) calloc ( 1, sizeof( FtLevelInfo ) );
  CHECK_PTR( tmp );
  num_ft_level_info++;
#ifdef MEMORY_INFO
  ggraph_memory += sizeof( FtLevelInfo );
#endif

  tmp->label = NULL;// 初始化NULL后在调用处设置label
  tmp->adders_pointer = NULL;
  tmp->is_dummy = FALSE;
  tmp->relaxedPlanEdges = NULL;

  tmp->updated = 0;
  tmp->is_goal = 0;
  tmp->is_true = 0;
  tmp->is_goal_for[0] = NULL;
  if(MUTEX_SCHEME!=MS_NONE){
    tmp->exclusives = new_FtExclusion();// 创建正负Fact的exclusive vector和label
    //  tmp->exclusives->pos_exclusives = new_excl_bit_vector( gft_vector_length );
    //tmp->exclusives->neg_exclusives = new_excl_bit_vector( gft_vector_length );
    /* adders erst spaeter allozieren !: groesse op - vecs unbekannt */
    
    /* achtung! funktioniert nur, falls bitvectoren konstant lang!! */
    if ( ft->positive ) {// 设置该fact是对应neg的exclusive
      (tmp->exclusives->neg_exclusives)[ft->uid_block] |= ft->uid_mask;
    } else {
      (tmp->exclusives->pos_exclusives)[ft->uid_block] |= ft->uid_mask;
    }
  }
  else
    tmp->exclusives=NULL;
  tmp->adders = NULL;
  tmp->adders_exclusives = NULL;

  tmp->memo_start = NULL;

  return tmp;

}


OpPair *new_op_pair( OpNode *o1, OpNode *o2 )

{

  OpPair *tmp = ( OpPair * ) calloc ( 1, sizeof( OpPair ) );
  CHECK_PTR( tmp );

  num_op_pair++;
#ifdef MEMORY_INFO
  gexcl_memory += sizeof( OpPair );
#endif

  tmp->o1 = o1;
  tmp->o2 = o2;

  tmp->next = NULL;

  return tmp;

}

EfPair *new_ef_pair( EfNode *o1, EfNode *o2 )

{

  EfPair *tmp = ( EfPair * ) calloc ( 1, sizeof( EfPair ) );
  CHECK_PTR( tmp );

  //num_op_pair++;
#ifdef MEMORY_INFO
  gexcl_memory += sizeof( EfPair );
#endif

  tmp->e1 = o1;
  tmp->e2 = o2;

  tmp->next = NULL;

  return tmp;

}
  

FtPair *new_ft_pair( FtNode *f1, FtNode *f2 )

{

  FtPair *tmp = ( FtPair * ) calloc ( 1, sizeof( FtPair ) );
  CHECK_PTR( tmp );
  num_ft_pair++;
#ifdef MEMORY_INFO
  gexcl_memory += sizeof( FtPair );
#endif

  tmp->f1 = f1;
  tmp->f2 = f2;

  tmp->next = NULL;

  return tmp;

}
  


MemoNode *new_memo_node( int double_index, int way )

{

  MemoNode *tmp = ( MemoNode * ) calloc( 1, sizeof( MemoNode ) );
  CHECK_PTR( tmp );
  num_memo_node++;
#ifdef MEMORY_INFO
  gmemo_memory += sizeof( MemoNode );
#endif

  tmp->double_index = double_index;

  tmp->sons = NULL;

  tmp->min_way = way;

  /* evtl. prev = NULL
   */
  tmp->next = NULL;

  return tmp;

}


MemoNode_table *new_memo_node_table( void )

{

  MemoNode_table *tmp = ( MemoNode_table * ) calloc( 1, sizeof( MemoNode_table ) );
  num_memo_node_table++;
  CHECK_PTR( tmp );
#ifdef MEMORY_INFO
  gmemo_memory += sizeof( MemoNode_table );
#endif

  return tmp;

}



Candidate *new_candidate( void )

{

  Candidate *tmp = ( Candidate * ) calloc( 1, sizeof( Candidate ) );
  CHECK_PTR( tmp );
  num_candidate++;
#ifdef MEMORY_INFO
  gwave_memory += sizeof( Candidate );
#endif

  return tmp;

}




// RelevantFact *new_RelevantFact( CodeNode *n )

// {

//   RelevantFact *r;
//   int i;

//   r = ( RelevantFact * ) calloc( 1, sizeof( RelevantFact ) );
//   num_relevantfact++;
//   CHECK_PTR( r );
// #ifdef MEMORY_INFO
//   ggraph_memory += sizeof( Candidate );
// #endif

//   if ( n ) {
//     r->predicate = n->predicate;
//     for ( i=0; i<garity[n->predicate]; i++ ) {
//       r->arguments[i] = n->arguments[i];
//     }
//   }

//   return r;

// }





BitOperator *new_BitOperator( const char *name )

{


  BitOperator *tmp = ( BitOperator * ) calloc( 1, sizeof( BitOperator ) );
  CHECK_PTR( tmp );
  num_BitOperator++;
#ifdef MEMORY_INFO
  gmemo_memory += sizeof( BitOperator );
#endif

  if ( name ) {
    tmp->name = copy_string( name );
  } else {
    tmp->name = NULL;
  }
  tmp->valid = 1;
  tmp->p_preconds = new_FactInfo();
  tmp->n_preconds = new_FactInfo();
  tmp->unconditional = NULL;
  tmp->conditionals = NULL;
  tmp->activated_conditionals = NULL;
  tmp->next = NULL;

  return tmp;

}
/**
 * label的浅拷贝
 */
Label * new_Label(DdNode* lab, double csd)
{

  Label *tmp = (Label *) calloc (1, sizeof(Label));
    CHECK_PTR( tmp );

    num_label++;
#ifdef MEMORY_INFO
  gmemory += sizeof( Label );
#endif
  

  
  tmp->label = lab;
  Cudd_Ref(tmp->label);
  tmp->CSD = csd;
  // printf("CSD set to %f %f", tmp->CSD, csd);
  tmp->next = NULL;

  return tmp;
}

void free_Label(Label* lab) {

  if(lab){
    num_label--;
    if(lab->label )
      Cudd_RecursiveDeref(manager, lab->label);
    free_Label(lab->next);
    free(lab);
  }
}


Antecedent * new_Antecedent(void)
{
  Antecedent *tmp = (Antecedent *) calloc (1, sizeof(Antecedent));
    CHECK_PTR( tmp );
    num_antecedent++;
#ifdef MEMORY_INFO
  gmemory += sizeof( Antecedent );
#endif

  tmp->p_conds = new_FactInfo();
  tmp->n_conds = new_FactInfo();
  tmp->next = NULL;
  tmp->b = NULL;  
  return tmp;
}

Consequent * new_Consequent(void)
{
  Consequent *tmp = (Consequent *) calloc (1, sizeof(Consequent));
    CHECK_PTR( tmp );
    num_consequent++;
#ifdef MEMORY_INFO
  gmemory += sizeof( Consequent );
#endif

  tmp->p_effects = new_FactInfo();
  tmp->n_effects = new_FactInfo();
  tmp->next = NULL;
  tmp->b = NULL;

  return tmp;
}


Effect *new_Effect( int index )

{

  Effect *tmp = ( Effect * ) calloc( 1, sizeof( Effect ) );
  CHECK_PTR( tmp );
  num_effect++;
#ifdef MEMORY_INFO
  gmemory += sizeof( Effect );
#endif
  //    cout << "new ef" <<endl;
  tmp->original = NULL;
  tmp->index = index;
  tmp->ant = new_Antecedent();
  tmp->cons = new_Consequent();
  tmp->is_nondeter = FALSE;
  tmp->in_rp = 0;
  tmp->reward = 0.0;
  tmp->next = NULL;
  tmp->op = NULL;
  tmp->outcome = new std::set<int>();
  tmp->probability = new std::map<int, double>();
  return tmp;

}



FactInfo *new_FactInfo( void )

{
  
  FactInfo *tmp = ( FactInfo * ) calloc( 1, sizeof( FactInfo ) );
  num_fact_info++;
  CHECK_PTR( tmp );
#ifdef MEMORY_INFO
  gmemory += sizeof( FactInfo );
#endif

  tmp->vector = new_bit_vector( gft_vector_length );
  tmp->indices = NULL;

  return tmp;

}


  

/**********************************************************************
 * Creates a new bit vector with all bits unset.
 *
 * int length: The length of the vector in integer.
 *
 * RETURNS a new bit vector with all bits set to 0.
 *********************************************************************/
BitVector *
new_bit_vector(int length)
{
  BitVector * result = (BitVector *) calloc(length, sizeof(int));// 分配一个初始化为0的指针

  num_bitvector++;

  CHECK_PTR(result);// 检测是否null
#ifdef MEMORY_INFO
  gmemory += length;
#endif
  memset(result, 0, length);// 置为0



  return result;
}

BitVector *
new_excl_bit_vector(int length)
{
  BitVector * result = (BitVector *) calloc(length, sizeof(int));
  num_bitvector++;
  CHECK_PTR(result);
#ifdef MEMORY_INFO
  gexcl_memory += length;
#endif
  memset(result, 0, length);

  return result;
}


/**********************************************************************
 * Creates a new instantiated effect, and creates all necessary
 * bit vectors with the global bit vector length "gft_vector_length".
 * 
 * NOTE: gft_vector_length is global!
 *       The conditions are NOT allocated! 
 *
 * RETURNS a new instantiated effect.
 *********************************************************************/
Effect * 
new_effect()
{
  Effect * result = (Effect *) malloc(sizeof(Effect));
  CHECK_PTR(result);
  num_effect++;
#ifdef MEMORY_INFO
  gmemory += sizeof(Effect);
#endif
  result->ant->p_conds = NULL;
  result->ant->n_conds = NULL;
  result->cons->p_effects = NULL;//new_fact_info();
  result->cons->n_effects = NULL;//new_fact_info();
  result->next = NULL;
  return result;
}




/*********************************************************************
 * Creates a new structure that holds one integer !!
 * 
 * NOTE: The index is initialized to -1.
 *
 * int i: The value for index.
 *
 * RETURNS a pointer to the new structure.
 *********************************************************************/
Integers * 
new_integers(int i)
{
  Integers * result = (Integers *) malloc(sizeof(Integers));
  CHECK_PTR(result);
  num_integers++;
#ifdef MEMORY_INFO
  gmemory += sizeof(Integers);
#endif

  result->index = i;
  result->next = NULL;
  return result;
}




/*********************************************************************
 * Creates a new fact info structure with a bit vector with the
 * correct length. A fact info is a tuple of a bit vector and a list
 * of integers. The integers are the indices of the set bits in the
 * vector. 
 * 
 * NOTE: indices is initialized to NULL;
 *
 * RETURNS a pointer to the new structure.
 *********************************************************************/
FactInfo * 
new_fact_info()
{
  FactInfo * result = (FactInfo *) malloc(sizeof(FactInfo));
  CHECK_PTR(result);
  num_fact_info++;
#ifdef MEMORY_INFO
  gmemory += sizeof(FactInfo);
#endif
  
  result->vector = new_bit_vector(gft_vector_length);
  result->indices = NULL;
  return result;
}

/*********************************************************************
 * Creates a new pair of two fact info structures.
 *
 * FactInfo * pos: This represents the positive part.
 * FactInfo * neg: This represents the negaitve part.
 *
 * RETURNS a pointer to the new structure.
 *********************************************************************/
FactInfoPair * 
new_fact_info_pair(FactInfo * pos, FactInfo * neg)
{
  FactInfoPair * result = (FactInfoPair *) malloc(sizeof(FactInfoPair));
  CHECK_PTR(result);
  num_fact_info_pair++;
#ifdef MEMORY_INFO
  gmemory += sizeof(FactInfoPair);
#endif
  
  result->positive = pos;
  result->negative = neg;
  return result;
}


//dan added these


ExclusionLabelPair* new_ExclusionLabelPair(DdNode* l1, DdNode* l2, int last){
  ExclusionLabelPair * result = NULL;
  if(!l1 || !l2) {
    printf("SHITTY\n");
    exit(0);
    return NULL;
  }
  num_exclusionlabelpair++;

   result = (ExclusionLabelPair  *) malloc( sizeof(ExclusionLabelPair ));
 CHECK_PTR(result);
#ifdef MEMORY_INFO
  gmemory += sizeof(ExclusionLabelPair);
#endif

  //printf("Creating new cross world mutex in memory.c:\n");

  result->last = last;

  

  result->elp1 = l1;
  // Cudd_Ref(result->elp1);
  result->elp2 = l2;
  //printf("2\n");
  //Cudd_Ref(result->elp2);
  result->next = NULL;
  //printBDD(result->elp1);
  //printBDD(result->elp2);
 
  return result;
}

FtExclusion* new_FtExclusion(void) {
  int i = 0;
  FtExclusion* result = (FtExclusion  *) calloc(1,sizeof(FtExclusion ));
  CHECK_PTR(result);
  num_ftexclusion++;
#ifdef MEMORY_INFO
  gmemory += sizeof(FtExclusion);
#endif

  result->pos_exclusives = new_excl_bit_vector(gft_vector_length);
  result->neg_exclusives = new_excl_bit_vector(gft_vector_length);
  // 创建 exclusive label(互斥的命题对应的label)
  if(MUTEX_SCHEME!=MS_REGULAR){
    result->p_exlabel = (//ExclusionLabelPair
			 DdNode**) calloc (num_alt_facts, sizeof(//ExclusionLabelPair
								       DdNode* ));
    result->n_exlabel = (//ExclusionLabelPair
			 DdNode**) calloc (num_alt_facts, sizeof(//ExclusionLabelPair
								       DdNode* ));
    for(; i<num_alt_facts;i++){
      result->p_exlabel[i] = Cudd_ReadLogicZero(manager);
      result->n_exlabel[i] = Cudd_ReadLogicZero(manager);
    }
  }
  else{
    result->p_exlabel = NULL;
    result->n_exlabel = NULL;
    
  }
  return result;
}

EfExclusion* new_EfExclusion(int length) {
  int i = 0;
  EfExclusion* result = (EfExclusion  *) calloc(1, sizeof(EfExclusion ));
  CHECK_PTR(result);
  num_efexclusion++;
#ifdef MEMORY_INFO
  gmemory += sizeof(EfExclusion);
#endif
  //result->set = FALSE;
  //  std::cout << "new EfExclusion " << length << std::endl;
  //  cout << ((num_alt_effs%gcword_size)+1) << " " << num_alt_effs << endl << flush;
  result->exclusives = new_excl_bit_vector((num_alt_effs%gcword_size)+2*gft_vector_length);
  result->pos_exclusives = new_excl_bit_vector(gft_vector_length);
  result->neg_exclusives = new_excl_bit_vector(gft_vector_length);

  if(MUTEX_SCHEME!=MS_NONE){
    result->exlabel = (DdNode**) calloc (length, sizeof(DdNode* ));
     result->n_exlabel = (DdNode**) calloc (num_alt_facts, sizeof(DdNode* ));
     result->p_exlabel = (DdNode**) calloc (num_alt_facts, sizeof(DdNode* ));


    for(i=0; i<length;i++){
      //       printf("Setting slot %d to null \n", i);
      result->exlabel[i] = /* new_ExclusionLabelPair(NULL, NULL, TRUE);//new_ExclusionLabelPair(Cudd_ReadLogicZero(manager),Cudd_ReadLogicZero(manager));// */// NULL;
      Cudd_ReadLogicZero(manager);
      Cudd_Ref(result->exlabel[i]);
    }
    for(i=0; i<num_alt_facts;i++){ 
      result->p_exlabel[i] = /* new_ExclusionLabelPair(NULL, NULL, TRUE);//new_ExclusionLabelPair(Cudd_ReadLogicZero(manager),Cudd_ReadLogicZero(manager)); *///NULL;
            Cudd_ReadLogicZero(manager);
      Cudd_Ref(result->p_exlabel[i]);
      result->n_exlabel[i] = /* new_ExclusionLabelPair(NULL, NULL, TRUE);//new_ExclusionLabelPair(Cudd_ReadLogicZero(manager),Cudd_ReadLogicZero(manager));  */ //NULL;
      Cudd_ReadLogicZero(manager);
      Cudd_Ref(result->n_exlabel[i]);
    }
  }


   return result;
}

/* EfExclusion* set_Labels_EfExclusion(EfExclusion* result, int length) { */
/*   int i = 0; */
/*   result->set = TRUE; */
/*   result->exclusives = new_excl_bit_vector(gef_vector_length); */
  
/*     result->exlabel = (ExclusionLabelPair**) calloc (length, sizeof(ExclusionLabelPair* )); */
/*   for(; i<length;i++){ */
/*     //   printf("Setting slot %d to null \n", i); */
/*     result->exlabel[i] = /\* new_ExclusionLabelPair(NULL, NULL, TRUE);//new_ExclusionLabelPair(Cudd_ReadLogicZero(manager),Cudd_ReadLogicZero(manager));// *\/NULL; */
/*   } */
 
/*    return result; */
/* } */

OpExclusion* new_OpExclusion(int length){
  int i = 0;
  OpExclusion* result = (OpExclusion  *) calloc(1, sizeof(OpExclusion ));
  CHECK_PTR(result);
#ifdef MEMORY_INFO
  gmemory += sizeof(OpExclusion);
#endif

  num_opexclusion++;

  result->exclusives = //new_bit_vector(length);//
    new_excl_bit_vector(gop_vector_length);
  //std::cout << "GOPS=" << (num_alt_acts+2*num_alt_facts) << std::endl;
  if(MUTEX_SCHEME!=MS_REGULAR){
    result->exlabel = (DdNode**) calloc (num_alt_acts+2*num_alt_facts,
					 sizeof(DdNode* ));
   
    for(; i<(num_alt_acts+2*num_alt_facts);i++)
      // printf("%d\n", i);
      result->exlabel[i] = Cudd_ReadLogicZero(manager);
  }
  return result;
}



//donedan



/**********************************************************************
 ********************** DELETION FUNCTIONS ****************************
 *********************************************************************/

/*********************************************************************
 * Free a complete TokenList, but do not delete predicates and 
 * constants. It is therefore important that all variables start 
 * with a unique identifier, we use '?'.
 * 
 * TokenList * tl: The beginning of the TokenList to free.
 *********************************************************************/
void
free_token_list(TokenList * tl)
{
    if (NULL != tl) {
      num_token_list--;
	free_token_list(tl->next);
	/* Now free the token, but only if it is a variable. */
	//	if (NULL != tl->item && '?' == *(tl->item)) {
	    free(tl->item);
	    num_tokens--;
	    //}
	free(tl); 
    }
}

/**********************************************************************
 * Free a complete FactList, but do not free any predicates or 
 * constants, i.e. only delete strings beginning with a '?'.
 *
 * FactList * list: the list that should be deleted.
 *********************************************************************/
void
free_fact_list(FactList * list)
{
  if (NULL != list)
    {
      num_fact_list--;
      free_fact_list(list->next);
      /* This function takes care of predicates and constants. */
      free_token_list(list->item);
      free(list);
    }
}

/*********************************************************************
 * Free a complete tree, but leave predicate names in memory.
 * 
 * PlNode * node: The root of the tree to free.
 *********************************************************************/
// void 
// free_tree(PlNode * node)
// {
//     if (NULL != node) {
//       num_pl_node--;
// 	free_tree(node->sons);
// 	free_tree(node->next);
// 	/* now free the node itself */
// 	free_token_list(node->atom);
// 	free(node);
//     }
// }


// void free_CodeNode( CodeNode *node )

// {

//   if ( node ) {
//     num_codenode--;
//     free_CodeNode( node->sons );
//     free_CodeNode( node->next );
// #ifdef MEMORY_INFO
//     gmemory -= sizeof( CodeNode );
// #endif
//     free( node );
//   }

// }


// void free_CodeOperator( CodeOperator *arme_sau )

// {

//   if ( arme_sau ) {
//     if ( arme_sau->name ) {
//       free( arme_sau->name );
//     }
//     free_CodeNode( arme_sau->preconds );
//     free_CodeNode( arme_sau->conditionals );
//     /* resources are still missing. (I'll be missing you...)
//      */
// #ifdef MEMORY_INFO
//     gmemory -= sizeof( CodeOperator );
// #endif
//     free( arme_sau );
//     num_codeoperator--;
//   }

// }


// /*********************************************************************
//  * Free a pl_node including its atom tokenlist, but leave predicate 
//  *     names in memory.
//  * 
//  * PlNode * node: The node to free.
//  *********************************************************************/
// void 
// free_pl_node(PlNode * node)
// {
//     if (NULL != node) {
//       num_pl_node--;
// 	free_token_list(node->atom);
// 	free(node);
//     }
// }

/**********************************************************************
 *
 *********************************************************************/
void 
free_complete_token_list(TokenList * source )
{
  if ( source )
    {
      free_complete_token_list( source->next );
      if ( source->item ) {
	
        free( source->item );
	source->item = NULL;
      }
      free( source );
      num_token_list--;
    }
}

/**********************************************************************
 *
 *********************************************************************/
void 
free_complete_fact_list(FactList * source )
{
  if ( source )
    {
      num_fact_list--;
      free_complete_fact_list( source->next );
      free_complete_token_list( source->item );
      free( source );
    }
}

/**********************************************************************
 * Delete a list of operators and take care of predicates and
 * constants.
 * 
 * PlOperator * op: The start of the list that is deleted.
 **********************************************************************/
// void 
// free_ops(PlOperator * op)
// {
//     if (NULL != op) {
      
// 	free_ops(op->next);
// 	num_pl_operator--;
// 	free_complete_token_list(op->name);
// 	free_complete_fact_list(op->params);
// 	free_tree(op->preconds);
// 	free_tree(op->effects);
// 	free(op);
// 	/* Resources are still missing. */
//     }
// }

// /**********************************************************************
//  * Delete a PlOperator and take care of predicates and
//  * constants.
//  * 
//  * PlOperator * op: The operator that is deleted.
//  **********************************************************************/
// void 
// free_pl_op(PlOperator * op)
// {
//     if (NULL != op) {
//       num_pl_operator--;
      
//       free_token_list(op->name);
//       free_fact_list(op->params);
//       free_tree(op->preconds);
//       free_tree(op->effects);
//       free(op);
//       /* Resources are still missing. */
//     }
// }

/**********************************************************************
 *
 *********************************************************************/


void free_integers( Integers *l, int intcount )

{

     

  

  if ( l != NULL /*&& intcount < num_alt_facts*/ ) {
     
   //   if(l->next ){
       
   //     printf("INT %d\n", intcount);
    num_integers--;
     
     free_integers( l->next, ++intcount );
     //printf("INT a\n");
     
     if(l){
       //printf("INT D\n");

	  free( l );
	//  printf("INT DONE\n");
       }
 }
 //else
   //   printf("INT END\n");
}

void free_partial_effect( Effect *ef )

{
    printf("FREEING\n");
    exit(0);
  if ( ef != NULL ) {
    //    free_integers( ef->ant->p_conds->indices, 0 );
    // free_fact_info( ef->ant->p_conds );
    // free_integers( ef->ant->p_conds->indices, 0 );
    //  free_fact_info( ef->ant->n_conds );
    //printf("FREEING\n");
    num_effect--;
    free_integers( ef->cons->p_effects->indices, 0 );
    // free( ef->cons->p_effects );
    free_integers( ef->cons->n_effects->indices, 0 );
    //   free( ef->cons->n_effects );
    free( ef );
  }
  //printf("Done FREEING\n");


}
/**
 * momo007 2022.09.26
 * 目前的没有调用进行内存释放
 */
void free_partial_operator( BitOperator *op )

{
  //printf("Freeing partial op\n");
    //    exit(0);
  if ( op != NULL ) {
    if ( op->name ) {
      num_BitOperator--;
      free( op->name );


    }
    /* preconds: we need the vectors in search; only free indices and
     * structure holding parts together
     */

    
    free_integers( op->p_preconds->indices, 0 );

  free( op->p_preconds );

  free_integers( op->n_preconds->indices, 0 );

  free( op->n_preconds );
    if ( op->unconditional ) {
      /* unconditional is thorwn away, except for the strings
       * describing the pos and neg effects;
       * keep those for exclusions (interfere) check
       */

     free_integers( op->unconditional->cons->p_effects->indices, 0 );

  free( op->unconditional->cons->p_effects );

  free_integers( op->unconditional->cons->n_effects->indices, 0 );

  free( op->unconditional->cons->n_effects );

    free_fact_info( op->unconditional->ant->p_conds );

    free_fact_info( op->unconditional->ant->n_conds );


      free( op->unconditional );
    }
    /* do not touch the conditionals at all, those can be activated in 
     * later time steps, so we have to keep them still
     * (get pointed to by op_node->unactivated_effects)
     */

    free( op );
  }
  //   printf("DONE\n");
}
    
void free_fact_info_pair( FactInfoPair *p )

{

  //   printf("hi\n");
  if ( p != NULL ) {

    //     printf("Pos\n");
    num_fact_info_pair--;
    // if(p->positive)
    free_fact_info( p->positive );

    // printf("Neg\n");

    //if(p->negative)
   free_fact_info( p->negative );

    // printf("ptr\n");

    free( p );

    //  printf("Done\n");

  }

}

  

/**********************************************************************
 *
 *********************************************************************/
void 
free_bit_vector(BitVector * bvec)
{
  if ( bvec != NULL ) {
    //   printf("bV %d \n", bvec);
    num_bitvector--;
    free(bvec);
  
	  //printf("DBV\n");

 }
}


/**********************************************************************
 *
 *********************************************************************/
void 
free_effect(Effect * effect)
{
  printf("Free EFF\n");
  exit(0);
  if (NULL != effect)
    {    
      free_effect(effect->next);
      num_effect--;
      free_fact_info(effect->ant->p_conds);
      free_fact_info(effect->ant->n_conds);
      free_fact_info(effect->cons->p_effects);
      free_fact_info(effect->cons->n_effects);
      free(effect);
    }
}




/**********************************************************************
 *
 *********************************************************************/
void 
free_fact_info(FactInfo * info)
{

  if ( info != NULL ) {
    
  
     
    //      printf("1\n");
    num_fact_info--;
    free_bit_vector(info->vector);
  
    //printf("2\n");
     //   if(info != NULL && info->indices != NULL)
       free_integers(info->indices, 0);
       //printf("3\n");
  
     free( info );
  
     //printf("4\n");

 }

}



void free_BitOperator( BitOperator *op )

{
 printf("Free EFF\n");
  exit(0);
  if ( op != NULL ) {
    num_BitOperator--;
    if ( op->name ) {
      free( op->name );
    }
    free_fact_info( op->p_preconds );
    free_fact_info( op->n_preconds );
    free_effect( op->unconditional );
    free_effect( op->conditionals );
    free( op );
  }

}



void free_complete_FactList( FactList *source )

{

  if ( source ) {
    num_fact_list--;
    free_complete_FactList( source->next );
    free_complete_TokenList( source->item );
    free( source );
  }

}



void free_complete_TokenList( TokenList *source )

{

  if ( source ) {
    num_token_list--;
    free_complete_TokenList( source->next );
    if ( source->item ) {
      free( source->item );
    }
    free( source );
  }

}



void free_TypedList( TypedList *t )

{

  if ( t ) {
    if ( t->name ) {
      free( t->name );
      t->name = NULL;
    }
    if ( t->type ) {
      free_token_list( t->type );
      t->type = NULL;
    }
    free_TypedList( t->next );
    num_typedlist--;
    free( t );
  }

}



void free_TypedListList( TypedListList *t )

{

  if ( t ) {
     num_typedlistlist--;
   if ( t->predicate ) {
      free( t->predicate );
      t->predicate = NULL;
    }
    if ( t->args ) {
      free_TypedList( t->args );
      t->args = NULL;
    }
    free_TypedListList( t->next );

    free( t );
  }

}



void free_op_node( OpNode *op )

{

  int i;
  Effect* tmpe, *tmpe1;
  

  if ( op ) {
    num_op_node--;

    free_op_node( op->next );
//      if(!op->is_noop)
//          printf("free op node %s\n", op->name);       
//        else
//          printf("free noop\n");
//        fflush(stdout);
   

   
     free_ft_edge( op->preconds ); 
    free_ef_node( op->unconditional );
    free_ef_node( op->conditionals );
    free_bit_vector(op->pos_precond_vector);
    free_bit_vector(op->neg_precond_vector);
     
 


    while(op->unactivated_effects){

      tmpe = op->unactivated_effects;
      op->unactivated_effects = op->unactivated_effects->next;
     
      //    printf("tmpe = %s\n",tmpe->index);  
     
      //if(tmpe->index )
/*        printf("tmpe = %d\n",tmpe->index); */

/*      printf("tmpe->op->activated_conditionals = %s\n", tmpe->op->activated_conditionals->index); */

       tmpe1 = tmpe->op->activated_conditionals;
       int repeat = FALSE;
       while(tmpe1){
	 if(tmpe->index == tmpe1->index){
	   repeat = TRUE;
	   break;
	 }
	 tmpe1=tmpe1->next;
       }
       if(!repeat){
	 tmpe->next = tmpe->op->activated_conditionals;
	 tmpe->op->activated_conditionals = tmpe;
       }
  
    }

    op->unactivated_effects = NULL;
     for ( i = 0; i < IPP_MAX_PLAN; i++ ) {
       free_op_level_info( op->info_at[i] );
     }
 
 
     free( op ); 
     //printf("IIHSF\n");  
  }

}

void free_Effect(Effect *ef){
  if(ef){
    num_effect--;
    free_Antecedent(ef->ant);
    free_Consequent(ef->cons);
    free_Effect(ef->next);
    delete ef->probability;
    delete ef->outcome;
    // free(ef);
  }
}
 
void free_Antecedent(Antecedent* ant){
  if(ant){
    num_antecedent--;
    free_fact_info(ant->p_conds);
    free_fact_info(ant->n_conds);
    //Cudd_RecursiveDeref(manager, ant->b);
    free_Antecedent(ant->next);
    free(ant);
  }
}

void free_Consequent(Consequent* cons){
 if(cons){
    num_consequent--;
    free_fact_info(cons->p_effects);
    free_fact_info(cons->n_effects);
    // Cudd_RecursiveDeref(manager, cons->b);
    free_Consequent(cons->next);
    free(cons);
  }
}


void free_ef_node( EfNode *ef )

{

  int i;
  if ( ef ) {
    //    printf("free ef node\n"); fflush(stdout);
    free_ef_node( ef->next );

    num_ef_node--;
    free_ft_edge( ef->conditions );
    free_ft_edge( ef->effects ); 
    // printf("POOF\n");  fflush(stdout);     
    if(1 || ef->op->is_noop || ef == ef->op->unconditional){
//       if(ef->op->is_noop)
// 	cout << "free nooP"<< endl;
//       else
// 	cout << "free uncond" << endl;
      free_Effect(ef->effect);
     /*     free( ef->pos_effect_vector );  */
     /*      free( ef->neg_effect_vector );  */
    //     free( ef->effect );
    }

    for ( i = 0; i < IPP_MAX_PLAN; i++ ) {
       free_ef_level_info( ef->info_at[i] );
     }

    
    free( ef );
 
  }

}



void free_ft_node( FtNode *ft )

{

  int i;

  if ( ft ) {
    num_ft_node--;
     free_ef_edge( ft->adders );
     free_op_edge( ft->preconds );

    for ( i = 0; i < IPP_MAX_PLAN; i++ ) {
      //cout << "i = " << i << endl;
      free_ft_level_info( ft->info_at[i] );
    }
    free( ft );
  }

}



void free_memo_node( MemoNode *m )

{

  int i;

  if ( m ) {
    
    free_memo_node( m->next );
    for ( i = 0; i < MEMO_HASHSIZE; i++ ) {
      free_memo_node( (*(m->sons))[i] );
    }
    free( m->sons );

    free( m );
  }

}



void free_op_level_info( OpLevelInfo *i )

{

  if ( i ) {
    num_op_level_info--;
    //printf("free op level info\n"); 
    
    free_op_exclusion( i->exclusives );
    
    //if(SENSORS && USESENSORS)
    free_bit_vector(i->sensor_dependencies);
    free_Label(i->label);

    //cout << "[" << flush;
    free( i );
     //cout << "]" << endl<<flush;
    // printf("exit free op level info\n"); fflush(stdout);
  }
}



void free_ef_level_info( EfLevelInfo *i )

{
  
  
  if ( i ) {
    //printf("free ef level info\n");  fflush(stdout);
    num_ef_level_info--;
    

  
    free_Label(i->label); 
    delete i->probability;
    // cout << "{" << flush;

    free_ef_exclusion(i->exclusives);
     //    cout << "HI" << endl << flush;
    //cout << "}" << endl<<flush;
    free( i );
    //printf("exit free ef level info\n");fflush(stdout);
  }
  

}



void free_ft_level_info( FtLevelInfo *i )

{
//   printf("free ft level info\n"); 
//   fflush(stdout);

  if ( i ) {
    num_ft_level_info--;
    free_ft_exclusion( i->exclusives );
    free_ef_exclusion( i->adders_exclusives );          
    //free_bit_vector( i->adders );
    free_ef_edge( i->relaxedPlanEdges );     
    free_Label(i->label);     
    free_memo_node( i->memo_start );     
    free( i );
  }
}

void free_op_exclusion(OpExclusion *oe){
  if(oe){
    //      std::cout << "free opex" << std::endl;
    num_opexclusion--;
    free_bit_vector( oe->exclusives );
    free( oe );
  }
}
void free_ef_exclusion(EfExclusion *oe){
  if(oe){
    //    std::cout << "free efex" << std::endl;
    //    exit(0);
    num_efexclusion--;
    free_bit_vector( oe->exclusives );
    free_bit_vector( oe->pos_exclusives );
    free_bit_vector( oe->neg_exclusives );
    free( oe );
  }
}
void free_ft_exclusion(FtExclusion *oe){
  if(oe){
    //   std::cout << "free ftex" << std::endl;
    num_ftexclusion--;
    free_bit_vector( oe->pos_exclusives );
    free_bit_vector( oe->neg_exclusives );
    free( oe );
  }
}
void free_op_edge( OpEdge *e )

{

  if ( e ) {
    num_op_edge--;
    free_op_edge( e->next );

    free( e );
  }

}



void free_ef_edge( EfEdge *e )

{

  if ( e ) {
    num_ef_edge--;
    free_ef_edge( e->next );
    free( e );
  }

}



void free_ft_edge( FtEdge *e )

{
  // printf("free ft edge\n");
  if ( e ) {
    num_ft_edge--;
    free_ft_edge( e->next );

    free( e );
  }

}



void free_op_pair( OpPair *p )

{

  if ( p ) {
    num_op_pair--;
    free_op_pair( p->next );

    free( p );
  }

}

void free_ef_pair( EfPair *p )

{

  if ( p ) {
    //    num_ef_pair--;
    free_ef_pair( p->next );

    free( p );
  }

}



void free_ft_pair( FtPair *p )

{

  if ( p ) {
    num_ft_pair--;
    free_ft_pair( p->next );

    free( p );
  }

}



void free_candidate( Candidate *c )

{

  if ( c ) {
    num_candidate--;
    free( c->next );

    free( c );
  }

}



void printMemInfo(){
  //printf("num_tokens =             %d\n", num_tokens);
//printf("num_token_list =         %d\n",num_token_list);
//printf("num_fact_list =          %d\n",num_fact_list);
//printf("num_typedlist =          %d\n",num_typedlist);
//printf("num_typedlistlist =      %d\n",num_typedlistlist);
//printf("num_pl_node =            %d\n",num_pl_node);
//printf("num_codenode =           %d\n",num_codenode);
//printf("num_pl_operator =        %d\n",num_pl_operator);
//printf("num_codeoperator =       %d\n",num_codeoperator);
//printf("num_axiom_op_list =      %d\n",num_axiom_op_list);
printf("num_op_node =            %d\n",num_op_node);
printf("num_ef_node =            %d\n",num_ef_node);
printf("num_ft_node =            %d\n",num_ft_node);
printf("num_ft_edge =            %d\n",num_ft_edge);
printf("num_ef_edge =            %d\n",num_ef_edge);
printf("num_op_edge =            %d\n",num_op_edge);
printf("num_op_level_info =      %d\n",num_op_level_info);
printf("num_ef_level_info =      %d\n",num_ef_level_info);
printf("num_ft_level_info =      %d\n",num_ft_level_info);
printf("num_op_pair =            %d\n",num_op_pair);
printf("num_ft_pair =            %d\n",num_ft_pair);
//printf("num_memo_node =          %d\n",num_memo_node);
//printf("num_memo_node_table =    %d\n",num_memo_node_table);
//printf("num_candidate =          %d\n",num_candidate);
//printf("num_relevantfact =       %d\n",num_relevantfact);
//printf("num_BitOperator =        %d\n",num_BitOperator);
//printf("num_label =              %d\n",num_label);
printf("num_antecedent =         %d\n",num_antecedent);
printf("num_effect =             %d\n",num_effect);
printf("num_consequent =         %d\n",num_consequent);
//printf("num_factinfo =           %d\n",num_factinfo);
printf("num_bitvector =          %d\n",num_bitvector);
//printf("num_excl_bit_vector =    %d\n",num_excl_bit_vector);
//printf("num_effect1 =            %d\n",num_effect1);
printf("num_integers =           %d\n",num_integers);
printf("num_fact_info =          %d\n",num_fact_info);
printf("num_fact_info_pair =     %d\n",num_fact_info_pair);
//printf("num_exclusionlabelpair = %d\n",num_exclusionlabelpair);
printf("num_ftexclusion =        %d\n",num_ftexclusion);
printf("num_efexclusion =        %d\n",num_efexclusion);
printf("num_opexclusion =        %d\n",num_opexclusion);
 fflush(stdout);
}

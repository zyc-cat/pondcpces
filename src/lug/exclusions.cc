



/*********************************************************************
 * File: exclusions.c
 * Description: routines for calculating exclusion relations
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
/*********************************************************************
 *
 * NOTE: the commentaries in this file, sparse as they are, are all
 *       in German, cause these are thoughts that I had while working
 *       on some really tedious parts of the code (potential effects...)
 * 
 *       If you have problems understanding the code (like I do when I have
 *       a look at it now), contact me at:
 *       
 *       hoffmann@informatik.uni-freiburg.de
 *
 *       and I'll be happy to answer your questions, if I can...
 *
 **********************************************************************/






#include "ipp.h"

#include "output.h"
#include "utilities.h"
#include "memory.h"

#include "exclusions.h"
#include "graph_wrapper.h"
#include "globals.h"

#include "cudd/dd.h"

extern int WORLD_CHECK;
#define WC_ALL 500
#define WC_SAME 501
#define WC_INTERSECT 502


extern int MUTEX_SCHEME;
#define MS_CROSS 350
#define MS_REGULAR 351
#define MS_NONE 352
#define MS_STATIC 353
#define INDUCED 99
extern int DO_INDUCED;

extern int num_ft_pair;
extern int num_op_pair;

int isOneOf(FtNode*, FtNode*);

/*************************************
 * SIMPLE HELPERS, SHOULD BE MACROS. *
 *************************************/

DdNode* add_worlds_to_mutex(DdNode* mutex, DdNode* newMutex){
  if(mutex == NULL){
    mutex = newMutex;
    Cudd_Ref(mutex);
  }
  else{
    DdNode* tmp = Cudd_bddOr(manager, mutex, newMutex);
    Cudd_Ref(tmp);
    Cudd_RecursiveDeref(manager, mutex);
    mutex = tmp;
    Cudd_Ref(mutex);
    Cudd_RecursiveDeref(manager, tmp);
  }
  //  std::cout << "New label = " << std::endl;
  //  printBDD(mutex);
  return mutex;
}

// DdNode* add_worlds_to_mutex(DdNode* mutex, DdNode* newMutexA, DdNode* newMutexB){
//   if(newMutexB != NULL){
//     DdNode* newMutexBp = Cudd_bddPermute(manager, newMutexB, varmap);
//     Cudd_Ref(newMutexBp);
//     DdNode* newMutex = Cudd_bddAnd(manager, newMutexA, newMutexBp);
//     Cudd_Ref(newMutex);
//     Cudd_RecursiveDeref(manager, newMutexBp);
//     return add_worlds_to_mutex(mutex, newMutex);
//   }
//   else if(newMutexA != NULL)
//     return add_worlds_to_mutex(mutex, newMutexA, newMutexA);
// }




void MAKE_EFS_EXCLUSIVE( int time, EfNode *ef1, EfNode *ef2, DdNode* label1)//, DdNode* label2 ) 
{

  //  printf("Making ef of %d mutex with ef of %d at time %d with labels = \n",ef1->alt_index,  ef2->alt_index, time);
  //printBDD(label1);
  // printf("YO\n"); 
   //   printBDD(label2);
  // printf("NO\n"); 
  
//   OpNode *i1 = ef1->op;
//   OpNode *i2 = ef2->op;
//        if(i1->is_noop){
//  	cout << "Noop " << (i1->preconds->ft->positive ? "pos " : "neg ");
//  	printFact(i1->preconds->ft->index);
//        }
//        else
//  	cout << i1->name << std::endl;
      
//        if(i2->is_noop){
//  	cout << "Noop " << (i2->preconds->ft->positive ? "pos " : "neg ");
//  	printFact(i2->preconds->ft->index);
//        }
//        else
//  	cout << i2->name << std::endl;
      
  

    set_bit(ef1->info_at[time]->exclusives->exclusives,ef2->alt_index);
    //  print_BitVector(ef1->info_at[time]->exclusives->exclusives, gef_vector_length);
    set_bit(ef2->info_at[time]->exclusives->exclusives, ef1->alt_index);
    //print_BitVector(ef2->info_at[time]->exclusives->exclusives, gef_vector_length);

    DdNode* tmp = add_worlds_to_mutex(ef1->info_at[time]->exclusives->exlabel[ef2->alt_index],
				      label1);
   Cudd_Ref(tmp);
   if(ef1->info_at[time]->exclusives->exlabel[ef2->alt_index])
     Cudd_RecursiveDeref(manager, ef1->info_at[time]->exclusives->exlabel[ef2->alt_index]);
   ef1->info_at[time]->exclusives->exlabel[ef2->alt_index] = tmp;
   Cudd_Ref(ef1->info_at[time]->exclusives->exlabel[ef2->alt_index]);
   Cudd_RecursiveDeref(manager, tmp);

   DdNode *lm = Cudd_bddPermute(manager, label1, varmap);
   Cudd_Ref(lm);

    tmp = add_worlds_to_mutex(ef2->info_at[time]->exclusives->exlabel[ef1->alt_index],
			      lm);//label2, label1);
   
   Cudd_Ref(tmp);
   if(ef2->info_at[time]->exclusives->exlabel[ef1->alt_index])
   Cudd_RecursiveDeref(manager, ef2->info_at[time]->exclusives->exlabel[ef1->alt_index]);
   ef2->info_at[time]->exclusives->exlabel[ef1->alt_index] = tmp;
   Cudd_Ref(ef2->info_at[time]->exclusives->exlabel[ef1->alt_index]);
   Cudd_RecursiveDeref(manager, tmp);
   Cudd_RecursiveDeref(manager, lm);

   //    printBDD(ef1->info_at[time]->exclusives->exlabel[ef2->alt_index]);
   //    printBDD(ef2->info_at[time]->exclusives->exlabel[ef1->alt_index]);
    //printf("Exit make ef exclusive\n");
}
/**
 * momo007 2022.09.27
 * update the o1 and o2 exlabel by each other.
 */
void MAKE_OPS_EXCLUSIVE( int time, 
			 OpNode *o1, 
			 OpNode *o2, 
			 DdNode* label1//, DdNode* label2
			 ) 
{
  

  //  printf("Making ops %d mutex with %d at time %d with labels = \n", 
  // (o1->alt_index-1), (o2->alt_index-1), time);
//   printBDD(label1);
//   printBDD(label2);
 
  (o1->info_at[time]->exclusives->exclusives)[o2->uid_block] |= o2->uid_mask;
  
  (o2->info_at[time]->exclusives->exclusives)[o1->uid_block] |= o1->uid_mask;
  // config o1
  DdNode* tmp = add_worlds_to_mutex(o1->info_at[time]->exclusives->exlabel[o2->alt_index-1],
				    label1);//, label2);
  Cudd_Ref(tmp);
  if(o1->info_at[time]->exclusives->exlabel[o2->alt_index-1])
    Cudd_RecursiveDeref(manager, o1->info_at[time]->exclusives->exlabel[o2->alt_index-1]);
  o1->info_at[time]->exclusives->exlabel[o2->alt_index-1] = tmp;
  Cudd_Ref(o1->info_at[time]->exclusives->exlabel[o2->alt_index-1]);
  Cudd_RecursiveDeref(manager, tmp);
  // config o2
  DdNode *lm = Cudd_bddPermute(manager, label1, varmap);
  Cudd_Ref(lm);
  tmp = add_worlds_to_mutex(o2->info_at[time]->exclusives->exlabel[o1->alt_index-1],
			    lm);//label2, label1);
  Cudd_Ref(tmp);
  if(o2->info_at[time]->exclusives->exlabel[o1->alt_index-1])
    Cudd_RecursiveDeref(manager, o2->info_at[time]->exclusives->exlabel[o1->alt_index-1]);
  o2->info_at[time]->exclusives->exlabel[o1->alt_index-1] = tmp;
  Cudd_Ref(o2->info_at[time]->exclusives->exlabel[o1->alt_index-1]);
  Cudd_RecursiveDeref(manager, tmp);
  Cudd_RecursiveDeref(manager, lm);
/*     printf("exit make op ex\n"); */
}

void MAKE_FTS_EXCLUSIVE( int time, FtNode *f1, FtNode *f2, DdNode* label1)//, DdNode* label2 )

{

  BitVector *v1, *v2;
  DdNode** ex1, **ex2;

//    if(f1->positive && f2->positive){
//    printf("Mutex %d ", f1->positive); printFact(f1->index); 
//    printf(" and %d ", f2->positive); printFact(f2->index);
//    }
  //printf(" EXclusive at time %d across:\n", time); 
/*  printBDD(label1); */
/*  printBDD(label2); */


    //        printf("DID it\n");
    if(f1->positive){
      //	  printf("pos\n");
      v1 = ( ( f2->positive ) ? 
	     gft_table[f1->index]->info_at[time]->exclusives->pos_exclusives :
	     gft_table[f1->index]->info_at[time]->exclusives->neg_exclusives );
      ex1 = ( ( f2->positive ) ? 
	      gft_table[f1->index]->info_at[time]->exclusives->p_exlabel :
	      gft_table[f1->index]->info_at[time]->exclusives->n_exlabel );      
    }
    else {
      //	  printf("neg\n");
      v1 = ( ( f2->positive ) ? 
	     gft_table[NEG_ADR(f1->index)]->info_at[time]->exclusives->pos_exclusives :
	     gft_table[NEG_ADR(f1->index)]->info_at[time]->exclusives->neg_exclusives );
      ex1 = ( ( f2->positive ) ? 
	      gft_table[NEG_ADR(f1->index)]->info_at[time]->exclusives->p_exlabel :
	      gft_table[NEG_ADR(f1->index)]->info_at[time]->exclusives->n_exlabel );      
    }
    if(f2->positive){
      //	  printf("pos\n");
      
      v2 = ( ( f1->positive ) ? 
	     gft_table[f2->index]->info_at[time]->exclusives->pos_exclusives :
	     gft_table[f2->index]->info_at[time]->exclusives->neg_exclusives );
      ex2 = ( ( f1->positive ) ? 
	      gft_table[f2->index]->info_at[time]->exclusives->p_exlabel :
	      gft_table[f2->index]->info_at[time]->exclusives->n_exlabel );      
    }
    else {
      //	  printf("neg\n");
      v2 = ( ( f1->positive ) ? 
	     gft_table[NEG_ADR(f2->index)]->info_at[time]->exclusives->pos_exclusives :
	     gft_table[NEG_ADR(f2->index)]->info_at[time]->exclusives->neg_exclusives );
      ex2 = ( ( f1->positive ) ? 
	      gft_table[NEG_ADR(f2->index)]->info_at[time]->exclusives->p_exlabel :
	      gft_table[NEG_ADR(f2->index)]->info_at[time]->exclusives->n_exlabel );      
    }
    
    v1[f2->uid_block] |= f2->uid_mask;
    v2[f1->uid_block] |= f1->uid_mask;
 
    
    DdNode* tmp = add_worlds_to_mutex(ex1[f2->index], label1);//, label2);
    Cudd_Ref(tmp);
    if( ex1[f2->index])
    Cudd_RecursiveDeref(manager, ex1[f2->index]);
    ex1[f2->index] = tmp;
    Cudd_Ref(ex1[f2->index]);
    Cudd_RecursiveDeref(manager, tmp);

   DdNode *lm = Cudd_bddPermute(manager, label1, varmap);
   Cudd_Ref(lm);


   tmp = add_worlds_to_mutex(ex2[f1->index], lm);//label1, label2);
   Cudd_Ref(tmp);
   if(ex2[f1->index])
     Cudd_RecursiveDeref(manager, ex2[f1->index]);
   ex2[f1->index] = tmp;
   Cudd_Ref(ex2[f1->index]);
   Cudd_RecursiveDeref(manager, tmp);
   Cudd_RecursiveDeref(manager, lm);
}



							
void MAKE_OPS_UNEXCLUSIVE( int time, OpNode *o1, OpNode *o2, DdNode* label1//, 
			   //DdNode* label2
 )

{
  //  printf("Making UNEXclusive at time %d\n", time);
  //printBDD(label1);
  DdNode* new_mutex = Cudd_bddAnd(manager,
				  o1->info_at[time]->exclusives->exlabel[o2->alt_index-1],
				  Cudd_Not(label1));
  Cudd_Ref(new_mutex);
  //  printBDD(new_mutex);
  Cudd_RecursiveDeref(manager, o1->info_at[time]->exclusives->exlabel[o2->alt_index-1]);
  o1->info_at[time]->exclusives->exlabel[o2->alt_index-1] = new_mutex;
  Cudd_Ref(o1->info_at[time]->exclusives->exlabel[o2->alt_index-1]);
  if(new_mutex == Cudd_ReadLogicZero(manager)){ 
    (o1->info_at[time]->exclusives->exclusives)[o2->uid_block] &= ~(o2->uid_mask);
    (o2->info_at[time]->exclusives->exclusives)[o1->uid_block] &= ~(o1->uid_mask);  
  }
  // 将后继状态变量替换为当前状态变量
  DdNode *t = Cudd_bddVarMap(manager, new_mutex);
  Cudd_Ref(t);
  //  printBDD(t);

  o2->info_at[time]->exclusives->exlabel[o1->alt_index-1] = t;
  Cudd_Ref(o2->info_at[time]->exclusives->exlabel[o1->alt_index-1]);

  Cudd_RecursiveDeref(manager, new_mutex);
  Cudd_RecursiveDeref(manager, t);
}
void MAKE_EFS_UNEXCLUSIVE( int time, EfNode *o1, EfNode *o2, DdNode* label1//, 
			   //DdNode* label2
 )

{
  //  printf("Making %s and %s UNEXclusive at time %d\n",o1->name, o2->name, time);

  DdNode* new_mutex = Cudd_bddAnd(manager,
				  o1->info_at[time]->exclusives->exlabel[o2->index],
				  Cudd_Not(label1));
  Cudd_Ref(new_mutex);
  Cudd_RecursiveDeref(manager, o1->info_at[time]->exclusives->exlabel[o2->index]);
  o1->info_at[time]->exclusives->exlabel[o2->index] = new_mutex;
  Cudd_Ref(o1->info_at[time]->exclusives->exlabel[o2->index]);
  if(new_mutex == Cudd_ReadLogicZero(manager)){
    (o1->info_at[time]->exclusives->exclusives)[o2->uid_block] &= ~(o2->uid_mask);
    (o2->info_at[time]->exclusives->exclusives)[o1->uid_block] &= ~(o1->uid_mask);  
  }
  DdNode *t = Cudd_bddVarMap(manager, new_mutex);
  Cudd_Ref(t);
  o2->info_at[time]->exclusives->exlabel[o1->index] = t;
  Cudd_Ref(o2->info_at[time]->exclusives->exlabel[o1->index]);

  Cudd_RecursiveDeref(manager, new_mutex);
  Cudd_RecursiveDeref(manager, t);
 
}

void MAKE_FTS_UNEXCLUSIVE( int time, FtNode *f1, FtNode *f2 , DdNode* label1//, DdNode* label2
			   )

{

  BitVector *v1, *v2;
  ExclusionLabelPair *tmpelp, *prevelp = NULL;
  
 
  //  printf("Making %d ", f1->positive); printFact(f1->index); printf(" and %d ", f2->positive); printFact(f2->index); printf(" UNEXclusive at time %d\n", time);
  //   printBDD(label1);

  bool set = false;
 
    DdNode* new_mutex;
    if(f2->positive){
      new_mutex = Cudd_bddAnd(manager,
			      f1->info_at[time]->exclusives->p_exlabel[f2->index],
			      label1);
			      //Cudd_Not(label1));
      Cudd_Ref(new_mutex);      
      if(Cudd_ReadLogicZero(manager) == new_mutex)
	set = true;

      Cudd_RecursiveDeref(manager, f1->info_at[time]->exclusives->p_exlabel[f2->index]);
      f1->info_at[time]->exclusives->p_exlabel[f2->index] = new_mutex;
      Cudd_Ref(f1->info_at[time]->exclusives->p_exlabel[f2->index]);
      Cudd_RecursiveDeref(manager, new_mutex);
    }
    else{
      new_mutex = Cudd_bddAnd(manager,
			      f1->info_at[time]->exclusives->n_exlabel[f2->index],
			      label1);
			      //Cudd_Not(label1));
      Cudd_Ref(new_mutex);
      if(Cudd_ReadLogicZero(manager) == new_mutex)
	set = true;
      Cudd_RecursiveDeref(manager, f1->info_at[time]->exclusives->n_exlabel[f2->index]);
      f1->info_at[time]->exclusives->n_exlabel[f2->index] = new_mutex;
      Cudd_Ref(f1->info_at[time]->exclusives->n_exlabel[f2->index]);
      Cudd_RecursiveDeref(manager, new_mutex);
    }


    DdNode *label2 = Cudd_bddVarMap(manager, label1);
    Cudd_Ref(label2);
    if(f1->positive){
      new_mutex = Cudd_bddAnd(manager,
			      f2->info_at[time]->exclusives->p_exlabel[f1->index],
			      label2);
			      //Cudd_Not(label2));
      Cudd_Ref(new_mutex);
      if(Cudd_ReadLogicZero(manager) == new_mutex)
	set = true;
      Cudd_RecursiveDeref(manager, f2->info_at[time]->exclusives->p_exlabel[f1->index]);
      f2->info_at[time]->exclusives->p_exlabel[f1->index] = new_mutex;
      Cudd_Ref(f2->info_at[time]->exclusives->p_exlabel[f1->index]);
      Cudd_RecursiveDeref(manager, new_mutex);
    }
    else{
      new_mutex = Cudd_bddAnd(manager,
			      f2->info_at[time]->exclusives->n_exlabel[f1->index],
			      label2);
			      //Cudd_Not(label2));
      Cudd_Ref(new_mutex);
      if(Cudd_ReadLogicZero(manager) == new_mutex)
	set = true;
      Cudd_RecursiveDeref(manager, f2->info_at[time]->exclusives->n_exlabel[f1->index]);
      f2->info_at[time]->exclusives->n_exlabel[f1->index] = new_mutex;
      Cudd_Ref(f2->info_at[time]->exclusives->n_exlabel[f1->index]);
      Cudd_RecursiveDeref(manager, new_mutex);
    }
    Cudd_RecursiveDeref(manager, label2);

  

    if(set){
      v1 = ( ( f2->positive ) ? f1->info_at[time]->exclusives->pos_exclusives :
	     f1->info_at[time]->exclusives->neg_exclusives );
      v2 = ( ( f1->positive ) ? f2->info_at[time]->exclusives->pos_exclusives :
	     f2->info_at[time]->exclusives->neg_exclusives );
      
      v1[f2->uid_block] &= ~(f2->uid_mask);
      v2[f1->uid_block] &= ~(f1->uid_mask);
    }

    
}



DdNode* ARE_MUTEX_OPS( int time, OpNode *o1, OpNode *o2, DdNode* label1, DdNode* label2 )

{

  if(MUTEX_SCHEME == MS_NONE)
    return Cudd_ReadLogicZero(manager);

  // printf("ARE_MUTEX_OPS %d, %d\n", (o1->alt_index-1), (o2->alt_index-1));
  if(o1->info_at[time]){
    DdNode* plabel2 = Cudd_bddVarMap(manager, label2);
    Cudd_Ref(plabel2);
    DdNode* worlds = Cudd_bddAnd(manager, label1, plabel2);
    Cudd_Ref(worlds);    
    DdNode* mutex = Cudd_bddAnd(manager, worlds, 
				o1->info_at[time]->exclusives->exlabel[o2->alt_index-1]);
    Cudd_Ref(mutex);
    Cudd_RecursiveDeref(manager, plabel2);
    Cudd_RecursiveDeref(manager, worlds);    
    return mutex;
  }
  else
    return Cudd_ReadLogicZero(manager);

 //  if ( o1->info_at[time]->exclusives->exclusives[o2->uid_block] & o2->uid_mask ) {

  
//     return Cudd_ReadOne(manager);
//       //}
//   } else {
//     return Cudd_ReadLogicZero(manager);
//   }

}
//int
DdNode* ARE_MUTEX_EFS( int time, EfNode *e1, EfNode *e2, DdNode* label1, DdNode* label2 )

{
  
  //   std::cout << "are_mutex_efs" << std::endl;
  //   if ( get_bit(e1->info_at[time]->exclusives->exclusives,gef_vector_length, e2->alt_index) ) {

  //if(e1->info_at[time]->exclusives->exlabel[e2->alt_index] == NULL)
    //    std::cout << "NULLMUTEX" << std::endl;

  if(MUTEX_SCHEME == MS_NONE)
    return Cudd_ReadLogicZero(manager);


 if(e1->info_at[time]){
    DdNode* plabel2 = Cudd_bddVarMap(manager, label2);
    Cudd_Ref(plabel2);
    DdNode* worlds = Cudd_bddAnd(manager, label1, plabel2);
    Cudd_Ref(worlds);    
    DdNode* mutex = Cudd_bddAnd(manager, worlds, 
				e1->info_at[time]->exclusives->exlabel[e2->alt_index]);
    Cudd_Ref(mutex);
    Cudd_RecursiveDeref(manager, plabel2);
    Cudd_RecursiveDeref(manager, worlds);    
    return mutex;
  }
  else
    return Cudd_ReadLogicZero(manager);


}

//int 
DdNode* ARE_MUTEX_FTS( int time, FtNode *f1, FtNode *f2, DdNode* label1, DdNode* label2 )

{

  if(MUTEX_SCHEME == MS_NONE)
    return Cudd_ReadLogicZero(manager);

  BitVector *a;
  DdNode* l;
  if(f1 && f1->info_at[time] && f2){
    a  = ( f2->positive  ?
	   f1->info_at[time]->exclusives->pos_exclusives :
	   f1->info_at[time]->exclusives->neg_exclusives);
    l =  ( f2->positive  ?
	   f1->info_at[time]->exclusives->p_exlabel[f2->index] :
	   f1->info_at[time]->exclusives->n_exlabel[f2->index]);
  }
  else
    return Cudd_ReadLogicZero(manager);//FALSE;
  
 
 
  if ( a[f2->uid_block] & f2->uid_mask ) {
    DdNode* plabel2 = Cudd_bddVarMap(manager, label2);
    Cudd_Ref(plabel2);
    DdNode* worlds = Cudd_bddAnd(manager, label1, plabel2);
    Cudd_Ref(worlds);    
    DdNode* mutex = Cudd_bddAnd(manager, worlds, l);
    Cudd_Ref(mutex);
    Cudd_RecursiveDeref(manager, plabel2);
    Cudd_RecursiveDeref(manager, worlds);    
    return mutex;

  } else {
    //    printf("not mutex fts\n");
    return Cudd_ReadLogicZero(manager);//FALSE;
  }

}





//end dan





/*****************************************************************
 * THE TWO MAIN FUNCIONS, CALLED FROM BUILD_GRAPH_EVOLUTION_STEP *        
 *****************************************************************/










void find_mutex_ops( int time )

{

  OpNode *i1, *i2;
  DdNode* label;
  OpPair *i, *tmp, *prev;
  DdNode** dda;
  DdNode** ddb;
  DdNode* tmpdd;
  int sizea, sizeb;
  ExclusionLabelPair* tmpelp, *tmpelp1 = NULL;

  ExclusionLabelPair* tmp1;
  
  BitVector *a, *b;
  int r;

  // printf("Enter find mutex ops, vec len = %d %d\n",gop_vector_length_at[time], gops_count);    
  DdNode *competeMutex;
  DdNode* worlds1, *worlds2;
  // 复制上一层time-1的exclusive vector到当前层
  for (i1=gprev_level_ops_pointer ; i1; i1=i1->next ) {
    // printf("New OpExclusion for %s\n",i1->name);
    // i1->info_at[time]->exclusives = new_OpExclusion(gop_vector_length_at[time]);// new_excl_bit_vector( gop_vector_length_at[time] );
    if ( time > 0 && i1->info_at[time-1] && i1->info_at[time] ) {
      a = i1->info_at[time]->exclusives->exclusives;
      b = i1->info_at[time-1]->exclusives->exclusives;
      // 将上一层opNode的mutex加入到当前层opNode
      for ( r = 0; r < gop_vector_length_at[time-1]; r++ ) {
        a[r] |= b[r];
      }
      // 将上一层opNode的mutex label加入到当前层opNode
      for ( r = 0; r < (num_alt_acts+2*num_alt_facts); r++ ) {
        i1->info_at[time]->exclusives->exlabel[r] = i1->info_at[time-1]->exclusives->exlabel[r];
        if(i1->info_at[time]->exclusives->exlabel[r])
          Cudd_Ref(i1->info_at[time]->exclusives->exlabel[r]);
      }

    }
  }//end-for

  if(time > 0) {    
    i = gop_mutex_pairs;
    // 对现有的mutex pair进行化简处理
    while (i) {
      //std::cout << "HI" << std::endl;
      if(MUTEX_SCHEME!=MS_NONE){
        worlds1 = i->o1->info_at[time]->label->label;
        worlds2 = i->o2->info_at[time]->label->label;
      }
      else {
        worlds1 = Cudd_ReadOne(manager);
        worlds2 = Cudd_ReadOne(manager);
      }
     
      // 计算和op2互斥的label加入到op1的互斥label中返回
      // 两个动作没有mutex则返回BDD 0
      competeMutex = competing_needs( time, i->o1, i->o2, worlds1, worlds2);
      Cudd_Ref(competeMutex);
      // 仅考虑没有mutex的状态去更新label
      MAKE_OPS_UNEXCLUSIVE( time, i->o1, i->o2, Cudd_Not(competeMutex) );
      if(i->o1->info_at[time]->exclusives->exlabel[i->o2->alt_index-1] == Cudd_ReadLogicZero(manager))
      {
        tmp = i;
        i = i->next;
	      free( tmp );
	      num_op_pair--;
#ifdef MEMORY_INFO
	      gexcl_memory -= sizeof( OpPair );
#endif
	      gops_exclusions_count--;

      }
    }// end-while 1
    // why do it again !?!??!
    gop_mutex_pairs = i;
    prev = i;
    if ( i ) i = i->next;
    while ( i ) {
    if(MUTEX_SCHEME!=MS_NONE){
      worlds1 = i->o1->info_at[time]->label->label;
      worlds2 = i->o2->info_at[time]->label->label;
    }
    else {
      worlds1 = Cudd_ReadOne(manager);
      worlds2 = Cudd_ReadOne(manager);
    }
    competeMutex = competing_needs( time, i->o1, i->o2, worlds1, worlds2 );
    Cudd_Ref(competeMutex);
    MAKE_OPS_UNEXCLUSIVE( time, i->o1, i->o2 , Cudd_Not(competeMutex));
    if (i->o1->info_at[time]->exclusives->exlabel[i->o2->alt_index-1] == Cudd_ReadLogicZero(manager))
    {
	    prev->next = i->next;
	    tmp = i;
	    i = i->next;
	    free( tmp );
	    num_op_pair--;
#ifdef MEMORY_INFO
	    gexcl_memory -= sizeof( OpPair );
#endif
	    gops_exclusions_count--;
      } 
      else
      {
	      prev = prev->next;
        i = i->next;
      }
    }// end-while 2
  }// end-if time > 0
  // create the next mutex layer
  for ( i1 = gall_ops_pointer; i1 != gprev_level_ops_pointer; i1 = i1->next ) {
    for ( i2 = i1->next; i2; i2 = i2->next ) {
      // std::cout << (i1->alt_index-1) << " " << (i2->alt_index-1) << std::endl;
      if ( interfere( i1, i2 ) ) {// 1. exclusive between effect and precond
        // printf("interfere\n");
        MAKE_OPS_EXCLUSIVE( time, i1, i2, Cudd_ReadOne(manager));//, worlds2 );
        gops_exclusions_count++;
        continue;
      }
      if ( noop_exclusive( i1, i2 ) ) {// 2. exclusive between noop and op
        // printf("noop exclusive\n");
        MAKE_OPS_EXCLUSIVE( time, i1, i2 , Cudd_ReadOne(manager));//, worlds2);
        gops_exclusions_count++;
        continue;
      }
      if (time > 0){// 第1层开始需要判断label是否exclusive
        // 计算两者互斥的状态label
	      DdNode *mutex = competing_needs( time, i1, i2, 
					 i1->info_at[time]->label->label,
					 i2->info_at[time]->label->label);
	      Cudd_Ref(mutex);
        // 存在mutex label,添加mutex pair
	      if(mutex != Cudd_ReadLogicZero(manager)){
	        // printf("compete\n");
          MAKE_OPS_EXCLUSIVE( time, i1, i2, mutex);//, NULL );	  
          gops_exclusions_count++;
          tmp = new_op_pair( i1, i2 );
          tmp->next = gop_mutex_pairs;
          gop_mutex_pairs = tmp;
        }
      }
    }
  }//end-for
    
  // printf("Exit find mutex ops\n");

}




void find_mutex_facts( int time )

{

  FtNode *i1, *i2;
  EfEdge *e;
  FtPair *i, *prev, *tmp;
  BitVector *a, *b;
  int r, j;
  ExclusionLabelPair *tmpPair1, *tmpPair2 = NULL;
  DdNode** dda;
  DdNode** ddb;

  DdNode* tmpdd, *worlds1, *worlds2, *mutex;
  int sizea, sizeb;
   


     
  if(MUTEX_SCHEME!=MS_NONE){
   //printf("Enter find mutex facts %d\n", time);


    for ( i1 = gprev_level_fts_pointer; i1; i1=i1->next ) {
    
 //    if(!(i1->info_at[time])) i1->info_at[time]= new_ft_level_info( i1 );//printf("GOT NULL\n");
//     if ( i1->info_at[time]->is_dummy ) {
//       printf( "\nein dummy in der alten liste ??\n\n" );
//       exit( 1 );
//     }
//     if ( i1->info_at[time-1]->is_dummy ) {
//       printf( "\nein ex-dummy in der alten liste ??\n\n" );
//       exit( 1 );
//     }
   
   
    // cout << "set adders " << (time) << std::endl; printFact(i1->index);
    //   i1->info_at[time]->adders = new_excl_bit_vector( gop_vector_length_at[time-1] );
  
      if ( time > 0 ) {
      /*
       * achtung! evtl. bei time > 0 die exclusives bereits kopieren;
       * wegen contradicting facts... im augenblick egal, da diese bits
       * bereits in new_ft_level_info gesetzt werden.
       */
        a = i1->info_at[time]->exclusives->pos_exclusives;
        b = i1->info_at[time-1]->exclusives->pos_exclusives;
        for ( r = 0; r < gft_vector_length; r++ ) {
          a[r] |= b[r];
        }
        for ( r = 0; r < (num_alt_facts); r++ ) {
          i1->info_at[time]->exclusives->p_exlabel[r] = i1->info_at[time-1]->exclusives->p_exlabel[r];
 	        if(i1->info_at[time]->exclusives->p_exlabel[r])
	          Cudd_Ref(i1->info_at[time]->exclusives->p_exlabel[r]);
        }

        a = i1->info_at[time]->exclusives->neg_exclusives;
        b = i1->info_at[time-1]->exclusives->neg_exclusives;
        for ( r = 0; r < gft_vector_length; r++ ) {
	        a[r] |= b[r];
        }
        for ( r = 0; r < (num_alt_facts); r++ ) {
	        i1->info_at[time]->exclusives->n_exlabel[r] = i1->info_at[time-1]->exclusives->n_exlabel[r];
 	        if(i1->info_at[time]->exclusives->n_exlabel[r])
	          Cudd_Ref(i1->info_at[time]->exclusives->n_exlabel[r]);
        }
//       a = i1->info_at[time]->adders;
//       b = i1->info_at[time-1]->adders;
//       for ( r = 0; r < gop_vector_length_at[time-2]; r++ ) {
// 	a[r] |= b[r];
//       }
      }

  
//     if ( i1->noop ) {
//       (i1->info_at[time]->adders)[i1->noop->uid_block] |= i1->noop->uid_mask;
//     }
   

//     for ( e = i1->adders; e && e->ef->first_occurence == time-1; e = e->next ) {
//       if ( e->ef->info_at[time-1]->is_dummy ) continue;
      
//       (i1->info_at[time]->adders)[e->ef->op->uid_block] |= e->ef->op->uid_mask;
      

//     }
//     i1->info_at[time]->adders_pointer = i1->adders;
  }

  i = gft_mutex_pairs;
  while ( i ){
    //std::cout << "check un mutex" << std::endl;
    if(MUTEX_SCHEME!=MS_NONE){
      worlds1 = i->f1->info_at[time]->label->label;
      worlds2 = i->f2->info_at[time]->label->label;
    }
    else {
      worlds1 = Cudd_ReadOne(manager);
      worlds2 = Cudd_ReadOne(manager);
    }
    mutex = facts_are_exclusive( time, i->f1, i->f2, worlds1,worlds2 );
    Cudd_Ref(mutex);

    MAKE_FTS_UNEXCLUSIVE( time, i->f1, i->f2, mutex );
    if(mutex == Cudd_ReadLogicZero(manager)){
      gexclusions_count--;
      if ( i->f1->positive && i->f2->positive ) {
        gprint_exnum--;
      }
      tmp = i;
      i = i->next;
      free( tmp );
      num_ft_pair--;
#ifdef MEMORY_INFO
      gexcl_memory -= sizeof( FtPair );
#endif
    }
    else
      break;
  }// end-while
  

  gft_mutex_pairs = i;
  prev = i;
  if ( i ) i = i->next;
  while ( i ) {
    if(MUTEX_SCHEME!=MS_NONE){
      worlds1 = i->f1->info_at[time]->label->label;
      worlds2 = i->f2->info_at[time]->label->label;
    }
    else {
      worlds1 = Cudd_ReadOne(manager);
      worlds2 = Cudd_ReadOne(manager);
    }
    mutex = facts_are_exclusive( time, i->f1, i->f2, worlds1, worlds2);
    Cudd_Ref(mutex);


    MAKE_FTS_UNEXCLUSIVE( time, i->f1, i->f2, mutex );
    if (mutex != Cudd_ReadLogicZero(manager) ){
      gexclusions_count--;
      if ( i->f1->positive && i->f2->positive ) {
        gprint_exnum--;
      }
      prev->next = i->next;
      tmp = i;
      i = i->next;
      free( tmp );
      num_ft_pair--;
#ifdef MEMORY_INFO
      gexcl_memory -= sizeof( FtPair );
#endif
    } else {
      prev = prev->next;
      i = i->next;
    }
  }// end-while
 
  //  std::cout << "find new ft mutexes" << std::endl;
 
    
  for ( i1 = gall_fts_pointer; i1 && i1->info_at[time]->updated //!= gprev_level_fts_pointer
	  ; i1 = i1->next ) {
    for ( i2 = i1; i2  // != gprev_level_fts_pointer
	    ; i2 = i2->next ) {

      /* siehe oben
       */
      if ( !i2 || !i2->info_at[time] || i2->info_at[time]->is_dummy ) continue;
//       if ( i1->index == i2->index ) {
// 	/* ==> contradicting facts!
// 	 *
// 	 * in dieser implemetierung WICHTIG: ft s muessen verschieden sein!
// 	 */
// 	continue;
//       }
      if(MUTEX_SCHEME!=MS_NONE){
	      worlds1 = i1->info_at[time]->label->label;
	      worlds2 = i2->info_at[time]->label->label;
      }
      else {
	      worlds1 = Cudd_ReadOne(manager);
	      worlds2 = Cudd_ReadOne(manager);
      }
 //      std::cout << "Check ft pair "
//        		<< i1->positive << " " << i1->index << " " 
//        		<< i2->positive << " " << i2->index << std::endl;
      mutex = facts_are_exclusive( time, i1, i2, worlds1, worlds2);
      Cudd_Ref(mutex);

      if (mutex == Cudd_ReadLogicZero(manager) ){
	      continue;
      }
      //      printBDD(mutex);


      //std::cout << "Mutex Facts" << std::endl;
      MAKE_FTS_EXCLUSIVE( time, i1, i2, mutex);//, mutex);
      if ( i1->positive && i2->positive ) {
	      gprint_exnum++;
      }
      gexclusions_count++;
      tmp = new_ft_pair( i1, i2 );
      tmp->next = gft_mutex_pairs;
      gft_mutex_pairs = tmp;
    }
  }
 
  }// end if MS != NONE

 // printf("Exit find mutex facts\n");
  
}








/**********************************************
 * HELPERS ON RELATIONS BETWEEN OPS AND FACTS *
 **********************************************/







//need to generalize for effects
//int 
DdNode* ef_competing_needs( int time, EfNode *e1, EfNode *e2, 
			    DdNode* labela, DdNode* labelb ){

  //  std::cout << "ef comp" << std::endl;
  BitVector *p = e1->info_at[time]->exclusives->pos_exclusives;
  BitVector *n = e1->info_at[time]->exclusives->neg_exclusives;

  BitVector *bp, *bn;
  int r;
  FtEdge *i;

  if(time == 0)
    return Cudd_ReadLogicZero(manager);//FALSE;
  if (MUTEX_SCHEME != MS_NONE){
    if ( !p && !n  ) {
      
      p = new_excl_bit_vector( gft_vector_length );
      n = new_excl_bit_vector( gft_vector_length );
      for ( i = e1->conditions; i; i = i->next ) {
 	bp = i->ft->info_at[time]->exclusives->pos_exclusives;
 	for ( r = 0; r < gft_vector_length; r++ ) {
 	  p[r] |= bp[r];
 	}
	//find worlds where mutex
	//	if(MUTEX_SCHEME != MS_REGULAR){
	  for(r = 0; r < num_alt_facts; r++){
	    if(get_bit(p, gft_vector_length, r)){
	      //      std::cout << "+ " << r << std::endl;
	      //	      printBDD(i->ft->info_at[time]->exclusives->p_exlabel[r]);
	      DdNode *tmp = Cudd_bddAnd(manager, e1->info_at[time]->label->label,
					i->ft->info_at[time]->exclusives->p_exlabel[r]);
	      Cudd_Ref(tmp);
	      add_worlds_to_mutex(e1->info_at[time]->exclusives->p_exlabel[r],
				  tmp);
	      Cudd_RecursiveDeref(manager, tmp);				
	    }
	  }
	  //	}
 	bn = i->ft->info_at[time]->exclusives->neg_exclusives;
 	for ( r = 0; r < gft_vector_length; r++ ) {
 	  n[r] |= bn[r];
 	}
	//find worlds where mutex
	//	if(MUTEX_SCHEME != MS_REGULAR){
	  for(r = 0; r < num_alt_facts; r++){
	    if(get_bit(n, gft_vector_length, r)){
	      ///	    std::cout << "- " << r << std::endl;
	      DdNode *tmp = Cudd_bddAnd(manager, e1->info_at[time]->label->label,
					i->ft->info_at[time]->exclusives->n_exlabel[r]);
	      Cudd_Ref(tmp);
	      add_worlds_to_mutex(e1->info_at[time]->exclusives->n_exlabel[r],
				  tmp);
	    }
	  }
	  //	}
       }
       e1->info_at[time]->exclusives->pos_exclusives = p;
       e1->info_at[time]->exclusives->neg_exclusives = n;
     }
    
    //if o2's preconds are in the mutex list for o1 then xor
    bp = e2->effect->ant->p_conds->vector;
    bn = e2->effect->ant->p_conds->vector;
    for ( r = 0; r < gft_vector_length; r++ ) {
      if ( (p[r] & bp[r]) || (n[r] & bn[r]) ) {
	//printf("COMPETE!!!!a\n");
	DdNode* mutex = Cudd_ReadLogicZero(manager);

	for(int i = 0; i < num_alt_facts; i++){
	  if(get_bit(p, gft_vector_length, i) &&
	     get_bit(bp, gft_vector_length, i)){
	    DdNode* tmpp = Cudd_bddPermute(manager, 	      
					   e2->info_at[time]->label->label,
					   varmap);
	    Cudd_Ref(tmpp);
	    DdNode *tmp2 = Cudd_bddAnd(manager, tmpp, 
				       e1->info_at[time]->exclusives->p_exlabel[i]);
	    Cudd_Ref(tmp2);
	    
	    DdNode *mtmp = Cudd_bddOr(manager, mutex, tmp2);
	    Cudd_Ref(mtmp);
	    mutex = mtmp;
	    Cudd_Ref(mutex);
	    Cudd_RecursiveDeref(manager, mtmp);
	    Cudd_RecursiveDeref(manager, tmp2);
 	    Cudd_RecursiveDeref(manager, tmpp);
	  }
	  if(get_bit(n, gft_vector_length, i) &&
	     get_bit(bn, gft_vector_length, i)){
	    DdNode* tmpp = Cudd_bddPermute(manager, 	      
					   e2->info_at[time]->label->label,
					   varmap);
	    Cudd_Ref(tmpp);
	    DdNode *tmp2 = Cudd_bddAnd(manager, tmpp, 
				       e1->info_at[time]->exclusives->n_exlabel[i]);
	    Cudd_Ref(tmp2);
	   
	    DdNode *mtmp = Cudd_bddOr(manager, mutex, tmp2);
	    Cudd_Ref(mtmp);
	    mutex = mtmp;
	    Cudd_Ref(mutex);
	    Cudd_RecursiveDeref(manager, mtmp);
	    Cudd_RecursiveDeref(manager, tmp2);
 	    Cudd_RecursiveDeref(manager, tmpp);
	  }
	}
	
	return mutex;//Cudd_ReadOne(manager);//TRUE;
      }
    }  
  }
  //  printf("Exit\n");
  return Cudd_ReadLogicZero(manager);


  
}


//int 
DdNode* competing_needs( int time, OpNode *o1, OpNode *o2, 
		     DdNode* labela, DdNode* labelb )

{

  BitVector *p = o1->unconditional->info_at[time]->exclusives->pos_exclusives;
  BitVector *n = o1->unconditional->info_at[time]->exclusives->neg_exclusives;



  BitVector *bp, *bn;
  int r;
  FtEdge *i;
  // printf("Checking competing needs for %d and %d at %d: \n", (o1->alt_index-1), 
  // (o2->alt_index-1), time);
  
  if(time == 0)// momo007 2022.09.27
    return FALSE; //// return Cudd_ReadLogicZero(manager); will increase the expand count.

   //record which propositions are exclusive with preconds
  if (!p && !n) {
    p = new_excl_bit_vector( gft_vector_length );
    n = new_excl_bit_vector( gft_vector_length );
    for ( i = o1->preconds; i; i = i->next ) {
      // 1.考虑pos
      bp = i->ft->info_at[time]->exclusives->pos_exclusives;
      for ( r = 0; r < gft_vector_length; r++ ) {
        p[r] |= bp[r];
 	    }
	    //find worlds where mutex
	    if(MUTEX_SCHEME != MS_REGULAR){
	      for(r = 0; r < num_alt_facts; r++){
	        if(get_bit(p, gft_vector_length, r)){
            // 将fact的exlabel加到opNode上
	          DdNode *tmp = Cudd_bddAnd(manager, o1->info_at[time]->label->label,
					                            i->ft->info_at[time]->exclusives->p_exlabel[r]);
	          Cudd_Ref(tmp);
	          add_worlds_to_mutex(o1->unconditional->info_at[time]->exclusives->p_exlabel[r], tmp);
	          Cudd_RecursiveDeref(manager, tmp);				
	        }
	      }
	    }
      // 2.考虑neg
 	    bn = i->ft->info_at[time]->exclusives->neg_exclusives;
 	    for ( r = 0; r < gft_vector_length; r++ ) {
 	      n[r] |= bn[r];
 	    }
	    //find worlds where mutex
	    if(MUTEX_SCHEME != MS_REGULAR){
	      for(r = 0; r < num_alt_facts; r++){
	        if(get_bit(n, gft_vector_length, r)){
	          DdNode *tmp = Cudd_bddAnd(manager, o1->info_at[time]->label->label,
					                            i->ft->info_at[time]->exclusives->n_exlabel[r]);
	          Cudd_Ref(tmp);
	          add_worlds_to_mutex(o1->unconditional->info_at[time]->exclusives->n_exlabel[r], tmp);
	        }
	      }
	    }
    }// end-for o1->preconds
    // set the exclusive vector
    o1->unconditional->info_at[time]->exclusives->pos_exclusives = p;
    o1->unconditional->info_at[time]->exclusives->neg_exclusives = n;
  }// endif !p && !n
    
  //if o2's preconds are in the mutex list for o1 then xor
  bp = o2->pos_precond_vector;
  bn = o2->neg_precond_vector;
  for ( r = 0; r < gft_vector_length; r++ ) {
    if ( (p[r] & bp[r]) || (n[r] & bn[r]) ) {
      //printf("COMPETE!!!!a\n");
	    DdNode* mutex = Cudd_ReadLogicZero(manager);
	    for(int i = 0; i < num_alt_facts; i++){
	      if(get_bit(p, gft_vector_length, i) &&
	          get_bit(bp, gft_vector_length, i)){
	        DdNode* tmpp = Cudd_bddPermute(manager, o2->info_at[time]->label->label, varmap);
	        Cudd_Ref(tmpp);
	        DdNode *tmp2 = Cudd_bddAnd(manager, tmpp, o1->unconditional->info_at[time]->exclusives->p_exlabel[i]);
	        Cudd_Ref(tmp2);
	        DdNode *mtmp = Cudd_bddOr(manager, mutex, tmp2);
          Cudd_Ref(mtmp);
          mutex = mtmp;
          Cudd_Ref(mutex);
          Cudd_RecursiveDeref(manager, mtmp);
	        Cudd_RecursiveDeref(manager, tmp2);
 	        Cudd_RecursiveDeref(manager, tmpp);
	      }
	      if(get_bit(n, gft_vector_length, i) &&
	          get_bit(bn, gft_vector_length, i)){
	        DdNode* tmpp = Cudd_bddPermute(manager, o2->info_at[time]->label->label, varmap);
	        Cudd_Ref(tmpp);
	        DdNode *tmp2 = Cudd_bddAnd(manager, tmpp, o1->unconditional->info_at[time]->exclusives->n_exlabel[i]);
	        Cudd_Ref(tmp2);
	        DdNode *mtmp = Cudd_bddOr(manager, mutex, tmp2);
	        Cudd_Ref(mtmp);
	        mutex = mtmp;
	        Cudd_Ref(mutex);
	        Cudd_RecursiveDeref(manager, mtmp);
	        Cudd_RecursiveDeref(manager, tmp2);
 	        Cudd_RecursiveDeref(manager, tmpp);
	      }
	    }// end-for one mutex compute
      // 计算得到和 op1和op2里，和op1冲突的label BDD状态集合。
	    return mutex;//Cudd_ReadOne(manager);//TRUE;
    }
  }// end-for all mutex check

 //    b = o2->neg_precond_vector;
//     for ( r = 0; r < gft_vector_length; r++ ) {
//       if ( n[r] & b[r] ) {
// 	//printf("COMPETE!!!!b\n");
// 	return Cudd_ReadOne(manager);//TRUE;
//       }
//     }
    //printf("NO COMPETE!!!!\n");
  return Cudd_ReadLogicZero(manager);
      //FALSE;
}

int ef_interfere( EfNode *e1, EfNode *e2 ){

//      printf("Enter EF_Interfere\n");

  BitVector *e1p = e1->effect->cons->p_effects->vector;//pos_effect_vector;
  BitVector *e1n = e1->effect->cons->n_effects->vector;//neg_effect_vector;
  BitVector *e2p = e2->effect->cons->p_effects->vector;//pos_effect_vector;
  BitVector *e2n = e2->effect->cons->n_effects->vector;//neg_effect_vector;

  //need to turn cond eff preconds into bit vectors from lists
  //or can use getbit to see if it is in the effs vector
  BitVector *p1p = e1->effect->ant->p_conds->vector;
  BitVector *p1n = e1->effect->ant->n_conds->vector; //this is getting the worng value for some stuff
  BitVector *p2p = e2->effect->ant->p_conds->vector;
  BitVector *p2n = e2->effect->ant->n_conds->vector;

  int r;


//   print_BitVector(e1p, gft_vector_length); 
//   print_BitVector(e1n, gft_vector_length);
//   std::cout << std::flush;
//   print_BitVector(e2p, gft_vector_length);
//   print_BitVector(e2n, gft_vector_length); 
//   std::cout << std::flush;
//    print_BitVector(p1p, gft_vector_length); 
//    print_BitVector(p1n, gft_vector_length); 
//    std::cout << std::flush;
//   print_BitVector(p2p, gft_vector_length); 
//   print_BitVector(p2n, gft_vector_length); 
//   std::cout << std::flush;

  //clobbering a precondition is okay if they are same action


  for ( r = 0; r < gft_vector_length; r++ ) {
    if ((e1->op == e2->op && 
	 ((e1p[r] & e2n[r]) || (p1p[r] & p2n[r]))) 
	|| 
	(e1->op != e2->op && 
	 (e1p[r] | p1p[r]) & (e2n[r] | p2n[r]) )) {
     
     //     printf("Exit do Interfere r = %d \n", r);
      return TRUE;
    }
  }

  for ( r = 0; r < gft_vector_length; r++ ) {
    if ( ((e1->op == e2->op && 
	 ((e2p[r] & e1n[r]) || (p2p[r] & p1n[r]))) 
	|| 
	(e1->op != e2->op && 
	 (e2p[r] | p2p[r]) & (e1n[r] | p1n[r])))
	) {

      // printf("1Exit do Interfere r = %d \n", r);
      return TRUE;
    }
  }

  //  printf("Exit don't Interfere\n");

  return FALSE;

}

/**
 * op1的前提或结果与
 * op2的前提或结果exclusive
 */
int interfere( OpNode *i1, OpNode *i2 )

{//need to extend for all disj effects not just the first one.


  BitVector *e1p = i1->unconditional->effect->cons->p_effects->vector;//pos_effect_vector;
  BitVector *e1n = i1->unconditional->effect->cons->n_effects->vector;//neg_effect_vector;
  BitVector *e2p = i2->unconditional->effect->cons->p_effects->vector;//pos_effect_vector;
  BitVector *e2n = i2->unconditional->effect->cons->n_effects->vector;//neg_effect_vector;
  BitVector *p1p = i1->pos_precond_vector;
  BitVector *p1n = i1->neg_precond_vector;
  BitVector *p2p = i2->pos_precond_vector;
  BitVector *p2n = i2->neg_precond_vector;

  int r;

  
  //  printf("Do %s and %s interfere\n", i1->name, i2->name);
  //	print_BitVector(p1p, gft_vector_length);
  //	print_BitVector(e1p, gft_vector_length);
  //	print_BitVector(p2n, gft_vector_length);
  //	print_BitVector(e2n, gft_vector_length); printf("\n");
  for ( r = 0; r < gft_vector_length; r++ ) {
    // printf("Dying\n");
   if ( (e1p[r] | p1p[r]) & (e2n[r] | p2n[r]) ) { 
  
    
  //        printf("Exit do Interfere 1\n");
//  	print_BitVector(e1p, gft_vector_length); 
//  	print_BitVector(p1p, gft_vector_length); 
//  	print_BitVector(e2n, gft_vector_length); 
//  	print_BitVector(p2n, gft_vector_length); 
// 	 printf("\n"); 

      return TRUE;
    }
  }
  // print_BitVector(p1n, gft_vector_length);
  //  print_BitVector(e1n, gft_vector_length);
  //  print_BitVector(p2p, gft_vector_length);
  //  print_BitVector(e2p, gft_vector_length);

  ///printf("\n");
     for ( r = 0; r < gft_vector_length; r++ ) {
    if ( (e2p[r] | p2p[r]) & (e1n[r] | p1n[r]) ) {
//          printf("Exit do Interfere 2\n");
//        print_BitVector(p1n, gft_vector_length); 
//        print_BitVector(e1n, gft_vector_length); 
//        print_BitVector(p2p, gft_vector_length); 
//        print_BitVector(e2p, gft_vector_length); 
// 	 printf("\n"); 
   

      return TRUE;
    }
  }

     //    printf("Exit don't Interfere\n");

  return FALSE;

}

/**
 * 1. 两个不是noop，不满足
 * 2. 两个都是noop,不冲突，返回false
 * 3. 其中一个是noop,判断其fact是否与op的结果冲突
 */
int noop_exclusive( OpNode *i1, OpNode *i2 )

{

  OpNode *noop, *op;
  FtNode *ft;
  BitVector *vec;


  if ( !i1->is_noop && !i2->is_noop ) return FALSE;
  if ( i1->is_noop && i2->is_noop ) return FALSE;

  noop = i1->is_noop ? i1 : i2;
  op = i1->is_noop ? i2 : i1;

 
  ft = noop->preconds->ft;

  /*
   * achtung! wenn s keinen unconditional effect gibt, werden
   * diese vectoren in build_graph.c mit 0 gesetzt!
   */
  if ( ft->positive ) {
    //        printf("ft is pos\n");
    vec = op->unconditional->effect->cons->n_effects->vector;//pos_effect_vector;
  } else {
    //   printf("ft is neg\n");
   vec = op->unconditional->effect->cons->p_effects->vector;//neg_effect_vector;
  }
  //  printf("%d\n", ft->index);
  //  print_BitVector(vec, gft_vector_length);


  if (vec[ft->uid_block] & ft->uid_mask) {
    return TRUE;
  } else {
    return FALSE;
  }

}
  
  
//int 
DdNode* facts_are_exclusive( int time, FtNode *f1, FtNode *f2, DdNode* l1, DdNode *l2 )

{
  //return the mutex worlds

  //EfExclusion *excl = f2->info_at[time]->adders_exclusives;
  //BitVector *b;
  //int r;
  EfEdge *i, *j;
  //int first = TRUE;
  //  int gotOne = FALSE;
  //  int gotTwo = FALSE;

 
  DdNode *l2s = Cudd_bddPermute(manager, l2, varmap);
  Cudd_Ref(l2s);
  DdNode *cross = Cudd_bddOr(manager, l1, l2s);
  Cudd_Ref(cross);

  DdNode* mutex = cross;//Cudd_ReadOne(manager);
  Cudd_Ref(mutex);


  // printf("enter facts_are_exclusive \n");
  // printFact(f1->index); printf(" %d with %d", f1->positive, f2->positive); printFact(f2->index); printf("\n");
  // printBDD(l1);
  //printBDD(l2);
  


   
  //look at all pairs of adders
  for ( i = f1->adders; i; i = i->next ) {
    if(!i->ef->info_at[time-1])continue;
    //  printf("Adder i = %d \n", i->ef->alt_index);
 
      //printf("YO\n");
      for ( j = f2->adders; j; j = j->next ) {
	if(!j->ef->info_at[time-1])continue;
	//printf("Adder j = %d \n", j->ef->alt_index);
	if (1){
	    
	    //!ARE_MUTEX_OPS( time-1, op1, op2, NULL, NULL ) &&
	
	  DdNode* n_mutex = Cudd_Not(ARE_MUTEX_EFS(time-1, i->ef, j->ef, l1, l2));
	  Cudd_Ref(n_mutex);


	  DdNode *e2s = Cudd_bddPermute(manager, j->ef->info_at[time-1]->label->label, varmap);
	  Cudd_Ref(e2s);
	  //	  DdNode *e2 = Cudd_bddOr(manager, e2s, j->ef->info_at[time-1]->label->label);
	  //	  Cudd_Ref(e2);

	  DdNode *e1s = Cudd_bddPermute(manager, i->ef->info_at[time-1]->label->label, varmap);
	  Cudd_Ref(e1s);
	  //	  DdNode *e1 = Cudd_bddOr(manager, e1s, i->ef->info_at[time-1]->label->label);
	  //	  Cudd_Ref(e1);

	  //	  DdNode *ecross = Cudd_bddAnd(manager, e1, e2);
	  DdNode *ecross = Cudd_bddOr(manager, i->ef->info_at[time-1]->label->label, e2s);
	  //	  DdNode *ecross = Cudd_bddAnd(manager, i->ef->info_at[time-1]->label->label, e2s);
	  Cudd_Ref(ecross);
	  //	  printBDD(ecross);
	  //DdNode* possible = Cudd_bddAnd(manager, n_mutex, ecross);
	  DdNode* possible = Cudd_bddAnd(manager, n_mutex, ecross);
	  Cudd_Ref(possible);
				
// 	  std::cout << "Supporter Mutex:" << std::endl;
// 	  printBDD(Cudd_Not(n_mutex));
// 	  std::cout << "Label Reachable:" << std::endl;
// 	  printBDD(ecross);
// 	  std::cout << "Non-Mutex Reachable:" << std::endl;
// 	  printBDD(possible);


	  //	  DdNode *tmp = Cudd_bddAnd(manager, mutex, Cudd_Not(possible));
	  //	  DdNode *tmp = Cudd_bddAnd(manager, mutex, Cudd_Not(n_mutex));
	  DdNode *tmp = Cudd_bddAnd(manager, mutex, Cudd_Not(possible));
	Cudd_Ref(tmp);
	Cudd_RecursiveDeref(manager, mutex);
	mutex = tmp;
	Cudd_Ref(mutex);
	Cudd_RecursiveDeref(manager, tmp);
	Cudd_RecursiveDeref(manager, n_mutex);
	Cudd_RecursiveDeref(manager, possible);
	Cudd_RecursiveDeref(manager, ecross);
	Cudd_RecursiveDeref(manager, e2s);

// 	  std::cout << "Fact Mutex:" << std::endl;
// 	  printBDD(mutex);

	if(mutex == Cudd_ReadLogicZero(manager)){
	Cudd_RecursiveDeref(manager, cross);
	Cudd_RecursiveDeref(manager, l2s);
	  return mutex;
	}

// 	  //  printf("HI\n");
// 	  for ( ip = i->ef->conditions; ip; ip = ip->next ) {
// 	    for ( jp = j->ef->conditions; jp; jp = jp->next ) {
// 	      if ( ARE_MUTEX_FTS( time-1, ip->ft, jp->ft, NULL, NULL ) ) break;
// 	    }
// 	    if ( jp ) break;
//	    
// 	    for ( jp = op2->preconds; jp; jp = jp->next ) {
// 	      if ( ARE_MUTEX_FTS( time-1, ip->ft, jp->ft, NULL, NULL ) ) break;
// 	    }
// 	    if ( jp ) break;
// 	  }
// 	  if ( ip ) {
// 	    if ( 0 ) {
// 	        printf("\nop pair bad!");
// 	    }
// 	    continue;
// 	  }
//	  
// 	  for ( ip = op1->preconds; ip; ip = ip->next ) {
// 	    for ( jp = j->ef->conditions; jp; jp = jp->next ) {
// 	      if ( ARE_MUTEX_FTS( time-1, ip->ft, jp->ft, NULL, NULL ) ) break;
// 	    }
// 	    if ( jp ) break;
// 	  }
// 	  if ( ip ) {
// 	    if ( 0 ) {
// 	      printf("\nop pair bad!");
// 	  }
// 	    continue;
// 	  }
	  
	//return Cudd_ReadLogicZero(manager);//FALSE;
	  
	}
      
    }
  }
//   //mutex only if you couldn't find non-mutex support
//printf("Facts are MUTEX\n");
//   if(gotOne && gotTwo && COMPUTE_LABELS)
//     return FALSE;
//   else
	Cudd_RecursiveDeref(manager, cross);
	Cudd_RecursiveDeref(manager, l2s);
  return mutex;

/*   /\* bit vector code a la STAN; */
/*    * funktioniert nur bei unconditional domains, */
/*    * sonst kann man effect conds - excl nicht */
/*    * abfragen. */
/*    * */
/*    * laengerfristig: EXCL UEBER EFFECTS DEFINIEREN!!! */
/*    *\/ */

 //   if ( !excl ) { 
//      excl = new_EfExclusion(0);
//      if ( f2->noop ) { 
// 	// more unsafe type casting...
//        free_bit_vector(excl->exclusives);
//        excl->exclusives = copy_bit_vector( (BitVector *)f2->noop->unconditional->info_at[time-1]->exclusives, (num_alt_effs%gcword_size)+1+2*gft_vector_length ); 
//        first = FALSE; 
//      } 
//      for ( i = f2->adders; i; i = i->next ) { 
//        if ( !f2->noop && first ) { 
//        free_bit_vector(excl->exclusives);
//  	excl->exclusives = copy_bit_vector( (BitVector *)i->ef->op->unconditional->info_at[time-1]->exclusives, (num_alt_effs%gcword_size)+1+2*gft_vector_length  ); 
//  	first = FALSE; 
//        } 
//        b = (BitVector *) i->ef->op->info_at[time-1]->exclusives; 
//        for ( r = 0; r < gop_vector_length_at[time-1]; r++ ) { 
//  	excl->exclusives[r] &= b[r]; 
//        } 
//      } 
//      free_ef_exclusion(f2->info_at[time]->adders_exclusives);
//      f2->info_at[time]->adders_exclusives = (EfExclusion *) excl; 
//    } 

//    b = f1->info_at[time]->adders; 
//    for ( r = 0; r < gop_vector_length_at[time-1]; r++ ) { 
//      if ( b[r] != ( b[r] & excl->exclusives[r] ) ) { 
//        break; 
//      } 
//    } 

//    //return ( r == gop_vector_length_at[time-1] ); 
//    return 1;

}

void find_mutex_effects1(int time) {
  OpNode *i1, *i2;
  EfNode *e1, *e2;
  EfPair* i, *prev, *tmp;
  BitVector *a, *b;
  int r;
  DdNode *worlds1, *worlds2, *competeMutex;

  //   std::cout << "Enter Find mutex Effects " << time << std::endl << std::flush;
 
   for (e1=gprev_level_efs_pointer ; e1; e1=e1->all_next ) {
     //          printf("New EfExclusion for %d\n",e1->alt_index);fflush(stdout);
     //     if(!e1->info_at[time]->exclusives)
     //e1->info_at[time-1]->exclusives = new_EfExclusion(num_alt_effs+2*num_alt_facts);// new_excl_bit_vector( gop_vector_length_at[time] );
    if ( time > 0 && e1->info_at[time-1] ) {
      a = e1->info_at[time]->exclusives->exclusives;
      b = e1->info_at[time-1]->exclusives->exclusives;
      for ( r = 0; r < gef_vector_length; r++ ) {
        a[r] |= b[r];
      }
      for ( r = 0; r < (2*num_alt_facts+num_alt_effs); r++ ) {
 	      e1->info_at[time]->exclusives->exlabel[r] = 
 	      e1->info_at[time-1]->exclusives->exlabel[r];
 	      if(e1->info_at[time]->exclusives->exlabel[r])
 	        Cudd_Ref(e1->info_at[time]->exclusives->exlabel[r]);
      }
   
      if(time > 1){
	      a = e1->info_at[time]->exclusives->pos_exclusives;
	      b = e1->info_at[time-1]->exclusives->pos_exclusives;

	      for ( r = 0; r < gft_vector_length; r++ ) {
	        a[r] |= b[r];
	      }

	      for ( r = 0; r < num_alt_facts; r++ ) {
	        //	e1->info_at[time-1]->exclusives->p_exlabel[r] = Cudd_ReadLogicZero(manager);
 	        if(e1 &&
 	            e1->info_at[time-1] &&
	            e1->info_at[time-1]->exclusives)
	          
            e1->info_at[time]->exclusives->p_exlabel[r] = 
	          e1->info_at[time-1]->exclusives->p_exlabel[r];
	        
          if(e1->info_at[time]->exclusives->p_exlabel[r])
	          Cudd_Ref(e1->info_at[time]->exclusives->p_exlabel[r]);
        }
 
 
        a = e1->info_at[time]->exclusives->neg_exclusives;
        b = e1->info_at[time-1]->exclusives->neg_exclusives;
        for ( r = 0; r < gft_vector_length; r++ ) {
          a[r] |= b[r];
        }
        for ( r = 0; r < num_alt_facts; r++ ) {

	        e1->info_at[time]->exclusives->n_exlabel[r] = 
	        e1->info_at[time-1]->exclusives->n_exlabel[r];
	        if(e1->info_at[time]->exclusives->n_exlabel[r])
	          Cudd_Ref(e1->info_at[time]->exclusives->n_exlabel[r]);
        }
      }
    }
  }

 
  if(time > 1) {   

    i = gef_mutex_pairs;
    while ( i){
      if(MUTEX_SCHEME!=MS_NONE){
	      worlds1 = i->e1->info_at[time]->label->label;
	      worlds2 = i->e2->info_at[time]->label->label;
      }
      else {
	      worlds1 = Cudd_ReadOne(manager);
	      worlds2 = Cudd_ReadOne(manager);
      }
      
      // std::cout << "a" << std::endl;   
      competeMutex = ef_competing_needs( time, i->e1, i->e2, worlds1, worlds2);      
      Cudd_Ref(competeMutex);
      MAKE_EFS_UNEXCLUSIVE( time, i->e1, i->e2, Cudd_Not(competeMutex) );
      if(competeMutex == Cudd_ReadLogicZero(manager)){
	      tmp = i;
	      i = i->next;
	      free( tmp );
	      //num_ef_pair--;

#ifdef MEMORY_INFO
        gexcl_memory -= sizeof( EfPair );
#endif
        gefs_exclusions_count--;
      }
      else
	      break;
    }// end-while


    gef_mutex_pairs = i;
    prev = i;
    if ( i ) i = i->next;
    while ( i ) {
      if(MUTEX_SCHEME!=MS_NONE){
	      worlds1 = i->e1->info_at[time]->label->label;
	      worlds2 = i->e2->info_at[time]->label->label;
      }
      else {
	      worlds1 = Cudd_ReadOne(manager);
	      worlds2 = Cudd_ReadOne(manager);
      }
      competeMutex = ef_competing_needs( time, i->e1, i->e2, worlds1, worlds2 );
      Cudd_Ref(competeMutex);
      MAKE_EFS_UNEXCLUSIVE( time, i->e1, i->e2 , Cudd_Not(competeMutex));
      if(competeMutex == Cudd_ReadLogicZero(manager)){
	      prev->next = i->next;
	      tmp = i;
	      i = i->next;
	      free( tmp );
	      // num_ef_pair--;
#ifdef MEMORY_INFO
	      gexcl_memory -= sizeof( EfPair );
#endif
	      gefs_exclusions_count--;
     
      }
      else {
	      prev = prev->next;
	      i = i->next;
      }
    }// end-while
  }// end-if

//   for ( i1 = gall_ops_pointer; i1   ; i1 = i1->next ) {
//     for ( i2 = i1->next; i2; i2 = i2->next ) {
  for ( e1 = gall_efs_pointer; e1 && e1->info_at[time]->updated
	  ; e1 = e1->all_next ) {
    for ( e2 = e1->all_next; e2; e2 = e2->all_next ) {
	


      //     std::cout << "Check Eff mutex ";

//        if(e1->op->is_noop){
// 	std::cout << "Noop " 
// 		  << (e2->op->alt_index-1) << " " 
// 		  << (e1->op->preconds->ft->positive ? "pos " : "neg ");
// 	printFact(e1->op->preconds->ft->index);
//       }
//       else
// 	std::cout << e1->alt_index << " " 
// 		  << (e1->op->alt_index-1) << " " 
// 		  << e1->op->name << std::endl;
      
//       if(e2->op->is_noop){
// 	std::cout << "Noop " 
// 		  << (e2->op->alt_index-1) << " " 
// 		  << (e2->op->preconds->ft->positive ? "pos " : "neg ");
// 	printFact(e2->op->preconds->ft->index);
//       }
//       else
// 	std::cout << e2->alt_index << " " 
// 		  << (e2->op->alt_index-1) << " " 
// 		  << e2->op->name << std::endl;


      DdNode* op_mutex;
      if(e1->op != e2->op)

	      op_mutex = ARE_MUTEX_OPS(time, e1->op, e2->op, //i1, i2, 
			  	          e1->info_at[time]->label->label, 
			  	          e2->info_at[time]->label->label);      
      else
	      op_mutex = Cudd_ReadLogicZero(manager);

 //       for(int t1 = 0; t1 < 2; t1++){
//  	if(t1== 0 ){
//  	  // std::cout << "uncond1" <<std::endl;
// 	  e1 = i1->unconditional;
// 	}
// 	else{
// 	  // std::cout << "cond1" <<std::endl;
// 	  e1 = i1->conditionals;
// 	}



//       // std::cout << "e1 " << std::endl<<std::flush;
// 	  for ( ; e1  ; e1 = e1->next ) {
// // 	      if(!e1->info_at[time]->exclusives->exclusives)
// // 		e1->info_at[time]->exclusives->exclusives = 
// // 		  new_excl_bit_vector(gef_vector_length);

// 	for(int t2 = 0; t2 < 2; t2++){
// 	  if(t2 == 0 ){
// 	    e2 = i2->unconditional;
// 	    //  std::cout << "uncond2" <<std::endl;
// 	  }
// 	  else if(t2==1){
// 	    e2 = i2->conditionals;
// 	    // std::cout << "cond2" <<std::endl;

// 	  }
//         std::cout << "e2 " << std::endl<<std::flush;
// 	    for ( ; e2; e2 = e2->next ) {

	    if((e1->effect->ant->b == Cudd_ReadLogicZero(manager) &&
          e1->effect->cons->b == Cudd_ReadLogicZero(manager)) ||
		    (e2->effect->ant->b == Cudd_ReadLogicZero(manager) &&
		      e2->effect->cons->b == Cudd_ReadLogicZero(manager)))
		    continue;

	      //since uncond has no effect, but a precond, it can
	      //be involved in meaningless mutexes, the effects with
	      //no antecedent should be moved into the unconditional
	    if(e1->op == e2->op && (e1 == e1->op->unconditional ||
				      e2 == e2->op->unconditional))
        continue;


	      //	      std::cout << "check mutex " << e1->alt_index << " " << e2->alt_index << std::endl;
	      //	      printBDD(e1->info_at[time]->exclusives->exlabel[e2->alt_index]);
//   	      printBDD(e1->effect->ant->b);
//   	      printBDD(e1->effect->cons->b);
//   	      printBDD(e2->effect->ant->b);
//   	      printBDD(e2->effect->cons->b);
	      //	      printf("checking interfere\n");

// 	      if(!e2->info_at[time]->exclusives->exclusives)
// 		e2->info_at[time]->exclusives->exclusives = 
// 		  new_excl_bit_vector(gef_vector_length);
	    if ( op_mutex && op_mutex != Cudd_ReadLogicZero(manager)){
		    //std::cout << "op_mutex" << std::endl;
		    MAKE_EFS_EXCLUSIVE( time, e1, e2, op_mutex);//, op_mutex);
		    continue;
	    }
	      else if(ef_interfere( e1, e2 ) ) {
		      //printf("Interfere\n");

		      MAKE_EFS_EXCLUSIVE( time, e1, e2, Cudd_ReadOne(manager));//, Cudd_ReadOne(manager));
		
// 	      if(e1 == e1->op->unconditional){
// 		//if unconditional is mutex, then all conditionals are too
// 		for(EfNode *e3 = e1->op->conditionals; e3; e3=e3->next){
// 		  // std::cout << "HIb"<<std::endl;
// 		  // 		    if(!e3->info_at[time]->exclusives->exclusives)
// 		  // 		      e3->info_at[time]->exclusives->exclusives = 
// 		  // 			new_excl_bit_vector(gef_vector_length);
// 		  MAKE_EFS_EXCLUSIVE( time, e3, e2, Cudd_ReadOne(manager));//, Cudd_ReadOne(manager));
// 		  }
// 		}
// 		if(e2->op->unconditional == e2){
// 		  //if unconditional is mutex, then all conditionals are too
// 		  for(EfNode *e3 = e2->op->conditionals; e3; e3=e3->next){
// 		    //	       std::cout << "HIa"<<std::endl;
// // 		    if(!e3->info_at[time]->exclusives->exclusives)
// // 		      e3->info_at[time]->exclusives->exclusives = 
// // 			new_excl_bit_vector(gef_vector_length);
// 		    MAKE_EFS_EXCLUSIVE( time, e1, e3, Cudd_ReadOne(manager));//,Cudd_ReadOne(manager));
// 		  }
// 		}

		      continue;
	      }
	      //	       std::cout << "HI"<<std::endl;

	      if ( time > 0){
		      DdNode* mutex;	       
		      // printf("COMPETE NEEDS\n");
		      mutex = ef_competing_needs( time, e1, e2, 
					        e1->info_at[time]->label->label,
					        e2->info_at[time]->label->label);
	        Cudd_Ref(mutex);
	        if(mutex == Cudd_ReadLogicZero(manager))
	          continue;

 	    
// 	    exit(0);
	        MAKE_EFS_EXCLUSIVE( time, e1, e2, mutex);//, mutex);
// 	    if(e1->op->unconditional == e1){
// 	      //if unconditional is mutex, then all conditionals are too
// 	      for(EfNode *e3 = e1->op->conditionals; e3; e3=e3->next){
// 		// 		    if(!e3->info_at[time]->exclusives->exclusives)
// 		// 		      e3->info_at[time]->exclusives->exclusives = 
// 		// 			new_excl_bit_vector(gef_vector_length);
// 		MAKE_EFS_EXCLUSIVE( time, e3, e2, mutex);//, mutex);
// 	      }
// 	    }
// 	    if(e2->op->unconditional == e2){
// 	      //if unconditional is mutex, then all conditionals are too
// 	      for(EfNode *e3 = e2->op->conditionals; e3; e3=e3->next){
// 		// 		    if(!e3->info_at[time]->exclusives->exclusives)
// 		// 		      e3->info_at[time]->exclusives->exclusives = 
// 		// 			new_excl_bit_vector(gef_vector_length);
// 		MAKE_EFS_EXCLUSIVE( time, e1, e3, mutex);//, mutex);
// 	      }
// 	    }
	    //continue;
	      }
	    //  std::cout << "HOI"<<std::endl;
	    //    }
    //}
    //  }
    // }



    }// end-for
    //induced mutexes
    if(DO_INDUCED){
      //add mutexes to e1, where some other effect of 
      //same action is mutex with some effect of another
      //action
      
      for(int t = 0; t < 2; t++){
	      if(t==0)
	        e2 = e1->op->unconditional;
	      else
	        e2 = e1->op->conditionals;

	      for(; e2; e2 = e2->next){
	        if(e1 == e2) continue;

	        DdNode *inducedWorlds = Cudd_bddAnd(manager, 
			  		          e1->info_at[time]->label->label,
			  		          e2->info_at[time]->label->label);
	        Cudd_Ref(inducedWorlds);
	        DdNode* pinducedWorlds = Cudd_bddVarMap(manager, inducedWorlds);
	        Cudd_Ref(pinducedWorlds);
	        DdNode* binducedWorlds = Cudd_bddAnd(manager, inducedWorlds, pinducedWorlds);
	        Cudd_Ref(binducedWorlds);
	        DdNode* nonMutexInducedWorlds = Cudd_bddAnd(manager,
			  			                            binducedWorlds,
			  			                            Cudd_Not(ARE_MUTEX_EFS(time, e1, e2, 
			  						                           e1->info_at[time]->label->label,
			  						                           e2->info_at[time]->label->label)));
	        Cudd_Ref(nonMutexInducedWorlds);

	        Cudd_RecursiveDeref(manager, pinducedWorlds);
	        Cudd_RecursiveDeref(manager, binducedWorlds);
	        pinducedWorlds = Cudd_bddVarMap(manager, nonMutexInducedWorlds);
	        Cudd_Ref(pinducedWorlds);
	        binducedWorlds = Cudd_bddAnd(manager, pinducedWorlds, nonMutexInducedWorlds);
	        Cudd_Ref(binducedWorlds);
	        DdNode* nonMutexSameInducedWorlds = Cudd_bddExistAbstract(manager, binducedWorlds, 
		      					       next_state_cube);
	        Cudd_Ref(nonMutexSameInducedWorlds);


	        Cudd_RecursiveDeref(manager, inducedWorlds);
	        Cudd_RecursiveDeref(manager, pinducedWorlds);
	        Cudd_RecursiveDeref(manager, binducedWorlds);
  
	        for(EfNode *e3 = gall_efs_pointer; e3; e3 = e3->next){
	          if(e3->op == e2->op) continue;
	          DdNode *m23 = ARE_MUTEX_EFS(time, e2, e3, nonMutexSameInducedWorlds, 
			  		              e3->info_at[time]->label->label);
	          Cudd_Ref(m23);
	          if(m23 != Cudd_ReadLogicZero(manager)){
	            add_worlds_to_mutex(e1->info_at[time]->exclusives->exlabel[e3->alt_index], m23);
	          }
	        }
	      }
      }
    }// end-if DO_INDUCED
  }
  //  std::cout << "Exit FIND mux effs" << std::endl; 
}



DdNode* removeMutexWorlds(DdNode* origWorlds, DdNode *f, int time){
  //remove worlds from origWorlds where f is impossible

  //  std::cout << "removeMutexWorlds" << std::endl;
  //  printBDD(origWorlds);
  //  printBDD(f);
  
  DdNode* sup = Cudd_Support(manager, f);
  Cudd_Ref(sup);

  for(int i = 0; i < num_alt_facts; i++){
    if(!bdd_entailed(manager, sup, Cudd_bddIthVar(manager, 2*i)))
      continue;
    for(int j = i; j < num_alt_facts; j++){
      if(!bdd_entailed(manager, sup, Cudd_bddIthVar(manager, 2*j)))
	continue;
      for(int ip = 0; ip < 2; ip++){
	for(int jp = 0; jp < 2; jp++){
	  DdNode *pair = Cudd_ReadOne(manager);
	  Cudd_Ref(pair);
	  
	

	  DdNode *tip, *tjp;
	  FtNode *f1, *f2;
	  if(ip == 0){
	    tip = Cudd_bddAnd(manager, pair, Cudd_Not(Cudd_bddIthVar(manager, 2*i)));	  
	    f1 = gft_table[NEG_ADR(i)];
	  }
	  else{
	    tip = Cudd_bddAnd(manager, pair, Cudd_bddIthVar(manager, 2*i));	  
	    f1 = gft_table[i];
	  }
	  Cudd_Ref(tip);	  
	  Cudd_RecursiveDeref(manager, pair);
	  pair = tip;
	  Cudd_Ref(pair);	  
	  Cudd_RecursiveDeref(manager, tip);

	  if(jp == 0){
	    tjp = Cudd_bddAnd(manager, pair, Cudd_Not(Cudd_bddIthVar(manager, 2*j)));	  
	    f2 = gft_table[NEG_ADR(j)];
	  }
	  else{
	    tjp = Cudd_bddAnd(manager, pair, Cudd_bddIthVar(manager, 2*j));	  
	    f2 = gft_table[j];
	  }
	  Cudd_Ref(tjp);
	  Cudd_RecursiveDeref(manager, pair);
	  pair = tjp;
	  Cudd_Ref(pair);	  
	  Cudd_RecursiveDeref(manager, tjp);

	  if(Cudd_ReadLogicZero(manager) != Cudd_bddIntersect(manager, pair, f)){
	    //    printBDD(pair);
	    DdNode *mutex = ARE_MUTEX_FTS(time, f1, f2, origWorlds, origWorlds);
	    Cudd_Ref(mutex);
	    

	    DdNode *mutex1 = Cudd_bddExistAbstract(manager, mutex, next_state_cube);
	    Cudd_Ref(mutex1);

	    DdNode *mutex2 = Cudd_bddExistAbstract(manager, mutex, current_state_cube);
	    Cudd_Ref(mutex2);
	    DdNode* mutex2p = Cudd_bddVarMap(manager, mutex2);
	    Cudd_Ref(mutex2p);

	    DdNode* amutex = Cudd_bddOr(manager, mutex1, mutex2p);
	    Cudd_Ref(amutex);

	    Cudd_RecursiveDeref(manager, mutex);
	    Cudd_RecursiveDeref(manager, mutex1);
	    Cudd_RecursiveDeref(manager, mutex2);
	    Cudd_RecursiveDeref(manager, mutex2p);


	    //printBDD(mutex);
	    //printBDD(amutex);

	    DdNode *newWorlds = Cudd_bddAnd(manager, origWorlds, Cudd_Not(amutex));
	    Cudd_Ref(newWorlds);
	    Cudd_RecursiveDeref(manager, origWorlds);
	    origWorlds = newWorlds;
	    Cudd_Ref(origWorlds);	  
	    Cudd_RecursiveDeref(manager, newWorlds);
	    Cudd_RecursiveDeref(manager, amutex);
	    if(origWorlds == Cudd_ReadLogicZero(manager))
	      break;
	  }
	  
	}
	if(origWorlds == Cudd_ReadLogicZero(manager))
	  break;
      }      
      if(origWorlds == Cudd_ReadLogicZero(manager))
	break;
    }
    if(origWorlds == Cudd_ReadLogicZero(manager))
      break;
  }
  Cudd_RecursiveDeref(manager, sup);

  //  printBDD(origWorlds);  
  return origWorlds;
}

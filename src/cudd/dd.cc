/**CFile***********************************************************************

  FileName    [dd.c]

  PackageName [dd]

  Synopsis [NuSMV interface to the Decision Diagram Package of the
  University of Colorado.]

  Description [This file implements the interface between the NuSMV
  system and the California University Decision Diagram (henceforth
  referred as CUDD). The CUDD package is a generic implementation of a
  decision diagram data structure. For the time being, only Boole
  expansion is implemented and the leaves in the in the nodes can be
  the constants zero, one or any arbitrary value. A coding standard
  has been defined. I.e all the functions acting on BDD and ADD have
  \"bdd\" and \"add\" respectively as prefix.
  <p><br>
  The BDD or ADD returned as a result of an operation are always
  referenced (see the CUDD User Manual for more details about this),
  and need to be dereferenced when the result is no more necessary to
  computation, in order to release the memory associated to it when
  garbage collection occurs.
  All the functions takes as first argument the decision diagram
  manager (henceforth referred as DdManager).]

  Author      [Marco Roveri]

  Copyright   [
  This file is part of the ``dd'' package of NuSMV version 2. 
  Copyright (C) 1998-2001 by CMU and ITC-irst. 

  NuSMV version 2 is free software; you can redistribute it and/or 
  modify it under the terms of the GNU Lesser General Public 
  License as published by the Free Software Foundation; either 
  version 2 of the License, or (at your option) any later version.

  NuSMV version 2 is distributed in the hope that it will be useful, 
  but WITHOUT ANY WARRANTY; without even the implied warranty of 
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public 
  License along with this library; if not, write to the Free Software 
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA.

  For more information of NuSMV see <http://nusmv.irst.itc.it>
  or email to <nusmv-users@irst.itc.it>.
  Please report bugs to <nusmv-users@irst.itc.it>.

  To contact the NuSMV development board, email to <nusmv@irst.itc.it>. ]

 ******************************************************************************/

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <math.h>
#include <float.h>

//extern int floor(double);

#include <map>

#include "globals.h"

#include "dd.h"
#include "cuddInt.h"

static int bddCheckPositiveCube ARGS((DdManager *, DdNode *));
static void ddClearFlag ARGS((DdNode *));
static CUDD_VALUE_TYPE Cudd_fatal_error ARGS((DdManager *, const char *));

EXTERN DdNode * cudd_bddCubeDiffRecur ARGS((DdManager *dd, DdNode *f,DdNode *g));
EXTERN int Cudd_BddGetLowestVarRecur ARGS((DdManager *dd, DdNode * N, int index));

static  DdNode * cuddBddVectorComposeRecur( DdManager * dd /* DD manager */,  
		DdHashTable * table /* computed table */,
		DdNode * f /* BDD in which to compose */,
		DdNode ** pvector /* functions to be composed */,
		DdNode ** nvector /* functions to be composed */,
		int deepest /* depth of the deepest substitution */);

extern void printBDD(DdNode*);

/**Function********************************************************************

  Synopsis    [Composes a BDD with a vector of BDDs.]

  Description [Given a vector of BDDs, creates a new BDD by
  substituting the BDDs for the variables of the BDD f.  There
  should be an entry in vector for each variable in the manager.
  If no substitution is sought for a given variable, the corresponding
  projection function should be specified in the vector.
  This function implements simultaneous composition.
  Returns a pointer to the resulting BDD if successful; NULL
  otherwise.]

  SideEffects [None]

  SeeAlso     [Cudd_bddPermute Cudd_bddCompose Cudd_addVectorCompose]

 ******************************************************************************/
DdNode *
Cudd_bddVectorCompose(
		DdManager * dd,
		DdNode * f,
		DdNode ** pvector,
		DdNode ** nvector)
		{
	DdHashTable		*table;
	DdNode		*res;
	int			deepest;
	int                 i;

	do {
		dd->reordered = 0;
		/* Initialize local cache. */
		table = cuddHashTableInit(dd,1,2);
		printf("got table\n"); fflush(stdout);

		if (table == NULL) return(NULL);

		/* Find deepest real substitution. */
		for (deepest = dd->size - 1; deepest >= 0; deepest--) {
			std::cout << "HI " << dd->size <<std::endl;
			i = Cudd_ReadInvPerm(dd, deepest);//invperm[deepest];
			// 	    assert(nvector[i]);
			// 	    assert(pvector[i]);
			if (nvector[i] != Cudd_Not(dd->vars[i]) ||
					pvector[i] != dd->vars[i]) {
				break;
			}
		}
		printf("Before recurse\n"); fflush(stdout);
		printBDD(f);

		/* Recursively solve the problem. */
		res = cuddBddVectorComposeRecur(dd,table,f,pvector, nvector, deepest);
		if (res != NULL) cuddRef(res);

		/* Dispose of local cache. */
		cuddHashTableQuit(table);
	} while (dd->reordered == 1);


	if (res != NULL) cuddDeref(res);
	return(res);

		}

DdNode *
Cudd_bddVectorCompose1(
		DdManager * dd,
		DdNode * f,
		DdNode ** pvector,
		DdNode ** nvector,
		int deepest)
		{
	DdHashTable		*table;
	DdNode		*res;
	//    int			deepest;
	int                 i;

	do {
		dd->reordered = 0;
		/* Initialize local cache. */
		table = cuddHashTableInit(dd,1,2);
		//	printf("got table\n"); fflush(stdout);

		if (table == NULL) return(NULL);

		/* Find deepest real substitution. */
		// 	for (deepest = dd->size - 1; deepest >= 0; deepest--) {
		// 	  std::cout << "HI " << dd->size <<std::endl;
		// 	  i = Cudd_ReadInvPerm(dd, deepest);//invperm[deepest];
		// // 	    assert(nvector[i]);
		// // 	    assert(pvector[i]);
		// 	    if (nvector[i] != Cudd_Not(dd->vars[i]) ||
		// 		pvector[i] != dd->vars[i]) {
		// 		break;
		// 	    }
		// 	}
//		   	printf("Before recurse\n"); fflush(stdout);
//		   	printBDD(f);

		/* Recursively solve the problem. */
		res = cuddBddVectorComposeRecur(dd,table,f,pvector, nvector, deepest);

		if (res != NULL) cuddRef(res);

		/* Dispose of local cache. */
		cuddHashTableQuit(table);
	} while (dd->reordered == 1);


	if (res != NULL) cuddDeref(res);
	return(res);

		} /* end of Cudd_bddVectorCompose */

/**Function********************************************************************

  Synopsis    [Performs the recursive step of Cudd_bddVectorCompose.]

  Description []

  SideEffects [None]

  SeeAlso     []

 ******************************************************************************/
static DdNode *
cuddBddVectorComposeRecur(
		DdManager * dd /* DD manager */,
		DdHashTable * table /* computed table */,
		DdNode * f /* BDD in which to compose */,
		DdNode ** pvector /* functions to be composed */,
		DdNode ** nvector /* functions to be composed */,
		int deepest /* depth of the deepest substitution */)
		{
	DdNode	*F,*T,*E,*T_,*E_;
	DdNode	*res;




	statLine(dd);
	F = Cudd_Regular(f);





	//	    printf("F %d %d %d\n", F->index, Cudd_ReadPerm(dd, F->index), deepest);fflush(stdout);
//	     Cudd_CheckKeys(dd);  fflush(stdout);
//	printBDD(F);
//	printBDD(pvector[0]);
	//    if(Cudd_IsComplement(F))
	//       printBDD(cuddT(Cudd_Not(F)));
	//     else
	//       printBDD(cuddT(F));
	//     if(Cudd_IsComplement(F))
	//       printBDD(cuddE(Cudd_Not(F)));
	//     else
	//     printBDD(cuddE(F));

	// printf("Findex = %d, %d %d\n ", F->index, f, Cudd_ReadPerm(dd, F->index));fflush(stdout);
	/* If we are past the deepest substitution, return f. */
	if(//Cudd_IsConstant(F) ||
			Cudd_ReadPerm(dd,F->index) > deepest || Cudd_ReadPerm(dd,F->index) < 0){
//		printBDD(f);
//		printBDD(pvector[0]);
//
//		printf("Deep\n");fflush(stdout);
		//Cudd_CheckKeys(dd);  fflush(stdout);
		//      printBDD(Cudd_NotCond(F, F == f));
		//return(Cudd_NotCond(f, F != f));
		// printBDD(f);
		//     printBDD(F);
		return(f);
	}

	if(cuddIsConstant(F)) {
//		 printf("Const\n");fflush(stdout);
		//    printBDD(Cudd_NotCond(f, F != f));
		//return(Cudd_NotCond(f, F != f));
		return f;
	}
//    printBDD(pvector[F->index]);

	/* If problem already solved, look up answer and return. */
	if ((res = cuddHashTableLookup1(table,F)) != NULL) {
#ifdef DD_DEBUG
		bddVectorComposeHits++;
#endif
		//	printBDD(Cudd_NotCond(res,F != f));
		return(Cudd_NotCond(res,F != f));
		return res;
	}
//	printBDD(F);
	//printf("Before split\n"); fflush(stdout);

	/* Split and recur on children of this node. */
	//    if(Cudd_IsComplement(cuddT(F)))
	T = cuddBddVectorComposeRecur(dd,table, Cudd_NotCond(cuddT(F), F != f),//cuddT(F),
			pvector, nvector, deepest);
//	printBDD(T);
//	printBDD(pvector[0]);

//	printf("T: %d %d %d\n", T->index, Cudd_ReadPerm(dd, T->index), deepest);fflush(stdout);

	//    else
	//      T = cuddBddVectorComposeRecur(dd,table,cuddT(F),pvector, nvector, deepest);
	//       printBDD(T);
	//        printf("exit T\n"); fflush(stdout);
	if (T == NULL) {printf("NULL\n");fflush(stdout);return(NULL);}
//	printBDD(pvector[0]);
	cuddRef(T);
//	printBDD(pvector[0]);

//	printf("enter %d %d %d\n", F->index, Cudd_ReadPerm(dd, F->index), deepest);fflush(stdout);
//	printBDD(pvector[0]);
//
//    printBDD(T);
//	printBDD(pvector[0]);
//
//	printBDD(pvector[F->index]);
	//  printf("end then\n"); fflush(stdout);
	//     Cudd_CheckKeys(dd);  fflush(stdout);

	//E = cuddBddVectorComposeRecur(dd,table,Cudd_Not(cuddE(F)),pvector, nvector, deepest);
	//    if(Cudd_IsComplement(cuddT(F)))
	//      E = cuddBddVectorComposeRecur(dd,table,Cudd_Not(cuddE(F)),pvector, nvector, deepest);
	//    else
	E = cuddBddVectorComposeRecur(dd,table, Cudd_NotCond(cuddE(F), f != F),//cuddE(F),
			pvector, nvector, deepest);
	//    E = Cudd_Not(cuddBddVectorComposeRecur(dd,table,cuddE(F),pvector, nvector, deepest));
	//   printBDD(E);
	//     printf("exit E\n"); fflush(stdout);
	//  Cudd_CheckKeys(dd);  fflush(stdout);

	//	printf("E: %d %d %d\n", E->index, Cudd_ReadPerm(dd, E->index), deepest);fflush(stdout);

	if (E == NULL) {
		printf("NULL\n");
		Cudd_IterDerefBdd(dd, T);
		//	printf("NULL1\n");fflush(stdout);
		return(NULL);
	}
	cuddRef(E);
	//    printf("end else\n"); fflush(stdout);
	//   printBDD(E);
	/* Call cuddBddIteRecur with the BDD that replaces the current top
	 ** variable and the T and E we just created.
	 */
	T_ = cuddBddAndRecur(dd,pvector[F->index],T);
	if (T_ == NULL)
	{
		printf("NULL\n");
		Cudd_IterDerefBdd(dd, T);
		Cudd_IterDerefBdd(dd, E);
		return(NULL);
	}
	cuddRef(T_);
	T_=Cudd_Not(T_);
	//    printBDD(pvector[F->index]);
	 //printBDD(T_);

	E_ = cuddBddAndRecur(dd,nvector[F->index],E);
	//printBDD(T_);

	if (E_ == NULL)
	{
		printf("NULL\n");
		Cudd_IterDerefBdd(dd, T);
		Cudd_IterDerefBdd(dd, E);
		Cudd_IterDerefBdd(dd, T_);
		return(NULL);
	}
	//printBDD(T_);

	cuddRef(E_);
	//printBDD(T_);
	E_=Cudd_Not(E_);
	//printBDD(T_);
	//     printBDD(nvector[F->index]);
//		printBDD(E_);
//		printBDD(T_);
	res = cuddBddAndRecur(dd,T_,E_);
//	printBDD(T_);
//	printBDD(E_);
//	printBDD(res);
	cuddRef(res);
	// printf("Findex = %d, %d %d\n ", F->index, f, Cudd_ReadPerm(dd, F->index));flush(stdout);
	//      printBDD(f);
	//        printBDD(T);
	//        printBDD(pvector[F->index]);
	//        printBDD(T_);

	//         printBDD(E);
	//        printBDD(nvector[F->index]);
	//       printBDD(E_);
	if (res == NULL) {
		printf("NULL\n");
		Cudd_IterDerefBdd(dd, T);
		Cudd_IterDerefBdd(dd, E);
		Cudd_IterDerefBdd(dd, T_);
		Cudd_IterDerefBdd(dd, E_);
		// 	printf("Exit one\n");fflush(stdout);
		//        Cudd_CheckKeys(dd);  fflush(stdout);
		return(NULL);
	}
	cuddRef(res);

   // printBDD(res);

	res = Cudd_Not(res);

	/*
    printf("[%d]",Cudd_Regular(T)->ref);
    printf("[%d]",Cudd_Regular(E)->ref);
    printf("[%d]",Cudd_Regular(T_)->ref);
    printf("[%d]",Cudd_Regular(E_)->ref);
    printf("[%d]",Cudd_Regular(res)->ref);
	 */
	Cudd_IterDerefBdd(dd, T);
	Cudd_IterDerefBdd(dd, E);
	Cudd_IterDerefBdd(dd, T_);
	Cudd_IterDerefBdd(dd, E_);

	// std::cout << "ho:"<<std::endl;
	//    printBDD(res);

	/* Do not keep the result if the reference count is only 1, since
	 ** it will not be visited again.
	 */
	if (F->ref != 1) {
		ptrint fanout = (ptrint) F->ref;
		cuddSatDec(fanout);
		if (!cuddHashTableInsert1(table,F,res,fanout)) {
			Cudd_IterDerefBdd(dd, res);
			// 	    printf("Exit two\n");fflush(stdout);
			// 	    Cudd_CheckKeys(dd);  fflush(stdout);
			return(NULL);
		}
	}
	/*   printf("beforeExit there\n");fflush(stdout);
     Cudd_CheckKeys(dd);  fflush(stdout);
     printBDD(res);*/
	cuddDeref(res);

	/*     printf("Exit there\n");fflush(stdout);
	    Cudd_CheckKeys(dd);  fflush(stdout);*/

	//return(Cudd_NotCond(res,F != f));
	return res;

		} /* end of cuddBddVectorComposeRecur */

extern double exAbstractAllLabels(DdNode*,int);

/**Function********************************************************************

  Synopsis    [Performs the recursive step of Cudd_addGeneralVectorCompose.]

  Description []

  SideEffects [None]

  SeeAlso     []

 ******************************************************************************/
static DdNode *
cuddAddGeneralVectorComposeRecur1(
		DdManager * dd /* DD manager */,
		DdHashTable * table /* computed table */,
		DdNode * f /* ADD in which to compose */,
		DdNode ** vectorOn /* functions to substitute for x_i */,
		DdNode ** vectorOff /* functions to substitute for x_i' */,
		int  deepest /* depth of deepest substitution */,
		int time)
		{
	DdNode	*T,*E,*t,*e;
	DdNode	*res;
	DdNode      *tmp, *fr, *fr1;

	if(!f){
		//cout << "NULL"<<endl;
		return NULL;
	}

	//       printf("Findex = %d %d %d\n ", f->index, f, Cudd_ReadPerm(dd, f->index));fflush(stdout);
	//       printBDD(f);
	//       cout << "deepest = " << deepest<<endl;

	/* If we are past the deepest substitution, return f. */
	if (//cuddI(dd,f->index) > deepest || 
			//Cudd_ReadPerm(dd,f->index) < 0 ||  //false
			Cudd_ReadPerm(dd,f->index) > deepest  ||  //true
			cuddIsConstant(f)) { //number
		//      	  cout << "Got Const" <<endl;
		return(f);
	}

	if ((res = cuddHashTableLookup1(table,f)) != NULL) {
#ifdef DD_DEBUG
		addGeneralVectorComposeHits++;
#endif
		return(res);
	}
	//     printf("enter T\n"); fflush(stdout);
	/* Split and recur on children of this node. */

	T = cuddAddGeneralVectorComposeRecur1(dd,table,cuddT(f),
			vectorOn,vectorOff,deepest,time);



	//    printf("exit T\n"); fflush(stdout);
	if (T == NULL){
		//   printf("null T\n"); fflush(stdout);
		return(NULL);
	}
	cuddRef(T);
	//     printf("enter E\n"); fflush(stdout);
	E = cuddAddGeneralVectorComposeRecur1(dd,table,cuddE(f),
			vectorOn,vectorOff,deepest,time);

	//     printf("exit E\n"); fflush(stdout);
	// printBDD(E);
	if (E == NULL) {
		//  printf("null E\n"); fflush(stdout);
		Cudd_RecursiveDeref(dd, T);
		return(NULL);
	}
	cuddRef(E);

	/* Retrieve the compose ADDs for the current top variable and call
	 ** cuddAddApplyRecur with the T and E we just created.
	 */

	//     cout << "t and recur" << endl;
	//     if(T == Cudd_ReadOne(dd))
	//       t = cuddAddApplyRecur(dd,Cudd_addTimes,vectorOn[f->index],T);
	//     else{
	//       t = cuddAddApplyRecur(dd,Cudd_addMaximum,vectorOn[f->index],T);

	//     }

	//    printBDD(T);
	//     printBDD(vectorOn[f->index]);

	fr = Cudd_addBddStrictThreshold(dd, vectorOn[f->index],0.0);
	Cudd_Ref(fr);
	fr1 = Cudd_addBddStrictThreshold(dd, T, 0.0);
	Cudd_Ref(fr1);

	tmp = Cudd_bddAnd(dd, fr, fr1);
	Cudd_Ref(tmp);
	Cudd_RecursiveDeref(dd, fr);
	Cudd_RecursiveDeref(dd, fr1);
	fr1 = Cudd_BddToAdd(dd, tmp);
	Cudd_Ref(fr1);
	Cudd_RecursiveDeref(dd, tmp);

	if(T != Cudd_ReadOne(dd))
		fr = Cudd_addApply(dd, Cudd_addMaximum, T, vectorOn[f->index]);
	else
		fr = vectorOn[f->index];
	Cudd_Ref(fr);

	t = Cudd_addApply(dd, Cudd_addTimes, fr, fr1);
	Cudd_RecursiveDeref(dd, fr);
	Cudd_RecursiveDeref(dd, fr1);



	if (t == NULL) {
		Cudd_RecursiveDeref(dd,T);
		Cudd_RecursiveDeref(dd,E);
		return(NULL);
	}
	cuddRef(t);
	//     cout << "e and recur " << f->index << endl;

	//     if(E == Cudd_ReadOne(dd))
	//       e = cuddAddApplyRecur(dd,Cudd_addTimes,vectorOff[f->index],E);
	//     else
	//       e = cuddAddApplyRecur(dd,Cudd_addMaximum,vectorOff[f->index],E);

	//   printBDD(vectorOff[f->index]);

	fr = Cudd_addBddStrictThreshold(dd, vectorOff[f->index],0.0);

	Cudd_Ref(fr);
	fr1 = Cudd_addBddStrictThreshold(dd, E, 0.0);
	Cudd_Ref(fr1);

	tmp = Cudd_bddAnd(dd, fr, fr1);
	Cudd_Ref(tmp);
	Cudd_RecursiveDeref(dd, fr);
	Cudd_RecursiveDeref(dd, fr1);
	fr1 = Cudd_BddToAdd(dd, tmp);
	Cudd_Ref(fr1);
	Cudd_RecursiveDeref(dd, tmp);

	if(E != Cudd_ReadOne(dd))
		fr = Cudd_addApply(dd, Cudd_addMaximum, E, vectorOff[f->index]);
	else
		fr = vectorOff[f->index];
	Cudd_Ref(fr);

	e = Cudd_addApply(dd, Cudd_addTimes, fr, fr1);
	Cudd_RecursiveDeref(dd, fr);
	Cudd_RecursiveDeref(dd, fr1);


	if (e == NULL) {
		Cudd_RecursiveDeref(dd,T);
		Cudd_RecursiveDeref(dd,E);
		Cudd_RecursiveDeref(dd,t);
		return(NULL);
	}
	cuddRef(e);

	//     cout << "et and recur " << f->index << endl;
	//res = cuddAddApplyRecur(dd,Cudd_addPlus,t,e);
	//    printBDD(res);
	//cout << exAbstractAllLabels(res, time-1) <<endl;

	//     printBDD(t);printBDD(e);

	res = cuddAddApplyRecur(dd,Cudd_addMaximum,t,e);
	if (res == NULL) {
		Cudd_RecursiveDeref(dd,T);
		Cudd_RecursiveDeref(dd,E);
		Cudd_RecursiveDeref(dd,t);
		Cudd_RecursiveDeref(dd,e);
		return(NULL);
	}
	cuddRef(res);
	Cudd_RecursiveDeref(dd,T);
	Cudd_RecursiveDeref(dd,E);
	Cudd_RecursiveDeref(dd,t);
	Cudd_RecursiveDeref(dd,e);

	/* Do not keep the result if the reference count is only 1, since
	 ** it will not be visited again
	 */
	//       cout << "check ref" <<endl;

	if (f->ref != 1) {
		ptrint fanout = (ptrint) f->ref;
		cuddSatDec(fanout);
		if (!cuddHashTableInsert1(table,f,res,fanout)) {
			Cudd_RecursiveDeref(dd, res);
			return(NULL);
		}
	}
	cuddDeref(res);
	return(res);

		} /* end of cuddAddGeneralVectorComposeRecur */

/**Function********************************************************************

  Synopsis    [Comparison of a function to the i-th ADD variable.]

  Description [Comparison of a function to the i-th ADD variable. Returns 1 if
  the function is the i-th ADD variable; 0 otherwise.]

  SideEffects [None]

  SeeAlso     []

 ******************************************************************************/
DD_INLINE
static int
ddIsIthAddVar(
		DdManager * dd,
		DdNode * f,
		unsigned int  i)
{
	return(f->index == i && cuddT(f) == DD_ONE(dd) && cuddE(f) == DD_ZERO(dd));

} /* end of ddIsIthAddVar */

/**Function********************************************************************

  Synopsis    [Comparison of a pair of functions to the i-th ADD variable.]

  Description [Comparison of a pair of functions to the i-th ADD
  variable. Returns 1 if the functions are the i-th ADD variable and its
  complement; 0 otherwise.]

  SideEffects [None]

  SeeAlso     []

 ******************************************************************************/
DD_INLINE
static int
ddIsIthAddVarPair(
		DdManager * dd,
		DdNode * f,
		DdNode * g,
		int  i)
{
	//   cout << "i = " << i
	//        << "fi = " <<  f->index
	//        << "gi = " << g->index
	//        << endl;
	//  printBDD(f);
	//  printBDD(g);
	return(f && g && f->index == i && g->index == i &&
			cuddT(f) == DD_ONE(dd) && cuddE(f) == DD_ZERO(dd) &&
			cuddT(g) == DD_ZERO(dd) && cuddE(g) == DD_ONE(dd));

} /* end of ddIsIthAddVarPair */

/**Function********************************************************************

  Synopsis    [Composes an ADD with a vector of ADDs.]

  Description [Given a vector of ADDs, creates a new ADD by substituting the
  ADDs for the variables of the ADD f. vectorOn contains ADDs to be substituted
  for the x_v and vectorOff the ADDs to be substituted for x_v'. There should
  be an entry in vector for each variable in the manager.  If no substitution
  is sought for a given variable, the corresponding projection function should
  be specified in the vector.  This function implements simultaneous
  composition.  Returns a pointer to the resulting ADD if successful; NULL
  otherwise.]

  SideEffects [None]

  SeeAlso [Cudd_addVectorCompose Cudd_addNonSimCompose Cudd_addPermute
  Cudd_addCompose Cudd_bddVectorCompose]

 ******************************************************************************/

DdNode *
Cudd_addGeneralVectorCompose1(
		DdManager * dd,
		DdNode * f,
		DdNode ** vectorOn,
		DdNode ** vectorOff,
		int deepest,
		int time)
		{
	DdHashTable		*table;
	DdNode		*res;
	//   int			deepest;
	int                 i;

	//printBDD(f);

	do {
		dd->reordered = 0;
		/* Initialize local cache. */
		table = cuddHashTableInit(dd,1,2);
		//printf("got table\n"); fflush(stdout);
		if (table == NULL) return(NULL);

		/* Find deepest real substitution. */
		// 	for (deepest = dd->size - 1; deepest >= 0; deepest--) {

		// 	  i = Cudd_ReadInvPerm(dd, deepest);//	    dd->invperm[deepest];
		// 	  if (printf("deep = %d, i = %d \n", deepest, i) &&
		// 	      i > -1 && vectorOn[i] && vectorOff[i] &&
		// 	      !ddIsIthAddVarPair(dd,vectorOn[i],vectorOff[i],i)) {
		// 	  //	    if (vectorOn[i] != Cudd_Not(dd->vars[i]) ||
		// 	  //		vectorOff[i] != dd->vars[i]) {
		// 	      //cout << "deepest = " << deepest << endl;
		// 		break;
		// 	    }
		// 	}
		//	printf("Before recurse\n"); fflush(stdout);
		/* Recursively solve the problem. */
		res = cuddAddGeneralVectorComposeRecur1(dd,table,f,vectorOn,
				vectorOff,deepest, time);
		if (res != NULL) cuddRef(res);

		/* Dispose of local cache. */
		cuddHashTableQuit(table);
	} while (dd->reordered == 1);

	if (res != NULL) cuddDeref(res);
	//printBDD(res);
	return(res);

		} /* end of Cudd_addGeneralVectorCompose */


int Cudd_BddGetLowestVar(DdManager *dd,DdNode * N);

DdNode *  Cudd_bddCubeDiff(DdManager * dd,DdNode * a,DdNode * b);

DdNode * bdd_one(DdManager * dd)
								{
	DdNode * result = Cudd_ReadOne(dd);

	Cudd_Ref(result);
	return((DdNode *)result);
								}

DdNode * bdd_zero(DdManager * dd)
								{
	DdNode * result = Cudd_ReadLogicZero(dd);

	Cudd_Ref(result);
	return((DdNode *)result);
								}

int bdd_is_one(DdManager * dd, DdNode * f)
{
	return((DdNode *)f == Cudd_ReadOne(dd));
}

int bdd_isnot_one(DdManager * dd, DdNode * f)
{
	return((DdNode *)f != Cudd_ReadOne(dd));
}

int bdd_is_zero(DdManager * dd, DdNode * f)
{
	return((DdNode *)f == Cudd_ReadLogicZero(dd));
}

int bdd_isnot_zero(DdManager * dd, DdNode * f)
{
	return((DdNode *)f != Cudd_ReadLogicZero(dd));
}

void bdd_ref(DdNode * dd_node)
{
	Cudd_Ref(dd_node);
}

void bdd_deref(DdNode * dd_node)
{

	Cudd_Deref(dd_node);
}

void bdd_free(DdManager * dd, DdNode * dd_node) 
{

	Cudd_RecursiveDeref(dd, (DdNode *)dd_node);
}

DdNode * bdd_dup(DdNode * dd_node)
								{
	Cudd_Ref(dd_node);
	return(dd_node);
								}

DdNode * bdd_not(DdManager * dd, DdNode * fn)
								{
	DdNode * result;

	result = Cudd_Not(fn);
	Cudd_Ref(result);
	return((DdNode *)result);
								}

DdNode * bdd_and(DdManager * dd, DdNode * a, DdNode * b)
								{
	DdNode * result;

	result = Cudd_bddAnd(dd, (DdNode *)a, (DdNode *)b);
	Cudd_Ref(result);
	return((DdNode *)result);
								}

DdNode * bdd_or(DdManager * dd, DdNode * a, DdNode * b)
								{
	DdNode * result;

	result = Cudd_bddOr(dd, (DdNode *)a, (DdNode *)b);
	Cudd_Ref(result);
	return((DdNode *)result);
								}

DdNode * bdd_xor(DdManager * dd, DdNode * a, DdNode * b)
								{
	DdNode * result;

	result = Cudd_bddXor(dd, (DdNode *)a, (DdNode *)b);
	Cudd_Ref(result);
	return((DdNode *)result);
								}

DdNode * bdd_imply(DdManager * dd, DdNode * a, DdNode * b)
								{
	DdNode * tmp_1;
	DdNode * result;

	tmp_1 = Cudd_Not((DdNode *)a);
	Cudd_Ref(tmp_1);
	result = Cudd_bddOr(dd, tmp_1, (DdNode *)b);
	Cudd_Ref(result);
	Cudd_RecursiveDeref(dd, tmp_1);
	return((DdNode *)result);
								}

DdNode * bdd_nand(DdManager * dd, DdNode * a, DdNode * b)
								{
	DdNode * result;

	result = Cudd_bddNand(dd, (DdNode *)a, (DdNode *)b);
	Cudd_Ref(result);
	return((DdNode *)result);
								}

DdNode * bdd_nor(DdManager * dd, DdNode * a, DdNode * b)
								{
	DdNode * result;

	result = Cudd_bddNor(dd, (DdNode *)a, (DdNode *)b);
	Cudd_Ref(result);
	return((DdNode *)result);
								}

DdNode * bdd_xnor(DdManager * dd, DdNode * a, DdNode * b)
								{
	DdNode * result;

	result = Cudd_bddXnor(dd, (DdNode *)a, (DdNode *)b);
	Cudd_Ref(result);
	return((DdNode *)result);
								}

DdNode * bdd_ite(DdManager * dd, DdNode * i, DdNode * t, DdNode * e)
								{
	DdNode * result;

	result = Cudd_bddIte(dd, (DdNode *)i, (DdNode *)t, (DdNode *)e);
	Cudd_Ref(result);
	return((DdNode *)result);
								}

void bdd_and_accumulate(DdManager * dd, DdNode * * a, DdNode * b)
{
	DdNode * result;

	result = Cudd_bddAnd(dd, (DdNode *)*a, (DdNode *)b);
	Cudd_Ref(result);
	Cudd_RecursiveDeref(dd, (DdNode *)*a);
	*a = result;
	return;
}

void bdd_or_accumulate(DdManager * dd, DdNode * * a, DdNode * b)
{
	DdNode * result;

	result = Cudd_bddOr(dd, (DdNode *)*a, (DdNode *)b);
	Cudd_Ref(result);
	Cudd_RecursiveDeref(dd, (DdNode *) *a);
	*a = result;
	return;
}

DdNode * bdd_forsome(DdManager * dd, DdNode * fn, DdNode * cube)
								{
	DdNode * result;

	result = Cudd_bddExistAbstract(dd, (DdNode *)fn, (DdNode *)cube);
	Cudd_Ref(result);
	return((DdNode *)result);
								}

DdNode * bdd_forall(DdManager * dd, DdNode * fn, DdNode * cube)
								{
	DdNode * result;

	result = Cudd_bddUnivAbstract(dd, (DdNode *)fn, (DdNode *)cube);
	Cudd_Ref(result);
	return((DdNode *)result);
								}

DdNode * bdd_permute(DdManager * dd, DdNode * fn, int * permut)
								{
	DdNode * result;

	result = Cudd_bddPermute(dd, (DdNode *)fn, permut);
	Cudd_Ref(result);
	return((DdNode *)result);
								}

DdNode * bdd_and_abstract(DdManager *dd, DdNode * T, DdNode * S, DdNode * V)
								{
	DdNode * result;

	result = Cudd_bddAndAbstract(dd, (DdNode *)T, (DdNode *)S, (DdNode *)V);
	Cudd_Ref(result);
	return((DdNode *)result);
								}

DdNode * bdd_simplify_assuming(DdManager *dd, DdNode * fn, DdNode * c)
								{
	DdNode * result;

	result = Cudd_bddRestrict(dd, (DdNode *)fn, (DdNode *)c);
	Cudd_Ref(result);
	return((DdNode *)result);
								}

DdNode * bdd_minimize(DdManager *dd, DdNode * fn, DdNode * c)
								{
	DdNode * result;

	result = Cudd_bddRestrict(dd, (DdNode *)fn, (DdNode *)c);
	Cudd_Ref(result);
	return((DdNode *)result);
								} /* end of bdd_minimize */

DdNode * bdd_cofactor(DdManager * dd, DdNode * f, DdNode * g)
								{
	DdNode *result;

	/* We use Cudd_bddConstrain instead of Cudd_Cofactor for generality. */
	result = Cudd_bddConstrain(dd, (DdNode *)f, (DdNode *)g);
	Cudd_Ref(result);
	return((DdNode *)result);
								} /* end of bdd_cofactor */


DdNode * bdd_between(DdManager *dd, DdNode * f_min, DdNode * f_max)
								{
	DdNode * care_set, * ret;

	care_set = bdd_imply(dd, f_min, f_max);
	ret = bdd_minimize(dd, f_min, care_set);
	bdd_free(dd, care_set);
	/*
    The size of ret is never larger than the size of f_min. We need
    only to check ret against f_max.
	 */
	if (bdd_size(dd, f_max) <= bdd_size(dd, ret)) {
		bdd_free(dd, ret);
		return(bdd_dup(f_max));
	} else {
		return(ret);
	}
								} /* end of bdd_between */

int bdd_entailed(DdManager * dd, DdNode * f, DdNode * g)
{
	int result;

	result = Cudd_bddLeq(dd, (DdNode *)f, (DdNode *)g);
	return(result);
}

DdNode * bdd_then(DdManager * dd, DdNode * f)
								{
	DdNode * result;

	if (Cudd_IsConstant((DdNode *)f)) {
		result = (DdNode *)NULL;
	}
	else {
		result = (DdNode *)Cudd_T(f);
	}
	return(result);
								}

DdNode * bdd_else(DdManager * dd, DdNode * f)
								{
	DdNode * result;

	if (Cudd_IsConstant((DdNode *)f)) {
		result = (DdNode *)NULL;
	}
	else {
		result = (DdNode *)Cudd_E(f);
	}
	return(result);
								}

int bdd_iscomplement(DdManager * dd, DdNode * f)
{
	return(Cudd_IsComplement((DdNode *)f));
}

int bdd_readperm(DdManager * dd, DdNode * f)
{
	int result;

	result = Cudd_ReadPerm(dd, Cudd_NodeReadIndex((DdNode *)f));
	return(result);
}

int bdd_index(DdManager * dd, DdNode * f)
{
	int result;

	result = Cudd_NodeReadIndex((DdNode *)f);
	return(result);
}

int bdd_size(DdManager * dd, DdNode * fn)
{
	return(Cudd_DagSize((DdNode *)fn));
}

double bdd_count_minterm(DdManager * dd, DdNode * fn, int nvars)
{
	return Cudd_CountMinterm(dd, fn, nvars);
}

DdNode * bdd_support(DdManager *dd, DdNode * fn)
								{
	DdNode * result;

	result = Cudd_Support(dd, (DdNode *)fn);
	Cudd_Ref(result);
	return((DdNode *)result);
								}

DdNode * bdd_pick_one_minterm(DdManager * dd, DdNode * fn, DdNode * * vars, int n)
								{
	DdNode * result;
	DdNode * zero = bdd_zero(dd);

	if (fn == zero) {
		Cudd_Ref(fn);
		Cudd_Deref(zero);
		return(fn);
	}
	else {
		result = Cudd_bddPickOneMintermNR(dd, (DdNode *)fn, (DdNode **)vars, n);
		Cudd_Deref(zero);
		Cudd_Ref(result);
		return((DdNode *)result);
	}
								}

DdNode * bdd_pick_one_minterm_rand(DdManager * dd, DdNode * fn, DdNode * * vars, int n)
								{
	DdNode * result;
	DdNode * zero = bdd_zero(dd);

	if (fn == zero) {
		Cudd_Ref(fn);
		Cudd_Deref(zero);
		return(fn);
	}
	else {
		result = Cudd_bddPickOneMinterm(dd, (DdNode *)fn, (DdNode **)vars, n);
		Cudd_Deref(zero);
		Cudd_Ref(result);
		return((DdNode *)result);
	}
								}

int bdd_pick_all_terms(
		DdManager * dd           /* dd manager */,
		DdNode *   pick_from_set  /* minterm from which to pick  all term */,
		DdNode *   * vars         /* The array of vars to be put in the returned array */,
		int       vars_dim       /* The size of the above array */,
		DdNode *   * result       /* The array used as return value */,
		int       result_dim     /* The size of the above array */)
{
	// 	printf("foo0\n");
	// 	fflush(stdout);

	if (bdd_is_one(dd, pick_from_set)) {

		// 	printf("foo1\n");
		// 	fflush(stdout);

		DdNode * not_var0 = bdd_not(dd, vars[0]);

		if (Cudd_PickAllTerms(dd, vars[0], vars, vars_dim, result,result_dim/2) == 1) {
			fprintf(dd->err, "Error from Cudd_PickAllTerms.\n");
			bdd_free(dd, not_var0);
			return 1;
		}

		// 	printf("foo2\n");
		// 	fflush(stdout);

		assert((result_dim % 2) == 0);
		if ( Cudd_PickAllTerms(dd, not_var0, vars, vars_dim,
				result + result_dim/2,result_dim/2) == 1 ) {
			fprintf(dd->err, "Error from Cudd_PickAllTerms.\n");
			bdd_free(dd, not_var0);
			return 1;
		}

		// 	printf("foo3\n");
		// 	fflush(stdout);

		bdd_free(dd, not_var0);
	}
	else
	{
		// 	printf("foo4\n");
		// 	fflush(stdout);

		if (Cudd_PickAllTerms(dd, pick_from_set, vars, vars_dim, result,result_dim) == 1) {
			fprintf(dd->err, "Error from Cudd_PickAllTerms.\n");
			return 1;
		}
	}
	return 0;
}

DdNode * bdd_new_var_with_index(DdManager * dd, int index)
								{
	DdNode * result;

	result = Cudd_bddIthVar(dd, index);
	/* bdd var does not require to be referenced when created */
	return bdd_dup((DdNode *) result);
								}


DdNode * bdd_cube_diff(DdManager * dd, DdNode * a, DdNode * b)
								{
	DdNode * result;

	result = Cudd_bddCubeDiff(dd, (DdNode *)a, (DdNode *)b);
	Cudd_Ref(result);
	return((DdNode *)result);
								}

DdNode * bdd_cube_union(DdManager * dd, DdNode * a, DdNode * b)
								{
	DdNode * result;
	result = bdd_and(dd,a,b);
	return(result);
								}

DdNode * bdd_cube_intersection(DdManager * dd, DdNode * a, DdNode * b)
								{
	DdNode * result,*tmp;
	tmp = bdd_cube_diff(dd , a , b);
	result= bdd_cube_diff(dd , a , tmp);
	bdd_free(dd,tmp);
	return(result);
								}

int bdd_get_lowest_index(DdManager * dd, DdNode * a)
{
	int result;

	result = Cudd_BddGetLowestVar(dd, (DdNode *)a);
	return(result);
}

/**Function********************************************************************

  Synopsis           [Returns the array of All Possible Minterms]

  Description        [Takes a minterm and returns an array of all its
  terms, according to variables specified in the array vars[].  Notice
  that the array of the result has to be previously allocated, and its
  size must be greater or equal the number of the minterms of the "minterm"
  function. The array contains referenced BDD so it is necessary to
  dereference them after their use.]

  SideEffects        []

  SeeAlso            []

 ******************************************************************************/
int 
Cudd_PickAllTerms(
		DdManager * dd      /* manager */,
		DdNode *    minterm /* minterm from which to pick  all term */,
		DdNode **   vars    /* The array of the vars to be put in the returned array */,
		int         n       /* The size of the above array */,
		DdNode **   result  /* The array used as return value */,
		int         result_dim /* The size of the above array */)
{
	CUDD_VALUE_TYPE value;
	DdGen * gen;
	int * cube;
	int q;
	int pos = 0;
	int reorder_status = 0;
	Cudd_ReorderingType Reorder_Method;
	int *** matrix;
	int *num_of_mint;
	int num_cube=0;

	matrix = ALLOC(int **, result_dim);
	num_of_mint = ALLOC(int,result_dim);

	if (result == NULL) {
		(void) fprintf(dd->err, "Cudd_PickAllTerms: result == NULL\n");
		return 1;
	}

	/*
    Check the dynamic reordering status. If enabled, then the status
    is saved in order to restore it after the operation. 
	 */
	if (Cudd_ReorderingStatus(dd, &Reorder_Method)) {
		Cudd_AutodynDisable(dd);
		reorder_status = 1;
	}

	int nd;

	Cudd_ForeachCube(dd, minterm, gen, cube, value) {

		// number of indifferent boolean variables
		nd = 0;
		// We build the cube of the minterm.

		//Debugging
		//printf("bar1\n");
		//fflush(stdout);


		for(q = 0; q < n; q++) {
			//Debugging
			//assert(vars);
			//assert(vars[q]);

			//printf("%i\n",vars[q]->index);
			//fflush(stdout);

			switch(cube[vars[q]->index]) {
			case 0:
				//Debugging
				//printf("wanted 0\n");
				//fflush(stdout);
				break;
			case 1:
				//Debugging
				//printf("wanted 1\n");
				//fflush(stdout);
				break;
			default:
				//Debugging
				//printf("unwanted\n");
				//fflush(stdout);
				nd += 1;
				break;
			}
		}

		{ // For each cube we expand the terms, i.e. all the assignments.
			int lim = 0;
			num_of_mint[num_cube] = 1 << nd;

			int l;
			//       for (l=0;l<=num_cube;l++)
			// 	printf("bar2 num_mint %i nd %i\n",num_of_mint[l],nd);
			// 	fflush(stdout);

			matrix[num_cube] = ALLOC(int *, num_of_mint[num_cube]+1);
			if (matrix[num_cube] == NULL) {
				fprintf(dd->err, "Cudd_PickAllTerms: Unable to allocate matrix[]\n");
				return 1;
			}
			{
				int i;
				for (i = 0; i <= num_of_mint[num_cube]; i++) {
					matrix[num_cube][i] = ALLOC(int, n+1);
					if (matrix[num_cube][i] == NULL) {
						fprintf(dd->err, "Cudd_PickAllTerms: Unable to allocate matrix[%d][]\n", i);
						return 1;
					}
				}
			}

			// 	printf("bar3\n");
			// 	fflush(stdout);

			lim = 1;
			for(q = 0; q < n; q++) {
				switch(cube[vars[q]->index]) {
				case 0: {
					int i;
					for(i = 0; i < num_of_mint[num_cube]; i++) matrix[num_cube][i][q] = 0;
					break;
				}
				case 1: {
					int i;
					for(i = 0; i < num_of_mint[num_cube]; i++) matrix[num_cube][i][q] = 1;
					break;
				}
				case 2: {
					//
					//  The current variable is indifferent. It has to be expanded
					//  considering the cases in which it is assigned a true value
					//  and the cases in which it is assigned a false value.

					int i, j;
					i = 0;
					j = (1<<(lim-1));
					for (; j <= num_of_mint[num_cube]; ) {
						for(; i < j  && i < num_of_mint[num_cube]; i++) matrix[num_cube][i][q] = 1;
						j += (1<<(lim-1));
						for(; i < j && i < num_of_mint[num_cube]; i++) matrix[num_cube][i][q] = 0;
						j += (1<<(lim-1));
					}
					lim++;
					break;
				}
				default:
					(void) fprintf(dd->err,"\nCudd_PickAllTerms: unexpected switch value %d\n", cube[vars[q]->index]);
					return 1;
				}
			}

		}

		num_cube++;


	} /* End of Cudd_ForeachCube */


	//printf("bar4 num_cube: %i\n",num_cube);
	//fflush(stdout);

	int c;
	for (c=0; c< num_cube;c++)
	{

		int i, j;

		for (j = 0; j < num_of_mint[c]; j++,pos++) {
			//printf("result num_mint: %i\n",num_of_mint[c]);
			//fflush(stdout);
			result[pos] = Cudd_ReadOne(dd);
			//printf("ref %i\n",result[pos]);
			//fflush(stdout);
			cuddRef(result[pos]);

			// sometimes it is better to start building the cube from
			//   the higher order variables to the lower order variable to
			//   avoid traversal of the BDD so far build to add the last
			//   variable. Usually the orders of variables in vars
			//   corresponds to the order of bdd variables

			for(i = n - 1; i >= 0; i--) {

				// 	printf("For variable i %i",i);
				// 	fflush(stdout);
				DdNode * New;

				switch(matrix[c][j][i]) {
				case 0: {
					New = Cudd_bddAnd(dd, result[pos], Cudd_Not(vars[i]));
					break;
				}
				case 1: {
					New = Cudd_bddAnd(dd, result[pos], vars[i]);
					break;
				}
				default: {
					int k;

					fprintf(dd->err, "Cudd_PickAllTerms: unexpected switch value %d\n", matrix[c][j][i]);
					for(k = 0; k <= pos; k++) Cudd_RecursiveDeref(dd, result[k]);
					for(c=0;c<num_cube;c++)
					{
						for(k = 0; k < num_of_mint[c]; k++) FREE(matrix[c][k]);
						FREE(matrix[c]);
					}
					FREE(matrix);
					FREE(num_of_mint);
					return 1;
				}
				}

				if (New == NULL) {
					int k;

					for(k = 0; k <= pos; k++) Cudd_RecursiveDeref(dd, result[k]);
					for(c=0;c<num_cube;c++)
					{
						for(k = 0; k < num_of_mint[c]; k++) FREE(matrix[c][k]);
						FREE(matrix[c]);
					}
					FREE(matrix);
					FREE(num_of_mint);
					return 1;
				}
				cuddRef(New);
				Cudd_RecursiveDeref(dd, result[pos]);
				result[pos] = New;

			}
		}
	}


	{
		int k;

		for(c=0;c<num_cube;c++)
		{
			for(k = 0; k < num_of_mint[c]; k++) FREE(matrix[c][k]);
			FREE(matrix[c]);
		}
		FREE(matrix);
		FREE(num_of_mint);

	}



	//   printf("bar8\n");
	//   fflush(stdout);

	/* If the case than the dynamic variable ordering is restored. */
	if (reorder_status) Cudd_AutodynEnable(dd, Reorder_Method);
	return 0;
}


DdNode *
Cudd_bddCubeDiff(
		DdManager * dd,
		DdNode * a,
		DdNode * b)
		{
	DdNode * res;

	if (bddCheckPositiveCube(dd, a) == 0) {
		(void) fprintf(dd->err,"Error: (arg_1) Can only abstract positive cubes\n");
		return(NULL);
	}
	if (bddCheckPositiveCube(dd, b) == 0) {
		(void) fprintf(dd->err,"Error: (arg_2) Can only abstract positive cubes\n");
		return(NULL);
	}
	do {
		dd->reordered = 0;
		res = cudd_bddCubeDiffRecur(dd, a, b);
	} while (dd->reordered == 1);
	return(res);
		}

int
Cudd_BddGetLowestVar(
		DdManager *dd,
		DdNode * N)
{
	int res = Cudd_BddGetLowestVarRecur(dd, N, 0);
	ddClearFlag(N);
	return(res);
}

DdNode *
cudd_bddCubeDiffRecur(
		DdManager * dd,
		DdNode    * f,
		DdNode    * g)
		{
	DdNode       * t,  * e , * res;
	DdNode       * tf, * tg;
	unsigned int topf, topg;
	DdNode       * one  = DD_ONE(dd);
	DdNode       * zero = Cudd_Not(one);

	if ((f == zero) || (g == zero))
		Cudd_fatal_error(dd, "cudd_bddCubeDiff: f == ZERO || g == ZERO");
	if (f == one) return(f);

	topf = cuddI(dd,f->index);
	topg = cuddI(dd,g->index);

	if (topf < topg) {
		e = zero;
		cuddRef(e);     tf = cuddT(f);
		t = cudd_bddCubeDiffRecur(dd, tf, g);
		if (t == NULL) {
			cuddDeref(e);
			return(NULL);
		}
		cuddRef(t);
		res = (t == e) ? t : cuddUniqueInter(dd, f->index, t, e);
		if (res == NULL) {
			cuddDeref(e);
			Cudd_RecursiveDeref(dd, t);
			return(NULL);
		}
		cuddDeref(t);
		cuddDeref(e);
		return(res);
	}
	else if (topf == topg) {
		tf = cuddT(f);
		tg = cuddT(g);
		res = cudd_bddCubeDiffRecur(dd, tf, tg);
		return(res);
	}
	else {
		tg = cuddT(g);
		res = cudd_bddCubeDiffRecur(dd, f, tg);
		return(res);
	}
		}

int
Cudd_BddGetLowestVarRecur(
		DdManager *dd,
		DdNode * N,
		int index)
{  
	int i;
	DdNode * RN = Cudd_Regular(N);

	if (Cudd_IsComplement(RN->next) || cuddIsConstant(RN)) return(index);
	RN->next = Cudd_Not(RN->next);
	i = RN->index;
	if (i > index) index = i;
	return(Cudd_BddGetLowestVarRecur(dd, cuddT(RN),
			Cudd_BddGetLowestVarRecur(dd, cuddE(RN), index)));
}  

static void  
ddClearFlag(
		DdNode *f)
{
	if (!Cudd_IsComplement(f->next)) {
		return;
	}
	/* Clear visited flag. */
	f->next = Cudd_Regular(f->next);
	if (cuddIsConstant(f)) {
		return;
	}
	ddClearFlag(cuddT(f));
	ddClearFlag(Cudd_Regular(cuddE(f)));
	return;

}

static int
bddCheckPositiveCube(
		DdManager *manager,
		DdNode    *cube)
{
	if (Cudd_IsComplement(cube)) return(0);
	if (cube == DD_ONE(manager)) return(1);
	if (cuddIsConstant(cube)) return(0);
	if (cuddE(cube) == Cudd_Not(DD_ONE(manager))) {
		return(bddCheckPositiveCube(manager, cuddT(cube)));
	}
	return(0);
}

static CUDD_VALUE_TYPE Cudd_fatal_error(DdManager * dd, const char * message)
{
	fprintf(dd->err,"\nFatal error: %s\n", message);
	return(NULL);
} 

DdNode * Cudd_bddPickOneMintermNR(
		DdManager *dd,
		DdNode *f,
		DdNode **vars,
		int n)
		{
	char *string;
	int i, size;
	int *indices;
	int result;
	DdNode *zero, *old, *newT;

	size = dd->size;
	string = ALLOC(char, size);
	if (string == NULL)
		return(NULL);
	indices = ALLOC(int,n);
	if (indices == NULL) {
		FREE(string);
		return(NULL);
	}

	for (i = 0; i < n; i++) {
		indices[i] = vars[i]->index;
	}

	result = Cudd_bddPickOneCubeNR(dd,f,string);
	if (result == 0) {
		FREE(string);
		FREE(indices);
		return(NULL);
	}

	/*
      Don't cares always set to 0.
      A cube is represented as an array of literals, which are integers in
      {0, 1, 2}; 0 represents a complemented literal, 1 represents an
      uncomplemented literal, and 2 stands for don't care. */

	for (i = 0; i < n; i++) {
		if (string[indices[i]] == 2) {
			/* For dont care we choose false */
			string[indices[i]] = 0;
		}
	}

	/* Build result BDD. */
	old = Cudd_ReadOne(dd);
	cuddRef(old);
	zero = Cudd_Not(Cudd_ReadOne(dd));

	for (i = 0; i < n; i++) {
		if (string[indices[i]] == 0) {
			newT = Cudd_bddIte(dd,old,Cudd_Not(vars[i]),zero);
		} else {
			newT = Cudd_bddIte(dd,old,vars[i],zero);
		}
		if (newT == NULL) {
			FREE(string);
			FREE(indices);
			Cudd_RecursiveDeref(dd,old);
			return(NULL);
		}
		cuddRef(newT);
		Cudd_RecursiveDeref(dd,old);
		old = newT;
	}

	/* Test. */
	if (Cudd_bddLeq(dd,old,f)) {
		cuddDeref(old);
	} else {
		Cudd_RecursiveDeref(dd,old);
		old = NULL;
	}

	/* Test. */
	if (Cudd_bddLeq(dd,old,f)) {
		cuddDeref(old);
	} else {
		Cudd_RecursiveDeref(dd,old);
		old = NULL;
	}

	FREE(string);
	FREE(indices);
	return(old);

		}  /* end of Cudd_bddPickOneMintermNR */
/* NuSMV: added end */


int
Cudd_bddPickOneCubeNR(
		DdManager *ddm,
		DdNode    *node,
		char      *string)
{
	DdNode *N, *T, *E;
	DdNode *one, *bzero;
	int    i;

	if (string == NULL || node == NULL) return(0);

	/* The constant 0 function has no on-set cubes. */
	one = DD_ONE(ddm);
	bzero = Cudd_Not(one);
	if (node == bzero) return(0);

	for (i = 0; i < ddm->size; i++) string[i] = 2;

	if (node == DD_ONE(ddm)) return(1);

	for (;;) {
		N = Cudd_Regular(node);

		T = cuddT(N);
		E = cuddE(N);
		if (Cudd_IsComplement(node)) {
			T = Cudd_Not(T);
			E = Cudd_Not(E);
		}
		if (T == one) {
			string[N->index] = 1;
			break;
		} else if (E == one) {
			string[N->index] = 0;
			break;
		} else if (T == bzero) {
			string[N->index] = 0;
			node = E;
		} else if (E == bzero) {
			string[N->index] = 1;
			node = T;
		} else {
			node = T;
			string[N->index] = 1;
		}
	}
	return(1);

} /* end of Cudd_bddPickOneCubeNR */
/* NuSMV: added end */


//a is an add, b is a bdd
int add_bdd_entailed(DdManager* manager, DdNode *a, DdNode* b){
	//  std::cout << "HI" << std::endl;
	DdNode* fr1 = Cudd_addBddStrictThreshold(manager, a,0.0);
	Cudd_Ref(fr1);

	int r = bdd_entailed(manager, fr1, b);
	Cudd_RecursiveDeref(manager, fr1);

	return r;
}

DdNode* overApprox(DdManager* manager, DdNode *a){
	DdNode* result;
	result = Cudd_RemapOverApprox(manager, a,
			Cudd_ReadSize(manager),
			((double)Cudd_ReadSize(manager)*1.0),
			0.0);

	//   result = Cudd_OverApprox(manager,a,Cudd_ReadSize(manager),
	// 			   10, 0, 0.1);



	Cudd_Ref(result);
	return result;
}

DdNode* underApprox(DdManager* manager, DdNode *a){
	DdNode* result;
	result = Cudd_RemapUnderApprox(manager, a,
			Cudd_ReadSize(manager),
			((double)Cudd_ReadSize(manager)*1.0),
			0.0);

	//   result = Cudd_UnderApprox(manager,a,Cudd_ReadSize(manager),
	// 			    ((double)Cudd_ReadSize(manager)*0.50),
	// 			    0, 1.0);



	Cudd_Ref(result);
	return result;
}


DdNode* Cudd_overApproxAnd(DdManager* manager, DdNode *a, DdNode* b){
	DdNode *aa, *ab, *result;

	aa = overApprox(manager, a);
	Cudd_Ref(aa);
	ab = overApprox(manager, b);
	Cudd_Ref(ab);

	result = Cudd_bddAnd(manager, aa, ab);
	Cudd_RecursiveDeref(manager, aa);
	Cudd_RecursiveDeref(manager, ab);
	return result;
}

DdNode* Cudd_overApproxOr(DdManager* manager, DdNode *a, DdNode* b){
	DdNode *aa, *ab, *result;

	aa = overApprox(manager, a);
	Cudd_Ref(aa);
	ab = overApprox(manager, b);
	Cudd_Ref(ab);

	result = Cudd_bddOr(manager, aa, ab);
	Cudd_RecursiveDeref(manager, aa);
	Cudd_RecursiveDeref(manager, ab);
	//  Cudd_Ref(result);
	//  printBDD(result);

	return result;
}
DdNode* Cudd_underApproxAnd(DdManager* manager, DdNode *a, DdNode* b){
	DdNode *aa, *ab, *result;

	aa = underApprox(manager, a);
	Cudd_Ref(aa);
	ab = underApprox(manager, b);
	Cudd_Ref(ab);

	result = Cudd_bddAnd(manager, aa, ab);
	Cudd_RecursiveDeref(manager, aa);
	Cudd_RecursiveDeref(manager, ab);
	return result;
}

DdNode* Cudd_underApproxOr(DdManager* manager, DdNode *a, DdNode* b){
	DdNode *aa, *ab, *result;

	aa = underApprox(manager, a);
	Cudd_Ref(aa);
	ab = underApprox(manager, b);
	Cudd_Ref(ab);

	result = Cudd_bddOr(manager, aa, ab);
	Cudd_RecursiveDeref(manager, aa);
	Cudd_RecursiveDeref(manager, ab);
	//  Cudd_Ref(result);
	//  printBDD(result);

	return result;
}


static	DdNode	*background, *zero;

static double
ddCountMintermAux1(
		DdNode * node,
		double  max,
		DdHashTable * table)
{
	DdNode	*N, *Nt, *Ne;
	double	min, minT, minE;
	DdNode	*res;

	N = Cudd_Regular(node);

	if (cuddIsConstant(N)) {
		if (node == background || node == zero) {
			return(0.0);
		} else {
			return(max);
		}
	}
	if (N->ref != 1 && (res = cuddHashTableLookup1(table,node)) != NULL) {
		min = cuddV(res);

		if (res->ref == 0) {
			table->manager->dead++;
			table->manager->constants.dead++;
		}
		return(min);
	}

	Nt = cuddT(N); Ne = cuddE(N);
	if (Cudd_IsComplement(node)) {
		Nt = Cudd_Not(Nt); Ne = Cudd_Not(Ne);
	}

	minT = ddCountMintermAux1(Nt,max,table);
	if (minT == (double)CUDD_OUT_OF_MEM) return((double)CUDD_OUT_OF_MEM);
	minE = ddCountMintermAux1(Ne,max,table);
	if (minE == (double)CUDD_OUT_OF_MEM) return((double)CUDD_OUT_OF_MEM);

	if(minE != 0.0 && minT != 0.0){
		minT *= 0.5;
		minE *= 0.5;
	}
	min = (minT + minE);

	if (N->ref != 1) {
		ptrint fanout = (ptrint) N->ref;
		cuddSatDec(fanout);
		res = cuddUniqueConst(table->manager,min);
		if (!cuddHashTableInsert1(table,node,res,fanout)) {
			cuddRef(res); Cudd_RecursiveDeref(table->manager, res);
			return((double)CUDD_OUT_OF_MEM);
		}
	}

	return(min);

} /* end of ddCountMintermAux */

double
Cudd_CountMinterm1(
		DdManager * manager,
		DdNode * node,
		int  nvars)
{
	double	max;
	DdHashTable	*table;
	double	res;
	CUDD_VALUE_TYPE epsilon;

	background = manager->background;
	zero = Cudd_Not(manager->one);

	max = pow(2.0,(double)nvars);
	//max = (double)nvars*log(2.0);

	// cout << "MAX = " << max << endl;

	table = cuddHashTableInit(manager,1,2);
	if (table == NULL) {
		return((double)CUDD_OUT_OF_MEM);
	}
	epsilon = Cudd_ReadEpsilon(manager);
	Cudd_SetEpsilon(manager,(CUDD_VALUE_TYPE)0.0);
	res = ddCountMintermAux1(node,max,table);
	cuddHashTableQuit(table);
	Cudd_SetEpsilon(manager,epsilon);

	return(res);

} /* end of Cudd_CountMinterm */

void writeBDD(DdNode* dd, std::ostream *o){
	//  (*o) << "DD" << std::endl;

	//  file filep("tmp");


	//   Dddmp_cuddBddStore(manager,
	// 		     NULL, //char * ddname, dd name (or NULL)
	// 		     dd, //DdNode * f, BDD root to be stored
	// 		     NULL, //char ** varnames, array of variable names (or NULL)
	// 		     auxids,//int * auxids, array of converted var ids
	// 		     0,//int  mode, storing mode selector
	// 		     NULL, //Dddmp_VarInfoType  varinfo, extra info for variables in text mode
	// 		     "tmp",//char * fname, file name
	// 		     filep //FILE * fp pointer to the store file
	// )
}


extern double get_sum(DdNode* d);
/* Computes the KL distance of P given Q */
double KLD(DdManager* manager, DdNode* P, DdNode* Q){
	std::cout << "enter" << std::endl;
	printBDD(P);
	printBDD(Q);
	DdNode *PdivQ = Cudd_addApply(manager, Cudd_addDivide, P, Q);
	Cudd_Ref(PdivQ);
	printBDD(PdivQ);
	DdNode *logPdivQ = Cudd_addLog(manager, PdivQ);
	Cudd_Ref(logPdivQ);
	printBDD(logPdivQ);
	DdNode *PlogPdivQ = Cudd_addApply(manager, Cudd_addTimes, P, logPdivQ);
	Cudd_Ref(PlogPdivQ);
	printBDD(PlogPdivQ);
	double r = get_sum(PlogPdivQ);

	Cudd_RecursiveDeref(manager, PlogPdivQ);
	Cudd_RecursiveDeref(manager, logPdivQ);
	Cudd_RecursiveDeref(manager, PdivQ);
	std::cout << "exit " << r << std::endl;
	return r;
}

//forward decl.
void populate_pr_map(std::map<DdNode*, double>* node_prs, DdNode* dd, int deepeset);
void create_samples(std::map<DdNode*,int>* samples, DdNode* sample,  std::map<DdNode*, double>* node_prs, DdNode* dd, int size, int num);
extern DdManager* manager;
#include "lug/randomGenerator.h"
extern randomGenerator* randomGen;

DdNode* make_sample_index(int index){
	DdNode* value = rbpf_sample_map[index];

	if(value == NULL){
		//		std::cout << "new" << std::endl;
		//	DdNode *varDD = Cudd_ReadZero(manager);
		//	Cudd_Ref(varDD);

		//	int ji=0;
		//for(map<int, double>::iterator j = prs->begin(); j != prs->end(); j++, ji++){
		int bitmask = 1;//0x80000000;
		value = Cudd_ReadOne(manager);
		Cudd_Ref(value);
		int start_index = (2*num_alt_facts)+max_num_aux_vars;

		for(int k=0; k < rbpf_bits; k++) {
			int bitIndex = start_index+k;

			DdNode* bi = Cudd_bddIthVar(manager, bitIndex);
			Cudd_Ref(bi);
			//Cudd_Ref(bi);
			DdNode* bit = (((index & bitmask) == 0) ? Cudd_Not(bi) : bi);
			Cudd_Ref(bit);
			//Cudd_Ref(bit);



			DdNode *t = Cudd_bddAnd(manager, value, bit);
			Cudd_Ref(t);
			Cudd_RecursiveDeref(manager, value);
			Cudd_RecursiveDeref(manager, bit);
			Cudd_RecursiveDeref(manager, bi);
			value = t;
			Cudd_Ref(value);
			Cudd_RecursiveDeref(manager, t);
			bitmask <<= 1;
		}
		rbpf_sample_map[index] = value;
		Cudd_Ref(value);
	}

	return value;
}

DdNode* expandSampledMinterm(DdNode *t, int sample_index, int num_samples, std::set<int>* varsToExpand, int max_samples){


	DdNode *result = Cudd_ReadLogicZero(manager);
	Cudd_Ref(result);
	//	int supports = Cudd_SupportSize(manager, t);
	//int toExpand = 1;
	//	if(!(supports == 0 && varsToExpand->size() == 1))
	//		toExpand = pow(2,varsToExpand->size()-supports);


	for(int j = 0; j < num_samples; j++){
			//std::cout << "sample: " << j << std::endl;

		DdNode* sample = t;
		Cudd_Ref(sample);
		for(std::set<int>::iterator  i = varsToExpand->begin(); i != varsToExpand->end(); i++){
			int var = *i;
			if(!Cudd_bddIsVarEssential(manager, t, var, 1) &&
					!Cudd_bddIsVarEssential(manager, t, var, 0)){
				//std::cout << "add var: " << i << std::endl;
				double s = ((double)randomGen->myrand()/((double)(RAND_MAX)+(double)(1)));
				DdNode *vi1 = Cudd_bddIthVar(manager, var);
				Cudd_Ref(vi1);
				DdNode* vi;
				if(s < 0.5){
					vi = vi1;
					Cudd_Ref(vi);
				}
				else{
					vi = Cudd_Not(vi1);
					Cudd_Ref(vi);
				}
				DdNode *vit = Cudd_bddAnd(manager, vi, sample);
				Cudd_Ref(vit);
				Cudd_RecursiveDeref(manager, vi);
				Cudd_RecursiveDeref(manager, sample);
				sample = vit;
				Cudd_Ref(sample);
				Cudd_RecursiveDeref(manager, vit);
				Cudd_RecursiveDeref(manager, vi1);
				//				printBDD(sample);
			}
		}
		//
		//    DdNode * addSample = Cudd_BddToAdd(manager, sample);
		//Cudd_Ref(addSample);

		int ns = sample_index+j;
		if(ns >= max_samples)
		  ns -= max_samples;
		assert(ns < max_samples);

		DdNode *sampleIndex = make_sample_index(ns);
		//all_samples.front();
		//Cudd_Ref(sampleIndex);


		//all_samples.remove(sampleIndex);
		//samples.push_back(sampleIndex);
		//new_samples.push_back(sampleIndex);

//		printBDD(sample);
//		printBDD(sampleIndex);

		DdNode *indexedSample = Cudd_bddAnd(manager, sample, sampleIndex);
		Cudd_Ref(indexedSample);

		//		printBDD(indexedSample);

		DdNode *tmp = Cudd_bddOr(manager, result, indexedSample);
		Cudd_Ref(tmp);
		Cudd_RecursiveDeref(manager, result);

		result = tmp;
		Cudd_Ref(result);
		Cudd_RecursiveDeref(manager, tmp);
		Cudd_RecursiveDeref(manager, indexedSample);
		Cudd_RecursiveDeref(manager, sample);

	}
	//	std::cout << "[" << std::endl;
	//				Cudd_CheckKeys(manager);
	//				Cudd_CheckKeys(manager);
	//				std::cout << "|" << std::endl;
	//	Cudd_CheckKeys(manager);
	//					std::cout << "]" << std::endl;


//	std::cout << "result:" << std::endl;
//	printBDD(result);
	return result;
}

DdNode* expandSampledMinterms(std::map<DdNode*, int> *t, std::set<int>* varsToExpand, int num_samples){

	//  DdNode **t1 = extractTermsFromMinterm(t);
	//printBDD(t1[0]);

	//	int* cube = (RBPF_SAMPLES > 0 ? new int[2*num_alt_facts+max_num_aux_vars+2*rbpf_bits] : new int[2*num_alt_facts+max_num_aux_vars]);
	//	CUDD_VALUE_TYPE value;
	//	DdGen* gen = Cudd_FirstCube(manager,t, &cube, &value);
	DdNode * result = Cudd_ReadLogicZero(manager);
	Cudd_Ref(result);

	int offset = ((double)randomGen->myrand()/((double)(RAND_MAX)+(double)(1)))*num_samples;
	int sample_index = offset;
	

	//	do {
	for(std::map<DdNode*, int>::iterator i = t->begin(); i != t->end(); i++){
		//		if(cube == NULL)
		//			break;

		//		DdNode* tmp_cube = Cudd_CubeArrayToBdd(manager, cube);
		//		Cudd_Ref(tmp_cube);

//		std::cout << "Cube: " << (*i).second << std::endl;
//		printBDD((*i).first);

	  DdNode * t = expandSampledMinterm((*i).first, sample_index, (*i).second, varsToExpand, num_samples);
		Cudd_Ref(t);

		sample_index += (*i).second ;
		if(sample_index >= num_samples)
		  sample_index -= num_samples;
		assert(sample_index < num_samples);
		DdNode *t1 = Cudd_bddOr(manager,
				t,
				result);
		Cudd_Ref(t1);

		Cudd_RecursiveDeref(manager, result);
		result = t1;
		Cudd_Ref(result);
		Cudd_RecursiveDeref(manager, t1);
		Cudd_RecursiveDeref(manager, t);
	//	    printBDD(result);
		//Cudd_RecursiveDeref(manager, tmp_cube);
	} // while(!Cudd_IsGenEmpty(gen) && Cudd_NextCube(gen, &cube, &value));
	//  std::cout << "done gen cubes" << std::endl;

	//Cudd_GenFree(gen);
	// delete [] cube;
	return result;
}


/*draw a set of n samples from a DD that is a probability distribution */
DdNode* draw_samples(DdNode* dd, int n){
	//  std::cout << "Drawing n = " << n << " Samples from:" << std::endl;
	//     printBDD(dd);

	std::map<DdNode*, double>* node_prs = new std::map<DdNode*, double>();

	populate_pr_map(node_prs, dd, manager->size);


	DdNode *sample = Cudd_ReadOne(manager);//Cudd_addConst(manager, (double)n);
	Cudd_Ref(sample);
	//	new_samples.clear();
	std::map<DdNode*, int> samples;
	create_samples(&samples, sample, node_prs, dd, 2*num_alt_facts, n);
	//Cudd_Ref(t);

	// printBDD(t);

	std::set<int> varsToExpand;
	for(int i = 0; i < num_alt_facts; i++)
		varsToExpand.insert(2*i);


	//std::cout << "Done" << std::endl;
	DdNode * res = expandSampledMinterms(&samples, &varsToExpand, n);
	Cudd_Ref(res);
	//std::cout << "Done1" << std::endl;


	delete node_prs;
	return res;
}



/*draw a set of n samples from a DD that is a probability distribution */
DdNode* draw_cpt_samples(DdNode* dd, int n, std::set<int>* varsToExpand){
	//std::cout << "Drawing n = " << n << " Samples from:" << std::endl;
	//printBDD(dd);
	std::map<DdNode*, double>* node_prs = new std::map<DdNode*, double>();

	populate_pr_map(node_prs, dd, manager->size);

	DdNode *sample = Cudd_ReadOne(manager);//Cudd_addConst(manager, (double)n);
	Cudd_Ref(sample);
	//new_samples.clear();
	//DdNode * t =
	std::map<DdNode*, int> samples;
	create_samples(&samples, sample, node_prs, dd, 2*num_alt_facts+max_num_aux_vars+2*rbpf_bits, n);
	//Cudd_Ref(t);

	//printBDD(t);



	//std::cout << "Done" << std::endl;
	DdNode * res = expandSampledMinterms(&samples, varsToExpand, n);
	Cudd_Ref(res);
	//std::cout << "Done1" << std::endl;

	DdNode* t1 = Cudd_BddToAdd(manager, res);
	Cudd_Ref(t1);
	Cudd_RecursiveDeref(manager, res);



	delete node_prs;
	return t1;
}



void create_samples(std::map<DdNode*, int> * samples,
		DdNode* sample, //starts as One
		std::map<DdNode*, double>* node_prs,
		DdNode* dd,
		int deepest,
		int num_samples){

	if(sample == Cudd_ReadLogicZero(manager))
		return;

// 	std::cout << "Creating samples: " << Cudd_ReadPerm(manager,dd->index) << " " <<  dd->index << " " << deepest << std::endl;
// 	printBDD(sample);
// 	printBDD(dd);


	if (//Cudd_ReadPerm(manager,
	    dd->index > deepest   ||  //true
	    	cuddIsConstant(dd)
	    ) { //number
		Cudd_Ref(sample);
		(*samples)[sample]=num_samples;

		//		std::cout << "Got sample: num = " << num_samples << std::endl;
		//		printBDD(sample);
		return;
	}
	else if(sample && num_samples > 0){

		//create samples for T and E
//		DdNode *T = sample;//Cudd_ReadZero(manager);
//		Cudd_Ref(T);
//		DdNode *E = sample;//Cudd_ReadZero(manager);
//		Cudd_Ref(E);

		DdNode *tmp;
		double tv = (*node_prs)[cuddT(dd)];
		double ev = (*node_prs)[cuddE(dd)];
		double sum = tv+ev;
		double norm_tv=tv/sum;
		int tc = 0, ec = 0;

		if(ev == 0){
//			tmp = Cudd_addApply(manager, Cudd_addPlus, T, Cudd_addConst(manager, num_samples));
//			Cudd_Ref(tmp);
//			Cudd_RecursiveDeref(manager, T);
//			T = tmp;
//			Cudd_Ref(T);
//			Cudd_RecursiveDeref(manager, tmp);
			tc = num_samples;
		}
		else if(tv == 0){
//			tmp = Cudd_addApply(manager, Cudd_addPlus, E,   Cudd_addConst(manager, num_samples));
//			Cudd_Ref(tmp);
//			Cudd_RecursiveDeref(manager, E);
//			E = tmp;
//			Cudd_Ref(E);
//			Cudd_RecursiveDeref(manager, tmp);
			ec = num_samples;
		}
		else{


			//double offset = ((double)randomGen->myrand()/((double)(RAND_MAX)+(double)(1)));
			//double inc =  ((double)1/(double)num_samples);
			//double value = offset;
			for(int i = 0; i < num_samples; i++){
				// 	std::cout << "i = " << i
				// 		  << " s = " << s
				// 		  << " T = " << (*node_prs)[cuddT(dd)]
				// 		  << " E = " << (*node_prs)[cuddE(dd)]
				// 		  << std::endl;

			double offset = ((double)randomGen->myrand()/((double)(RAND_MAX)+(double)(1)));
			//double inc =  ((double)1/(double)num_samples+1);
			double value = offset;

				if(value >= norm_tv){
//					std::cout << "sample E" << std::endl;
//					tmp = Cudd_addApply(manager, Cudd_addMinus, T, Cudd_ReadOne(manager));
//					Cudd_Ref(tmp);
//					Cudd_RecursiveDeref(manager, T);
//					T = tmp;
//					Cudd_Ref(T);
//					Cudd_RecursiveDeref(manager, tmp);
					ec++;
				}
				else{

//					std::cout << "sample T" << std::endl;
//					tmp = Cudd_addApply(manager, Cudd_addMinus, E, Cudd_ReadOne(manager));
//					Cudd_Ref(tmp);
//					Cudd_RecursiveDeref(manager, E);
//					E = tmp;
//					Cudd_Ref(E);
//					Cudd_RecursiveDeref(manager, tmp);
					tc++;

				}
				//value += inc;
				//if(value > 1) value -= 1;


			}
//			E = Cudd_addApply(manager, Cudd_addMinus, sample, T);
//			Cudd_Ref(E);

		}





//		printBDD(T);
//		printBDD(E);

		if(tc>0){
//			std::cout << "recurse T" << std::endl;
			DdNode * ts = Cudd_bddIte(manager, Cudd_bddIthVar(manager, dd->index), sample, Cudd_ReadLogicZero(manager));
			Cudd_Ref(ts);
			//		DdNode * ts1 = Cudd_addIte(manager, sample, ts, Cudd_ReadZero(manager));
			//		Cudd_Ref(ts1);
			//		//DdNode * t =
			//		printBDD(T);
//			printBDD(ts);
			create_samples(samples, ts, node_prs, cuddT(dd), deepest, tc);
			//Cudd_Ref(t);
			Cudd_RecursiveDeref(manager, ts);
			//Cudd_RecursiveDeref(manager, ts1);
//			std::cout << "exit T" << std::endl;
		}
		if(ec>0){

//			std::cout << "recurse E" << std::endl;
			DdNode * es = Cudd_bddIte(manager, Cudd_bddIthVar(manager, dd->index), Cudd_ReadLogicZero(manager), sample);
			Cudd_Ref(es);
			//		DdNode * es1 = Cudd_addIte(manager, sample, es, Cudd_ReadZero(manager));
			//		Cudd_Ref(es1);

			//DdNode * e =
			//		printBDD(E);
//			printBDD(es);
			create_samples(samples, es, node_prs, cuddE(dd), deepest, ec);
//			std::cout << "exit E" << std::endl;

			Cudd_RecursiveDeref(manager, es);
			//Cudd_RecursiveDeref(manager, es1);
			//		samples = Cudd_addIthVar(manager, dd->index);
			//		Cudd_Ref(samples);
		}
		//
		//		     std::cout << "ite: " << std::endl;
		//		     printBDD(dd);
		//		     printBDD(samples);
		//		     printBDD(t);
		//		     printBDD(cuddT(dd));
		//		     printBDD(e);
		//		     printBDD(cuddE(dd));
		//perform surgery on samples DD
		//		DdNode *tmp1 = Cudd_addIte(manager, samples, t, e);
		//		Cudd_Ref(tmp1);
		//		Cudd_RecursiveDeref(manager, samples);
		//		samples = tmp1;
		//		Cudd_Ref(samples);
		//		Cudd_RecursiveDeref(manager, tmp1);



		//		    printBDD(samples);

		//return samples;
	}


}


void populate_pr_map(std::map<DdNode*, double>* node_prs, 
		DdNode* dd,
		int deepest){

	if (Cudd_ReadPerm(manager,dd->index) > deepest  ||  //true
			cuddIsConstant(dd)) { //number
		//    std::cout << "Leaf: " << (double)dd->type.value << std::endl;
		(*node_prs)[dd]= (double)dd->type.value;
	}
	else{
		//    std::cout << "T: " << std::endl;
		//    printBDD(cuddT(dd));
		populate_pr_map(node_prs, cuddT(dd), deepest);
		//    std::cout << "E: " << std::endl;
		//    printBDD(cuddE(dd));
		populate_pr_map(node_prs, cuddE(dd), deepest);
		(*node_prs)[dd]= (*node_prs)[cuddT(dd)] + (*node_prs)[cuddE(dd)];
		//    std::cout << "Interior: " << (*node_prs)[dd] << std::endl;
		//    printBDD(dd);
	}

}

DdNode *
Cudd_addStrictThreshold(
  DdManager * dd,
  DdNode ** f,
  DdNode ** g)
{
    DdNode *F, *G;

    F = *f; G = *g;
    if (//F == G || 
			F == DD_PLUS_INFINITY(dd)) return(F);
    if (cuddIsConstant(F) && cuddIsConstant(G)) {
	if (cuddV(F) > cuddV(G)) {
	    return(F);
	} else {
	    return(DD_ZERO(dd));
	}
    }
    return(NULL);

} /* end of Cudd_addThreshold */

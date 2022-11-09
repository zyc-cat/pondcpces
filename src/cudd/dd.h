/**CHeaderFile*****************************************************************

  FileName    [dd.h]

  PackageName [dd]

  Synopsis    [Header file for Decisison Diagram Package.]

  Description [External functions and data strucures of the DD
  package. The BDD or ADD returned as a result of an operation are
  always referenced (see the CUDD User Manual for more details about
  this), and need to be dereferenced when the result is no more
  necessary to computation, in order to release the memory associated
  to it when garbage collection occurs.]

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

  Revision    [$Id: dd.h,v 1.3 2006/11/02 16:53:59 dan Exp $]

******************************************************************************/

#ifndef _DD_H
#define _DD_H

#include "cudd.h"
#include "cuddInt.h"
#include <iostream>
#include <set>



#define EXTERN extern
#define ARGS(x) x
#undef FREE
#undef ALLOC
#define FREE(x) delete x
#define ALLOC(x,y) new x[y]

EXTERN void     bdd_ref                 ARGS((DdNode *));
EXTERN void     bdd_deref               ARGS((DdNode *));
EXTERN DdNode *  bdd_dup                 ARGS((DdNode *));
EXTERN DdNode *  bdd_one                 ARGS((DdManager *));
EXTERN DdNode *  bdd_zero                ARGS((DdManager *));
EXTERN int      bdd_is_one              ARGS((DdManager *, DdNode *));
EXTERN int      bdd_is_zero             ARGS((DdManager *, DdNode *));
EXTERN int      bdd_isnot_one              ARGS((DdManager *, DdNode *));
EXTERN int      bdd_isnot_zero             ARGS((DdManager *, DdNode *));
EXTERN void     bdd_free                ARGS((DdManager *, DdNode *));
EXTERN DdNode *  bdd_not                 ARGS((DdManager *, DdNode *));
EXTERN DdNode *  bdd_and                 ARGS((DdManager *, DdNode *, DdNode *));
EXTERN DdNode *  bdd_nand                 ARGS((DdManager *, DdNode *, DdNode *));
EXTERN DdNode *  bdd_nor                 ARGS((DdManager *, DdNode *, DdNode *));
EXTERN DdNode *  bdd_xnor                 ARGS((DdManager *, DdNode *, DdNode *));
EXTERN void     bdd_and_accumulate      ARGS((DdManager *, DdNode * *, DdNode *));
EXTERN DdNode *  bdd_or                  ARGS((DdManager *, DdNode *, DdNode *));
EXTERN void     bdd_or_accumulate       ARGS((DdManager *, DdNode * *, DdNode *));
EXTERN DdNode *  bdd_xor                 ARGS((DdManager *, DdNode *, DdNode *));
EXTERN DdNode *  bdd_imply               ARGS((DdManager *, DdNode *, DdNode *));
EXTERN DdNode *  bdd_forsome             ARGS((DdManager *, DdNode *, DdNode *));
EXTERN DdNode *  bdd_forall              ARGS((DdManager *, DdNode *, DdNode *));
EXTERN DdNode *  bdd_permute             ARGS((DdManager *, DdNode *, int *));
EXTERN DdNode *  bdd_and_abstract        ARGS((DdManager *, DdNode *, DdNode *, DdNode *));
EXTERN DdNode *  bdd_simplify_assuming   ARGS((DdManager *, DdNode *, DdNode *));
EXTERN DdNode *  bdd_minimize            ARGS((DdManager *, DdNode *, DdNode *));
EXTERN DdNode *  bdd_cofactor            ARGS((DdManager *, DdNode *, DdNode *));
EXTERN DdNode *  bdd_between             ARGS((DdManager *, DdNode *, DdNode *));
EXTERN int      bdd_entailed            ARGS((DdManager * dd, DdNode * f, DdNode * g));
EXTERN DdNode *  bdd_then                ARGS((DdManager *, DdNode *));
EXTERN DdNode *  bdd_else                ARGS((DdManager *, DdNode *));
EXTERN DdNode *  bdd_ite                 ARGS((DdManager *, DdNode *, DdNode *, DdNode *));
EXTERN int      bdd_iscomplement        ARGS((DdManager *, DdNode *));
EXTERN int      bdd_readperm            ARGS((DdManager *, DdNode *));
EXTERN int      bdd_index               ARGS((DdManager *, DdNode *));
EXTERN DdNode *  bdd_pick_one_minterm    ARGS((DdManager *, DdNode *, DdNode * *, int));
EXTERN DdNode *  bdd_pick_one_minterm_rand  ARGS((DdManager *, DdNode *, DdNode * *, int));
EXTERN int      bdd_pick_all_terms      ARGS((DdManager *, DdNode *,  DdNode * *, int, DdNode * *, int));
EXTERN DdNode *  bdd_support             ARGS((DdManager *, DdNode *));
EXTERN int      bdd_size                ARGS((DdManager *, DdNode *));
EXTERN double   bdd_count_minterm       ARGS((DdManager *, DdNode *, int));
EXTERN DdNode *  bdd_new_var_with_index  ARGS((DdManager *, int));
EXTERN DdNode *  bdd_cube_diff           ARGS((DdManager *, DdNode *, DdNode *));
EXTERN DdNode *  bdd_cube_union          ARGS((DdManager *, DdNode *, DdNode *));
EXTERN DdNode *  bdd_cube_intersection   ARGS((DdManager *, DdNode *, DdNode *));
EXTERN int      bdd_get_lowest_index    ARGS((DdManager *, DdNode *));

EXTERN DdNode * Cudd_bddVectorCompose(  DdManager * dd,  DdNode * f,  DdNode ** pvector,  DdNode ** nvector);
EXTERN DdNode * Cudd_bddVectorCompose1(  DdManager * dd,  DdNode * f,  DdNode ** pvector,  DdNode ** nvector, int deepest);
EXTERN DdNode * Cudd_addGeneralVectorCompose1(  DdManager * dd,  DdNode * f,  DdNode ** pvector,  DdNode ** nvector, int n, int time);
// included for legacy pond support
EXTERN int Cudd_PickAllTerms(
DdManager * dd      /* manager */,
DdNode *    minterm /* minterm from which to pick  all term */,
DdNode **   vars    /* The array of the vars to be put in the returned array */,
int         n       /* The size of the above array */,
DdNode **   result  /* The array used as return value */,
int   result_dim  /* The size of the above array */);

// also for legacy pond support, called by above
EXTERN DdNode * Cudd_bddPickOneMintermNR ARGS((DdManager *dd, DdNode *fn, DdNode **vars, int n));

// called by above, legacy support.  (dependence upon nusmv customizations to cudd)
int Cudd_bddPickOneCubeNR(DdManager *ddm,DdNode    *node,char      *string);
int add_bdd_entailed(DdManager* manager, DdNode *a, DdNode* b);
DdNode* overApprox(DdManager* manager, DdNode *a);
DdNode* Cudd_overApproxAnd(DdManager* manager, DdNode *a, DdNode* b);
DdNode* Cudd_overApproxOr(DdManager* manager, DdNode *a, DdNode* b);
DdNode* underApprox(DdManager* manager, DdNode *a);
DdNode* Cudd_underApproxAnd(DdManager* manager, DdNode *a, DdNode* b);
DdNode* Cudd_underApproxOr(DdManager* manager, DdNode *a, DdNode* b);
double Cudd_CountMinterm1(DdManager * manager, DdNode * node, int  nvars);
void writeBDD(DdNode* dd, std::ostream *o);
double KLD(DdManager *m, DdNode* P, DdNode* Q);
DdNode* draw_samples(DdNode*, int);
DdNode* make_sample_index(int i);
DdNode* draw_cpt_samples(DdNode*, int, std::set<int>*);
DdNode *
Cudd_addStrictThreshold(
  DdManager * dd,
  DdNode ** f,
  DdNode ** g);
#endif /* _DD_H */

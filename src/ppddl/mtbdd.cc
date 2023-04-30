/*
 * Copyright (C) 2004 Carnegie Mellon University
 * Written by H�kan L. S. Younes.
 *
 * Permission is hereby granted to distribute this software for
 * non-commercial research purposes, provided that this copyright
 * notice is included with any such distribution.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
 * EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE.  THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE OF THE
 * SOFTWARE IS WITH YOU.  SHOULD THE PROGRAM PROVE DEFECTIVE, YOU
 * ASSUME THE COST OF ALL NECESSARY SERVICING, REPAIR OR CORRECTION.
 *
 * $Id: mtbdd.cc,v 1.12 2007/04/10 18:53:52 dan Exp $
 */
#include "mtbdd.h"
#include "problems.h"
#include "domains.h"
#include "effects.h"
#include "formulas.h"
#include "functions.h"
#include <util.h>
#include <cudd.h>
#include <map>
#include <list>
#include <stdexcept>
#include <typeinfo>
#include "globals.h"
//#include "stl.h"
#include <stdlib.h>
#include <stdio.h>
#include "dd.h"
#include <exception>
#include <assert.h>
#include <math.h>

#include "dbn.h"
#include "lao_wrapper.h"
#include <float.h>
#include "solve.h"
#include "graph_wrapper.h"  // pickKRandomWorlds

extern int gnum_cond_effects;
/* Verbosity level. */
extern int verbosity;
extern void set_cubes();

DdNode* col_cube;
// /* The reward function. */
extern  Function reward_function;
// /* Whether the current domain defines a reward function. */
// static bool valid_reward_function;
// /* State variables for the current problem. */
// static std::map<const Atom*, int> state_variables;
// /* A mapping from variable indices to atoms. */
// static std::map<int, const Atom*> dynamic_atoms;
// /* DD manager. */
static DdManager* dd_man;
// /* Total number of state variables. */
static int nvars;
// /* BDDs representing identity between the `current state' and `next
//    state' versions of a variable. */
// static std::vector<DdNode*> identity_bdds;
// /* BDD representing identity between the `current state' and `next
//    state' versions of all state variables. */
// static DdNode* identity_bdd;
// /* MTBDDs representing transition probability matrices for actions. */
// static std::map<const Action*, DdNode*> action_transitions;
// /* MTBDDs representing reward vector for actions. */
// static std::map<const Action*, DdNode*> action_rewards;
// /* Mapping from action ids to actions used by current policy. */
static std::map<size_t, const Action*> policy_actions;

static __gnu_cxx::hash_set<const Atom*> init_variables;

/* ====================================================================== */
/* state_bdd */

/*
 * Returns a BDD representing the given state.
 */
static DdNode* state_bdd(DdManager* dd_man,
		const std::map<int, const Atom*>& dynamic_atoms,
		const AtomSet& atoms) {
	/* This is going to be the BDD representing the given state. */
	DdNode* dds = Cudd_ReadOne(dd_man);
	Cudd_Ref(dds);

	/*
	 * Set Boolean state variables to the values specified by the given
	 * atom set.
	 */
	for (std::map<int, const Atom*>::const_reverse_iterator ai =
			dynamic_atoms.rbegin();
			ai != dynamic_atoms.rend(); ai++) {
		int i = (*ai).first;
		DdNode* ddv = Cudd_bddIthVar(dd_man, 2*i);
		if (atoms.find((*ai).second) == atoms.end()) {
			ddv = Cudd_Not(ddv);
		}
		DdNode* ddt = Cudd_bddAnd(dd_man, ddv, dds);
		Cudd_Ref(ddt);
		Cudd_RecursiveDeref(dd_man, dds);
		dds = ddt;
	}

	return dds;
}


/* ====================================================================== */
/* collect_state_variables */
/*
 * Collects init state variables from the given formula.
 */
static void collect_init_state_variables(const StateFormula& formula) {

	// 是一个常数，结束
	if (typeid(formula) == typeid(Constant)) {
		/*
		 * The formula is either TRUE or FALSE, so it contains no state
		 * variables.
		 */
		return;
	}
	// 是一个atom
	const Atom* af = dynamic_cast<const Atom*>(&formula);
	if (af != NULL) {
		/*
		 * The formula is an atom representing a single state variable.
		 */
		if (init_variables.find(af) == init_variables.end()) {
			//std::cout << "got var" << std::endl;
			init_variables.insert(af);//第一次遇见加入
		}
		return;
	}
	// 后续根据是clause或者item或者one-of，forall均进行递归查找
	const Negation* nf = dynamic_cast<const Negation*>(&formula);
	if (nf != NULL) {
		/*
		 * The state variables of a negation are the state variables of the
		 * negand.
		 */
		collect_init_state_variables(nf->negand());
		return;
	}
	const Conjunction* cf = dynamic_cast<const Conjunction*>(&formula);
	if (cf != NULL) {
		/*
		 * The state variables of a conjunction are the state variables of
		 * the conjuncts.
		 */
		for (size_t i = 0; i < cf->size(); i++) {
			collect_init_state_variables(cf->conjunct(i));
		}
		return;
	}

	const Disjunction* df = dynamic_cast<const Disjunction*>(&formula);
	if (df != NULL) {
		/*
		 * The state variables of a disjunction are the state variables of
		 * the disjuncts.
		 */
		for (size_t i = 0; i < df->size(); i++) {
			collect_init_state_variables(df->disjunct(i));
		}
		return;
	}

	const OneOfDisjunction* odf = dynamic_cast<const OneOfDisjunction*>(&formula);
	if (odf != NULL) {
		/*
		 * The state variables of a disjunction are the state variables of
		 * the disjuncts.
		 */
		for (size_t i = 0; i < odf->size(); i++) {
			collect_init_state_variables(odf->disjunct(i));
		}
		return;
	}

	const Forall* faf = dynamic_cast<const Forall*>(&formula);
	if (faf != NULL) {
		/*
		 * The state variables of a disjunction are the state variables of
		 * the disjuncts.
		 */
		collect_init_state_variables(faf->body());
		return;
	}



	/*
	 * No other types of formulas should appear in fully instantiated
	 * action preconditions and effect conditions.
	 */
	throw std::logic_error("unexpected formula");
}


/*
 * Collects state variables from the given formula.
 * initState是否是初始状态
 * 	仅收集非初始状态,或初始状态但不是top level的atom
 * toplevel是否是最高一层的公式
 */
static void collect_state_variables(const StateFormula& formula, bool initState, bool topLevel = true) {
	if (typeid(formula) == typeid(Constant)) {
		/*
		 * The formula is either TRUE or FALSE, so it contains no state
		 * variables.
		 */
		return;
	}

	const Atom* af = dynamic_cast<const Atom*>(&formula);
	if (af != NULL) {
		/*
		 * The formula is an atom representing a single state variable.
		 */
		if (state_variables.find(af) == state_variables.end()) {
			if((!topLevel && initState) ||  !initState){
				//	std::cout << "new var = " << dynamic_atoms.size() << " " << initState <<std::endl;
				dynamic_atoms.insert(std::make_pair(state_variables.size(), af));
				// std::cout << "AC::" << std::flush;
				// af->print(std::cout, my_problem->domain().predicates(), my_problem->domain().functions(),
				// 		  my_problem->terms());
				// printf("\n");
			}
			// else{
			// 	std::cout << "MISS::" << std::flush;
			// 	af->print(std::cout, my_problem->domain().predicates(), my_problem->domain().functions(),
			// 			  my_problem->terms());
			// 	printf("\n");
			// }
			state_variables.insert(std::make_pair(af, state_variables.size()));

		}
		// else{
		// 	af->print(std::cout, my_problem->domain().predicates(), my_problem->domain().functions(),
		// 				  my_problem->terms());
		// 		printf("has beed added\n");
		// }
		return;
	}

	const Negation* nf = dynamic_cast<const Negation*>(&formula);
	if (nf != NULL) {
		/*
		 * The state variables of a negation are the state variables of the
		 * negand.
		 */
		if(topLevel)
			collect_state_variables(nf->negand(), initState, true);
		else
			collect_state_variables(nf->negand(), initState, false);
		return;
	}

	const Conjunction* cf = dynamic_cast<const Conjunction*>(&formula);
	if (cf != NULL) {
		/*
		 * The state variables of a conjunction are the state variables of
		 * the conjuncts.
		 */
		for (size_t i = 0; i < cf->size(); i++) {
			if(topLevel && initState)
				collect_state_variables(cf->conjunct(i), initState, true);
			else
				collect_state_variables(cf->conjunct(i), initState, false);
		}
		return;
	}

	const Disjunction* df = dynamic_cast<const Disjunction*>(&formula);
	if (df != NULL) {
		/*
		 * The state variables of a disjunction are the state variables of
		 * the disjuncts.
		 */
		for (size_t i = 0; i < df->size(); i++) {
			collect_state_variables(df->disjunct(i), initState, false);
		}
		return;
	}

	const OneOfDisjunction* odf = dynamic_cast<const OneOfDisjunction*>(&formula);
	if (odf != NULL) {
		/*
		 * The state variables of a disjunction are the state variables of
		 * the disjuncts.
		 */
		for (size_t i = 0; i < odf->size(); i++) {
			collect_state_variables(odf->disjunct(i), initState, false);
		}
		return;
	}
	const Forall* faf = dynamic_cast<const Forall*>(&formula);
	if (faf != NULL) {
		/*
		 * The state variables of a disjunction are the state variables of
		 * the disjuncts.
		 */
		collect_state_variables(faf->body(), initState, false);
		return;
	}
	/*
	 * No other types of formulas should appear in fully instantiated
	 * action preconditions and effect conditions.
	 */
	throw std::logic_error("unexpected formula");
}


/*
 * Collects state variables from the given effect.
 */
static void collect_state_variables(const pEffect& effect) {
	const AssignmentEffect* fe = dynamic_cast<const AssignmentEffect*>(&effect);
	if (fe != NULL) {
		/*
		 * Only reward assignments are supported, and they do not involve
		 * any state variables.
		 */
		// std::cout << "AssigE:" << std::endl;
		if (fe->assignment().application().function() != reward_function)
		{
			throw std::logic_error("numeric state variables not supported");
		}
		// std::cout << "done AssigE:" << std::endl;
		return;
	}
	// effect最终都是simple Effect,涉及到的都是dynamic并且状态变量
	const SimpleEffect* se = dynamic_cast<const SimpleEffect*>(&effect);
	if (se != NULL) {
		/*
		 * A simple effect involves a single state variable.
		 */
		const Atom *atom = &se->atom();

		if (state_variables.find(atom) == state_variables.end()) {
			//std::cout << "AE " << dynamic_atoms.size() <<std::endl;
			dynamic_atoms.insert(std::make_pair(state_variables.size(), atom));
			state_variables.insert(std::make_pair(atom, state_variables.size()));
			// std::cout << "AE::" << std::flush;
			// atom->print(std::cout, my_problem->domain().predicates(), my_problem->domain().functions(),
			// 			  my_problem->terms());
			// 	printf("\n");
		}
		// else
		// {
		// 	atom->print(std::cout, my_problem->domain().predicates(), my_problem->domain().functions(),
		// 				  my_problem->terms());
		// 		printf("has beed added\n");
		// }
		return;
	}

	const ConjunctiveEffect* ce =
			dynamic_cast<const ConjunctiveEffect*>(&effect);
	if (ce != NULL) {
		/*
		 * The state variables of a conjunctive effect are the state
		 * variables of the conjuncts.
		 */
		// std::cout << "CE" <<std::endl;
		for (size_t i = 0; i < ce->size(); i++) {
			collect_state_variables(ce->conjunct(i));
		}
		// std::cout << "done CE" <<std::endl;
		return;
	}

	const ConditionalEffect* we =
			dynamic_cast<const ConditionalEffect*>(&effect);
	if (we != NULL) {
		/*
		 * The state variables of a conditional effect are the state
		 * variables of the condition and the effect.
		 */
		// std::cout << "CndE" <<std::endl;
		collect_state_variables(we->condition(), false);
		collect_state_variables(we->effect());
		// std::cout << "done CondE" << std::endl;
		return;
	}

	const ProbabilisticEffect* pe =
			dynamic_cast<const ProbabilisticEffect*>(&effect);
	if (pe != NULL) {
		/*
		 * The state variables of a probabilistic effect are the state
		 * variables of the possible effects.
		 */
		// std::cout << "PrE" <<std::endl;
		for (size_t i = 0; i < pe->size(); i++) {
			collect_state_variables(pe->effect(i));
		}
		// std::cout << "done PrE" << std::endl;
		return;
	}

	/*
	 * No other types of effects exist.
	 */
	throw std::logic_error("unexpected effect");
}

/*
 * Checks if an effect is consistent
 */
static bool check_consistent(const pEffect& effect, 
		std::set<Atom*>* pos,
		std::set<Atom*>* neg) {
	const AssignmentEffect* fe = dynamic_cast<const AssignmentEffect*>(&effect);
	if (fe != NULL) {
		/*
		 * Only reward assignments are supported, and they do not involve
		 * any state variables.
		 */
		if (fe->assignment().application().function() != reward_function) {
			throw std::logic_error("numeric state variables not supported");
		}
		return false;
	}

	const AddEffect* adde = dynamic_cast<const AddEffect*>(&effect);
	if (adde != NULL) {
		Atom* atom = (Atom*) &adde->atom();
		pos->insert(atom);
		return true;
	}

	const DeleteEffect* deletee = dynamic_cast<const DeleteEffect*>(&effect);
	if (deletee != NULL) {
		Atom* atom = (Atom*) &deletee->atom();
		neg->insert(atom);
		return true;
	}

	const ConjunctiveEffect* ce =
			dynamic_cast<const ConjunctiveEffect*>(&effect);
	if (ce != NULL) {
		std::set<Atom*> mpos, mneg;
		mpos.insert(pos->begin(), pos->end());
		mneg.insert(neg->begin(), neg->end());
		for (size_t i = 0; i < ce->size(); i++) {
			if(!check_consistent(ce->conjunct(i), &mpos, &mneg))
				return false;

			std::set<Atom*> inter;
			std::insert_iterator<std::set<Atom*> > iter(inter, inter.begin());
			set_intersection(mpos.begin(), mpos.end(),
					mneg.begin(), mneg.end(),
					iter);
			if(inter.size() > 0)
				return false;

		}
		pos->insert(mpos.begin(), mpos.end());
		neg->insert(mneg.begin(), mneg.end());

		return true;
	}

	const ConditionalEffect* we =
			dynamic_cast<const ConditionalEffect*>(&effect);
	if (we != NULL) {
		return check_consistent(we->effect(), pos, neg);
	}

	const ProbabilisticEffect* pe =
			dynamic_cast<const ProbabilisticEffect*>(&effect);
	if (pe != NULL) {
		/*
		 * The state variables of a probabilistic effect are the state
		 * variables of the possible effects.
		 */
		for (size_t i = 0; i < pe->size(); i++) {
			check_consistent(pe->effect(i), pos, neg);
		}
		return true;
	}

	/*
	 * No other types of effects exist.
	 */
	throw std::logic_error("unexpected effect");
}

extern void printBDD(DdNode*);
/* ====================================================================== */
/* formula_bdd */

/*
 * Constructs a BDD representing the given formula.
 */
DdNode* formula_bdd(const StateFormula& formula, bool primed = false) {
	if (typeid(formula) == typeid(Constant)) {//判断是否常量
		/*
		 * The formula is either TRUE or FALSE, so the BDD is either
		 * constant 1 or 0.
		 */
		DdNode* ddf = (formula.tautology() ?
				Cudd_ReadOne(dd_man) : Cudd_ReadLogicZero(dd_man));//here we use the logic zero
		Cudd_Ref(ddf);
		return ddf;
	}

	const Atom* af = dynamic_cast<const Atom*>(&formula);
	if (af != NULL) {
		/*
		 * The BDD for an atom is the `current-state' (or `next-state' if
		 * primed is true) DD variable for the state variable represented
		 * by the atom.
		 * primed代表状态变量加了'，即后即状态变量。
		 */
		// std::cout << "atom = " << state_variables[af] << std::endl;
		if(dynamic_atoms.find((state_variables[af])) == dynamic_atoms.end()){
			DdNode *ddf = Cudd_ReadOne(manager);
			Cudd_Ref(ddf);
			return ddf;// 该变量不是dynamic,忽略,直接返回true.
		}

		// 假设状态变量有[a,b,c,d]后即状态[a',b',c',d']排放位置为：[a,a',b,b',c,c',d,d']
		// 当前atom是状态变量，获取他的位置，然后状态BDD，并返回该Node
		DdNode* ddf = Cudd_bddIthVar(dd_man,
				2*state_variables[af] + (primed ? 1 : 0));
		Cudd_Ref(ddf);
		return ddf;
	}
	// 接下来分别考滤否定，合取，析取，one-of,forall的情况，递归处理一个formula

	const Negation* nf = dynamic_cast<const Negation*>(&formula);
	if (nf != NULL) {
		/*
		 * The BDD for a negation is the negation of the BDD for the
		 * negand.
		 *	递归对否定的内层处理，随后在对内层取否
		 */
		// std::cout << "negation(#) ";
		DdNode* ddn = formula_bdd(nf->negand(), primed);
		DdNode* ddf = Cudd_Not(ddn);
		Cudd_Ref(ddf);
		Cudd_RecursiveDeref(dd_man, ddn);
		return ddf;
	}
	
	const Conjunction* cf = dynamic_cast<const Conjunction*>(&formula);
	if (cf != NULL) {
		/*
		 * The BDD for a conjunction is the conjunction of the BDDs for
		 * the conjuncts.
		 * 这里readone为了和下面的conjunct合取
		 */
		// std::cout << "conjunction\n";
		DdNode *ddf = Cudd_ReadOne(dd_man);
		Cudd_Ref(ddf);

		for (size_t i = 0; i < cf->size(); i++) {
			DdNode* ddi = formula_bdd(cf->conjunct(i), primed);
			// momo007 012 2022.04.27
			// 这里确实应该不用加
			Cudd_Ref(ddi);//DON'T REALLY NEED THIS, except in uts k30.pddl
			DdNode* dda = Cudd_bddAnd(dd_man, ddf, ddi);
			Cudd_Ref(dda);
			// cf->conjunct(i).print(std::cout, my_problem->domain().predicates(),
			// 					  my_problem->domain().functions(), my_problem->terms());
			// std::cout << "\n";
			if (dda == Cudd_ReadLogicZero(manager))
			{
				std::cout << "formula_bdd conjunct get FALSE\n";
				assert(0);
			}
			Cudd_RecursiveDeref(dd_man, ddf);
			Cudd_RecursiveDeref(dd_man, ddi);
			ddf = dda;
		}
		// std::cout << "done conjunction\n";
		return ddf;
	}
	
	const Disjunction* df = dynamic_cast<const Disjunction*>(&formula);
	if (df != NULL) {
		/*
		 * The BDD for a disjunction is the disjunction of the BDDs for
		 * the disjuncts.
		 * 类似前面的，这里clause所以是readzero随后析取
		 */
		// std::cout << "disjunction\n";
		DdNode* ddf = Cudd_ReadLogicZero(dd_man);
		Cudd_Ref(ddf);
		for (size_t i = 0; i < df->size(); i++) {
			DdNode* ddi = formula_bdd(df->disjunct(i), primed);
			DdNode* ddo = Cudd_bddOr(dd_man, ddf, ddi);
			Cudd_Ref(ddo);
			Cudd_RecursiveDeref(dd_man, ddf);
			Cudd_RecursiveDeref(dd_man, ddi);
			ddf = ddo;
		}
		// std::cout << "done disjunction\n";
		return ddf;
	}

	const OneOfDisjunction* odf = dynamic_cast<const OneOfDisjunction*>(&formula);
	if (odf != NULL) {
		/*
		 * The BDD for a one of disjunction
		 */
		DdNode* ddx, *ddn;
		// std::cout << "oneOfDisjunction\n";
		if(odf->size() == 2){
			ddx = formula_bdd(odf->disjunct(0), primed);
			Cudd_Ref(ddx);
			ddn = formula_bdd(odf->disjunct(1), primed);
			Cudd_Ref(ddn);
			if(ddx == Cudd_Not(ddn)){// 两者互斥，则等于1
				Cudd_RecursiveDeref(manager, ddx);
				Cudd_RecursiveDeref(manager, ddn);
				ddx = Cudd_ReadOne(manager);
				Cudd_Ref(ddx);
				return ddx;
			}
			else{
				Cudd_RecursiveDeref(manager, ddx);
				Cudd_RecursiveDeref(manager, ddn);
			}
		}

		ddx = Cudd_ReadLogicZero(dd_man);// the conjuncts that only one literal is true
		Cudd_Ref(ddx);
		ddn = Cudd_ReadOne(dd_man);// the conjuncts that all literal is false
		Cudd_Ref(ddn);
		// 考虑公式的每一个disjunct,利用ITE实现互斥的关系，最后ddx存储oneof情况下的全部disjunct
		for (size_t i = 0; i < odf->size(); i++) {
			DdNode* ddi = formula_bdd(odf->disjunct(i), primed);// 考虑每个disjunct
			// Cudd_bddIte实现If-then-else
			DdNode* ddxp = Cudd_bddIte(dd_man, ddi, ddn, ddx);// (ddi and ddn) or (!ddi and ddx)
			Cudd_Ref(ddxp);

			DdNode *ddnp;
			if(i < odf->size()-1){
				ddnp = Cudd_bddIte(dd_man, ddi, Cudd_ReadLogicZero(manager), ddn);
				Cudd_Ref(ddnp);
			}
			Cudd_RecursiveDeref(dd_man, ddi);
			Cudd_RecursiveDeref(dd_man, ddx);
			Cudd_RecursiveDeref(dd_man, ddn);
			
			ddx = ddxp;
			Cudd_Ref(ddx);
			Cudd_RecursiveDeref(dd_man, ddxp);

			if(i < odf->size()-1){
				ddn = ddnp;
				Cudd_Ref(ddn);
				Cudd_RecursiveDeref(dd_man, ddnp);
			}


		}
		// std::cout << "done oneOfDisjunction\n";
		return ddx;
	}
	const Forall* faf = dynamic_cast<const Forall*>(&formula);
	// 对于forall，将其实例化，去除量词
	if (faf != NULL) {
		/*
		 * The state variables of a disjunction are the state variables of
		 * the disjuncts.
		 */
		const SubstitutionMap args;
		return formula_bdd(faf->instantiation(args, *my_problem));
	}

	/*
	 * No other types of formulae should appear in fully instantiated
	 * action preconditions and effect conditions.
	 */
	throw std::logic_error("unexpected formula");
}


void bdd_goal_cnf(std::list<DdNode*>* goal_cnf){
	DdNode *tmp;
	const Conjunction* cf = dynamic_cast<const Conjunction*>(&(my_problem->goal()));
	if (cf != NULL) {
		for (size_t i = 0; i < cf->size(); i++) {
			std::cout << i << " ";
			const Atom* af1 = dynamic_cast<const Atom*>(&(cf->conjunct(i)));
			if(af1 != NULL){
				tmp = formula_bdd(*af1);
				Cudd_Ref(tmp);
				goal_cnf->push_back(tmp);
				continue;
			}
			const Negation* nf = dynamic_cast<const Negation*>(&(cf->conjunct(i)));
			if (nf != NULL) {
				tmp = formula_bdd(*nf);
				Cudd_Ref(tmp);
				goal_cnf->push_back(tmp);
				continue;
			}
			const Disjunction* df = dynamic_cast<const Disjunction*>(&(cf->conjunct(i)));
			if (df != NULL) {
				tmp = formula_bdd(*df);
				Cudd_Ref(tmp);
				goal_cnf->push_back(tmp);
				continue;
			}
			const OneOfDisjunction* odf = dynamic_cast<const OneOfDisjunction*>(&(cf->conjunct(i)));
			if (odf != NULL) {
				tmp = formula_bdd(*odf);
				Cudd_Ref(tmp);
				goal_cnf->push_back(tmp);
				continue;
			}
		}
	}
	// 没有则查找，添加到goal_cnt中，外层调用该函数进行添加
	else{
		tmp = formula_bdd((my_problem->goal()));
		Cudd_Ref(tmp);
		goal_cnf->push_back(tmp);
	}
}


/* ====================================================================== */
/* TransitionSet */

struct TransitionSetList;



/*
 * A list of transition sets.
 */


/* Fills the provided list with the conjunction of the given
   transition sets. */
void TransitionSet::conjunction(TransitionSetList& transitions,
		const TransitionSet& t1,
		const TransitionSet& t2) {
	/*
	 * Case 1: transitions in both sets.
	 */
	DdNode* dda = Cudd_bddAnd(dd_man, t1.condition_bdd(), t2.condition_bdd());
	Cudd_Ref(dda);
	if (dda != Cudd_ReadLogicZero(dd_man)) {
		DdNode* dde = Cudd_bddAnd(dd_man, t1.effect_bdd(), t2.effect_bdd());
		Cudd_Ref(dde);
		TransitionSet* t = new TransitionSet(dda, dde, t1.reward() + t2.reward(),
				t1.touched_variables());
		t->touched_variables().insert(t2.touched_variables().begin(),
				t2.touched_variables().end());
		transitions.push_back(t);
	} else {
		Cudd_RecursiveDeref(dd_man, dda);
	}
	/*
	 * Case 2: transitions only in the first set.  Ignore this case if
	 * the first transition set is a null transition set.
	 */
	if (!t1.is_null()) {
		DdNode* dd2 = Cudd_Not(t2.condition_bdd());
		Cudd_Ref(dd2);
		dda = Cudd_bddAnd(dd_man, t1.condition_bdd(), dd2);
		Cudd_Ref(dda);
		Cudd_RecursiveDeref(dd_man, dd2);
		if (dda != Cudd_ReadLogicZero(dd_man)) {
			DdNode* dde = t1.effect_bdd();
			Cudd_Ref(dde);
			TransitionSet* t = new TransitionSet(dda, dde, t1.reward(),
					t1.touched_variables());
			transitions.push_back(t);
		} else {
			Cudd_RecursiveDeref(dd_man, dda);
		}
	}
	/*
	 * Case 3: transition only in the second set.  Ignore this case if
	 * the second transition set is a null transition set.
	 */
	if (!t2.is_null()) {
		DdNode* dd1 = Cudd_Not(t1.condition_bdd());
		Cudd_Ref(dd1);
		dda = Cudd_bddAnd(dd_man, dd1, t2.condition_bdd());
		Cudd_Ref(dda);
		Cudd_RecursiveDeref(dd_man, dd1);
		if (dda != Cudd_ReadLogicZero(dd_man)) {
			DdNode* dde = t2.effect_bdd();
			Cudd_Ref(dde);
			TransitionSet* t = new TransitionSet(dda, dde, t2.reward(),
					t2.touched_variables());
			transitions.push_back(t);
		} else {
			Cudd_RecursiveDeref(dd_man, dda);
		}
	}
}


/* Constructs a null transition set. */
TransitionSet::TransitionSet()
: condition_bdd_(Cudd_ReadOne(dd_man)), reward_(0.0) {
	Cudd_Ref(condition_bdd_);
	effect_bdd_ = Cudd_ReadOne(dd_man);
	Cudd_Ref(effect_bdd_);
	set_index(gnum_cond_effects++);
}


/* Constructs a copy of the give transition set. */
TransitionSet::TransitionSet(const TransitionSet& t)
: condition_bdd_(t.condition_bdd_), effect_bdd_(t.effect_bdd_),
  reward_(t.reward_), touched_variables_(t.touched_variables_) {
	Cudd_Ref(condition_bdd_);
	Cudd_Ref(effect_bdd_);
	set_index(t.index());

}


/* Constructs a transition set for a simple effect. */
TransitionSet::TransitionSet(DdNode* condition_bdd,
		const Atom& atom, bool is_true)
: condition_bdd_(condition_bdd), reward_(0.0) {
	Cudd_Ref(condition_bdd_);
	int i = state_variables[&atom];
	// 获取该状态变量的BDD
	DdNode* ddv = Cudd_bddIthVar(dd_man, 2*i + 1);
	Cudd_Ref(ddv);
	if (is_true) {// 判断命题是true还是false
		effect_bdd_ = ddv;
	} else {
		effect_bdd_ = Cudd_Not(ddv);
		Cudd_Ref(effect_bdd_);
		Cudd_RecursiveDeref(dd_man, ddv);
	}
	// 设计的effect增加
	touched_variables_.insert(i);
	set_index(gnum_cond_effects++);
}


/* Constructs a transition set for a reward effect. */
TransitionSet::TransitionSet(DdNode* condition_bdd, const Rational& reward)
: condition_bdd_(condition_bdd), reward_(reward) {
	Cudd_Ref(condition_bdd_);
	effect_bdd_ = Cudd_ReadOne(dd_man);
	Cudd_Ref(effect_bdd_);
	set_index(gnum_cond_effects++);
}


/* Deletes this transition set. */
TransitionSet::~TransitionSet() {
	Cudd_RecursiveDeref(dd_man, condition_bdd_);
	Cudd_RecursiveDeref(dd_man, effect_bdd_);
}


/* ====================================================================== */
/* OutcomeSet */

/*
 * Collection of probability weighted transition sets.
 */



/*
 * Deletes the given outcome set.
 */
static void free_outcomes(const OutcomeSet& outcomes) {
	for (std::vector<TransitionSetList>::const_iterator ei =
			outcomes.transitions.begin();
			ei != outcomes.transitions.end(); ei++) {
		for (TransitionSetList::const_iterator ti = (*ei).begin();
				ti != (*ei).end(); ti++) {
			delete *ti;
		}
	}
}


/*
 * Prints the given outcome set on the given stream.
 */
static void print_outcomes(std::ostream& os, const OutcomeSet& outcomes) {
	size_t n = outcomes.probabilities.size();
	for (size_t i = 0; i < n; i++) {
		os << "outcome " << i << ": " << outcomes.probabilities[i] << std::endl;
		for (TransitionSetList::const_iterator ti =
				outcomes.transitions[i].begin();
				ti != outcomes.transitions[i].end(); ti++) {
			os << "condition " << ti - outcomes.transitions[i].begin() << ':'
					<< std::endl;
			Cudd_PrintDebug(dd_man, (*ti)->condition_bdd(), 2*nvars, 2);
			os << "effect:" << std::endl;
			Cudd_PrintDebug(dd_man, (*ti)->effect_bdd(), 2*nvars, 2);
			os << "reward: " << (*ti)->reward() << std::endl;
			os << "touched variables:";
			for (std::set<int>::const_iterator vi =
					(*ti)->touched_variables().begin();
					vi != (*ti)->touched_variables().end(); vi++) {
				os << ' ' << *vi;
			}
			os << std::endl;
		}
	}
}


/*
 * Combines the given outcome sets, modifying the first set to make it
 * the combined outcome set.
 */
void combine_outcomes(OutcomeSet& outcomes1, const OutcomeSet& outcomes2) {
	size_t n = outcomes1.probabilities.size();
	size_t m = outcomes2.probabilities.size();
	//   cout << "n = " << n << " m = " << m <<endl;



	if(m == 0)
		return;

	for (size_t j = 1; j < m; j++) {
		const Rational& pj = outcomes2.probabilities[j];
		for (size_t i = 0; i < n; i++) {
			outcomes1.probabilities.push_back(outcomes1.probabilities[i]*pj);
			outcomes1.transitions.push_back(TransitionSetList());
			if (outcomes1.transitions[i].size() == 1
					&& outcomes1.transitions[i][0]->is_null()
					&& outcomes2.transitions[j].size() == 1
					&& outcomes2.transitions[j][0]->is_null()) {
				if (i + 1 == n) {
					outcomes1.transitions.back().push_back(outcomes2.transitions[j][0]);
				} else {
					outcomes1.transitions.back().push_back(new TransitionSet());
				}
			} else {
				for (TransitionSetList::const_iterator ti =
						outcomes1.transitions[i].begin();
						ti != outcomes1.transitions[i].end(); ti++) {
					if (!(*ti)->is_null()) {
						outcomes1.transitions.back().push_back(new TransitionSet(**ti));
					}
				}
				for (TransitionSetList::const_iterator ti =
						outcomes2.transitions[j].begin();
						ti != outcomes2.transitions[j].end(); ti++) {
					if (!(*ti)->is_null()) {
						if (i + 1 == n) {
							outcomes1.transitions.back().push_back(*ti);
						} else {
							outcomes1.transitions.back().push_back(new TransitionSet(**ti));
						}
					} else if (i + 1 == n) {
						delete *ti;
					}
				}
			}
		}
	}
	const Rational& p = outcomes2.probabilities[0];
	for (size_t i = 0; i < n; i++) {
		outcomes1.probabilities[i] = outcomes1.probabilities[i]*p;
		if (outcomes2.transitions[0].size() == 1
				&& outcomes2.transitions[0][0]->is_null()) {
			if (i + 1 == n) {
				delete outcomes2.transitions[0][0];
			}
		} else {
			if (outcomes1.transitions[i].size() == 1
					&& outcomes1.transitions[i][0]->is_null()) {
				delete outcomes1.transitions[i][0];
				outcomes1.transitions[i].clear();
			}
			if (i + 1 == n) {
				copy(outcomes2.transitions[0].begin(),
						outcomes2.transitions[0].end(),
						back_inserter(outcomes1.transitions[i]));
			} else {
				for (TransitionSetList::const_iterator ti =
						outcomes2.transitions[0].begin();
						ti != outcomes2.transitions[0].end(); ti++) {
					if (!(*ti)->is_null()) {
						outcomes1.transitions[i].push_back(new TransitionSet(**ti));
					}
				}
			}
		}
	}
}


/*
 * Collects outcomes for the given effect.
 */
static void effect_outcomes(OutcomeSet& outcomes,
		DdNode* condition_bdd, const pEffect& effect) {
	if (condition_bdd == Cudd_ReadLogicZero(dd_man)) {
		/*
		 * The condition is false, so no new outcomes are added.
		 */
		//    cout << "FALSE"<<endl;
		std::cout << "empty effect\n";
		return;
	}

	const AssignmentEffect* fe = dynamic_cast<const AssignmentEffect*>(&effect);
	if (fe != NULL) {
		/*
		 * Only reward assignments are supported.  Add an outcome with
		 * probability 1 for the reward assigned by this effect.
		 */
		//      cout << "HI AE"<<endl;
		std::cout << "Assignment Effect outcome sta\n";
		const Application& application = fe->assignment().application();
		if (application.function() != reward_function) {
			throw std::logic_error("numeric state variables not supported");
		}
		ValueMap values;
		values[&application] = 0.0;
		fe->assignment().affect(values);
		outcomes.probabilities.push_back(1);
		outcomes.transitions.push_back(TransitionSetList());
		const TransitionSet* t = new TransitionSet(condition_bdd,
				values[&application]);
		outcomes.transitions.back().push_back(t);
		//      cout << "BYE AE"<<endl;
		std::cout << "Assignment Effect outcome done\n";
		return;
	}

	const SimpleEffect* se = dynamic_cast<const SimpleEffect*>(&effect);
	if (se != NULL) {
		/*
		 * Add an outcome with probability 1 and a single transition for
		 * the simple effect.
		 */
		//    cout << "HI SE"<<endl;
		// std::cout << "simple Effect outcome sta\n";
		bool is_true = typeid(*se) == typeid(AddEffect);
		outcomes.probabilities.push_back(1);
		outcomes.transitions.push_back(TransitionSetList());
		const TransitionSet* t = new TransitionSet(condition_bdd,
				se->atom(), is_true);
		outcomes.transitions.back().push_back(t);
		//     cout << "BYE SE"<<endl;
		// std::cout << "simple Effect outcome done\n";
		return;
	}

	const ConjunctiveEffect* ce =
			dynamic_cast<const ConjunctiveEffect*>(&effect);
	if (ce != NULL) {
		/*
		 * Collect the outcomes for the conjuncts.  All combinations of
		 * these outcomes are the outcomes for the conjunctive effect.
		 */
		//    cout << "HI CJE"<<endl;
		// std::cout << "conjunction Effect outcome sta\n";
		size_t n = ce->size();
		if (n > 0) {
			effect_outcomes(outcomes, condition_bdd, ce->conjunct(0));
			for (size_t i = 1; i < n; i++) {
				OutcomeSet c_outcomes;
				effect_outcomes(c_outcomes, condition_bdd, ce->conjunct(i));
				combine_outcomes(outcomes, c_outcomes);
				if(LUGTOTEXT){//since not calling ensure mutex, need to combine
					size_t n1 = outcomes.probabilities.size();
					for (size_t i = 0; i < n1; i++) {
						size_t m = outcomes.transitions[i].size();
						for (size_t j = 0; j < m - 1; j++) {
							for (size_t k = j + 1; k < m; k++) {
								const TransitionSet* t1 = outcomes.transitions[i][j];
								const TransitionSet* t2 = outcomes.transitions[i][k];
								if(t1->condition_bdd() == t2->condition_bdd()){
									DdNode* dde = Cudd_bddAnd(dd_man, t1->effect_bdd(),
											t2->effect_bdd());
									Cudd_Ref(dde);

									if(dde == Cudd_ReadLogicZero(manager))
										throw std::logic_error("inconsistent effect");

									DdNode *dda = t1->condition_bdd();
									Cudd_Ref(dda);
									TransitionSet* t = new TransitionSet(dda, dde,
											t1->reward() + t2->reward(),
											t1->touched_variables());
									t->touched_variables().insert(t2->touched_variables().begin(),
											t2->touched_variables().end());
									/* Replace transition set j with the new transition set. */
									outcomes.transitions[i][j] = t;
									/*
									 * Remove transition set k.
									 */
									outcomes.transitions[i][k] = outcomes.transitions[i].back();
									k--;
									outcomes.transitions[i].pop_back();
									m = outcomes.transitions[i].size();
									delete t1;
									delete t2;
								}
							}
						}


					}
				}
			}
		} else {
			outcomes.probabilities.push_back(1);
			outcomes.transitions.push_back(TransitionSetList());
			const TransitionSet* t = new TransitionSet(condition_bdd, 0.0);
			outcomes.transitions.back().push_back(t);
		}
		//      cout << "BYE CJE"<<endl;
		// std::cout << "conjunction Effect outcome done\n";
		return;
	}

	const ConditionalEffect* we =
			dynamic_cast<const ConditionalEffect*>(&effect);
	if (we != NULL) {
		/*
		 * Take the conjunction of the condition f this effect and the
		 * condition inherited from nesting.  Make this the condition for
		 * the outcomes of the effect.
		 */
		//     cout << "HI CE"<<endl;
		// std::cout << "conditional Effect outcome sta\n";
		DdNode *ddf = formula_bdd(we->condition());
		DdNode* ddc = Cudd_bddAnd(dd_man, condition_bdd, ddf);
		Cudd_Ref(ddc);
		Cudd_RecursiveDeref(dd_man, ddf);
		effect_outcomes(outcomes, ddc, we->effect());
		Cudd_RecursiveDeref(dd_man, ddc);
		//     cout << "BYE CE"<<endl;
		// std::cout << "conditional Effect outcome done\n";
		return;
	}

	const ProbabilisticEffect* pe =
			dynamic_cast<const ProbabilisticEffect*>(&effect);
	if (pe != NULL) {
		/*
		 * Add the outcomes of this probabilistic effect.
		 */
		//    cout << "HI PE"<<endl;
		std::cout << "probability effect outcome[warning!!!]\n";
		Rational p_rest = 1;
		size_t n = pe->size();
		std::cout << "n = " << n << " pr0 = " << (pe->probability(0).double_value()) << std::endl;
		if(n == 1 && (pe->probability(0).double_value()) == -2.0) {//unknown effect, do not enter in
			Rational p(0.5);
			OutcomeSet p_outcomes;
			effect_outcomes(p_outcomes, condition_bdd, pe->effect(0));
			for (std::vector<Rational>::const_iterator pi =
					p_outcomes.probabilities.begin();
					pi != p_outcomes.probabilities.end(); pi++) {
				outcomes.probabilities.push_back(p*(*pi));
			}
			copy(p_outcomes.transitions.begin(), p_outcomes.transitions.end(),
					back_inserter(outcomes.transitions));

			Rational p1(0.5);
			OutcomeSet p1_outcomes;
			DeleteEffect p1e (((SimpleEffect&)pe->effect(0)).atom());
			effect_outcomes(p1_outcomes, condition_bdd, p1e);
			for (std::vector<Rational>::const_iterator pi =
					p1_outcomes.probabilities.begin();
					pi != p1_outcomes.probabilities.end(); pi++) {
				outcomes.probabilities.push_back(p1*(*pi));
			}
			copy(p1_outcomes.transitions.begin(), p1_outcomes.transitions.end(),
					back_inserter(outcomes.transitions));


		}
		else if(n > 1 && fabs(pe->probability(0).double_value()) == 1.0){ //oneof effect
			for (size_t i = 0; i < n; i++) {

				Rational p (1.0/(double)n);
				OutcomeSet p_outcomes;
				effect_outcomes(p_outcomes, condition_bdd, pe->effect(i));
				for (std::vector<Rational>::const_iterator pi =
						p_outcomes.probabilities.begin();
						pi != p_outcomes.probabilities.end(); pi++) {
					outcomes.probabilities.push_back(p*(*pi));
				}
				copy(p_outcomes.transitions.begin(), p_outcomes.transitions.end(),
						back_inserter(outcomes.transitions));
			}
		}
		else{ //probabilistic effect
			for (size_t i = 0; i < n; i++) {
				Rational p = pe->probability(i);
				//	cout << "i = " << i << " " << p  << endl;
				p_rest = p_rest - p;
				OutcomeSet p_outcomes;
				effect_outcomes(p_outcomes, condition_bdd, pe->effect(i));
				for (std::vector<Rational>::const_iterator pi =
						p_outcomes.probabilities.begin();
						pi != p_outcomes.probabilities.end(); pi++) {
					outcomes.probabilities.push_back(p*(*pi));
				}
				copy(p_outcomes.transitions.begin(), p_outcomes.transitions.end(),
						back_inserter(outcomes.transitions));
			}
			if (p_rest > 0.0 && pe->probability(0) > 0.0) {//only for prob effects
				/*
				 * Add an outcome with a null transition set, weighted by the
				 * remaining probability.
				 */
				outcomes.probabilities.push_back(p_rest);
				outcomes.transitions.push_back(TransitionSetList());
				outcomes.transitions.back().push_back(new TransitionSet());
			}
		}
		//    cout << "BYE PE"<<endl;
		return;
	}

	/*
	 * No other types of effects exist.
	 */
	throw std::logic_error("unexpected effect");
}


/*
 * Collects dbn for the given effect.
 */
static dbn* effect_dbn(const pEffect& effect, std::map<const pEffect*, dbn_node*> * pr_nodes) {



	const AssignmentEffect* fe = dynamic_cast<const AssignmentEffect*>(&effect);
	if (fe != NULL) {
		/*
		 * Only reward assignments are supported.  Add an outcome with
		 * probability 1 for the reward assigned by this effect.
		 */
		//           std::cout << "HI AE"<<std::endl;
		const Application& application = fe->assignment().application();
		if (application.function() != reward_function) {
			throw std::logic_error("numeric state variables not supported");
		}
		ValueMap values;
		values[&application] = 0.0;
		fe->assignment().affect(values);
		//    outcomes.probabilities.push_back(1);
		//    outcomes.transitions.push_back(TransitionSetList());
		//    const TransitionSet* t = new TransitionSet(condition_bdd,
		//					       values[&application]);
		//    outcomes.transitions.back().push_back(t);
		//std::cout << "BYE AE"<<std::endl;
		return dbn::reward(new dbn(num_alt_facts), &values,(*pr_nodes)[&effect]);//new dbn(num_alt_facts);
	}

	const SimpleEffect* se = dynamic_cast<const SimpleEffect*>(&effect);
	if (se != NULL) {
		/*
		 * Add an outcome with probability 1 and a single transition for
		 * the simple effect.
		 */
		//     std::cout << "HI SE"<<std::endl;
		bool is_true = typeid(*se) == typeid(AddEffect);
		//    outcomes.probabilities.push_back(1);
		//    outcomes.transitions.push_back(TransitionSetList());
		//    const TransitionSet* t = new TransitionSet(condition_bdd,
		//					       se->atom(), is_true);
		//    outcomes.transitions.back().push_back(t);


		dbn *mdbn = dbn::simple_effect(new dbn(num_alt_facts),
				state_variables[&(se->atom())],
				(is_true ? 1 : 0));
		//     std::cout << "se: " << state_variables[&(se->atom())] << " "
		// 	      << (is_true ? 1 : 0) << std::endl;

		//std::cout << *mdbn << std::endl;
		//std::cout << "BYE SE"<<std::endl;

		return mdbn;
	}

	const ConjunctiveEffect* ce =
			dynamic_cast<const ConjunctiveEffect*>(&effect);
	if (ce != NULL) {
		/*
		 * Collect the outcomes for the conjuncts.  All combinations of
		 * these outcomes are the outcomes for the conjunctive effect.
		 */
		//     std::cout << "HI CJE" << std::endl;
		size_t n = ce->size();
		std::list<dbn*> conjuncts;
		//    std::cout << n << " conjuncts"<<std::endl;
		if (n > 0) {
			for (size_t i = 0; i < n; i++) {
				dbn *d = effect_dbn(ce->conjunct(i), pr_nodes);
						
				if(d != NULL){
					conjuncts.push_back(d);
				}
			}
		}


		dbn* mdbn = dbn::conjoin_dbns(&conjuncts, NULL);
		for(std::list<dbn*>::iterator i = conjuncts.begin();
				i != conjuncts.end(); i++)
			delete *i;
		//std::cout << *mdbn << std::endl;
		//  std::cout << "BYE CJE"<<std::endl;
		//      std::cout << *mdbn << std::endl;
		return mdbn;
	}

	const ConditionalEffect* we =
			dynamic_cast<const ConditionalEffect*>(&effect);
	if (we != NULL) {
		/*
		 * Take the conjunction of the condition for this effect and the
		 * condition inherited from nesting.  Make this the condition for
		 * the outcomes of the effect.
		 */
		//        std::cout << "HI CE"<<std::endl;
		DdNode* ddf = formula_bdd(we->condition());
		Cudd_Ref(ddf);

		//printBDD(ddf);

		dbn *edbn = effect_dbn(we->effect(), pr_nodes);
		//std::cout << "[" << std::flush; Cudd_CheckKeys(manager);
		dbn *mdbn = dbn::condition_dbn(ddf, edbn);
		//std::cout << "|" << std::flush; Cudd_CheckKeys(manager); std::cout << "]" << std::endl;
		//Cudd_RecursiveDeref(dd_man, ddf);
		delete edbn;


		//std::cout << *mdbn << std::endl;
		//std::cout << "BYE CE"<<std::endl;
		//std::cout << *mdbn << std::endl;
		return mdbn;
	}

	const ProbabilisticEffect* pe =
			dynamic_cast<const ProbabilisticEffect*>(&effect);
	if (pe != NULL) {
		/*
		 * Add the outcomes of this probabilistic effect.
		 */
		//    std::cout << "HI PE"<<std::endl;
		Rational p_rest = 1;
		size_t n = pe->size();
		std::list<dbn*> conjuncts;
		std::list<double> prs;
		for (size_t i = 0; i < n; i++) {
			Rational p = pe->probability(i);
			if(p.double_value() == 0.0)
				continue;
			// std::cout << "i = " << i << " " << p  << std::endl;
			p_rest = p_rest - p;
			conjuncts.push_back(effect_dbn(pe->effect(i), pr_nodes));
			prs.push_back(p.double_value());
			if(p.double_value() == 1.0)
				break;
		}
		if(p_rest > 0.0){
			dbn* mydbn = new dbn(num_alt_facts);
			conjuncts.push_back(mydbn);
			prs.push_back(p_rest.double_value());
		}



		dbn *mdbn = (prs.size() > 1 ?
				dbn::probabilistic_dbns(&prs, &conjuncts, (*pr_nodes)[&effect]) :
				conjuncts.front());


		for(std::list<dbn*>::iterator i = conjuncts.begin();
				(prs.size() > 1 &&  i != conjuncts.end()); i++)
			delete *i;

		//   std::cout << *mdbn << std::endl;
		//  std::cout << "BYE PE"<< std::endl;
		return mdbn;
	}

	/*
	 * No other types of effects exist.
	 */
	throw std::logic_error("unexpected effect");
}

/* ====================================================================== */
/* ensure_mutex */

/*
 * Ensures that all transition set conditions are mutually exclusive
 * for each probabilistic outcome.
 * 考虑每个outcome,对每个outcome的transitionSet进行处理，保证同一个outcome内的effect肯定是两两互斥的。
 * 通过不互斥进行合并，最后仅包含互斥的transition
 */
static void ensure_mutex(OutcomeSet& outcomes) {
	size_t n = outcomes.probabilities.size();// 获取outcome个数
	for (size_t i = 0; i < n; i++) {// 考虑每个outcome
		size_t m = outcomes.transitions[i].size();
		for (size_t j = 0; j < m - 1; j++) {
			for (size_t k = j + 1; k < m; k++) {
				const TransitionSet* t1 = outcomes.transitions[i][j];
				const TransitionSet* t2 = outcomes.transitions[i][k];
				DdNode* dda = Cudd_bddAnd(dd_man, t1->condition_bdd(),
						t2->condition_bdd());// 前提条件合取
				Cudd_Ref(dda);
				if (dda != Cudd_ReadLogicZero(dd_man)) {// 没有互斥
					/*
					 * Transition sets j and k do not have mutually exclusive
					 * conditions, so split them into transition sets that have.
					 *
					 * Case 1: both transition set conditions are enabled.
					 *  同时满足，进行合并
					 */
					DdNode* dde = Cudd_bddAnd(dd_man, t1->effect_bdd(),
							t2->effect_bdd());
					Cudd_Ref(dde);
					TransitionSet* t = new TransitionSet(dda, dde,
							t1->reward() + t2->reward(),
							t1->touched_variables());
					t->touched_variables().insert(t2->touched_variables().begin(),
							t2->touched_variables().end());
					/* Replace transition set j with the new transition set.
						这里没有消去k，在后续消除。
					*/
					outcomes.transitions[i][j] = t;

					/*
					 * Case 2: only condition for transition set j is enabled.
					 */
					DdNode* dd2 = Cudd_Not(t2->condition_bdd());
					Cudd_Ref(dd2);
					dda = Cudd_bddAnd(dd_man, t1->condition_bdd(), dd2);
					Cudd_Ref(dda);
					Cudd_RecursiveDeref(dd_man, dd2);
					if (dda != Cudd_ReadLogicZero(dd_man)) {
						dde = t1->effect_bdd();
						Cudd_Ref(dde);
						TransitionSet* t = new TransitionSet(dda, dde, t1->reward(),
								t1->touched_variables());
						/* Add new transition set. */
						outcomes.transitions[i].push_back(t);
					} else {
						Cudd_RecursiveDeref(dd_man, dda);
					}

					/*
					 * Case 3: only condition for transition set k is enabled.
					 */
					DdNode* dd1 = Cudd_Not(t1->condition_bdd());
					Cudd_Ref(dd1);
					dda = Cudd_bddAnd(dd_man, dd1, t2->condition_bdd());
					Cudd_Ref(dda);
					Cudd_RecursiveDeref(dd_man, dd1);
					if (dda != Cudd_ReadLogicZero(dd_man)) {
						dde = t2->effect_bdd();
						Cudd_Ref(dde);
						TransitionSet* t = new TransitionSet(dda, dde, t2->reward(),
								t2->touched_variables());
						/* Add new transition set. */
						outcomes.transitions[i].push_back(t);
					} else {
						Cudd_RecursiveDeref(dd_man, dda);
					}

					/*
					 * Remove transition set k.
					 */
					outcomes.transitions[i][k] = outcomes.transitions[i].back();
					k--;
					outcomes.transitions[i].pop_back();
					m = outcomes.transitions[i].size();
					delete t1;
					delete t2;
				} else {
					Cudd_RecursiveDeref(dd_man, dda);
				}
			}
		}
	}
}


/* ====================================================================== */
/* matrix_to_dbn */

/* Constructs a DBN representation of the given transition probability
   matrix. */
static void matrix_to_dbn(DdNode* ddP) {
	DdNode* ddp = ddP;
	Cudd_Ref(ddp);
	DdNode** cube_vars = new DdNode*[nvars];
	for (int i = 0; i < nvars; i++) {
		cube_vars[i] = Cudd_addIthVar(dd_man, 2*i + 1);
		Cudd_Ref(cube_vars[i]);
	}
	/*
	 * Construct a DBN from the joint probability distribution
	 * (represented by the full transition probability matrix).  We have
	 * chosen to process the variables in a fixed order, but a different
	 * ordering could very well produce a more compact representation.
	 *
	 * The DBN for an action will contain synchronic arcs if the action
	 * has correlated effects.
	 */
	for (int i = nvars - 1; i >= 0; i--) {
		/*
		 * Sum over `next-state' variables that have not yet been
		 * processes.  This ensures an acyclic structure.
		 */
		DdNode* ddc = Cudd_addComputeCube(dd_man, cube_vars, NULL, i);
		Cudd_Ref(ddc);
		DdNode* ddt = Cudd_addExistAbstract(dd_man, ddp, ddc);
		Cudd_Ref(ddt);
		Cudd_RecursiveDeref(dd_man, ddc);
		/*
		 * We are only interested in cases where the current `next-state'
		 * variable is true.  This is because we are trying to compute the
		 * probability that the current `next-state' variable is true
		 * conditioned on (possibly all) `current-state' variables and
		 * unprocesses `next-state' variables.
		 */
		DdNode* dds = Cudd_addApply(dd_man, Cudd_addTimes, cube_vars[i], ddt);
		Cudd_Ref(dds);
		/*
		 * The above step ensures that the current `next-state' variable
		 * is true, so we do not need to encode this in the MTBDD.
		 */
		ddc = Cudd_addComputeCube(dd_man, &cube_vars[i], NULL, 1);
		Cudd_Ref(ddc);
		DdNode* ddm = Cudd_addExistAbstract(dd_man, dds, ddc);
		Cudd_Ref(ddm);
		Cudd_RecursiveDeref(dd_man, dds);
		Cudd_RecursiveDeref(dd_man, ddc);
		/*
		 * At this point, ddm is an MTBDD representing the conditional
		 * probability table for the current `next-state' variable being
		 * true.
		 */
		std::cout << "CPT for variable " << i << std::endl;
		Cudd_PrintDebug(dd_man, ddm, 2*nvars, 2);
		Cudd_RecursiveDeref(dd_man, ddm);
		/*
		 * Factor the joint probability distribution before we continue
		 * with the next variable.
		 */
		dds = Cudd_addApply(dd_man, Cudd_addDivide, ddp, ddt);
		Cudd_Ref(dds);
		Cudd_RecursiveDeref(dd_man, ddp);
		Cudd_RecursiveDeref(dd_man, ddt);
		ddp = dds;
	}
	for (int i = 0; i < nvars; i++) {
		Cudd_RecursiveDeref(dd_man, cube_vars[i]);
	}
	Cudd_RecursiveDeref(dd_man, ddp);
}

extern void printBDD(DdNode*);

void recurse_compose_outcomes(std::map<DdNode*, std::pair<double, double>*>* observables, 
		std::list<DdNode*>* observations,
		DdNode* obs){
	if(observables->empty()){
		if(obs == Cudd_ReadLogicZero(manager))
			return;

		Cudd_Ref(obs);
		observations->push_back(obs);

	}
	else{
		std::map<DdNode*, std::pair<double, double>*>::iterator i = observables->begin();
		std::pair<DdNode*, std::pair<double, double>*> t = *i;
		observables->erase(i);

		DdNode *pos = Cudd_bddAnd(manager, obs, (*i).first);
		Cudd_Ref(pos);
		DdNode *neg = Cudd_bddAnd(manager, obs, Cudd_Not((*i).first));
		Cudd_Ref(neg);

		if(pos != Cudd_ReadLogicZero(manager))
			recurse_compose_outcomes(observables, observations, pos);
		if(neg != Cudd_ReadLogicZero(manager))
			recurse_compose_outcomes(observables, observations, neg);
		Cudd_RecursiveDeref(manager, pos);
		Cudd_RecursiveDeref(manager, neg);

		(*observables)[t.first] = t.second;
	}
}

struct effcomp{ bool operator()(const pEffect* e1, const pEffect* e2) const { return e1->operator<(*e2);}};

void recurse_compose_outcomes( std::set<std::map<const pEffect*, DdNode*, effcomp>*> * observables, 
		std::map<std::set<const pEffect*>*, DdNode*>* observations,
		DdNode* obs,
		std::set<const pEffect*>* atoms){
	if(observables->empty()){
		// if(obs == Cudd_ReadLogicZero(manager))
			//return;

		Cudd_Ref(obs);
		std::set<const pEffect*> *ac = new std::set<const pEffect*>();
		ac->insert(atoms->begin(), atoms->end());
		(*observations)[ac] = obs;

//		std::cout << "GOT OBS:" << std::endl;
//		printBDD(obs);
	}
	else{
		std::set<std::map<const pEffect*, DdNode*, effcomp>*>::iterator i = observables->begin();
		std::map<const pEffect*, DdNode*, effcomp>* t = *i;
		observables->erase(i);
		  //  std::cout << (*i)->size() << std::endl;
		int is = (*i)->size();
		for(std::map<const pEffect*, DdNode*,effcomp>::iterator j = (*i)->begin();
				j != (*i)->end(); j++){
//			std::cout << "hi" << std::endl;

				if(is == 0)
				break;
			is--;

			//if(!(*j).first || !(*j).second)
			//	continue;
			//     printBDD(obs);
			//printBDD((*j).second);
			DdNode *pos = Cudd_addApply(manager, Cudd_addTimes,  obs, (*j).second);
			Cudd_Ref(pos);

		//	std::cout << " pos " << std::endl;
//			printBDD(pos);

			atoms->insert((*j).first);
			//if(pos != Cudd_ReadLogicZero(manager))
			recurse_compose_outcomes(observables, observations, pos, atoms);
		    atoms->erase((*j).first);
			Cudd_RecursiveDeref(manager, pos);

		}
		  //  std::cout << "exit" << std::endl;
		observables->insert(t);
	}
}


/* ====================================================================== */
/* action_mtbdds */

//Pr(o1 | s) 1-Pr(-o1 | s)
//
//Pr(o1 | s) Pr(o1 | -s)

//Pr(o1,o2,o3 | s)
//
//Pr(o1,o2,o3 | s)

std::list<DdNode*>*
pomdp_observation_mtbdds(ObservationVector& ob,
		const Problem& problem){
	ObservationEntry *e;
	std::map<DdNode*, DdNode*> observables;
	std::list<DdNode*>* observations = new std::list<DdNode*>();
	DdNode *symbol, *states, *tmp, *pr;

	//  cout << "|vars| = " << state_variables.size() << endl;


	for(ObservationVector::iterator i = //(const struct ObservationEntry**)
			ob.begin();
			i != ob.end(); i++){

		e = ((ObservationEntry*)(*i));
		// cout << "symbol"<<endl;
		symbol = formula_bdd(e->symbol());
		Cudd_Ref(symbol);

		//     e->symbol().print(cout, problem.domain().predicates(),
		// 		      problem.domain().functions(),
		// 		      problem.terms());
		//printBDD(symbol);

		tmp = formula_bdd(e->formula());
		Cudd_Ref(tmp);



		states = Cudd_BddToAdd(manager, tmp);
		Cudd_Ref(states);
		Cudd_RecursiveDeref(manager, tmp);
		pr = Cudd_addConst(manager, e->posProbability().double_value());
		Cudd_Ref(pr);
		tmp = Cudd_addApply(manager, Cudd_addTimes, pr, states);
		Cudd_Ref(tmp);
		Cudd_RecursiveDeref(manager, pr);
		Cudd_RecursiveDeref(manager, states);

		if(observables.find(symbol) == observables.end())
			states = Cudd_ReadZero(manager);
		else
			states = observables[symbol];

		Cudd_Ref(states);

		pr = Cudd_addApply(manager, Cudd_addPlus, states, tmp);

		Cudd_Ref(pr);

		observables[symbol] = pr;

		//printBDD(pr);

		Cudd_RecursiveDeref(manager, states);
		Cudd_RecursiveDeref(manager, tmp);
		Cudd_RecursiveDeref(manager, symbol);

	}

	for(std::map<DdNode*, DdNode*>::iterator j = observables.begin();
			j != observables.end(); j++){
		observations->push_back((*j).second);
		// printBDD((*j).second);
	}
	//cout << "done" << endl;
	return observations;
}



std::list<std::map<const pEffect*, DdNode*>*> *
observation_cpt_mtbdds(const Action& action,
		const Problem& problem){
  //std::set<std::map<const pEffect*, DdNode*, effcomp>*> combined_cpts;


  //  const Atom *atom;
  const pEffect* eff;
  ObservationEntry *e;
  ObservationVector& ob = (ObservationVector&)((Observation&)((Action&)action).observation()).obVector();
  
  // std::cout << "doing obs for " << action.name() << " " << ob.size() <<std::endl;
  
  DdNode * tmp, *tmp1, *tmp2, *fr, *fr1;
  std::list<std::map<const pEffect*, DdNode*, effcomp >*> *observations = new std::list<std::map<const pEffect*, DdNode*, effcomp>*>();

  std::map<const pEffect*, DdNode*, effcomp>* cpt;

  for(ObservationVector::iterator i =ob.begin(); i != ob.end(); i++){
    //TODO if next cpt, then make new cpt
    e = ((ObservationEntry*)(*i));
    ProbabilisticEffect obs = e->obs();

    //check existing cpts to see if one exists yet
    bool found = false;
    for(std::list<std::map<const pEffect*, DdNode*, effcomp>*>::iterator j = observations->begin();
	j != observations->end(); j++){
      //does there exist at least one of the observables in the cpt distribution
      for(int k = 0; k < obs.size(); k++){
	const pEffect* eff = &obs.effect(k);
	std::map<const pEffect*, DdNode*, effcomp>::iterator q = (*j)->find(eff);
	if(q != (*j)->end()){
	  found = true;
	  cpt = *j;
	  break;
	}
	//check if negation is in cpt
	const AddEffect *ae = dynamic_cast<const AddEffect*>(eff);
	const DeleteEffect *de = dynamic_cast<const DeleteEffect*>(eff);
	pEffect *negatedEffect;
	if(ae != NULL){
	  negatedEffect = new DeleteEffect(ae->atom());
	}
	else if(de != NULL){
	  negatedEffect = new AddEffect(de->atom());
	}
	q = (*j)->find(negatedEffect);
	if(q != (*j)->end()){
	  found = true;
	  cpt = *j;
	  break;
	}
      }

    }
    if(!found){
      //std::cout << "New cpt" << std::endl;
      cpt =  new std::map<const pEffect*, DdNode*, effcomp>();
      observations->push_back(cpt);
    }



    
    fr = formula_bdd(e->formula());
    Cudd_Ref(fr);
    tmp1 = Cudd_BddToAdd(manager, fr);
    Cudd_Ref(tmp1);
    Cudd_RecursiveDeref(manager, fr);



    //std::cout << "eff" << std::endl;
    for(int j = 0 ; j < obs.size(); j++){
      double pr = obs.probability(j).double_value();
      eff = &(obs.effect(j));
      tmp2 = Cudd_addConst(manager, pr);
      Cudd_Ref(tmp2);

 //      eff->print(std::cout, problem.domain().predicates(),
// 		 problem.domain().functions(),
// 		 problem.terms());
      //      printBDD(tmp1);
      //xprintBDD(tmp2);
      
      fr = Cudd_addApply(manager, Cudd_addTimes, tmp1, tmp2);
      Cudd_Ref(fr);
      Cudd_RecursiveDeref(manager, tmp2);

      std::map<const pEffect*, DdNode*, effcomp>::iterator k = cpt->find(eff);
      if(k != cpt->end()){
	//      fr1 = (*cpt)[eff];
	//printBDD(fr);
	//printBDD(fr1);
	
	//      if(fr1 != NULL){
	//std::cout << "found" << std:: endl;
	fr1 = (*k).second;
	tmp = Cudd_addApply(manager, Cudd_addMaximum, fr, fr1);
	Cudd_Ref(tmp);
	Cudd_RecursiveDeref(manager, fr1);
	//(*cpt)[eff] = tmp;
	//	Cudd_RecursiveDeref(manager, fr1);
	(*k).second = tmp;
	Cudd_RecursiveDeref(manager, fr);
      }
      else{
	//std::cout << "new" << std:: endl;
	//	printBDD(fr);
	//(*cpt)[eff] = fr;
	std::pair<const pEffect*, DdNode*>* n = new std::pair<const pEffect*, DdNode*>(eff, fr);
	cpt->insert(*n);
			}
      //      printBDD((*cpt)[eff]);
    }
    Cudd_RecursiveDeref(manager, tmp1);

  }

  //	recurse_compose_outcomes(&combined_cpts, observations, Cudd_ReadOne(manager), new std::set<const pEffect*>());
  return (std::list<std::map<const pEffect*, DdNode*>*>*)observations;
}



//std::map<const StateFormula*, DdNode*>
//std::map<DdNode*, DdNode*> *
//map<DdNode*, double>*
std::list<DdNode*>*
observation_mtbdds(const Action& action,
		const Problem& problem){
	//  std::map<const StateFormula*, DdNode*> ret;
	std::map<DdNode*, DdNode*>* ret = new std::map<DdNode*, DdNode*>();


	ObservationVector& ob = (ObservationVector&)((Observation&)((Action&)action).observation()).obVector();
	ObservationEntry *e;




	//  std::cout << "doing obs for " << action.name() << " " << ob.size() <<std::endl;

	/* CHECK for POMDP style observations */
	e = ((ObservationEntry*)(*(ob.begin())));
	if(&(e->symbol()) != NULL){
		return pomdp_observation_mtbdds(ob, problem);
	}


	DdNode * tmp, *tmp1, *tmp2, *fr, *fr1;
	std::map<DdNode*, std::pair<double, double>*> observables;
	std::list<DdNode*> observations;
	//map<DdNode*, std::pair<double, double>*>* outcomes = new map<DdNode*, std::pair<double, double>*>();
	std::list<DdNode*>* observFns = new std::list<DdNode*>();



	//compose all joint outcomes and compute their probabilitie
	// 存储每个fact BDD的概率
	for(ObservationVector::iterator i = //(const struct ObservationEntry**)
			ob.begin();
			i != ob.end(); i++){

		e = ((ObservationEntry*)(*i));
		fr = formula_bdd(e->formula());
		Cudd_Ref(fr);
		observables[fr] = new std::pair<double, double>(e->posProbability().double_value(),
				e->negProbability().double_value());

		//  cout << "((" << e->probability().double_value() << "))" <<endl;
	}

	// combine_observables(observables

	recurse_compose_outcomes(&observables, &observations, Cudd_ReadOne(manager));



	for(std::list<DdNode*>::iterator j = observations.begin();
			j != observations.end(); j++){



		DdNode* obsV = Cudd_ReadOne(manager);//Cudd_BddToAdd(manager, *j);
		Cudd_Ref(obsV);

		//   std::cout << "obsV:"<<endl;
		//  printBDD(obsV);

		for(std::map<DdNode*, std::pair<double, double>*>::iterator i = observables.begin();
				problem.domain().requirements.probabilistic && i != observables.end(); i++){
			std::pair<DdNode*, std::pair<double, double>*> obs = *i;


			//     std::cout << "obs:"<<std::endl;
			//          printBDD(obs.first);

			tmp1 = Cudd_ReadZero(manager);
			Cudd_Ref(tmp1);


			//probabilities that observation is right or wrong
			DdNode* posRight = Cudd_addConst(manager, (double)obs.second->first);
			Cudd_Ref(posRight);
			DdNode* posWrong = Cudd_addConst(manager, (1.0-(double)obs.second->first));
			Cudd_Ref(posWrong);
			DdNode* negRight = Cudd_addConst(manager, (double)obs.second->second);
			Cudd_Ref(negRight);
			DdNode* negWrong = Cudd_addConst(manager, (1.0-(double)obs.second->second));
			Cudd_Ref(negWrong);

			//the positive outcome in current state or in outcome
			DdNode* posB = Cudd_BddToAdd(manager, obs.first);
			Cudd_Ref(posB);
			DdNode* posA =  Cudd_addPermute(manager, posB, varmap);
			Cudd_Ref(posA);

			//the negative outcome in current state or in outcome
			DdNode* negB = Cudd_BddToAdd(manager, Cudd_Not(obs.first));
			Cudd_Ref(negB);
			DdNode* negA =  Cudd_addPermute(manager, negB, varmap);
			Cudd_Ref(negA);

			//     printBDD(obs.first);
			//     printBDD(posA);
			//     printBDD(Cudd_Not(obs.first));
			//     printBDD(negA);
			//      printBDD(right);
			//      printBDD(wrong);
			if(bdd_entailed(manager, *j, obs.first)){
				//	std::cout << "TP " << posRight << std::endl;
				//obs is observed true, and is true
				DdNode* obsTT = Cudd_addApply(manager, Cudd_addTimes, posA, posRight);
				Cudd_Ref(obsTT);
				tmp = Cudd_addApply(manager, Cudd_addPlus, tmp1, obsTT);
				Cudd_Ref(tmp);
				Cudd_RecursiveDeref(manager, tmp1);
				tmp1 = tmp;
				Cudd_Ref(tmp1);
				Cudd_RecursiveDeref(manager, tmp);
				Cudd_RecursiveDeref(manager, obsTT);
			}

			if(bdd_entailed(manager, *j, Cudd_Not(obs.first))){
				///	std::cout << "TF " << posWrong << std::endl;
				//obs is observed true, and is false
				DdNode* obsTF = Cudd_addApply(manager, Cudd_addTimes, posA, posWrong);
				Cudd_Ref(obsTF);
				tmp = Cudd_addApply(manager, Cudd_addPlus, tmp1, obsTF);
				Cudd_Ref(tmp);
				Cudd_RecursiveDeref(manager, tmp1);
				tmp1 = tmp;
				Cudd_Ref(tmp1);
				Cudd_RecursiveDeref(manager, tmp);
				Cudd_RecursiveDeref(manager, obsTF);
			}

			if(bdd_entailed(manager, *j, obs.first)){
				///	std::cout << "FT " << negWrong << std::endl;
				//obs is observed false, and is true
				DdNode* obsFT = Cudd_addApply(manager, Cudd_addTimes, negA, negWrong);
				Cudd_Ref(obsFT);
				tmp = Cudd_addApply(manager, Cudd_addPlus, tmp1, obsFT);
				Cudd_Ref(tmp);
				Cudd_RecursiveDeref(manager, tmp1);
				tmp1 = tmp;
				Cudd_Ref(tmp1);
				Cudd_RecursiveDeref(manager, tmp);
				Cudd_RecursiveDeref(manager, obsFT);
			}

			if(bdd_entailed(manager, *j, Cudd_Not(obs.first))){
				//	std::cout << "FF " << negRight << std::endl;
				//obs is observed false, and is false
				DdNode* obsFF = Cudd_addApply(manager, Cudd_addTimes, negA, negRight);
				Cudd_Ref(obsFF);
				tmp = Cudd_addApply(manager, Cudd_addPlus, tmp1, obsFF);
				Cudd_Ref(tmp);
				Cudd_RecursiveDeref(manager, tmp1);
				tmp1 = tmp;
				Cudd_Ref(tmp1);
				Cudd_RecursiveDeref(manager, tmp);
				Cudd_RecursiveDeref(manager, obsFF);
			}
			//    cout <<"["<<endl;
			//    Cudd_CheckKeys(manager);
			//    cout <<"|"<<endl;

			//       cout << "HO"<<endl;
			//       printBDD(tmp1);
			//       printBDD(obsV);

			tmp = Cudd_addApply(manager, Cudd_addTimes, tmp1, obsV);
			Cudd_Ref(tmp);
			Cudd_RecursiveDeref(manager, obsV);
			obsV = tmp;
			Cudd_Ref(obsV);
			Cudd_RecursiveDeref(manager, tmp);

			//   Cudd_CheckKeys(manager);
			//    cout <<"]"<<endl;

			Cudd_RecursiveDeref(manager, posRight);
			Cudd_RecursiveDeref(manager, posWrong);
			Cudd_RecursiveDeref(manager, negRight);
			Cudd_RecursiveDeref(manager, negWrong);
			Cudd_RecursiveDeref(manager, posA);
			Cudd_RecursiveDeref(manager, posB);
			Cudd_RecursiveDeref(manager, negA);
			Cudd_RecursiveDeref(manager, negB);



		}

		if(problem.domain().requirements.probabilistic){
			//swap vars
			tmp1 = Cudd_addPermute(manager, obsV, varmap);
			Cudd_Ref(tmp1);
			Cudd_RecursiveDeref(manager, obsV);
		}
		else{
			tmp1 = (*j);
			Cudd_Ref(tmp1);
		}
		//store observation

		//printBDD(tmp1);
		if(problem.domain().requirements.probabilistic)
			observFns->push_back(tmp1);
		else{
			observFns->push_back(tmp1);
		}
		//      std::cout << "obsVE:"<<std::endl;
		// printBDD(tmp1);



	}
	// printBDD(observFn);

	for(std::map<DdNode*, std::pair<double, double>*>::iterator i = observables.begin();
			i != observables.end(); i++){
		Cudd_RecursiveDeref(manager, (*i).first);
		delete (*i).second;
	}
	for(std::list<DdNode*>::iterator i = observations.begin();
			i != observations.end(); i++){
		Cudd_RecursiveDeref(manager, (*i));
	}

	return observFns;




	//   //for every observation formula
	//   for(ObservationVector::iterator i = (const ObservationEntry**)ob.begin();
	//       i != ob.end(); i++){

	//     //get the formula as an ADD weigted by its probability of being observed
	//     //correctly
	//     e = ((ObservationEntry*)(*i));
	//     fr = formula_bdd(e->formula());
	//     Cudd_Ref(fr);
	//     tmp = Cudd_BddToAdd(manager, fr);
	//     Cudd_Ref(tmp);
	//     // printBDD(tmp);
	//     Cudd_RecursiveDeref(manager, fr);

	//     tmp1 = Cudd_addConst(manager, e->probability().double_value());
	//     Cudd_Ref(tmp1);
	//     //printBDD(tmp1);

	//     tmp2 = Cudd_addApply(manager, Cudd_addTimes, tmp1, tmp);
	//     Cudd_Ref(tmp2);
	//     //printBDD(tmp2);
	//     //cout <<endl;

	//      Cudd_RecursiveDeref(manager, tmp);
	//     Cudd_RecursiveDeref(manager, tmp1);
	//     //     cout << "sym "<<endl;
	//      //     e->symbol().print(cout, ((Domain)problem.domain()).observables(),
	//      // 		      problem.domain().functions(),
	//      // 		      problem.terms());




	//     fr1 =  formula_bdd(e->symbol());
	//     Cudd_Ref(fr1);

	//     //     printBDD(fr1);
	//     // std::map<const StateFormula*, DdNode*>::iterator j = ret.find(&e->symbol());
	//     std::map<DdNode*, DdNode*>::iterator j = (*ret).find(fr1);

	//     //check repeats
	//     //  for(std::map<DdNode*, DdNode*>::iterator i =  ret->begin();
	//     //      i != ret->end(); i++){
	//     //    if(tmp2 == (*i).second){
	//     //      cout << "REPEAT" <<endl;
	//     //      printBDD((*i).first);
	//     //    }
	//     //  }


	//     if(j == ret->end()){
	//       //ret.insert(*new std::pair<const StateFormula*, DdNode*>(&e->symbol(),tmp2));
	//       // cout << "New"<<endl;
	//       //ret[&e->symbol()] = tmp2;
	//       (*ret)[fr1] = tmp2;

	//     }
	//     else{
	//       // cout << "Old"<<endl;
	//       tmp = Cudd_addApply(manager, Cudd_addPlus, tmp2, (*j).second);
	//       Cudd_Ref(tmp);
	//       Cudd_RecursiveDeref(manager, (*j).second);
	//       Cudd_RecursiveDeref(manager, tmp2);
	//       (*j).second = tmp;
	//       Cudd_Ref((*j).second);
	//       Cudd_RecursiveDeref(manager, tmp);
	//     }



	//   }
	// //   Cudd_CheckKeys(manager);
	// //   cout <<"]"<<endl;


	//   // for(std::map<DdNode*, DdNode*>::iterator j = ret->begin();
	//   //   j != ret->end(); j++)
	//   //    printBDD((*j).second);

	//   if(ret->size() > 0)
	//     return ret;
	//   else{
	//     delete ret;
	//     return NULL;
	//   }

}



/* Constructs MTBDDs representing the transition probability matrix and
   reward vector for the given action. */
//static
std::pair<DdNode*, DdNode*> action_mtbdds(const Action& action,
		const Problem& problem,
		DdNode* ddng,
		DdNode* ddgp,
		DdNode* col_cube,
		bool event) {
	/*
	 * 前面以及分配好了每一种effect的概率，接下来收集这些信息构建转换矩阵，同时需要保证每一种outcome是互斥的
	 * Collect probabilistic outcomes for the action, and make sure that
	 * all transition sets for each outcome have mutually exclusive
	 * conditions.
	 */
	// 注释掉输出
	/*
	std::cout << "####### start action mtbdds######\n";
	action.print(std::cout, problem.terms());
	std::cout << std::endl;
	action.precondition().print(std::cout, problem.domain().predicates(), problem.domain().functions(), problem.terms());
	std::cout << std::endl;
	action.effect().print(std::cout, problem.domain().predicates(), problem.domain().functions(), problem.terms());
	std::cout << std::endl;
	*/

	//printBDD(ddng);


	OutcomeSet* outcomes = new OutcomeSet();
	DdNode *ddc;

	// 根据action获取BDD
	if(!event)
		ddc = action_preconds[&action];//formula_bdd(action.precondition());
	else
		ddc = event_preconds[&action];//formula_bdd(action.precondition());
	Cudd_Ref(ddc);

	//  std::cout << "getting outcomes" << std::endl;
	effect_outcomes(*outcomes, ddc, action.effect());
	//  std::cout << "got outcomes" <<std::endl;

	if(!LUGTOTEXT)
	{
		// std::cout << "!LUG TO TEXT\n";
		ensure_mutex(*outcomes); // 进行outcome转换的互斥优化
	}
	else{
		std::cout << "for unconditional\n";
		// 这里仅有一个outcome，记录其effect的个数
		for (TransitionSetList::const_iterator ti =
				 outcomes->transitions[0].begin();
			 ti != outcomes->transitions[0].end(); ti++)
		{
			num_alt_effs++;
		}
		num_alt_effs++;//for unconditional
	}
	// std::cout << "done mutex" << std::endl;

	if(!event){
		// std::cout << "store the action outcome bdd into table\n";
		action_outcomes.insert(std::pair<const Action*, OutcomeSet*>(&action, outcomes));

	}
	else{
		std::cout << "store the event outcome bdd into table\n";
		event_outcomes.insert(std::pair<const Action *, OutcomeSet *>(&action, outcomes));
	}


	if (verbosity >= 3) {
		std::cout << std::endl << "Outcomes for ";
		action.print(std::cout, problem.terms());
		std::cout << ':' << std::endl;
		print_outcomes(std::cout, *outcomes);
	}

	if(LUGTOTEXT){
		std::cout << "LUG Zero mtbdd\n";
		assert(0);
		return std::make_pair(Cudd_ReadZero(manager), Cudd_ReadZero(manager));
	}

	/*
	 * Construct MTBDD representations of the transition probability
	 * matrix and transition reward matrix for the action.
	 *
	 * The transition probability matrix for an action is the sum of the
	 * transition probability matrices of each probabilistic outcome.
	 *
	 * The transition reward matrix for an action is NOT simply the sum
	 * of the transition reward matrices for each probabilistic outcome.
	 * Each probabilistic outcome associates rewards with a set of state
	 * transitions (a subset of all state transitions).  The reward for
	 * a specific state transition must be the same for all
	 * probabilistic outcomes that associate a reward with that state
	 * transition.  An exception is thrown if the action defines
	 * inconsistent transition rewards across probabilistic outcomes.
	 * The reward for a state transition in the transition reward matrix
	 * for the action is the reward defined by some probabilistic
	 * outcome.
	 */
	/**
	 * momo007 2022.05.27
	 * 对于reward matrix直接返回0，内部创建相关的代码注释
	 */
	/* This is going to be the transition probability matrix. */
	DdNode* ddP = Cudd_ReadZero(dd_man);
	Cudd_Ref(ddP);
	/* This is going to be the transition reward matrix. */
	DdNode* ddR = Cudd_ReadZero(dd_man);
	Cudd_Ref(ddR);
	/* This is a BDD representing all transitions by any outcome, and is
     used to accurately compute the transition reward matrix. */
	DdNode* ddD = Cudd_ReadLogicZero(dd_man);
	Cudd_Ref(ddD);
	size_t n = outcomes->probabilities.size();


	// Cudd_CheckKeys(manager);

	for (size_t i = 0; i < n; i++) {// 考虑每个outcome

		/*
		 * Construct MTBDD representations of the transition probability
		 * and reward matrices for the ith probabilistic outcome.
		 */
		/* This is going to be a BDD representing the state transitions
       defined by this outcome. */
		DdNode* ddT = Cudd_ReadLogicZero(dd_man);
		Cudd_Ref(ddT);

		/* This is going to be a BDD representing the conjunction of the
       negations of each individual transition set condition.  We need
       to add self-loops for all states satisfying this formula (this
       makes the transition set conditions not only mutually
       exclusive, but also exhaustive). */
		DdNode* ddN = Cudd_ReadOne(dd_man);
		Cudd_Ref(ddN);

		for (TransitionSetList::const_iterator ti =
				outcomes->transitions[i].begin();
				ti != outcomes->transitions[i].end(); ti++) {

			/*
			 * Process an individual transition set.
			 */
			int err = 0;
			const TransitionSet& t = **ti;
			if (t.effect_bdd() == Cudd_ReadLogicZero(dd_man)) {
				err = 1;
			}

			if(event && t.effect_bdd() == Cudd_ReadOne(dd_man))
				err = 1;


			/**
			 * 单独的一个transitionSet通过 condition /\ effect /\ identity(whhich is untouched)
			 * The BDD representation of the transition matrix for a single
			 * transition set is computed as the conjunction of the
			 * condition BDD, the effect BDD, and the identity BDDs for each
			 * untouched state variable.
			 */
			DdNode* ddt = Cudd_ReadOne(dd_man);
			Cudd_Ref(ddt);
			//  cout << "Effect:"<<endl;
			//printBDD(t.effect_bdd());
			/*
			 * First account for untouched state variables.
			 */
			//     cout << "[" << endl;
			//      Cudd_CheckKeys(manager);
			if(!event){
				// 将没有修改的状态变量BDD合取保存在ddt中
				for (int vi = nvars - 1; vi >= 0; vi--) {
					// 前后状态一样
					if (t.touched_variables().find(vi) == t.touched_variables().end()) {
						// 直接和取原始BDD x==x'
						DdNode* ddi = Cudd_bddAnd(dd_man, identity_bdds[vi], ddt);
						Cudd_Ref(ddi);
						Cudd_RecursiveDeref(dd_man, ddt);
						ddt = ddi;
						// 	  Cudd_Ref(ddt);
						// 	  Cudd_RecursiveDeref(dd_man, ddi);
					}
				}
			}
			//   cout << "|" << endl;
			//      Cudd_CheckKeys(manager);
			//      cout << "]" <<endl;

			DdNode *dda;
			if(!err){
				/*
				 * Next, the effect. 和区effect的BDD
				 */
				//       cout << "anding effect"<<endl;
				dda = Cudd_bddAnd(dd_man, t.effect_bdd(), ddt);
				Cudd_Ref(dda);
				Cudd_RecursiveDeref(dd_man, ddt);
				ddt = dda;
				/*
				 * Finally, the condition.和区action的BDD
				 */
				dda = Cudd_bddAnd(dd_man, t.condition_bdd(), ddt);
				Cudd_Ref(dda);
				Cudd_RecursiveDeref(dd_man, ddt);
				ddt = dda;


				/*
				 * Add the transition matrix for the current transition set to
				 * the transition matrix for the outcome.
				 * 将该transitionSet析取到outcome中
				 */
				//      cout << "Oring effect"<<endl;
				DdNode* ddo = Cudd_bddOr(dd_man, ddt, ddT);
				Cudd_Ref(ddo);
				Cudd_RecursiveDeref(dd_man, ddT);
				ddT = ddo;// ddT存储了一个outcome的所有的转换关系

				//      printBDD(ddt);
				Cudd_RecursiveDeref(dd_man, ddt);
			}
			else{//err - inconsistent effect
				/*
				 * Finally, the condition.
				 */
				// std::cout << "err inconsistent effect\n";
				dda = Cudd_bddAnd(dd_man, Cudd_Not(t.condition_bdd()), ddt);
				Cudd_Ref(dda);
				Cudd_RecursiveDeref(dd_man, ddt);
				ddt = dda;

			}

			/*
			 * Remove the condition for the current transition set from the
			 * condition representing uncovered states.
			 * (用于构造frame axioms)
			 * ddN存储所有transitionSet的否定con的合取。
			 */
			DdNode* ddn = Cudd_Not(t.condition_bdd());

			Cudd_Ref(ddn);
			dda = Cudd_bddAnd(dd_man, ddn, ddN);
			Cudd_Ref(dda);
			Cudd_RecursiveDeref(dd_man, ddn);
			Cudd_RecursiveDeref(dd_man, ddN);
			ddN = dda;

		}// end-for transitionSet

		/**
		 * Add self-loops for all states not covered by any transition set
		 * conditions.
		 * 为该outcome的转换集合没有覆盖到的状态添加自循环。
		 */
		bool PPDDLTOST=false;
		DdNode* dda;
		if(!PPDDLTOST){

			dda = Cudd_bddAnd(dd_man, ddc, ddN);// action的precondition合取ddN，Effect‘ axioms完整
			Cudd_Ref(dda);
			Cudd_RecursiveDeref(dd_man, ddN);
			if (dda != Cudd_ReadLogicZero(dd_man)) {// 非空说明Effect存在转换
				ddN = dda;
				// dda计算得到Frame axioms
				dda = Cudd_bddAnd(dd_man, ddN, identity_bdd);// 合取X=X'等价的BDD 
				Cudd_Ref(dda);
				Cudd_RecursiveDeref(dd_man, ddN);
				ddN = dda;
				// Frame axioms析取到outcome set中
				DdNode* ddo = Cudd_bddOr(dd_man, ddN, ddT);
				Cudd_Ref(ddo);
				Cudd_RecursiveDeref(dd_man, ddN);
				Cudd_RecursiveDeref(dd_man, ddT);
				ddT = ddo;
			} else {
				Cudd_RecursiveDeref(dd_man, dda);
			}
		}
		/*
		 * Add the transitions of this outcome to the BDD representing all
		 * transitions.
		 * ddT \/ ddD 将当前outcome的BDD合取outcomeSet
		 */
		DdNode* ddo = Cudd_bddOr(dd_man, ddT, ddD);
		Cudd_Ref(ddo);
		Cudd_RecursiveDeref(dd_man, ddD);
		ddD = ddo;
		Cudd_RecursiveDeref(dd_man, ddT);
	}// end-for outcome


	/**
	 * momo007 2022.06.29 下面改行需要注释，否则在部分domain上会出现def次数部匹配问题
	 */
	// Cudd_RecursiveDeref(dd_man, ddD);
	//DAN -- need them for pg construction//  free_outcomes(outcomes);
	if (verbosity >= 4) {
		std::cout << std::endl << "Transition probability matrix for ";
		action.print(std::cout, problem.terms());
		std::cout << ':' << std::endl;
		Cudd_PrintDebug(dd_man, ddP, 2*nvars, 2);
	}
	Cudd_RecursiveDeref(dd_man, ddc);
	// momo007 2022.05.27 not used reward
	// if(problem.domain().requirements.non_deterministic){
	// 	// Converts an ADD to a BDD.
	// 	// Replaces all discriminants STRICTLY greater than value with 1, and all other discriminants with 0.
	// 	DdNode *nd = Cudd_addBddStrictThreshold(manager,ddP,0.0);
	// 	Cudd_Ref(nd);
	// 	Cudd_RecursiveDeref(manager, ddP);
	// 	ddP = nd;
	// 	Cudd_Ref(ddP);
	// 	Cudd_RecursiveDeref(manager, nd);
	// }
	// 注释掉输出
	// std::cout << "####### end action mtbdds######\n";
	return std::make_pair(ddD, ddR);
}



DdNode* iterative_assemble_init(std::list<std::list<DdNode*>* >* preffects,
		std::list<std::list<DdNode*>* >::iterator p,
		DdNode* my_initial_state,
		DdNode* model){
	DdNode* result = my_initial_state;
	Cudd_Ref(result);
	for(;p != preffects->end(); p++){
		//    DdNode *t = Cudd_ReadZero(manager);
		//    Cudd_Ref(t);
		std::list<DdNode*> r;
		for(std::list<DdNode*>::iterator o = (*p)->begin();
				o != (*p)->end(); o++){
			DdNode *t1 = Cudd_addApply(manager,
					Cudd_addTimes,
					result,
					*o);
			Cudd_Ref(t1);
			r.push_back(t1);
			//       Cudd_RecursiveDeref(manager, t);
			//       t = t1;
			//       Cudd_Ref(t);
			//       Cudd_RecursiveDeref(manager, t1);
			//              std::cout << "o, t"<< std::endl;
			//        printBDD(*o);
			//        printBDD(t1);
		}
		Cudd_RecursiveDeref(manager, result);
		result = Cudd_ReadZero(manager);
		Cudd_Ref(result);
		for(std::list<DdNode*>::iterator o = r.begin();
				o != r.end(); o++){
			DdNode *t1 = Cudd_addApply(manager,
					Cudd_addPlus,
					result,
					*o);
			Cudd_Ref(t1);
			Cudd_RecursiveDeref(manager, result);
			result = t1;
			Cudd_Ref(result);
			Cudd_RecursiveDeref(manager, t1);
			//             std::cout << "o, t"<< std::endl;
			//       printBDD(*o);
			//       printBDD(t);
		}
		//     DdNode *t1 = Cudd_addApply(manager,
		// 			       Cudd_addTimes,
		// 			       t,
		// 			       result);
		//       Cudd_Ref(t1);
		//       Cudd_RecursiveDeref(manager, result);
		//       result = t1;
		//       Cudd_Ref(result);
		//       Cudd_RecursiveDeref(manager, t1);

		//          std::cout << "result" << std::endl;
		//       printBDD(result);
	}
	return result;

	//   DdNode* result = my_initial_state;
	//   Cudd_Ref(result);
	//   for(;p != preffects->end(); p++){
	//     DdNode *t = Cudd_ReadZero(manager);
	//     Cudd_Ref(t);
	//     for(std::list<DdNode*>::iterator o = (*p)->begin();
	// 	o != (*p)->end(); o++){
	//       DdNode *t1 = Cudd_addApply(manager,
	// 				Cudd_addPlus,
	// 				 t,
	// 				 *o);
	//       Cudd_Ref(t1);
	//       Cudd_RecursiveDeref(manager, t);
	//       t = t1;
	//       Cudd_Ref(t);
	//       Cudd_RecursiveDeref(manager, t1);
	//             std::cout << "o, t"<< std::endl;
	//       printBDD(*o);
	//       printBDD(t);
	//     }
	//     DdNode *t1 = Cudd_addApply(manager,
	// 			       Cudd_addTimes,
	// 			       t,
	// 			       result);
	//       Cudd_Ref(t1);
	//       Cudd_RecursiveDeref(manager, result);
	//       result = t1;
	//       Cudd_Ref(result);
	//       Cudd_RecursiveDeref(manager, t1);
	//          std::cout << "result" << std::endl;
	//       printBDD(result);
	//   }
	//   return result;
}

DdNode* recurse_assemble_init(std::list<std::list<DdNode*>* >* preffects,
		std::list<std::list<DdNode*>* >::iterator p,
		DdNode* my_initial_state,
		DdNode* model){
	if(p == preffects->end()){//found joint effect model
		//add joint effect to init
		//    cout << "model = " <<endl;
		//    printBDD(model);
		DdNode* tmp;
		if(my_initial_state == Cudd_ReadOne(manager)){
			tmp = model;
		}
		else{
			tmp = Cudd_addApply(manager,
					Cudd_addPlus,
					model,
					my_initial_state);
		}
		Cudd_Ref(tmp);
		//printBDD(tmp);
		return tmp;

	}
	else{//recurse over possibilities of effect
		std::list<std::list<DdNode*>* >::iterator nextP = (p);
		nextP++;
		for(std::list<DdNode*>::iterator o = (*p)->begin();
				o != (*p)->end(); o++){
			DdNode* tmp = Cudd_addApply(manager,
					Cudd_addTimes,
					model,
					*o);
			Cudd_Ref(tmp);
			DdNode* tmp1 = recurse_assemble_init(preffects, nextP, my_initial_state, tmp);
			Cudd_Ref(tmp1);
			Cudd_RecursiveDeref(manager, tmp);
			Cudd_RecursiveDeref(manager, my_initial_state);
			my_initial_state = tmp1;
			Cudd_Ref(my_initial_state);
			Cudd_RecursiveDeref(manager, tmp1);

		}
	}
	//  printBDD(my_initial_state);

	return my_initial_state;

}

void make_mutex1(std::list<DdNode*>* preffect, DdNode *varsUsed){
	//for each outcome, if not mention var, then negate
	for(std::list<DdNode*>::iterator i = preffect->begin();
			i != preffect->end(); i++){
		DdNode *tmp = Cudd_addBddThreshold(manager, *i, Cudd_ReadEpsilon(manager));
		Cudd_Ref(tmp);
		//      printBDD(*i);
		//      printBDD(tmp);
		//      printBDD(varsUsed);
		for(int j = 0; j < num_alt_facts; j++){
			if(Cudd_bddIsVarEssential(manager,varsUsed, (2*j)+1, 1) &&
					!bdd_entailed(manager, tmp, Cudd_bddIthVar(manager, (2*j)+1))){
				//!Cudd_bddIsVarEssential(manager,tmp, (2*j)+1, 1) ){
				DdNode *t = Cudd_bddAnd(manager, tmp, Cudd_Not(Cudd_bddIthVar(manager, (2*j)+1)));
				Cudd_Ref(t);
				Cudd_RecursiveDeref(manager, tmp);
				tmp = t;
				Cudd_Ref(tmp);
				Cudd_RecursiveDeref(manager, t);
			}
			//	printBDD(tmp);
		}
		DdNode* t = Cudd_BddToAdd(manager, tmp);
		Cudd_Ref(t);
		Cudd_RecursiveDeref(manager, tmp);
		tmp = t;
		Cudd_Ref(tmp);
		Cudd_RecursiveDeref(manager, t);

		t = Cudd_addApply(manager, Cudd_addTimes, tmp, *i);
		Cudd_Ref(t);
		Cudd_RecursiveDeref(manager, *i);
		*i = t;
		Cudd_Ref(*i);
		Cudd_RecursiveDeref(manager, t);
		Cudd_RecursiveDeref(manager, tmp);
		//      printBDD(*i);
	}
}

void make_mutex(std::list<DdNode*>* preffect){
	std::list<DdNode*> out;

	for(std::list<DdNode*>::iterator i = preffect->begin();
			i != preffect->end(); i++){
		// std::cout << "i = " <<std::endl;
		// printBDD(*i);
		DdNode* tmpd = *i;
		Cudd_Ref(tmpd);
		for(std::list<DdNode*>::iterator j = preffect->begin();
				j != preffect->end(); j++){
			if(i != j){
				// std::cout << "j = " <<std::endl;
				// printBDD(*j);
				DdNode* tmpi = Cudd_addBddStrictThreshold(manager, tmpd, 0.0);
				Cudd_Ref(tmpi);


				DdNode* tmpj = Cudd_addBddStrictThreshold(manager, *j, 0.0);
				Cudd_Ref(tmpj);
				DdNode *tmpnj = Cudd_Not(tmpj);
				Cudd_Ref(tmpnj);
				Cudd_RecursiveDeref(dd_man, tmpj);

				if(tmpnj == Cudd_ReadLogicZero(manager)){
					Cudd_RecursiveDeref(dd_man, tmpnj);
					tmpnj = Cudd_ReadOne(manager);
					Cudd_Ref(tmpnj);
				}


				DdNode* tmp = Cudd_bddAnd(manager, tmpi, tmpnj);
				Cudd_Ref(tmp);
				Cudd_RecursiveDeref(dd_man, tmpi);
				Cudd_RecursiveDeref(dd_man, tmpnj);
				tmpi = Cudd_BddToAdd(manager, tmp);
				Cudd_Ref(tmpi);
				Cudd_RecursiveDeref(dd_man, tmp);
				tmpj = Cudd_addApply(manager, Cudd_addTimes, tmpd, tmpi);
				Cudd_Ref(tmpj);
				Cudd_RecursiveDeref(dd_man, tmpi);
				Cudd_RecursiveDeref(dd_man, tmpd);
				tmpd = tmpj;
				Cudd_Ref(tmpd);
				Cudd_RecursiveDeref(dd_man, tmpj);
			}
		}
		out.push_back(tmpd);
		// std::cout << "tmpd = " <<std::endl;
		// printBDD(tmpd);
	}

	for(std::list<DdNode*>::iterator i = preffect->begin();
			i != preffect->end(); i++){
		Cudd_RecursiveDeref(manager, *i);
	}
	preffect->clear();
	preffect->merge(out);
}

/**
 *  momo007 013 2022.04.27
 * 这个函数实现了初始状态公式到obdd的转换,如果初始状态不是公式即概率类型还会计算一些DBN
 * 1. collect_init_state_variable，查找所有状态变量个数
 * 2. formula_bdd，将初始状态formula转化为obdd，其中one-of disjunction没太理解
 * 3. 随后将每个状态变量的否定和区上第二部的obdd，为什么需要这一步操作？
 */
void collectInit(const Problem* problem){
	/*
	 * Construct an ADD representing initial states.
	 */
	std::list<DdNode *> worlds;	// zyc 初始化时存放初始状态
	problem->init_formula().print(std::cout, problem->domain().predicates(),
			problem->domain().functions(),
			problem->terms());
	std::cout << std::endl;

	if(&problem->init_formula()){
		// std::cout << "construct the bdd for init formula\n";
		collect_init_state_variables(problem->init_formula()); // 公式中涉及到的状态变量即atom
		// zyc 提取一个可能的初始状态
		DdNode* tmp1 = formula_bdd(problem->init_formula());// 根据初始状态公式创建BDD
		pickKRandomWorlds(tmp1, 1, &worlds);
		DdNode *tmp = worlds.front();
		Cudd_Ref(tmp);
		
		for(int i = 0; i < num_alt_facts; i++){//考虑每个状态变量
			const Atom *a = (*(dynamic_atoms.find(i))).second;// 查看该状态变量的atom
			if(init_variables.find(a) == init_variables.end()){// 查找该公式是否涉及该状态变量
				// clsoed word，没有涉及的值置为0
				DdNode* tmp1 = Cudd_bddAnd(manager, tmp, Cudd_Not(Cudd_bddIthVar(manager, 2*i)));
				Cudd_Ref(tmp1);
				Cudd_RecursiveDeref(manager, tmp);
				tmp = tmp1;
				Cudd_Ref(tmp);
				Cudd_RecursiveDeref(manager, tmp1);
				if(bdd_is_zero(manager,tmp))
				{
					a->print(std::cout, problem->domain().predicates(), problem->domain().functions(), problem->terms());
					abort();
				}
			}
		}
		// zyc 初始化样本(b_initial_state)完毕，为一个可能的初始状态
		b_initial_state = tmp;
		Cudd_Ref(b_initial_state);
		Cudd_RecursiveDeref(manager, tmp);
		// printBDD(b_initial_state);
		return;
	}

	std::cout << "执行了深度信念网络的相关构造[Error]\n";
	assert(false);
	// 这部分对于Conformant planning不会执行
	const Atom *a;
	if(0)  {
		//     ConjunctiveEffect* ce = new ConjunctiveEffect();
		//     for(EffectList::iterator ie=(const pEffect**)problem->init_effects().begin();
		// 	ie!= problem->init_effects().end(); ie++){
		//       ce->add_conjunct(**ie);
		//     }
		//     for(int i = 0; i < num_alt_facts; i+=1){
		//       a = (*(dynamic_atoms.find(i))).second;
		//       if(((problem)->init_atoms().find(a)) !=
		// 	 (problem)->init_atoms().end()){
		// 	ce->add_conjunct(*(new AddEffect(*a)));
		//       }
		//     }

		//     OutcomeSet* outcomes = new OutcomeSet();
		//     DdNode *ddc = Cudd_ReadOne(manager);
		//     Cudd_Ref(ddc);
		//     effect_outcomes(*outcomes, ddc, *ce);
		//     cout << "hi"<<endl;
		//     //ensure_mutex(*outcomes);

		//     /* This is going to be the transition probability matrix. */
		//     DdNode* ddP = Cudd_ReadZero(dd_man);
		//     Cudd_Ref(ddP);
		//     /* This is a BDD representing all transitions by any outcome, and is
		//        used to accurately compute the transition reward matrix. */
		//     DdNode* ddD = Cudd_ReadLogicZero(dd_man);
		//     Cudd_Ref(ddD);
		//     size_t n = outcomes->probabilities.size();
		//     for (size_t i = 0; i < n; i++) {
		//       /* This is going to be a BDD representing the state transitions
		// 	 defined by this outcome. */
		//       cout << "*";
		//       DdNode* ddT = Cudd_ReadLogicZero(dd_man);
		//       Cudd_Ref(ddT);
		//       /* This is going to be a BDD representing the conjunction of the
		// 	 negations of each individual transition set condition.  We need
		// 	 to add self-loops for all states satisfying this formula (this
		// 	 makes the transition set conditions not only mutually
		// 	 exclusive, but also exhaustive). */
		//       DdNode* ddN = Cudd_ReadOne(dd_man);
		//       Cudd_Ref(ddN);
		//       for (TransitionSetList::const_iterator ti =
		// 	     outcomes->transitions[i].begin();
		// 	   ti != outcomes->transitions[i].end(); ti++) {
		// 	cout << "-";
		// 	/*
		// 	 * Process an individual transition set.
		// 	 */
		// 	const TransitionSet& t = **ti;
		// 	if (t.effect_bdd() == Cudd_ReadLogicZero(dd_man)) {
		// 	  /*
		// 	   * The effect of this transition set is inconsistent, meaning
		// 	   * that the same atom is both added and deleted.
		// 	   */
		// 	  //cout << "ERROR: same atom added and deleted"<<endl;
		// 	  //	   action_outcomes.erase(&action);
		// 	  //continue;

		// 	  throw std::logic_error("initial belief has inconsistent effects");
		// 	}
		// 	/*
		// 	 * The BDD representation of the transition matrix for a single
		// 	 * transition set is computed as the conjunction of the
		// 	 * condition BDD, the effect BDD, and the identity BDDs for each
		// 	 * untouched state variable.
		// 	 */
		// 	DdNode* ddt = Cudd_ReadOne(dd_man);
		// 	Cudd_Ref(ddt);
		// 	/*
		// 	 * First account for untouched state variables.
		// 	 */
		// 	for (int vi = nvars - 1; vi >= 0; vi--) {
		// 	  if (t.touched_variables().find(vi) == t.touched_variables().end()) {
		// 	    DdNode* ddi = Cudd_bddAnd(dd_man, identity_bdds[vi], ddt);
		// 	    Cudd_Ref(ddi);
		// 	    Cudd_RecursiveDeref(dd_man, ddt);
		// 	    ddt = ddi;
		// 	    Cudd_Ref(ddt);
		// 	    Cudd_RecursiveDeref(dd_man, ddi);
		// 	  }
		// 	}
		//  	/*
		// 	 * Next, the effect.
		// 	 */
		// 	//       cout << "anding effect"<<endl;
		// 	DdNode* dda = Cudd_bddAnd(dd_man, t.effect_bdd(), ddt);
		// 	Cudd_Ref(dda);
		// 	Cudd_RecursiveDeref(dd_man, ddt);
		// 	ddt = dda;
		// 	/*
		// 	 * Add the transition matrix for the current transition set to
		// 	 * the transition matrix for the outcome.
		// 	 */
		// 	//      cout << "Oring effect"<<endl;
		// 	DdNode* ddo = Cudd_bddOr(dd_man, ddt, ddT);
		// 	Cudd_Ref(ddo);
		// 	Cudd_RecursiveDeref(dd_man, ddT);
		// 	ddT = ddo;
		// 	Cudd_RecursiveDeref(dd_man, ddt);
		// 	/*
		// 	 * Remove the condition for the current transition set from the
		// 	 * condition representing uncovered states.
		// 	 */
		// 	DdNode* ddn = Cudd_Not(t.condition_bdd());

		// 	Cudd_Ref(ddn);
		// 	dda = Cudd_bddAnd(dd_man, ddn, ddN);
		// 	Cudd_Ref(dda);
		// 	Cudd_RecursiveDeref(dd_man, ddn);
		// 	Cudd_RecursiveDeref(dd_man, ddN);
		// 	ddN = dda;
		//       }




		//       /*
		//        * Add self-loops for all states not covered by any transition set
		//        * conditions.
		//        */
		//       DdNode* dda;
		//       dda = Cudd_bddAnd(dd_man, ddc, ddN);
		//       Cudd_Ref(dda);
		//       Cudd_RecursiveDeref(dd_man, ddN);
		//       if (dda != Cudd_ReadLogicZero(dd_man)) {
		// 	ddN = dda;
		// 	dda = Cudd_bddAnd(dd_man, ddN, identity_bdd);
		// 	Cudd_Ref(dda);
		// 	Cudd_RecursiveDeref(dd_man, ddN);
		// 	ddN = dda;
		// 	DdNode* ddo = Cudd_bddOr(dd_man, ddN, ddT);
		// 	Cudd_Ref(ddo);
		// 	Cudd_RecursiveDeref(dd_man, ddN);
		// 	Cudd_RecursiveDeref(dd_man, ddT);
		// 	ddT = ddo;
		//       } else {
		// 	Cudd_RecursiveDeref(dd_man, dda);
		//       }

		//       /*
		//        * Multiply the transition matrix for the current outcome with the
		//        * probability of the outcome, and add the result to the
		//        * transition probability matrix for the action.
		//        */
		//       DdNode* ddp = Cudd_BddToAdd(dd_man, ddT);
		//       Cudd_Ref(ddp);
		//       DdNode* ddk = Cudd_addConst(dd_man,
		// 				  outcomes->probabilities[i].double_value());
		//       Cudd_Ref(ddk);
		//       DdNode* ddt = Cudd_addApply(dd_man, Cudd_addTimes, ddp, ddk);
		//       Cudd_Ref(ddt);
		//       Cudd_RecursiveDeref(dd_man, ddp);
		//       Cudd_RecursiveDeref(dd_man, ddk);
		//       ddp = Cudd_addApply(dd_man, Cudd_addPlus, ddt, ddP);
		//       Cudd_Ref(ddp);
		//       Cudd_RecursiveDeref(dd_man, ddt);
		//       Cudd_RecursiveDeref(dd_man, ddP);
		//       ddP = ddp;
		//       /*
		//        * Add the transitions of this outcome to the BDD representing all
		//        * transitions.
		//        */
		//       DdNode* ddo = Cudd_bddOr(dd_man, ddT, ddD);
		//       Cudd_Ref(ddo);
		//       Cudd_RecursiveDeref(dd_man, ddD);
		//       ddD = ddo;
		//       Cudd_RecursiveDeref(dd_man, ddT);

		//     }


		//     Cudd_RecursiveDeref(dd_man, ddD);
		//     Cudd_RecursiveDeref(dd_man, ddc);
		//     delete ce;
		//     free_outcomes(*outcomes);



		//     DdNode* init;
		//     if(problem->domain().requirements.non_deterministic){
		//       DdNode *nd = Cudd_addBddStrictThreshold(manager,ddP,0.0);
		//       Cudd_Ref(nd);
		//       Cudd_RecursiveDeref(manager, ddP);
		//       ddP = nd;
		//       Cudd_Ref(ddP);
		//       Cudd_RecursiveDeref(manager, nd);



		//       init = Cudd_ReadOne(manager);
		//             cout << "num facts = " << num_alt_facts<<endl;
		//       for(int k = 0; k < num_alt_facts; k++){
		// 	nd = Cudd_Not(Cudd_bddIthVar(manager, 2*k));
		// 	Cudd_Ref(nd);
		// 	ddD = Cudd_bddAnd(manager, init, nd);
		// 	Cudd_Ref(ddD);
		// 	Cudd_RecursiveDeref(manager, nd);
		// 	Cudd_RecursiveDeref(manager, init);
		// 	init = ddD;
		// 	Cudd_Ref(init);
		// 	Cudd_RecursiveDeref(manager, ddD);
		//       }
		//       // cout << "HI"<<endl;
		//       //printBDD(init);
		//       //printBDD(ddP);
		//       // printBDD(current_state_cube);

		//       nd = Cudd_bddAndAbstract(manager,init,
		// 			       ddP,current_state_cube);
		//       Cudd_Ref(nd);
		//       //      printBDD(nd);
		//       b_initial_state = Cudd_bddVarMap(manager, nd);
		//       Cudd_Ref(b_initial_state);
		//       //printBDD(b_initial_state);
		//       Cudd_RecursiveDeref(manager, nd);
		//       Cudd_RecursiveDeref(manager, init);
		//       //      return b_initial_state;


		//     }
		//     else{
		//       DdNode *nd;
		//       init = Cudd_ReadOne(manager);
		//       //      cout << "num facts = " << num_alt_facts<<endl;
		//       for(int k = 0; k < num_alt_facts; k++){
		// 	nd = Cudd_Not(Cudd_bddIthVar(manager, 2*k));
		// 	Cudd_Ref(nd);
		// 	ddD = Cudd_bddAnd(manager, init, nd);
		// 	Cudd_Ref(ddD);
		// 	Cudd_RecursiveDeref(manager, nd);
		// 	Cudd_RecursiveDeref(manager, init);
		// 	init = ddD;
		// 	Cudd_Ref(init);
		// 	Cudd_RecursiveDeref(manager, ddD);
		//       }
		//       ddD = Cudd_BddToAdd(manager, init);
		//       Cudd_Ref(ddD);
		//       Cudd_RecursiveDeref(manager, init);
		//       init = ddD;
		//       Cudd_Ref(init);
		//       Cudd_RecursiveDeref(manager, ddD);



		//       //printBDD(init);
		//       //printBDD(ddP);
		//       // printBDD(current_state_cube);

		//       nd = Cudd_addApply(manager, Cudd_addTimes, init, ddP);
		//       Cudd_Ref(nd);
		//       // cout << "HI"<<endl;
		//       ddD = Cudd_addExistAbstract(manager, nd, current_state_cube);
		//       Cudd_Ref(ddD);



		//       b_initial_state = Cudd_addPermute(manager, ddD, varmap);
		//       Cudd_Ref(b_initial_state);
		//       //printBDD(b_initial_state);
		//       Cudd_RecursiveDeref(manager, nd);
		//       Cudd_RecursiveDeref(manager, init);
		//       Cudd_RecursiveDeref(manager, ddD);
		//       //      return b_initial_state;

		//     }

	}
}


dbn* action_dbn(const Action& action){


	if(action_dbns.count(&action)>0){
		return action_dbns[&action];
	}
	//      std::cout << "Make action DBN: ";
	//  action.print(std::cout, (*my_problem).terms()); std::cout <<std::endl;
	action.setProbabilityFromExpressions(my_problem); //dan
	std::map<const pEffect*, dbn_node*> *pr_nodes = dbn::generate_probabilistic_nodes(&(action.effect()));

	action_dbns[&action] = effect_dbn(action.effect(), pr_nodes);

	//	action_dbns[&action]->add_noops(num_alt_facts);

	//action_dbns[&action]->apply_rewards_and_sink();
	//   for(int i = 0; i < num_alt_facts; i++){
	//     std::cout << (2*i+1) << std::endl;
	//     printBDD(action_dbns[&action]->vars[2*i+1]->cpt);
	//   }
	//   for(int i = 0; i < action_dbns[&action]->num_aux_vars; i++){
	//     std::cout << (2*num_alt_facts+i) << std::endl;
	//     printBDD(action_dbns[&action]->vars[2*num_alt_facts+i]->cpt);
	//   }
	//action_dbns[&action]->get_dds();
	// action_rewards[&action] = action_dbns[&action]->vars[-1]->cpt;
	//action_dbns[&action]->get_reward_dd();

	//    if(action_dbns[&action]->vars[-1]!=NULL)
	//   printBDD(action_dbns[&action]->vars[-1]->cpt);





	std::list<DdNode*>* ddos = NULL;
	std::list<std::map<const pEffect*, DdNode*>*>* ddosc = NULL;
	//   std::map<DdNode*, const ProbabilisticEffect*>* ddosc = NULL;
	int err = 0;
	try{
		if(action.hasObservation()){
			//std::cout << "making obs" << std::endl;

			//	ObservationCpt& cpt = ((ObservationCpt&)((Action&)action).observation_cpt());


			if(OBS_TYPE == OBS_CPT
			){
				ddosc =  observation_cpt_mtbdds(action, *my_problem);
			}
			else{
				ddos = observation_mtbdds(action, *my_problem);
			}

		}
	} catch (std::logic_error& e) {
		std::cout << "caught Exception"<<std::endl;
		//exit(0);
		err = 1;
		//printBDD(dds.first);
	}


	if(!err){
		if(ddos && ddos->size() > 0){//&((Action)action).observation()){
			OBSERVABILITY = OBS_PART;
			action_observations.insert(std::make_pair(&action, ddos));
			ddos = NULL;
		}
		else       if(ddosc && ddosc->size() > 0){//&((Action)action).observation()){
			OBSERVABILITY = OBS_PART;
			action_observations_cpt.insert(std::make_pair(&action, ddosc));
			ddosc = NULL;
		}
	}
	//    std::cout << *action_dbns[&action] << std::endl;
	return action_dbns[&action];
	//    std::cout << "HI" << std::endl;
	//action_dbns[&action]->get_dds();

}
DdNode* getObservationDD(const Action& action){
	assert(action.hasObservation());
	if(action_observations.count(&action) > 0){
		return action_observations[&action]->front();
	}
	std::list<DdNode*>* ddos = NULL; 
	int err = 0;
	action.setProbabilityFromExpressions(my_problem);
	try{
		std::cout << "start Observation BDD()\n";
		ddos = observation_mtbdds(action, *my_problem);
		std::cout << "done Observation BDD()\n";
	} catch (std::logic_error& e) {
		err = 1;
	}
	if(!err){
		if(ddos && ddos->size() > 0){
			OBSERVABILITY = OBS_PART;
			action_observations.insert(std::make_pair(&action, ddos));
			ddos = NULL;
		}
	}
	else{
		std::cout << "ERROR, pruning: ";
		if(LUGTOTEXT)
			num_alt_effs++;//for dummy unconditional effect index
		return Cudd_ReadZero(manager);
	}
	return action_observations[&action]->front();
}
/**
 * momo007 2022.05.12 获取动作的BDD
 * 
 */
DdNode* groundActionDD(const Action& action){
	//  const Problem& problem, DdNode* ddgp, DdNode* ddng,   DdNode* col_cube){

	// std::cout << "lookup: ";
	// action.print(std::cout, (*my_problem).terms());
	// std::cout << std::endl
	//			 << std::flush;
	// 忽略该分支
	if(0 && DBN_PROGRESSION){
		action_dbn(action);
		return Cudd_ReadLogicZero(manager);
	}
	else{
		// 查找该action是否处理过，存在直接返回
		if(action_transitions.count(&action) > 0){
			return action_transitions[&action];
		}

		//    cout << "grounding: ";
		//    action.print(cout, (*my_problem).terms()); cout <<endl;

		int err = 0;
		// 设置每种effect发生的概率
		action.setProbabilityFromExpressions(my_problem); //dan

		std::pair<DdNode*, DdNode*> dds;// 当前状态BDD和后继状态BDD pair
		std::list<DdNode*>* ddos = NULL; // observation的BDD list
		try{
			DdNode *a = Cudd_ReadOne(manager), *b = Cudd_ReadOne(manager);
			Cudd_Ref(a);
			Cudd_Ref(b);
			// 为该动作创建MTBDD表示转换矩阵
			dds = action_mtbdds(action, *my_problem,
					a, b, col_cube, false);
			Cudd_RecursiveDeref(manager, a);
			Cudd_RecursiveDeref(manager, b);


			// 该动作存在observation
			if(action.hasObservation()){
				std::cout << "start Observation BDD()\n";
				ddos = observation_mtbdds(action, *my_problem);
				std::cout << "done Observation BDD()\n";
			}
		} catch (std::logic_error& e) {
			///    std::cout << "caught Exception"<<std::endl;
			//exit(0);
			err = 1;
			//printBDD(dds.first);
		}

		// 处理成功
		if(!err){
			// 添加action和BDD的对应关系。
			action_transitions.insert(std::make_pair(&action, dds.first));
			// 添加动作和reward对应关系
			// action_rewards.insert(std::make_pair(&action, dds.second));
			// 如果是observation，添加observation的转换关系BDD
			if(ddos && ddos->size() > 0){//&((Action)action).observation()){
				OBSERVABILITY = OBS_PART;
				action_observations.insert(std::make_pair(&action, ddos));
				ddos = NULL;
			}
			//     cout << action.name()
			// 	 << " cost = " << alt_act_costs[action.id()-1]
			// 	 << endl;
		}
		else{
			  std::cout << "ERROR, pruning: ";
			//action.print(cout, (*my_problem).terms()); cout <<endl;
			action_transitions.insert(std::make_pair(&action, Cudd_ReadZero(manager)));
			// action_rewards.insert(std::make_pair(&action, Cudd_ReadZero(manager)));
			//     Cudd_RecursiveDeref(manager, dds.first);
			//     Cudd_RecursiveDeref(manager, dds.second);
			//    if(ddos)
			//       delete ddos;

			if(LUGTOTEXT)
				num_alt_effs++;//for dummy unconditional effect index
			return Cudd_ReadZero(manager);
		}
		// printf("%s\n", action.name().c_str());
		// printBDD(dds.first);
		return dds.first;
	}

}

DdNode* groundEventDD(const Action& action){

	//  const Problem& problem, DdNode* ddgp, DdNode* ddng,   DdNode* col_cube){

	//     cout << "lookup: ";
	//      action.print(cout, (*my_problem).terms()); cout <<endl;

	if(event_transitions.count(&action) > 0){
		return event_transitions[&action];
	}

	//cout << "grounding: ";
	//action.print(cout, (*my_problem).terms()); cout <<endl;

	int err = 0;
	action.setProbabilityFromExpressions(my_problem); //dan

	std::pair<DdNode*, DdNode*> dds;
	std::list<DdNode*>* ddos = NULL;
	try{
		DdNode *a = Cudd_ReadOne(manager), *b = Cudd_ReadOne(manager);
		Cudd_Ref(a);
		Cudd_Ref(b);
		dds = action_mtbdds(action, *my_problem,
				a, b, col_cube, true);
		Cudd_RecursiveDeref(manager, a);
		Cudd_RecursiveDeref(manager, b);



		//    if(action.hasObservation()){
		//       ddos = observation_mtbdds(action, *my_problem);
		//     }
	} catch (std::logic_error& e) {
		//    std::cout << "caught Exception"<<std::endl;
		//exit(0);
		err = 1;
		//printBDD(dds.first);
	}

	//  cout << "err = " << err << endl;
	if(!err){
		event_transitions.insert(std::make_pair(&action, dds.first));
		event_rewards.insert(std::make_pair(&action, dds.second));

		//     if(ddos && ddos->size() > 0){//&((Action)action).observation()){
		//       OBSERVABILITY = OBS_PART;
		//       event_observations.insert(std::make_pair(&action, ddos));
		//       ddos = NULL;
		//     }
		//     cout << action.name()
		// 	 << " cost = " << alt_act_costs[action.id()-1]
		// 	 << endl;
	}
	else{
		//   cout << "ERROR, pruning: ";
		//action.print(cout, (*my_problem).terms()); cout <<endl;
		event_transitions.insert(std::make_pair(&action, Cudd_ReadZero(manager)));
		event_rewards.insert(std::make_pair(&action, Cudd_ReadZero(manager)));
		//     Cudd_RecursiveDeref(manager, dds.first);
		//     Cudd_RecursiveDeref(manager, dds.second);
		//    if(ddos)
		//       delete ddos;


		return Cudd_ReadZero(manager);
	}


	return dds.first;
}

void computeDecisiveSets(){
	//mark atoms as static that are not in the decisive set

	std::set<const Atom*> decisive;

	std::map<const Atom*, std::set<const Atom*>* > fpos_dependencies;
	std::map<const Atom*, std::set<const Atom*>* > fneg_dependencies;

	std::map<const Action*, std::set<const Atom*>* > apos_dependencies;
	std::map<const Action*, std::set<const Atom*>* > aneg_dependencies;

	for(std::map<int, const Atom*>::iterator i = dynamic_atoms.begin();
			i != dynamic_atoms.end(); i++){

		//compute literal dependencies

		//compute action dependencies


	}


}


double transform_reward_to_probability(double reward) {
	double pr = ((reward-min_reward)*(1-gDiscount-gEpsilon))/(max_reward-min_reward);
	return pr;

}
double transform_probability_to_reward(double pr) {
	double rew = ((pr*(max_reward-min_reward))/(1-gDiscount-gEpsilon)) + min_reward;
	return rew;

}

/* ====================================================================== */
/* solve_problem */

/* Solves the given problem. */
DdNode* solve_problem(const Problem& problem,
		double gamma, double epsilon) {
	// momo007
	std::cout << "current domain if define the reward function: " << valid_reward_function << std::endl;

	/*
	 * Collect state variables and assign indices to them.
	 */
	std::cout << "start collect action state variales" << std::endl;
	for (ActionList::const_iterator ai = problem.actions().begin();
			ai != problem.actions().end(); ai++) {

		const Action& action = **ai;

		std::set<Atom*> pos;
		std::set<Atom*> neg;
		if(LUGTOTEXT && !check_consistent(action.effect(), &pos, &neg)){
			continue;
		}
		std::cout << "Action: ";
		(*ai)->print(std::cout, problem.terms());
		std::cout << "\nPrecondtion: ";
		// 前提条件涉及到的是state varibale和dynamic variable
		action.precondition().print(std::cout, problem.domain().predicates(), problem.domain().functions(),
									problem.terms());
		std::cout << "\ncollect the var in precondtion:\n";
		collect_state_variables(action.precondition(), false);
		// 装备收集effect的state ariable和dynamic variable
		std::cout << "\nEffect: ";
		action.effect().print(std::cout, problem.domain().predicates(), problem.domain().functions(),
									problem.terms());
		std::cout << "\ncollect the var in effect:\n";
		collect_state_variables(action.effect());
		std::cout << "\n";
		if (&((Action *)(*ai))->observation())
		{
			SENSORS=TRUE;
			ObservationVector& ob =
					(ObservationVector&) ((Observation&)((Action*)(*ai))->observation()).obVector();
			ObservationEntry *e;
			for(ObservationVector::iterator i = ob.begin();i != ob.end(); i++){
				e = ((ObservationEntry*)(*i));
				std::cout << "observation action collect "<< std::endl;
				collect_state_variables(e->formula(), false);
				if(&(e->symbol()) != NULL)
					collect_state_variables(e->symbol(), false);
			}
		}
	}
	std::cout << "start collect init state variales" << std::endl;
	for (EffectList::const_iterator ei = problem.init_effects().begin();
			ei != problem.init_effects().end(); ei++) {
		std::cout << "collect init eff" << std::endl;
		collect_state_variables(**ei);


	}
	if(&problem.init_formula()){
		  std::cout << "collect Init" << std::endl;
		collect_state_variables(problem.init_formula(), true);
	}
	//nvars = state_variables.size(); //dan


	//set Son KR'06
	//computeDescisiveSets();



	nvars = dynamic_atoms.size();
	// std::cout << "check the dynamic_atoms and state vairables\n";
	std::cout << "dynamic atoms:" << dynamic_atoms.size() << std::endl;
	// for (int i = 0; i < dynamic_atoms.size();++i)
	// {
	// 	dynamic_atoms[i]->print(std::cout, problem.domain().predicates(),
	// 							problem.domain().functions(), problem.terms());
	// }	
	std::cout << "state variables:" << state_variables.size() << std::endl;
	// std::map<const Atom *, int>::iterator ite = state_variables.begin();
	// while(ite != state_variables.end())
	// {
	// 	ite->first->print(std::cout, problem.domain().predicates(),
	// 					  		problem.domain().functions(), problem.terms());
	// 	ite++;
	// }
	// assert(dynamic_atoms.size() == state_variables.size());

	// not used here
	if(  my_problem->domain().requirements.rewards && DBN_PROGRESSION){
		std::cout << "add extract state variable for reward and terminal states" << std::endl;
		nvars += 2; // for goal and terminal states
	}


	if (true || verbosity >= 3) {

		std::cout << std::endl << "Number of state variables: " << nvars
				<< std::endl;
		if (true || verbosity >= 3) {
			std::cout << "show dynamic_atoms\n";
			for (std::map<int, const Atom *>::const_iterator vi =
					 dynamic_atoms.begin();
				 vi != dynamic_atoms.end(); vi++)
			{
				std::cout << (*vi).first <<  '\t';                          
				(*vi).second->print(std::cout, problem.domain().predicates(),
						problem.domain().functions(), problem.terms());
				std::cout << std::endl;
			}
			std::cout << "show state_variables\n";
			for (std::map<const Atom*, int>::const_iterator vi =
					state_variables.begin();
					vi != state_variables.end(); vi++) {
				std::cout << (*vi).second <<  '\t';
				(*vi).first->print(std::cout, problem.domain().predicates(),
						problem.domain().functions(), problem.terms());
				std::cout << std::endl;
			}
		}
	}

	/*
	 * Iniiatlize CUDD.
	 */
	std::cout << "==================================\n";
	std::cout << "start to initialize the cudd" << std::endl;
	int num = (2 * nvars) + max_num_aux_vars + (2 * rbpf_bits);
	dd_man = Cudd_Init(num, 0, CUDD_UNIQUE_SLOTS, CUDD_CACHE_SLOTS, 0);
	num_alt_facts = nvars;
	manager = dd_man;



// 	  	 	Cudd_AutodynEnable(manager,
// 	// // 	 			   //CUDD_REORDER_SAME
// 	// //  			   //
// CUDD_REORDER_LINEAR
// 	//   			   
// 					   //					   CUDD_REORDER_SIFT
// 					   //					   CUDD_REORDER_WINDOW2
// 					   //			   CUDD_REORDER_WINDOW3
// 	// //  			   //CUDD_REORDER_ANNEALING
// 	// //  			   //CUDD_REORDER_GROUP_SIFT
// 	   			   );


	/*
	 * Collect column variables and compute their cube.
	 */
	std::cout << "start to create the cube and identity for state variables" << std::endl;
	DdNode** col_variables = new DdNode*[nvars];
	for (int i = 0; i < nvars; i++) {
		col_variables[i] = Cudd_addIthVar(dd_man, 2*i + 1);
		Cudd_Ref(col_variables[i]);
	}
	col_cube = Cudd_addComputeCube(dd_man, col_variables, NULL, nvars);
	Cudd_Ref(col_cube);
	for (int i = 0; i < nvars; i++) {
		Cudd_RecursiveDeref(dd_man, col_variables[i]);
	}
	delete col_variables;

	/*
	 * Construct identity BDDs for state variables.
	 * 构造 x == x'的BDD，用于frame axioms
	 */
	DdNode** row_vars = new DdNode*[nvars];
	DdNode** col_vars = new DdNode*[nvars];
	for (int i = 0; i < nvars; i++) {
		DdNode* x = Cudd_bddIthVar(dd_man, 2*i);
		DdNode* y = Cudd_bddIthVar(dd_man, 2*i + 1);
		identity_bdds.push_back(Cudd_Xeqy(dd_man, 1, &x, &y));// x==x'
		Cudd_Ref(identity_bdds.back());// 存储等价pair
		row_vars[i] = x;
		col_vars[i] = y;
	}
	// 构造当前和后继状态变量等价 x[1,2,...] == x[1,2,3,...]'
	identity_bdd = Cudd_Xeqy(dd_man, nvars, row_vars, col_vars);
	Cudd_Ref(identity_bdd);
	delete row_vars;
	delete col_vars;
	std::cout << "done to create the cube and identity for state variables" << std::endl;

	/**
	 * Construct a BDDs representing goal states.
	 * momo007 2022.05.26 rewrite only support goal without reward
	 */
	DdNode* ddg;

	std::cout << "construct the bdd for goal formula" << std::endl;
	ddg = formula_bdd(problem.goal());
	std::cout << "done with goal" << std::endl;
	/**
	 * 设置goal BDD to b_goal_state
	 */
	b_goal_state = ddg;
	Cudd_Ref(b_goal_state);

	if (verbosity >= 3) {

		std::cout << std::endl << "Goal state BDD:" << std::endl;
		Cudd_PrintDebug(dd_man, ddg, 2*nvars, 2);
	}
	int* row_to_col = new int[2*nvars];
	for (int i = 0; i < nvars; i++) {
		row_to_col[2*i] = 2*i + 1;
		row_to_col[2*i + 1] = 2*i + 1;
	}
	DdNode* ddgp = Cudd_bddPermute(dd_man, ddg, row_to_col);
	Cudd_Ref(ddgp);
	delete row_to_col;



	//Set up variable mappings
	varmap = new int[2*nvars];
	for(int i = 0; i < nvars; i++){
		varmap[2*i] = 2*i+1;
		varmap[2*i+1] = 2*i;
	}

	std::cout << "setting action preconds" <<std::endl;

	/*
	 * Construct transition probability and reward MTBDDs for actions.
	 */
	DdNode* ddng = Cudd_Not(ddg);
	Cudd_Ref(ddng);
	Cudd_RecursiveDeref(dd_man, ddg);

	for (ActionList::const_iterator ai = problem.actions().begin();
			ai != problem.actions().end(); ai++) {


		const Action& action = **ai;
		action.print(std::cout, problem.terms());
		std::cout << "\n";

		action_preconds.insert(std::make_pair<const Action*, DdNode*>(&action,
				formula_bdd(action.precondition())));
		printBDD(action_preconds[&action]);
		if(LUGTOTEXT)
			groundActionDD(action);//, problem, ddgp, ddng, col_cube);

	}

	std::cout << "done constructing action preconditon BDD" << std::endl;

	set_cubes();
	collectInit(&problem);
	std::cout << "done constructing the init state BDD" << std::endl;

	Cudd_RecursiveDeref(dd_man, ddgp);
	if (verbosity >= 3) {

		for (ActionList::const_iterator ai = problem.actions().begin();
				ai != problem.actions().end(); ai++) {
			const Action& action = **ai;
			std::cout << "Grounding Action: " << action.name() << std::endl;
			groundActionDD(action);
		}
		for (std::map<const Action*, DdNode*>::iterator ai =
				action_transitions.begin();
				ai != action_transitions.end(); ai++) {
			std::cout << std::endl << "Transition probability matrix for ";
			(*ai).first->print(std::cout, problem.terms());
			std::cout << ':' << std::endl;
			Cudd_PrintDebug(dd_man, (*ai).second, 2*nvars, 1);
#if 0
			matrix_to_dbn((*ai).second);
#endif
			std::cout << "Reward vector for ";
			(*ai).first->print(std::cout, problem.terms());
			std::cout << ':' << std::endl;
			// Cudd_PrintDebug(dd_man, action_rewards[(*ai).first], 2*nvars, 1);
		}
	}



	DdNode* ddP;// = value_iteration(problem, ddng, col_cube, gamma, epsilon);
	return ddP;
}


/* ====================================================================== */
/* MBDDPlanner */

/* Deletes this MTBDD planner. */
MTBDDPlanner::~MTBDDPlanner() {
	if (mapping_ != NULL) {
		Cudd_RecursiveDeref(dd_man_, mapping_);
		Cudd_DebugCheck(dd_man_);
		int unrel = Cudd_CheckZeroRef(dd_man_);
		if (unrel != 0) {
			std::cerr << unrel << " unreleased DDs" << std::endl;
		}
		Cudd_Quit(dd_man_);
	}
}  


void MTBDDPlanner::initRound() {
	if (mapping_ == NULL) {
		mapping_ = solve_problem(_problem, gamma_, epsilon_);
		dd_man_ = dd_man;
		actions_ = policy_actions;
		policy_actions.clear();
		dynamic_atoms_ = dynamic_atoms;
		dynamic_atoms.clear();
	}
}


const Action* MTBDDPlanner::decideAction(const State& state) {
	DdNode* dds = state_bdd(dd_man_, dynamic_atoms_, state.atoms());
	DdNode* ddS = Cudd_BddToAdd(dd_man_, dds);
	Cudd_Ref(ddS);
	Cudd_RecursiveDeref(dd_man_, dds);
	DdNode* dda = Cudd_addEvalConst(dd_man_, ddS, mapping_);
	Cudd_Ref(dda);
	Cudd_RecursiveDeref(dd_man_, ddS);
	size_t id = int(Cudd_V(dda) + 0.5);
	Cudd_RecursiveDeref(dd_man_, dda);
	std::map<size_t, const Action*>::const_iterator ai = actions_.find(id);
	return (ai != actions_.end()) ? (*ai).second : NULL;
}


void MTBDDPlanner::endRound() {
}

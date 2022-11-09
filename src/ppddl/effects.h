/* -*-C++-*- */
/*
 * Effects.
 *
 * Copyright (C) 2003 Carnegie Mellon University and Rutgers University
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
 * $Id: effects.h,v 1.1 2006/07/11 17:53:06 dan Exp $
 */
#ifndef EFFECTS_H
#define EFFECTS_H

#include <config.h>
#include "terms.h"
#include <iostream>
#include <vector>

struct Rational;
struct PredicateTable;
struct FunctionTable;
struct ValueMap;
struct Expression;
struct Application;
struct StateFormula;
struct Atom;
struct AtomSet;
struct AtomList;
struct Problem;

extern int PRINT_ND;

/* ====================================================================== */
/* Assignment */

/*
 * An assignment.
 */
struct Assignment {
  /* Assignment operators. 
      包括多种基础操作：赋值、倍增/乘法、倍减/除法、加法、减法
  */
  typedef enum { ASSIGN_OP, SCALE_UP_OP, SCALE_DOWN_OP,
		 INCREASE_OP, DECREASE_OP } AssignOp;

  /* Constructs an assignment. 
    通过封装一个Application(实现计算结果)、Expression(待计算的值)、还有操作符号
  */
  Assignment(AssignOp oper, const Application& application,
	     const Expression& expr);

  /* Deletes this assignment. */
  ~Assignment();

  /* Returns the application of this assignment. */
  const Application& application() const { return *application_; }

  /* Returns the expression of this assignment. */
  const Expression& expression() const { return *expr_; }

  /* Changes the given state according to this assignment. 
    根据Map的值的情况，计算Assignment的结果
  */
  void affect(ValueMap& values) const;

  /* Returns an instantiaion of this assignment. */
  const Assignment& instantiation(const SubstitutionMap& subst,
				  const Problem& problem) const;

  /* Prints this object on the given stream. */
  void print(std::ostream& os, const FunctionTable& functions,
	     const TermTable& terms) const;
  virtual bool operator == (const Assignment&) const;
 virtual bool operator < (const Assignment&) const;
 
private:
  /* Assignment operator. */
  AssignOp operator_;
  /* Application affected by this assignment. */
  const Application* application_;
  /* Expression. */
  const Expression* expr_;
};


/* ====================================================================== */
/* AssignmentList */

/*
 * List of assignments.
 */
struct AssignmentList : public std::vector<const Assignment*> {
};


/* ====================================================================== */
/* Effect */

/*
 * An effect.
 */
struct pEffect {
  /* Register use of the given effect. */
  static void register_use(const pEffect* e) {
    if (e != NULL) {
      e->ref_count_++;
    }
  }

  /* Unregister use of the given effect. */
  static void unregister_use(const pEffect* e) {
    if (e != NULL) {
      e->ref_count_--;
      if (e->ref_count_ == 0) {
	delete e;
      }
    }
  }

  /* Deletes this effect. */
  virtual ~pEffect() {}

  /* Fills the provided lists with a sampled state change for this
     effect in the given state. 
     给出了一个virtual接口，后续继承的子类这块使用不同的实现。
     对于相应的add或del方法，填充相应的List
  */
  virtual void state_change(AtomList& adds, AtomList& deletes,
			    AssignmentList& assignments,
			    const AtomSet& atoms,
			    const ValueMap& values) const = 0;

  /* Returns an instantiation of this effect. */
  virtual const pEffect& instantiation(const SubstitutionMap& subst,
				      const Problem& problem) const = 0;

  /* Prints this object on the given stream. */
  virtual void print(std::ostream& os, const PredicateTable& predicates,
		     const FunctionTable& functions,
		     const TermTable& terms) const = 0;

  virtual void getMinMaxRewards(double *min_reward, double *max_reward, int* num_aux) const {};

  virtual bool operator == (const pEffect&) const {};
  virtual bool operator < (const pEffect&) const {};
  

protected:
  /* Constructs an effect. */
  pEffect() : ref_count_(0) {}

private:
  /* Reference counter. */
  mutable size_t ref_count_;
};


/* ====================================================================== */
/* EffectList */

/*
 * List of effects.
 */
struct EffectList : public std::vector<const pEffect*> {
};


/* ====================================================================== */
/* SimpleEffect */

/*
 * A simple effect.
  继承该类包括AddEffect和DeleteEffect
 */
struct SimpleEffect : public pEffect {
  /* Deletes this simple effect. */
  virtual ~SimpleEffect();

  /* Returns the atom associated with this simple effect. */
  const Atom& atom() const { return *atom_; }


  virtual bool operator == (const pEffect&) const;
  virtual bool operator < (const pEffect&) const;

protected:
  /* Constructs a simple effect. */
  explicit SimpleEffect(const Atom& atom);

private:
  /* Atom added by this effect. */
  const Atom* atom_;
};


/* ====================================================================== */
/* AddEffect */

/*
 * An add effect.
 */
struct AddEffect : public SimpleEffect {
  /* Constructs an add effect. */
  explicit AddEffect(const Atom& atom) : SimpleEffect(atom) {}

  /* Fills the provided lists with a sampled state change for this
     effect in the given state. 
    实现：AddList中添加AtomSet
  */
  virtual void state_change(AtomList& adds, AtomList& deletes,
			    AssignmentList& assignments,
			    const AtomSet& atoms,
			    const ValueMap& values) const;

  /* Returns an instantiation of this effect. */
  virtual const pEffect& instantiation(const SubstitutionMap& subst,
				      const Problem& problem) const;

  /* Prints this object on the given stream. */
  virtual void print(std::ostream& os, const PredicateTable& predicates,
		     const FunctionTable& functions,
		     const TermTable& terms) const;

  virtual void getMinMaxRewards(double *min_reward, double *max_reward, int* num_aux) const;
virtual bool operator == (const pEffect&) const;
virtual bool operator < (const pEffect&) const;

};


/* ====================================================================== */
/* DeleteEffect */

/*
 * A delete effect.
 */
struct DeleteEffect : public SimpleEffect {
  /* Constructs a delete effect. */
  explicit DeleteEffect(const Atom& atom) : SimpleEffect(atom) {}

  /* Fills the provided lists with a sampled state change for this
     effect in the given state. */
  virtual void state_change(AtomList& adds, AtomList& deletes,
			    AssignmentList& assignments,
			    const AtomSet& atoms,
			    const ValueMap& values) const;

  /* Returns an instantiation of this effect. */
  virtual const pEffect& instantiation(const SubstitutionMap& subst,
				      const Problem& problem) const;

  /* Prints this object on the given stream. */
  virtual void print(std::ostream& os, const PredicateTable& predicates,
		     const FunctionTable& functions,
		     const TermTable& terms) const;
  virtual void getMinMaxRewards(double *min_reward, double *max_reward, int* num_aux) const;
virtual bool operator == (const pEffect&) const;
virtual bool operator<(const pEffect&) const;

};


/* ====================================================================== */
/* AssignmentEffect */

/*
 * An assignment effect.
  底层存储一个Assignment，然后通过利用该Assignment进行Effect的效果
 */
struct AssignmentEffect : public pEffect {
  /* Constructs an assignment effect. */
  AssignmentEffect(const Assignment& assignment);

  /* Deletes this assignment effect. */
  virtual ~AssignmentEffect();

  /* Returns the assignment performed by this effect. */
  const Assignment& assignment() const { return *assignment_; }

  /* Fills the provided lists with a sampled state change for this
     effect in the given state. */
  virtual void state_change(AtomList& adds, AtomList& deletes,
			    AssignmentList& assignments,
			    const AtomSet& atoms,
			    const ValueMap& values) const;

  /* Returns an instantiation of this effect. */
  virtual const pEffect& instantiation(const SubstitutionMap& subst,
				      const Problem& problem) const;

  /* Prints this object on the given stream. */
  virtual void print(std::ostream& os, const PredicateTable& predicates,
		     const FunctionTable& functions,
		     const TermTable& terms) const;

  virtual void getMinMaxRewards(double *min_reward, double *max_reward, int* num_aux) const;
  virtual bool operator == (const pEffect&) const;
virtual bool operator<(const pEffect&) const;
private:
  /* Assignment performed by this effect. */
  const Assignment* assignment_;
};


/* ====================================================================== */
/* ConjunctiveEffect */

/*
 * A conjunctive effect.
  实际上就是利用EffectList封装了合取Effect的效果
 */
struct ConjunctiveEffect : public pEffect {
  /* Deletes this conjunctive effect. */
  virtual ~ConjunctiveEffect();

  /* Adds a conjunct to this conjunctive effect. */
  void add_conjunct(const pEffect& conjunct);

  /* Returns the number of conjuncts of this conjunctive effect. */
  size_t size() const { return conjuncts_.size(); }

  /* Returns the ith conjunct of this conjunctive effect.
    返回一个conjunct中的第i个部分
  */
  const pEffect& conjunct(size_t i) const { return *conjuncts_[i]; }

  /* Fills the provided lists with a sampled state change for this
     effect in the given state. */
  virtual void state_change(AtomList& adds, AtomList& deletes,
			    AssignmentList& assignments,
			    const AtomSet& atoms,
			    const ValueMap& values) const;

  /* Returns an instantiation of this effect. */
  virtual const pEffect& instantiation(const SubstitutionMap& subst,
				      const Problem& problem) const;

  /* Prints this object on the given stream. */
  virtual void print(std::ostream& os, const PredicateTable& predicates,
		     const FunctionTable& functions,
		     const TermTable& terms) const;
  virtual void getMinMaxRewards(double *min_reward, double *max_reward, int* num_aux) const;

  virtual bool operator == (const pEffect&) const;
virtual bool operator<(const pEffect&) const;
private:
  /* The conjuncts. */
  EffectList conjuncts_;//一系列Effects的合取
};


/* ====================================================================== */
/* ConditionalEffect */

/*
 * A conditional effect.
 类似PDDL中effect附加when的类型
  封装一个formula表示when的前提，effect实现结果
 */
struct ConditionalEffect : public pEffect {
  /* Returns a conditional effect. */
  static const pEffect& make(const StateFormula& condition,
			    const pEffect& effect);

  /* Deletes this conditional effect. */
  virtual ~ConditionalEffect();

  /* Returns the condition of this effect. */
  const StateFormula& condition() const { return *condition_; }

  /* Returns the conditional effect of this effect. */
  const pEffect& effect() const { return *effect_; }

  /* Fills the provided lists with a sampled state change for this
     effect in the given state. */
  virtual void state_change(AtomList& adds, AtomList& deletes,
			    AssignmentList& assignments,
			    const AtomSet& atoms,
			    const ValueMap& values) const;

  /* Returns an instantiation of this effect. */
  virtual const pEffect& instantiation(const SubstitutionMap& subst,
				      const Problem& problem) const;

  /* Prints this object on the given stream. */
  virtual void print(std::ostream& os, const PredicateTable& predicates,
		     const FunctionTable& functions,
		     const TermTable& terms) const;
  virtual void getMinMaxRewards(double *min_reward, double *max_reward, int* num_aux) const;

  virtual bool operator == (const pEffect&) const;
virtual bool operator<(const pEffect&) const;
private:
  /* pEffect condition. */
  const StateFormula* condition_;
  /* pEffect. */
  const pEffect* effect_;

  /* Constructs a conditional effect. */
  ConditionalEffect(const StateFormula& condition, const pEffect& effect);
};


/* ====================================================================== */
/* ProbabilisticEffect */

/*
 * A probabilistic effect.

 */
struct ProbabilisticEffect : public pEffect {
  /* Constructs an empty probabilistic effect. */
  ProbabilisticEffect() : weight_sum_(0), isObservation_(false) {}

  /* Deletes this probabilistic effect. */
  virtual ~ProbabilisticEffect();

  /* Adds an outcome to this probabilistic effect. */
  bool add_outcome(const Rational& p, const pEffect& effect);
  /* Adds an outcome to this probabilistic effect. */ //DAN
  bool add_foutcome(const Expression& p, const pEffect& effect);
  

  /* Returns the number of outcomes of this probabilistic effect. */
  size_t size() const { return weights_.size(); }
  /* Returns the number of outcomes of this probabilistic effect. */
  size_t fsize() const { return fweights_.size(); }

  /* Returns the ith outcome's probability. */
  Rational probability(size_t i) const;
 
 /* Returns the ith outcome's probability. */
  const Expression* fprobability(size_t i) const;

  /* Returns the ith outcome's effect. */
  const pEffect& effect(size_t i) const { return *effects_[i]; }

  /* Fills the provided lists with a sampled state change for this
     effect in the given state. */
  virtual void state_change(AtomList& adds, AtomList& deletes,
			    AssignmentList& assignments,
			    const AtomSet& atoms,
			    const ValueMap& values) const;

  /* Returns an instantiation of this effect. */
  virtual const pEffect& instantiation(const SubstitutionMap& subst,
				      const Problem& problem) const;

  /* Prints this object on the given stream. */
  virtual void print(std::ostream& os, const PredicateTable& predicates,
		     const FunctionTable& functions,
		     const TermTable& terms) const;
  virtual void getMinMaxRewards(double *min_reward, double *max_reward, int* num_aux) const;
  //dan
  void setProbabilityFromExpressions(const Problem* problem);
   bool isObservation() const {return isObservation_;}
  void setObservation() {isObservation_=true;}
  //dan
//   std::vector<int>* weights() {return &weights_;}
//   int* weight_sum() const {return &weight_sum_;}
  virtual bool operator == (const pEffect&) const;
  virtual bool operator<(const pEffect&) const;
   private://DAN
  //protected:  //DAN -- So ND effects can inherit this junk
  /* Weights associated with outcomes. 
    这面这块的ifelse的宏定义用于不同情况下的概率effect的权重情况
    包括：int、double、表示计算等
  */
#ifdef FRACT_RATIONALS  
  std::vector<int> weights_;
#else
  std::vector<double> weights_;
#endif
  std::vector<const Expression*> fweights_;
  /* The sum of weights. */
  int weight_sum_;
  /* Outcome effects. */
  EffectList effects_;
  bool isObservation_;
};




/* ====================================================================== */
/* QuantifiedEffect */

/*
 * A universally quantified effect.
 */
struct QuantifiedEffect : public pEffect {
  /* Constructs a universally quantified effect. */
  explicit QuantifiedEffect(const pEffect& effect);

  /* Deletes this universally quantifed effect. */
  virtual ~QuantifiedEffect();

  /* Adds a quantified variable to this universally quantified effect. */
  void add_parameter(Variable parameter) { parameters_.push_back(parameter); }

  /* Returns the number quantified variables of this universally
     quantified effect. */
  size_t arity() const { return parameters_.size(); }

  /* Returns the ith parameter of this universally quantified effect. */
  Variable parameter(size_t i) const { return parameters_[i]; }

  /* Returns the quantified effect. */
  const pEffect& effect() const { return *effect_; }

  /* Fills the provided lists with a sampled state change for this
     effect in the given state. */
  virtual void state_change(AtomList& adds, AtomList& deletes,
			    AssignmentList& assignments,
			    const AtomSet& atoms,
			    const ValueMap& values) const;

  /* Returns an instantiation of this effect. */
  virtual const pEffect& instantiation(const SubstitutionMap& subst,
				      const Problem& problem) const;

  virtual void getMinMaxRewards(double *min_reward, double *max_reward, int* num_aux) const;

  /* Prints this object on the given stream. */
  virtual void print(std::ostream& os, const PredicateTable& predicates,
		     const FunctionTable& functions,
		     const TermTable& terms) const;
  virtual bool operator == (const pEffect&) const;
  virtual bool operator<(const pEffect&) const;
private:
  /* Quantified variables. */
  VariableList parameters_;
  /* The quantified effect. */
  const pEffect* effect_;
};


#endif /* EFFECTS_H */

#ifndef OBSERVATIONS_H
#define OBSERVATIONS_H

#include <vector>
#include <set>
#include "predicates.h"
#include "formulas.h"
#include "rational.h"
#include "effects.h"

struct Observation;//内部使用vector存储多个ObservationEntry
struct ObservationEntry;

typedef std::vector<const struct ObservationEntry*> ObservationVector;
typedef std::vector<const struct ObservationCptRow*> ObservationCptRowVector;
/**
 * Observation，添加全局所有的observations
 */
struct Observation {
  Observation() {};

 /* Register use of the given observation. */
  static void register_use(const Observation* e) {
    if (e != NULL) {
      e->ref_count_++;
    }   
  }

  /* Unregister use of the given observation. */
  static void unregister_use(const Observation* e) {
    if (e != NULL) {
      e->ref_count_--;
      if (e->ref_count_ == 0) {
	  delete e;
      }
    }
  }
  
  void add_entry(const ObservationEntry* ent){ observations_.push_back(ent);}

  const ObservationVector& obVector() {return observations_;}

  const Observation& instantiation(const SubstitutionMap& subst,
				   const Problem& problem) const ;
  
  private:
  /* Reference counter. */
  mutable size_t ref_count_;
  ObservationVector observations_; 
};
/**
 *这里的observation Entity都是基于概率的实现，如何实现一致性规划的方法
  momo007 006 2022.03.29
 **/
struct ObservationEntry{
  /*创建公式及相应的正负概率*/
  ObservationEntry(const StateFormula& f, const Rational& r, const Rational& r1) :
    formula_(&f), symbol_(NULL), posProbability_(r), negProbability_(r1), obs_(NULL) {};
  /*创建公式及概率Effect*/
  ObservationEntry(const StateFormula& f, const ProbabilisticEffect& a) :
    formula_(&f), symbol_(NULL), obs_(&a) {};
  /*创建两个实数间的操作*/
  ObservationEntry(const StateFormula& f, const StateFormula* o, 
		   const Rational& r, const Rational& r1) :
    formula_(&f), symbol_(o),  posProbability_(r), negProbability_(r1), obs_(NULL) {};
  /*Entity的实例化*/
  const ObservationEntry& instantiation(const SubstitutionMap& subst,
					const Problem& problem) const ;
 
  const StateFormula& symbol() {return *symbol_;}
  const StateFormula& formula() {return *formula_;}
  const Rational& posProbability() {return posProbability_;}
  const Rational& negProbability() {return negProbability_;}
  const ProbabilisticEffect& obs() {return *obs_;}

  private:
  const StateFormula* symbol_;
  const StateFormula* formula_;
  const Rational posProbability_;
  const Rational negProbability_;
  // list<const Atom*>* atoms_;
  const ProbabilisticEffect* obs_;
};

struct ObservationCptRow{
  ObservationCptRow(const StateFormula& form, const ProbabilisticEffect& eff) :
    formula_(&form), effect_(&eff){}
  /* Prints this action schema on the given stream. */
  void print(std::ostream& os, const PredicateTable& predicates,
	     const FunctionTable& functions, const TermTable& terms) const;
  const ObservationCptRow& instantiation(const SubstitutionMap& subst,
				   const Problem& problem) const ;
  //包含一个概率effect和一个状态formual
  const StateFormula* formula_;
  const ProbabilisticEffect* effect_;
};

// 对ObservationRow进行进行封装
struct ObservationCpt{
  ObservationCpt(){}

  /* Prints this action schema on the given stream. */
  void print(std::ostream& os, const PredicateTable& predicates,
	     const FunctionTable& functions, const TermTable& terms) const;
  void add_cpt_entry(const ObservationCptRow* ent){ observations_.push_back(ent);}
  const ObservationCpt& instantiation(const SubstitutionMap& subst,
				   const Problem& problem) const ;
  ObservationCptRowVector observations_;
};



#endif

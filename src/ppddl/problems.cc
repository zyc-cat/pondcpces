/*
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
 * $Id: problems.cc,v 1.3 2006/11/09 07:43:43 dan Exp $
 */
#include "problems.h"
#include "domains.h"
#include "exceptions.h"
#include <typeinfo>


/* ====================================================================== */
/* Problem */

/* Table of defined problems. */
Problem::ProblemMap Problem::problems = Problem::ProblemMap();


/* Returns a const_iterator pointing to the first problem. */
Problem::ProblemMap::const_iterator Problem::begin() {
  return problems.begin();
}


/* Returns a const_iterator pointing beyond the last problem. */
Problem::ProblemMap::const_iterator Problem::end() {
  return problems.end();
}


/* Returns the problem with the given name, or NULL if it is undefined. */
const Problem* Problem::find(const std::string& name) {
  ProblemMap::const_iterator pi = problems.find(name);
  return (pi != problems.end()) ? (*pi).second : NULL;
}


/* Removes all defined problems. */
void Problem::clear() {
  ProblemMap::const_iterator pi = begin();
  while (pi != end()) {
    delete (*pi).second;
    pi = begin();
  }
  problems.clear();
}


/**
 * Constructs a problem. 
*/
Problem::Problem(const std::string& name, const Domain& domain)
  : name_(name), domain_(&domain), terms_(TermTable(domain.terms())),
    goal_(&StateFormula::FALSE), goal_reward_(NULL), metric_(new Value(0.0)),
    horizon_(-1.0), tau_(0.0), discount_(1.0), init_formula_(NULL) {
  StateFormula::register_use(goal_);
  Expression::register_use(metric_);
  const Problem* p = find(name);
  if (p != NULL) {
    delete p;
  }
  problems[name] = this;
}


/* Deletes a problem. */
Problem::~Problem() {
  problems.erase(name());
  for (AtomSet::const_iterator ai = init_atoms_.begin();
       ai != init_atoms_.end(); ai++) {
    StateFormula::unregister_use(*ai);
  }
  for (ValueMap::const_iterator vi = init_values_.begin();
       vi != init_values_.end(); vi++) {
    Expression::unregister_use((*vi).first);
  }
  for (EffectList::const_iterator ei = init_effects_.begin();
       ei != init_effects_.end(); ei++) {
    delete *ei;
  }
  StateFormula::unregister_use(goal_);
  if (goal_reward_ != NULL) {
    delete goal_reward_;
  }
  Expression::unregister_use(metric_);
}


/* Adds an atomic state formula to the initial conditions of this
   problem. */
void Problem::add_init_atom(const Atom& atom) {
  if (init_atoms_.find(&atom) == init_atoms_.end()) {
    init_atoms_.insert(&atom);
    StateFormula::register_use(&atom);
  }
}


/* Adds a function application value to the initial conditions of
   this problem. */
void Problem::add_init_value(const Application& application,
			     const Rational& value) {
  if (init_values_.find(&application) == init_values_.end()) {
    init_values_.insert(std::make_pair(&application, value));
    Expression::register_use(&application);
  } else {
    init_values_[&application] = value;
  }
}


/* Adds an initial effect for this problem. */
void Problem::add_init_effect(const pEffect& effect) {
  init_effects_.push_back(&effect);
}

/* Sets an initial formuala for this problem. */
void Problem::set_init_formula(const StateFormula& init){
  init_formula_ = &init;
}

/* Sets the goal for this problem. */
void Problem::set_goal(const StateFormula& goal, bool problem_goal) {
  if (&goal != goal_) {
    const StateFormula* tmp = goal_;
    goal_ = &goal;
    StateFormula::register_use(goal_);
    StateFormula::unregister_use(tmp);
    if(problem_goal)
      set_goal_cnf();//DAN
  }
}

/* Sets the goal for this problem. */
void Problem::set_goal(const StateFormula& goal, const Rational& tau) {
  tau_ = tau.double_value();
  set_goal(goal, true);
}


/* Sets the goal reward for this problem. */
void Problem::set_goal_reward(const Assignment& goal_reward) {
  if (&goal_reward != goal_reward_) {
    delete goal_reward_;
    goal_reward_ = &goal_reward;
  }
}


/* Sets the metric to maximize for this problem. */
void Problem::set_metric(const Expression& metric, bool negate) {
  const Expression* real_metric;
  if (negate) {
    real_metric = new Subtraction(*new Value(0.0), metric);
  } else {
    real_metric = &metric;
  }
  if (real_metric != metric_) {
    const Expression* tmp = metric_;
    metric_ = real_metric;
    Expression::register_use(metric_);
    Expression::unregister_use(tmp);
  }
}


/* Instantiates all actions. */
void Problem::instantiate_actions() {
  set_goal(goal().instantiation(SubstitutionMap(), *this), false);
  if (goal_reward() != NULL) {
    set_goal_reward(goal_reward()->instantiation(SubstitutionMap(), *this));
  }
  set_metric(metric().instantiation(SubstitutionMap(), *this));
  domain().instantiated_actions(actions_, *this);
}
/* Instantiates all events. */
void Problem::instantiate_events() {
  set_goal(goal().instantiation(SubstitutionMap(), *this), false);
  if (goal_reward() != NULL) {
    set_goal_reward(goal_reward()->instantiation(SubstitutionMap(), *this));
  }
  set_metric(metric().instantiation(SubstitutionMap(), *this));
  domain().instantiated_events(events_, *this);
}


/* Tests if the metric is constant. */
bool Problem::constant_metric() const {
  return typeid(*metric_) == typeid(Value);
}


/* Fills the provided object list with objects (including constants
   declared in the domain) that are compatible with the given
   type. */
void Problem::compatible_objects(ObjectList& objects, Type type) const {
  domain().compatible_constants(objects, type);
  Object last = terms().last_object();
  for (Object i = terms().first_object(); i <= last; i++) {
    if (domain().types().subtype(terms().type(i), type)) {
      objects.push_back(i);
    }
  }
}


/* Fills the given list with actions enabled in the given state. */
void Problem::enabled_actions(ActionList& actions, const AtomSet& atoms,
			      const ValueMap& values) const {
  for (ActionList::const_iterator ai = actions_.begin();
       ai != actions_.end(); ai++) {
    if ((*ai)->enabled(atoms, values)) {//判断action的前体是否满足，内部调用pre公式的hold函数
      actions.push_back(*ai);
    }
  }
}


bool is_cnf_negand_unnested( const Negation* nf){
  const Atom* af1 = dynamic_cast<const Atom*>(&(nf->negand()));
  if(af1 != NULL){// atom说明不是其他conjunction或disjunction等复合结构
    return true;
  }
  return false;

}
/**
 * 考虑CNF的conjunction是一个DNF的情况
 */
bool is_cnf_disjunct_unnested(const Disjunction* df){
  for (size_t i = 0; i < df->size(); i++) {
    // 1. 如果是atom则满足，continue
    const Atom* af1 = dynamic_cast<const Atom*>(&(df->disjunct(i)));
    if(af1 != NULL){
      continue;
    }
    // 2. 如果是negation，需要底层判断是否为negand
    const Negation* nf = dynamic_cast<const Negation*>(&(df->disjunct(i)));
    if (nf != NULL) {
      if(!is_cnf_negand_unnested(nf))
	      return false;
      else
	      continue;
    }
    // 3. 其他情况：CNF/DNF/OneOf/Exist/Forall，返回false
    const Conjunction* cf = dynamic_cast<const Conjunction*>(&(df->disjunct(i)));
    if (cf != NULL) {
      return false;
    }
    const Disjunction* df = dynamic_cast<const Disjunction*>(&(df->disjunct(i)));
    if (df != NULL) {
      return false;
    }
    const OneOfDisjunction* odf = dynamic_cast<const OneOfDisjunction*>(&(df->disjunct(i)));
    if (odf != NULL) {
      return false;
    }
    const Exists* ef = dynamic_cast<const Exists*>(&(df->disjunct(i)));
    if (ef != NULL) {
      return false;
    }
    const Forall* faf = dynamic_cast<const Forall*>(&(df->disjunct(i)));
    if (faf != NULL) {
      return false;
    }

  }
  return true;
}
/**
 * 判断CNF的过程，转化为各种情况判断，
 * 其中处理OneOfDisjunction和Disjunction一致，有bug
 * momo007 008 2022.04.05 11.03
 */
void Problem::set_goal_cnf(){ 
    bool is_cnf = true;

    const Negation* nf = dynamic_cast<const Negation*>(goal_);
    if (nf != NULL) {
      if(!is_cnf_negand_unnested(nf))//判断内层是否为CNF符合结构
	      is_cnf = false;
    }
    const Conjunction* cf = dynamic_cast<const Conjunction*>(goal_);
    if (cf != NULL) {
      for (size_t i = 0; i < cf->size() && is_cnf; i++) {// 需要考虑每个clause
	//    std::cout << "goal cnf"<<std::endl;
        // 1. 是negation，需要判定是否为negand
        const Negation* nf = dynamic_cast<const Negation*>(&(cf->conjunct(i)));
        if (nf != NULL && is_cnf) {
          if(!is_cnf_negand_unnested(nf))
            is_cnf = false;
        }
        // 2. 是dijunction即clause，判断每个部件都是negation或atom
        const Disjunction* df = dynamic_cast<const Disjunction*>(&(cf->conjunct(i)));
        if (df != NULL && is_cnf) {
          if(!is_cnf_disjunct_unnested(df))
            is_cnf = false;
        }
        // 3. 这里再一次印证了该代码将OneOfDisjunction实现为Disjunction
        const OneOfDisjunction* odf = dynamic_cast<const OneOfDisjunction*>(&(cf->conjunct(i)));
        if (odf != NULL && is_cnf) {
          if(!is_cnf_disjunct_unnested((Disjunction*)odf))
            is_cnf = false;
        }
        // 4. 其他情况，返回false
        const Exists* ef = dynamic_cast<const Exists*>(&(cf->conjunct(i)));
        if (ef != NULL && is_cnf) {
            is_cnf = false;
        }
        const Forall* faf = dynamic_cast<const Forall*>(&(cf->conjunct(i)));
        if (faf != NULL && is_cnf) {
            is_cnf = false;
        }
      }// end-for
    }//end-if
    // CNF是一个clause
    const Disjunction* df = dynamic_cast<const Disjunction*>(goal_);
    if (df != NULL) {
      is_cnf = is_cnf_disjunct_unnested(df);
    }
    // CNF是一个clause
    const OneOfDisjunction* odf = dynamic_cast<const OneOfDisjunction*>(goal_);
    if (odf != NULL) {
      is_cnf = is_cnf_disjunct_unnested((Disjunction*)odf);      
    }
    // 对于其他情况，返回false
    const Exists* ef = dynamic_cast<const Exists*>(goal_);
    if (ef != NULL) {
      is_cnf = false;
    }
	  const Forall* faf = dynamic_cast<const Forall*>(goal_);
    if (faf != NULL) {
        is_cnf = false;
    }
    goal_cnf_=is_cnf;
}


/* Output operator for problems. */
std::ostream& operator<<(std::ostream& os, const Problem& p) {
  os << "name: " << p.name();
  os << std::endl << "domain: " << p.domain().name();
  os << std::endl << "objects:";
  for (Object i = p.terms().first_object();
       i <= p.terms().last_object(); i++) {
    os << std::endl << "  ";
    p.terms().print_term(os, i);
    os << " - ";
    p.domain().types().print_type(os, p.terms().type(i));
  }
  os << std::endl << "init:";
  for (AtomSet::const_iterator ai = p.init_atoms_.begin();
       ai != p.init_atoms_.end(); ai++) {
    os << std::endl << "  ";
    (*ai)->print(os, p.domain().predicates(), p.domain().functions(),
		 p.terms());
  }
  for (ValueMap::const_iterator vi = p.init_values_.begin();
       vi != p.init_values_.end(); vi++) {
    os << std::endl << "  (= ";
    (*vi).first->print(os, p.domain().functions(), p.terms());
    os << ' ' << (*vi).second << ")";
  }
  for (EffectList::const_iterator ei = p.init_effects_.begin();
       ei != p.init_effects_.end(); ei++) {
    os << std::endl << "  ";
    (*ei)->print(os, p.domain().predicates(), p.domain().functions(),
		 p.terms());
  }
  os << std::endl << "goal: ";
  p.goal().print(os, p.domain().predicates(), p.domain().functions(),
		 p.terms());
  if (p.goal_reward() != NULL) {
    os << std::endl << "goal reward: ";
    p.goal_reward()->expression().print(os, p.domain().functions(), p.terms());
  }
  os << std::endl << "metric: ";
  p.metric().print(os, p.domain().functions(), p.terms());
  os << std::endl << "actions:";
  for (ActionList::const_iterator ai = p.actions_.begin();
       ai != p.actions_.end(); ai++) {
    os << std::endl << "  ";
    (*ai)->print(os, p.terms());
  }
  return os;
}

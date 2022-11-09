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
 * $Id: states.cc,v 1.1 2006/07/11 17:53:06 dan Exp $
 */
#include "states.h"
#include "problems.h"
#include "domains.h"


/* Verbosity level. */
extern int verbosity;


/* ====================================================================== */
/* State */

/* Constructs an initial state for the given problem.
  momo007 010 2022.04.05 14:32
*/
State::State(const Problem& problem)
  : problem_(&problem) {
  for (AtomSet::const_iterator ai = problem.init_atoms().begin();
       ai != problem.init_atoms().end(); ai++) {
         // 考虑非static predicate,静态说明不会改变不需要记录
    if (!problem.domain().predicates().static_predicate((*ai)->predicate())) {
      atoms_.insert(*ai);
    }
  }
  // 考虑非static function
  for (ValueMap::const_iterator vi = problem.init_values().begin();
       vi != problem.init_values().end(); vi++) {
    Function function = (*vi).first->function();
    if (!problem.domain().functions().static_function(function)) {
      values_.insert(*vi);
    }
  }
  // 考虑初始状态下的effect，计算三个List
  for (EffectList::const_iterator ei = problem.init_effects().begin();
       ei != problem.init_effects().end(); ei++) {
    AtomList adds;
    AtomList deletes;
    AssignmentList assignments;
    // 为什么没有考虑delete？
    (*ei)->state_change(adds, deletes, assignments, atoms_, values_);
    atoms_.insert(adds.begin(), adds.end());
    for (AssignmentList::const_iterator ai = assignments.begin();
      ai != assignments.end(); ai++) {
          (*ai)->affect(values_);
        }
  }
  if (problem.goal_reward() != NULL && problem.goal().holds(atoms_, values_)) {
    problem.goal_reward()->affect(values_);
  }
}


/* Returns a sampled successor of this state. */
const State& State::next(const Action* action) const {
  State* next_state = new State(*this);
  if (verbosity >= 3) {
    std::cerr << "selected action: ";
    if (action != NULL) {
      action->print(std::cerr, problem_->terms());
    } else {
      std::cerr << "<none>";
    }
    std::cerr << std::endl;
  }
  // 利用affect完成状态的更新
  if (action != NULL) {
    action->affect(next_state->atoms_, next_state->values_);
  }
  // 修改goal_reward信息
  if (problem_->goal_reward() != NULL
      && problem_->goal().holds(next_state->atoms_, next_state->values_)) {
    problem_->goal_reward()->affect(next_state->values_);
  }
  if (verbosity >= 3) {
    std::cerr << std::endl;
  }
  // 返回下一个状态的引用
  return *next_state;
}


/* Prints this object on the given stream. */
void State::print(std::ostream& os, const PredicateTable& predicates,
		  const FunctionTable& functions,
		  const TermTable& terms) const {
  bool first = true;
  for (AtomSet::const_iterator ai = atoms_.begin(); ai != atoms_.end(); ai++) {
    if (first) {
      first = false;
    } else {
      os << ' ';
    }
    (*ai)->print(os, predicates, functions, terms);
  }
  for (ValueMap::const_iterator vi = values_.begin();
       vi != values_.end(); vi++) {
    if (first) {
      first = false;
    } else {
      os << ' ';
    }
    os << "(= ";
    (*vi).first->print(os, functions, terms);
    os << ' ' << (*vi).second << ")";
  }
}


/* Prints this object on the given stream. */
void State::printXML(std::ostream& os, const PredicateTable& predicates,
		     const FunctionTable& functions,
		     const TermTable& terms, bool goal) const {
  os << "<state>";
  if (goal) {
    os << "<is-goal/>";
  }

  for (AtomSet::const_iterator ai = atoms_.begin(); ai != atoms_.end(); ai++) {
    (*ai)->printXML(os, predicates, functions, terms);
  }
  for (ValueMap::const_iterator vi = values_.begin();
       vi != values_.end(); vi++) {
    const Application *ap = vi->first;
    os << "<fluent><function>";
    functions.print_function(os, ap->function());
    os << "</function>";
    
    for (size_t i=0; i<ap->arity(); i++) {
      Term t = ap->term(i);
      terms.print_term(os, t);
    }
    os << "<value>" << vi->second << "</value>";
    
    os << "</fluent>";
  }
  os << "</state>";
  
}


/* Serializes the state and sends it on a stream */
void State::send(std::ostream& os, const PredicateTable& predicates,
		 const FunctionTable& functions,
		 const TermTable& terms) const {
  //send the count of the atoms
  os << atoms_.size() << std::endl;
  //send each of the serialized atoms, separated by newlines
  for (AtomSet::const_iterator ai = atoms_.begin(); ai != atoms_.end(); ai++) {
    (*ai)->print(os, predicates, functions, terms);
    os << std::endl;
  }
  //send the count of thet values
  os << values_.size() << std::endl;
  //send each of the serialized values, separated by newlines
  for (ValueMap::const_iterator vi = values_.begin();
       vi != values_.end(); vi++) {
    os << "(= ";
    vi->first->print(os, functions, terms);
    os << " " << vi->second << ")" << std::endl;
  }
}

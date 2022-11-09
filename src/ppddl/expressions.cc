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
 * $Id: expressions.cc,v 1.1 2006/07/11 17:53:06 dan Exp $
 */
#include "expressions.h"
#include "problems.h"
#include "domains.h"
#include "exceptions.h"


/* ====================================================================== */
/* Value */

/* Returns the value of this expression in the given state. */
Rational Value::value(const ValueMap& values) const {
  return value_;
}


/* Returns an instantiation of this expression. */
const Value& Value::instantiation(const SubstitutionMap& subst,
				  const Problem& problem) const {
  return *this;
}


/* Prints this object on the given stream. */
void Value::print(std::ostream& os, const FunctionTable& functions,
		  const TermTable& terms) const {
  os << value_;
}


/* ====================================================================== */
/* Application */

/* Table of function applications. */
Application::ApplicationTable Application::applications;


/* Comparison function. */
bool Application::ApplicationLess::operator()(const Application* a1,
					      const Application* a2) const {
  if (a1->function() < a2->function()) {
    return true;
  } else if (a1->function() > a2->function()) {
    return false;
  } else {
    for (size_t i = 0; i < a1->arity(); i++) {
      if (a1->term(i) < a2->term(i)) {
	return true;
      } else if (a1->term(i) > a2->term(i)) {
	return false;
      }
    }
    return false;
  }
}


/* Returns a function application with the given function and terms. */
const Application& Application::make_application(Function function,
						 const TermList& terms) {
  Application* application = new Application(function);
  bool ground = true;
  for (TermList::const_iterator ti = terms.begin(); ti != terms.end(); ti++) {
    application->add_term(*ti);
    if (ground && is_variable(*ti)) {
      ground = false;
    }
  }
  if (!ground) {
    return *application;
  } else {
    std::pair<ApplicationTable::const_iterator, bool> result =
      applications.insert(application);
    if (!result.second) {
      delete application;
      return **result.first;
    } else {
      return *application;
    }
  }
}


/* Deletes this function application. */
Application::~Application() {
  ApplicationTable::const_iterator ai = applications.find(this);
  if (*ai == this) {
    applications.erase(ai);
  }
}


/* Returns the value of this expression in the given state. */
Rational Application::value(const ValueMap& values) const {
  ValueMap::const_iterator vi = values.find(this);
  if (vi != values.end()) {
    return (*vi).second;
  } else {
    throw Exception("value of function application undefined");
  }
}


/* Returns this application subject to the given substitution. */
const Application&
Application::substitution(const SubstitutionMap& subst) const {
  TermList inst_terms;
  bool substituted = false;
  for (TermList::const_iterator ti = terms_.begin();
       ti != terms_.end(); ti++) {
    SubstitutionMap::const_iterator si =
      is_variable(*ti) ? subst.find(*ti) : subst.end();
    if (si != subst.end()) {
      inst_terms.push_back((*si).second);
      substituted = true;
    } else {
      inst_terms.push_back(*ti);
    }
  }
  if (substituted) {
    return make_application(function(), inst_terms);
  } else { 
    return *this;
  }
}


/* Returns an instantiation of this expression. */
const Expression& Application::instantiation(const SubstitutionMap& subst,
					     const Problem& problem) const {
  TermList inst_terms;
  bool substituted = false;
  size_t objects = 0;
  //  cout << "instan expr"<<endl;
  for (TermList::const_iterator ti = terms_.begin();
       ti != terms_.end(); ti++) {
    SubstitutionMap::const_iterator si =
      is_variable(*ti) ? subst.find(*ti) : subst.end();
    if (si != subst.end()) {
      inst_terms.push_back((*si).second);
      substituted = true;
      objects++;
    } else {
      inst_terms.push_back(*ti);
      if (is_object(*ti)) {
	objects++;
      }
    }
  }

 if (substituted) {

    const Application& inst_appl = make_application(function(), inst_terms);
    if (problem.domain().functions().static_function(function())
	&& objects == inst_terms.size()) {
      ValueMap::const_iterator vi = problem.init_values().find(&inst_appl);

      if (vi != problem.init_values().end()) {

	return *new Value((*vi).second);
      } else {
	throw Exception("value of static function application undefined");
      }
    } else {
      return inst_appl;
    }
  } else { 
    return *this;
  }

}


/* Prints this object on the given stream. */
void Application::print(std::ostream& os, const FunctionTable& functions,
			const TermTable& terms) const {
  os << '(';
  functions.print_function(os, function());
  for (TermList::const_iterator ti = terms_.begin();
       ti != terms_.end(); ti++) {
    os << ' ';
    terms.print_term(os, *ti);
  }
  os << ')';
}

/* Prints this object on the given stream in XML. */
void Application::printXML(std::ostream& os, const FunctionTable& functions,
			const TermTable& terms) const {
  os << "<fluent><function>";
  functions.print_function(os, function());
  os << "</function>";
  for (TermList::const_iterator ti = terms_.begin();
       ti != terms_.end(); ti++) {
    os << "<term>";
    terms.print_term(os, *ti);
    os << "</term>";
  }
  os << "</fluent>";
}


/* ====================================================================== */
/* Addition */

/* Constructs an addition. */
Addition::Addition(const Expression& term1, const Expression& term2)
  : term1_(&term1), term2_(&term2) {
  register_use(term1_);
  register_use(term2_);
}


/* Deletes this addition. */
Addition::~Addition() {
  unregister_use(term1_);
  unregister_use(term2_);
}


/* Returns the value of this expression in the given state. */
Rational Addition::value(const ValueMap& values) const {
  return term1().value(values) + term2().value(values);
}


/* Returns an instantiation of this expression. */
const Expression& Addition::instantiation(const SubstitutionMap& subst,
					  const Problem& problem) const {
  const Expression& inst_term1 = term1().instantiation(subst, problem);
  const Expression& inst_term2 = term2().instantiation(subst, problem);
  const Value* v1 = dynamic_cast<const Value*>(&inst_term1);
  if (v1 != NULL) {
    const Value* v2 = dynamic_cast<const Value*>(&inst_term2);
    if (v2 != NULL) {
      const Value* value = new Value(v1->value() + v2->value());
      register_use(v1);
      register_use(v2);
      unregister_use(v1);
      unregister_use(v2);
      return *value;
    }
  }
  return *new Addition(inst_term1, inst_term2);
}


/* Prints this object on the given stream. */
void Addition::print(std::ostream& os, const FunctionTable& functions,
		     const TermTable& terms) const {
  os << "(+ ";
  term1().print(os, functions, terms);
  os << ' ';
  term2().print(os, functions, terms);
  os << ")";
}


/* ====================================================================== */
/* Subtraction */

/* Constructs a subtraction. */
Subtraction::Subtraction(const Expression& term1, const Expression& term2)
  : term1_(&term1), term2_(&term2) {
  register_use(term1_);
  register_use(term2_);
}


/* Deletes this subtraction. */
Subtraction::~Subtraction() {
  unregister_use(term1_);
  unregister_use(term2_);
}


/* Returns the value of this expression in the given state. */
Rational Subtraction::value(const ValueMap& values) const {
  return term1().value(values) - term2().value(values);
}


/* Returns an instantiation of this expression. */
const Expression& Subtraction::instantiation(const SubstitutionMap& subst,
					     const Problem& problem) const {
  const Expression& inst_term1 = term1().instantiation(subst, problem);
  const Expression& inst_term2 = term2().instantiation(subst, problem);
  const Value* v1 = dynamic_cast<const Value*>(&inst_term1);
  if (v1 != NULL) {
    const Value* v2 = dynamic_cast<const Value*>(&inst_term2);
    if (v2 != NULL) {
      const Value* value = new Value(v1->value() - v2->value());
      register_use(v1);
      register_use(v2);
      unregister_use(v1);
      unregister_use(v2);
      return *value;
    }
  }
  return *new Subtraction(inst_term1, inst_term2);
}


/* Prints this object on the given stream. */
void Subtraction::print(std::ostream& os, const FunctionTable& functions,
			const TermTable& terms) const {
  os << "(- ";
  term1().print(os, functions, terms);
  os << ' ';
  term2().print(os, functions, terms);
  os << ")";
}


/* ====================================================================== */
/* Multiplication */

/* Constructs a multiplication. */
Multiplication::Multiplication(const Expression& factor1,
			       const Expression& factor2)
  : factor1_(&factor1), factor2_(&factor2) {
  register_use(factor1_);
  register_use(factor2_);
}


/* Deletes this multiplication. */
Multiplication::~Multiplication() {
  unregister_use(factor1_);
  unregister_use(factor2_);
}


/* Returns the value of this expression in the given state. */
Rational Multiplication::value(const ValueMap& values) const {
  return factor1().value(values) * factor2().value(values);
}


/* Returns an instantiation of this expression. */
const Expression& Multiplication::instantiation(const SubstitutionMap& subst,
						const Problem& problem) const {
  const Expression& inst_factor1 = factor1().instantiation(subst, problem);
  const Expression& inst_factor2 = factor2().instantiation(subst, problem);
  const Value* v1 = dynamic_cast<const Value*>(&inst_factor1);
  if (v1 != NULL) {
    const Value* v2 = dynamic_cast<const Value*>(&inst_factor2);
    if (v2 != NULL) {
      const Value* value = new Value(v1->value() * v2->value());
      register_use(v1);
      register_use(v2);
      unregister_use(v1);
      unregister_use(v2);
      return *value;
    }
  }
  return *new Multiplication(inst_factor1, inst_factor2);
}


/* Prints this object on the given stream. */
void Multiplication::print(std::ostream& os, const FunctionTable& functions,
			   const TermTable& terms) const {
  os << "(* ";
  factor1().print(os, functions, terms);
  os << ' ';
  factor2().print(os, functions, terms);
  os << ")";
}


/* ====================================================================== */
/* Division */

/* Constructs a division. */
Division::Division(const Expression& factor1, const Expression& factor2)
  : factor1_(&factor1), factor2_(&factor2) {
  register_use(factor1_);
  register_use(factor2_);
}


/* Deletes this division. */
Division::~Division() {
  unregister_use(factor1_);
  unregister_use(factor2_);
}


/* Returns the value of this expression in the given state. */
Rational Division::value(const ValueMap& values) const {
  return factor1().value(values) / factor2().value(values);
}


/* Returns an instantiation of this expression. */
const Expression& Division::instantiation(const SubstitutionMap& subst,
					  const Problem& problem) const {
  const Expression& inst_factor1 = factor1_->instantiation(subst, problem);
  const Expression& inst_factor2 = factor2_->instantiation(subst, problem);
  const Value* v1 = dynamic_cast<const Value*>(&inst_factor1);
  if (v1 != NULL) {
    const Value* v2 = dynamic_cast<const Value*>(&inst_factor2);
    if (v2 != NULL) {
      const Value* value = new Value(v1->value() / v2->value());
      register_use(v1);
      register_use(v2);
      unregister_use(v1);
      unregister_use(v2);
      return *value;
    }
  }
  return *new Division(inst_factor1, inst_factor2);
}


/* Prints this object on the given stream. */
void Division::print(std::ostream& os, const FunctionTable& functions,
		     const TermTable& terms) const {
  os << "(/ ";
  factor1().print(os, functions, terms);
  os << ' ';
  factor2().print(os, functions, terms);
  os << ")";
}

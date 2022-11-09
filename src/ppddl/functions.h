/* -*-C++-*- */
/*
 * Functions.
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
 * $Id: functions.h,v 1.1 2006/07/11 17:53:06 dan Exp $
 * 对Function实际上进行了编号，命名。
 * 利用FunctionTable存储function相关的信息：
 *  内部存储了一个FunctionSet，本质是一个hash map实现function到value的映射。
 */
#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <config.h>
#include "types.h"
#include "hashing.h"
#include <iostream>
#include <map>
#include <string>
#include <vector>


/* Function index. */
typedef int Function;

/* Name of number type. */
const std::string NUMBER_NAME("number");


/* ====================================================================== */
/* FunctionSet */

/* Set of function declarations. */
struct FunctionSet : public hashing::hash_set<Function> {
};


/* ====================================================================== */
/* FunctionTable */

/*
 * Function table.
 */
struct FunctionTable {
  /* Constructs an empty function table. */
  FunctionTable() : first_(next) {}

  /* Adds a function with the given name to this table and returns the
     function. */
  Function add_function(const std::string& name);

  /* Returns the function with the given name.  If no function with
     the given name exists, false is returned in the second part of
     the result. */
  std::pair<Function, bool> find_function(const std::string& name) const;

  /* Returns the first function of this function table. */
  Function first_function() const { return first_; }

  /* Returns the last function of this function table. */
  Function last_function() const { return first_ + names_.size() - 1; }

  /* Adds a parameter with the given type to the given function. */
  void add_parameter(Function function, Type type) {
    parameters_[function - first_].push_back(type);
  }

  /* Returns the name of the given function. */
  const std::string& name(Function function) const {
    return names_[function - first_];
  }

  /* Returns the arity of the given function. */
  size_t arity(Function function) const {
    return parameters_[function - first_].size();
  }

  /* Returns the ith parameter type of the given function. */
  Type parameter(Function function, size_t i) const {
    return parameters_[function - first_][i];
  }

  /* Makes the given function dynamic. */
  void make_dynamic(Function function) {
    static_functions_.erase(function);
  }

  /* Tests if the given function is static. */
  bool static_function(Function function) const {
    return static_functions_.find(function) != static_functions_.end();
  }

  /* Prints the given function on the given stream. */
  void print_function(std::ostream& os, Function function) const;

private:
  /* Next function. */
  static Function next;

  /* The first function. */
  Function first_;
  /* Function names. */
  std::vector<std::string> names_;
  /* Mapping of function names to functions. */
  std::map<std::string, Function> functions_;
  /* Function parameters. */
  std::vector<TypeList> parameters_;
  /* Static functions. */
  FunctionSet static_functions_;
};


#endif /* FUNCTIONS_H */

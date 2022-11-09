/* -*-C++-*- */
/*
 * Rational numbers.
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
 * $Id: rational.h,v 1.1 2006/07/11 17:53:06 dan Exp $
 * 实现基本的一个实数类，包括i多个构造函数。
 * 同时重载了一组对Rational的操作
 */
#ifndef RATIONAL_H
#define RATIONAL_H

#include <config.h>
#include <iostream>
#include <utility>

extern int FRACT_RATIONALS;

/* ====================================================================== */
/* Rational */

/*
 * A rational number.
 */
struct Rational {
  /* Returns the multipliers for the two integers. */
  static std::pair<unsigned long int, unsigned long int> multipliers( unsigned long int n,  unsigned long int m);

#ifdef FRACT_RATIONALS  
  /* Constructs a rational number. */
  Rational(int n = 0) : numerator_(n), denominator_(1) {}
#else
  Rational(double n = 0.0) : value_(n) {}
#endif

  /* Constructs a rational number. */
  Rational( unsigned long int n,  unsigned long int m);

  /* Constructs a rational number. */
  Rational(const char* s);

#ifdef FRACT_RATIONALS  
  /* Returns the numerator of this rational number. */
  unsigned long int numerator() const { return numerator_; }

  /* Returns the denominator of this rational number. */
  unsigned long int denominator() const { return denominator_; }
#endif

  /* Returns the double value of this rational number. */
  double double_value() const { 
#ifdef FRACT_RATIONALS  
      return double(numerator())/denominator(); 
#else
      return value_;
#endif
  }

private:
#ifdef FRACT_RATIONALS  
  /* The numerator. */
  unsigned long int numerator_;
  /* The denominator. */
  unsigned long int denominator_;
#else
  double value_;
#endif
};

/* Less-than comparison operator for rational numbers. */
bool operator<(const Rational& q, const Rational& p);

/* Less-than-or-equal comparison operator for rational numbers. */
bool operator<=(const Rational& q, const Rational& p);

/* Equality comparison operator for rational numbers. */
bool operator==(const Rational& q, const Rational& p);

/* Inequality comparison operator for rational numbers. */
bool operator!=(const Rational& q, const Rational& p);

/* Greater-than-or-equal comparison operator for rational numbers. */
bool operator>=(const Rational& q, const Rational& p);

/* Greater-than comparison operator for rational numbers. */
bool operator>(const Rational& q, const Rational& p);

/* Addition operator for rational numbers. */
Rational operator+(const Rational& q, const Rational& p);

/* Subtraction operator for rational numbers. */
Rational operator-(const Rational& q, const Rational& p);

/* Multiplication operator for rational numbers. */
Rational operator*(const Rational& q, const Rational& p);

/* Division operator for rational numbers. */
Rational operator/(const Rational& q, const Rational& p);

/* Output operator for rational numbers. */
std::ostream& operator<<(std::ostream& os, const Rational& q);


#endif /* RATIONAL_H */

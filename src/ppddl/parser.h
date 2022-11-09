/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     DEFINE = 258,
     DOMAIN_TOKEN = 259,
     PROBLEM = 260,
     REQUIREMENTS = 261,
     TYPES = 262,
     CONSTANTS = 263,
     PREDICATES = 264,
     FUNCTIONS = 265,
     OBSERVABLES = 266,
     STRIPS = 267,
     TYPING = 268,
     NEGATIVE_PRECONDITIONS = 269,
     DISJUNCTIVE_PRECONDITIONS = 270,
     EQUALITY = 271,
     EXISTENTIAL_PRECONDITIONS = 272,
     UNIVERSAL_PRECONDITIONS = 273,
     QUANTIFIED_PRECONDITIONS = 274,
     CONDITIONAL_EFFECTS = 275,
     FLUENTS = 276,
     ADL = 277,
     DURATIVE_ACTIONS = 278,
     DURATION_INEQUALITIES = 279,
     CONTINUOUS_EFFECTS = 280,
     PROBABILISTIC_EFFECTS = 281,
     REWARDS = 282,
     MDP = 283,
     ONEOF = 284,
     UNKNOWN = 285,
     NON_DETERMINISTIC_DYNAMICS = 286,
     PROBABILISTIC_DYNAMICS = 287,
     ACTION = 288,
     EVENT = 289,
     PARAMETERS = 290,
     PRECONDITION = 291,
     EFFECT = 292,
     OBSERVATION = 293,
     PDOMAIN = 294,
     OBJECTS = 295,
     INIT = 296,
     GOAL = 297,
     GOAL_REWARD = 298,
     METRIC = 299,
     GOAL_PROBABILITY = 300,
     WHEN = 301,
     NOT = 302,
     AND = 303,
     OR = 304,
     IMPLY = 305,
     EXISTS = 306,
     FORALL = 307,
     PROBABILISTIC = 308,
     ASSIGN = 309,
     SCALE_UP = 310,
     SCALE_DOWN = 311,
     INCREASE = 312,
     DECREASE = 313,
     MINIMIZE = 314,
     MAXIMIZE = 315,
     NUMBER_TOKEN = 316,
     OBJECT_TOKEN = 317,
     EITHER = 318,
     LE = 319,
     GE = 320,
     NAME = 321,
     VARIABLE = 322,
     NUMBER = 323,
     HORIZON = 324,
     DISC = 325,
     ILLEGAL_TOKEN = 326,
     PLANTIME = 327,
     PLAN = 328,
     FORPROBLEM = 329,
     IF = 330,
     THEN = 331,
     ELSE = 332,
     CASE = 333,
     GOTO = 334,
     DONE = 335,
     ANTI_COMMENT = 336,
     OBSERVE = 337
   };
#endif
/* Tokens.  */
#define DEFINE 258
#define DOMAIN_TOKEN 259
#define PROBLEM 260
#define REQUIREMENTS 261
#define TYPES 262
#define CONSTANTS 263
#define PREDICATES 264
#define FUNCTIONS 265
#define OBSERVABLES 266
#define STRIPS 267
#define TYPING 268
#define NEGATIVE_PRECONDITIONS 269
#define DISJUNCTIVE_PRECONDITIONS 270
#define EQUALITY 271
#define EXISTENTIAL_PRECONDITIONS 272
#define UNIVERSAL_PRECONDITIONS 273
#define QUANTIFIED_PRECONDITIONS 274
#define CONDITIONAL_EFFECTS 275
#define FLUENTS 276
#define ADL 277
#define DURATIVE_ACTIONS 278
#define DURATION_INEQUALITIES 279
#define CONTINUOUS_EFFECTS 280
#define PROBABILISTIC_EFFECTS 281
#define REWARDS 282
#define MDP 283
#define ONEOF 284
#define UNKNOWN 285
#define NON_DETERMINISTIC_DYNAMICS 286
#define PROBABILISTIC_DYNAMICS 287
#define ACTION 288
#define EVENT 289
#define PARAMETERS 290
#define PRECONDITION 291
#define EFFECT 292
#define OBSERVATION 293
#define PDOMAIN 294
#define OBJECTS 295
#define INIT 296
#define GOAL 297
#define GOAL_REWARD 298
#define METRIC 299
#define GOAL_PROBABILITY 300
#define WHEN 301
#define NOT 302
#define AND 303
#define OR 304
#define IMPLY 305
#define EXISTS 306
#define FORALL 307
#define PROBABILISTIC 308
#define ASSIGN 309
#define SCALE_UP 310
#define SCALE_DOWN 311
#define INCREASE 312
#define DECREASE 313
#define MINIMIZE 314
#define MAXIMIZE 315
#define NUMBER_TOKEN 316
#define OBJECT_TOKEN 317
#define EITHER 318
#define LE 319
#define GE 320
#define NAME 321
#define VARIABLE 322
#define NUMBER 323
#define HORIZON 324
#define DISC 325
#define ILLEGAL_TOKEN 326
#define PLANTIME 327
#define PLAN 328
#define FORPROBLEM 329
#define IF 330
#define THEN 331
#define ELSE 332
#define CASE 333
#define GOTO 334
#define DONE 335
#define ANTI_COMMENT 336
#define OBSERVE 337



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 295 "parser.yy"
{
  Assignment::AssignOp setop;
  const pEffect* effect;
  ConjunctiveEffect* ceffect;
  ProbabilisticEffect* peffect; 
  Observation* observation_defs;
  ObservationEntry* observation;
  const StateFormula* formula;
  const Atom* atom;
  Conjunction* conj;
  Disjunction* disj;
  OneOfDisjunction* odisj;
  const Expression* expr;
  const Application* appl;
  Comparison::CmpPredicate comp;
  Type type;
  TypeSet* types;
  const std::string* str;
  std::vector<const std::string*>* strs;
  const Rational* num;
  //DAN
  plan* t_plan;
  Instruction * t_instr;
  Guards *t_guards;
  label_symbol* t_label_symbol;

  //NAD
}
/* Line 1529 of yacc.c.  */
#line 240 "parser.h"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;


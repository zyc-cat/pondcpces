/* A Bison parser, made by GNU Bison 3.5.1.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2020 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Undocumented macros, especially those whose name start with YY_,
   are private implementation details.  Do not rely on them.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.5.1"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 20 "parser.yy"

#include <config.h>
#include "problems.h"
#include "domains.h"
#include "actions.h"
#include "effects.h"
#include "formulas.h"
#include "expressions.h"
#include "functions.h"
#include "predicates.h"
#include "terms.h"
#include "types.h"
#include "rational.h"
#include "exceptions.h"
#include "observations.h"
#include <iostream>
#include <map>
#include <string>
#include <typeinfo>

  //dan
#include "globals.h"
  //#include <FlexLexer.h>


/* Workaround for bug in Bison 1.35 that disables stack growth. */
#define YYLTYPE_IS_TRIVIAL 1


/*
 * Context of free variables.
 */
struct Context {
    void push_frame() {
        frames_.push_back(VariableMap());
    }

    void pop_frame() {
        frames_.pop_back();
    }

    void insert(const std::string& name, Variable v) {
        frames_.back()[name] = v;
    }

    std::pair<Variable, bool> shallow_find(const std::string& name) const {
        VariableMap::const_iterator vi = frames_.back().find(name);
        if (vi != frames_.back().end()) {
            return std::make_pair((*vi).second, true);
        } else {
            return std::make_pair(0, false);
        }
    }

    std::pair<Variable, bool> find(const std::string& name) const {
        for (std::vector<VariableMap>::const_reverse_iterator fi = frames_.rbegin(); fi != frames_.rend(); fi++) {
            VariableMap::const_iterator vi = (*fi).find(name);
            if (vi != (*fi).end()) {
                return std::make_pair((*vi).second, true);
            }
        }
        return std::make_pair(0, false);
    }

private:
    struct VariableMap : public std::map<std::string, Variable> {};
    
    std::vector<VariableMap> frames_;// 存储每一个变量名对应的index即variable
};

//DAN
 extern int yyrestart(FILE*);
 // extern int yyrestart();
 // extern int yy_scan_string(char*);


/* The lexer. */
extern int yylex();
/* Current line number. */
extern size_t line_number;
/* Name of current file. */
extern std::string current_file;
/* Level of warnings. */
extern int warning_level;

/* Whether the last parsing attempt succeeded. */
static bool success = true;
/* Current domain. */
static Domain* domain;
/* Domains. */
static std::map<std::string, Domain*> domains;
/* Problem being parsed, or NULL if no problem is being parsed. */
static Problem* problem;
/* Current requirements. */
static Requirements* requirements;
/* The goal probability function, if probabilistic effects are required. */
static Function goal_prob_function; 
/* The reward function, if rewards are required. */
static Function reward_function;
/* Predicate being parsed. */
static Predicate predicate;
/* Whether a predicate is being parsed. */
static bool parsing_predicate;
/* Whether predicate declaration is repeated. */
static bool repeated_predicate;
/* Function being parsed. */
static Function function;
/* Whether a function is being parsed. */
static bool parsing_function;
/* Whether function declaration is repeated. */
static bool repeated_function;
/* Action being parsed, or NULL if no action is being parsed. */
static ActionSchema* action;
/* Current variable context. */
static Context context;
/* Predicate for atomic state formula being parsed. */
static Predicate atom_predicate;
/* Whether the predicate of the currently parsed atom was undeclared. */
static bool undeclared_atom_predicate;
/* Whether parsing effect fluents. */
static bool effect_fluent;
/* Whether parsing metric fluent. */
static bool metric_fluent;
/* Function for function application being parsed. */
static Function appl_function;
/* Whether the function of the currently parsed application was undeclared. */
static bool undeclared_appl_function;
/* Paramerers for atomic state formula or function application being parsed. 
  状态变量表，一个domain/problem中使用的状态变量
*/
static TermList term_parameters;
/* Whether parsing an atom. */
static bool parsing_atom;
/* Whether parsing a function application. */
static bool parsing_application;
/* Whether parsing a obs token. */
static bool parsing_obs_token = false;
/* Quantified variables for effect or formula being parsed. */
static VariableList quantified;
/* Most recently parsed term for equality formula. */
static Term eq_term;
/* Most recently parsed expression for equality formula. */
static const Expression* eq_expr;
/* The first term for equality formula. */
static Term first_eq_term;
/* The first expression for equality formula. */
static const Expression* first_eq_expr;
/* Kind of name map being parsed. */
static enum { TYPE_KIND, CONSTANT_KIND, OBJECT_KIND, VOID_KIND } name_kind;

/* Outputs an error message. */
static void yyerror(const std::string& s); 
/* Outputs a warning message. */
static void yywarning(const std::string& s);
/* Creates an empty domain with the given name. */
static void make_domain(const std::string* name);
/* Creates an empty problem with the given name. */
static void make_problem(const std::string* name,
			 const std::string* domain_name);
/* Adds :typing to the requirements. */
static void require_typing();
/* Adds :fluents to the requirements. */
static void require_fluents();
/* Adds :disjunctive-preconditions to the requirements. */
static void require_disjunction();
/* Adds :conditional-effects to the requirements. */ 
static void require_conditional_effects();
/* Returns a simple type with the given name. */
static Type make_type(const std::string* name);
/* Returns the union of the given types. */
static Type make_type(const TypeSet& types);
/* Returns a simple term with the given name. */
static Term make_term(const std::string* name);
/* Creates a predicate with the given name. */
static void make_predicate(const std::string* name);
/* Creates an observation token with the given name. */
static void make_observable(const std::string* name);
/* Creates a function with the given name. */
static void make_function(const std::string* name);
/* Creates an action with the given name. */
 static ObservationEntry* make_observation(const StateFormula&, const StateFormula&, const Rational&);
 static ObservationEntry* make_observation(const StateFormula&, const Rational&, const Rational &);
 static ObservationEntry* make_observation(const StateFormula&, const ProbabilisticEffect&);
/* Creates an action with the given name. */
static void make_action(const std::string* name);
/* Adds the current action to the current domain. */
static void add_action();
/* Adds the current event to the current domain. */
static void add_event();
/* Prepares for the parsing of a universally quantified effect. */ 
static void prepare_forall_effect();
/* Creates a universally quantified effect. */
static const pEffect* make_forall_effect(const pEffect& effect);
/* Adds an outcome to the given probabilistic effect. */
static void add_effect_outcome(ProbabilisticEffect& peffect,
			        const Rational* p, 
			       const pEffect& effect);
//dan
static void add_feffect_outcome(ProbabilisticEffect& peffect,
			        const Expression* p, 
			       const pEffect& effect);
/* Creates an add effect. */
static const pEffect* make_add_effect(const Atom& atom);
/* Creates a delete effect. */
static const pEffect* make_delete_effect(const Atom& atom);
/* Creates an assignment effect. */
static const pEffect* make_assignment_effect(Assignment::AssignOp oper,
					    const Application& application,
					    const Expression& expr);
/* Adds types, constants, or objects to the current domain or problem. */
static void add_names(const std::vector<const std::string*>* names, Type type);
/* Adds variables to the current variable list. */
static void add_variables(const std::vector<const std::string*>* names,
			  Type type);
/* Prepares for the parsing of an atomic state formula. */ 
static void prepare_atom(const std::string* name);
/* Prepares for the parsing of a function application. */ 
static void prepare_application(const std::string* name);
/* Adds a term with the given name to the current atomic state formula. */
static void add_term(const std::string* name);
/* Creates the atomic formula just parsed. */
static const Atom* make_atom();
/* Creates the function application just parsed. */
static const Application* make_application();
/* Creates a subtraction. */
static const Expression* make_subtraction(const Expression& term,
					  const Expression* opt_term);
/* Creates an atom or fluent for the given name to be used in an
   equality formula. */
static void make_eq_name(const std::string* name);
/* Creates an equality formula. */
static const StateFormula* make_equality();
/* Creates a negated formula. */
static const StateFormula* make_negation(const StateFormula& negand);
/* Creates an implication. */
static const StateFormula* make_implication(const StateFormula& f1,
					    const StateFormula& f2);
/* Prepares for the parsing of an existentially quantified formula. */
static void prepare_exists();
/* Prepares for the parsing of a universally quantified formula. */
static void prepare_forall();
/* Creates an existentially quantified formula. */
static const StateFormula* make_exists(const StateFormula& body);
/* Creates a universally quantified formula. */
static const StateFormula* make_forall(const StateFormula& body);
/* Sets the goal reward for the current problem. */
static void set_goal_reward(const Expression& goal_reward);
/* Sets the default metric for the current problem. */
static void set_default_metric();
/* if we use logical formula for init, then need to get atoms holding for later grounding 
   purposes*/
 static void get_init_elts();
 static void set_discount(const Rational& discount);
/*DAN */



#line 328 "parser.cc"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Use api.header.include to #include this header
   instead of duplicating it here.  */
#ifndef YY_YY_PARSER_HH_INCLUDED
# define YY_YY_PARSER_HH_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
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

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 296 "parser.yy"

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

#line 572 "parser.cc"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_PARSER_HH_INCLUDED  */



#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))

/* Stored state numbers (used for stacks). */
typedef yytype_int16 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && ! defined __ICC && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                            \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  5
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1903

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  92
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  175
/* YYNRULES -- Number of rules.  */
#define YYNRULES  388
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  668

#define YYUNDEFTOK  2
#define YYMAXUTOK   337


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      83,    84,    90,    89,     2,    85,     2,    91,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      87,    86,    88,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   356,   356,   356,   358,   358,   359,   359,   369,   377,
     387,   393,   396,   403,   410,   411,   412,   413,   414,   419,
     425,   428,   447,   448,   451,   458,   465,   470,   477,   484,
     490,   494,   501,   509,   510,   511,   518,   518,   522,   523,
     524,   525,   528,   529,   530,   536,   537,   538,   539,   540,
     541,   542,   543,   544,   547,   548,   549,   550,   551,   552,
     553,   556,   557,   558,   559,   560,   563,   564,   565,   566,
     567,   568,   571,   572,   573,   576,   577,   578,   579,   580,
     583,   584,   585,   591,   591,   596,   597,   600,   604,   607,
     608,   611,   612,   613,   615,   617,   618,   620,   622,   624,
     625,   626,   631,   632,   633,   635,   637,   639,   646,   651,
     667,   667,   671,   671,   675,   678,   678,   685,   686,   689,
     689,   693,   694,   695,   698,   699,   702,   702,   705,   705,
     709,   710,   714,   713,   726,   726,   729,   729,   736,   737,
     740,   741,   744,   745,   746,   749,   750,   754,   757,   763,
     764,   765,   771,   772,   773,   773,   775,   775,   777,   778,
     779,   780,   783,   784,   787,   792,   794,   800,   807,   809,
     813,   815,   819,   826,   829,   830,   831,   831,   835,   836,
     837,   838,   839,   845,   846,   849,   850,   851,   852,   869,
     868,   872,   871,   878,   879,   887,   888,   940,   941,   941,
     955,   955,   957,   956,   962,   963,   966,   970,   972,   974,
     976,   977,   987,   992,   996,   998,  1002,  1007,  1008,  1011,
    1012,  1015,  1016,  1020,  1023,  1024,  1025,  1026,  1029,  1030,
    1033,  1034,  1038,  1039,  1039,  1041,  1041,  1050,  1051,  1053,
    1052,  1055,  1055,  1057,  1058,  1059,  1059,  1060,  1060,  1061,
    1062,  1062,  1064,  1064,  1068,  1069,  1072,  1073,  1075,  1076,
    1079,  1079,  1081,  1084,  1084,  1086,  1089,  1090,  1091,  1092,
    1099,  1100,  1101,  1102,  1103,  1104,  1107,  1109,  1109,  1111,
    1111,  1113,  1113,  1115,  1115,  1117,  1117,  1119,  1120,  1123,
    1124,  1127,  1127,  1129,  1132,  1133,  1135,  1137,  1139,  1141,
    1142,  1146,  1147,  1150,  1150,  1152,  1160,  1161,  1162,  1165,
    1166,  1169,  1170,  1171,  1171,  1174,  1175,  1178,  1179,  1180,
    1180,  1183,  1184,  1187,  1187,  1190,  1191,  1192,  1195,  1196,
    1197,  1198,  1201,  1208,  1211,  1214,  1217,  1220,  1223,  1226,
    1229,  1232,  1235,  1238,  1241,  1244,  1247,  1250,  1253,  1256,
    1259,  1262,  1265,  1268,  1271,  1274,  1274,  1274,  1275,  1276,
    1276,  1277,  1280,  1281,  1281,  1286,  1289,  1289,  1289,  1290,
    1290,  1290,  1291,  1291,  1291,  1291,  1291,  1291,  1291,  1291,
    1292,  1292,  1292,  1292,  1292,  1293,  1293,  1294,  1297
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "DEFINE", "DOMAIN_TOKEN", "PROBLEM",
  "REQUIREMENTS", "TYPES", "CONSTANTS", "PREDICATES", "FUNCTIONS",
  "OBSERVABLES", "STRIPS", "TYPING", "NEGATIVE_PRECONDITIONS",
  "DISJUNCTIVE_PRECONDITIONS", "EQUALITY", "EXISTENTIAL_PRECONDITIONS",
  "UNIVERSAL_PRECONDITIONS", "QUANTIFIED_PRECONDITIONS",
  "CONDITIONAL_EFFECTS", "FLUENTS", "ADL", "DURATIVE_ACTIONS",
  "DURATION_INEQUALITIES", "CONTINUOUS_EFFECTS", "PROBABILISTIC_EFFECTS",
  "REWARDS", "MDP", "ONEOF", "UNKNOWN", "NON_DETERMINISTIC_DYNAMICS",
  "PROBABILISTIC_DYNAMICS", "ACTION", "EVENT", "PARAMETERS",
  "PRECONDITION", "EFFECT", "OBSERVATION", "PDOMAIN", "OBJECTS", "INIT",
  "GOAL", "GOAL_REWARD", "METRIC", "GOAL_PROBABILITY", "WHEN", "NOT",
  "AND", "OR", "IMPLY", "EXISTS", "FORALL", "PROBABILISTIC", "ASSIGN",
  "SCALE_UP", "SCALE_DOWN", "INCREASE", "DECREASE", "MINIMIZE", "MAXIMIZE",
  "NUMBER_TOKEN", "OBJECT_TOKEN", "EITHER", "LE", "GE", "NAME", "VARIABLE",
  "NUMBER", "HORIZON", "DISC", "ILLEGAL_TOKEN", "PLANTIME", "PLAN",
  "FORPROBLEM", "IF", "THEN", "ELSE", "CASE", "GOTO", "DONE",
  "ANTI_COMMENT", "OBSERVE", "'('", "')'", "'-'", "'='", "'<'", "'>'",
  "'+'", "'*'", "'/'", "$accept", "file", "$@1", "$@2", "$@3", "c_plan",
  "c_plan_body", "c_steps", "c_step", "c_instruction", "c_action",
  "c_symbols", "c_then", "c_else", "c_if", "c_case", "c_guards", "c_goto",
  "c_label", "domains_and_problems", "domain_def", "$@4", "domain_body",
  "domain_body2", "domain_body3", "domain_body4", "domain_body5",
  "domain_body6", "domain_body7", "domain_body8", "domain_body9",
  "observables_def", "$@5", "structure_defs", "structure_def",
  "require_def", "require_keys", "require_key", "types_def", "$@6",
  "constants_def", "$@7", "predicates_def", "functions_def", "$@8",
  "predicate_decls", "predicate_decl", "$@9", "function_decls",
  "function_decl_seq", "function_type_spec", "$@10", "function_decl",
  "$@11", "observable_decls", "observable_decl", "$@12", "action_def",
  "$@13", "$@14", "parameters", "action_body", "action_body2",
  "action_body3", "precondition", "effect", "observations", "eff_formula",
  "$@15", "$@16", "eff_formulas", "prob_effs", "oneof_effs", "or_effs",
  "unknown_effs", "probability", "p_effect", "$@17", "assign_op",
  "observation_defs", "observation", "problem_def", "$@18", "$@19",
  "problem_body", "problem_body_r", "object_decl", "$@23", "init", "$@24",
  "$@25", "init_elements", "init_element", "prob_inits", "oneof_inits",
  "unknown_inits", "simple_init", "one_inits", "one_init", "value",
  "goal_spec", "discount", "goal_reward", "metric_spec", "$@26", "$@27",
  "formula", "$@28", "$@29", "$@30", "$@31", "$@32", "$@33", "conjuncts",
  "disjuncts", "oneof_disjuncts", "atomic_term_formula", "$@34",
  "atomic_name_formula", "$@35", "binary_comp", "f_exp", "term_or_f_exp",
  "$@36", "$@37", "$@38", "$@39", "$@40", "opt_f_exp", "f_head", "$@41",
  "ground_f_exp", "opt_ground_f_exp", "ground_f_head", "$@42", "terms",
  "names", "variables", "$@43", "variable_seq", "typed_names", "$@44",
  "name_seq", "type_spec", "$@45", "type", "types", "function_type",
  "define", "domain", "problem", "when", "not", "and", "or", "imply",
  "exists", "forall", "probabilistic", "assign", "scale_up", "scale_down",
  "increase", "decrease", "minimize", "maximize", "number", "object",
  "either", "plantime", "type_name", "predicate", "function", "name",
  "variable", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_int16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,    40,    41,    45,    61,    60,    62,    43,
      42,    47
};
# endif

#define YYPACT_NINF (-514)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-263)

#define yytable_value_is_error(Yyn) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
      11,    39,  -514,     6,  1570,  -514,    10,   108,  -514,  -514,
    -514,  -514,  -514,  -514,  -514,  -514,  -514,  -514,   667,  -514,
    -514,  -514,  -514,  -514,   121,  -514,  -514,    63,  -514,  -514,
    -514,  -514,  -514,  -514,  -514,  -514,  -514,  -514,   717,  -514,
    1318,  -514,  -514,  1345,  -514,  1600,  1600,  -514,  -514,  1600,
    -514,  -514,    -9,  -514,    70,    90,  -514,  -514,  -514,  -514,
    -514,  -514,  -514,  -514,  -514,  -514,  -514,  -514,  -514,  -514,
    -514,  -514,  -514,  -514,  -514,  -514,  -514,  -514,  -514,  -514,
    -514,  -514,   381,  -514,  -514,  -514,  -514,   114,   114,  1422,
     124,   153,   390,  -514,  1600,   141,   157,  -514,   100,  1780,
     401,  -514,  -514,  -514,  -514,  -514,  -514,  1318,   162,   172,
    -514,   578,  1422,  -514,  -514,   211,  -514,  -514,  -514,   535,
     182,   220,   220,   977,  -514,  -514,  1780,  1780,   201,  -514,
    -514,  1422,  1422,  1422,  1422,  -514,   207,  -514,  -514,  1422,
    1422,  1422,  1422,  -514,   215,  -514,  1460,  -514,  -514,  -514,
     219,   -41,  -514,   223,  -514,  -514,  -514,   227,   235,   246,
    1422,  1422,  1422,  1422,  1016,  -514,  1422,  1422,  1422,  1422,
    -514,  -514,  -514,   872,  1608,  1608,  1600,  -514,  -514,  -514,
    1600,  -514,   250,   277,  -514,   254,   272,   278,   279,  -514,
     288,   294,   295,   303,  1055,   304,  1608,  1608,   843,  -514,
    -514,  -514,  -514,   308,   264,   220,   312,   292,   322,  1780,
    -514,  -514,  -514,  -514,  -514,  -514,  -514,  -514,  -514,  -514,
    -514,  -514,  1608,  1634,  1608,  -514,  -514,  -514,  -514,  -514,
    -514,  -514,  1643,  -514,  -514,  1460,  -514,  -514,  -514,  -514,
    -514,  -514,  -514,   307,  -514,  -514,  -514,  -514,  -514,   202,
     317,  -514,  -514,   319,  -514,   320,   324,   331,   332,   335,
    -514,  1780,   336,  -514,   675,  1816,   337,  -514,  -514,  1159,
    1498,  1600,   350,   338,  1167,   340,   911,  -514,   943,  1871,
    -514,  -514,  -514,  -514,  1780,  1780,  -514,   228,  -514,   243,
    -514,   271,  -514,   156,  -514,   319,   341,   342,   468,  -514,
     363,   319,   365,   372,   205,  -514,   319,   342,   374,   385,
     392,  -514,  -514,  -514,  -514,  -514,  1780,  1422,  1608,  -514,
    -514,  -514,   220,  -514,   938,  -514,  -514,  -514,  -514,  -514,
    -514,  -514,  -514,  -514,  -514,  -514,  -514,  -514,  -514,  -514,
    -514,  -514,  -514,  -514,  -514,  -514,  1785,  -514,  1780,  1780,
     218,   398,  -514,  -514,    21,  -514,   365,   319,   319,   195,
    -514,   319,   399,  -514,   282,  -514,   287,  -514,   262,  -514,
     319,   319,  -514,   457,  -514,  -514,  -514,   769,   402,   403,
     404,   405,  -514,  -514,  -514,  -514,  -514,   408,   808,  -514,
     409,   350,  -514,  -514,  1780,   410,    74,  -514,   355,   355,
    -514,   319,   326,   319,   319,  -514,   319,   400,   400,  -514,
    1716,   411,   769,  -514,    16,  -514,  -514,  -514,  -514,  1608,
    -514,  -514,  -514,  -514,  -514,  -514,  -514,  -514,   398,  -514,
     414,    79,    79,   319,   234,   145,   416,   420,  -514,   417,
     422,   423,  1600,  -514,  1780,   -32,   425,   427,   428,   429,
    -514,  -514,  -514,  -514,   431,  1780,   220,   220,   445,  -514,
     220,  1600,  1608,   433,   439,   440,  -514,  -514,    96,    48,
    -514,   441,   350,  -514,  -514,  -514,  1652,   291,  -514,  -514,
     482,   422,   177,  -514,  -514,   443,   444,   447,  -514,   465,
    -514,  -514,  -514,  -514,  -514,  1780,  -514,  -514,   450,   451,
    -514,  -514,  -514,   452,  -514,  -514,  1562,  1570,  -514,  -514,
    -514,  -514,  -514,  1780,   212,  -514,  -514,  -514,  -514,  1600,
     286,   469,   461,   459,  1600,   463,  -514,  -514,  -514,  -514,
    1197,  -514,  1570,  1205,   220,   464,  1677,  1677,  1536,  -514,
     114,  -514,  -514,   -19,  -514,  -514,  -514,  -514,  -514,   477,
     769,   472,  -514,   769,  -514,  -514,  -514,  1231,  -514,   466,
    -514,   140,  1241,  -514,  -514,  -514,   474,  -514,  1780,   484,
    -514,  1250,  1677,    54,  -514,  1269,   475,   150,  1384,  1384,
     476,   478,   769,   479,  -514,  -514,  1536,  -514,  -514,  -514,
    -514,  -514,   480,  -514,  -514,  -514,  1677,  1094,   305,  -514,
    -514,   470,   495,    -6,   470,  -514,  -514,  -514,  -514,   599,
     483,  -514,   485,  -514,  -514,   487,  -514,   498,  1279,  -514,
    -514,   488,  -514,  -514,  -514,   306,  -514,   500,  1384,   509,
    -514,  1384,  1384,  1384,  1384,  -514,  -514,  -514,  -514,   501,
     132,  -514,  -514,  1133,  -514,   502,   503,   504,  1384,  1384,
    1384,  1384,  -514,  -514,  -514,   496,  -514,  -514,   505,   515,
     521,   522,   540,  -514,  -514,  -514,  -514,  -514
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_int16 yydefact[] =
{
       6,     0,    33,     0,     0,     1,     3,     0,     5,   355,
     356,   357,   359,   360,   364,   363,   358,   361,     0,     7,
     188,   238,   362,   262,     0,    34,    35,     0,   247,   336,
     337,   338,   339,   340,   341,   342,   267,   268,     0,   237,
       0,   266,   269,     0,   241,     0,     0,   254,   245,     0,
     250,   252,   260,   333,     0,     0,   258,   260,   366,   367,
     368,   372,   373,   374,   375,   376,   377,   378,   379,   380,
     381,   382,   383,   384,   385,   386,   369,   370,   371,   387,
     388,   276,     0,   239,   287,   288,   173,     0,     0,     0,
       0,     0,     0,   256,     0,     0,     0,   306,     0,     0,
       0,   279,   277,   281,   283,   285,   365,     0,     0,     0,
     270,     0,     0,   275,   293,     0,   243,   244,   255,     0,
       0,   311,   311,     0,   334,   335,     0,     0,     0,   248,
     259,     0,     0,     0,     0,   306,     0,   185,   186,     0,
       0,     0,     0,   291,     0,   343,     0,   246,   257,   249,
       0,   312,   315,     0,   261,   307,   308,     0,     0,     0,
     289,     0,     0,     0,     0,   240,   289,     0,     0,     0,
     306,   242,   173,     0,     0,     0,     0,   323,   313,   316,
       0,    36,     0,     0,   290,     0,     0,     0,     0,   286,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   164,
     152,   174,   166,     0,     0,   311,     0,    38,     0,     0,
     280,   278,   282,   284,   272,   271,   273,   274,   292,   187,
     165,   167,     0,     0,     0,   344,   345,   346,   347,   348,
     176,   156,     0,   162,   154,     0,   178,   179,   180,   181,
     182,   251,   352,     0,   324,   325,   326,   314,   253,     0,
       0,    41,    44,    53,    85,    39,    42,    45,    46,    48,
      87,     0,     0,   169,     0,     0,     0,   172,   171,     0,
       0,     0,     0,     0,     0,     0,     0,   353,     0,     0,
     110,   112,   117,   115,     0,     0,    37,     0,    86,     0,
      40,     0,    43,     0,    49,    60,    54,    56,     0,    50,
      47,    65,    61,    62,     0,    52,    71,    66,    67,     0,
       0,   159,   168,   161,   160,   170,     0,     0,     0,   175,
     153,   163,   311,   158,     0,   328,   329,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   102,   103,   104,   105,
     106,   107,   108,   109,   101,   100,     0,    89,   317,   317,
       0,   121,   134,   136,     0,    57,    55,    74,    72,     0,
      59,    79,    75,    83,     0,    51,     0,    63,     0,    64,
      82,    80,    69,     0,    70,    68,   189,     0,     0,     0,
       0,     0,   327,   330,   331,    88,    90,     0,   318,   321,
       0,     0,   114,   118,     0,     0,   122,   124,   138,   138,
      58,    73,     0,    76,    77,   130,    81,     0,     0,    20,
       0,     0,     0,    13,     0,    32,     8,   177,   157,     0,
     111,   319,   322,   113,   119,   128,   116,   126,   121,   125,
       0,   145,   145,    78,     0,     0,     0,     0,   194,     0,
     232,     0,     0,    30,     0,     0,     0,     0,     0,     0,
      23,     9,    10,    12,     0,   317,   311,   311,     0,   123,
     311,     0,     0,     0,     0,     0,   141,   142,   145,   144,
     146,     0,     0,    84,   131,   198,   211,     0,   193,   190,
       0,   232,     0,   195,   227,     0,     0,    28,    31,     0,
      18,    14,    15,    16,    17,    21,   155,   320,     0,     0,
     351,   127,   332,     0,   147,   148,     0,     0,   135,   140,
     143,   137,   132,   317,     0,   200,   206,   265,   196,     0,
       0,     0,     0,    26,     0,     0,    22,   120,   129,   139,
       0,   184,     0,     0,   311,     0,     0,     0,     0,   254,
       0,   263,   204,     0,   349,   350,   235,   233,   354,     0,
       0,     0,    27,     0,    19,   151,   183,     0,   149,     0,
     199,     0,     0,   215,   217,   221,     0,   216,     0,     0,
     305,     0,     0,     0,   309,     0,     0,   232,     0,     0,
       0,     0,     0,     0,   150,   133,     0,   219,   209,   214,
     210,   303,     0,   202,   212,   208,     0,     0,     0,   201,
     205,   232,     0,     0,   232,   224,   230,   300,   294,     0,
       0,   299,     0,   192,    24,     0,    29,     0,     0,   309,
     207,     0,   213,   264,   310,     0,   225,     0,     0,     0,
     226,     0,     0,     0,     0,   236,   234,    25,   223,     0,
       0,   218,   220,     0,   203,     0,     0,     0,   301,     0,
       0,     0,   222,   304,   229,   232,   228,   302,     0,     0,
       0,     0,     0,   231,   296,   295,   297,   298
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -514,  -514,  -514,  -514,  -514,  -514,  -514,  -398,  -514,   194,
    -514,  -514,  -514,  -514,  -514,  -514,  -514,  -514,   165,  -514,
    -514,  -514,  -514,   356,   354,  -514,   313,  -514,  -281,   309,
     315,  -280,  -514,   -14,  -174,   407,  -514,   266,  -514,  -514,
    -208,  -514,  -201,  -177,  -514,  -514,  -514,  -514,   187,  -514,
    -514,  -514,   221,  -514,  -514,  -514,  -514,  -514,  -514,  -514,
     222,   188,   174,   197,  -514,  -514,  -514,  -156,  -514,  -514,
    -514,   438,  -514,  -514,  -514,   -42,   453,  -514,  -514,  -460,
      -1,  -514,  -514,  -514,   256,   238,  -514,  -514,   204,  -514,
    -514,  -514,   102,  -514,  -514,  -514,  -495,  -514,    57,  -514,
     200,  -514,  -381,  -433,  -514,  -514,   -16,  -514,  -514,  -514,
    -514,  -514,  -514,   143,  -514,  -514,  -162,  -514,  -466,  -514,
    -514,     8,   576,  -514,  -514,  -514,  -514,  -514,   519,   421,
    -514,  -251,  -514,  -513,  -514,  -113,    67,  -117,  -514,  -514,
    -341,  -514,  -514,   299,  -514,  -514,  -514,  -514,  -514,  -514,
    -514,   494,  -180,  -192,  -514,  -514,  -514,   497,  -100,  -514,
    -514,  -514,  -514,  -514,  -514,  -514,  -514,  -255,  -514,  -514,
    -195,   -18,   -58,   -36,     3
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     3,     4,     8,   378,   411,   412,   413,
     446,   495,   523,   552,   447,   448,   487,   449,   414,     6,
      25,   207,   250,   251,   252,   294,   299,   305,   355,   360,
     369,   300,   405,   253,   254,   436,   346,   347,   256,   348,
     257,   349,   258,   259,   351,   350,   393,   456,   395,   396,
     428,   458,   397,   457,   434,   474,   534,   260,   398,   399,
     431,   465,   466,   467,   468,   469,   470,   199,   275,   271,
     274,   173,   264,   269,   266,   174,   200,   270,   230,   530,
     531,    26,   407,   408,   437,   438,   439,   513,   440,   542,
     621,   575,   515,   573,   562,   566,   563,   618,   564,   639,
     483,   604,   605,   606,   579,   578,    20,   107,    89,    93,
      56,    95,    96,    92,   119,   100,    21,    97,   565,   574,
      44,   175,    83,   132,   131,   133,   134,   135,   185,   113,
     170,   610,   658,   611,   619,   123,   597,   150,   205,   151,
     387,   455,   388,   178,   204,   244,   324,   501,    54,   126,
     127,    45,    46,    47,    48,    49,    50,    51,   540,   236,
     237,   238,   239,   240,   546,   547,   502,   245,   278,   549,
      22,    23,   114,   106,   152
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      52,    87,    43,    19,    84,   153,   233,   484,   390,   246,
     516,    -2,   201,   201,   452,   146,   356,   409,   232,   202,
      57,   367,   164,   325,   105,   569,    80,    88,   375,    90,
      91,   283,   363,    94,   201,   201,   489,   628,   520,     5,
     220,   221,   567,    85,   177,   108,   109,   533,   484,   576,
     302,   307,   490,   143,   284,   285,   296,   194,   308,  -262,
     201,   201,   201,   128,   629,   577,   263,   589,   268,   383,
     273,    84,   557,   617,  -262,   400,   118,   594,   120,   288,
     297,   303,   403,   326,   130,   232,   463,   155,   247,     7,
     157,   158,   302,    24,    -2,   371,   362,   112,   235,   410,
     371,   622,   201,   148,   124,   125,   362,   201,   312,   516,
      85,    27,   201,   315,   497,   461,   462,   463,   321,   358,
     144,   288,    86,   303,    53,   358,   156,   288,   155,   384,
     464,   196,   288,   462,   463,     9,    10,    11,   595,   160,
     161,   162,   163,     9,    10,    11,    55,   166,   167,   168,
     169,   279,   581,    98,   179,   583,   201,   394,   155,   427,
     203,   464,   380,    99,   206,   282,   283,   156,   184,   186,
     187,   188,   535,   262,   184,   191,   192,   193,   464,   358,
      57,   197,    86,   288,   615,   475,   476,   288,    31,   284,
     285,    12,    13,    14,    15,    16,   288,   156,    17,    12,
      13,    14,    15,    16,   282,   381,    17,   115,   279,   280,
     281,   282,   283,   281,   282,     9,    10,    11,   586,   519,
     626,   520,   663,   630,   121,   309,   586,   288,   284,   285,
     288,   602,   288,   603,   196,   284,   285,   116,   284,   285,
     122,   536,   537,   295,   301,   306,   137,    57,   352,   353,
     280,   281,   282,   283,    57,   318,   138,   201,   143,   288,
      31,   284,   285,   454,   145,   145,   149,     9,    10,    11,
     281,    12,    13,    14,    15,    16,   284,   285,    17,   281,
     282,   283,   357,   361,   197,   159,   301,    80,   357,   370,
     281,   165,   283,   361,   370,   284,   285,   283,   538,   171,
     201,   391,   392,   176,   284,   285,   505,   180,     9,    10,
      11,   181,   389,   389,   532,   284,   285,   472,   473,   182,
     284,   285,   539,    12,    13,   379,   242,    16,   612,   183,
      17,   475,   476,   208,   536,   537,   425,   363,   210,   498,
     499,   415,   357,   503,   401,   544,   545,   243,   404,   628,
     520,   209,   422,     9,    10,    11,   211,   406,   145,   284,
     285,   261,   212,   213,    12,    13,    14,    15,    16,   587,
     277,    17,   214,   424,   450,   249,   415,   646,   215,   216,
     648,   649,   650,   651,    58,    59,    60,   217,   219,   433,
     430,   538,   241,     9,    10,    11,   248,   657,   659,   660,
     661,   286,   287,   289,     9,    10,    11,   291,   415,    12,
      13,    14,    15,    16,   293,   298,    17,   559,   304,   389,
     310,   313,   319,   322,   354,   359,   486,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,   504,   364,    79,   366,    12,
      13,    14,    15,    16,   512,   368,    17,   373,   517,   526,
      12,    13,    14,    15,    16,   281,   101,    17,   363,   376,
     102,   103,   104,    38,   117,   377,   281,   389,   283,   363,
     570,   394,   402,   435,    38,   129,   416,   417,   418,   419,
     284,   285,   420,   423,   426,   451,   541,   460,   572,   477,
     480,   284,   285,   543,   479,   482,   500,   485,   553,   491,
     591,   492,   493,   494,   415,   496,   506,   415,   517,   517,
     570,   570,   507,   476,   508,   511,   521,   522,   570,   556,
     524,   596,   556,   525,   527,   528,   529,   550,     9,    10,
      11,   548,   551,   541,   517,   580,   415,   554,   560,   582,
     585,   591,   592,   625,   517,   118,   556,   517,   590,   601,
     613,   624,   614,   616,   620,   627,   638,   635,   645,   636,
     570,   637,   644,   570,   570,   570,   570,   647,   517,   662,
     541,    58,    59,    60,   520,   652,   654,   655,   656,   664,
     570,   570,   570,   570,    12,    13,    14,    15,    16,   665,
     517,    17,    58,    59,    60,   666,   667,   624,   453,   488,
     292,   290,   386,   365,   255,   459,   372,   429,    38,   147,
     471,   432,   541,   374,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,   509,   481,    79,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,   139,   441,    79,   510,   140,   141,   142,
       9,    10,    11,   276,   478,   642,   267,   600,     9,    10,
      11,   518,   571,   136,   631,   190,   643,   421,   632,   633,
     634,   317,   231,     0,     0,   234,    28,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    29,    30,    31,    32,    33,    34,    35,
       9,    10,    11,     0,     0,     0,    12,    13,    14,    15,
      16,    36,    37,    17,    12,    13,    14,    15,    16,     0,
       0,    17,     0,     0,     0,     0,    28,     0,     0,     0,
      38,    39,     0,    40,    41,    42,     0,     0,   198,   311,
       0,     0,     0,     0,    30,    31,    32,    33,    34,    35,
     409,     0,    58,    59,    60,     0,    12,    13,    14,    15,
      16,    36,    37,    17,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    39,     0,    40,    41,    42,     0,     0,     0,     0,
       0,    58,    59,    60,     0,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,     0,     0,    79,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     9,    10,    11,     0,
       0,     0,   410,   -11,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,   222,   223,    79,    58,    59,    60,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    29,
      30,    31,   224,   177,     0,    35,   145,   225,   226,   227,
     228,   229,    12,    13,    14,    15,    16,     0,     0,    17,
       0,     0,     0,     0,    58,    59,    60,     0,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,     0,     0,    79,     0,
     172,     9,    10,    11,     0,     0,     9,    10,    11,     0,
       0,     0,     0,     0,     0,   111,   195,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,     0,     0,    79,     0,   172,
      58,    59,    60,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   111,   323,     0,    12,    13,     0,
     242,    16,    12,    13,    17,   242,    16,     0,     0,    17,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    58,
      59,    60,   382,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,     0,     0,    79,    80,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    58,    59,
      60,   154,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
       0,     0,    79,    80,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    58,    59,    60,
     189,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,     0,
       0,    79,    80,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    58,    59,    60,   218,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,     0,     0,
      79,     0,     9,    10,    11,     0,     0,     0,     0,     0,
       9,    10,    11,     0,     0,     0,     0,     0,   623,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,     0,     0,    79,
       9,    10,    11,     0,     0,     0,     0,     0,     9,    10,
      11,     0,     0,     0,     0,     0,     0,   653,    12,    13,
      14,    15,    16,     0,     0,    17,    12,    13,    14,    15,
      16,     0,     0,    17,     9,    10,    11,     0,     0,     0,
       0,     0,   198,   314,     9,    10,    11,     0,     0,     0,
     198,   320,     0,     9,    10,    11,    12,    13,    14,    15,
      16,     0,     0,    17,    12,    13,    14,    15,    16,     0,
       0,    17,     9,    10,    11,     0,     0,     0,     0,     0,
      18,   555,     9,    10,    11,     0,     0,     0,    18,   558,
      12,    13,    14,    15,    16,     0,     0,    17,     0,     0,
      12,    13,    14,    15,    16,     0,     0,    17,     0,    12,
      13,    14,    15,    16,    18,   584,    17,     0,     0,     0,
       0,    58,    59,    60,   561,   588,     0,     0,    12,    13,
      14,    15,    16,    38,   593,    17,     0,     0,    12,    13,
      14,    15,    16,     0,     0,    17,     0,     0,     9,    10,
      11,     0,   598,   599,     0,     0,     0,     0,     0,     0,
       0,     0,   640,   641,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,     0,     0,    79,    80,    81,    58,    59,    60,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    82,     0,     0,    12,    13,    14,    15,    16,     0,
       0,    17,     0,    86,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    58,    59,    60,    38,   607,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,     0,     0,
      79,     0,   608,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    58,    59,    60,     0,   609,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,     0,     0,    79,     0,
     110,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    58,    59,    60,     0,   111,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,     0,     0,    79,     0,   172,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    58,
      59,    60,     0,   111,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,     0,     0,    79,     9,    10,    11,     0,     0,
       0,     0,     0,     9,    10,    11,     0,     0,     0,     0,
       0,   316,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
       0,     0,    79,     9,    10,    11,     0,     0,     0,     0,
      31,     9,    10,    11,     0,     0,     0,     0,     0,   568,
       0,    12,    13,    14,    15,    16,     0,     0,    17,    12,
      13,    14,    15,    16,     0,     0,    17,     9,    10,    11,
       0,     0,     0,     0,     0,    18,     9,    10,    11,     0,
       0,     0,     0,    18,     0,     9,    10,    11,     0,    12,
      13,    14,    15,    16,     0,     0,    17,    12,    13,    14,
      15,    16,     0,     0,    17,     0,     0,     0,     0,     0,
       9,    10,    11,    38,     0,     0,     0,     0,     0,     0,
       0,   198,     0,    12,    13,    14,    15,    16,     0,     0,
      17,     0,    12,    13,    14,    15,    16,     0,     0,    17,
       0,    12,    13,    14,    15,    16,     0,   265,    17,    58,
      59,    60,     0,     0,     0,     0,   272,     0,     0,     0,
       0,     0,     0,     0,     0,   514,    12,    13,    14,    15,
      16,     0,     0,    17,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     561,     0,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
       0,     0,    79,    58,    59,    60,     0,     0,     0,     0,
       0,   442,     0,     0,   443,   444,   445,   327,   328,   329,
     330,   331,   332,   333,   334,   335,   336,   337,   338,   339,
     340,   341,   342,   343,     0,     0,   344,   345,     0,     9,
      10,    11,     0,     0,     0,     0,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,     0,     0,    79,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    30,     0,     0,     0,     0,     0,   385,
     225,   226,   227,   228,   229,    12,    13,    14,    15,    16,
       0,     0,    17,   327,   328,   329,   330,   331,   332,   333,
     334,   335,   336,   337,   338,   339,   340,   341,   342,   343,
       0,     0,   344,   345
};

static const yytype_int16 yycheck[] =
{
      18,    43,    18,     4,    40,   122,   198,   440,   349,   204,
     476,     0,   174,   175,   412,   115,   296,     1,   198,   175,
      38,   302,   135,   278,    82,   538,    67,    43,   308,    45,
      46,    10,    11,    49,   196,   197,    68,    43,    44,     0,
     196,   197,   537,    40,    85,    87,    88,   507,   481,    68,
     258,   259,    84,   111,    33,    34,   257,   170,   259,    68,
     222,   223,   224,    99,    70,    84,   222,   562,   224,   324,
     232,   107,   532,   586,    83,   356,    92,   572,    94,   253,
     257,   258,   362,   278,   100,   265,    38,   123,   205,    83,
     126,   127,   300,    83,    83,   303,   297,    89,   198,    83,
     308,   596,   264,   119,     4,     5,   307,   269,   264,   575,
     107,     3,   274,   269,   455,    36,    37,    38,   274,   296,
     112,   295,    68,   300,     3,   302,   123,   301,   164,   324,
      82,   173,   306,    37,    38,     3,     4,     5,    84,   131,
     132,   133,   134,     3,     4,     5,    83,   139,   140,   141,
     142,     6,   550,    83,   151,   553,   318,    83,   194,    85,
     176,    82,   318,    73,   180,     9,    10,   164,   160,   161,
     162,   163,   513,   209,   166,   167,   168,   169,    82,   356,
     198,   173,    68,   357,   582,    40,    41,   361,    48,    33,
      34,    59,    60,    61,    62,    63,   370,   194,    66,    59,
      60,    61,    62,    63,     9,   322,    66,    83,     6,     7,
       8,     9,    10,     8,     9,     3,     4,     5,    86,    42,
     601,    44,   655,   604,    83,   261,    86,   401,    33,    34,
     404,    81,   406,    83,   276,    33,    34,    84,    33,    34,
      83,    29,    30,   257,   258,   259,    84,   265,   284,   285,
       7,     8,     9,    10,   272,   271,    84,   419,   316,   433,
      48,    33,    34,   419,    53,    53,    84,     3,     4,     5,
       8,    59,    60,    61,    62,    63,    33,    34,    66,     8,
       9,    10,   296,   297,   276,    84,   300,    67,   302,   303,
       8,    84,    10,   307,   308,    33,    34,    10,    86,    84,
     462,    83,    84,    84,    33,    34,   462,    84,     3,     4,
       5,    84,   348,   349,   506,    33,    34,    83,    84,    84,
      33,    34,   514,    59,    60,   317,    62,    63,   579,    83,
      66,    40,    41,    83,    29,    30,   394,    11,    84,   456,
     457,   377,   356,   460,   358,    59,    60,    83,   362,    43,
      44,    74,   388,     3,     4,     5,    84,   371,    53,    33,
      34,    39,    84,    84,    59,    60,    61,    62,    63,   561,
      63,    66,    84,   391,   410,    83,   412,   628,    84,    84,
     631,   632,   633,   634,     3,     4,     5,    84,    84,   403,
      35,    86,    84,     3,     4,     5,    84,   648,   649,   650,
     651,    84,    83,    83,     3,     4,     5,    83,   444,    59,
      60,    61,    62,    63,    83,    83,    66,   534,    83,   455,
      84,    84,    84,    83,    83,    83,   442,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,   461,    83,    66,    83,    59,
      60,    61,    62,    63,   472,    83,    66,    83,   476,   495,
      59,    60,    61,    62,    63,     8,    85,    66,    11,    84,
      89,    90,    91,    83,    84,    83,     8,   513,    10,    11,
     538,    83,    83,    83,    83,    84,    84,    84,    84,    84,
      33,    34,    84,    84,    84,    84,   514,    83,   540,    83,
      83,    33,    34,   519,    84,    83,    61,    84,   524,    84,
     568,    84,    84,    84,   550,    84,    83,   553,   536,   537,
     578,   579,    83,    41,    84,    84,    83,    83,   586,   530,
      83,   573,   533,    68,    84,    84,    84,    76,     3,     4,
       5,    72,    83,   561,   562,    68,   582,    84,    84,    77,
      84,   609,    68,    83,   572,   571,   557,   575,    84,    84,
      84,   597,    84,    84,    84,    70,    68,    84,    68,    84,
     628,    84,    84,   631,   632,   633,   634,    68,   596,    83,
     598,     3,     4,     5,    44,    84,    84,    84,    84,    84,
     648,   649,   650,   651,    59,    60,    61,    62,    63,    84,
     618,    66,     3,     4,     5,    84,    84,   643,   414,   444,
     256,   255,   346,   300,   207,   428,   307,   396,    83,    84,
     432,   399,   640,   308,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,   468,   439,    66,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    85,   408,    66,   469,    89,    90,    91,
       3,     4,     5,   235,   436,   618,   223,   575,     3,     4,
       5,   481,   539,   107,    85,   166,   619,   388,    89,    90,
      91,   270,   198,    -1,    -1,   198,    29,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    46,    47,    48,    49,    50,    51,    52,
       3,     4,     5,    -1,    -1,    -1,    59,    60,    61,    62,
      63,    64,    65,    66,    59,    60,    61,    62,    63,    -1,
      -1,    66,    -1,    -1,    -1,    -1,    29,    -1,    -1,    -1,
      83,    84,    -1,    86,    87,    88,    -1,    -1,    83,    84,
      -1,    -1,    -1,    -1,    47,    48,    49,    50,    51,    52,
       1,    -1,     3,     4,     5,    -1,    59,    60,    61,    62,
      63,    64,    65,    66,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    84,    -1,    86,    87,    88,    -1,    -1,    -1,    -1,
      -1,     3,     4,     5,    -1,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    -1,    -1,    66,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,     3,     4,     5,    -1,
      -1,    -1,    83,    84,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    29,    30,    66,     3,     4,     5,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,
      47,    48,    49,    85,    -1,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    -1,    -1,    66,
      -1,    -1,    -1,    -1,     3,     4,     5,    -1,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    -1,    -1,    66,    -1,
      68,     3,     4,     5,    -1,    -1,     3,     4,     5,    -1,
      -1,    -1,    -1,    -1,    -1,    83,    84,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    -1,    -1,    66,    -1,    68,
       3,     4,     5,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    83,    84,    -1,    59,    60,    -1,
      62,    63,    59,    60,    66,    62,    63,    -1,    -1,    66,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     3,
       4,     5,    84,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    -1,    -1,    66,    67,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     3,     4,
       5,    84,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      -1,    -1,    66,    67,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,     3,     4,     5,
      84,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    -1,
      -1,    66,    67,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,     3,     4,     5,    84,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    -1,    -1,
      66,    -1,     3,     4,     5,    -1,    -1,    -1,    -1,    -1,
       3,     4,     5,    -1,    -1,    -1,    -1,    -1,    84,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    -1,    -1,    66,
       3,     4,     5,    -1,    -1,    -1,    -1,    -1,     3,     4,
       5,    -1,    -1,    -1,    -1,    -1,    -1,    84,    59,    60,
      61,    62,    63,    -1,    -1,    66,    59,    60,    61,    62,
      63,    -1,    -1,    66,     3,     4,     5,    -1,    -1,    -1,
      -1,    -1,    83,    84,     3,     4,     5,    -1,    -1,    -1,
      83,    84,    -1,     3,     4,     5,    59,    60,    61,    62,
      63,    -1,    -1,    66,    59,    60,    61,    62,    63,    -1,
      -1,    66,     3,     4,     5,    -1,    -1,    -1,    -1,    -1,
      83,    84,     3,     4,     5,    -1,    -1,    -1,    83,    84,
      59,    60,    61,    62,    63,    -1,    -1,    66,    -1,    -1,
      59,    60,    61,    62,    63,    -1,    -1,    66,    -1,    59,
      60,    61,    62,    63,    83,    84,    66,    -1,    -1,    -1,
      -1,     3,     4,     5,    83,    84,    -1,    -1,    59,    60,
      61,    62,    63,    83,    84,    66,    -1,    -1,    59,    60,
      61,    62,    63,    -1,    -1,    66,    -1,    -1,     3,     4,
       5,    -1,    83,    84,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    83,    84,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    -1,    -1,    66,    67,    68,     3,     4,     5,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    83,    -1,    -1,    59,    60,    61,    62,    63,    -1,
      -1,    66,    -1,    68,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,     3,     4,     5,    83,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    -1,    -1,
      66,    -1,    68,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,     3,     4,     5,    -1,    83,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    -1,    -1,    66,    -1,
      68,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,     3,     4,     5,    -1,    83,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    -1,    -1,    66,    -1,    68,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     3,
       4,     5,    -1,    83,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    -1,    -1,    66,     3,     4,     5,    -1,    -1,
      -1,    -1,    -1,     3,     4,     5,    -1,    -1,    -1,    -1,
      -1,    83,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      -1,    -1,    66,     3,     4,     5,    -1,    -1,    -1,    -1,
      48,     3,     4,     5,    -1,    -1,    -1,    -1,    -1,    83,
      -1,    59,    60,    61,    62,    63,    -1,    -1,    66,    59,
      60,    61,    62,    63,    -1,    -1,    66,     3,     4,     5,
      -1,    -1,    -1,    -1,    -1,    83,     3,     4,     5,    -1,
      -1,    -1,    -1,    83,    -1,     3,     4,     5,    -1,    59,
      60,    61,    62,    63,    -1,    -1,    66,    59,    60,    61,
      62,    63,    -1,    -1,    66,    -1,    -1,    -1,    -1,    -1,
       3,     4,     5,    83,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    83,    -1,    59,    60,    61,    62,    63,    -1,    -1,
      66,    -1,    59,    60,    61,    62,    63,    -1,    -1,    66,
      -1,    59,    60,    61,    62,    63,    -1,    83,    66,     3,
       4,     5,    -1,    -1,    -1,    -1,    83,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    83,    59,    60,    61,    62,
      63,    -1,    -1,    66,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      83,    -1,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      -1,    -1,    66,     3,     4,     5,    -1,    -1,    -1,    -1,
      -1,    75,    -1,    -1,    78,    79,    80,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    -1,    -1,    31,    32,    -1,     3,
       4,     5,    -1,    -1,    -1,    -1,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    -1,    -1,    66,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    47,    -1,    -1,    -1,    -1,    -1,    84,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      -1,    -1,    66,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      -1,    -1,    31,    32
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_int16 yystos[] =
{
       0,    93,    94,    95,    96,     0,   111,    83,    97,     3,
       4,     5,    59,    60,    61,    62,    63,    66,    83,   172,
     198,   208,   262,   263,    83,   112,   173,     3,    29,    46,
      47,    48,    49,    50,    51,    52,    64,    65,    83,    84,
      86,    87,    88,   198,   212,   243,   244,   245,   246,   247,
     248,   249,   263,     3,   240,    83,   202,   263,     3,     4,
       5,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    66,
      67,    68,    83,   214,   265,   266,    68,   167,   198,   200,
     198,   198,   205,   201,   198,   203,   204,   209,    83,    73,
     207,    85,    89,    90,    91,   264,   265,   199,   167,   167,
      68,    83,   213,   221,   264,    83,    84,    84,   198,   206,
     198,    83,    83,   227,     4,     5,   241,   242,   265,    84,
     198,   216,   215,   217,   218,   219,   214,    84,    84,    85,
      89,    90,    91,   264,   213,    53,   250,    84,   198,    84,
     229,   231,   266,   229,    84,   265,   266,   265,   265,    84,
     213,   213,   213,   213,   227,    84,   213,   213,   213,   213,
     222,    84,    68,   163,   167,   213,    84,    85,   235,   266,
      84,    84,    84,    83,   213,   220,   213,   213,   213,    84,
     220,   213,   213,   213,   227,    84,   167,   213,    83,   159,
     168,   208,   159,   198,   236,   230,   198,   113,    83,    74,
      84,    84,    84,    84,    84,    84,    84,    84,    84,    84,
     159,   159,    29,    30,    49,    54,    55,    56,    57,    58,
     170,   243,   244,   245,   249,   250,   251,   252,   253,   254,
     255,    84,    62,    83,   237,   259,   262,   229,    84,    83,
     114,   115,   116,   125,   126,   127,   130,   132,   134,   135,
     149,    39,   265,   159,   164,    83,   166,   168,   159,   165,
     169,   161,    83,   208,   162,   160,   163,    63,   260,     6,
       7,     8,     9,    10,    33,    34,    84,    83,   126,    83,
     115,    83,   116,    83,   117,   125,   134,   135,    83,   118,
     123,   125,   132,   135,    83,   119,   125,   132,   134,   265,
      84,    84,   159,    84,    84,   159,    83,   221,   198,    84,
      84,   159,    83,    84,   238,   259,   262,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    31,    32,   128,   129,   131,   133,
     137,   136,   265,   265,    83,   120,   123,   125,   135,    83,
     121,   125,   134,    11,    83,   118,    83,   120,    83,   122,
     125,   132,   121,    83,   122,   123,    84,    83,    98,   213,
     159,   229,    84,   259,   262,    84,   129,   232,   234,   265,
     232,    83,    84,   138,    83,   140,   141,   144,   150,   151,
     120,   125,    83,   123,   125,   124,   125,   174,   175,     1,
      83,    99,   100,   101,   110,   265,    84,    84,    84,    84,
      84,   235,   265,    84,   263,   264,    84,    85,   142,   144,
      35,   152,   152,   125,   146,    83,   127,   176,   177,   178,
     180,   176,    75,    78,    79,    80,   102,   106,   107,   109,
     265,    84,    99,   101,   159,   233,   139,   145,   143,   140,
      83,    36,    37,    38,    82,   153,   154,   155,   156,   157,
     158,   153,    83,    84,   147,    40,    41,    83,   177,    84,
      83,   180,    83,   192,   195,    84,   198,   108,   110,    68,
      84,    84,    84,    84,    84,   103,    84,   232,   229,   229,
      61,   239,   258,   229,   198,   159,    83,    83,    84,   154,
     155,    84,   263,   179,    83,   184,   210,   263,   192,    42,
      44,    83,    83,   104,    83,    68,   265,    84,    84,    84,
     171,   172,   245,   171,   148,   232,    29,    30,    86,   245,
     250,   263,   181,   198,    59,    60,   256,   257,    72,   261,
      76,    83,   105,   198,    84,    84,   172,   171,    84,   229,
      84,    83,   186,   188,   190,   210,   187,   188,    83,   225,
     264,   205,   167,   185,   211,   183,    68,    84,   197,   196,
      68,    99,    77,    99,    84,    84,    86,   245,    84,   188,
      84,   264,    68,    84,   188,    84,   167,   228,    83,    84,
     184,    84,    81,    83,   193,   194,   195,    45,    68,    83,
     223,   225,   223,    84,    84,    99,    84,   225,   189,   226,
      84,   182,   188,    84,   265,    83,   194,    70,    43,    70,
     194,    85,    89,    90,    91,    84,    84,    84,    68,   191,
      83,    84,   190,   228,    84,    68,   223,    68,   223,   223,
     223,   223,    84,    84,    84,    84,    84,   223,   224,   223,
     223,   223,    83,   195,    84,    84,    84,    84
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_int16 yyr1[] =
{
       0,    92,    94,    93,    95,    93,    96,    93,    97,    98,
      99,    99,   100,   100,   101,   101,   101,   101,   101,   101,
     101,   102,   103,   103,   104,   105,   106,   106,   107,   108,
     108,   109,   110,   111,   111,   111,   113,   112,   114,   114,
     114,   114,   115,   115,   115,   116,   116,   116,   116,   116,
     116,   116,   116,   116,   117,   117,   117,   117,   117,   117,
     117,   118,   118,   118,   118,   118,   119,   119,   119,   119,
     119,   119,   120,   120,   120,   121,   121,   121,   121,   121,
     122,   122,   122,   124,   123,   125,   125,   126,   127,   128,
     128,   129,   129,   129,   129,   129,   129,   129,   129,   129,
     129,   129,   129,   129,   129,   129,   129,   129,   129,   129,
     131,   130,   133,   132,   134,   136,   135,   137,   137,   139,
     138,   140,   140,   140,   141,   141,   143,   142,   145,   144,
     146,   146,   148,   147,   150,   149,   151,   149,   152,   152,
     153,   153,   154,   154,   154,   155,   155,   156,   157,   158,
     158,   158,   159,   159,   160,   159,   161,   159,   159,   159,
     159,   159,   162,   162,   163,   163,   163,   163,   164,   164,
     165,   165,   166,   167,   168,   168,   169,   168,   170,   170,
     170,   170,   170,   171,   171,   172,   172,   172,   172,   174,
     173,   175,   173,   176,   176,   177,   177,   178,   179,   178,
     181,   180,   182,   180,   183,   183,   184,   184,   184,   184,
     184,   184,   185,   185,   186,   186,   187,   188,   188,   189,
     189,   190,   190,   191,   192,   192,   192,   192,   193,   193,
     194,   194,   195,   196,   195,   197,   195,   198,   198,   199,
     198,   200,   198,   198,   198,   201,   198,   202,   198,   198,
     203,   198,   204,   198,   205,   205,   206,   206,   207,   207,
     209,   208,   208,   211,   210,   210,   212,   212,   212,   212,
     213,   213,   213,   213,   213,   213,   214,   215,   214,   216,
     214,   217,   214,   218,   214,   219,   214,   214,   214,   220,
     220,   222,   221,   221,   223,   223,   223,   223,   223,   223,
     223,   224,   224,   226,   225,   225,   227,   227,   227,   228,
     228,   229,   229,   230,   229,   231,   231,   232,   232,   233,
     232,   234,   234,   236,   235,   237,   237,   237,   238,   238,
     238,   238,   239,   240,   241,   242,   243,   244,   245,   246,
     247,   248,   249,   250,   251,   252,   253,   254,   255,   256,
     257,   258,   259,   260,   261,   262,   262,   262,   262,   262,
     262,   262,   263,   263,   263,   264,   265,   265,   265,   265,
     265,   265,   265,   265,   265,   265,   265,   265,   265,   265,
     265,   265,   265,   265,   265,   265,   265,   265,   266
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     0,     2,     0,     2,     0,     2,    12,     3,
       2,     0,     2,     1,     3,     3,     3,     3,     3,     5,
       1,     2,     2,     0,     4,     4,     3,     4,     2,     5,
       0,     2,     1,     0,     2,     2,     0,     9,     0,     1,
       2,     1,     1,     2,     1,     1,     1,     2,     1,     2,
       2,     3,     2,     1,     1,     2,     1,     2,     3,     2,
       1,     1,     1,     2,     2,     1,     1,     1,     2,     2,
       2,     1,     1,     2,     1,     1,     2,     2,     3,     1,
       1,     2,     1,     0,     5,     1,     2,     1,     4,     1,
       2,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       0,     5,     0,     5,     4,     0,     5,     0,     2,     0,
       5,     0,     1,     3,     1,     2,     0,     3,     0,     5,
       0,     2,     0,     5,     0,     7,     0,     7,     0,     4,
       2,     1,     1,     2,     1,     0,     1,     2,     2,     4,
       5,     4,     1,     4,     0,     8,     0,     6,     4,     4,
       4,     4,     0,     2,     2,     3,     2,     3,     2,     1,
       2,     1,     1,     1,     1,     4,     0,     6,     1,     1,
       1,     1,     1,     2,     1,     5,     5,     8,     1,     0,
      13,     0,    17,     2,     1,     2,     3,     0,     0,     5,
       0,     6,     0,     8,     0,     2,     1,     5,     4,     4,
       4,     0,     2,     3,     2,     1,     1,     1,     4,     0,
       2,     1,     5,     1,     5,     6,     6,     1,     4,     4,
       1,     5,     0,     0,     6,     0,     6,     2,     1,     0,
       6,     0,     6,     4,     4,     0,     5,     0,     5,     5,
       0,     8,     0,     8,     0,     2,     0,     2,     0,     2,
       0,     5,     1,     0,     5,     1,     1,     1,     1,     1,
       1,     5,     5,     5,     5,     1,     1,     0,     6,     0,
       6,     0,     6,     0,     6,     0,     5,     1,     1,     0,
       1,     0,     5,     1,     1,     5,     5,     5,     5,     1,
       1,     0,     1,     0,     5,     1,     0,     2,     2,     0,
       2,     0,     1,     0,     4,     1,     2,     0,     1,     0,
       4,     1,     2,     0,     3,     1,     1,     4,     1,     1,
       2,     2,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256



/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)

/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo, int yytype, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YYUSE (yyoutput);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyo, yytoknum[yytype], *yyvaluep);
# endif
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo, int yytype, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyo, yytype, yyvaluep);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp, int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[+yyssp[yyi + 1 - yynrhs]],
                       &yyvsp[(yyi + 1) - (yynrhs)]
                                              );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen(S) (YY_CAST (YYPTRDIFF_T, strlen (S)))
#  else
/* Return the length of YYSTR.  */
static YYPTRDIFF_T
yystrlen (const char *yystr)
{
  YYPTRDIFF_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYPTRDIFF_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYPTRDIFF_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            else
              goto append;

          append:
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (yyres)
    return yystpcpy (yyres, yystr) - yyres;
  else
    return yystrlen (yystr);
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYPTRDIFF_T *yymsg_alloc, char **yymsg,
                yy_state_t *yyssp, int yytoken)
{
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat: reported tokens (one for the "unexpected",
     one per "expected"). */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Actual size of YYARG. */
  int yycount = 0;
  /* Cumulated lengths of YYARG.  */
  YYPTRDIFF_T yysize = 0;

  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[+*yyssp];
      YYPTRDIFF_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
      yysize = yysize0;
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYPTRDIFF_T yysize1
                    = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
                    yysize = yysize1;
                  else
                    return 2;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
    default: /* Avoid compiler warnings. */
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    /* Don't count the "%s"s in the final size, but reserve room for
       the terminator.  */
    YYPTRDIFF_T yysize1 = yysize + (yystrlen (yyformat) - 2 * yycount) + 1;
    if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
      yysize = yysize1;
    else
      return 2;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          ++yyp;
          ++yyformat;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
{
  YYUSE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    yy_state_fast_t yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss;
    yy_state_t *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYPTRDIFF_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYPTRDIFF_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    goto yyexhaustedlab;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
# undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 2:
#line 356 "parser.yy"
       { success = true; line_number = 1; }
#line 2525 "parser.cc"
    break;

  case 3:
#line 356 "parser.yy"
                                                                 {}
#line 2531 "parser.cc"
    break;

  case 4:
#line 358 "parser.yy"
   { success = true; line_number = 1; }
#line 2537 "parser.cc"
    break;

  case 5:
#line 358 "parser.yy"
                                               {}
#line 2543 "parser.cc"
    break;

  case 6:
#line 359 "parser.yy"
  { success = true; line_number = 1; }
#line 2549 "parser.cc"
    break;

  case 7:
#line 359 "parser.yy"
                                                   {}
#line 2555 "parser.cc"
    break;

  case 8:
#line 374 "parser.yy"
            {the_plan=(yyvsp[-1].t_plan); (yyval.t_plan)->name = (char*)(yyvsp[-7].str)->c_str(); (yyval.t_plan)->problem_name = (char*)(yyvsp[-3].str)->c_str();}
#line 2561 "parser.cc"
    break;

  case 9:
#line 378 "parser.yy"
{
    //cout << "Plan" << endl;
    (yyval.t_plan) = new plan(); 
    (yyval.t_plan)->begin = (yyvsp[-1].t_instr);
}
#line 2571 "parser.cc"
    break;

  case 10:
#line 388 "parser.yy"
        {
	  //cout << "Steps" << endl;
	  (yyval.t_instr)= (yyvsp[-1].t_instr);
	  (yyval.t_instr)->next = (yyvsp[0].t_instr);
	}
#line 2581 "parser.cc"
    break;

  case 11:
#line 393 "parser.yy"
                { (yyval.t_instr)= 0; }
#line 2587 "parser.cc"
    break;

  case 12:
#line 397 "parser.yy"
{
  //cout << "Label Step" << endl;
  // mark label as associated with this instruction
  (yyvsp[-1].t_label_symbol)->instr = (yyvsp[0].t_instr);
  (yyval.t_instr) = (yyvsp[0].t_instr); 
}
#line 2598 "parser.cc"
    break;

  case 13:
#line 404 "parser.yy"
{ 
  //cout << "Step" << endl;
  (yyval.t_instr) = (yyvsp[0].t_instr); 
}
#line 2607 "parser.cc"
    break;

  case 14:
#line 410 "parser.yy"
                                 { (yyval.t_instr) = (yyvsp[-1].t_instr); }
#line 2613 "parser.cc"
    break;

  case 15:
#line 411 "parser.yy"
                       { (yyval.t_instr) = (yyvsp[-1].t_instr); }
#line 2619 "parser.cc"
    break;

  case 16:
#line 412 "parser.yy"
                         { (yyval.t_instr) = (yyvsp[-1].t_instr); }
#line 2625 "parser.cc"
    break;

  case 17:
#line 413 "parser.yy"
                         { (yyval.t_instr) = (yyvsp[-1].t_instr); }
#line 2631 "parser.cc"
    break;

  case 18:
#line 415 "parser.yy"
{ 
  //cout << "Done" << endl;
  (yyval.t_instr) = new Done(); 
}
#line 2640 "parser.cc"
    break;

  case 19:
#line 420 "parser.yy"
{
  //cout << "LCPP"<<endl;
  (yyval.t_instr) = new Lcp_Done((yyvsp[-2].num)->double_value(), (yyvsp[-1].num)->double_value());
}
#line 2649 "parser.cc"
    break;

  case 20:
#line 425 "parser.yy"
        {std::cout <<"trouble parsing instruction"<<std::endl;}
#line 2655 "parser.cc"
    break;

  case 21:
#line 429 "parser.yy"
  {
    //cout << "Action" << endl;
    std::string tmp;
    tmp += "(";
    tmp += (std::string)*(yyvsp[-1].str);
    std::vector<std::string*>::iterator c;
    for(c=((std::vector<std::string*>*)((yyvsp[0].strs)))->begin(); c!=((std::vector<std::string*>*)((yyvsp[0].strs)))->end(); c++)
      {
	tmp += " ";
	tmp += (**c);
      }
    tmp += ")";

    (yyval.t_instr) = new mAction(findAct(tmp));
  }
#line 2675 "parser.cc"
    break;

  case 22:
#line 447 "parser.yy"
                  {(yyval.strs)=(yyvsp[-1].strs); (yyvsp[-1].strs)->push_back((yyvsp[0].str));}
#line 2681 "parser.cc"
    break;

  case 23:
#line 448 "parser.yy"
              {(yyval.strs)=new   std::vector<const std::string*>;}
#line 2687 "parser.cc"
    break;

  case 24:
#line 452 "parser.yy"
{
  //cout << "Then" << endl;
  (yyval.t_instr) = (yyvsp[-1].t_instr);
}
#line 2696 "parser.cc"
    break;

  case 25:
#line 459 "parser.yy"
{
  //cout << "Else" << endl;
  (yyval.t_instr) = (yyvsp[-1].t_instr);
}
#line 2705 "parser.cc"
    break;

  case 26:
#line 466 "parser.yy"
{
  //cout << "IfThen" << endl;
  (yyval.t_instr) = new IfThen(formula_bdd(*(yyvsp[-1].formula), false),(yyvsp[0].t_instr));
}
#line 2714 "parser.cc"
    break;

  case 27:
#line 471 "parser.yy"
{
  //cout << "IfThenElse" << endl;
  (yyval.t_instr) = new IfThenElse(formula_bdd(*(yyvsp[-2].formula), false),(yyvsp[-1].t_instr),(yyvsp[0].t_instr));
}
#line 2723 "parser.cc"
    break;

  case 28:
#line 478 "parser.yy"
{
  //cout << "Case" << endl;
  (yyval.t_instr) = new Case((yyvsp[0].t_guards));
}
#line 2732 "parser.cc"
    break;

  case 29:
#line 485 "parser.yy"
{
  //cout << "Guards" << endl;
  (yyval.t_guards) = (yyvsp[-4].t_guards);
  (yyval.t_guards)->push_back(std::make_pair(formula_bdd(*(yyvsp[-2].formula), false),(yyvsp[-1].t_instr)));
}
#line 2742 "parser.cc"
    break;

  case 30:
#line 490 "parser.yy"
              { (yyval.t_guards) = new Guards(); }
#line 2748 "parser.cc"
    break;

  case 31:
#line 495 "parser.yy"
{
  //cout << "Goto" << endl;
  (yyval.t_instr) = new Goto((yyvsp[0].t_label_symbol));
}
#line 2757 "parser.cc"
    break;

  case 32:
#line 502 "parser.yy"
{
  // make a label object
  // put/find in a hash table
  (yyval.t_label_symbol) = new label_symbol(*(yyvsp[0].str));//current_analysis->label_tab.symbol_ref($1);
}
#line 2767 "parser.cc"
    break;

  case 34:
#line 510 "parser.yy"
                                                        {std::cout << "finish definition of domain" << std::endl;}
#line 2773 "parser.cc"
    break;

  case 35:
#line 511 "parser.yy"
                                                         {std::cout << "finish definition of problem" << std::endl;}
#line 2779 "parser.cc"
    break;

  case 36:
#line 518 "parser.yy"
                                            { make_domain((yyvsp[-1].str)); }
#line 2785 "parser.cc"
    break;

  case 83:
#line 591 "parser.yy"
                                   {// cout << "ob tokens"<<endl;
}
#line 2792 "parser.cc"
    break;

  case 84:
#line 592 "parser.yy"
                       {// cout << "done ob tokens"<<endl;
}
#line 2799 "parser.cc"
    break;

  case 91:
#line 611 "parser.yy"
                     { requirements->strips = true; }
#line 2805 "parser.cc"
    break;

  case 92:
#line 612 "parser.yy"
                     { requirements->typing = true; }
#line 2811 "parser.cc"
    break;

  case 93:
#line 614 "parser.yy"
                { requirements->negative_preconditions = true; }
#line 2817 "parser.cc"
    break;

  case 94:
#line 616 "parser.yy"
                { requirements->disjunctive_preconditions = true; }
#line 2823 "parser.cc"
    break;

  case 95:
#line 617 "parser.yy"
                       { requirements->equality = true; }
#line 2829 "parser.cc"
    break;

  case 96:
#line 619 "parser.yy"
                { requirements->existential_preconditions = true; }
#line 2835 "parser.cc"
    break;

  case 97:
#line 621 "parser.yy"
                { requirements->universal_preconditions = true; }
#line 2841 "parser.cc"
    break;

  case 98:
#line 623 "parser.yy"
                { requirements->quantified_preconditions(); }
#line 2847 "parser.cc"
    break;

  case 99:
#line 624 "parser.yy"
                                  { requirements->conditional_effects = true; }
#line 2853 "parser.cc"
    break;

  case 100:
#line 625 "parser.yy"
                                     { requirements->probabilistic = true; }
#line 2859 "parser.cc"
    break;

  case 101:
#line 626 "parser.yy"
                                         { // 开启不确定性 :non-deterministic
				requirements->non_deterministic = true;
				requirements->probabilistic_effects = true;// 这里还是开启了概率Effect
				requirements->probabilistic = false;
				}
#line 2869 "parser.cc"
    break;

  case 102:
#line 631 "parser.yy"
                      { requirements->fluents = true; }
#line 2875 "parser.cc"
    break;

  case 103:
#line 632 "parser.yy"
                  { requirements->adl(); }
#line 2881 "parser.cc"
    break;

  case 104:
#line 634 "parser.yy"
                { throw Exception("`:durative-actions' not supported"); }
#line 2887 "parser.cc"
    break;

  case 105:
#line 636 "parser.yy"
                { throw Exception("`:duration-inequalities' not supported"); }
#line 2893 "parser.cc"
    break;

  case 106:
#line 638 "parser.yy"
                { throw Exception("`:continuous-effects' not supported"); }
#line 2899 "parser.cc"
    break;

  case 107:
#line 640 "parser.yy"
                {
					requirements->probabilistic_effects = true;
					requirements->probabilistic = true;
					requirements->non_deterministic = false;// 关闭不确定性
					goal_prob_function = domain->functions().add_function("goal-probability");
				}
#line 2910 "parser.cc"
    break;

  case 108:
#line 647 "parser.yy"
                {
					requirements->rewards = true;
					reward_function = domain->functions().add_function("reward");
				}
#line 2919 "parser.cc"
    break;

  case 109:
#line 652 "parser.yy"
                {
					requirements->probabilistic_effects = true;
					requirements->rewards = true;
					goal_prob_function = domain->functions().add_function("goal-probability");
					reward_function = domain->functions().add_function("reward");
				}
#line 2930 "parser.cc"
    break;

  case 110:
#line 667 "parser.yy"
                      { require_typing(); name_kind = TYPE_KIND; }
#line 2936 "parser.cc"
    break;

  case 111:
#line 668 "parser.yy"
                              { name_kind = VOID_KIND; }
#line 2942 "parser.cc"
    break;

  case 112:
#line 671 "parser.yy"
                              { name_kind = CONSTANT_KIND; }
#line 2948 "parser.cc"
    break;

  case 113:
#line 672 "parser.yy"
                  { name_kind = VOID_KIND; }
#line 2954 "parser.cc"
    break;

  case 115:
#line 678 "parser.yy"
                              { require_fluents(); }
#line 2960 "parser.cc"
    break;

  case 119:
#line 689 "parser.yy"
                               { make_predicate((yyvsp[0].str)); }
#line 2966 "parser.cc"
    break;

  case 120:
#line 690 "parser.yy"
                   { parsing_predicate = false; }
#line 2972 "parser.cc"
    break;

  case 126:
#line 702 "parser.yy"
                         { require_typing(); }
#line 2978 "parser.cc"
    break;

  case 128:
#line 705 "parser.yy"
                             { make_function((yyvsp[0].str)); }
#line 2984 "parser.cc"
    break;

  case 129:
#line 706 "parser.yy"
                  { parsing_function = false; }
#line 2990 "parser.cc"
    break;

  case 132:
#line 714 "parser.yy"
                                        { make_predicate((yyvsp[0].str));
					// make_observable($2); parsing_obs_token = false;
					}
#line 2998 "parser.cc"
    break;

  case 133:
#line 717 "parser.yy"
                                        { //parsing_obs_token = false;
 					  parsing_predicate = false; 
					}
#line 3006 "parser.cc"
    break;

  case 134:
#line 726 "parser.yy"
                             {// cout << *$3<<endl;
               make_action((yyvsp[0].str));  }
#line 3013 "parser.cc"
    break;

  case 135:
#line 728 "parser.yy"
                                          { add_action(); }
#line 3019 "parser.cc"
    break;

  case 136:
#line 729 "parser.yy"
                            {
               make_action((yyvsp[0].str));  }
#line 3026 "parser.cc"
    break;

  case 137:
#line 731 "parser.yy"
                                          { add_event(); }
#line 3032 "parser.cc"
    break;

  case 145:
#line 749 "parser.yy"
                           { std::cout << "empty observations\n"; }
#line 3038 "parser.cc"
    break;

  case 147:
#line 754 "parser.yy"
                                    { if((yyvsp[0].formula) != NULL) action->set_precondition(*(yyvsp[0].formula)); }
#line 3044 "parser.cc"
    break;

  case 148:
#line 757 "parser.yy"
                            { action->set_effect(*(yyvsp[0].effect)); }
#line 3050 "parser.cc"
    break;

  case 149:
#line 763 "parser.yy"
                                               {action->set_observation(*(yyvsp[-1].observation_defs));}
#line 3056 "parser.cc"
    break;

  case 150:
#line 764 "parser.yy"
                                                         {action->set_observation(*(yyvsp[-1].observation_defs));}
#line 3062 "parser.cc"
    break;

  case 151:
#line 765 "parser.yy"
                                                     {action->set_observation(*(yyvsp[-1].observation_defs)); // cout << "parse ob"<<endl;
}
#line 3069 "parser.cc"
    break;

  case 153:
#line 772 "parser.yy"
                                       { (yyval.effect) = (yyvsp[-1].ceffect); }
#line 3075 "parser.cc"
    break;

  case 154:
#line 773 "parser.yy"
                         { prepare_forall_effect(); }
#line 3081 "parser.cc"
    break;

  case 155:
#line 774 "parser.yy"
                                { (yyval.effect) = make_forall_effect(*(yyvsp[-1].effect)); }
#line 3087 "parser.cc"
    break;

  case 156:
#line 775 "parser.yy"
                       { require_conditional_effects(); }
#line 3093 "parser.cc"
    break;

  case 157:
#line 776 "parser.yy"
                                { (yyval.effect) = &ConditionalEffect::make(*(yyvsp[-2].formula), *(yyvsp[-1].effect)); }
#line 3099 "parser.cc"
    break;

  case 158:
#line 777 "parser.yy"
                                              { if((yyvsp[-1].peffect)->size() ==1 && (yyvsp[-1].peffect)->probability(0) == 1.0){ (yyval.effect)=&(yyvsp[-1].peffect)->effect(0);} else (yyval.effect) = (yyvsp[-1].peffect); }
#line 3105 "parser.cc"
    break;

  case 159:
#line 778 "parser.yy"
                                       { (yyval.effect) = (yyvsp[-1].peffect); }
#line 3111 "parser.cc"
    break;

  case 160:
#line 779 "parser.yy"
                                 { (yyval.effect) = (yyvsp[-1].peffect); }
#line 3117 "parser.cc"
    break;

  case 161:
#line 780 "parser.yy"
                                           { (yyval.effect) = (yyvsp[-1].peffect); }
#line 3123 "parser.cc"
    break;

  case 162:
#line 783 "parser.yy"
                           { (yyval.ceffect) = new ConjunctiveEffect(); }
#line 3129 "parser.cc"
    break;

  case 163:
#line 784 "parser.yy"
                                        { (yyval.ceffect) = (yyvsp[-1].ceffect); (yyval.ceffect)->add_conjunct(*(yyvsp[0].effect)); }
#line 3135 "parser.cc"
    break;

  case 164:
#line 788 "parser.yy"
               {
 		(yyval.peffect) = new ProbabilisticEffect();
 		add_effect_outcome(*(yyval.peffect), (yyvsp[-1].num), *(yyvsp[0].effect));
 	      }
#line 3144 "parser.cc"
    break;

  case 165:
#line 793 "parser.yy"
              { (yyval.peffect) = (yyvsp[-2].peffect); add_effect_outcome(*(yyval.peffect), (yyvsp[-1].num), *(yyvsp[0].effect)); }
#line 3150 "parser.cc"
    break;

  case 166:
#line 795 "parser.yy"
              {
                //$1->print(cout,problem->domain().functions(),problem->terms()); 
		(yyval.peffect) = new ProbabilisticEffect();
		add_feffect_outcome(*(yyval.peffect), (yyvsp[-1].expr), *(yyvsp[0].effect));
	      }
#line 3160 "parser.cc"
    break;

  case 167:
#line 802 "parser.yy"
          {
	    //$2->print(cout,problem->domain().functions(),problem->terms()); 
	   (yyval.peffect) = (yyvsp[-2].peffect); add_feffect_outcome(*(yyval.peffect), (yyvsp[-1].expr), *(yyvsp[0].effect)); }
#line 3168 "parser.cc"
    break;

  case 168:
#line 808 "parser.yy"
             { (yyval.peffect) = (yyvsp[-1].peffect); add_effect_outcome(*(yyval.peffect), new Rational(-1.0), *(yyvsp[0].effect)); }
#line 3174 "parser.cc"
    break;

  case 169:
#line 810 "parser.yy"
{ (yyval.peffect) = new ProbabilisticEffect(); add_effect_outcome(*(yyval.peffect), new Rational(-1.0), *(yyvsp[0].effect)); }
#line 3180 "parser.cc"
    break;

  case 170:
#line 814 "parser.yy"
             { (yyval.peffect) = (yyvsp[-1].peffect); add_effect_outcome(*(yyval.peffect), new Rational(-3.0), *(yyvsp[0].effect)); }
#line 3186 "parser.cc"
    break;

  case 171:
#line 816 "parser.yy"
                         { (yyval.peffect) = new ProbabilisticEffect(); add_effect_outcome(*(yyval.peffect), new Rational(-3.0), *(yyvsp[0].effect)); }
#line 3192 "parser.cc"
    break;

  case 172:
#line 820 "parser.yy"
               { (yyval.peffect) = new ProbabilisticEffect(); 
                 add_effect_outcome(*(yyval.peffect), new Rational(-2.0), *(yyvsp[0].effect)); }
#line 3199 "parser.cc"
    break;

  case 174:
#line 829 "parser.yy"
                               {(yyval.effect) = make_add_effect(*(yyvsp[0].atom)); }
#line 3205 "parser.cc"
    break;

  case 175:
#line 830 "parser.yy"
                                           { (yyval.effect) = make_delete_effect(*(yyvsp[-1].atom)); }
#line 3211 "parser.cc"
    break;

  case 176:
#line 831 "parser.yy"
                         { effect_fluent = true; }
#line 3217 "parser.cc"
    break;

  case 177:
#line 832 "parser.yy"
             { (yyval.effect) = make_assignment_effect((yyvsp[-4].setop), *(yyvsp[-2].appl), *(yyvsp[-1].expr)); }
#line 3223 "parser.cc"
    break;

  case 178:
#line 835 "parser.yy"
                   { (yyval.setop) = Assignment::ASSIGN_OP; }
#line 3229 "parser.cc"
    break;

  case 179:
#line 836 "parser.yy"
                     { (yyval.setop) = Assignment::SCALE_UP_OP; }
#line 3235 "parser.cc"
    break;

  case 180:
#line 837 "parser.yy"
                       { (yyval.setop) = Assignment::SCALE_DOWN_OP; }
#line 3241 "parser.cc"
    break;

  case 181:
#line 838 "parser.yy"
                     { (yyval.setop) = Assignment::INCREASE_OP; }
#line 3247 "parser.cc"
    break;

  case 182:
#line 839 "parser.yy"
                     { (yyval.setop) = Assignment::DECREASE_OP; }
#line 3253 "parser.cc"
    break;

  case 183:
#line 845 "parser.yy"
                                                {(yyval.observation_defs)->add_entry((yyvsp[0].observation)); }
#line 3259 "parser.cc"
    break;

  case 184:
#line 846 "parser.yy"
                               {(yyval.observation_defs) = new Observation(); (yyval.observation_defs)->add_entry((yyvsp[0].observation)); }
#line 3265 "parser.cc"
    break;

  case 185:
#line 849 "parser.yy"
                                                        {(yyval.observation) = make_observation(*(yyvsp[-3].formula), *(yyvsp[-2].num), *(yyvsp[-1].num));}
#line 3271 "parser.cc"
    break;

  case 186:
#line 850 "parser.yy"
                                        {(yyval.observation) = make_observation(*(yyvsp[-3].formula), *(yyvsp[-2].formula) , *(yyvsp[-1].num)); }
#line 3277 "parser.cc"
    break;

  case 187:
#line 851 "parser.yy"
                                                        {(yyval.observation) = make_observation(*(yyvsp[-5].formula), *(yyvsp[-2].peffect)); (yyvsp[-2].peffect)->setObservation();}
#line 3283 "parser.cc"
    break;

  case 188:
#line 852 "parser.yy"
           { (yyval.observation) = make_observation(*(yyvsp[0].formula),Rational(0.5),Rational(0.5));}
#line 3289 "parser.cc"
    break;

  case 189:
#line 869 "parser.yy"
                { make_problem((yyvsp[-5].str), (yyvsp[-1].str)); }
#line 3295 "parser.cc"
    break;

  case 190:
#line 870 "parser.yy"
                { problem->instantiate_actions(); problem->instantiate_events(); delete requirements; }
#line 3301 "parser.cc"
    break;

  case 191:
#line 872 "parser.yy"
               { make_problem((yyvsp[-5].str), (yyvsp[-1].str)); }
#line 3307 "parser.cc"
    break;

  case 192:
#line 873 "parser.yy"
               { problem->instantiate_actions();  problem->instantiate_events(); problem->set_plan_time(*(yyvsp[-1].num));
	         delete (yyvsp[-1].num); delete requirements; }
#line 3314 "parser.cc"
    break;

  case 198:
#line 941 "parser.yy"
                           { name_kind = OBJECT_KIND; std::cout << "start object:" << std::endl;}
#line 3320 "parser.cc"
    break;

  case 199:
#line 942 "parser.yy"
                { name_kind = VOID_KIND; std::cout << "end object" << std::endl;}
#line 3326 "parser.cc"
    break;

  case 200:
#line 955 "parser.yy"
                             {std::cout << "init_element " << std::endl;}
#line 3332 "parser.cc"
    break;

  case 202:
#line 957 "parser.yy"
       { std::cout << "init and conjuncts\n"; problem->set_init_formula(*(yyvsp[-1].conj)); get_init_elts();}
#line 3338 "parser.cc"
    break;

  case 204:
#line 962 "parser.yy"
                            {std::cout << "empty init" << std::endl;}
#line 3344 "parser.cc"
    break;

  case 205:
#line 963 "parser.yy"
                                           {std::cout << "init element" << std::endl;}
#line 3350 "parser.cc"
    break;

  case 206:
#line 967 "parser.yy"
                                {
					std::cout << "atom formula" << std::endl; 
					problem->add_init_atom(*(yyvsp[0].atom));  problem->add_init_effect(*(new AddEffect(*(yyvsp[0].atom))));}
#line 3358 "parser.cc"
    break;

  case 207:
#line 971 "parser.yy"
                 { problem->add_init_value(*(yyvsp[-2].appl), *(yyvsp[-1].num)); delete (yyvsp[-1].num); }
#line 3364 "parser.cc"
    break;

  case 208:
#line 973 "parser.yy"
                 { problem->add_init_effect(*(yyvsp[-1].peffect)); }
#line 3370 "parser.cc"
    break;

  case 209:
#line 975 "parser.yy"
                                { std::cout << "oneof_init" << std::endl; problem->add_init_effect(*(yyvsp[-1].peffect)); }
#line 3376 "parser.cc"
    break;

  case 210:
#line 976 "parser.yy"
                                            { problem->add_init_effect(*(yyvsp[-1].peffect)); }
#line 3382 "parser.cc"
    break;

  case 212:
#line 988 "parser.yy"
               {
		 (yyval.peffect) = new ProbabilisticEffect();
		 add_effect_outcome(*(yyval.peffect), (yyvsp[-1].num), *(yyvsp[0].effect));
	       }
#line 3391 "parser.cc"
    break;

  case 213:
#line 993 "parser.yy"
           { (yyval.peffect) = (yyvsp[-2].peffect); add_effect_outcome(*(yyval.peffect), (yyvsp[-1].num), *(yyvsp[0].effect)); }
#line 3397 "parser.cc"
    break;

  case 214:
#line 997 "parser.yy"
             { (yyval.peffect) = (yyvsp[-1].peffect); add_effect_outcome(*(yyval.peffect), new Rational(-1.0), *(yyvsp[0].effect)); }
#line 3403 "parser.cc"
    break;

  case 215:
#line 999 "parser.yy"
{ (yyval.peffect) = new ProbabilisticEffect(); add_effect_outcome(*(yyval.peffect), new Rational(-1.0), *(yyvsp[0].effect)); }
#line 3409 "parser.cc"
    break;

  case 216:
#line 1003 "parser.yy"
               { (yyval.peffect) = new ProbabilisticEffect(); 
                 add_effect_outcome(*(yyval.peffect), new Rational(-2.0), *(yyvsp[0].effect)); }
#line 3416 "parser.cc"
    break;

  case 217:
#line 1007 "parser.yy"
                       {(yyval.effect) = (yyvsp[0].effect);}
#line 3422 "parser.cc"
    break;

  case 218:
#line 1008 "parser.yy"
                                    { (yyval.effect) = (yyvsp[-1].ceffect); }
#line 3428 "parser.cc"
    break;

  case 219:
#line 1011 "parser.yy"
                        { (yyval.ceffect) = new ConjunctiveEffect(); }
#line 3434 "parser.cc"
    break;

  case 220:
#line 1012 "parser.yy"
                               { (yyval.ceffect) = (yyvsp[-1].ceffect); (yyval.ceffect)->add_conjunct(*(yyvsp[0].effect)); }
#line 3440 "parser.cc"
    break;

  case 221:
#line 1015 "parser.yy"
                               { (yyval.effect) = make_add_effect(*(yyvsp[0].atom)); }
#line 3446 "parser.cc"
    break;

  case 222:
#line 1017 "parser.yy"
             { (yyval.effect) = make_assignment_effect(Assignment::ASSIGN_OP, *(yyvsp[-2].appl), *(yyvsp[-1].expr)); }
#line 3452 "parser.cc"
    break;

  case 223:
#line 1020 "parser.yy"
               { (yyval.expr) = new Value(*(yyvsp[0].num)); delete (yyvsp[0].num); }
#line 3458 "parser.cc"
    break;

  case 224:
#line 1023 "parser.yy"
                                             { problem->set_goal(*(yyvsp[-2].formula), true); }
#line 3464 "parser.cc"
    break;

  case 225:
#line 1024 "parser.yy"
                                                    { problem->set_goal(*(yyvsp[-3].formula), *(yyvsp[-2].num)); delete (yyvsp[-2].num); }
#line 3470 "parser.cc"
    break;

  case 226:
#line 1025 "parser.yy"
                                                      { problem->set_goal(*(yyvsp[-3].formula), true); }
#line 3476 "parser.cc"
    break;

  case 228:
#line 1029 "parser.yy"
                               {set_discount(*(yyvsp[-1].num));}
#line 3482 "parser.cc"
    break;

  case 229:
#line 1030 "parser.yy"
                                        {set_discount(*(yyvsp[-1].num));}
#line 3488 "parser.cc"
    break;

  case 231:
#line 1035 "parser.yy"
                { set_goal_reward(*(yyvsp[-2].expr)); }
#line 3494 "parser.cc"
    break;

  case 232:
#line 1038 "parser.yy"
                          { std::cout << "Set default metric()" << std::endl; set_default_metric(); }
#line 3500 "parser.cc"
    break;

  case 233:
#line 1039 "parser.yy"
                                  { metric_fluent = true; }
#line 3506 "parser.cc"
    break;

  case 234:
#line 1040 "parser.yy"
                { problem->set_metric(*(yyvsp[-1].expr)); metric_fluent = false; }
#line 3512 "parser.cc"
    break;

  case 235:
#line 1041 "parser.yy"
                                  { metric_fluent = true; }
#line 3518 "parser.cc"
    break;

  case 236:
#line 1042 "parser.yy"
                { problem->set_metric(*(yyvsp[-1].expr), true); metric_fluent = false; }
#line 3524 "parser.cc"
    break;

  case 237:
#line 1050 "parser.yy"
                        { (yyval.formula) = new Conjunction(); std::cout << "empty()\n";}
#line 3530 "parser.cc"
    break;

  case 238:
#line 1051 "parser.yy"
                                      { (yyval.formula) = (yyvsp[0].atom); }
#line 3536 "parser.cc"
    break;

  case 239:
#line 1053 "parser.yy"
            { first_eq_term = eq_term; first_eq_expr = eq_expr; }
#line 3542 "parser.cc"
    break;

  case 240:
#line 1054 "parser.yy"
                              { (yyval.formula) = make_equality(); }
#line 3548 "parser.cc"
    break;

  case 241:
#line 1055 "parser.yy"
                          { require_fluents(); }
#line 3554 "parser.cc"
    break;

  case 242:
#line 1056 "parser.yy"
            { (yyval.formula) = new Comparison((yyvsp[-4].comp), *(yyvsp[-2].expr), *(yyvsp[-1].expr)); }
#line 3560 "parser.cc"
    break;

  case 243:
#line 1057 "parser.yy"
                              { (yyval.formula) = make_negation(*(yyvsp[-1].formula)); }
#line 3566 "parser.cc"
    break;

  case 244:
#line 1058 "parser.yy"
                                { (yyval.formula) = (yyvsp[-1].conj); }
#line 3572 "parser.cc"
    break;

  case 245:
#line 1059 "parser.yy"
                 { require_disjunction(); }
#line 3578 "parser.cc"
    break;

  case 246:
#line 1059 "parser.yy"
                                                          { (yyval.formula) = (yyvsp[-1].disj); }
#line 3584 "parser.cc"
    break;

  case 247:
#line 1060 "parser.yy"
                    { require_disjunction(); }
#line 3590 "parser.cc"
    break;

  case 248:
#line 1060 "parser.yy"
                                                                   { (yyval.formula) = (yyvsp[-1].odisj); }
#line 3596 "parser.cc"
    break;

  case 249:
#line 1061 "parser.yy"
                                        { (yyval.formula) = make_implication(*(yyvsp[-2].formula), *(yyvsp[-1].formula)); }
#line 3602 "parser.cc"
    break;

  case 250:
#line 1062 "parser.yy"
                     { prepare_exists(); }
#line 3608 "parser.cc"
    break;

  case 251:
#line 1063 "parser.yy"
            { (yyval.formula) = make_exists(*(yyvsp[-1].formula)); }
#line 3614 "parser.cc"
    break;

  case 252:
#line 1064 "parser.yy"
                     { prepare_forall(); }
#line 3620 "parser.cc"
    break;

  case 253:
#line 1065 "parser.yy"
            { (yyval.formula) = make_forall(*(yyvsp[-1].formula)); }
#line 3626 "parser.cc"
    break;

  case 254:
#line 1068 "parser.yy"
                        { (yyval.conj) = new Conjunction(); }
#line 3632 "parser.cc"
    break;

  case 255:
#line 1069 "parser.yy"
                              { (yyval.conj)->add_conjunct(*(yyvsp[0].formula)); }
#line 3638 "parser.cc"
    break;

  case 256:
#line 1072 "parser.yy"
                        { (yyval.disj) = new Disjunction(); }
#line 3644 "parser.cc"
    break;

  case 257:
#line 1073 "parser.yy"
                              { (yyval.disj)->add_disjunct(*(yyvsp[0].formula)); }
#line 3650 "parser.cc"
    break;

  case 258:
#line 1075 "parser.yy"
                              { (yyval.odisj) = new OneOfDisjunction(); }
#line 3656 "parser.cc"
    break;

  case 259:
#line 1076 "parser.yy"
                                    { (yyval.odisj)->add_oneof_disjunct(*(yyvsp[0].formula)); }
#line 3662 "parser.cc"
    break;

  case 260:
#line 1079 "parser.yy"
                                    { prepare_atom((yyvsp[0].str)); }
#line 3668 "parser.cc"
    break;

  case 261:
#line 1080 "parser.yy"
                        { (yyval.atom) = make_atom(); }
#line 3674 "parser.cc"
    break;

  case 262:
#line 1081 "parser.yy"
                                { prepare_atom((yyvsp[0].str)); (yyval.atom) = make_atom(); }
#line 3680 "parser.cc"
    break;

  case 263:
#line 1084 "parser.yy"
                                    { prepare_atom((yyvsp[0].str)); }
#line 3686 "parser.cc"
    break;

  case 264:
#line 1085 "parser.yy"
                        { (yyval.atom) = make_atom(); }
#line 3692 "parser.cc"
    break;

  case 265:
#line 1086 "parser.yy"
                                { prepare_atom((yyvsp[0].str)); (yyval.atom) = make_atom(); }
#line 3698 "parser.cc"
    break;

  case 266:
#line 1089 "parser.yy"
                  { (yyval.comp) = Comparison::LT_CMP; }
#line 3704 "parser.cc"
    break;

  case 267:
#line 1090 "parser.yy"
                 { (yyval.comp) = Comparison::LE_CMP; }
#line 3710 "parser.cc"
    break;

  case 268:
#line 1091 "parser.yy"
                 { (yyval.comp) = Comparison::GE_CMP; }
#line 3716 "parser.cc"
    break;

  case 269:
#line 1092 "parser.yy"
                  {(yyval.comp) = Comparison::GT_CMP; }
#line 3722 "parser.cc"
    break;

  case 270:
#line 1099 "parser.yy"
               { (yyval.expr) = new Value(*(yyvsp[0].num)); delete (yyvsp[0].num); }
#line 3728 "parser.cc"
    break;

  case 271:
#line 1100 "parser.yy"
                                { (yyval.expr) = new Addition(*(yyvsp[-2].expr), *(yyvsp[-1].expr)); }
#line 3734 "parser.cc"
    break;

  case 272:
#line 1101 "parser.yy"
                                    { (yyval.expr) = make_subtraction(*(yyvsp[-2].expr), (yyvsp[-1].expr)); }
#line 3740 "parser.cc"
    break;

  case 273:
#line 1102 "parser.yy"
                                { (yyval.expr) = new Multiplication(*(yyvsp[-2].expr), *(yyvsp[-1].expr)); }
#line 3746 "parser.cc"
    break;

  case 274:
#line 1103 "parser.yy"
                                { (yyval.expr) = new Division(*(yyvsp[-2].expr), *(yyvsp[-1].expr)); }
#line 3752 "parser.cc"
    break;

  case 275:
#line 1104 "parser.yy"
               { (yyval.expr) = (yyvsp[0].appl); }
#line 3758 "parser.cc"
    break;

  case 276:
#line 1108 "parser.yy"
                  { require_fluents(); eq_expr = new Value(*(yyvsp[0].num)); delete (yyvsp[0].num); }
#line 3764 "parser.cc"
    break;

  case 277:
#line 1109 "parser.yy"
                        { require_fluents(); }
#line 3770 "parser.cc"
    break;

  case 278:
#line 1110 "parser.yy"
                  { eq_expr = new Addition(*(yyvsp[-2].expr), *(yyvsp[-1].expr)); }
#line 3776 "parser.cc"
    break;

  case 279:
#line 1111 "parser.yy"
                        { require_fluents(); }
#line 3782 "parser.cc"
    break;

  case 280:
#line 1112 "parser.yy"
                  { eq_expr = make_subtraction(*(yyvsp[-2].expr), (yyvsp[-1].expr)); }
#line 3788 "parser.cc"
    break;

  case 281:
#line 1113 "parser.yy"
                        { require_fluents(); }
#line 3794 "parser.cc"
    break;

  case 282:
#line 1114 "parser.yy"
                  { eq_expr = new Multiplication(*(yyvsp[-2].expr), *(yyvsp[-1].expr)); }
#line 3800 "parser.cc"
    break;

  case 283:
#line 1115 "parser.yy"
                        { require_fluents(); }
#line 3806 "parser.cc"
    break;

  case 284:
#line 1116 "parser.yy"
                  { eq_expr = new Division(*(yyvsp[-2].expr), *(yyvsp[-1].expr)); }
#line 3812 "parser.cc"
    break;

  case 285:
#line 1117 "parser.yy"
                             { require_fluents(); prepare_application((yyvsp[0].str)); }
#line 3818 "parser.cc"
    break;

  case 286:
#line 1118 "parser.yy"
                            { eq_expr = make_application(); }
#line 3824 "parser.cc"
    break;

  case 287:
#line 1119 "parser.yy"
                     { make_eq_name((yyvsp[0].str)); }
#line 3830 "parser.cc"
    break;

  case 288:
#line 1120 "parser.yy"
                         { eq_term = make_term((yyvsp[0].str)); eq_expr = NULL; }
#line 3836 "parser.cc"
    break;

  case 289:
#line 1123 "parser.yy"
                        { (yyval.expr) = NULL; }
#line 3842 "parser.cc"
    break;

  case 291:
#line 1127 "parser.yy"
                      { prepare_application((yyvsp[0].str)); }
#line 3848 "parser.cc"
    break;

  case 292:
#line 1128 "parser.yy"
           { (yyval.appl) = make_application(); }
#line 3854 "parser.cc"
    break;

  case 293:
#line 1129 "parser.yy"
                  { prepare_application((yyvsp[0].str)); (yyval.appl) = make_application(); }
#line 3860 "parser.cc"
    break;

  case 294:
#line 1132 "parser.yy"
                      { (yyval.expr) = new Value(*(yyvsp[0].num)); delete (yyvsp[0].num); }
#line 3866 "parser.cc"
    break;

  case 295:
#line 1134 "parser.yy"
                 { (yyval.expr) = new Addition(*(yyvsp[-2].expr), *(yyvsp[-1].expr)); }
#line 3872 "parser.cc"
    break;

  case 296:
#line 1136 "parser.yy"
                 { (yyval.expr) = make_subtraction(*(yyvsp[-2].expr), (yyvsp[-1].expr)); }
#line 3878 "parser.cc"
    break;

  case 297:
#line 1138 "parser.yy"
                 { (yyval.expr) = new Multiplication(*(yyvsp[-2].expr), *(yyvsp[-1].expr)); }
#line 3884 "parser.cc"
    break;

  case 298:
#line 1140 "parser.yy"
                 { (yyval.expr) = new Division(*(yyvsp[-2].expr), *(yyvsp[-1].expr)); }
#line 3890 "parser.cc"
    break;

  case 299:
#line 1141 "parser.yy"
                             { (yyval.expr) = (yyvsp[0].appl); }
#line 3896 "parser.cc"
    break;

  case 300:
#line 1143 "parser.yy"
                 { prepare_application((yyvsp[0].str)); (yyval.expr) = make_application(); }
#line 3902 "parser.cc"
    break;

  case 301:
#line 1146 "parser.yy"
                               { (yyval.expr) = NULL; }
#line 3908 "parser.cc"
    break;

  case 303:
#line 1150 "parser.yy"
                             { prepare_application((yyvsp[0].str)); }
#line 3914 "parser.cc"
    break;

  case 304:
#line 1151 "parser.yy"
                  { (yyval.appl) = make_application(); }
#line 3920 "parser.cc"
    break;

  case 305:
#line 1153 "parser.yy"
                    { prepare_application((yyvsp[0].str)); (yyval.appl) = make_application(); }
#line 3926 "parser.cc"
    break;

  case 307:
#line 1161 "parser.yy"
                   { add_term((yyvsp[0].str)); }
#line 3932 "parser.cc"
    break;

  case 308:
#line 1162 "parser.yy"
                       { add_term((yyvsp[0].str)); }
#line 3938 "parser.cc"
    break;

  case 310:
#line 1166 "parser.yy"
                   { add_term((yyvsp[0].str)); }
#line 3944 "parser.cc"
    break;

  case 312:
#line 1170 "parser.yy"
                         { add_variables((yyvsp[0].strs), OBJECT_TYPE); }
#line 3950 "parser.cc"
    break;

  case 313:
#line 1171 "parser.yy"
                                   { add_variables((yyvsp[-1].strs), (yyvsp[0].type)); }
#line 3956 "parser.cc"
    break;

  case 315:
#line 1174 "parser.yy"
                        { (yyval.strs) = new std::vector<const std::string*>(1, (yyvsp[0].str)); }
#line 3962 "parser.cc"
    break;

  case 316:
#line 1175 "parser.yy"
                                     { (yyval.strs) = (yyvsp[-1].strs); (yyval.strs)->push_back((yyvsp[0].str)); }
#line 3968 "parser.cc"
    break;

  case 317:
#line 1178 "parser.yy"
                         {std::cout << "empty types line " << line_number << std::endl;}
#line 3974 "parser.cc"
    break;

  case 318:
#line 1179 "parser.yy"
                       { add_names((yyvsp[0].strs), OBJECT_TYPE); }
#line 3980 "parser.cc"
    break;

  case 319:
#line 1180 "parser.yy"
                                 { add_names((yyvsp[-1].strs), (yyvsp[0].type)); }
#line 3986 "parser.cc"
    break;

  case 321:
#line 1183 "parser.yy"
                { (yyval.strs) = new std::vector<const std::string*>(1, (yyvsp[0].str)); }
#line 3992 "parser.cc"
    break;

  case 322:
#line 1184 "parser.yy"
                         { (yyval.strs) = (yyvsp[-1].strs); (yyval.strs)->push_back((yyvsp[0].str)); }
#line 3998 "parser.cc"
    break;

  case 323:
#line 1187 "parser.yy"
                { require_typing(); }
#line 4004 "parser.cc"
    break;

  case 324:
#line 1187 "parser.yy"
                                           { (yyval.type) = (yyvsp[0].type); }
#line 4010 "parser.cc"
    break;

  case 325:
#line 1190 "parser.yy"
              { (yyval.type) = OBJECT_TYPE; }
#line 4016 "parser.cc"
    break;

  case 326:
#line 1191 "parser.yy"
                 { (yyval.type) = make_type((yyvsp[0].str)); }
#line 4022 "parser.cc"
    break;

  case 327:
#line 1192 "parser.yy"
                            { (yyval.type) = make_type(*(yyvsp[-1].types)); delete (yyvsp[-1].types); }
#line 4028 "parser.cc"
    break;

  case 328:
#line 1195 "parser.yy"
               { (yyval.types) = new TypeSet(); }
#line 4034 "parser.cc"
    break;

  case 329:
#line 1196 "parser.yy"
                  { (yyval.types) = new TypeSet(); (yyval.types)->insert(make_type((yyvsp[0].str))); }
#line 4040 "parser.cc"
    break;

  case 330:
#line 1197 "parser.yy"
                     { (yyval.types) = (yyvsp[-1].types); }
#line 4046 "parser.cc"
    break;

  case 331:
#line 1198 "parser.yy"
                        { (yyval.types) = (yyvsp[-1].types); (yyval.types)->insert(make_type((yyvsp[0].str))); }
#line 4052 "parser.cc"
    break;

  case 333:
#line 1208 "parser.yy"
                { delete (yyvsp[0].str); }
#line 4058 "parser.cc"
    break;

  case 334:
#line 1211 "parser.yy"
                      { delete (yyvsp[0].str); }
#line 4064 "parser.cc"
    break;

  case 335:
#line 1214 "parser.yy"
                  { delete (yyvsp[0].str); }
#line 4070 "parser.cc"
    break;

  case 336:
#line 1217 "parser.yy"
            { delete (yyvsp[0].str); }
#line 4076 "parser.cc"
    break;

  case 337:
#line 1220 "parser.yy"
          { delete (yyvsp[0].str); }
#line 4082 "parser.cc"
    break;

  case 338:
#line 1223 "parser.yy"
          { delete (yyvsp[0].str); }
#line 4088 "parser.cc"
    break;

  case 339:
#line 1226 "parser.yy"
        { delete (yyvsp[0].str); }
#line 4094 "parser.cc"
    break;

  case 340:
#line 1229 "parser.yy"
              { delete (yyvsp[0].str); }
#line 4100 "parser.cc"
    break;

  case 341:
#line 1232 "parser.yy"
                { delete (yyvsp[0].str); }
#line 4106 "parser.cc"
    break;

  case 342:
#line 1235 "parser.yy"
                { delete (yyvsp[0].str); }
#line 4112 "parser.cc"
    break;

  case 343:
#line 1238 "parser.yy"
                              { delete (yyvsp[0].str); }
#line 4118 "parser.cc"
    break;

  case 344:
#line 1241 "parser.yy"
                { delete (yyvsp[0].str); }
#line 4124 "parser.cc"
    break;

  case 345:
#line 1244 "parser.yy"
                    { delete (yyvsp[0].str); }
#line 4130 "parser.cc"
    break;

  case 346:
#line 1247 "parser.yy"
                        { delete (yyvsp[0].str); }
#line 4136 "parser.cc"
    break;

  case 347:
#line 1250 "parser.yy"
                    { delete (yyvsp[0].str); }
#line 4142 "parser.cc"
    break;

  case 348:
#line 1253 "parser.yy"
                    { delete (yyvsp[0].str); }
#line 4148 "parser.cc"
    break;

  case 349:
#line 1256 "parser.yy"
                    { delete (yyvsp[0].str); }
#line 4154 "parser.cc"
    break;

  case 350:
#line 1259 "parser.yy"
                    { delete (yyvsp[0].str); }
#line 4160 "parser.cc"
    break;

  case 351:
#line 1262 "parser.yy"
                      { delete (yyvsp[0].str); }
#line 4166 "parser.cc"
    break;

  case 352:
#line 1265 "parser.yy"
                      { delete (yyvsp[0].str); }
#line 4172 "parser.cc"
    break;

  case 353:
#line 1268 "parser.yy"
                { delete (yyvsp[0].str); }
#line 4178 "parser.cc"
    break;

  case 354:
#line 1271 "parser.yy"
                    { delete (yyvsp[0].str); }
#line 4184 "parser.cc"
    break;


#line 4188 "parser.cc"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = YY_CAST (char *, YYSTACK_ALLOC (YY_CAST (YYSIZE_T, yymsg_alloc)));
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYTERROR;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;


#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif


/*-----------------------------------------------------.
| yyreturn -- parsing is finished, return the result.  |
`-----------------------------------------------------*/
yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[+*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  return yyresult;
}
#line 1300 "parser.yy"


/* Outputs an error message. */
static void yyerror(const std::string& s) {
	std::cerr << PACKAGE ":" << current_file << ':' << line_number << ": " << s << std::endl;
	success = false;
}


/* Outputs a warning. */
static void yywarning(const std::string& s) {
	if (warning_level > 0) {
		std::cerr << PACKAGE ":" << current_file << ':' << line_number << ": " << s << std::endl;
		if (warning_level > 1) {
			success = false;
		}
	}
}


/* Creates an empty domain with the given name. */
static void make_domain(const std::string* name) {
	domain = new Domain(*name);
	domains[*name] = domain;// 添加映射关系
	requirements = &domain->requirements;
	problem = NULL;
	delete name;
}



/* Creates an empty problem with the given name. */
static void make_problem(const std::string* name, const std::string* domain_name) {
	// 获取problem对应的domain是否存在
	std::map<std::string, Domain*>::const_iterator di = domains.find(*domain_name);
	if (di != domains.end()) {
		domain = (*di).second;
	}
	// domain不存在直接结束
	else {
		domain = new Domain(*domain_name);
		domains[*domain_name] = domain;
		yyerror("undeclared domain `" + *domain_name + "' used");
  	}
	// 获取domain的requirement
	requirements = new Requirements(domain->requirements);
	problem = new Problem(*name, *domain);
	my_problem = problem;
	// 判断是否需要reward
	if (requirements->rewards) {
		Action* noopAct = new Action("noop_action");
		const Application& reward_appl =
			Application::make_application(reward_function, TermList());
		const Assignment* reward_assignment =
			new Assignment(Assignment::ASSIGN_OP, reward_appl, *new Value(0.0));
		noopAct->set_effect(*new AssignmentEffect(*reward_assignment));
		//add_action(noopAct);
		problem->add_action(noopAct);
     
//     const Application& reward_appl =
//       Application::make_application(reward_function, TermList());
//     const Assignment* reward_assignment =
//       new Assignment(Assignment::ASSIGN_OP, reward_appl, *new Value(0.0));
//     problem->add_init_effect(*new AssignmentEffect(*reward_assignment));
	}
	delete name;
	delete domain_name;
}


/* Adds :typing to the requirements. */
static void require_typing() {
	if (!requirements->typing) {
		yywarning("assuming `:typing' requirement");
		requirements->typing = true;
	}
}


/* Adds :fluents to the requirements. */
static void require_fluents() {
	if (!requirements->fluents) {
		yywarning("assuming `:fluents' requirement");
		requirements->fluents = true;
	}
}


/* Adds :disjunctive-preconditions to the requirements. */
static void require_disjunction() {
	if (!requirements->disjunctive_preconditions) {
		yywarning("assuming `:disjunctive-preconditions' requirement");
		requirements->disjunctive_preconditions = true;
	}
}


/* Adds :conditional-effects to the requirements. */ 
static void require_conditional_effects() {
	if (!requirements->conditional_effects) {
		yywarning("assuming `:conditional-effects' requirement");
		requirements->conditional_effects = true;
	}
}


/* Returns a simple type with the given name. */
static Type make_type(const std::string* name) {
	std::pair<Type, bool> t = domain->types().find_type(*name);
	if (!t.second)
	{
		t.first = domain->types().add_type(*name);
		
		if (name_kind != TYPE_KIND)
			yywarning("implicit declaration of type `" + *name + "'");
	}

	delete name;
	return t.first;
}


/* Returns the union of the given types. */
static Type make_type(const TypeSet& types) {
	return domain->types().add_type(types);
}


/* Returns a simple term with the given name. */
static Term make_term(const std::string* name) {
	if ((*name)[0] == '?') {
		std::pair<Variable, bool> v = context.find(*name);
		if (!v.second) {
			if (problem != NULL)
				v.first = problem->terms().add_variable(OBJECT_TYPE);
			else
				v.first = domain->terms().add_variable(OBJECT_TYPE);

			context.insert(*name, v.first);
			yyerror("free variable `" + *name + "' used");
    	}
		delete name;
		return v.first;
	} 
	else {	
		TermTable& terms = (problem != NULL) ? problem->terms() : domain->terms();
		
		const PredicateTable& predicates = (parsing_obs_token ? domain->observables() : domain->predicates());
		
		std::pair<Object, bool> o = terms.find_object(*name);

		if (!o.second) {
			size_t n = term_parameters.size();
			if (parsing_atom && predicates.arity(atom_predicate) > n) {
				o.first = terms.add_object(*name, predicates.parameter(atom_predicate, n));
		} else {
			o.first = terms.add_object(*name, OBJECT_TYPE);
		}
			yywarning("implicit declaration of object `" + *name + "'");
		}
		delete name;
		return o.first;
	}
}


/* Creates a predicate with the given name. */
static void make_predicate(const std::string* name) {
	repeated_predicate = false;
	std::pair<Predicate, bool> p = domain->predicates().find_predicate(*name);
	if (!p.second) {
		//    cout << "Make pred " << *name << endl;
		p.first = domain->predicates().add_predicate(*name);
	} else {
		repeated_predicate = true;
		yywarning("ignoring repeated declaration of predicate `" + *name + "'");
	}
	predicate = p.first;
	parsing_predicate = true;
	delete name;
}

/* Creates a observation token with the given name. */
static void make_observable(const std::string* name) {
	repeated_predicate = false;
	std::cout << "observable: " << *name <<std::endl;
	std::pair<Predicate, bool> p = domain->observables().find_predicate(*name);
	if (!p.second) {
		p.first = domain->observables().add_predicate(*name);
	} else {
		repeated_predicate = true;
		yywarning("ignoring repeated declaration of observable `" + *name + "'");
	}
	predicate = p.first;
	//  parsing_obs_token = true;
	parsing_predicate = true;
	delete name;
}

//static ObservationCptRow* make_observation(
static ObservationEntry* make_observation(
					   const StateFormula& form,
					   const ProbabilisticEffect& eff){
	//  ObservationCptRow *row = new ObservationCptRow(form, eff);
	OBS_TYPE=OBS_CPT;
	//  const Rational& prob = eff.probability(0);
	// const Atom& symbol = ((SimpleEffect&)eff.effect(0)).atom();
	ObservationEntry *row = new ObservationEntry(form, eff);
	return row;
}

/* Appends new observation entry to list of observation entries */
static ObservationEntry* make_observation(
			       const StateFormula& form,
			       const Rational& posprob,
			       const Rational& negprob){
	//  cout << "make entry"<<endl;
	OBS_TYPE=OBS_TPFP;
	if (typeid(form) == typeid(Constant)) {
		std::cout << "token is cosnt"<<std::endl;
	}
/* token.print(cout, ((Domain)problem->domain()).observables(),
    	problem->domain().functions(),
    	problem->terms()); */
	ObservationEntry *ob = new ObservationEntry(form, posprob, negprob);
  	return ob;
}

/* Appends new observation entry to list of observation entries */
static ObservationEntry* make_observation(
			       const StateFormula& form,
			       const StateFormula& o,
			       const Rational& prob){
	//  cout << "make entry"<<endl;
	OBS_TYPE=OBS_TPFP;
	if (typeid(form) == typeid(Constant)) {
		std::cout << "token is cosnt"<<std::endl;
	}

	if (typeid(o) == typeid(Constant)) {
		std::cout << "token is cosnt"<<std::endl;
	}

    /* o.print(cout, ((Domain)problem->domain()).predicates(),
  	      problem->domain().functions(),
  	      problem->terms()); */

	/* token */
	ObservationEntry *ob = new ObservationEntry(form, &o, prob, prob);
  	return ob;
}

/* Creates a function with the given name. */
static void make_function(const std::string* name) {
	repeated_function = false;
	std::pair<Function, bool> f = domain->functions().find_function(*name);
	if (!f.second) {
		f.first = domain->functions().add_function(*name);
	} else {
    	repeated_function = true;
    	if (requirements->rewards && f.first == reward_function) {
    		yywarning("ignoring declaration of reserved function `reward'");
    	} else {
    		yywarning("ignoring repeated declaration of function `" + *name + "'");
    	}
	}
	function = f.first;
	parsing_function = true;
	delete name;
}


/* Creates an action with the given name. */
static void make_action(const std::string* name) {
	context.push_frame();
	action = new ActionSchema(*name);
	delete name;
}


/* Adds the current action to the current domain. */
static void add_action() {
	context.pop_frame();
	if (domain->find_action(action->name()) == NULL) {
		domain->add_action(*action);
	} else {
		yywarning("ignoring repeated declaration of action `" + action->name() + "'");
		delete action;
	}
	action = NULL;
}

/* Adds the current event to the current domain. */
static void add_event() {
	context.pop_frame();
	if (domain->find_event(action->name()) == NULL) {
		domain->add_event(*action);
	} else {
		yywarning("ignoring repeated declaration of event `" + action->name() + "'");
		delete action;
	}
	action = NULL;
}

/* Prepares for the parsing of a universally quantified effect. */ 
static void prepare_forall_effect() {
	if (!requirements->conditional_effects) {
		yywarning("assuming `:conditional-effects' requirement");
		requirements->conditional_effects = true;
	}
	context.push_frame();
	quantified.push_back(NULL_TERM);
}


/* Creates a universally quantified effect. */
static const pEffect* make_forall_effect(const pEffect& effect) {
	context.pop_frame();
	QuantifiedEffect* qeffect = new QuantifiedEffect(effect);
	size_t n = quantified.size() - 1;
	size_t m = n;

	while (is_variable(quantified[n])) n--;
	
	for (size_t i = n + 1; i <= m; i++) {
		qeffect->add_parameter(quantified[i]);
	}

	quantified.resize(n);
	return qeffect;
}


/* Adds an outcome to the given probabilistic effect.*/
static void add_effect_outcome(ProbabilisticEffect& peffect,
			        const Rational* p, const pEffect& effect) {
    std::cout << "*p = " << *p << std::endl; 
    // oneof涉及到non-deterministic效果
    std::cout << "requirements->non_deterministic:" << requirements->non_deterministic;
	if((*p == -1.0 || *p == -2.0 || *p == -3.0) && !requirements->non_deterministic){
		yywarning("assuming `:non-deterministic' requirement");
		requirements->non_deterministic = true;    
		/* requirements->probabilistic_effects = true; */
	}
    // 一般的probability effect
	else if ((*p != -1.0 && *p != -2.0 || *p != -3.0) && !requirements->probabilistic_effects) {
    	yywarning("assuming `:probabilistic-effects' requirement1");
		requirements->probabilistic_effects = true;
	} 
    
	if(*p == -1.0){ // okay, its an oneof nd-effect
	}
	else if(*p == -2.0){ // okay, its an unknown nd-effect
	}
	else if(*p == -3.0){ // okay, its an or nd-effect
	}
	else if (*p < 0.0 || *p > 1.0) {
		yyerror("outcome probability needs to be in the interval [0,1]");
	}
	if (!peffect.add_outcome(*p, effect)) {
		yyerror("effect outcome probabilities add up to more than 1");
	}
	delete p;
}
static void add_feffect_outcome(ProbabilisticEffect& peffect,
			        const Expression* p, const pEffect& effect) {
   
	if (!requirements->probabilistic_effects) {
		yywarning("assuming `:probabilistic-effects' requirement2");
		requirements->probabilistic_effects = true;
	}
	/* if (*p < 0 || *p > 1) {
		yyerror("outcome probability needs to be in the interval [0,1]");
	} */
	if (!peffect.add_foutcome(*p, effect)) {
		yyerror("effect outcome probabilities add up to more than 1");
	}
	/* cout << "done adding feffect" <<endl; */
	/* delete p; */
}

/* Creates an add effect. */
static const pEffect* make_add_effect(const Atom& atom) {
	domain->predicates().make_dynamic(atom.predicate());
	return new AddEffect(atom);
}


/* Creates a delete effect. */
static const pEffect* make_delete_effect(const Atom& atom) {
	domain->predicates().make_dynamic(atom.predicate());
	return new DeleteEffect(atom);
}


/* Creates an assignment effect. */
static const pEffect* make_assignment_effect(Assignment::AssignOp oper,
					    const Application& application,
					    const Expression& expr) {
	if (requirements->rewards && application.function() == reward_function) {
		if ((oper != Assignment::INCREASE_OP && oper != Assignment::DECREASE_OP) 
			|| typeid(expr) != typeid(Value)) {
				yyerror("only constant reward increments/decrements allowed");
		}
	} else {
		require_fluents();
	}
	effect_fluent = false;
	domain->functions().make_dynamic(application.function());
	const Assignment& assignment = *new Assignment(oper, application, expr);
	return new AssignmentEffect(assignment);
}


/* Adds types, constants, or objects to the current domain or problem. */
static void add_names(const std::vector<const std::string*>* names, Type type) {
	for (std::vector<const std::string*>::const_iterator si = names->begin(); si != names->end(); si++) {
		const std::string* s = *si;
		if (name_kind == TYPE_KIND) {// 1-if
			if (*s == OBJECT_NAME) {// 2-if
				yywarning("ignoring declaration of reserved type `object'");
			} else if (*s == NUMBER_NAME) {
				yywarning("ignoring declaration of reserved type `number'");
			} else {
				std::pair<Type, bool> t = domain->types().find_type(*s);
				if (!t.second) {
				  t.first = domain->types().add_type(*s);
				}
				if (!domain->types().add_supertype(t.first, type)) {
				  yyerror("cyclic type hierarchy");
				}
			}// end 2-if

		} else if (name_kind == CONSTANT_KIND) {// 1-if
			std::pair<Object, bool> o = domain->terms().find_object(*s);
			if (!o.second) {
				domain->terms().add_object(*s, type);
			} else {
				TypeSet components;
				domain->types().components(components, domain->terms().type(o.first));
				components.insert(type);
				domain->terms().set_type(o.first, make_type(components));
			}

		} else { /* name_kind == OBJECT_KIND */
			if (domain->terms().find_object(*s).second) {
				yywarning("ignoring declaration of object `" + *s + "' previously declared as constant");
			} else {
				std::pair<Object, bool> o = problem->terms().find_object(*s);
				if (!o.second) {
					problem->terms().add_object(*s, type);
				} else {
					TypeSet components;
					domain->types().components(components, problem->terms().type(o.first));
					components.insert(type);
					problem->terms().set_type(o.first, make_type(components));
				}
			}
		}// end 1-if
		delete s;
	}// end-for

	delete names;
}


/* Adds variables to the current variable list. */
static void add_variables(const std::vector<const std::string*>* names, Type type) {
	for (std::vector<const std::string*>::const_iterator si = names->begin(); si != names->end(); si++) {
    	const std::string* s = *si;// 获取名字
		// 是普通谓词
		if (parsing_predicate && !parsing_obs_token) {
			if (!repeated_predicate)
				domain->predicates().add_parameter(predicate, type);
		}
		// 函数
		else if (parsing_function && !parsing_obs_token ) {
			if (!repeated_function) {
				domain->functions().add_parameter(function, type);
			}
		}
		// 可观察谓词
		else if(parsing_predicate && parsing_obs_token){
			if (!repeated_function) {
				domain->observables().add_parameter(function, type);
			}
		}
		// term
		else {
			if (context.shallow_find(*s).second) {
				yyerror("repetition of parameter `" + *s + "'");
			} else if (context.find(*s).second) {
				yywarning("shadowing parameter `" + *s + "'");
			}

			Variable var;
			if (problem != NULL) {
				var = problem->terms().add_variable(type);
			} else {
				var = domain->terms().add_variable(type);
			}
			context.insert(*s, var);
			if (!quantified.empty()) {
				quantified.push_back(var);
			} else { /* action != NULL */
				action->add_parameter(var);
			}
		}
		delete s;
	}// end-for
	delete names;
}


/* Prepares for the parsing of an atomic state formula. */ 
static void prepare_atom(const std::string* name) {
	std::pair<Predicate, bool> p;
	if(parsing_obs_token)
		p = domain->observables().find_predicate(*name);
	else
		p = domain->predicates().find_predicate(*name);
	if (!p.second) {
		if(parsing_obs_token)
			atom_predicate = domain->observables().add_predicate(*name);      
		else
			atom_predicate = domain->predicates().add_predicate(*name);
	
		undeclared_atom_predicate = true;
	
		if (problem != NULL)
			yywarning("undeclared predicate `" + *name + "' used");
		else
			yywarning("implicit declaration of predicate `" + *name + "'");
	} 
	else {
		atom_predicate = p.first;
		undeclared_atom_predicate = false;
	}
	term_parameters.clear();
	parsing_atom = true;
	delete name;
}


/* Prepares for the parsing of a function application. */ 
static void prepare_application(const std::string* name) {
	std::pair<Function, bool> f = domain->functions().find_function(*name);
	if (!f.second) {
		appl_function = domain->functions().add_function(*name);
		undeclared_appl_function = true;
		if (problem != NULL)
			yywarning("undeclared function `" + *name + "' used");
		else 
			yywarning("implicit declaration of function `" + *name + "'");
	} else {
		appl_function = f.first;
		undeclared_appl_function = false;
	}
	
	if (requirements->rewards && f.first == reward_function) {
		if (!effect_fluent && !metric_fluent) {
			yyerror("reserved function `reward' not allowed here");
		}
	} else {
		require_fluents();
	}
	term_parameters.clear();
	parsing_application = true;
	delete name;
}


/* Adds a term with the given name to the current atomic state formula. */
static void add_term(const std::string* name) {
    Term term = make_term(name);// 根据名字创建term
    // 获取term table
    const TermTable& terms = (problem != NULL) ? problem->terms() : domain->terms();
    // 当前正在解析atom
    if (parsing_atom) {
        // 根据是否解析obs判断是普通谓词还是可观察变量
        PredicateTable& predicates = (!parsing_obs_token ? domain->predicates() : domain->observables());
        size_t n = term_parameters.size();
        if (undeclared_atom_predicate) {
            predicates.add_parameter(atom_predicate, terms.type(term));
        } else if (predicates.arity(atom_predicate) > n 
                && !domain->types().subtype(terms.type(term),predicates.parameter(atom_predicate,n))) {
            yyerror("type mismatch");
        }
    // 当前在解析application
    } else if (parsing_application) {
        FunctionTable& functions = domain->functions();
        size_t n = term_parameters.size();
        if (undeclared_appl_function) {
            functions.add_parameter(appl_function, terms.type(term));
        } else if (functions.arity(appl_function) > n 
                && !domain->types().subtype(terms.type(term), functions.parameter(appl_function, n))) {
            yyerror("type mismatch");
        }
    }
  term_parameters.push_back(term);
}


/* Creates the atomic formula just parsed. */
static const Atom* make_atom() {
    size_t n = term_parameters.size();
    if(parsing_obs_token){// 当前正在解析observation
        // 参数个数情况判断
        if (domain->observables().arity(atom_predicate) < n) {
            yyerror("too many parameters passed to obs `"
                + domain->observables().name(atom_predicate) + "'");
        } else if (domain->observables().arity(atom_predicate) > n) {
            yyerror("too few parameters passed to obs `"
                + domain->observables().name(atom_predicate) + "'");
        }
    }
    // 当前解析普通atom
    else{
        // 参数个数情况判断
        if (domain->predicates().arity(atom_predicate) < n) {
            yyerror("too many parameters passed to predicate `"
                + domain->predicates().name(atom_predicate) + "'");
        } else if (domain->predicates().arity(atom_predicate) > n) {
            std::cout << "need " << domain->predicates().arity(atom_predicate) << " but " << n << std::endl;
            yyerror("too few parameters passed to predicate `"
                + domain->predicates().name(atom_predicate) + "'");
        }
    }
    parsing_atom = false;
    // 创建atom
    return &Atom::make_atom(atom_predicate, term_parameters);
}


/* Creates the function application just parsed. */
static const Application* make_application() {

    size_t n = term_parameters.size();
    if (domain->functions().arity(appl_function) < n) {
        yyerror("too many parameters passed to function `"
            + domain->functions().name(appl_function) + "'");
    } else if (domain->functions().arity(appl_function) > n) {
        yyerror("too few parameters passed to function `"
            + domain->functions().name(appl_function) + "'");
    }
    parsing_application = false;
    return &Application::make_application(appl_function, term_parameters);
}


/* Creates a subtraction.  term - opt_term */
static const Expression* make_subtraction(const Expression& term,
					  const Expression* opt_term) {
    if (opt_term != NULL) {// 两个数实现减法
        return new Subtraction(term, *opt_term);
    } else {// 一个数实现负数
        return new Subtraction(*new Value(0.0), term);
    }
}


/* Creates an atom or fluent for the given name to be used in an
   equality formula. */
static void make_eq_name(const std::string* name) {
    std::pair<Function, bool> f = domain->functions().find_function(*name);
    if (f.second) {// domain中定义的function
        prepare_application(name);
        eq_expr = make_application();
    } else {
        /* Assume this is a term. */
        eq_term = make_term(name);
        eq_expr = NULL;
    }
}


/* Creates an equality formula. 使用到了 =的前提条件 */
static const StateFormula* make_equality() {
    if (!requirements->equality) {
        yywarning("assuming `:equality' requirement");
        requirements->equality = true;
    }
    // 等号两侧表达式不为空
    if (first_eq_expr != NULL && eq_expr != NULL) {
        return new Comparison(Comparison::EQ_CMP, *first_eq_expr, *eq_expr);
    } else if (first_eq_expr == NULL && eq_expr == NULL) {// 两者均为空
        // 获取term表格
        const TermTable& terms = (problem != NULL) ? problem->terms() : domain->terms();
        // 判断类型是否相同
        if (domain->types().subtype(terms.type(first_eq_term), terms.type(eq_term))
            || domain->types().subtype(terms.type(eq_term),  terms.type(first_eq_term)))
        {
            return new Equality(first_eq_term, eq_term);
        } else {
            return 0;//&StateFormula::FALSE;
        }
    } else {
        yyerror("comparison of term and numeric expression");
        return 0;//&StateFormula::FALSE;
    }
}


/* Creates a negated formula. */
static const StateFormula* make_negation(const StateFormula& negand) {
    if (typeid(negand) == typeid(Atom)) {
        if (!requirements->negative_preconditions) {
            yywarning("assuming `:negative-preconditions' requirement");
            requirements->negative_preconditions = true;
        }
        // 需要否定的formual不是赋值或者比较，则需要支持析取式
        } else if (typeid(negand) != typeid(Equality) && typeid(negand) != typeid(Comparison)) {
            require_disjunction();
        }
    //创建并返回指针
    return &Negation::make_negation(negand);
}


/* Creates an implication. f1 -> f2 */
static const StateFormula* make_implication(const StateFormula& f1, const StateFormula& f2) {
    require_disjunction();// 判断是否有disjunction，没有则warning并开启
    Disjunction* disj = new Disjunction();//创建析取式
    disj->add_disjunct(Negation::make_negation(f1));//创建 !f1 \/ f2 
    disj->add_disjunct(f2);
    return disj;
}


/* Prepares for the parsing of an existentially quantified formula. */
static void prepare_exists() {
	if (!requirements->existential_preconditions) {
		yywarning("assuming `:existential-preconditions' requirement");
		requirements->existential_preconditions = true;
	}
	context.push_frame();
	quantified.push_back(NULL_TERM);
}


/* Prepares for the parsing of a universally quantified formula. */
static void prepare_forall() {
	if (!requirements->universal_preconditions) {
		yywarning("assuming `:universal-preconditions' requirement");
		requirements->universal_preconditions = true;
	}
	context.push_frame();
	quantified.push_back(NULL_TERM);
}


/* Creates an existentially quantified formula. */
static const StateFormula* make_exists(const StateFormula& body) {
	context.pop_frame();
	size_t m = quantified.size() - 1;
	size_t n = m;
	
	while (is_variable(quantified[n])) n--;

	if (n < m) {
		Exists* exists = new Exists();
		
		for (size_t i = n + 1; i <= m; i++) {
			exists->add_parameter(quantified[i]);
		}

		exists->set_body(body);
		quantified.resize(n);
		return exists;
	} else {
		quantified.pop_back();
		return &body;
	}
}


/* Creates a universally quantified formula. */
static const StateFormula* make_forall(const StateFormula& body) {
	context.pop_frame();
	size_t m = quantified.size() - 1;
	size_t n = m;
	while (is_variable(quantified[n])) {
		n--;
	}
	if (n < m) {
		Forall* forall = new Forall();
		for (size_t i = n + 1; i <= m; i++) {
			forall->add_parameter(quantified[i]);
		}
		forall->set_body(body);
		quantified.resize(n);
		return forall;
	} else {
		quantified.pop_back();
		return &body;
	}
}

void set_discount(const Rational& discount){
	problem->set_discount(discount);
}


/* Sets the goal reward for the current problem. */
void set_goal_reward(const Expression& goal_reward) {
    if (!requirements->rewards) {// goal中含有reward判断是否使用reward关键字
        yyerror("goal reward only allowed with the `:rewards' requirement");
    } else {
        const Application& reward_appl =
            Application::make_application(reward_function, TermList());// 创建一个application存储reward的操作
        const Assignment* reward_assignment =
            new Assignment(Assignment::INCREASE_OP, reward_appl, goal_reward);// 对该aplication赋值
        problem->set_goal_reward(*reward_assignment);//设置reward目标
    }
}


/* Sets the default metric for the current problem. */
static void set_default_metric() {
    if (requirements->rewards) {
        const Application& reward_appl =
            Application::make_application(reward_function, TermList());
        problem->set_metric(reward_appl);//设置评估函数
    }
}

/* make all atoms in formula or subformula dynamic */
void make_all_dynamic(const StateFormula &formula){
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
    domain->predicates().make_dynamic(af->predicate());
    return;
  }

  const Negation* nf = dynamic_cast<const Negation*>(&formula);
  if (nf != NULL) {
    /*
     * The state variables of a negation are the state variables of the
     * negand.
     */
    make_all_dynamic(nf->negand());
    return;
  }

  const Conjunction* cf = dynamic_cast<const Conjunction*>(&formula);
  if (cf != NULL) {
    /*
     * The state variables of a conjunction are the state variables of
     * the conjuncts.
     */
    for (size_t i = 0; i < cf->size(); i++) {
      make_all_dynamic(cf->conjunct(i));
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
      make_all_dynamic(df->disjunct(i));
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
      make_all_dynamic(odf->disjunct(i));
    }
    return;
  }
  /*
   * No other types of formulas should appear in fully instantiated
   * action preconditions and effect conditions.
   */

}


/* if we use logical formula for init, then need to get atoms holding for later grounding 
   purposes*/
static void get_init_elts(){
  
    const StateFormula &init = problem->init_formula();// 得到初始状态公式
    if(!&init)
        return;
    // 判断是否为和取式
    const Conjunction *c = dynamic_cast<const Conjunction*>(&init);
    if(c != NULL){
        // 将初始状态的每一个atom添加
        for (size_t i = 0; i < c->size(); i++) {
            const Atom *a = dynamic_cast<const Atom*>(&c->conjunct(i));
            if(a != NULL){// 普通atom直接添加
                problem->add_init_atom(*a);	
            }
            else{// oneof等形式的atom，递归处理
                make_all_dynamic(c->conjunct(i));
            }      
        }
    }
    else
        make_all_dynamic(init);
}

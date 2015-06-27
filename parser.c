/* A Bison parser, made by GNU Bison 3.0.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2013 Free Software Foundation, Inc.

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

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.0.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* Copy the first part of user declarations.  */
#line 1 "parser.y" /* yacc.c:339  */

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "compile.h"
#include "jv_alloc.h"
#define YYMALLOC jv_mem_alloc
#define YYFREE jv_mem_free

#line 77 "parser.c" /* yacc.c:339  */

# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 1
#endif

/* In a future release of Bison, this section will be replaced
   by #include "y.tab.h".  */
#ifndef YY_YY_PARSER_H_INCLUDED
# define YY_YY_PARSER_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif
/* "%code requires" blocks.  */
#line 11 "parser.y" /* yacc.c:355  */

#include "locfile.h"
struct lexer_param;

#define YYLTYPE location
#define YYLLOC_DEFAULT(Loc, Rhs, N)             \
  do {                                          \
    if (N) {                                    \
      (Loc).start = YYRHSLOC(Rhs, 1).start;     \
      (Loc).end = YYRHSLOC(Rhs, N).end;         \
    } else {                                    \
      (Loc).start = YYRHSLOC(Rhs, 0).end;       \
      (Loc).end = YYRHSLOC(Rhs, 0).end;         \
    }                                           \
  } while (0)
 

#line 125 "parser.c" /* yacc.c:355  */

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    INVALID_CHARACTER = 258,
    IDENT = 259,
    FIELD = 260,
    LITERAL = 261,
    FORMAT = 262,
    REC = 263,
    SETMOD = 264,
    EQ = 265,
    NEQ = 266,
    DEFINEDOR = 267,
    AS = 268,
    DEF = 269,
    MODULE = 270,
    IMPORT = 271,
    IF = 272,
    THEN = 273,
    ELSE = 274,
    ELSE_IF = 275,
    REDUCE = 276,
    FOREACH = 277,
    END = 278,
    AND = 279,
    OR = 280,
    TRY = 281,
    CATCH = 282,
    LABEL = 283,
    BREAK = 284,
    LOC = 285,
    SETPIPE = 286,
    SETPLUS = 287,
    SETMINUS = 288,
    SETMULT = 289,
    SETDIV = 290,
    SETDEFINEDOR = 291,
    LESSEQ = 292,
    GREATEREQ = 293,
    QQSTRING_START = 294,
    QQSTRING_TEXT = 295,
    QQSTRING_INTERP_START = 296,
    QQSTRING_INTERP_END = 297,
    QQSTRING_END = 298
  };
#endif
/* Tokens.  */
#define INVALID_CHARACTER 258
#define IDENT 259
#define FIELD 260
#define LITERAL 261
#define FORMAT 262
#define REC 263
#define SETMOD 264
#define EQ 265
#define NEQ 266
#define DEFINEDOR 267
#define AS 268
#define DEF 269
#define MODULE 270
#define IMPORT 271
#define IF 272
#define THEN 273
#define ELSE 274
#define ELSE_IF 275
#define REDUCE 276
#define FOREACH 277
#define END 278
#define AND 279
#define OR 280
#define TRY 281
#define CATCH 282
#define LABEL 283
#define BREAK 284
#define LOC 285
#define SETPIPE 286
#define SETPLUS 287
#define SETMINUS 288
#define SETMULT 289
#define SETDIV 290
#define SETDEFINEDOR 291
#define LESSEQ 292
#define GREATEREQ 293
#define QQSTRING_START 294
#define QQSTRING_TEXT 295
#define QQSTRING_INTERP_START 296
#define QQSTRING_INTERP_END 297
#define QQSTRING_END 298

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE YYSTYPE;
union YYSTYPE
{
#line 31 "parser.y" /* yacc.c:355  */

  jv literal;
  block blk;

#line 228 "parser.c" /* yacc.c:355  */
};
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE YYLTYPE;
struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif



int yyparse (block* answer, int* errors, struct locfile* locations, struct lexer_param* lexer_param_ptr);

#endif /* !YY_YY_PARSER_H_INCLUDED  */

/* Copy the second part of user declarations.  */
#line 113 "parser.y" /* yacc.c:358  */

#include "lexer.h"
struct lexer_param {
  yyscan_t lexer;
};
#define FAIL(loc, msg)                                             \
  do {                                                             \
    location l = loc;                                              \
    yyerror(&l, answer, errors, locations, lexer_param_ptr, msg);  \
    /*YYERROR*/;                                                   \
  } while (0)

void yyerror(YYLTYPE* loc, block* answer, int* errors, 
             struct locfile* locations, struct lexer_param* lexer_param_ptr, const char *s){
  (*errors)++;
  if (strstr(s, "unexpected")) {
#ifdef WIN32
      locfile_locate(locations, *loc, "jq: error: %s (Windows cmd shell quoting issues?)", s);
#else
      locfile_locate(locations, *loc, "jq: error: %s (Unix shell quoting issues?)", s);
#endif
  } else {
      locfile_locate(locations, *loc, "jq: error: %s", s);
  }
}

int yylex(YYSTYPE* yylval, YYLTYPE* yylloc, block* answer, int* errors, 
          struct locfile* locations, struct lexer_param* lexer_param_ptr) {
  yyscan_t lexer = lexer_param_ptr->lexer;
  int tok = jq_yylex(yylval, yylloc, lexer);
  if ((tok == LITERAL || tok == QQSTRING_TEXT) && !jv_is_valid(yylval->literal)) {
    jv msg = jv_invalid_get_msg(jv_copy(yylval->literal));
    if (jv_get_kind(msg) == JV_KIND_STRING) {
      FAIL(*yylloc, jv_string_value(msg));
    } else {
      FAIL(*yylloc, "Invalid literal");
    }
    jv_free(msg);
    jv_free(yylval->literal);
    yylval->literal = jv_null();
  }
  return tok;
}

static block gen_dictpair(block k, block v) {
  return BLOCK(gen_subexp(k), gen_subexp(v), gen_op_simple(INSERT));
}

static block gen_index(block obj, block key) {
  return BLOCK(gen_subexp(key), obj, gen_op_simple(INDEX));
}

static block gen_index_opt(block obj, block key) {
  return BLOCK(gen_subexp(key), obj, gen_op_simple(INDEX_OPT));
}

static block gen_slice_index(block obj, block start, block end, opcode idx_op) {
  block key = BLOCK(gen_subexp(gen_const(jv_object())),
                    gen_subexp(gen_const(jv_string("start"))),
                    gen_subexp(start),
                    gen_op_simple(INSERT),
                    gen_subexp(gen_const(jv_string("end"))),
                    gen_subexp(end),
                    gen_op_simple(INSERT));
  return BLOCK(key, obj, gen_op_simple(idx_op));
}

static block constant_fold(block a, block b, int op) {
  if (!block_is_single(a) || !block_is_const(a) ||
      !block_is_single(b) || !block_is_const(b))
    return gen_noop();
  if (block_const_kind(a) != block_const_kind(b))
    return gen_noop();

  jv res = jv_invalid();

  if (block_const_kind(a) == JV_KIND_NUMBER) {
    double na = jv_number_value(block_const(a));
    double nb = jv_number_value(block_const(b));
    switch (op) {
    case '+': res = jv_number(na + nb); break;
    case '-': res = jv_number(na - nb); break;
    case '*': res = jv_number(na * nb); break;
    case '/': res = jv_number(na / nb); break;
    case EQ:  res = (na == nb ? jv_true() : jv_false()); break;
    case NEQ: res = (na != nb ? jv_true() : jv_false()); break;
    case '<': res = (na < nb ? jv_true() : jv_false()); break;
    case '>': res = (na > nb ? jv_true() : jv_false()); break;
    case LESSEQ: res = (na <= nb ? jv_true() : jv_false()); break;
    case GREATEREQ: res = (na >= nb ? jv_true() : jv_false()); break;
    default: break;
    }
  } else if (op == '+' && block_const_kind(a) == JV_KIND_STRING) {
    res = jv_string_concat(block_const(a),  block_const(b));
  } else {
    return gen_noop();
  }

  if (jv_get_kind(res) == JV_KIND_INVALID)
    return gen_noop();

  block_free(a);
  block_free(b);
  return gen_const(res);
}

static block gen_binop(block a, block b, int op) {
  block folded = constant_fold(a, b, op);
  if (!block_is_noop(folded))
    return folded;

  const char* funcname = 0;
  switch (op) {
  case '+': funcname = "_plus"; break;
  case '-': funcname = "_minus"; break;
  case '*': funcname = "_multiply"; break;
  case '/': funcname = "_divide"; break;
  case '%': funcname = "_mod"; break;
  case EQ: funcname = "_equal"; break;
  case NEQ: funcname = "_notequal"; break;
  case '<': funcname = "_less"; break;
  case '>': funcname = "_greater"; break;
  case LESSEQ: funcname = "_lesseq"; break;
  case GREATEREQ: funcname = "_greatereq"; break;
  }
  assert(funcname);

  return gen_call(funcname, BLOCK(gen_lambda(a), gen_lambda(b)));
}

static block gen_format(block a, jv fmt) {
  return BLOCK(a, gen_call("format", BLOCK(gen_lambda(gen_const(fmt)))));
}

static block gen_definedor_assign(block object, block val) {
  block tmp = gen_op_var_fresh(STOREV, "tmp");
  return BLOCK(gen_op_simple(DUP),
               val, tmp,
               gen_call("_modify", BLOCK(gen_lambda(object),
                                         gen_lambda(gen_definedor(gen_noop(), 
                                                                  gen_op_bound(LOADV, tmp))))));
}
 
static block gen_update(block object, block val, int optype) {
  block tmp = gen_op_var_fresh(STOREV, "tmp");
  return BLOCK(gen_op_simple(DUP),
               val,
               tmp,
               gen_call("_modify", BLOCK(gen_lambda(object), 
                                         gen_lambda(gen_binop(gen_noop(),
                                                              gen_op_bound(LOADV, tmp),
                                                              optype)))));
}


#line 411 "parser.c" /* yacc.c:358  */

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

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

#ifndef YY_ATTRIBUTE
# if (defined __GNUC__                                               \
      && (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)))  \
     || defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#  define YY_ATTRIBUTE(Spec) __attribute__(Spec)
# else
#  define YY_ATTRIBUTE(Spec) /* empty */
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# define YY_ATTRIBUTE_PURE   YY_ATTRIBUTE ((__pure__))
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# define YY_ATTRIBUTE_UNUSED YY_ATTRIBUTE ((__unused__))
#endif

#if !defined _Noreturn \
     && (!defined __STDC_VERSION__ || __STDC_VERSION__ < 201112)
# if defined _MSC_VER && 1200 <= _MSC_VER
#  define _Noreturn __declspec (noreturn)
# else
#  define _Noreturn YY_ATTRIBUTE ((__noreturn__))
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
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
         || (defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL \
             && defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
  YYLTYPE yyls_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE) + sizeof (YYLTYPE)) \
      + 2 * YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  27
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1976

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  65
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  26
/* YYNRULES -- Number of rules.  */
#define YYNRULES  153
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  304

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   298

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,    58,    53,     2,     2,
      56,    57,    51,    49,    45,    50,    60,    52,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    59,    55,
      47,    46,    48,    54,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    61,     2,    62,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    63,    44,    64,     2,     2,     2,     2,
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
      35,    36,    37,    38,    39,    40,    41,    42,    43
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   271,   271,   274,   279,   282,   292,   295,   300,   303,
     308,   312,   316,   320,   324,   328,   331,   336,   340,   344,
     349,   361,   365,   369,   373,   377,   381,   385,   389,   393,
     397,   401,   405,   409,   413,   417,   421,   425,   431,   437,
     441,   445,   449,   453,   457,   461,   465,   469,   474,   483,
     490,   502,   516,   521,   527,   530,   535,   540,   547,   547,
     551,   551,   558,   561,   564,   570,   573,   578,   581,   584,
     590,   593,   596,   604,   608,   611,   614,   617,   620,   623,
     626,   629,   632,   636,   642,   645,   648,   651,   654,   657,
     660,   663,   666,   669,   672,   675,   678,   681,   684,   687,
     690,   697,   701,   705,   717,   722,   723,   724,   725,   728,
     731,   736,   741,   745,   748,   753,   756,   761,   764,   769,
     772,   775,   778,   781,   786,   789,   792,   795,   798,   801,
     804,   807,   810,   813,   816,   819,   822,   825,   828,   831,
     834,   837,   842,   845,   846,   847,   850,   853,   856,   859,
     863,   867,   871,   874
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 1
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "INVALID_CHARACTER", "IDENT", "FIELD",
  "LITERAL", "FORMAT", "\"..\"", "\"%=\"", "\"==\"", "\"!=\"", "\"//\"",
  "\"as\"", "\"def\"", "\"module\"", "\"import\"", "\"if\"", "\"then\"",
  "\"else\"", "\"elif\"", "\"reduce\"", "\"foreach\"", "\"end\"",
  "\"and\"", "\"or\"", "\"try\"", "\"catch\"", "\"label\"", "\"break\"",
  "\"__loc__\"", "\"|=\"", "\"+=\"", "\"-=\"", "\"*=\"", "\"/=\"",
  "\"//=\"", "\"<=\"", "\">=\"", "QQSTRING_START", "QQSTRING_TEXT",
  "QQSTRING_INTERP_START", "QQSTRING_INTERP_END", "QQSTRING_END", "'|'",
  "','", "'='", "'<'", "'>'", "'+'", "'-'", "'*'", "'/'", "'%'", "'?'",
  "';'", "'('", "')'", "'$'", "':'", "'.'", "'['", "']'", "'{'", "'}'",
  "$accept", "TopLevel", "Module", "Imports", "FuncDefs", "Exp", "Import",
  "FuncDef", "Params", "Param", "String", "@1", "@2", "QQString",
  "ElseBody", "ExpD", "Term", "Args", "Arg", "Pattern", "ArrayPats",
  "ObjPats", "ObjPat", "Keyword", "MkDict", "MkDictPair", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   124,    44,    61,    60,    62,    43,
      45,    42,    47,    37,    63,    59,    40,    41,    36,    58,
      46,    91,    93,   123,   125
};
# endif

#define YYPACT_NINF -141

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-141)))

#define YYTABLE_NINF -143

#define yytable_value_is_error(Yytable_value) \
  (!!((Yytable_value) == (-143)))

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
      25,   806,    31,    33,    -5,    -2,  -141,    17,  -141,    64,
     806,   666,   666,   806,    18,     0,  -141,   806,   452,    11,
     271,   426,   333,  1284,   806,  -141,     6,  -141,     3,   806,
      33,   806,  -141,  -141,   -21,  1659,     9,    48,    45,    71,
    -141,    73,  -141,    -6,    56,  1129,  -141,  -141,  -141,    78,
      17,    63,    57,  -141,   942,   -32,    68,  -141,  -141,  -141,
    -141,  -141,  -141,  -141,  -141,  -141,  -141,  -141,  -141,  -141,
    -141,  -141,  -141,  -141,  -141,   513,   116,    69,    72,    59,
      85,   806,   806,   806,   806,   806,   806,   806,   806,   806,
     806,   806,   806,   806,   806,   806,   806,   806,   806,   806,
     806,   806,   806,   806,   806,  -141,  -141,  1813,    79,   -24,
       3,   143,   122,  -141,  1813,   806,  -141,  1813,    16,  -141,
    -141,     2,   806,   539,   -24,   -24,   600,    92,  -141,    14,
    -141,  -141,  -141,  -141,  -141,  -141,   386,  -141,   204,    83,
    1160,  -141,   204,   204,  -141,   386,  1875,   328,   328,  1844,
    1922,  1893,  1875,  1875,  1875,  1875,  1875,  1875,   328,   328,
    1813,  1844,  1875,   328,   328,    -6,    -6,    87,    87,    87,
    -141,   138,   -24,   861,    99,    91,    90,   806,   100,   896,
      20,  -141,   806,  -141,    22,  -141,   149,    61,  -141,  1331,
    -141,  1613,   102,   103,  -141,  -141,   806,  -141,   806,  -141,
     110,  -141,   204,   112,    54,   104,   107,   112,   112,  -141,
    -141,  -141,   -19,   108,   806,   157,   111,   -20,  -141,   114,
     806,  -141,  -141,   988,  -141,   626,   121,   686,   158,  -141,
    -141,  -141,     2,   118,  -141,   806,   806,  -141,   806,   806,
    1813,  1690,  -141,   204,   204,   204,   -24,  -141,   -24,  1191,
    -141,   -24,   861,  -141,   -24,  1813,   124,   125,  1034,  -141,
    -141,  1378,   746,  -141,   806,  1736,  1782,  1425,  1472,  -141,
     112,   112,   112,  -141,  -141,   126,  -141,  -141,  -141,  -141,
    -141,   129,  -141,  -141,  1519,  1566,  -141,   806,   806,   806,
     -24,  -141,  -141,  -141,  1613,  1222,  1080,  -141,  -141,  -141,
     806,  -141,  1253,  -141
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       4,     0,     0,     6,   103,    79,    94,    96,    71,     0,
       0,     0,     0,     0,     0,     0,    58,     0,     0,     0,
       0,     0,     0,     0,     0,    95,    47,     1,     0,     8,
       6,     0,    75,    60,     0,     0,     0,     0,    18,     0,
      73,     0,    62,    32,     0,     0,   102,   101,    82,     0,
       0,    81,     0,    99,     0,     0,   151,   124,   125,   126,
     127,   128,   129,   130,   131,   132,   133,   134,   135,   136,
     137,   138,   139,   140,   141,     0,     0,   149,     0,     0,
     143,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    21,     5,    10,    78,     0,
       0,     0,     0,     3,     2,     8,     7,   111,     0,   109,
      62,     0,     0,     0,     0,     0,     0,     0,    72,     0,
     105,    97,    83,    77,   106,    98,     0,   108,     0,     0,
       0,   150,     0,     0,   100,     0,    40,    41,    42,    25,
      24,    23,    27,    31,    34,    36,    39,    26,    45,    46,
      28,    29,    22,    43,    44,    30,    33,    35,    37,    38,
      74,     0,     0,     0,     0,    80,     0,     0,    87,     0,
       0,     9,     0,   104,     0,    57,     0,     0,    54,     0,
      16,     0,     0,     0,    19,    17,     0,    63,     0,    59,
       0,   145,     0,   146,    69,     0,     0,   148,   147,   144,
     112,   115,     0,     0,     0,     0,     0,     0,   117,     0,
       0,    76,   107,     0,    86,     0,    85,     0,     0,   110,
      61,    56,     0,     0,    52,     0,     0,    15,     0,     0,
      20,     0,    68,     0,     0,     0,     0,   113,     0,     0,
     119,     0,     0,   114,     0,    11,    93,    92,     0,    84,
      49,     0,     0,    55,     0,     0,     0,     0,     0,    64,
      67,   153,   152,   116,   120,     0,   122,   118,   121,    90,
      89,    91,    50,    48,     0,     0,    66,     0,     0,     0,
       0,    88,    51,    53,     0,     0,     0,   123,    65,    12,
       0,    14,     0,    13
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -141,  -141,  -141,   150,    74,    -1,  -141,   -11,  -141,   -48,
       1,  -141,  -141,    66,  -107,  -138,    -4,  -141,     8,  -122,
    -141,  -141,   -64,  -140,  -109,  -141
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     2,     3,    29,   113,   107,    30,    24,   187,   188,
      25,    42,   120,   129,   237,   203,    26,   118,   119,   174,
     212,   217,   218,    78,    79,    80
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      23,    40,   192,   193,   207,   208,   185,    36,    37,    35,
      50,   108,    38,   136,   108,    46,    43,    45,   115,   109,
      54,    51,   124,    77,   227,   252,   246,   201,   114,   112,
     117,    27,   137,   219,   171,   121,   209,   172,   122,   173,
       1,    47,    16,   247,   253,   102,   103,   104,   105,    28,
     211,    31,    32,   108,   197,   198,    33,   199,    41,   108,
     186,   125,   197,   198,   242,   230,   110,   111,    34,   110,
     111,   182,   126,   183,   140,   127,    39,   128,   228,   132,
     146,   147,   148,   149,   150,   151,   152,   153,   154,   155,
     156,   157,   158,   159,   160,   161,   162,   163,   164,   165,
     166,   167,   168,   169,   115,   270,   271,   272,   110,   111,
     179,   175,   219,   130,   110,   111,   232,   133,   233,   134,
     141,   189,   191,   144,   273,   195,   274,   138,   142,   276,
     145,   143,   278,   170,   204,   180,   196,    77,   204,   204,
     205,   105,   210,   220,   176,   221,    77,     4,     5,     6,
       7,     8,   222,   231,   224,   136,   243,     9,   238,   239,
      10,   250,   262,   244,    11,    12,   245,   248,   297,    13,
     251,    14,    15,   254,   216,   259,   223,   264,   279,   280,
     116,   117,    16,   291,   263,   290,   184,   298,   277,   181,
     229,     0,     0,    17,     0,   240,     0,   241,   204,    18,
       0,    19,   177,    20,    21,   178,    22,     0,     4,     5,
       6,     7,     8,   249,     0,     0,     0,     0,     0,   255,
       0,     0,     0,     0,   258,     0,   261,     0,     0,     0,
       0,     0,     0,    15,   265,   266,     0,   267,   268,   204,
     204,   204,     0,    16,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   216,   202,     0,     0,     0,     0,     0,
      18,   284,    19,   285,    20,    21,     0,    22,     0,     0,
       0,   -70,    48,     0,     0,    49,   -70,     0,    50,     0,
     -70,   -70,   -70,   -70,   -70,     0,   294,   295,   296,   -70,
     -70,   -70,     0,     0,   -70,   -70,   -70,     0,   -70,   302,
       0,     0,   -70,   -70,   -70,   -70,   -70,   -70,   -70,   -70,
      16,     0,     0,   -70,     0,   -70,   -70,   -70,   -70,   -70,
     -70,   -70,   -70,   -70,   -70,   -70,   -70,     0,   -70,     0,
     -70,   -70,   -70,   -70,    55,   -70,     0,    56,  -143,  -143,
      50,     0,     0,     0,     0,     0,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,     0,  -143,  -143,     0,     0,     0,
       0,     0,    16,     0,     0,  -143,  -143,   100,   101,   102,
     103,   104,   105,     0,     0,     0,     0,   200,     0,    75,
      56,    76,     0,    50,     0,     0,     0,  -142,     0,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,     0,     0,     0,
       0,     0,     0,     0,     0,    16,     0,    52,     0,     0,
       4,     5,     6,     7,     8,     0,     0,     0,     0,     0,
       9,     0,    75,    10,    76,     0,     0,    11,    12,     0,
    -142,     0,    13,    44,    14,    15,     4,     5,     6,     7,
       8,     0,     0,     0,     0,    16,     9,     0,     0,    10,
       0,     0,     0,    11,    12,     0,    17,     0,    13,     0,
      14,    15,    18,     0,    19,     0,    20,    21,    53,    22,
       0,    16,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    17,     0,     0,     0,     0,     0,    18,     0,
      19,     0,    20,    21,   139,    22,     0,     4,     5,     6,
       7,     8,     0,     0,     0,     0,     0,     9,     0,     0,
      10,     0,     0,     0,    11,    12,     0,     0,     0,    13,
     190,    14,    15,     4,     5,     6,     7,     8,     0,     0,
       0,     0,    16,     9,     0,     0,    10,     0,     0,     0,
      11,    12,     0,    17,     0,    13,     0,    14,    15,    18,
       0,    19,     0,    20,    21,     0,    22,     0,    16,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    17,
       0,     0,     0,     0,     0,    18,     0,    19,     0,    20,
      21,   194,    22,     0,     4,     5,     6,     7,     8,     0,
       0,     0,     0,     0,     9,     0,     0,    10,     0,     0,
       0,    11,    12,     0,     0,     0,    13,     0,    14,    15,
       4,     5,     6,     7,     8,     0,     0,     0,     0,    16,
       9,     0,     0,    10,     0,     0,     0,    11,    12,     0,
      17,     0,    13,     0,    14,    15,    18,     0,    19,     0,
      20,    21,     0,    22,     0,    16,     0,     0,     0,     0,
       4,     5,     6,     7,     8,     0,    17,     0,     0,     0,
       0,     0,    18,     0,    19,     0,    20,    21,   257,    22,
       4,     5,     6,     7,     8,    15,     0,     0,     0,     0,
       9,     0,     0,    10,     0,    16,     0,    11,    12,     0,
       0,     0,    13,     0,    14,    15,     0,     0,     0,     0,
       0,     0,    18,     0,    19,    16,    20,    21,     0,    22,
       0,     0,     0,     0,     0,     0,    17,     0,     0,     0,
       0,   260,    18,     0,    19,     0,    20,    21,     0,    22,
       4,     5,     6,     7,     8,     0,     0,     0,     0,     0,
       9,     0,     0,    10,     0,     0,     0,    11,    12,     0,
       0,     0,    13,     0,    14,    15,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    16,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    17,     0,     0,     0,
       0,   283,    18,     0,    19,     0,    20,    21,     0,    22,
       4,     5,     6,     7,     8,     0,     0,     0,     0,     0,
       9,     0,     0,    10,     0,     0,     0,    11,    12,     0,
       0,     0,    13,     0,    14,    15,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    16,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    17,     0,     0,     0,
       0,     0,    18,     0,    19,   213,    20,    21,    50,    22,
       0,     0,     0,     0,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,     0,     0,     0,     0,     0,     0,     0,     0,
      16,     0,     0,     0,     0,    81,    82,    83,    84,     0,
       0,     0,     0,     0,     0,     0,     0,   214,     0,   215,
      85,    86,     0,     0,     0,     0,     0,    87,    88,    89,
      90,    91,    92,    93,    94,     0,     0,     0,     0,     0,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,    81,    82,    83,    84,   225,     0,     0,   226,     0,
       0,     0,     0,     0,     0,     0,    85,    86,     0,     0,
       0,     0,     0,    87,    88,    89,    90,    91,    92,    93,
      94,     0,     0,     0,     0,     0,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,    81,    82,    83,
      84,     0,     0,     0,   135,     0,     0,     0,     0,     0,
       0,     0,    85,    86,     0,     0,     0,     0,     0,    87,
      88,    89,    90,    91,    92,    93,    94,     0,     0,     0,
       0,     0,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,    81,    82,    83,    84,     0,     0,     0,
     256,     0,     0,     0,     0,     0,     0,     0,    85,    86,
       0,     0,     0,     0,     0,    87,    88,    89,    90,    91,
      92,    93,    94,     0,     0,     0,     0,     0,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,    81,
      82,    83,    84,     0,     0,     0,   281,     0,     0,     0,
       0,     0,     0,     0,    85,    86,     0,     0,     0,     0,
       0,    87,    88,    89,    90,    91,    92,    93,    94,     0,
       0,     0,     0,     0,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   300,     0,   301,    81,    82,
      83,    84,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    85,    86,     0,     0,     0,     0,     0,
      87,    88,    89,    90,    91,    92,    93,    94,     0,    81,
      82,    83,    84,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,    85,    86,   131,     0,     0,     0,
       0,    87,    88,    89,    90,    91,    92,    93,    94,     0,
      81,    82,    83,    84,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,    85,    86,   206,     0,     0,
       0,     0,    87,    88,    89,    90,    91,    92,    93,    94,
       0,    81,    82,    83,    84,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,    85,    86,   275,     0,
       0,     0,     0,    87,    88,    89,    90,    91,    92,    93,
      94,     0,    81,    82,    83,    84,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,    85,    86,   299,
       0,     0,     0,     0,    87,    88,    89,    90,    91,    92,
      93,    94,     0,    81,    82,    83,    84,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,    85,    86,
     303,     0,     0,     0,     0,    87,    88,    89,    90,    91,
      92,    93,    94,     0,     0,     0,     0,     0,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
      81,    82,    83,    84,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    85,    86,     0,     0,     0,
       0,     0,    87,    88,    89,    90,    91,    92,    93,    94,
       0,     0,     0,     0,     0,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   234,    81,    82,    83,
      84,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    85,    86,     0,     0,     0,     0,     0,    87,
      88,    89,    90,    91,    92,    93,    94,     0,     0,     0,
       0,     0,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   282,    81,    82,    83,    84,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    85,
      86,     0,     0,     0,     0,     0,    87,    88,    89,    90,
      91,    92,    93,    94,     0,     0,     0,     0,     0,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     288,    81,    82,    83,    84,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    85,    86,     0,     0,
       0,     0,     0,    87,    88,    89,    90,    91,    92,    93,
      94,     0,     0,     0,     0,     0,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   289,    81,    82,
      83,    84,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    85,    86,     0,     0,     0,     0,     0,
      87,    88,    89,    90,    91,    92,    93,    94,     0,     0,
       0,     0,     0,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   292,    81,    82,    83,    84,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      85,    86,     0,     0,     0,     0,     0,    87,    88,    89,
      90,    91,    92,    93,    94,     0,     0,     0,     0,     0,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   293,    81,    82,    83,    84,     0,     0,     0,     0,
       0,     0,   235,   236,     0,     0,     0,    85,    86,     0,
       0,     0,     0,     0,    87,    88,    89,    90,    91,    92,
      93,    94,     0,     0,     0,     0,     0,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,    81,    82,
      83,    84,     0,     0,     0,     0,     0,   123,     0,     0,
       0,     0,     0,    85,    86,     0,     0,     0,     0,     0,
      87,    88,    89,    90,    91,    92,    93,    94,     0,    81,
      82,    83,    84,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,    85,    86,     0,     0,     0,     0,
       0,    87,    88,    89,    90,    91,    92,    93,    94,     0,
       0,     0,   269,     0,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,    81,    82,    83,    84,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   286,
      85,    86,     0,     0,     0,     0,     0,    87,    88,    89,
      90,    91,    92,    93,    94,     0,     0,     0,     0,     0,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,    81,    82,    83,    84,     0,     0,     0,     0,     0,
     287,     0,     0,     0,     0,     0,    85,    86,     0,     0,
       0,     0,     0,    87,    88,    89,    90,    91,    92,    93,
      94,     0,    81,    82,    83,    84,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,    85,    86,     0,
       0,     0,     0,     0,    87,    88,    89,    90,    91,    92,
      93,    94,     0,    81,    82,    83,    84,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,    85,    86,
       0,     0,     0,     0,     0,    87,    88,    89,    90,    91,
      92,    93,    94,     0,  -143,    82,    83,     0,     0,     0,
      97,    98,    99,   100,   101,   102,   103,   104,   105,    85,
      86,     0,     0,    82,    83,     0,  -143,  -143,  -143,  -143,
    -143,  -143,    93,    94,     0,     0,     0,    85,     0,     0,
       0,  -143,    98,    99,   100,   101,   102,   103,   104,   105,
      93,    94,    82,    83,     0,     0,     0,     0,     0,     0,
      98,    99,   100,   101,   102,   103,   104,   105,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    93,
      94,     0,     0,     0,     0,     0,     0,     0,     0,    98,
      99,   100,   101,   102,   103,   104,   105
};

static const yytype_int16 yycheck[] =
{
       1,     1,   124,   125,   142,   143,     4,    11,    12,    10,
       7,     5,    13,    45,     5,     4,    17,    18,    29,    13,
      21,    20,    13,    22,     4,    45,    45,   136,    29,    28,
      31,     0,    64,   173,    58,    56,   145,    61,    59,    63,
      15,    30,    39,    62,    64,    51,    52,    53,    54,    16,
     172,    56,    54,     5,    40,    41,    39,    43,    58,     5,
      58,    13,    40,    41,   202,    43,    60,    61,     4,    60,
      61,    55,    27,    57,    75,     4,    58,     4,    58,     1,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   115,   243,   244,   245,    60,    61,
     111,   110,   252,    57,    60,    61,    55,    54,    57,    62,
       4,   122,   123,    64,   246,   126,   248,    59,    59,   251,
      45,    59,   254,    54,   138,    13,    44,   136,   142,   143,
      57,    54,     4,    44,     1,    54,   145,     4,     5,     6,
       7,     8,    62,     4,    54,    45,    44,    14,    56,    56,
      17,     4,     4,    59,    21,    22,    59,    59,   290,    26,
      59,    28,    29,    59,   173,    54,   177,    59,    54,    54,
      30,   182,    39,    54,   232,    59,   120,   294,   252,   115,
     182,    -1,    -1,    50,    -1,   196,    -1,   198,   202,    56,
      -1,    58,    59,    60,    61,    62,    63,    -1,     4,     5,
       6,     7,     8,   214,    -1,    -1,    -1,    -1,    -1,   220,
      -1,    -1,    -1,    -1,   225,    -1,   227,    -1,    -1,    -1,
      -1,    -1,    -1,    29,   235,   236,    -1,   238,   239,   243,
     244,   245,    -1,    39,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   252,    50,    -1,    -1,    -1,    -1,    -1,
      56,   262,    58,   264,    60,    61,    -1,    63,    -1,    -1,
      -1,     0,     1,    -1,    -1,     4,     5,    -1,     7,    -1,
       9,    10,    11,    12,    13,    -1,   287,   288,   289,    18,
      19,    20,    -1,    -1,    23,    24,    25,    -1,    27,   300,
      -1,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    -1,    -1,    42,    -1,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    57,    -1,
      59,    60,    61,    62,     1,    64,    -1,     4,    10,    11,
       7,    -1,    -1,    -1,    -1,    -1,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    -1,    37,    38,    -1,    -1,    -1,
      -1,    -1,    39,    -1,    -1,    47,    48,    49,    50,    51,
      52,    53,    54,    -1,    -1,    -1,    -1,     1,    -1,    56,
       4,    58,    -1,     7,    -1,    -1,    -1,    64,    -1,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    39,    -1,     1,    -1,    -1,
       4,     5,     6,     7,     8,    -1,    -1,    -1,    -1,    -1,
      14,    -1,    56,    17,    58,    -1,    -1,    21,    22,    -1,
      64,    -1,    26,     1,    28,    29,     4,     5,     6,     7,
       8,    -1,    -1,    -1,    -1,    39,    14,    -1,    -1,    17,
      -1,    -1,    -1,    21,    22,    -1,    50,    -1,    26,    -1,
      28,    29,    56,    -1,    58,    -1,    60,    61,    62,    63,
      -1,    39,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    50,    -1,    -1,    -1,    -1,    -1,    56,    -1,
      58,    -1,    60,    61,     1,    63,    -1,     4,     5,     6,
       7,     8,    -1,    -1,    -1,    -1,    -1,    14,    -1,    -1,
      17,    -1,    -1,    -1,    21,    22,    -1,    -1,    -1,    26,
       1,    28,    29,     4,     5,     6,     7,     8,    -1,    -1,
      -1,    -1,    39,    14,    -1,    -1,    17,    -1,    -1,    -1,
      21,    22,    -1,    50,    -1,    26,    -1,    28,    29,    56,
      -1,    58,    -1,    60,    61,    -1,    63,    -1,    39,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,
      -1,    -1,    -1,    -1,    -1,    56,    -1,    58,    -1,    60,
      61,     1,    63,    -1,     4,     5,     6,     7,     8,    -1,
      -1,    -1,    -1,    -1,    14,    -1,    -1,    17,    -1,    -1,
      -1,    21,    22,    -1,    -1,    -1,    26,    -1,    28,    29,
       4,     5,     6,     7,     8,    -1,    -1,    -1,    -1,    39,
      14,    -1,    -1,    17,    -1,    -1,    -1,    21,    22,    -1,
      50,    -1,    26,    -1,    28,    29,    56,    -1,    58,    -1,
      60,    61,    -1,    63,    -1,    39,    -1,    -1,    -1,    -1,
       4,     5,     6,     7,     8,    -1,    50,    -1,    -1,    -1,
      -1,    -1,    56,    -1,    58,    -1,    60,    61,    62,    63,
       4,     5,     6,     7,     8,    29,    -1,    -1,    -1,    -1,
      14,    -1,    -1,    17,    -1,    39,    -1,    21,    22,    -1,
      -1,    -1,    26,    -1,    28,    29,    -1,    -1,    -1,    -1,
      -1,    -1,    56,    -1,    58,    39,    60,    61,    -1,    63,
      -1,    -1,    -1,    -1,    -1,    -1,    50,    -1,    -1,    -1,
      -1,    55,    56,    -1,    58,    -1,    60,    61,    -1,    63,
       4,     5,     6,     7,     8,    -1,    -1,    -1,    -1,    -1,
      14,    -1,    -1,    17,    -1,    -1,    -1,    21,    22,    -1,
      -1,    -1,    26,    -1,    28,    29,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    39,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    50,    -1,    -1,    -1,
      -1,    55,    56,    -1,    58,    -1,    60,    61,    -1,    63,
       4,     5,     6,     7,     8,    -1,    -1,    -1,    -1,    -1,
      14,    -1,    -1,    17,    -1,    -1,    -1,    21,    22,    -1,
      -1,    -1,    26,    -1,    28,    29,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    39,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    50,    -1,    -1,    -1,
      -1,    -1,    56,    -1,    58,     4,    60,    61,     7,    63,
      -1,    -1,    -1,    -1,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      39,    -1,    -1,    -1,    -1,     9,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    56,    -1,    58,
      24,    25,    -1,    -1,    -1,    -1,    -1,    31,    32,    33,
      34,    35,    36,    37,    38,    -1,    -1,    -1,    -1,    -1,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,     9,    10,    11,    12,    59,    -1,    -1,    62,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    24,    25,    -1,    -1,
      -1,    -1,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    -1,    -1,    -1,    -1,    -1,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,     9,    10,    11,
      12,    -1,    -1,    -1,    62,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    24,    25,    -1,    -1,    -1,    -1,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    -1,    -1,    -1,
      -1,    -1,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,     9,    10,    11,    12,    -1,    -1,    -1,
      62,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    24,    25,
      -1,    -1,    -1,    -1,    -1,    31,    32,    33,    34,    35,
      36,    37,    38,    -1,    -1,    -1,    -1,    -1,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,     9,
      10,    11,    12,    -1,    -1,    -1,    62,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    24,    25,    -1,    -1,    -1,    -1,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    -1,
      -1,    -1,    -1,    -1,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    57,     9,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    24,    25,    -1,    -1,    -1,    -1,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    -1,     9,
      10,    11,    12,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    24,    25,    57,    -1,    -1,    -1,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    -1,
       9,    10,    11,    12,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    24,    25,    57,    -1,    -1,
      -1,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      -1,     9,    10,    11,    12,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    24,    25,    57,    -1,
      -1,    -1,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    -1,     9,    10,    11,    12,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    24,    25,    57,
      -1,    -1,    -1,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    -1,     9,    10,    11,    12,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    24,    25,
      57,    -1,    -1,    -1,    -1,    31,    32,    33,    34,    35,
      36,    37,    38,    -1,    -1,    -1,    -1,    -1,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
       9,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    24,    25,    -1,    -1,    -1,
      -1,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      -1,    -1,    -1,    -1,    -1,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,     9,    10,    11,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    24,    25,    -1,    -1,    -1,    -1,    -1,    31,
      32,    33,    34,    35,    36,    37,    38,    -1,    -1,    -1,
      -1,    -1,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,     9,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    24,
      25,    -1,    -1,    -1,    -1,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    -1,    -1,    -1,    -1,    -1,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,     9,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    24,    25,    -1,    -1,
      -1,    -1,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    -1,    -1,    -1,    -1,    -1,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,     9,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    24,    25,    -1,    -1,    -1,    -1,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    -1,    -1,
      -1,    -1,    -1,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,     9,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      24,    25,    -1,    -1,    -1,    -1,    -1,    31,    32,    33,
      34,    35,    36,    37,    38,    -1,    -1,    -1,    -1,    -1,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,     9,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    19,    20,    -1,    -1,    -1,    24,    25,    -1,
      -1,    -1,    -1,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    -1,    -1,    -1,    -1,    -1,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,     9,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    18,    -1,    -1,
      -1,    -1,    -1,    24,    25,    -1,    -1,    -1,    -1,    -1,
      31,    32,    33,    34,    35,    36,    37,    38,    -1,     9,
      10,    11,    12,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    24,    25,    -1,    -1,    -1,    -1,
      -1,    31,    32,    33,    34,    35,    36,    37,    38,    -1,
      -1,    -1,    42,    -1,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,     9,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    23,
      24,    25,    -1,    -1,    -1,    -1,    -1,    31,    32,    33,
      34,    35,    36,    37,    38,    -1,    -1,    -1,    -1,    -1,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,     9,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      18,    -1,    -1,    -1,    -1,    -1,    24,    25,    -1,    -1,
      -1,    -1,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    -1,     9,    10,    11,    12,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    24,    25,    -1,
      -1,    -1,    -1,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    -1,     9,    10,    11,    12,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    24,    25,
      -1,    -1,    -1,    -1,    -1,    31,    32,    33,    34,    35,
      36,    37,    38,    -1,     9,    10,    11,    -1,    -1,    -1,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    24,
      25,    -1,    -1,    10,    11,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    -1,    -1,    -1,    24,    -1,    -1,
      -1,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      37,    38,    10,    11,    -1,    -1,    -1,    -1,    -1,    -1,
      47,    48,    49,    50,    51,    52,    53,    54,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    37,
      38,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    47,
      48,    49,    50,    51,    52,    53,    54
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    15,    66,    67,     4,     5,     6,     7,     8,    14,
      17,    21,    22,    26,    28,    29,    39,    50,    56,    58,
      60,    61,    63,    70,    72,    75,    81,     0,    16,    68,
      71,    56,    54,    39,     4,    70,    81,    81,    70,    58,
       1,    58,    76,    70,     1,    70,     4,    30,     1,     4,
       7,    75,     1,    62,    70,     1,     4,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    56,    58,    75,    88,    89,
      90,     9,    10,    11,    12,    24,    25,    31,    32,    33,
      34,    35,    36,    37,    38,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    70,     5,    13,
      60,    61,    75,    69,    70,    72,    68,    70,    82,    83,
      77,    56,    59,    18,    13,    13,    27,     4,     4,    78,
      57,    57,     1,    54,    62,    62,    45,    64,    59,     1,
      70,     4,    59,    59,    64,    45,    70,    70,    70,    70,
      70,    70,    70,    70,    70,    70,    70,    70,    70,    70,
      70,    70,    70,    70,    70,    70,    70,    70,    70,    70,
      54,    58,    61,    63,    84,    75,     1,    59,    62,    70,
      13,    69,    55,    57,    78,     4,    58,    73,    74,    70,
       1,    70,    84,    84,     1,    70,    44,    40,    41,    43,
       1,    89,    50,    80,    81,    57,    57,    80,    80,    89,
       4,    84,    85,     4,    56,    58,    75,    86,    87,    88,
      44,    54,    62,    70,    54,    59,    62,     4,    58,    83,
      43,     4,    55,    57,    55,    19,    20,    79,    56,    56,
      70,    70,    80,    44,    59,    59,    45,    62,    59,    70,
       4,    59,    45,    64,    59,    70,    62,    62,    70,    54,
      55,    70,     4,    74,    59,    70,    70,    70,    70,    42,
      80,    80,    80,    84,    84,    57,    84,    87,    84,    54,
      54,    62,    55,    55,    70,    70,    23,    18,    55,    55,
      59,    54,    55,    55,    70,    70,    70,    84,    79,    57,
      55,    57,    70,    57
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    65,    66,    66,    67,    67,    68,    68,    69,    69,
      70,    70,    70,    70,    70,    70,    70,    70,    70,    70,
      70,    70,    70,    70,    70,    70,    70,    70,    70,    70,
      70,    70,    70,    70,    70,    70,    70,    70,    70,    70,
      70,    70,    70,    70,    70,    70,    70,    70,    71,    71,
      71,    71,    72,    72,    73,    73,    74,    74,    76,    75,
      77,    75,    78,    78,    78,    79,    79,    80,    80,    80,
      81,    81,    81,    81,    81,    81,    81,    81,    81,    81,
      81,    81,    81,    81,    81,    81,    81,    81,    81,    81,
      81,    81,    81,    81,    81,    81,    81,    81,    81,    81,
      81,    81,    81,    81,    81,    81,    81,    81,    81,    82,
      82,    83,    84,    84,    84,    85,    85,    86,    86,    87,
      87,    87,    87,    87,    88,    88,    88,    88,    88,    88,
      88,    88,    88,    88,    88,    88,    88,    88,    88,    88,
      88,    88,    89,    89,    89,    89,    90,    90,    90,    90,
      90,    90,    90,    90
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     3,     3,     0,     3,     0,     2,     0,     2,
       2,     5,     9,    11,     9,     5,     4,     4,     2,     4,
       5,     2,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     2,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     1,     6,     5,
       6,     7,     5,     8,     1,     3,     2,     1,     0,     4,
       0,     5,     0,     2,     4,     5,     3,     3,     2,     1,
       1,     1,     3,     2,     3,     2,     4,     3,     2,     1,
       3,     2,     2,     3,     5,     4,     4,     3,     7,     6,
       6,     6,     5,     5,     1,     1,     1,     3,     3,     2,
       3,     2,     2,     1,     4,     3,     3,     4,     3,     1,
       3,     1,     2,     3,     3,     1,     3,     1,     3,     2,
       3,     3,     3,     5,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     0,     1,     3,     3,     3,     3,     3,     1,
       2,     1,     5,     5
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
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
      yyerror (&yylloc, answer, errors, locations, lexer_param_ptr, YY_("syntax error: cannot back up")); \
      YYERROR;                                                  \
    }                                                           \
while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)                                \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;        \
          (Current).first_column = YYRHSLOC (Rhs, 1).first_column;      \
          (Current).last_line    = YYRHSLOC (Rhs, N).last_line;         \
          (Current).last_column  = YYRHSLOC (Rhs, N).last_column;       \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).first_line   = (Current).last_line   =              \
            YYRHSLOC (Rhs, 0).last_line;                                \
          (Current).first_column = (Current).last_column =              \
            YYRHSLOC (Rhs, 0).last_column;                              \
        }                                                               \
    while (0)
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K])


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


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL

/* Print *YYLOCP on YYO.  Private, do not rely on its existence. */

YY_ATTRIBUTE_UNUSED
static unsigned
yy_location_print_ (FILE *yyo, YYLTYPE const * const yylocp)
{
  unsigned res = 0;
  int end_col = 0 != yylocp->last_column ? yylocp->last_column - 1 : 0;
  if (0 <= yylocp->first_line)
    {
      res += YYFPRINTF (yyo, "%d", yylocp->first_line);
      if (0 <= yylocp->first_column)
        res += YYFPRINTF (yyo, ".%d", yylocp->first_column);
    }
  if (0 <= yylocp->last_line)
    {
      if (yylocp->first_line < yylocp->last_line)
        {
          res += YYFPRINTF (yyo, "-%d", yylocp->last_line);
          if (0 <= end_col)
            res += YYFPRINTF (yyo, ".%d", end_col);
        }
      else if (0 <= end_col && yylocp->first_column < end_col)
        res += YYFPRINTF (yyo, "-%d", end_col);
    }
  return res;
 }

#  define YY_LOCATION_PRINT(File, Loc)          \
  yy_location_print_ (File, &(Loc))

# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value, Location, answer, errors, locations, lexer_param_ptr); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, block* answer, int* errors, struct locfile* locations, struct lexer_param* lexer_param_ptr)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  YYUSE (yylocationp);
  YYUSE (answer);
  YYUSE (errors);
  YYUSE (locations);
  YYUSE (lexer_param_ptr);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  YYUSE (yytype);
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, block* answer, int* errors, struct locfile* locations, struct lexer_param* lexer_param_ptr)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  YY_LOCATION_PRINT (yyoutput, *yylocationp);
  YYFPRINTF (yyoutput, ": ");
  yy_symbol_value_print (yyoutput, yytype, yyvaluep, yylocationp, answer, errors, locations, lexer_param_ptr);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
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
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, YYLTYPE *yylsp, int yyrule, block* answer, int* errors, struct locfile* locations, struct lexer_param* lexer_param_ptr)
{
  unsigned long int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[yyssp[yyi + 1 - yynrhs]],
                       &(yyvsp[(yyi + 1) - (yynrhs)])
                       , &(yylsp[(yyi + 1) - (yynrhs)])                       , answer, errors, locations, lexer_param_ptr);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, yylsp, Rule, answer, errors, locations, lexer_param_ptr); \
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
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
yystrlen (const char *yystr)
{
  YYSIZE_T yylen;
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
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
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
            /* Fall through.  */
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

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
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
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

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
      int yyn = yypact[*yyssp];
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
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (! (yysize <= yysize1
                         && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  yysize = yysize1;
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
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
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
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, YYLTYPE *yylocationp, block* answer, int* errors, struct locfile* locations, struct lexer_param* lexer_param_ptr)
{
  YYUSE (yyvaluep);
  YYUSE (yylocationp);
  YYUSE (answer);
  YYUSE (errors);
  YYUSE (locations);
  YYUSE (lexer_param_ptr);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  switch (yytype)
    {
          case 4: /* IDENT  */
#line 36 "parser.y" /* yacc.c:1257  */
      { jv_free(((*yyvaluep).literal)); }
#line 1857 "parser.c" /* yacc.c:1257  */
        break;

    case 5: /* FIELD  */
#line 36 "parser.y" /* yacc.c:1257  */
      { jv_free(((*yyvaluep).literal)); }
#line 1863 "parser.c" /* yacc.c:1257  */
        break;

    case 6: /* LITERAL  */
#line 36 "parser.y" /* yacc.c:1257  */
      { jv_free(((*yyvaluep).literal)); }
#line 1869 "parser.c" /* yacc.c:1257  */
        break;

    case 7: /* FORMAT  */
#line 36 "parser.y" /* yacc.c:1257  */
      { jv_free(((*yyvaluep).literal)); }
#line 1875 "parser.c" /* yacc.c:1257  */
        break;

    case 40: /* QQSTRING_TEXT  */
#line 36 "parser.y" /* yacc.c:1257  */
      { jv_free(((*yyvaluep).literal)); }
#line 1881 "parser.c" /* yacc.c:1257  */
        break;

    case 67: /* Module  */
#line 37 "parser.y" /* yacc.c:1257  */
      { block_free(((*yyvaluep).blk)); }
#line 1887 "parser.c" /* yacc.c:1257  */
        break;

    case 68: /* Imports  */
#line 37 "parser.y" /* yacc.c:1257  */
      { block_free(((*yyvaluep).blk)); }
#line 1893 "parser.c" /* yacc.c:1257  */
        break;

    case 69: /* FuncDefs  */
#line 37 "parser.y" /* yacc.c:1257  */
      { block_free(((*yyvaluep).blk)); }
#line 1899 "parser.c" /* yacc.c:1257  */
        break;

    case 70: /* Exp  */
#line 37 "parser.y" /* yacc.c:1257  */
      { block_free(((*yyvaluep).blk)); }
#line 1905 "parser.c" /* yacc.c:1257  */
        break;

    case 71: /* Import  */
#line 37 "parser.y" /* yacc.c:1257  */
      { block_free(((*yyvaluep).blk)); }
#line 1911 "parser.c" /* yacc.c:1257  */
        break;

    case 72: /* FuncDef  */
#line 37 "parser.y" /* yacc.c:1257  */
      { block_free(((*yyvaluep).blk)); }
#line 1917 "parser.c" /* yacc.c:1257  */
        break;

    case 73: /* Params  */
#line 37 "parser.y" /* yacc.c:1257  */
      { block_free(((*yyvaluep).blk)); }
#line 1923 "parser.c" /* yacc.c:1257  */
        break;

    case 74: /* Param  */
#line 37 "parser.y" /* yacc.c:1257  */
      { block_free(((*yyvaluep).blk)); }
#line 1929 "parser.c" /* yacc.c:1257  */
        break;

    case 75: /* String  */
#line 37 "parser.y" /* yacc.c:1257  */
      { block_free(((*yyvaluep).blk)); }
#line 1935 "parser.c" /* yacc.c:1257  */
        break;

    case 78: /* QQString  */
#line 37 "parser.y" /* yacc.c:1257  */
      { block_free(((*yyvaluep).blk)); }
#line 1941 "parser.c" /* yacc.c:1257  */
        break;

    case 79: /* ElseBody  */
#line 37 "parser.y" /* yacc.c:1257  */
      { block_free(((*yyvaluep).blk)); }
#line 1947 "parser.c" /* yacc.c:1257  */
        break;

    case 80: /* ExpD  */
#line 37 "parser.y" /* yacc.c:1257  */
      { block_free(((*yyvaluep).blk)); }
#line 1953 "parser.c" /* yacc.c:1257  */
        break;

    case 81: /* Term  */
#line 37 "parser.y" /* yacc.c:1257  */
      { block_free(((*yyvaluep).blk)); }
#line 1959 "parser.c" /* yacc.c:1257  */
        break;

    case 82: /* Args  */
#line 37 "parser.y" /* yacc.c:1257  */
      { block_free(((*yyvaluep).blk)); }
#line 1965 "parser.c" /* yacc.c:1257  */
        break;

    case 83: /* Arg  */
#line 37 "parser.y" /* yacc.c:1257  */
      { block_free(((*yyvaluep).blk)); }
#line 1971 "parser.c" /* yacc.c:1257  */
        break;

    case 84: /* Pattern  */
#line 37 "parser.y" /* yacc.c:1257  */
      { block_free(((*yyvaluep).blk)); }
#line 1977 "parser.c" /* yacc.c:1257  */
        break;

    case 85: /* ArrayPats  */
#line 37 "parser.y" /* yacc.c:1257  */
      { block_free(((*yyvaluep).blk)); }
#line 1983 "parser.c" /* yacc.c:1257  */
        break;

    case 86: /* ObjPats  */
#line 37 "parser.y" /* yacc.c:1257  */
      { block_free(((*yyvaluep).blk)); }
#line 1989 "parser.c" /* yacc.c:1257  */
        break;

    case 87: /* ObjPat  */
#line 37 "parser.y" /* yacc.c:1257  */
      { block_free(((*yyvaluep).blk)); }
#line 1995 "parser.c" /* yacc.c:1257  */
        break;

    case 88: /* Keyword  */
#line 36 "parser.y" /* yacc.c:1257  */
      { jv_free(((*yyvaluep).literal)); }
#line 2001 "parser.c" /* yacc.c:1257  */
        break;

    case 89: /* MkDict  */
#line 37 "parser.y" /* yacc.c:1257  */
      { block_free(((*yyvaluep).blk)); }
#line 2007 "parser.c" /* yacc.c:1257  */
        break;

    case 90: /* MkDictPair  */
#line 37 "parser.y" /* yacc.c:1257  */
      { block_free(((*yyvaluep).blk)); }
#line 2013 "parser.c" /* yacc.c:1257  */
        break;


      default:
        break;
    }
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/*----------.
| yyparse.  |
`----------*/

int
yyparse (block* answer, int* errors, struct locfile* locations, struct lexer_param* lexer_param_ptr)
{
/* The lookahead symbol.  */
int yychar;


/* The semantic value of the lookahead symbol.  */
/* Default value used for initialization, for pacifying older GCCs
   or non-GCC compilers.  */
YY_INITIAL_VALUE (static YYSTYPE yyval_default;)
YYSTYPE yylval YY_INITIAL_VALUE (= yyval_default);

/* Location data for the lookahead symbol.  */
static YYLTYPE yyloc_default
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
  = { 1, 1, 1, 1 }
# endif
;
YYLTYPE yylloc = yyloc_default;

    /* Number of syntax errors so far.  */
    int yynerrs;

    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.
       'yyls': related to locations.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    /* The location stack.  */
    YYLTYPE yylsa[YYINITDEPTH];
    YYLTYPE *yyls;
    YYLTYPE *yylsp;

    /* The locations where the error started and ended.  */
    YYLTYPE yyerror_range[3];

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
  YYLTYPE yyloc;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N), yylsp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yylsp = yyls = yylsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  yylsp[0] = yylloc;
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        YYSTYPE *yyvs1 = yyvs;
        yytype_int16 *yyss1 = yyss;
        YYLTYPE *yyls1 = yyls;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * sizeof (*yyssp),
                    &yyvs1, yysize * sizeof (*yyvsp),
                    &yyls1, yysize * sizeof (*yylsp),
                    &yystacksize);

        yyls = yyls1;
        yyss = yyss1;
        yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yytype_int16 *yyss1 = yyss;
        union yyalloc *yyptr =
          (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
        YYSTACK_RELOCATE (yyls_alloc, yyls);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
      yylsp = yyls + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
                  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

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
      yychar = yylex (&yylval, &yylloc, answer, errors, locations, lexer_param_ptr);
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

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END
  *++yylsp = yylloc;
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
| yyreduce -- Do a reduction.  |
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

  /* Default location.  */
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 271 "parser.y" /* yacc.c:1646  */
    {
  *answer = BLOCK((yyvsp[-2].blk), (yyvsp[-1].blk), gen_op_simple(TOP), (yyvsp[0].blk));
}
#line 2309 "parser.c" /* yacc.c:1646  */
    break;

  case 3:
#line 274 "parser.y" /* yacc.c:1646  */
    {
  *answer = BLOCK((yyvsp[-2].blk), (yyvsp[-1].blk), (yyvsp[0].blk));
}
#line 2317 "parser.c" /* yacc.c:1646  */
    break;

  case 4:
#line 279 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_noop();
}
#line 2325 "parser.c" /* yacc.c:1646  */
    break;

  case 5:
#line 282 "parser.y" /* yacc.c:1646  */
    {
  if (!block_is_const((yyvsp[-1].blk))) {
    FAIL((yyloc), "Module metadata must be constant.");
    (yyval.blk) = gen_noop();
  } else {
    (yyval.blk) = gen_module((yyvsp[-1].blk));
  }
}
#line 2338 "parser.c" /* yacc.c:1646  */
    break;

  case 6:
#line 292 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_noop();
}
#line 2346 "parser.c" /* yacc.c:1646  */
    break;

  case 7:
#line 295 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = BLOCK((yyvsp[-1].blk), (yyvsp[0].blk));
}
#line 2354 "parser.c" /* yacc.c:1646  */
    break;

  case 8:
#line 300 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_noop();
}
#line 2362 "parser.c" /* yacc.c:1646  */
    break;

  case 9:
#line 303 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = block_bind((yyvsp[-1].blk), (yyvsp[0].blk), OP_IS_CALL_PSEUDO);
}
#line 2370 "parser.c" /* yacc.c:1646  */
    break;

  case 10:
#line 308 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = block_bind_referenced((yyvsp[-1].blk), (yyvsp[0].blk), OP_IS_CALL_PSEUDO);
}
#line 2378 "parser.c" /* yacc.c:1646  */
    break;

  case 11:
#line 312 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_destructure((yyvsp[-4].blk), (yyvsp[-2].blk), (yyvsp[0].blk));
}
#line 2386 "parser.c" /* yacc.c:1646  */
    break;

  case 12:
#line 316 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_reduce((yyvsp[-7].blk), (yyvsp[-5].blk), (yyvsp[-3].blk), (yyvsp[-1].blk));
}
#line 2394 "parser.c" /* yacc.c:1646  */
    break;

  case 13:
#line 320 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_foreach((yyvsp[-9].blk), (yyvsp[-7].blk), (yyvsp[-5].blk), (yyvsp[-3].blk), (yyvsp[-1].blk));
}
#line 2402 "parser.c" /* yacc.c:1646  */
    break;

  case 14:
#line 324 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_foreach((yyvsp[-7].blk), (yyvsp[-5].blk), (yyvsp[-3].blk), (yyvsp[-1].blk), gen_noop());
}
#line 2410 "parser.c" /* yacc.c:1646  */
    break;

  case 15:
#line 328 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_cond((yyvsp[-3].blk), (yyvsp[-1].blk), (yyvsp[0].blk));
}
#line 2418 "parser.c" /* yacc.c:1646  */
    break;

  case 16:
#line 331 "parser.y" /* yacc.c:1646  */
    {
  FAIL((yyloc), "Possibly unterminated 'if' statement");
  (yyval.blk) = (yyvsp[-2].blk);
}
#line 2427 "parser.c" /* yacc.c:1646  */
    break;

  case 17:
#line 336 "parser.y" /* yacc.c:1646  */
    {
  //$$ = BLOCK(gen_op_target(FORK_OPT, $2), $2, $4);
  (yyval.blk) = gen_try((yyvsp[-2].blk), gen_try_handler((yyvsp[0].blk)));
}
#line 2436 "parser.c" /* yacc.c:1646  */
    break;

  case 18:
#line 340 "parser.y" /* yacc.c:1646  */
    {
  //$$ = BLOCK(gen_op_target(FORK_OPT, $2), $2, gen_op_simple(BACKTRACK));
  (yyval.blk) = gen_try((yyvsp[0].blk), gen_op_simple(BACKTRACK));
}
#line 2445 "parser.c" /* yacc.c:1646  */
    break;

  case 19:
#line 344 "parser.y" /* yacc.c:1646  */
    {
  FAIL((yyloc), "Possibly unterminated 'try' statement");
  (yyval.blk) = (yyvsp[-2].blk);
}
#line 2454 "parser.c" /* yacc.c:1646  */
    break;

  case 20:
#line 349 "parser.y" /* yacc.c:1646  */
    {
  jv v = jv_string_fmt("*label-%s", jv_string_value((yyvsp[-2].literal)));
  (yyval.blk) = gen_location((yyloc), locations, gen_label(jv_string_value(v), (yyvsp[0].blk)));
  jv_free((yyvsp[-2].literal));
  jv_free(v);
}
#line 2465 "parser.c" /* yacc.c:1646  */
    break;

  case 21:
#line 361 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_try((yyvsp[-1].blk), gen_op_simple(BACKTRACK));
}
#line 2473 "parser.c" /* yacc.c:1646  */
    break;

  case 22:
#line 365 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_call("_assign", BLOCK(gen_lambda((yyvsp[-2].blk)), gen_lambda((yyvsp[0].blk))));
}
#line 2481 "parser.c" /* yacc.c:1646  */
    break;

  case 23:
#line 369 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_or((yyvsp[-2].blk), (yyvsp[0].blk));
}
#line 2489 "parser.c" /* yacc.c:1646  */
    break;

  case 24:
#line 373 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_and((yyvsp[-2].blk), (yyvsp[0].blk));
}
#line 2497 "parser.c" /* yacc.c:1646  */
    break;

  case 25:
#line 377 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_definedor((yyvsp[-2].blk), (yyvsp[0].blk));
}
#line 2505 "parser.c" /* yacc.c:1646  */
    break;

  case 26:
#line 381 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_definedor_assign((yyvsp[-2].blk), (yyvsp[0].blk));
}
#line 2513 "parser.c" /* yacc.c:1646  */
    break;

  case 27:
#line 385 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_call("_modify", BLOCK(gen_lambda((yyvsp[-2].blk)), gen_lambda((yyvsp[0].blk))));
}
#line 2521 "parser.c" /* yacc.c:1646  */
    break;

  case 28:
#line 389 "parser.y" /* yacc.c:1646  */
    { 
  (yyval.blk) = block_join((yyvsp[-2].blk), (yyvsp[0].blk)); 
}
#line 2529 "parser.c" /* yacc.c:1646  */
    break;

  case 29:
#line 393 "parser.y" /* yacc.c:1646  */
    { 
  (yyval.blk) = gen_both((yyvsp[-2].blk), (yyvsp[0].blk)); 
}
#line 2537 "parser.c" /* yacc.c:1646  */
    break;

  case 30:
#line 397 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_binop((yyvsp[-2].blk), (yyvsp[0].blk), '+');
}
#line 2545 "parser.c" /* yacc.c:1646  */
    break;

  case 31:
#line 401 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_update((yyvsp[-2].blk), (yyvsp[0].blk), '+');
}
#line 2553 "parser.c" /* yacc.c:1646  */
    break;

  case 32:
#line 405 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = BLOCK((yyvsp[0].blk), gen_call("_negate", gen_noop()));
}
#line 2561 "parser.c" /* yacc.c:1646  */
    break;

  case 33:
#line 409 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_binop((yyvsp[-2].blk), (yyvsp[0].blk), '-');
}
#line 2569 "parser.c" /* yacc.c:1646  */
    break;

  case 34:
#line 413 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_update((yyvsp[-2].blk), (yyvsp[0].blk), '-');
}
#line 2577 "parser.c" /* yacc.c:1646  */
    break;

  case 35:
#line 417 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_binop((yyvsp[-2].blk), (yyvsp[0].blk), '*');
}
#line 2585 "parser.c" /* yacc.c:1646  */
    break;

  case 36:
#line 421 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_update((yyvsp[-2].blk), (yyvsp[0].blk), '*');
}
#line 2593 "parser.c" /* yacc.c:1646  */
    break;

  case 37:
#line 425 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_binop((yyvsp[-2].blk), (yyvsp[0].blk), '/');
  if (block_is_const_inf((yyval.blk)))
    FAIL((yyloc), "Division by zero?");
}
#line 2603 "parser.c" /* yacc.c:1646  */
    break;

  case 38:
#line 431 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_binop((yyvsp[-2].blk), (yyvsp[0].blk), '%');
  if (block_is_const_inf((yyval.blk)))
    FAIL((yyloc), "Remainder by zero?");
}
#line 2613 "parser.c" /* yacc.c:1646  */
    break;

  case 39:
#line 437 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_update((yyvsp[-2].blk), (yyvsp[0].blk), '/');
}
#line 2621 "parser.c" /* yacc.c:1646  */
    break;

  case 40:
#line 441 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_update((yyvsp[-2].blk), (yyvsp[0].blk), '%');
}
#line 2629 "parser.c" /* yacc.c:1646  */
    break;

  case 41:
#line 445 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_binop((yyvsp[-2].blk), (yyvsp[0].blk), EQ);
}
#line 2637 "parser.c" /* yacc.c:1646  */
    break;

  case 42:
#line 449 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_binop((yyvsp[-2].blk), (yyvsp[0].blk), NEQ);
}
#line 2645 "parser.c" /* yacc.c:1646  */
    break;

  case 43:
#line 453 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_binop((yyvsp[-2].blk), (yyvsp[0].blk), '<');
}
#line 2653 "parser.c" /* yacc.c:1646  */
    break;

  case 44:
#line 457 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_binop((yyvsp[-2].blk), (yyvsp[0].blk), '>');
}
#line 2661 "parser.c" /* yacc.c:1646  */
    break;

  case 45:
#line 461 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_binop((yyvsp[-2].blk), (yyvsp[0].blk), LESSEQ);
}
#line 2669 "parser.c" /* yacc.c:1646  */
    break;

  case 46:
#line 465 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_binop((yyvsp[-2].blk), (yyvsp[0].blk), GREATEREQ);
}
#line 2677 "parser.c" /* yacc.c:1646  */
    break;

  case 47:
#line 469 "parser.y" /* yacc.c:1646  */
    { 
  (yyval.blk) = (yyvsp[0].blk); 
}
#line 2685 "parser.c" /* yacc.c:1646  */
    break;

  case 48:
#line 474 "parser.y" /* yacc.c:1646  */
    {
  jv v = block_const((yyvsp[-4].blk));
  // XXX Make gen_import take only blocks and the int is_data so we
  // don't have to free so much stuff here
  (yyval.blk) = gen_import(jv_string_value(v), gen_noop(), jv_string_value((yyvsp[-1].literal)), 1);
  block_free((yyvsp[-4].blk));
  jv_free((yyvsp[-1].literal));
  jv_free(v);
}
#line 2699 "parser.c" /* yacc.c:1646  */
    break;

  case 49:
#line 483 "parser.y" /* yacc.c:1646  */
    {
  jv v = block_const((yyvsp[-3].blk));
  (yyval.blk) = gen_import(jv_string_value(v), gen_noop(), jv_string_value((yyvsp[-1].literal)), 0);
  block_free((yyvsp[-3].blk));
  jv_free((yyvsp[-1].literal));
  jv_free(v);
}
#line 2711 "parser.c" /* yacc.c:1646  */
    break;

  case 50:
#line 490 "parser.y" /* yacc.c:1646  */
    {
  if (!block_is_const((yyvsp[-1].blk))) {
    FAIL((yyloc), "Module metadata must be constant.");
    (yyval.blk) = gen_noop();
  } else {
    jv v = block_const((yyvsp[-4].blk));
    (yyval.blk) = gen_import(jv_string_value(v), (yyvsp[-1].blk), jv_string_value((yyvsp[-2].literal)), 0);
    jv_free(v);
  }
  block_free((yyvsp[-4].blk));
  jv_free((yyvsp[-2].literal));
}
#line 2728 "parser.c" /* yacc.c:1646  */
    break;

  case 51:
#line 502 "parser.y" /* yacc.c:1646  */
    {
  if (!block_is_const((yyvsp[-1].blk))) {
    FAIL((yyloc), "Module metadata must be constant.");
    (yyval.blk) = gen_noop();
  } else {
    jv v = block_const((yyvsp[-5].blk));
    (yyval.blk) = gen_import(jv_string_value(v), (yyvsp[-1].blk), jv_string_value((yyvsp[-2].literal)), 1);
    jv_free(v);
  }
  block_free((yyvsp[-5].blk));
  jv_free((yyvsp[-2].literal));
}
#line 2745 "parser.c" /* yacc.c:1646  */
    break;

  case 52:
#line 516 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_function(jv_string_value((yyvsp[-3].literal)), gen_noop(), (yyvsp[-1].blk));
  jv_free((yyvsp[-3].literal));
}
#line 2754 "parser.c" /* yacc.c:1646  */
    break;

  case 53:
#line 521 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_function(jv_string_value((yyvsp[-6].literal)), (yyvsp[-4].blk), (yyvsp[-1].blk));
  jv_free((yyvsp[-6].literal));
}
#line 2763 "parser.c" /* yacc.c:1646  */
    break;

  case 54:
#line 527 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = (yyvsp[0].blk);
}
#line 2771 "parser.c" /* yacc.c:1646  */
    break;

  case 55:
#line 530 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = BLOCK((yyvsp[-2].blk), (yyvsp[0].blk));
}
#line 2779 "parser.c" /* yacc.c:1646  */
    break;

  case 56:
#line 535 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_param_regular(jv_string_value((yyvsp[0].literal)));
  jv_free((yyvsp[0].literal));
}
#line 2788 "parser.c" /* yacc.c:1646  */
    break;

  case 57:
#line 540 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_param(jv_string_value((yyvsp[0].literal)));
  jv_free((yyvsp[0].literal));
}
#line 2797 "parser.c" /* yacc.c:1646  */
    break;

  case 58:
#line 547 "parser.y" /* yacc.c:1646  */
    { (yyval.literal) = jv_string("text"); }
#line 2803 "parser.c" /* yacc.c:1646  */
    break;

  case 59:
#line 547 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = (yyvsp[-1].blk);
  jv_free((yyvsp[-2].literal));
}
#line 2812 "parser.c" /* yacc.c:1646  */
    break;

  case 60:
#line 551 "parser.y" /* yacc.c:1646  */
    { (yyval.literal) = (yyvsp[-1].literal); }
#line 2818 "parser.c" /* yacc.c:1646  */
    break;

  case 61:
#line 551 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = (yyvsp[-1].blk);
  jv_free((yyvsp[-2].literal));
}
#line 2827 "parser.c" /* yacc.c:1646  */
    break;

  case 62:
#line 558 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_const(jv_string(""));
}
#line 2835 "parser.c" /* yacc.c:1646  */
    break;

  case 63:
#line 561 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_binop((yyvsp[-1].blk), gen_const((yyvsp[0].literal)), '+');
}
#line 2843 "parser.c" /* yacc.c:1646  */
    break;

  case 64:
#line 564 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_binop((yyvsp[-3].blk), gen_format((yyvsp[-1].blk), jv_copy((yyvsp[-4].literal))), '+');
}
#line 2851 "parser.c" /* yacc.c:1646  */
    break;

  case 65:
#line 570 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_cond((yyvsp[-3].blk), (yyvsp[-1].blk), (yyvsp[0].blk));
}
#line 2859 "parser.c" /* yacc.c:1646  */
    break;

  case 66:
#line 573 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = (yyvsp[-1].blk);
}
#line 2867 "parser.c" /* yacc.c:1646  */
    break;

  case 67:
#line 578 "parser.y" /* yacc.c:1646  */
    { 
  (yyval.blk) = block_join((yyvsp[-2].blk), (yyvsp[0].blk));
}
#line 2875 "parser.c" /* yacc.c:1646  */
    break;

  case 68:
#line 581 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = BLOCK((yyvsp[0].blk), gen_call("_negate", gen_noop()));
}
#line 2883 "parser.c" /* yacc.c:1646  */
    break;

  case 69:
#line 584 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = (yyvsp[0].blk);
}
#line 2891 "parser.c" /* yacc.c:1646  */
    break;

  case 70:
#line 590 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_noop(); 
}
#line 2899 "parser.c" /* yacc.c:1646  */
    break;

  case 71:
#line 593 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_call("recurse", gen_noop());
}
#line 2907 "parser.c" /* yacc.c:1646  */
    break;

  case 72:
#line 596 "parser.y" /* yacc.c:1646  */
    {
  jv v = jv_string_fmt("*label-%s", jv_string_value((yyvsp[0].literal)));     // impossible symbol
  (yyval.blk) = gen_location((yyloc), locations,
                    BLOCK(gen_op_unbound(LOADV, jv_string_value(v)),
                    gen_call("error", gen_noop())));
  jv_free(v);
  jv_free((yyvsp[0].literal));
}
#line 2920 "parser.c" /* yacc.c:1646  */
    break;

  case 73:
#line 604 "parser.y" /* yacc.c:1646  */
    {
  FAIL((yyloc), "break requires a label to break to");
  (yyval.blk) = gen_noop();
}
#line 2929 "parser.c" /* yacc.c:1646  */
    break;

  case 74:
#line 608 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_index_opt((yyvsp[-2].blk), gen_const((yyvsp[-1].literal)));
}
#line 2937 "parser.c" /* yacc.c:1646  */
    break;

  case 75:
#line 611 "parser.y" /* yacc.c:1646  */
    { 
  (yyval.blk) = gen_index_opt(gen_noop(), gen_const((yyvsp[-1].literal))); 
}
#line 2945 "parser.c" /* yacc.c:1646  */
    break;

  case 76:
#line 614 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_index_opt((yyvsp[-3].blk), (yyvsp[-1].blk));
}
#line 2953 "parser.c" /* yacc.c:1646  */
    break;

  case 77:
#line 617 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_index_opt(gen_noop(), (yyvsp[-1].blk));
}
#line 2961 "parser.c" /* yacc.c:1646  */
    break;

  case 78:
#line 620 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_index((yyvsp[-1].blk), gen_const((yyvsp[0].literal)));
}
#line 2969 "parser.c" /* yacc.c:1646  */
    break;

  case 79:
#line 623 "parser.y" /* yacc.c:1646  */
    { 
  (yyval.blk) = gen_index(gen_noop(), gen_const((yyvsp[0].literal))); 
}
#line 2977 "parser.c" /* yacc.c:1646  */
    break;

  case 80:
#line 626 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_index((yyvsp[-2].blk), (yyvsp[0].blk));
}
#line 2985 "parser.c" /* yacc.c:1646  */
    break;

  case 81:
#line 629 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_index(gen_noop(), (yyvsp[0].blk));
}
#line 2993 "parser.c" /* yacc.c:1646  */
    break;

  case 82:
#line 632 "parser.y" /* yacc.c:1646  */
    {
  FAIL((yyloc), "try .[\"field\"] instead of .field for unusually named fields");
  (yyval.blk) = gen_noop();
}
#line 3002 "parser.c" /* yacc.c:1646  */
    break;

  case 83:
#line 636 "parser.y" /* yacc.c:1646  */
    {
  jv_free((yyvsp[-1].literal));
  FAIL((yyloc), "try .[\"field\"] instead of .field for unusually named fields");
  (yyval.blk) = gen_noop();
}
#line 3012 "parser.c" /* yacc.c:1646  */
    break;

  case 84:
#line 642 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_index_opt((yyvsp[-4].blk), (yyvsp[-2].blk)); 
}
#line 3020 "parser.c" /* yacc.c:1646  */
    break;

  case 85:
#line 645 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_index((yyvsp[-3].blk), (yyvsp[-1].blk)); 
}
#line 3028 "parser.c" /* yacc.c:1646  */
    break;

  case 86:
#line 648 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = block_join((yyvsp[-3].blk), gen_op_simple(EACH_OPT)); 
}
#line 3036 "parser.c" /* yacc.c:1646  */
    break;

  case 87:
#line 651 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = block_join((yyvsp[-2].blk), gen_op_simple(EACH)); 
}
#line 3044 "parser.c" /* yacc.c:1646  */
    break;

  case 88:
#line 654 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_slice_index((yyvsp[-6].blk), (yyvsp[-4].blk), (yyvsp[-2].blk), INDEX_OPT);
}
#line 3052 "parser.c" /* yacc.c:1646  */
    break;

  case 89:
#line 657 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_slice_index((yyvsp[-5].blk), (yyvsp[-3].blk), gen_const(jv_null()), INDEX_OPT);
}
#line 3060 "parser.c" /* yacc.c:1646  */
    break;

  case 90:
#line 660 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_slice_index((yyvsp[-5].blk), gen_const(jv_null()), (yyvsp[-2].blk), INDEX_OPT);
}
#line 3068 "parser.c" /* yacc.c:1646  */
    break;

  case 91:
#line 663 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_slice_index((yyvsp[-5].blk), (yyvsp[-3].blk), (yyvsp[-1].blk), INDEX);
}
#line 3076 "parser.c" /* yacc.c:1646  */
    break;

  case 92:
#line 666 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_slice_index((yyvsp[-4].blk), (yyvsp[-2].blk), gen_const(jv_null()), INDEX);
}
#line 3084 "parser.c" /* yacc.c:1646  */
    break;

  case 93:
#line 669 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_slice_index((yyvsp[-4].blk), gen_const(jv_null()), (yyvsp[-1].blk), INDEX);
}
#line 3092 "parser.c" /* yacc.c:1646  */
    break;

  case 94:
#line 672 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_const((yyvsp[0].literal)); 
}
#line 3100 "parser.c" /* yacc.c:1646  */
    break;

  case 95:
#line 675 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = (yyvsp[0].blk);
}
#line 3108 "parser.c" /* yacc.c:1646  */
    break;

  case 96:
#line 678 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_format(gen_noop(), (yyvsp[0].literal));
}
#line 3116 "parser.c" /* yacc.c:1646  */
    break;

  case 97:
#line 681 "parser.y" /* yacc.c:1646  */
    { 
  (yyval.blk) = (yyvsp[-1].blk); 
}
#line 3124 "parser.c" /* yacc.c:1646  */
    break;

  case 98:
#line 684 "parser.y" /* yacc.c:1646  */
    { 
  (yyval.blk) = gen_collect((yyvsp[-1].blk)); 
}
#line 3132 "parser.c" /* yacc.c:1646  */
    break;

  case 99:
#line 687 "parser.y" /* yacc.c:1646  */
    { 
  (yyval.blk) = gen_const(jv_array()); 
}
#line 3140 "parser.c" /* yacc.c:1646  */
    break;

  case 100:
#line 690 "parser.y" /* yacc.c:1646  */
    { 
  block o = gen_const_object((yyvsp[-1].blk));
  if (o.first != NULL)
    (yyval.blk) = o;
  else
    (yyval.blk) = BLOCK(gen_subexp(gen_const(jv_object())), (yyvsp[-1].blk), gen_op_simple(POP));
}
#line 3152 "parser.c" /* yacc.c:1646  */
    break;

  case 101:
#line 697 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_const(JV_OBJECT(jv_string("file"), jv_copy(locations->fname),
                           jv_string("line"), jv_number(locfile_get_line(locations, (yyloc).start) + 1)));
}
#line 3161 "parser.c" /* yacc.c:1646  */
    break;

  case 102:
#line 701 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_location((yyloc), locations, gen_op_unbound(LOADV, jv_string_value((yyvsp[0].literal))));
  jv_free((yyvsp[0].literal));
}
#line 3170 "parser.c" /* yacc.c:1646  */
    break;

  case 103:
#line 705 "parser.y" /* yacc.c:1646  */
    {
  const char *s = jv_string_value((yyvsp[0].literal));
  if (strcmp(s, "false") == 0)
    (yyval.blk) = gen_const(jv_false());
  else if (strcmp(s, "true") == 0)
    (yyval.blk) = gen_const(jv_true());
  else if (strcmp(s, "null") == 0)
    (yyval.blk) = gen_const(jv_null());
  else
    (yyval.blk) = gen_location((yyloc), locations, gen_call(s, gen_noop()));
  jv_free((yyvsp[0].literal));
}
#line 3187 "parser.c" /* yacc.c:1646  */
    break;

  case 104:
#line 717 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_call(jv_string_value((yyvsp[-3].literal)), (yyvsp[-1].blk));
  (yyval.blk) = gen_location((yylsp[-3]), locations, (yyval.blk));
  jv_free((yyvsp[-3].literal));
}
#line 3197 "parser.c" /* yacc.c:1646  */
    break;

  case 105:
#line 722 "parser.y" /* yacc.c:1646  */
    { (yyval.blk) = gen_noop(); }
#line 3203 "parser.c" /* yacc.c:1646  */
    break;

  case 106:
#line 723 "parser.y" /* yacc.c:1646  */
    { (yyval.blk) = gen_noop(); }
#line 3209 "parser.c" /* yacc.c:1646  */
    break;

  case 107:
#line 724 "parser.y" /* yacc.c:1646  */
    { (yyval.blk) = (yyvsp[-3].blk); }
#line 3215 "parser.c" /* yacc.c:1646  */
    break;

  case 108:
#line 725 "parser.y" /* yacc.c:1646  */
    { (yyval.blk) = gen_noop(); }
#line 3221 "parser.c" /* yacc.c:1646  */
    break;

  case 109:
#line 728 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = (yyvsp[0].blk);
}
#line 3229 "parser.c" /* yacc.c:1646  */
    break;

  case 110:
#line 731 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = BLOCK((yyvsp[-2].blk), (yyvsp[0].blk));
}
#line 3237 "parser.c" /* yacc.c:1646  */
    break;

  case 111:
#line 736 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_lambda((yyvsp[0].blk));
}
#line 3245 "parser.c" /* yacc.c:1646  */
    break;

  case 112:
#line 741 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_op_unbound(STOREV, jv_string_value((yyvsp[0].literal)));
  jv_free((yyvsp[0].literal));
}
#line 3254 "parser.c" /* yacc.c:1646  */
    break;

  case 113:
#line 745 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = BLOCK((yyvsp[-1].blk), gen_op_simple(POP));
}
#line 3262 "parser.c" /* yacc.c:1646  */
    break;

  case 114:
#line 748 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = BLOCK((yyvsp[-1].blk), gen_op_simple(POP));
}
#line 3270 "parser.c" /* yacc.c:1646  */
    break;

  case 115:
#line 753 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_array_matcher(gen_noop(), (yyvsp[0].blk));
}
#line 3278 "parser.c" /* yacc.c:1646  */
    break;

  case 116:
#line 756 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_array_matcher((yyvsp[-2].blk), (yyvsp[0].blk));
}
#line 3286 "parser.c" /* yacc.c:1646  */
    break;

  case 117:
#line 761 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = (yyvsp[0].blk);
}
#line 3294 "parser.c" /* yacc.c:1646  */
    break;

  case 118:
#line 764 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = BLOCK((yyvsp[-2].blk), (yyvsp[0].blk));
}
#line 3302 "parser.c" /* yacc.c:1646  */
    break;

  case 119:
#line 769 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_object_matcher(gen_const((yyvsp[0].literal)), gen_op_unbound(STOREV, jv_string_value((yyvsp[0].literal))));
}
#line 3310 "parser.c" /* yacc.c:1646  */
    break;

  case 120:
#line 772 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_object_matcher(gen_const((yyvsp[-2].literal)), (yyvsp[0].blk));
}
#line 3318 "parser.c" /* yacc.c:1646  */
    break;

  case 121:
#line 775 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_object_matcher(gen_const((yyvsp[-2].literal)), (yyvsp[0].blk));
}
#line 3326 "parser.c" /* yacc.c:1646  */
    break;

  case 122:
#line 778 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_object_matcher((yyvsp[-2].blk), (yyvsp[0].blk));
}
#line 3334 "parser.c" /* yacc.c:1646  */
    break;

  case 123:
#line 781 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_object_matcher((yyvsp[-3].blk), (yyvsp[0].blk));
}
#line 3342 "parser.c" /* yacc.c:1646  */
    break;

  case 124:
#line 786 "parser.y" /* yacc.c:1646  */
    {
  (yyval.literal) = jv_string("as");
}
#line 3350 "parser.c" /* yacc.c:1646  */
    break;

  case 125:
#line 789 "parser.y" /* yacc.c:1646  */
    {
  (yyval.literal) = jv_string("def");
}
#line 3358 "parser.c" /* yacc.c:1646  */
    break;

  case 126:
#line 792 "parser.y" /* yacc.c:1646  */
    {
  (yyval.literal) = jv_string("module");
}
#line 3366 "parser.c" /* yacc.c:1646  */
    break;

  case 127:
#line 795 "parser.y" /* yacc.c:1646  */
    {
  (yyval.literal) = jv_string("import");
}
#line 3374 "parser.c" /* yacc.c:1646  */
    break;

  case 128:
#line 798 "parser.y" /* yacc.c:1646  */
    {
  (yyval.literal) = jv_string("if");
}
#line 3382 "parser.c" /* yacc.c:1646  */
    break;

  case 129:
#line 801 "parser.y" /* yacc.c:1646  */
    {
  (yyval.literal) = jv_string("then");
}
#line 3390 "parser.c" /* yacc.c:1646  */
    break;

  case 130:
#line 804 "parser.y" /* yacc.c:1646  */
    {
  (yyval.literal) = jv_string("else");
}
#line 3398 "parser.c" /* yacc.c:1646  */
    break;

  case 131:
#line 807 "parser.y" /* yacc.c:1646  */
    {
  (yyval.literal) = jv_string("elif");
}
#line 3406 "parser.c" /* yacc.c:1646  */
    break;

  case 132:
#line 810 "parser.y" /* yacc.c:1646  */
    {
  (yyval.literal) = jv_string("reduce");
}
#line 3414 "parser.c" /* yacc.c:1646  */
    break;

  case 133:
#line 813 "parser.y" /* yacc.c:1646  */
    {
  (yyval.literal) = jv_string("foreach");
}
#line 3422 "parser.c" /* yacc.c:1646  */
    break;

  case 134:
#line 816 "parser.y" /* yacc.c:1646  */
    {
  (yyval.literal) = jv_string("end");
}
#line 3430 "parser.c" /* yacc.c:1646  */
    break;

  case 135:
#line 819 "parser.y" /* yacc.c:1646  */
    {
  (yyval.literal) = jv_string("and");
}
#line 3438 "parser.c" /* yacc.c:1646  */
    break;

  case 136:
#line 822 "parser.y" /* yacc.c:1646  */
    {
  (yyval.literal) = jv_string("or");
}
#line 3446 "parser.c" /* yacc.c:1646  */
    break;

  case 137:
#line 825 "parser.y" /* yacc.c:1646  */
    {
  (yyval.literal) = jv_string("try");
}
#line 3454 "parser.c" /* yacc.c:1646  */
    break;

  case 138:
#line 828 "parser.y" /* yacc.c:1646  */
    {
  (yyval.literal) = jv_string("catch");
}
#line 3462 "parser.c" /* yacc.c:1646  */
    break;

  case 139:
#line 831 "parser.y" /* yacc.c:1646  */
    {
  (yyval.literal) = jv_string("label");
}
#line 3470 "parser.c" /* yacc.c:1646  */
    break;

  case 140:
#line 834 "parser.y" /* yacc.c:1646  */
    {
  (yyval.literal) = jv_string("break");
}
#line 3478 "parser.c" /* yacc.c:1646  */
    break;

  case 141:
#line 837 "parser.y" /* yacc.c:1646  */
    {
  (yyval.literal) = jv_string("__loc__");
}
#line 3486 "parser.c" /* yacc.c:1646  */
    break;

  case 142:
#line 842 "parser.y" /* yacc.c:1646  */
    { 
  (yyval.blk)=gen_noop(); 
}
#line 3494 "parser.c" /* yacc.c:1646  */
    break;

  case 143:
#line 845 "parser.y" /* yacc.c:1646  */
    { (yyval.blk) = (yyvsp[0].blk); }
#line 3500 "parser.c" /* yacc.c:1646  */
    break;

  case 144:
#line 846 "parser.y" /* yacc.c:1646  */
    { (yyval.blk)=block_join((yyvsp[-2].blk), (yyvsp[0].blk)); }
#line 3506 "parser.c" /* yacc.c:1646  */
    break;

  case 145:
#line 847 "parser.y" /* yacc.c:1646  */
    { (yyval.blk) = (yyvsp[0].blk); }
#line 3512 "parser.c" /* yacc.c:1646  */
    break;

  case 146:
#line 850 "parser.y" /* yacc.c:1646  */
    { 
  (yyval.blk) = gen_dictpair(gen_const((yyvsp[-2].literal)), (yyvsp[0].blk));
 }
#line 3520 "parser.c" /* yacc.c:1646  */
    break;

  case 147:
#line 853 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_dictpair(gen_const((yyvsp[-2].literal)), (yyvsp[0].blk));
  }
#line 3528 "parser.c" /* yacc.c:1646  */
    break;

  case 148:
#line 856 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_dictpair((yyvsp[-2].blk), (yyvsp[0].blk));
  }
#line 3536 "parser.c" /* yacc.c:1646  */
    break;

  case 149:
#line 859 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_dictpair((yyvsp[0].blk), BLOCK(gen_op_simple(POP), gen_op_simple(DUP2),
                              gen_op_simple(DUP2), gen_op_simple(INDEX)));
  }
#line 3545 "parser.c" /* yacc.c:1646  */
    break;

  case 150:
#line 863 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_dictpair(gen_const((yyvsp[0].literal)),
                    gen_location((yyloc), locations, gen_op_unbound(LOADV, jv_string_value((yyvsp[0].literal)))));
  }
#line 3554 "parser.c" /* yacc.c:1646  */
    break;

  case 151:
#line 867 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_dictpair(gen_const(jv_copy((yyvsp[0].literal))),
                    gen_index(gen_noop(), gen_const((yyvsp[0].literal))));
  }
#line 3563 "parser.c" /* yacc.c:1646  */
    break;

  case 152:
#line 871 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_dictpair((yyvsp[-3].blk), (yyvsp[0].blk));
  }
#line 3571 "parser.c" /* yacc.c:1646  */
    break;

  case 153:
#line 874 "parser.y" /* yacc.c:1646  */
    { (yyval.blk) = (yyvsp[0].blk); }
#line 3577 "parser.c" /* yacc.c:1646  */
    break;


#line 3581 "parser.c" /* yacc.c:1646  */
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
  *++yylsp = yyloc;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

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
      yyerror (&yylloc, answer, errors, locations, lexer_param_ptr, YY_("syntax error"));
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
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
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
        yyerror (&yylloc, answer, errors, locations, lexer_param_ptr, yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }

  yyerror_range[1] = yylloc;

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
                      yytoken, &yylval, &yylloc, answer, errors, locations, lexer_param_ptr);
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

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  yyerror_range[1] = yylsp[1-yylen];
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

      yyerror_range[1] = *yylsp;
      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp, yylsp, answer, errors, locations, lexer_param_ptr);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  yyerror_range[2] = yylloc;
  /* Using YYLLOC is tempting, but would change the location of
     the lookahead.  YYLOC is available though.  */
  YYLLOC_DEFAULT (yyloc, yyerror_range, 2);
  *++yylsp = yyloc;

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
  yyerror (&yylloc, answer, errors, locations, lexer_param_ptr, YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, &yylloc, answer, errors, locations, lexer_param_ptr);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp, yylsp, answer, errors, locations, lexer_param_ptr);
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
#line 875 "parser.y" /* yacc.c:1906  */


int jq_parse(struct locfile* locations, block* answer) {
  struct lexer_param scanner;
  YY_BUFFER_STATE buf;
  jq_yylex_init_extra(0, &scanner.lexer);
  buf = jq_yy_scan_bytes(locations->data, locations->length, scanner.lexer);
  int errors = 0;
  *answer = gen_noop();
  yyparse(answer, &errors, locations, &scanner);
  jq_yy_delete_buffer(buf, scanner.lexer);
  jq_yylex_destroy(scanner.lexer);
  if (errors > 0) {
    block_free(*answer);
    *answer = gen_noop();
  }
  return errors;
}

int jq_parse_library(struct locfile* locations, block* answer) {
  int errs = jq_parse(locations, answer);
  if (errs) return errs;
  if (block_has_main(*answer)) {
    locfile_locate(locations, UNKNOWN_LOCATION, "jq: error: library should only have function definitions, not a main expression");
    return 1;
  }
  assert(block_has_only_binders_and_imports(*answer, OP_IS_CALL_PSEUDO));
  return 0;
}

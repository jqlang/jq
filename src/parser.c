/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

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
#define YYBISON_VERSION "3.0.4"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* Copy the first part of user declarations.  */
#line 1 "src/parser.y" /* yacc.c:339  */

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "compile.h"
#include "jv_alloc.h"
#define YYMALLOC jv_mem_alloc
#define YYFREE jv_mem_free

#line 77 "src/parser.c" /* yacc.c:339  */

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
#ifndef YY_YY_SRC_PARSER_H_INCLUDED
# define YY_YY_SRC_PARSER_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif
/* "%code requires" blocks.  */
#line 11 "src/parser.y" /* yacc.c:355  */

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

#line 124 "src/parser.c" /* yacc.c:355  */

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
    CODEF = 270,
    MODULE = 271,
    IMPORT = 272,
    INCLUDE = 273,
    IF = 274,
    THEN = 275,
    ELSE = 276,
    ELSE_IF = 277,
    REDUCE = 278,
    FOREACH = 279,
    END = 280,
    AND = 281,
    OR = 282,
    TRY = 283,
    CATCH = 284,
    LABEL = 285,
    BREAK = 286,
    LOC = 287,
    SETPIPE = 288,
    SETPLUS = 289,
    SETMINUS = 290,
    SETMULT = 291,
    SETDIV = 292,
    SETDEFINEDOR = 293,
    LESSEQ = 294,
    GREATEREQ = 295,
    ALTERNATION = 296,
    COEXPR = 297,
    QQSTRING_START = 298,
    QQSTRING_TEXT = 299,
    QQSTRING_INTERP_START = 300,
    QQSTRING_INTERP_END = 301,
    QQSTRING_END = 302,
    FUNCDEF = 303,
    NONOPT = 304
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
#define CODEF 270
#define MODULE 271
#define IMPORT 272
#define INCLUDE 273
#define IF 274
#define THEN 275
#define ELSE 276
#define ELSE_IF 277
#define REDUCE 278
#define FOREACH 279
#define END 280
#define AND 281
#define OR 282
#define TRY 283
#define CATCH 284
#define LABEL 285
#define BREAK 286
#define LOC 287
#define SETPIPE 288
#define SETPLUS 289
#define SETMINUS 290
#define SETMULT 291
#define SETDIV 292
#define SETDEFINEDOR 293
#define LESSEQ 294
#define GREATEREQ 295
#define ALTERNATION 296
#define COEXPR 297
#define QQSTRING_START 298
#define QQSTRING_TEXT 299
#define QQSTRING_INTERP_START 300
#define QQSTRING_INTERP_END 301
#define QQSTRING_END 302
#define FUNCDEF 303
#define NONOPT 304

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 31 "src/parser.y" /* yacc.c:355  */

  jv literal;
  block blk;

#line 239 "src/parser.c" /* yacc.c:355  */
};

typedef union YYSTYPE YYSTYPE;
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

#endif /* !YY_YY_SRC_PARSER_H_INCLUDED  */

/* Copy the second part of user declarations.  */
#line 126 "src/parser.y" /* yacc.c:358  */

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

/* Returns string message if the block is a constant that is not valid as an
 * object key. */
static jv check_object_key(block k) {
  if (block_is_const(k) && block_const_kind(k) != JV_KIND_STRING) {
    char errbuf[15];
    return jv_string_fmt("Cannot use %s (%s) as object key",
        jv_kind_name(block_const_kind(k)),
        jv_dump_string_trunc(block_const(k), errbuf, sizeof(errbuf)));
  }
  return jv_invalid();
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
  if (op == '+') {
    if (block_const_kind(a) == JV_KIND_NULL) {
      block_free(a);
      return b;
    }
    if (block_const_kind(b) == JV_KIND_NULL) {
      block_free(b);
      return a;
    }
  }
  if (block_const_kind(a) != block_const_kind(b))
    return gen_noop();

  jv res = jv_invalid();

  if (block_const_kind(a) == JV_KIND_NUMBER) {
    jv jv_a = block_const(a);
    jv jv_b = block_const(b);

    double na = jv_number_value(jv_a);
    double nb = jv_number_value(jv_b);

    int cmp = jv_cmp(jv_a, jv_b);

    switch (op) {
    case '+': res = jv_number(na + nb); break;
    case '-': res = jv_number(na - nb); break;
    case '*': res = jv_number(na * nb); break;
    case '/': res = jv_number(na / nb); break;
    case EQ:  res = (cmp == 0 ? jv_true() : jv_false()); break;
    case NEQ: res = (cmp != 0 ? jv_true() : jv_false()); break;
    case '<': res = (cmp < 0 ? jv_true() : jv_false()); break;
    case '>': res = (cmp > 0 ? jv_true() : jv_false()); break;
    case LESSEQ: res = (cmp <= 0 ? jv_true() : jv_false()); break;
    case GREATEREQ: res = (cmp >= 0 ? jv_true() : jv_false()); break;
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
  return BLOCK(a, gen_call("format", gen_lambda(gen_const(fmt))));
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


#line 448 "src/parser.c" /* yacc.c:358  */

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
#define YYFINAL  28
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   2145

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  71
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  30
/* YYNRULES -- Number of rules.  */
#define YYNRULES  172
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  334

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   304

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,    63,    58,     2,     2,
      62,    64,    56,    54,    50,    55,    66,    57,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    65,    61,
      52,    51,    53,    60,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    67,     2,    68,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    69,    49,    70,     2,     2,     2,     2,
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
      45,    46,    47,    48,    59
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   308,   308,   311,   316,   319,   330,   333,   338,   341,
     346,   350,   370,   373,   377,   381,   385,   388,   391,   396,
     399,   402,   407,   414,   418,   422,   426,   430,   434,   438,
     442,   446,   450,   454,   458,   462,   466,   470,   474,   478,
     484,   490,   494,   498,   502,   506,   510,   514,   518,   522,
     527,   530,   547,   556,   563,   571,   582,   587,   593,   596,
     601,   605,   609,   613,   620,   620,   624,   624,   631,   634,
     637,   643,   646,   651,   654,   657,   663,   666,   669,   677,
     681,   684,   687,   690,   693,   696,   699,   702,   705,   709,
     715,   718,   721,   724,   727,   730,   733,   736,   739,   742,
     745,   748,   751,   754,   757,   760,   763,   766,   769,   776,
     780,   789,   801,   806,   807,   808,   809,   812,   815,   820,
     825,   828,   833,   836,   841,   845,   848,   853,   856,   861,
     864,   869,   872,   875,   878,   881,   884,   892,   898,   901,
     904,   907,   910,   913,   916,   919,   922,   925,   928,   931,
     934,   937,   940,   943,   946,   949,   952,   955,   960,   963,
     964,   965,   968,   971,   974,   977,   981,   985,   989,   993,
     997,  1001,  1009
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 1
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "INVALID_CHARACTER", "IDENT", "FIELD",
  "LITERAL", "FORMAT", "\"..\"", "\"%=\"", "\"==\"", "\"!=\"", "\"//\"",
  "\"as\"", "\"def\"", "\"codef\"", "\"module\"", "\"import\"",
  "\"include\"", "\"if\"", "\"then\"", "\"else\"", "\"elif\"",
  "\"reduce\"", "\"foreach\"", "\"end\"", "\"and\"", "\"or\"", "\"try\"",
  "\"catch\"", "\"label\"", "\"break\"", "\"__loc__\"", "\"|=\"", "\"+=\"",
  "\"-=\"", "\"*=\"", "\"/=\"", "\"//=\"", "\"<=\"", "\">=\"", "\"?//\"",
  "\"@@\"", "QQSTRING_START", "QQSTRING_TEXT", "QQSTRING_INTERP_START",
  "QQSTRING_INTERP_END", "QQSTRING_END", "FUNCDEF", "'|'", "','", "'='",
  "'<'", "'>'", "'+'", "'-'", "'*'", "'/'", "'%'", "NONOPT", "'?'", "';'",
  "'('", "'$'", "')'", "':'", "'.'", "'['", "']'", "'{'", "'}'", "$accept",
  "TopLevel", "Module", "Imports", "FuncDefs", "Exp", "Import",
  "ImportWhat", "ImportFrom", "FuncDef", "Params", "Param", "String", "@1",
  "@2", "QQString", "ElseBody", "ExpD", "Term", "Args", "Arg",
  "RepPatterns", "Patterns", "Pattern", "ArrayPats", "ObjPats", "ObjPat",
  "Keyword", "MkDict", "MkDictPair", YY_NULLPTR
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
     295,   296,   297,   298,   299,   300,   301,   302,   303,   124,
      44,    61,    60,    62,    43,    45,    42,    47,    37,   304,
      63,    59,    40,    36,    41,    58,    46,    91,    93,   123,
     125
};
# endif

#define YYPACT_NINF -162

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-162)))

#define YYTABLE_NINF -159

#define yytable_value_is_error(Yytable_value) \
  (!!((Yytable_value) == (-159)))

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
      46,   864,     3,    42,     4,    12,  -162,    31,  -162,    72,
      19,   864,   175,   175,   864,    22,     0,  -162,   864,   604,
    2055,   297,   535,   365,  1350,   864,  -162,     1,  -162,    -3,
      -3,   864,    42,   768,   864,  -162,  -162,   -13,    47,  1723,
      52,    56,    95,   121,  -162,   126,  -162,   -10,    68,  1175,
    -162,  -162,  -162,  -162,  -162,  -162,  -162,  -162,  -162,  -162,
    -162,  -162,  -162,  -162,  -162,  -162,  -162,  -162,  -162,  -162,
    -162,  -162,  -162,   133,    31,    75,    69,  -162,   995,   -34,
      73,   864,  2084,    74,    76,    70,    92,   864,   864,   864,
     864,   864,   864,   864,   864,   864,   864,   864,   864,   864,
     864,   864,   864,   864,   864,   864,   864,   864,   864,   864,
     864,  -162,  -162,  1897,    85,   -24,     8,   466,   134,  -162,
    -162,  -162,  1897,   864,  -162,  -162,  1386,  1897,   -23,  -162,
    -162,   111,   864,    84,   671,   -24,   -24,   738,   100,  -162,
       9,  -162,  -162,  -162,  -162,  -162,  -162,   425,   165,  -162,
     165,  1210,    89,  -162,   165,   165,  -162,   425,  1967,   215,
     215,  1932,   360,  2000,  1967,  1967,  1967,  1967,  1967,  1967,
     215,   215,  1897,  1932,  1967,   215,   215,   -10,   -10,    96,
      96,    96,  -162,   151,   -24,   921,   117,   110,   119,   864,
     101,    97,   864,   103,   959,    15,  -162,  -162,   864,  -162,
      26,  -162,   160,  2113,    -6,  -162,  1422,   162,  -162,  1619,
     106,   113,  -162,  -162,   864,  -162,   864,  -162,   -21,  -162,
     165,   127,    17,   127,   112,   165,   127,   127,  -162,  -162,
    -162,   -26,   122,   124,   864,   174,   128,   -45,  -162,   129,
     -24,   864,  1031,  -162,  -162,  1067,  -162,   834,   132,  -162,
     180,  -162,  -162,  -162,  -162,  -162,   111,   130,  -162,   135,
     864,   864,  -162,  -162,   864,   864,  1897,  1758,  -162,   165,
     165,   127,   -24,  -162,   -24,   -24,  1245,   136,   -24,   921,
    -162,   -24,   145,  1897,   138,   139,   142,  1103,  -162,  -162,
    -162,   864,   864,  1810,  1862,  1458,  1494,  -162,   127,   127,
    -162,  -162,  -162,   140,   -24,  -162,  -162,  -162,  -162,  -162,
    -162,   143,  1530,  1566,  -162,   864,   864,   864,   -24,  -162,
    -162,  -162,   864,  1671,  1280,  1139,  -162,  1897,  -162,  -162,
     864,  -162,  1315,  -162
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       4,     0,     0,     6,   111,    85,   102,   104,    77,     0,
       0,     0,     0,     0,     0,     0,     0,    64,     0,     0,
       0,     0,     0,     0,     0,     0,   103,    49,     1,     0,
       0,     8,     6,     0,     0,    81,    66,     0,     0,     0,
       0,     0,    20,     0,    79,     0,    68,    34,     0,     0,
     109,   138,   139,   140,   141,   142,   143,   144,   145,   146,
     147,   148,   149,   150,   151,   152,   153,   154,   155,   156,
     157,   110,    88,     0,     0,    87,     0,   107,     0,     0,
     169,     0,     0,   165,   170,     0,   159,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    23,     5,    10,    84,     0,     0,     0,     0,    55,
      54,     3,     2,     8,     7,    50,     0,   119,     0,   117,
      68,     0,     0,     0,     0,     0,     0,     0,     0,    78,
       0,   113,   105,    89,    83,   114,   106,     0,     0,   116,
       0,     0,   167,   168,     0,     0,   108,     0,    42,    43,
      44,    27,    26,    25,    29,    33,    36,    38,    41,    28,
      47,    48,    30,    31,    24,    45,    46,    32,    35,    37,
      39,    40,    80,     0,     0,     0,     0,     0,   123,     0,
      86,     0,     0,    95,     0,     0,     9,    51,     0,   112,
       0,    63,     0,     0,     0,    58,     0,     0,    18,     0,
       0,     0,    21,    19,     0,    69,     0,    65,     0,   161,
       0,   172,    75,   162,     0,     0,   164,   163,   160,   124,
     127,     0,     0,     0,     0,     0,     0,     0,   129,     0,
       0,     0,     0,    82,   115,     0,    94,     0,    91,    53,
       0,   118,    67,    62,    60,    61,     0,     0,    56,     0,
       0,     0,    17,    16,     0,     0,    22,     0,    74,     0,
       0,   166,     0,   125,     0,     0,     0,   131,     0,     0,
     126,     0,   122,    12,    93,   101,   100,     0,    90,    52,
      59,     0,     0,     0,     0,     0,     0,    70,    73,   171,
     128,   137,   133,     0,     0,   135,   130,   134,    92,    98,
      97,    99,     0,     0,    72,     0,     0,     0,     0,   132,
      96,    57,     0,     0,     0,     0,   136,    11,    71,    13,
       0,    15,     0,    14
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -162,  -162,  -162,   172,    86,    -1,  -162,  -162,   177,   -11,
    -162,   -46,     5,  -162,  -162,    81,  -111,  -143,    -4,  -162,
      16,  -162,    -7,  -161,  -162,  -162,   -62,   -18,  -120,  -162
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     2,     3,    31,   121,   113,    32,    33,   118,    25,
     204,   205,    26,    46,   130,   140,   263,   221,    27,   128,
     129,   186,   187,   188,   231,   237,   238,    84,    85,    86
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      24,    44,    71,    28,    74,   279,   114,   223,    40,    41,
      39,   226,   227,    42,   115,    74,   147,    47,    49,   249,
     123,    78,   114,   230,   272,   280,    75,   219,    83,   147,
     122,   148,   126,   127,   119,   119,   149,   228,   198,   183,
      17,   199,   273,   184,   148,   185,   108,   109,   110,   131,
     111,    17,   132,   215,   216,   256,   217,   114,   257,    29,
      30,   114,     1,    45,   153,   135,    34,   116,   117,   136,
     215,   216,    35,   252,    36,   189,    37,   268,   250,   282,
     151,    38,   271,   116,   117,    43,   158,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   169,   170,   171,
     172,   173,   174,   175,   176,   177,   178,   179,   180,   181,
     133,   300,   123,   301,   302,   201,   194,   305,   116,   117,
     307,   190,   116,   117,   137,   138,   298,   299,   210,   211,
     139,   206,   141,   209,   143,   144,   213,   145,   150,   154,
     156,   155,   157,   319,   222,   182,   222,   195,   207,   214,
     222,   222,    83,   202,   225,   229,   111,   326,   240,   241,
    -121,   243,    83,   246,   253,   244,   259,   239,   264,     4,
       5,     6,     7,     8,   203,   265,   269,   270,   277,     4,
       5,     6,     7,     8,   289,   255,  -120,   274,   242,   275,
     236,   245,   288,   278,   281,   291,    16,   127,   308,   309,
     292,   304,   310,   320,   124,   318,    16,   120,    17,   196,
     290,   200,   328,   266,   251,   267,   222,   306,    17,     0,
     220,   222,     0,     0,     0,  -159,  -159,    19,    20,     0,
       0,    21,    22,   276,    23,     0,     0,    19,    20,     0,
     283,    21,    22,     0,    23,     0,   287,     0,     0,     0,
       0,     0,     0,     0,  -159,  -159,     0,     0,     0,   293,
     294,   239,     0,   295,   296,   222,   222,  -159,  -159,   106,
     107,   108,   109,   110,     0,   111,     0,     0,     0,     0,
       0,     0,     0,     0,   236,     0,     0,     0,     0,     0,
     312,   313,     0,     0,     0,     0,     0,   -76,    72,     0,
       0,    73,   -76,     0,    74,     0,   -76,   -76,   -76,   -76,
     -76,     0,     0,     0,   323,   324,   325,   -76,   -76,   -76,
       0,   327,   -76,   -76,   -76,     0,   -76,     0,     0,   332,
     -76,   -76,   -76,   -76,   -76,   -76,   -76,   -76,     0,     0,
      17,     0,     0,   -76,     0,     0,   -76,   -76,   -76,   -76,
     -76,   -76,   -76,   -76,   -76,   -76,     0,   -76,   -76,     0,
       0,   -76,   -76,   -76,   -76,   -76,    79,   -76,     0,    80,
      88,    89,    74,     0,     0,     0,     0,     0,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,     0,    99,
     100,     0,     0,     0,     0,     0,     0,     0,    17,     0,
       0,     0,   104,   105,   106,   107,   108,   109,   110,     0,
     111,     0,     0,     0,     0,     0,   218,    81,    82,    80,
       0,     0,    74,     0,     0,  -158,     0,     0,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   191,    17,     0,
       4,     5,     6,     7,     8,     0,     0,     0,     0,     0,
       9,    10,     0,     0,     0,    11,     0,    81,    82,    12,
      13,     0,     0,     0,    14,  -158,    15,    16,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    17,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    18,     0,     0,     0,     0,     0,     0,    19,    20,
       0,   192,    21,    22,   193,    23,    76,     0,     0,     4,
       5,     6,     7,     8,     0,     0,     0,     0,     0,     9,
      10,     0,     0,     0,    11,     0,     0,     0,    12,    13,
       0,     0,     0,    14,     0,    15,    16,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    17,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      18,     0,     0,     0,     0,     0,     0,    19,    20,     0,
       0,    21,    22,    77,    23,    48,     0,     0,     4,     5,
       6,     7,     8,     0,     0,     0,     0,     0,     9,    10,
       0,     0,     0,    11,     0,     0,     0,    12,    13,     0,
       0,     0,    14,     0,    15,    16,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    17,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    18,
       0,     0,     0,     0,     0,     0,    19,    20,     0,     0,
      21,    22,   208,    23,     0,     4,     5,     6,     7,     8,
       0,     0,     0,     0,     0,     9,    10,     0,     0,     0,
      11,     0,     0,     0,    12,    13,     0,     0,     0,    14,
       0,    15,    16,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    17,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    18,     0,     0,     0,
       0,     0,     0,    19,    20,     0,     0,    21,    22,   212,
      23,     0,     4,     5,     6,     7,     8,     0,     0,     0,
       0,     0,     9,    10,     0,     0,     0,    11,     0,     0,
       0,    12,    13,     0,     0,     0,    14,     0,    15,    16,
       0,     0,     4,     5,     6,     7,     8,     0,     0,     0,
       0,    17,     9,    10,     0,     0,     0,    11,     0,     0,
       0,    12,    13,    18,     0,     0,    14,     0,    15,    16,
      19,    20,     0,     0,    21,    22,     0,    23,     0,     0,
       0,    17,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    18,     0,     0,     0,     0,     0,   125,
      19,    20,     0,     0,    21,    22,     0,    23,     4,     5,
       6,     7,     8,     0,     0,     0,     0,     0,     9,    10,
       0,     0,     0,    11,     0,     0,     0,    12,    13,     0,
       0,     0,    14,     0,    15,    16,     0,     0,     4,     5,
       6,     7,     8,     0,     0,     0,     0,    17,     9,    10,
       0,     0,     0,    11,     0,     0,     0,    12,    13,    18,
       0,     0,    14,     0,    15,    16,    19,    20,     0,     0,
      21,    22,   286,    23,     0,     0,     0,    17,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    18,
       0,     0,   232,     0,     0,   233,    19,    20,    74,     0,
      21,    22,     0,    23,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    17,     0,     0,     0,    87,    88,
      89,    90,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   234,   235,    91,    92,     0,     0,     0,
       0,     0,    93,    94,    95,    96,    97,    98,    99,   100,
       0,     0,     0,     0,    87,    88,    89,    90,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,     0,   111,
       0,    91,    92,     0,   247,     0,     0,   248,    93,    94,
      95,    96,    97,    98,    99,   100,     0,     0,     0,     0,
      87,    88,    89,    90,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,     0,   111,     0,    91,    92,     0,
       0,     0,     0,   146,    93,    94,    95,    96,    97,    98,
      99,   100,     0,     0,     0,     0,    87,    88,    89,    90,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
       0,   111,     0,    91,    92,     0,     0,     0,     0,   284,
      93,    94,    95,    96,    97,    98,    99,   100,     0,     0,
       0,     0,    87,    88,    89,    90,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,     0,   111,     0,    91,
      92,     0,     0,     0,     0,   285,    93,    94,    95,    96,
      97,    98,    99,   100,     0,     0,     0,     0,    87,    88,
      89,    90,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,     0,   111,     0,    91,    92,     0,     0,     0,
       0,   311,    93,    94,    95,    96,    97,    98,    99,   100,
       0,     0,     0,     0,    87,    88,    89,    90,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,     0,   111,
     330,    91,    92,   331,     0,     0,     0,     0,    93,    94,
      95,    96,    97,    98,    99,   100,     0,     0,     0,    87,
      88,    89,    90,     0,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,     0,   111,    91,    92,     0,   142,
       0,     0,     0,    93,    94,    95,    96,    97,    98,    99,
     100,     0,     0,     0,    87,    88,    89,    90,     0,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,     0,
     111,    91,    92,     0,   224,     0,     0,     0,    93,    94,
      95,    96,    97,    98,    99,   100,     0,     0,     0,    87,
      88,    89,    90,     0,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,     0,   111,    91,    92,     0,   303,
       0,     0,     0,    93,    94,    95,    96,    97,    98,    99,
     100,     0,     0,     0,    87,    88,    89,    90,     0,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,     0,
     111,    91,    92,     0,   329,     0,     0,     0,    93,    94,
      95,    96,    97,    98,    99,   100,     0,     0,     0,    87,
      88,    89,    90,     0,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,     0,   111,    91,    92,     0,   333,
       0,     0,     0,    93,    94,    95,    96,    97,    98,    99,
     100,     0,     0,     0,     0,    87,    88,    89,    90,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,     0,
     111,   112,    91,    92,     0,     0,     0,     0,     0,    93,
      94,    95,    96,    97,    98,    99,   100,     0,     0,     0,
       0,    87,    88,    89,    90,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,     0,   111,   197,    91,    92,
       0,     0,     0,     0,     0,    93,    94,    95,    96,    97,
      98,    99,   100,     0,     0,     0,     0,    87,    88,    89,
      90,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,     0,   111,   258,    91,    92,     0,     0,     0,     0,
       0,    93,    94,    95,    96,    97,    98,    99,   100,     0,
       0,     0,     0,    87,    88,    89,    90,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,     0,   111,   316,
      91,    92,     0,     0,     0,     0,     0,    93,    94,    95,
      96,    97,    98,    99,   100,     0,     0,     0,     0,    87,
      88,    89,    90,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,     0,   111,   317,    91,    92,     0,     0,
       0,     0,     0,    93,    94,    95,    96,    97,    98,    99,
     100,     0,     0,     0,     0,    87,    88,    89,    90,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,     0,
     111,   321,    91,    92,     0,     0,     0,     0,     0,    93,
      94,    95,    96,    97,    98,    99,   100,     0,     0,     0,
       0,     0,     0,     0,     0,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,     0,   111,   322,    87,    88,
      89,    90,     0,     0,     0,     0,     0,     0,     0,     0,
     260,   261,     0,     0,   262,    91,    92,     0,     0,     0,
       0,     0,    93,    94,    95,    96,    97,    98,    99,   100,
       0,     0,     0,     0,     0,     0,     0,     0,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,     0,   111,
      87,    88,    89,    90,     0,     0,     0,     0,     0,     0,
       0,     0,   260,   261,     0,     0,     0,    91,    92,     0,
       0,     0,     0,     0,    93,    94,    95,    96,    97,    98,
      99,   100,     0,     0,     0,     0,     0,     0,     0,     0,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
       0,   111,    87,    88,    89,    90,     0,     0,     0,     0,
       0,     0,     0,   134,     0,     0,     0,     0,     0,    91,
      92,     0,     0,     0,     0,     0,    93,    94,    95,    96,
      97,    98,    99,   100,     0,     0,     0,    87,    88,    89,
      90,     0,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,     0,   111,    91,    92,     0,     0,     0,     0,
       0,    93,    94,    95,    96,    97,    98,    99,   100,     0,
       0,     0,     0,     0,   297,     0,     0,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,     0,   111,    87,
      88,    89,    90,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   314,    91,    92,     0,     0,
       0,     0,     0,    93,    94,    95,    96,    97,    98,    99,
     100,     0,     0,     0,     0,     0,     0,     0,     0,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,     0,
     111,    87,    88,    89,    90,     0,     0,     0,     0,     0,
       0,     0,   315,     0,     0,     0,     0,     0,    91,    92,
       0,     0,     0,     0,     0,    93,    94,    95,    96,    97,
      98,    99,   100,     0,     0,     0,    87,    88,    89,    90,
       0,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,     0,   111,    91,    92,     0,     0,     0,     0,     0,
      93,    94,    95,    96,    97,    98,    99,   100,     0,     0,
       0,    87,    88,    89,    90,     0,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,     0,   111,    91,    92,
       0,     0,     0,     0,     0,    93,    94,    95,    96,    97,
      98,    99,   100,     0,     0,     0,  -159,    88,    89,     0,
       0,     0,     0,   103,   104,   105,   106,   107,   108,   109,
     110,     0,   111,    91,    92,     0,     0,     0,     0,     0,
    -159,  -159,  -159,  -159,  -159,  -159,    99,   100,     0,     0,
      88,    89,     0,     0,     0,     0,     0,     0,  -159,   104,
     105,   106,   107,   108,   109,   110,    91,   111,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    99,
     100,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   104,   105,   106,   107,   108,   109,   110,    50,
     111,     0,     0,     0,     0,     0,     0,     0,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,   152,     0,
       0,     0,     0,     0,     0,     0,     0,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,   254,     0,     0,
       0,     0,     0,     0,     0,     0,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70
};

static const yytype_int16 yycheck[] =
{
       1,     1,    20,     0,     7,    50,     5,   150,    12,    13,
      11,   154,   155,    14,    13,     7,    50,    18,    19,     4,
      31,    22,     5,   184,    50,    70,    21,   147,    23,    50,
      31,    65,    33,    34,    29,    30,    70,   157,    61,    63,
      43,    64,    68,    67,    65,    69,    56,    57,    58,    62,
      60,    43,    65,    44,    45,    61,    47,     5,    64,    17,
      18,     5,    16,    63,    82,    13,    62,    66,    67,    13,
      44,    45,    60,    47,    43,    67,     4,   220,    63,   240,
      81,    62,   225,    66,    67,    63,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
      63,   272,   123,   274,   275,     4,   117,   278,    66,    67,
     281,   116,    66,    67,    29,     4,   269,   270,   135,   136,
       4,   132,    64,   134,     1,    60,   137,    68,    65,    65,
      70,    65,    50,   304,   148,    60,   150,    13,    64,    49,
     154,   155,   147,    42,    65,     4,    60,   318,    41,    49,
      41,    60,   157,    60,     4,    68,     4,   185,    62,     4,
       5,     6,     7,     8,    63,    62,    49,    65,     4,     4,
       5,     6,     7,     8,     4,   203,    41,    65,   189,    65,
     185,   192,    60,    65,    65,    65,    31,   198,    60,    60,
      65,    65,    60,    60,    32,    65,    31,    30,    43,   123,
     256,   130,   323,   214,   198,   216,   220,   279,    43,    -1,
      55,   225,    -1,    -1,    -1,    10,    11,    62,    63,    -1,
      -1,    66,    67,   234,    69,    -1,    -1,    62,    63,    -1,
     241,    66,    67,    -1,    69,    -1,   247,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    39,    40,    -1,    -1,    -1,   260,
     261,   279,    -1,   264,   265,   269,   270,    52,    53,    54,
      55,    56,    57,    58,    -1,    60,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   279,    -1,    -1,    -1,    -1,    -1,
     291,   292,    -1,    -1,    -1,    -1,    -1,     0,     1,    -1,
      -1,     4,     5,    -1,     7,    -1,     9,    10,    11,    12,
      13,    -1,    -1,    -1,   315,   316,   317,    20,    21,    22,
      -1,   322,    25,    26,    27,    -1,    29,    -1,    -1,   330,
      33,    34,    35,    36,    37,    38,    39,    40,    -1,    -1,
      43,    -1,    -1,    46,    -1,    -1,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    -1,    60,    61,    -1,
      -1,    64,    65,    66,    67,    68,     1,    70,    -1,     4,
      10,    11,     7,    -1,    -1,    -1,    -1,    -1,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    -1,    39,
      40,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    43,    -1,
      -1,    -1,    52,    53,    54,    55,    56,    57,    58,    -1,
      60,    -1,    -1,    -1,    -1,    -1,     1,    62,    63,     4,
      -1,    -1,     7,    -1,    -1,    70,    -1,    -1,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,     1,    43,    -1,
       4,     5,     6,     7,     8,    -1,    -1,    -1,    -1,    -1,
      14,    15,    -1,    -1,    -1,    19,    -1,    62,    63,    23,
      24,    -1,    -1,    -1,    28,    70,    30,    31,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    43,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    55,    -1,    -1,    -1,    -1,    -1,    -1,    62,    63,
      -1,    65,    66,    67,    68,    69,     1,    -1,    -1,     4,
       5,     6,     7,     8,    -1,    -1,    -1,    -1,    -1,    14,
      15,    -1,    -1,    -1,    19,    -1,    -1,    -1,    23,    24,
      -1,    -1,    -1,    28,    -1,    30,    31,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    43,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      55,    -1,    -1,    -1,    -1,    -1,    -1,    62,    63,    -1,
      -1,    66,    67,    68,    69,     1,    -1,    -1,     4,     5,
       6,     7,     8,    -1,    -1,    -1,    -1,    -1,    14,    15,
      -1,    -1,    -1,    19,    -1,    -1,    -1,    23,    24,    -1,
      -1,    -1,    28,    -1,    30,    31,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    43,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    55,
      -1,    -1,    -1,    -1,    -1,    -1,    62,    63,    -1,    -1,
      66,    67,     1,    69,    -1,     4,     5,     6,     7,     8,
      -1,    -1,    -1,    -1,    -1,    14,    15,    -1,    -1,    -1,
      19,    -1,    -1,    -1,    23,    24,    -1,    -1,    -1,    28,
      -1,    30,    31,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    43,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    55,    -1,    -1,    -1,
      -1,    -1,    -1,    62,    63,    -1,    -1,    66,    67,     1,
      69,    -1,     4,     5,     6,     7,     8,    -1,    -1,    -1,
      -1,    -1,    14,    15,    -1,    -1,    -1,    19,    -1,    -1,
      -1,    23,    24,    -1,    -1,    -1,    28,    -1,    30,    31,
      -1,    -1,     4,     5,     6,     7,     8,    -1,    -1,    -1,
      -1,    43,    14,    15,    -1,    -1,    -1,    19,    -1,    -1,
      -1,    23,    24,    55,    -1,    -1,    28,    -1,    30,    31,
      62,    63,    -1,    -1,    66,    67,    -1,    69,    -1,    -1,
      -1,    43,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    55,    -1,    -1,    -1,    -1,    -1,    61,
      62,    63,    -1,    -1,    66,    67,    -1,    69,     4,     5,
       6,     7,     8,    -1,    -1,    -1,    -1,    -1,    14,    15,
      -1,    -1,    -1,    19,    -1,    -1,    -1,    23,    24,    -1,
      -1,    -1,    28,    -1,    30,    31,    -1,    -1,     4,     5,
       6,     7,     8,    -1,    -1,    -1,    -1,    43,    14,    15,
      -1,    -1,    -1,    19,    -1,    -1,    -1,    23,    24,    55,
      -1,    -1,    28,    -1,    30,    31,    62,    63,    -1,    -1,
      66,    67,    68,    69,    -1,    -1,    -1,    43,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    55,
      -1,    -1,     1,    -1,    -1,     4,    62,    63,     7,    -1,
      66,    67,    -1,    69,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    43,    -1,    -1,    -1,     9,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    62,    63,    26,    27,    -1,    -1,    -1,
      -1,    -1,    33,    34,    35,    36,    37,    38,    39,    40,
      -1,    -1,    -1,    -1,     9,    10,    11,    12,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    -1,    60,
      -1,    26,    27,    -1,    65,    -1,    -1,    68,    33,    34,
      35,    36,    37,    38,    39,    40,    -1,    -1,    -1,    -1,
       9,    10,    11,    12,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    -1,    60,    -1,    26,    27,    -1,
      -1,    -1,    -1,    68,    33,    34,    35,    36,    37,    38,
      39,    40,    -1,    -1,    -1,    -1,     9,    10,    11,    12,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      -1,    60,    -1,    26,    27,    -1,    -1,    -1,    -1,    68,
      33,    34,    35,    36,    37,    38,    39,    40,    -1,    -1,
      -1,    -1,     9,    10,    11,    12,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    -1,    60,    -1,    26,
      27,    -1,    -1,    -1,    -1,    68,    33,    34,    35,    36,
      37,    38,    39,    40,    -1,    -1,    -1,    -1,     9,    10,
      11,    12,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    -1,    60,    -1,    26,    27,    -1,    -1,    -1,
      -1,    68,    33,    34,    35,    36,    37,    38,    39,    40,
      -1,    -1,    -1,    -1,     9,    10,    11,    12,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    -1,    60,
      61,    26,    27,    64,    -1,    -1,    -1,    -1,    33,    34,
      35,    36,    37,    38,    39,    40,    -1,    -1,    -1,     9,
      10,    11,    12,    -1,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    -1,    60,    26,    27,    -1,    64,
      -1,    -1,    -1,    33,    34,    35,    36,    37,    38,    39,
      40,    -1,    -1,    -1,     9,    10,    11,    12,    -1,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    -1,
      60,    26,    27,    -1,    64,    -1,    -1,    -1,    33,    34,
      35,    36,    37,    38,    39,    40,    -1,    -1,    -1,     9,
      10,    11,    12,    -1,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    -1,    60,    26,    27,    -1,    64,
      -1,    -1,    -1,    33,    34,    35,    36,    37,    38,    39,
      40,    -1,    -1,    -1,     9,    10,    11,    12,    -1,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    -1,
      60,    26,    27,    -1,    64,    -1,    -1,    -1,    33,    34,
      35,    36,    37,    38,    39,    40,    -1,    -1,    -1,     9,
      10,    11,    12,    -1,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    -1,    60,    26,    27,    -1,    64,
      -1,    -1,    -1,    33,    34,    35,    36,    37,    38,    39,
      40,    -1,    -1,    -1,    -1,     9,    10,    11,    12,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    -1,
      60,    61,    26,    27,    -1,    -1,    -1,    -1,    -1,    33,
      34,    35,    36,    37,    38,    39,    40,    -1,    -1,    -1,
      -1,     9,    10,    11,    12,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    -1,    60,    61,    26,    27,
      -1,    -1,    -1,    -1,    -1,    33,    34,    35,    36,    37,
      38,    39,    40,    -1,    -1,    -1,    -1,     9,    10,    11,
      12,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    -1,    60,    61,    26,    27,    -1,    -1,    -1,    -1,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    -1,
      -1,    -1,    -1,     9,    10,    11,    12,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    -1,    60,    61,
      26,    27,    -1,    -1,    -1,    -1,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    -1,    -1,    -1,    -1,     9,
      10,    11,    12,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    -1,    60,    61,    26,    27,    -1,    -1,
      -1,    -1,    -1,    33,    34,    35,    36,    37,    38,    39,
      40,    -1,    -1,    -1,    -1,     9,    10,    11,    12,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    -1,
      60,    61,    26,    27,    -1,    -1,    -1,    -1,    -1,    33,
      34,    35,    36,    37,    38,    39,    40,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    -1,    60,    61,     9,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      21,    22,    -1,    -1,    25,    26,    27,    -1,    -1,    -1,
      -1,    -1,    33,    34,    35,    36,    37,    38,    39,    40,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    -1,    60,
       9,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    21,    22,    -1,    -1,    -1,    26,    27,    -1,
      -1,    -1,    -1,    -1,    33,    34,    35,    36,    37,    38,
      39,    40,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      -1,    60,     9,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    20,    -1,    -1,    -1,    -1,    -1,    26,
      27,    -1,    -1,    -1,    -1,    -1,    33,    34,    35,    36,
      37,    38,    39,    40,    -1,    -1,    -1,     9,    10,    11,
      12,    -1,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    -1,    60,    26,    27,    -1,    -1,    -1,    -1,
      -1,    33,    34,    35,    36,    37,    38,    39,    40,    -1,
      -1,    -1,    -1,    -1,    46,    -1,    -1,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    -1,    60,     9,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    25,    26,    27,    -1,    -1,
      -1,    -1,    -1,    33,    34,    35,    36,    37,    38,    39,
      40,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    -1,
      60,     9,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    20,    -1,    -1,    -1,    -1,    -1,    26,    27,
      -1,    -1,    -1,    -1,    -1,    33,    34,    35,    36,    37,
      38,    39,    40,    -1,    -1,    -1,     9,    10,    11,    12,
      -1,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    -1,    60,    26,    27,    -1,    -1,    -1,    -1,    -1,
      33,    34,    35,    36,    37,    38,    39,    40,    -1,    -1,
      -1,     9,    10,    11,    12,    -1,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    -1,    60,    26,    27,
      -1,    -1,    -1,    -1,    -1,    33,    34,    35,    36,    37,
      38,    39,    40,    -1,    -1,    -1,     9,    10,    11,    -1,
      -1,    -1,    -1,    51,    52,    53,    54,    55,    56,    57,
      58,    -1,    60,    26,    27,    -1,    -1,    -1,    -1,    -1,
      33,    34,    35,    36,    37,    38,    39,    40,    -1,    -1,
      10,    11,    -1,    -1,    -1,    -1,    -1,    -1,    51,    52,
      53,    54,    55,    56,    57,    58,    26,    60,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    39,
      40,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    52,    53,    54,    55,    56,    57,    58,     4,
      60,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,     4,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,     4,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    16,    72,    73,     4,     5,     6,     7,     8,    14,
      15,    19,    23,    24,    28,    30,    31,    43,    55,    62,
      63,    66,    67,    69,    76,    80,    83,    89,     0,    17,
      18,    74,    77,    78,    62,    60,    43,     4,    62,    76,
      89,    89,    76,    63,     1,    63,    84,    76,     1,    76,
       4,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    98,     1,     4,     7,    83,     1,    68,    76,     1,
       4,    62,    63,    83,    98,    99,   100,     9,    10,    11,
      12,    26,    27,    33,    34,    35,    36,    37,    38,    39,
      40,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    60,    61,    76,     5,    13,    66,    67,    79,    83,
      79,    75,    76,    80,    74,    61,    76,    76,    90,    91,
      85,    62,    65,    63,    20,    13,    13,    29,     4,     4,
      86,    64,    64,     1,    60,    68,    68,    50,    65,    70,
      65,    76,     4,    98,    65,    65,    70,    50,    76,    76,
      76,    76,    76,    76,    76,    76,    76,    76,    76,    76,
      76,    76,    76,    76,    76,    76,    76,    76,    76,    76,
      76,    76,    60,    63,    67,    69,    92,    93,    94,    67,
      83,     1,    65,    68,    76,    13,    75,    61,    61,    64,
      86,     4,    42,    63,    81,    82,    76,    64,     1,    76,
      93,    93,     1,    76,    49,    44,    45,    47,     1,    99,
      55,    88,    89,    88,    64,    65,    88,    88,    99,     4,
      94,    95,     1,     4,    62,    63,    83,    96,    97,    98,
      41,    49,    76,    60,    68,    76,    60,    65,    68,     4,
      63,    91,    47,     4,     4,    98,    61,    64,    61,     4,
      21,    22,    25,    87,    62,    62,    76,    76,    88,    49,
      65,    88,    50,    68,    65,    65,    76,     4,    65,    50,
      70,    65,    94,    76,    68,    68,    68,    76,    60,     4,
      82,    65,    65,    76,    76,    76,    76,    46,    88,    88,
      94,    94,    94,    64,    65,    94,    97,    94,    60,    60,
      60,    68,    76,    76,    25,    20,    61,    61,    65,    94,
      60,    61,    61,    76,    76,    76,    94,    76,    87,    64,
      61,    64,    76,    64
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    71,    72,    72,    73,    73,    74,    74,    75,    75,
      76,    76,    76,    76,    76,    76,    76,    76,    76,    76,
      76,    76,    76,    76,    76,    76,    76,    76,    76,    76,
      76,    76,    76,    76,    76,    76,    76,    76,    76,    76,
      76,    76,    76,    76,    76,    76,    76,    76,    76,    76,
      77,    77,    78,    78,    78,    79,    80,    80,    81,    81,
      82,    82,    82,    82,    84,    83,    85,    83,    86,    86,
      86,    87,    87,    88,    88,    88,    89,    89,    89,    89,
      89,    89,    89,    89,    89,    89,    89,    89,    89,    89,
      89,    89,    89,    89,    89,    89,    89,    89,    89,    89,
      89,    89,    89,    89,    89,    89,    89,    89,    89,    89,
      89,    89,    89,    89,    89,    89,    89,    90,    90,    91,
      92,    92,    93,    93,    94,    94,    94,    95,    95,    96,
      96,    97,    97,    97,    97,    97,    97,    97,    98,    98,
      98,    98,    98,    98,    98,    98,    98,    98,    98,    98,
      98,    98,    98,    98,    98,    98,    98,    98,    99,    99,
      99,    99,   100,   100,   100,   100,   100,   100,   100,   100,
     100,   100,   100
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     3,     3,     0,     3,     0,     2,     0,     2,
       2,     9,     5,     9,    11,     9,     5,     5,     4,     4,
       2,     4,     5,     2,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     2,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     1,
       2,     3,     5,     4,     2,     1,     5,     8,     1,     3,
       2,     2,     2,     1,     0,     4,     0,     5,     0,     2,
       4,     5,     3,     3,     2,     1,     1,     1,     3,     2,
       3,     2,     4,     3,     2,     1,     3,     2,     2,     3,
       5,     4,     6,     5,     4,     3,     7,     6,     6,     6,
       5,     5,     1,     1,     1,     3,     3,     2,     3,     2,
       2,     1,     4,     3,     3,     4,     3,     1,     3,     1,
       3,     1,     3,     1,     2,     3,     3,     1,     3,     1,
       3,     2,     4,     3,     3,     3,     5,     3,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     0,     1,
       3,     3,     3,     3,     3,     1,     4,     2,     2,     1,
       1,     5,     3
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
#line 36 "src/parser.y" /* yacc.c:1257  */
      { jv_free(((*yyvaluep).literal)); }
#line 1947 "src/parser.c" /* yacc.c:1257  */
        break;

    case 5: /* FIELD  */
#line 36 "src/parser.y" /* yacc.c:1257  */
      { jv_free(((*yyvaluep).literal)); }
#line 1953 "src/parser.c" /* yacc.c:1257  */
        break;

    case 6: /* LITERAL  */
#line 36 "src/parser.y" /* yacc.c:1257  */
      { jv_free(((*yyvaluep).literal)); }
#line 1959 "src/parser.c" /* yacc.c:1257  */
        break;

    case 7: /* FORMAT  */
#line 36 "src/parser.y" /* yacc.c:1257  */
      { jv_free(((*yyvaluep).literal)); }
#line 1965 "src/parser.c" /* yacc.c:1257  */
        break;

    case 44: /* QQSTRING_TEXT  */
#line 36 "src/parser.y" /* yacc.c:1257  */
      { jv_free(((*yyvaluep).literal)); }
#line 1971 "src/parser.c" /* yacc.c:1257  */
        break;

    case 73: /* Module  */
#line 37 "src/parser.y" /* yacc.c:1257  */
      { block_free(((*yyvaluep).blk)); }
#line 1977 "src/parser.c" /* yacc.c:1257  */
        break;

    case 74: /* Imports  */
#line 37 "src/parser.y" /* yacc.c:1257  */
      { block_free(((*yyvaluep).blk)); }
#line 1983 "src/parser.c" /* yacc.c:1257  */
        break;

    case 75: /* FuncDefs  */
#line 37 "src/parser.y" /* yacc.c:1257  */
      { block_free(((*yyvaluep).blk)); }
#line 1989 "src/parser.c" /* yacc.c:1257  */
        break;

    case 76: /* Exp  */
#line 37 "src/parser.y" /* yacc.c:1257  */
      { block_free(((*yyvaluep).blk)); }
#line 1995 "src/parser.c" /* yacc.c:1257  */
        break;

    case 77: /* Import  */
#line 37 "src/parser.y" /* yacc.c:1257  */
      { block_free(((*yyvaluep).blk)); }
#line 2001 "src/parser.c" /* yacc.c:1257  */
        break;

    case 78: /* ImportWhat  */
#line 37 "src/parser.y" /* yacc.c:1257  */
      { block_free(((*yyvaluep).blk)); }
#line 2007 "src/parser.c" /* yacc.c:1257  */
        break;

    case 79: /* ImportFrom  */
#line 37 "src/parser.y" /* yacc.c:1257  */
      { block_free(((*yyvaluep).blk)); }
#line 2013 "src/parser.c" /* yacc.c:1257  */
        break;

    case 80: /* FuncDef  */
#line 37 "src/parser.y" /* yacc.c:1257  */
      { block_free(((*yyvaluep).blk)); }
#line 2019 "src/parser.c" /* yacc.c:1257  */
        break;

    case 81: /* Params  */
#line 37 "src/parser.y" /* yacc.c:1257  */
      { block_free(((*yyvaluep).blk)); }
#line 2025 "src/parser.c" /* yacc.c:1257  */
        break;

    case 82: /* Param  */
#line 37 "src/parser.y" /* yacc.c:1257  */
      { block_free(((*yyvaluep).blk)); }
#line 2031 "src/parser.c" /* yacc.c:1257  */
        break;

    case 83: /* String  */
#line 37 "src/parser.y" /* yacc.c:1257  */
      { block_free(((*yyvaluep).blk)); }
#line 2037 "src/parser.c" /* yacc.c:1257  */
        break;

    case 86: /* QQString  */
#line 37 "src/parser.y" /* yacc.c:1257  */
      { block_free(((*yyvaluep).blk)); }
#line 2043 "src/parser.c" /* yacc.c:1257  */
        break;

    case 87: /* ElseBody  */
#line 37 "src/parser.y" /* yacc.c:1257  */
      { block_free(((*yyvaluep).blk)); }
#line 2049 "src/parser.c" /* yacc.c:1257  */
        break;

    case 88: /* ExpD  */
#line 37 "src/parser.y" /* yacc.c:1257  */
      { block_free(((*yyvaluep).blk)); }
#line 2055 "src/parser.c" /* yacc.c:1257  */
        break;

    case 89: /* Term  */
#line 37 "src/parser.y" /* yacc.c:1257  */
      { block_free(((*yyvaluep).blk)); }
#line 2061 "src/parser.c" /* yacc.c:1257  */
        break;

    case 90: /* Args  */
#line 37 "src/parser.y" /* yacc.c:1257  */
      { block_free(((*yyvaluep).blk)); }
#line 2067 "src/parser.c" /* yacc.c:1257  */
        break;

    case 91: /* Arg  */
#line 37 "src/parser.y" /* yacc.c:1257  */
      { block_free(((*yyvaluep).blk)); }
#line 2073 "src/parser.c" /* yacc.c:1257  */
        break;

    case 92: /* RepPatterns  */
#line 37 "src/parser.y" /* yacc.c:1257  */
      { block_free(((*yyvaluep).blk)); }
#line 2079 "src/parser.c" /* yacc.c:1257  */
        break;

    case 93: /* Patterns  */
#line 37 "src/parser.y" /* yacc.c:1257  */
      { block_free(((*yyvaluep).blk)); }
#line 2085 "src/parser.c" /* yacc.c:1257  */
        break;

    case 94: /* Pattern  */
#line 37 "src/parser.y" /* yacc.c:1257  */
      { block_free(((*yyvaluep).blk)); }
#line 2091 "src/parser.c" /* yacc.c:1257  */
        break;

    case 95: /* ArrayPats  */
#line 37 "src/parser.y" /* yacc.c:1257  */
      { block_free(((*yyvaluep).blk)); }
#line 2097 "src/parser.c" /* yacc.c:1257  */
        break;

    case 96: /* ObjPats  */
#line 37 "src/parser.y" /* yacc.c:1257  */
      { block_free(((*yyvaluep).blk)); }
#line 2103 "src/parser.c" /* yacc.c:1257  */
        break;

    case 97: /* ObjPat  */
#line 37 "src/parser.y" /* yacc.c:1257  */
      { block_free(((*yyvaluep).blk)); }
#line 2109 "src/parser.c" /* yacc.c:1257  */
        break;

    case 98: /* Keyword  */
#line 36 "src/parser.y" /* yacc.c:1257  */
      { jv_free(((*yyvaluep).literal)); }
#line 2115 "src/parser.c" /* yacc.c:1257  */
        break;

    case 99: /* MkDict  */
#line 37 "src/parser.y" /* yacc.c:1257  */
      { block_free(((*yyvaluep).blk)); }
#line 2121 "src/parser.c" /* yacc.c:1257  */
        break;

    case 100: /* MkDictPair  */
#line 37 "src/parser.y" /* yacc.c:1257  */
      { block_free(((*yyvaluep).blk)); }
#line 2127 "src/parser.c" /* yacc.c:1257  */
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
#line 308 "src/parser.y" /* yacc.c:1646  */
    {
  *answer = BLOCK((yyvsp[-2].blk), (yyvsp[-1].blk), gen_op_simple(TOP), (yyvsp[0].blk));
}
#line 2423 "src/parser.c" /* yacc.c:1646  */
    break;

  case 3:
#line 311 "src/parser.y" /* yacc.c:1646  */
    {
  *answer = BLOCK((yyvsp[-2].blk), (yyvsp[-1].blk), (yyvsp[0].blk));
}
#line 2431 "src/parser.c" /* yacc.c:1646  */
    break;

  case 4:
#line 316 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_noop();
}
#line 2439 "src/parser.c" /* yacc.c:1646  */
    break;

  case 5:
#line 319 "src/parser.y" /* yacc.c:1646  */
    {
  if (!block_is_const((yyvsp[-1].blk))) {
    FAIL((yyloc), "Module metadata must be constant");
    (yyval.blk) = gen_noop();
    block_free((yyvsp[-1].blk));
  } else {
    (yyval.blk) = gen_module((yyvsp[-1].blk));
  }
}
#line 2453 "src/parser.c" /* yacc.c:1646  */
    break;

  case 6:
#line 330 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_noop();
}
#line 2461 "src/parser.c" /* yacc.c:1646  */
    break;

  case 7:
#line 333 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = BLOCK((yyvsp[-1].blk), (yyvsp[0].blk));
}
#line 2469 "src/parser.c" /* yacc.c:1646  */
    break;

  case 8:
#line 338 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_noop();
}
#line 2477 "src/parser.c" /* yacc.c:1646  */
    break;

  case 9:
#line 341 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = block_join((yyvsp[-1].blk), (yyvsp[0].blk));
}
#line 2485 "src/parser.c" /* yacc.c:1646  */
    break;

  case 10:
#line 346 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = block_bind_referenced((yyvsp[-1].blk), (yyvsp[0].blk), OP_IS_CALL_PSEUDO);
}
#line 2493 "src/parser.c" /* yacc.c:1646  */
    break;

  case 11:
#line 350 "src/parser.y" /* yacc.c:1646  */
    {
  /* coexp(body) */
  block coexp = gen_call("coexp", gen_lambda((yyvsp[-2].blk)));
  /* as $IDENT | */
  block covar = gen_op_var_fresh(STOREV, jv_string_value((yyvsp[-4].literal)));
  /* def IDENT: $IDENT | fhread; */
  block codef = gen_function(jv_string_value((yyvsp[-4].literal)), gen_noop(), BLOCK(gen_op_unbound(LOADV, jv_string_value((yyvsp[-4].literal))), gen_call("fhread", gen_noop())));

  /* Now bind $6 so it sees the codef */
  block b = block_bind_referenced(codef, (yyvsp[0].blk), OP_IS_CALL_PSEUDO | OP_HAS_BINDING);
  
  /* Now bind that so it sees the variable $IDENT */
  b = block_bind_referenced(covar, b, OP_HAS_VARIABLE);

  /* Now do the rest of the binding for a $IDENT | Exp */
  covar = block_take_block(&b);
  (yyval.blk) = gen_destructure(coexp, covar, b);
  jv_free((yyvsp[-4].literal));
}
#line 2517 "src/parser.c" /* yacc.c:1646  */
    break;

  case 12:
#line 370 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_destructure((yyvsp[-4].blk), (yyvsp[-2].blk), (yyvsp[0].blk));
}
#line 2525 "src/parser.c" /* yacc.c:1646  */
    break;

  case 13:
#line 373 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_reduce((yyvsp[-7].blk), (yyvsp[-5].blk), (yyvsp[-3].blk), (yyvsp[-1].blk));
}
#line 2533 "src/parser.c" /* yacc.c:1646  */
    break;

  case 14:
#line 377 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_foreach((yyvsp[-9].blk), (yyvsp[-7].blk), (yyvsp[-5].blk), (yyvsp[-3].blk), (yyvsp[-1].blk));
}
#line 2541 "src/parser.c" /* yacc.c:1646  */
    break;

  case 15:
#line 381 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_foreach((yyvsp[-7].blk), (yyvsp[-5].blk), (yyvsp[-3].blk), (yyvsp[-1].blk), gen_noop());
}
#line 2549 "src/parser.c" /* yacc.c:1646  */
    break;

  case 16:
#line 385 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_cond((yyvsp[-3].blk), (yyvsp[-1].blk), (yyvsp[0].blk));
}
#line 2557 "src/parser.c" /* yacc.c:1646  */
    break;

  case 17:
#line 388 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_cond((yyvsp[-3].blk), (yyvsp[-1].blk), gen_noop());
}
#line 2565 "src/parser.c" /* yacc.c:1646  */
    break;

  case 18:
#line 391 "src/parser.y" /* yacc.c:1646  */
    {
  FAIL((yyloc), "Possibly unterminated 'if' statement");
  (yyval.blk) = (yyvsp[-2].blk);
}
#line 2574 "src/parser.c" /* yacc.c:1646  */
    break;

  case 19:
#line 396 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_try((yyvsp[-2].blk), (yyvsp[0].blk));
}
#line 2582 "src/parser.c" /* yacc.c:1646  */
    break;

  case 20:
#line 399 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_try((yyvsp[0].blk), gen_op_simple(BACKTRACK));
}
#line 2590 "src/parser.c" /* yacc.c:1646  */
    break;

  case 21:
#line 402 "src/parser.y" /* yacc.c:1646  */
    {
  FAIL((yyloc), "Possibly unterminated 'try' statement");
  (yyval.blk) = (yyvsp[-2].blk);
}
#line 2599 "src/parser.c" /* yacc.c:1646  */
    break;

  case 22:
#line 407 "src/parser.y" /* yacc.c:1646  */
    {
  jv v = jv_string_fmt("*label-%s", jv_string_value((yyvsp[-2].literal)));
  (yyval.blk) = gen_location((yyloc), locations, gen_label(jv_string_value(v), (yyvsp[0].blk)));
  jv_free((yyvsp[-2].literal));
  jv_free(v);
}
#line 2610 "src/parser.c" /* yacc.c:1646  */
    break;

  case 23:
#line 414 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_try((yyvsp[-1].blk), gen_op_simple(BACKTRACK));
}
#line 2618 "src/parser.c" /* yacc.c:1646  */
    break;

  case 24:
#line 418 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_call("_assign", BLOCK(gen_lambda((yyvsp[-2].blk)), gen_lambda((yyvsp[0].blk))));
}
#line 2626 "src/parser.c" /* yacc.c:1646  */
    break;

  case 25:
#line 422 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_or((yyvsp[-2].blk), (yyvsp[0].blk));
}
#line 2634 "src/parser.c" /* yacc.c:1646  */
    break;

  case 26:
#line 426 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_and((yyvsp[-2].blk), (yyvsp[0].blk));
}
#line 2642 "src/parser.c" /* yacc.c:1646  */
    break;

  case 27:
#line 430 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_definedor((yyvsp[-2].blk), (yyvsp[0].blk));
}
#line 2650 "src/parser.c" /* yacc.c:1646  */
    break;

  case 28:
#line 434 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_definedor_assign((yyvsp[-2].blk), (yyvsp[0].blk));
}
#line 2658 "src/parser.c" /* yacc.c:1646  */
    break;

  case 29:
#line 438 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_call("_modify", BLOCK(gen_lambda((yyvsp[-2].blk)), gen_lambda((yyvsp[0].blk))));
}
#line 2666 "src/parser.c" /* yacc.c:1646  */
    break;

  case 30:
#line 442 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = block_join((yyvsp[-2].blk), (yyvsp[0].blk));
}
#line 2674 "src/parser.c" /* yacc.c:1646  */
    break;

  case 31:
#line 446 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_both((yyvsp[-2].blk), (yyvsp[0].blk));
}
#line 2682 "src/parser.c" /* yacc.c:1646  */
    break;

  case 32:
#line 450 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_binop((yyvsp[-2].blk), (yyvsp[0].blk), '+');
}
#line 2690 "src/parser.c" /* yacc.c:1646  */
    break;

  case 33:
#line 454 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_update((yyvsp[-2].blk), (yyvsp[0].blk), '+');
}
#line 2698 "src/parser.c" /* yacc.c:1646  */
    break;

  case 34:
#line 458 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = BLOCK((yyvsp[0].blk), gen_call("_negate", gen_noop()));
}
#line 2706 "src/parser.c" /* yacc.c:1646  */
    break;

  case 35:
#line 462 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_binop((yyvsp[-2].blk), (yyvsp[0].blk), '-');
}
#line 2714 "src/parser.c" /* yacc.c:1646  */
    break;

  case 36:
#line 466 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_update((yyvsp[-2].blk), (yyvsp[0].blk), '-');
}
#line 2722 "src/parser.c" /* yacc.c:1646  */
    break;

  case 37:
#line 470 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_binop((yyvsp[-2].blk), (yyvsp[0].blk), '*');
}
#line 2730 "src/parser.c" /* yacc.c:1646  */
    break;

  case 38:
#line 474 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_update((yyvsp[-2].blk), (yyvsp[0].blk), '*');
}
#line 2738 "src/parser.c" /* yacc.c:1646  */
    break;

  case 39:
#line 478 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_binop((yyvsp[-2].blk), (yyvsp[0].blk), '/');
  if (block_is_const_inf((yyval.blk)))
    FAIL((yyloc), "Division by zero?");
}
#line 2748 "src/parser.c" /* yacc.c:1646  */
    break;

  case 40:
#line 484 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_binop((yyvsp[-2].blk), (yyvsp[0].blk), '%');
  if (block_is_const_inf((yyval.blk)))
    FAIL((yyloc), "Remainder by zero?");
}
#line 2758 "src/parser.c" /* yacc.c:1646  */
    break;

  case 41:
#line 490 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_update((yyvsp[-2].blk), (yyvsp[0].blk), '/');
}
#line 2766 "src/parser.c" /* yacc.c:1646  */
    break;

  case 42:
#line 494 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_update((yyvsp[-2].blk), (yyvsp[0].blk), '%');
}
#line 2774 "src/parser.c" /* yacc.c:1646  */
    break;

  case 43:
#line 498 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_binop((yyvsp[-2].blk), (yyvsp[0].blk), EQ);
}
#line 2782 "src/parser.c" /* yacc.c:1646  */
    break;

  case 44:
#line 502 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_binop((yyvsp[-2].blk), (yyvsp[0].blk), NEQ);
}
#line 2790 "src/parser.c" /* yacc.c:1646  */
    break;

  case 45:
#line 506 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_binop((yyvsp[-2].blk), (yyvsp[0].blk), '<');
}
#line 2798 "src/parser.c" /* yacc.c:1646  */
    break;

  case 46:
#line 510 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_binop((yyvsp[-2].blk), (yyvsp[0].blk), '>');
}
#line 2806 "src/parser.c" /* yacc.c:1646  */
    break;

  case 47:
#line 514 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_binop((yyvsp[-2].blk), (yyvsp[0].blk), LESSEQ);
}
#line 2814 "src/parser.c" /* yacc.c:1646  */
    break;

  case 48:
#line 518 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_binop((yyvsp[-2].blk), (yyvsp[0].blk), GREATEREQ);
}
#line 2822 "src/parser.c" /* yacc.c:1646  */
    break;

  case 49:
#line 522 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = (yyvsp[0].blk);
}
#line 2830 "src/parser.c" /* yacc.c:1646  */
    break;

  case 50:
#line 527 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = (yyvsp[-1].blk);
}
#line 2838 "src/parser.c" /* yacc.c:1646  */
    break;

  case 51:
#line 530 "src/parser.y" /* yacc.c:1646  */
    {
  if (!block_is_const((yyvsp[-1].blk))) {
    FAIL((yyloc), "Module metadata must be constant");
    (yyval.blk) = gen_noop();
    block_free((yyvsp[-2].blk));
    block_free((yyvsp[-1].blk));
  } else if (block_const_kind((yyvsp[-1].blk)) != JV_KIND_OBJECT) {
    FAIL((yyloc), "Module metadata must be an object");
    (yyval.blk) = gen_noop();
    block_free((yyvsp[-2].blk));
    block_free((yyvsp[-1].blk));
  } else {
    (yyval.blk) = gen_import_meta((yyvsp[-2].blk), (yyvsp[-1].blk));
  }
}
#line 2858 "src/parser.c" /* yacc.c:1646  */
    break;

  case 52:
#line 547 "src/parser.y" /* yacc.c:1646  */
    {
  jv v = block_const((yyvsp[-3].blk));
  // XXX Make gen_import take only blocks and the int is_data so we
  // don't have to free so much stuff here
  (yyval.blk) = gen_import(jv_string_value(v), jv_string_value((yyvsp[0].literal)), 1);
  block_free((yyvsp[-3].blk));
  jv_free((yyvsp[0].literal));
  jv_free(v);
}
#line 2872 "src/parser.c" /* yacc.c:1646  */
    break;

  case 53:
#line 556 "src/parser.y" /* yacc.c:1646  */
    {
  jv v = block_const((yyvsp[-2].blk));
  (yyval.blk) = gen_import(jv_string_value(v), jv_string_value((yyvsp[0].literal)), 0);
  block_free((yyvsp[-2].blk));
  jv_free((yyvsp[0].literal));
  jv_free(v);
}
#line 2884 "src/parser.c" /* yacc.c:1646  */
    break;

  case 54:
#line 563 "src/parser.y" /* yacc.c:1646  */
    {
  jv v = block_const((yyvsp[0].blk));
  (yyval.blk) = gen_import(jv_string_value(v), NULL, 0);
  block_free((yyvsp[0].blk));
  jv_free(v);
}
#line 2895 "src/parser.c" /* yacc.c:1646  */
    break;

  case 55:
#line 571 "src/parser.y" /* yacc.c:1646  */
    {
  if (!block_is_const((yyvsp[0].blk))) {
    FAIL((yyloc), "Import path must be constant");
    (yyval.blk) = gen_const(jv_string(""));
    block_free((yyvsp[0].blk));
  } else {
    (yyval.blk) = (yyvsp[0].blk);
  }
}
#line 2909 "src/parser.c" /* yacc.c:1646  */
    break;

  case 56:
#line 582 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_function(jv_string_value((yyvsp[-3].literal)), gen_noop(), (yyvsp[-1].blk));
  jv_free((yyvsp[-3].literal));
}
#line 2918 "src/parser.c" /* yacc.c:1646  */
    break;

  case 57:
#line 587 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_function(jv_string_value((yyvsp[-6].literal)), (yyvsp[-4].blk), (yyvsp[-1].blk));
  jv_free((yyvsp[-6].literal));
}
#line 2927 "src/parser.c" /* yacc.c:1646  */
    break;

  case 58:
#line 593 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = (yyvsp[0].blk);
}
#line 2935 "src/parser.c" /* yacc.c:1646  */
    break;

  case 59:
#line 596 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = BLOCK((yyvsp[-2].blk), (yyvsp[0].blk));
}
#line 2943 "src/parser.c" /* yacc.c:1646  */
    break;

  case 60:
#line 601 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_param_regular(jv_string_value((yyvsp[0].literal)));
  jv_free((yyvsp[0].literal));
}
#line 2952 "src/parser.c" /* yacc.c:1646  */
    break;

  case 61:
#line 605 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_param_regular(jv_string_value((yyvsp[0].literal)));
  jv_free((yyvsp[0].literal));
}
#line 2961 "src/parser.c" /* yacc.c:1646  */
    break;

  case 62:
#line 609 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_param_coexpr(jv_string_value((yyvsp[0].literal)));
  jv_free((yyvsp[0].literal));
}
#line 2970 "src/parser.c" /* yacc.c:1646  */
    break;

  case 63:
#line 613 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_param(jv_string_value((yyvsp[0].literal)));
  jv_free((yyvsp[0].literal));
}
#line 2979 "src/parser.c" /* yacc.c:1646  */
    break;

  case 64:
#line 620 "src/parser.y" /* yacc.c:1646  */
    { (yyval.literal) = jv_string("text"); }
#line 2985 "src/parser.c" /* yacc.c:1646  */
    break;

  case 65:
#line 620 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = (yyvsp[-1].blk);
  jv_free((yyvsp[-2].literal));
}
#line 2994 "src/parser.c" /* yacc.c:1646  */
    break;

  case 66:
#line 624 "src/parser.y" /* yacc.c:1646  */
    { (yyval.literal) = (yyvsp[-1].literal); }
#line 3000 "src/parser.c" /* yacc.c:1646  */
    break;

  case 67:
#line 624 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = (yyvsp[-1].blk);
  jv_free((yyvsp[-2].literal));
}
#line 3009 "src/parser.c" /* yacc.c:1646  */
    break;

  case 68:
#line 631 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_const(jv_string(""));
}
#line 3017 "src/parser.c" /* yacc.c:1646  */
    break;

  case 69:
#line 634 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_binop((yyvsp[-1].blk), gen_const((yyvsp[0].literal)), '+');
}
#line 3025 "src/parser.c" /* yacc.c:1646  */
    break;

  case 70:
#line 637 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_binop((yyvsp[-3].blk), gen_format((yyvsp[-1].blk), jv_copy((yyvsp[-4].literal))), '+');
}
#line 3033 "src/parser.c" /* yacc.c:1646  */
    break;

  case 71:
#line 643 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_cond((yyvsp[-3].blk), (yyvsp[-1].blk), (yyvsp[0].blk));
}
#line 3041 "src/parser.c" /* yacc.c:1646  */
    break;

  case 72:
#line 646 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = (yyvsp[-1].blk);
}
#line 3049 "src/parser.c" /* yacc.c:1646  */
    break;

  case 73:
#line 651 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = block_join((yyvsp[-2].blk), (yyvsp[0].blk));
}
#line 3057 "src/parser.c" /* yacc.c:1646  */
    break;

  case 74:
#line 654 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = BLOCK((yyvsp[0].blk), gen_call("_negate", gen_noop()));
}
#line 3065 "src/parser.c" /* yacc.c:1646  */
    break;

  case 75:
#line 657 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = (yyvsp[0].blk);
}
#line 3073 "src/parser.c" /* yacc.c:1646  */
    break;

  case 76:
#line 663 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_noop();
}
#line 3081 "src/parser.c" /* yacc.c:1646  */
    break;

  case 77:
#line 666 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_call("recurse", gen_noop());
}
#line 3089 "src/parser.c" /* yacc.c:1646  */
    break;

  case 78:
#line 669 "src/parser.y" /* yacc.c:1646  */
    {
  jv v = jv_string_fmt("*label-%s", jv_string_value((yyvsp[0].literal)));     // impossible symbol
  (yyval.blk) = gen_location((yyloc), locations,
                    BLOCK(gen_op_unbound(LOADV, jv_string_value(v)),
                    gen_call("error", gen_noop())));
  jv_free(v);
  jv_free((yyvsp[0].literal));
}
#line 3102 "src/parser.c" /* yacc.c:1646  */
    break;

  case 79:
#line 677 "src/parser.y" /* yacc.c:1646  */
    {
  FAIL((yyloc), "break requires a label to break to");
  (yyval.blk) = gen_noop();
}
#line 3111 "src/parser.c" /* yacc.c:1646  */
    break;

  case 80:
#line 681 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_index_opt((yyvsp[-2].blk), gen_const((yyvsp[-1].literal)));
}
#line 3119 "src/parser.c" /* yacc.c:1646  */
    break;

  case 81:
#line 684 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_index_opt(gen_noop(), gen_const((yyvsp[-1].literal)));
}
#line 3127 "src/parser.c" /* yacc.c:1646  */
    break;

  case 82:
#line 687 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_index_opt((yyvsp[-3].blk), (yyvsp[-1].blk));
}
#line 3135 "src/parser.c" /* yacc.c:1646  */
    break;

  case 83:
#line 690 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_index_opt(gen_noop(), (yyvsp[-1].blk));
}
#line 3143 "src/parser.c" /* yacc.c:1646  */
    break;

  case 84:
#line 693 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_index((yyvsp[-1].blk), gen_const((yyvsp[0].literal)));
}
#line 3151 "src/parser.c" /* yacc.c:1646  */
    break;

  case 85:
#line 696 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_index(gen_noop(), gen_const((yyvsp[0].literal)));
}
#line 3159 "src/parser.c" /* yacc.c:1646  */
    break;

  case 86:
#line 699 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_index((yyvsp[-2].blk), (yyvsp[0].blk));
}
#line 3167 "src/parser.c" /* yacc.c:1646  */
    break;

  case 87:
#line 702 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_index(gen_noop(), (yyvsp[0].blk));
}
#line 3175 "src/parser.c" /* yacc.c:1646  */
    break;

  case 88:
#line 705 "src/parser.y" /* yacc.c:1646  */
    {
  FAIL((yyloc), "try .[\"field\"] instead of .field for unusually named fields");
  (yyval.blk) = gen_noop();
}
#line 3184 "src/parser.c" /* yacc.c:1646  */
    break;

  case 89:
#line 709 "src/parser.y" /* yacc.c:1646  */
    {
  jv_free((yyvsp[-1].literal));
  FAIL((yyloc), "try .[\"field\"] instead of .field for unusually named fields");
  (yyval.blk) = gen_noop();
}
#line 3194 "src/parser.c" /* yacc.c:1646  */
    break;

  case 90:
#line 715 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_index_opt((yyvsp[-4].blk), (yyvsp[-2].blk));
}
#line 3202 "src/parser.c" /* yacc.c:1646  */
    break;

  case 91:
#line 718 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_index((yyvsp[-3].blk), (yyvsp[-1].blk));
}
#line 3210 "src/parser.c" /* yacc.c:1646  */
    break;

  case 92:
#line 721 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_index_opt((yyvsp[-5].blk), (yyvsp[-2].blk));
}
#line 3218 "src/parser.c" /* yacc.c:1646  */
    break;

  case 93:
#line 724 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_index((yyvsp[-4].blk), (yyvsp[-1].blk));
}
#line 3226 "src/parser.c" /* yacc.c:1646  */
    break;

  case 94:
#line 727 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = block_join((yyvsp[-3].blk), gen_op_simple(EACH_OPT));
}
#line 3234 "src/parser.c" /* yacc.c:1646  */
    break;

  case 95:
#line 730 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = block_join((yyvsp[-2].blk), gen_op_simple(EACH));
}
#line 3242 "src/parser.c" /* yacc.c:1646  */
    break;

  case 96:
#line 733 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_slice_index((yyvsp[-6].blk), (yyvsp[-4].blk), (yyvsp[-2].blk), INDEX_OPT);
}
#line 3250 "src/parser.c" /* yacc.c:1646  */
    break;

  case 97:
#line 736 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_slice_index((yyvsp[-5].blk), (yyvsp[-3].blk), gen_const(jv_null()), INDEX_OPT);
}
#line 3258 "src/parser.c" /* yacc.c:1646  */
    break;

  case 98:
#line 739 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_slice_index((yyvsp[-5].blk), gen_const(jv_null()), (yyvsp[-2].blk), INDEX_OPT);
}
#line 3266 "src/parser.c" /* yacc.c:1646  */
    break;

  case 99:
#line 742 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_slice_index((yyvsp[-5].blk), (yyvsp[-3].blk), (yyvsp[-1].blk), INDEX);
}
#line 3274 "src/parser.c" /* yacc.c:1646  */
    break;

  case 100:
#line 745 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_slice_index((yyvsp[-4].blk), (yyvsp[-2].blk), gen_const(jv_null()), INDEX);
}
#line 3282 "src/parser.c" /* yacc.c:1646  */
    break;

  case 101:
#line 748 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_slice_index((yyvsp[-4].blk), gen_const(jv_null()), (yyvsp[-1].blk), INDEX);
}
#line 3290 "src/parser.c" /* yacc.c:1646  */
    break;

  case 102:
#line 751 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_const((yyvsp[0].literal));
}
#line 3298 "src/parser.c" /* yacc.c:1646  */
    break;

  case 103:
#line 754 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = (yyvsp[0].blk);
}
#line 3306 "src/parser.c" /* yacc.c:1646  */
    break;

  case 104:
#line 757 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_format(gen_noop(), (yyvsp[0].literal));
}
#line 3314 "src/parser.c" /* yacc.c:1646  */
    break;

  case 105:
#line 760 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = (yyvsp[-1].blk);
}
#line 3322 "src/parser.c" /* yacc.c:1646  */
    break;

  case 106:
#line 763 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_collect((yyvsp[-1].blk));
}
#line 3330 "src/parser.c" /* yacc.c:1646  */
    break;

  case 107:
#line 766 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_const(jv_array());
}
#line 3338 "src/parser.c" /* yacc.c:1646  */
    break;

  case 108:
#line 769 "src/parser.y" /* yacc.c:1646  */
    {
  block o = gen_const_object((yyvsp[-1].blk));
  if (o.first != NULL)
    (yyval.blk) = o;
  else
    (yyval.blk) = BLOCK(gen_subexp(gen_const(jv_object())), (yyvsp[-1].blk), gen_op_simple(POP));
}
#line 3350 "src/parser.c" /* yacc.c:1646  */
    break;

  case 109:
#line 776 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_location((yyloc), locations, gen_op_unbound(LOADV, jv_string_value((yyvsp[0].literal))));
  jv_free((yyvsp[0].literal));
}
#line 3359 "src/parser.c" /* yacc.c:1646  */
    break;

  case 110:
#line 780 "src/parser.y" /* yacc.c:1646  */
    {
  if (strcmp(jv_string_value((yyvsp[0].literal)), "__loc__") == 0) {
    (yyval.blk) = gen_const(JV_OBJECT(jv_string("file"), jv_copy(locations->fname),
                             jv_string("line"), jv_number(locfile_get_line(locations, (yyloc).start) + 1)));
  } else {
    (yyval.blk) = gen_location((yyloc), locations, gen_op_unbound(LOADV, jv_string_value((yyvsp[0].literal))));
  }
  jv_free((yyvsp[0].literal));
}
#line 3373 "src/parser.c" /* yacc.c:1646  */
    break;

  case 111:
#line 789 "src/parser.y" /* yacc.c:1646  */
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
#line 3390 "src/parser.c" /* yacc.c:1646  */
    break;

  case 112:
#line 801 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_call(jv_string_value((yyvsp[-3].literal)), (yyvsp[-1].blk));
  (yyval.blk) = gen_location((yylsp[-3]), locations, (yyval.blk));
  jv_free((yyvsp[-3].literal));
}
#line 3400 "src/parser.c" /* yacc.c:1646  */
    break;

  case 113:
#line 806 "src/parser.y" /* yacc.c:1646  */
    { (yyval.blk) = gen_noop(); }
#line 3406 "src/parser.c" /* yacc.c:1646  */
    break;

  case 114:
#line 807 "src/parser.y" /* yacc.c:1646  */
    { (yyval.blk) = gen_noop(); }
#line 3412 "src/parser.c" /* yacc.c:1646  */
    break;

  case 115:
#line 808 "src/parser.y" /* yacc.c:1646  */
    { (yyval.blk) = (yyvsp[-3].blk); }
#line 3418 "src/parser.c" /* yacc.c:1646  */
    break;

  case 116:
#line 809 "src/parser.y" /* yacc.c:1646  */
    { (yyval.blk) = gen_noop(); }
#line 3424 "src/parser.c" /* yacc.c:1646  */
    break;

  case 117:
#line 812 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = (yyvsp[0].blk);
}
#line 3432 "src/parser.c" /* yacc.c:1646  */
    break;

  case 118:
#line 815 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = BLOCK((yyvsp[-2].blk), (yyvsp[0].blk));
}
#line 3440 "src/parser.c" /* yacc.c:1646  */
    break;

  case 119:
#line 820 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_lambda((yyvsp[0].blk));
}
#line 3448 "src/parser.c" /* yacc.c:1646  */
    break;

  case 120:
#line 825 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = BLOCK((yyvsp[-2].blk), gen_destructure_alt((yyvsp[0].blk)));
}
#line 3456 "src/parser.c" /* yacc.c:1646  */
    break;

  case 121:
#line 828 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_destructure_alt((yyvsp[0].blk));
}
#line 3464 "src/parser.c" /* yacc.c:1646  */
    break;

  case 122:
#line 833 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = BLOCK((yyvsp[-2].blk), (yyvsp[0].blk));
}
#line 3472 "src/parser.c" /* yacc.c:1646  */
    break;

  case 123:
#line 836 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = (yyvsp[0].blk);
}
#line 3480 "src/parser.c" /* yacc.c:1646  */
    break;

  case 124:
#line 841 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_op_unbound(STOREV, jv_string_value((yyvsp[0].literal)));
  jv_free((yyvsp[0].literal));
}
#line 3489 "src/parser.c" /* yacc.c:1646  */
    break;

  case 125:
#line 845 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = BLOCK((yyvsp[-1].blk), gen_op_simple(POP));
}
#line 3497 "src/parser.c" /* yacc.c:1646  */
    break;

  case 126:
#line 848 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = BLOCK((yyvsp[-1].blk), gen_op_simple(POP));
}
#line 3505 "src/parser.c" /* yacc.c:1646  */
    break;

  case 127:
#line 853 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_array_matcher(gen_noop(), (yyvsp[0].blk));
}
#line 3513 "src/parser.c" /* yacc.c:1646  */
    break;

  case 128:
#line 856 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_array_matcher((yyvsp[-2].blk), (yyvsp[0].blk));
}
#line 3521 "src/parser.c" /* yacc.c:1646  */
    break;

  case 129:
#line 861 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = (yyvsp[0].blk);
}
#line 3529 "src/parser.c" /* yacc.c:1646  */
    break;

  case 130:
#line 864 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = BLOCK((yyvsp[-2].blk), (yyvsp[0].blk));
}
#line 3537 "src/parser.c" /* yacc.c:1646  */
    break;

  case 131:
#line 869 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_object_matcher(gen_const((yyvsp[0].literal)), gen_op_unbound(STOREV, jv_string_value((yyvsp[0].literal))));
}
#line 3545 "src/parser.c" /* yacc.c:1646  */
    break;

  case 132:
#line 872 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_object_matcher(gen_const((yyvsp[-2].literal)), BLOCK(gen_op_simple(DUP), gen_op_unbound(STOREV, jv_string_value((yyvsp[-2].literal))), (yyvsp[0].blk)));
}
#line 3553 "src/parser.c" /* yacc.c:1646  */
    break;

  case 133:
#line 875 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_object_matcher(gen_const((yyvsp[-2].literal)), (yyvsp[0].blk));
}
#line 3561 "src/parser.c" /* yacc.c:1646  */
    break;

  case 134:
#line 878 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_object_matcher(gen_const((yyvsp[-2].literal)), (yyvsp[0].blk));
}
#line 3569 "src/parser.c" /* yacc.c:1646  */
    break;

  case 135:
#line 881 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_object_matcher((yyvsp[-2].blk), (yyvsp[0].blk));
}
#line 3577 "src/parser.c" /* yacc.c:1646  */
    break;

  case 136:
#line 884 "src/parser.y" /* yacc.c:1646  */
    {
  jv msg = check_object_key((yyvsp[-3].blk));
  if (jv_is_valid(msg)) {
    FAIL((yyloc), jv_string_value(msg));
  }
  jv_free(msg);
  (yyval.blk) = gen_object_matcher((yyvsp[-3].blk), (yyvsp[0].blk));
}
#line 3590 "src/parser.c" /* yacc.c:1646  */
    break;

  case 137:
#line 892 "src/parser.y" /* yacc.c:1646  */
    {
  FAIL((yyloc), "May need parentheses around object key expression");
  (yyval.blk) = (yyvsp[0].blk);
}
#line 3599 "src/parser.c" /* yacc.c:1646  */
    break;

  case 138:
#line 898 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.literal) = jv_string("as");
}
#line 3607 "src/parser.c" /* yacc.c:1646  */
    break;

  case 139:
#line 901 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.literal) = jv_string("def");
}
#line 3615 "src/parser.c" /* yacc.c:1646  */
    break;

  case 140:
#line 904 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.literal) = jv_string("codef");
}
#line 3623 "src/parser.c" /* yacc.c:1646  */
    break;

  case 141:
#line 907 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.literal) = jv_string("module");
}
#line 3631 "src/parser.c" /* yacc.c:1646  */
    break;

  case 142:
#line 910 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.literal) = jv_string("import");
}
#line 3639 "src/parser.c" /* yacc.c:1646  */
    break;

  case 143:
#line 913 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.literal) = jv_string("include");
}
#line 3647 "src/parser.c" /* yacc.c:1646  */
    break;

  case 144:
#line 916 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.literal) = jv_string("if");
}
#line 3655 "src/parser.c" /* yacc.c:1646  */
    break;

  case 145:
#line 919 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.literal) = jv_string("then");
}
#line 3663 "src/parser.c" /* yacc.c:1646  */
    break;

  case 146:
#line 922 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.literal) = jv_string("else");
}
#line 3671 "src/parser.c" /* yacc.c:1646  */
    break;

  case 147:
#line 925 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.literal) = jv_string("elif");
}
#line 3679 "src/parser.c" /* yacc.c:1646  */
    break;

  case 148:
#line 928 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.literal) = jv_string("reduce");
}
#line 3687 "src/parser.c" /* yacc.c:1646  */
    break;

  case 149:
#line 931 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.literal) = jv_string("foreach");
}
#line 3695 "src/parser.c" /* yacc.c:1646  */
    break;

  case 150:
#line 934 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.literal) = jv_string("end");
}
#line 3703 "src/parser.c" /* yacc.c:1646  */
    break;

  case 151:
#line 937 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.literal) = jv_string("and");
}
#line 3711 "src/parser.c" /* yacc.c:1646  */
    break;

  case 152:
#line 940 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.literal) = jv_string("or");
}
#line 3719 "src/parser.c" /* yacc.c:1646  */
    break;

  case 153:
#line 943 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.literal) = jv_string("try");
}
#line 3727 "src/parser.c" /* yacc.c:1646  */
    break;

  case 154:
#line 946 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.literal) = jv_string("catch");
}
#line 3735 "src/parser.c" /* yacc.c:1646  */
    break;

  case 155:
#line 949 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.literal) = jv_string("label");
}
#line 3743 "src/parser.c" /* yacc.c:1646  */
    break;

  case 156:
#line 952 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.literal) = jv_string("break");
}
#line 3751 "src/parser.c" /* yacc.c:1646  */
    break;

  case 157:
#line 955 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.literal) = jv_string("__loc__");
}
#line 3759 "src/parser.c" /* yacc.c:1646  */
    break;

  case 158:
#line 960 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk)=gen_noop();
}
#line 3767 "src/parser.c" /* yacc.c:1646  */
    break;

  case 159:
#line 963 "src/parser.y" /* yacc.c:1646  */
    { (yyval.blk) = (yyvsp[0].blk); }
#line 3773 "src/parser.c" /* yacc.c:1646  */
    break;

  case 160:
#line 964 "src/parser.y" /* yacc.c:1646  */
    { (yyval.blk)=block_join((yyvsp[-2].blk), (yyvsp[0].blk)); }
#line 3779 "src/parser.c" /* yacc.c:1646  */
    break;

  case 161:
#line 965 "src/parser.y" /* yacc.c:1646  */
    { (yyval.blk) = (yyvsp[0].blk); }
#line 3785 "src/parser.c" /* yacc.c:1646  */
    break;

  case 162:
#line 968 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_dictpair(gen_const((yyvsp[-2].literal)), (yyvsp[0].blk));
 }
#line 3793 "src/parser.c" /* yacc.c:1646  */
    break;

  case 163:
#line 971 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_dictpair(gen_const((yyvsp[-2].literal)), (yyvsp[0].blk));
  }
#line 3801 "src/parser.c" /* yacc.c:1646  */
    break;

  case 164:
#line 974 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_dictpair((yyvsp[-2].blk), (yyvsp[0].blk));
  }
#line 3809 "src/parser.c" /* yacc.c:1646  */
    break;

  case 165:
#line 977 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_dictpair((yyvsp[0].blk), BLOCK(gen_op_simple(POP), gen_op_simple(DUP2),
                              gen_op_simple(DUP2), gen_op_simple(INDEX)));
  }
#line 3818 "src/parser.c" /* yacc.c:1646  */
    break;

  case 166:
#line 981 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_dictpair(gen_location((yyloc), locations, gen_op_unbound(LOADV, jv_string_value((yyvsp[-2].literal)))),
                    (yyvsp[0].blk));
  }
#line 3827 "src/parser.c" /* yacc.c:1646  */
    break;

  case 167:
#line 985 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_dictpair(gen_const((yyvsp[0].literal)),
                    gen_location((yyloc), locations, gen_op_unbound(LOADV, jv_string_value((yyvsp[0].literal)))));
  }
#line 3836 "src/parser.c" /* yacc.c:1646  */
    break;

  case 168:
#line 989 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_dictpair(gen_const((yyvsp[0].literal)),
                    gen_location((yyloc), locations, gen_op_unbound(LOADV, jv_string_value((yyvsp[0].literal)))));
  }
#line 3845 "src/parser.c" /* yacc.c:1646  */
    break;

  case 169:
#line 993 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_dictpair(gen_const(jv_copy((yyvsp[0].literal))),
                    gen_index(gen_noop(), gen_const((yyvsp[0].literal))));
  }
#line 3854 "src/parser.c" /* yacc.c:1646  */
    break;

  case 170:
#line 997 "src/parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_dictpair(gen_const(jv_copy((yyvsp[0].literal))),
                    gen_index(gen_noop(), gen_const((yyvsp[0].literal))));
  }
#line 3863 "src/parser.c" /* yacc.c:1646  */
    break;

  case 171:
#line 1001 "src/parser.y" /* yacc.c:1646  */
    {
  jv msg = check_object_key((yyvsp[-3].blk));
  if (jv_is_valid(msg)) {
    FAIL((yyloc), jv_string_value(msg));
  }
  jv_free(msg);
  (yyval.blk) = gen_dictpair((yyvsp[-3].blk), (yyvsp[0].blk));
  }
#line 3876 "src/parser.c" /* yacc.c:1646  */
    break;

  case 172:
#line 1009 "src/parser.y" /* yacc.c:1646  */
    {
  FAIL((yyloc), "May need parentheses around object key expression");
  (yyval.blk) = (yyvsp[0].blk);
  }
#line 3885 "src/parser.c" /* yacc.c:1646  */
    break;


#line 3889 "src/parser.c" /* yacc.c:1646  */
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
#line 1013 "src/parser.y" /* yacc.c:1906  */


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

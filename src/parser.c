/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
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
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 1 "src/parser.y"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "compile.h"
#include "jv_alloc.h"
#include "builtin.h"
#define YYMALLOC jv_mem_alloc
#define YYFREE jv_mem_free

#line 83 "src/parser.c"

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

/* Use api.header.include to #include this header
   instead of duplicating it here.  */
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
#line 12 "src/parser.y"

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

#line 135 "src/parser.c"

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    INVALID_CHARACTER = 258,       /* INVALID_CHARACTER  */
    IDENT = 259,                   /* IDENT  */
    FIELD = 260,                   /* FIELD  */
    BINDING = 261,                 /* BINDING  */
    LITERAL = 262,                 /* LITERAL  */
    FORMAT = 263,                  /* FORMAT  */
    REC = 264,                     /* ".."  */
    SETMOD = 265,                  /* "%="  */
    EQ = 266,                      /* "=="  */
    NEQ = 267,                     /* "!="  */
    DEFINEDOR = 268,               /* "//"  */
    AS = 269,                      /* "as"  */
    DEF = 270,                     /* "def"  */
    MODULE = 271,                  /* "module"  */
    IMPORT = 272,                  /* "import"  */
    INCLUDE = 273,                 /* "include"  */
    IF = 274,                      /* "if"  */
    THEN = 275,                    /* "then"  */
    ELSE = 276,                    /* "else"  */
    ELSE_IF = 277,                 /* "elif"  */
    REDUCE = 278,                  /* "reduce"  */
    FOREACH = 279,                 /* "foreach"  */
    END = 280,                     /* "end"  */
    AND = 281,                     /* "and"  */
    OR = 282,                      /* "or"  */
    TRY = 283,                     /* "try"  */
    CATCH = 284,                   /* "catch"  */
    LABEL = 285,                   /* "label"  */
    BREAK = 286,                   /* "break"  */
    LOC = 287,                     /* "$__loc__"  */
    SETPIPE = 288,                 /* "|="  */
    SETPLUS = 289,                 /* "+="  */
    SETMINUS = 290,                /* "-="  */
    SETMULT = 291,                 /* "*="  */
    SETDIV = 292,                  /* "/="  */
    SETDEFINEDOR = 293,            /* "//="  */
    LESSEQ = 294,                  /* "<="  */
    GREATEREQ = 295,               /* ">="  */
    ALTERNATION = 296,             /* "?//"  */
    QQSTRING_START = 297,          /* QQSTRING_START  */
    QQSTRING_TEXT = 298,           /* QQSTRING_TEXT  */
    QQSTRING_INTERP_START = 299,   /* QQSTRING_INTERP_START  */
    QQSTRING_INTERP_END = 300,     /* QQSTRING_INTERP_END  */
    QQSTRING_END = 301,            /* QQSTRING_END  */
    FUNCDEF = 302,                 /* FUNCDEF  */
    NONOPT = 303                   /* NONOPT  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif
/* Token kinds.  */
#define YYEMPTY -2
#define YYEOF 0
#define YYerror 256
#define YYUNDEF 257
#define INVALID_CHARACTER 258
#define IDENT 259
#define FIELD 260
#define BINDING 261
#define LITERAL 262
#define FORMAT 263
#define REC 264
#define SETMOD 265
#define EQ 266
#define NEQ 267
#define DEFINEDOR 268
#define AS 269
#define DEF 270
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
#define QQSTRING_START 297
#define QQSTRING_TEXT 298
#define QQSTRING_INTERP_START 299
#define QQSTRING_INTERP_END 300
#define QQSTRING_END 301
#define FUNCDEF 302
#define NONOPT 303

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 32 "src/parser.y"

  jv literal;
  block blk;

#line 256 "src/parser.c"

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
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_INVALID_CHARACTER = 3,          /* INVALID_CHARACTER  */
  YYSYMBOL_IDENT = 4,                      /* IDENT  */
  YYSYMBOL_FIELD = 5,                      /* FIELD  */
  YYSYMBOL_BINDING = 6,                    /* BINDING  */
  YYSYMBOL_LITERAL = 7,                    /* LITERAL  */
  YYSYMBOL_FORMAT = 8,                     /* FORMAT  */
  YYSYMBOL_REC = 9,                        /* ".."  */
  YYSYMBOL_SETMOD = 10,                    /* "%="  */
  YYSYMBOL_EQ = 11,                        /* "=="  */
  YYSYMBOL_NEQ = 12,                       /* "!="  */
  YYSYMBOL_DEFINEDOR = 13,                 /* "//"  */
  YYSYMBOL_AS = 14,                        /* "as"  */
  YYSYMBOL_DEF = 15,                       /* "def"  */
  YYSYMBOL_MODULE = 16,                    /* "module"  */
  YYSYMBOL_IMPORT = 17,                    /* "import"  */
  YYSYMBOL_INCLUDE = 18,                   /* "include"  */
  YYSYMBOL_IF = 19,                        /* "if"  */
  YYSYMBOL_THEN = 20,                      /* "then"  */
  YYSYMBOL_ELSE = 21,                      /* "else"  */
  YYSYMBOL_ELSE_IF = 22,                   /* "elif"  */
  YYSYMBOL_REDUCE = 23,                    /* "reduce"  */
  YYSYMBOL_FOREACH = 24,                   /* "foreach"  */
  YYSYMBOL_END = 25,                       /* "end"  */
  YYSYMBOL_AND = 26,                       /* "and"  */
  YYSYMBOL_OR = 27,                        /* "or"  */
  YYSYMBOL_TRY = 28,                       /* "try"  */
  YYSYMBOL_CATCH = 29,                     /* "catch"  */
  YYSYMBOL_LABEL = 30,                     /* "label"  */
  YYSYMBOL_BREAK = 31,                     /* "break"  */
  YYSYMBOL_LOC = 32,                       /* "$__loc__"  */
  YYSYMBOL_SETPIPE = 33,                   /* "|="  */
  YYSYMBOL_SETPLUS = 34,                   /* "+="  */
  YYSYMBOL_SETMINUS = 35,                  /* "-="  */
  YYSYMBOL_SETMULT = 36,                   /* "*="  */
  YYSYMBOL_SETDIV = 37,                    /* "/="  */
  YYSYMBOL_SETDEFINEDOR = 38,              /* "//="  */
  YYSYMBOL_LESSEQ = 39,                    /* "<="  */
  YYSYMBOL_GREATEREQ = 40,                 /* ">="  */
  YYSYMBOL_ALTERNATION = 41,               /* "?//"  */
  YYSYMBOL_QQSTRING_START = 42,            /* QQSTRING_START  */
  YYSYMBOL_QQSTRING_TEXT = 43,             /* QQSTRING_TEXT  */
  YYSYMBOL_QQSTRING_INTERP_START = 44,     /* QQSTRING_INTERP_START  */
  YYSYMBOL_QQSTRING_INTERP_END = 45,       /* QQSTRING_INTERP_END  */
  YYSYMBOL_QQSTRING_END = 46,              /* QQSTRING_END  */
  YYSYMBOL_FUNCDEF = 47,                   /* FUNCDEF  */
  YYSYMBOL_48_ = 48,                       /* '|'  */
  YYSYMBOL_49_ = 49,                       /* ','  */
  YYSYMBOL_50_ = 50,                       /* '='  */
  YYSYMBOL_51_ = 51,                       /* '<'  */
  YYSYMBOL_52_ = 52,                       /* '>'  */
  YYSYMBOL_53_ = 53,                       /* '+'  */
  YYSYMBOL_54_ = 54,                       /* '-'  */
  YYSYMBOL_55_ = 55,                       /* '*'  */
  YYSYMBOL_56_ = 56,                       /* '/'  */
  YYSYMBOL_57_ = 57,                       /* '%'  */
  YYSYMBOL_NONOPT = 58,                    /* NONOPT  */
  YYSYMBOL_59_ = 59,                       /* '?'  */
  YYSYMBOL_60_ = 60,                       /* '.'  */
  YYSYMBOL_61_ = 61,                       /* '['  */
  YYSYMBOL_62_ = 62,                       /* ';'  */
  YYSYMBOL_63_ = 63,                       /* ':'  */
  YYSYMBOL_64_ = 64,                       /* '('  */
  YYSYMBOL_65_ = 65,                       /* ')'  */
  YYSYMBOL_66_ = 66,                       /* ']'  */
  YYSYMBOL_67_ = 67,                       /* '{'  */
  YYSYMBOL_68_ = 68,                       /* '}'  */
  YYSYMBOL_69_ = 69,                       /* '$'  */
  YYSYMBOL_YYACCEPT = 70,                  /* $accept  */
  YYSYMBOL_TopLevel = 71,                  /* TopLevel  */
  YYSYMBOL_Module = 72,                    /* Module  */
  YYSYMBOL_Imports = 73,                   /* Imports  */
  YYSYMBOL_FuncDefs = 74,                  /* FuncDefs  */
  YYSYMBOL_Query = 75,                     /* Query  */
  YYSYMBOL_Expr = 76,                      /* Expr  */
  YYSYMBOL_Import = 77,                    /* Import  */
  YYSYMBOL_ImportWhat = 78,                /* ImportWhat  */
  YYSYMBOL_ImportFrom = 79,                /* ImportFrom  */
  YYSYMBOL_FuncDef = 80,                   /* FuncDef  */
  YYSYMBOL_Params = 81,                    /* Params  */
  YYSYMBOL_Param = 82,                     /* Param  */
  YYSYMBOL_StringStart = 83,               /* StringStart  */
  YYSYMBOL_String = 84,                    /* String  */
  YYSYMBOL_QQString = 85,                  /* QQString  */
  YYSYMBOL_ElseBody = 86,                  /* ElseBody  */
  YYSYMBOL_Term = 87,                      /* Term  */
  YYSYMBOL_Args = 88,                      /* Args  */
  YYSYMBOL_Arg = 89,                       /* Arg  */
  YYSYMBOL_RepPatterns = 90,               /* RepPatterns  */
  YYSYMBOL_Patterns = 91,                  /* Patterns  */
  YYSYMBOL_Pattern = 92,                   /* Pattern  */
  YYSYMBOL_ArrayPats = 93,                 /* ArrayPats  */
  YYSYMBOL_ObjPats = 94,                   /* ObjPats  */
  YYSYMBOL_ObjPat = 95,                    /* ObjPat  */
  YYSYMBOL_Keyword = 96,                   /* Keyword  */
  YYSYMBOL_DictPairs = 97,                 /* DictPairs  */
  YYSYMBOL_DictPair = 98,                  /* DictPair  */
  YYSYMBOL_DictExpr = 99                   /* DictExpr  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;


/* Second part of user prologue.  */
#line 127 "src/parser.y"

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
  locfile_locate(locations, *loc, "jq: error: %s", s);
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

  jv jv_a = block_const(a);
  block_free(a);
  jv jv_b = block_const(b);
  block_free(b);

  jv res = jv_invalid();
  switch (op) {
  case '+': res = binop_plus(jv_a, jv_b); break;
  case '-': res = binop_minus(jv_a, jv_b); break;
  case '*': res = binop_multiply(jv_a, jv_b); break;
  case '/': res = binop_divide(jv_a, jv_b); break;
  case '%': res = binop_mod(jv_a, jv_b); break;
  case EQ: res = binop_equal(jv_a, jv_b); break;
  case NEQ: res = binop_notequal(jv_a, jv_b); break;
  case '<': res = binop_less(jv_a, jv_b); break;
  case '>': res = binop_greater(jv_a, jv_b); break;
  case LESSEQ: res = binop_lesseq(jv_a, jv_b); break;
  case GREATEREQ: res = binop_greatereq(jv_a, jv_b); break;
  }

  if (jv_is_valid(res))
    return gen_const(res);

  return gen_error(jv_invalid_get_msg(res));
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

static block gen_loc_object(location *loc, struct locfile *locations) {
  return gen_const(JV_OBJECT(jv_string("file"), jv_copy(locations->fname),
                             jv_string("line"), jv_number(locfile_get_line(locations, loc->start) + 1)));
}


#line 543 "src/parser.c"


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

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
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
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
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

#if 1

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
#endif /* 1 */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL \
             && defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
  YYLTYPE yyls_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE) \
             + YYSIZEOF (YYLTYPE)) \
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
#define YYFINAL  31
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1184

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  70
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  30
/* YYNRULES -- Number of rules.  */
#define YYNRULES  169
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  316

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   303


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,    69,    57,     2,     2,
      64,    65,    55,    53,    49,    54,    60,    56,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    63,    62,
      51,    50,    52,    59,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    61,     2,    66,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    67,    48,    68,     2,     2,     2,     2,
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
      45,    46,    47,    58
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   278,   278,   281,   286,   289,   304,   307,   312,   315,
     321,   324,   327,   333,   336,   339,   345,   348,   351,   354,
     357,   360,   363,   366,   369,   372,   375,   378,   381,   384,
     387,   390,   393,   396,   399,   402,   405,   408,   411,   417,
     420,   437,   446,   453,   461,   472,   476,   480,   486,   489,
     494,   498,   505,   508,   514,   521,   524,   527,   534,   537,
     540,   546,   549,   552,   560,   564,   567,   570,   573,   576,
     579,   582,   585,   588,   592,   598,   601,   604,   607,   610,
     613,   616,   619,   622,   625,   628,   631,   634,   637,   640,
     643,   646,   649,   654,   657,   660,   663,   666,   673,   676,
     679,   682,   685,   689,   692,   696,   714,   718,   722,   725,
     737,   742,   743,   744,   745,   748,   751,   756,   761,   764,
     769,   772,   777,   781,   784,   789,   792,   797,   800,   805,
     808,   811,   814,   817,   820,   828,   834,   837,   840,   843,
     846,   849,   852,   855,   858,   861,   864,   867,   870,   873,
     876,   879,   882,   885,   891,   894,   897,   902,   905,   908,
     911,   915,   920,   924,   928,   932,   936,   944,   950,   953
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if 1
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "INVALID_CHARACTER",
  "IDENT", "FIELD", "BINDING", "LITERAL", "FORMAT", "\"..\"", "\"%=\"",
  "\"==\"", "\"!=\"", "\"//\"", "\"as\"", "\"def\"", "\"module\"",
  "\"import\"", "\"include\"", "\"if\"", "\"then\"", "\"else\"",
  "\"elif\"", "\"reduce\"", "\"foreach\"", "\"end\"", "\"and\"", "\"or\"",
  "\"try\"", "\"catch\"", "\"label\"", "\"break\"", "\"$__loc__\"",
  "\"|=\"", "\"+=\"", "\"-=\"", "\"*=\"", "\"/=\"", "\"//=\"", "\"<=\"",
  "\">=\"", "\"?//\"", "QQSTRING_START", "QQSTRING_TEXT",
  "QQSTRING_INTERP_START", "QQSTRING_INTERP_END", "QQSTRING_END",
  "FUNCDEF", "'|'", "','", "'='", "'<'", "'>'", "'+'", "'-'", "'*'", "'/'",
  "'%'", "NONOPT", "'?'", "'.'", "'['", "';'", "':'", "'('", "')'", "']'",
  "'{'", "'}'", "'$'", "$accept", "TopLevel", "Module", "Imports",
  "FuncDefs", "Query", "Expr", "Import", "ImportWhat", "ImportFrom",
  "FuncDef", "Params", "Param", "StringStart", "String", "QQString",
  "ElseBody", "Term", "Args", "Arg", "RepPatterns", "Patterns", "Pattern",
  "ArrayPats", "ObjPats", "ObjPat", "Keyword", "DictPairs", "DictPair",
  "DictExpr", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-148)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-155)

#define yytable_value_is_error(Yyn) \
  ((Yyn) == YYTABLE_NINF)

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
      -7,   894,    18,   181,   -40,     0,  -148,  -148,    20,  -148,
      70,   894,   960,   960,   960,    63,    16,  -148,  -148,   960,
     283,   349,   416,   612,    10,    27,   999,   894,  -148,  -148,
       6,  -148,    31,    31,   894,   181,   696,   894,  -148,  -148,
     145,    55,    75,  1031,  1063,   106,    93,  -148,  -148,     6,
    -148,   149,    20,   103,   108,  -148,   -47,   130,   129,     9,
     109,   118,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,
    -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,
    -148,   894,   139,   141,   148,   179,   166,   894,   894,  -148,
     960,   960,   960,   960,    -3,   960,   960,   960,   960,   960,
     960,   960,   960,   960,   960,   960,   960,   960,   960,   960,
     960,   960,   960,   171,   180,   173,  -148,    21,   206,   233,
    -148,  -148,  -148,   171,   894,  -148,  -148,    72,   171,    64,
    -148,   894,    77,   894,   483,    -3,    -3,   550,   894,  -148,
    -148,  -148,  -148,  -148,  -148,   960,  -148,   960,   960,   131,
     960,   960,  -148,   667,   243,   171,  -148,  1127,   610,   610,
    1095,  -148,    -3,    26,   211,   209,   220,   920,   509,  1127,
    1127,  1127,  1127,  1127,  1127,   610,   610,  1127,   610,   610,
     189,   189,  -148,  -148,  -148,  -148,   894,  -148,  -148,   762,
     205,   199,   894,   215,    79,   163,  -148,  -148,   894,  -148,
     169,  -148,  -148,   128,  -148,   191,  -148,   122,   204,   213,
    -148,  -148,   171,  1095,   234,   234,   234,   218,   234,   234,
     222,  -148,  -148,  -148,   -43,   223,   226,   227,   894,   229,
     -37,  -148,   235,    -3,   894,    91,   247,    22,  -148,  -148,
     107,  -148,   828,   248,  -148,  -148,  -148,  -148,    77,   239,
    -148,   894,   894,  -148,  -148,   894,   894,   960,   960,    -3,
    -148,    -3,    -3,    -3,   135,    -3,    26,  -148,    -3,   274,
     171,  -148,  -148,   267,   270,   271,   116,  -148,  -148,   894,
      36,   105,   194,   214,   234,   234,  -148,  -148,  -148,  -148,
     261,  -148,  -148,  -148,  -148,  -148,  -148,   282,   265,  -148,
     894,   894,   894,    -3,  -148,  -148,   122,   138,   127,  -148,
    -148,  -148,   894,  -148,   157,  -148
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       4,     0,     0,     6,   109,    70,   107,    90,    92,    62,
       0,     0,     0,     0,     0,     0,     0,   108,    53,     0,
       0,     0,     0,     0,     0,     0,    15,     0,    55,    91,
      38,     1,     0,     0,     8,     6,     0,     0,    66,    52,
       0,     0,     0,     0,     0,   105,     0,    64,    63,    93,
      73,     0,     0,    72,     0,    96,     0,     0,     0,     0,
     163,   162,   136,   137,   138,   139,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,   150,   151,   152,   153,
     164,     0,   160,   165,     0,   155,     0,     0,     0,     5,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    10,     0,    69,    89,     0,     0,     0,
      44,    43,     3,     2,     8,     7,    39,     0,   117,     0,
     115,     0,     0,     0,     0,     0,     0,     0,     0,    74,
      68,   112,    95,   111,    94,     0,   114,     0,     0,     0,
       0,     0,    97,     0,     0,    13,    14,    31,    32,    33,
      16,   122,     0,     0,     0,     0,   121,    19,    18,    21,
      23,    25,    27,    30,    20,    36,    37,    17,    34,    35,
      22,    24,    26,    28,    29,    56,     0,    54,    65,     0,
      71,     0,     0,    80,     0,     0,     9,    40,     0,   110,
       0,    51,    50,     0,    48,     0,   102,     0,     0,     0,
     104,   103,    12,   169,   167,   157,   161,     0,   159,   158,
       0,   156,   106,   125,     0,     0,     0,   129,     0,     0,
       0,   127,     0,     0,     0,     0,    82,     0,    67,   113,
       0,    79,     0,    76,    42,    41,   116,    45,     0,     0,
      47,     0,     0,    60,   101,     0,     0,     0,     0,     0,
     123,     0,     0,     0,     0,     0,     0,   124,     0,   120,
      11,    57,    81,    78,    88,    87,     0,    75,    49,     0,
       0,     0,     0,     0,   168,   166,   126,   135,   131,   130,
       0,   133,   128,   132,    77,    85,    84,    86,     0,    59,
       0,     0,     0,     0,    83,    46,     0,     0,     0,   134,
      58,    98,     0,   100,     0,    99
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -148,  -148,  -148,   312,   228,    -1,     1,  -148,  -148,   326,
      -8,  -148,   112,  -148,     5,  -148,    56,   342,  -148,   165,
    -148,   144,  -102,  -148,  -148,    99,  -147,   216,  -148,  -143
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_uint8 yydefgoto[] =
{
       0,     2,     3,    34,   122,   113,    26,    35,    36,   119,
      27,   203,   204,    28,    29,   114,   254,    30,   129,   130,
     164,   165,   166,   224,   230,   231,    83,    84,    85,   214
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      25,    87,    88,   161,   215,   216,   259,   218,   219,     1,
      42,   115,   266,    43,    44,    45,   232,    47,    31,   142,
      56,    58,    48,   260,    37,    53,   124,   225,    82,    52,
     226,   267,   227,   123,    52,   127,   128,   120,   120,    52,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,   162,    38,
     223,   299,    39,    18,   163,   116,   117,   118,    18,    46,
      87,    88,   145,    18,    40,    87,    88,   146,    41,    86,
     149,   201,   189,   202,    87,    88,   155,   156,   273,    89,
     228,   157,   158,   159,   160,   134,   167,   168,   169,   170,
     171,   172,   173,   174,   175,   176,   177,   178,   179,   180,
     181,   182,   183,   184,   284,   285,   124,   194,   133,   232,
      87,    88,   190,    87,    88,   300,   198,    87,    88,   199,
     200,   269,   205,   207,   197,   137,   271,   212,   211,    87,
      88,   138,   242,   251,   252,   243,   213,   253,   213,   213,
     139,   213,   213,    87,    88,    87,    88,   286,    82,   287,
     288,   289,   140,   291,    87,    88,   293,   244,   229,   245,
      87,    88,   147,   274,   141,    87,    88,    87,    88,    87,
      88,   148,   297,    87,    88,   235,    87,    88,   237,   312,
     248,   240,   313,   249,   144,   143,   217,   128,    32,    33,
     290,   309,   150,   311,   151,    87,    88,   191,   131,   132,
       4,     5,     6,     7,     8,     9,   152,    87,    88,    87,
      88,    10,   315,   185,   186,    11,   187,   264,   153,    12,
      13,   247,   188,   270,    14,   154,    15,    16,    17,    87,
      88,   276,    87,    88,   110,   111,   112,   195,    18,   222,
     280,   281,   233,   250,   282,   283,   301,   234,   213,   213,
      19,  -119,    87,    88,   238,   239,    20,    21,   255,   192,
      22,   229,   193,    23,   241,    24,   302,   256,   298,   208,
     209,   258,   257,   -61,    50,   145,   261,    51,   -61,   262,
     263,    52,   265,   -61,   -61,   -61,   -61,   -61,   268,   306,
     307,   308,   279,   -61,   -61,   -61,   272,   277,   -61,   -61,
     -61,   314,   -61,    87,    88,  -118,   -61,   -61,   -61,   -61,
     -61,   -61,   -61,   -61,   303,    18,   294,   305,   -61,   295,
     296,   -61,   -61,   -61,   -61,   -61,   -61,   -61,   -61,   -61,
     -61,   304,   -61,   -61,   -61,   -61,   -61,   125,   -61,   -61,
      54,   -61,   196,     4,     5,     6,     7,     8,     9,   121,
     278,    49,   310,   246,    10,   292,     0,     0,    11,   221,
       0,     0,    12,    13,     0,     0,     0,    14,     0,    15,
      16,    17,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    18,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    19,     0,     0,     0,     0,     0,    20,
      21,     0,     0,    22,     0,    55,    23,    57,    24,     0,
       4,     5,     6,     7,     8,     9,     0,     0,     0,     0,
       0,    10,     0,     0,     0,    11,     0,     0,     0,    12,
      13,     0,     0,     0,    14,     0,    15,    16,    17,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    18,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      19,     0,     0,     0,     0,     0,    20,    21,     0,     0,
      22,     0,     0,    23,   206,    24,     0,     4,     5,     6,
       7,     8,     9,     0,     0,     0,     0,     0,    10,     0,
       0,     0,    11,     0,     0,     0,    12,    13,     0,     0,
       0,    14,     0,    15,    16,    17,     0,     0,     0,     0,
      91,    92,     0,     0,     0,    18,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    95,     0,    19,     0,     0,
       0,     0,     0,    20,    21,     0,     0,    22,   103,   104,
      23,   210,    24,     0,     4,     5,     6,     7,     8,     9,
     106,   107,   108,   109,   110,   111,   112,     0,     0,    11,
       0,     0,     0,    12,    13,     0,     0,     0,    14,     0,
       0,    16,    17,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    18,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    19,     0,     0,     0,     0,     0,
      20,    21,     0,    59,    22,     0,    60,    23,    61,    24,
      52,  -155,  -155,     0,     0,     0,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,     0,     0,     0,     0,  -155,
    -155,     0,     0,     0,    18,     0,     0,     0,     0,     0,
       0,  -155,  -155,   108,   109,   110,   111,   112,   220,     0,
       0,    60,     0,    61,     0,    52,    81,     0,     0,     0,
    -154,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
       4,     5,     6,     7,     8,     9,     0,     0,     0,    18,
       0,    10,     0,     0,     0,    11,     0,     0,     0,    12,
      13,     0,     0,     0,    14,     0,    15,    16,    17,     0,
       0,    81,     0,     0,     0,  -154,     0,     0,    18,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      19,     0,     0,     0,     0,     0,    20,    21,   126,     0,
      22,     0,     0,    23,     0,    24,     4,     5,     6,     7,
       8,     9,     0,     0,     0,     0,     0,    10,     0,     0,
       0,    11,     0,     0,     0,    12,    13,     0,     0,     0,
      14,     0,    15,    16,    17,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    18,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    19,     0,     0,     0,
       0,     0,    20,    21,     0,     0,    22,     0,   236,    23,
       0,    24,     4,     5,     6,     7,     8,     9,     0,     0,
       0,     0,     0,    10,     0,     0,     0,    11,     0,     0,
       0,    12,    13,     0,     0,     0,    14,     0,    15,    16,
      17,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      18,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    19,     0,     0,     0,     0,     0,    20,    21,
       0,     0,    22,     0,   275,    23,     0,    24,     4,     5,
       6,     7,     8,     9,     0,     0,     0,     0,     0,    10,
       0,     0,     0,    11,     0,     0,     0,    12,    13,     0,
       0,     0,    14,     0,    15,    16,    17,     0,     0,     0,
       0,    91,    92,     0,     0,     0,    18,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    19,     0,
       0,     0,     0,     0,    20,    21,     0,     0,    22,   103,
     104,    23,     0,    24,     4,     5,     6,     7,     8,     9,
       0,   106,   107,   108,   109,   110,   111,   112,     0,    11,
       0,     0,     0,    12,    13,     0,     0,     0,    14,     0,
       0,    16,    17,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    18,     0,     0,     0,     0,     0,     0,    90,
      91,    92,    93,    94,    19,     0,     0,     0,     0,     0,
      20,    21,     0,     0,    22,    95,    96,    23,     0,    24,
       0,     0,    97,    98,    99,   100,   101,   102,   103,   104,
       0,    90,    91,    92,    93,   135,     0,     0,     0,   105,
     106,   107,   108,   109,   110,   111,   112,    95,    96,     0,
       0,     0,     0,     0,    97,    98,    99,   100,   101,   102,
     103,   104,     0,    90,    91,    92,    93,   136,     0,     0,
       0,   105,   106,   107,   108,   109,   110,   111,   112,    95,
      96,     0,     0,     0,     0,     0,    97,    98,    99,   100,
     101,   102,   103,   104,     0,    90,    91,    92,    93,     0,
       0,     0,     0,   105,   106,   107,   108,   109,   110,   111,
     112,    95,    96,     0,     0,     0,     0,     0,    97,    98,
      99,   100,   101,   102,   103,   104,     0,  -155,    91,    92,
       0,     0,     0,     0,     0,   105,   106,   107,   108,   109,
     110,   111,   112,    95,    96,     0,     0,     0,     0,     0,
    -155,  -155,  -155,  -155,  -155,  -155,   103,   104,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  -155,   106,   107,
     108,   109,   110,   111,   112
};

static const yytype_int16 yycheck[] =
{
       1,    48,    49,     6,   147,   148,    49,   150,   151,    16,
      11,     5,    49,    12,    13,    14,   163,     1,     0,    66,
      21,    22,     6,    66,    64,    20,    34,     1,    23,     8,
       4,    68,     6,    34,     8,    36,    37,    32,    33,     8,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    61,    59,
     162,    25,    42,    42,    67,    59,    60,    61,    42,     6,
      48,    49,    63,    42,     4,    48,    49,    68,     8,    69,
      81,     4,    61,     6,    48,    49,    87,    88,    66,    62,
      64,    90,    91,    92,    93,    20,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   257,   258,   124,   118,    63,   266,
      48,    49,   117,    48,    49,    20,    62,    48,    49,    65,
     131,   233,   133,   134,    62,    29,    45,   138,   137,    48,
      49,    48,    63,    21,    22,    66,   145,    25,   147,   148,
       1,   150,   151,    48,    49,    48,    49,   259,   153,   261,
     262,   263,    59,   265,    48,    49,   268,     4,   163,     6,
      48,    49,    63,    66,    66,    48,    49,    48,    49,    48,
      49,    63,    66,    48,    49,   186,    48,    49,   189,    62,
      62,   192,    65,    65,    65,    65,    65,   198,    17,    18,
      65,   303,    63,    65,    63,    48,    49,     1,    63,    64,
       4,     5,     6,     7,     8,     9,    68,    48,    49,    48,
      49,    15,    65,    43,    44,    19,    46,   228,    49,    23,
      24,    62,    59,   234,    28,    69,    30,    31,    32,    48,
      49,   242,    48,    49,    55,    56,    57,    14,    42,     6,
     251,   252,    41,    62,   255,   256,    62,    48,   257,   258,
      54,    41,    48,    49,    59,    66,    60,    61,    64,    63,
      64,   266,    66,    67,    59,    69,    62,    64,   279,   135,
     136,    63,    48,     0,     1,    63,    63,     4,     5,    63,
      63,     8,    63,    10,    11,    12,    13,    14,    63,   300,
     301,   302,    63,    20,    21,    22,    59,    59,    25,    26,
      27,   312,    29,    48,    49,    41,    33,    34,    35,    36,
      37,    38,    39,    40,    63,    42,    59,    62,    45,    59,
      59,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    59,    59,    60,    61,    62,    63,    35,    65,    66,
       1,    68,   124,     4,     5,     6,     7,     8,     9,    33,
     248,    19,   306,   198,    15,   266,    -1,    -1,    19,   153,
      -1,    -1,    23,    24,    -1,    -1,    -1,    28,    -1,    30,
      31,    32,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    42,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    54,    -1,    -1,    -1,    -1,    -1,    60,
      61,    -1,    -1,    64,    -1,    66,    67,     1,    69,    -1,
       4,     5,     6,     7,     8,     9,    -1,    -1,    -1,    -1,
      -1,    15,    -1,    -1,    -1,    19,    -1,    -1,    -1,    23,
      24,    -1,    -1,    -1,    28,    -1,    30,    31,    32,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      54,    -1,    -1,    -1,    -1,    -1,    60,    61,    -1,    -1,
      64,    -1,    -1,    67,     1,    69,    -1,     4,     5,     6,
       7,     8,     9,    -1,    -1,    -1,    -1,    -1,    15,    -1,
      -1,    -1,    19,    -1,    -1,    -1,    23,    24,    -1,    -1,
      -1,    28,    -1,    30,    31,    32,    -1,    -1,    -1,    -1,
      11,    12,    -1,    -1,    -1,    42,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    26,    -1,    54,    -1,    -1,
      -1,    -1,    -1,    60,    61,    -1,    -1,    64,    39,    40,
      67,     1,    69,    -1,     4,     5,     6,     7,     8,     9,
      51,    52,    53,    54,    55,    56,    57,    -1,    -1,    19,
      -1,    -1,    -1,    23,    24,    -1,    -1,    -1,    28,    -1,
      -1,    31,    32,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    42,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    54,    -1,    -1,    -1,    -1,    -1,
      60,    61,    -1,     1,    64,    -1,     4,    67,     6,    69,
       8,    11,    12,    -1,    -1,    -1,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    -1,    -1,    -1,    -1,    39,
      40,    -1,    -1,    -1,    42,    -1,    -1,    -1,    -1,    -1,
      -1,    51,    52,    53,    54,    55,    56,    57,     1,    -1,
      -1,     4,    -1,     6,    -1,     8,    64,    -1,    -1,    -1,
      68,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
       4,     5,     6,     7,     8,     9,    -1,    -1,    -1,    42,
      -1,    15,    -1,    -1,    -1,    19,    -1,    -1,    -1,    23,
      24,    -1,    -1,    -1,    28,    -1,    30,    31,    32,    -1,
      -1,    64,    -1,    -1,    -1,    68,    -1,    -1,    42,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      54,    -1,    -1,    -1,    -1,    -1,    60,    61,    62,    -1,
      64,    -1,    -1,    67,    -1,    69,     4,     5,     6,     7,
       8,     9,    -1,    -1,    -1,    -1,    -1,    15,    -1,    -1,
      -1,    19,    -1,    -1,    -1,    23,    24,    -1,    -1,    -1,
      28,    -1,    30,    31,    32,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    42,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    54,    -1,    -1,    -1,
      -1,    -1,    60,    61,    -1,    -1,    64,    -1,    66,    67,
      -1,    69,     4,     5,     6,     7,     8,     9,    -1,    -1,
      -1,    -1,    -1,    15,    -1,    -1,    -1,    19,    -1,    -1,
      -1,    23,    24,    -1,    -1,    -1,    28,    -1,    30,    31,
      32,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      42,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    54,    -1,    -1,    -1,    -1,    -1,    60,    61,
      -1,    -1,    64,    -1,    66,    67,    -1,    69,     4,     5,
       6,     7,     8,     9,    -1,    -1,    -1,    -1,    -1,    15,
      -1,    -1,    -1,    19,    -1,    -1,    -1,    23,    24,    -1,
      -1,    -1,    28,    -1,    30,    31,    32,    -1,    -1,    -1,
      -1,    11,    12,    -1,    -1,    -1,    42,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    54,    -1,
      -1,    -1,    -1,    -1,    60,    61,    -1,    -1,    64,    39,
      40,    67,    -1,    69,     4,     5,     6,     7,     8,     9,
      -1,    51,    52,    53,    54,    55,    56,    57,    -1,    19,
      -1,    -1,    -1,    23,    24,    -1,    -1,    -1,    28,    -1,
      -1,    31,    32,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    42,    -1,    -1,    -1,    -1,    -1,    -1,    10,
      11,    12,    13,    14,    54,    -1,    -1,    -1,    -1,    -1,
      60,    61,    -1,    -1,    64,    26,    27,    67,    -1,    69,
      -1,    -1,    33,    34,    35,    36,    37,    38,    39,    40,
      -1,    10,    11,    12,    13,    14,    -1,    -1,    -1,    50,
      51,    52,    53,    54,    55,    56,    57,    26,    27,    -1,
      -1,    -1,    -1,    -1,    33,    34,    35,    36,    37,    38,
      39,    40,    -1,    10,    11,    12,    13,    14,    -1,    -1,
      -1,    50,    51,    52,    53,    54,    55,    56,    57,    26,
      27,    -1,    -1,    -1,    -1,    -1,    33,    34,    35,    36,
      37,    38,    39,    40,    -1,    10,    11,    12,    13,    -1,
      -1,    -1,    -1,    50,    51,    52,    53,    54,    55,    56,
      57,    26,    27,    -1,    -1,    -1,    -1,    -1,    33,    34,
      35,    36,    37,    38,    39,    40,    -1,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    50,    51,    52,    53,    54,
      55,    56,    57,    26,    27,    -1,    -1,    -1,    -1,    -1,
      33,    34,    35,    36,    37,    38,    39,    40,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    50,    51,    52,
      53,    54,    55,    56,    57
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,    16,    71,    72,     4,     5,     6,     7,     8,     9,
      15,    19,    23,    24,    28,    30,    31,    32,    42,    54,
      60,    61,    64,    67,    69,    75,    76,    80,    83,    84,
      87,     0,    17,    18,    73,    77,    78,    64,    59,    42,
       4,     8,    75,    76,    76,    76,     6,     1,     6,    87,
       1,     4,     8,    84,     1,    66,    75,     1,    75,     1,
       4,     6,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    64,    84,    96,    97,    98,    69,    48,    49,    62,
      10,    11,    12,    13,    14,    26,    27,    33,    34,    35,
      36,    37,    38,    39,    40,    50,    51,    52,    53,    54,
      55,    56,    57,    75,    85,     5,    59,    60,    61,    79,
      84,    79,    74,    75,    80,    73,    62,    75,    75,    88,
      89,    63,    64,    63,    20,    14,    14,    29,    48,     1,
      59,    66,    66,    65,    65,    63,    68,    63,    63,    75,
      63,    63,    68,    49,    69,    75,    75,    76,    76,    76,
      76,     6,    61,    67,    90,    91,    92,    76,    76,    76,
      76,    76,    76,    76,    76,    76,    76,    76,    76,    76,
      76,    76,    76,    76,    76,    43,    44,    46,    59,    61,
      84,     1,    63,    66,    75,    14,    74,    62,    62,    65,
      75,     4,     6,    81,    82,    75,     1,    75,    91,    91,
       1,    76,    75,    76,    99,    99,    99,    65,    99,    99,
       1,    97,     6,    92,    93,     1,     4,     6,    64,    84,
      94,    95,    96,    41,    48,    75,    66,    75,    59,    66,
      75,    59,    63,    66,     4,     6,    89,    62,    62,    65,
      62,    21,    22,    25,    86,    64,    64,    48,    63,    49,
      66,    63,    63,    63,    75,    63,    49,    68,    63,    92,
      75,    45,    59,    66,    66,    66,    75,    59,    82,    63,
      75,    75,    75,    75,    99,    99,    92,    92,    92,    92,
      65,    92,    95,    92,    59,    59,    59,    66,    75,    25,
      20,    62,    62,    63,    59,    62,    75,    75,    75,    92,
      86,    65,    62,    65,    75,    65
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    70,    71,    71,    72,    72,    73,    73,    74,    74,
      75,    75,    75,    75,    75,    75,    76,    76,    76,    76,
      76,    76,    76,    76,    76,    76,    76,    76,    76,    76,
      76,    76,    76,    76,    76,    76,    76,    76,    76,    77,
      77,    78,    78,    78,    79,    80,    80,    80,    81,    81,
      82,    82,    83,    83,    84,    85,    85,    85,    86,    86,
      86,    87,    87,    87,    87,    87,    87,    87,    87,    87,
      87,    87,    87,    87,    87,    87,    87,    87,    87,    87,
      87,    87,    87,    87,    87,    87,    87,    87,    87,    87,
      87,    87,    87,    87,    87,    87,    87,    87,    87,    87,
      87,    87,    87,    87,    87,    87,    87,    87,    87,    87,
      87,    87,    87,    87,    87,    88,    88,    89,    90,    90,
      91,    91,    92,    92,    92,    93,    93,    94,    94,    95,
      95,    95,    95,    95,    95,    95,    96,    96,    96,    96,
      96,    96,    96,    96,    96,    96,    96,    96,    96,    96,
      96,    96,    96,    96,    97,    97,    97,    98,    98,    98,
      98,    98,    98,    98,    98,    98,    98,    98,    99,    99
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     3,     3,     0,     3,     0,     2,     0,     2,
       2,     5,     4,     3,     3,     1,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     1,     2,
       3,     4,     4,     2,     1,     5,     8,     5,     1,     3,
       1,     1,     2,     1,     3,     0,     2,     4,     5,     3,
       1,     1,     1,     2,     2,     3,     2,     4,     3,     2,
       1,     3,     2,     2,     3,     5,     4,     6,     5,     4,
       3,     5,     4,     7,     6,     6,     6,     5,     5,     2,
       1,     1,     1,     2,     3,     3,     2,     3,     9,    11,
       9,     5,     4,     4,     4,     2,     4,     1,     1,     1,
       4,     3,     3,     4,     3,     1,     3,     1,     3,     1,
       3,     1,     1,     3,     3,     1,     3,     1,     3,     1,
       3,     3,     3,     3,     5,     3,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     0,     1,     3,     3,     3,     3,
       1,     3,     1,     1,     1,     1,     5,     3,     3,     1
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


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
        yyerror (&yylloc, answer, errors, locations, lexer_param_ptr, YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF

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


/* YYLOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

# ifndef YYLOCATION_PRINT

#  if defined YY_LOCATION_PRINT

   /* Temporary convenience wrapper in case some people defined the
      undocumented and private YY_LOCATION_PRINT macros.  */
#   define YYLOCATION_PRINT(File, Loc)  YY_LOCATION_PRINT(File, *(Loc))

#  elif defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL

/* Print *YYLOCP on YYO.  Private, do not rely on its existence. */

YY_ATTRIBUTE_UNUSED
static int
yy_location_print_ (FILE *yyo, YYLTYPE const * const yylocp)
{
  int res = 0;
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

#   define YYLOCATION_PRINT  yy_location_print_

    /* Temporary convenience wrapper in case some people defined the
       undocumented and private YY_LOCATION_PRINT macros.  */
#   define YY_LOCATION_PRINT(File, Loc)  YYLOCATION_PRINT(File, &(Loc))

#  else

#   define YYLOCATION_PRINT(File, Loc) ((void) 0)
    /* Temporary convenience wrapper in case some people defined the
       undocumented and private YY_LOCATION_PRINT macros.  */
#   define YY_LOCATION_PRINT  YYLOCATION_PRINT

#  endif
# endif /* !defined YYLOCATION_PRINT */


# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value, Location, answer, errors, locations, lexer_param_ptr); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, block* answer, int* errors, struct locfile* locations, struct lexer_param* lexer_param_ptr)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  YY_USE (yylocationp);
  YY_USE (answer);
  YY_USE (errors);
  YY_USE (locations);
  YY_USE (lexer_param_ptr);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, block* answer, int* errors, struct locfile* locations, struct lexer_param* lexer_param_ptr)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  YYLOCATION_PRINT (yyo, yylocationp);
  YYFPRINTF (yyo, ": ");
  yy_symbol_value_print (yyo, yykind, yyvaluep, yylocationp, answer, errors, locations, lexer_param_ptr);
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
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp, YYLTYPE *yylsp,
                 int yyrule, block* answer, int* errors, struct locfile* locations, struct lexer_param* lexer_param_ptr)
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
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)],
                       &(yylsp[(yyi + 1) - (yynrhs)]), answer, errors, locations, lexer_param_ptr);
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
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
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


/* Context of a parse error.  */
typedef struct
{
  yy_state_t *yyssp;
  yysymbol_kind_t yytoken;
  YYLTYPE *yylloc;
} yypcontext_t;

/* Put in YYARG at most YYARGN of the expected tokens given the
   current YYCTX, and return the number of tokens stored in YYARG.  If
   YYARG is null, return the number of expected tokens (guaranteed to
   be less than YYNTOKENS).  Return YYENOMEM on memory exhaustion.
   Return 0 if there are more than YYARGN expected tokens, yet fill
   YYARG up to YYARGN. */
static int
yypcontext_expected_tokens (const yypcontext_t *yyctx,
                            yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
  int yycount = 0;
  int yyn = yypact[+*yyctx->yyssp];
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
        if (yycheck[yyx + yyn] == yyx && yyx != YYSYMBOL_YYerror
            && !yytable_value_is_error (yytable[yyx + yyn]))
          {
            if (!yyarg)
              ++yycount;
            else if (yycount == yyargn)
              return 0;
            else
              yyarg[yycount++] = YY_CAST (yysymbol_kind_t, yyx);
          }
    }
  if (yyarg && yycount == 0 && 0 < yyargn)
    yyarg[0] = YYSYMBOL_YYEMPTY;
  return yycount;
}




#ifndef yystrlen
# if defined __GLIBC__ && defined _STRING_H
#  define yystrlen(S) (YY_CAST (YYPTRDIFF_T, strlen (S)))
# else
/* Return the length of YYSTR.  */
static YYPTRDIFF_T
yystrlen (const char *yystr)
{
  YYPTRDIFF_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
# endif
#endif

#ifndef yystpcpy
# if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#  define yystpcpy stpcpy
# else
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
# endif
#endif

#ifndef yytnamerr
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
#endif


static int
yy_syntax_error_arguments (const yypcontext_t *yyctx,
                           yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
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
  if (yyctx->yytoken != YYSYMBOL_YYEMPTY)
    {
      int yyn;
      if (yyarg)
        yyarg[yycount] = yyctx->yytoken;
      ++yycount;
      yyn = yypcontext_expected_tokens (yyctx,
                                        yyarg ? yyarg + 1 : yyarg, yyargn - 1);
      if (yyn == YYENOMEM)
        return YYENOMEM;
      else
        yycount += yyn;
    }
  return yycount;
}

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return -1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return YYENOMEM if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYPTRDIFF_T *yymsg_alloc, char **yymsg,
                const yypcontext_t *yyctx)
{
  enum { YYARGS_MAX = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat: reported tokens (one for the "unexpected",
     one per "expected"). */
  yysymbol_kind_t yyarg[YYARGS_MAX];
  /* Cumulated lengths of YYARG.  */
  YYPTRDIFF_T yysize = 0;

  /* Actual size of YYARG. */
  int yycount = yy_syntax_error_arguments (yyctx, yyarg, YYARGS_MAX);
  if (yycount == YYENOMEM)
    return YYENOMEM;

  switch (yycount)
    {
#define YYCASE_(N, S)                       \
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
#undef YYCASE_
    }

  /* Compute error message size.  Don't count the "%s"s, but reserve
     room for the terminator.  */
  yysize = yystrlen (yyformat) - 2 * yycount + 1;
  {
    int yyi;
    for (yyi = 0; yyi < yycount; ++yyi)
      {
        YYPTRDIFF_T yysize1
          = yysize + yytnamerr (YY_NULLPTR, yytname[yyarg[yyi]]);
        if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
          yysize = yysize1;
        else
          return YYENOMEM;
      }
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return -1;
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
          yyp += yytnamerr (yyp, yytname[yyarg[yyi++]]);
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


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep, YYLTYPE *yylocationp, block* answer, int* errors, struct locfile* locations, struct lexer_param* lexer_param_ptr)
{
  YY_USE (yyvaluep);
  YY_USE (yylocationp);
  YY_USE (answer);
  YY_USE (errors);
  YY_USE (locations);
  YY_USE (lexer_param_ptr);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  switch (yykind)
    {
    case YYSYMBOL_IDENT: /* IDENT  */
#line 37 "src/parser.y"
            { jv_free(((*yyvaluep).literal)); }
#line 1992 "src/parser.c"
        break;

    case YYSYMBOL_FIELD: /* FIELD  */
#line 37 "src/parser.y"
            { jv_free(((*yyvaluep).literal)); }
#line 1998 "src/parser.c"
        break;

    case YYSYMBOL_BINDING: /* BINDING  */
#line 37 "src/parser.y"
            { jv_free(((*yyvaluep).literal)); }
#line 2004 "src/parser.c"
        break;

    case YYSYMBOL_LITERAL: /* LITERAL  */
#line 37 "src/parser.y"
            { jv_free(((*yyvaluep).literal)); }
#line 2010 "src/parser.c"
        break;

    case YYSYMBOL_FORMAT: /* FORMAT  */
#line 37 "src/parser.y"
            { jv_free(((*yyvaluep).literal)); }
#line 2016 "src/parser.c"
        break;

    case YYSYMBOL_QQSTRING_TEXT: /* QQSTRING_TEXT  */
#line 37 "src/parser.y"
            { jv_free(((*yyvaluep).literal)); }
#line 2022 "src/parser.c"
        break;

    case YYSYMBOL_Module: /* Module  */
#line 38 "src/parser.y"
            { block_free(((*yyvaluep).blk)); }
#line 2028 "src/parser.c"
        break;

    case YYSYMBOL_Imports: /* Imports  */
#line 38 "src/parser.y"
            { block_free(((*yyvaluep).blk)); }
#line 2034 "src/parser.c"
        break;

    case YYSYMBOL_FuncDefs: /* FuncDefs  */
#line 38 "src/parser.y"
            { block_free(((*yyvaluep).blk)); }
#line 2040 "src/parser.c"
        break;

    case YYSYMBOL_Query: /* Query  */
#line 38 "src/parser.y"
            { block_free(((*yyvaluep).blk)); }
#line 2046 "src/parser.c"
        break;

    case YYSYMBOL_Expr: /* Expr  */
#line 38 "src/parser.y"
            { block_free(((*yyvaluep).blk)); }
#line 2052 "src/parser.c"
        break;

    case YYSYMBOL_Import: /* Import  */
#line 38 "src/parser.y"
            { block_free(((*yyvaluep).blk)); }
#line 2058 "src/parser.c"
        break;

    case YYSYMBOL_ImportWhat: /* ImportWhat  */
#line 38 "src/parser.y"
            { block_free(((*yyvaluep).blk)); }
#line 2064 "src/parser.c"
        break;

    case YYSYMBOL_ImportFrom: /* ImportFrom  */
#line 38 "src/parser.y"
            { block_free(((*yyvaluep).blk)); }
#line 2070 "src/parser.c"
        break;

    case YYSYMBOL_FuncDef: /* FuncDef  */
#line 38 "src/parser.y"
            { block_free(((*yyvaluep).blk)); }
#line 2076 "src/parser.c"
        break;

    case YYSYMBOL_Params: /* Params  */
#line 38 "src/parser.y"
            { block_free(((*yyvaluep).blk)); }
#line 2082 "src/parser.c"
        break;

    case YYSYMBOL_Param: /* Param  */
#line 38 "src/parser.y"
            { block_free(((*yyvaluep).blk)); }
#line 2088 "src/parser.c"
        break;

    case YYSYMBOL_StringStart: /* StringStart  */
#line 37 "src/parser.y"
            { jv_free(((*yyvaluep).literal)); }
#line 2094 "src/parser.c"
        break;

    case YYSYMBOL_String: /* String  */
#line 38 "src/parser.y"
            { block_free(((*yyvaluep).blk)); }
#line 2100 "src/parser.c"
        break;

    case YYSYMBOL_QQString: /* QQString  */
#line 38 "src/parser.y"
            { block_free(((*yyvaluep).blk)); }
#line 2106 "src/parser.c"
        break;

    case YYSYMBOL_ElseBody: /* ElseBody  */
#line 38 "src/parser.y"
            { block_free(((*yyvaluep).blk)); }
#line 2112 "src/parser.c"
        break;

    case YYSYMBOL_Term: /* Term  */
#line 38 "src/parser.y"
            { block_free(((*yyvaluep).blk)); }
#line 2118 "src/parser.c"
        break;

    case YYSYMBOL_Args: /* Args  */
#line 38 "src/parser.y"
            { block_free(((*yyvaluep).blk)); }
#line 2124 "src/parser.c"
        break;

    case YYSYMBOL_Arg: /* Arg  */
#line 38 "src/parser.y"
            { block_free(((*yyvaluep).blk)); }
#line 2130 "src/parser.c"
        break;

    case YYSYMBOL_RepPatterns: /* RepPatterns  */
#line 38 "src/parser.y"
            { block_free(((*yyvaluep).blk)); }
#line 2136 "src/parser.c"
        break;

    case YYSYMBOL_Patterns: /* Patterns  */
#line 38 "src/parser.y"
            { block_free(((*yyvaluep).blk)); }
#line 2142 "src/parser.c"
        break;

    case YYSYMBOL_Pattern: /* Pattern  */
#line 38 "src/parser.y"
            { block_free(((*yyvaluep).blk)); }
#line 2148 "src/parser.c"
        break;

    case YYSYMBOL_ArrayPats: /* ArrayPats  */
#line 38 "src/parser.y"
            { block_free(((*yyvaluep).blk)); }
#line 2154 "src/parser.c"
        break;

    case YYSYMBOL_ObjPats: /* ObjPats  */
#line 38 "src/parser.y"
            { block_free(((*yyvaluep).blk)); }
#line 2160 "src/parser.c"
        break;

    case YYSYMBOL_ObjPat: /* ObjPat  */
#line 38 "src/parser.y"
            { block_free(((*yyvaluep).blk)); }
#line 2166 "src/parser.c"
        break;

    case YYSYMBOL_Keyword: /* Keyword  */
#line 37 "src/parser.y"
            { jv_free(((*yyvaluep).literal)); }
#line 2172 "src/parser.c"
        break;

    case YYSYMBOL_DictPairs: /* DictPairs  */
#line 38 "src/parser.y"
            { block_free(((*yyvaluep).blk)); }
#line 2178 "src/parser.c"
        break;

    case YYSYMBOL_DictPair: /* DictPair  */
#line 38 "src/parser.y"
            { block_free(((*yyvaluep).blk)); }
#line 2184 "src/parser.c"
        break;

    case YYSYMBOL_DictExpr: /* DictExpr  */
#line 38 "src/parser.y"
            { block_free(((*yyvaluep).blk)); }
#line 2190 "src/parser.c"
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
/* Lookahead token kind.  */
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
    int yynerrs = 0;

    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

    /* The location stack: array, bottom, top.  */
    YYLTYPE yylsa[YYINITDEPTH];
    YYLTYPE *yyls = yylsa;
    YYLTYPE *yylsp = yyls;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
  YYLTYPE yyloc;

  /* The locations where the error started and ended.  */
  YYLTYPE yyerror_range[3];

  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYPTRDIFF_T yymsg_alloc = sizeof yymsgbuf;

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N), yylsp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  yylsp[0] = yylloc;
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
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
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
        YYLTYPE *yyls1 = yyls;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yyls1, yysize * YYSIZEOF (*yylsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
        yyls = yyls1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
        YYSTACK_RELOCATE (yyls_alloc, yyls);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
      yylsp = yyls + yysize - 1;

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

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex (&yylval, &yylloc, answer, errors, locations, lexer_param_ptr);
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      yyerror_range[1] = yylloc;
      goto yyerrlab1;
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
  *++yylsp = yylloc;

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

  /* Default location. */
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
  yyerror_range[1] = yyloc;
  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 2: /* TopLevel: Module Imports Query  */
#line 278 "src/parser.y"
                     {
  *answer = BLOCK((yyvsp[-2].blk), (yyvsp[-1].blk), gen_op_simple(TOP), (yyvsp[0].blk));
}
#line 2498 "src/parser.c"
    break;

  case 3: /* TopLevel: Module Imports FuncDefs  */
#line 281 "src/parser.y"
                        {
  *answer = BLOCK((yyvsp[-2].blk), (yyvsp[-1].blk), (yyvsp[0].blk));
}
#line 2506 "src/parser.c"
    break;

  case 4: /* Module: %empty  */
#line 286 "src/parser.y"
       {
  (yyval.blk) = gen_noop();
}
#line 2514 "src/parser.c"
    break;

  case 5: /* Module: "module" Query ';'  */
#line 289 "src/parser.y"
                   {
  if (!block_is_const((yyvsp[-1].blk))) {
    FAIL((yylsp[-1]), "Module metadata must be constant");
    (yyval.blk) = gen_noop();
    block_free((yyvsp[-1].blk));
  } else if (block_const_kind((yyvsp[-1].blk)) != JV_KIND_OBJECT) {
    FAIL((yylsp[-1]), "Module metadata must be an object");
    (yyval.blk) = gen_noop();
    block_free((yyvsp[-1].blk));
  } else {
    (yyval.blk) = gen_module((yyvsp[-1].blk));
  }
}
#line 2532 "src/parser.c"
    break;

  case 6: /* Imports: %empty  */
#line 304 "src/parser.y"
       {
  (yyval.blk) = gen_noop();
}
#line 2540 "src/parser.c"
    break;

  case 7: /* Imports: Import Imports  */
#line 307 "src/parser.y"
               {
  (yyval.blk) = BLOCK((yyvsp[-1].blk), (yyvsp[0].blk));
}
#line 2548 "src/parser.c"
    break;

  case 8: /* FuncDefs: %empty  */
#line 312 "src/parser.y"
       {
  (yyval.blk) = gen_noop();
}
#line 2556 "src/parser.c"
    break;

  case 9: /* FuncDefs: FuncDef FuncDefs  */
#line 315 "src/parser.y"
                 {
  (yyval.blk) = block_join((yyvsp[-1].blk), (yyvsp[0].blk));
}
#line 2564 "src/parser.c"
    break;

  case 10: /* Query: FuncDef Query  */
#line 321 "src/parser.y"
                            {
  (yyval.blk) = block_bind_referenced((yyvsp[-1].blk), (yyvsp[0].blk), OP_IS_CALL_PSEUDO);
}
#line 2572 "src/parser.c"
    break;

  case 11: /* Query: Expr "as" Patterns '|' Query  */
#line 324 "src/parser.y"
                             {
  (yyval.blk) = gen_destructure((yyvsp[-4].blk), (yyvsp[-2].blk), (yyvsp[0].blk));
}
#line 2580 "src/parser.c"
    break;

  case 12: /* Query: "label" BINDING '|' Query  */
#line 327 "src/parser.y"
                          {
  jv v = jv_string_fmt("*label-%s", jv_string_value((yyvsp[-2].literal)));
  (yyval.blk) = gen_location((yyloc), locations, gen_label(jv_string_value(v), (yyvsp[0].blk)));
  jv_free((yyvsp[-2].literal));
  jv_free(v);
}
#line 2591 "src/parser.c"
    break;

  case 13: /* Query: Query '|' Query  */
#line 333 "src/parser.y"
                {
  (yyval.blk) = block_join((yyvsp[-2].blk), (yyvsp[0].blk));
}
#line 2599 "src/parser.c"
    break;

  case 14: /* Query: Query ',' Query  */
#line 336 "src/parser.y"
                {
  (yyval.blk) = gen_both((yyvsp[-2].blk), (yyvsp[0].blk));
}
#line 2607 "src/parser.c"
    break;

  case 15: /* Query: Expr  */
#line 339 "src/parser.y"
     {
  (yyval.blk) = (yyvsp[0].blk);
}
#line 2615 "src/parser.c"
    break;

  case 16: /* Expr: Expr "//" Expr  */
#line 345 "src/parser.y"
               {
  (yyval.blk) = gen_definedor((yyvsp[-2].blk), (yyvsp[0].blk));
}
#line 2623 "src/parser.c"
    break;

  case 17: /* Expr: Expr '=' Expr  */
#line 348 "src/parser.y"
              {
  (yyval.blk) = gen_call("_assign", BLOCK(gen_lambda((yyvsp[-2].blk)), gen_lambda((yyvsp[0].blk))));
}
#line 2631 "src/parser.c"
    break;

  case 18: /* Expr: Expr "or" Expr  */
#line 351 "src/parser.y"
               {
  (yyval.blk) = gen_or((yyvsp[-2].blk), (yyvsp[0].blk));
}
#line 2639 "src/parser.c"
    break;

  case 19: /* Expr: Expr "and" Expr  */
#line 354 "src/parser.y"
                {
  (yyval.blk) = gen_and((yyvsp[-2].blk), (yyvsp[0].blk));
}
#line 2647 "src/parser.c"
    break;

  case 20: /* Expr: Expr "//=" Expr  */
#line 357 "src/parser.y"
                {
  (yyval.blk) = gen_definedor_assign((yyvsp[-2].blk), (yyvsp[0].blk));
}
#line 2655 "src/parser.c"
    break;

  case 21: /* Expr: Expr "|=" Expr  */
#line 360 "src/parser.y"
               {
  (yyval.blk) = gen_call("_modify", BLOCK(gen_lambda((yyvsp[-2].blk)), gen_lambda((yyvsp[0].blk))));
}
#line 2663 "src/parser.c"
    break;

  case 22: /* Expr: Expr '+' Expr  */
#line 363 "src/parser.y"
              {
  (yyval.blk) = gen_binop((yyvsp[-2].blk), (yyvsp[0].blk), '+');
}
#line 2671 "src/parser.c"
    break;

  case 23: /* Expr: Expr "+=" Expr  */
#line 366 "src/parser.y"
               {
  (yyval.blk) = gen_update((yyvsp[-2].blk), (yyvsp[0].blk), '+');
}
#line 2679 "src/parser.c"
    break;

  case 24: /* Expr: Expr '-' Expr  */
#line 369 "src/parser.y"
              {
  (yyval.blk) = gen_binop((yyvsp[-2].blk), (yyvsp[0].blk), '-');
}
#line 2687 "src/parser.c"
    break;

  case 25: /* Expr: Expr "-=" Expr  */
#line 372 "src/parser.y"
               {
  (yyval.blk) = gen_update((yyvsp[-2].blk), (yyvsp[0].blk), '-');
}
#line 2695 "src/parser.c"
    break;

  case 26: /* Expr: Expr '*' Expr  */
#line 375 "src/parser.y"
              {
  (yyval.blk) = gen_binop((yyvsp[-2].blk), (yyvsp[0].blk), '*');
}
#line 2703 "src/parser.c"
    break;

  case 27: /* Expr: Expr "*=" Expr  */
#line 378 "src/parser.y"
               {
  (yyval.blk) = gen_update((yyvsp[-2].blk), (yyvsp[0].blk), '*');
}
#line 2711 "src/parser.c"
    break;

  case 28: /* Expr: Expr '/' Expr  */
#line 381 "src/parser.y"
              {
  (yyval.blk) = gen_binop((yyvsp[-2].blk), (yyvsp[0].blk), '/');
}
#line 2719 "src/parser.c"
    break;

  case 29: /* Expr: Expr '%' Expr  */
#line 384 "src/parser.y"
              {
  (yyval.blk) = gen_binop((yyvsp[-2].blk), (yyvsp[0].blk), '%');
}
#line 2727 "src/parser.c"
    break;

  case 30: /* Expr: Expr "/=" Expr  */
#line 387 "src/parser.y"
               {
  (yyval.blk) = gen_update((yyvsp[-2].blk), (yyvsp[0].blk), '/');
}
#line 2735 "src/parser.c"
    break;

  case 31: /* Expr: Expr "%=" Expr  */
#line 390 "src/parser.y"
                 {
  (yyval.blk) = gen_update((yyvsp[-2].blk), (yyvsp[0].blk), '%');
}
#line 2743 "src/parser.c"
    break;

  case 32: /* Expr: Expr "==" Expr  */
#line 393 "src/parser.y"
               {
  (yyval.blk) = gen_binop((yyvsp[-2].blk), (yyvsp[0].blk), EQ);
}
#line 2751 "src/parser.c"
    break;

  case 33: /* Expr: Expr "!=" Expr  */
#line 396 "src/parser.y"
               {
  (yyval.blk) = gen_binop((yyvsp[-2].blk), (yyvsp[0].blk), NEQ);
}
#line 2759 "src/parser.c"
    break;

  case 34: /* Expr: Expr '<' Expr  */
#line 399 "src/parser.y"
              {
  (yyval.blk) = gen_binop((yyvsp[-2].blk), (yyvsp[0].blk), '<');
}
#line 2767 "src/parser.c"
    break;

  case 35: /* Expr: Expr '>' Expr  */
#line 402 "src/parser.y"
              {
  (yyval.blk) = gen_binop((yyvsp[-2].blk), (yyvsp[0].blk), '>');
}
#line 2775 "src/parser.c"
    break;

  case 36: /* Expr: Expr "<=" Expr  */
#line 405 "src/parser.y"
               {
  (yyval.blk) = gen_binop((yyvsp[-2].blk), (yyvsp[0].blk), LESSEQ);
}
#line 2783 "src/parser.c"
    break;

  case 37: /* Expr: Expr ">=" Expr  */
#line 408 "src/parser.y"
               {
  (yyval.blk) = gen_binop((yyvsp[-2].blk), (yyvsp[0].blk), GREATEREQ);
}
#line 2791 "src/parser.c"
    break;

  case 38: /* Expr: Term  */
#line 411 "src/parser.y"
                  {
  (yyval.blk) = (yyvsp[0].blk);
}
#line 2799 "src/parser.c"
    break;

  case 39: /* Import: ImportWhat ';'  */
#line 417 "src/parser.y"
               {
  (yyval.blk) = (yyvsp[-1].blk);
}
#line 2807 "src/parser.c"
    break;

  case 40: /* Import: ImportWhat Query ';'  */
#line 420 "src/parser.y"
                     {
  if (!block_is_const((yyvsp[-1].blk))) {
    FAIL((yylsp[-1]), "Module metadata must be constant");
    (yyval.blk) = gen_noop();
    block_free((yyvsp[-2].blk));
    block_free((yyvsp[-1].blk));
  } else if (block_const_kind((yyvsp[-1].blk)) != JV_KIND_OBJECT) {
    FAIL((yylsp[-1]), "Module metadata must be an object");
    (yyval.blk) = gen_noop();
    block_free((yyvsp[-2].blk));
    block_free((yyvsp[-1].blk));
  } else {
    (yyval.blk) = gen_import_meta((yyvsp[-2].blk), (yyvsp[-1].blk));
  }
}
#line 2827 "src/parser.c"
    break;

  case 41: /* ImportWhat: "import" ImportFrom "as" BINDING  */
#line 437 "src/parser.y"
                                 {
  jv v = block_const((yyvsp[-2].blk));
  // XXX Make gen_import take only blocks and the int is_data so we
  // don't have to free so much stuff here
  (yyval.blk) = gen_import(jv_string_value(v), jv_string_value((yyvsp[0].literal)), 1);
  block_free((yyvsp[-2].blk));
  jv_free((yyvsp[0].literal));
  jv_free(v);
}
#line 2841 "src/parser.c"
    break;

  case 42: /* ImportWhat: "import" ImportFrom "as" IDENT  */
#line 446 "src/parser.y"
                               {
  jv v = block_const((yyvsp[-2].blk));
  (yyval.blk) = gen_import(jv_string_value(v), jv_string_value((yyvsp[0].literal)), 0);
  block_free((yyvsp[-2].blk));
  jv_free((yyvsp[0].literal));
  jv_free(v);
}
#line 2853 "src/parser.c"
    break;

  case 43: /* ImportWhat: "include" ImportFrom  */
#line 453 "src/parser.y"
                     {
  jv v = block_const((yyvsp[0].blk));
  (yyval.blk) = gen_import(jv_string_value(v), NULL, 0);
  block_free((yyvsp[0].blk));
  jv_free(v);
}
#line 2864 "src/parser.c"
    break;

  case 44: /* ImportFrom: String  */
#line 461 "src/parser.y"
       {
  if (!block_is_const((yyvsp[0].blk))) {
    FAIL((yylsp[0]), "Import path must be constant");
    (yyval.blk) = gen_const(jv_string(""));
    block_free((yyvsp[0].blk));
  } else {
    (yyval.blk) = (yyvsp[0].blk);
  }
}
#line 2878 "src/parser.c"
    break;

  case 45: /* FuncDef: "def" IDENT ':' Query ';'  */
#line 472 "src/parser.y"
                          {
  (yyval.blk) = gen_function(jv_string_value((yyvsp[-3].literal)), gen_noop(), (yyvsp[-1].blk));
  jv_free((yyvsp[-3].literal));
}
#line 2887 "src/parser.c"
    break;

  case 46: /* FuncDef: "def" IDENT '(' Params ')' ':' Query ';'  */
#line 476 "src/parser.y"
                                         {
  (yyval.blk) = gen_function(jv_string_value((yyvsp[-6].literal)), (yyvsp[-4].blk), (yyvsp[-1].blk));
  jv_free((yyvsp[-6].literal));
}
#line 2896 "src/parser.c"
    break;

  case 47: /* FuncDef: "def" FORMAT ':' Query ';'  */
#line 480 "src/parser.y"
                           {
  (yyval.blk) = gen_function(jv_string_value((yyvsp[-3].literal)), gen_noop(), (yyvsp[-1].blk));
  jv_free((yyvsp[-3].literal));
}
#line 2905 "src/parser.c"
    break;

  case 48: /* Params: Param  */
#line 486 "src/parser.y"
      {
  (yyval.blk) = (yyvsp[0].blk);
}
#line 2913 "src/parser.c"
    break;

  case 49: /* Params: Params ';' Param  */
#line 489 "src/parser.y"
                 {
  (yyval.blk) = BLOCK((yyvsp[-2].blk), (yyvsp[0].blk));
}
#line 2921 "src/parser.c"
    break;

  case 50: /* Param: BINDING  */
#line 494 "src/parser.y"
        {
  (yyval.blk) = gen_param_regular(jv_string_value((yyvsp[0].literal)));
  jv_free((yyvsp[0].literal));
}
#line 2930 "src/parser.c"
    break;

  case 51: /* Param: IDENT  */
#line 498 "src/parser.y"
      {
  (yyval.blk) = gen_param(jv_string_value((yyvsp[0].literal)));
  jv_free((yyvsp[0].literal));
}
#line 2939 "src/parser.c"
    break;

  case 52: /* StringStart: FORMAT QQSTRING_START  */
#line 505 "src/parser.y"
                      {
  (yyval.literal) = (yyvsp[-1].literal);
}
#line 2947 "src/parser.c"
    break;

  case 53: /* StringStart: QQSTRING_START  */
#line 508 "src/parser.y"
               {
  (yyval.literal) = jv_string("@text");
}
#line 2955 "src/parser.c"
    break;

  case 54: /* String: StringStart QQString QQSTRING_END  */
#line 514 "src/parser.y"
                                  {
  (yyval.blk) = (yyvsp[-1].blk);
  jv_free((yyvsp[-2].literal));
}
#line 2964 "src/parser.c"
    break;

  case 55: /* QQString: %empty  */
#line 521 "src/parser.y"
       {
  (yyval.blk) = gen_const(jv_string(""));
}
#line 2972 "src/parser.c"
    break;

  case 56: /* QQString: QQString QQSTRING_TEXT  */
#line 524 "src/parser.y"
                       {
  (yyval.blk) = gen_binop((yyvsp[-1].blk), gen_const((yyvsp[0].literal)), '+');
}
#line 2980 "src/parser.c"
    break;

  case 57: /* QQString: QQString QQSTRING_INTERP_START Query QQSTRING_INTERP_END  */
#line 527 "src/parser.y"
                                                         {
  const char *s = jv_string_value((yyvsp[-4].literal));
  (yyval.blk) = gen_binop((yyvsp[-3].blk), BLOCK((yyvsp[-1].blk), gen_location((yylsp[-1]), locations, gen_call(s, gen_noop()))), '+');
}
#line 2989 "src/parser.c"
    break;

  case 58: /* ElseBody: "elif" Query "then" Query ElseBody  */
#line 534 "src/parser.y"
                                   {
  (yyval.blk) = gen_cond((yyvsp[-3].blk), (yyvsp[-1].blk), (yyvsp[0].blk));
}
#line 2997 "src/parser.c"
    break;

  case 59: /* ElseBody: "else" Query "end"  */
#line 537 "src/parser.y"
                   {
  (yyval.blk) = (yyvsp[-1].blk);
}
#line 3005 "src/parser.c"
    break;

  case 60: /* ElseBody: "end"  */
#line 540 "src/parser.y"
      {
  (yyval.blk) = gen_noop();
}
#line 3013 "src/parser.c"
    break;

  case 61: /* Term: '.'  */
#line 546 "src/parser.y"
    {
  (yyval.blk) = gen_noop();
}
#line 3021 "src/parser.c"
    break;

  case 62: /* Term: ".."  */
#line 549 "src/parser.y"
    {
  (yyval.blk) = gen_call("recurse", gen_noop());
}
#line 3029 "src/parser.c"
    break;

  case 63: /* Term: "break" BINDING  */
#line 552 "src/parser.y"
              {
  jv v = jv_string_fmt("*label-%s", jv_string_value((yyvsp[0].literal)));     // impossible symbol
  (yyval.blk) = gen_location((yyloc), locations,
                    BLOCK(gen_op_unbound(LOADV, jv_string_value(v)),
                    gen_call("error", gen_noop())));
  jv_free(v);
  jv_free((yyvsp[0].literal));
}
#line 3042 "src/parser.c"
    break;

  case 64: /* Term: "break" error  */
#line 560 "src/parser.y"
            {
  FAIL((yyloc), "break requires a label to break to");
  (yyval.blk) = gen_noop();
}
#line 3051 "src/parser.c"
    break;

  case 65: /* Term: Term FIELD '?'  */
#line 564 "src/parser.y"
               {
  (yyval.blk) = gen_index_opt((yyvsp[-2].blk), gen_const((yyvsp[-1].literal)));
}
#line 3059 "src/parser.c"
    break;

  case 66: /* Term: FIELD '?'  */
#line 567 "src/parser.y"
          {
  (yyval.blk) = gen_index_opt(gen_noop(), gen_const((yyvsp[-1].literal)));
}
#line 3067 "src/parser.c"
    break;

  case 67: /* Term: Term '.' String '?'  */
#line 570 "src/parser.y"
                    {
  (yyval.blk) = gen_index_opt((yyvsp[-3].blk), (yyvsp[-1].blk));
}
#line 3075 "src/parser.c"
    break;

  case 68: /* Term: '.' String '?'  */
#line 573 "src/parser.y"
               {
  (yyval.blk) = gen_index_opt(gen_noop(), (yyvsp[-1].blk));
}
#line 3083 "src/parser.c"
    break;

  case 69: /* Term: Term FIELD  */
#line 576 "src/parser.y"
                        {
  (yyval.blk) = gen_index((yyvsp[-1].blk), gen_const((yyvsp[0].literal)));
}
#line 3091 "src/parser.c"
    break;

  case 70: /* Term: FIELD  */
#line 579 "src/parser.y"
                   {
  (yyval.blk) = gen_index(gen_noop(), gen_const((yyvsp[0].literal)));
}
#line 3099 "src/parser.c"
    break;

  case 71: /* Term: Term '.' String  */
#line 582 "src/parser.y"
                             {
  (yyval.blk) = gen_index((yyvsp[-2].blk), (yyvsp[0].blk));
}
#line 3107 "src/parser.c"
    break;

  case 72: /* Term: '.' String  */
#line 585 "src/parser.y"
                        {
  (yyval.blk) = gen_index(gen_noop(), (yyvsp[0].blk));
}
#line 3115 "src/parser.c"
    break;

  case 73: /* Term: '.' error  */
#line 588 "src/parser.y"
          {
  FAIL((yyloc), "try .[\"field\"] instead of .field for unusually named fields");
  (yyval.blk) = gen_noop();
}
#line 3124 "src/parser.c"
    break;

  case 74: /* Term: '.' IDENT error  */
#line 592 "src/parser.y"
                {
  jv_free((yyvsp[-1].literal));
  FAIL((yyloc), "try .[\"field\"] instead of .field for unusually named fields");
  (yyval.blk) = gen_noop();
}
#line 3134 "src/parser.c"
    break;

  case 75: /* Term: Term '[' Query ']' '?'  */
#line 598 "src/parser.y"
                       {
  (yyval.blk) = gen_index_opt((yyvsp[-4].blk), (yyvsp[-2].blk));
}
#line 3142 "src/parser.c"
    break;

  case 76: /* Term: Term '[' Query ']'  */
#line 601 "src/parser.y"
                                {
  (yyval.blk) = gen_index((yyvsp[-3].blk), (yyvsp[-1].blk));
}
#line 3150 "src/parser.c"
    break;

  case 77: /* Term: Term '.' '[' Query ']' '?'  */
#line 604 "src/parser.y"
                           {
  (yyval.blk) = gen_index_opt((yyvsp[-5].blk), (yyvsp[-2].blk));
}
#line 3158 "src/parser.c"
    break;

  case 78: /* Term: Term '.' '[' Query ']'  */
#line 607 "src/parser.y"
                                    {
  (yyval.blk) = gen_index((yyvsp[-4].blk), (yyvsp[-1].blk));
}
#line 3166 "src/parser.c"
    break;

  case 79: /* Term: Term '[' ']' '?'  */
#line 610 "src/parser.y"
                 {
  (yyval.blk) = block_join((yyvsp[-3].blk), gen_op_simple(EACH_OPT));
}
#line 3174 "src/parser.c"
    break;

  case 80: /* Term: Term '[' ']'  */
#line 613 "src/parser.y"
                          {
  (yyval.blk) = block_join((yyvsp[-2].blk), gen_op_simple(EACH));
}
#line 3182 "src/parser.c"
    break;

  case 81: /* Term: Term '.' '[' ']' '?'  */
#line 616 "src/parser.y"
                     {
  (yyval.blk) = block_join((yyvsp[-4].blk), gen_op_simple(EACH_OPT));
}
#line 3190 "src/parser.c"
    break;

  case 82: /* Term: Term '.' '[' ']'  */
#line 619 "src/parser.y"
                              {
  (yyval.blk) = block_join((yyvsp[-3].blk), gen_op_simple(EACH));
}
#line 3198 "src/parser.c"
    break;

  case 83: /* Term: Term '[' Query ':' Query ']' '?'  */
#line 622 "src/parser.y"
                                 {
  (yyval.blk) = gen_slice_index((yyvsp[-6].blk), (yyvsp[-4].blk), (yyvsp[-2].blk), INDEX_OPT);
}
#line 3206 "src/parser.c"
    break;

  case 84: /* Term: Term '[' Query ':' ']' '?'  */
#line 625 "src/parser.y"
                           {
  (yyval.blk) = gen_slice_index((yyvsp[-5].blk), (yyvsp[-3].blk), gen_const(jv_null()), INDEX_OPT);
}
#line 3214 "src/parser.c"
    break;

  case 85: /* Term: Term '[' ':' Query ']' '?'  */
#line 628 "src/parser.y"
                           {
  (yyval.blk) = gen_slice_index((yyvsp[-5].blk), gen_const(jv_null()), (yyvsp[-2].blk), INDEX_OPT);
}
#line 3222 "src/parser.c"
    break;

  case 86: /* Term: Term '[' Query ':' Query ']'  */
#line 631 "src/parser.y"
                                          {
  (yyval.blk) = gen_slice_index((yyvsp[-5].blk), (yyvsp[-3].blk), (yyvsp[-1].blk), INDEX);
}
#line 3230 "src/parser.c"
    break;

  case 87: /* Term: Term '[' Query ':' ']'  */
#line 634 "src/parser.y"
                                    {
  (yyval.blk) = gen_slice_index((yyvsp[-4].blk), (yyvsp[-2].blk), gen_const(jv_null()), INDEX);
}
#line 3238 "src/parser.c"
    break;

  case 88: /* Term: Term '[' ':' Query ']'  */
#line 637 "src/parser.y"
                                    {
  (yyval.blk) = gen_slice_index((yyvsp[-4].blk), gen_const(jv_null()), (yyvsp[-1].blk), INDEX);
}
#line 3246 "src/parser.c"
    break;

  case 89: /* Term: Term '?'  */
#line 640 "src/parser.y"
         {
  (yyval.blk) = gen_try((yyvsp[-1].blk), gen_op_simple(BACKTRACK));
}
#line 3254 "src/parser.c"
    break;

  case 90: /* Term: LITERAL  */
#line 643 "src/parser.y"
        {
  (yyval.blk) = gen_const((yyvsp[0].literal));
}
#line 3262 "src/parser.c"
    break;

  case 91: /* Term: String  */
#line 646 "src/parser.y"
       {
  (yyval.blk) = (yyvsp[0].blk);
}
#line 3270 "src/parser.c"
    break;

  case 92: /* Term: FORMAT  */
#line 649 "src/parser.y"
       {
  const char *s = jv_string_value((yyvsp[0].literal));
  (yyval.blk) = gen_location((yyloc), locations, gen_call(s, gen_noop()));
  jv_free((yyvsp[0].literal));
}
#line 3280 "src/parser.c"
    break;

  case 93: /* Term: '-' Term  */
#line 654 "src/parser.y"
         {
  (yyval.blk) = BLOCK((yyvsp[0].blk), gen_call("_negate", gen_noop()));
}
#line 3288 "src/parser.c"
    break;

  case 94: /* Term: '(' Query ')'  */
#line 657 "src/parser.y"
              {
  (yyval.blk) = (yyvsp[-1].blk);
}
#line 3296 "src/parser.c"
    break;

  case 95: /* Term: '[' Query ']'  */
#line 660 "src/parser.y"
              {
  (yyval.blk) = gen_collect((yyvsp[-1].blk));
}
#line 3304 "src/parser.c"
    break;

  case 96: /* Term: '[' ']'  */
#line 663 "src/parser.y"
        {
  (yyval.blk) = gen_const(jv_array());
}
#line 3312 "src/parser.c"
    break;

  case 97: /* Term: '{' DictPairs '}'  */
#line 666 "src/parser.y"
                  {
  block o = gen_const_object((yyvsp[-1].blk));
  if (o.first != NULL)
    (yyval.blk) = o;
  else
    (yyval.blk) = BLOCK(gen_subexp(gen_const(jv_object())), (yyvsp[-1].blk), gen_op_simple(POP));
}
#line 3324 "src/parser.c"
    break;

  case 98: /* Term: "reduce" Expr "as" Patterns '(' Query ';' Query ')'  */
#line 673 "src/parser.y"
                                                    {
  (yyval.blk) = gen_reduce((yyvsp[-7].blk), (yyvsp[-5].blk), (yyvsp[-3].blk), (yyvsp[-1].blk));
}
#line 3332 "src/parser.c"
    break;

  case 99: /* Term: "foreach" Expr "as" Patterns '(' Query ';' Query ';' Query ')'  */
#line 676 "src/parser.y"
                                                               {
  (yyval.blk) = gen_foreach((yyvsp[-9].blk), (yyvsp[-7].blk), (yyvsp[-5].blk), (yyvsp[-3].blk), (yyvsp[-1].blk));
}
#line 3340 "src/parser.c"
    break;

  case 100: /* Term: "foreach" Expr "as" Patterns '(' Query ';' Query ')'  */
#line 679 "src/parser.y"
                                                     {
  (yyval.blk) = gen_foreach((yyvsp[-7].blk), (yyvsp[-5].blk), (yyvsp[-3].blk), (yyvsp[-1].blk), gen_noop());
}
#line 3348 "src/parser.c"
    break;

  case 101: /* Term: "if" Query "then" Query ElseBody  */
#line 682 "src/parser.y"
                                 {
  (yyval.blk) = gen_cond((yyvsp[-3].blk), (yyvsp[-1].blk), (yyvsp[0].blk));
}
#line 3356 "src/parser.c"
    break;

  case 102: /* Term: "if" Query "then" error  */
#line 685 "src/parser.y"
                        {
  FAIL((yyloc), "Possibly unterminated 'if' statement");
  (yyval.blk) = (yyvsp[-2].blk);
}
#line 3365 "src/parser.c"
    break;

  case 103: /* Term: "try" Expr "catch" Expr  */
#line 689 "src/parser.y"
                        {
  (yyval.blk) = gen_try((yyvsp[-2].blk), (yyvsp[0].blk));
}
#line 3373 "src/parser.c"
    break;

  case 104: /* Term: "try" Expr "catch" error  */
#line 692 "src/parser.y"
                         {
  FAIL((yyloc), "Possibly unterminated 'try' statement");
  (yyval.blk) = (yyvsp[-2].blk);
}
#line 3382 "src/parser.c"
    break;

  case 105: /* Term: "try" Expr  */
#line 696 "src/parser.y"
           {
  (yyval.blk) = gen_try((yyvsp[0].blk), gen_op_simple(BACKTRACK));
}
#line 3390 "src/parser.c"
    break;

  case 106: /* Term: '$' '$' '$' BINDING  */
#line 714 "src/parser.y"
                    {
  (yyval.blk) = gen_location((yyloc), locations, gen_op_unbound(LOADVN, jv_string_value((yyvsp[0].literal))));
  jv_free((yyvsp[0].literal));
}
#line 3399 "src/parser.c"
    break;

  case 107: /* Term: BINDING  */
#line 718 "src/parser.y"
        {
  (yyval.blk) = gen_location((yyloc), locations, gen_op_unbound(LOADV, jv_string_value((yyvsp[0].literal))));
  jv_free((yyvsp[0].literal));
}
#line 3408 "src/parser.c"
    break;

  case 108: /* Term: "$__loc__"  */
#line 722 "src/parser.y"
           {
  (yyval.blk) = gen_loc_object(&(yyloc), locations);
}
#line 3416 "src/parser.c"
    break;

  case 109: /* Term: IDENT  */
#line 725 "src/parser.y"
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
#line 3433 "src/parser.c"
    break;

  case 110: /* Term: IDENT '(' Args ')'  */
#line 737 "src/parser.y"
                   {
  (yyval.blk) = gen_call(jv_string_value((yyvsp[-3].literal)), (yyvsp[-1].blk));
  (yyval.blk) = gen_location((yylsp[-3]), locations, (yyval.blk));
  jv_free((yyvsp[-3].literal));
}
#line 3443 "src/parser.c"
    break;

  case 111: /* Term: '(' error ')'  */
#line 742 "src/parser.y"
              { (yyval.blk) = gen_noop(); }
#line 3449 "src/parser.c"
    break;

  case 112: /* Term: '[' error ']'  */
#line 743 "src/parser.y"
              { (yyval.blk) = gen_noop(); }
#line 3455 "src/parser.c"
    break;

  case 113: /* Term: Term '[' error ']'  */
#line 744 "src/parser.y"
                   { (yyval.blk) = (yyvsp[-3].blk); }
#line 3461 "src/parser.c"
    break;

  case 114: /* Term: '{' error '}'  */
#line 745 "src/parser.y"
              { (yyval.blk) = gen_noop(); }
#line 3467 "src/parser.c"
    break;

  case 115: /* Args: Arg  */
#line 748 "src/parser.y"
    {
  (yyval.blk) = (yyvsp[0].blk);
}
#line 3475 "src/parser.c"
    break;

  case 116: /* Args: Args ';' Arg  */
#line 751 "src/parser.y"
             {
  (yyval.blk) = BLOCK((yyvsp[-2].blk), (yyvsp[0].blk));
}
#line 3483 "src/parser.c"
    break;

  case 117: /* Arg: Query  */
#line 756 "src/parser.y"
      {
  (yyval.blk) = gen_lambda((yyvsp[0].blk));
}
#line 3491 "src/parser.c"
    break;

  case 118: /* RepPatterns: RepPatterns "?//" Pattern  */
#line 761 "src/parser.y"
                          {
  (yyval.blk) = BLOCK((yyvsp[-2].blk), gen_destructure_alt((yyvsp[0].blk)));
}
#line 3499 "src/parser.c"
    break;

  case 119: /* RepPatterns: Pattern  */
#line 764 "src/parser.y"
        {
  (yyval.blk) = gen_destructure_alt((yyvsp[0].blk));
}
#line 3507 "src/parser.c"
    break;

  case 120: /* Patterns: RepPatterns "?//" Pattern  */
#line 769 "src/parser.y"
                          {
  (yyval.blk) = BLOCK((yyvsp[-2].blk), (yyvsp[0].blk));
}
#line 3515 "src/parser.c"
    break;

  case 121: /* Patterns: Pattern  */
#line 772 "src/parser.y"
        {
  (yyval.blk) = (yyvsp[0].blk);
}
#line 3523 "src/parser.c"
    break;

  case 122: /* Pattern: BINDING  */
#line 777 "src/parser.y"
        {
  (yyval.blk) = gen_op_unbound(STOREV, jv_string_value((yyvsp[0].literal)));
  jv_free((yyvsp[0].literal));
}
#line 3532 "src/parser.c"
    break;

  case 123: /* Pattern: '[' ArrayPats ']'  */
#line 781 "src/parser.y"
                  {
  (yyval.blk) = BLOCK((yyvsp[-1].blk), gen_op_simple(POP));
}
#line 3540 "src/parser.c"
    break;

  case 124: /* Pattern: '{' ObjPats '}'  */
#line 784 "src/parser.y"
                {
  (yyval.blk) = BLOCK((yyvsp[-1].blk), gen_op_simple(POP));
}
#line 3548 "src/parser.c"
    break;

  case 125: /* ArrayPats: Pattern  */
#line 789 "src/parser.y"
        {
  (yyval.blk) = gen_array_matcher(gen_noop(), (yyvsp[0].blk));
}
#line 3556 "src/parser.c"
    break;

  case 126: /* ArrayPats: ArrayPats ',' Pattern  */
#line 792 "src/parser.y"
                      {
  (yyval.blk) = gen_array_matcher((yyvsp[-2].blk), (yyvsp[0].blk));
}
#line 3564 "src/parser.c"
    break;

  case 127: /* ObjPats: ObjPat  */
#line 797 "src/parser.y"
       {
  (yyval.blk) = (yyvsp[0].blk);
}
#line 3572 "src/parser.c"
    break;

  case 128: /* ObjPats: ObjPats ',' ObjPat  */
#line 800 "src/parser.y"
                   {
  (yyval.blk) = BLOCK((yyvsp[-2].blk), (yyvsp[0].blk));
}
#line 3580 "src/parser.c"
    break;

  case 129: /* ObjPat: BINDING  */
#line 805 "src/parser.y"
        {
  (yyval.blk) = gen_object_matcher(gen_const((yyvsp[0].literal)), gen_op_unbound(STOREV, jv_string_value((yyvsp[0].literal))));
}
#line 3588 "src/parser.c"
    break;

  case 130: /* ObjPat: BINDING ':' Pattern  */
#line 808 "src/parser.y"
                    {
  (yyval.blk) = gen_object_matcher(gen_const((yyvsp[-2].literal)), BLOCK(gen_op_simple(DUP), gen_op_unbound(STOREV, jv_string_value((yyvsp[-2].literal))), (yyvsp[0].blk)));
}
#line 3596 "src/parser.c"
    break;

  case 131: /* ObjPat: IDENT ':' Pattern  */
#line 811 "src/parser.y"
                  {
  (yyval.blk) = gen_object_matcher(gen_const((yyvsp[-2].literal)), (yyvsp[0].blk));
}
#line 3604 "src/parser.c"
    break;

  case 132: /* ObjPat: Keyword ':' Pattern  */
#line 814 "src/parser.y"
                    {
  (yyval.blk) = gen_object_matcher(gen_const((yyvsp[-2].literal)), (yyvsp[0].blk));
}
#line 3612 "src/parser.c"
    break;

  case 133: /* ObjPat: String ':' Pattern  */
#line 817 "src/parser.y"
                   {
  (yyval.blk) = gen_object_matcher((yyvsp[-2].blk), (yyvsp[0].blk));
}
#line 3620 "src/parser.c"
    break;

  case 134: /* ObjPat: '(' Query ')' ':' Pattern  */
#line 820 "src/parser.y"
                          {
  jv msg = check_object_key((yyvsp[-3].blk));
  if (jv_is_valid(msg)) {
    FAIL((yylsp[-3]), jv_string_value(msg));
  }
  jv_free(msg);
  (yyval.blk) = gen_object_matcher((yyvsp[-3].blk), (yyvsp[0].blk));
}
#line 3633 "src/parser.c"
    break;

  case 135: /* ObjPat: error ':' Pattern  */
#line 828 "src/parser.y"
                  {
  FAIL((yyloc), "May need parentheses around object key expression");
  (yyval.blk) = (yyvsp[0].blk);
}
#line 3642 "src/parser.c"
    break;

  case 136: /* Keyword: "as"  */
#line 834 "src/parser.y"
     {
  (yyval.literal) = jv_string("as");
}
#line 3650 "src/parser.c"
    break;

  case 137: /* Keyword: "def"  */
#line 837 "src/parser.y"
      {
  (yyval.literal) = jv_string("def");
}
#line 3658 "src/parser.c"
    break;

  case 138: /* Keyword: "module"  */
#line 840 "src/parser.y"
         {
  (yyval.literal) = jv_string("module");
}
#line 3666 "src/parser.c"
    break;

  case 139: /* Keyword: "import"  */
#line 843 "src/parser.y"
         {
  (yyval.literal) = jv_string("import");
}
#line 3674 "src/parser.c"
    break;

  case 140: /* Keyword: "include"  */
#line 846 "src/parser.y"
          {
  (yyval.literal) = jv_string("include");
}
#line 3682 "src/parser.c"
    break;

  case 141: /* Keyword: "if"  */
#line 849 "src/parser.y"
     {
  (yyval.literal) = jv_string("if");
}
#line 3690 "src/parser.c"
    break;

  case 142: /* Keyword: "then"  */
#line 852 "src/parser.y"
       {
  (yyval.literal) = jv_string("then");
}
#line 3698 "src/parser.c"
    break;

  case 143: /* Keyword: "else"  */
#line 855 "src/parser.y"
       {
  (yyval.literal) = jv_string("else");
}
#line 3706 "src/parser.c"
    break;

  case 144: /* Keyword: "elif"  */
#line 858 "src/parser.y"
       {
  (yyval.literal) = jv_string("elif");
}
#line 3714 "src/parser.c"
    break;

  case 145: /* Keyword: "reduce"  */
#line 861 "src/parser.y"
         {
  (yyval.literal) = jv_string("reduce");
}
#line 3722 "src/parser.c"
    break;

  case 146: /* Keyword: "foreach"  */
#line 864 "src/parser.y"
          {
  (yyval.literal) = jv_string("foreach");
}
#line 3730 "src/parser.c"
    break;

  case 147: /* Keyword: "end"  */
#line 867 "src/parser.y"
      {
  (yyval.literal) = jv_string("end");
}
#line 3738 "src/parser.c"
    break;

  case 148: /* Keyword: "and"  */
#line 870 "src/parser.y"
      {
  (yyval.literal) = jv_string("and");
}
#line 3746 "src/parser.c"
    break;

  case 149: /* Keyword: "or"  */
#line 873 "src/parser.y"
     {
  (yyval.literal) = jv_string("or");
}
#line 3754 "src/parser.c"
    break;

  case 150: /* Keyword: "try"  */
#line 876 "src/parser.y"
      {
  (yyval.literal) = jv_string("try");
}
#line 3762 "src/parser.c"
    break;

  case 151: /* Keyword: "catch"  */
#line 879 "src/parser.y"
        {
  (yyval.literal) = jv_string("catch");
}
#line 3770 "src/parser.c"
    break;

  case 152: /* Keyword: "label"  */
#line 882 "src/parser.y"
        {
  (yyval.literal) = jv_string("label");
}
#line 3778 "src/parser.c"
    break;

  case 153: /* Keyword: "break"  */
#line 885 "src/parser.y"
        {
  (yyval.literal) = jv_string("break");
}
#line 3786 "src/parser.c"
    break;

  case 154: /* DictPairs: %empty  */
#line 891 "src/parser.y"
       {
  (yyval.blk) = gen_noop();
}
#line 3794 "src/parser.c"
    break;

  case 155: /* DictPairs: DictPair  */
#line 894 "src/parser.y"
         {
  (yyval.blk) = (yyvsp[0].blk);
}
#line 3802 "src/parser.c"
    break;

  case 156: /* DictPairs: DictPair ',' DictPairs  */
#line 897 "src/parser.y"
                       {
  (yyval.blk) = block_join((yyvsp[-2].blk), (yyvsp[0].blk));
}
#line 3810 "src/parser.c"
    break;

  case 157: /* DictPair: IDENT ':' DictExpr  */
#line 902 "src/parser.y"
                   {
  (yyval.blk) = gen_dictpair(gen_const((yyvsp[-2].literal)), (yyvsp[0].blk));
}
#line 3818 "src/parser.c"
    break;

  case 158: /* DictPair: Keyword ':' DictExpr  */
#line 905 "src/parser.y"
                     {
  (yyval.blk) = gen_dictpair(gen_const((yyvsp[-2].literal)), (yyvsp[0].blk));
}
#line 3826 "src/parser.c"
    break;

  case 159: /* DictPair: String ':' DictExpr  */
#line 908 "src/parser.y"
                    {
  (yyval.blk) = gen_dictpair((yyvsp[-2].blk), (yyvsp[0].blk));
}
#line 3834 "src/parser.c"
    break;

  case 160: /* DictPair: String  */
#line 911 "src/parser.y"
       {
  (yyval.blk) = gen_dictpair((yyvsp[0].blk), BLOCK(gen_op_simple(POP), gen_op_simple(DUP2),
                              gen_op_simple(DUP2), gen_op_simple(INDEX)));
}
#line 3843 "src/parser.c"
    break;

  case 161: /* DictPair: BINDING ':' DictExpr  */
#line 915 "src/parser.y"
                     {
  (yyval.blk) = gen_dictpair(gen_location((yyloc), locations, gen_op_unbound(LOADV, jv_string_value((yyvsp[-2].literal)))),
                    (yyvsp[0].blk));
  jv_free((yyvsp[-2].literal));
}
#line 3853 "src/parser.c"
    break;

  case 162: /* DictPair: BINDING  */
#line 920 "src/parser.y"
        {
  (yyval.blk) = gen_dictpair(gen_const((yyvsp[0].literal)),
                    gen_location((yyloc), locations, gen_op_unbound(LOADV, jv_string_value((yyvsp[0].literal)))));
}
#line 3862 "src/parser.c"
    break;

  case 163: /* DictPair: IDENT  */
#line 924 "src/parser.y"
      {
  (yyval.blk) = gen_dictpair(gen_const(jv_copy((yyvsp[0].literal))),
                    gen_index(gen_noop(), gen_const((yyvsp[0].literal))));
}
#line 3871 "src/parser.c"
    break;

  case 164: /* DictPair: "$__loc__"  */
#line 928 "src/parser.y"
           {
  (yyval.blk) = gen_dictpair(gen_const(jv_string("__loc__")),
                    gen_loc_object(&(yyloc), locations));
}
#line 3880 "src/parser.c"
    break;

  case 165: /* DictPair: Keyword  */
#line 932 "src/parser.y"
        {
  (yyval.blk) = gen_dictpair(gen_const(jv_copy((yyvsp[0].literal))),
                    gen_index(gen_noop(), gen_const((yyvsp[0].literal))));
}
#line 3889 "src/parser.c"
    break;

  case 166: /* DictPair: '(' Query ')' ':' DictExpr  */
#line 936 "src/parser.y"
                           {
  jv msg = check_object_key((yyvsp[-3].blk));
  if (jv_is_valid(msg)) {
    FAIL((yylsp[-3]), jv_string_value(msg));
  }
  jv_free(msg);
  (yyval.blk) = gen_dictpair((yyvsp[-3].blk), (yyvsp[0].blk));
}
#line 3902 "src/parser.c"
    break;

  case 167: /* DictPair: error ':' DictExpr  */
#line 944 "src/parser.y"
                   {
  FAIL((yylsp[-2]), "May need parentheses around object key expression");
  (yyval.blk) = (yyvsp[0].blk);
}
#line 3911 "src/parser.c"
    break;

  case 168: /* DictExpr: DictExpr '|' DictExpr  */
#line 950 "src/parser.y"
                      {
  (yyval.blk) = block_join((yyvsp[-2].blk), (yyvsp[0].blk));
}
#line 3919 "src/parser.c"
    break;

  case 169: /* DictExpr: Expr  */
#line 953 "src/parser.y"
     {
  (yyval.blk) = (yyvsp[0].blk);
}
#line 3927 "src/parser.c"
    break;


#line 3931 "src/parser.c"

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
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;
  *++yylsp = yyloc;

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
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      {
        yypcontext_t yyctx
          = {yyssp, yytoken, &yylloc};
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = yysyntax_error (&yymsg_alloc, &yymsg, &yyctx);
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == -1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = YY_CAST (char *,
                             YYSTACK_ALLOC (YY_CAST (YYSIZE_T, yymsg_alloc)));
            if (yymsg)
              {
                yysyntax_error_status
                  = yysyntax_error (&yymsg_alloc, &yymsg, &yyctx);
                yymsgp = yymsg;
              }
            else
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = YYENOMEM;
              }
          }
        yyerror (&yylloc, answer, errors, locations, lexer_param_ptr, yymsgp);
        if (yysyntax_error_status == YYENOMEM)
          YYNOMEM;
      }
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
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

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

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
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
                  YY_ACCESSING_SYMBOL (yystate), yyvsp, yylsp, answer, errors, locations, lexer_param_ptr);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  yyerror_range[2] = yylloc;
  ++yylsp;
  YYLLOC_DEFAULT (*yylsp, yyerror_range, 2);

  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (&yylloc, answer, errors, locations, lexer_param_ptr, YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
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
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp, yylsp, answer, errors, locations, lexer_param_ptr);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
  return yyresult;
}

#line 956 "src/parser.y"


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

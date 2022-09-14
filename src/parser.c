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
#define YYMALLOC jv_mem_alloc
#define YYFREE jv_mem_free

#line 82 "src/parser.c"

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
#line 11 "src/parser.y"

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

#line 134 "src/parser.c"

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
    LITERAL = 261,                 /* LITERAL  */
    FORMAT = 262,                  /* FORMAT  */
    REC = 263,                     /* ".."  */
    SETMOD = 264,                  /* "%="  */
    EQ = 265,                      /* "=="  */
    NEQ = 266,                     /* "!="  */
    DEFINEDOR = 267,               /* "//"  */
    AS = 268,                      /* "as"  */
    DEF = 269,                     /* "def"  */
    MODULE = 270,                  /* "module"  */
    IMPORT = 271,                  /* "import"  */
    INCLUDE = 272,                 /* "include"  */
    IF = 273,                      /* "if"  */
    THEN = 274,                    /* "then"  */
    ELSE = 275,                    /* "else"  */
    ELSE_IF = 276,                 /* "elif"  */
    REDUCE = 277,                  /* "reduce"  */
    FOREACH = 278,                 /* "foreach"  */
    END = 279,                     /* "end"  */
    AND = 280,                     /* "and"  */
    OR = 281,                      /* "or"  */
    TRY = 282,                     /* "try"  */
    CATCH = 283,                   /* "catch"  */
    LABEL = 284,                   /* "label"  */
    BREAK = 285,                   /* "break"  */
    LOC = 286,                     /* "__loc__"  */
    SETPIPE = 287,                 /* "|="  */
    SETPLUS = 288,                 /* "+="  */
    SETMINUS = 289,                /* "-="  */
    SETMULT = 290,                 /* "*="  */
    SETDIV = 291,                  /* "/="  */
    SETDEFINEDOR = 292,            /* "//="  */
    LESSEQ = 293,                  /* "<="  */
    GREATEREQ = 294,               /* ">="  */
    ALTERNATION = 295,             /* "?//"  */
    QQSTRING_START = 296,          /* QQSTRING_START  */
    QQSTRING_TEXT = 297,           /* QQSTRING_TEXT  */
    QQSTRING_INTERP_START = 298,   /* QQSTRING_INTERP_START  */
    QQSTRING_INTERP_END = 299,     /* QQSTRING_INTERP_END  */
    QQSTRING_END = 300,            /* QQSTRING_END  */
    FUNCDEF = 301,                 /* FUNCDEF  */
    NONOPT = 302                   /* NONOPT  */
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
#define INCLUDE 272
#define IF 273
#define THEN 274
#define ELSE 275
#define ELSE_IF 276
#define REDUCE 277
#define FOREACH 278
#define END 279
#define AND 280
#define OR 281
#define TRY 282
#define CATCH 283
#define LABEL 284
#define BREAK 285
#define LOC 286
#define SETPIPE 287
#define SETPLUS 288
#define SETMINUS 289
#define SETMULT 290
#define SETDIV 291
#define SETDEFINEDOR 292
#define LESSEQ 293
#define GREATEREQ 294
#define ALTERNATION 295
#define QQSTRING_START 296
#define QQSTRING_TEXT 297
#define QQSTRING_INTERP_START 298
#define QQSTRING_INTERP_END 299
#define QQSTRING_END 300
#define FUNCDEF 301
#define NONOPT 302

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 31 "src/parser.y"

  jv literal;
  block blk;

#line 253 "src/parser.c"

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
  YYSYMBOL_LITERAL = 6,                    /* LITERAL  */
  YYSYMBOL_FORMAT = 7,                     /* FORMAT  */
  YYSYMBOL_REC = 8,                        /* ".."  */
  YYSYMBOL_SETMOD = 9,                     /* "%="  */
  YYSYMBOL_EQ = 10,                        /* "=="  */
  YYSYMBOL_NEQ = 11,                       /* "!="  */
  YYSYMBOL_DEFINEDOR = 12,                 /* "//"  */
  YYSYMBOL_AS = 13,                        /* "as"  */
  YYSYMBOL_DEF = 14,                       /* "def"  */
  YYSYMBOL_MODULE = 15,                    /* "module"  */
  YYSYMBOL_IMPORT = 16,                    /* "import"  */
  YYSYMBOL_INCLUDE = 17,                   /* "include"  */
  YYSYMBOL_IF = 18,                        /* "if"  */
  YYSYMBOL_THEN = 19,                      /* "then"  */
  YYSYMBOL_ELSE = 20,                      /* "else"  */
  YYSYMBOL_ELSE_IF = 21,                   /* "elif"  */
  YYSYMBOL_REDUCE = 22,                    /* "reduce"  */
  YYSYMBOL_FOREACH = 23,                   /* "foreach"  */
  YYSYMBOL_END = 24,                       /* "end"  */
  YYSYMBOL_AND = 25,                       /* "and"  */
  YYSYMBOL_OR = 26,                        /* "or"  */
  YYSYMBOL_TRY = 27,                       /* "try"  */
  YYSYMBOL_CATCH = 28,                     /* "catch"  */
  YYSYMBOL_LABEL = 29,                     /* "label"  */
  YYSYMBOL_BREAK = 30,                     /* "break"  */
  YYSYMBOL_LOC = 31,                       /* "__loc__"  */
  YYSYMBOL_SETPIPE = 32,                   /* "|="  */
  YYSYMBOL_SETPLUS = 33,                   /* "+="  */
  YYSYMBOL_SETMINUS = 34,                  /* "-="  */
  YYSYMBOL_SETMULT = 35,                   /* "*="  */
  YYSYMBOL_SETDIV = 36,                    /* "/="  */
  YYSYMBOL_SETDEFINEDOR = 37,              /* "//="  */
  YYSYMBOL_LESSEQ = 38,                    /* "<="  */
  YYSYMBOL_GREATEREQ = 39,                 /* ">="  */
  YYSYMBOL_ALTERNATION = 40,               /* "?//"  */
  YYSYMBOL_QQSTRING_START = 41,            /* QQSTRING_START  */
  YYSYMBOL_QQSTRING_TEXT = 42,             /* QQSTRING_TEXT  */
  YYSYMBOL_QQSTRING_INTERP_START = 43,     /* QQSTRING_INTERP_START  */
  YYSYMBOL_QQSTRING_INTERP_END = 44,       /* QQSTRING_INTERP_END  */
  YYSYMBOL_QQSTRING_END = 45,              /* QQSTRING_END  */
  YYSYMBOL_FUNCDEF = 46,                   /* FUNCDEF  */
  YYSYMBOL_47_ = 47,                       /* '|'  */
  YYSYMBOL_48_ = 48,                       /* ','  */
  YYSYMBOL_49_ = 49,                       /* '='  */
  YYSYMBOL_50_ = 50,                       /* '<'  */
  YYSYMBOL_51_ = 51,                       /* '>'  */
  YYSYMBOL_52_ = 52,                       /* '+'  */
  YYSYMBOL_53_ = 53,                       /* '-'  */
  YYSYMBOL_54_ = 54,                       /* '*'  */
  YYSYMBOL_55_ = 55,                       /* '/'  */
  YYSYMBOL_56_ = 56,                       /* '%'  */
  YYSYMBOL_NONOPT = 57,                    /* NONOPT  */
  YYSYMBOL_58_ = 58,                       /* '?'  */
  YYSYMBOL_59_ = 59,                       /* '!'  */
  YYSYMBOL_60_ = 60,                       /* ';'  */
  YYSYMBOL_61_ = 61,                       /* '('  */
  YYSYMBOL_62_ = 62,                       /* ')'  */
  YYSYMBOL_63_ = 63,                       /* '$'  */
  YYSYMBOL_64_ = 64,                       /* ':'  */
  YYSYMBOL_65_ = 65,                       /* '.'  */
  YYSYMBOL_66_ = 66,                       /* '['  */
  YYSYMBOL_67_ = 67,                       /* ']'  */
  YYSYMBOL_68_ = 68,                       /* '{'  */
  YYSYMBOL_69_ = 69,                       /* '}'  */
  YYSYMBOL_YYACCEPT = 70,                  /* $accept  */
  YYSYMBOL_TopLevel = 71,                  /* TopLevel  */
  YYSYMBOL_Module = 72,                    /* Module  */
  YYSYMBOL_Imports = 73,                   /* Imports  */
  YYSYMBOL_FuncDefs = 74,                  /* FuncDefs  */
  YYSYMBOL_Exp = 75,                       /* Exp  */
  YYSYMBOL_Import = 76,                    /* Import  */
  YYSYMBOL_ImportWhat = 77,                /* ImportWhat  */
  YYSYMBOL_ImportFrom = 78,                /* ImportFrom  */
  YYSYMBOL_FuncDef = 79,                   /* FuncDef  */
  YYSYMBOL_Params = 80,                    /* Params  */
  YYSYMBOL_Param = 81,                     /* Param  */
  YYSYMBOL_String = 82,                    /* String  */
  YYSYMBOL_83_1 = 83,                      /* @1  */
  YYSYMBOL_84_2 = 84,                      /* @2  */
  YYSYMBOL_QQString = 85,                  /* QQString  */
  YYSYMBOL_ElseBody = 86,                  /* ElseBody  */
  YYSYMBOL_ExpD = 87,                      /* ExpD  */
  YYSYMBOL_Term = 88,                      /* Term  */
  YYSYMBOL_Args = 89,                      /* Args  */
  YYSYMBOL_Arg = 90,                       /* Arg  */
  YYSYMBOL_RepPatterns = 91,               /* RepPatterns  */
  YYSYMBOL_Patterns = 92,                  /* Patterns  */
  YYSYMBOL_Pattern = 93,                   /* Pattern  */
  YYSYMBOL_ArrayPats = 94,                 /* ArrayPats  */
  YYSYMBOL_ObjPats = 95,                   /* ObjPats  */
  YYSYMBOL_ObjPat = 96,                    /* ObjPat  */
  YYSYMBOL_Keyword = 97,                   /* Keyword  */
  YYSYMBOL_MkDict = 98,                    /* MkDict  */
  YYSYMBOL_MkDictPair = 99                 /* MkDictPair  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;


/* Second part of user prologue.  */
#line 126 "src/parser.y"

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


#line 571 "src/parser.c"


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
#define YYFINAL  28
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   2185

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  70
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  30
/* YYNRULES -- Number of rules.  */
#define YYNRULES  171
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  328

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   302


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
       2,     2,     2,    59,     2,     2,    63,    56,     2,     2,
      61,    62,    54,    52,    48,    53,    65,    55,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    64,    60,
      50,    49,    51,    58,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    66,     2,    67,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    68,    47,    69,     2,     2,     2,     2,
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
      45,    46,    57
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   308,   308,   311,   316,   319,   330,   333,   338,   341,
     346,   350,   353,   357,   361,   365,   368,   371,   376,   380,
     384,   389,   396,   400,   404,   408,   412,   416,   420,   424,
     428,   432,   436,   440,   444,   448,   452,   456,   460,   466,
     472,   476,   480,   484,   488,   492,   496,   500,   504,   508,
     513,   516,   533,   542,   549,   557,   568,   573,   579,   582,
     587,   591,   595,   602,   602,   606,   606,   613,   616,   619,
     625,   628,   633,   636,   639,   645,   648,   651,   659,   663,
     666,   669,   672,   675,   678,   681,   684,   687,   691,   697,
     700,   703,   706,   709,   712,   715,   718,   721,   724,   727,
     730,   733,   736,   739,   742,   745,   748,   751,   773,   777,
     781,   790,   802,   807,   808,   809,   810,   813,   816,   821,
     826,   829,   834,   837,   842,   846,   849,   854,   857,   862,
     865,   870,   873,   876,   879,   882,   885,   893,   899,   902,
     905,   908,   911,   914,   917,   920,   923,   926,   929,   932,
     935,   938,   941,   944,   947,   950,   953,   958,   961,   962,
     963,   966,   969,   972,   975,   979,   983,   987,   991,   995,
     999,  1007
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
  "IDENT", "FIELD", "LITERAL", "FORMAT", "\"..\"", "\"%=\"", "\"==\"",
  "\"!=\"", "\"//\"", "\"as\"", "\"def\"", "\"module\"", "\"import\"",
  "\"include\"", "\"if\"", "\"then\"", "\"else\"", "\"elif\"",
  "\"reduce\"", "\"foreach\"", "\"end\"", "\"and\"", "\"or\"", "\"try\"",
  "\"catch\"", "\"label\"", "\"break\"", "\"__loc__\"", "\"|=\"", "\"+=\"",
  "\"-=\"", "\"*=\"", "\"/=\"", "\"//=\"", "\"<=\"", "\">=\"", "\"?//\"",
  "QQSTRING_START", "QQSTRING_TEXT", "QQSTRING_INTERP_START",
  "QQSTRING_INTERP_END", "QQSTRING_END", "FUNCDEF", "'|'", "','", "'='",
  "'<'", "'>'", "'+'", "'-'", "'*'", "'/'", "'%'", "NONOPT", "'?'", "'!'",
  "';'", "'('", "')'", "'$'", "':'", "'.'", "'['", "']'", "'{'", "'}'",
  "$accept", "TopLevel", "Module", "Imports", "FuncDefs", "Exp", "Import",
  "ImportWhat", "ImportFrom", "FuncDef", "Params", "Param", "String", "@1",
  "@2", "QQString", "ElseBody", "ExpD", "Term", "Args", "Arg",
  "RepPatterns", "Patterns", "Pattern", "ArrayPats", "ObjPats", "ObjPat",
  "Keyword", "MkDict", "MkDictPair", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-141)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-158)

#define yytable_value_is_error(Yyn) \
  ((Yyn) == YYTABLE_NINF)

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
      -8,   784,    25,    37,   -30,   -18,  -141,     9,  -141,    57,
     784,   823,   823,   784,     8,     0,  -141,   784,   784,   531,
     915,   294,   463,   361,  1421,   784,  -141,    47,  -141,    -3,
      -3,   784,    37,   691,   784,  -141,  -141,   -22,  1833,    53,
      59,    50,   106,  -141,   110,  -141,     1,  -141,    61,  1251,
    -141,  -141,  -141,  -141,  -141,  -141,  -141,  -141,  -141,  -141,
    -141,  -141,  -141,  -141,  -141,  -141,  -141,  -141,  -141,  -141,
      52,  -141,  -141,   125,     9,    71,    63,  -141,   997,   -28,
      69,   784,   852,    70,    72,    68,    90,   784,   784,   784,
     784,   784,   784,   784,   784,   784,   784,   784,   784,   784,
     784,   784,   784,   784,   784,   784,   784,   784,   784,   784,
     784,  -141,  -141,  2001,    82,   -17,     4,   213,   129,  -141,
    -141,  -141,  2001,   784,  -141,  -141,  1473,  2001,    15,  -141,
    -141,     2,   784,   597,   -17,   -17,   663,    96,  -141,    31,
    -141,  -141,    81,  -141,  -141,  -141,  -141,   419,   503,  -141,
     503,  1285,    83,  -141,   503,   503,  -141,   419,  2069,   356,
     356,  2035,   445,  2101,  2069,  2069,  2069,  2069,  2069,  2069,
     356,   356,  2001,  2035,  2069,   356,   356,     1,     1,    91,
      91,    91,  -141,   149,   -17,   886,   115,   109,   118,   784,
     101,    93,   784,   103,   947,    18,  -141,  -141,   784,  -141,
      40,  -141,  2154,    60,  -141,  1525,  -141,  1733,   102,   104,
    -141,  -141,   784,  -141,   784,  -141,   160,   -35,  -141,   503,
     119,     3,   119,   105,   503,   119,   119,  -141,  -141,  -141,
     -24,   107,   108,   784,   163,   111,   -25,  -141,   112,   -17,
     784,  1047,  -141,  -141,  1097,  -141,   756,   116,  -141,   164,
    -141,  -141,  -141,  -141,     2,   113,  -141,   784,   784,  -141,
    -141,   784,   784,  2001,  1867,  -141,  -141,   503,   503,   119,
     -17,  -141,   -17,   -17,  1319,   114,   -17,   886,  -141,   -17,
     133,  2001,   122,   123,   124,  1147,  -141,  -141,  -141,   784,
    1917,  1967,  1577,  1629,  -141,   119,   119,  -141,  -141,  -141,
     120,   -17,  -141,  -141,  -141,  -141,  -141,  -141,   127,  1681,
    -141,   784,   784,   784,   -17,  -141,  -141,  -141,  1783,  1353,
    1197,  -141,  -141,  -141,   784,  -141,  1387,  -141
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       4,     0,     0,     6,   111,    84,   101,   103,    76,     0,
       0,     0,     0,     0,     0,     0,    63,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   102,    49,     1,     0,
       0,     8,     6,     0,     0,    80,    65,     0,     0,     0,
       0,    19,     0,    78,     0,    67,    33,    48,     0,     0,
     109,   138,   139,   140,   141,   142,   143,   144,   145,   146,
     147,   148,   149,   150,   151,   152,   153,   154,   155,   156,
       0,   110,    87,     0,     0,    86,     0,   106,     0,     0,
     168,     0,     0,   164,   169,     0,   158,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    22,     5,    10,    83,     0,     0,     0,     0,    55,
      54,     3,     2,     8,     7,    50,     0,   119,     0,   117,
      67,     0,     0,     0,     0,     0,     0,     0,    77,     0,
     113,   104,     0,    88,    82,   114,   105,     0,     0,   116,
       0,     0,   166,   167,     0,     0,   107,     0,    41,    42,
      43,    26,    25,    24,    28,    32,    35,    37,    40,    27,
      46,    47,    29,    30,    23,    44,    45,    31,    34,    36,
      38,    39,    79,     0,     0,     0,     0,     0,   123,     0,
      85,     0,     0,    94,     0,     0,     9,    51,     0,   112,
       0,    62,     0,     0,    58,     0,    17,     0,     0,     0,
      20,    18,     0,    68,     0,    64,     0,     0,   160,     0,
     171,    74,   161,     0,     0,   163,   162,   159,   124,   127,
       0,     0,     0,     0,     0,     0,     0,   129,     0,     0,
       0,     0,    81,   115,     0,    93,     0,    90,    53,     0,
     118,    66,    60,    61,     0,     0,    56,     0,     0,    16,
      15,     0,     0,    21,     0,   108,    73,     0,     0,   165,
       0,   125,     0,     0,     0,   131,     0,     0,   126,     0,
     122,    11,    92,   100,    99,     0,    89,    52,    59,     0,
       0,     0,     0,     0,    69,    72,   170,   128,   137,   133,
       0,     0,   135,   130,   134,    91,    97,    96,    98,     0,
      71,     0,     0,     0,     0,   132,    95,    57,     0,     0,
       0,   136,    70,    12,     0,    14,     0,    13
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -141,  -141,  -141,   151,    66,    -1,  -141,  -141,   156,   -12,
    -141,   -61,     5,  -141,  -141,    64,  -123,  -140,    -9,  -141,
      -2,  -141,   -87,  -122,  -141,  -141,   -79,   -15,  -120,  -141
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,     2,     3,    31,   121,   113,    32,    33,   118,    25,
     203,   204,    26,    45,   130,   139,   260,   220,    27,   128,
     129,   186,   187,   188,   230,   236,   237,    84,    85,    86
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      24,    43,    39,    40,    74,    71,   201,     1,   114,    38,
     222,    74,    41,   147,   225,   226,    46,    47,    49,   123,
     147,    78,   248,   277,   270,    28,    75,   218,    83,   148,
     122,    34,   126,   127,   119,   119,   148,   227,    16,   131,
      35,   149,   132,   271,   278,    16,   183,   208,   209,   184,
      36,   185,   114,    29,    30,   108,   109,   110,   114,   111,
     115,    37,   229,    44,   114,   202,   134,   153,   116,   117,
     189,    42,   135,   213,   214,   198,   215,   199,   136,   266,
     151,   249,   213,   214,   269,   251,   158,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   169,   170,   171,
     172,   173,   174,   175,   176,   177,   178,   179,   180,   181,
     137,   123,   116,   117,   138,   142,   194,   280,   116,   117,
     254,   190,   255,   140,   116,   117,   143,   295,   296,   144,
     145,   205,   207,   150,   154,   211,   155,   156,   157,   221,
     182,   221,   195,   212,   216,   221,   221,   224,   297,   111,
     298,   299,    83,   228,   302,   239,   240,   304,  -121,   242,
     243,   245,    83,   261,   265,   262,   267,   275,   287,   268,
     238,   272,   273,  -120,   286,   276,   279,   289,   301,   315,
     305,   306,   307,   124,   314,   316,   120,   253,   241,   196,
     235,   244,   321,   288,   200,   322,   250,   127,   303,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     221,   263,     0,   264,   191,   221,     0,     4,     5,     6,
       7,     8,     0,     0,     0,     0,     0,     9,     0,     0,
       0,    10,   274,     0,     0,    11,    12,     0,     0,   281,
      13,     0,    14,    15,     0,   285,     0,     0,     0,     0,
       0,     0,     0,     0,    16,     0,   290,   291,   221,   221,
     292,   293,   238,     0,     0,     0,    17,     0,     0,     0,
       0,     0,    18,     0,    19,     0,    20,   192,    21,    22,
     193,    23,   235,     0,     0,     0,     0,     0,   309,     0,
       0,     0,     0,     0,   -75,    72,     0,     0,    73,   -75,
       0,    74,     0,   -75,   -75,   -75,   -75,   -75,     0,     0,
     318,   319,   320,   -75,   -75,   -75,     0,     0,   -75,   -75,
     -75,     0,   -75,   326,     0,     0,   -75,   -75,   -75,   -75,
     -75,   -75,   -75,   -75,     0,    16,     0,     0,   -75,     0,
       0,   -75,   -75,   -75,   -75,   -75,   -75,   -75,   -75,   -75,
     -75,     0,   -75,     0,   -75,     0,   -75,     0,   -75,   -75,
     -75,   -75,    79,   -75,     0,    80,  -158,  -158,    74,     0,
       0,     0,     0,     0,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,     0,  -158,  -158,     0,     0,     0,     0,
       0,     0,    16,     0,     0,     0,  -158,  -158,   106,   107,
     108,   109,   110,     0,   111,     0,     0,     0,     0,     0,
     217,     0,    81,    80,    82,     0,    74,     0,     0,     0,
    -157,     0,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,     0,     0,     0,     0,    88,    89,     0,     0,     0,
      16,     0,     0,     0,    76,     0,     0,     4,     5,     6,
       7,     8,     0,     0,     0,     0,     0,     9,     0,     0,
      81,    10,    82,    99,   100,    11,    12,     0,  -157,     0,
      13,     0,    14,    15,     0,   104,   105,   106,   107,   108,
     109,   110,     0,   111,    16,     0,     0,     4,     5,     6,
       7,     8,     0,     0,     0,     0,    17,     0,     0,     0,
       0,     0,    18,     0,    19,     0,    20,     0,    21,    22,
      77,    23,    48,    15,     0,     4,     5,     6,     7,     8,
       0,     0,     0,     0,    16,     9,     0,     0,     0,    10,
       0,     0,     0,    11,    12,     0,   219,     0,    13,     0,
      14,    15,     0,     0,    19,     0,    20,     0,    21,    22,
       0,    23,    16,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    17,     0,     0,     0,     0,     0,
      18,     0,    19,     0,    20,     0,    21,    22,   206,    23,
       0,     4,     5,     6,     7,     8,     0,     0,     0,     0,
       0,     9,     0,     0,     0,    10,     0,     0,     0,    11,
      12,     0,     0,     0,    13,     0,    14,    15,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    16,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      17,     0,     0,     0,     0,     0,    18,     0,    19,     0,
      20,     0,    21,    22,   210,    23,     0,     4,     5,     6,
       7,     8,     0,     0,     0,     0,     0,     9,     0,     0,
       0,    10,     0,     0,     0,    11,    12,     0,     0,     0,
      13,     0,    14,    15,     0,     4,     5,     6,     7,     8,
       0,     0,     0,     0,    16,     9,     0,     0,     0,    10,
       0,     0,     0,    11,    12,     0,    17,     0,    13,     0,
      14,    15,    18,     0,    19,     0,    20,     0,    21,    22,
       0,    23,    16,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    17,     0,     0,     0,     0,     0,
      18,   125,    19,     0,    20,     0,    21,    22,     0,    23,
       4,     5,     6,     7,     8,     0,     0,     0,     0,     0,
       9,     0,     0,     0,    10,     0,     0,     0,    11,    12,
       0,     0,     0,    13,     0,    14,    15,     0,     4,     5,
       6,     7,     8,     0,     0,     0,     0,    16,     9,     0,
       0,     0,    10,     0,     0,     0,    11,    12,     0,    17,
       0,    13,     0,    14,    15,    18,     0,    19,     0,    20,
       0,    21,    22,   284,    23,    16,     0,     4,     5,     6,
       7,     8,     0,     0,     0,     0,     0,    17,     0,     0,
       0,     0,     0,    18,     0,    19,     0,    20,     0,    21,
      22,     0,    23,    15,     0,     0,   152,     0,     0,     0,
       0,     0,     0,     0,    16,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    19,     0,    20,   231,    21,    22,
     232,    23,     0,    74,     0,     0,     0,     0,     0,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,     0,    50,
       0,     0,     0,     0,     0,     0,     0,    16,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,   233,     0,   234,
       0,     0,     0,     0,     0,     0,    87,    88,    89,    90,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    91,    92,     0,     0,     0,     0,    70,    93,
      94,    95,    96,    97,    98,    99,   100,     0,     0,     0,
       0,     0,     0,     0,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,     0,   111,    87,    88,    89,    90,
       0,   246,     0,     0,   247,     0,     0,     0,     0,     0,
       0,     0,    91,    92,     0,     0,     0,     0,     0,    93,
      94,    95,    96,    97,    98,    99,   100,     0,     0,     0,
       0,     0,     0,     0,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,     0,   111,    87,    88,    89,    90,
       0,     0,     0,     0,   146,     0,     0,     0,     0,     0,
       0,     0,    91,    92,     0,     0,     0,     0,     0,    93,
      94,    95,    96,    97,    98,    99,   100,     0,     0,     0,
       0,     0,     0,     0,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,     0,   111,    87,    88,    89,    90,
       0,     0,     0,     0,   282,     0,     0,     0,     0,     0,
       0,     0,    91,    92,     0,     0,     0,     0,     0,    93,
      94,    95,    96,    97,    98,    99,   100,     0,     0,     0,
       0,     0,     0,     0,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,     0,   111,    87,    88,    89,    90,
       0,     0,     0,     0,   283,     0,     0,     0,     0,     0,
       0,     0,    91,    92,     0,     0,     0,     0,     0,    93,
      94,    95,    96,    97,    98,    99,   100,     0,     0,     0,
       0,     0,     0,     0,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,     0,   111,    87,    88,    89,    90,
       0,     0,     0,     0,   308,     0,     0,     0,     0,     0,
       0,     0,    91,    92,     0,     0,     0,     0,     0,    93,
      94,    95,    96,    97,    98,    99,   100,     0,     0,     0,
       0,     0,     0,     0,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,     0,   111,     0,   324,     0,   325,
      87,    88,    89,    90,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    91,    92,     0,     0,
       0,     0,     0,    93,    94,    95,    96,    97,    98,    99,
     100,     0,     0,     0,    87,    88,    89,    90,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,     0,   111,
      91,    92,     0,   141,     0,     0,     0,    93,    94,    95,
      96,    97,    98,    99,   100,     0,     0,     0,    87,    88,
      89,    90,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,     0,   111,    91,    92,     0,   223,     0,     0,
       0,    93,    94,    95,    96,    97,    98,    99,   100,     0,
       0,     0,    87,    88,    89,    90,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,     0,   111,    91,    92,
       0,   300,     0,     0,     0,    93,    94,    95,    96,    97,
      98,    99,   100,     0,     0,     0,    87,    88,    89,    90,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
       0,   111,    91,    92,     0,   323,     0,     0,     0,    93,
      94,    95,    96,    97,    98,    99,   100,     0,     0,     0,
      87,    88,    89,    90,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,     0,   111,    91,    92,     0,   327,
       0,     0,     0,    93,    94,    95,    96,    97,    98,    99,
     100,     0,     0,     0,     0,     0,     0,     0,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,     0,   111,
       0,   112,    87,    88,    89,    90,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    91,    92,
       0,     0,     0,     0,     0,    93,    94,    95,    96,    97,
      98,    99,   100,     0,     0,     0,     0,     0,     0,     0,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
       0,   111,     0,   197,    87,    88,    89,    90,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      91,    92,     0,     0,     0,     0,     0,    93,    94,    95,
      96,    97,    98,    99,   100,     0,     0,     0,     0,     0,
       0,     0,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,     0,   111,     0,   256,    87,    88,    89,    90,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    91,    92,     0,     0,     0,     0,     0,    93,
      94,    95,    96,    97,    98,    99,   100,     0,     0,     0,
       0,     0,     0,     0,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,     0,   111,     0,   312,    87,    88,
      89,    90,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    91,    92,     0,     0,     0,     0,
       0,    93,    94,    95,    96,    97,    98,    99,   100,     0,
       0,     0,     0,     0,     0,     0,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,     0,   111,     0,   313,
      87,    88,    89,    90,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    91,    92,     0,     0,
       0,     0,     0,    93,    94,    95,    96,    97,    98,    99,
     100,     0,     0,     0,     0,     0,     0,     0,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,     0,   111,
       0,   317,    87,    88,    89,    90,     0,     0,     0,     0,
       0,     0,     0,   257,   258,     0,     0,   259,    91,    92,
       0,     0,     0,     0,     0,    93,    94,    95,    96,    97,
      98,    99,   100,     0,     0,     0,     0,     0,     0,     0,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
       0,   111,    87,    88,    89,    90,     0,     0,     0,     0,
       0,     0,     0,   257,   258,     0,     0,     0,    91,    92,
       0,     0,     0,     0,     0,    93,    94,    95,    96,    97,
      98,    99,   100,     0,     0,     0,     0,     0,     0,     0,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
       0,   111,    87,    88,    89,    90,     0,     0,     0,     0,
       0,     0,   133,     0,     0,     0,     0,     0,    91,    92,
       0,     0,     0,     0,     0,    93,    94,    95,    96,    97,
      98,    99,   100,     0,     0,     0,    87,    88,    89,    90,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
       0,   111,    91,    92,     0,     0,     0,     0,     0,    93,
      94,    95,    96,    97,    98,    99,   100,     0,     0,     0,
       0,   294,     0,     0,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,     0,   111,    87,    88,    89,    90,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   310,    91,    92,     0,     0,     0,     0,     0,    93,
      94,    95,    96,    97,    98,    99,   100,     0,     0,     0,
       0,     0,     0,     0,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,     0,   111,    87,    88,    89,    90,
       0,     0,     0,     0,     0,     0,   311,     0,     0,     0,
       0,     0,    91,    92,     0,     0,     0,     0,     0,    93,
      94,    95,    96,    97,    98,    99,   100,     0,     0,     0,
      87,    88,    89,    90,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,     0,   111,    91,    92,     0,     0,
       0,     0,     0,    93,    94,    95,    96,    97,    98,    99,
     100,     0,     0,     0,    87,    88,    89,    90,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,     0,   111,
      91,    92,     0,     0,     0,     0,     0,    93,    94,    95,
      96,    97,    98,    99,   100,     0,     0,     0,  -158,    88,
      89,     0,     0,     0,   103,   104,   105,   106,   107,   108,
     109,   110,     0,   111,    91,    92,     0,     0,     0,     0,
       0,  -158,  -158,  -158,  -158,  -158,  -158,    99,   100,     0,
       0,    88,    89,     0,     0,     0,     0,     0,  -158,   104,
     105,   106,   107,   108,   109,   110,    91,   111,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    99,
     100,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   104,   105,   106,   107,   108,   109,   110,   252,   111,
       0,     0,     0,     0,     0,     0,     0,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69
};

static const yytype_int16 yycheck[] =
{
       1,     1,    11,    12,     7,    20,     4,    15,     5,    10,
     150,     7,    13,    48,   154,   155,    17,    18,    19,    31,
      48,    22,     4,    48,    48,     0,    21,   147,    23,    64,
      31,    61,    33,    34,    29,    30,    64,   157,    41,    61,
      58,    69,    64,    67,    69,    41,    63,   134,   135,    66,
      41,    68,     5,    16,    17,    54,    55,    56,     5,    58,
      13,     4,   184,    63,     5,    63,    13,    82,    65,    66,
      66,    63,    13,    42,    43,    60,    45,    62,    28,   219,
      81,    63,    42,    43,   224,    45,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
       4,   123,    65,    66,     4,    63,   117,   239,    65,    66,
      60,   116,    62,    62,    65,    66,     1,   267,   268,    58,
      67,   132,   133,    64,    64,   136,    64,    69,    48,   148,
      58,   150,    13,    47,    63,   154,   155,    64,   270,    58,
     272,   273,   147,     4,   276,    40,    47,   279,    40,    58,
      67,    58,   157,    61,     4,    61,    47,     4,     4,    64,
     185,    64,    64,    40,    58,    64,    64,    64,    64,   301,
      58,    58,    58,    32,    64,    58,    30,   202,   189,   123,
     185,   192,   314,   254,   130,   318,   198,   198,   277,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     219,   212,    -1,   214,     1,   224,    -1,     4,     5,     6,
       7,     8,    -1,    -1,    -1,    -1,    -1,    14,    -1,    -1,
      -1,    18,   233,    -1,    -1,    22,    23,    -1,    -1,   240,
      27,    -1,    29,    30,    -1,   246,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    41,    -1,   257,   258,   267,   268,
     261,   262,   277,    -1,    -1,    -1,    53,    -1,    -1,    -1,
      -1,    -1,    59,    -1,    61,    -1,    63,    64,    65,    66,
      67,    68,   277,    -1,    -1,    -1,    -1,    -1,   289,    -1,
      -1,    -1,    -1,    -1,     0,     1,    -1,    -1,     4,     5,
      -1,     7,    -1,     9,    10,    11,    12,    13,    -1,    -1,
     311,   312,   313,    19,    20,    21,    -1,    -1,    24,    25,
      26,    -1,    28,   324,    -1,    -1,    32,    33,    34,    35,
      36,    37,    38,    39,    -1,    41,    -1,    -1,    44,    -1,
      -1,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    -1,    58,    -1,    60,    -1,    62,    -1,    64,    65,
      66,    67,     1,    69,    -1,     4,    10,    11,     7,    -1,
      -1,    -1,    -1,    -1,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    -1,    38,    39,    -1,    -1,    -1,    -1,
      -1,    -1,    41,    -1,    -1,    -1,    50,    51,    52,    53,
      54,    55,    56,    -1,    58,    -1,    -1,    -1,    -1,    -1,
       1,    -1,    61,     4,    63,    -1,     7,    -1,    -1,    -1,
      69,    -1,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    -1,    -1,    -1,    -1,    10,    11,    -1,    -1,    -1,
      41,    -1,    -1,    -1,     1,    -1,    -1,     4,     5,     6,
       7,     8,    -1,    -1,    -1,    -1,    -1,    14,    -1,    -1,
      61,    18,    63,    38,    39,    22,    23,    -1,    69,    -1,
      27,    -1,    29,    30,    -1,    50,    51,    52,    53,    54,
      55,    56,    -1,    58,    41,    -1,    -1,     4,     5,     6,
       7,     8,    -1,    -1,    -1,    -1,    53,    -1,    -1,    -1,
      -1,    -1,    59,    -1,    61,    -1,    63,    -1,    65,    66,
      67,    68,     1,    30,    -1,     4,     5,     6,     7,     8,
      -1,    -1,    -1,    -1,    41,    14,    -1,    -1,    -1,    18,
      -1,    -1,    -1,    22,    23,    -1,    53,    -1,    27,    -1,
      29,    30,    -1,    -1,    61,    -1,    63,    -1,    65,    66,
      -1,    68,    41,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    53,    -1,    -1,    -1,    -1,    -1,
      59,    -1,    61,    -1,    63,    -1,    65,    66,     1,    68,
      -1,     4,     5,     6,     7,     8,    -1,    -1,    -1,    -1,
      -1,    14,    -1,    -1,    -1,    18,    -1,    -1,    -1,    22,
      23,    -1,    -1,    -1,    27,    -1,    29,    30,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    41,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      53,    -1,    -1,    -1,    -1,    -1,    59,    -1,    61,    -1,
      63,    -1,    65,    66,     1,    68,    -1,     4,     5,     6,
       7,     8,    -1,    -1,    -1,    -1,    -1,    14,    -1,    -1,
      -1,    18,    -1,    -1,    -1,    22,    23,    -1,    -1,    -1,
      27,    -1,    29,    30,    -1,     4,     5,     6,     7,     8,
      -1,    -1,    -1,    -1,    41,    14,    -1,    -1,    -1,    18,
      -1,    -1,    -1,    22,    23,    -1,    53,    -1,    27,    -1,
      29,    30,    59,    -1,    61,    -1,    63,    -1,    65,    66,
      -1,    68,    41,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    53,    -1,    -1,    -1,    -1,    -1,
      59,    60,    61,    -1,    63,    -1,    65,    66,    -1,    68,
       4,     5,     6,     7,     8,    -1,    -1,    -1,    -1,    -1,
      14,    -1,    -1,    -1,    18,    -1,    -1,    -1,    22,    23,
      -1,    -1,    -1,    27,    -1,    29,    30,    -1,     4,     5,
       6,     7,     8,    -1,    -1,    -1,    -1,    41,    14,    -1,
      -1,    -1,    18,    -1,    -1,    -1,    22,    23,    -1,    53,
      -1,    27,    -1,    29,    30,    59,    -1,    61,    -1,    63,
      -1,    65,    66,    67,    68,    41,    -1,     4,     5,     6,
       7,     8,    -1,    -1,    -1,    -1,    -1,    53,    -1,    -1,
      -1,    -1,    -1,    59,    -1,    61,    -1,    63,    -1,    65,
      66,    -1,    68,    30,    -1,    -1,     4,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    41,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    61,    -1,    63,     1,    65,    66,
       4,    68,    -1,     7,    -1,    -1,    -1,    -1,    -1,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    -1,     4,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    41,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    61,    -1,    63,
      -1,    -1,    -1,    -1,    -1,    -1,     9,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    25,    26,    -1,    -1,    -1,    -1,    63,    32,
      33,    34,    35,    36,    37,    38,    39,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    -1,    58,     9,    10,    11,    12,
      -1,    64,    -1,    -1,    67,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    25,    26,    -1,    -1,    -1,    -1,    -1,    32,
      33,    34,    35,    36,    37,    38,    39,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    -1,    58,     9,    10,    11,    12,
      -1,    -1,    -1,    -1,    67,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    25,    26,    -1,    -1,    -1,    -1,    -1,    32,
      33,    34,    35,    36,    37,    38,    39,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    -1,    58,     9,    10,    11,    12,
      -1,    -1,    -1,    -1,    67,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    25,    26,    -1,    -1,    -1,    -1,    -1,    32,
      33,    34,    35,    36,    37,    38,    39,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    -1,    58,     9,    10,    11,    12,
      -1,    -1,    -1,    -1,    67,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    25,    26,    -1,    -1,    -1,    -1,    -1,    32,
      33,    34,    35,    36,    37,    38,    39,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    -1,    58,     9,    10,    11,    12,
      -1,    -1,    -1,    -1,    67,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    25,    26,    -1,    -1,    -1,    -1,    -1,    32,
      33,    34,    35,    36,    37,    38,    39,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    -1,    58,    -1,    60,    -1,    62,
       9,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    25,    26,    -1,    -1,
      -1,    -1,    -1,    32,    33,    34,    35,    36,    37,    38,
      39,    -1,    -1,    -1,     9,    10,    11,    12,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    -1,    58,
      25,    26,    -1,    62,    -1,    -1,    -1,    32,    33,    34,
      35,    36,    37,    38,    39,    -1,    -1,    -1,     9,    10,
      11,    12,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    -1,    58,    25,    26,    -1,    62,    -1,    -1,
      -1,    32,    33,    34,    35,    36,    37,    38,    39,    -1,
      -1,    -1,     9,    10,    11,    12,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    -1,    58,    25,    26,
      -1,    62,    -1,    -1,    -1,    32,    33,    34,    35,    36,
      37,    38,    39,    -1,    -1,    -1,     9,    10,    11,    12,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      -1,    58,    25,    26,    -1,    62,    -1,    -1,    -1,    32,
      33,    34,    35,    36,    37,    38,    39,    -1,    -1,    -1,
       9,    10,    11,    12,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    -1,    58,    25,    26,    -1,    62,
      -1,    -1,    -1,    32,    33,    34,    35,    36,    37,    38,
      39,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    -1,    58,
      -1,    60,     9,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,    26,
      -1,    -1,    -1,    -1,    -1,    32,    33,    34,    35,    36,
      37,    38,    39,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      -1,    58,    -1,    60,     9,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      25,    26,    -1,    -1,    -1,    -1,    -1,    32,    33,    34,
      35,    36,    37,    38,    39,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    -1,    58,    -1,    60,     9,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    25,    26,    -1,    -1,    -1,    -1,    -1,    32,
      33,    34,    35,    36,    37,    38,    39,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    -1,    58,    -1,    60,     9,    10,
      11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    25,    26,    -1,    -1,    -1,    -1,
      -1,    32,    33,    34,    35,    36,    37,    38,    39,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    -1,    58,    -1,    60,
       9,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    25,    26,    -1,    -1,
      -1,    -1,    -1,    32,    33,    34,    35,    36,    37,    38,
      39,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    -1,    58,
      -1,    60,     9,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    20,    21,    -1,    -1,    24,    25,    26,
      -1,    -1,    -1,    -1,    -1,    32,    33,    34,    35,    36,
      37,    38,    39,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      -1,    58,     9,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    20,    21,    -1,    -1,    -1,    25,    26,
      -1,    -1,    -1,    -1,    -1,    32,    33,    34,    35,    36,
      37,    38,    39,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      -1,    58,     9,    10,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    19,    -1,    -1,    -1,    -1,    -1,    25,    26,
      -1,    -1,    -1,    -1,    -1,    32,    33,    34,    35,    36,
      37,    38,    39,    -1,    -1,    -1,     9,    10,    11,    12,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      -1,    58,    25,    26,    -1,    -1,    -1,    -1,    -1,    32,
      33,    34,    35,    36,    37,    38,    39,    -1,    -1,    -1,
      -1,    44,    -1,    -1,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    -1,    58,     9,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    24,    25,    26,    -1,    -1,    -1,    -1,    -1,    32,
      33,    34,    35,    36,    37,    38,    39,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    -1,    58,     9,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    19,    -1,    -1,    -1,
      -1,    -1,    25,    26,    -1,    -1,    -1,    -1,    -1,    32,
      33,    34,    35,    36,    37,    38,    39,    -1,    -1,    -1,
       9,    10,    11,    12,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    -1,    58,    25,    26,    -1,    -1,
      -1,    -1,    -1,    32,    33,    34,    35,    36,    37,    38,
      39,    -1,    -1,    -1,     9,    10,    11,    12,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    -1,    58,
      25,    26,    -1,    -1,    -1,    -1,    -1,    32,    33,    34,
      35,    36,    37,    38,    39,    -1,    -1,    -1,     9,    10,
      11,    -1,    -1,    -1,    49,    50,    51,    52,    53,    54,
      55,    56,    -1,    58,    25,    26,    -1,    -1,    -1,    -1,
      -1,    32,    33,    34,    35,    36,    37,    38,    39,    -1,
      -1,    10,    11,    -1,    -1,    -1,    -1,    -1,    49,    50,
      51,    52,    53,    54,    55,    56,    25,    58,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    38,
      39,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    50,    51,    52,    53,    54,    55,    56,     4,    58,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,    15,    71,    72,     4,     5,     6,     7,     8,    14,
      18,    22,    23,    27,    29,    30,    41,    53,    59,    61,
      63,    65,    66,    68,    75,    79,    82,    88,     0,    16,
      17,    73,    76,    77,    61,    58,    41,     4,    75,    88,
      88,    75,    63,     1,    63,    83,    75,    75,     1,    75,
       4,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      63,    97,     1,     4,     7,    82,     1,    67,    75,     1,
       4,    61,    63,    82,    97,    98,    99,     9,    10,    11,
      12,    25,    26,    32,    33,    34,    35,    36,    37,    38,
      39,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    58,    60,    75,     5,    13,    65,    66,    78,    82,
      78,    74,    75,    79,    73,    60,    75,    75,    89,    90,
      84,    61,    64,    19,    13,    13,    28,     4,     4,    85,
      62,    62,    63,     1,    58,    67,    67,    48,    64,    69,
      64,    75,     4,    97,    64,    64,    69,    48,    75,    75,
      75,    75,    75,    75,    75,    75,    75,    75,    75,    75,
      75,    75,    75,    75,    75,    75,    75,    75,    75,    75,
      75,    75,    58,    63,    66,    68,    91,    92,    93,    66,
      82,     1,    64,    67,    75,    13,    74,    60,    60,    62,
      85,     4,    63,    80,    81,    75,     1,    75,    92,    92,
       1,    75,    47,    42,    43,    45,    63,     1,    98,    53,
      87,    88,    87,    62,    64,    87,    87,    98,     4,    93,
      94,     1,     4,    61,    63,    82,    95,    96,    97,    40,
      47,    75,    58,    67,    75,    58,    64,    67,     4,    63,
      90,    45,     4,    97,    60,    62,    60,    20,    21,    24,
      86,    61,    61,    75,    75,     4,    87,    47,    64,    87,
      48,    67,    64,    64,    75,     4,    64,    48,    69,    64,
      93,    75,    67,    67,    67,    75,    58,     4,    81,    64,
      75,    75,    75,    75,    44,    87,    87,    93,    93,    93,
      62,    64,    93,    96,    93,    58,    58,    58,    67,    75,
      24,    19,    60,    60,    64,    93,    58,    60,    75,    75,
      75,    93,    86,    62,    60,    62,    75,    62
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    70,    71,    71,    72,    72,    73,    73,    74,    74,
      75,    75,    75,    75,    75,    75,    75,    75,    75,    75,
      75,    75,    75,    75,    75,    75,    75,    75,    75,    75,
      75,    75,    75,    75,    75,    75,    75,    75,    75,    75,
      75,    75,    75,    75,    75,    75,    75,    75,    75,    75,
      76,    76,    77,    77,    77,    78,    79,    79,    80,    80,
      81,    81,    81,    83,    82,    84,    82,    85,    85,    85,
      86,    86,    87,    87,    87,    88,    88,    88,    88,    88,
      88,    88,    88,    88,    88,    88,    88,    88,    88,    88,
      88,    88,    88,    88,    88,    88,    88,    88,    88,    88,
      88,    88,    88,    88,    88,    88,    88,    88,    88,    88,
      88,    88,    88,    88,    88,    88,    88,    89,    89,    90,
      91,    91,    92,    92,    93,    93,    93,    94,    94,    95,
      95,    96,    96,    96,    96,    96,    96,    96,    97,    97,
      97,    97,    97,    97,    97,    97,    97,    97,    97,    97,
      97,    97,    97,    97,    97,    97,    97,    98,    98,    98,
      98,    99,    99,    99,    99,    99,    99,    99,    99,    99,
      99,    99
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     3,     3,     0,     3,     0,     2,     0,     2,
       2,     5,     9,    11,     9,     5,     5,     4,     4,     2,
       4,     5,     2,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     2,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     2,     1,
       2,     3,     5,     4,     2,     1,     5,     8,     1,     3,
       2,     2,     1,     0,     4,     0,     5,     0,     2,     4,
       5,     3,     3,     2,     1,     1,     1,     3,     2,     3,
       2,     4,     3,     2,     1,     3,     2,     2,     3,     5,
       4,     6,     5,     4,     3,     7,     6,     6,     6,     5,
       5,     1,     1,     1,     3,     3,     2,     3,     5,     2,
       2,     1,     4,     3,     3,     4,     3,     1,     3,     1,
       3,     1,     3,     1,     2,     3,     3,     1,     3,     1,
       3,     2,     4,     3,     3,     3,     5,     3,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     0,     1,     3,
       3,     3,     3,     3,     1,     4,     2,     2,     1,     1,
       5,     3
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
#line 36 "src/parser.y"
            { jv_free(((*yyvaluep).literal)); }
#line 2225 "src/parser.c"
        break;

    case YYSYMBOL_FIELD: /* FIELD  */
#line 36 "src/parser.y"
            { jv_free(((*yyvaluep).literal)); }
#line 2231 "src/parser.c"
        break;

    case YYSYMBOL_LITERAL: /* LITERAL  */
#line 36 "src/parser.y"
            { jv_free(((*yyvaluep).literal)); }
#line 2237 "src/parser.c"
        break;

    case YYSYMBOL_FORMAT: /* FORMAT  */
#line 36 "src/parser.y"
            { jv_free(((*yyvaluep).literal)); }
#line 2243 "src/parser.c"
        break;

    case YYSYMBOL_QQSTRING_TEXT: /* QQSTRING_TEXT  */
#line 36 "src/parser.y"
            { jv_free(((*yyvaluep).literal)); }
#line 2249 "src/parser.c"
        break;

    case YYSYMBOL_Module: /* Module  */
#line 37 "src/parser.y"
            { block_free(((*yyvaluep).blk)); }
#line 2255 "src/parser.c"
        break;

    case YYSYMBOL_Imports: /* Imports  */
#line 37 "src/parser.y"
            { block_free(((*yyvaluep).blk)); }
#line 2261 "src/parser.c"
        break;

    case YYSYMBOL_FuncDefs: /* FuncDefs  */
#line 37 "src/parser.y"
            { block_free(((*yyvaluep).blk)); }
#line 2267 "src/parser.c"
        break;

    case YYSYMBOL_Exp: /* Exp  */
#line 37 "src/parser.y"
            { block_free(((*yyvaluep).blk)); }
#line 2273 "src/parser.c"
        break;

    case YYSYMBOL_Import: /* Import  */
#line 37 "src/parser.y"
            { block_free(((*yyvaluep).blk)); }
#line 2279 "src/parser.c"
        break;

    case YYSYMBOL_ImportWhat: /* ImportWhat  */
#line 37 "src/parser.y"
            { block_free(((*yyvaluep).blk)); }
#line 2285 "src/parser.c"
        break;

    case YYSYMBOL_ImportFrom: /* ImportFrom  */
#line 37 "src/parser.y"
            { block_free(((*yyvaluep).blk)); }
#line 2291 "src/parser.c"
        break;

    case YYSYMBOL_FuncDef: /* FuncDef  */
#line 37 "src/parser.y"
            { block_free(((*yyvaluep).blk)); }
#line 2297 "src/parser.c"
        break;

    case YYSYMBOL_Params: /* Params  */
#line 37 "src/parser.y"
            { block_free(((*yyvaluep).blk)); }
#line 2303 "src/parser.c"
        break;

    case YYSYMBOL_Param: /* Param  */
#line 37 "src/parser.y"
            { block_free(((*yyvaluep).blk)); }
#line 2309 "src/parser.c"
        break;

    case YYSYMBOL_String: /* String  */
#line 37 "src/parser.y"
            { block_free(((*yyvaluep).blk)); }
#line 2315 "src/parser.c"
        break;

    case YYSYMBOL_QQString: /* QQString  */
#line 37 "src/parser.y"
            { block_free(((*yyvaluep).blk)); }
#line 2321 "src/parser.c"
        break;

    case YYSYMBOL_ElseBody: /* ElseBody  */
#line 37 "src/parser.y"
            { block_free(((*yyvaluep).blk)); }
#line 2327 "src/parser.c"
        break;

    case YYSYMBOL_ExpD: /* ExpD  */
#line 37 "src/parser.y"
            { block_free(((*yyvaluep).blk)); }
#line 2333 "src/parser.c"
        break;

    case YYSYMBOL_Term: /* Term  */
#line 37 "src/parser.y"
            { block_free(((*yyvaluep).blk)); }
#line 2339 "src/parser.c"
        break;

    case YYSYMBOL_Args: /* Args  */
#line 37 "src/parser.y"
            { block_free(((*yyvaluep).blk)); }
#line 2345 "src/parser.c"
        break;

    case YYSYMBOL_Arg: /* Arg  */
#line 37 "src/parser.y"
            { block_free(((*yyvaluep).blk)); }
#line 2351 "src/parser.c"
        break;

    case YYSYMBOL_RepPatterns: /* RepPatterns  */
#line 37 "src/parser.y"
            { block_free(((*yyvaluep).blk)); }
#line 2357 "src/parser.c"
        break;

    case YYSYMBOL_Patterns: /* Patterns  */
#line 37 "src/parser.y"
            { block_free(((*yyvaluep).blk)); }
#line 2363 "src/parser.c"
        break;

    case YYSYMBOL_Pattern: /* Pattern  */
#line 37 "src/parser.y"
            { block_free(((*yyvaluep).blk)); }
#line 2369 "src/parser.c"
        break;

    case YYSYMBOL_ArrayPats: /* ArrayPats  */
#line 37 "src/parser.y"
            { block_free(((*yyvaluep).blk)); }
#line 2375 "src/parser.c"
        break;

    case YYSYMBOL_ObjPats: /* ObjPats  */
#line 37 "src/parser.y"
            { block_free(((*yyvaluep).blk)); }
#line 2381 "src/parser.c"
        break;

    case YYSYMBOL_ObjPat: /* ObjPat  */
#line 37 "src/parser.y"
            { block_free(((*yyvaluep).blk)); }
#line 2387 "src/parser.c"
        break;

    case YYSYMBOL_Keyword: /* Keyword  */
#line 36 "src/parser.y"
            { jv_free(((*yyvaluep).literal)); }
#line 2393 "src/parser.c"
        break;

    case YYSYMBOL_MkDict: /* MkDict  */
#line 37 "src/parser.y"
            { block_free(((*yyvaluep).blk)); }
#line 2399 "src/parser.c"
        break;

    case YYSYMBOL_MkDictPair: /* MkDictPair  */
#line 37 "src/parser.y"
            { block_free(((*yyvaluep).blk)); }
#line 2405 "src/parser.c"
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
  case 2: /* TopLevel: Module Imports Exp  */
#line 308 "src/parser.y"
                   {
  *answer = BLOCK((yyvsp[-2].blk), (yyvsp[-1].blk), gen_op_simple(TOP), (yyvsp[0].blk));
}
#line 2713 "src/parser.c"
    break;

  case 3: /* TopLevel: Module Imports FuncDefs  */
#line 311 "src/parser.y"
                        {
  *answer = BLOCK((yyvsp[-2].blk), (yyvsp[-1].blk), (yyvsp[0].blk));
}
#line 2721 "src/parser.c"
    break;

  case 4: /* Module: %empty  */
#line 316 "src/parser.y"
       {
  (yyval.blk) = gen_noop();
}
#line 2729 "src/parser.c"
    break;

  case 5: /* Module: "module" Exp ';'  */
#line 319 "src/parser.y"
                 {
  if (!block_is_const((yyvsp[-1].blk))) {
    FAIL((yyloc), "Module metadata must be constant");
    (yyval.blk) = gen_noop();
    block_free((yyvsp[-1].blk));
  } else {
    (yyval.blk) = gen_module((yyvsp[-1].blk));
  }
}
#line 2743 "src/parser.c"
    break;

  case 6: /* Imports: %empty  */
#line 330 "src/parser.y"
       {
  (yyval.blk) = gen_noop();
}
#line 2751 "src/parser.c"
    break;

  case 7: /* Imports: Import Imports  */
#line 333 "src/parser.y"
               {
  (yyval.blk) = BLOCK((yyvsp[-1].blk), (yyvsp[0].blk));
}
#line 2759 "src/parser.c"
    break;

  case 8: /* FuncDefs: %empty  */
#line 338 "src/parser.y"
       {
  (yyval.blk) = gen_noop();
}
#line 2767 "src/parser.c"
    break;

  case 9: /* FuncDefs: FuncDef FuncDefs  */
#line 341 "src/parser.y"
                 {
  (yyval.blk) = block_join((yyvsp[-1].blk), (yyvsp[0].blk));
}
#line 2775 "src/parser.c"
    break;

  case 10: /* Exp: FuncDef Exp  */
#line 346 "src/parser.y"
                          {
  (yyval.blk) = block_bind_referenced((yyvsp[-1].blk), (yyvsp[0].blk), OP_IS_CALL_PSEUDO);
}
#line 2783 "src/parser.c"
    break;

  case 11: /* Exp: Term "as" Patterns '|' Exp  */
#line 350 "src/parser.y"
                           {
  (yyval.blk) = gen_destructure((yyvsp[-4].blk), (yyvsp[-2].blk), (yyvsp[0].blk));
}
#line 2791 "src/parser.c"
    break;

  case 12: /* Exp: "reduce" Term "as" Patterns '(' Exp ';' Exp ')'  */
#line 353 "src/parser.y"
                                                {
  (yyval.blk) = gen_reduce((yyvsp[-7].blk), (yyvsp[-5].blk), (yyvsp[-3].blk), (yyvsp[-1].blk));
}
#line 2799 "src/parser.c"
    break;

  case 13: /* Exp: "foreach" Term "as" Patterns '(' Exp ';' Exp ';' Exp ')'  */
#line 357 "src/parser.y"
                                                         {
  (yyval.blk) = gen_foreach((yyvsp[-9].blk), (yyvsp[-7].blk), (yyvsp[-5].blk), (yyvsp[-3].blk), (yyvsp[-1].blk));
}
#line 2807 "src/parser.c"
    break;

  case 14: /* Exp: "foreach" Term "as" Patterns '(' Exp ';' Exp ')'  */
#line 361 "src/parser.y"
                                                 {
  (yyval.blk) = gen_foreach((yyvsp[-7].blk), (yyvsp[-5].blk), (yyvsp[-3].blk), (yyvsp[-1].blk), gen_noop());
}
#line 2815 "src/parser.c"
    break;

  case 15: /* Exp: "if" Exp "then" Exp ElseBody  */
#line 365 "src/parser.y"
                             {
  (yyval.blk) = gen_cond((yyvsp[-3].blk), (yyvsp[-1].blk), (yyvsp[0].blk));
}
#line 2823 "src/parser.c"
    break;

  case 16: /* Exp: "if" Exp "then" Exp "end"  */
#line 368 "src/parser.y"
                          {
  (yyval.blk) = gen_cond((yyvsp[-3].blk), (yyvsp[-1].blk), gen_noop());
}
#line 2831 "src/parser.c"
    break;

  case 17: /* Exp: "if" Exp "then" error  */
#line 371 "src/parser.y"
                      {
  FAIL((yyloc), "Possibly unterminated 'if' statement");
  (yyval.blk) = (yyvsp[-2].blk);
}
#line 2840 "src/parser.c"
    break;

  case 18: /* Exp: "try" Exp "catch" Exp  */
#line 376 "src/parser.y"
                      {
  //$$ = BLOCK(gen_op_target(FORK_OPT, $2), $2, $4);
  (yyval.blk) = gen_try((yyvsp[-2].blk), gen_try_handler((yyvsp[0].blk)));
}
#line 2849 "src/parser.c"
    break;

  case 19: /* Exp: "try" Exp  */
#line 380 "src/parser.y"
          {
  //$$ = BLOCK(gen_op_target(FORK_OPT, $2), $2, gen_op_simple(BACKTRACK));
  (yyval.blk) = gen_try((yyvsp[0].blk), gen_op_simple(BACKTRACK));
}
#line 2858 "src/parser.c"
    break;

  case 20: /* Exp: "try" Exp "catch" error  */
#line 384 "src/parser.y"
                        {
  FAIL((yyloc), "Possibly unterminated 'try' statement");
  (yyval.blk) = (yyvsp[-2].blk);
}
#line 2867 "src/parser.c"
    break;

  case 21: /* Exp: "label" '$' IDENT '|' Exp  */
#line 389 "src/parser.y"
                          {
  jv v = jv_string_fmt("*label-%s", jv_string_value((yyvsp[-2].literal)));
  (yyval.blk) = gen_location((yyloc), locations, gen_label(jv_string_value(v), (yyvsp[0].blk)));
  jv_free((yyvsp[-2].literal));
  jv_free(v);
}
#line 2878 "src/parser.c"
    break;

  case 22: /* Exp: Exp '?'  */
#line 396 "src/parser.y"
        {
  (yyval.blk) = gen_try((yyvsp[-1].blk), gen_op_simple(BACKTRACK));
}
#line 2886 "src/parser.c"
    break;

  case 23: /* Exp: Exp '=' Exp  */
#line 400 "src/parser.y"
            {
  (yyval.blk) = gen_call("_assign", BLOCK(gen_lambda((yyvsp[-2].blk)), gen_lambda((yyvsp[0].blk))));
}
#line 2894 "src/parser.c"
    break;

  case 24: /* Exp: Exp "or" Exp  */
#line 404 "src/parser.y"
             {
  (yyval.blk) = gen_or((yyvsp[-2].blk), (yyvsp[0].blk));
}
#line 2902 "src/parser.c"
    break;

  case 25: /* Exp: Exp "and" Exp  */
#line 408 "src/parser.y"
              {
  (yyval.blk) = gen_and((yyvsp[-2].blk), (yyvsp[0].blk));
}
#line 2910 "src/parser.c"
    break;

  case 26: /* Exp: Exp "//" Exp  */
#line 412 "src/parser.y"
             {
  (yyval.blk) = gen_definedor((yyvsp[-2].blk), (yyvsp[0].blk));
}
#line 2918 "src/parser.c"
    break;

  case 27: /* Exp: Exp "//=" Exp  */
#line 416 "src/parser.y"
              {
  (yyval.blk) = gen_definedor_assign((yyvsp[-2].blk), (yyvsp[0].blk));
}
#line 2926 "src/parser.c"
    break;

  case 28: /* Exp: Exp "|=" Exp  */
#line 420 "src/parser.y"
             {
  (yyval.blk) = gen_call("_modify", BLOCK(gen_lambda((yyvsp[-2].blk)), gen_lambda((yyvsp[0].blk))));
}
#line 2934 "src/parser.c"
    break;

  case 29: /* Exp: Exp '|' Exp  */
#line 424 "src/parser.y"
            {
  (yyval.blk) = block_join((yyvsp[-2].blk), (yyvsp[0].blk));
}
#line 2942 "src/parser.c"
    break;

  case 30: /* Exp: Exp ',' Exp  */
#line 428 "src/parser.y"
            {
  (yyval.blk) = gen_both((yyvsp[-2].blk), (yyvsp[0].blk));
}
#line 2950 "src/parser.c"
    break;

  case 31: /* Exp: Exp '+' Exp  */
#line 432 "src/parser.y"
            {
  (yyval.blk) = gen_binop((yyvsp[-2].blk), (yyvsp[0].blk), '+');
}
#line 2958 "src/parser.c"
    break;

  case 32: /* Exp: Exp "+=" Exp  */
#line 436 "src/parser.y"
             {
  (yyval.blk) = gen_update((yyvsp[-2].blk), (yyvsp[0].blk), '+');
}
#line 2966 "src/parser.c"
    break;

  case 33: /* Exp: '-' Exp  */
#line 440 "src/parser.y"
        {
  (yyval.blk) = BLOCK((yyvsp[0].blk), gen_call("_negate", gen_noop()));
}
#line 2974 "src/parser.c"
    break;

  case 34: /* Exp: Exp '-' Exp  */
#line 444 "src/parser.y"
            {
  (yyval.blk) = gen_binop((yyvsp[-2].blk), (yyvsp[0].blk), '-');
}
#line 2982 "src/parser.c"
    break;

  case 35: /* Exp: Exp "-=" Exp  */
#line 448 "src/parser.y"
             {
  (yyval.blk) = gen_update((yyvsp[-2].blk), (yyvsp[0].blk), '-');
}
#line 2990 "src/parser.c"
    break;

  case 36: /* Exp: Exp '*' Exp  */
#line 452 "src/parser.y"
            {
  (yyval.blk) = gen_binop((yyvsp[-2].blk), (yyvsp[0].blk), '*');
}
#line 2998 "src/parser.c"
    break;

  case 37: /* Exp: Exp "*=" Exp  */
#line 456 "src/parser.y"
             {
  (yyval.blk) = gen_update((yyvsp[-2].blk), (yyvsp[0].blk), '*');
}
#line 3006 "src/parser.c"
    break;

  case 38: /* Exp: Exp '/' Exp  */
#line 460 "src/parser.y"
            {
  (yyval.blk) = gen_binop((yyvsp[-2].blk), (yyvsp[0].blk), '/');
  if (block_is_const_inf((yyval.blk)))
    FAIL((yyloc), "Division by zero?");
}
#line 3016 "src/parser.c"
    break;

  case 39: /* Exp: Exp '%' Exp  */
#line 466 "src/parser.y"
            {
  (yyval.blk) = gen_binop((yyvsp[-2].blk), (yyvsp[0].blk), '%');
  if (block_is_const_inf((yyval.blk)))
    FAIL((yyloc), "Remainder by zero?");
}
#line 3026 "src/parser.c"
    break;

  case 40: /* Exp: Exp "/=" Exp  */
#line 472 "src/parser.y"
             {
  (yyval.blk) = gen_update((yyvsp[-2].blk), (yyvsp[0].blk), '/');
}
#line 3034 "src/parser.c"
    break;

  case 41: /* Exp: Exp "%=" Exp  */
#line 476 "src/parser.y"
               {
  (yyval.blk) = gen_update((yyvsp[-2].blk), (yyvsp[0].blk), '%');
}
#line 3042 "src/parser.c"
    break;

  case 42: /* Exp: Exp "==" Exp  */
#line 480 "src/parser.y"
             {
  (yyval.blk) = gen_binop((yyvsp[-2].blk), (yyvsp[0].blk), EQ);
}
#line 3050 "src/parser.c"
    break;

  case 43: /* Exp: Exp "!=" Exp  */
#line 484 "src/parser.y"
             {
  (yyval.blk) = gen_binop((yyvsp[-2].blk), (yyvsp[0].blk), NEQ);
}
#line 3058 "src/parser.c"
    break;

  case 44: /* Exp: Exp '<' Exp  */
#line 488 "src/parser.y"
            {
  (yyval.blk) = gen_binop((yyvsp[-2].blk), (yyvsp[0].blk), '<');
}
#line 3066 "src/parser.c"
    break;

  case 45: /* Exp: Exp '>' Exp  */
#line 492 "src/parser.y"
            {
  (yyval.blk) = gen_binop((yyvsp[-2].blk), (yyvsp[0].blk), '>');
}
#line 3074 "src/parser.c"
    break;

  case 46: /* Exp: Exp "<=" Exp  */
#line 496 "src/parser.y"
             {
  (yyval.blk) = gen_binop((yyvsp[-2].blk), (yyvsp[0].blk), LESSEQ);
}
#line 3082 "src/parser.c"
    break;

  case 47: /* Exp: Exp ">=" Exp  */
#line 500 "src/parser.y"
             {
  (yyval.blk) = gen_binop((yyvsp[-2].blk), (yyvsp[0].blk), GREATEREQ);
}
#line 3090 "src/parser.c"
    break;

  case 48: /* Exp: '!' Exp  */
#line 504 "src/parser.y"
        {
  (yyval.blk) = BLOCK((yyvsp[0].blk), gen_call("_not", gen_noop()));
}
#line 3098 "src/parser.c"
    break;

  case 49: /* Exp: Term  */
#line 508 "src/parser.y"
     {
  (yyval.blk) = (yyvsp[0].blk);
}
#line 3106 "src/parser.c"
    break;

  case 50: /* Import: ImportWhat ';'  */
#line 513 "src/parser.y"
               {
  (yyval.blk) = (yyvsp[-1].blk);
}
#line 3114 "src/parser.c"
    break;

  case 51: /* Import: ImportWhat Exp ';'  */
#line 516 "src/parser.y"
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
#line 3134 "src/parser.c"
    break;

  case 52: /* ImportWhat: "import" ImportFrom "as" '$' IDENT  */
#line 533 "src/parser.y"
                                   {
  jv v = block_const((yyvsp[-3].blk));
  // XXX Make gen_import take only blocks and the int is_data so we
  // don't have to free so much stuff here
  (yyval.blk) = gen_import(jv_string_value(v), jv_string_value((yyvsp[0].literal)), 1);
  block_free((yyvsp[-3].blk));
  jv_free((yyvsp[0].literal));
  jv_free(v);
}
#line 3148 "src/parser.c"
    break;

  case 53: /* ImportWhat: "import" ImportFrom "as" IDENT  */
#line 542 "src/parser.y"
                               {
  jv v = block_const((yyvsp[-2].blk));
  (yyval.blk) = gen_import(jv_string_value(v), jv_string_value((yyvsp[0].literal)), 0);
  block_free((yyvsp[-2].blk));
  jv_free((yyvsp[0].literal));
  jv_free(v);
}
#line 3160 "src/parser.c"
    break;

  case 54: /* ImportWhat: "include" ImportFrom  */
#line 549 "src/parser.y"
                     {
  jv v = block_const((yyvsp[0].blk));
  (yyval.blk) = gen_import(jv_string_value(v), NULL, 0);
  block_free((yyvsp[0].blk));
  jv_free(v);
}
#line 3171 "src/parser.c"
    break;

  case 55: /* ImportFrom: String  */
#line 557 "src/parser.y"
       {
  if (!block_is_const((yyvsp[0].blk))) {
    FAIL((yyloc), "Import path must be constant");
    (yyval.blk) = gen_const(jv_string(""));
    block_free((yyvsp[0].blk));
  } else {
    (yyval.blk) = (yyvsp[0].blk);
  }
}
#line 3185 "src/parser.c"
    break;

  case 56: /* FuncDef: "def" IDENT ':' Exp ';'  */
#line 568 "src/parser.y"
                        {
  (yyval.blk) = gen_function(jv_string_value((yyvsp[-3].literal)), gen_noop(), (yyvsp[-1].blk));
  jv_free((yyvsp[-3].literal));
}
#line 3194 "src/parser.c"
    break;

  case 57: /* FuncDef: "def" IDENT '(' Params ')' ':' Exp ';'  */
#line 573 "src/parser.y"
                                       {
  (yyval.blk) = gen_function(jv_string_value((yyvsp[-6].literal)), (yyvsp[-4].blk), (yyvsp[-1].blk));
  jv_free((yyvsp[-6].literal));
}
#line 3203 "src/parser.c"
    break;

  case 58: /* Params: Param  */
#line 579 "src/parser.y"
      {
  (yyval.blk) = (yyvsp[0].blk);
}
#line 3211 "src/parser.c"
    break;

  case 59: /* Params: Params ';' Param  */
#line 582 "src/parser.y"
                 {
  (yyval.blk) = BLOCK((yyvsp[-2].blk), (yyvsp[0].blk));
}
#line 3219 "src/parser.c"
    break;

  case 60: /* Param: '$' IDENT  */
#line 587 "src/parser.y"
          {
  (yyval.blk) = gen_param_regular(jv_string_value((yyvsp[0].literal)));
  jv_free((yyvsp[0].literal));
}
#line 3228 "src/parser.c"
    break;

  case 61: /* Param: '$' Keyword  */
#line 591 "src/parser.y"
            {
  (yyval.blk) = gen_param_regular(jv_string_value((yyvsp[0].literal)));
  jv_free((yyvsp[0].literal));
}
#line 3237 "src/parser.c"
    break;

  case 62: /* Param: IDENT  */
#line 595 "src/parser.y"
      {
  (yyval.blk) = gen_param(jv_string_value((yyvsp[0].literal)));
  jv_free((yyvsp[0].literal));
}
#line 3246 "src/parser.c"
    break;

  case 63: /* @1: %empty  */
#line 602 "src/parser.y"
               { (yyval.literal) = jv_string("text"); }
#line 3252 "src/parser.c"
    break;

  case 64: /* String: QQSTRING_START @1 QQString QQSTRING_END  */
#line 602 "src/parser.y"
                                                                          {
  (yyval.blk) = (yyvsp[-1].blk);
  jv_free((yyvsp[-2].literal));
}
#line 3261 "src/parser.c"
    break;

  case 65: /* @2: %empty  */
#line 606 "src/parser.y"
                      { (yyval.literal) = (yyvsp[-1].literal); }
#line 3267 "src/parser.c"
    break;

  case 66: /* String: FORMAT QQSTRING_START @2 QQString QQSTRING_END  */
#line 606 "src/parser.y"
                                                                  {
  (yyval.blk) = (yyvsp[-1].blk);
  jv_free((yyvsp[-2].literal));
}
#line 3276 "src/parser.c"
    break;

  case 67: /* QQString: %empty  */
#line 613 "src/parser.y"
       {
  (yyval.blk) = gen_const(jv_string(""));
}
#line 3284 "src/parser.c"
    break;

  case 68: /* QQString: QQString QQSTRING_TEXT  */
#line 616 "src/parser.y"
                       {
  (yyval.blk) = gen_binop((yyvsp[-1].blk), gen_const((yyvsp[0].literal)), '+');
}
#line 3292 "src/parser.c"
    break;

  case 69: /* QQString: QQString QQSTRING_INTERP_START Exp QQSTRING_INTERP_END  */
#line 619 "src/parser.y"
                                                       {
  (yyval.blk) = gen_binop((yyvsp[-3].blk), gen_format((yyvsp[-1].blk), jv_copy((yyvsp[-4].literal))), '+');
}
#line 3300 "src/parser.c"
    break;

  case 70: /* ElseBody: "elif" Exp "then" Exp ElseBody  */
#line 625 "src/parser.y"
                               {
  (yyval.blk) = gen_cond((yyvsp[-3].blk), (yyvsp[-1].blk), (yyvsp[0].blk));
}
#line 3308 "src/parser.c"
    break;

  case 71: /* ElseBody: "else" Exp "end"  */
#line 628 "src/parser.y"
                 {
  (yyval.blk) = (yyvsp[-1].blk);
}
#line 3316 "src/parser.c"
    break;

  case 72: /* ExpD: ExpD '|' ExpD  */
#line 633 "src/parser.y"
              {
  (yyval.blk) = block_join((yyvsp[-2].blk), (yyvsp[0].blk));
}
#line 3324 "src/parser.c"
    break;

  case 73: /* ExpD: '-' ExpD  */
#line 636 "src/parser.y"
         {
  (yyval.blk) = BLOCK((yyvsp[0].blk), gen_call("_negate", gen_noop()));
}
#line 3332 "src/parser.c"
    break;

  case 74: /* ExpD: Term  */
#line 639 "src/parser.y"
     {
  (yyval.blk) = (yyvsp[0].blk);
}
#line 3340 "src/parser.c"
    break;

  case 75: /* Term: '.'  */
#line 645 "src/parser.y"
    {
  (yyval.blk) = gen_noop();
}
#line 3348 "src/parser.c"
    break;

  case 76: /* Term: ".."  */
#line 648 "src/parser.y"
    {
  (yyval.blk) = gen_call("recurse", gen_noop());
}
#line 3356 "src/parser.c"
    break;

  case 77: /* Term: "break" '$' IDENT  */
#line 651 "src/parser.y"
                {
  jv v = jv_string_fmt("*label-%s", jv_string_value((yyvsp[0].literal)));     // impossible symbol
  (yyval.blk) = gen_location((yyloc), locations,
                    BLOCK(gen_op_unbound(LOADV, jv_string_value(v)),
                    gen_call("error", gen_noop())));
  jv_free(v);
  jv_free((yyvsp[0].literal));
}
#line 3369 "src/parser.c"
    break;

  case 78: /* Term: "break" error  */
#line 659 "src/parser.y"
            {
  FAIL((yyloc), "break requires a label to break to");
  (yyval.blk) = gen_noop();
}
#line 3378 "src/parser.c"
    break;

  case 79: /* Term: Term FIELD '?'  */
#line 663 "src/parser.y"
               {
  (yyval.blk) = gen_index_opt((yyvsp[-2].blk), gen_const((yyvsp[-1].literal)));
}
#line 3386 "src/parser.c"
    break;

  case 80: /* Term: FIELD '?'  */
#line 666 "src/parser.y"
          {
  (yyval.blk) = gen_index_opt(gen_noop(), gen_const((yyvsp[-1].literal)));
}
#line 3394 "src/parser.c"
    break;

  case 81: /* Term: Term '.' String '?'  */
#line 669 "src/parser.y"
                    {
  (yyval.blk) = gen_index_opt((yyvsp[-3].blk), (yyvsp[-1].blk));
}
#line 3402 "src/parser.c"
    break;

  case 82: /* Term: '.' String '?'  */
#line 672 "src/parser.y"
               {
  (yyval.blk) = gen_index_opt(gen_noop(), (yyvsp[-1].blk));
}
#line 3410 "src/parser.c"
    break;

  case 83: /* Term: Term FIELD  */
#line 675 "src/parser.y"
                        {
  (yyval.blk) = gen_index((yyvsp[-1].blk), gen_const((yyvsp[0].literal)));
}
#line 3418 "src/parser.c"
    break;

  case 84: /* Term: FIELD  */
#line 678 "src/parser.y"
                   {
  (yyval.blk) = gen_index(gen_noop(), gen_const((yyvsp[0].literal)));
}
#line 3426 "src/parser.c"
    break;

  case 85: /* Term: Term '.' String  */
#line 681 "src/parser.y"
                             {
  (yyval.blk) = gen_index((yyvsp[-2].blk), (yyvsp[0].blk));
}
#line 3434 "src/parser.c"
    break;

  case 86: /* Term: '.' String  */
#line 684 "src/parser.y"
                        {
  (yyval.blk) = gen_index(gen_noop(), (yyvsp[0].blk));
}
#line 3442 "src/parser.c"
    break;

  case 87: /* Term: '.' error  */
#line 687 "src/parser.y"
          {
  FAIL((yyloc), "try .[\"field\"] instead of .field for unusually named fields");
  (yyval.blk) = gen_noop();
}
#line 3451 "src/parser.c"
    break;

  case 88: /* Term: '.' IDENT error  */
#line 691 "src/parser.y"
                {
  jv_free((yyvsp[-1].literal));
  FAIL((yyloc), "try .[\"field\"] instead of .field for unusually named fields");
  (yyval.blk) = gen_noop();
}
#line 3461 "src/parser.c"
    break;

  case 89: /* Term: Term '[' Exp ']' '?'  */
#line 697 "src/parser.y"
                     {
  (yyval.blk) = gen_index_opt((yyvsp[-4].blk), (yyvsp[-2].blk));
}
#line 3469 "src/parser.c"
    break;

  case 90: /* Term: Term '[' Exp ']'  */
#line 700 "src/parser.y"
                              {
  (yyval.blk) = gen_index((yyvsp[-3].blk), (yyvsp[-1].blk));
}
#line 3477 "src/parser.c"
    break;

  case 91: /* Term: Term '.' '[' Exp ']' '?'  */
#line 703 "src/parser.y"
                         {
  (yyval.blk) = gen_index_opt((yyvsp[-5].blk), (yyvsp[-2].blk));
}
#line 3485 "src/parser.c"
    break;

  case 92: /* Term: Term '.' '[' Exp ']'  */
#line 706 "src/parser.y"
                                  {
  (yyval.blk) = gen_index((yyvsp[-4].blk), (yyvsp[-1].blk));
}
#line 3493 "src/parser.c"
    break;

  case 93: /* Term: Term '[' ']' '?'  */
#line 709 "src/parser.y"
                 {
  (yyval.blk) = block_join((yyvsp[-3].blk), gen_op_simple(EACH_OPT));
}
#line 3501 "src/parser.c"
    break;

  case 94: /* Term: Term '[' ']'  */
#line 712 "src/parser.y"
                          {
  (yyval.blk) = block_join((yyvsp[-2].blk), gen_op_simple(EACH));
}
#line 3509 "src/parser.c"
    break;

  case 95: /* Term: Term '[' Exp ':' Exp ']' '?'  */
#line 715 "src/parser.y"
                             {
  (yyval.blk) = gen_slice_index((yyvsp[-6].blk), (yyvsp[-4].blk), (yyvsp[-2].blk), INDEX_OPT);
}
#line 3517 "src/parser.c"
    break;

  case 96: /* Term: Term '[' Exp ':' ']' '?'  */
#line 718 "src/parser.y"
                         {
  (yyval.blk) = gen_slice_index((yyvsp[-5].blk), (yyvsp[-3].blk), gen_const(jv_null()), INDEX_OPT);
}
#line 3525 "src/parser.c"
    break;

  case 97: /* Term: Term '[' ':' Exp ']' '?'  */
#line 721 "src/parser.y"
                         {
  (yyval.blk) = gen_slice_index((yyvsp[-5].blk), gen_const(jv_null()), (yyvsp[-2].blk), INDEX_OPT);
}
#line 3533 "src/parser.c"
    break;

  case 98: /* Term: Term '[' Exp ':' Exp ']'  */
#line 724 "src/parser.y"
                                      {
  (yyval.blk) = gen_slice_index((yyvsp[-5].blk), (yyvsp[-3].blk), (yyvsp[-1].blk), INDEX);
}
#line 3541 "src/parser.c"
    break;

  case 99: /* Term: Term '[' Exp ':' ']'  */
#line 727 "src/parser.y"
                                  {
  (yyval.blk) = gen_slice_index((yyvsp[-4].blk), (yyvsp[-2].blk), gen_const(jv_null()), INDEX);
}
#line 3549 "src/parser.c"
    break;

  case 100: /* Term: Term '[' ':' Exp ']'  */
#line 730 "src/parser.y"
                                  {
  (yyval.blk) = gen_slice_index((yyvsp[-4].blk), gen_const(jv_null()), (yyvsp[-1].blk), INDEX);
}
#line 3557 "src/parser.c"
    break;

  case 101: /* Term: LITERAL  */
#line 733 "src/parser.y"
        {
  (yyval.blk) = gen_const((yyvsp[0].literal));
}
#line 3565 "src/parser.c"
    break;

  case 102: /* Term: String  */
#line 736 "src/parser.y"
       {
  (yyval.blk) = (yyvsp[0].blk);
}
#line 3573 "src/parser.c"
    break;

  case 103: /* Term: FORMAT  */
#line 739 "src/parser.y"
       {
  (yyval.blk) = gen_format(gen_noop(), (yyvsp[0].literal));
}
#line 3581 "src/parser.c"
    break;

  case 104: /* Term: '(' Exp ')'  */
#line 742 "src/parser.y"
            {
  (yyval.blk) = (yyvsp[-1].blk);
}
#line 3589 "src/parser.c"
    break;

  case 105: /* Term: '[' Exp ']'  */
#line 745 "src/parser.y"
            {
  (yyval.blk) = gen_collect((yyvsp[-1].blk));
}
#line 3597 "src/parser.c"
    break;

  case 106: /* Term: '[' ']'  */
#line 748 "src/parser.y"
        {
  (yyval.blk) = gen_const(jv_array());
}
#line 3605 "src/parser.c"
    break;

  case 107: /* Term: '{' MkDict '}'  */
#line 751 "src/parser.y"
               {
  block o = gen_const_object((yyvsp[-1].blk));
  if (o.first != NULL)
    (yyval.blk) = o;
  else
    (yyval.blk) = BLOCK(gen_subexp(gen_const(jv_object())), (yyvsp[-1].blk), gen_op_simple(POP));
}
#line 3617 "src/parser.c"
    break;

  case 108: /* Term: '$' '$' '$' '$' IDENT  */
#line 773 "src/parser.y"
                      {
  (yyval.blk) = gen_location((yyloc), locations, gen_op_unbound(LOADVN, jv_string_value((yyvsp[0].literal))));
  jv_free((yyvsp[0].literal));
}
#line 3626 "src/parser.c"
    break;

  case 109: /* Term: '$' IDENT  */
#line 777 "src/parser.y"
          {
  (yyval.blk) = gen_location((yyloc), locations, gen_op_unbound(LOADV, jv_string_value((yyvsp[0].literal))));
  jv_free((yyvsp[0].literal));
}
#line 3635 "src/parser.c"
    break;

  case 110: /* Term: '$' Keyword  */
#line 781 "src/parser.y"
            {
  if (strcmp(jv_string_value((yyvsp[0].literal)), "__loc__") == 0) {
    (yyval.blk) = gen_const(JV_OBJECT(jv_string("file"), jv_copy(locations->fname),
                             jv_string("line"), jv_number(locfile_get_line(locations, (yyloc).start) + 1)));
  } else {
    (yyval.blk) = gen_location((yyloc), locations, gen_op_unbound(LOADV, jv_string_value((yyvsp[0].literal))));
  }
  jv_free((yyvsp[0].literal));
}
#line 3649 "src/parser.c"
    break;

  case 111: /* Term: IDENT  */
#line 790 "src/parser.y"
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
#line 3666 "src/parser.c"
    break;

  case 112: /* Term: IDENT '(' Args ')'  */
#line 802 "src/parser.y"
                   {
  (yyval.blk) = gen_call(jv_string_value((yyvsp[-3].literal)), (yyvsp[-1].blk));
  (yyval.blk) = gen_location((yylsp[-3]), locations, (yyval.blk));
  jv_free((yyvsp[-3].literal));
}
#line 3676 "src/parser.c"
    break;

  case 113: /* Term: '(' error ')'  */
#line 807 "src/parser.y"
              { (yyval.blk) = gen_noop(); }
#line 3682 "src/parser.c"
    break;

  case 114: /* Term: '[' error ']'  */
#line 808 "src/parser.y"
              { (yyval.blk) = gen_noop(); }
#line 3688 "src/parser.c"
    break;

  case 115: /* Term: Term '[' error ']'  */
#line 809 "src/parser.y"
                   { (yyval.blk) = (yyvsp[-3].blk); }
#line 3694 "src/parser.c"
    break;

  case 116: /* Term: '{' error '}'  */
#line 810 "src/parser.y"
              { (yyval.blk) = gen_noop(); }
#line 3700 "src/parser.c"
    break;

  case 117: /* Args: Arg  */
#line 813 "src/parser.y"
    {
  (yyval.blk) = (yyvsp[0].blk);
}
#line 3708 "src/parser.c"
    break;

  case 118: /* Args: Args ';' Arg  */
#line 816 "src/parser.y"
             {
  (yyval.blk) = BLOCK((yyvsp[-2].blk), (yyvsp[0].blk));
}
#line 3716 "src/parser.c"
    break;

  case 119: /* Arg: Exp  */
#line 821 "src/parser.y"
    {
  (yyval.blk) = gen_lambda((yyvsp[0].blk));
}
#line 3724 "src/parser.c"
    break;

  case 120: /* RepPatterns: RepPatterns "?//" Pattern  */
#line 826 "src/parser.y"
                          {
  (yyval.blk) = BLOCK((yyvsp[-2].blk), gen_destructure_alt((yyvsp[0].blk)));
}
#line 3732 "src/parser.c"
    break;

  case 121: /* RepPatterns: Pattern  */
#line 829 "src/parser.y"
        {
  (yyval.blk) = gen_destructure_alt((yyvsp[0].blk));
}
#line 3740 "src/parser.c"
    break;

  case 122: /* Patterns: RepPatterns "?//" Pattern  */
#line 834 "src/parser.y"
                          {
  (yyval.blk) = BLOCK((yyvsp[-2].blk), (yyvsp[0].blk));
}
#line 3748 "src/parser.c"
    break;

  case 123: /* Patterns: Pattern  */
#line 837 "src/parser.y"
        {
  (yyval.blk) = (yyvsp[0].blk);
}
#line 3756 "src/parser.c"
    break;

  case 124: /* Pattern: '$' IDENT  */
#line 842 "src/parser.y"
          {
  (yyval.blk) = gen_op_unbound(STOREV, jv_string_value((yyvsp[0].literal)));
  jv_free((yyvsp[0].literal));
}
#line 3765 "src/parser.c"
    break;

  case 125: /* Pattern: '[' ArrayPats ']'  */
#line 846 "src/parser.y"
                  {
  (yyval.blk) = BLOCK((yyvsp[-1].blk), gen_op_simple(POP));
}
#line 3773 "src/parser.c"
    break;

  case 126: /* Pattern: '{' ObjPats '}'  */
#line 849 "src/parser.y"
                {
  (yyval.blk) = BLOCK((yyvsp[-1].blk), gen_op_simple(POP));
}
#line 3781 "src/parser.c"
    break;

  case 127: /* ArrayPats: Pattern  */
#line 854 "src/parser.y"
        {
  (yyval.blk) = gen_array_matcher(gen_noop(), (yyvsp[0].blk));
}
#line 3789 "src/parser.c"
    break;

  case 128: /* ArrayPats: ArrayPats ',' Pattern  */
#line 857 "src/parser.y"
                      {
  (yyval.blk) = gen_array_matcher((yyvsp[-2].blk), (yyvsp[0].blk));
}
#line 3797 "src/parser.c"
    break;

  case 129: /* ObjPats: ObjPat  */
#line 862 "src/parser.y"
       {
  (yyval.blk) = (yyvsp[0].blk);
}
#line 3805 "src/parser.c"
    break;

  case 130: /* ObjPats: ObjPats ',' ObjPat  */
#line 865 "src/parser.y"
                   {
  (yyval.blk) = BLOCK((yyvsp[-2].blk), (yyvsp[0].blk));
}
#line 3813 "src/parser.c"
    break;

  case 131: /* ObjPat: '$' IDENT  */
#line 870 "src/parser.y"
          {
  (yyval.blk) = gen_object_matcher(gen_const((yyvsp[0].literal)), gen_op_unbound(STOREV, jv_string_value((yyvsp[0].literal))));
}
#line 3821 "src/parser.c"
    break;

  case 132: /* ObjPat: '$' IDENT ':' Pattern  */
#line 873 "src/parser.y"
                      {
  (yyval.blk) = gen_object_matcher(gen_const((yyvsp[-2].literal)), BLOCK(gen_op_simple(DUP), gen_op_unbound(STOREV, jv_string_value((yyvsp[-2].literal))), (yyvsp[0].blk)));
}
#line 3829 "src/parser.c"
    break;

  case 133: /* ObjPat: IDENT ':' Pattern  */
#line 876 "src/parser.y"
                  {
  (yyval.blk) = gen_object_matcher(gen_const((yyvsp[-2].literal)), (yyvsp[0].blk));
}
#line 3837 "src/parser.c"
    break;

  case 134: /* ObjPat: Keyword ':' Pattern  */
#line 879 "src/parser.y"
                    {
  (yyval.blk) = gen_object_matcher(gen_const((yyvsp[-2].literal)), (yyvsp[0].blk));
}
#line 3845 "src/parser.c"
    break;

  case 135: /* ObjPat: String ':' Pattern  */
#line 882 "src/parser.y"
                   {
  (yyval.blk) = gen_object_matcher((yyvsp[-2].blk), (yyvsp[0].blk));
}
#line 3853 "src/parser.c"
    break;

  case 136: /* ObjPat: '(' Exp ')' ':' Pattern  */
#line 885 "src/parser.y"
                        {
  jv msg = check_object_key((yyvsp[-3].blk));
  if (jv_is_valid(msg)) {
    FAIL((yyloc), jv_string_value(msg));
  }
  jv_free(msg);
  (yyval.blk) = gen_object_matcher((yyvsp[-3].blk), (yyvsp[0].blk));
}
#line 3866 "src/parser.c"
    break;

  case 137: /* ObjPat: error ':' Pattern  */
#line 893 "src/parser.y"
                  {
  FAIL((yyloc), "May need parentheses around object key expression");
  (yyval.blk) = (yyvsp[0].blk);
}
#line 3875 "src/parser.c"
    break;

  case 138: /* Keyword: "as"  */
#line 899 "src/parser.y"
     {
  (yyval.literal) = jv_string("as");
}
#line 3883 "src/parser.c"
    break;

  case 139: /* Keyword: "def"  */
#line 902 "src/parser.y"
      {
  (yyval.literal) = jv_string("def");
}
#line 3891 "src/parser.c"
    break;

  case 140: /* Keyword: "module"  */
#line 905 "src/parser.y"
         {
  (yyval.literal) = jv_string("module");
}
#line 3899 "src/parser.c"
    break;

  case 141: /* Keyword: "import"  */
#line 908 "src/parser.y"
         {
  (yyval.literal) = jv_string("import");
}
#line 3907 "src/parser.c"
    break;

  case 142: /* Keyword: "include"  */
#line 911 "src/parser.y"
          {
  (yyval.literal) = jv_string("include");
}
#line 3915 "src/parser.c"
    break;

  case 143: /* Keyword: "if"  */
#line 914 "src/parser.y"
     {
  (yyval.literal) = jv_string("if");
}
#line 3923 "src/parser.c"
    break;

  case 144: /* Keyword: "then"  */
#line 917 "src/parser.y"
       {
  (yyval.literal) = jv_string("then");
}
#line 3931 "src/parser.c"
    break;

  case 145: /* Keyword: "else"  */
#line 920 "src/parser.y"
       {
  (yyval.literal) = jv_string("else");
}
#line 3939 "src/parser.c"
    break;

  case 146: /* Keyword: "elif"  */
#line 923 "src/parser.y"
       {
  (yyval.literal) = jv_string("elif");
}
#line 3947 "src/parser.c"
    break;

  case 147: /* Keyword: "reduce"  */
#line 926 "src/parser.y"
         {
  (yyval.literal) = jv_string("reduce");
}
#line 3955 "src/parser.c"
    break;

  case 148: /* Keyword: "foreach"  */
#line 929 "src/parser.y"
          {
  (yyval.literal) = jv_string("foreach");
}
#line 3963 "src/parser.c"
    break;

  case 149: /* Keyword: "end"  */
#line 932 "src/parser.y"
      {
  (yyval.literal) = jv_string("end");
}
#line 3971 "src/parser.c"
    break;

  case 150: /* Keyword: "and"  */
#line 935 "src/parser.y"
      {
  (yyval.literal) = jv_string("and");
}
#line 3979 "src/parser.c"
    break;

  case 151: /* Keyword: "or"  */
#line 938 "src/parser.y"
     {
  (yyval.literal) = jv_string("or");
}
#line 3987 "src/parser.c"
    break;

  case 152: /* Keyword: "try"  */
#line 941 "src/parser.y"
      {
  (yyval.literal) = jv_string("try");
}
#line 3995 "src/parser.c"
    break;

  case 153: /* Keyword: "catch"  */
#line 944 "src/parser.y"
        {
  (yyval.literal) = jv_string("catch");
}
#line 4003 "src/parser.c"
    break;

  case 154: /* Keyword: "label"  */
#line 947 "src/parser.y"
        {
  (yyval.literal) = jv_string("label");
}
#line 4011 "src/parser.c"
    break;

  case 155: /* Keyword: "break"  */
#line 950 "src/parser.y"
        {
  (yyval.literal) = jv_string("break");
}
#line 4019 "src/parser.c"
    break;

  case 156: /* Keyword: "__loc__"  */
#line 953 "src/parser.y"
          {
  (yyval.literal) = jv_string("__loc__");
}
#line 4027 "src/parser.c"
    break;

  case 157: /* MkDict: %empty  */
#line 958 "src/parser.y"
       {
  (yyval.blk)=gen_noop();
}
#line 4035 "src/parser.c"
    break;

  case 158: /* MkDict: MkDictPair  */
#line 961 "src/parser.y"
            { (yyval.blk) = (yyvsp[0].blk); }
#line 4041 "src/parser.c"
    break;

  case 159: /* MkDict: MkDictPair ',' MkDict  */
#line 962 "src/parser.y"
                        { (yyval.blk)=block_join((yyvsp[-2].blk), (yyvsp[0].blk)); }
#line 4047 "src/parser.c"
    break;

  case 160: /* MkDict: error ',' MkDict  */
#line 963 "src/parser.y"
                   { (yyval.blk) = (yyvsp[0].blk); }
#line 4053 "src/parser.c"
    break;

  case 161: /* MkDictPair: IDENT ':' ExpD  */
#line 966 "src/parser.y"
               {
  (yyval.blk) = gen_dictpair(gen_const((yyvsp[-2].literal)), (yyvsp[0].blk));
 }
#line 4061 "src/parser.c"
    break;

  case 162: /* MkDictPair: Keyword ':' ExpD  */
#line 969 "src/parser.y"
                   {
  (yyval.blk) = gen_dictpair(gen_const((yyvsp[-2].literal)), (yyvsp[0].blk));
  }
#line 4069 "src/parser.c"
    break;

  case 163: /* MkDictPair: String ':' ExpD  */
#line 972 "src/parser.y"
                  {
  (yyval.blk) = gen_dictpair((yyvsp[-2].blk), (yyvsp[0].blk));
  }
#line 4077 "src/parser.c"
    break;

  case 164: /* MkDictPair: String  */
#line 975 "src/parser.y"
         {
  (yyval.blk) = gen_dictpair((yyvsp[0].blk), BLOCK(gen_op_simple(POP), gen_op_simple(DUP2),
                              gen_op_simple(DUP2), gen_op_simple(INDEX)));
  }
#line 4086 "src/parser.c"
    break;

  case 165: /* MkDictPair: '$' IDENT ':' ExpD  */
#line 979 "src/parser.y"
                     {
  (yyval.blk) = gen_dictpair(gen_location((yyloc), locations, gen_op_unbound(LOADV, jv_string_value((yyvsp[-2].literal)))),
                    (yyvsp[0].blk));
  }
#line 4095 "src/parser.c"
    break;

  case 166: /* MkDictPair: '$' IDENT  */
#line 983 "src/parser.y"
            {
  (yyval.blk) = gen_dictpair(gen_const((yyvsp[0].literal)),
                    gen_location((yyloc), locations, gen_op_unbound(LOADV, jv_string_value((yyvsp[0].literal)))));
  }
#line 4104 "src/parser.c"
    break;

  case 167: /* MkDictPair: '$' Keyword  */
#line 987 "src/parser.y"
              {
  (yyval.blk) = gen_dictpair(gen_const((yyvsp[0].literal)),
                    gen_location((yyloc), locations, gen_op_unbound(LOADV, jv_string_value((yyvsp[0].literal)))));
  }
#line 4113 "src/parser.c"
    break;

  case 168: /* MkDictPair: IDENT  */
#line 991 "src/parser.y"
        {
  (yyval.blk) = gen_dictpair(gen_const(jv_copy((yyvsp[0].literal))),
                    gen_index(gen_noop(), gen_const((yyvsp[0].literal))));
  }
#line 4122 "src/parser.c"
    break;

  case 169: /* MkDictPair: Keyword  */
#line 995 "src/parser.y"
          {
  (yyval.blk) = gen_dictpair(gen_const(jv_copy((yyvsp[0].literal))),
                    gen_index(gen_noop(), gen_const((yyvsp[0].literal))));
  }
#line 4131 "src/parser.c"
    break;

  case 170: /* MkDictPair: '(' Exp ')' ':' ExpD  */
#line 999 "src/parser.y"
                       {
  jv msg = check_object_key((yyvsp[-3].blk));
  if (jv_is_valid(msg)) {
    FAIL((yyloc), jv_string_value(msg));
  }
  jv_free(msg);
  (yyval.blk) = gen_dictpair((yyvsp[-3].blk), (yyvsp[0].blk));
  }
#line 4144 "src/parser.c"
    break;

  case 171: /* MkDictPair: error ':' ExpD  */
#line 1007 "src/parser.y"
                 {
  FAIL((yyloc), "May need parentheses around object key expression");
  (yyval.blk) = (yyvsp[0].blk);
  }
#line 4153 "src/parser.c"
    break;


#line 4157 "src/parser.c"

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

#line 1011 "src/parser.y"


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

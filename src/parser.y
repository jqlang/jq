%{
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "compile.h"
#include "jv_alloc.h"
#define YYMALLOC jv_mem_alloc
#define YYFREE jv_mem_free
%}
%code requires {
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
}

%locations
%error-verbose
%define api.pure
%union {
  jv literal;
  block blk;
}

%destructor { jv_free($$); } <literal>
%destructor { block_free($$); } <blk>

%parse-param {block* answer}
%parse-param {int* errors}
%parse-param {struct locfile* locations}
%parse-param {struct lexer_param* lexer_param_ptr}
%lex-param {block* answer}
%lex-param {int* errors}
%lex-param {struct locfile* locations}
%lex-param {struct lexer_param* lexer_param_ptr}


%token INVALID_CHARACTER
%token <literal> IDENT
%token <literal> FIELD
%token <literal> LITERAL
%token <literal> FORMAT
%token REC ".."
%token SETMOD "%="
%token EQ "=="
%token NEQ "!="
%token DEFINEDOR "//"
%token AS "as"
%token DEF "def"
%token MODULE "module"
%token IMPORT "import"
%token INCLUDE "include"
%token IF "if"
%token THEN "then"
%token ELSE "else"
%token ELSE_IF "elif"
%token REDUCE "reduce"
%token FOREACH "foreach"
%token END "end"
%token AND "and"
%token OR "or"
%token TRY "try"
%token CATCH "catch"
%token LABEL "label"
%token BREAK "break"
%token LOC "__loc__"
%token SETPIPE "|="
%token SETPLUS "+="
%token SETMINUS "-="
%token SETMULT "*="
%token SETDIV "/="
%token SETDEFINEDOR "//="
%token LESSEQ "<="
%token GREATEREQ ">="
%token ALTERNATION "?//"

%token QQSTRING_START
%token <literal> QQSTRING_TEXT
%token QQSTRING_INTERP_START
%token QQSTRING_INTERP_END
%token QQSTRING_END

/* Instead of raising this, find a way to use precedence to resolve
 * shift-reduce conflicts. */
%expect 0

%precedence FUNCDEF
%right '|'
%left ','
%right "//"
%nonassoc '=' SETPIPE SETPLUS SETMINUS SETMULT SETDIV SETMOD SETDEFINEDOR
%left OR
%left AND
%nonassoc NEQ EQ '<' '>' LESSEQ GREATEREQ
%left '+' '-'
%left '*' '/' '%'
%precedence NONOPT /* non-optional; rules for which a specialized
                      '?' rule should be preferred over Exp '?' */
%precedence '?'
%precedence "try"
%precedence "catch"


%type <blk> Exp Term
%type <blk> MkDict MkDictPair ExpD
%type <blk> ElseBody
%type <blk> String QQString
%type <blk> FuncDef FuncDefs
%type <blk> Module Import Imports ImportWhat ImportFrom
%type <blk> Param Params Arg Args
%type <blk> Patterns RepPatterns Pattern ArrayPats ObjPats ObjPat
%type <literal> Keyword
%{
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
        jv_dump_string_trunc(jv_copy(block_const(k)), errbuf, sizeof(errbuf)));
  }
  return jv_invalid();
}

static block jq_gen_index(block obj, block key) {
  return BLOCK(jq_gen_subexp(key), obj, jq_gen_op_simple(INDEX));
}

static block jq_gen_index_opt(block obj, block key) {
  return BLOCK(jq_gen_subexp(key), obj, jq_gen_op_simple(INDEX_OPT));
}

static block jq_gen_slice_index(block obj, block start, block end, opcode idx_op) {
  block key = BLOCK(jq_gen_subexp(jq_gen_const(jv_object())),
                    jq_gen_subexp(jq_gen_const(jv_string("start"))),
                    jq_gen_subexp(start),
                    jq_gen_op_simple(INSERT),
                    jq_gen_subexp(jq_gen_const(jv_string("end"))),
                    jq_gen_subexp(end),
                    jq_gen_op_simple(INSERT));
  return BLOCK(key, obj, jq_gen_op_simple(idx_op));
}

static block constant_fold(block a, block b, int op) {
  if (!block_is_single(a) || !block_is_const(a) ||
      !block_is_single(b) || !block_is_const(b))
    return jq_gen_noop();
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
    return jq_gen_noop();

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
    return jq_gen_noop();
  }

  if (jv_get_kind(res) == JV_KIND_INVALID)
    return jq_gen_noop();

  block_free(a);
  block_free(b);
  return jq_gen_const(res);
}

static block jq_gen_binop(block a, block b, int op) {
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

  return jq_gen_call(funcname, BLOCK(jq_gen_lambda(a), jq_gen_lambda(b)));
}

static block jq_gen_format(block a, jv fmt) {
  return BLOCK(a, jq_gen_call("format", jq_gen_lambda(jq_gen_const(fmt))));
}

static block jq_gen_definedor_assign(block object, block val) {
  block tmp = jq_gen_op_var_fresh(STOREV, "tmp");
  return BLOCK(jq_gen_op_simple(DUP),
               val, tmp,
               jq_gen_call("_modify", BLOCK(jq_gen_lambda(object),
                                         jq_gen_lambda(jq_gen_definedor(jq_gen_noop(),
                                                                  jq_gen_op_bound(LOADV, tmp))))));
}

static block jq_gen_update(block object, block val, int optype) {
  block tmp = jq_gen_op_var_fresh(STOREV, "tmp");
  return BLOCK(jq_gen_op_simple(DUP),
               val,
               tmp,
               jq_gen_call("_modify", BLOCK(jq_gen_lambda(object),
                                         jq_gen_lambda(jq_gen_binop(jq_gen_noop(),
                                                              jq_gen_op_bound(LOADV, tmp),
                                                              optype)))));
}

%}

%%
TopLevel:
Module Imports Exp {
  *answer = BLOCK($1, $2, jq_gen_op_simple(TOP), $3);
} |
Module Imports FuncDefs {
  *answer = BLOCK($1, $2, $3);
}

Module:
%empty {
  $$ = jq_gen_noop();
} |
"module" Exp ';' {
  if (!block_is_const($2)) {
    FAIL(@$, "Module metadata must be constant");
    $$ = jq_gen_noop();
    block_free($2);
  } else {
    $$ = jq_gen_module($2);
  }
}

Imports:
%empty {
  $$ = jq_gen_noop();
} |
Import Imports {
  $$ = BLOCK($1, $2);
}

FuncDefs:
%empty {
  $$ = jq_gen_noop();
} |
FuncDef FuncDefs {
  $$ = block_bind($1, $2, OP_IS_CALL_PSEUDO);
}

Exp:
FuncDef Exp %prec FUNCDEF {
  $$ = block_bind_referenced($1, $2, OP_IS_CALL_PSEUDO);
} |

Term "as" Patterns '|' Exp {
  $$ = jq_gen_destructure($1, $3, $5);
} |
"reduce" Term "as" Patterns '(' Exp ';' Exp ')' {
  $$ = jq_gen_reduce($2, $4, $6, $8);
} |

"foreach" Term "as" Patterns '(' Exp ';' Exp ';' Exp ')' {
  $$ = jq_gen_foreach($2, $4, $6, $8, $10);
} |

"foreach" Term "as" Patterns '(' Exp ';' Exp ')' {
  $$ = jq_gen_foreach($2, $4, $6, $8, jq_gen_noop());
} |

"if" Exp "then" Exp ElseBody {
  $$ = jq_gen_cond($2, $4, $5);
} |
"if" Exp "then" error {
  FAIL(@$, "Possibly unterminated 'if' statement");
  $$ = $2;
} |

"try" Exp "catch" Exp {
  //$$ = BLOCK(jq_gen_op_target(FORK_OPT, $2), $2, $4);
  $$ = jq_gen_try($2, jq_gen_try_handler($4));
} |
"try" Exp {
  //$$ = BLOCK(jq_gen_op_target(FORK_OPT, $2), $2, jq_gen_op_simple(BACKTRACK));
  $$ = jq_gen_try($2, jq_gen_op_simple(BACKTRACK));
} |
"try" Exp "catch" error {
  FAIL(@$, "Possibly unterminated 'try' statement");
  $$ = $2;
} |

"label" '$' IDENT '|' Exp {
  jv v = jv_string_fmt("*label-%s", jv_string_value($3));
  $$ = jq_gen_location(@$, locations, jq_gen_label(jv_string_value(v), $5));
  jv_free($3);
  jv_free(v);
} |

Exp '?' {
  $$ = jq_gen_try($1, jq_gen_op_simple(BACKTRACK));
} |

Exp '=' Exp {
  $$ = jq_gen_call("_assign", BLOCK(jq_gen_lambda($1), jq_gen_lambda($3)));
} |

Exp "or" Exp {
  $$ = jq_gen_or($1, $3);
} |

Exp "and" Exp {
  $$ = jq_gen_and($1, $3);
} |

Exp "//" Exp {
  $$ = jq_gen_definedor($1, $3);
} |

Exp "//=" Exp {
  $$ = jq_gen_definedor_assign($1, $3);
} |

Exp "|=" Exp {
  $$ = jq_gen_call("_modify", BLOCK(jq_gen_lambda($1), jq_gen_lambda($3)));
} |

Exp '|' Exp {
  $$ = block_join($1, $3);
} |

Exp ',' Exp {
  $$ = jq_gen_both($1, $3);
} |

Exp '+' Exp {
  $$ = jq_gen_binop($1, $3, '+');
} |

Exp "+=" Exp {
  $$ = jq_gen_update($1, $3, '+');
} |

'-' Exp {
  $$ = BLOCK($2, jq_gen_call("_negate", jq_gen_noop()));
} |

Exp '-' Exp {
  $$ = jq_gen_binop($1, $3, '-');
} |

Exp "-=" Exp {
  $$ = jq_gen_update($1, $3, '-');
} |

Exp '*' Exp {
  $$ = jq_gen_binop($1, $3, '*');
} |

Exp "*=" Exp {
  $$ = jq_gen_update($1, $3, '*');
} |

Exp '/' Exp {
  $$ = jq_gen_binop($1, $3, '/');
  if (block_is_const_inf($$))
    FAIL(@$, "Division by zero?");
} |

Exp '%' Exp {
  $$ = jq_gen_binop($1, $3, '%');
  if (block_is_const_inf($$))
    FAIL(@$, "Remainder by zero?");
} |

Exp "/=" Exp {
  $$ = jq_gen_update($1, $3, '/');
} |

Exp SETMOD Exp {
  $$ = jq_gen_update($1, $3, '%');
} |

Exp "==" Exp {
  $$ = jq_gen_binop($1, $3, EQ);
} |

Exp "!=" Exp {
  $$ = jq_gen_binop($1, $3, NEQ);
} |

Exp '<' Exp {
  $$ = jq_gen_binop($1, $3, '<');
} |

Exp '>' Exp {
  $$ = jq_gen_binop($1, $3, '>');
} |

Exp "<=" Exp {
  $$ = jq_gen_binop($1, $3, LESSEQ);
} |

Exp ">=" Exp {
  $$ = jq_gen_binop($1, $3, GREATEREQ);
} |

Term {
  $$ = $1;
}

Import:
ImportWhat ';' {
  $$ = $1;
} |
ImportWhat Exp ';' {
  if (!block_is_const($2)) {
    FAIL(@$, "Module metadata must be constant");
    $$ = jq_gen_noop();
    block_free($1);
    block_free($2);
  } else if (block_const_kind($2) != JV_KIND_OBJECT) {
    FAIL(@$, "Module metadata must be an object");
    $$ = jq_gen_noop();
    block_free($1);
    block_free($2);
  } else {
    $$ = jq_gen_import_meta($1, $2);
  }
}

ImportWhat:
"import" ImportFrom "as" '$' IDENT {
  jv v = block_const($2);
  // XXX Make jq_gen_import take only blocks and the int is_data so we
  // don't have to free so much stuff here
  $$ = jq_gen_import(jv_string_value(v), jv_string_value($5), 1);
  block_free($2);
  jv_free($5);
  jv_free(v);
} |
"import" ImportFrom "as" IDENT {
  jv v = block_const($2);
  $$ = jq_gen_import(jv_string_value(v), jv_string_value($4), 0);
  block_free($2);
  jv_free($4);
  jv_free(v);
} |
"include" ImportFrom {
  jv v = block_const($2);
  $$ = jq_gen_import(jv_string_value(v), NULL, 0);
  block_free($2);
  jv_free(v);
}

ImportFrom:
String {
  if (!block_is_const($1)) {
    FAIL(@$, "Import path must be constant");
    $$ = jq_gen_const(jv_string(""));
    block_free($1);
  } else {
    $$ = $1;
  }
}

FuncDef:
"def" IDENT ':' Exp ';' {
  $$ = jq_gen_function(jv_string_value($2), jq_gen_noop(), $4);
  jv_free($2);
} |

"def" IDENT '(' Params ')' ':' Exp ';' {
  $$ = jq_gen_function(jv_string_value($2), $4, $7);
  jv_free($2);
}

Params:
Param {
  $$ = $1;
} |
Params ';' Param {
  $$ = BLOCK($1, $3);
}

Param:
'$' IDENT {
  $$ = jq_gen_param_regular(jv_string_value($2));
  jv_free($2);
} |

IDENT {
  $$ = jq_gen_param(jv_string_value($1));
  jv_free($1);
}


String:
QQSTRING_START { $<literal>$ = jv_string("text"); } QQString QQSTRING_END {
  $$ = $3;
  jv_free($<literal>2);
} |
FORMAT QQSTRING_START { $<literal>$ = $1; } QQString QQSTRING_END {
  $$ = $4;
  jv_free($<literal>3);
}


QQString:
%empty {
  $$ = jq_gen_const(jv_string(""));
} |
QQString QQSTRING_TEXT {
  $$ = jq_gen_binop($1, jq_gen_const($2), '+');
} |
QQString QQSTRING_INTERP_START Exp QQSTRING_INTERP_END {
  $$ = jq_gen_binop($1, jq_gen_format($3, jv_copy($<literal>0)), '+');
}


ElseBody:
"elif" Exp "then" Exp ElseBody {
  $$ = jq_gen_cond($2, $4, $5);
} |
"else" Exp "end" {
  $$ = $2;
}

ExpD:
ExpD '|' ExpD {
  $$ = block_join($1, $3);
} |
'-' ExpD {
  $$ = BLOCK($2, jq_gen_call("_negate", jq_gen_noop()));
} |
Term {
  $$ = $1;
}


Term:
'.' {
  $$ = jq_gen_noop();
} |
REC {
  $$ = jq_gen_call("recurse", jq_gen_noop());
} |
BREAK '$' IDENT {
  jv v = jv_string_fmt("*label-%s", jv_string_value($3));     // impossible symbol
  $$ = jq_gen_location(@$, locations,
                    BLOCK(jq_gen_op_unbound(LOADV, jv_string_value(v)),
                    jq_gen_call("error", jq_gen_noop())));
  jv_free(v);
  jv_free($3);
} |
BREAK error {
  FAIL(@$, "break requires a label to break to");
  $$ = jq_gen_noop();
} |
Term FIELD '?' {
  $$ = jq_gen_index_opt($1, jq_gen_const($2));
} |
FIELD '?' {
  $$ = jq_gen_index_opt(jq_gen_noop(), jq_gen_const($1));
} |
Term '.' String '?' {
  $$ = jq_gen_index_opt($1, $3);
} |
'.' String '?' {
  $$ = jq_gen_index_opt(jq_gen_noop(), $2);
} |
Term FIELD %prec NONOPT {
  $$ = jq_gen_index($1, jq_gen_const($2));
} |
FIELD %prec NONOPT {
  $$ = jq_gen_index(jq_gen_noop(), jq_gen_const($1));
} |
Term '.' String %prec NONOPT {
  $$ = jq_gen_index($1, $3);
} |
'.' String %prec NONOPT {
  $$ = jq_gen_index(jq_gen_noop(), $2);
} |
'.' error {
  FAIL(@$, "try .[\"field\"] instead of .field for unusually named fields");
  $$ = jq_gen_noop();
} |
'.' IDENT error {
  jv_free($2);
  FAIL(@$, "try .[\"field\"] instead of .field for unusually named fields");
  $$ = jq_gen_noop();
} |
/* FIXME: string literals */
Term '[' Exp ']' '?' {
  $$ = jq_gen_index_opt($1, $3);
} |
Term '[' Exp ']' %prec NONOPT {
  $$ = jq_gen_index($1, $3);
} |
Term '[' ']' '?' {
  $$ = block_join($1, jq_gen_op_simple(EACH_OPT));
} |
Term '[' ']' %prec NONOPT {
  $$ = block_join($1, jq_gen_op_simple(EACH));
} |
Term '[' Exp ':' Exp ']' '?' {
  $$ = jq_gen_slice_index($1, $3, $5, INDEX_OPT);
} |
Term '[' Exp ':' ']' '?' {
  $$ = jq_gen_slice_index($1, $3, jq_gen_const(jv_null()), INDEX_OPT);
} |
Term '[' ':' Exp ']' '?' {
  $$ = jq_gen_slice_index($1, jq_gen_const(jv_null()), $4, INDEX_OPT);
} |
Term '[' Exp ':' Exp ']' %prec NONOPT {
  $$ = jq_gen_slice_index($1, $3, $5, INDEX);
} |
Term '[' Exp ':' ']' %prec NONOPT {
  $$ = jq_gen_slice_index($1, $3, jq_gen_const(jv_null()), INDEX);
} |
Term '[' ':' Exp ']' %prec NONOPT {
  $$ = jq_gen_slice_index($1, jq_gen_const(jv_null()), $4, INDEX);
} |
LITERAL {
  $$ = jq_gen_const($1);
} |
String {
  $$ = $1;
} |
FORMAT {
  $$ = jq_gen_format(jq_gen_noop(), $1);
} |
'(' Exp ')' {
  $$ = $2;
} |
'[' Exp ']' {
  $$ = jq_gen_collect($2);
} |
'[' ']' {
  $$ = jq_gen_const(jv_array());
} |
'{' MkDict '}' {
  block o = jq_gen_const_object($2);
  if (o.first != NULL)
    $$ = o;
  else
    $$ = BLOCK(jq_gen_subexp(jq_gen_const(jv_object())), $2, jq_gen_op_simple(POP));
} |
'$' LOC {
  $$ = jq_gen_const(JV_OBJECT(jv_string("file"), jv_copy(locations->fname),
                           jv_string("line"), jv_number(locfile_get_line(locations, @$.start) + 1)));
} |
'$' IDENT {
  $$ = jq_gen_location(@$, locations, jq_gen_op_unbound(LOADV, jv_string_value($2)));
  jv_free($2);
} |
IDENT {
  const char *s = jv_string_value($1);
  if (strcmp(s, "false") == 0)
    $$ = jq_gen_const(jv_false());
  else if (strcmp(s, "true") == 0)
    $$ = jq_gen_const(jv_true());
  else if (strcmp(s, "null") == 0)
    $$ = jq_gen_const(jv_null());
  else
    $$ = jq_gen_location(@$, locations, jq_gen_call(s, jq_gen_noop()));
  jv_free($1);
} |
IDENT '(' Args ')' {
  $$ = jq_gen_call(jv_string_value($1), $3);
  $$ = jq_gen_location(@1, locations, $$);
  jv_free($1);
} |
'(' error ')' { $$ = jq_gen_noop(); } |
'[' error ']' { $$ = jq_gen_noop(); } |
Term '[' error ']' { $$ = $1; } |
'{' error '}' { $$ = jq_gen_noop(); }

Args:
Arg {
  $$ = $1;
} |
Args ';' Arg {
  $$ = BLOCK($1, $3);
}

Arg:
Exp {
  $$ = jq_gen_lambda($1);
}

RepPatterns:
RepPatterns "?//" Pattern {
  $$ = BLOCK($1, jq_gen_destructure_alt($3));
} |
Pattern {
  $$ = jq_gen_destructure_alt($1);
}

Patterns:
RepPatterns "?//" Pattern {
  $$ = BLOCK($1, $3);
} |
Pattern {
  $$ = $1;
}

Pattern:
'$' IDENT {
  $$ = jq_gen_op_unbound(STOREV, jv_string_value($2));
  jv_free($2);
} |
'[' ArrayPats ']' {
  $$ = BLOCK($2, jq_gen_op_simple(POP));
} |
'{' ObjPats '}' {
  $$ = BLOCK($2, jq_gen_op_simple(POP));
}

ArrayPats:
Pattern {
  $$ = jq_gen_array_matcher(jq_gen_noop(), $1);
} |
ArrayPats ',' Pattern {
  $$ = jq_gen_array_matcher($1, $3);
}

ObjPats:
ObjPat {
  $$ = $1;
} |
ObjPats ',' ObjPat {
  $$ = BLOCK($1, $3);
}

ObjPat:
'$' IDENT {
  $$ = jq_gen_object_matcher(jq_gen_const($2), jq_gen_op_unbound(STOREV, jv_string_value($2)));
} |
'$' IDENT ':' Pattern {
  $$ = jq_gen_object_matcher(jq_gen_const($2), BLOCK(jq_gen_op_simple(DUP), jq_gen_op_unbound(STOREV, jv_string_value($2)), $4));
} |
IDENT ':' Pattern {
  $$ = jq_gen_object_matcher(jq_gen_const($1), $3);
} |
Keyword ':' Pattern {
  $$ = jq_gen_object_matcher(jq_gen_const($1), $3);
} |
String ':' Pattern {
  $$ = jq_gen_object_matcher($1, $3);
} |
'(' Exp ')' ':' Pattern {
  jv msg = check_object_key($2);
  if (jv_is_valid(msg)) {
    FAIL(@$, jv_string_value(msg));
  }
  jv_free(msg);
  $$ = jq_gen_object_matcher($2, $5);
} |
error ':' Pattern {
  FAIL(@$, "May need parentheses around object key expression");
  $$ = $3;
}

Keyword:
"as" {
  $$ = jv_string("as");
} |
"def" {
  $$ = jv_string("def");
} |
"module" {
  $$ = jv_string("module");
} |
"import" {
  $$ = jv_string("import");
} |
"include" {
  $$ = jv_string("include");
} |
"if" {
  $$ = jv_string("if");
} |
"then" {
  $$ = jv_string("then");
} |
"else" {
  $$ = jv_string("else");
} |
"elif" {
  $$ = jv_string("elif");
} |
"reduce" {
  $$ = jv_string("reduce");
} |
"foreach" {
  $$ = jv_string("foreach");
} |
"end" {
  $$ = jv_string("end");
} |
"and" {
  $$ = jv_string("and");
} |
"or" {
  $$ = jv_string("or");
} |
"try" {
  $$ = jv_string("try");
} |
"catch" {
  $$ = jv_string("catch");
} |
"label" {
  $$ = jv_string("label");
} |
"break" {
  $$ = jv_string("break");
} |
"__loc__" {
  $$ = jv_string("__loc__");
}

MkDict:
%empty {
  $$=jq_gen_noop();
} |
 MkDictPair { $$ = $1; }
| MkDictPair ',' MkDict { $$=block_join($1, $3); }
| error ',' MkDict { $$ = $3; }

MkDictPair:
IDENT ':' ExpD {
  $$ = jq_gen_dictpair(jq_gen_const($1), $3);
 }
| Keyword ':' ExpD {
  $$ = jq_gen_dictpair(jq_gen_const($1), $3);
  }
| String ':' ExpD {
  $$ = jq_gen_dictpair($1, $3);
  }
| String {
  $$ = jq_gen_dictpair($1, BLOCK(jq_gen_op_simple(POP), jq_gen_op_simple(DUP2),
                              jq_gen_op_simple(DUP2), jq_gen_op_simple(INDEX)));
  }
| '$' IDENT {
  $$ = jq_gen_dictpair(jq_gen_const($2),
                    jq_gen_location(@$, locations, jq_gen_op_unbound(LOADV, jv_string_value($2))));
  }
| IDENT {
  $$ = jq_gen_dictpair(jq_gen_const(jv_copy($1)),
                    jq_gen_index(jq_gen_noop(), jq_gen_const($1)));
  }
| '(' Exp ')' ':' ExpD {
  jv msg = check_object_key($2);
  if (jv_is_valid(msg)) {
    FAIL(@$, jv_string_value(msg));
  }
  jv_free(msg);
  $$ = jq_gen_dictpair($2, $5);
  }
| error ':' ExpD {
  FAIL(@$, "May need parentheses around object key expression");
  $$ = $3;
  }
%%

int jq_parse(struct locfile* locations, block* answer) {
  struct lexer_param scanner;
  YY_BUFFER_STATE buf;
  jq_yylex_init_extra(0, &scanner.lexer);
  buf = jq_yy_scan_bytes(locations->data, locations->length, scanner.lexer);
  int errors = 0;
  *answer = jq_gen_noop();
  yyparse(answer, &errors, locations, &scanner);
  jq_yy_delete_buffer(buf, scanner.lexer);
  jq_yylex_destroy(scanner.lexer);
  if (errors > 0) {
    block_free(*answer);
    *answer = jq_gen_noop();
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

%{
#include <stdio.h>
#include <string.h>
#include <assert.h>
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
%token SETPIPE "|="
%token SETPLUS "+="
%token SETMINUS "-="
%token SETMULT "*="
%token SETDIV "/="
%token SETDEFINEDOR "//="
%token LESSEQ "<="
%token GREATEREQ ">="

%token QQSTRING_START
%token <literal> QQSTRING_TEXT
%token QQSTRING_INTERP_START
%token QQSTRING_INTERP_END
%token QQSTRING_END

/* see Exp '?' rule */
%expect 34

%right '|'
%left ','
%right "//"
%nonassoc '=' SETPIPE SETPLUS SETMINUS SETMULT SETDIV SETMOD SETDEFINEDOR
%left OR
%left AND
%nonassoc NEQ EQ '<' '>' LESSEQ GREATEREQ
%left '+' '-'
%left '*' '/' '%'
%precedence '?'
%precedence "try"
%precedence "catch"


%type <blk> Exp Term MkDict MkDictPair ExpD ElseBody QQString
%type <blk> FuncDef FuncDefs String Import Imports Param Params
%type <blk> Arg Args Module
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

%}

%%
TopLevel:
Module Imports Exp {
  *answer = BLOCK($1, $2, gen_op_simple(TOP), $3);
} |
Module Imports FuncDefs {
  *answer = BLOCK($1, $2, $3);
} 

Module:
%empty {
  $$ = gen_noop();
} |
"module" IDENT Exp ';' {
  if (!block_is_const($3)) {
    FAIL(@$, "Module metadata must be constant.");
    $$ = gen_noop();
  } else {
    $$ = gen_module(jv_string_value($2), $3);
  }
  jv_free($2);
}

Imports:
%empty {
  $$ = gen_noop();
} |
Import Imports {
  $$ = BLOCK($1, $2);
}

FuncDefs:
%empty {
  $$ = gen_noop();
} |
FuncDef FuncDefs {
  $$ = block_bind($1, $2, OP_IS_CALL_PSEUDO);
}

Exp:
FuncDef Exp %prec ';' {
  $$ = block_bind_referenced($1, $2, OP_IS_CALL_PSEUDO);
} |

Term "as" '$' IDENT '|' Exp {
  $$ = gen_var_binding($1, jv_string_value($4), $6);
  jv_free($4);
} |

"reduce" Term "as" '$' IDENT '(' Exp ';' Exp ')' {
  $$ = gen_reduce(jv_string_value($5), $2, $7, $9);
  jv_free($5);
} |

"foreach" Term "as" '$' IDENT '(' Exp ';' Exp ';' Exp ')' {
  $$ = gen_foreach(jv_string_value($5), $2, $7, $9, $11);
  jv_free($5);
} |

"foreach" Term "as" '$' IDENT '(' Exp ';' Exp ')' {
  $$ = gen_foreach(jv_string_value($5), $2, $7, $9, gen_noop());
  jv_free($5);
} |

"if" Exp "then" Exp ElseBody {
  $$ = gen_cond($2, $4, $5);
} |
"if" Exp "then" error {
  FAIL(@$, "Possibly unterminated 'if' statement");
  $$ = $2;
} |

"try" Exp "catch" Exp {
  //$$ = BLOCK(gen_op_target(FORK_OPT, $2), $2, $4);
  $$ = gen_try($2, $4);
} |
"try" Exp {
  //$$ = BLOCK(gen_op_target(FORK_OPT, $2), $2, gen_op_simple(BACKTRACK));
  $$ = gen_try($2, gen_op_simple(BACKTRACK));
} |
"try" Exp "catch" error {
  FAIL(@$, "Possibly unterminated 'try' statement");
  $$ = $2;
} |

// This rule conflicts with all the other rules using the '?' operator.
// It doesn't matter which bison prefers: all of them result in the same
// syntax and semantics, but the more specific rules optimize better
// because they use opcodes specifically made for the purpose.  We
// expect 9 such conflicts.
Exp '?' {
  $$ = gen_try($1, gen_op_simple(BACKTRACK));
} |

Exp '=' Exp {
  $$ = gen_call("_assign", BLOCK(gen_lambda($1), gen_lambda($3)));
} |

Exp "or" Exp {
  $$ = gen_or($1, $3);
} | 

Exp "and" Exp {
  $$ = gen_and($1, $3);
} |

Exp "//" Exp {
  $$ = gen_definedor($1, $3);
} |

Exp "//=" Exp {
  $$ = gen_definedor_assign($1, $3);
} |

Exp "|=" Exp {
  $$ = gen_call("_modify", BLOCK(gen_lambda($1), gen_lambda($3)));
} |

Exp '|' Exp { 
  $$ = block_join($1, $3); 
} |

Exp ',' Exp { 
  $$ = gen_both($1, $3); 
} |

Exp '+' Exp {
  $$ = gen_binop($1, $3, '+');
} |

Exp "+=" Exp {
  $$ = gen_update($1, $3, '+');
} |

'-' Exp {
  $$ = BLOCK($2, gen_call("_negate", gen_noop()));
} |

Exp '-' Exp {
  $$ = gen_binop($1, $3, '-');
} |

Exp "-=" Exp {
  $$ = gen_update($1, $3, '-');
} |

Exp '*' Exp {
  $$ = gen_binop($1, $3, '*');
} |

Exp "*=" Exp {
  $$ = gen_update($1, $3, '*');
} |

Exp '/' Exp {
  $$ = gen_binop($1, $3, '/');
} |

Exp '%' Exp {
  $$ = gen_binop($1, $3, '%');
} |

Exp "/=" Exp {
  $$ = gen_update($1, $3, '/');
} |

Exp SETMOD Exp {
  $$ = gen_update($1, $3, '%');
} |

Exp "==" Exp {
  $$ = gen_binop($1, $3, EQ);
} |

Exp "!=" Exp {
  $$ = gen_binop($1, $3, NEQ);
} |

Exp '<' Exp {
  $$ = gen_binop($1, $3, '<');
} |

Exp '>' Exp {
  $$ = gen_binop($1, $3, '>');
} |

Exp "<=" Exp {
  $$ = gen_binop($1, $3, LESSEQ);
} |

Exp ">=" Exp {
  $$ = gen_binop($1, $3, GREATEREQ);
} |

Term { 
  $$ = $1; 
}

Import:
"import" IDENT ';' {
  $$ = gen_import(jv_string_value($2), gen_noop(), NULL);
  jv_free($2);
} |
"import" IDENT Exp ';' {
  if (!block_is_const($3)) {
    FAIL(@$, "Module metadata must be constant.");
    $$ = gen_noop();
  } else {
    $$ = gen_import(jv_string_value($2), $3, NULL);
  }
  jv_free($2);
} |
"import" IDENT "as" IDENT ';' {
  $$ = gen_import(jv_string_value($2), gen_noop(), jv_string_value($4));
  jv_free($2);
  jv_free($4);
} |
"import" IDENT "as" IDENT Exp ';' {
  if (!block_is_const($5)) {
    FAIL(@$, "Module metadata must be constant.");
    $$ = gen_noop();
  } else {
    $$ = gen_import(jv_string_value($2), $5, jv_string_value($4));
  }
  jv_free($2);
  jv_free($4);
}

FuncDef:
"def" IDENT ':' Exp ';' {
  $$ = gen_function(jv_string_value($2), gen_noop(), $4);
  jv_free($2);
} |

"def" IDENT '(' Params ')' ':' Exp ';' {
  $$ = gen_function(jv_string_value($2), $4, $7);
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
  $$ = gen_param_regular(jv_string_value($2));
  jv_free($2);
} |

IDENT {
  $$ = gen_param(jv_string_value($1));
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
  $$ = gen_const(jv_string(""));
} |
QQString QQSTRING_TEXT {
  $$ = gen_binop($1, gen_const($2), '+');
} |
QQString QQSTRING_INTERP_START Exp QQSTRING_INTERP_END {
  $$ = gen_binop($1, gen_format($3, jv_copy($<literal>0)), '+');
}


ElseBody:
"elif" Exp "then" Exp ElseBody {
  $$ = gen_cond($2, $4, $5);
} |
"else" Exp "end" {
  $$ = $2;
}

ExpD:
ExpD '|' ExpD { 
  $$ = block_join($1, $3);
} |
'-' ExpD {
  $$ = BLOCK($2, gen_call("_negate", gen_noop()));
} |
Term {
  $$ = $1;
}


Term:
'.' {
  $$ = gen_noop(); 
} |
REC {
  $$ = gen_call("recurse", gen_noop());
} |
Term FIELD '?' {
  $$ = gen_index_opt($1, gen_const($2));
} |
FIELD '?' { 
  $$ = gen_index_opt(gen_noop(), gen_const($1)); 
} |
Term '.' String '?' {
  $$ = gen_index_opt($1, $3);
} |
'.' String '?' {
  $$ = gen_index_opt(gen_noop(), $2);
} |
Term FIELD {
  $$ = gen_index($1, gen_const($2));
} |
FIELD { 
  $$ = gen_index(gen_noop(), gen_const($1)); 
} |
Term '.' String {
  $$ = gen_index($1, $3);
} |
'.' String {
  $$ = gen_index(gen_noop(), $2);
} |
'.' error {
  FAIL(@$, "try .[\"field\"] instead of .field for unusually named fields");
  $$ = gen_noop();
} |
'.' IDENT error {
  jv_free($2);
  FAIL(@$, "try .[\"field\"] instead of .field for unusually named fields");
  $$ = gen_noop();
} | 
/* FIXME: string literals */
Term '[' Exp ']' '?' {
  $$ = gen_index_opt($1, $3); 
} |
Term '[' Exp ']' {
  $$ = gen_index($1, $3); 
} |
Term '[' ']' '?' {
  $$ = block_join($1, gen_op_simple(EACH_OPT)); 
} |
Term '[' ']' {
  $$ = block_join($1, gen_op_simple(EACH)); 
} |
Term '[' Exp ':' Exp ']' '?' {
  $$ = gen_slice_index($1, $3, $5, INDEX_OPT);
} |
Term '[' Exp ':' ']' '?' {
  $$ = gen_slice_index($1, $3, gen_const(jv_null()), INDEX_OPT);
} |
Term '[' ':' Exp ']' '?' {
  $$ = gen_slice_index($1, gen_const(jv_null()), $4, INDEX_OPT);
} |
Term '[' Exp ':' Exp ']' {
  $$ = gen_slice_index($1, $3, $5, INDEX);
} |
Term '[' Exp ':' ']' {
  $$ = gen_slice_index($1, $3, gen_const(jv_null()), INDEX);
} |
Term '[' ':' Exp ']' {
  $$ = gen_slice_index($1, gen_const(jv_null()), $4, INDEX);
} |
LITERAL {
  $$ = gen_const($1); 
} |
String {
  $$ = $1;
} |
FORMAT {
  $$ = gen_format(gen_noop(), $1);
} |
'(' Exp ')' { 
  $$ = $2; 
} | 
'[' Exp ']' { 
  $$ = gen_collect($2); 
} |
'[' ']' { 
  $$ = gen_const(jv_array()); 
} |
'{' MkDict '}' { 
  block o = gen_const_object($2);
  if (o.first != NULL)
    $$ = o;
  else
    $$ = BLOCK(gen_subexp(gen_const(jv_object())), $2, gen_op_simple(POP));
} |
'$' IDENT {
  $$ = gen_location(@$, locations, gen_op_unbound(LOADV, jv_string_value($2)));
  jv_free($2);
} | 
IDENT {
  const char *s = jv_string_value($1);
  if (strcmp(s, "false") == 0)
    $$ = gen_const(jv_false());
  else if (strcmp(s, "true") == 0)
    $$ = gen_const(jv_true());
  else if (strcmp(s, "null") == 0)
    $$ = gen_const(jv_null());
  else
    $$ = gen_location(@$, locations, gen_call(s, gen_noop()));
  jv_free($1);
} |
IDENT '(' Args ')' {
  $$ = gen_call(jv_string_value($1), $3);
  $$ = gen_location(@1, locations, $$);
  jv_free($1);
} |
'(' error ')' { $$ = gen_noop(); } |
'[' error ']' { $$ = gen_noop(); } |
Term '[' error ']' { $$ = $1; } |
'{' error '}' { $$ = gen_noop(); }

Args:
Arg {
  $$ = $1;
} |
Args ';' Arg {
  $$ = BLOCK($1, $3);
}

Arg:
Exp {
  $$ = gen_lambda($1);
}

MkDict:
%empty { 
  $$=gen_noop(); 
} |
 MkDictPair { $$ = $1; }
| MkDictPair ',' MkDict { $$=block_join($1, $3); }
| error ',' MkDict { $$ = $3; }

MkDictPair
: IDENT ':' ExpD { 
  $$ = gen_dictpair(gen_const($1), $3);
 }
| String ':' ExpD {
  $$ = gen_dictpair($1, $3);
  }
| String {
  $$ = gen_dictpair($1, BLOCK(gen_op_simple(POP), gen_op_simple(DUP2),
                              gen_op_simple(DUP2), gen_op_simple(INDEX)));
  }
| IDENT {
  $$ = gen_dictpair(gen_const(jv_copy($1)),
                    gen_index(gen_noop(), gen_const($1)));
  }
| '(' Exp ')' ':' ExpD {
  $$ = gen_dictpair($2, $5);
  }
| '(' error ')' ':' ExpD { $$ = $5; }
%%

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

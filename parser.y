%{
#include <stdio.h>
#include <string.h>
#include "compile.h"

struct lexer_param;

%}
%code requires {
#include "locfile.h"
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
%token <literal> LITERAL
%token EQ "=="
%token NEQ "!="
%token DEFINEDOR "//"
%token AS "as"
%token DEF "def"
%token IF "if"
%token THEN "then"
%token ELSE "else"
%token ELSE_IF "elif"
%token END "end"
%token AND "and"
%token OR "or"
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

 /* revolting hack */
%left ';'
%right '|'
%left ','
%right "//"
%nonassoc '=' SETPIPE SETPLUS SETMINUS SETMULT SETDIV SETDEFINEDOR
%left OR
%left AND
%nonassoc NEQ EQ '<' '>' LESSEQ GREATEREQ
%left '+' '-'
%left '*' '/'


%type <blk> Exp Term MkDict MkDictPair ExpD ElseBody QQString FuncDef FuncDefs String
%{
#include "lexer.gen.h"
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
  locfile_locate(locations, *loc, "error: %s", s);
}

int yylex(YYSTYPE* yylval, YYLTYPE* yylloc, block* answer, int* errors, 
          struct locfile* locations, struct lexer_param* lexer_param_ptr) {
  yyscan_t lexer = lexer_param_ptr->lexer;
  while (1) {
    int tok = jq_yylex(yylval, yylloc, lexer);
    if (tok == INVALID_CHARACTER) {
      FAIL(*yylloc, "Invalid character");
    } else {
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
  }
}

static block gen_dictpair(block k, block v) {
  return BLOCK(gen_subexp(k), gen_subexp(v), gen_op_simple(INSERT));
}

static block gen_index(block obj, block key) {
  return BLOCK(obj, gen_subexp(key), gen_op_simple(INDEX));
}

static block gen_binop(block a, block b, int op) {
  const char* funcname = 0;
  switch (op) {
  case '+': funcname = "_plus"; break;
  case '-': funcname = "_minus"; break;
  case '*': funcname = "_multiply"; break;
  case '/': funcname = "_divide"; break;
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

static block gen_format(block a) {
  return BLOCK(a, gen_call("tostring", gen_noop()));
}
 
static block gen_update(block a, block op, int optype) {
  if (optype) {
    op = gen_binop(gen_noop(), op, optype);
  }
  return gen_assign(BLOCK(a, gen_op_simple(DUP), op));
}

%}

%%
TopLevel:
Exp {
  *answer = $1;
} |
FuncDefs {
  *answer = $1;
} 

FuncDefs:
/* empty */ {
  $$ = gen_noop();
} |
FuncDef FuncDefs {
  $$ = block_join($1, $2);
}

Exp:
FuncDef Exp %prec ';' {
  $$ = block_bind($1, $2, OP_IS_CALL_PSEUDO);
} |

Term "as" '$' IDENT '|' Exp {
  $$ = BLOCK(gen_op_simple(DUP), $1, 
             block_bind(gen_op_var_unbound(STOREV, jv_string_value($4)), 
                        $6, OP_HAS_VARIABLE));
  jv_free($4);
} |

"if" Exp "then" Exp ElseBody {
  $$ = gen_cond($2, $4, $5);
} |
"if" Exp error {
  FAIL(@$, "Possibly unterminated 'if' statment");
  $$ = $2;
} |

Exp '=' Exp {
  $$ = gen_assign(BLOCK(gen_op_simple(DUP), $3, gen_op_simple(SWAP), $1, gen_op_simple(SWAP)));
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
  $$ = gen_update($1, gen_definedor(gen_noop(), $3), 0);
} |

Exp "|=" Exp {
  $$ = gen_update($1, $3, 0);
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

Exp "/=" Exp {
  $$ = gen_update($1, $3, '/');
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

String:
QQSTRING_START QQString QQSTRING_END {
  $$ = $2;
}

FuncDef:
"def" IDENT ':' Exp ';' {
  $$ = gen_function(jv_string_value($2), $4);
  jv_free($2);
} |

"def" IDENT '(' IDENT ')' ':' Exp ';' {
  block body = block_bind(gen_op_block_unbound(CLOSURE_PARAM, jv_string_value($4)), $7, OP_IS_CALL_PSEUDO);
  $$ = gen_function(jv_string_value($2), body);
  jv_free($2);
  jv_free($4);
}

QQString:
/* empty */ {
  $$ = gen_op_const(LOADK, jv_string(""));
} |
QQString QQSTRING_TEXT {
  $$ = gen_binop($1, gen_op_const(LOADK, $2), '+');
} |
QQString QQSTRING_INTERP_START Exp QQSTRING_INTERP_END {
  $$ = gen_binop($1, gen_format($3), '+');
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

Term {
  $$ = $1;
}


Term:
'.' {
  $$ = gen_noop(); 
} |
Term '.' IDENT {
  $$ = gen_index($1, gen_op_const(LOADK, $3)); 
} |
'.' IDENT { 
  $$ = gen_index(gen_noop(), gen_op_const(LOADK, $2)); 
} |
/* FIXME: string literals */
Term '[' Exp ']' {
  $$ = gen_index($1, $3); 
} |
Term '[' ']' {
  $$ = block_join($1, gen_op_simple(EACH)); 
} |
LITERAL {
  $$ = gen_op_const(LOADK, $1); 
} |
String {
  $$ = $1;
} |
'(' Exp ')' { 
  $$ = $2; 
} | 
'[' Exp ']' { 
  $$ = gen_collect($2); 
} |
'[' ']' { 
  $$ = gen_op_const(LOADK, jv_array()); 
} |
'{' MkDict '}' { 
  $$ = BLOCK(gen_subexp(gen_op_const(LOADK, jv_object())), $2, gen_op_simple(POP));
} |
'$' IDENT {
  $$ = gen_location(@$, gen_op_var_unbound(LOADV, jv_string_value($2)));
  jv_free($2);
} | 
IDENT {
  $$ = gen_location(@$, gen_call(jv_string_value($1), gen_noop()));
  jv_free($1);
} |
IDENT '(' Exp ')' {
  $$ = gen_call(jv_string_value($1), gen_lambda($3));
  $$ = gen_location(@1, $$);
  jv_free($1);
} |
'(' error ')' { $$ = gen_noop(); } |
'[' error ']' { $$ = gen_noop(); } |
Term '[' error ']' { $$ = $1; } |
'{' error '}' { $$ = gen_noop(); }

MkDict:
{ 
  $$=gen_noop(); 
} |
 MkDictPair { $$ = $1; }
| MkDictPair ',' MkDict { $$=block_join($1, $3); }
| error ',' MkDict { $$ = $3; }

MkDictPair
: IDENT ':' ExpD { 
  $$ = gen_dictpair(gen_op_const(LOADK, $1), $3);
 }
| String ':' ExpD {
  $$ = gen_dictpair($1, $3);
  }
| IDENT {
  $$ = gen_dictpair(gen_op_const(LOADK, jv_copy($1)),
                    gen_index(gen_noop(), gen_op_const(LOADK, $1)));
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
  if (!block_has_only_binders(*answer, OP_IS_CALL_PSEUDO)) {
    locfile_locate(locations, UNKNOWN_LOCATION, "error: library should only have function definitions, not a main expression");
    return 1;
  }
  return 0;
}

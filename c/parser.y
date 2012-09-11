%{
#include <stdio.h>
#include <string.h>
#include "compile.h"
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
%parse-param {yyscan_t lexer}
%lex-param {block* answer}
%lex-param {int* errors}
%lex-param {struct locfile* locations}
%lex-param {yyscan_t lexer}


%token INVALID_CHARACTER
%token <literal> IDENT
%token <literal> LITERAL
%token EQ "=="
%token DEFINEDOR "//"
%token AS "as"
%token DEF "def"
%token SETPIPE "|="
%token IF "if"
%token THEN "then"
%token ELSE "else"
%token ELSE_IF "elif"
%token END "end"
%token AND "and"
%token OR "or"
%token NOT "not"


 /* revolting hack */
%left ';'
%left '|'
%left ','
%right "//"
%nonassoc '=' SETPIPE
%nonassoc EQ
%left OR
%left AND
%left '+' '-'
%left '*' '/'


%type <blk> Exp Term MkDict MkDictPair ExpD ElseBody
%{
#include "lexer.yy.h"
#define FAIL(loc, msg)                                   \
  do {                                                   \
    location l = loc;                                    \
    yyerror(&l, answer, errors, locations, lexer, msg);  \
    /*YYERROR*/;                                         \
  } while (0)

void yyerror(YYLTYPE* loc, block* answer, int* errors, 
             struct locfile* locations, yyscan_t lexer, const char *s){
  (*errors)++;
  printf("error: %s\n", s);
  locfile_locate(locations, *loc);
}

int yylex(YYSTYPE* yylval, YYLTYPE* yylloc, block* answer, int* errors, 
          struct locfile* locations, yyscan_t lexer) {
  while (1) {
    int tok = jq_yylex(yylval, yylloc, lexer);
    if (tok == INVALID_CHARACTER) {
      FAIL(*yylloc, "Invalid character");
    } else {
      if (tok == LITERAL && !jv_is_valid(yylval->literal)) {
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
  block b = gen_subexp(k);
  block_append(&b, gen_subexp(v));
  block_append(&b, gen_op_simple(INSERT));
  return b;
}

static block gen_index(block obj, block key) {
  return block_join(obj, block_join(gen_subexp(key), gen_op_simple(INDEX)));
}

static block gen_binop(block a, block b, int op) {
  const char* funcname = 0;
  switch (op) {
  case '+': funcname = "_plus"; break;
  case '-': funcname = "_minus"; break;
  case '*': funcname = "_multiply"; break;
  case '/': funcname = "_divide"; break;
  case EQ: funcname = "_equal"; break;
  }
  assert(funcname);

  block c = gen_noop();
  block_append(&c, gen_subexp(a));
  block_append(&c, gen_subexp(b));
  block_append(&c, gen_op_call(CALL_1_1, gen_op_block_unbound(CLOSURE_REF, funcname)));
  return c;
}

%}

%%
program: Exp { *answer = $1; }

Exp:
"def" IDENT ':' Exp ';' Exp {
  block body = block_join($4, gen_op_simple(RET));
  $$ = block_bind(gen_op_block_defn(CLOSURE_CREATE, jv_string_value($2), body), 
                  $6, OP_IS_CALL_PSEUDO);
  jv_free($2);
} |

"def" IDENT '(' IDENT ')' ':' Exp ';' Exp {
  block body = block_bind(gen_op_block_unbound(CLOSURE_PARAM, jv_string_value($4)), block_join($7, gen_op_simple(RET)), OP_IS_CALL_PSEUDO);
  $$ = block_bind(gen_op_block_defn(CLOSURE_CREATE, jv_string_value($2), body), $9, OP_IS_CALL_PSEUDO);
  jv_free($2);
  jv_free($4);
} |

Term "as" '$' IDENT '|' Exp {
  $$ = gen_op_simple(DUP);
  block_append(&$$, $1);
  block_append(&$$, block_bind(gen_op_var_unbound(STOREV, jv_string_value($4)), $6, OP_HAS_VARIABLE));
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
  block assign = gen_op_simple(DUP);
  block_append(&assign, $3);
  block_append(&assign, gen_op_simple(SWAP));
  block_append(&assign, $1);
  block_append(&assign, gen_op_simple(SWAP));
  $$ = gen_assign(assign);
} |

Exp "or" Exp {
  $$ = gen_or($1, $3);
} | 

Exp "and" Exp {
  $$ = gen_and($1, $3);
} |

"not" Exp {
  $$ = gen_not($2);
} |

Exp "//" Exp {
  $$ = gen_definedor($1, $3);
} |

Exp "|=" Exp {
  block assign = $1;
  block_append(&assign, gen_op_simple(DUP));
  block_append(&assign, $3);
  $$ = gen_assign(assign);
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

Exp '-' Exp {
  $$ = gen_binop($1, $3, '-');
} |

Exp '*' Exp {
  $$ = gen_binop($1, $3, '*');
} |

Exp '/' Exp {
  $$ = gen_binop($1, $3, '/');
} |

Exp "==" Exp {
  $$ = gen_binop($1, $3, EQ);
} |

Term { 
  $$ = $1; 
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
  $$ = gen_subexp(gen_op_const(LOADK, jv_object()));
  block_append(&$$, $2);
  block_append(&$$, gen_op_simple(POP));
} |
'$' IDENT {
  $$ = gen_op_var_unbound(LOADV, jv_string_value($2)); 
  jv_free($2);
} | 
IDENT {
  $$ = gen_op_call(CALL_1_1, gen_op_block_unbound(CLOSURE_REF, jv_string_value($1)));
  jv_free($1);
} |
IDENT '(' Exp ')' {
  $$ = gen_op_call(CALL_1_1, 
                   block_join(gen_op_block_unbound(CLOSURE_REF, jv_string_value($1)),
                              block_bind(gen_op_block_defn(CLOSURE_CREATE,
                                                "lambda",
                                                           block_join($3, gen_op_simple(RET))),
                                         gen_noop(), OP_IS_CALL_PSEUDO)));
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
| LITERAL ':' ExpD {
  if (jv_get_kind($1) != JV_KIND_STRING) {
    FAIL(@1, "Object keys must be strings");
  }
  $$ = gen_dictpair(gen_op_const(LOADK, $1), $3);
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

int compile(const char* str, block* answer) {
  yyscan_t scanner;
  YY_BUFFER_STATE buf;
  jq_yylex_init_extra(0, &scanner);
  buf = jq_yy_scan_string(str, scanner);
  int errors = 0;
  struct locfile locations;
  locfile_init(&locations, str, strlen(str));
  *answer = gen_noop();
  yyparse(answer, &errors, &locations, scanner);
  locfile_free(&locations);
  jq_yy_delete_buffer(buf, scanner);
  jq_yylex_destroy(scanner);
  if (errors > 0) {
    block_free(*answer);
    *answer = gen_noop();
  }
  return errors;
}

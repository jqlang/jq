%{
#include <stdio.h>
#include <string.h>
#include "compile.h"
%}

%locations
%define api.pure
%union {
  jv literal;
  block blk;
}

%parse-param {block* answer}
%parse-param {yyscan_t lexer}
%lex-param {yyscan_t lexer}


%token <literal> IDENT
%token <literal> LITERAL

 /* revolting hack */
%left ';'

%left '|'
%left ','
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
%right "//"
%nonassoc '=' SETPIPE
%nonassoc EQ
%left OR
%left AND
%left '+'


%type <blk> Exp Term MkDict MkDictPair ExpD ElseBody

%{
#include "lexer.yy.h"
void yyerror(YYLTYPE* loc, block* answer, yyscan_t lexer, const char *s){
  printf("ERROR: %s\n", s); 
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
  $$ = gen_noop();
  block_append(&$$, gen_subexp($1));
  block_append(&$$, gen_subexp($3));
  block_append(&$$, gen_op_call(CALL_1_1, gen_op_block_unbound(CLOSURE_REF, "_plus")));
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
}

MkDict:
{ 
  $$=gen_noop(); 
}
|
MkDictPair
{ $$ = $1; }
| MkDictPair ',' MkDict { $$=block_join($1, $3); }

MkDictPair
: IDENT ':' ExpD { 
  $$ = gen_dictpair(gen_op_const(LOADK, $1), $3);
 }
| LITERAL ':' ExpD {
  assert(jv_get_kind($1) == JV_KIND_STRING);
  $$ = gen_dictpair(gen_op_const(LOADK, $1), $3);
  }
| IDENT {
  $$ = gen_dictpair(gen_op_const(LOADK, jv_copy($1)),
                    gen_index(gen_noop(), gen_op_const(LOADK, $1)));
  }
| '(' Exp ')' ':' ExpD {
  $$ = gen_dictpair($2, $5);
  }
%%

block compile(const char* str) {
  yyscan_t scanner;
  YY_BUFFER_STATE buf;
  block answer = gen_noop();
  yylex_init(&scanner);
  buf = yy_scan_string(str, scanner);
  yyparse(&answer, scanner);
  yy_delete_buffer(buf, scanner);
  yylex_destroy(scanner);
  return answer;
}

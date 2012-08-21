%{
#include <stdio.h>
#include <string.h>
#include "compile.h"
%}

%locations
%define api.pure
%union {
  int num;
  char* str;
  block blk;
}

%parse-param {block* answer}
%parse-param {yyscan_t lexer}
%lex-param {yyscan_t lexer}


%token <str> IDENT
%token <num> NUMBER

 /* revolting hack */
%left ';'

%left '|'
%left ','
%token EQ "=="
%token AS "as"
%token DEF "def"
%nonassoc EQ
%left '+'

%type <blk> Exp Term MkDict MkDictPair ExpD

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

static block gen_string(const char* str) {
  return gen_op_const(LOADK, json_string(str));
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
  $$ = block_bind(gen_op_block_defn(CLOSURE_CREATE, $2, body), $6, OP_IS_CALL_PSEUDO);
} |

Term "as" '$' IDENT '|' Exp {
  $$ = gen_op_simple(DUP);
  block_append(&$$, $1);
  block_append(&$$, block_bind(gen_op_var_unbound(STOREV, $4), $6, OP_HAS_VARIABLE));
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
  block_append(&$$, gen_op_symbol(CALL_BUILTIN_3_1, "_plus"));
} |

Term { 
  $$ = $1; 
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
  $$ = gen_index($1, gen_string($3)); 
} |
'.' IDENT { 
  $$ = gen_index(gen_noop(), gen_string($2)); 
} |
/* FIXME: string literals */
Term '[' Exp ']' {
  $$ = gen_index($1, $3); 
} |
Term '[' ']' {
  $$ = block_join($1, gen_op_simple(EACH)); 
} |
NUMBER {
  $$ = gen_op_const(LOADK, json_integer($1)); 
} |
'(' Exp ')' { 
  $$ = $2; 
} | 
'[' Exp ']' { 
  $$ = gen_collect($2); 
} |
'[' ']' { 
  $$ = gen_op_const(LOADK, json_array()); 
} |
'{' MkDict '}' { 
  $$ = gen_subexp(gen_op_const(LOADK, json_object()));
  block_append(&$$, $2);
  block_append(&$$, gen_op_simple(POP));
} |
IDENT {
  $$ = gen_op_symbol(CALL_BUILTIN_1_1, $1);
} |
'$' IDENT {
  $$ = gen_op_var_unbound(LOADV, $2); 
} | 
'$' '$' IDENT {
  $$ = gen_op_call(CALL_1_1, gen_op_block_unbound(CLOSURE_REF, $3));
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
  $$ = gen_dictpair(gen_string($1), $3);
 }
| IDENT {
  $$ = gen_dictpair(gen_string($1),
                    gen_index(gen_noop(), gen_string($1)));
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

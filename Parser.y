{
module Parser where
import Lexer
import JQ
import Text.JSON
import Debug.Trace
import Data.List
import Control.Monad.Error
import Control.Monad.Reader
}

%name runParser Exp
%tokentype { Token }

%monad { Either String }
%error { \t -> fail $ "parse error: unexpected " ++ if null t then [] else (show $ head t) }

%token
  '|' { TRes "|" }
  '.' { TRes "." }
  '[' { TRes "[" }
  ']' { TRes "]" }
  '{' { TRes "{" }
  '}' { TRes "}" }
  '(' { TRes "(" }
  ')' { TRes ")" }
  ',' { TRes "," }
  ':' { TRes ":" }
  '==' { TRes "==" }
  '+' { TRes "+" }
  Ident { TIdent $$ }
  String { TString $$ }
  Int { TInt $$ }

%left '|'
%left ','
%nonassoc '=='
%left '+'

%%

Exp 
  : Exp '|' Exp         { $1 >| $3 }
  | Exp ',' Exp         { $1 `mplus` $3 }
  | Exp '==' Exp        { liftM2 eqj $1 $3 }
  | Exp '+' Exp         { liftM2 plusj $1 $3 }
  | Term                { $1 }

ExpD
  : ExpD '|' ExpD       { $1 >| $3 }
  | ExpD '==' ExpD      { liftM2 eqj $1 $3 }
  | Term                { $1 }


Term 
  : '.'                 { ask }
  | Term '.' Ident      { $1 >| index $3 }
  | '.' Ident           { index $2 }
  | String              { constStr $1 }
  | Term '[' Exp ']'    { do {t <- $1; i <- $3; lookupj t i} }
  | Term '[' ']'        { $1 >| childp }
  | '(' Exp ')'         { $2 }
  | '[' Exp ']'         { arrayp $2 }
  | Int                 { constInt $1 }
  | '{' MkDict '}'      { dictp $2 }
  | Ident '(' Exp ')'   { callp $1 [$3] }
  | Ident               { callp $1 [] }

MkDict
  :                     { [] }
  | MkDictPair          { [$1] }
  | MkDictPair ',' MkDict { $1:$3 }
 
MkDictPair
  : Ident ':' ExpD      { (constStr $1, $3) }
  | Ident               { (constStr $1, index $1) }
  | String ':' ExpD     { (constStr $1, $3) }
  | '(' Exp ')' ':' ExpD{ ($2, $5) }

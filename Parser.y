{
module Parser where
import Lexer
import JQ
import Text.JSON
import Debug.Trace
import Data.List
import Control.Monad.Error
import Control.Monad.Reader

instance Error (Maybe a) where
  noMsg = Nothing
  strMsg = const Nothing

instance (Error a, Error b) => Error (a, b) where
  noMsg = (noMsg, noMsg)
  strMsg s = (strMsg s, strMsg s)
}

%name runParser TopLevel
%tokentype { Token }

%monad { Either String }
%error { \t -> throwError $ "parse error: unexpected " ++ if null t then [] else (show $ t) }

%token
  '|' { TRes "|" }
  '.' { TRes "." }
  '[' { TRes "[" }
  ']' { TRes "]" }
  '{' { TRes "{" }
  '}' { TRes "}" }
  '(' { TRes "(" }
  ')' { TRes ")" }
  '$' { TRes "$" }
  'as' { TRes "as" }
  ',' { TRes "," }
  ':' { TRes ":" }
  '==' { TRes "==" }
  '=' { TRes "=" }
  '+' { TRes "+" }
  'def' { TRes "def" }
  ';' { TRes ";" }
  'else' { TRes "else" }
  'and' { TRes "and" }
  'or' { TRes "or" }
  Ident { TIdent $$ }
  String { TString $$ }
  Int { TInt $$ }


%left 'else'
%right '|'
%left '='
%left ','
%left 'and' 'or'
%nonassoc '=='
%left '+'

%%

TopLevel
  : Defn TopLevel	{ withDefn $1 $2 }
  | Exp			{ $1 }

Exp 
  : Exp 'else' Exp      { $1 `elsep` $3 }
  | Assign '|' Exp	{ do { v <- snd $1; setvar (fst $1) v $3 } }
  | Exp '|' Exp         { $1 >| $3 }
  | Exp ',' Exp         { $1 `mplus` $3 }
  | Exp 'and' Exp	{ liftM2 andj $1 $3 }
  | Exp 'or' Exp	{ liftM2 orj $1 $3 }
  | Exp '=' Exp         { assignp $1 $3 }
  | Exp '==' Exp        { liftM2 eqj (subexp $1) (subexp $3) }
  | Exp '+' Exp         { liftM2 plusj (subexp $1) (subexp $3) }
  | Term                { $1 }

Assign
  : Term 'as' '$' Ident { ($4, $1) }

Defn
  : 'def' Ident '=' Exp ';' { ($2, [], $4) }
  | 'def' Ident '(' ParamList ')' '=' Exp ';' { ($2, $4, $7) }

ParamList
  :			{ [] }
  | Ident		{ [$1] }
  | Ident ';' ParamList { $1:$3 }
  
ExpD
  : ExpD '|' ExpD       { $1 >| $3 }
  | ExpD '==' ExpD      { liftM2 eqj $1 $3 }
  | Term                { $1 }


Term 
  : '.'                 { input }
  | Term '.' Ident      { $1 >| index $3 }
  | '.' Ident           { index $2 }
  | String              { constStr $1 }
  | Term '[' Exp ']'    { do {t <- $1; i <- subexp $3; lookupj t i} }
  | Term '[' ']'        { $1 >| childp }
  | '(' Exp ')'         { $2 }
  | '[' Exp ']'         { arrayp $2 }
  | '[' ']' 		{ arrayp (callp "empty" []) }
  | Int                 { constInt $1 }
  | '{' MkDict '}'      { dictp $2 }
  | Ident '(' Exp ')'   { callp $1 [$3] }
  | Ident               { callp $1 [] }
  | '$' Ident		{ getvar $2 }

MkDict
  :                     { [] }
  | MkDictPair          { [$1] }
  | MkDictPair ',' MkDict { $1:$3 }

MkDictPair
  : Ident ':' ExpD      { (constStr $1, $3) }
  | Ident               { (constStr $1, index $1) }
  | String ':' ExpD     { (constStr $1, $3) }
  | '(' Exp ')' ':' ExpD{ ($2, $5) }

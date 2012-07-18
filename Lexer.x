{
module Lexer where
import Control.Monad.Error
}

%wrapper "monadUserState"

$digit = 0-9
$alpha = [a-zA-Z_]
@reserved = "."|"["|"]"|","|":"|"("|")"|"{"|"}"|"|"|"=="|"+"
@ident = $alpha [$alpha $digit]*
@string = \" ($printable)* \"


tokens :- 

<0> $white+             ;
<0> @reserved           { tok TRes }
<0> @ident              { tok TIdent }
<0> $digit+             { tok $ TInt . read }


<0> \"                  { enterString }
<string> \"             { leaveString }
<string> ($printable # [\"\\]) { pushString id }
<string> \\ [\"\\\/]    { pushString (drop 1) }
<string> \\ [nrt]       { pushString (escape . drop 1) }
--<string> \\ 'u' [0-9a-fA-F]{4}
--                        { pushString (parseUnicode . drop 2) }

-- @string { \s -> TString $ init $ tail s}


{
  
escape :: String -> String
escape "r" = "\r"
escape "n" = "\n"
escape "t" = "\t"

getState :: Alex AlexState
getState = Alex $ \s -> Right (s, s)

getUserState :: Alex AlexUserState
getUserState = liftM alex_ust getState

setUserState :: AlexUserState -> Alex ()
setUserState s' = Alex $ \s -> Right (s{alex_ust = s'}, ()) 

alexEOF = return $ Nothing

enterString input len = do
  setUserState []
  alexSetStartCode string
  skip input len

pushString f i@(p, _, s) len = do
  buf <- getUserState
  setUserState (buf ++ [f $ take len s])
  skip i len
  
leaveString input len = do
  s <- getUserState
  alexSetStartCode 0
  return $ Just $ TString $ concat s
  

tok f (p,_,s) len = return $ Just $ f (take len s)
data Token = TRes String | TString String | TIdent String | TInt Int

instance Show Token where
  show (TRes t) = "token " ++ t
  show (TString t) = "string " ++ t
  show (TIdent t) = "identifier " ++ t
  show (TInt t) = "integer " ++ show t


type AlexUserState = [String]

alexInitUserState = undefined

wrapError (Alex scanner) = Alex $ \s -> case scanner s of
  Left message -> Left (message ++ " at " ++ showpos (alex_pos s))
    where
      showpos (AlexPn off line col) = "line " ++ show line ++ ", column " ++ show col
  x -> x

scanner = do
  tok <- wrapError alexMonadScan
  case tok of
    Nothing -> do
      s <- getState
      case alex_scd s of
        0 -> return []
        string -> alexError "Unterminated string literal"
    Just tok -> liftM (tok:) scanner

runLexer :: String -> Either String [Token]
runLexer input = runAlex input scanner

}
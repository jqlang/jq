import Parser
import Lexer
import JQ
import Text.JSON
import Text.JSON.String
import PrettyJSON
import System.Environment
import Control.Monad
import System.IO

parseJS :: String -> JSValue
parseJS s = case runGetJSON readJSValue s of
  Left err -> error err
  Right val -> val

  
main = do
  [program] <- getArgs
  stdlib <- openFile "stdlib.jq" ReadMode >>= hGetContents
  json <- liftM parseJS $ hGetContents stdin
  case runLexer (stdlib ++ program) >>= runParser of
    Left err -> putStrLn err
    Right program -> mapM_ (putStrLn . show . renderJSON) (runJQ program json)
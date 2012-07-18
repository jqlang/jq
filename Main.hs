import Parser
import Lexer
import JQ
import Text.JSON
import Text.JSON.String
import System.Environment
import Control.Monad
import System.IO


parseJS :: String -> JSValue
parseJS s = case runGetJSON readJSValue s of
  Left err -> error err
  Right val -> val

  
main = do
  [program] <- getArgs
  json <- liftM parseJS $ hGetContents stdin
  case runLexer program >>= runParser of
    Left err -> putStrLn err
    Right program -> mapM_ (putStrLn . encode) (runJQ program json)
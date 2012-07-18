module JQ where
import Text.JSON
import Text.JSON.String
import Data.Maybe
import Data.List (sortBy,sort,groupBy)
import Data.Function (on)
import Data.Ord (comparing)
import Control.Monad
import Control.Monad.Writer
import Control.Monad.List
import Control.Monad.Reader

type Path = [Either Int String]

type Program = JSValue -> [(JSValue, Path)]

type JQ = ReaderT JSValue (WriterT Path [])

runJQ :: JQ a -> JSValue -> [a]
runJQ prog val = map fst $ runWriterT $ runReaderT prog val

(>|) :: JQ JSValue -> JQ a -> JQ a
a >| b = do
  val <- a
  local (const val) b

collect :: JQ a -> JQ [a]
collect prog = do
  arg <- ask
  return $ runJQ prog arg
  
collectPaths :: JQ a -> JQ [(a,Path)]
collectPaths prog = do
  arg <- ask
  return $ runWriterT $ runReaderT prog arg

insert :: JSValue -> (JSValue, Path) -> JSValue
insert base (replace, []) = replace
insert (JSArray values) (replace, ((Left n):rest)) = JSArray values'
    where
      (left, (_:right)) = splitAt n values
      values' = left ++ [replace] ++ right
insert (JSObject obj) (replace, ((Right k):rest))= JSObject $ toJSObject obj'
    where
      withoutK = filter ((/= k) . fst) $ fromJSObject obj
      obj' = (k, replace):withoutK


eqj a b = JSBool $ a == b


liftp :: (JSValue -> JSValue) -> JQ JSValue
liftp f = liftM f ask

idp = undefined
failp t = []

constp :: JSValue -> Program
constp t t' = idp t

anyj :: [JSValue] -> Bool
anyj values = any isTrue values
  where
    isTrue (JSBool False) = False
    isTrue (JSNull) = False
    isTrue _ = True

selectp prog = do
  match <- collect prog
  guard $ anyj match
  ask

constStr :: String -> JQ JSValue
constStr = return . JSString . toJSString

constInt :: Int -> JQ JSValue
constInt = return . JSRational False . toRational

updatep p = do
  t <- ask
  liftM (foldl insert t) $ collectPaths p

arrayp prog = liftM JSArray $ collect prog


childp' :: JSValue -> JQ JSValue
childp' (JSArray values) = msum [tell [Left i] >> return v | (v,i) <- zip values [0..]]
childp' (JSObject obj) = msum [tell [Right k] >> return v | (k,v) <- fromJSObject obj]
childp' _ = mzero

childp = ask >>= childp'

--findp :: Program -> Program
findp prog = do
  found <- collect prog
  if anyj found then ask else childp >| findp prog

groupp prog = do
  list <- ask
  case list of
    JSArray values -> do
      marked <- forM values $ \v -> do 
        m <- collect (return v >| prog)
        return (m,v)
      msum $
        map (return . JSArray . map snd) $ 
        groupBy ((==) `on` fst) $ 
        sortBy (comparing fst) $ 
        marked
    _ -> return JSNull


  

withArray f (JSArray values) = JSArray $ f values
withArray f x = x

callp "select" [p] = selectp p
callp "find" [p] = findp p
callp "set" [p] = updatep p
callp "sort" [] = liftp (withArray sort)
callp "group" [p] = groupp p

lookupj :: JSValue -> JSValue -> JQ JSValue
lookupj (JSArray values) (JSRational _ n) = do
  let idx = round n
  tell [Left idx]
  if idx >= 0 && idx < length values
    then return $ values !! idx
    else return $ JSNull
lookupj (JSObject obj) (JSString s) = do
  tell [Right (fromJSString s)]
  case (lookup (fromJSString s) (fromJSObject obj)) of
    Just x -> return x
    Nothing -> return JSNull
lookupj _ _ = mzero


plusj (JSRational _ n1) (JSRational _ n2) = JSRational True (n1 + n2)
plusj (JSString s1) (JSString s2) = JSString $ toJSString (fromJSString s1 ++ fromJSString s2)
plusj (JSArray a1) (JSArray a2) = JSArray $ a1 ++ a2


js :: JSON a => a -> JSValue
js = showJSON

index s = do
  v <- ask
  lookupj v (js s)


dictp progs = do
  liftM (JSObject . toJSObject) $ forM progs $ \(k,v) -> do
    JSString k' <- k
    v' <- v
    return (fromJSString k', v')


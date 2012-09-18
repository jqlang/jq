module JQ where
import Text.JSON
import Text.JSON.String
import Data.Maybe
import Data.Char
import Data.List (sortBy,sort,groupBy,partition,intercalate)
import Data.Function (on)
import Data.Ord (comparing)
import Control.Monad
import Control.Monad.Writer
import Control.Monad.List
import Control.Monad.Reader
import qualified Data.Map as M
import qualified Data.Set as S
import Data.Map ((!))
import Debug.Trace

type Path = [Either Int String]

type Program = JSValue -> [(JSValue, Path)]

type Filter = JQ JSValue
newtype Operator = Operator {runOperator:: [Filter] -> Filter}

type JQ = ReaderT (JSValue, M.Map String JSValue, M.Map (String, Int) Operator) (WriterT Path [])

runJQ :: JQ a -> JSValue -> [a]
runJQ prog val = map fst $ runWriterT $ runReaderT prog (val,M.empty,primitives)

(>|) :: JQ JSValue -> JQ a -> JQ a
a >| b = do
  val <- a
  local (\(v,s,d) -> (val,s,d)) b

setvar name val prog = 
  local (\(v,s,d) -> (v, M.insert name val s, d)) prog
getvar name = liftM (! name) $ asks (\(v,s,d) -> s)

input = asks (\(v,s,d) -> v)

collect :: JQ a -> JQ [a]
collect prog = liftM (map fst) $ collectPaths prog
  
collectPaths :: JQ a -> JQ [(a,Path)]
collectPaths prog = do
  rd <- ask
  return $ runWriterT $ runReaderT prog rd

withDefn :: (String, [String], Filter) -> Filter -> Filter
withDefn (name, formals, body) subexp = 
  local (\(v,s,d) -> (v,s,M.insert (name,length formals) (Operator func) d)) subexp
  where
    func args = local (\(v,s,d) -> (v,M.empty,M.fromList (zip (zip formals (repeat 0)) (map (Operator . const) args)) `M.union` d)) body

subexp :: JQ a -> JQ a
subexp = censor $ const []


yieldPaths :: [(a,Path)] -> JQ a
yieldPaths ps = ReaderT $ const $ WriterT ps


insert :: JQ JSValue -> JSValue -> Path -> JQ JSValue
insert replace base [] = replace
insert replace (JSArray values) ((Left n):rest) = do
  let array = take (max (n+1) (length values)) (values ++ repeat JSNull)
  replacement <- insert replace (array !! n) rest
  let (left, (_:right)) = splitAt n array
  return $ JSArray $ left ++ [replacement] ++ right
insert replace (JSObject obj) ((Right k):rest) = do
  let oldval = maybe JSNull id (lookup k $ fromJSObject obj)
  replacement <- insert replace oldval rest
  let withoutK = filter ((/= k) . fst) $ fromJSObject obj
  return $ JSObject $ toJSObject $ (k, replacement):withoutK

insert replace JSNull p@((Right k):rest) = insert replace (JSObject $ toJSObject []) p
insert replace JSNull p@((Left n):rest)  = insert replace (JSArray []) p
insert _ base p = error $ "Cannot insert into " ++ intercalate ", " (map (either show show) p) ++ " of " ++ encode base


eqj a b = JSBool $ a == b

boolj (JSBool false) = False
boolj (JSNull) = False
boolj _ = True

andj a b = JSBool $ boolj a && boolj b
orj a b = JSBool $ boolj a || boolj b

liftp :: (JSValue -> JSValue) -> JQ JSValue
liftp f = liftM f input

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
  input

constStr :: String -> JQ JSValue
constStr = return . JSString . toJSString

constInt :: Int -> JQ JSValue
constInt = return . JSRational False . toRational

tr x = trace (show x) x


assignp sel replace = do
  paths <- collectPaths sel
  t <- input
  foldM (\base (val,path) -> insert (return val >| replace) base path) t paths

arrayp prog = liftM JSArray $ collect prog


childp' :: JSValue -> JQ JSValue
childp' (JSArray values) = msum [tell [Left i] >> return v | (v,i) <- zip values [0..]]
childp' (JSObject obj) = msum [tell [Right k] >> return v | (k,v) <- fromJSObject obj]
childp' _ = mzero

childp = input >>= childp'

--findp :: Program -> Program
findp prog = do
  found <- collect prog
  if anyj found then input else childp >| findp prog

groupp prog = do
  list <- input
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

recp prog = do
  found <- collectPaths prog
  let (roots,subs) = partition (null . snd) found
  msum $ 
    [tell p >> return x | (x,p) <- roots] ++
    [tell p >> (return x >| recp prog) | (x,p) <- subs]

elsep p1 p2 = do
  p1' <- collectPaths p1
  if null p1' then p2 else yieldPaths p1'

fullresultp prog = do
  res <- collectPaths prog
  msum [return $ JSObject $ toJSObject $ [("val",a),("path",JSArray $ map fromPath p)] | (a,p) <- res]
    where
      fromPath (Left n) = js n
      fromPath (Right s) = js s


withArray f (JSArray values) = JSArray $ f values
withArray f x = x

withString f (JSString str) = JSString $ toJSString $ f $ fromJSString str
withString f x = x

{-
callp "select" [p] = selectp p
callp "find" [p] = findp p
callp "sort" [] = liftp (withArray sort)
callp "group" [p] = groupp p
callp "rec" [p] = recp p
callp "empty" [] = mzero
callp "true" [] = return $ JSBool True
callp "false" [] = return $ JSBool False
callp "null" [] = return $ JSNull
callp "count" [] = liftp countj
callp "fullresult" [p] = fullresultp p
callp "uppercase" [] = liftp $ withString $ map toUpper
callp "lowercase" [] = liftp $ withString $ map toLower
-}

primitives = M.fromList [((name,arglen),Operator func) | 
                         (name,arglen,func) <- prim]
  where
    prim = [("if",1,\[p] -> selectp p),
            ("find", 1, \[p] -> findp p),
            ("group", 1, \[p] -> groupp p),
            ("rec", 1, \[p] -> recp p), 
            ("true", 0, const $ return $ JSBool True),
            ("false", 0, const $ return $ JSBool False),
            ("null", 0, const $ return $ JSNull),
            ("count", 0, const $ liftp countj),
            ("fullresult", 1, \[p] -> fullresultp p),
            ("zip", 0, const $ liftp zipj),
            ("keys", 0, const $ liftp keysj)
            ]

callp :: String -> [Filter] -> Filter

callp name args = do
  (v,s,d) <- ask
  runOperator (d ! (name, length args)) args

countj (JSArray v) = js$ length v
countj (JSObject o) = js$ length $ fromJSObject o
countj _ = js$ (1::Int)

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
lookupj JSNull (JSRational _ n) = do
  tell [Left $ round n]
  return JSNull
lookupj JSNull (JSString s) = do
  tell [Right (fromJSString s)]
  return JSNull
--lookupj v i = error $ "Cannot get element " ++ encode i ++ " of " ++ encode v
lookupj _ _ = mzero


plusj (JSRational f1 n1) (JSRational f2 n2) = JSRational (f1 || f2) (n1 + n2)
plusj (JSString s1) (JSString s2) = JSString $ toJSString (fromJSString s1 ++ fromJSString s2)
plusj (JSArray a1) (JSArray a2) = JSArray $ a1 ++ a2
plusj (JSObject o1) (JSObject o2) = JSObject $ toJSObject $ o1' ++ fromJSObject o2
  where
    newkeys = map fst $ fromJSObject o2
    o1' = filter (not . (`elem` newkeys) . fst) $ fromJSObject o1

keysj (JSArray v) = js [0..length v - 1]
keysj (JSObject obj) = js (map fst $ fromJSObject obj)
keysj _ = JSArray []

zipj jsonValue = result $ tx values ctor
  where
    (result, values) = extract packed
    (packed, ctor) = case jsonValue of
      JSArray values -> (values, (\vs' -> JSArray $ [v' | Just v' <- vs']))
      JSObject jsObject -> 
        let object = fromJSObject jsObject
            keys = map fst object
            values = map snd object
            build vs' = [(k,v') | (k,Just v') <- zip keys vs']
        in (values, JSObject . toJSObject . build)
      
      _ -> error "only arrays and objects may be zipped"

    extract values | all isArray values = (JSArray, [map Just arr | JSArray arr <- values])
                   | all isObject values = 
      let objects = [fromJSObject o | JSObject o <- values]
          keys = S.toList $ S.fromList [k | obj <- objects, (k,_) <- obj]
          values' :: [[Maybe JSValue]]
          values' = [[lookup k object | k <- keys] | object <- objects]
          result r = JSObject $ toJSObject $ zip keys r
      in (result, values')
                   | otherwise = error "elements of zipped value must be all objects or all arrays"
      where
        isArray (JSArray a) = True
        isArray _ = False
        isObject (JSObject o) = True
        isObject _ = False
    
    head' [] = Nothing
    head' (x:xs) = x
    tail' [] = []
    tail' (x:xs) = xs
    tx values ctor | all null values = []
                   | otherwise = ctor (map head' values):tx (map tail' values) ctor

js :: JSON a => a -> JSValue
js = showJSON

index s = do
  v <- input
  lookupj v (js s)


dictp progs = do
  liftM (JSObject . toJSObject) $ forM progs $ \(k,v) -> do
    JSString k' <- subexp k
    v' <- subexp v
    return (fromJSString k', v')


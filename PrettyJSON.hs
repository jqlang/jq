module PrettyJSON where
import Text.JSON
import Text.PrettyPrint

renderJSON (JSArray vals) = brackets $ fsep $ punctuate comma $ map renderJSON vals

renderJSON (JSObject jsObject) = 
  let object = fromJSObject jsObject
  in braces $ fsep $ punctuate comma $ 
     [hang (renderJSON (JSString $ toJSString $ k) <> colon) 2 (renderJSON v)
     | (k,v) <- object]
renderJSON x = text $ encode x
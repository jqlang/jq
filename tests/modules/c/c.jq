module c {whatever:null};
import a as foo;
import d as d {search:"./"};
import d {search:"./"};
import e as e {search:"./../lib/jq"};
import f as f {search:"./../lib/jq"};
def a: 0;
def c:
  if meh != d::meh then error("import into namespace doesn't work")
  elif foo::a != "a" then error("foo::a didn't work as expected")
  elif d::meh != "meh" then error("d::meh didn't work as expected")
  elif e::bah != "bah" then error("e::bah didn't work as expected")
  elif f::f != "f is here" then error("f::f didn't work as expected")
  else foo::a + "c" + d::meh + e::bah end;

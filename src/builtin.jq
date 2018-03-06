def halt_error: halt_error(5);
def error: error(.);
def map(f): [.[] | f];
def select(f): if f then . else empty end;
def sort_by(f): _sort_by_impl(map([f]));
def group_by(f): _group_by_impl(map([f]));
def unique: group_by(.) | map(.[0]);
def unique_by(f): group_by(f) | map(.[0]);
def max_by(f): _max_by_impl(map([f]));
def min_by(f): _min_by_impl(map([f]));
def add: reduce .[] as $x (null; . + $x);
def del(f): delpaths([path(f)]);
def _assign(paths; value): value as $v | reduce path(paths) as $p (.; setpath($p; $v));
def _modify(paths; update): reduce path(paths) as $p (.; label $out | (setpath($p; getpath($p) | update) | ., break $out), delpaths([$p]));
def map_values(f): .[] |= f;

# recurse
def recurse(f): def r: ., (f | r); r;
def recurse(f; cond): def r: ., (f | select(cond) | r); r;
def recurse: recurse(.[]?);
def recurse_down: recurse;

def to_entries: [keys_unsorted[] as $k | {key: $k, value: .[$k]}];
def from_entries: map({(.key // .Key // .name // .Name): (if has("value") then .value else .Value end)}) | add | .//={};
def with_entries(f): to_entries | map(f) | from_entries;
def reverse: [.[length - 1 - range(0;length)]];
def indices($i): if type == "array" and ($i|type) == "array" then .[$i]
  elif type == "array" then .[[$i]]
  elif type == "string" and ($i|type) == "string" then _strindices($i)
  else .[$i] end;
def index($i):   indices($i) | .[0];       # TODO: optimize
def rindex($i):  indices($i) | .[-1:][0];  # TODO: optimize
def paths: path(recurse(if (type|. == "array" or . == "object") then .[] else empty end))|select(length > 0);
def paths(node_filter): . as $dot|paths|select(. as $p|$dot|getpath($p)|node_filter);
def any(generator; condition):
        [label $out | foreach generator as $i
                 (false;
                  if . then break $out elif $i | condition then true else . end;
                  if . then . else empty end)] | length == 1;
def any(condition): any(.[]; condition);
def any: any(.);
def all(generator; condition):
        [label $out | foreach generator as $i
                 (true;
                  if .|not then break $out elif $i | condition then . else false end;
                  if .|not then . else empty end)] | length == 0;
def all(condition): all(.[]; condition);
def all: all(.);
def isfinite: type == "number" and (isinfinite | not);
def arrays: select(type == "array");
def objects: select(type == "object");
def iterables: arrays, objects;
def booleans: select(type == "boolean");
def numbers: select(type == "number");
def normals: select(isnormal);
def finites: select(isfinite);
def strings: select(type == "string");
def nulls: select(type == "null");
def values: select(. != null);
def scalars: select(. == null or . == true or . == false or type == "number" or type == "string");
def scalars_or_empty: select(. == null or . == true or . == false or type == "number" or type == "string" or ((type=="array" or type=="object") and length==0));
def leaf_paths: paths(scalars);
def join($x): reduce .[] as $i (null;
            (if .==null then "" else .+$x end) +
            ($i | if type=="boolean" or type=="number" then tostring else .//"" end)
        ) // "";
def _flatten($x): reduce .[] as $i ([]; if $i | type == "array" and $x != 0 then . + ($i | _flatten($x-1)) else . + [$i] end);
def flatten($x): if $x < 0 then error("flatten depth must not be negative") else _flatten($x) end;
def flatten: _flatten(-1);
def range($x): range(0;$x);
def fromdateiso8601: strptime("%Y-%m-%dT%H:%M:%SZ")|mktime;
def todateiso8601: strftime("%Y-%m-%dT%H:%M:%SZ");
def fromdate: fromdateiso8601;
def todate: todateiso8601;
def match(re; mode): _match_impl(re; mode; false)|.[];
def match($val): ($val|type) as $vt | if $vt == "string" then match($val; null)
   elif $vt == "array" and ($val | length) > 1 then match($val[0]; $val[1])
   elif $vt == "array" and ($val | length) > 0 then match($val[0]; null)
   else error( $vt + " not a string or array") end;
def test(re; mode): _match_impl(re; mode; true);
def test($val): ($val|type) as $vt | if $vt == "string" then test($val; null)
   elif $vt == "array" and ($val | length) > 1 then test($val[0]; $val[1])
   elif $vt == "array" and ($val | length) > 0 then test($val[0]; null)
   else error( $vt + " not a string or array") end;
def capture(re; mods): match(re; mods) | reduce ( .captures | .[] | select(.name != null) | { (.name) : .string } ) as $pair ({}; . + $pair);
def capture($val): ($val|type) as $vt | if $vt == "string" then capture($val; null)
   elif $vt == "array" and ($val | length) > 1 then capture($val[0]; $val[1])
   elif $vt == "array" and ($val | length) > 0 then capture($val[0]; null)
   else error( $vt + " not a string or array") end;
def scan(re):
  match(re; "g")
  |  if (.captures|length > 0)
      then [ .captures | .[] | .string ]
      else .string
      end ;
#
# If input is an array, then emit a stream of successive subarrays of length n (or less),
# and similarly for strings.
def _nwise(a; $n): if a|length <= $n then a else a[0:$n] , _nwise(a[$n:]; $n) end;
def _nwise($n): _nwise(.; $n);
#
# splits/1 produces a stream; split/1 is retained for backward compatibility.
def splits($re; flags): . as $s
#  # multiple occurrences of "g" are acceptable
  | [ match($re; "g" + flags) | (.offset, .offset + .length) ]
  | [0] + . +[$s|length]
  | _nwise(2)
  | $s[.[0]:.[1] ] ;
def splits($re): splits($re; null);
#
# split emits an array for backward compatibility
def split($re; flags): [ splits($re; flags) ];
#
# If s contains capture variables, then create a capture object and pipe it to s
def sub($re; s):
  . as $in
  | [match($re)]
  | if length == 0 then $in
    else .[0]
    | . as $r
#  # create the "capture" object:
    | reduce ( $r | .captures | .[] | select(.name != null) | { (.name) : .string } ) as $pair
        ({}; . + $pair)
    | $in[0:$r.offset] + s + $in[$r.offset+$r.length:]
    end ;
#
# If s contains capture variables, then create a capture object and pipe it to s
def sub($re; s; flags):
  def subg: [explode[] | select(. != 103)] | implode;
  # "fla" should be flags with all occurrences of g removed; gs should be non-nil if flags has a g
  def sub1(fla; gs):
    def mysub:
      . as $in
      | [match($re; fla)]
      | if length == 0 then $in
        else .[0] as $edit
        | ($edit | .offset + .length) as $len
        # create the "capture" object:
        | reduce ( $edit | .captures | .[] | select(.name != null) | { (.name) : .string } ) as $pair
            ({}; . + $pair)
        | $in[0:$edit.offset]
          + s
          + ($in[$len:] | if length > 0 and gs then mysub else . end)
        end ;
    mysub ;
    (flags | index("g")) as $gs
    | (flags | if $gs then subg else . end) as $fla
    | sub1($fla; $gs);
#
def sub($re; s): sub($re; s; "");
# repeated substitution of re (which may contain named captures)
def gsub($re; s; flags): sub($re; s; flags + "g");
def gsub($re; s): sub($re; s; "g");

########################################################################
# range/3, with a `by` expression argument
def range($init; $upto; $by):
    def _range:
        if ($by > 0 and . < $upto) or ($by < 0 and . > $upto) then ., ((.+$by)|_range) else . end;
    if $by == 0 then $init else $init|_range end | select(($by > 0 and . < $upto) or ($by < 0 and . > $upto));
# generic iterator/generator
def while(cond; update):
     def _while:
         if cond then ., (update | _while) else empty end;
     _while;
def until(cond; next):
     def _until:
         if cond then . else (next|_until) end;
     _until;
def limit($n; exp):
  if $n < 0 then exp
  else label $out | foreach exp as $item ($n; .-1; $item, if . <= 0 then break $out else empty end)
  end;
def isempty(g): 0 == ((label $go | g | (1, break $go)) // 0);
def first(g): label $out | g | ., break $out;
def last(g): reduce g as $item (null; $item);
def nth($n; g): if $n < 0 then error("nth doesn't support negative indices") else last(limit($n + 1; g)) end;
def first: .[0];
def last: .[-1];
def nth($n): .[$n];
def combinations:
    if length == 0 then [] else
        .[0][] as $x
          | (.[1:] | combinations) as $y
          | [$x] + $y
    end;
def combinations(n):
    . as $dot
      | [range(n) | $dot]
      | combinations;
# transpose a possibly jagged matrix, quickly;
# rows are padded with nulls so the result is always rectangular.
def transpose:
  if . == [] then []
  else . as $in
  | (map(length) | max) as $max
  | length as $length
  | reduce range(0; $max) as $j
      ([]; . + [reduce range(0;$length) as $i ([]; . + [ $in[$i][$j] ] )] )
	        end;
def in(xs): . as $x | xs | has($x);
def inside(xs): . as $x | xs | contains($x);
def input: _input;
def repeat(exp):
     def _repeat:
         exp, _repeat;
     _repeat;
def inputs: try repeat(_input) catch if .=="break" then empty else .|error end;
# like ruby's downcase - only characters A to Z are affected
def ascii_downcase:
  explode | map( if 65 <= . and . <= 90 then . + 32  else . end) | implode;
# like ruby's upcase - only characters a to z are affected
def ascii_upcase:
  explode | map( if 97 <= . and . <= 122 then . - 32  else . end) | implode;

# Streaming utilities
def truncate_stream(stream):
  . as $n | null | stream | . as $input | if (.[0]|length) > $n then setpath([0];$input[0][$n:]) else empty end;
def fromstream(i):
  foreach i as $item (
    [null,false,null,false];
    if ($item[0]|length) == 0 then [null,false,.[2],.[3]]
    elif ($item|length) == 1 and ($item[0]|length) < 2 then [null,false,.[0],.[1]]
    else . end |
    . as $state |
    if ($item|length) > 1 and ($item[0]|length) > 0 then
      [.[0]|setpath(($item|.[0]); ($item|.[1])),
      true,
      $state[2],
      $state[3]]
    else .
    end;
    if ($item[0]|length) == 1 and ($item|length == 1) and .[3] then .[2] else empty end,
    if ($item[0]|length) == 0 then $item[1] else empty end
    );
def tostream:
  {string:true,number:true,boolean:true,null:true} as $leaf_types |
  . as $dot |
  if $leaf_types[$dot|type] or length==0 then [[],$dot]
  else
    # We really need a _streaming_ form of `keys`.
    # We can use `range` for arrays, but not for objects.
    keys_unsorted as $keys |
    $keys[-1] as $last|
    ((# for each key
      $keys[] | . as $key |
      $dot[$key] | . as $dot |
      # recurse on each key/value
      tostream|.[0]|=[$key]+.),
     # then add the closing marker
     [[$last]])
  end;


# Assuming the input array is sorted, bsearch/1 returns
# the index of the target if the target is in the input array; and otherwise
#  (-1 - ix), where ix is the insertion point that would leave the array sorted.
# If the input is not sorted, bsearch will terminate but with irrelevant results.
def bsearch(target):
  if length == 0 then -1
  elif length == 1 then
     if target == .[0] then 0 elif target < .[0] then -1 else -2 end
  else . as $in
    # state variable: [start, end, answer]
    # where start and end are the upper and lower offsets to use.
    | [0, length-1, null]
    | until( .[0] > .[1] ;
             if .[2] != null then (.[1] = -1)               # i.e. break
             else
               ( ( (.[1] + .[0]) / 2 ) | floor ) as $mid
               | $in[$mid] as $monkey
               | if $monkey == target  then (.[2] = $mid)   # success
                 elif .[0] == .[1]     then (.[1] = -1)     # failure
                 elif $monkey < target then (.[0] = ($mid + 1))
                 else (.[1] = ($mid - 1))
                 end
             end )
    | if .[2] == null then          # compute the insertion point
         if $in[ .[0] ] < target then (-2 -.[0])
         else (-1 -.[0])
         end
      else .[2]
      end
  end;

# Apply f to composite entities recursively, and to atoms
def walk(f):
  . as $in
  | if type == "object" then
      reduce keys_unsorted[] as $key
        ( {}; . + { ($key):  ($in[$key] | walk(f)) } ) | f
  elif type == "array" then map( walk(f) ) | f
  else f
  end;

# SQL-ish operators here:
def INDEX(stream; idx_expr):
  reduce stream as $row ({};
    .[$row|idx_expr|
      if type != "string" then tojson
      else .
      end] |= $row);
def INDEX(idx_expr): INDEX(.[]; idx_expr);
def JOIN($idx; idx_expr):
  [.[] | [., $idx[idx_expr]]];
def JOIN($idx; stream; idx_expr):
  stream | [., $idx[idx_expr]];
def JOIN($idx; stream; idx_expr; join_expr):
  stream | [., $idx[idx_expr]] | join_expr;
def IN(s): reduce (first(select(. == s)) | true) as $v (false; if . or $v then true else false end);
def IN(src; s): reduce (src|IN(s)) as $v (false; if . or $v then true else false end);

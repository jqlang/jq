
# Filter and adjust streamed values so that only values from the .th
# level are output.
def trunc(stream):
  . as $n | stream | . as $input | if (.[0]|length) > $n then setpath([0];$input[0][$n:]) else empty end;

# Reduce streamed values back to normal
def tovalues(i):
  def debug(msg): . as $dot | [msg, .] | debug | $dot;
  foreach i as $item (
    [null,false,null];

    # Updator
    # 
    # If the new $item is a top-level value,
    # then clear out the current value
    .                   as [$cur, $cur_isvalid, $prev]  |
    $item               as [$path, $leaf]               |
    ($item|length > 1)  as $has_leaf                    |
    ($item|length == 1) as $closing                     |
    ($path|length)      as $plen                        |
    # if the new $item terminates the current value, then cur is ready
    # for extraction and we'll start building a new value with the next
    # inputs
    if ($plen == 0) or # top-level scalar
       ($closing and $plen < 2) then [null,false,$cur]
    # else continue building up cur
    else . end                                          |
    . as [$cur, $cur_isvalid, $prev]                    |
    # If the new $item has a leaf, upate the current value
    if $has_leaf and $plen > 0 then
      [$cur|setpath(($path); $leaf),              # update current value
      true,                                         # current value is now valid (if, perhaps, incomplete)
      $prev]                                        # previous value is unchanged
    else .
    end;

    # Extractor
    #
    . as [$cur, $cur_isvalid, $prev]                    |
    $item as [$path, $leaf]                             |
    ($item|length > 1) as $has_leaf                     |
    ($item|length == 1) as $closing                     |
    ($path|length) as $plen                             |
    # If previous value is valid, output it
    if $plen == 1 and $closing then $prev else empty end,
    # and/or if the new $item is a top-level scalar, output it
    if $plen == 0 then $leaf else empty end
    );

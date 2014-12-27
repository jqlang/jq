
# Filter and adjust streamed values so that only values from the .th
# level are output.
def trunc(inputs):
  . as $n | inputs | . as $input | if (.[0]|length) > 1 then setpath([0];$input[0][1:]) else empty end;

# Reduce streamed values back to normal
def tovalues(i):
  foreach i as $item (
    # [<current value being built>,
    #  <is current value valid?>,
    #  <previous, complete value>,
    #  <is previous value valid?>]
    [null,false,null,false];

    # Updator
    # 
    # If the new $item is a top-level value,
    # then clear out the current value
    if ($item[0]|length) == 0 then [null,false,.[2],.[3]]
    # else if the new $item terminates the current value,
    # then rotate the current value into the previous value slot.
    elif ($item|length) == 1 and ($item[0]|length) < 2 then [null,false,.[0],.[1]]
    else . end |
    . as $state |
    # If the new $item has a leaf, upate the current value
    if ($item|length) > 1 and ($item[0]|length) > 0 then
      [.[0]|setpath(($item|.[0]); ($item|.[1])),    # update current value
      true,                                         # current value is now valid (if, perhaps, incomplete)
      $state[2],                                    # previous value is unchanged
      $state[3]]                                    # previous value is unchanged
    else .
    end;

    # Extractor
    #
    # If previous value is valid, output it
    if ($item[0]|length) == 1 and ($item|length == 1) and .[3] then .[2] else empty end,
    # and/or if the new $item is a top-level scalar, output it
    if ($item[0]|length) == 0 then $item[1] else empty end
    );

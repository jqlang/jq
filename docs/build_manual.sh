#!/bin/bash
echo "headline: jq Manual"
echo "body: |"
pandoc content/manual/dev/manual.md --lua-filter filters/filter.lua \
  --shift-heading-level-by=1 --section-divs | \
  sed 's/\(.*\)/  \1/g'
echo "toc:"
pandoc content/manual/dev/manual.md --lua-filter filters/toc.lua

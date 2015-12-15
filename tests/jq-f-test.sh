#!/bin/sh
# this next line is ignored by jq, which otherwise does not continue comments \
exec jq -nef "$0" "$@"
true

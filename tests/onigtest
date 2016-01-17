#!/bin/sh

. "${0%/*}/setup" "$@"

$VALGRIND $Q $JQ -L "$mods" --run-tests $JQTESTDIR/onig.test

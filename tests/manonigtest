#!/bin/sh

. "${0%/*}/setup" "$@"

$VALGRIND $Q $JQ -L "$mods" --run-tests $JQBASEDIR/tests/manonig.test

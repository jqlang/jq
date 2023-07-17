#!/bin/sh

. "${0%/*}/setup" "$@"

# We set PAGER because there's a mantest for `env` that uses it.
env PAGER=less $VALGRIND $Q $JQ -L "$mods" --run-tests $JQBASEDIR/tests/man.test

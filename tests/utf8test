#!/bin/sh

. "${0%/*}/setup" "$@"

if [ "$($VALGRIND $Q $JQ -nf $JQTESTDIR/utf8-truncate.jq)" != "true" ]; then
	echo "UTF-8 byte sequences that span the jv_load_file read buffer are mangled"
	exit 1
fi

exit 0

#!/bin/sh

# This script is used to cross-compile binaries for
# platforms other than the current one

# Usage: $0 <platformname> <configure options>
# <platformname> is arbitrary, it is the name
# of the directory which will be created to contain
# the output binaries.

# e.g. $0 win32 --host=i686-w64-mingw32

set -e
cd `dirname "$0"`/../build

jobs=-j4
case "X$1" in
X-j*) jobs=$1; shift;;
esac

plat="$1"
[ -z "$plat" ] && exit 1
shift

case "$plat" in
*/*) echo "platform name must not be a path"; exit 1;;
*..*) echo "platform name must not be a path"; exit 1;;
*) plat=$PWD/$plat;;
esac

[ -d "$plat" ] || mkdir "$plat"
rm -rf "$plat/tmp"
mkdir "$plat/tmp"
cd "$plat/tmp"
../../../configure "$@"
make "$jobs" DESTDIR=$plat install
set -x
for jq in `find . -type f \( -name jq -o -name jq.exe \) -print`; do
    cp "$jq" ..
done
cd ..
rm -rf tmp


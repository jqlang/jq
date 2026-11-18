#!/usr/bin/env bash
# Build and install this jq (with yyjson and stb_sprintf) from a git checkout.
# Usage: ./install.sh [--prefix DIR] [--no-check]
# Run as your normal user; sudo is only used for the final "make install".
set -euo pipefail

prefix=/usr/local
check=1
while [ $# -gt 0 ]; do
  case "$1" in
    --prefix) prefix="$2"; shift 2 ;;
    --no-check) check=0; shift ;;
    -h|--help) sed -n '2,4p' "$0"; exit 0 ;;
    *) echo "unknown option: $1" >&2; exit 2 ;;
  esac
done

if [ "$(id -u)" -eq 0 ]; then
  echo "Run this as a normal user, not root (it uses sudo only to install)." >&2
  exit 1
fi

cd "$(dirname "$0")"
jobs=$(nproc 2>/dev/null || echo 4)

# Root-owned leftovers from an earlier "sudo make" break autoreconf.
if [ -n "$(find . -not -user "$(id -un)" -not -path './.git/*' -print -quit 2>/dev/null)" ]; then
  echo "Some files are not owned by you; fixing with sudo chown..."
  sudo chown -R "$(id -un):$(id -gn)" .
fi

echo "==> Fetching submodules (oniguruma, yyjson, stb_sprintf)"
git submodule update --init

echo "==> Generating build files"
rm -rf autom4te.cache
autoreconf -fi

echo "==> Configuring (prefix=$prefix)"
# -O3 + LTO measured ~5% faster than -O2.  Override with CFLAGS/LDFLAGS in the
# environment (e.g. CFLAGS="-O3 -march=native" for a machine-specific build).
./configure --with-oniguruma=builtin --prefix="$prefix" \
  CFLAGS="${CFLAGS:--O3 -flto}" LDFLAGS="${LDFLAGS:--flto}"

echo "==> Building"
make clean >/dev/null 2>&1 || true
make -j"$jobs"

if [ "$check" -eq 1 ]; then
  echo "==> Running tests"
  make check -j"$jobs"
fi

echo "==> Installing (sudo)"
sudo make install
sudo ldconfig 2>/dev/null || true

hash -r
echo "==> Installed: $(command -v jq) -> $(jq --version)"
echo "    libjq loaded at run time: $(ldd "$prefix/bin/jq" 2>/dev/null | awk '/libjq/{print $3}')"
echo "    (empty above means libjq is linked into the jq binary, which is what we want)"

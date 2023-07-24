#!/usr/bin/env bash
# Mac C. compile-ios.sh for JQ.

# Defaults
set -e
oniguruma='6.9.3'

unset CFLAGS
unset CXXFLAGS
unset LDFLAGS

# Parse args. 
usage(){
cat << EOF
${0##*/}: usage

    Description:
       This simple script builds oniguruma and jq for all *-apple-darwin devices.

    Arguments:
    --extra-cflags <arg>: Pass defines or includes to clang.
    --extra-ldflags <arg>: Pass libs or includes to ld64.

    --with-oniguruma <arg>: Change default version of onigurma from ${oniguruma}.
EOF
exit 1
}

while (( $# )); do
   case "$1" in
      --with-oniguruma) shift; oniguruma="${1}" ;;

      --extra-cflags) shift; export CFLAGS_="${1}" ;;
      --extra-ldflags) shift; export LDFLAGS_="${1}" ;;

      --help) usage ;;
      *) echo -e "Unknown option: ${1}\n"; usage ;;
   esac
   shift
done 

# Start building.
echo "Building..."
MAKEJOBS="$(sysctl -n hw.ncpu || echo 1)"
CC_="$(xcrun -f clang || echo clang)"

onig_url="https://github.com/kkos/oniguruma/releases/download/v${oniguruma}/onig-${oniguruma}.tar.gz"
builddir="${TMPDIR:-/tmp}/${RANDOM:-'xxxxx'}-compile-ios-build"
cwd="$(realpath ${PWD} 2>/dev/null || echo ${PWD})"

t_exit() {
cat << EOF

A error as occurred.
    oniguruma location: ${builddir}/onig/onig-${oniguruma}
    jq location: ${cwd}

    Provide config.log and console logs when posting a issue.

EOF
}
trap t_exit ERR

#  Onig.
mkdir -p "${builddir}/onig"
cd "${builddir}/"
 curl -L ${onig_url} | tar xz
 for arch in i386 x86_64 armv7 armv7s arm64; do
     if [[ "$arch" = "i386" || "$arch" = "x86_64" ]]; then
         SYSROOT=$(xcrun -f --sdk iphonesimulator --show-sdk-path)
     else
         SYSROOT=$(xcrun -f --sdk iphoneos --show-sdk-path)
     fi
     HOST="${arch}-apple-darwin"
     [[ "${arch}" = "arm64" ]] && HOST="aarch64-apple-darwin"

     CFLAGS="-arch ${arch} -miphoneos-version-min=9.0 -isysroot ${SYSROOT} ${CFLAGS_} -D_REENTRANT"
     LDFLAGS="-arch ${arch} -miphoneos-version-min=9.0 -isysroot ${SYSROOT} ${LDFLAGS_}"
     CC="${CC_} ${CFLAGS}"

     # ./configure; make install
     cd "${builddir}/onig-${oniguruma}"
     CC=${CC} LDFLAGS=${LDFLAGS} \
     ./configure --host=${HOST} --build=$(./config.guess) --enable-shared=no --enable-static=yes --prefix=/
     make -j${MAKEJOBS} install DESTDIR="${cwd}/ios/onig/${arch}"
     make clean
     
     # Jump back to JQ.
     cd ${cwd}
     [[ ! -f ./configure ]] && autoreconf -ivf
     CC=${CC} LDFLAGS=${LDFLAGS} \
     ./configure --host=${HOST} --build=$(./config/config.guess) --enable-docs=no --enable-shared=no --enable-static=yes --prefix=/ --with-oniguruma=${cwd}/ios/onig/${arch} $(test -z ${BISON+x} || echo '--enable-maintainer-mode')
     make -j${MAKEJOBS} install DESTDIR="${cwd}/ios/jq/${arch}"
     make clean
 done

mkdir -p "${cwd}/ios/dest/lib"
# lipo, make a static lib.
lipo -create -output ${cwd}/ios/dest/lib/libonig.a ${cwd}/ios/onig/{i386,x86_64,armv7,armv7s,arm64}/lib/libonig.a
lipo -create -output ${cwd}/ios/dest/lib/libjq.a ${cwd}/ios/jq/{i386,x86_64,armv7,armv7s,arm64}/lib/libjq.a

# Take the arm64 headers- the most common target.
cp -r ${cwd}/ios/jq/arm64/include ${cwd}/ios/dest/
rm -rf ${cwd}/build/ios/{i386,x86_64,armv7,armv7s,arm64}

echo "Output to ${cwd}/ios/dest"

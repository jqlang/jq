#!/bin/bash

# old values
OLD_CFLAGS=$CFLAGS
OLD_LDFLAGS=$LDFLAGS
ORIG_PWD=`pwd`

# For parallelism in make
NJOBS="-j`sysctl -n hw.ncpu || echo 1`"


# Get oniguruma
rm -rf $PWD/build/ios oniguruma-5.9.6
echo "Downloading oniguruma 5.9.6"
curl -L https://github.com/kkos/oniguruma/archive/v5.9.6.tar.gz | tar xz
cd oniguruma-5.9.6

# So, we need to remake the configure scripts so that the arm64 architecture
# exists in config.sub.  In order to keep autoreconf from failing, create
# NEWS and ChangeLog.
touch NEWS ChangeLog
autoreconf -fi >/dev/null 2>&1




CC=`xcrun -f clang`
cd $ORIG_PWD

autoreconf -fi
for arch in i386 x86_64 armv7 armv7s arm64; do

	# Some of the architectures are a bit different...
	if [[ "$arch" = "i386" || "$arch" = "x86_64" ]]
	then
		SYSROOT=`xcrun -f --sdk iphonesimulator --show-sdk-path`
	else
		SYSROOT=`xcrun -f --sdk iphoneos --show-sdk-path`
	fi
	if [[ "$arch" = "arm64" ]]
	then
		HOST="aarch64-apple-darwin"
	else
		HOST="$arch-apple-darwin"
	fi

	CFLAGS="-arch $arch -miphoneos-version-min=6.0 -isysroot $SYSROOT $OLD_CFLAGS"
	LDFLAGS="-arch $arch -miphoneos-version-min=6.0 -isysroot $SYSROOT $OLD_LDFLAGS"



	# Build oniguruma for this architecture
	cd oniguruma-5.9.6
	CC=$CC CFLAGS=$CFLAGS LDFLAGS=$LDFLAGS ./configure --disable-shared --enable-static --host=$HOST --prefix=$ORIG_PWD/build/ios/$arch
	STATUS=$?
	if [ $STATUS -ne 0 ]
	then
		echo "Failed to configure oniguruma for architecture $arch.  Check `pwd`/config.log for details."
		cd $PWD
		exit $STATUS
	fi
	make clean
	make $NJBOS install
	STATUS=$?
	if [ $STATUS -ne 0 ]
	then
		echo "Failed to make oniguruma for architecture $arch."
		cd $PWD
		exit $STATUS
	fi



	# Build jq for this architecture
	cd $ORIG_PWD
	CC=$CC CFLAGS=$CFLAGS LDFLAGS=$LDFLAGS ./configure --disable-shared --enable-static -host=$HOST --prefix=$ORIG_PWD/build/ios/$arch --with-oniguruma=$ORIG_PWD/build/ios/$arch
	STATUS=$?
	if [ $STATUS -ne 0 ]
	then
		echo "Failed to configure jq for architecture $arch"
		exit $STATUS
	fi
	make clean
	make $NJOBS install
	STATUS=$?
	if [ $STATUS -ne 0 ]
	then
		echo "Failed to make jq for architecture $arch"
		exit $STATUS
	fi
done


# lipo together the different architectures into a universal 'fat' file
lipo -create -output $ORIG_PWD/build/ios/libonig.a $ORIG_PWD/build/ios/{i386,x86_64,armv7,armv7s,arm64}/lib/libonig.a
lipo -create -output $ORIG_PWD/build/ios/libjq.a $ORIG_PWD/build/ios/{i386,x86_64,armv7,armv7s,arm64}/lib/libjq.a

# copy the products into the destination directory and clean up the single-architecture files.
cp $ORIG_PWD/build/ios/i386/include/*.h $ORIG_PWD/build/ios/
rm -rf $ORIG_PWD/build/ios/{i386,x86_64,armv7,armv7s,arm64}

echo "Products are in $ORIG_PWD/build/ios"

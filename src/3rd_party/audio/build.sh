#!/bin/bash

set -x
set -e

CUR_FILE=`readlink -f $0`
CUR_DIR=`dirname $CUR_FILE`
set -e
set -x

DST=$CUR_DIR/../../../${PLATFORM}/local
DST_C=$(echo $DST | sed 's/\//\\\//g')
#CPU_COUNT=$(cat /proc/cpuinfo | grep "processor" | awk -F": " '{print $2}' | wc -l)
CPU_COUNT=12

mkdir -p ${CUR_DIR}/../../../build
cd ${CUR_DIR}/../../../build

export PATH=$DST/bin:$PATH
export LD_LIBRARY_PATH=$DST/lib:$LD_LIBRARY_PATH
export PKG_CONFIG_PATH=$DST/lib/pkgconfig:$PKG_CONFIG_PATH
. $CUR_DIR/../build_conf.sh

export SAMPLE_RATE_VERSION=0.2.2
cp -r $CUR_DIR/pkg/libsamplerate/ libsamplerate-${SAMPLE_RATE_VERSION}
cd libsamplerate-${SAMPLE_RATE_VERSION}
mkdir build
cd build
if [ "$PLATFORM" = "arm" ]; then
  cmake \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_SHARED_LIBS=OFF \
  -DCMAKE_PREFIX_PATH=$DST \
  -DCMAKE_INSTALL_PREFIX=$DST \
  -DCMAKE_C_COMPILER=${CROSS_COMPILE}gcc \
  -DCMAKE_CXX_COMPILER=${CROSS_COMPILE}g++ \
  -DCMAKE_LINKER=${CROSS_COMPILE}ld \
  -DCMAKE_SYSTEM_PROCESSOR=aarch64 \
  -DCMAKE_SYSTEM_NAME=Linux \
  -DBUILD_TESTING=OFF \
  -DLIBSAMPLERATE_EXAMPLES=OFF \
  ..
else
  cmake \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_SHARED_LIBS=OFF \
  -DCMAKE_PREFIX_PATH=$DST \
  -DCMAKE_INSTALL_PREFIX=$DST \
  ..
fi
make VERBOSE=1 -j${CPU_COUNT}
make install
cd ..
cd ..
rm -rf libsamplerate-${SAMPLE_RATE_VERSION}

export LAME_VERSION=3.100
tar zxf $CUR_DIR/lame-${LAME_VERSION}.tar.gz
cd lame-${LAME_VERSION}
if [ "$PLATFORM" = "arm" ]; then
  CFLAGS="-I$DST/include/ $PARAM" CPPFLAGS="-I$DST/include/ $PARAM" CXXFLAGS="-I$DST/include/ $PARAM" LDFLAGS="-L$DST/lib $PARAM_LD -lz" ./configure ${CROSS_HOST_PARAM} --prefix=$DST --enable-shared=no --enable-static=yes --with-pic
else
  CFLAGS="-Wno-error -I$DST/include/" CPPFLAGS="-Wno-error -I$DST/include/" LDFLAGS="-L$DST/lib -lz"  ./configure --prefix=$DST --enable-shared=no --with-pic=yes
fi
make -j$CPU_COUNT
make install
cd ..
rm -rf lame-${LAME_VERSION}

export SNDFILE_VERSION=1.2.2
cp -r $CUR_DIR/pkg/libsndfile/ libsndfile-${SNDFILE_VERSION}
cd libsndfile-${SNDFILE_VERSION}
mkdir build
cd build
if [ "$PLATFORM" = "arm" ]; then
  cmake \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_SHARED_LIBS=OFF \
  -DCMAKE_PREFIX_PATH=$DST \
  -DCMAKE_INSTALL_PREFIX=$DST \
  -DCMAKE_C_COMPILER=${CROSS_COMPILE}gcc \
  -DCMAKE_CXX_COMPILER=${CROSS_COMPILE}g++ \
  -DCMAKE_LINKER=${CROSS_COMPILE}ld \
  -DCMAKE_SYSTEM_PROCESSOR=aarch64 \
  -DCMAKE_SYSTEM_NAME=Linux \
  -DBUILD_TESTING=OFF \
  -DBUILD_EXAMPLES=OFF \
  ..
else
  cmake \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_SHARED_LIBS=OFF \
  -DCMAKE_PREFIX_PATH=$DST \
  -DCMAKE_INSTALL_PREFIX=$DST \
  ..
fi
make VERBOSE=1 -j${CPU_COUNT}
make install
cd ..
cd ..
rm -rf libsndfile-${SNDFILE_VERSION}

# vim: set expandtab ts=4 sw=4 sts=4:

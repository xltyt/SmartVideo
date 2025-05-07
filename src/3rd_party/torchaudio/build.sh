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
CPU_COUNT=4

mkdir -p ${CUR_DIR}/../../../build
cd ${CUR_DIR}/../../../build

export PATH=$DST/bin:$PATH
export LD_LIBRARY_PATH=$DST/lib:$LD_LIBRARY_PATH
export PKG_CONFIG_PATH=$DST/lib/pkgconfig:$PKG_CONFIG_PATH
. $CUR_DIR/../build_conf.sh

if [ "$PLATFORM" = "arm" ]; then
  ARM_FLAGS="CMAKE_C_COMPILER=${CROSS_COMPILE}gcc CMAKE_CXX_COMPILER=${CROSS_COMPILE}g++ CMAKE_SYSTEM_PROCESSOR=aarch64"
else
  ARM_FLAGS=""
fi

export TORCH_AUDIO_VERSION=2.5.1
cp -r $CUR_DIR/pkg torchaudio
cd torchaudio
mkdir build_cpu
cd build_cpu
if [ "$PLATFORM" = "arm" ]; then
  cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_C_FLAGS="-fPIC -I$DST/include" \
    -DCMAKE_CXX_FLAGS="-fPIC -I$DST/include" \
    -DCMAKE_EXE_LINKER_FLAGS="-L$DST/lib -static-libstdc++ -static-libgcc -lm -lpthread -ldl -Wl,-rpath=$DST/lib" \
    -DCMAKE_MODULE_LINKER_FLAGS="-L$DST/lib -static-libstdc++ -static-libgcc -lm -lpthread -ldl -Wl,-rpath=$DST/lib" \
    -DCMAKE_SHARED_LINKER_FLAGS="-L$DST/lib -static-libstdc++ -static-libgcc -lm -lpthread -ldl -Wl,-rpath=$DST/lib" \
    -DCMAKE_STATIC_LINKER_FLAGS="" \
    -DBUILD_SHARED_LIBS=ON \
    -DCMAKE_INSTALL_PREFIX=$DST \
    -DTorch_DIR=$DST/torch/cpu/share/cmake/Torch/ \
    -DCMAKE_C_COMPILER=${CROSS_COMPILE}gcc \
    -DCMAKE_CXX_COMPILER=${CROSS_COMPILE}g++ \
    -DCMAKE_LINKER=${CROSS_COMPILE}ld \
    -DCMAKE_SYSTEM_PROCESSOR=aarch64 \
    -DCMAKE_SYSTEM_NAME=Linux \
    -DProtobuf_PROTOC_EXECUTABLE=$DST/../../amd64/local/bin/protoc \
    ..
else
  cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_C_FLAGS="-fPIC -I$DST/include" \
    -DCMAKE_CXX_FLAGS="-fPIC -I$DST/include" \
    -DCMAKE_EXE_LINKER_FLAGS="-L$DST/lib -static-libstdc++ -static-libgcc -lm -lpthread -ldl -Wl,-rpath=$DST/lib" \
    -DCMAKE_MODULE_LINKER_FLAGS="-L$DST/lib -static-libstdc++ -static-libgcc -lm -lpthread -ldl -Wl,-rpath=$DST/lib" \
    -DCMAKE_SHARED_LINKER_FLAGS="-L$DST/lib -static-libstdc++ -static-libgcc -lm -lpthread -ldl -Wl,-rpath=$DST/lib" \
    -DCMAKE_STATIC_LINKER_FLAGS="" \
    -DBUILD_SHARED_LIBS=ON \
    -DCMAKE_INSTALL_PREFIX=$DST \
    -DTorch_DIR=$DST/torch/cpu/share/cmake/Torch/ \
    ..
fi
make -j${CPU_COUNT}
make install
rm -rf $DST/torchaudio/include
cp -r ../src/torchaudio/ $DST/torchaudio/include
find $DST/torchaudio/include ! -name '*.h' -type f | xargs -i rm -f {}
find $DST/torchaudio/include -name 'pybind' | xargs -i rm -rf {}
mkdir $DST/torchaudio/cpu
mv $DST/torchaudio/include $DST/torchaudio/cpu
mv $DST/torchaudio/lib $DST/torchaudio/cpu
cd ..
rm -rf build_cpu
cd ..
rm -rf torchaudio

# vim: set expandtab ts=4 sw=4 sts=4:

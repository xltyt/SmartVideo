#!/bin/bash

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

if [ "$PLATFORM" = "arm" ]; then
  wget https://github.com/BtbN/FFmpeg-Builds/releases/download/latest/ffmpeg-n7.1-latest-linuxarm64-gpl-7.1.tar.xz
  tar Jxf ffmpeg-n7.1-latest-linuxarm64-gpl-7.1.tar.xz
  rm -f ffmpeg-n7.1-latest-linuxarm64-gpl-7.1.tar.xz
  mv -f ffmpeg-n7.1-latest-linuxarm64-gpl-7.1/bin/ffmpeg $DST/bin/
  rm -rf ffmpeg-n7.1-latest-linuxarm64-gpl-7.1
else
  wget https://github.com/BtbN/FFmpeg-Builds/releases/download/latest/ffmpeg-n7.1-latest-linux64-gpl-7.1.tar.xz
  tar Jxf ffmpeg-n7.1-latest-linux64-gpl-7.1.tar.xz
  rm -f ffmpeg-n7.1-latest-linux64-gpl-7.1.tar.xz
  mv -f ffmpeg-n7.1-latest-linux64-gpl-7.1/bin/ffmpeg $DST/bin/
  rm -rf ffmpeg-n7.1-latest-linux64-gpl-7.1
fi

# vim: set expandtab ts=4 sw=4 sts=4:

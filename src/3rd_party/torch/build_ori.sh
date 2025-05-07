#!/bin/bash

set -x
set -e

#
# wget https://download.pytorch.org/libtorch/cpu/libtorch-shared-with-deps-2.5.1%2Bcpu.zip -O /opt/libtorch.zip
# cd /opt
# unzip libtorch.zip
# rm -f libtorch.zip
#
# git clone https://github.com/pytorch/pytorch #--recursive
# cd pytorch
# git checkout v2.5.1
# git submodule sync
# git submodule update --init --recursive
# mkdir build
# cd build
# python ../tools/build_libtorch.py
#

#
# https://github.com/pytorch/pytorch/blob/main/CONTRIBUTING.md
# https://github.com/pytorch/pytorch/issues/13130
# https://blog.csdn.net/chen499093551/article/details/129489295
# https://github.com/pytorch/pytorch/issues/67757
#
mkdir /build
cd /build/
tar zxf /data/pytorch-2.5.1.tar.gz
cd pytorch
mkdir build
cd build
pip install PyYAML typing_extensions --index-url https://pypi.tuna.tsinghua.edu.cn/simple
#export TORCH_CXX_FLAGS="-D_GLIBCXX_USE_CXX11_ABI=0"
#export GLIBCXX_USE_CXX11_ABI=0
sed -i '71aset(GLIBCXX_USE_CXX11_ABI 0)' ../CMakeLists.txt
USE_CUDA=False BUILD_TEST=False python ../tools/build_libtorch.py
rm -rf /opt/libtorch
mkdir -p /opt/libtorch/lib
cp -r ../torch/include /opt/libtorch/include
cp build/lib/*.so /opt/libtorch/lib
mkdir -p /opt/libtorch/share
cp -r ../torch/share/cmake/ /opt/libtorch/share/cmake
cd ../../
#rm -rf pytorch
rm -f /data/pytorch-2.5.1.tar.gz

# vim: set expandtab ts=4 sw=4 sts=4:

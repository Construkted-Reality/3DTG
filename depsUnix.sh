#!/bin/bash
mkdir -p ./lib
# Build GLTF format
cd ./3rdparty/collada2gltf/GLTF
mkdir -p build
cd ./build
cmake ../ -G "Unix Makefiles"
make
rm -f "./../../../../lib/libGLTF.a"
rm -f "./../../../../lib/libdraco.a"
rm -f "./../../../../lib/libdracoenc.a"
rm -f "./../../../../lib/libdracodec.a"
cp "./libGLTF.a" "./../../../../lib/libGLTF.a"
cp "./dependencies/draco/libdraco.a" "./../../../../lib/libdraco.a"
cp "./dependencies/draco/libdracoenc.a" "./../../../../lib/libdracoenc.a"
cp "./dependencies/draco/libdracodec.a" "./../../../../lib/libdracodec.a"
make clean
cd ./..
rm -rf "./build"
cd ./../../..
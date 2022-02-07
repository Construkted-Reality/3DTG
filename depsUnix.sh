# Build GLTF format
cd ./3rdparty/collada2gltf/GLTF
mkdir build
cd ./build
cmake ../ -G "Unix Makefiles"
make
rm "./../../../../lib/libGLTF.a"
rm "./../../../../lib/libdraco.a"
rm "./../../../../lib/libdracoenc.a"
rm "./../../../../lib/libdracodec.a"
cp "./libGLTF.a" "./../../../../lib/libGLTF.a"
cp "./dependencies/draco/libdraco.a" "./../../../../lib/libdraco.a"
cp "./dependencies/draco/libdracoenc.a" "./../../../../lib/libdracoenc.a"
cp "./dependencies/draco/libdracodec.a" "./../../../../lib/libdracodec.a"
make clean
cd ./..
rm -rf "./build"
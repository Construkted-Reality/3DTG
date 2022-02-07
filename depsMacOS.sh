# Build GLTF format
cd /d .\3rdparty\collada2gltf\GLTF
mkdir build
cd /d .\build
cmake ../ -G "MinGW Makefiles"
make
rm ".\..\..\..\..\lib\libGLTF.a"
rm ".\..\..\..\..\lib\libdraco.a"
rm ".\..\..\..\..\lib\libdracoenc.a"
rm ".\..\..\..\..\lib\libdracodec.a"
cp ".\libGLTF.a" ".\..\..\..\..\lib\libGLTF.a"
cp ".\dependencies\draco\libdraco.a" ".\..\..\..\..\lib\libdraco.a"
cp ".\dependencies\draco\libdracoenc.a" ".\..\..\..\..\lib\libdracoenc.a"
cp ".\dependencies\draco\libdracodec.a" ".\..\..\..\..\lib\libdracodec.a"
make clean
cd /d .\..
rm ".\build"
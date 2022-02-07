:: Build GLTF format
cd /d .\3rdparty\collada2gltf\GLTF
mkdir build
cd /d .\build
cmake ../ -G "MinGW Makefiles"
make
del ".\..\..\..\..\lib\libGLTF.a"
del ".\..\..\..\..\lib\libdraco.a"
del ".\..\..\..\..\lib\libdracoenc.a"
del ".\..\..\..\..\lib\libdracodec.a"
copy ".\libGLTF.a" ".\..\..\..\..\lib\libGLTF.a"
copy ".\dependencies\draco\libdraco.a" ".\..\..\..\..\lib\libdraco.a"
copy ".\dependencies\draco\libdracoenc.a" ".\..\..\..\..\lib\libdracoenc.a"
copy ".\dependencies\draco\libdracodec.a" ".\..\..\..\..\lib\libdracodec.a"
make clean
cd /d .\..
del ".\build"
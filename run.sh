rm -r CMakeFiles
rm -r build
rm cmake_install.cmake
rm CMakeCache.txt
rm CPackConfig.cmake
rm CPackSourceConfig.cmake
rm spiel
cmake . -DCMAKE_POLICY_VERSION_MINIMUM=3.5
make
chmod 760 spiel
./spiel
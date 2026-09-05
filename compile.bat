mkdir build
cd build
cmake -G "Visual Studio 18 2026" .. && cmake --build . && games.exe
cd ..
echo off
set BOX3D_LIB=D:\GitHub\box3d\src\build\box3d.a
set RAYLIB_LIB=D:\GitHub\raylib\src\build\libraylib.a
set LUA_LIB=D:\GitHub\lua\lua.a

set BOX3D_INC=D:\GitHub\box3d\include
set RAYLIB_INC=D:\GitHub\raylib\src\include
set LUA_INC=D:\GitHub\lua-5.5.0

:: emcc -o game.html ../main.cpp ../Game.cpp ../Playground.cpp ../Animation.cpp ../Camera.cpp ../EventHandler.cpp ../Prefabs.cpp ../Object.cpp ../LuaBind.cpp ../WindowHandler.cpp -Os -Wall -I%BOX3D_INC% -I%RAYLIB_INC% -I%LUA_INC% -l%BOX3D_LIB% -l%RAYLIB_LIB% -l%LUA_LIB% -s USE_GLFW=3 -s ASYNCIFY  -DPLATFORM_WEB

emcc -o game.html ../main.c ../Game.c ../Playground.c ../Animation.c ../Camera.c ../EventHandler.c ../Prefabs.c ../Object.c ../LuaBind.c ../WindowHandler.c %BOX3D_LIB% %RAYLIB_LIB% %LUA_LIB% -Os -O3 -Wall -I%BOX3D_INC% -I%RAYLIB_INC% -I%LUA_INC%  -s USE_GLFW=3 -s ASYNCIFY -DPLATFORM_WEB -DSET_FPS=38 --shell-file ../shell.html --embed-file ./res@res ^
-s INITIAL_MEMORY=335544320

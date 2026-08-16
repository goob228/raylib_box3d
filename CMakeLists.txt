cmake_minimum_required(VERSION 3.11)

project(games)

file(GLOB_RECURSE BOX3D_SOURCE CONFIGURE_DEPENDS "thirdparty/src/box3d/*.c")

file(GLOB_RECURSE LUA_SOURCE CONFIGURE_DEPENDS "thirdparty/src/lua/*.c")

set(RAYLIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/src/raylib")

set(RAYLIB_SOURCE
    ${RAYLIB_DIR}/rcore.c
    ${RAYLIB_DIR}/rglfw.c
    ${RAYLIB_DIR}/rmodels.c
    ${RAYLIB_DIR}/rshapes.c
    ${RAYLIB_DIR}/rtextures.c
    ${RAYLIB_DIR}/rtext.c
    ${RAYLIB_DIR}/raudio.c
)

file(GLOB_RECURSE TEMPLATE_SOURCE CONFIGURE_DEPENDS "template/*.c")

add_executable( ${PROJECT_NAME} WIN32 ${BOX3D_SOURCE} ${LUA_SOURCE} ${RAYLIB_SOURCE} ${TEMPLATE_SOURCE} )

target_compile_definitions(${PROJECT_NAME} PRIVATE PLATFORM_DESKTOP)

target_include_directories(${PROJECT_NAME} PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/include 
    ${RAYLIB_DIR}
    ${RAYLIB_DIR}/external/glfw/include)


target_link_libraries(${PROJECT_NAME} PRIVATE
    opengl32
    gdi32
    winmm
    user32
    shell32
)
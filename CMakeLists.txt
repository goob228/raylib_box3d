cmake_minimum_required(VERSION 3.11)

project(games)

set(CMAKE_C_STANDARD 17)

file(GLOB_RECURSE BOX3D_SOURCE CONFIGURE_DEPENDS "thirdparty/src/box3d/*.c")

set(RAYLIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/src/raylib")

set(RAYLIB_SOURCE
    ${RAYLIB_DIR}/rcore.c
    ${RAYLIB_DIR}/rmodels.c
    ${RAYLIB_DIR}/rshapes.c
    ${RAYLIB_DIR}/rtextures.c
    ${RAYLIB_DIR}/rtext.c
    ${RAYLIB_DIR}/raudio.c
)

file(GLOB_RECURSE TEMPLATE_SOURCE CONFIGURE_DEPENDS "template/*.c")

add_executable( ${PROJECT_NAME} ${BOX3D_SOURCE} ${RAYLIB_SOURCE} ${TEMPLATE_SOURCE} )

target_compile_definitions(${PROJECT_NAME} PRIVATE 
    PLATFORM_DESKTOP_SDL
    USING_SDL3_PROJECT
)

target_include_directories(${PROJECT_NAME} PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/include 
    ${RAYLIB_DIR}
    )

target_link_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/lib
)


target_link_libraries(${PROJECT_NAME} PRIVATE
    SDL3
    opengl32
    gdi32
    winmm
    user32
    shell32
    setupapi
    imm32
    version
)


add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/lib/SDL3.dll"
    $<TARGET_FILE_DIR:${PROJECT_NAME}>
)
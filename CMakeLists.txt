cmake_minimum_required(VERSION 3.4.1)
set(CMAKE_C_COMPILER_WORKS 1)
set(CMAKE_CXX_COMPILER_WORKS 1)

project(RailRoad C)

set(BUILD_PRC build-prc)
set(PILRC pilrc)

set(CMAKE_C_COMPILER m68k-palmos-gcc)
set(CMAKE_C_FLAGS "-O3")
set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${SOURCE_DIR})

add_executable(RailRoad
        main.c
        tables.h
        game.c
        game.h
        wireframe.c
        wireframe.h
        fixed_point.c
        fixed_point.h)

add_custom_target(RailRoad_prc ALL ${BUILD_PRC} RailRoad.prc "RailRoad" Foss RailRoad)

add_dependencies(RailRoad_prc RailRoad)

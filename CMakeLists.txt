cmake_minimum_required(VERSION 3.20)
project(slime_mold_simulator LANGUAGES CXX)

# === UI Backend Selection ===
set(UI_BACKEND "sdl" CACHE STRING "UI backend: sdl or qml")
set_property(CACHE UI_BACKEND PROPERTY STRINGS "sdl" "qml")

option(USE_AVX2 "Enable AVX2 support" OFF)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set default build type to Release
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to 'RelWithDebInfo' as none was specified.")
    set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Choose the type of build." FORCE)
endif()

add_subdirectory(source/libs/common)
if(UI_BACKEND STREQUAL "sdl")
    find_package(SDL3 REQUIRED CONFIG)
    add_subdirectory(source/libs/ui_imgui)
    add_subdirectory(source/apps/sdl)
    add_subdirectory(source/apps/test_palettes)
elseif(UI_BACKEND STREQUAL "qml")
    find_package(Qt6 REQUIRED COMPONENTS Quick)
    message(FATAL_ERROR "QtQuick/QML build not yet ready.")
    #add_subdirectory(source/apps/qt)
else()
    message(FATAL_ERROR "UI_BACKEND must be 'sdl' or 'qml'")
endif()


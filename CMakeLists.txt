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

add_subdirectory(libs/common)
if(UI_BACKEND STREQUAL "sdl")
    find_package(SDL3 REQUIRED CONFIG)
    add_subdirectory(libs/ui_imgui)
    add_subdirectory(apps/sdl)
elseif(UI_BACKEND STREQUAL "qml")
    find_package(Qt6 REQUIRED COMPONENTS Quick)
    message(FATAL_ERROR "QtQuick/QML build not yet ready.")
    #add_subdirectory(apps/qt)
else()
    message(FATAL_ERROR "UI_BACKEND must be 'sdl' or 'qml'")
endif()


#if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
#    message(STATUS "Enabling AVX2 for Linux build with GCC/Clang")
#    target_compile_options(slime_mold PRIVATE -mavx2 -O3 -ffast-math)
#endif()

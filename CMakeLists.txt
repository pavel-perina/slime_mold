cmake_minimum_required(VERSION 3.20)
project(slime_mold_simulator)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set default build type to Release
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to 'RelWithDebInfo' as none was specified.")
    set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Choose the type of build." FORCE)
endif()

find_package(SDL3)

add_subdirectory(libs/common)
add_subdirectory(libs/ui_imgui)
add_subdirectory(apps/sdl)


#if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
#    message(STATUS "Enabling AVX2 for Linux build with GCC/Clang")
#    target_compile_options(slime_mold PRIVATE -mavx2 -O3 -ffast-math)
#endif()

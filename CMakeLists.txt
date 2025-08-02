cmake_minimum_required(VERSION 3.20)
project(slime_mold_simulator)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set default build type to Release
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

find_package(SDL3 REQUIRED)

# ImGui setup
set(IMGUI_DIR external/imgui)


set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdl3.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdlrenderer3.cpp
)

add_executable(slime_mold src/slimemold.cpp)
# Add ImGui sources to your existing target
target_sources(slime_mold PRIVATE ${IMGUI_SOURCES})

# Add ImGui include directories
target_include_directories(slime_mold PRIVATE
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

target_link_libraries(slime_mold PRIVATE SDL3::SDL3)

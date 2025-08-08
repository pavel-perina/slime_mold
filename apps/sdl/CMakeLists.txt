cmake_minimum_required(VERSION 3.20)
find_package(SDL3 REQUIRED)

# Common source files
set(SLIME_MOLD_SOURCES main.cpp)

# Common libraries
set(SLIME_MOLD_LIBS common ui_imgui)

# Common include dirs
set(SLIME_MOLD_INCLUDE_DIRS
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

# Emscripten-specific flags
set(EMSCRIPTEN_COMMON_FLAGS
    -sUSE_SDL=3
    -sWASM=1
    -sALLOW_MEMORY_GROWTH=1
)

if(NOT EMSCRIPTEN)
    # Native build
    add_executable(slime_mold ${SLIME_MOLD_SOURCES})
    target_link_libraries(slime_mold PRIVATE ${SLIME_MOLD_LIBS} SDL3::SDL3)
    target_include_directories(slime_mold PRIVATE ${SLIME_MOLD_INCLUDE_DIRS})
else()
    # Web builds

    # HTML target (with shell)
    add_executable(slime_mold_html ${SLIME_MOLD_SOURCES})
    set_target_properties(slime_mold_html PROPERTIES SUFFIX ".html")
    target_link_libraries(slime_mold_html PRIVATE ${SLIME_MOLD_LIBS})
    target_include_directories(slime_mold_html PRIVATE ${SLIME_MOLD_INCLUDE_DIRS})
    target_link_options(slime_mold_html PRIVATE
        ${EMSCRIPTEN_COMMON_FLAGS}
        --shell-file ${CMAKE_CURRENT_SOURCE_DIR}/shell-minimal.html
    )

    # JS-only target (for embedding)
    add_executable(slime_mold_js ${SLIME_MOLD_SOURCES})
    set_target_properties(slime_mold_js PROPERTIES SUFFIX ".js")
    target_link_libraries(slime_mold_js PRIVATE ${SLIME_MOLD_LIBS})
    target_include_directories(slime_mold_js PRIVATE ${SLIME_MOLD_INCLUDE_DIRS})
    target_link_options(slime_mold_js PRIVATE ${EMSCRIPTEN_COMMON_FLAGS})

endif()

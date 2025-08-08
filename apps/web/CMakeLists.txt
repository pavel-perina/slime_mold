if(NOT EMSCRIPTEN)
    message(FATAL_ERROR "Web target requires Emscripten")
endif()


add_executable(slime_mold_html main.cpp)
set_target_properties(slime_mold_html PROPERTIES SUFFIX ".html")
target_link_libraries(slime_mold_html PRIVATE common ui_imgui)
target_include_directories(slime_mold_html PRIVATE
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)
target_link_options(slime_mold_html PRIVATE
    -sUSE_SDL=3
    -sWASM=1
    -sALLOW_MEMORY_GROWTH=1
    --shell-file ${CMAKE_CURRENT_SOURCE_DIR}/shell-minimal.html
)

add_executable(slime_mold_js main.cpp)
set_target_properties(slime_mold_js PROPERTIES SUFFIX ".js")
target_link_libraries(slime_mold_js PRIVATE common ui_imgui)
target_include_directories(slime_mold_js PRIVATE
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)
target_link_options(slime_mold_js PRIVATE
    -sUSE_SDL=3
    -sWASM=1
    -sALLOW_MEMORY_GROWTH=1
)
if(NOT EMSCRIPTEN)
    message(FATAL_ERROR "Web target requires Emscripten")
endif()

add_executable(slime_mold_web main.cpp)

target_link_libraries(slime_mold_web PRIVATE
    common
    ui_imgui
)

# Emscripten-specific settings
set_target_properties(slime_mold_web PROPERTIES
    SUFFIX ".html"
)

target_link_options(slime_mold_web PRIVATE
    -sUSE_SDL=3    
    -sWASM=1
    -sALLOW_MEMORY_GROWTH=1
)

if(NOT EMSCRIPTEN)
    add_executable(test_palettes main.cpp)
    target_link_libraries(test_palettes PRIVATE common)
endif()


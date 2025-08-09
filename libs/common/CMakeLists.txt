set(SOURCES
    source/colors.cpp
    source/presets.cpp
    source/slime_mold_simulation.cpp
    source/slime_mold_viewmodel.cpp)

set(PUBLIC_HEADERS
    include/common/colors.h
    include/common/presets.h
    include/common/slime_mold_simulation.h
    include/common/slime_mold_viewmodel.h)

add_library(common STATIC ${SOURCES} ${PUBLIC_HEADERS})

target_include_directories(common PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
if(USE_AVX2)
    target_compile_definitions(common PRIVATE USE_AVX2)
endif()

# IDE support
source_group("Public Headers" FILES ${PUBLIC_HEADERS})
source_group("Source Files" FILES ${SOURCES})
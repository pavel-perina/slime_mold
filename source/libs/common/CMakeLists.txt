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
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
	target_compile_options(common PRIVATE -mavx2)
        message(STATUS "AVX2 support enabled")
    elseif(MSVC)
	add_compile_options(common PRIVATE /arch:AVX2)
        message(STATUS "AVX2 support enabled (MSVC)")
    else()
        message(WARNING "AVX2 requested but compiler not recognized")
   endif()
endif()

# Debugging
get_target_property(COMMON_COMPILE_OPTIONS common COMPILE_OPTIONS)
message(STATUS "Common library compile options: ${COMMON_COMPILE_OPTIONS}")

# IDE support
source_group("Public Headers" FILES ${PUBLIC_HEADERS})
source_group("Source Files" FILES ${SOURCES})


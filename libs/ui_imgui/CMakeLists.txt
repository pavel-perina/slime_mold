# ImGui setup
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/external/imgui)

add_library(ui_imgui STATIC
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdl3.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdlrenderer3.cpp
 "src/ui.cpp")

target_include_directories(ui_imgui
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${IMGUI_DIR}
        ${IMGUI_DIR}/backends
)

target_link_libraries(ui_imgui 
    PUBLIC 
        common
    PRIVATE
        SDL3::SDL3
)

target_compile_features(ui_imgui PUBLIC cxx_std_23)
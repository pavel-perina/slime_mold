find_package(SDL3 REQUIRED)

add_executable(slime_mold main.cpp)

target_link_libraries(slime_mold PRIVATE
    common
    ui_imgui
    SDL3::SDL3
)

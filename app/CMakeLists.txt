file(GLOB_RECURSE APP_SOURCES "src/*.cpp" "src/*.h")

add_executable(RD++ ${APP_SOURCES}
    ../dependencies/imgui/imgui.cpp
    ../dependencies/imgui/imgui_draw.cpp
    ../dependencies/imgui/imgui_widgets.cpp
    ../dependencies/imgui/imgui_tables.cpp
    ../dependencies/imgui/imgui_demo.cpp
    ../dependencies/imgui/backends/imgui_impl_sdl3.cpp
    ../dependencies/imgui/backends/imgui_impl_sdlrenderer3.cpp
    ../dependencies/imgui/misc/cpp/imgui_stdlib.cpp
)

target_compile_definitions(RD++ PRIVATE $<$<CONFIG:Debug>:DEBUG>)

add_subdirectory(../dependencies/SDL3-3.2.24 EXCLUDE_FROM_ALL ${CMAKE_BINARY_DIR}/SDL3)

target_include_directories(RD++ PRIVATE
    src
    ../dependencies/SDL3-3.2.24/include/
    ../dependencies/enet-1.3.18/include/
    ../dependencies/imgui/
    ../dependencies/imgui/backends/
    ../dependencies/imgui/misc/cpp/
)

target_link_libraries(RD++ PRIVATE
    rdpp_common
    SDL3::SDL3
    enet
)


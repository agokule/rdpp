cmake_minimum_required(VERSION 3.10)

project(HelloWorld)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# set the output directory for built objects.
# This makes sure that the dynamic library goes into the build directory automatically.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

find_package(FFMPEG REQUIRED)

add_subdirectory(./dependencies/screen_capture_lite-17.1.2539/src_cpp)
add_subdirectory(./dependencies/SDL3-3.2.24/ EXCLUDE_FROM_ALL)

add_executable(HelloWorld
    helloworld.cpp
    lodepng.cpp
    dependencies/imgui/imgui.cpp
    dependencies/imgui/imgui_draw.cpp
    dependencies/imgui/imgui_tables.cpp
    dependencies/imgui/imgui_widgets.cpp
    dependencies/imgui/imgui_demo.cpp
    dependencies/imgui/backends/imgui_impl_sdl3.cpp
    dependencies/imgui/backends/imgui_impl_sdlrenderer3.cpp
    dependencies/imgui/misc/cpp/imgui_stdlib.cpp
)

target_include_directories(HelloWorld PRIVATE ${FFMPEG_INCLUDE_DIRS}
    ./dependencies/screen_capture_lite-17.1.2539/include
    ./
    ./dependencies/imgui
    ./dependencies/imgui/backends
    ./dependencies/imgui/misc/cpp
)
target_link_directories(HelloWorld PRIVATE ${FFMPEG_LIBRARY_DIRS} ${CMAKE_BINARY_DIR}/dependencies/screen_capture_lite-17.1.2539/src_cpp ${CMAKE_BINARY_DIR})
target_link_libraries(HelloWorld PRIVATE ${FFMPEG_LIBRARIES} screen_capture_lite_shared SDL3::SDL3)



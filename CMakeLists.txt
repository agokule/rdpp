cmake_minimum_required(VERSION 3.10)

project(HelloWorld)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(FFMPEG REQUIRED)

add_subdirectory(./dependencies/screen_capture_lite-17.1.2539/src_cpp)

add_executable(HelloWorld helloworld.cpp lodepng.cpp)

target_include_directories(HelloWorld PRIVATE ${FFMPEG_INCLUDE_DIRS} ./dependencies/screen_capture_lite-17.1.2539/include ./)
target_link_directories(HelloWorld PRIVATE ${FFMPEG_LIBRARY_DIRS} ${CMAKE_BINARY_DIR}/dependencies/screen_capture_lite-17.1.2539/src_cpp ${CMAKE_BINARY_DIR})
target_link_libraries(HelloWorld PRIVATE ${FFMPEG_LIBRARIES} screen_capture_lite_shared)

# Copy the screen_capture_lite DLL to the executable directory
add_custom_command(TARGET HelloWorld POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:screen_capture_lite_shared>"
        "$<TARGET_FILE_DIR:HelloWorld>"
    COMMENT "Copying screen_capture_lite DLL to output directory"
)


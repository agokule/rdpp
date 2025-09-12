cmake_minimum_required(VERSION 3.10)

project(HelloWorld)

find_package(FFMPEG REQUIRED)

add_executable(HelloWorld helloworld.cpp)

target_include_directories(HelloWorld PRIVATE ${FFMPEG_INCLUDE_DIRS})
target_link_directories(HelloWorld PRIVATE ${FFMPEG_LIBRARY_DIRS})
target_link_libraries(HelloWorld PRIVATE ${FFMPEG_LIBRARIES})


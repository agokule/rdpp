file(GLOB_RECURSE LIBSERVER_SOURCES "src/*.cpp" "include/*.hpp")

add_library(rdpp_server STATIC ${LIBSERVER_SOURCES})

target_compile_definitions(rdpp_server PRIVATE $<$<CONFIG:Debug>:DEBUG>)

set(BUILD_SHARED_LIBS FALSE)
add_subdirectory(../../dependencies/screen_capture_lite-17.1.2539/src_cpp/ ${CMAKE_BINARY_DIR}/dependencies/screen_capture_lite-17.1.2539/src_cpp)

target_include_directories(rdpp_server
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${FFMPEG_INCLUDE_DIRS}
        ../../dependencies/enet-1.3.18/include/
        ../../dependencies/screen_capture_lite-17.1.2539/include/
)

target_link_directories(rdpp_server PRIVATE ${FFMPEG_LIBRARY_DIRS}
    ${CMAKE_BINARY_DIR}
)

target_link_libraries(rdpp_server
    PUBLIC
        rdpp_common
        screen_capture_lite_static
    PRIVATE
        ${FFMPEG_LIBRARIES}
        enet
)


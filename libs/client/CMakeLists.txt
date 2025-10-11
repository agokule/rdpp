file(GLOB_RECURSE LIBCLIENT_SOURCES "src/*.cpp" "include/*.h")

add_library(rdpp_client STATIC ${LIBCLIENT_SOURCES})

target_compile_definitions(rdpp_client PRIVATE $<$<CONFIG:Debug>:DEBUG>)

target_include_directories(rdpp_client
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${FFMPEG_INCLUDE_DIRS}
)

target_link_directories(rdpp_client PRIVATE ${FFMPEG_LIBRARY_DIRS} ${CMAKE_BINARY_DIR})

target_link_libraries(rdpp_client
    PUBLIC
        rdpp_common
    PRIVATE
        ${FFMPEG_LIBRARIES}
)


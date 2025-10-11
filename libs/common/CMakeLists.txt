file(GLOB_RECURSE COMMON_SOURCES "src/*.cpp" "include/*.h")

add_library(rdpp_common STATIC ${COMMON_SOURCES})

target_compile_definitions(rdpp_common PRIVATE $<$<CONFIG:Debug>:DEBUG>)

target_include_directories(rdpp_common
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)


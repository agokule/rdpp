cmake_minimum_required(VERSION 3.10)

project(RD++ LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>/bin")

# Platform-specific settings
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(VCPKG_TARGET_TRIPLET "x64-linux")
    find_package(Threads REQUIRED)
    set(THREADS_PREFER_PTHREAD_FLAG TRUE)
    set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
endif()

if(WIN32 AND MINGW)
    # otherwise vcpkg might try to compile ffmpeg with msvc
    # while cmake does everything else with mingw
    set(VCPKG_TARGET_TRIPLET "x64-mingw-dynamic")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libstdc++ -static-libgcc")
endif()

set(CMAKE_TOOLCHAIN_FILE ./dependencies/vcpkg/scripts/buildsystems/vcpkg.cmake)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Add common third-party dependencies everything needs
add_subdirectory(./dependencies/enet-1.3.18)
find_package(FFMPEG REQUIRED)

# Add project libraries
add_subdirectory(libs/common)
add_subdirectory(libs/server)
add_subdirectory(libs/client)

# Add main application
add_subdirectory(app)

cmake_minimum_required(VERSION 3.15)

project(
    extract
    LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include(cmake/CPM.cmake)

CPMAddPackage(
    NAME zlib-ng
    GITHUB_REPOSITORY zlib-ng/zlib-ng
    GIT_TAG 2.3.2
    OPTIONS
        "ZLIB_COMPAT OFF"
        "ZLIB_ENABLE_TESTS OFF"
        "ZLIBNG_ENABLE_TESTS OFF"
)

CPMAddPackage(
    NAME cryptopp-cmake
    GITHUB_REPOSITORY abdes/cryptopp-cmake
    GIT_TAG CRYPTOPP_8_9_0
    OPTIONS
        "CRYPTOPP_BUILD_TESTING OFF"
)

add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE src/main.cpp src/steam2.cpp src/win32console.cpp)
target_include_directories(${PROJECT_NAME} PRIVATE include/ include/thirdparty)
target_link_libraries(${PROJECT_NAME} cryptopp::cryptopp zlib-ng::zlibstatic)
cmake_minimum_required(VERSION 3.25)
project(notify_cli VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra -Wall -Wpedantic")

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    find_package(Boost REQUIRED)

    find_program(NOTIFY_SEND_EXECUTABLE NAMES notify-send)
    if (NOT NOTIFY_SEND_EXECUTABLE)
        message(FATAL_ERROR "Missing notify-send. Please install libnotify-bin (run ./setup_linux.sh).")
    endif()

    find_path(ZSTD_INCLUDE_DIR NAMES zstd.h)
    find_library(ZSTD_LIBRARY NAMES zstd)
    if (NOT ZSTD_INCLUDE_DIR OR NOT ZSTD_LIBRARY)
        message(FATAL_ERROR "Missing Zstandard development files. Please install libzstd-dev (run ./setup_linux.sh).")
    endif()

    find_path(NGHTTP2_INCLUDE_DIR NAMES nghttp2/nghttp2.h)
    find_library(NGHTTP2_LIBRARY NAMES nghttp2)
    if (NOT NGHTTP2_INCLUDE_DIR OR NOT NGHTTP2_LIBRARY)
        message(FATAL_ERROR "Missing nghttp2 development files. Please install libnghttp2-dev (run ./setup_linux.sh).")
    endif()
endif()

set(EXEC_NAME "clinotify")
set(FETCHCONTENT_BASE_DIR ${CMAKE_SOURCE_DIR}/ext)

set(GENERATED_INCLUDE_DIR "${CMAKE_BINARY_DIR}/generated")
file(MAKE_DIRECTORY "${GENERATED_INCLUDE_DIR}")
configure_file(
    "${CMAKE_SOURCE_DIR}/inc/configure.h.in"
    "${GENERATED_INCLUDE_DIR}/configure.h"
    @ONLY
)

include(FetchContent)

FetchContent_Declare(
    cli11_proj
    QUIET
    GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
    GIT_TAG v2.3.2
)
FetchContent_MakeAvailable(cli11_proj)

FetchContent_Declare(
  libcurl
  GIT_REPOSITORY https://github.com/curl/curl.git
  GIT_TAG        curl-8_5_0
)
FetchContent_MakeAvailable(libcurl)

file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(${EXEC_NAME} ${SOURCES})
target_include_directories(${EXEC_NAME} PRIVATE inc "${GENERATED_INCLUDE_DIR}" ${Boost_INCLUDE_DIRS})
target_link_libraries(${EXEC_NAME} PRIVATE CLI11::CLI11)
target_link_libraries(${EXEC_NAME} PRIVATE CURL::libcurl)

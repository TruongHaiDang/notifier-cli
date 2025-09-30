cmake_minimum_required(VERSION 3.25)
project(notify_cli VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra -Wall -Wpedantic")

set(EXEC_NAME "clinotify")
set(FETCHCONTENT_BASE_DIR ${CMAKE_SOURCE_DIR}/ext)

include(FetchContent)

FetchContent_Declare(
    cli11_proj
    QUIET
    GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
    GIT_TAG v2.3.2
)

FetchContent_MakeAvailable(cli11_proj)

include_directories(inc)
file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(${EXEC_NAME} ${SOURCES})
target_link_libraries(${EXEC_NAME} PRIVATE CLI11::CLI11)

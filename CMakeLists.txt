cmake_minimum_required(VERSION 3.25)
project(notify_cli VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra -Wall -Wpedantic")

set(EXEC_NAME "clinotify")
set(FETCHCONTENT_BASE_DIR ${CMAKE_SOURCE_DIR}/ext)

set(GENERATED_INCLUDE_DIR "${CMAKE_BINARY_DIR}/generated")
file(MAKE_DIRECTORY "${GENERATED_INCLUDE_DIR}")
configure_file(
    "${CMAKE_SOURCE_DIR}/inc/configure.h.in"
    "${GENERATED_INCLUDE_DIR}/configure.h"
    @ONLY
)

if(UNIX AND NOT APPLE)
  find_program(NOTIFY_SEND notify-send)
  option(TRY_AUTO_INSTALL_NOTIFY_SEND "Auto-install notify-send if missing" ON)

  if(NOT NOTIFY_SEND)
    if(TRY_AUTO_INSTALL_NOTIFY_SEND)
      message(STATUS "notify-send not found. Attempting install...")
      find_program(APT apt-get)
      find_program(DNF dnf)
      find_program(PACMAN pacman)

      if(APT)
        execute_process(COMMAND sudo apt-get update)
        execute_process(COMMAND sudo apt-get install -y libnotify-bin)
      elseif(DNF)
        execute_process(COMMAND sudo dnf -y install libnotify)
      elseif(PACMAN)
        execute_process(COMMAND sudo pacman -Sy --noconfirm libnotify)
      else()
        message(FATAL_ERROR "No supported package manager. Install notify-send manually.")
      endif()

      find_program(NOTIFY_SEND notify-send REQUIRED)
    else()
      message(FATAL_ERROR "notify-send missing. Install: libnotify-bin (Debian/Ubuntu) | libnotify (Fedora/Arch).")
    endif()
  else()
    message(STATUS "notify-send found: ${NOTIFY_SEND}")
  endif()
endif()

include(FetchContent)

FetchContent_Declare(
    cli11_proj
    QUIET
    GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
    GIT_TAG v2.3.2
)

FetchContent_MakeAvailable(cli11_proj)

file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(${EXEC_NAME} ${SOURCES})
target_include_directories(${EXEC_NAME} PRIVATE inc "${GENERATED_INCLUDE_DIR}")
target_link_libraries(${EXEC_NAME} PRIVATE CLI11::CLI11)

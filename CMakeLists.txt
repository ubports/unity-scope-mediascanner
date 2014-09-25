project(unity-scope-mediascanner C CXX)
cmake_minimum_required(VERSION 2.8.9)

find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)
find_package(Boost COMPONENTS regex REQUIRED)
pkg_check_modules(GIO_DEPS REQUIRED gio-2.0 gio-unix-2.0)

pkg_check_modules(UNITY REQUIRED
  "libunity-scopes >= 0.6.0"
  net-cpp>=1.1.0
  jsoncpp
  mediascanner-2.0
)

find_program(XGETTEXT_BIN xgettext)
find_program(MSGFMT_BIN msgfmt)
find_program(INTLTOOL_EXTRACT_BIN intltool-extract)
find_program(INTLTOOL_MERGE_BIN intltool-merge)

if(NOT MSGFMT_BIN)
  message(FATAL_ERROR "Gettext binaries not found.")
endif()
if(NOT INTLTOOL_MERGE_BIN)
  message(FATAL_ERROR "Intltool binaries not found.")
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wmissing-prototypes -pedantic -std=c99")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wmissing-declarations -pedantic -std=c++11")

set(DATA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/data")

set(SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(SOURCE_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/src")

include(GNUInstallDirs)

set(GETTEXT_PACKAGE unity-scope-mediascanner)

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/config.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/config.h"
)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory("po")
add_subdirectory("src")
add_subdirectory("data")

enable_testing()
add_subdirectory("tests")
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} -V --output-on-failure)

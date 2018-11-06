cmake_minimum_required(VERSION 2.6)

project(gpio2mqttd C)

set (VERSION_MAJOR 0)
set (VERSION_MINOR 0)
set (VERSION_PATCH 1)

execute_process(COMMAND git describe --tags --dirty
		OUTPUT_VARIABLE GIT_DESCRIBE
		OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
execute_process(COMMAND git describe --abbrev=0
                OUTPUT_VARIABLE GIT_LAST_TAG
                OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)

string(REGEX REPLACE "^${GIT_LAST_TAG}-" "" GIT_DESCRIBE_NOTAG "${GIT_DESCRIBE}")

if (NOT "${GIT_DESCRIBE}" STREQUAL "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
    if ("${GIT_LAST_TAG}" VERSION_LESS
        "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
        string(REGEX REPLACE "^${GIT_LAST_TAG}-"
                "" VERSION_TWEAK "0-pre-${GIT_DESCRIBE_NOTAG}")
    else ()
	string(REGEX REPLACE
		"^${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}-"
		"" VERSION_TWEAK "${GIT_DESCRIBE}")
    endif ()
endif ()

configure_file (
  "${PROJECT_SOURCE_DIR}/src/config.h.in"
  "${PROJECT_BINARY_DIR}/src/config.h"
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Wall -Werror -Wextra")

add_definitions(-D_GNU_SOURCE)

find_package(PkgConfig REQUIRED)

option (WITH_TLS "Enable TLS support" ON)
if (${WITH_TLS} STREQUAL ON)
    pkg_search_module(LIBSSL QUIET "libssl")
    if (LIBSSL_FOUND)
        add_definitions("-DWITH_TLS")
        list(APPEND ext_libs ssl)
    else ()
        message(FATAL_ERROR "TLS enabled but LibreSSL or OpenSSL not found!")
    endif ()
endif ()

list(APPEND ext_libs gpiod)
list(APPEND ext_libs mosquitto)

add_executable(gpio2mqttd src/main.c src/gpio.c src/mosquitto.c)
target_link_libraries(gpio2mqttd ${ext_libs})

install(TARGETS gpio2mqttd RUNTIME DESTINATION bin)

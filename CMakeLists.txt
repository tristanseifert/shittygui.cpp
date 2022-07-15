####################################################################################################
# ShittyGUI main CMake file
#
# This file defines the GUI library target, as well as auxiliary targets (for example code,
# supporting libraries, etc.) which are included only if this is the top-level CMake file.
####################################################################################################
cmake_minimum_required(VERSION 3.18 FATAL_ERROR)
project(shittygui VERSION 0.1 LANGUAGES C CXX)

include(ExternalProject)
include(FetchContent)

#######################################
# Find dependencies
find_package(PkgConfig REQUIRED)

# Required
pkg_search_module(PKG_FREETYPE REQUIRED freetype2)
link_directories(${PKG_FREETYPE_LIBRARY_DIRS})
include_directories(${PKG_FREETYPE_INCLUDE_DIRS})

pkg_search_module(PKG_CAIRO REQUIRED cairo)
link_directories(${PKG_CAIRO_LIBRARY_DIRS})
include_directories(${PKG_CAIRO_INCLUDE_DIRS})

pkg_search_module(PKG_HARFBUZZ REQUIRED harfbuzz)
link_directories(${PKG_HARFBUZZ_LIBRARY_DIRS})
include_directories(${PKG_HARFBUZZ_INCLUDE_DIRS})

pkg_search_module(PKG_PANGO REQUIRED pango)
link_directories(${PKG_PANGO_LIBRARY_DIRS})
include_directories(${PKG_PANGO_INCLUDE_DIRS})
pkg_search_module(PKG_PANGOCAIRO REQUIRED pangocairo)
link_directories(${PKG_PANGOCAIRO_LIBRARY_DIRS})
include_directories(${PKG_PANGOCAIRO_INCLUDE_DIRS})

# glib is a dependency of harfbuzz and pango
pkg_search_module(PKG_GLIB2 REQUIRED glib-2.0)
link_directories(${PKG_GLIB2_LIBRARY_DIRS})
include_directories(${PKG_GLIB2_INCLUDE_DIRS})
pkg_search_module(PKG_GOBJECT2 REQUIRED gobject-2.0)
link_directories(${PKG_GOBJECT2_LIBRARY_DIRS})
include_directories(${PKG_GOBJECT2_INCLUDE_DIRS})

# Optional
pkg_search_module(PKG_PNG libpng)
if(PKG_PNG_FOUND)
    link_directories(${PKG_PNG_LIBRARY_DIRS})
    include_directories(${PKG_PNG_INCLUDE_DIRS})
endif()

pkg_search_module(PKG_FONTCONFIG fontconfig)
if(PKG_FONTCONFIG_FOUND)
    link_directories(${PKG_FONTCONFIG_LIBRARY_DIRS})
    include_directories(${PKG_FONTCONFIG_INCLUDE_DIRS})
endif()

#######################################
# Set warning levels and language version
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wmissing-declarations -Wformat=2 -fdiagnostics-color=always -Wundef
    -Wcast-qual -Wwrite-strings -Wno-format-nonliteral)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    add_compile_options(-Werror -Wimplicit-fallthrough -Wno-deprecated-copy -Wno-address-of-packed-member
        -Wno-expansion-to-defined -Wno-undef -Wno-unused-private-field -Wno-deprecated-volatile)
endif()

#######################################
# Find our dependencies
find_package(PkgConfig REQUIRED)

#######################################
# Create version file
find_package(Git REQUIRED)
execute_process(
    COMMAND ${GIT_EXECUTABLE} log -1 --format=%h
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
    OUTPUT_VARIABLE GIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

set(VERSION_FILE ${CMAKE_CURRENT_BINARY_DIR}/version.c)
configure_file(${CMAKE_CURRENT_LIST_DIR}/src/version.c.in ${VERSION_FILE} @ONLY)

#######################################
# Main library
add_library(shittygui STATIC
    ${VERSION_FILE}
    src/Screen.cpp
)
target_include_directories(shittygui PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
target_include_directories(shittygui PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include/shittygui)
target_include_directories(shittygui PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src)

target_link_libraries(shittygui PUBLIC ${PKG_FREETYPE_LIBRARIES} ${PKG_CAIRO_LIBRARIES}
    ${PKG_HARFBUZZ_LIBRARIES} ${PKG_PANGO_LIBRARIES} ${PKG_PANGOCAIRO_LIBRARIES}
    ${PKG_GLIB2_LIBRARIES} ${PKG_GOBJECT2_LIBRARIES})

add_library(shittygui::shittygui ALIAS shittygui)

#######################################
# PNG support
if(PKG_PNG_FOUND)
    message(STATUS "✅ PNG loading support")
    target_link_libraries(shittygui PUBLIC ${PKG_PNG_LIBRARIES})
    # TODO: add extra sources
else()
    message(STATUS "❌ PNG loading support")
endif()

#######################################
# Fontconfig support
if(PKG_FONTCONFIG_FOUND)
    message(STATUS "✅ Fontconfig support")
    target_link_libraries(shittygui PUBLIC ${PKG_FONTCONFIG_LIBRARIES})
    # TODO: add extra sources
else()
    message(STATUS "❌ Fontconfig support")
endif()

#######################################
# Include examples if this is the top level CMake file
if(PROJECT_IS_TOP_LEVEL)
    add_subdirectory(examples)
endif()

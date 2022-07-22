# Grafkit3

cmake_minimum_required(VERSION 3.15)

project(
  Grafkit
  VERSION 3.0.1.0
  LANGUAGES C CXX)

# -- Options
option(GK_BUILD_TESTS "Build all tests" ON)
option(GK_BUILD_EDITOR "Build editor" ON)
option(GK_BUILD_PLAYER "Build Player" ON)

# --- Project and build options
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# --- Module and prefix path
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_DEBUG_POSTFIX
    "d"
    CACHE STRING "")

list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR}
     "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

add_subdirectory(thirdparties)
add_subdirectory(src)

if(GK_BUILD_TESTS)
  add_subdirectory(tests)
endif()

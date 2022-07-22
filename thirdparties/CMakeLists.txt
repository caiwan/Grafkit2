cmake_minimum_required(VERSION 3.15)

include(CPM)

# This part is unused, using CPM instead.

# include("thirdparties/package-autobuild.cmake")
# include("thirdparties/package-config.cmake")

# CPMUsePackageLock(package-lock.cmake)

# -- Google Test

cpmaddpackage(
  NAME
  googletest
  GITHUB_REPOSITORY
  google/googletest
  GIT_TAG
  release-1.8.1
  VERSION
  1.8.1
  OPTIONS
  "INSTALL_GTEST OFF"
  "gtest_force_shared_crt ON")

add_library(GTest::GTest ALIAS gtest)

# -- Google Benchmark

cpmaddpackage(
  NAME
  benchmark
  GITHUB_REPOSITORY
  google/benchmark
  VERSION
  1.5.0
  OPTIONS
  "BENCHMARK_ENABLE_TESTING Off"
  # "BENCHMARK_USE_LIBCXX ON"
)

if(benchmark_ADDED)
  # patch benchmark target
  set_target_properties(benchmark PROPERTIES CXX_STANDARD 17)
endif()

# -- Json

# CPMAddPackage( NAME nlohmann_json VERSION 3.7.3 # the git repo is incredibly
# large, so we download the archived include directory URL
# https://github.com/nlohmann/json/releases/download/v3.7.3/include.zip URL_HASH
# SHA256=87b5884741427220d3a33df1363ae0e8b898099fbc59f1c451113f6732891014 )
#
# if (nlohmann_json_ADDED) add_library(nlohmann_json INTERFACE IMPORTED)
# target_include_directories(nlohmann_json INTERFACE
# ${nlohmann_json_SOURCE_DIR}/include) endif()

# -- SOL2

# CPMAddPackage( NAME sol2 URL https://github.com/ThePhD/sol2/archive/v3.0.2.zip
# VERSION 3.0.2 DOWNLOAD_ONLY YES )
#
# if (sol2_ADDED) add_library(sol2 INTERFACE IMPORTED)
# target_include_directories(sol2 INTERFACE ${sol2_SOURCE_DIR}/include)
# target_link_libraries(sol2 INTERFACE lua) endif()

# -- cxxopts

# CPMAddPackage( NAME cxxopts GITHUB_REPOSITORY jarro2783/cxxopts VERSION 2.2.0
# OPTIONS "CXXOPTS_BUILD_EXAMPLES Off" "CXXOPTS_BUILD_TESTS Off" )

# -- Sound? -- Bullet?

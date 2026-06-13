cmake_minimum_required(VERSION 3.20)

project(Atlas VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(ATLAS_BUILD_TESTS "Build Atlas tests" ON)
option(ATLAS_BUILD_BENCHMARKS "Build Atlas benchmarks" ON)

add_library(atlas
  src/document.cpp
  src/tokenizer.cpp
)

target_include_directories(atlas
  PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

target_compile_features(atlas PUBLIC cxx_std_20)

if(MSVC)
  target_compile_options(atlas PRIVATE /W4 /permissive-)
else()
  target_compile_options(atlas PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(atlas-cli apps/atlas.cpp)
target_link_libraries(atlas-cli PRIVATE atlas)
set_target_properties(atlas-cli PROPERTIES OUTPUT_NAME atlas)

if(ATLAS_BUILD_TESTS)
  enable_testing()
  add_executable(atlas-tests tests/test_main.cpp)
  target_link_libraries(atlas-tests PRIVATE atlas)
  add_test(NAME atlas-tests COMMAND atlas-tests)
endif()

if(ATLAS_BUILD_BENCHMARKS)
  add_executable(atlas-bench bench/benchmark.cpp)
  target_link_libraries(atlas-bench PRIVATE atlas)
endif()

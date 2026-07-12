cmake_minimum_required(VERSION 3.20)

project(
  spagyrist
  VERSION 0.1.0
  LANGUAGES CXX
)

option(SPAGYRIST_BUILD_TESTS "Build Spagyrist tests" ON)

add_library(spagyrist STATIC
  src/candidate.cpp
  src/document.cpp
  src/output/stdout.cpp
  src/renderer/markdown_renderer.cpp
  src/renderer/plain_renderer.cpp
  src/renderer/renderer.cpp
  src/renderer/terminal_renderer.cpp
  src/selector/selector.cpp
  src/validation.cpp
)

add_library(spagyrist::spagyrist ALIAS spagyrist)

target_compile_features(spagyrist PUBLIC cxx_std_20)

target_include_directories(spagyrist
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_options(spagyrist
  PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall -Wextra -Wpedantic>
)

if(SPAGYRIST_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

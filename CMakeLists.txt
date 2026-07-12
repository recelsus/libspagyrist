cmake_minimum_required(VERSION 3.20)

project(
  spagyrist
  VERSION 0.1.0
  LANGUAGES CXX
)

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(SPAGYRIST_BUILD_TESTS "Build Spagyrist tests" ON)

add_library(spagyrist STATIC
  src/candidate.cpp
  src/document.cpp
  src/output/stdout.cpp
  src/renderer/markdown_renderer.cpp
  src/renderer/plain_renderer.cpp
  src/renderer/renderer.cpp
  src/renderer/terminal_renderer.cpp
  src/selector/fzf.cpp
  src/selector/number.cpp
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

install(
  TARGETS spagyrist
  EXPORT spagyristTargets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
  DIRECTORY include/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
  EXPORT spagyristTargets
  NAMESPACE spagyrist::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/spagyrist
)

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/spagyristConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/spagyristConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/spagyristConfig.cmake"
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/spagyrist
)

install(
  FILES
    "${CMAKE_CURRENT_BINARY_DIR}/spagyristConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/spagyristConfigVersion.cmake"
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/spagyrist
)

if(SPAGYRIST_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

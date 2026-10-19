# © Joseph Cameron - All Rights Reserved

cmake_minimum_required(VERSION 3.21 FATAL_ERROR)

project(jfchash
    VERSION 0.1
    DESCRIPTION "stable, non-cryptographic hashing: the same bits on every platform, compiler and version"
    LANGUAGES CXX)

if (TARGET jfchash)
    return()
endif()

include(GNUInstallDirs)

option(JFCHASH_BUILD_TESTS "Build unit tests" ${PROJECT_IS_TOP_LEVEL})
option(JFCHASH_BUILD_DOCS "Build documentation" OFF)

if (JFCHASH_BUILD_TESTS OR JFCHASH_BUILD_DOCS)
    include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/jfc-cmake/jfclib.cmake")
endif()

add_library(jfchash INTERFACE)

add_library(jfc::hash ALIAS jfchash)

set_target_properties(jfchash PROPERTIES EXPORT_NAME hash)

target_include_directories(jfchash INTERFACE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")

target_compile_features(jfchash INTERFACE cxx_std_20)

include(CMakePackageConfigHelpers)

install(TARGETS jfchash EXPORT jfchash-targets)

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/" DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

install(EXPORT jfchash-targets
    FILE jfchash-targets.cmake
    NAMESPACE jfc::
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/jfchash")

write_basic_package_version_file(
    "${PROJECT_BINARY_DIR}/jfchash-config-version.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
    ARCH_INDEPENDENT)

file(WRITE "${PROJECT_BINARY_DIR}/jfchash-config.cmake"
"include(\"\${CMAKE_CURRENT_LIST_DIR}/jfchash-targets.cmake\")
")

install(FILES
    "${PROJECT_BINARY_DIR}/jfchash-config.cmake"
    "${PROJECT_BINARY_DIR}/jfchash-config-version.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/jfchash")

if (JFCHASH_BUILD_TESTS)
    add_subdirectory(test)
endif()

if (JFCHASH_BUILD_DOCS)
    add_subdirectory(docs)
endif()


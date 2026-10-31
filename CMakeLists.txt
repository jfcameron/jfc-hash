# © Joseph Cameron - MIT License

cmake_minimum_required(VERSION 3.21 FATAL_ERROR)

if (TARGET jfc::hash)
    return()
endif()

project(jfchash
    VERSION 1.0.0
    DESCRIPTION "stable, non-cryptographic hashing: the same bits on every platform, compiler and version"
    LANGUAGES CXX)

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

install(TARGETS jfchash EXPORT jfc-hash-targets)

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/" DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

install(EXPORT jfc-hash-targets
    FILE jfc-hash-targets.cmake
    NAMESPACE jfc::
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/jfc-hash")

write_basic_package_version_file(
    "${PROJECT_BINARY_DIR}/jfc-hash-config-version.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
    ARCH_INDEPENDENT)

file(WRITE "${PROJECT_BINARY_DIR}/jfc-hash-config.cmake"
"include(\"\${CMAKE_CURRENT_LIST_DIR}/jfc-hash-targets.cmake\")
")

install(FILES
    "${PROJECT_BINARY_DIR}/jfc-hash-config.cmake"
    "${PROJECT_BINARY_DIR}/jfc-hash-config-version.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/jfc-hash")

if (JFCHASH_BUILD_TESTS)
    add_subdirectory(test)
endif()

if (JFCHASH_BUILD_DOCS)
    jfc_generate_documentation_doxygen(
        README_PATH      README.md
        OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/docs

        INCLUDE_DIRECTORIES
            include/
    )
endif()


# Cpp Utilities
cmake_minimum_required(VERSION 3.18)

# Building in-tree is not allowed (we take care of your craziness).
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "Prevented in-tree built. Please create a build directory outside of the source code and call cmake from there. Thank you.")
endif()

# Enable if want to see more compile details
# set(CMAKE_VERBOSE_MAKEFILE ON)

# Project configuration
project(CppUtilities VERSION 1.3.0)

set(PROJECT_AUTHOR "ZgblKylin")
set(PROJECT_AUTHOR_EMAIL "feichong.zgbl@foxmail.com")
message("*")
message("* ${PROJECT_NAME} v${PROJECT_VERSION} (${CMAKE_BUILD_TYPE})")
message("* Copyright (c) ${PROJECT_AUTHOR} <${PROJECT_AUTHOR_EMAIL}>")
message("*")

include(CheckCXXCompilerFlag)
if(MSVC)
    add_compile_options("/Zc:__cplusplus")
endif()
if(NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 11)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# CMake configuration
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Referenced packages
set(THREADS_PREFER_PTHREAD_FLAG ON)
include(FindThreads)
find_package(Threads REQUIRED)

# Add main target
add_library(${PROJECT_NAME} INTERFACE IMPORTED GLOBAL)
target_include_directories(${PROJECT_NAME} INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/include
)
target_compile_definitions(${PROJECT_NAME} INTERFACE
    CXX_STANDARD=${CMAKE_CXX_STANDARD}
)
target_compile_options(${PROJECT_NAME} INTERFACE
    $<$<CXX_COMPILER_ID:MSVC>:$<$<COMPILE_LANGUAGE:CXX>:/Zc:__cplusplus>>
)
target_link_libraries(${PROJECT_NAME} INTERFACE Threads::Threads)

# add a target to generate API documentation with Doxygen
find_package(Doxygen 1.8)
OPTION(BUILD_DOCUMENTATION "Create and install the HTML based API documentation (requires Doxygen)" ${DOXYGEN_FOUND})
IF(BUILD_DOCUMENTATION)
    IF(NOT DOXYGEN_FOUND)
        MESSAGE(FATAL_ERROR "Doxygen is needed to build the documentation.")
    ENDIF()

    set(TARGET_DOCS ${PROJECT_NAME}.docs)

    set(DOXY_SOURCE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})
    set(DOXY_DOCS_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/docs)
    set(DOXY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/docs)
    set(DOXY_CFG_FILE ${CMAKE_CURRENT_LIST_DIR}/Doxyfile.cfg)

    add_custom_target(
        ${TARGET_DOCS}
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXY_CFG_FILE}
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
        VERBATIM
        SOURCES ${DOXY_CFG_FILE}
    )

    install(
        DIRECTORY ${DOXY_OUTPUT_DIRECTORY}
        DESTINATION share/${PROJECT_NAME}-${PROJECT_VERSION}/
    )
ENDIF()

# Referenced directories and targets
OPTION(BUILD_TESTING "Build test case with google test" ON)
if(BUILD_TESTING)
    enable_testing()

    # gtest, gtest_main, gmock and gmock_main targets are available from now on
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    add_subdirectory(googletest)

    add_subdirectory(test)
endif()

# AOB
add_custom_target(
    ${PROJECT_NAME}.aob
    SOURCES
        LICENSE
        README.md
        .github/workflows/ccpp.yml
)

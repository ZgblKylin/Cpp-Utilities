#
# Utilities
#

cmake_minimum_required(VERSION 3.2)

#
# Building in-tree is not allowed (we take care of your craziness).
#

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "Prevented in-tree built. Please create a build directory outside of the source code and call cmake from there. Thank you.")
endif()

#
# Project configuration
#

project(Utilities VERSION 1.1.1)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

set(PROJECT_AUTHOR "ZgblKylin")
set(PROJECT_AUTHOR_EMAIL "feichong.zgbl@foxmail.com")

message("*")
message("* ${PROJECT_NAME} v${PROJECT_VERSION} (${CMAKE_BUILD_TYPE})")
message("* Copyright (c) ${PROJECT_AUTHOR} <${PROJECT_AUTHOR_EMAIL}>")
message("*")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g -DDEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DRELEASE")
endif()

#
# CMake configuration
#

set(PROJECT_CMAKE_IN ${Utilities_SOURCE_DIR}/cmake/in)
set(PROJECT_DEPS_DIR ${Utilities_SOURCE_DIR}/deps)
set(PROJECT_SRC_DIR ${Utilities_SOURCE_DIR}/Utilities)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin)

# add a target to generate API documentation with Doxygen

find_package(Doxygen 1.8)
OPTION(BUILD_DOCUMENTATION "Create and install the HTML based API documentation (requires Doxygen)" ${DOXYGEN_FOUND})

IF(BUILD_DOCUMENTATION)
    IF(NOT DOXYGEN_FOUND)
        MESSAGE(FATAL_ERROR "Doxygen is needed to build the documentation.")
    ENDIF()

    set(TARGET_DOCS docs)

    set(DOXY_IN_FILE Doxyfile.in)

    set(DOXY_SOURCE_DIRECTORY ${PROJECT_SRC_DIR})
    set(DOXY_DOCS_DIRECTORY ${Utilities_SOURCE_DIR}/docs)
    set(DOXY_OUTPUT_DIRECTORY ${Utilities_SOURCE_DIR}/docs)
    set(DOXY_CFG_FILE Doxyfile.cfg)

    configure_file(${DOXY_IN_FILE} ${DOXY_CFG_FILE} @ONLY)

    add_custom_target(
        ${TARGET_DOCS}
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXY_DOCS_DIRECTORY}/${DOXY_CFG_FILE}
        WORKING_DIRECTORY ${Utilities_SOURCE_DIR}
        VERBATIM
        SOURCES ${DOXY_IN_FILE}
    )

    install(
        DIRECTORY ${DOXY_OUTPUT_DIRECTORY}/html
        DESTINATION share/${PROJECT_NAME}-${PROJECT_VERSION}/
    )
ENDIF()

#
# Referenced packages
#

set(THREADS_PREFER_PTHREAD_FLAG ON)

include(FindThreads)

#
# Referenced directories and targets
#
OPTION(BUILD_TESTING "Build test case with google test" ON)
if(BUILD_TESTING)
    enable_testing()
    set(GOOGLETEST_DEPS_DIR ${PROJECT_DEPS_DIR}/googletest)

    configure_file(${PROJECT_CMAKE_IN}/deps.in ${PROJECT_DEPS_DIR}/CMakeLists.txt)
    execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" . WORKING_DIRECTORY ${PROJECT_DEPS_DIR})
    execute_process(COMMAND ${CMAKE_COMMAND} --build . WORKING_DIRECTORY ${PROJECT_DEPS_DIR})

    # gtest, gtest_main, gmock and gmock_main targets are available from now on
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    add_subdirectory(${GOOGLETEST_DEPS_DIR})

    add_subdirectory(test)
endif()

#
# Keep your stuff and go further away, foolish.
#

set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VENDOR ${PROJECT_AUTHOR})
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_PACKAGE_VERSION ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${PROJECT_NAME})
set(CPACK_PACKAGE_FILE_NAME ${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION})
set(CPACK_SOURCE_PACKAGE_FILE_NAME ${CPACK_PACKAGE_FILE_NAME}-src)
set(CPACK_RESOURCE_FILE_LICENSE ${Utilities_SOURCE_DIR}/LICENSE)
set(CPACK_RESOURCE_FILE_README ${Utilities_SOURCE_DIR}/README.md)
set(CPACK_GENERATOR TGZ)
set(CPACK_SOURCE_GENERATOR TGZ)
set(CPACK_PACKAGING_INSTALL_DIRECTORY "Utilities-${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}")

include(CPack)

#
# Install headers
#

INCLUDE(GNUInstallDirs)

IF(NOT DEFINED INCLUDE_INSTALL_DIR)
    SET(INCLUDE_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR}")
ENDIF()

file(GLOB_RECURSE HEADERS Utilities/*.h??)

install(
    FILES ${HEADERS}
    COMPONENT ${PROJECT_NAME}
    DESTINATION ${INCLUDE_INSTALL_DIR}/Utilities
    PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
)

#
# AOB
#

add_custom_target(
    Utilities_aob
    SOURCES
        LICENSE
        README.md
)

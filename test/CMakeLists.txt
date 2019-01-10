#
# Tests configuration
#

set(Utilities_SRC_DIR ${PROJECT_SRC_DIR})

set(
    COMMON_LINK_LIBS
    gtest_main
    Threads::Threads
)

macro(ADD_Utilities_TEST TEST_NAME TEST_SOURCE)
    add_executable(${TEST_NAME} ${TEST_SOURCE})
    target_include_directories(${TEST_NAME} PRIVATE ${Utilities_SRC_DIR})
    target_link_libraries(${TEST_NAME} PRIVATE ${COMMON_LINK_LIBS})
    add_test(NAME Utilities_${TEST_NAME} COMMAND $<TARGET_FILE:${TEST_NAME}>)
endmacro()

macro(ADD_Utilities_DIR_TEST TEST_NAME TEST_SOURCE)
    ADD_Utilities_TEST(${TEST_NAME} ${TEST_SOURCE})
    string(TOUPPER "${TEST_NAME}" TEST_NAME_UPPER)
    set(TEST_DIR_VAR "TARGET_${TEST_NAME_UPPER}_DIR")
    set(TEST_DIR ${CMAKE_CURRENT_BINARY_DIR}/${TEST_NAME})
    file(MAKE_DIRECTORY ${TEST_DIR})
    target_compile_definitions(${TEST_NAME} PRIVATE ${TEST_DIR_VAR}="${TEST_DIR}")
endmacro()

macro(ADD_Utilities_LIB_TEST TEST_NAME TEST_SOURCE)
    ADD_Utilities_TEST(${TEST_NAME} ${TEST_SOURCE})
    target_link_libraries(${TEST_NAME} PRIVATE ${CMAKE_DL_LIBS})
    target_compile_definitions(${TEST_NAME} PRIVATE TARGET_LIB_SO="$<TARGET_FILE:uv>")
endmacro()

# List of available targets

ADD_Utilities_TEST(DimensionalAnalysis.Ratios DimensionalAnalysis/Ratios.cpp)
ADD_Utilities_TEST(DimensionalAnalysis.DimensionalAnalysis DimensionalAnalysis/DimensionalAnalysis.cpp)

set(
    COMMON_LINK_LIBS
        CppUtilities
        gtest_main
        Threads::Threads
)

macro(ADD_Utilities_TEST TEST_NAME TEST_SOURCE)
    set(TARGET_NAME ${PROJECT_NAME}.${TEST_NAME})
    add_executable(${TARGET_NAME} ${TEST_SOURCE})
    target_link_libraries(${TARGET_NAME} PRIVATE ${COMMON_LINK_LIBS})
    target_compile_definitions(${TARGET_NAME} PRIVATE CMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD})
    add_test(NAME ${TARGET_NAME} COMMAND $<TARGET_FILE:${TARGET_NAME}>)
endmacro()

macro(ADD_Utilities_DIR_TEST TEST_NAME TEST_SOURCE)
    set(TARGET_NAME ${PROJECT_NAME}.${TEST_NAME})
    ADD_Utilities_TEST(${TARGET_NAME} ${TEST_SOURCE})
    string(TOUPPER "${TARGET_NAME}" TEST_NAME_UPPER)
    set(TEST_DIR_VAR "TARGET_${TEST_NAME_UPPER}_DIR")
    set(TEST_DIR ${CMAKE_CURRENT_BINARY_DIR}/${TEST_NAME})
    file(MAKE_DIRECTORY ${TEST_DIR})
    target_compile_definitions(${TEST_NAME} PRIVATE ${TEST_DIR_VAR}="${TEST_DIR}")
endmacro()

macro(ADD_Utilities_LIB_TEST TEST_NAME TEST_SOURCE)
    set(TARGET_NAME ${PROJECT_NAME}.${TEST_NAME})
    ADD_Utilities_TEST(${TARGET_NAME} ${TEST_SOURCE})
    target_link_libraries(${TARGET_NAME} PRIVATE ${CMAKE_DL_LIBS})
    target_compile_definitions(${TARGET_NAME} PRIVATE TARGET_LIB_SO="$<TARGET_FILE:uv>")
endmacro()

# List of available targets
ADD_Utilities_TEST(DimensionalAnalysis.Ratios DimensionalAnalysis/Ratios.cpp)
ADD_Utilities_TEST(DimensionalAnalysis.DimensionalAnalysis DimensionalAnalysis/DimensionalAnalysis.cpp)
ADD_Utilities_TEST(MemorySafety.SafeSharedPtr MemorySafety/SafeSharedPtr.cpp)
ADD_Utilities_TEST(Container.SequencialMap Container/SequencialMap.cpp)

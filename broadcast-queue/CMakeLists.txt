cmake_minimum_required(VERSION 3.22 FATAL_ERROR)

project(broadcast-queue LANGUAGES CXX)

include(FetchContent)

fetchcontent_declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG b796f7d44681514f58a683a3a71ff17c94edb0c1
)
if(WIN32)
    # Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif()
fetchcontent_getproperties(googletest)
if(NOT googletest_POPULATED)
    fetchcontent_populate(googletest)
    add_subdirectory(
        ${googletest_SOURCE_DIR} ${googletest_BINARY_DIR}
        EXCLUDE_FROM_ALL
    )
endif()

include(GoogleTest)

enable_testing()

message(CHECK_START "Finding memcheck")
find_program(MEMCHECK_PROGRAM valgrind)
if(MEMCHECK_PROGRAM STREQUAL MEMCHECK_PROGRAM-NOTFOUND)
    message(CHECK_FAIL "Could not find memcheck")
    message(WARNING
        "Could not find memcheck - tests will not run with memory checking")
    set(MEMCHECK_PROGRAM "")
else()
    message(CHECK_PASS "Found memcheck")
    set(MEMCHECK_PROGRAM
        ${MEMCHECK_PROGRAM} --leak-check=full --error-exitcode=1)
endif()

macro(add_memcheck_test TARGET)
    if(MEMCHECK_PROGRAM STREQUAL MEMCHECK_PROGRAM-NOTFOUND)
        add_test(NAME ${TARGET} COMMAND $<TARGET_FILE:${TARGET}>)
    else()
        add_test(
            NAME ${TARGET}
            COMMAND ${MEMCHECK_PROGRAM}
                --leak-check=full
                --error-exitcode=1
                $<TARGET_FILE:${TARGET}>
        )
    endif()
endmacro()

add_executable(test-broadcast-queue test-broadcast-queue.cpp)
target_link_libraries(test-broadcast-queue PRIVATE GTest::gtest_main)
target_compile_features(test-broadcast-queue PUBLIC cxx_std_20)
add_memcheck_test(test-broadcast-queue)

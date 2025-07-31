cmake_minimum_required(VERSION 3.22 FATAL_ERROR)

project(observable LANGUAGES CXX)

include(FetchContent)

fetchcontent_declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG b796f7d44681514f58a683a3a71ff17c94edb0c1
)

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

add_executable(test-observable test-observable.cpp)
target_link_libraries(test-observable PRIVATE GTest::gtest_main)
target_compile_features(test-observable PRIVATE cxx_std_20)
target_compile_options(test-observable PRIVATE -fsanitize=address)
target_link_options(test-observable PRIVATE -fsanitize=address)
gtest_discover_tests(test-observable)

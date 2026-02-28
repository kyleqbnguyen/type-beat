cmake_minimum_required(VERSION 3.22)

project(
  type-beat
  VERSION 0.1.0
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_library(core OBJECT src/core/file_validator.cpp)
target_include_directories(core PUBLIC ${CMAKE_SOURCE_DIR}/src)

add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE core Qt6::Widgets)

# Testing
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.15.2)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(tests tests/test_file_validator.cpp)
target_link_libraries(tests PRIVATE core GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(tests)

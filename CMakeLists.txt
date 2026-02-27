cmake_minimum_required(VERSION 3.22)

project(
  type-beat
  VERSION 0.1.0
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(
  ${PROJECT_NAME}
  src/main.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets)

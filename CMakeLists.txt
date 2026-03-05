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

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  add_compile_options(-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Werror)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  add_compile_options(/W4 /WX /permissive-)
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

add_library(core OBJECT src/core/file_utils.cpp src/core/ffprobe.cpp
                        src/core/ffmpeg.cpp)
target_include_directories(core PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(core PUBLIC Qt6::Core)

add_library(settings OBJECT src/settings/app_settings.cpp)
target_include_directories(settings PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(settings PUBLIC Qt6::Core)

add_library(ui OBJECT src/ui/file_input.cpp src/ui/main_window.cpp)
target_include_directories(ui PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(ui PUBLIC Qt6::Widgets Qt6::Core core settings)

add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE ui core settings Qt6::Widgets
                                              Qt6::Core)

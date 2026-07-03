cmake_minimum_required(VERSION 3.23)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(rgb-matrix-4096 LANGUAGES C CXX)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/rpi-rgb-led-matrix/include)

file(GLOB_RECURSE SRC_FILES src/*.cpp)

add_executable(${CMAKE_PROJECT_NAME} ${SRC_FILES})
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/rpi-rgb-led-matrix/rgbmatrix.so)
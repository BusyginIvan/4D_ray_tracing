cmake_minimum_required(VERSION 3.0)
project(ray_tracing)

set(CMAKE_CXX_STANDARD 14)

include_directories(inc)
add_executable(ray_tracing src/main.cpp src/controls.cpp src/util.cpp)

set(SFML_STATIC_LIBRARIES TRUE)
set(SFML_DIR C:\\Users\\busyg\\Desktop\\developer\\SFML-2.5.1\\lib\\cmake\\SFML)
find_package(SFML COMPONENTS system window graphics REQUIRED)
include_directories(C:\\Users\\busyg\\Desktop\\developer\\SFML-2.5.1\\include)
target_link_libraries(ray_tracing sfml-system sfml-window sfml-graphics)
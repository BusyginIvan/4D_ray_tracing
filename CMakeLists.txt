cmake_minimum_required(VERSION 3.0)
project(ray_tracing)

set(CMAKE_CXX_STANDARD 14)
set(SFML_STATIC_LIBRARIES TRUE)
set(SFML_ROOT_DIR «ваш путь»\\SFML-«версия»)

include_directories(inc)
add_executable(ray_tracing src/main.cpp src/controls.cpp src/util.cpp)

set(SFML_DIR ${SFML_ROOT_DIR}\\lib\\cmake\\SFML)
find_package(SFML COMPONENTS system window graphics REQUIRED)
include_directories(${SFML_ROOT_DIR}\\include)
target_link_libraries(ray_tracing sfml-system sfml-window sfml-graphics)
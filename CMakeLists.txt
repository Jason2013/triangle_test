cmake_minimum_required(VERSION 3.16)
project(triangle-test)

# Set glfw options
set(GLFW_BUILD_DOCS OFF)
set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)

add_subdirectory(external/glfw)

add_library(deps INTERFACE)
target_include_directories(deps INTERFACE ${GLFW_SOURCE_DIR}/deps)

add_library(getopt STATIC ${GLFW_SOURCE_DIR}/deps/getopt.c)
target_link_libraries(getopt deps)

add_executable(triangle-test triangle.c)
target_link_libraries(triangle-test glfw getopt)

cmake_minimum_required(VERSION 3.20)
project(triangle-test)

include(FetchContent)

FetchContent_Declare(glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG        dd8a678a66f1967372e5a5e3deac41ebf65ee127
)

# Set glfw options
set(GLFW_BUILD_DOCS OFF)
set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)

FetchContent_MakeAvailable(glfw)

add_library(getopt STATIC ${GLFW_SOURCE_DIR}/deps/getopt.c)
target_include_directories(getopt PUBLIC ${GLFW_SOURCE_DIR}/deps)

add_executable(triangle-test triangle.c)
target_link_libraries(triangle-test glfw getopt)

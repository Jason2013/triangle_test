cmake_minimum_required(VERSION 3.20)
project(triangle-test)

include(FetchContent)
FetchContent_Declare(glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.3.8
)

#FetchContent_Declare(glew
#    GIT_REPOSITORY https://github.com/nigels-com/glew.git
#    GIT_TAG glew-2.2.0
#)

# Set glfw options
set(GLFW_BUILD_DOCS OFF)
set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)

FetchContent_MakeAvailable(glfw)

add_executable(main triangle-opengles.c)
target_link_libraries(main glfw)

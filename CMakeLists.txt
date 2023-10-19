cmake_minimum_required(VERSION 3.18)

# Project
project(pycstl CXX)

set(CMAKE_CXX_STANDARD 20)

if (MSVC)
    set(CMAKE_CXX_FLAGS "/W4 /WX /EHsc")
else()
    set(CMAKE_CXX_FLAGS "-Wall -Wextra -Werror -Wno-unused-function")
endif()

# Options
option(BUILD_TESTS "Build tests" OFF)

# 设置目录
set(SRC_DIR ${PROJECT_SOURCE_DIR}/src)

# 设置导出目录
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# include_directories(${SRC_DIR})

# 编译静态库
file(GLOB_RECURSE SRCS
    ${SRC_DIR}/*.cpp
)
add_library(${PROJECT_NAME} STATIC ${SRCS})
target_include_directories(${PROJECT_NAME} PUBLIC ${SRC_DIR})

add_executable(test test.cpp)
target_link_libraries(test PRIVATE ${PROJECT_NAME})

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
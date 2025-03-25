cmake_minimum_required(VERSION 3.30)
project(file_bundler)

set(CMAKE_CXX_STANDARD 20)

add_executable(file_bundler src/main.cpp)

if (${MSVC})
    target_compile_options(file_bundler PRIVATE "/utf-8")
endif ()

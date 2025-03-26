cmake_minimum_required(VERSION 3.30)
project(file_bundler)

set(CMAKE_CXX_STANDARD 20)

add_executable(file_bundler src/main.cpp)

if (${MSVC})
    target_compile_options(file_bundler PRIVATE "/utf-8")
endif ()

include(FetchContent)
FetchContent_Declare(
        cpp-libs
        GIT_REPOSITORY https://github.com/saku-shirakura/cpp-libs.git
        GIT_TAG v0.1.3-alpha.2 # 必要に応じてバージョンをあげること。
)

FetchContent_MakeAvailable(cpp-libs)

target_link_libraries(file_bundler PRIVATE cpp-libs)

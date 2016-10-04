cmake_minimum_required(VERSION 2.8)
project(MultithreadedObserver)

find_package (Threads)

set(SOURCE_FILES tests/tests.cpp)
include_directories(observer/external/bandit)

add_executable(MultithreadedObserver ${SOURCE_FILES})
set_property(TARGET MultithreadedObserver PROPERTY CXX_STANDARD 14)
target_link_libraries (MultithreadedObserver ${CMAKE_THREAD_LIBS_INIT})
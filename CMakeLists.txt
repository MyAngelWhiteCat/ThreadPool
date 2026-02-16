cmake_minimum_required (VERSION 3.10)

project(ThreadPool CXX) 
set(CMAKE_CXX_STANDARD 20)

add_library(ThreadPool 
    src/ThreadPool/thread_pool.h
    src/ThreadPool/thread_pool.cpp

)

add_executable(TestUseCase 
    src/main.cpp 
)

target_link_libraries(TestUseCase PRIVATE 
    ThreadPool 
)



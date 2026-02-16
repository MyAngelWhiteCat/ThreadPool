#include "ThreadPool/thread_pool.h"

#include <iostream>
#include <string>
#include <syncstream>
#include <Windows.h>


void Print(const std::string& data) {
    std::osyncstream strm{ std::cout };
    strm << data << " from thread id:" << GetThreadId(GetCurrentThread()) << std::endl;
}

int main() {
    try {
        DWORD max_count = GetMaximumProcessorCount(ALL_PROCESSOR_GROUPS);
        std::cout << "Running thread pool with " << max_count << " threads\n";
        ThreadPool thread_pool(max_count);
        for (int i = 0; i < max_count; ++i) {
            thread_pool.AddTask([]() {
                Print("Hello from threadpool!");
                });
        }

    }
    catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
    }
}
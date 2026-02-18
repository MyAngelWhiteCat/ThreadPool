#include "thread_pool.h"

#include <functional>
#include <mutex>
#include <utility>
#include <exception>
#include <string_view>
#include <iostream>
#include <string>

#include <Windows.h>


ThreadPool::ThreadPool(size_t count) {
    hThreads_.reserve(count);
    for (size_t i = 0; i < count; ++i) {
        hThreads_.push_back(
            CreateThread(NULL, 0, ThreadStartWrapper, this, 0, NULL)
        );
    }
    SetDebugLogger([this](const std::string_view info) {
        std::lock_guard lk{ default_logger_mutex_ };
        std::clog << info << std::endl;
        });
    SetErrorLogger([this](const std::string_view error) {
        std::lock_guard lk{ default_logger_mutex_ };
        std::cerr << error << std::endl;
        });
}

ThreadPool::~ThreadPool() {
    destruction_flag_.store(true);
    convar_.notify_all();
    for (HANDLE hThread : hThreads_) {
        WaitForSingleObject(hThread, INFINITE);
        CloseHandle(hThread);
    }
}

void ThreadPool::Work() {
    while (true) {
        std::function<void()> task;
        {
            std::unique_lock lk{ task_mutex_ };
            convar_.wait(lk, [this] {
                return !tasks_.empty() || destruction_flag_.load();
                });
            if (destruction_flag_.load() && tasks_.empty()) {
                return;
            }
            task = std::move(tasks_.front());
            tasks_.pop();
        }
        try {
            task();
        }
        catch (const std::exception& e) {
            error_logger_(std::string("ThreadPool task error: ") + e.what());
        }
    }
}



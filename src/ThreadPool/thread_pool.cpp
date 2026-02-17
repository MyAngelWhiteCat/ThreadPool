#include "thread_pool.h"

#include <functional>
#include <mutex>
#include <utility>
#include <Windows.h>


ThreadPool::ThreadPool(size_t count) {
    hThreads_.reserve(count);
    for (size_t i = 0; i < count; ++i) {
        hThreads_.push_back(
            CreateThread(NULL, 0, ThreadStartWrapper, this, 0, NULL)
        );
    }
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
        task();
    }
}



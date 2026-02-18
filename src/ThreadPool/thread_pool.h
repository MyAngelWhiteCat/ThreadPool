#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <utility>
#include <vector>
#include <string_view>
#include <syncstream>
#include <iostream>

#include <Windows.h>


class ThreadPool {
public:

    ThreadPool(size_t count);
    ~ThreadPool();

    template <typename Fn>
    void AddTask(Fn&& task);


private:
    std::mutex task_mutex_;
    std::queue<std::function<void()>> tasks_;
    std::condition_variable convar_;
    std::vector<HANDLE> hThreads_;
    std::atomic_bool destruction_flag_{ false };

    std::function<void(const std::string_view)> debug_logger_;
    std::function<void(const std::string_view)> error_logger_;
    std::mutex default_logger_mutex_;

    static DWORD WINAPI ThreadStartWrapper(LPVOID lpParam) {
        static_cast<ThreadPool*>(lpParam)->Work();
        return 0;
    }

    void Work();

    template<typename Logger>
    void SetDebugLogger(Logger&& logger);
    template<typename Logger>
    void SetErrorLogger(Logger&& logger);
};

template<typename Fn>
inline void ThreadPool::AddTask(Fn&& task) {
    if (destruction_flag_.load()) {
        throw std::runtime_error("Can't add task to thread pool after d-tor called");
    }
    {
        std::unique_lock lk{ task_mutex_ };
        tasks_.push(std::forward<Fn>(task));
    }
    convar_.notify_one();
}

template<typename Logger>
inline void ThreadPool::SetDebugLogger(Logger&& logger) {
    debug_logger_ = std::forward<Logger>(logger);
}

template<typename Logger>
inline void ThreadPool::SetErrorLogger(Logger&& logger) {
    error_logger_ = std::forward<Logger>(logger);
}

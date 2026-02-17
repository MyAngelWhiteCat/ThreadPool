#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <utility>
#include <vector>
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

    static DWORD WINAPI ThreadStartWrapper(LPVOID lpParam) {
        static_cast<ThreadPool*>(lpParam)->Work();
        return 0;
    }

    void Work();
};

template<typename Fn>
inline void ThreadPool::AddTask(Fn&& task) {
    if (destruction_flag_.load()) {
        throw std::runtime_error("Can't add task to thread pool after d-tor called");
    }
    std::unique_lock lk{ task_mutex_ };
    tasks_.push(std::forward<Fn>(task));
    convar_.notify_one();
}

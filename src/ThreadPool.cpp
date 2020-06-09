#include "../atomy/ThreadPool.h"

#include <chrono>

namespace atomy
{
ThreadPool::ThreadPool(uint8_t n, uint32_t thread_count) : _active(true), _tasks(n)
{
    if (thread_count == 0)
    {
        thread_count = std::thread::hardware_concurrency();
    }
    for (uint32_t i = 0; i < thread_count; ++i)
    {
        _workers.emplace_back([this]() {
            this->workerProcess();
        });
    }
}

ThreadPool::~ThreadPool()
{
    _active = false;
    for (auto &worker : _workers)
    {
        worker.join();
    }
}

bool ThreadPool::excecute(std::function<void()> task)
{
    return _tasks.push(task);
}

void ThreadPool::workerProcess()
{
    uint32_t retry = 0;
    std::function<void()> task;
    for (;;)
    {
        if (!_tasks.pop(task))
        {
            if (!_active)
            {
                break;
            }
            if (++retry >= ATOMY_THREADPOOL_MAX_RETRY)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(ATOMY_THREADPOOL_SLEEP_DURATION));
            }
            continue;
        }
        task();
        retry = 0;
    }
}
} /* namespace atomy */
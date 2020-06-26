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
    std::function<void()> task;
    while (_active)
    {
        if (_tasks.pop(task, std::chrono::milliseconds(100)))
        {
            task();
        }
    }
    while (_tasks.try_pop(task))
    {
        task();
    }
}
} /* namespace atomy */
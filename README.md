# High performance, thread-safe atomic stack, queue, channel and thread pool.

## Usage

An example of using atomy::ThreadPool to execute multiple concurrent tasks.

```c++
#include <atomy/ThreadPool.h>

#include <atomic>
#include <chrono>
#include <iostream>

#define TASK_COUNT 1000000

int main(int argc, char const *argv[])
{
    // Create a pool with queue size of 2^20.
    auto pool = new atomy::ThreadPool(20);
    std::atomic<int> count(0);

    auto start = std::chrono::system_clock::now();
    // Execute multiple concurrent tasks.
    for (int i = 0; i < TASK_COUNT; ++i)
    {
        pool->excecute([&count]() {
            // Count number of tasks done.
            ++count;
        });
    }
    // Delete the pool causing the main thread to stop and wait for all tasks to be completed.
    delete pool;
    auto end = std::chrono::system_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "Completed " << count << " tasks in " << duration << " ms." << std::endl;

    return 0;
}
```

## Build

To build shared library:

    $ make

To build test example:

    $ make test

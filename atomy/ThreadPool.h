#ifndef ATOMY_THREADPOOL_H_
#define ATOMY_THREADPOOL_H_

#include "Queue.h"

#include <exception>
#include <functional>
#include <future>
#include <thread>
#include <vector>

/**
 * Number of retires to execute next task if available before putting worker thread to sleep.
 */
#ifndef ATOMY_THREADPOOL_MAX_RETRY
#define ATOMY_THREADPOOL_MAX_RETRY 5
#endif

/**
 *  Sleep time duration in milliseconds of worker thread when there is no task available.
 */
#ifndef ATOMY_THREADPOOL_SLEEP_DURATION
#define ATOMY_THREADPOOL_SLEEP_DURATION 5
#endif

namespace atomy
{
class ThreadPool
{
public:
  /**
   * @brief atomy::ThreadPool constructor.
   * 
   * @param n             Size of task queue is 2^n.
   * @param thread_count  Number of worker threads (default is number of CPUs).
   */
  ThreadPool(uint8_t n, uint32_t thread_count = 0);

  /**
   * @brief atomy::ThreadPool destructor.
   * 
   * @note When thread pool is deleted, it causes the calling thread 
   * to be blocked until all tasks in the task queue to be completed.
   * Normally you want to keep the thread pool during the execution of 
   * your application hence should never mamually delete the thread pool.
   */
  ~ThreadPool();

  /**
   * @brief Execute a task.
   * 
   * @param task  Task to be executed.
   * 
   * @return true if execute successfully and false if task queue is full.
   */
  bool excecute(std::function<void()> task);

  /**
   * @brief Execute a task that returns result in future.
   * 
   * @param task  Task to be executed.
   * 
   * @return The future result of the task.
   */
  template <typename T>
  std::future<T> excecute(std::function<T()> task);

private:
  bool _active;
  std::vector<std::thread> _workers;
  Queue<std::function<void()>> _tasks;

  void workerProcess();
};
} /* namespace atomy */

template <typename T>
inline std::future<T> atomy::ThreadPool::excecute(std::function<T()> task)
{
  std::promise<T> *p = new std::promise<T>();
  std::shared_future<T> f(p->get_future());
  bool ok = execute([task, p]() {
    p->set_value(task());
    delete p;
  });
  if (!ok)
  {
    p->set_exception(std::make_exception_ptr(std::exception("task queue is full")));
    delete p;
  }
  return f;
}

#endif
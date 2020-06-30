#ifndef ATOMY_CHANNEL_H_
#define ATOMY_CHANNEL_H_

#include "Queue.h"

#include <condition_variable>
#include <chrono>
#include <memory>
#include <mutex>

namespace atomy
{
template <typename T>
class Channel
{
  public:
    /**
     * @brief atomy::Channel constructor.
     * 
     * @param n     Size of channel is 2^n-1.
     */
    Channel(uint8_t n = 1);

    virtual ~Channel();

    /**
     * @brief Push an item to the channel.
     *
     * @param item  An item.
     * 
     * @return true if push successfully and false if the channel is full.
     */
    bool push(T &&item);

    /**
     * @brief Push an item to the channel.
     *
     * @param item  An item.
     * 
     * @return true if push successfully and false if the channel is full.
     */
    bool push(const T &item);

    /**
     * @brief Block the current thread until there is available item and pop it from the channel.
     * 
     * @param item  An item pop from the channel.
     */
    void pop(T &item);

    /**
     * @brief Block the current thread until there is available item and pop it from the channel.
     * 
     * @param item      An item pop from the channel.
     * @param timeout   Wait duration before timeout.
     * 
     * @return true if pop successfully and false on timeout.
     */
    template <typename Rep, typename Period>
    bool pop(T &item, std::chrono::duration<Rep, Period> timeout);

    /**
     * @brief If there is available item, pop it from the channel.
     * 
     * @param item  An item pop from the channel.
     * 
     * @return true if pop successfully and false if channel is empty.
     */
    bool try_pop(T &item);

  private:
    std::shared_ptr<Queue<T>> _queue;
    std::shared_ptr<std::mutex> _mtx;
    std::shared_ptr<std::condition_variable> _cv;
};
} /* namespace atomy */

template <typename T>
inline atomy::Channel<T>::Channel(uint8_t n)
    : _queue(new Queue<T>(n)), _mtx(new std::mutex()), _cv(new std::condition_variable()) {}

template <typename T>
inline atomy::Channel<T>::~Channel() {}

template <typename T>
inline bool atomy::Channel<T>::push(T &&item)
{
    if (_queue->push(std::move(item)))
    {
        _cv->notify_one();
        return true;
    }
    return false;
}

template <typename T>
inline bool atomy::Channel<T>::push(const T &item)
{
    if (_queue->push(item))
    {
        _cv->notify_one();
        return true;
    }
    return false;
}

template <typename T>
inline void atomy::Channel<T>::pop(T &item)
{
    if (_queue->pop(item))
    {
        return;
    }
    std::unique_lock<std::mutex> lck(*_mtx);
    _cv->wait(lck, [this, &item] { return this->_queue->pop(item); });
    lck.unlock();
}

template <typename T>
template <typename Rep, typename Period>
inline bool atomy::Channel<T>::pop(T &item, std::chrono::duration<Rep, Period> timeout)
{
    if (_queue->pop(item))
    {
        return true;
    }
    std::unique_lock<std::mutex> lck(*_mtx);
    if (_cv->wait_for(lck, timeout, [this, &item] { return this->_queue->pop(item); }))
    {
        lck.unlock();
        return true;
    }
    return false;
}

template <typename T>
inline bool atomy::Channel<T>::try_pop(T &item)
{
    return _queue->pop(item);
}

#endif
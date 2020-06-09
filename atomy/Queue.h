#ifndef ATOMY_QUEUE_H_
#define ATOMY_QUEUE_H_

#include <atomic>
#include <stdexcept>
#include <thread>
#include <utility>

namespace atomy
{
template <typename T>
class Queue
{
  public:
    /**
     * @brief atomy::Queue constructor.
     * 
     * @param n     Queue size is equal to 2^n.
     */
    Queue(uint8_t n);

    virtual ~Queue();

    /**
     * @brief Push an item into the queue.
     * 
     * @param item  An item.
     * 
     * @return true if push successfully and false if the queue is full.
     */
    bool push(T &&item);

    /**
     * @brief Push an item into the queue.
     * 
     * @param item  An item.
     * 
     * @return true if push successfully and false if the queue is full.
     */
    bool push(const T &item);

    /**
     * @brief Pop an item from the queue.
     * 
     * @param item  An item pop from the queue.
     * 
     * @return true if pop successfully and false if the queue is empty.
     */
    bool pop(T &item);

    /**
     * @brief Checks whether the queue is full.
     * 
     * @return true if the queue is full and false otherwise.
     */
    bool full() const;

    /**
     * @brief Checks whether the queue is empty.
     * 
     * @return true if the queue is empty and false otherwise.
     */
    bool empty() const;

    /**
     * @brief Get maximum size of the queue.
     * 
     * @return Queue size.
     */
    size_t size() const;

    /**
     * @brief Get current number of items inside the queue.
     * 
     * @return Number of items.
     */
    size_t count() const;

  private:
    const size_t _mask;
    std::atomic<size_t> _pre_head;
    std::atomic<size_t> _pos_head;
    std::atomic<size_t> _pre_tail;
    std::atomic<size_t> _pos_tail;
    T *_items;
};
} /* namespace atomy */

template <typename T>
inline atomy::Queue<T>::Queue(uint8_t n)
    : _mask((1 << n) - 1), _pre_head(0), _pos_head(0), _pre_tail(0), _pos_tail(0), _items(new T[_mask + 1]) {}

template <typename T>
inline atomy::Queue<T>::~Queue()
{
    delete[] _items;
}

template <typename T>
inline bool atomy::Queue<T>::push(T &&item)
{
    size_t tail = _pre_tail;
    do
    {
        if (full())
        {
            return false;
        }
    } while (!_pre_tail.compare_exchange_weak(tail, (tail + 1) & _mask));
    _items[tail] = std::move(item);
    while (_pos_tail != tail)
    {
        std::this_thread::yield();
    }
    _pos_tail = (tail + 1) & _mask;
    return true;
}

template <typename T>
inline bool atomy::Queue<T>::push(const T &item)
{
    size_t tail = _pre_tail;
    do
    {
        if (full())
        {
            return false;
        }
    } while (!_pre_tail.compare_exchange_weak(tail, (tail + 1) & _mask));
    _items[tail] = item;
    while (_pos_tail != tail)
    {
        std::this_thread::yield();
    }
    _pos_tail = (tail + 1) & _mask;
    return true;
}

template <typename T>
inline bool atomy::Queue<T>::pop(T &item)
{
    size_t head = _pre_head;
    do
    {
        if (empty())
        {
            return false;
        }
    } while (!_pre_head.compare_exchange_weak(head, (head + 1) & _mask));
    item = std::move(_items[head]);
    while (_pos_head != head)
    {
        std::this_thread::yield();
    }
    _pos_head = (head + 1) & _mask;
    return true;
}

template <typename T>
inline bool atomy::Queue<T>::full() const
{
    return _pos_head == ((_pre_tail + 1) & _mask);
}

template <typename T>
inline bool atomy::Queue<T>::empty() const
{
    return _pre_head == _pos_tail;
}

template <typename T>
inline size_t atomy::Queue<T>::size() const
{
    return _mask + 1;
}

template <typename T>
inline size_t atomy::Queue<T>::count() const
{
    return (_pre_tail - _pre_head) & _mask;
}

#endif
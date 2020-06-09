#ifndef ATOMY_STACK_H_
#define ATOMY_STACK_H_

#include <atomic>
#include <stdexcept>
#include <utility>

namespace atomy
{
template <typename T>
class Stack
{
    struct Node
    {
        T value;
        Node *next;
        Node(T &&value, Node *next = nullptr);
        Node(const T &value, Node *next = nullptr);
    };

  public:
    Stack();

    virtual ~Stack();

    /**
     * @brief Push an item into the stack.
     * 
     * @param item  An item.
     */
    void push(T &&item);

    /**
     * @brief Push an item into the stack.
     * 
     * @param item  An item.
     */
    void push(const T &item);

    /**
     * @brief Pop an item from the stack.
     * 
     * @param item  An item pop from the stack.
     * 
     * @return true if pop successfully and false if the stack is empty.
     */
    bool pop(T &item);

    /**
     * @brief Check whether the stack is empty.
     * 
     * @return true if the stack is empty and false otherwise.
     */
    bool empty() const;

  private:
    std::atomic<Node *> _top;
};
} /* namespace atomy */

template <typename T>
inline atomy::Stack<T>::Node::Node(T &&value, Node *next) : value(std::move(value)), next(next) {}

template <typename T>
inline atomy::Stack<T>::Node::Node(const T &value, Node *next) : value(value), next(next) {}

template <typename T>
inline atomy::Stack<T>::Stack() : _top(nullptr) {}

template <typename T>
inline atomy::Stack<T>::~Stack()
{
    Node *node;
    while ((node = _top) != nullptr)
    {
        _top = _top->next;
        delete node;
    }
}

template <typename T>
inline atomy::Stack<T>::push(T &&item)
{
    Node *node = new Node(std::move(item), _top);
    while (!_top.compare_exchange_weak(node->next, node))
    {
    }
}

template <typename T>
inline atomy::Stack<T>::push(const T &item)
{
    Node *node = new Node(item, _top);
    while (!_top.compare_exchange_weak(node->next, node))
    {
    }
}

template <typename T>
inline bool atomy::Stack<T>::pop(T &item)
{
    Node *node = _top;
    do
    {
        if (node == nullptr)
        {
            return false;
        }
    } while (!_top.compare_exchange_weak(node, node->next));
    item = std::move(node->value);
    delete node;
    return true;
}

template <typename T>
inline bool atomy::Stack<T>::empty() const
{
    return _top == nullptr;
}

#endif
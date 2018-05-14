/*
 * queue.hpp
 *
 *  Created on: May 14, 2018
 *      Author: cannon
 */

#ifndef QUEUE_HPP_
#define QUEUE_HPP_

#include <atomic>
#include <memory>
#include <mutex>

#define MAX_QUEUE_SIZE 100

class spinlock
{
    std::atomic_flag flag = ATOMIC_FLAG_INIT;
public:
    void lock() noexcept {
        while (flag.test_and_set(std::memory_order_acquire))
            ;
    }
    void unlock() noexcept { flag.clear(std::memory_order_release); }
    bool try_lock() noexcept {
        return !flag.test_and_set(std::memory_order_acquire);
    }
};

template < typename T >
class Queue
{
    //static constexpt unsigned MAX_QUEUE_SIZE = 100;
public:
    Queue();
    explicit Queue(unsigned int maxSize = MAX_QUEUE_SIZE);

    Queue(const Queue &other);
    Queue& operator=(const Queue &) = delete;

    ~Queue();

    // pushes an item to Queue tail
    void enqueue(const T& item);

    // pops an item from Queue head
    std::shared_ptr< T > dequeue();

    // try to push an item to Queue tail
    bool try_and_enqueue(const T& item);

    // try to pop and item from Queue head
    std::shared_ptr< T > try_and_dequeue();

    bool full();
    bool empty();
    unsigned capacity() { return CAPACITY; }
    unsigned count();

protected:
    spinlock lock;
    const unsigned CAPACITY;  // Queue capacity
    T *data;                  // array to store the items
    unsigned cnt;             // Queue count
    unsigned head;            // also the readIndex
    unsigned tail;            // also the writeIndex
};

template < typename T >
Queue< T >::Queue(unsigned int maxSize): CAPACITY(maxSize), cnt(0), head(0), tail(0)
{
    data = new T[CAPACITY];
}

template < typename T >
Queue< T >::Queue(const Queue &other)
{
    std::lock_guard< spinlock > lg(lock);
    CAPACITY = other.CAPACITY;
    cnt = other.cnt;
    head = other.head;
    tail = other.tail;
    data = new T[CAPACITY];
    for (unsigned i = 0; i < CAPACITY; ++i)
        data[i] = other.data[i];
}

template < typename T >
Queue< T >::~Queue()
{
    delete[] data;
}

template < typename T >
void Queue< T >::enqueue(const T &item)
{
    while (!try_and_enqueue(item))
        ;
}

template < typename T >
std::shared_ptr< T > Queue< T >::dequeue()
{
    thread_local std::shared_ptr< T > ptr;
    while ((ptr = try_and_dequeue()) == nullptr)
        ;
    return ptr;
}

template < typename T >
bool Queue< T >::try_and_enqueue(const T &item)
{
    std::lock_guard< spinlock > lg(lock);
    if (cnt == CAPACITY)
        return false;    // full
    ++cnt;
    data[tail++] = std::move(item);
    if (tail == CAPACITY)
        tail -= CAPACITY;
    return true;
}

template < typename T >
std::shared_ptr< T > Queue< T >::try_and_dequeue()
{
    std::lock_guard< spinlock > lg(lock);
    if (cnt == 0)
        return std::shared_ptr< T >();    // empty
    --cnt;
    unsigned idx = head;
    ++head;
    if (head == CAPACITY)
        head -= CAPACITY;
    return std::make_shared< T >(std::move(data[idx]));
}

template < typename T >
bool Queue< T >::full()
{
    std::lock_guard< spinlock > lg(lock);
    return cnt == CAPACITY;
}

template < typename T >
bool Queue< T >::empty()
{
    std::lock_guard< spinlock > lg(lock);
    return cnt == 0;
}

template < typename T >
unsigned Queue< T >::count()
{
    std::lock_guard< spinlock > lg(lock);
    return cnt;
}
#endif /* QUEUE_HPP_ */

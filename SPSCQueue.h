#ifndef SINGLE_PRODUCER_SINGLE_CONSUMER_LOCKLESS_QUEUE_H
#define SINGLE_PRODUCER_SINGLE_CONSUMER_LOCKLESS_QUEUE_H

#include <vector>
#include <atomic>
#include <iostream>

template <typename T>
class SPSCQueue {
public:
    SPSCQueue(int capacity)
        : capacity_(capacity)
        , buffer_(capacity_ + 1) // to indicate full, we leave an empty space
        , head_(0)
        , tail_(0)
    {
    }

    bool push(const T& item)
    {
        // We can use relaxed here because there's only a single writer thread updating tail (current one)
        int tail = tail_.load(std::memory_order_relaxed);
        int nextTail = next(tail);

        if (nextTail == head_.load(std::memory_order_acquire)) {
            // next tail is set to head so queue is full
            return false;
        }

        buffer_[tail] = item;
        tail_.store(nextTail, std::memory_order_release);
    }

    bool pop(T& item)
    {
        // We can use relaxed here because there's only a single reader thread updating head (current one)
        int head = head_.load(std::memory_order_relaxed);  
        
        if (head == tail_.load(std::memory_order_acquire)) {
            // queue is empty, head == tail
            return false;
        }

        item = buffer_[head];
        head_.store(next(head), std::memory_order_release);
    }

    ~SPSCQueue()
    {        
    }
private:
    int next(int current) {
        return (current + 1) % capacity_;
    }
private:
    int capacity_;
    std::vector<T> buffer_;
    std::atomic<int> head_; // read/pop from head
    std::atomic<int> tail_; // write/push into tail
};

#endif
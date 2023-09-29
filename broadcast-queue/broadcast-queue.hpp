#pragma once

#include <cstdio>
#include <deque>
#include <list>
#include <memory>
#include <queue>

namespace recap::app::broadcast_queue {

template <class T, class Container> class BroadcastQueueSubscriber;

/**
 * A simple queue container for queuing items to multiple consumers.
 *
 *   BroadcastQueue<int> queue;
 *   auto sub1 = queue.subscribe();
 *   auto sub2 = queue.subscribe();
 *   queue.push(10);
 *   sub1.front() // returns 10
 *   sub1.pop();
 *   sub2.front() // returns 10
 *
 * NOT THREAD SAFE.
 */
template <class T, class Container = std::deque<T>> class BroadcastQueue {
public:
    using container_type = Container;
    using queue_type = std::queue<T, container_type>;
    using size_type = typename queue_type::size_type;
    using subscriber_type = BroadcastQueueSubscriber<T, Container>;

    subscriber_type subscribe()
    {
        return subscriber_type(
            *this, observers.emplace(observers.end(), std::make_unique<queue_type>())
        );
    }

    void clear()
    {
        for (const auto& queue : observers)
            queue_type().swap(*queue);
    }

    void push(const typename queue_type::value_type& value)
    {
        for (const auto& queue : observers)
            queue->push(value);
    }

    void push(typename queue_type::value_type&& value)
    {
        for (const auto& queue : observers)
            queue->push(value);
    }

    size_type num_subscribers() const { return observers.size(); }

private:
    friend subscriber_type;

    using observer_list_t = std::list<std::unique_ptr<queue_type>>;
    using handle_t = typename observer_list_t::iterator;

    observer_list_t observers;

    void unsubscribe(handle_t handle) { observers.erase(handle); }
};

/**
 * Provides the same element access and capacity interfaces as std::queue<T>
 */
template <class T, class Container> class BroadcastQueueSubscriber {
public:
    using queue_type = BroadcastQueue<T, Container>;
    using value_type = typename queue_type::queue_type::value_type;
    using size_type = typename queue_type::queue_type::size_type;
    using reference = typename queue_type::queue_type::reference;
    using const_reference = typename queue_type::queue_type::const_reference;

    BroadcastQueueSubscriber(const BroadcastQueueSubscriber&) = delete;
    BroadcastQueueSubscriber& operator=(const BroadcastQueueSubscriber&) = delete;
    BroadcastQueueSubscriber(BroadcastQueueSubscriber&&) = delete;
    BroadcastQueueSubscriber& operator=(BroadcastQueueSubscriber&&) = delete;

    ~BroadcastQueueSubscriber() { controller.unsubscribe(handle); }

    bool empty() const { return (*handle)->empty(); }

    const_reference back() const { return (*handle)->back(); }

    reference back() { return (*handle)->back(); }

    const_reference front() const { return (*handle)->front(); }

    reference front() { return (*handle)->front(); }

    void pop() { (*handle)->pop(); }

    size_type size() const { return (*handle)->size(); }

private:
    friend queue_type;

    queue_type& controller;
    typename queue_type::handle_t handle;

    BroadcastQueueSubscriber(queue_type& controller, typename queue_type::handle_t handle) :
        controller(controller), handle(handle)
    {}
};

}; // namespace recap::app::broadcast_queue

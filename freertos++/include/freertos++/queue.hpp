#ifndef FREERTOS_QUEUE_HPP_INCLUDE
#define FREERTOS_QUEUE_HPP_INCLUDE

extern "C" {
#include <FreeRTOS.h>
#include <queue.h>
};

#include <cstdint>
#include <type_traits>
#include <limits>

namespace freertos {

template <typename T> class Queue {
public:
    // Or does it need to be trivial?
    static_assert(std::is_trivially_copyable<T>::value, "T must be trivially copyable");

    using value_type = T;
    using Timeout = TickType_t;
    using size_type = UBaseType_t;

    ~Queue() noexcept { delete_queue_handle(); }

    Queue(Queue&& other) noexcept
    : m_queue_handle(other.m_queue_handle)
    {
        other.m_queue_handle = nullptr;
    }

    Queue& operator=(Queue&& other) noexcept
    {
        if (this != &other) {
            delete_queue_handle();
            m_queue_handle = other.m_queue_handle;
            other.m_queue_handle = nullptr;
        }

        return *this;
    }

    Queue(const Queue&) = delete;
    Queue& operator=(const Queue&) = delete;

    QueueHandle_t handle() { return m_queue_handle; }

    QueueHandle_t handle() const { return m_queue_handle; }

    bool send(const T& val, Timeout ticks = 0)
    {
        return xQueueSend(m_queue_handle, &val, ticks) == pdTRUE;
    }

    void overwrite(const T& val)
    {
        xQueueOverwrite(m_queue_handle, &val);
    }

    bool
    send_from_isr(const T& val, BaseType_t *higher_pri_task_woken = nullptr)
    {
        return xQueueSendFromISR(
            m_queue_handle, &val, higher_pri_task_woken
        ) == pdTRUE;
    }

    void overwrite_from_isr(
        const T& val, BaseType_t *higher_pri_task_woken = nullptr
    )
    {
        xQueueOverwriteFromISR(m_queue_handle, &val, higher_pri_task_woken);
    }

    bool receive(T& val, Timeout ticks = 0)
    {
        return xQueueReceive(m_queue_handle, &val, ticks) == pdTRUE;
    }

    bool peek(T& val, Timeout ticks = 0)
    {
        return xQueuePeek(m_queue_handle, &val, ticks) == pdTRUE;
    }

    bool peek_from_isr(T& val)
    {
        return xQueuePeekFromISR(m_queue_handle, &val);
    }

    size_type messages_waiting() const
    {
        return uxQueueMessagesWaiting(m_queue_handle);
    }

    size_type spaces_available() const
    {
        return uxQueueSpacesAvailable(m_queue_handle);
    }

protected:
    explicit Queue(QueueHandle_t queue_handle) : m_queue_handle(queue_handle) {}

private:
    QueueHandle_t m_queue_handle;

    void delete_queue_handle() noexcept
    {
        if (m_queue_handle != nullptr) {
            vQueueDelete(m_queue_handle);
        }
    }
};

#if configSUPPORT_STATIC_ALLOCATION
template <typename T, Queue<T>::size_type N> class StaticQueue : public Queue<T> {
public:
    using size_type = Queue<T>::size_type;

    static_assert(N > 0, "N must be non-zero");
    static_assert(
        N < std::numeric_limits<size_type>::max() / sizeof(T),
        "N is too large"
    );

    StaticQueue() : Queue<T>(xQueueCreateStatic(
        N,
        sizeof(T),
        m_queue_storage_buffer,
        &m_static_queue
    )) {}

    constexpr size_type capacity() { return N; }

private:
    StaticQueue_t m_static_queue;
    uint8_t m_queue_storage_buffer[N * sizeof(T)] = {};
};
#endif // configSUPPORT_STATIC_ALLOCATION

#if configSUPPORT_DYNAMIC_ALLOCATION
template <typename T> class DynamicQueue : public Queue<T> {
public:
    using size_type = Queue<T>::size_type;

    explicit DynamicQueue(size_type length)
    : Queue<T>(xQueueCreate(length, sizeof(T)))
    {}
};
#endif // configSUPPORT_DYNAMIC_ALLOCATION

};  // namespace freertos

#endif // FREERTOS_QUEUE_HPP_INCLUDE

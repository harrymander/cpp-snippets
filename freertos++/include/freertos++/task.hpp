#ifndef FREERTOS_TASK_HPP_INCLUDE
#define FREERTOS_TASK_HPP_INCLUDE

extern "C" {
#include <FreeRTOS.h>
#include <task.h>
};

#include <cstddef>

namespace freertos {

using StackDepth = configSTACK_DEPTH_TYPE;
using TaskPriority = UBaseType_t;

// A wrapper around the TaskHandle_t pointer
class Task {
public:
    explicit Task(TaskHandle_t handle) : m_task_handle(handle) {}

    TaskHandle_t handle() const { return m_task_handle; }

    bool good() const { return m_task_handle != nullptr; }

    explicit operator bool() const { return good(); }

    // Can add extra member functions for the FreeRTOS API functions that take
    // task handles. E.g. vTaskSuspend

private:
    TaskHandle_t m_task_handle = nullptr;
};


#if configSUPPORT_STATIC_ALLOCATION
template <StackDepth StackSize> class StaticTaskData {
    static_assert(StackSize > 0, "StackSize must be non-zero");
    static_assert(
        StackSize >= configMINIMAL_STACK_SIZE,
        "StackSize must be at least configMINIMAL_STACK_SIZE"
    );

public:
    constexpr StackDepth stack_size() const { return StackSize; }

    StaticTask_t *task_buffer() { return &m_task_buffer; }

    StackType_t *stack_buffer() { return m_stack_buffer; }

private:
    StaticTask_t m_task_buffer;
    StackType_t m_stack_buffer[StackSize];
};
#endif // configSUPPORT_STATIC_ALLOCATION

// Create a task with static allocation
template <typename CB, typename StaticData>
inline Task create_task(
    CB&& callback,
    const char *name,
    TaskPriority priority,
    StaticData& static_data
) {
    auto handle = xTaskCreateStatic(
        callback.callback,
        name,
        static_data.stack_size(),
        callback.callback_data(),
        priority,
        static_data.stack_buffer(),
        static_data.task_buffer()
    );
    return Task{handle};
}

// Create a task with dynamic allocation
template <typename CB>
inline Task create_task(
    CB&& callback,
    const char *name,
    TaskPriority priority,
    StackDepth stack_depth
) {
    configASSERT(stack_depth > 0);
    TaskHandle_t handle;
    auto ret = xTaskCreate(
        callback.callback,
        name,
        stack_depth,
        callback.callback_data(),
        priority,
        &handle
    );
    return Task{ret == pdPASS ? handle : nullptr};
}

}; // namespace freertos

#endif // FREERTOS_TASK_HPP_INCLUDE

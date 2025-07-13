#ifndef FREERTOS_MUTEX_HPP_INCLUDE
#define FREERTOS_MUTEX_HPP_INCLUDE

extern "C" {
#include <FreeRTOS.h>
#include <semphr.h>
};

namespace freertos {

class Mutex {
public:
    using Timeout = TickType_t;

    explicit Mutex(SemaphoreHandle_t handle) : m_handle(handle) {}

    void lock() const { xSemaphoreTake(m_handle, portMAX_DELAY); }

    bool try_lock(Timeout timeout = 0) const
    {
        return xSemaphoreTake(m_handle, timeout) == pdTRUE;
    }

    void unlock() const { xSemaphoreGive(m_handle); }

private:
    SemaphoreHandle_t m_handle;
};

#if configSUPPORT_DYNAMIC_ALLOCATION
class DynamicMutex : public Mutex {
public:
    DynamicMutex() : Mutex(xSemaphoreCreateMutex()) {}
};
#endif // configSUPPORT_DYNAMIC_ALLOCATION

#if configSUPPORT_STATIC_ALLOCATION
class StaticMutex : public Mutex {
public:
    StaticMutex() : Mutex(xSemaphoreCreateMutexStatic(&m_buffer)) {}

private:
    StaticSemaphore_t m_buffer;
};
#endif // configSUPPORT_STATIC_ALLOCATION


}; // namespace freertos

#endif // FREERTOS_MUTEX_HPP_INCLUDE

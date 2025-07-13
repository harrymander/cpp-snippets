#ifndef FREERTOS_LOCK_DELAY_TIMER_HPP_INCLUDE
#define FREERTOS_LOCK_DELAY_TIMER_HPP_INCLUDE

extern "C" {
#include <FreeRTOS.h>
#include <task.h>
};

namespace freertos {

class DelayTimer {
public:
    using tick_type = TickType_t;

    DelayTimer() : m_wake_time(xTaskGetTickCount()) {}

    // Returns true if task was actually delayed
    bool delay_until(tick_type ticks)
    {
        return xTaskDelayUntil(&m_wake_time, ticks) == pdTRUE;
    }

private:
    tick_type m_wake_time;
};

}; // namespace freertos

#endif // FREERTOS_LOCK_DELAY_TIMER_HPP_INCLUDE

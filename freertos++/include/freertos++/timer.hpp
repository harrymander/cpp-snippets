#ifndef FREERTOS_TIMER_HPP_INCLUDE
#define FREERTOS_TIMER_HPP_INCLUDE

extern "C" {
#include <FreeRTOS.h>
#include <timers.h>
};

#include <concepts>
#include <utility>

namespace freertos {

// Encapsulated interface to a timer handle
class Timer {
public:
    using tick_type = TickType_t;

    explicit Timer(TimerHandle_t timer_handle) : m_timer_handle(timer_handle) {}

    TimerHandle_t handle() const { return m_timer_handle; }

    bool good() const { return m_timer_handle != nullptr; }

    explicit operator bool() const { return good(); }

    bool is_active() const { return xTimerIsTimerActive(m_timer_handle); }

    enum class ReloadMode {
        Auto,
        OneShot,
    };

    ReloadMode reload_mode() const
    {
        return xTimerGetReloadMode(m_timer_handle) ? ReloadMode::Auto
                                                   : ReloadMode::OneShot;
    }

    void set_reload_mode(ReloadMode mode)
    {
        vTimerSetReloadMode(m_timer_handle, mode == ReloadMode::Auto);
    }

    tick_type expiry_time() const { return xTimerGetExpiryTime(m_timer_handle); }

    tick_type period() const { return xTimerGetPeriod(m_timer_handle); }

    // Cannot be called from ISR (TODO: ISR specific method)
    bool start(tick_type block_time = portMAX_DELAY)
    {
        return xTimerStart(m_timer_handle, block_time) == pdPASS;
    }

    // Cannot be called from ISR (TODO: ISR specific method)
    bool stop(tick_type block_time = portMAX_DELAY)
    {
        return xTimerStop(m_timer_handle, block_time) == pdPASS;
    }

    // Cannot be called from ISR (TODO: ISR specific method)
    bool set_period(tick_type new_period, tick_type block_time)
    {
        configASSERT(new_period > 0);
        return xTimerChangePeriod(m_timer_handle, new_period, block_time) == pdPASS;
    }

private:
    TimerHandle_t m_timer_handle;
};

template <typename T> concept TimerFunction = std::invocable<T>;

namespace internal::timer {

template <TimerFunction Callback> inline void timer_callback(TimerHandle_t timer)
{
    // There is some overhead in calling the callback through a pointer that I
    // don't think can be optimised out. If you are really concerned about
    // performance create a timer with a custom callback directly using
    // `xTimerCreate` and pass the handle to `Timer` directly.
    (*static_cast<Callback *>(pvTimerGetTimerID(timer)))();
}

// Create a timer using `create_timer` function, passing any additional
// argument in `...extra_args`. The timer is assigned a callback function that
// invokes `callback` without any arguments.
template <typename CreateTimer, TimerFunction Callback, typename... Args>
inline TimerHandle_t create_timer(
    CreateTimer create_timer,
    const char *name,
    Timer::tick_type period,
    Timer::ReloadMode reload_mode,
    Callback callback,
    Args&&... extra_args
) {
    configASSERT(period > 0);
    return create_timer(
        name,
        period,
        reload_mode == Timer::ReloadMode::Auto,
        &callback,  // Store the callback in the timer ID
        internal::timer::timer_callback<Callback>,
        std::forward<Args>(extra_args)...
    );
}

}; // namespace internal::timer

#if configSUPPORT_STATIC_ALLOCATION
template <TimerFunction Callback> class StaticTimer : public Timer {
public:
    StaticTimer(
        const char *name,
        tick_type period,
        Timer::ReloadMode reload_mode,
        Callback callback
    ) : Timer(
        internal::timer::create_timer(
            xTimerCreateStatic,
            name,
            period,
            reload_mode,
            callback,
            &m_buffer
        )
    ) {}

private:
    StaticTimer_t m_buffer;
};
#endif // configSUPPORT_STATIC_ALLOCATION

#if configSUPPORT_DYNAMIC_ALLOCATION
template <TimerFunction Callback> class DynamicTimer : public Timer {
public:
    DynamicTimer(
        const char *name,
        tick_type period,
        Timer::ReloadMode reload_mode,
        Callback callback
    ) : Timer(
        internal::timer::create_timer(
            xTimerCreate,
            name,
            period,
            reload_mode,
            callback
        )
    ) {}
};
#endif // configSUPPORT_DYNAMIC_ALLOCATION

}; // namespace freertos

#endif // FREERTOS_TIMER_HPP_INCLUDE

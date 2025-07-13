#ifndef FREERTOS_LOCK_GUARD_HPP_INCLUDE
#define FREERTOS_LOCK_GUARD_HPP_INCLUDE

extern "C" {
#include <FreeRTOS.h>
};

#include <concepts>

namespace freertos {

template <typename T>
concept Lockable = requires(T lockable)
{
    lockable.lock();
    lockable.unlock();
};

template <Lockable Mutex> class LockGuard {
public:
    explicit LockGuard(Mutex& mutex) : m_mutex(mutex)
    {
        m_mutex.lock();
    }

    ~LockGuard() noexcept { m_mutex.unlock(); }

    LockGuard(const LockGuard&) = delete;
    LockGuard& operator=(const LockGuard&) = delete;
    LockGuard(LockGuard&&) = delete;
    LockGuard& operator=(LockGuard&&) = delete;

private:
    Mutex& m_mutex;
};


template <typename T>
concept TryLockable =
    Lockable<T> &&
    requires(T lockable, typename T::Timeout timeout)
    {
        { lockable.try_lock() } -> std::same_as<bool>;
        { lockable.try_lock(timeout) } -> std::same_as<bool>;
    } &&
    std::is_integral_v<typename T::Timeout>;

/**
 * Like LockGuard, but allows specifying a timeout for locking. The lock
 * status must be checked after construction, e.g.:
 *
 * LockGuardTimeout<Mutex> lock(mutex, pdMS_TO_TICKS(100));
 * if (lock) {
 *     // Lock successfully acquired. Use the resource...
 * } else {
 *     // Handle error...
 * }
 */
template <TryLockable Mutex> class LockGuardTimeout {
public:
    using Timeout = Mutex::Timeout;

    explicit LockGuardTimeout(Mutex& mutex, Timeout timeout = 0)
    : m_mutex(mutex),
      m_locked(m_mutex.try_lock(timeout))
    {}

    ~LockGuardTimeout() noexcept
    {
        if (m_locked) {
            m_mutex.unlock();
        }
    }

    bool locked() const { return m_locked; }

    explicit operator bool() const { return locked(); }

    LockGuardTimeout(const LockGuardTimeout&) = delete;
    LockGuardTimeout& operator=(const LockGuardTimeout&) = delete;
    LockGuardTimeout(LockGuardTimeout&&) = delete;
    LockGuardTimeout& operator=(LockGuardTimeout&&) = delete;

private:
    Mutex& m_mutex;
    bool m_locked;
};

}; // namespace freertos

#endif // FREERTOS_LOCK_GUARD_HPP_INCLUDE

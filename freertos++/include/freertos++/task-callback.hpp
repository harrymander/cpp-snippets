#ifndef FREERTOS_TASK_CALLBACK_HPP_INCLUDE
#define FREERTOS_TASK_CALLBACK_HPP_INCLUDE

extern "C" {
#include <FreeRTOS.h>
};

#include <concepts>
#include <concepts>
#include <cstddef>
#include <new> // for placement new
#include <type_traits>
#include <utility>

namespace freertos {


template <typename Tag, typename... Args> class TaskCallback;

namespace internal::task_callback {
class StatelessLambdaTag {};
};

// Stateless lambda that takes an argument:
template <typename F, typename Arg>
class TaskCallback<internal::task_callback::StatelessLambdaTag, F, Arg> {
public:
    explicit TaskCallback(Arg& arg) : m_arg(arg) {}

    static void callback(void *data)
    {
        (F{})(*static_cast<Arg *>(data));
    }

    void *callback_data() const
    {
        return const_cast<std::remove_const_t<Arg> *>(&m_arg);
    }

private:
    Arg& m_arg;
};

namespace internal::task_callback {
template <typename F, typename... Args> concept StatelessLambda =
    std::invocable<F, Args...> &&
    std::is_empty_v<F>;
}; // namespace internal::task_callback

template <typename F, typename Arg>
requires internal::task_callback::StatelessLambda<F, Arg&>
inline auto make_task_callback(F, Arg& arg)
{
    return TaskCallback<internal::task_callback::StatelessLambdaTag, F, Arg>{arg};
}


// Stateless lambda not taking an argument:
template <typename F>
struct TaskCallback<internal::task_callback::StatelessLambdaTag, F> {
    static void callback(void *)
    {
        (F{})();
    }

    static void *callback_data() { return nullptr; }
};

template <typename F>
requires internal::task_callback::StatelessLambda<F>
inline auto make_task_callback(F)
{
    return TaskCallback<internal::task_callback::StatelessLambdaTag, F>{};
}


namespace internal::task_callback {
class CapturingLambdaTag {};
};

// Capturing lambda not taking an argument
template <typename F>
class TaskCallback<internal::task_callback::CapturingLambdaTag, F> {
public:
    explicit TaskCallback(F& function) : m_function(function) {}

    static void callback(void *data)
    {
        (*static_cast<F *>(data))();
    }

    void *callback_data() const
    {
        return &m_function;
    }

private:
    F& m_function;
};

template <typename F>
requires
    std::invocable<F> &&
    (!std::is_empty_v<F>)
inline auto make_task_callback(F& f)
{
    return TaskCallback<internal::task_callback::CapturingLambdaTag, F>(f);
}

namespace internal::task_callback {
template <auto F, typename Arg>
class FunctionPointerTag {};
};

// Function pointer without argument
template <auto F>
struct TaskCallback<internal::task_callback::FunctionPointerTag<F, void>> {
    static void callback(void *)
    {
        F();
    }

    static void *callback_data() { return nullptr; }
};

template <auto F>
requires
    std::is_pointer_v<decltype(F)> &&
    std::invocable<decltype(F)>
inline auto make_task_callback()
{
    return TaskCallback<internal::task_callback::FunctionPointerTag<F, void>>{};
}


// Function pointer that takes an argument
template <auto F, typename Arg>
class TaskCallback<internal::task_callback::FunctionPointerTag<F, Arg>> {
public:
    explicit TaskCallback(Arg& arg) : m_data(arg) {}

    static void callback(void *data)
    {
        F(*static_cast<Arg *>(data));
    }

    void *callback_data() const
    {
        return const_cast<std::remove_const_t<Arg> *>(&m_data);
    }

private:
    Arg& m_data;
};

template <auto F, typename Arg>
requires std::is_pointer_v<decltype(F)>
inline auto make_task_callback(Arg& arg)
{
    return TaskCallback<internal::task_callback::FunctionPointerTag<F, Arg>>{arg};
}


// Regular function pointer with void* argument. (As accepted by base FreeRTOS API.)
template <>
class TaskCallback<void (*)(void *), void *> {
public:
    explicit TaskCallback(void (*func)(void *), void *data = nullptr) : callback(func), m_data(data) {}

    void (*callback)(void *);

    void *callback_data() const { return m_data; }

private:
    void *m_data;
};

inline auto make_task_callback(void (*func)(void *), void *data = nullptr)
{
    return TaskCallback<void (*)(void *), void *>{func, data};
}

#if configSUPPORT_DYNAMIC_ALLOCATION
namespace internal::task_callback {
class AllocatedCapturingLambdaTag {};
}; // namespace internal::task_callback

template <typename F>
class TaskCallback<internal::task_callback::AllocatedCapturingLambdaTag, F> {
public:
    explicit TaskCallback(F* function) : m_function(function) {}

    static void callback(void *data)
    {
        (*static_cast<F *>(data))();
    }

    void *callback_data() const { return m_function; }

private:
    F *m_function;
};

// Allocates storage for the callback function on the heap. This technically
// 'leaks' memory, but it doesn't really matter since the task callback will
// last the entire duration.
template <typename F> requires std::invocable<F>
inline auto make_dynamic_task_callback(F&& f)
{
    void *data = pvPortMalloc(sizeof(F));
    configASSERT(data != nullptr);
    return TaskCallback<internal::task_callback::AllocatedCapturingLambdaTag, F>{
        new(data) F(std::forward<F>(f))
    };
}
#endif // configSUPPORT_DYNAMIC_ALLOCATION

}; // namespace freertos

#endif // FREERTOS_TASK_CALLBACK_HPP_INCLUDE

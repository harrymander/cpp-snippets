#ifndef HARRYMANDER_CPP_SNIPPETS_OBSERVABLE_HPP_INCLUDE
#define HARRYMANDER_CPP_SNIPPETS_OBSERVABLE_HPP_INCLUDE

#include <functional>
#include <list>
#include <memory>
#include <utility>

template <typename... Ts> class Observable {
public:
    using Function = std::function<void(Ts...)>;
    using ObserverList = std::list<Function>;

    Observable() : m_observers(std::make_shared<ObserverList>()) {}

    class Observer {
    private:
        using Handle = typename ObserverList::iterator;
        std::weak_ptr<ObserverList> list_ptr;
        Handle handle;

        explicit Observer(const std::shared_ptr<ObserverList>& list, Handle handle) :
            list_ptr(list), handle(handle)
        {}

        friend class Observable;

    public:
        ~Observer()
        {
            std::shared_ptr<ObserverList> list = list_ptr.lock();
            if (list) {
                list->erase(handle);
            }
        }

        Observer(const Observer&) = delete;
        Observer& operator=(const Observer&) = delete;
        Observer(Observer&&) = delete;
        Observer& operator=(Observer&&) = delete;
    };

    [[nodiscard]] typename ObserverList::size_type num_observers() const
    {
        return m_observers->size();
    }

    template <typename... Args> void notify(Args&&...args) const
    {
        for (const auto& observer : *m_observers) {
            observer(args...);
        }
    }

    [[nodiscard]] Observer subscribe(Function function)
    {
        m_observers->emplace_back(std::move(function));
        return Observer(m_observers, std::prev(m_observers->end()));
    }

private:
    std::shared_ptr<ObserverList> m_observers;
};

template <typename... Ts> using Observer = typename Observable<Ts...>::Observer;

#endif // HARRYMANDER_CPP_SNIPPETS_OBSERVABLE_HPP_INCLUDE

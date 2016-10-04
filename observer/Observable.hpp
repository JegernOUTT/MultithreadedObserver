#ifndef MULTITHREADEDOBSERVER_OBSERVABLE_H
#define MULTITHREADEDOBSERVER_OBSERVABLE_H

#include <thread>
#include <mutex>
#include <memory>
#include <chrono>
#include <future>
#include <algorithm>
#include <unordered_map>

#include "Trait.hpp"

namespace observer
{
    template<typename Observer>
    using ObserverTrait = typename std::enable_if<is_observer<Observer>::value>::type;

    using std::unordered_map;
    using std::shared_ptr;
    using std::weak_ptr;
    using std::timed_mutex;
    using std::unique_ptr;
    using std::thread;
    using std::chrono::duration;
    using std::defer_lock;
    using std::future;

    using std::lock_guard;
    using std::unique_lock;
    using std::find_if;
    using std::remove_if;
    using std::forward;
    using std::async;
    using std::result_of;

    enum class AddStatus { Success, Timeout, AlreadyAdded, InvalidPtr };
    enum class RemoveStatus { Success, Timeout, NotFound, InvalidPtr };

    template< typename ContainerT, typename PredicateT >
    void erase_if(ContainerT& items, const PredicateT& predicate)
    {
        for( auto it = items.begin(); it != items.end(); )
        {
            if( predicate(*it) ) it = items.erase(it);
            else ++it;
        }
    };

    template<typename Observer,
             typename Enable = void>
    class Observable
    {
    };

    template<typename Observer>
    class Observable<Observer, ObserverTrait<Observer>>
    {
        using HashType = typename result_of<decltype(&Observer::Hash)(Observer)>::type;
        using ObserverShared = shared_ptr<Observer>;
        using ObserverWeak = weak_ptr<Observer>;
        using CountType = typename unordered_map<HashType, ObserverWeak>::size_type;

    public:
        template<typename _Rep, typename _Period>
        static AddStatus TryAddObserver(ObserverWeak, duration<_Rep, _Period>) noexcept;
        template<typename _Rep, typename _Period>
        static RemoveStatus TryRemoveObserver(ObserverWeak, duration<_Rep, _Period>) noexcept;
        template<typename _Rep, typename _Period>
        static RemoveStatus TryRemoveObserver(HashType, duration<_Rep, _Period>) noexcept;
        template<typename _Rep, typename _Period>
        static RemoveStatus TryRemoveAll(duration<_Rep, _Period>) noexcept;
        template<typename _Rep, typename _Period>
        static RemoveStatus TryRemoveExpired(duration<_Rep, _Period>) noexcept;
        template<typename _Rep, typename _Period, typename... NotifyArguments>
        static void TryNotifyObservers(duration<_Rep, _Period>, NotifyArguments&&...) noexcept;

        static AddStatus AddObserverLocked(ObserverWeak) noexcept;
        static RemoveStatus RemoveObserverLocked(ObserverWeak) noexcept;
        static RemoveStatus RemoveObserverLocked(HashType) noexcept;
        static RemoveStatus RemoveAllLocked() noexcept;
        static RemoveStatus RemoveExpiredLocked() noexcept;
        template<typename... NotifyArguments>
        static void NotifyObserversLocked(NotifyArguments&&...) noexcept;

        template<typename... NotifyArguments>
        static void AsyncNotifyObservers(NotifyArguments&&... args) noexcept;
        template<typename Functional, typename... NotifyArguments>
        static void AsyncNotifyObserversCallback(Functional callback, NotifyArguments&&... args) noexcept;

        static CountType ObserversCount() noexcept;

    private:
        static unordered_map<HashType, ObserverWeak> observers_;
        static timed_mutex obervers_mu_;
    };

    template<typename Observer>
    unordered_map<typename Observable<Observer, ObserverTrait<Observer>>::HashType,
                  typename Observable<Observer, ObserverTrait<Observer>>::ObserverWeak>
            Observable<Observer, ObserverTrait<Observer>>::observers_;

    template<typename Observer>
    timed_mutex Observable<Observer, ObserverTrait<Observer>>::obervers_mu_;


    template<typename Observer>
    template<typename _Rep, typename _Period>
    AddStatus
    Observable<Observer, ObserverTrait<Observer>>::TryAddObserver(typename Observable<Observer, ObserverTrait<Observer>>::ObserverWeak observer,
                                                        duration<_Rep, _Period> timeout) noexcept
    {
        if (observer.expired()) return AddStatus::InvalidPtr;
        if (observers_.count(observer.lock()->Hash()) > 0) return AddStatus::AlreadyAdded;

        unique_lock<timed_mutex> lock(obervers_mu_, defer_lock);
        if (lock.try_lock_for(timeout))
            observers_[observer.lock()->Hash()] = observer;
        else
            return AddStatus::Timeout;

        return AddStatus::Success;
    }

    template<typename Observer>
    template<typename _Rep, typename _Period>
    RemoveStatus
    Observable<Observer, ObserverTrait<Observer>>::TryRemoveObserver(typename Observable<Observer, ObserverTrait<Observer>>::ObserverWeak
                                                            observer,
                                                            duration<_Rep, _Period> timeout) noexcept
    {
        if (observer.expired()) return RemoveStatus::InvalidPtr;
        if (observers_.count(observer.lock()->Hash()) == 0) return RemoveStatus::NotFound;

        unique_lock<timed_mutex> lock(obervers_mu_, defer_lock);
        if (lock.try_lock_for(timeout))
            observers_.erase(observer.lock()->Hash());
        else
            return RemoveStatus::Timeout;

        return RemoveStatus::Success;
    }

    template<typename Observer>
    template<typename _Rep, typename _Period>
    RemoveStatus
    Observable<Observer, ObserverTrait<Observer>>::TryRemoveObserver(HashType observer_hash,
                                                           duration<_Rep, _Period> timeout) noexcept
    {
        if (observers_.count(observer_hash) == 0) return RemoveStatus::NotFound;

        unique_lock<timed_mutex> lock(obervers_mu_, defer_lock);
        if (lock.try_lock_for(timeout))
            observers_.erase(observer_hash);
        else
            return RemoveStatus::Timeout;

        return RemoveStatus::Success;
    }

    template<typename Observer>
    template<typename _Rep, typename _Period>
    RemoveStatus
    Observable<Observer, ObserverTrait<Observer>>::TryRemoveAll(duration<_Rep, _Period> timeout) noexcept
    {
        unique_lock<timed_mutex> lock(obervers_mu_, defer_lock);
        if (lock.try_lock_for(timeout))
            observers_.clear();
        else
            return RemoveStatus::Timeout;

        return RemoveStatus::Success;
    };

    template<typename Observer>
    template<typename _Rep, typename _Period>
    RemoveStatus
    Observable<Observer, ObserverTrait<Observer>>::TryRemoveExpired(duration<_Rep, _Period> timeout) noexcept
    {
        unique_lock<timed_mutex> lock(obervers_mu_, defer_lock);
        if (lock.try_lock_for(timeout))
            erase_if(observers_, [](const auto& element) { return element.second.expired(); });
        else
            return RemoveStatus::Timeout;

        return RemoveStatus::Success;
    }

    template<typename Observer>
    template<typename _Rep, typename _Period, typename... NotifyArguments>
    void
    Observable<Observer, ObserverTrait<Observer>>::TryNotifyObservers(duration<_Rep, _Period> timeout,
                                                             NotifyArguments&&... args) noexcept
    {
        unique_lock<timed_mutex> lock(obervers_mu_, defer_lock);
        if (lock.try_lock_for(timeout))
        {
            for (const auto& observer: observers_)
            {
                if (!observer.second.expired())
                    observer.second.lock()->HandleEvent(forward<NotifyArguments>(args)...);
            }
        }
    };

    template<typename Observer>
    AddStatus
    Observable<Observer, ObserverTrait<Observer>>::AddObserverLocked(typename Observable<Observer, ObserverTrait<Observer>>::ObserverWeak
                                                           observer) noexcept
    {
        if (observer.expired()) return AddStatus::InvalidPtr;
        if (observers_.count(observer.lock()->Hash()) > 0) return AddStatus::AlreadyAdded;

        {
            lock_guard<timed_mutex> lock(obervers_mu_);
            observers_[observer.lock()->Hash()] = observer;
        }

        return AddStatus::Success;
    }

    template<typename Observer>
    RemoveStatus
    Observable<Observer, ObserverTrait<Observer>>::RemoveObserverLocked(typename Observable<Observer, ObserverTrait<Observer>>::ObserverWeak
                                                              observer) noexcept
    {
        if (observer.expired()) return RemoveStatus::InvalidPtr;
        if (observers_.count(observer.lock()->Hash()) == 0) return RemoveStatus::NotFound;

        {
            lock_guard<timed_mutex> lock(obervers_mu_);
            observers_.erase(observer.lock()->Hash());
        }

        return RemoveStatus::Success;
    }

    template<typename Observer>
    RemoveStatus
    Observable<Observer, ObserverTrait<Observer>>::RemoveObserverLocked(HashType observer_hash) noexcept
    {
        if (observers_.count(observer_hash) == 0) return RemoveStatus::NotFound;

        {
            lock_guard<timed_mutex> lock(obervers_mu_);
            observers_.erase(observer_hash);
        }

        return RemoveStatus::Success;
    }

    template<typename Observer>
    RemoveStatus
    Observable<Observer, ObserverTrait<Observer>>::RemoveAllLocked() noexcept
    {
        {
            lock_guard<timed_mutex> lock(obervers_mu_);
            observers_.clear();
        }

        return RemoveStatus::Success;
    };

    template<typename Observer>
    RemoveStatus
    Observable<Observer, ObserverTrait<Observer>>::RemoveExpiredLocked() noexcept
    {
        {
            erase_if(observers_, [](const auto& element) { return element.second.expired(); });
            observers_.clear();
        }

        return RemoveStatus::Success;
    }

    template<typename Observer>
    template<typename... NotifyArguments>
    void
    Observable<Observer, ObserverTrait<Observer>>::NotifyObserversLocked(NotifyArguments&&... args) noexcept
    {
        lock_guard<timed_mutex> lock(obervers_mu_);
        for (const auto& observer: observers_)
        {
            if (!observer.second.expired())
                observer.second.lock()->HandleEvent(forward<NotifyArguments>(args)...);
        }
    }

    template<typename Observer>
    template<typename... NotifyArguments>
    void
    Observable<Observer, ObserverTrait<Observer>>::AsyncNotifyObservers(NotifyArguments&&... args) noexcept
    {
        decltype(observers_) observers_copy;
        {
            lock_guard<timed_mutex> lock(obervers_mu_);
            observers_copy = observers_;
        }

        thread{[observers_copy](auto&&... args){
            for (const auto& observer: observers_copy)
            {
                if (!observer.second.expired())
                    observer.second.lock()->HandleEvent(args...);
            }
        }, forward<NotifyArguments>(args)...}.detach();
    }

    template<typename Observer>
    template<typename Functional, typename... NotifyArguments>
    void
    Observable<Observer, ObserverTrait<Observer>>::AsyncNotifyObserversCallback(Functional callback,
                                                                                NotifyArguments&&... args) noexcept
    {
        decltype(observers_) observers_copy;
        {
            lock_guard<timed_mutex> lock(obervers_mu_);
            observers_copy = observers_;
        }

        thread{[observers_copy, callback](auto&&... args){
            for (const auto& observer: observers_copy)
            {
                if (!observer.second.expired())
                    observer.second.lock()->HandleEvent(args...);
            }
            callback();
        }, forward<NotifyArguments>(args)...}.detach();
    };

    template<typename Observer>
    typename Observable<Observer, ObserverTrait<Observer>>::CountType
    Observable<Observer, ObserverTrait<Observer>>::ObserversCount() noexcept
    {
        return observers_.size();
    }
}

#endif //MULTITHREADEDOBSERVER_OBSERVABLE_H

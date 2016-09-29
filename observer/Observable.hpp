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

    template<typename Observer,
             typename Enable = void>
    class Observable
    {

    };

    template<typename Observer>
    class Observable<Observer, ObserverTrait>
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

        static CountType ObserversCount() noexcept;

    private:
        static unordered_map<HashType, ObserverWeak> observers_;
        static timed_mutex obervers_mu_;
    };

    template<typename Observer>
    unordered_map<typename Observable<Observer, ObserverTrait>::HashType,
                  typename Observable<Observer, ObserverTrait>::ObserverWeak>
            Observable<Observer, ObserverTrait>::observers_;

    template<typename Observer>
    timed_mutex Observable<Observer, ObserverTrait>::obervers_mu_;


    template<typename Observer>
    template<typename _Rep, typename _Period>
    AddStatus
    Observable<Observer, ObserverTrait>::TryAddObserver(typename Observable<Observer, ObserverTrait>::ObserverWeak observer,
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
    Observable<Observer, ObserverTrait>::TryRemoveObserver(typename Observable<Observer, ObserverTrait>::ObserverWeak
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
    Observable<Observer, ObserverTrait>::TryRemoveObserver(HashType observer_hash,
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
    Observable<Observer, ObserverTrait>::TryRemoveAll(duration<_Rep, _Period> timeout) noexcept
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
    Observable<Observer, ObserverTrait>::TryRemoveExpired(duration<_Rep, _Period> timeout) noexcept
    {
        unique_lock<timed_mutex> lock(obervers_mu_, defer_lock);
        if (lock.try_lock_for(timeout))
            remove_if(observers_.begin(), observers_.end(), [](const auto& element) { return element.second.expired(); });
        else
            return RemoveStatus::Timeout;

        return RemoveStatus::Success;
    }

    template<typename Observer>
    template<typename _Rep, typename _Period, typename... NotifyArguments>
    void
    Observable<Observer, ObserverTrait>::TryNotifyObservers(duration<_Rep, _Period> timeout,
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
    Observable<Observer, ObserverTrait>::AddObserverLocked(typename Observable<Observer, ObserverTrait>::ObserverWeak
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
    Observable<Observer, ObserverTrait>::RemoveObserverLocked(typename Observable<Observer, ObserverTrait>::ObserverWeak
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
    Observable<Observer, ObserverTrait>::RemoveObserverLocked(HashType observer_hash) noexcept
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
    Observable<Observer, ObserverTrait>::RemoveAllLocked() noexcept
    {
        {
            lock_guard<timed_mutex> lock(obervers_mu_);
            observers_.clear();
        }

        return RemoveStatus::Success;
    };

    template<typename Observer>
    RemoveStatus
    Observable<Observer, ObserverTrait>::RemoveExpiredLocked() noexcept
    {
        {
            remove_if(observers_.begin(), observers_.end(), [](const auto& element) { return element.second.expired(); });
            observers_.clear();
        }

        return RemoveStatus::Success;
    }

    template<typename Observer>
    template<typename... NotifyArguments>
    void
    Observable<Observer, ObserverTrait>::NotifyObserversLocked(NotifyArguments&&... args) noexcept
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
    Observable<Observer, ObserverTrait>::AsyncNotifyObservers(NotifyArguments&&... args) noexcept
    {
        decltype(observers_) observers_copy;
        {
            lock_guard<timed_mutex> lock(obervers_mu_);
            observers_copy = observers_;
        }

        thread{[observers_copy](){
            for (const auto& observer: observers_copy)
            {
                if (!observer.second.expired())
                    async(std::launch::async, [&](auto&&... args){
                        observer.second.lock()->HandleEvent(forward<NotifyArguments>(args)...);
                    });
            }
        }, forward<NotifyArguments>(args)...}.detach();
    }

    template<typename Observer>
    typename Observable<Observer, ObserverTrait>::CountType
    Observable<Observer, ObserverTrait>::ObserversCount() noexcept
    {
        return observers_.size();
    }
}

#endif //MULTITHREADEDOBSERVER_OBSERVABLE_H

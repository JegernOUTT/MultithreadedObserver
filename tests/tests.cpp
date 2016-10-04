#include <iostream>
#include <random>
#include <chrono>
#include <memory>
#include <list>

#include <bandit/bandit.h>
#include "observer_mock.hpp"

#include "../observer/Observable.hpp"


namespace observertest
{
    inline int32_t RandomValue(int32_t min, int32_t max)
    {
        std::default_random_engine generator(static_cast<uint64_t>(high_resolution_clock::now()
                                                                                        .time_since_epoch()
                                                                                        .count()));
        std::uniform_int_distribution<int32_t> distribution(min, max);
        return distribution(generator);
    }

    go_bandit([]()
              {
                  using namespace bandit;
                  using namespace std::chrono;

                  using std::list;
                  using std::shared_ptr;
                  using std::weak_ptr;

                  using observer::Observable;
                  using observer::AddStatus;
                  using observer::RemoveStatus;

                  using std::make_shared;

                  describe("Observer:", []()
                  {
                      list<shared_ptr<Observer_1>> observers;
                      for (int i = 0; i < 100; ++i) observers.emplace_back(make_shared<Observer_1>());

                      it("TryAddObserver with Observer_1", [&]()
                      {
                          using ObserverWeak = std::weak_ptr<Observer_1>;

                          for (const auto& element: observers)
                          {
                              auto result = Observable<Observer_1>::TryAddObserver(ObserverWeak{element},
                                                                                   5s);
                              AssertThat(result, Equals(AddStatus::Success));
                          }
                          AssertThat(Observable<Observer_1>::ObserversCount(), Equals(observers.size()));
                      });

                      it("TryRemoveObservers(object) with Observer_1", [&]()
                      {
                          using ObserverWeak = std::weak_ptr<Observer_1>;
                          AssertThat(Observable<Observer_1>::ObserversCount(), Equals(observers.size()));
                          for (const auto& element: observers)
                          {
                              auto result = Observable<Observer_1>::TryRemoveObserver(ObserverWeak{element},
                                                                                      5s);
                              AssertThat(result, Equals(RemoveStatus::Success));
                          }
                          AssertThat(Observable<Observer_1>::ObserversCount(), Equals(0));
                      });

                      it("TryRemoveObservers(hash) with Observer_1", [&]()
                      {
                          using ObserverWeak = std::weak_ptr<Observer_1>;
                          for (const auto& element: observers)
                          {
                              auto result = Observable<Observer_1>::TryAddObserver(ObserverWeak{element},
                                                                                   5s);
                              AssertThat(result, Equals(AddStatus::Success));
                          }
                          AssertThat(Observable<Observer_1>::ObserversCount(), Equals(observers.size()));
                          for (const auto& element: observers)
                          {
                              auto result = Observable<Observer_1>::TryRemoveObserver(element->Hash(),
                                                                                      5s);
                              AssertThat(result, Equals(RemoveStatus::Success));
                          }
                          AssertThat(Observable<Observer_1>::ObserversCount(), Equals(0));
                      });

                      it("TryRemoveAll with Observer_1", [&]()
                      {
                          using ObserverWeak = std::weak_ptr<Observer_1>;
                          for (const auto& element: observers)
                          {
                              auto result = Observable<Observer_1>::TryAddObserver(ObserverWeak{element},
                                                                                   5s);
                              AssertThat(result, Equals(AddStatus::Success));
                          }
                          AssertThat(Observable<Observer_1>::ObserversCount(), Equals(observers.size()));

                          auto result = Observable<Observer_1>::TryRemoveAll(5s);
                          AssertThat(result, Equals(RemoveStatus::Success));
                          AssertThat(Observable<Observer_1>::ObserversCount(), Equals(0));
                      });

                      it("TryRemoveExpired with Observer_1", [&]()
                      {
                          using ObserverWeak = std::weak_ptr<Observer_1>;
                          for (const auto& i: {0, 1, 2, 3, 4, 5, 6, 7, 8, 9})
                          {
                              auto result = Observable<Observer_1>::TryAddObserver(ObserverWeak{make_shared<Observer_1>()},
                                                                                   5s);
                              AssertThat(result, Equals(AddStatus::Success));
                          }
                          AssertThat(Observable<Observer_1>::ObserversCount(), Equals(10));
                          auto result = Observable<Observer_1>::TryRemoveExpired(5s);
                          AssertThat(result, Equals(RemoveStatus::Success));
                          AssertThat(Observable<Observer_1>::ObserversCount(), Equals(0));
                      });

                      it("TryNotifyObservers with Observer_1", [&]()
                      {
                          using ObserverWeak = std::weak_ptr<Observer_1>;
                          for (const auto& element: observers)
                          {
                              auto result = Observable<Observer_1>::TryAddObserver(ObserverWeak{element},
                                                                                   5s);
                              AssertThat(result, Equals(AddStatus::Success));
                          }
                          AssertThat(Observable<Observer_1>::ObserversCount(), Equals(observers.size()));

                          Observable<Observer_1>::TryNotifyObservers(5s, "Hello", 49);
                          for (const auto& observer: observers)
                          {
                              AssertThat(get<0>(observer->val), Equals("Hello"));
                              AssertThat(get<1>(observer->val), Equals(49));
                          }
                          Observable<Observer_1>::TryRemoveAll(5s);
                          AssertThat(Observable<Observer_1>::ObserversCount(), Equals(0));
                      });

                      it("AddObserverLocked with Observer_1", [&]()
                      {
                          using ObserverWeak = std::weak_ptr<Observer_1>;

                          for (const auto& element: observers)
                          {
                              auto result = Observable<Observer_1>::AddObserverLocked(ObserverWeak{element});
                              AssertThat(result, Equals(AddStatus::Success));
                          }
                          AssertThat(Observable<Observer_1>::ObserversCount(), Equals(observers.size()));
                      });

                      it("RemoveObserverLocked(object) with Observer_1", [&]()
                      {
                          using ObserverWeak = std::weak_ptr<Observer_1>;
                          AssertThat(Observable<Observer_1>::ObserversCount(), Equals(observers.size()));
                          for (const auto& element: observers)
                          {
                              auto result = Observable<Observer_1>::RemoveObserverLocked(ObserverWeak{element});
                              AssertThat(result, Equals(RemoveStatus::Success));
                          }
                          AssertThat(Observable<Observer_1>::ObserversCount(), Equals(0));
                      });

                      it("RemoveObserverLocked(hash) with Observer_1", [&]()
                      {
                          using ObserverWeak = std::weak_ptr<Observer_1>;
                          for (const auto& element: observers)
                          {
                              auto result = Observable<Observer_1>::AddObserverLocked(ObserverWeak{element});
                              AssertThat(result, Equals(AddStatus::Success));
                          }
                          AssertThat(Observable<Observer_1>::ObserversCount(), Equals(observers.size()));
                          for (const auto& element: observers)
                          {
                              auto result = Observable<Observer_1>::RemoveObserverLocked(element->Hash());
                              AssertThat(result, Equals(RemoveStatus::Success));
                          }
                          AssertThat(Observable<Observer_1>::ObserversCount(), Equals(0));
                      });

                      it("RemoveAllLocked with Observer_1", [&]()
                      {
                          using ObserverWeak = std::weak_ptr<Observer_1>;
                          for (const auto& element: observers)
                          {
                              auto result = Observable<Observer_1>::AddObserverLocked(ObserverWeak{element});
                              AssertThat(result, Equals(AddStatus::Success));
                          }
                          AssertThat(Observable<Observer_1>::ObserversCount(), Equals(observers.size()));

                          auto result = Observable<Observer_1>::RemoveAllLocked();
                          AssertThat(result, Equals(RemoveStatus::Success));
                          AssertThat(Observable<Observer_1>::ObserversCount(), Equals(0));
                      });

                      it("RemoveExpiredLocked with Observer_1", [&]()
                      {
                          using ObserverWeak = std::weak_ptr<Observer_1>;
                          for (const auto& i: {0, 1, 2, 3, 4, 5, 6, 7, 8, 9})
                          {
                              auto result = Observable<Observer_1>::AddObserverLocked(
                                      ObserverWeak{make_shared<Observer_1>()});
                              AssertThat(result, Equals(AddStatus::Success));
                          }
                          AssertThat(Observable<Observer_1>::ObserversCount(), Equals(10));
                          auto result = Observable<Observer_1>::RemoveExpiredLocked();
                          AssertThat(result, Equals(RemoveStatus::Success));
                          AssertThat(Observable<Observer_1>::ObserversCount(), Equals(0));
                      });

                      it("NotifyObserversLocked with Observer_1", [&]()
                      {
                          using ObserverWeak = std::weak_ptr<Observer_1>;
                          for (const auto& element: observers)
                          {
                              auto result = Observable<Observer_1>::AddObserverLocked(ObserverWeak{element});
                              AssertThat(result, Equals(AddStatus::Success));
                          }
                          AssertThat(Observable<Observer_1>::ObserversCount(), Equals(observers.size()));

                          Observable<Observer_1>::NotifyObserversLocked("Hello", 49);
                          for (const auto& observer: observers)
                          {
                              AssertThat(get<0>(observer->val), Equals("Hello"));
                              AssertThat(get<1>(observer->val), Equals(49));
                          }
                      });

                      it("AsyncNotifyObservers with Observer_1", [&]()
                      {
                          AssertThat(Observable<Observer_1>::ObserversCount(), Equals(observers.size()));

                          uint16_t repeats = 0;
                          while (repeats++ < 100)
                          {
                              auto int_val = RandomValue(0, 0xFFFF);
                              Observable<Observer_1>::AsyncNotifyObservers("Hello", int_val);

//                            Waiting a little because async is detaching extra thread
                              std::this_thread::sleep_for(20ms);

                              for (const auto& observer: observers)
                              {
                                  AssertThat(get<0>(observer->val), Equals("Hello"));
                                  AssertThat(get<1>(observer->val), Equals(int_val));
                              }
                          }
                      });

                      it("AsyncNotifyObserversCallback with Observer_1", [&]()
                      {
                          AssertThat(Observable<Observer_1>::ObserversCount(), Equals(observers.size()));

                          auto int_val = RandomValue(0, 0xFFFF);
                          Observable<Observer_1>::AsyncNotifyObserversCallback([observers, int_val](){
                               for (const auto& observer: observers)
                               {
                                   AssertThat(get<0>(observer->val), Equals("Hello"));
                                   AssertThat(get<1>(observer->val), Equals(int_val));
                               }}, "Hello", int_val);
                          std::this_thread::sleep_for(1s);
                      });
                  });
              });
}


int main(int argc, char ** argv)
{
    return bandit::run(argc, argv);
}
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
                          using ObservableSpec = Observable<Observer_1>;

                          for (const auto& element: observers)
                          {
                              auto result = ObservableSpec::TryAddObserver(ObserverWeak{element}, 5s);
                              AssertThat(result, Equals(AddStatus::Success));
                          }
                          AssertThat(Observable<Observer_1>::ObserversCount(), Equals(observers.size()));
                      });

                      it("TryRemoveObservers with Observer_1", [&]()
                      {
                          using ObserverWeak = std::weak_ptr<Observer_1>;
                          using ObservableSpec = Observable<Observer_1>;

                          AssertThat(Observable<Observer_1>::ObserversCount(), Equals(observers.size()));

                          for (const auto& element: observers)
                          {
                              auto result = ObservableSpec::TryRemoveObserver(ObserverWeak{element}, 5s);
                              AssertThat(result, Equals(RemoveStatus::Success));
                          }
                          AssertThat(Observable<Observer_1>::ObserversCount(), Equals(0));
                      });

                  });
              });
}


int main(int argc, char ** argv)
{
    return bandit::run(argc, argv);
}
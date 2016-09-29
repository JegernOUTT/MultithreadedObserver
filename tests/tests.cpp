#include <iostream>
#include <random>
#include <chrono>
#include <memory>

#include <bandit/bandit.h>
#include "observer_mock.hpp"

#include "../observer/Observable.hpp"

go_bandit([](){
    using namespace bandit;
    using namespace std::chrono;

    describe("Observer:", [](){
        std::shared_ptr<Observer> o1;
        std::shared_ptr<Observer> o2;
        std::shared_ptr<Observer2> o3;

        before_each([&](){
            o1 = std::make_shared<Observer>();
            o2 = std::make_shared<Observer>();
            o3 = std::make_shared<Observer2>();
        });

        it("Add to observerable with Observer", [&](){
            using ObserverWeak = std::weak_ptr<Observer>;
            AssertThat(observer::Observable<Observer>::
                           AddObserverLocked(ObserverWeak{ o1->shared_from_this() }),
                       Equals(observer::AddStatus::Success));
            AssertThat(observer::Observable<Observer>::
                           AddObserverLocked(ObserverWeak{ o2->shared_from_this() }),
                       Equals(observer::AddStatus::Success));
        });
        it("Check observers lenght", [&](){
            AssertThat(observer::Observable<Observer>::ObserversCount(),
                       Equals(2));
        });

        it("Add to observerable with Observer2", [&](){
            using Observer2Weak = std::weak_ptr<Observer2>;
            AssertThat(observer::Observable<Observer2>::
                           AddObserverLocked(Observer2Weak{ o3->shared_from_this() }),
                       Equals(observer::AddStatus::Success));
        });
        it("Check observers2 lenght", [&](){
            AssertThat(observer::Observable<Observer2>::ObserversCount(),
                       Equals(1));
        });
    });
});


int main(int argc, char ** argv)
{
    return bandit::run(argc, argv);
}
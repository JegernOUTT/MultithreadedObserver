#ifndef MULTITHREADEDOBSERVER_OBSERVER_MOCK_HPP
#define MULTITHREADEDOBSERVER_OBSERVER_MOCK_HPP

#include <random>
#include <chrono>
#include <tuple>
#include <string>

namespace observertest
{
    using namespace std::chrono;
    using std::tuple;
    using std::string;
    using std::default_random_engine;
    using std::uniform_real_distribution;
    using std::uniform_int_distribution;

    using std::make_tuple;
    using std::to_string;
    using std::forward;
    using std::get;

    struct Observer_1 {
        Observer_1()
        {

            default_random_engine generator(
                    static_cast<uint64_t>(high_resolution_clock::now().time_since_epoch().count()));
            uniform_real_distribution<float> distribution(0.f, 1000000.f);
            hash_ =  distribution(generator);
        }

        float Hash()
        {
            return hash_;
        }

        template<typename... t>
        void HandleEvent(t&&... args){
            auto tup = make_tuple(forward<t>(args)...);
        }

    private:
        float hash_;
    };


    struct Observer_2 {
        uint64_t Hash()
        {
            default_random_engine generator(
                    static_cast<uint64_t>(high_resolution_clock::now().time_since_epoch().count()));
            uniform_int_distribution<uint64_t> distribution(0, 0xffffff);
            return distribution(generator);
        }

        void HandleEvent(string){}
    };


    struct Observer_3 {
        int Hash()
        {
            return 10;
        }

        template<typename... t>
        void HandleEvent(t&&...){}
    };


    struct Observer_4 {
        string Hash()
        {
            default_random_engine generator(
                    static_cast<uint64_t>(high_resolution_clock::now().time_since_epoch().count()));
            uniform_int_distribution<uint64_t> distribution(0, 0xffffff);
            return "hash " + to_string(distribution(generator));
        }

        void HandleEvent(int){}
    };


    struct Observer_5 {
        uint64_t Hash()
        {
            default_random_engine generator(
                    static_cast<uint64_t>(high_resolution_clock::now().time_since_epoch().count()));
            uniform_int_distribution<uint64_t> distribution(0, 0xffffff);
            return distribution(generator);
        }

        template<typename func, typename... args>
        void HandleEvent(func, args&&...) {}
    };
}

#endif //MULTITHREADEDOBSERVER_OBSERVER_MOCK_HPP

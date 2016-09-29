#ifndef MULTITHREADEDOBSERVER_OBSERVER_MOCK_HPP
#define MULTITHREADEDOBSERVER_OBSERVER_MOCK_HPP


struct Observer : public std::enable_shared_from_this<Observer> {
    uint64_t Hash()
    {
        std::default_random_engine generator(
                (uint64_t) std::chrono::high_resolution_clock::now().time_since_epoch().count());
        std::uniform_int_distribution<uint64_t> distribution(0, 0xffffff);
        return distribution(generator);
    }

    template<typename... t>
    void HandleEvent(t...){}
};

struct Observer2 : public std::enable_shared_from_this<Observer2> {
    uint64_t Hash()
    {
        std::default_random_engine generator(
                (uint64_t) std::chrono::high_resolution_clock::now().time_since_epoch().count());
        std::uniform_int_distribution<uint64_t> distribution(0, 0xffffff);
        return distribution(generator);
    }

    template<typename... t>
    void HandleEvent(t...){}
};

#endif //MULTITHREADEDOBSERVER_OBSERVER_MOCK_HPP

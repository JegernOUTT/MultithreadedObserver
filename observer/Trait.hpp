#ifndef MULTITHREADEDOBSERVER_SFINAEOBSERVERTEST_H
#define MULTITHREADEDOBSERVER_SFINAEOBSERVERTEST_H

#include <type_traits>

namespace observer
{
    using std::true_type;
    using std::false_type;
    using std::is_same;

    template<typename T>
    struct is_observer
    {
    private:
        static auto detect_hash(...)->false_type;
        template<typename U> static auto detect_hash(U * p) -> decltype(p->Hash());

        static auto detect_handleevet(...)->false_type;
        template<typename U> static auto detect_handleevet(U * p) -> decltype(p->HandleEvent());
    public:
        static constexpr bool value = !is_same<false_type, decltype(detect_hash(static_cast<T*>(nullptr)))>::value &&
                                      !is_same<false_type, decltype(detect_handleevet(static_cast<T*>(nullptr)))>::value;
    };
}

#endif //MULTITHREADEDOBSERVER_SFINAEOBSERVERTEST_H

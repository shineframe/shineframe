#pragma once
#include <iostream>
#include <type_traits>
#include "../common/define.hpp"

#define SHINE_CHECK_MEMBER(member) \
template<typename T>\
struct check_##member{\
    template <typename _T>\
    static auto check(_T)->typename std::decay<decltype(_T::member)>::type;\
    static void check(...);\
    using type = decltype(check(std::declval<T>()));\
    enum{ value = !std::is_void<type>::value };\
};

namespace shine
{
}

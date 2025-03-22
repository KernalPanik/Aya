#pragma once

#include <string>
#include <iostream>
#include <sstream>
#include <tuple>
#include <vector>

/*
Unexplored caveat: std::string() operator overrides for more complex data types
*/

// TODO: Explore making this a template class

template <typename T, typename = void>
struct can_ostream : std::false_type {};

template <typename T>
struct can_ostream<T, std::void_t<decltype(std::declval<std::ostream&>() << std::declval<T>())>> : std::true_type {};

template <typename T>
inline constexpr bool can_ostream_v = can_ostream<T>::value;

// Internal
template<class Tuple, size_t N>
struct TupleStringify {
    static void UpdateBaseString(const Tuple& t, std::string& base) {
        //base += std::get<N-1>(t);
        TupleStringify<Tuple, N-1>::UpdateBaseString(t, base);

        // TODO: SFINAE check for the existence of ToString() function in a type.
        // If there is no such function, fall back to printing Some_<DATA_TYPE>
        std::stringstream s;

        if constexpr (can_ostream_v<decltype(std::get<N-1>(t))>) {
            s << std::get<N-1>(t);
        }
        s << "; ";
        base += s.str();
    }
};

// Internal
template<class Tuple>
struct TupleStringify<Tuple, 1> {
    static void UpdateBaseString(const Tuple& t, std::string& base) {
        std::stringstream s;
        s << std::get<0>(t);
        s << "; ";
        base += s.str();
    }
};

/*
    Tuple Util API: ToString()
    Input: Tuple
    Output: Tuple Elements as std::string
    NOTE:
*/
template<class... Args>
std::string TupleToString(std::tuple<Args...>& t) {
    std::string base("[");
    TupleStringify<decltype(t), sizeof...(Args)>::UpdateBaseString(t, base);
    base += "]";

    return base;
}

/*
    Returns decayed tuple -- Tuple where each element is value, not reference.
    Useful when Invoking TestableFunction via it's interface -- state is a tuple of references, not values.
*/
template<class... types>
auto TupleDecay(const std::tuple<types...>& t) {
    return std::apply([](auto&&... args) {
        return std::make_tuple(std::decay_t<decltype(args)>(std::forward<decltype(args)>(args))...);
    }, t);
}

template<typename Tuple1, typename Tuple2>
bool CompareTuples(const Tuple1& t1, const Tuple2& t2) {
    static_assert(std::tuple_size<Tuple1>::value == std::tuple_size<Tuple2>::value, "Tuple sizes must match");
    return TupleDecay(t1) == TupleDecay(t2);
}

template <typename Tuple, std::size_t... Is>
std::vector<void*> TupleVecImpl(Tuple&& tuple, std::index_sequence<Is...>) {
    std::vector<void*> vec;
    vec.reserve(sizeof...(Is));
    (vec.push_back(&std::get<Is>(std::forward<Tuple>(tuple))), ...);

    return vec;
}

template <typename Tuple>
std::vector<void*> TupleVec(Tuple&& tuple) {
    return TupleVecImpl(std::forward<Tuple>(tuple), std::make_index_sequence<std::tuple_size<std::decay_t<Tuple>>::value>());
}

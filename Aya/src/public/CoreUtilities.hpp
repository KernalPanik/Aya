#pragma once

#include <string>
#include <iostream>
#include <sstream>
#include <tuple>
#include <vector>

namespace Aya {
    template<typename T, typename = void>
    struct can_ostream : std::false_type {
    };

    template<typename T>
    struct can_ostream<T, std::void_t<decltype(std::declval<std::ostream &>() << std::declval<T>())>>
        : std::true_type {
    };

    template<typename T>
    inline constexpr bool can_ostream_v = can_ostream<T>::value;

    // Internal
    template<class Tuple, size_t N>
    struct TupleStringify {
        static void UpdateBaseString(const Tuple &t, std::string &base) {
            //base += std::get<N-1>(t);
            TupleStringify<Tuple, N - 1>::UpdateBaseString(t, base);

            std::stringstream s;
            if constexpr (can_ostream_v<decltype(std::get<N - 1>(t))>) {
                s << std::get<N - 1>(t);
            }

            s << ", ";
            base += s.str();
        }
    };

    // Internal
    template<class Tuple>
    struct TupleStringify<Tuple, 1> {
        static void UpdateBaseString(const Tuple &t, std::string &base) {
            std::stringstream s;
            s << std::get<0>(t);
            s << ", ";
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
    std::string TupleToString(std::tuple<Args...> &t) {
        std::string base;
        TupleStringify<decltype(t), sizeof...(Args)>::UpdateBaseString(t, base);
        base += "";

        return base;
    }

    /*
        Returns decayed tuple -- Tuple where each element is value, not reference.
        Useful when Invoking TestableFunction via it's interface -- state is a tuple of references, not values.
    */
    template<class... types>
    auto TupleDecay(const std::tuple<types...> &t) {
        return std::apply([](auto &&... args) {
            return std::make_tuple(std::decay_t<decltype(args)>(std::forward<decltype(args)>(args))...);
        }, t);
    }

    template<typename Tuple1, typename Tuple2>
    bool CompareTuples(const Tuple1 &t1, const Tuple2 &t2) {
        static_assert(std::tuple_size_v<Tuple1> == std::tuple_size_v<Tuple2>, "Tuple sizes must match");
        return TupleDecay(t1) == TupleDecay(t2);
    }

    template<typename Tuple, std::size_t... Is>
    std::vector<void *> TupleVecImpl(Tuple &&tuple, std::index_sequence<Is...>) {
        std::vector<void *> vec;
        vec.reserve(sizeof...(Is));
        (vec.push_back(&std::get<Is>(std::forward<Tuple>(tuple))), ...);

        return vec;
    }

    template<typename Tuple>
    std::vector<void *> TupleVec(Tuple &&tuple) {
        return TupleVecImpl(std::forward<Tuple>(tuple),
                            std::make_index_sequence<std::tuple_size_v<std::decay_t<Tuple>>>());
    }

    template<typename T>
    std::vector<T> VecFlat(std::vector<std::vector<T>> vecs) {
        std::vector<T> result;
        for (auto &v: vecs) {
            result.insert(result.end(), v.begin(), v.end());
        }

        return result;
    }

    template<typename... Args, std::size_t... I>
    std::tuple<Args...> ConstructTupleFromVec(const std::vector<std::any> &v, std::index_sequence<I...>) {
        return std::make_tuple(std::any_cast<Args>(v[I])...);
    }

    template<typename... Args>
    std::tuple<Args...> Tuplify(std::vector<std::any> v) {
        if (v.size() != sizeof...(Args)) {
            throw std::invalid_argument("Vector size doesn't match tuple type count");
        }
        return ConstructTupleFromVec<Args...>(v, std::index_sequence_for<Args...>{});
    }

    template<typename T, typename U, typename... Args>
    std::vector<std::any> CaptureProducedState(std::function<T(Args...)> func, const std::vector<std::any> &inputs) {
        std::vector<std::any> producedState = inputs;
        if constexpr (std::is_void_v<T>) {
            std::apply(func, Tuplify<Args...>(producedState));
        } else {
            U returnValue = std::apply(func, Tuplify<Args...>(producedState));
            producedState.insert(producedState.begin(), returnValue);
        }

        return producedState;
    }
}

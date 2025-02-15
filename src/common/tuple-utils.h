#pragma once

#include <string>
#include <iostream>
#include <sstream>
#include <tuple>

/*
Unexplored caveat: std::string() operator overrides for more complex data types
*/

// Internal
template<class Tuple, size_t N>
struct TupleStringify {
    static void UpdateBaseString(const Tuple& t, std::string& base) {
        //base += std::get<N-1>(t);
        TupleStringify<Tuple, N-1>::UpdateBaseString(t, base);

        std::stringstream s;
        s << std::get<N-1>(t);
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
*/
template<class... Args>
std::string TupleToString(std::tuple<Args...>& t) {
    std::string base("[");
    TupleStringify<decltype(t), sizeof...(Args)>::UpdateBaseString(t, base);
    base += "]";

    return base;
}
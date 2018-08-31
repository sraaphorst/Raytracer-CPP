/**
 * transformers.h
 *
 * By Sebastian Raaphorst, 2018.
 */

#pragma once

#include <array>
#include <functional>
#include <utility>

#include "common.h"

namespace raytracer::transformers {
    namespace details {
        /// Reduce two arrays by applying a function for each index and combining the terms.
        template<class T, class R, size_t N, size_t k>
        struct ReducerAux {
            static constexpr inline R result(std::function<R(T, T)> f, std::function<R(R, R)> r, R defaultval, const std::array<T, N> &a, const std::array<T, N> &b) {
                return r(f(a[k-1], b[k-1]), ReducerAux<T, R, N, k - 1>::result(f, r, defaultval, a, b));
            }
        };

        template<class T, class R, size_t N>
        struct ReducerAux<T, R, N, 0> {
            static constexpr inline R result(std::function<R(T, T)> f, std::function<R(R, R)> r, R defaultval, const std::array<T, N>&, const std::array<T, N>&) {
                return defaultval;
            }
        };

        template<class T, unsigned long int N, size_t... Indices>
        constexpr std::array<T, N> indextransform_helper(std::function<T(int)> f, std::index_sequence<Indices...>) {
            return {{ f(Indices)... }};
        }

        template<class R, class T, unsigned long int N, size_t... Indices>
        constexpr std::array<T, N> unitransform_helper(std::function<R(const T&)> f, const std::array<T, N> &a,
                                                       std::index_sequence<Indices...>) {
            return {{ f(a[Indices])... }};
        }

        template<class R, class T, unsigned long int N, size_t... Indices>
        constexpr std::array<T, N> bitransform_helper(std::function<R(const T&, const T&)> f, const std::array<T, N> &a1, const std::array<T, N> &a2,
                                                      std::index_sequence<Indices...>) {
            return {{ f(a1[Indices], a2[Indices])... }};
        }
    }

    template<class T, class R, size_t N>
    struct Reducer {
        static constexpr inline R result(std::function<R(T, T)> f, std::function<R(R, R)> r, R defaultval, const std::array<T, N> &a, const std::array<T, N> &b) {
            return details::ReducerAux<T, R, N, N>::result(f, r, defaultval, a, b);
        }
    };

    /// Execute a transormation on a range of indices.
    template<class T, unsigned long int N>
    constexpr std::array<T, N> indextransform(std::function<T(int)> f) {
        return details::indextransform_helper<T, N>(f, std::make_index_sequence<N>{});
    }

    /// Execute a transformation on an array for each index.
    template<class R, class T, unsigned long int N>
    constexpr std::array<T, N> unitransform(std::function<R(const T&)> f, const std::array<T, N> &a) {
        return details::unitransform_helper(f, a, std::make_index_sequence<N>{});
    }

    /// Execute a transformation on a pair of arrays for each index.
    template<class R, class T, unsigned long int N>
    constexpr std::array<T, N> bitransform(std::function<R(const T&, const T&)> f, const std::array<T, N> &a1, const std::array<T, N> &a2) {
        return details::bitransform_helper(f, a1, a2, std::make_index_sequence<N>{});
    }

    /// Simple type specifiers.
    template<typename T, size_t N> constexpr std::array<T,N> operator+(const std::array<T,N> &t1, const std::array<T,N> &t2) {
        return bitransform<T,T,N>([](const T &a, const T &b) { return a + b; }, t1, t2);
    }

    template<typename T, size_t N> constexpr std::array<T,N> operator-(const std::array<T,N> &t1, const std::array<T,N> &t2) {
        return bitransform<T,T,N>([](const T &a, const T &b) { return a - b; }, t1, t2);
    }

    template<typename T, size_t N>
    constexpr std::array<T,N> operator*(const std::array<T,N> &t1, const std::array<T,N> &t2) {
        return bitransform<T,T,N>([](const T &a, const T &b) { return a * b; }, t1, t2);
    }

    template<typename F, typename T, size_t N,
            typename = typename std::enable_if<std::is_arithmetic<F>::value, F>::type>
    constexpr std::array<T,N> operator*(F f, const std::array<T,N> &t) {
        return unitransform<T,T,N>([f](const T &a) { return f * a; }, t);
    }

    template<typename T, size_t N> constexpr std::array<T,N> operator/(const std::array<T,N> &t1, const std::array<T,N> &t2) {
        return bitransform<T,T,N>([](const T &a, const T &b) { return a / b; }, t1, t2);
    }

    template<typename F, typename T, size_t N,
            typename = typename std::enable_if<std::is_arithmetic<F>::value, F>::type>
    constexpr std::array<T,N> operator/(const std::array<T,N> &t, F f) {
        return unitransform<T,T,N>([f](const T &a) { return a / f; }, t);
    }

    template<typename T, size_t N> constexpr std::array<T,N> operator-(const std::array<T,N> &t) {
        return unitransform<T,T,N>([](const T &a) { return -a; }, t);
    }

    /// If floating point, use ALMOST_EQUALS.
    template<typename T>
    constexpr typename std::enable_if<std::is_floating_point<T>::value, bool>::type
    equals(T t1, T t2) { return ALMOST_EQUALS(t1, t2); }

    /// If integral, just use ==.
    template<typename T>
    constexpr typename std::enable_if<std::is_integral<T>::value, bool>::type
    equals(T t1, T t2) { return t1 == t2; }

    /// If an array, iterate. This will allow us to check multidimensional arrays.
    template<typename T, size_t N>
    constexpr bool equals(const std::array<T,N> &t1, const std::array<T,N> &t2) {
        return Reducer<T, bool, N>::result([](const T &d1, const T &d2) { return equals(d1, d2); },
                [](bool b1, bool b2) { return b1 && b2; }, true, t1, t2);
    }

    /// Create an array where the elements are determined by a supplied function invoked on index.
    template<size_t N, class C, class F, int... Indices>
    static constexpr std::array<C, N> make_array(F f, std::index_sequence<Indices...>) {
        return std::array<C,N>{{ f(Indices)...}};
    }

//    template<class T, unsigned long int N>
//    constexpr std::array<T, N> indextransform(std::function<T(int)> f) {
//        return details::indextransform_helper<T, N>(f, std::make_index_sequence<N>{});
//    }
    template<class T, unsigned long int N>
    static constexpr std::array<T, N> make_array(const std::function<T(int)> f) {
        return indextransform<T,N>(f);
    }
}

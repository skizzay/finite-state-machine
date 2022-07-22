#pragma once

#include "skizzay/fsm/detected.h"
#include "skizzay/fsm/nonvoid.h"
#include <concepts>
#include <type_traits>

namespace skizzay::fsm {
namespace concepts {
template <typename T>
concept state = std::destructible<T> && std::copy_constructible<T> &&
    nonvoid<std::remove_cvref_t<T>> &&
    std::negation_v<std::is_pointer<std::remove_cvref_t<T>>>;
} // namespace concepts

template <typename T> using is_state = std::bool_constant<concepts::state<T>>;

template <typename> struct basic_state_t;

namespace state_t_details_ {
template <typename> struct impl;

template <typename T>
requires requires {
  typename T::state_type;
} && concepts::state<typename T::state_type>
struct impl<T> : std::true_type {
};

template <typename T>
requires concepts::state<typename basic_state_t<T>::type> &&
    (!requires { typename T::state_type; }) struct impl<T> {
  using type = typename basic_state_t<T>::type;
};

template <typename> struct current_state_t_impl;
template <typename T>
requires concepts::state<typename T::current_state_type>
struct current_state_t_impl<T> {
  using type = typename T::current_state_type;
};

template <typename> struct next_state_t_impl;
template <typename T>
requires concepts::state<typename T::next_state_type>
struct next_state_t_impl<T> {
  using type = typename T::next_state_type;
};
} // namespace state_t_details_

template <typename T> using state_t = typename state_t_details_::impl<T>::type;

template <typename T>
using current_state_t =
    typename state_t_details_::current_state_t_impl<T>::type;

template <typename T>
using next_state_t = typename state_t_details_::next_state_t_impl<T>::type;

template <typename, typename> struct is_current_state_in : std::false_type {};

template <concepts::state State, typename T>
requires requires { typename current_state_t<std::remove_cvref_t<T>>; }
struct is_current_state_in<State, T>
    : std::is_same<std::remove_cvref_t<State>,
                   current_state_t<std::remove_cvref_t<T>>> {};

template <typename, typename> struct is_next_state_in : std::false_type {};

template <concepts::state State, typename T>
requires requires { typename next_state_t<std::remove_cvref_t<T>>; }
struct is_next_state_in<State, T>
    : std::is_same<std::remove_cvref_t<State>,
                   next_state_t<std::remove_cvref_t<T>>> {};

} // namespace skizzay::fsm

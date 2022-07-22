#pragma once

#include "skizzay/fsm/detected.h"
#include "skizzay/fsm/nonvoid.h"

#include <concepts>
#include <type_traits>

namespace skizzay::fsm {
namespace concepts {
template <typename T>
concept event =
    nonvoid<std::remove_cvref_t<T>> && std::movable<std::remove_cvref_t<T>> &&
    std::negation_v<std::is_pointer<std::remove_cvref_t<T>>> &&
    std::negation_v<std::is_same<nonesuch, std::decay_t<T>>>;
} // namespace concepts

inline constexpr struct epsilon_event_t final {
} epsilon_event = {};

inline constexpr struct initial_entry_event_t final {
} initial_entry_event = {};

inline constexpr struct final_exit_event_t final {
} final_exit_event = {};

template <typename> struct basic_event_t;

namespace event_t_details_ {
template <typename> struct impl;

template <typename T>
requires concepts::event<typename basic_event_t<T>::type>
struct impl<T> {
  using type = typename basic_event_t<T>::type;
};
} // namespace event_t_details_

template <typename T> using event_t = typename event_t_details_::impl<T>::type;

template <typename T>
requires concepts::event<typename T::event_type>
struct basic_event_t<T> {
  using type = typename T::event_type;
};

template <typename T>
requires requires(T const &tc) {
  { tc.event() }
  noexcept->concepts::event;
} && (!requires { typename T::event_type; })
struct basic_event_t<T> {
  using type = std::remove_cvref_t<decltype(std::declval<T const &>().event())>;
};

} // namespace skizzay::fsm

#pragma once

#include <type_traits>

namespace skizzay::fsm {

template <typename T>
struct is_event
    : std::negation<std::disjunction<std::is_void<T>, std::is_pointer<T>>> {};
template <typename T>
struct is_event<T &> : is_event<std::remove_const_t<T>> {};

namespace concepts
{
  template<typename T>
  concept event = is_event<T>::value;
} // namespace concepts


inline constexpr struct epsilon_event_t final {
} epsilon_event = {};

inline constexpr struct initial_entry_event_t final {
} initial_entry_event = {};

inline constexpr struct final_exit_event_t final {
} final_exit_event = {};

} // namespace skizzay::fsm

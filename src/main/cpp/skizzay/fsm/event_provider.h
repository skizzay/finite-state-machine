#pragma once

#include "skizzay/fsm/event.h"

#include <concepts>
#include <type_traits>

namespace skizzay::fsm {
template <typename> struct is_event_provider : std::false_type {};

template <typename T>
requires concepts::event<typename T::event_type> && requires(T const &tc) {
  { tc.event() }
  noexcept
      ->std::same_as<std::add_lvalue_reference_t<std::add_const_t<typename T::event_type>>>;
}
struct is_event_provider<T> : std::true_type {};

namespace concepts {
template <typename T>
concept event_provider = is_event_provider<T>::value;
} // namespace concepts

} // namespace skizzay::fsm

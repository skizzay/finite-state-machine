#pragma once

#include "skizzay/fsm/const_ref.h"
#include "skizzay/fsm/event.h"

#include <concepts>
#include <type_traits>

namespace skizzay::fsm {

namespace concepts {
template <typename T>
concept event_provider = requires(T const &tc) {
  { tc.event() }
  noexcept->concepts::event;
};

template <typename T, typename Event>
concept event_provider_for = event<Event> && event_provider<T> &&
    requires(T const &tc) {
  { tc.event() }
  noexcept->std::convertible_to<add_cref_t<Event>>;
};
} // namespace concepts

template <concepts::event_provider EventProvider>
requires(!requires {
  typename EventProvider::event_type;
}) struct basic_event_t<EventProvider> {
  using type = std::remove_cvref_t<
      decltype(std::declval<EventProvider const &>().event())>;
};

template <typename T>
using is_event_provider = std::bool_constant<concepts::event_provider<T>>;

template <typename T, typename Event>
using is_event_provider_for =
    std::bool_constant<concepts::event_provider_for<T, Event>>;

} // namespace skizzay::fsm

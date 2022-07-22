#pragma once

#include "skizzay/fsm/const_ref.h"
#include "skizzay/fsm/event.h"

#include <concepts>
#include <type_traits>

namespace skizzay::fsm {

template <typename, typename> struct is_event_provider_for : std::false_type {};

namespace concepts {
template <typename T>
concept event_provider = requires(T const &tc) {
  { tc.event() }
  noexcept->concepts::event;
};

template <typename T, typename... Events>
concept event_provider_for = event_provider<T> && (event<Events> && ...) && 
                             (is_event_provider_for<T, Events>::value || ...);
} // namespace concepts

template <concepts::event_provider EventProvider, concepts::event Event>
requires requires(EventProvider const &event_provider) {
  { event_provider.event() }
  noexcept->std::convertible_to<add_cref_t<Event>>;
}
struct is_event_provider_for<EventProvider, Event> : std::true_type {};

template <typename T>
using is_event_provider = std::bool_constant<concepts::event_provider<T>>;

} // namespace skizzay::fsm

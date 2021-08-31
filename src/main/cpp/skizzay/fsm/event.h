#pragma once

#include <concepts>
#include <skizzay/fsm/concepts.h>
#include <type_traits>

namespace skizzay::fsm {

inline constexpr struct epsilon_event_t final {
} epsilon_event = {};

inline constexpr struct initial_entry_event_t final {
} initial_entry_event = {};

inline constexpr struct final_exit_event_t final {
} final_exit_event = {};

template <typename T> void as_event(T const &) = delete;

template <concepts::event Event>
constexpr std::unwrap_reference_t<Event> const &
as_event(Event const &event) noexcept {
  return event;
}

template <typename Event>
requires requires(Event &&e) {
  { *std::forward<Event>(e) }
  noexcept;
}
constexpr Event const &as_event(Event &&event) noexcept {
  return as_event(*std::forward<Event>(event));
}

} // namespace skizzay::fsm

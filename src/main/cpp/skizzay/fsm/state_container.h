#pragma once

#include "skizzay/fsm/boolean.h"
#include "skizzay/fsm/events_list.h"
#include "skizzay/fsm/simple_transition.h"
#include "skizzay/fsm/state_accessible.h"
#include "skizzay/fsm/states_list.h"
#include "skizzay/fsm/type_list.h"

#include <tuple>
#include <type_traits>

namespace skizzay::fsm {
namespace is_state_container_details_ {
struct fake_event_transition_context {
  struct event_type {};
  struct state_type {};
  using states_list_type = states_list<state_type>;
  using events_list_type = events_list<event_type>;
  using transition_table_type =
      std::tuple<simple_transition<state_type, state_type, event_type>>;

  event_type e;
  state_type s;
  [[no_unique_address]] simple_transition<state_type, state_type, event_type> t;

  constexpr event_type const &event() const noexcept { return e; }

  template <std::same_as<state_type> State> constexpr State &state() noexcept {
    return s;
  }

  template <std::same_as<state_type> State>
  constexpr State const &state() const noexcept {
    return s;
  }

  constexpr void
  post_event(concepts::event_in<events_list_type> auto const &) noexcept {}

  constexpr std::tuple<simple_transition<state_type, state_type, event_type> &>
  get_transitions(state_type const &) noexcept {
    return {t};
  }

  template <std::same_as<state_type>>
  constexpr void schedule_entry() noexcept {}
};
} // namespace is_state_container_details_

template <typename> struct is_state_container : std::false_type {};

template <typename T>
requires concepts::state_accessible<T> &&
    requires(T &t, T const &tc,
             is_state_container_details_::fake_event_transition_context &fetc) {
  { tc.is_active() }
  noexcept->concepts::boolean;
  { tc.is_inactive() }
  noexcept->concepts::boolean;
  // t.on_entry();
  { t.on_event(fetc) } -> concepts::boolean;
}
struct is_state_container<T> : std::true_type {};

namespace concepts {
template <typename T>
concept state_container = is_state_container<T>::value;
} // namespace concepts

} // namespace skizzay::fsm

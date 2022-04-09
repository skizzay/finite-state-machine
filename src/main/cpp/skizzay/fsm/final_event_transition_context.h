#pragma once

#include "skizzay/fsm/event.h"
#include "skizzay/fsm/event_engine.h"
#include "skizzay/fsm/events_list.h"
#include "skizzay/fsm/state.h"
#include "skizzay/fsm/states_list.h"
#include "skizzay/fsm/types.h"

#include <tuple>

namespace skizzay::fsm {

template <concepts::event_engine T>
requires concepts::state_provider<T>
struct final_event_transition_context {
  using event_type = final_exit_event_t;
  using events_list_type = events_list_t<T>;
  using states_list_type = states_list_t<T>;
  using transition_table_type = std::tuple<>;

  constexpr explicit final_event_transition_context(T &t) noexcept : t_{t} {}

  constexpr event_type const &event() const noexcept {
    return final_exit_event;
  }

  template <concepts::state_in<states_list_type> State>
  constexpr State &state() noexcept {
    return t_.template state<State>();
  }

  template <concepts::state_in<states_list_type> State>
  constexpr State const &state() const noexcept {
    return t_.template state<State>();
  }

  template <concepts::event_in<events_list_type> Event>
  constexpr void post_event(Event const &event) {
    t_.post_event(event);
  }

  constexpr std::tuple<>
  get_transitions(concepts::state auto const &) noexcept {
    return {};
  }

private:
  T &t_;
};

} // namespace skizzay::fsm

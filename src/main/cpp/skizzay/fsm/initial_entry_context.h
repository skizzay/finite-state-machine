#pragma once

#include "skizzay/fsm/event.h"
#include "skizzay/fsm/event_engine.h"
#include "skizzay/fsm/events_list.h"
#include "skizzay/fsm/state_containers_list.h"
#include "skizzay/fsm/state_provider.h"
#include "skizzay/fsm/states_list.h"
#include "skizzay/fsm/types.h"

namespace skizzay::fsm {

template <concepts::event_engine T>
requires concepts::state_provider<T>
struct initial_entry_context final {
  using states_list_type = states_list_t<T>;
  using events_list_type = events_list_t<T>;
  using event_type = initial_entry_event_t;
  using current_states_list_type = states_list<>;
  using next_states_list_type = states_list_type;

  constexpr explicit initial_entry_context(T &t) noexcept : t_{t} {}

  constexpr initial_entry_event_t const &event() const noexcept {
    return initial_entry_event;
  }

  template <concepts::state_in<next_states_list_type>>
  constexpr bool is_scheduled() const noexcept {
    return false;
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

private:
  T &t_;
};

} // namespace skizzay::fsm

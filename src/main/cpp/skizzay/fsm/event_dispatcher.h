#pragma once

#include "skizzay/fsm/entry_context_root.h"
#include "skizzay/fsm/entry_state_scheduler.h"
#include "skizzay/fsm/event.h"
#include "skizzay/fsm/event_context_root.h"
#include "skizzay/fsm/events_list.h"
#include "skizzay/fsm/state_container.h"
#include "skizzay/fsm/transition_table.h"

#include <functional>
#include <queue>
#include <type_traits>
#include <utility>

namespace skizzay::fsm {
namespace event_dispatcher_details_ {
struct ignore_flag final {
  constexpr inline ignore_flag(bool const) noexcept {}
};
} // namespace event_dispatcher_details_

template <concepts::transition_table TransitionTable,
          concepts::state_container RootStateContainer>
struct event_dispatcher {
  using transition_table_type = TransitionTable;

  constexpr explicit event_dispatcher(transition_table_type &&transition_table,
                                      RootStateContainer &root_state_container)
      : q_{}, root_state_container_{root_state_container},
        transition_table_{
            std::forward<transition_table_type>(transition_table)},
        accept_epsilon_events_{true} {}

  template <concepts::event_in<transition_table_type> Event>
  constexpr void post_event(Event &&event) {
    if constexpr (std::is_same_v<std::remove_cvref_t<Event>, epsilon_event_t>) {
      accept_epsilon_events_ = false;
    }
    q_.emplace(
        [this, event = std::forward<Event>(event)]() { process_event(event); });
  }

  template <concepts::event Event>
  requires concepts::event_in<Event, transition_table_type> ||
      std::same_as<std::remove_cvref_t<Event>, initial_entry_event_t> ||
      std::same_as<std::remove_cvref_t<Event>, final_exit_event_t>
  constexpr bool dispatch_event(Event const &event) {
    if (process_event(event)) {
      process_internal_events();
      return true;
    } else {
      return false;
    }
  }

private:
  std::queue<std::function<void()>> q_;
  RootStateContainer &root_state_container_;
  [[no_unique_address]] transition_table_type transition_table_;
  [[no_unique_address]] std::conditional_t<
      concepts::event_in<epsilon_event_t, transition_table_type>, bool,
      event_dispatcher_details_::ignore_flag>
      accept_epsilon_events_;

  template <concepts::event_in<transition_table_type> Event>
  constexpr bool process_event(Event const &event) {
    using next_states_list_type =
        as_container_t<map_t<filter_t<transition_table_type,
                                      curry<is_event_in, Event>::template type>,
                             next_state_t>,
                       states_list>;
    if constexpr (std::is_same_v<std::remove_cvref_t<Event>, epsilon_event_t>) {
      accept_epsilon_events_ = true;
    }
    entry_state_scheduler<next_states_list_type> scheduler;
    event_context_root event_context{event, *this, root_state_container_,
                                     transition_table_, scheduler};
    if (root_state_container_.on_event(event_context)) {
      entry_context_root entry_context{event, *this, root_state_container_,
                                       scheduler};
      root_state_container_.on_entry(entry_context);
      return true;
    } else {
      return false;
    }
  }

  void process_internal_events() {
    while (!std::empty(q_)) {
      std::function<void()> const callback{std::move(q_.front())};
      q_.pop();
      callback();
    }
  }
};

} // namespace skizzay::fsm

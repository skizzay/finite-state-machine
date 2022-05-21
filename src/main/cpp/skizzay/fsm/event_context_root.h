#pragma once

#include "skizzay/fsm/event.h"
#include "skizzay/fsm/event_engine.h"
#include "skizzay/fsm/events_list.h"
#include "skizzay/fsm/state_provider.h"
#include "skizzay/fsm/state_scheduler.h"
#include "skizzay/fsm/states_list.h"
#include "skizzay/fsm/transition_table.h"
#include "skizzay/fsm/trigger.h"
#include "skizzay/fsm/type_list.h"

#include <cassert>
#include <type_traits>

namespace skizzay::fsm {

template <concepts::event Event, concepts::event_engine EventEngine,
          concepts::state_provider StateProvider,
          concepts::transition_table TransitionTable,
          concepts::state_scheduler StateScheduler>
struct event_context_root {
  using event_type = std::remove_cvref_t<Event>;
  using events_list_type = events_list_t<EventEngine>;
  using states_list_type = states_list_t<StateProvider>;
  using next_states_list_type = next_states_list_t<StateScheduler>;
  using transition_table_type = map_t<TransitionTable, std::remove_cvref_t>;

  constexpr explicit event_context_root(
      Event const &event, EventEngine &event_engine,
      StateProvider &state_provider, TransitionTable &transition_table,
      StateScheduler &state_scheduler) noexcept
      : event_{event}, event_engine_{event_engine},
        state_provider_{state_provider}, transition_table_{transition_table},
        state_scheduler_{state_scheduler} {}

  constexpr event_type const &event() const noexcept { return event_; }

  template <concepts::self_transition_in<transition_table_type> Transition>
  constexpr void on_transition(Transition &transition) {
    trigger(transition, *this);
    this->template schedule_entry<next_state_t<Transition>>();
  }

  [[noreturn]] constexpr void
  on_transition(concepts::transition_in<transition_table_type> auto &) {
    assert(false && "Root attempting to transition to a state outside of "
                    "the machine.");
  }

  template <concepts::event_in<events_list_type> PostEvent>
  constexpr void post_event(PostEvent &&event) {
    event_engine_.post_event(std::forward<PostEvent>(event));
  }

  template <concepts::state_in<states_list_type> State>
  constexpr State const &state() const noexcept {
    return state_provider_.template state<State>();
  }

  template <concepts::state_in<states_list_type> State>
  constexpr State &state() noexcept {
    return state_provider_.template state<State>();
  }

  constexpr concepts::transition_table auto
  get_transitions(concepts::state auto const &state) noexcept {
    return get_transition_table_for_current_state(transition_table_, event(),
                                                  state);
  }

  template <concepts::state_in<next_states_list_type> State>
  constexpr void schedule_entry() noexcept {
    state_scheduler_.template schedule_entry<State>();
  }

private:
  event_type const &event_;
  EventEngine &event_engine_;
  StateProvider &state_provider_;
  transition_table_type &transition_table_;
  StateScheduler &state_scheduler_;
};

} // namespace skizzay::fsm

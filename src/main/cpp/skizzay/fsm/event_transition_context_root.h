#pragma once

#include "skizzay/fsm/entry_state_scheduler.h"
#include "skizzay/fsm/state.h"
#include "skizzay/fsm/states_list.h"
#include "skizzay/fsm/transition_table.h"
#include "skizzay/fsm/trigger.h"

#include <cassert>
#include <type_traits>

namespace skizzay::fsm {

template <concepts::transition_table TransitionTable>
struct event_transition_context_root {
  using next_states_list_type = next_states_list_t<TransitionTable>;
  using transition_table_type = TransitionTable;

  constexpr explicit event_transition_context_root(
      TransitionTable &&transition_table) noexcept
      : transition_table_{std::move(transition_table)} {}

  template <concepts::self_transition_in<transition_table_type> Transition>
  constexpr void
  on_transition(Transition &transition,
                concepts::event_in<Transition> auto const &event) {
    trigger(transition, event);
    this->template schedule_entry<next_state_t<Transition>>();
  }

  template <concepts::transition_in<transition_table_type> Transition>
  [[noreturn]] constexpr void
  on_transition(Transition &, concepts::event_in<Transition> auto const &) {
    assert(false && "Root attempting to transition to a state outside of "
                    "the machine.");
  }

  constexpr concepts::transition_table auto
  get_transitions(concepts::state auto const &state) noexcept {
    return get_transition_table_for(transition_table_, state);
  }

  template <concepts::state_in<next_states_list_type> State>
  constexpr void schedule_entry() noexcept {
    state_scheduler_.template schedule_entry<State>();
  }

  constexpr concepts::state_schedule auto const &
  state_schedule() const noexcept {
    return state_scheduler_;
  }

private:
  entry_state_scheduler<next_states_list_type> state_scheduler_;
  [[no_unique_address]] transition_table_type transition_table_;
};

template <concepts::transition_table TransitionTable>
event_transition_context_root(TransitionTable &&)
    -> event_transition_context_root<std::remove_reference_t<TransitionTable>>;
} // namespace skizzay::fsm

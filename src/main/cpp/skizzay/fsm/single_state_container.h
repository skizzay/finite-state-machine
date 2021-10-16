#pragma once

#include <skizzay/fsm/basic_state_container.h>
#include <skizzay/fsm/concepts.h>
#include <skizzay/fsm/traits.h>
#include <type_traits>

namespace skizzay::fsm {

template <concepts::state State>
requires(!concepts::state_container<State>) struct single_state
    : private basic_state_container<State> {
  using basic_state_container<State>::states_list_type;

  using basic_state_container<State>::basic_state_container;
  using basic_state_container<State>::is;
  using basic_state_container<State>::is_active;
  using basic_state_container<State>::is_inactive;
  using basic_state_container<State>::current_state;
  using basic_state_container<State>::get;
  using basic_state_container<State>::ancestry_to;
  using basic_state_container<State>::on_initial_entry;
  using basic_state_container<State>::on_final_exit;
  using basic_state_container<State>::on_entry;
  using basic_state_container<State>::on_event;
};

template <concepts::state State>
single_state(State &&) -> single_state<std::remove_cvref_t<State>>;
} // namespace skizzay::fsm
#pragma once

#include "skizzay/fsm/basic_state_container.h"
#include "skizzay/fsm/state.h"
#include "skizzay/fsm/state_container.h"
#include "skizzay/fsm/states_list.h"

#include <type_traits>

namespace skizzay::fsm {

template <concepts::state> struct single_state;

template <concepts::state State>
requires(!concepts::state_container<State>) struct basic_states_list_t<
    single_state<State>> {
  using type = states_list<State>;
};

template <concepts::state State>
requires(!concepts::state_container<State>) struct single_state<State>
    : basic_state_container<single_state<State>, State> {

  using basic_state_container<single_state<State>,
                              State>::basic_state_container;
};

template <concepts::state State>
single_state(State &&) -> single_state<std::remove_cvref_t<State>>;
} // namespace skizzay::fsm
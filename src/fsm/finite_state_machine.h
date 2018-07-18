#pragma once

#include "fsm/traits.h"
#include <utility>

namespace skizzay::fsm {

template<class TransitionTable, class StateContainer>
class finite_state_machine {
   TransitionTable transitions_;
   StateContainer current_state_;

public:
   using state_type = StateContainer;
   using transition_table_type = TransitionTable;

   constexpr finite_state_machine(TransitionTable transitions, StateContainer initial_state) :
      transitions_{std::move(transitions)},
      current_state_{std::move(initial_state)}
   {
   }

   template<class Event>
   constexpr std::enable_if_t<is_event_v<Event>> on(Event const &e) {
      current_state_.dispatch(e, transitions_);
   }

   template<class Function>
   constexpr decltype(auto) query(Function &&f) const {
      return current_state_.query(f);
   }

   template<class State>
   constexpr bool is() const noexcept {
      return current_state_.template is<State>();
   }
};

}

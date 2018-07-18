#pragma once

#include "fsm/detected.h"
#include "fsm/dispatcher.h"
#include "fsm/state.h"
#include "fsm/transition_traits.h"
#include <cstddef>
#include <tuple>
#include <utility>

namespace skizzay::fsm {

template<class Dispatcher>
struct dispatcher_traits {
   template<class State, class Event, class TransitionTuple, class StateContainer, std::size_t... Indices>
   static constexpr void dispatch(Dispatcher &d, State &s, Event const &e, StateContainer &container, TransitionTuple &transitions, std::index_sequence<Indices...>) {
      d.on_no_trigger_found(const_cast<State const &>(s), e);
   }

   template<class State, class Event, class TransitionTuple, class StateContainer, std::size_t I, std::size_t... Indices>
   static constexpr void dispatch(Dispatcher &d, State &s, Event const &e, StateContainer &container, TransitionTuple &transitions, std::index_sequence<I, Indices...>) {
      using transition_type = std::remove_reference_t<decltype(std::get<I>(transitions))>;

      transition_type &t = std::get<I>(transitions);
      if (const_cast<transition_type const &>(t).accepts(const_cast<State const &>(s), e)) {
         d.on_accepted(const_cast<State const &>(s), e, transitions, std::index_sequence<Indices...>{});
         if constexpr (std::is_same_v<State, details_::typename_next_state_type<transition_type>>) {
            transition_traits<transition_type>::on_triggered(t, e);
            state_traits<State>::on_reentered(s, e);
         }
         else {
            state_traits<State>::on_exited(s);
            transition_traits<transition_type>::on_triggered(t, e);
            decltype(auto) next_state = container.template construct<details_::typename_next_state_type<transition_type>>(std::move(s), e);
            state_traits<std::remove_reference_t<decltype(next_state)>>::on_entered(next_state, e);
         }
      }
      else {
         dispatch(d, s, e, container, transitions, std::index_sequence<Indices...>{});
      }
   }
};

}

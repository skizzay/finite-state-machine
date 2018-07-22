#pragma once

#include "fsm/traits.h"
#include <type_traits>
#include <utility>
#include <variant>

namespace skizzay::fsm {

template<class> class state_container;

template<class ...States>
class state_container<std::variant<States...>> {
   std::variant<States...> states_;

public:
   state_container() = delete;

   constexpr state_container(state_container const &other) :
      states_{other.states_}
   {
   }

   constexpr state_container(state_container &&other) :
      states_{std::move(other.states_)}
   {
   }

   template<class InitialState, std::enable_if_t<!is_in_place_type_v<std::decay_t<InitialState>>, int> = 0>
   constexpr state_container(InitialState &&initial_state) :
      states_{std::forward<InitialState>(initial_state)}
   {
   }

   template<class InitialState, class ...Args>
   constexpr state_container(std::in_place_type_t<InitialState> const t, Args &&...args) :
      states_{t, std::forward<Args>(args)...}
   {
   }

   constexpr state_container & operator =(state_container const &other) {
      if (this != &other) {
         this->states_ = other.states_;
      }
      return *this;
   }

   constexpr state_container & operator =(state_container &&other) {
      if (this != &other) {
         this->states_ = std::move(other.states_);
      }
      return *this;
   }

   template<class NextState, class PreviousState, class Event>
   constexpr decltype(auto) construct(PreviousState &&s, Event const &e) {
      if constexpr (std::is_constructible_v<NextState, PreviousState &&, Event const &>) {
         return states_.template emplace<NextState>(std::forward<PreviousState>(s), e);
      }
      else if constexpr (std::is_constructible_v<NextState, Event const &, PreviousState &&>) {
         return states_.template emplace<NextState>(e, std::forward<PreviousState>(s));
      }
      else if constexpr (std::is_constructible_v<NextState, Event const &>) {
         return states_.template emplace<NextState>(e);
      }
      else if constexpr (std::is_constructible_v<NextState, PreviousState &&>) {
         return states_.template emplace<NextState>(std::forward<PreviousState>(s));
      }
      else if constexpr (std::is_default_constructible_v<NextState>) {
         return states_.template emplace<NextState>();
      }
      else {
         // The predicate is bogus and will always be false.  It prevents
         // warnings from the compiler though.
         static_assert(std::is_same_v<NextState, PreviousState>,
               "Cannot construct next state.");
      }
   }

   template<class State>
   constexpr bool is() const noexcept {
      return std::holds_alternative<State>(states_);
   }

   template<class Visitor>
   constexpr decltype(auto) query(Visitor &&visitor) const {
      return std::visit(std::forward<Visitor>(visitor), states_);
   }

   template<class Event, class TransitionTable>
   constexpr std::enable_if_t<is_event_v<Event>> dispatch(Event const &e, TransitionTable &transitions) {
      std::visit([&](auto &state) {
            transitions.dispatch(state, e, *this);
         }, states_);
   }
};

}

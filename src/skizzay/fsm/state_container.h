#pragma once

#include "skizzay/fsm/traits.h"
#include <functional>
#include <memory>
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
   constexpr decltype(auto) construct(PreviousState &&s[[maybe_unused]], Event const &e[[maybe_unused]]) {
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


template<class T>
class state_container<std::unique_ptr<T>> {
   std::unique_ptr<T> state_;

public:
   state_container() = delete;

   constexpr state_container(state_container &&other) :
      state_{std::move(other.state_)}
   {
   }

   template<class InitialState>
   constexpr state_container(InitialState &&initial_state) :
      state_{std::move(initial_state)}
   {
   }

   constexpr state_container & operator =(state_container &&other) {
      if (this != &other) {
         this->states_ = std::move(other.states_);
      }
      return *this;
   }

   template<class NextState, class PreviousState, class Event>
   constexpr decltype(auto) construct(PreviousState &&s[[maybe_unused]], Event const &e[[maybe_unused]]) {
      if constexpr (std::is_constructible_v<NextState, PreviousState &&, Event const &>) {
         state_ = std::make_unique<NextState>(std::forward<PreviousState>(s), e);
      }
      else if constexpr (std::is_constructible_v<NextState, Event const &, PreviousState &&>) {
         return state_ = std::make_unique<NextState>(e, std::forward<PreviousState>(s));
      }
      else if constexpr (std::is_constructible_v<NextState, Event const &>) {
         return state_ = std::make_unique<NextState>(e);
      }
      else if constexpr (std::is_constructible_v<NextState, PreviousState &&>) {
         return state_ = std::make_unique<NextState>(std::forward<PreviousState>(s));
      }
      else if constexpr (std::is_default_constructible_v<NextState>) {
         return state_ = std::make_unique<NextState>();
      }
      else {
         // The predicate is bogus and will always be false.  It prevents
         // warnings from the compiler though.
         static_assert(std::is_same_v<NextState, PreviousState>,
               "Cannot construct next state.");
      }
      return static_cast<NextState &>(*state_);
   }

   template<class State>
   constexpr bool is() const noexcept {
      return nullptr != dynamic_cast<State const *>(state_.get());
   }

   template<class Visitor>
   constexpr decltype(auto) query(Visitor &&visitor) const {
      return std::invoke(std::forward<Visitor>(visitor), *state_);
   }

   template<class Event, class TransitionTable>
   constexpr std::enable_if_t<is_event_v<Event>> dispatch(Event const &e, TransitionTable &transitions) {
      transitions.dispatch(*state_, e, *this);
   }
};

}

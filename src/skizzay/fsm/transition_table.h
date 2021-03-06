#pragma once

#include "skizzay/fsm/dispatcher_traits.h"
#include "skizzay/fsm/traits.h"
#include "skizzay/utilz/type_sequence.h"
#include <type_traits>
#include <utility>

namespace skizzay::fsm {

namespace details_ {

template<class State, class Event>
struct handles_event {
   template<class Transition>
   struct apply : std::conjunction<
      std::is_convertible<State const &, typename_current_state_type<Transition> const &>,
      std::is_convertible<Event const &, typename_event_type<Transition> const &>,
      std::is_convertible<bool, accepts_method<Transition>>
   > {
   };

   template<std::size_t I, class Transition>
   struct apply<utilz::type_sequence<utilz::size_constant<I>, Transition>> : apply<Transition> {};
};

template<class State, class Event, class... Transitions>
struct acceptable_transitions {
   template<class T> using predicate_f = utilz::apply_f<handles_event<State, Event>, T>;
   using type = utilz::to_filtered_indices_t<utilz::type_sequence<Transitions...>, predicate_f>;
};

}

template<class Dispatcher, class ...Transitions>
class transition_table {
   static_assert(std::conjunction_v<is_transition<Transitions>...>);

   struct storage : Dispatcher {
      std::tuple<Transitions...> transitions;

      constexpr storage(Dispatcher dispatcher, Transitions &&...transitions) :
         Dispatcher{std::move(dispatcher)},
         transitions{std::move(transitions)...}
      {
      }

   } storage_;

public:
   constexpr transition_table(Dispatcher dispatcher, Transitions && ...transitions) :
      storage_{std::move(dispatcher), std::forward<Transitions>(transitions)...}
   {
   }

   template<class State, class Event, class StateContainer>
   constexpr void dispatch(State &s, Event const &e, StateContainer &container) {
      using traits = dispatcher_traits<Dispatcher>;
      using transition_candidates = typename details_::acceptable_transitions<State, Event, Transitions...>::type;

      Dispatcher &dispatcher = this->storage_;
      std::tuple<Transitions...> &transitions = this->storage_.transitions;

      traits::dispatch(dispatcher, s, e, container, transitions, transition_candidates{});
   }
};

}

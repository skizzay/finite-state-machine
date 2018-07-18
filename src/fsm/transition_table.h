#pragma once

#include "fsm/dispatcher_traits.h"
#include "fsm/traits.h"
#include <type_traits>
#include <utility>

namespace skizzay::fsm {

namespace details_ {
template<class, class> struct concat;

template <std::size_t... Is, std::size_t... Js>
struct concat<std::index_sequence<Is...>, std::index_sequence<Js...>> {
   using type = std::index_sequence<Is..., Js...>;
};


template<class T, class U>
using concat_t = typename concat<T, U>::type;

template<class T, class S, class E>
using handles_event = std::conjunction<
      std::is_convertible<S const &, typename_current_state_type<T> const &>,
      std::is_convertible<E const &, typename_event_type<T> const &>,
      std::is_convertible<bool, accepts_method<T>>
>;

template<class State, class Event, std::size_t Index, class... Transitions>
struct acceptable_transitions_impl {
   using type = std::index_sequence<>;
};

template<class State, class Event, std::size_t Index, class Transition, class... Transitions>
struct acceptable_transitions_impl<State, Event, Index, Transition, Transitions...> {
   using type = concat_t<
      std::conditional_t<handles_event<Transition, State, Event>::value,
                         std::index_sequence<Index>, 
                         std::index_sequence<>>,
      typename acceptable_transitions_impl<State, Event, Index + 1, Transitions...>::type>;
};


template<class State, class Event, class ...Transitions>
using acceptable_transitions = typename acceptable_transitions_impl<State, Event, 0, Transitions...>::type;

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
      using transition_candidates = details_::acceptable_transitions<State, Event, Transitions...>;

      Dispatcher &dispatcher = this->storage_;
      std::tuple<Transitions...> &transitions = this->storage_.transitions;

      traits::dispatch(dispatcher, s, e, container, transitions, transition_candidates{});
   }
};

}

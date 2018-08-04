#pragma once

#include "skizzay/fsm/traits.h"
#include <skizzay/utilz/detected.h>
#include <skizzay/utilz/traits.h>
#include <cstddef>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <utility>

namespace skizzay::fsm {

namespace details_ {
template<class D, class ...Args>
using no_trigger_found_method = decltype(std::declval<D>().on_no_trigger_found(std::declval<Args const &>()...));
template<class D, class ...Args>
constexpr bool has_no_trigger_found_method_v = utilz::is_detected_v<no_trigger_found_method, D, Args...>;
template<class D, class ...Args>
using on_ambiguous_trigger_found_method = decltype(std::declval<D>().on_ambiguous_trigger_found(std::declval<Args const &>()...));
template <class D, class... Args>
using has_on_ambiguous_trigger_found_method = utilz::is_detected<on_ambiguous_trigger_found_method, D, Args...>;
template <class D, class... Args>
constexpr bool has_on_ambiguous_trigger_found_method_v = has_on_ambiguous_trigger_found_method<D, Args...>::value;
template <class Dispatcher, class State, class Event>
using handles_ambiguous_triggers = std::disjunction<
         has_on_ambiguous_trigger_found_method<Dispatcher, State, Event>,
         has_on_ambiguous_trigger_found_method<Dispatcher, Event, State>,
         has_on_ambiguous_trigger_found_method<Dispatcher, Event>,
         has_on_ambiguous_trigger_found_method<Dispatcher, State>,
         has_on_ambiguous_trigger_found_method<Dispatcher>
>;

template<class Dispatcher, class State, class Event>
constexpr void on_no_trigger_found(Dispatcher &dispatcher, State const &s, Event const &e) {
   if constexpr (details_::has_no_trigger_found_method_v<Dispatcher, State, Event>) {
      dispatcher.on_no_trigger_found(s, e);
   }
   else if constexpr (details_::has_no_trigger_found_method_v<Dispatcher, Event, State>) {
      dispatcher.on_no_trigger_found(e, s);
   }
   else if constexpr (details_::has_no_trigger_found_method_v<Dispatcher, Event>) {
      dispatcher.on_no_trigger_found(e);
   }
   else if constexpr (details_::has_no_trigger_found_method_v<Dispatcher, State>) {
      dispatcher.on_no_trigger_found(s);
   }
   else if constexpr (details_::has_no_trigger_found_method_v<Dispatcher>) {
      dispatcher.on_no_trigger_found();
   }
}

template<class Dispatcher, class State, class Event>
constexpr void on_ambiguous_trigger_found(Dispatcher &dispatcher, State const &s, Event const &e) {
   if constexpr (details_::has_on_ambiguous_trigger_found_method_v<Dispatcher, State, Event>) {
      dispatcher.on_ambiguous_trigger_found(s, e);
   }
   else if constexpr (details_::has_on_ambiguous_trigger_found_method_v<Dispatcher, Event, State>) {
      dispatcher.on_ambiguous_trigger_found(e, s);
   }
   else if constexpr (details_::has_on_ambiguous_trigger_found_method_v<Dispatcher, Event>) {
      dispatcher.on_ambiguous_trigger_found(e);
   }
   else if constexpr (details_::has_on_ambiguous_trigger_found_method_v<Dispatcher, State>) {
      dispatcher.on_ambiguous_trigger_found(s);
   }
   else if constexpr (details_::has_on_ambiguous_trigger_found_method_v<Dispatcher>) {
      dispatcher.on_ambiguous_trigger_found();
   }
}

template <class State, class Event, class TransitionTuple, std::size_t ...Indices>
constexpr bool has_on_ambiguous_trigger(State const &s, Event const &e, TransitionTuple transitions, std::index_sequence<Indices...>) {
   return (std::get<Indices>(transitions).accepts(s, e) || ...);
}
}

struct ambiguous_trigger_found : std::logic_error {
   using std::logic_error::logic_error;
};

struct throw_on_ambiguous_trigger {
   constexpr throw_on_ambiguous_trigger() noexcept = default;

   template<class State, class Event>
   constexpr void on_ambiguous_trigger_found(State const &, Event const &) {
      throw ambiguous_trigger_found{"Found an ambiguous trigger"};
   }
};

struct no_trigger_found : std::logic_error {
   using std::logic_error::logic_error;
};

struct throw_on_no_trigger {
   constexpr throw_on_no_trigger() noexcept = default;

   template<class State, class Event>
   constexpr void on_no_trigger_found(State const &, Event const &) {
      throw no_trigger_found{"No trigger found for state and event"};
   }
};

template<class ...DispatcherPolicies>
class dispatcher : DispatcherPolicies... {
   template<class State, class Event>
   constexpr void on_ambiguous_trigger_found(State const &s, Event const &e) {
      (details_::on_ambiguous_trigger_found(*static_cast<DispatcherPolicies *>(this), s, e), ...);
   }

public:
   constexpr dispatcher(DispatcherPolicies &&...policies) :
      DispatcherPolicies{std::forward<DispatcherPolicies>(policies)}...
   {
   }

   template <class State, class Event, class TransitionTuple, std::size_t... Indices>
   constexpr void on_accepted(State const &s[[maybe_unused]], Event const &e[[maybe_unused]],
         TransitionTuple const &transitions[[maybe_unused]],
         std::index_sequence<Indices...> const is[[maybe_unused]]) {
      static_assert(is_event_v<Event>);
      static_assert(utilz::is_tuple_v<TransitionTuple>);
      if constexpr (std::disjunction_v<details_::handles_ambiguous_triggers<DispatcherPolicies, State, Event>...>
            && (0 < sizeof...(Indices))) {
            if (details_::has_on_ambiguous_trigger(s, e, transitions, is)) {
               on_ambiguous_trigger_found(s, e);
            }
         }
   }

   template <class State, class Event>
   constexpr void on_no_trigger_found(State const &s, Event const &e) {
      static_assert(is_event_v<Event>);
      (details_::on_no_trigger_found(*static_cast<DispatcherPolicies *>(this), s, e), ...);
   }
};

template<class ...Ts> dispatcher(Ts &&...) -> dispatcher<std::remove_const_t<std::remove_reference_t<Ts>>...>;

}

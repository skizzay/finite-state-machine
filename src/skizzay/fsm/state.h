#pragma once

#include "skizzay/fsm/traits.h"
#include <type_traits>
#include <utility>

namespace skizzay::fsm {

namespace details_ {
template <class T>
using on_exited_method = decltype(std::declval<T>().on_exited());
template <class T>
using has_on_exited_method = utils::is_detected<on_exited_method, T>;

template <class T>
using on_entered_method = decltype(std::declval<T>().on_entered());
template <class T>
using has_on_entered_method = utils::is_detected<on_entered_method, T>;

template <class T, class E>
using on_entered_with_event_method = decltype(std::declval<T>().on_entered(std::declval<E const &>()));
template <class T, class E>
using has_on_entered_with_event_method = utils::is_detected<on_entered_with_event_method, T, E>;

template <class T>
using on_reentered_method = decltype(std::declval<T>().on_reentered());
template <class T>
using has_on_reentered_method = utils::is_detected<on_reentered_method, T>;

template <class T, class E>
using on_reentered_with_event_method = decltype(std::declval<T>().on_reentered(std::declval<E const &>()));
template <class T, class E>
using has_on_reentered_with_event_method = utils::is_detected<on_reentered_with_event_method, T, E>;
}

template<class State>
struct state_traits {
   constexpr static void on_exited(State &s[[maybe_unused]]) {
      if constexpr (details_::has_on_exited_method<State>::value) {
         s.on_exited();
      }
   }

   template<class Event>
   constexpr static void on_entered(State &s[[maybe_unused]], Event const &e[[maybe_unused]]) {
      if constexpr (details_::has_on_entered_with_event_method<State, Event>::value) {
         s.on_entered(e);
      }
      else if constexpr (details_::has_on_entered_method<State>::value) {
         s.on_entered();
      }
   }

   template<class Event>
   constexpr static void on_reentered(State &s[[maybe_unused]], Event const &e[[maybe_unused]]) {
      if constexpr (details_::has_on_reentered_with_event_method<State, Event>::value) {
         s.on_reentered(e);
      }
      else if constexpr (details_::has_on_reentered_method<State>::value) {
         s.on_reentered();
      }
   }
};

}

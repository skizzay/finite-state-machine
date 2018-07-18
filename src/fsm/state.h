#pragma once

#include "fsm/traits.h"
#include <type_traits>
#include <utility>

namespace skizzay::fsm {

namespace details_ {
template <class T>
using on_exited_method = decltype(std::declval<T>().on_exited());
template <class T>
using has_on_exited_method = is_detected<on_exited_method, T>;

template <class T, class E>
using on_entered_method = decltype(std::declval<T>().on_entered(std::declval<E const &>()));
template <class T, class E>
using has_on_entered_method = is_detected<on_entered_method, T, E>;

template <class T, class E>
using on_reentered_method = decltype(std::declval<T>().on_reentered(std::declval<E const &>()));
template <class T, class E>
using has_on_reentered_method = is_detected<on_reentered_method, T, E>;
}

template<class State>
struct state_traits {
   constexpr static void on_exited(State &s) {
      if constexpr (details_::has_on_exited_method<State>::value) {
         s.on_exited();
      }
   }

   template<class Event>
   constexpr static void on_entered(State &s, Event const &e) {
      if constexpr (details_::has_on_exited_method<State>::value) {
         s.on_entered(e);
      }
   }

   template<class Event>
   constexpr static void on_reentered(State &s, Event const &e) {
      if constexpr (details_::has_on_reentered_method<State, Event>::value) {
         s.on_reentered(e);
      }
   }
};

}

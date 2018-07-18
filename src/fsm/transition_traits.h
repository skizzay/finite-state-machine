#pragma once

#include "fsm/traits.h"
#include <type_traits>

namespace skizzay::fsm {

namespace details_ {
template<class T> using on_triggered_method = decltype(std::declval<T>().on_triggered(std::declval<typename_event_type<T> const &>()));
template<class T> using has_on_triggered_method = is_detected<on_triggered_method, T>;
}

template<class Transition>
struct transition_traits {
   using current_state_type = typename Transition::current_state_type;
   using next_state_type = typename Transition::next_state_type;
   using event_type = typename Transition::event_type;

   constexpr static void on_triggered(Transition &t, event_type const &e) {
      if constexpr (details_::has_on_triggered_method<Transition>::value) {
         t.on_triggered(e);
      }
   }
};

}

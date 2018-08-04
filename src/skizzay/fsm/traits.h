#pragma once

#include "skizzay/fsm/event.h"
#include <skizzay/utilz/detected.h>
#include <skizzay/utilz/traits.h>
#include <type_traits>
#include <variant>

namespace skizzay::fsm {

namespace details_ {
template<class T> using typename_current_state_type = typename T::current_state_type;
template<class T> using has_typename_current_state_type = utilz::is_detected<typename_current_state_type, T>;
template<class T> using typename_next_state_type = typename T::next_state_type;
template<class T> using has_typename_next_state_type = utilz::is_detected<typename_next_state_type, T>;
template<class T> using typename_event_type = typename T::event_type;

template<class T>
using accepts_method = decltype(std::declval<T const>().accepts(std::declval<typename_current_state_type<T> const &>(),
                                                                std::declval<typename_event_type<T> const &>()));
template<class T> using has_accepts_method = utilz::is_detected_convertible<bool, accepts_method, T>;
}

template <class T> using is_event = std::disjunction<
   utilz::is_template<T, event>,
   utilz::inherits_from_template<T, event>
>;
template<class T> constexpr bool is_event_v = is_event<T>::value;

template<class T> using is_transition = std::conjunction<
   details_::has_typename_current_state_type<T>,
   details_::has_typename_next_state_type<T>,
   is_event<utilz::detected_t<details_::typename_event_type, T>>,
   details_::has_accepts_method<T>
>;
template<class T> constexpr bool is_transition_v = is_transition<T>::value;

}

#pragma once

#include "fsm/detected.h"
#include <type_traits>
#include <variant>

namespace skizzay::fsm {

template<class> struct tag {};

template<class ...> struct event;

namespace details_ {
template<class T, template<class ...> class Template>
class is_template {
   static nonesuch test(...);
   template<class ...Args>
   static Template<Args...> test(Template<Args...> * const);

public:
   using type = decltype(is_template<T, Template>::test(static_cast<T *>(nullptr)));
   using value_t = std::is_same<T, type>;
   static constexpr bool value = value_t::value;
};

template<template<class ...> class Template>
class is_template<nonesuch, Template> {
public:
   using type = nonesuch;
   using value_t = std::false_type;
   static constexpr bool value = false;
};

template<class T, template<class ...> class Template>
struct inherits_from_template : std::is_base_of<typename is_template<T, Template>::type, T> {};

template<template<class ...> class Template>
struct inherits_from_template<nonesuch, Template> : std::false_type {};

template<class T>
using dereference = decltype(*std::declval<T>());

template<class T>
using pointer_operator = decltype(std::declval<T>().operator->());

template<class, class=void> struct is_dereferencible_to_reference : std::false_type {};
template<class T> struct is_dereferencible_to_reference<T, std::void_t<dereference<T>>> : std::is_reference<dereference<T>> {};

template<class T>
using is_pointer_like = std::disjunction<std::is_pointer<T>,
      std::conjunction<is_dereferencible_to_reference<T>,
                       is_detected<pointer_operator, T>
      >>;

template<class T>
constexpr std::enable_if_t<!is_pointer_like<T>::value, std::remove_reference_t<T> &> to_reference(T &t) noexcept {
   return static_cast<std::remove_reference_t<T> &>(t);
}

template<class T>
constexpr auto to_reference(T &t) noexcept -> dereference<std::remove_reference_t<T>> {
   return *t;
}

template<class T> using typename_current_state_type = typename T::current_state_type;
template<class T> using has_typename_current_state_type = is_detected<typename_current_state_type, T>;
template<class T> using typename_next_state_type = typename T::next_state_type;
template<class T> using has_typename_next_state_type = is_detected<typename_next_state_type, T>;
template<class T> using typename_event_type = typename T::event_type;

template<class T>
using accepts_method = decltype(std::declval<T const>().accepts(std::declval<typename_current_state_type<T> const &>(),
                                                                std::declval<typename_event_type<T> const &>()));
template<class T> using has_accepts_method = is_detected_convertible<bool, accepts_method, T>;

}

template <class T> using is_event = std::disjunction<
   details_::is_template<T, event>,
   details_::inherits_from_template<T, event>
>;
template<class T> constexpr bool is_event_v = is_event<T>::value;

template<class T> using is_transition = std::conjunction<
   details_::has_typename_current_state_type<T>,
   details_::has_typename_next_state_type<T>,
   is_event<detected_t<details_::typename_event_type, T>>,
   details_::has_accepts_method<T>
>;
template<class T> constexpr bool is_transition_v = is_transition<T>::value;

template<class T> using is_variant = typename details_::is_template<T, std::variant>::value_t;
template<class T> constexpr bool is_variant_v = is_variant<T>::value;

template<class T> using is_tuple = typename details_::is_template<T, std::tuple>::value_t;
template<class T> constexpr bool is_tuple_v = is_tuple<T>::value;

}

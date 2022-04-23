#pragma once

#include <skizzay/fsm/event.h>
#include <skizzay/fsm/type_list.h>

#include <type_traits>

namespace skizzay::fsm {
template <typename> struct is_events_list : std::false_type {};
template <typename, typename> struct is_event_in : std::false_type {};

template <template <typename...> typename Template, concepts::event... Events>
struct is_events_list<Template<Events...>> : std::true_type {};

template <concepts::event Event, typename EventsList>
requires is_events_list<EventsList>::value && contains_v<EventsList, Event>
struct is_event_in<Event, EventsList> : std::true_type {
};

namespace concepts {
template <typename T>
concept events_list = is_events_list<T>::value;

template <typename Event, typename EventsList>
concept event_in = is_event_in<Event, EventsList>::value;
} // namespace concepts

template <typename> struct basic_events_list_t;

namespace events_list_t_details_ {
template <typename> struct impl;

template <concepts::events_list T>
requires(!requires { typename basic_events_list_t<T>::type; }) struct impl<T> {
  using type = T;
};

template <typename T>
requires concepts::events_list<typename basic_events_list_t<T>::type>
struct impl<T> {
  using type = typename basic_events_list_t<T>::type;
};
} // namespace events_list_t_details_

template <typename T>
using events_list_t = typename events_list_t_details_::impl<T>::type;

template <typename T>
requires concepts::events_list<typename T::events_list_type>
struct basic_events_list_t<T> {
  using type = events_list_t<typename T::events_list_type>;
};

template <concepts::event... Events> struct events_list {};
} // namespace skizzay::fsm
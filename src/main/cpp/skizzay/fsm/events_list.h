#pragma once

#include "skizzay/fsm/event.h"
#include "skizzay/fsm/type_list.h"

#include <type_traits>

namespace skizzay::fsm {
template <concepts::event... Events> struct events_list {};

template <typename> struct is_events_list : std::false_type {};
template <typename, typename> struct is_event_in : std::false_type {};

template <concepts::event... Events>
struct is_events_list<events_list<Events...>> : std::true_type {};

namespace concepts {
template <typename T>
concept events_list = is_events_list<T>::value;

template <typename Event, typename EventsList>
concept event_in = event<Event> && is_event_in<Event, EventsList>::value;
} // namespace concepts

template <concepts::event Event, concepts::events_list EventsList>
requires contains_v<map_t<EventsList, std::remove_cvref_t>,
                    std::remove_cvref_t<Event>>
struct is_event_in<Event, EventsList> : std::true_type {
};

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
} // namespace skizzay::fsm
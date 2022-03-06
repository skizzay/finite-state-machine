#pragma once

#include "skizzay/fsm/event.h"
#include "skizzay/fsm/events_list.h"
#include "skizzay/fsm/type_list.h"

#include <type_traits>

namespace skizzay::fsm {
template <typename, typename...>
struct is_event_engine_for : std::false_type {};

namespace event_engine_details_ {
template <typename> struct template_member_function {
  template <typename> struct post_event : std::false_type {};
};

template <typename T> template <typename Event>
requires concepts::event<Event> && requires(T &t, Event const &event) {
  t.post_event(event);
}
struct template_member_function<T>::post_event<Event> : std::true_type {};
} // namespace event_engine_details_

template <typename T, concepts::event Event0, concepts::event... EventsN>
requires all_v<events_list<Event0, EventsN...>,
               event_engine_details_::template_member_function<T>::
                   template post_event> struct is_event_engine_for<T, Event0,
                                                                   EventsN...>
    : std::true_type {
};

namespace concepts {
template <typename T, typename... Events>
concept event_engine_for = is_event_engine_for<T, Events...>::value;

template <typename T>
concept event_engine = events_list<typename T::events_list_type> &&
    (0 < length_v<typename T::events_list_type>)&&all_v<
        typename T::events_list_type,
        event_engine_details_::template_member_function<
            T>::template post_event>;
} // namespace concepts

template <typename T>
struct is_event_engine : std::bool_constant<concepts::event_engine<T>> {};
} // namespace skizzay::fsm

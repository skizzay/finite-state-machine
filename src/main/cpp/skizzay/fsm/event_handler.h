#pragma once

#include <skizzay/fsm/boolean.h>
#include <skizzay/fsm/event.h>
#include <skizzay/fsm/events_list.h>
#include <skizzay/fsm/type_list.h>
#include <type_traits>

namespace skizzay::fsm {
template <typename, typename...>
struct is_event_handler_for : std::false_type {};

namespace event_handler_details_ {
template <typename> struct template_member_function {
  template <typename> struct on : std::false_type {};
};

template <typename T> template <typename Event>
requires concepts::event<Event> && requires(T &t, Event const &event) {
  { t.on(event) } -> concepts::boolean;
}
struct template_member_function<T>::on<Event> : std::true_type {};
} // namespace event_handler_details_

template <typename T, concepts::event... Events>
requires all_v<events_list<Events...>,
               event_handler_details_::template_member_function<
                   T>::template on> struct is_event_handler_for<T, Events...>
    : std::true_type {
};

namespace concepts {
template <typename T, typename... Events>
concept event_handler_for = is_event_handler_for<T, Events...>::value;

template <typename T>
concept event_handler = events_list<typename T::events_list_type> &&
    (0 < length_v<typename T::events_list_type>)&&all_v<
        typename T::events_list_type,
        event_handler_details_::template_member_function<T>::template on>;
} // namespace concepts

template <typename T>
struct is_event_handler : std::bool_constant<concepts::event_handler<T>> {};

} // namespace skizzay::fsm

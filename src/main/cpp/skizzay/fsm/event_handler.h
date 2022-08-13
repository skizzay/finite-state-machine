#pragma once

#include "skizzay/fsm/boolean.h"
#include "skizzay/fsm/event.h"
#include "skizzay/fsm/events_list.h"
#include "skizzay/fsm/type_list.h"
#include <type_traits>

namespace skizzay::fsm {
namespace event_handler_details_ {
template <typename, typename>
struct has_on_template_member_function : std::false_type {};

template <typename T, concepts::event Event>
requires requires(T &t, Event const &event) {
  { t.on(event) } -> concepts::boolean;
}
struct has_on_template_member_function<T, Event> : std::true_type {};
} // namespace event_handler_details_

namespace concepts {
template <typename T, typename... Events>
concept event_handler_for =
    (event<Events> && ...) &&
    (event_handler_details_::has_on_template_member_function<T,
                                                             Events>::value &&
     ...);

template <typename T>
concept event_handler = requires {
  typename events_list_t<T>;
}
&&(!empty_v<events_list_t<T>>)&&all_v<
    events_list_t<T>,
    curry<event_handler_details_::has_on_template_member_function,
          T>::template type>;
} // namespace concepts

template <typename T>
using is_event_handler = std::bool_constant<concepts::event_handler<T>>;

template <typename T, typename... Events>
using is_event_handler_for =
    std::bool_constant<concepts::event_handler_for<T, Events...>>;

} // namespace skizzay::fsm

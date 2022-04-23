#pragma once

#include "skizzay/fsm/event.h"
#include "skizzay/fsm/events_list.h"
#include "skizzay/fsm/type_list.h"

#include <type_traits>

namespace skizzay::fsm {
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

namespace concepts {
template <typename T, typename... Events>
concept event_engine_for = (event<Events> && ...) &&
                           (event_engine_details_::template_member_function<
                                T>::template post_event<Events>::value &&
                            ...);

template <typename T>
concept event_engine = requires {
  typename events_list_t<T>;
}
&&(!empty_v<events_list_t<T>>)&&all_v<
    events_list_t<T>,
    event_engine_details_::template_member_function<T>::template post_event>;
} // namespace concepts

template <typename T>
using is_event_engine = std::bool_constant<concepts::event_engine<T>>;

template <typename T, typename... Events>
using is_event_engine_for =
    std::bool_constant<concepts::event_engine_for<T, Events...>>;
} // namespace skizzay::fsm

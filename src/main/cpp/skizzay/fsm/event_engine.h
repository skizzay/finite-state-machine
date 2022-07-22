#pragma once

#include "skizzay/fsm/event.h"
#include "skizzay/fsm/events_list.h"
#include "skizzay/fsm/type_list.h"

#include <type_traits>

namespace skizzay::fsm {
namespace event_engine_details_ {
struct ignore_flag final {
  constexpr inline ignore_flag(bool const) noexcept {}
};

template <typename, typename>
struct has_post_event_template_member_function : std::false_type {};

template <typename T, concepts::event Event>
requires requires(T &t, Event &&event) { t.post_event((Event &&) event); }
struct has_post_event_template_member_function<T, Event> : std::true_type {};
} // namespace event_engine_details_

namespace concepts {
template <typename T, typename... Events>
concept event_engine_for =
    (event<Events> && ...) &&
    (event_engine_details_::has_post_event_template_member_function<
         T, Events>::value &&
     ...);

template <typename T>
concept event_engine = requires {
  typename events_list_t<T>;
}
&&(!empty_v<events_list_t<T>>)&&all_v<
    events_list_t<T>,
    curry<event_engine_details_::has_post_event_template_member_function,
          T>::template type>;
} // namespace concepts

template <typename T>
using is_event_engine = std::bool_constant<concepts::event_engine<T>>;

template <typename T, typename... Events>
using is_event_engine_for =
    std::bool_constant<concepts::event_engine_for<T, Events...>>;

} // namespace skizzay::fsm

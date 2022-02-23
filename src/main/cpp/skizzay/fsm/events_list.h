#pragma once

#include <skizzay/fsm/event.h>
#include <type_traits>

namespace skizzay::fsm {
template <typename> struct is_events_list : std::false_type {};

template <template <typename...> typename Template, concepts::event... Events>
struct is_events_list<Template<Events...>> : std::true_type {};

namespace concepts {
template <typename T>
concept events_list = is_events_list<T>::value;
} // namespace concepts

template <concepts::event... Events> struct events_list {};
} // namespace skizzay::fsm
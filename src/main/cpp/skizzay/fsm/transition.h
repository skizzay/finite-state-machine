#pragma once

#include <skizzay/fsm/event.h>
#include <skizzay/fsm/state.h>
#include <type_traits>

namespace skizzay::fsm {
template <typename> struct is_transition : std::false_type {};

template <typename T>
requires concepts::event<typename T::event_type> &&
    concepts::state<typename T::current_state_type> &&
    concepts::state<typename T::next_state_type>
struct is_transition<T> : std::true_type {
};

namespace concepts {
template <typename T>
concept transition = is_transition<T>::value;
} // namespace concepts

} // namespace skizzay::fsm
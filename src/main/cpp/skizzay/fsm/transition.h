#pragma once

#include "skizzay/fsm/event.h"
#include "skizzay/fsm/state.h"

#include <concepts>
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

template <typename T>
concept self_transition = transition<T> &&
    std::same_as<typename T::current_state_type, typename T::next_state_type>;
} // namespace concepts

template <typename T>
struct is_self_transition : std::bool_constant<concepts::self_transition<T>> {};

} // namespace skizzay::fsm
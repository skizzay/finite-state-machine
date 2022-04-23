#pragma once

#include "skizzay/fsm/event.h"
#include "skizzay/fsm/state.h"

#include <concepts>
#include <type_traits>

namespace skizzay::fsm {
namespace concepts {
template <typename T>
concept transition = requires {
  typename event_t<T>;
  typename current_state_t<T>;
  typename next_state_t<T>;
};

template <typename T>
concept self_transition =
    transition<T> && std::same_as<current_state_t<T>, next_state_t<T>>;
} // namespace concepts

template <typename T>
using is_transition = std::bool_constant<concepts::transition<T>>;

template <typename T>
using is_self_transition = std::bool_constant<concepts::self_transition<T>>;

} // namespace skizzay::fsm
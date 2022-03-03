#pragma once

#include "skizzay/fsm/state_provider.h"
#include "skizzay/fsm/state_queryable.h"

#include <type_traits>

namespace skizzay::fsm {
template <typename T>
struct is_state_accessible
    : std::conjunction<is_state_queryable<T>, is_state_provider<T>> {};

namespace concepts {
template <typename T>
concept state_accessible = is_state_accessible<T>::value;
} // namespace concepts

} // namespace skizzay::fsm

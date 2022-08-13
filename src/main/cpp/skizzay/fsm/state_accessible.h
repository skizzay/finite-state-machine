#pragma once

#include "skizzay/fsm/state_provider.h"
#include "skizzay/fsm/state_queryable.h"

#include <type_traits>

namespace skizzay::fsm {
namespace concepts {
template <typename T>
concept state_accessible = state_queryable<T> && state_provider<T>;
} // namespace concepts

template <typename T>
using is_state_accessible = std::bool_constant<concepts::state_accessible<T>>;

} // namespace skizzay::fsm

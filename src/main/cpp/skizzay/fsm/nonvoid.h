#pragma once

#include <type_traits>

namespace skizzay::fsm {
namespace concepts {
template <typename T>
concept nonvoid = std::negation_v<std::is_void<T>>;
} // namespace concepts

} // namespace skizzay::fsm
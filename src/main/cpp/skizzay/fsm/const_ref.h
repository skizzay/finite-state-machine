#pragma once

#include <type_traits>

namespace skizzay::fsm {
template <typename T>
using add_cref_t = std::add_lvalue_reference_t<std::add_const_t<T>>;
} // namespace skizzay::fsm

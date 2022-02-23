#pragma once

#include <type_traits>

namespace skizzay::fsm
{
template <typename T>
struct is_state
    : std::conjunction<
          std::is_destructible<T>,
          std::negation<std::disjunction<std::is_void<T>, std::is_pointer<T>>>,
          std::disjunction<std::is_copy_constructible<T>,
                           std::is_move_constructible<T>>> {};

template <typename T>
struct is_state<T &> : is_state<std::remove_const_t<T>> {};

template <typename T>
struct is_state<T &&> : is_state<std::remove_const_t<T>> {};

namespace concepts
{
   template<typename T>
   concept state = is_state<T>::value;
} // namespace concepts

} // namespace skizzay::fsm

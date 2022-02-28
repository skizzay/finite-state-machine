#pragma once

#include <skizzay/fsm/traits.h>
#include <skizzay/fsm/transition.h>
#include <skizzay/fsm/type_list.h>

namespace skizzay::fsm {
template <typename T>
struct is_transition_table
    : std::conjunction<is_template<std::remove_cvref_t<T>, std::tuple>,
                       all<std::remove_cvref_t<T>, is_transition>> {};

namespace concepts {
template <typename T>
concept transition_table = is_transition_table<T>::value;

template <typename Transition, typename TransitionTable>
concept transition_in =
    transition<Transition> && transition_table<TransitionTable> &&
    contains_v<TransitionTable, Transition>;
} // namespace concepts

} // namespace skizzay::fsm

#pragma once

#include "skizzay/fsm/traits.h"
#include "skizzay/fsm/transition.h"
#include "skizzay/fsm/type_list.h"

namespace skizzay::fsm {
template <typename> struct is_transition_table : std::false_type {};

template <typename... Ts>
struct is_transition_table<std::tuple<Ts...>>
    : all<std::tuple<std::remove_cvref_t<Ts>...>, is_transition> {};

namespace concepts {
template <typename T>
concept transition_table = is_transition_table<T>::value;

template <typename Transition, typename TransitionTable>
concept transition_in = transition<std::remove_cvref_t<Transition>> &&
    transition_table<TransitionTable> &&
    contains_v<TransitionTable, Transition>;
} // namespace concepts

template <typename Transition, typename TransitionTable>
struct is_transition_in
    : std::bool_constant<concepts::transition_in<Transition, TransitionTable>> {
};

} // namespace skizzay::fsm

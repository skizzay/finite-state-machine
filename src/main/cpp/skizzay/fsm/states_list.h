#pragma once

#include <skizzay/fsm/state.h>
#include <skizzay/fsm/type_list.h>

#include <type_traits>

namespace skizzay::fsm {
template <typename> struct is_states_list : std::false_type {};

template <template <typename...> typename Template, concepts::state... States>
struct is_states_list<Template<States...>> : std::true_type {};

namespace concepts {
template <typename T>
concept states_list = is_states_list<T>::value;

template <typename State, typename StatesList>
concept state_in =
    state<State> && states_list<State> && contains_v<StatesList, State>;
} // namespace concepts

template <concepts::state... States> struct states_list {};
} // namespace skizzay::fsm

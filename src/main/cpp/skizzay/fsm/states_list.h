#pragma once

#include <skizzay/fsm/state.h>
#include <type_traits>

namespace skizzay::fsm {
template <typename> struct is_states_list : std::false_type {};

template <template <typename...> typename Template, concepts::state... States>
struct is_states_list<Template<States...>> : std::true_type {};

namespace concepts {
template <typename T>
concept states_list = is_states_list<T>::value;
} // namespace concepts

template <concepts::state... States> struct states_list {};
} // namespace skizzay::fsm

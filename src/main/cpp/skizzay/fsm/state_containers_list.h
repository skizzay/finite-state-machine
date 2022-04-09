#pragma once

#include "skizzay/fsm/state_container.h"
#include "skizzay/fsm/type_list.h"
#include <type_traits>

namespace skizzay::fsm {
template <typename> struct is_state_containers_list : std::false_type {};
template <typename, typename> struct is_state_container_in : std::false_type {};

template <template <typename...> typename Template,
          concepts::state_container... StateContainers>
struct is_state_containers_list<Template<StateContainers...>> : std::true_type {
};

template <typename StateContainer, typename StateContainersList>
requires is_state_container<StateContainer>::value &&
    is_state_containers_list<StateContainersList>::
        value struct is_state_container_in<StateContainer, StateContainersList>
    : contains<StateContainersList, StateContainer> {
};

namespace concepts {
template <typename T>
concept state_containers_list = is_state_containers_list<T>::value;

template <typename StateContainer, typename StateContainersList>
concept state_container_in =
    is_state_container_in<StateContainer, StateContainersList>::value;
} // namespace concepts

template <concepts::state_container... StateContainers>
struct state_containers_list {};

} // namespace skizzay::fsm
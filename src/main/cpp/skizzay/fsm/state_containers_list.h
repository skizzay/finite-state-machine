#pragma once

#include "skizzay/fsm/state.h"
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

namespace state_containers_for_details_ {
template <typename, typename> struct impl {};

template <concepts::states_list StatesList> struct has_state_in {
  template <concepts::state_container StateContainer>
  using test = contains_any<states_list_t<StateContainer>, StatesList>;
};

template <concepts::state_containers_list StateContainersList,
          concepts::states_list StatesList>
struct impl<StateContainersList, StatesList> {
  using type =
      filter_t<StateContainersList, has_state_in<StatesList>::template test>;
};

template <concepts::state_containers_list StateContainersList,
          concepts::state State>
requires(!concepts::states_list<State>) struct impl<StateContainersList, State>
    : impl<StateContainersList, states_list<State>> {
};
} // namespace state_containers_for_details_

template <typename StateContainersList, typename... StatesOrStatesLists>
using state_containers_for_t =
    concat_t<as_container_t<typename state_containers_for_details_::impl<
                                StateContainersList, StatesOrStatesLists>::type,
                            state_containers_list>...>;

template <typename StateContainersList, typename... StatesOrStatesLists>
using state_container_for_t = front_t<
    state_containers_for_t<StateContainersList, StatesOrStatesLists...>>;

} // namespace skizzay::fsm
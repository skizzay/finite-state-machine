#pragma once

#include <concepts>
#include <skizzay/fsm/traits.h>
#include <skizzay/fsm/type_list.h>
#include <utility>

namespace skizzay::fsm::concepts {
template <typename T>
concept type_list = is_type_list<T>::value;

template <typename T>
concept list_of_events = type_list<T> && all_v<T, is_event>;

template <typename Event, typename EventsList>
concept event_in =
    event<Event> && list_of_events<EventsList> && contains_v<EventsList, Event>;

template <typename T>
concept events_domain = list_of_events<T> &&
    (0 < skizzay::fsm::template length<T>::value);

template <typename T>
concept list_of_states = type_list<T> && skizzay::fsm::all<T, is_state>::value;

template <typename State, typename StatesList>
concept state_in =
    state<State> && list_of_states<StatesList> && contains_v<StatesList, State>;

template <typename T>
concept states_domain = list_of_states<T> &&
    (0 < skizzay::fsm::length<T>::value);

template <typename T>
concept actionable_transition = is_actionable_transition<T>::value;

template <typename T>
concept self_transition = is_self_transition<T>::value;

template <typename Transition, typename TransitionTable>
concept transition_in =
    transition<Transition> && transition_table<TransitionTable> &&
    contains_v<TransitionTable, Transition>;

template <typename T>
concept transition_coordinator =
    skizzay::fsm::is_transition_coordinator<T>::value;

template <typename T>
concept entry_coordinator = is_entry_coordinator<T>::value;

template <typename T>
concept state_container = skizzay::fsm::is_state_container<T>::value;

template <typename T>
concept list_of_state_containers =
    type_list<T> && skizzay::fsm::all_v<T, is_state_container>;

template <typename T, typename StateContainerList>
concept state_container_in =
    state_container<T> && list_of_state_containers<StateContainerList> &&
    contains_v<StateContainerList, T>;

template <typename StateContainer, typename State>
concept state_container_for = state_container<StateContainer> && state<State> &&
    contains_v<states_list_t<StateContainer>, State>;
} // namespace skizzay::fsm::concepts
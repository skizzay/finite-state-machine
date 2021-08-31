#pragma once

#include <concepts>
#include <skizzay/fsm/traits.h>
#include <skizzay/fsm/type_list.h>
#include <utility>

namespace skizzay::fsm::concepts {
template <typename T>
concept type_list = is_type_list<T>::value;

template <typename T>
concept event = is_event<T>::value;

template <typename T>
concept list_of_events = type_list<T> && all_v<T, is_event>;

template <typename Event, typename EventsList>
concept event_in =
    event<Event> && list_of_events<EventsList> && contains_v<EventsList, Event>;

template <typename T>
concept events_domain = list_of_events<T> &&
    (0 < skizzay::fsm::template length<T>::value);

template <typename T>
concept state = is_state<T>::value;

template <typename T>
concept list_of_states = type_list<T> && skizzay::fsm::all<T, is_state>::value;

template <typename State, typename StatesList>
concept state_in =
    state<State> && list_of_states<StatesList> && contains_v<StatesList, State>;

template <typename T>
concept states_domain = list_of_states<T> &&
    (0 < skizzay::fsm::length<T>::value);

template <typename T>
concept transition = is_transition<T>::value;

template <typename T>
concept actionable_transition = is_actionable_transition<T>::value;

template <typename T>
concept self_transition = is_self_transition<T>::value;

template <typename T>
concept transition_table = is_transition_table<T>::value;

template <typename T>
concept machine = is_machine<T>::value;

template <typename M, typename T>
concept machine_for = is_machine_for<M, T>::value;

template <typename T>
concept ancestry = skizzay::fsm::is_ancestry<T>::value;

template <typename T>
concept child_ancestry = skizzay::fsm::is_child_ancestry<T>::value;

template <typename T>
concept parent_ancestry = skizzay::fsm::is_parent_ancestry<T>::value;

template <typename T>
concept full_ancestry = skizzay::fsm::is_full_ancestry<T>::value;

template <typename T>
concept machine_ancestry = skizzay::fsm::is_machine_ancestry<T>::value;

template <typename T, typename State>
concept child_ancestry_for =
    child_ancestry<T> && state<State> && std::same_as<child_state_t<T>, State>;

template <typename T>
concept transition_coordinator =
    skizzay::fsm::is_transition_coordinator<T>::value;

template <typename T>
concept state_container = skizzay::fsm::is_state_container<T>::value;

template <typename StateContainer, typename State>
concept state_container_for = state_container<StateContainer> && state<State> &&
    contains_v<states_list_t<StateContainer>, State>;

template <typename F, typename... Args>
concept predicate = std::is_nothrow_invocable_r_v<bool, F const, Args...>;
} // namespace skizzay::fsm::concepts
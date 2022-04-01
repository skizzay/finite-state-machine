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

namespace transition_table_details_ {
template <concepts::event Event> struct has_event {
  template <concepts::transition Transition>
  using test = std::is_convertible<Event const &,
                                   typename Transition::event_type const &>;
};

template <concepts::event Event, concepts::state State>
struct has_current_state {
  template <concepts::transition Transition>
  using test = std::conjunction<
      std::is_same<State, typename Transition::current_state_type>,
      typename has_event<Event>::template test<Transition>>;
};

template <typename State, typename TransitionTable, typename Event>
concept current_state_in =
    concepts::state<State> && concepts::transition_table<TransitionTable> &&
    concepts::event_in<Event, TransitionTable> &&
    any_v<TransitionTable, has_current_state<Event, State>::template test>;
} // namespace transition_table_details_

template <concepts::event Event, concepts::transition_table TransitionTable>
struct is_event_in<Event, TransitionTable>
    : any<TransitionTable,
          transition_table_details_::has_event<Event>::template test> {};

template <
    concepts::transition_table TransitionTable,
    concepts::event_in<TransitionTable> Event,
    transition_table_details_::current_state_in<TransitionTable, Event> State>
constexpr concepts::transition_table auto
get_transition_table_for_current_state(TransitionTable &transition_table,
                                       Event const &, State const &) noexcept {
  using candidate_transitions_list_type = as_container_t<
      filter_t<TransitionTable, transition_table_details_::has_current_state<
                                    Event, State>::template test>,
      simple_type_list>;

  return []<concepts::transition... Transitions>(
      TransitionTable & transition_table,
      simple_type_list<Transitions...>) noexcept {
    return std::tie(std::get<Transitions>(transition_table)...);
  }
  (transition_table, candidate_transitions_list_type{});
}

template <concepts::transition_table TransitionTable, concepts::event Event,
          concepts::state State>
requires(!transition_table_details_::current_state_in<State, TransitionTable,
                                                      Event>) constexpr std::
    tuple<> get_transition_table_for_current_state(TransitionTable &,
                                                   Event const &,
                                                   State const &) noexcept {
  return {};
}

} // namespace skizzay::fsm

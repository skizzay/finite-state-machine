#pragma once

#include <bitset>
#include <concepts>
#include <cstddef>
#include <iostream>
#include <skizzay/fsm/concepts.h>
#include <skizzay/fsm/entry_coordinator.h>
// #include <skizzay/fsm/transition_queue.h>
#include <skizzay/fsm/trigger.h>
#include <skizzay/fsm/type_list.h>
#include <stdexcept>
#include <tuple>
#include <type_traits>

namespace skizzay::fsm {

struct duplicate_source_transitions_found : std::domain_error {
  using domain_error::domain_error;
};

struct duplicate_target_transitions_found : std::domain_error {
  using domain_error::domain_error;
};

namespace transition_coordinator_details_ {

template <concepts::event Event> struct has_compatible_event {
  template <concepts::transition Transition>
  using test = std::is_convertible<std::remove_cvref_t<Event> const &,
                                   event_t<Transition> const &>;
};

template <typename TransitionTable, typename Event>
using candidate_transition_table_t =
    filter_t<TransitionTable, has_compatible_event<Event>::template test>;

template <typename TransitionTable>
using candidate_entry_states_list_t = as_container_t<
    unique_t<map_t<
        filter_t<TransitionTable, negate<is_self_transition>::template apply>,
        next_state_t>>,
    states_list>;

template <typename State> struct is_current_state {
  template <typename Transition>
  using for_transition = std::is_same<State, current_state_t<Transition>>;
};

template <concepts::machine Machine,
          concepts::event_in<events_list_t<Machine>> Event>
struct impl {
  using transition_table_type =
      candidate_transition_table_t<transition_table_t<Machine>, Event>;

  constexpr explicit impl(
      transition_table_t<Machine> &transition_table) noexcept
      : transition_table_{transition_table} {}

  template <concepts::transition_in<transition_table_type> Transition>
  [[noreturn]] constexpr void on_transition(Transition &, Machine &,
                                            Event const &) {
    std::cerr << "Root of FSM attempting to transition to a state outside of "
                 "the FSM.";
    std::terminate();
  }

  template <concepts::state_in<next_states_list_t<transition_table_type>> State>
  constexpr void schedule_entry() noexcept {
    entry_coordinator_.template schedule_entry<State>();
  }

  template <
      concepts::state_in<current_states_list_t<transition_table_type>> State>
  constexpr auto get_transitions(State const &) noexcept {
    using transitions_for_state_type = as_container_t<
        filter_t<transition_table_type,
                 is_current_state<State>::template for_transition>,
        transitions_list>;
    return []<template <typename...> typename Template,
              concepts::transition... Transitions>(
        transition_table_t<Machine> & transition_table,
        Template<Transitions...> const) noexcept {
      return std::tie((std::get<Transitions>(transition_table), ...));
    }
    (transition_table_, transitions_for_state_type{});
  }

  constexpr std::tuple<>
  get_transitions(concepts::state auto const &) noexcept {
    return {};
  }

private:
  transition_table_t<Machine> &transition_table_;
  entry_coordinator<next_states_list_t<transition_table_type>>
      entry_coordinator_;
};

// template <typename Event> struct is_event {
//   template <typename Transition>
//   using for_transition = std::is_convertible<std::remove_cvref_t<Event> const
//   &,
//                                              event_t<Transition> const &>;
// };

// template <typename State, typename TransitionTable, typename Event>
// concept current_state_for =
//     concepts::state<State> && concepts::transition_table<TransitionTable> &&
//     concepts::event<Event> && contains_any_v<
//         filter_t<TransitionTable,
//                  is_current_state<State>::template for_transition>,
//         filter_t<TransitionTable, is_event<Event>::template for_transition>>;

// template <typename Event, typename TransitionTable>
// concept event_of =
//     concepts::event<Event> && concepts::transition_table<TransitionTable> &&
//     contains_v<events_list_t<TransitionTable>, Event>;

// template <typename TransitionTable, typename Event, typename State>
// class transition_type_finder {
//   template <std::size_t I>
//   using transition_at_t = element_at_t<I, TransitionTable>;

//   template <std::size_t I>
//   inline static constexpr bool const is_applicable_transition_at_v =
//       std::conjunction_v<typename is_current_state<State>::
//                              template for_transition<transition_at_t<I>>,
//                          typename is_event<Event>::template for_transition<
//                              transition_at_t<I>>>;

//   template <std::size_t I>
//   using applicable_transition_index =
//       std::conditional_t<is_applicable_transition_at_v<I>,
//                          std::index_sequence<I>, std::index_sequence<>>;

//   template <typename> struct applicable_transition_indices;

//   template <std::size_t... Is>
//   struct applicable_transition_indices<std::index_sequence<Is...>> {
//     using type = concat_t<applicable_transition_index<Is>...>;
//   };

//   template <typename> struct applicable_transition_types;

//   template <std::size_t... Is>
//   struct applicable_transition_types<std::index_sequence<Is...>> {
//     using type = std::tuple<element_at_t<Is, TransitionTable>...>;
//   };

// public:
//   using indices = typename applicable_transition_indices<
//       std::make_index_sequence<length_v<TransitionTable>>>::type;

//   using types = typename applicable_transition_types<indices>::type;
// };

// template <typename TransitionTable, typename Event, typename State>
// using transition_indices_for =
//     typename transition_type_finder<TransitionTable, Event, State>::indices;

// template <typename TransitionTable, typename Event, typename State>
// using transition_types_for =
//     typename transition_type_finder<TransitionTable, Event, State>::types;

// template <typename TransitionTable, template <typename> typename
// ExtractState,
//           std::derived_from<std::exception> Exception>
// class state_transition_tracker {
//   using states_domain_list =
//       as_container_t<unique_t<map_t<TransitionTable, ExtractState>>,
//                      states_list>;
//   template <std::size_t I>
//   inline static constexpr std::size_t const state_index =
//       index_of_v<states_domain_list,
//                  ExtractState<element_at_t<I, TransitionTable>>>;

//   std::bitset<length_v<states_domain_list>> states_encountered;

// public:
//   template <std::size_t I> constexpr void validate() const {
//     if (states_encountered.test(state_index<I>)) {
//       throw Exception{"Transition for state already established"};
//     }
//   }

//   template <std::size_t I> constexpr void mark_encountered() noexcept {
//     states_encountered.set(state_index<I>);
//   }
// };

// template <typename Event> struct is_compatible_event {
//   template <typename T>
//   using test = std::is_convertible<T const &, Event const &>;
// };

// template <typename TransitionTable, typename Event>
// inline constexpr std::size_t const num_transitions_for_event_v =
//     length_v<filter_t<events_list_t<TransitionTable>,
//                       is_compatible_event<Event>::template test>>;

// template <concepts::machine Machine,
//           concepts::event_in<events_list_t<Machine>> Event>
// class coordinator {
//   transition_table_t<Machine> &transition_table_;
//   Event const &event_;
//   state_transition_tracker<transition_table_t<Machine>, current_state_t,
//                            duplicate_source_transitions_found>
//       current_states_;
//   state_transition_tracker<transition_table_t<Machine>, next_state_t,
//                            duplicate_target_transitions_found>
//       next_states_;
//   transition_queue<Machine, Event> scheduled_transitions_;

//   template <std::size_t I> constexpr void validate() const {
//     current_states_.template validate<I>();
//     next_states_.template validate<I>();
//   }

//   template <std::size_t I> constexpr void mark_encountered() {
//     current_states_.template mark_encountered<I>();
//     next_states_.template mark_encountered<I>();
//   }

//   template <concepts::transition Transition>
//   requires concepts::self_transition<Transition>
//   constexpr auto make_transition_function(Transition &transition) {
//     return [&transition](Machine &machine, Event const &event) {
//       trigger(transition, event);
//       machine.template on_reentry<current_state_t<Transition>>(event);
//     };
//   }

//   template <concepts::transition Transition>
//   constexpr auto make_transition_function(Transition &transition) {
//     return [&transition](Machine &machine, Event const &event) {
//       machine.template on_exit<current_state_t<Transition>,
//                                next_state_t<Transition>>(event);
//       trigger(transition, event);
//       machine.template on_entry<current_state_t<Transition>,
//                                 next_state_t<Transition>>(event);
//     };
//   }

//   template <std::size_t I, typename State>
//   constexpr bool schedule_transition_if_accepted_at(State const &state,
//                                                     Machine const &machine) {
//     auto &transition = std::get<I>(transition_table_);
//     if (std::as_const(transition).accepts(state, machine, event_)) {
//       this->template schedule_transition<I>();
//       return true;
//     } else {
//       return false;
//     }
//   }

// public:
//   using machine_type = Machine;
//   using event_type = Event;

//   constexpr explicit coordinator(transition_table_t<Machine>
//   &transition_table,
//                                  Event const &event) noexcept
//       : transition_table_{transition_table}, event_{event} {}

//   template <std::size_t I>
//   requires(I <
//            length_v<transition_table_t<Machine>>) void schedule_transition()
//            {
//     std::as_const(*this).template validate<I>();
//     this->template mark_encountered<I>();
//     this->scheduled_transitions_.enqueue(
//         this->make_transition_function(std::get<I>(transition_table_)));
//   }

//   template <current_state_for<transition_table_t<Machine>, Event> State>
//   constexpr bool schedule_accepted_transitions(State const &state,
//                                                Machine const &machine) {
//     return [&]<std::size_t... Is>(std::index_sequence<Is...> const) {
//       return std::apply(
//           [](std::same_as<bool> auto const... values) noexcept {
//             return (values || ... || false);
//           },
//           std::tuple{this->template schedule_transition_if_accepted_at<Is>(
//               state, machine)...});
//     }
//     (transition_indices_for<transition_table_t<Machine>, Event, State>{});
//   }

//   template <concepts::state State>
//   constexpr bool schedule_accepted_transitions(State const &,
//                                                Machine const &) noexcept {
//     return false;
//   }

//   constexpr void execute_scheduled_transitions(Machine &machine) {
//     this->scheduled_transitions_.execute(machine, event_);
//   }
// };
} // namespace transition_coordinator_details_

template <concepts::machine Machine,
          concepts::event_in<events_list_t<Machine>> Event>
using transition_coordinator =
    transition_coordinator_details_::impl<Machine, Event>;
} // namespace skizzay::fsm
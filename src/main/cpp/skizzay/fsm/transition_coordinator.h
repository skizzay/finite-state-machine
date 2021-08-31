#pragma once

#include <bitset>
#include <concepts>
#include <cstddef>
#include <skizzay/fsm/concepts.h>
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

template <typename State> struct is_current_state {
  template <typename Transition>
  using for_transition = std::is_same<State, current_state_t<Transition>>;
};

template <typename Event> struct is_event {
  template <typename Transition>
  using for_transition = std::is_convertible<std::remove_cvref_t<Event> const &,
                                             event_t<Transition> const &>;
};

template <typename State, typename TransitionTable, typename Event>
concept current_state_for =
    concepts::state<State> && concepts::transition_table<TransitionTable> &&
    concepts::event<Event> && contains_any_v<
        filter_t<TransitionTable,
                 is_current_state<State>::template for_transition>,
        filter_t<TransitionTable, is_event<Event>::template for_transition>>;

template <typename Event, typename TransitionTable>
concept event_of = event<Event> && transition_table<TransitionTable> &&
    contains_v<events_list_t<Transitiontable>, Event>;

template <typename TransitionTable, typename Event, typename State>
class transition_indices_finder {
  template <std::size_t I>
  using transition_at_t = element_at_t<I, TransitionTable>;

  template <std::size_t I>
  inline static constexpr bool const is_applicable_transition_at_v =
      std::conjunction_v<
          is_current_state<State>::template for_transtion<transition_at_t<I>>,
          is_event<Event>::template for_transition<transition_at_t<I>>>;

  template <std::size_t I>
  using applicable_transition_index =
      std::conditional_t<is_applicable_transition_at_v<I>,
                         std::index_sequence<I>, std::index_sequence<>>;

  template <typename> struct application_transition_indices;

  template <std::size_t... Is>
  struct applicable_transition_indices<std::index_sequence<Is...>> {
    using type = concat_t<applicable_transition_index<Is>...>;
  };

public:
  using type = typename applicable_transition_indices<
      std::make_index_sequence<length_v<TransitionTable>>>::type;
};

template <typename TransitionTable, typename Event, typename State>
using transition_indices_for =
    typename transition_indices_finder<TransitionTable, Event, State>::type;

template <typename TransitionTable, template <typename> typename ExtractState,
          std::derived_from<std::exception> Exception>
class state_transition_tracker {
  using states_domain_list = unique_t<map_t<TransitionTable, ExtractState>>;
  template <std::size_t I>
  inline static constexpr std::size_t const state_index =
      index_of_v<unique_t<map_t<TransitionTable, ExtractState>>,
                 element_at_t<I, TransitionTable>>;

  std::bitset<length_v<states_domain_list>> states_encountered;

public:
  template <std::size_t I> constexpr void validate() const {
    if (states_encountered.test(state_index<I>)) {
      throw Exception{"Transition for state already established"};
    }
  }

  template <std::size_t I> constexpr void mark_encountered() noexcept {
    states_encountered.set(state_index<I>);
  }
};

template <concepts::transition_table TransitionTable,
          event_of<TransitionTable> Event>
class coordinator {
  state_transition_tracker<TransitionTable, current_state_t,
                           duplicate_source_transitions_found>
      current_states_;
  state_transition_tracker<TransitionTable, next_state_t,
                           duplicate_target_transitions_found>
      next_states_;

  template <std::size_t I> constexpr void validate() const {
    current_states_.template validate<I>();
    next_states_.template validate<I>();
  }

  template <std::size_t I> constexpr void mark_encountered() {
    current_states_.template mark_encountered<I>();
    next_states_.template mark_encountered<I>();
  }

  template <std::invocable<Event const &> TransitionFunction>
  void schedule_transition(
      [[maybe_unused]] TransitionFunction &&transition_function) {}

  template <concepts::self_transition Transition>
  constexpr auto make_transition_function(Transition &transition) {
    return [&transition](Event const &event) {
      trigger(transition, event);
      machine_.template on_reentry<current_state_t<Transition>>(event);
    };
  }

  template <concepts::transition Transition>
  constexpr auto make_transition_function(Transition &transition) {
    return [&transition](Event const &event) {
      machine_.template on_exit<current_state_t<Transition>,
                                next_state_t<Transition>>(event);
      trigger(transition, event);
      machine_.template on_entry<current_state_t<Transition>,
                                 next_state_t<Transition>>(event);
    };
  }

  template <std::size_t I, typename State, typename Machine>
  constexpr bool schedule_transition_if_accepted_at(State const &state,
                                                    Machine const &machine) {
    auto &transition = std::get<I>(transition_table_);
    if (std::as_const(transition).accepts(state, machine, event)) {
      std::as_const(this)->template validate<I>();
      this->template mark_encountered<I>();
      this->schedule_transition(
          this->template make_transition_function(transition));
      return true;
    } else {
      return false;
    }
  }

public:
  template <current_state_for<TransitionTable, Event> State,
            concepts::machine Machine>
  constexpr bool schedule_accepted_transitions(State const &state,
                                               Machine const &machine) {
    return [&state, &
            machine ]<std::size_t... Is>(std::index_sequence<Is...> const) {
      return std::apply(
          [](std::same_as<bool> auto const... values) noexcept {
            return (values || ... || false);
          },
          std::tuple{this->template schedule_transition_if_accepted_at<Is>(
              state, machine)...});
    }
    (transition_indices_for<TransitionTable, Event, State>{});
  }

  template <concepts::state State, concepts::machine Machine>
  constexpr bool schedule_accepted_transitions(State const &, Machine const &) {
    return false;
  }
};
} // namespace transition_coordinator_details_

template <concepts::transition_table TransitionTable,
          transition_coordinator_details_::event_of<TransitionTable> Event>
using transition_coordinator =
    transition_coordinator_details_::coordinator<TransitionTable, Event>;
} // namespace skizzay::fsm
#pragma once

#include <bitset>
#include <skizzay/fsm/concepts.h>
#include <skizzay/fsm/node_transition_coordinator.h>
#include <skizzay/fsm/optional_reference.h>
#include <tuple>

namespace skizzay::fsm {

namespace orthogonal_state_container_details_ {

template <typename, typename, std::size_t> struct container_index_calc;

template <template <typename...> typename Template, typename Tn,
          typename... Tns, typename S, std::size_t N>
requires contains_v<states_list_t<Tn>, S>
struct container_index_calc<Template<Tn, Tns...>, S, N>
    : std::integral_constant<std::size_t, N> {
};

template <template <typename...> typename Template, typename Tn,
          typename... Tns, typename S, std::size_t N>
struct container_index_calc<Template<Tn, Tns...>, S, N>
    : container_index_calc<Template<Tns...>, S, N + 1> {};

template <typename StatesList> struct contains_entry_state {
  template <typename Container>
  using test =
      std::bool_constant<contains_any_v<StatesList, states_list_t<Container>>>;
};

template <typename TransitionCoordinator> struct is_state_container_for {
  template <typename StateContainer>
  using test =
      contains_any<states_list_t<transition_table_t<TransitionCoordinator>>,
                   states_list_t<StateContainer>>;
};

template <concepts::state_container... StateContainers> class container {
  static_assert(0 < sizeof...(StateContainers),
                "At least 1 state container must be defined");
  using tuple_type = std::tuple<StateContainers...>;
  tuple_type state_containers_;

  template <typename> friend struct transition_coordinator;
  template <typename ParentTransitionCoordinator>
  struct transition_coordinator
      : node_transition_coordinator<container<StateContainers...>,
                                    ParentTransitionCoordinator> {
    template <typename State>
    static constexpr std::size_t container_index =
        container_index_calc<typename container<StateContainers...>::tuple_type,
                             State, 0>::value;

    using node_transition_coordinator<
        container<StateContainers...>,
        ParentTransitionCoordinator>::node_transition_coordinator;

    template <concepts::transition Transition, concepts::machine Machine>
    constexpr void on_transition(Transition &transition, Machine &machine,
                                 event_t<Transition> const &event) {
      triggered_containers_.set(container_index<current_state_t<Transition>>);
      this->node_transition_coordinator<
          container<StateContainers...>,
          ParentTransitionCoordinator>::on_transition(transition, machine,
                                                      event);
    }

    template <concepts::state_container Container, concepts::machine Machine>
    constexpr void ensure_exited(Container &child_container, Machine &machine) {
      if (!triggered_containers_.test(
              index_of_v<typename container<StateContainers...>::tuple_type,
                         Container>)) {
        child_container.on_final_exit(machine);
      }
    }

  private:
    std::bitset<sizeof...(StateContainers)> triggered_containers_;
  };

public:
  using states_list_type = concat_t<states_list_t<StateContainers>...>;

  constexpr container() noexcept(
      std::is_nothrow_default_constructible_v<tuple_type>)
      : state_containers_{} {}

  constexpr explicit container(StateContainers &&...containers) noexcept(
      std::is_nothrow_constructible_v<tuple_type, StateContainers...>)
      : state_containers_{std::forward<StateContainers>(containers)...} {}

  template <concepts::state_in<states_list_type> S>
  constexpr bool is() const noexcept {
    return std::get<state_container_for_t<tuple_type, S>>(state_containers_)
        .template is<S>();
  }

  constexpr bool is_active() const noexcept {
    return std::get<0>(state_containers_).is_active();
  }

  constexpr bool is_inactive() const noexcept {
    return std::get<0>(state_containers_).is_inactive();
  }

  template <concepts::state_in<states_list_type> S>
  constexpr optional_reference<S const> current_state() const noexcept {
    return std::get<state_container_for_t<tuple_type, S>>(state_containers_)
        .template current_state<S>();
  }

  template <concepts::state_in<states_list_type> S>
  constexpr S const &get() const noexcept {
    return std::get<state_container_for_t<tuple_type, S>>(state_containers_)
        .template get<S>();
  }

  template <concepts::state_in<states_list_type> S>
  constexpr S &get() noexcept {
    return std::get<state_container_for_t<tuple_type, S>>(state_containers_)
        .template get<S>();
  }

  template <concepts::state_in<states_list_type> S, concepts::ancestry Ancestry>
  constexpr auto ancestry_to(Ancestry ancestry) noexcept {
    return std::get<state_container_for_t<tuple_type, S>>(state_containers_)
        .template ancestry_to<S>(ancestry);
  }

  template <concepts::machine Machine>
  constexpr void on_initial_entry(Machine &machine) {
    std::apply(
        [&machine](StateContainers &...state_containers) {
          (state_containers.on_initial_entry(machine), ...);
        },
        state_containers_);
  }

  template <concepts::machine Machine>
  constexpr void on_final_exit(Machine &machine) {
    std::apply(
        [&machine](StateContainers &...state_containers) {
          (state_containers.on_final_exit(machine), ...);
        },
        state_containers_);
  }

  template <concepts::entry_coordinator EntryCoordinator,
            concepts::machine Machine,
            concepts::event_in<events_list_t<Machine>> Event>
  constexpr void on_entry(EntryCoordinator const &entry_coordinator,
                          Machine &machine, Event const &event) {
    using scheduled_containers_list =
        typename EntryCoordinator::applicable_state_containers_list_type<
            tuple_type>;
    auto const enter_child_container =
        [&]<typename Container>(Container &child_container) {
          if constexpr (contains_v<scheduled_containers_list, Container>) {
            child_container.on_entry(entry_coordinator, machine, event);
          } else {
            if (child_container.is_inactive()) {
              child_container.on_initial_entry(machine);
            }
          }
        };
    std::apply(
        [enter_child_container](auto &...containers) {
          (enter_child_container(containers), ...);
        },
        state_containers_);
  }

  template <typename ParentTransitionCoordinator, concepts::machine Machine,
            concepts::event_in<events_list_t<Machine>> Event>
  constexpr bool
  on_event(ParentTransitionCoordinator &parent_transition_coordinator,
           Machine &machine, Event const &event) {
    using candidate_states_containers_list = as_container_t<
        filter_t<tuple_type, is_state_container_for<
                                 ParentTransitionCoordinator>::template test>,
        state_containers_list>;
    transition_coordinator<ParentTransitionCoordinator> coordinator{
        parent_transition_coordinator, *this};
    auto const result = [&]<template <typename...> typename Template,
                            typename... FoundStateContainers>(
        Template<FoundStateContainers...> const) {
      return std::tuple{std::get<FoundStateContainers>(state_containers_)
                            .on_event(coordinator, machine, event)...};
    }
    (candidate_states_containers_list{});
    if (coordinator.will_exit_container()) {
      std::apply(
          [&](auto &...containers) {
            (coordinator.ensure_exited(containers, machine), ...);
          },
          state_containers_);
    }
    return std::apply(
        [](std::same_as<bool> auto const... values) noexcept {
          return (values || ... || false);
        },
        result);
  }
};
} // namespace orthogonal_state_container_details_

template <concepts::state_container... StateContainers>
using orthogonal_states =
    orthogonal_state_container_details_::container<StateContainers...>;
} // namespace skizzay::fsm
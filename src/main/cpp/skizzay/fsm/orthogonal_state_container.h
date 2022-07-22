#pragma once

#include "skizzay/fsm/entry_context.h"
#include "skizzay/fsm/event_context_node.h"
#include "skizzay/fsm/event_transition_context.h"
#include "skizzay/fsm/state.h"
#include "skizzay/fsm/state_container.h"
#include "skizzay/fsm/states_list.h"
#include "skizzay/fsm/type_list.h"
#include "skizzay/fsm/types.h"

#include <algorithm>
#include <bitset>
#include <cstddef>
#include <functional>
#include <tuple>
#include <type_traits>

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

  template <concepts::state State>
  constexpr auto &get_state_container_for() noexcept {
    return std::get<state_container_for_t<tuple_type, State>>(
        state_containers_);
  }

  template <concepts::state State>
  constexpr auto const &get_state_container_for() const noexcept {
    return std::get<state_container_for_t<tuple_type, State>>(
        state_containers_);
  }

  template <std::size_t N>
  static constexpr bool
  handled_event(std::array<bool, N> const values) noexcept {
    return [values]<std::size_t... Is>(
        std::index_sequence<Is...> const) noexcept {
      return (values[Is] || ...);
    }
    (std::make_index_sequence<N>{});
  }

  template <typename> friend struct entry_context;
  template <concepts::event_transition_context ParentEventTransitionContext>
  struct event_transition_context
      : event_context_node<ParentEventTransitionContext,
                           states_list_t<container<StateContainers...>>> {
    template <typename State>
    static constexpr std::size_t container_index =
        container_index_calc<typename container<StateContainers...>::tuple_type,
                             State, 0>::value;

    using event_context_node<
        ParentEventTransitionContext,
        states_list_t<container<StateContainers...>>>::event_context_node;

    template <concepts::transition_in<
        transition_table_t<ParentEventTransitionContext>>
                  Transition>
    constexpr void on_transition(Transition &transition,
                                 event_t<Transition> const &event) {
      triggered_containers_.set(container_index<current_state_t<Transition>>);
      this->event_context_node<ParentEventTransitionContext,
                               states_list_t<container<StateContainers...>>>::
          on_transition(transition, event);
    }

    template <concepts::state_container_in<tuple_type> StateContainer>
    constexpr void
    ensure_exited(StateContainer &child_container,
                  concepts::event_engine auto &event_engine,
                  concepts::state_provider auto &state_provider) {
      if (!triggered_containers_.test(
              index_of_v<typename container<StateContainers...>::tuple_type,
                         StateContainer>)) {
        execute_final_exit(child_container, event_engine, state_provider);
      }
    }

  private:
    std::bitset<sizeof...(StateContainers)> triggered_containers_;
  };

public:
  using states_list_type = flat_map_t<map_t<tuple_type, states_list_t>>;

  constexpr container() noexcept(
      std::is_nothrow_default_constructible_v<tuple_type>)
      : state_containers_{} {}

  constexpr explicit container(StateContainers &&...containers) noexcept(
      std::is_nothrow_constructible_v<tuple_type, StateContainers...>)
      : state_containers_{std::forward<StateContainers>(containers)...} {}

  template <concepts::state_in<states_list_type> S>
  constexpr bool is() const noexcept {
    return this->template get_state_container_for<S>().template is<S>();
  }

  template <concepts::state_in<states_list_type> S>
  constexpr optional_reference<S const> current_state() const noexcept {
    return this->template get_state_container_for<S>()
        .template current_state<S>();
  }

  template <concepts::state_in<states_list_type> S>
  constexpr S const &state() const noexcept {
    return this->template get_state_container_for<S>().template state<S>();
  }

  template <concepts::state_in<states_list_type> S>
  constexpr S &state() noexcept {
    return this->template get_state_container_for<S>().template state<S>();
  }

  constexpr bool is_active() const noexcept {
    return std::apply(
        [](concepts::state_container auto const &...state_containers) noexcept {
          return (state_containers.is_active() || ...);
        },
        state_containers_);
  }

  constexpr bool is_inactive() const noexcept {
    return std::apply(
        [](concepts::state_container auto const &...state_containers) noexcept {
          return (state_containers.is_inactive() && ...);
        },
        state_containers_);
  }

  template <concepts::query<states_list_type> Query>
  constexpr bool query(Query &&query) const
      noexcept(concepts::nothrow_query<Query, states_list_type>) {
    return [this]<std::size_t... Is>(
        Query && query,
        std::index_sequence<
            Is...> const) noexcept(concepts::nothrow_query<Query,
                                                           states_list_type>) {
      return !(
          !std::get<Is>(state_containers_).query(std::forward<Query>(query)) &&
          ...);
    }
    (std::forward<Query>(query),
     std::make_index_sequence<length_v<tuple_type>>{});
  }

  constexpr auto memento() const
      noexcept((is_memento_nothrow_v<StateContainers> && ...)) {
    return std::apply(
        [](StateContainers const &...state_containers) noexcept(
            (is_memento_nothrow_v<StateContainers> && ...)) {
          return std::tuple{state_containers.memento()...};
        },
        state_containers_);
  }

  constexpr void on_entry(concepts::state_schedule auto const &state_schedule,
                          initial_entry_event_t const &event,
                          concepts::event_engine auto event_engine,
                          concepts::state_provider auto state_provider) {
    std::apply(
        [&](StateContainers &...state_containers) {
          (state_containers.on_entry(state_schedule, event, event_engine,
                                     state_provider),
           ...);
        },
        state_containers_);
  }

  constexpr void on_entry(concepts::state_schedule auto const &state_schedule,
                          concepts::event auto const &event,
                          concepts::event_engine auto event_engine,
                          concepts::state_provider auto state_provider) {
    using scheduled_containers_list =
        state_containers_for_t<tuple_type,
                               next_states_list_t<decltype(state_schedule)>>;
    auto const enter_child_container =
        [&]<concepts::state_container StateContainer>(
            StateContainer &child_container) {
          if constexpr (contains_v<scheduled_containers_list, StateContainer>) {
            child_container.on_entry(state_schedule, event, event_engine,
                                     state_provider);
          } else {
            if (child_container.is_inactive()) {
              execute_initial_entry(child_container, event_engine,
                                    state_provider);
            }
          }
        };
    std::apply(
        [enter_child_container](concepts::state_container auto &...containers) {
          (enter_child_container(containers), ...);
        },
        state_containers_);
  }

  constexpr bool
  on_event(concepts::event_transition_context auto &event_transition_context,
           final_exit_event_t const &event,
           concepts::event_engine auto event_engine,
           concepts::state_provider auto state_provider) {
    return handled_event(std::apply(
        [&](StateContainers &...state_containers) {
          return std::array{
              (state_containers.on_event(event_transition_context, event,
                                         event_engine, state_provider),
               ...)};
        },
        state_containers_));
  }

  constexpr bool on_event(
      concepts::event_transition_context auto &parent_event_transition_context,
      concepts::event auto const &event,
      concepts::event_engine auto event_engine,
      concepts::state_provider auto state_provider) {
    using candidate_state_containers_list = as_container_t<
        state_containers_for_t<
            tuple_type,
            current_states_list_t<decltype(parent_event_transition_context)>>,
        state_containers_list>;

    event_transition_context<
        std::remove_cvref_t<decltype(parent_event_transition_context)>>
        child_event_transition_context{parent_event_transition_context};
    std::array const
        result = [&]<concepts::state_container... CandidateStateContainers>(
            tuple_type & state_containers,
            state_containers_list<CandidateStateContainers...> const) {
      return std::array<bool, sizeof...(CandidateStateContainers)>{
          std::get<CandidateStateContainers>(state_containers)
              .on_event(child_event_transition_context, event, event_engine,
                        state_provider)...};
    }
    (state_containers_, candidate_state_containers_list{});
    if (child_event_transition_context.will_exit_container()) {
      std::apply(
          [&](concepts::state_container auto &...state_containers) {
            (child_event_transition_context.ensure_exited(
                 state_containers, event_engine, state_provider),
             ...);
          },
          state_containers_);
    }
    return handled_event(result);
  }
};
} // namespace orthogonal_state_container_details_

template <concepts::state_container... StateContainers>
using orthogonal_states =
    orthogonal_state_container_details_::container<StateContainers...>;
} // namespace skizzay::fsm
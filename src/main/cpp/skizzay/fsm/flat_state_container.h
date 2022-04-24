#pragma once

#include "skizzay/fsm/entry_context.h"
#include "skizzay/fsm/event_context_node.h"
#include "skizzay/fsm/event_transition_context.h"
#include "skizzay/fsm/optional_reference.h"
#include "skizzay/fsm/state_container.h"
#include "skizzay/fsm/state_containers_list.h"
#include "skizzay/fsm/states_list.h"
#include "skizzay/fsm/transition_table.h"
#include "skizzay/fsm/type_list.h"

#include <array>
#include <cassert>
#include <cstddef>
#include <functional>
#include <tuple>
#include <utility>

namespace skizzay::fsm {

namespace flat_state_container_details_ {

template <concepts::states_list StatesList> struct contains_any_states {
  template <concepts::state_container StateContainer>
  using test = contains_any<StatesList, states_list_t<StateContainer>>;
};

// template <concepts::states_list StatesList,
//           concepts::state_container StateContainer>
// using contains_any_states =
//     contains_any<StatesList, states_list_t<StateContainer>>;

template <concepts::entry_context EntryContext,
          concepts::state_containers_list StateContainersList>
using next_state_containers_indices_t = as_index_sequence_t<
    filter_t<StateContainersList, contains_any_states<next_states_list_t<
                                      EntryContext>>::template test>,
    StateContainersList>;

template <concepts::state_container... StateContainers> class container {
  using tuple_type = std::tuple<StateContainers...>;
  template <concepts::state_in<tuple_type> State>
  static constexpr std::size_t const container_index_ =
      index_of_v<tuple_type, state_container_for_t<tuple_type, State>>;

  tuple_type state_containers_;
  std::size_t current_index_ = sizeof...(StateContainers);

  constexpr void reset_index() noexcept {
    current_index_ = sizeof...(StateContainers);
  }

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

  template <std::size_t... Is>
  constexpr std::array<std::size_t, sizeof...(Is)>
  get_next_state_container_indices(
      concepts::entry_context auto const &entry_context,
      std::index_sequence<Is...> const) const noexcept {
    return {{(has_state_scheduled_for_entry(entry_context,
                                            std::get<Is>(state_containers_))
                  ? Is
                  : sizeof...(StateContainers))...}};
  }

  template <std::size_t N>
  constexpr void validate_container_indices(
      std::array<std::size_t, N> const &indices) const noexcept {
    constexpr auto const is_invalid_index =
        [](std::size_t const index) noexcept {
          return index == sizeof...(StateContainers);
        };
    auto const end_iter = std::end(indices);
    auto const index_iter =
        std::find_if_not(std::begin(indices), end_iter, is_invalid_index);
    assert(end_iter != index_iter && "Next state container could not be found");
    assert(std::find_if_not(std::next(index_iter), end_iter,
                            is_invalid_index) == end_iter &&
           "Multiple state containers identified for entry");
  }

  template <std::size_t I, concepts::entry_context EntryContext>
  constexpr void maybe_enter_state_container(std::size_t const index,
                                             EntryContext &entry_context) {
    if (I == index) {
      std::get<I>(state_containers_).on_entry(entry_context);
      current_index_ = I;
    }
  }

  template <std::size_t N, concepts::entry_context EntryContext,
            typename StateContainerIndexSequence>
  requires(N == length_v<StateContainerIndexSequence>) constexpr void enter_state_container(
      std::array<std::size_t, N> const &indices, EntryContext &entry_context,
      StateContainerIndexSequence const) {
    [&]<std::size_t... Is>(std::index_sequence<Is...> const) {
      ((this->template maybe_enter_state_container<
           value_at_v<Is, StateContainerIndexSequence>>(indices[Is],
                                                        entry_context)),
       ...);
    }
    (std::make_index_sequence<N>{});
  }

  constexpr bool dispatch_event(
      concepts::event_transition_context auto &event_transition_context) {
    return [&]<std::size_t... Is>(std::index_sequence<Is...> const) {
      return (
          (Is == current_index_ && std::get<Is>(state_containers_)
                                       .on_event(event_transition_context)) ||
          ...);
    }
    (std::make_index_sequence<sizeof...(StateContainers)>{});
  }

public:
  using states_list_type = flat_map_t<map_t<tuple_type, states_list_t>>;

  constexpr container() noexcept(
      std::is_nothrow_default_constructible_v<tuple_type>) = default;

  constexpr explicit container(StateContainers &&...containers) noexcept(
      std::is_nothrow_constructible_v<tuple_type, StateContainers...>)
      : state_containers_{std::forward<StateContainers>(containers)...} {}

  template <concepts::state_in<states_list_type> S>
  constexpr bool is() const noexcept {
    return is_active() &&
           this->template get_state_container_for<S>().template is<S>();
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
    return sizeof...(StateContainers) != current_index_;
  }

  constexpr bool is_inactive() const noexcept {
    return sizeof...(StateContainers) == current_index_;
  }

  constexpr void on_entry(concepts::initial_entry_context auto &entry_context) {
    assert(is_inactive() && "Conducting initial entry into flat state "
                            "container which is already active");
    std::get<0>(state_containers_).on_entry(entry_context);
    current_index_ = 0;
  }

  template <concepts::entry_context EntryContext>
  constexpr void on_entry(EntryContext &entry_context) {
    constexpr auto const next_state_container_indices =
        next_state_containers_indices_t<EntryContext, tuple_type>{};
    std::array const indices = get_next_state_container_indices(
        entry_context, next_state_container_indices);
    validate_container_indices(indices);
    enter_state_container(indices, entry_context, next_state_container_indices);
  }

  constexpr bool on_event(concepts::final_exit_event_transition_context auto
                              &final_exit_event_context) {
    assert(is_active() && "Handling a final exit event in a flat state "
                          "container which is inactive");
    bool const result = dispatch_event(final_exit_event_context);
    reset_index();
    return result;
  }

  constexpr bool
  on_event(concepts::event_transition_context auto &event_transition_context) {
    event_context_node current_context{*this, event_transition_context};
    bool const result = dispatch_event(current_context);
    if (current_context.has_been_scheduled()) {
      reset_index();
    }
    return result;
  }
};
} // namespace flat_state_container_details_

template <concepts::state_container... StateContainers>
using flat_states =
    flat_state_container_details_::container<StateContainers...>;

} // namespace skizzay::fsm
#pragma once

#include <bitset>
#include <skizzay/fsm/concepts.h>

namespace skizzay::fsm {
namespace entry_coordinator_details_ {

template <typename T, typename U>
inline constexpr bool is_contained_v = contains_v<T, U>;
template <concepts::list_of_states StatesList,
          concepts::state_container StateContainer>
inline constexpr bool is_contained_v<StatesList, StateContainer> =
    contains_any_v<StatesList, states_list_t<StateContainer>>;

template <concepts::list_of_states StatesList> struct impl {
  using states_list_type = as_container_t<StatesList, states_list>;
  using entry_states_table = std::bitset<length_v<StatesList>>;
  template <typename T>
  using contains = std::bool_constant<is_contained_v<StatesList, T>>;
  template <typename T>
  static inline constexpr bool contains_v = is_contained_v<StatesList, T>;
  template <typename T>
  using applicable_state_containers_list_type =
      as_container_t<filter_t<T, impl<states_list_type>::template contains>,
                     state_containers_list>;

  template <concepts::state_in<StatesList> State>
  constexpr bool is_scheduled() const noexcept {
    return entry_states_table_.test(index_of_v<StatesList, State>);
  }

  template <concepts::state_container StateContainer>
  requires contains_v<StateContainer>
  constexpr bool has_scheduled_state(StateContainer const &) const noexcept {
    using applicable_states_list_type = intersection_of_t<
        states_list_type,
        as_container_t<states_list_t<StateContainer>, states_list>>;
    return [&]<template <typename...> typename Template,
               typename... ApplicableStates>(
        Template<ApplicableStates...> const) noexcept {
      return (this->template is_scheduled<ApplicableStates>() || ... || false);
    }
    (applicable_states_list_type{});
  }

  template <concepts::state_in<StatesList> State>
  constexpr void schedule_entry() noexcept {
    entry_states_table_.set(index_of_v<StatesList, State>);
  }

  template <concepts::list_of_state_containers StateContainersList>
  constexpr auto applicable_state_containers(
      StateContainersList const &state_containers) const noexcept {
    return []<template <typename...> typename Template,
              concepts::state_container... StateContainers>(
        Template<StateContainers...> const &) noexcept {
      return concat_t<
          applicable_state_containers_list_type<StateContainers>...>{};
    }
    (state_containers);
  }

private:
  entry_states_table entry_states_table_;
};

} // namespace entry_coordinator_details_

template <concepts::list_of_states StatesList>
using entry_coordinator = entry_coordinator_details_::impl<StatesList>;
} // namespace skizzay::fsm
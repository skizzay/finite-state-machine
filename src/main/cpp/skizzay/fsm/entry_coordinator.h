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
  using states_list_type = StatesList;
  using entry_states_table = std::bitset<length_v<StatesList>>;
  template <typename T>
  using contains = std::bool_constant<is_contained_v<StatesList, T>>;
  template <typename T>
  static inline constexpr bool contains_v = is_contained_v<StatesList, T>;

  template <concepts::state_in<StatesList> State>
  constexpr bool is_scheduled() const noexcept {
    return entry_states_table_.test(index_of_v<StatesList, State>);
  }

  template <concepts::state_in<StatesList> State>
  constexpr void schedule_entry() noexcept {
    entry_states_table_.set(index_of_v<StatesList, State>);
  }

private:
  entry_states_table entry_states_table_;
};

} // namespace entry_coordinator_details_

template <concepts::list_of_states StatesList>
using entry_coordinator = entry_coordinator_details_::impl<StatesList>;
} // namespace skizzay::fsm
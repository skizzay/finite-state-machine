#pragma once

#include "skizzay/fsm/states_list.h"
#include "skizzay/fsm/type_list.h"
#include <bitset>

namespace skizzay::fsm {

template <concepts::states_list NextStatesList> struct entry_state_scheduler {
  using next_states_list_type = NextStatesList;

  template <concepts::state_in<next_states_list_type> State>
  constexpr bool is_scheduled() const noexcept {
    return scheduled_states_.test(index_of_<State>);
  }

  template <concepts::state_in<next_states_list_type> State>
  constexpr void schedule_entry() noexcept {
    scheduled_states_.set(index_of_<State>);
  }

  constexpr void reset() noexcept { scheduled_states_.reset(); }

private:
  template <concepts::state_in<next_states_list_type> State>
  static inline constexpr std::size_t const index_of_ =
      index_of_v<next_states_list_type, State>;
  std::bitset<length_v<next_states_list_type>> scheduled_states_;
};
} // namespace skizzay::fsm
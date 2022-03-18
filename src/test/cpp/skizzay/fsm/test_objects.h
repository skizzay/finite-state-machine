#pragma once

#include "skizzay/fsm/event.h"
#include "skizzay/fsm/events_list.h"
#include "skizzay/fsm/states_list.h"
#include "skizzay/fsm/transition_table.h"
#include "skizzay/fsm/type_list.h"
#include "skizzay/fsm/types.h"

#include <array>
#include <cstddef>
#include <numeric>

namespace test_objects {
template <std::size_t> struct test_event { bool pass_acceptance = true; };

template <std::size_t Id, std::size_t NumEvents> struct test_state {
  std::size_t initial_entry_count = 0;
  std::size_t final_exit_count = 0;
  std::size_t epsilon_event_entry_count = 0;
  std::size_t epsilon_event_exit_count = 0;
  std::array<std::size_t, NumEvents> event_entry_count;
  std::array<std::size_t, NumEvents> event_reentry_count;
  std::array<std::size_t, NumEvents> event_exit_count;

  test_state() noexcept {
    event_entry_count.fill(0);
    event_reentry_count.fill(0);
    event_exit_count.fill(0);
  }

  auto total_event_entry_count() const noexcept {
    return std::accumulate(std::begin(event_entry_count),
                           std::end(event_entry_count),
                           epsilon_event_entry_count);
  }

  auto total_event_reentry_count() const noexcept {
    return std::accumulate(std::begin(event_reentry_count),
                           std::end(event_reentry_count), std::size_t{});
  }

  auto total_event_exit_count() const noexcept {
    return std::accumulate(std::begin(event_exit_count),
                           std::end(event_exit_count),
                           epsilon_event_exit_count);
  }

  void on_entry(skizzay::fsm::initial_entry_event_t const &) noexcept {
    initial_entry_count += 1;
  }
  template <std::size_t I> void on_entry(test_event<I> const &) noexcept {
    event_entry_count[I] += 1;
  }
  void on_entry(skizzay::fsm::epsilon_event_t const) noexcept {
    epsilon_event_entry_count += 1;
  }

  void on_exit(skizzay::fsm::final_exit_event_t const &) noexcept {
    final_exit_count += 1;
  }
  template <std::size_t I> void on_exit(test_event<I> const &) noexcept {
    event_exit_count[I] += 1;
  }
  void on_exit(skizzay::fsm::epsilon_event_t const) noexcept {
    epsilon_event_exit_count += 1;
  }

  template <std::size_t I> void on_reentry(test_event<I> const &) noexcept {
    event_reentry_count[I] += 1;
  }

  template <std::size_t I>
  bool accepts(test_event<I> const &event) const noexcept {
    return event.pass_acceptance;
  }
};

template <skizzay::fsm::concepts::event Event,
          skizzay::fsm::concepts::transition_table TransitionTable>
struct fake_event_transition_context {
  using event_type = Event;
  using transition_table_type = TransitionTable;
  using states_list_type = skizzay::fsm::states_list_t<transition_table_type>;
  using events_list_type = skizzay::fsm::events_list_t<transition_table_type>;

  event_type e;
  skizzay::fsm::as_container_t<states_list_type, std::tuple> states;
  [[no_unique_address]] transition_table_type transition_table;
  std::array<std::size_t, skizzay::fsm::length_v<events_list_type>>
      posted_event_counts;
  std::size_t posted_epsilon_events = 0;

  constexpr event_type const &event() const noexcept { return e; }

  template <skizzay::fsm::concepts::state_in<states_list_type> State>
  constexpr State &state() noexcept {
    return std::get<State>(states);
  }

  template <skizzay::fsm::concepts::state_in<states_list_type> State>
  constexpr State const &state() const noexcept {
    return std::get<State>(states);
  }

  template <std::size_t I>
  constexpr void post_event(test_event<I> const &) noexcept {
    posted_event_counts[I] += 1;
  }

  constexpr void post_event(skizzay::fsm::epsilon_event_t const &) noexcept {
    posted_epsilon_events += 1;
  }

  constexpr void on_transition(
      skizzay::fsm::concepts::transition_in<transition_table_type> auto
          &) noexcept {}

  constexpr skizzay::fsm::concepts::transition_table auto
  get_transitions(skizzay::fsm::concepts::state_in<states_list_type> auto const
                      &state) noexcept {
    return get_transition_table_for_current_state(transition_table, state, e);
  }
};
} // namespace test_objects
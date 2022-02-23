#pragma once

#include "skizzay/fsm/event.h"

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

// template <typename FakeMachine, std::size_t I>
// struct fake_transition_coordinator
//     : skizzay::fsm::transition_coordinator<FakeMachine, test_event<I>> {
//   using transition_table_type = skizzay::fsm::transition_table_t<
//       skizzay::fsm::transition_coordinator<FakeMachine, test_event<I>>>;

//   fake_transition_coordinator(FakeMachine &machine,
//                               test_event<I> const &) noexcept
//       : skizzay::fsm::transition_coordinator<FakeMachine, test_event<I>>{
//             machine.transition_table} {
//     transition_activations.fill(0);
//   }

//   std::array<std::size_t, skizzay::fsm::length_v<transition_table_type>>
//       transition_activations;

//   template <
//       skizzay::fsm::concepts::transition_in<transition_table_type>
//       Transition>
//   requires(!skizzay::fsm::concepts::self_transition<
//            Transition>) constexpr void on_transition(Transition &,
//                                                      FakeMachine &,
//                                                      test_event<I> const &) {
//     std::get<skizzay::fsm::index_of_v<transition_table_type, Transition>>(
//         transition_activations) += 1;
//   }
// };

// template <skizzay::fsm::concepts::transition_table TransitionTable>
// struct fake_machine {
//   using states_list_type = skizzay::fsm::states_list_t<TransitionTable>;
//   using events_list_type = skizzay::fsm::events_list_t<TransitionTable>;
//   using transition_table_type = TransitionTable;
//   template <typename T>
//   using contains =
//   std::bool_constant<skizzay::fsm::entry_coordinator_details_::
//                                           is_contained_v<states_list_type,
//                                           T>>;

//   transition_table_type transition_table;

//   void start() noexcept {}
//   void stop() noexcept {}
//   template <std::size_t I>
//   requires skizzay::fsm::concepts::event_in<test_event<I>, events_list_type>
//   bool on(test_event<I> const &) { return true; }
//   template <skizzay::fsm::concepts::state... State> bool is() const noexcept
//   {
//     return false;
//   }
// };
} // namespace test_objects
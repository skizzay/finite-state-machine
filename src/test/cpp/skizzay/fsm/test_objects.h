#pragma once

#include "skizzay/fsm/event.h"
#include "skizzay/fsm/events_list.h"
#include "skizzay/fsm/states_list.h"
#include "skizzay/fsm/transition_table.h"
#include "skizzay/fsm/type_list.h"

#include <array>
#include <cstddef>
#include <numeric>

namespace test_objects {
template <std::size_t> struct test_event { bool pass_acceptance = true; };

namespace test_events_list_details_ {
template <typename> struct impl {};
template <std::size_t... Is> struct impl<std::index_sequence<Is...>> {
  using type = skizzay::fsm::events_list<test_event<Is>...>;
};
} // namespace test_events_list_details_

template <std::size_t N>
using test_events_list =
    typename test_events_list_details_::impl<std::make_index_sequence<N>>::type;

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
  bool is_accepted(test_event<I> const &event) const noexcept {
    return event.pass_acceptance;
  }
};

namespace test_states_list_details_ {
template <std::size_t, typename> struct impl {};

template <std::size_t NumEvents, std::size_t... Ids>
struct impl<NumEvents, std::index_sequence<Ids...>> {
  using type = skizzay::fsm::states_list<test_state<Ids, NumEvents>...>;
};
} // namespace test_states_list_details_

template <std::size_t NumEvents, std::size_t NumStates>
using test_states_list = typename test_states_list_details_::impl<
    NumEvents, std::make_index_sequence<NumStates>>::type;

template <skizzay::fsm::concepts::events_list EventsList>
struct fake_event_engine {
  using events_list_type = EventsList;

  std::array<std::size_t, skizzay::fsm::length_v<events_list_type>>
      posted_event_counts;
  std::size_t posted_epsilon_events = 0;

  template <std::size_t I>
  constexpr void post_event(test_event<I> const &) noexcept {
    posted_event_counts[I] += 1;
  }

  constexpr void post_event(skizzay::fsm::epsilon_event_t const &) noexcept {
    posted_epsilon_events += 1;
  }
};

template <skizzay::fsm::concepts::event Event,
          skizzay::fsm::concepts::states_list StatesList>
struct fake_context {
  using event_type = Event;
  using states_list_type = StatesList;

  event_type e;
  skizzay::fsm::as_container_t<states_list_type, std::tuple> states;

  constexpr event_type const &event() const noexcept { return e; }

  template <skizzay::fsm::concepts::state_in<states_list_type> State>
  constexpr State &state() noexcept {
    return std::get<State>(states);
  }

  template <skizzay::fsm::concepts::state_in<states_list_type> State>
  constexpr State const &state() const noexcept {
    return std::get<State>(states);
  }
};

template <skizzay::fsm::concepts::event Event,
          skizzay::fsm::concepts::transition_table TransitionTable,
          skizzay::fsm::concepts::states_list StatesList =
              skizzay::fsm::states_list_t<TransitionTable>,
          skizzay::fsm::concepts::events_list EventsList =
              skizzay::fsm::events_list_t<TransitionTable>>
struct fake_event_transition_context : fake_event_engine<EventsList>,
                                       fake_context<Event, StatesList> {
  using transition_table_type = TransitionTable;
  using states_list_type =
      skizzay::fsm::states_list_t<fake_context<Event, StatesList>>;

  [[no_unique_address]] transition_table_type transition_table;

  constexpr void on_transition(
      skizzay::fsm::concepts::transition_in<transition_table_type> auto
          &) noexcept {}

  constexpr skizzay::fsm::concepts::transition_table auto
  get_transitions(skizzay::fsm::concepts::state_in<states_list_type> auto const
                      &state) noexcept {
    return get_transition_table_for_current_state(transition_table,
                                                  this->event(), state);
  }

  constexpr std::tuple<>
  get_transitions(skizzay::fsm::concepts::state auto const &) const noexcept {
    return {};
  }

  template <skizzay::fsm::concepts::state_in<
      skizzay::fsm::next_states_list_t<transition_table_type>>
                State>
  constexpr void schedule_entry() noexcept {}
};

template <skizzay::fsm::concepts::event Event,
          skizzay::fsm::concepts::states_list StatesList,
          skizzay::fsm::concepts::events_list EventsList,
          skizzay::fsm::concepts::states_list NextStatesList = StatesList,
          skizzay::fsm::concepts::states_list ScheduledNextStatesList =
              NextStatesList>
struct fake_entry_context : fake_event_engine<EventsList>,
                            fake_context<Event, StatesList> {
  static_assert(
      skizzay::fsm::contains_all_v<NextStatesList, ScheduledNextStatesList>,
      "Scheduled states must be a subset of next states");
  using next_states_list_type = NextStatesList;

  template <skizzay::fsm::concepts::state_in<next_states_list_type> State>
  constexpr bool is_scheduled() const noexcept {
    return skizzay::fsm::concepts::state_in<State, ScheduledNextStatesList>;
  }
};

template <std::size_t NumStates> struct test_query {
  std::array<std::size_t, NumStates> states_queried = {0};

  constexpr test_query() noexcept = default;
  constexpr explicit test_query(std::size_t const stop_after_index) noexcept
      : states_queried{0}, done_index_{stop_after_index} {}

  template <std::size_t Id, std::size_t NumEvents>
  constexpr void operator()(test_state<Id, NumEvents> const &) noexcept {
    states_queried[Id] += 1;
  }

  constexpr void stop_at(std::size_t const index) noexcept {
    done_index_ = index;
  }

  constexpr void dont_stop() noexcept { stop_at(NumStates); }

  constexpr bool is_done() const noexcept {
    return [this]<std::size_t... Is>(
        std::index_sequence<Is...> const) noexcept {
      return (((0 < states_queried[Is]) && (Is == done_index_)) || ...);
    }
    (std::make_index_sequence<NumStates>{});
  }

private:
  std::size_t done_index_ = NumStates;
};
} // namespace test_objects
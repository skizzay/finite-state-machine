#pragma once

#include <skizzay/fsm/current_state.h>
#include <skizzay/fsm/event.h>
#include <skizzay/fsm/state.h>
#include <skizzay/fsm/traits.h>
#include <skizzay/fsm/transition.h>
#include <skizzay/fsm/transition_schedulers/root_transition_scheduler.h>
#include <utility>

namespace skizzay::fsm {

namespace finite_state_machine_details_ {

template <typename RootStateContainer, typename TransitionTable>
concept root_state_container = concepts::state_container<RootStateContainer> &&
    concepts::transition_table<TransitionTable> &&
    contains_all_v<states_list_t<RootStateContainer>,
                   states_list_t<TransitionTable>> &&
    contains_all_v<states_list_t<TransitionTable>,
                   states_list_t<RootStateContainer>>;

template <typename TransitionTable, typename RootStateContainer> class impl {
  TransitionTable transition_table_;
  RootStateContainer root_state_container_;

  static constexpr bool is_nothrow_constructible() noexcept {
    return std::conjuction_v<
        std::is_nothrow_move_constructible<TransitionTable>,
        std::is_nothrow_move_constructible<RootStateContainer>>;
  }

  template <concepts::event_in<events_list_type> Event>
  constexpr bool post_event(Event const &event) {
    transition_coordinator coordinator{event, *this};
    bool const event_handled =
        root_state_container_.schedule_acceptable_transitions(
            std::as_const(*this), coordinator);
    coordinate.execute_transitions();
    return event_handled;
  }

  template <typename Event> constexpr bool post_event(Event const &) {
    return false;
  }

  constexpr void post_epsilon_events(bool post_event) {
    while (post_epsilon) {
      post_epsilon = post_event(epsilon_event);
    }
  }

public:
  using states_list_type = states_list_t<RootStateContainer>;
  using events_list_type = events_list_t<TransitionTable>;

  template <concepts::state_in<states_list_type> State>
  constexpr bool is() const noexcept {
    return root_state_container_.template is<State>();
  }

  template <concepts::state_in<states_list_type> State>
  constexpr optional_reference<State const> current_state() const noexcept {
    return root_state_container_.template current_state<State>();
  }

  template <concepts::state_in<states_list_type> State>
  constexpr State &any_state() noexcept {
    return root_state_container_.template any_state<State>();
  }

  template <concepts::state_in<states_list_type> State>
  constexpr auto ancestry_to() noexcept {
    return impl_.template ancestry_to<State>(ancestors{});
  }

  constexpr void start() {
    // TODO: Track if we've already started
    // TODO: Track if we're already processing an event
    root_state_container_.on_initial_entry(*this);
  }

  template <concepts::event_in<events_list_type> Event>
  constexpr bool on(Event const &event) {
    // TODO: Track if we've already started
    // TODO: Track if we're already processing an event
    bool const handled = post_event(event);
    post_epsilon_events(handled);
    return handled;
  }

  constexpr void stop() {
    // TODO: Track if we've already started
    // TODO: Track if we're already processing an event
    root_state_container_.on_final_exit(*this);
  }
};

template <concepts::transition_table TransitionTable,
          root_state_container<TransitionTable> RootStateContainer>
class machine {
  using impl_type = impl<TransitionTable, RootStateContainer>;
  impl_type impl_;

public:
  using states_list_type = states_list_t<impl_type>;
  using events_list_type = events_list_t<impl_type>;

  constexpr machine(
      TransitionTable &&transition_table,
      RootStateContainer
          &&root_state_container) noexcept(std::
                                               is_nothrow_constructible_v<
                                                   impl_type, TransitionTable,
                                                   RootStateContainer>)
      : impl_{std::forward<TransitionTable>(transition_table),
              std::forward<RootStateContainer>(root_state_container)} {}

  template <concepts::state_in<states_list_type> State>
  constexpr bool is() const noexcept {
    return impl_.template is<State>();
  }

  template <concepts::state_in<states_list_type> State>
  constexpr optional_reference<State const> current_state() const noexcept {
    return impl_.template current_state<State>();
  }

  constexpr void start() { impl_.start(); }

  constexpr void stop() { impl_.stop(); }

  template <concepts::event_in<events_list_type> Event>
  constexpr bool on(Event const &event) {
    return impl_.on(event);
  }
};
} // namespace finite_state_machine_details_

template <concepts::transition_table TransitionTable,
          finite_state_machine_details_::root_state_container<TransitionTable>
              RootStateContainer>
using state_chart =
    finite_state_machine_details_::machine<TransitionTable, RootStateContainer>;
} // namespace skizzay::fsm

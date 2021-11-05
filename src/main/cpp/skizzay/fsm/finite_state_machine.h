#pragma once

#include <skizzay/fsm/ancestors.h>
#include <skizzay/fsm/concepts.h>
#include <skizzay/fsm/event.h>
#include <skizzay/fsm/internal_event_executor.h>
#include <skizzay/fsm/optional_reference.h>
#include <skizzay/fsm/traits.h>
#include <skizzay/fsm/transition_coordinator.h>
#include <type_traits>
#include <utility>

namespace skizzay::fsm {

enum class machine_status { stopped, stopping, starting, running };

namespace finite_state_machine_details_ {

template <typename RootStateContainer, typename TransitionTable>
concept root_state_container = concepts::state_container<RootStateContainer> &&
    concepts::transition_table<TransitionTable> &&
    // contains_all_v<states_list_t<TransitionTable>,
    //                states_list_t<RootStateContainer>> &&
    contains_all_v<states_list_t<RootStateContainer>,
                   states_list_t<TransitionTable>>;

template <concepts::transition_table TransitionTable,
          root_state_container<TransitionTable>>
class machine;

template <typename TransitionTable, typename RootStateContainer> class impl {
  friend class machine<TransitionTable, RootStateContainer>;
  TransitionTable transition_table_;
  RootStateContainer root_state_container_;
  internal_event_executor internal_event_executor_;
  bool accept_epsilon_events_ = true;
  machine_status status_ = machine_status::stopped;

  static constexpr bool is_nothrow_constructible() noexcept {
    return std::conjunction_v<
        std::is_nothrow_move_constructible<TransitionTable>,
        std::is_nothrow_move_constructible<RootStateContainer>>;
  }

  template <typename Event> void post_event(Event const &event) {
    accept_epsilon_events_ = true;
    transition_coordinator<impl<TransitionTable, RootStateContainer>, Event>
        coordinator{transition_table_};
    root_state_container_.on_event(coordinator, *this, event);
    if (auto const &entries = coordinator.entries();
        entries.has_scheduled_state(root_state_container_)) {
      root_state_container_.on_entry(entries, *this, event);
    }
  }

  void update_status(machine_status const new_status) noexcept {
    // Updates status in an asynchronous manner. This method ensures that all
    // other events are fired before the actual status has been updated. By
    // doing this we guarantee that the status is reflective of FSM's running
    // state.
    internal_event_executor_.schedule_execution([this, new_status]() {
      if (std::empty(internal_event_executor_)) {
        status_ = new_status;
      } else {
        update_status(new_status);
      }
    });
  }

public:
  using transition_table_type = TransitionTable;
  using states_list_type = states_list_t<RootStateContainer>;
  using events_list_type = events_list_t<transition_table_type>;

  constexpr impl(
      TransitionTable transition_table,
      RootStateContainer
          root_state_container) noexcept(impl::is_nothrow_constructible())
      : transition_table_{std::move_if_noexcept(transition_table)},
        root_state_container_{std::move_if_noexcept(root_state_container)} {}

  constexpr machine_status status() const noexcept { return status_; }

  template <concepts::state_in<states_list_type>... States>
  constexpr bool is() const noexcept {
    return (root_state_container_.template is<States>() || ... || false);
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
    return root_state_container_.template ancestry_to<State>(ancestors{});
  }

  template <concepts::state_in<states_list_type> State>
  constexpr auto ancestry_to() const noexcept {
    return root_state_container_.template ancestry_to<State const>(ancestors{});
  }

  constexpr void start() {
    if (machine_status::stopped ==
        std::exchange(status_, machine_status::starting)) {
      root_state_container_.on_initial_entry(*this);
      update_status(machine_status::running);
    }
  }

  template <concepts::event_in<events_list_type> Event>
  void on(Event const &event) {
    if (machine_status::stopped != status_) {
      internal_event_executor_.schedule_execution(
          [this, &event]() { post_event(event); });
    }
  }

  void on(epsilon_event_t const &) requires(
      contains_v<events_list_type, epsilon_event_t>) {
    // This is to ensure that we don't post multiple epsilon events. This will
    // collapse them down to a single event.
    if (machine_status::stopped != status_ && accept_epsilon_events_) {
      accept_epsilon_events_ = false;
      internal_event_executor_.schedule_execution(
          [this]() { post_event(epsilon_event); });
    }
  }

  constexpr void stop() {
    if (machine_status::running ==
        std::exchange(status_, machine_status::stopping)) {
      root_state_container_.on_final_exit(*this);
      update_status(machine_status::stopped);
    }
  }

  template <concepts::state_in<states_list_type> CurrentState,
            concepts::state_in<states_list_type> NextState,
            concepts::event Event>
  constexpr void on_exit(Event const &event) {
    root_state_container_.template on_exit<CurrentState, NextState>(*this,
                                                                    event);
  }

  template <concepts::state_in<states_list_type> PreviousState,
            concepts::state_in<states_list_type> CurrentState,
            concepts::event Event>
  constexpr void on_entry(Event const &event) {
    root_state_container_.template on_entry<PreviousState, CurrentState>(*this,
                                                                         event);
  }

  template <concepts::state_in<states_list_type> CurrentState,
            concepts::event Event>
  constexpr void on_reentry(Event const &event) {
    root_state_container_.template on_reentry<CurrentState>(*this, event);
  }
};

template <concepts::transition_table TransitionTable,
          root_state_container<TransitionTable> RootStateContainer>
class machine {
  using impl_type = impl<TransitionTable, RootStateContainer>;
  impl_type impl_;

public:
  using transition_table_type = transition_table_t<impl_type>;
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

  constexpr bool is_running() const noexcept {
    return machine_status::running == impl_.status();
  }

  constexpr bool is_stopped() const noexcept {
    return machine_status::stopped == impl_.status();
  }

  template <concepts::state_in<states_list_type>... States>
  constexpr bool is() const noexcept {
    return impl_.template is<States...>();
  }

  template <concepts::state_in<states_list_type> State>
  constexpr optional_reference<State const> current_state() const noexcept {
    return impl_.template current_state<State>();
  }

  constexpr void start() {
    if (is_stopped()) {
      impl_.start();
      impl_.internal_event_executor_.process_internal_events();
    }
  }

  constexpr void stop() {
    if (is_running()) {
      impl_.stop();
      impl_.internal_event_executor_.process_internal_events();
    }
  }

  template <concepts::event_in<events_list_type> Event>
  constexpr void on(Event const &event) {
    if (is_running()) {
      impl_.on(event);
      impl_.internal_event_executor_.process_internal_events();
    }
  }
};

template <concepts::transition_table TransitionTable,
          finite_state_machine_details_::root_state_container<TransitionTable>
              RootStateContainer>
machine(TransitionTable, RootStateContainer)
    -> machine<std::remove_cvref_t<TransitionTable>,
               std::remove_cvref_t<RootStateContainer>>;
} // namespace finite_state_machine_details_

template <concepts::transition_table TransitionTable,
          finite_state_machine_details_::root_state_container<TransitionTable>
              RootStateContainer>
using finite_state_machine =
    finite_state_machine_details_::machine<TransitionTable, RootStateContainer>;
} // namespace skizzay::fsm

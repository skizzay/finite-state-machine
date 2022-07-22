#pragma once

#include "skizzay/fsm/event.h"
#include "skizzay/fsm/events_list.h"
#include "skizzay/fsm/state_container.h"
#include "skizzay/fsm/states_list.h"
#include "skizzay/fsm/transition_table.h"
#include "skizzay/fsm/type_list.h"

#include <type_traits>

namespace skizzay::fsm {

template <typename, typename> struct machine_state;

namespace machine_state_details_ {

template <typename, typename> struct is_current_state_of : std::false_type {};

template <concepts::state State, concepts::transition Transition>
struct is_current_state_of<State, Transition>
    : std::is_same<std::remove_cvref_t<State>,
                   std::remove_cvref_t<current_state_t<Transition>>> {};

template <concepts::state_scheduler StateScheduler,
          concepts::transition_table TransitionTable>
struct local_event_transition_context {
  using next_states_list_type =
      map_t<next_states_list_t<TransitionTable>, std::remove_cvref_t>;
  using transition_table_type = TransitionTable;

  constexpr explicit local_event_transition_context(
      StateScheduler &state_scheduler,
      TransitionTable &&transition_table) noexcept
      : state_scheduler_{state_scheduler}, transition_table_{
                                               std::move(transition_table)} {}

  template <concepts::transition_in<TransitionTable> Transition>
  constexpr void on_transition(Transition &transition,
                               event_t<Transition> const &event) {
    trigger(transition, event);
    this->template schedule_entry<next_state_t<Transition>>();
  }

  template <concepts::transition_in<transition_table_type> Transition>
  [[noreturn]] constexpr void
  on_transition(Transition &, concepts::event_in<Transition> auto const &) {
    assert(false && "Root attempting to transition to a state outside of "
                    "the machine.");
  }

  constexpr concepts::transition_table auto get_transitions(
      concepts::state_in<current_states_list_t<TransitionTable>> const
          &state) noexcept {
    return get_transition_table_for(transition_table_, state);
  }

  template <concepts::state_in<next_states_list_type> State>
  constexpr void schedule_entry() noexcept {
    state_scheduler_.template schedule_entry<next_state_t<Transition>>();
  }

private:
  StateScheduler &state_scheduler_;
  [[no_unique_address]] TransitionTable transition_table_;
};

template <concepts::state MachineState,
          concepts::state_scheduler InternalStateScheduler,
          concepts::state_scheduler ExternalStateScheduler>
struct local_state_scheduler {
  using next_states_list_type = next_states_list_t<InternalStateScheduler>;

  constexpr explicit local_state_scheduler(
      InternalStateScheduler &internal_state_scheduler,
      ExternalStateScheduler &external_state_scheduler) noexcept
      : internal_state_scheduler_{internal_state_scheduler},
        external_state_scheduler_{external_state_scheduler} {}

  template <concepts::state_in<next_states_list_type> State>
  constexpr bool is_scheduled() const noexcept {
    return internal_state_scheduler_.template is_scheduled<State>();
  }

  template <concepts::state_in<next_states_list_type> State>
  constexpr void schedule_entry() noexcept {
    internal_state_scheduler_.template schedule_entry<State>();
    external_state_scheduler_.template schedule_entry<MachineState>();
  }

private:
  InternalStateScheduler &internal_state_scheduler;
  ExternalStateScheduler &external_state_scheduler;
};

inline constexpr auto make_local_state_scheduler =
    []<concepts::state MachineState>(
        concepts::state_scheduler auto &internal_state_scheduler,
        concepts::state_scheduler auto &external_state_scheduler) noexcept
    -> local_state_scheduler<MachineState, decltype(internal_state_scheduler),
                             decltype(external_state_scheduler)> {
  return {internal_state_scheduler, external_state_scheduler};
}

} // namespace machine_state_details_

template <
    concepts::root_transition_table RootTransitionTable,
    concepts::root_state_container<RootTransitionTable> RootStateContainer>
struct machine_state<RootTransitionTable, RootStateContainer> {
  using transition_table_type = TransitionTable;
  using states_list_type =
      states_list_type<machine_state<RootStateContainer, TransitionTable>>;

  template <std::same_as<machine_state<RootStateContainer, TransitionTable>>>
  constexpr bool is() const noexcept {
    return is_active();
  }

  template <concepts::state_in<states_list_t<RootStateContainer>> State>
  constexpr bool is() const noexcept {
    return root_state_container_.template is<State>();
  }

  template <std::same_as<machine_state<RootStateContainer, TransitionTable>>>
  constexpr optional_reference<
      machine_state<RootStateContainer, TransitionTable> const>
  current_state() const noexcept {
    return is_active() ? optional_reference{*this} : std::nullopt;
  }

  template <concepts::state_in<states_list_t<RootStateContainer>> State>
  constexpr optional_reference<State const> current_state() const noexcept {
    return root_state_container_.template current_state<State>();
  }

  template <std::same_as<machine_state<RootStateContainer, TransitionTable>>>
  constexpr machine_state<RootStateContainer, TransitionTable> &
  state() noexcept {
    return *this;
  }

  template <concepts::state_in<states_list_t<RootStateContainer>> State>
  constexpr State &state() noexcept {
    return root_state_container_.template state<State>();
  }

  template <std::same_as<machine_state<RootStateContainer, TransitionTable>>>
  constexpr machine_state<RootStateContainer, TransitionTable> const &
  state() const noexcept {
    return *this;
  }

  template <concepts::state_in<states_list_t<RootStateContainer>> State>
  constexpr State const &state() const noexcept {
    return root_state_container_.template state<State>();
  }

  constexpr bool is_active() const noexcept {
    return root_state_container_.is_active();
  }

  constexpr bool is_inactive() const noexcept {
    return root_state_container_.is_inactive();
  }

  template <concepts::query<states_list_t<RootStateContainer>> Query>
  constexpr bool query(Query &&query) const noexcept(
      concepts::nothrow_query<Query, states_list_t<RootStateContainer>>) {
    return root_state_container_.query(std::forward<Query>(query));
  }

  template <
      concepts::query<machine_state<RootStateContainer, TransitionTable>> Query>
  constexpr bool query(Query &&query) const
      noexcept(concepts::nothrow_query<
               Query, machine_state<RootStateContainer, TransitionTable>>) {
    assert(is_active() || "Trying to query an inactive machine state");
    std::invoke(std::forward<Query>(query), *this);
    return is_done(query);
  }

  constexpr void on_entry(concepts::state_schedule auto const &,
                          concepts::event auto const &event,
                          concepts::event_engine auto &event_engine,
                          concepts::state_provider auto &) {
    if (is_inactive()) {
      enter_root_state_container(empty_state_schedule{}, initial_entry_event);
    }
    enter_root_state_container(std::as_const(state_scheduler_), event);
  }

  constexpr void on_entry(concepts::state_schedule auto const &,
                          initial_entry_event_t const &event,
                          concepts::event_engine auto &event_engine,
                          concepts::state_provider auto &) {
    assert(is_inactive() ||
           "Attempting initial entry on an active machine state");
    enter_root_state_container(empty_state_schedule{}, initial_entry_event);
  }

  constexpr bool
  on_event(concepts::event_transition_context auto
               &external_event_transition_context,
           concepts::event auto const &event,
           concepts::event_engine auto &external_event_engine,
           concepts::state_provider auto &external_state_provider) {
    internal_event_engine event_engine{root_state_container_, transition_table_,
                                       q_};
    auto local_state_scheduler{
        machine_state_details_::make_local_state_scheduler<
            machine_state<RootStateContainer, TransitionTable>>(
            state_scheduler_, external_state_scheduler)};
    machine_state_details_::local_event_transition_context
        local_event_transition_context{
            local_state_scheduler,
            get_transition_table_for(transition_table_, event)};

    state_scheduler_.reset();
    if (root_state_container_.on_event(local_event_transition_context, event,
                                       event_engine, root_state_container_)) {
      // We'll run the task queue during entry
      return true;
    } else {
      return attempt_transitions(external_event_transition_context, *this,
                                 event, external_state_provider,
                                 [this]() { exit_root_state_container(); })
    }
  }

  constexpr bool on_event(concepts::event_transition_context auto &,
                          final_exit_event_t const &,
                          concepts::event_engine auto &,
                          concepts::state_provider auto &) {
    return exit_root_state_container();
  }

private:
  entry_state_scheduler<states_list_t<RootStateContainer>> state_scheduler_;
  RootStateContainer root_state_container_;
  TransitionTable transition_table_;
  task_queue q_;

  constexpr void
  enter_root_state_container(concepts::state_schedule const &state_schedule,
                             concepts::event auto const &event) {
    internal_event_engine event_engine{root_state_container_, transition_table_,
                                       q_};
    root_state_container_.on_entry(state_schedule, event, event_engine,
                                   root_state_container_);
    q_.run();
  }

  constexpr bool exit_root_state_container() {
    internal_event_engine event_engine{root_state_container_, transition_table_,
                                       q_};
    return execute_final_exit(root_state_container_, event_engine,
                              root_state_container_);
  }
};
} // namespace skizzay::fsm

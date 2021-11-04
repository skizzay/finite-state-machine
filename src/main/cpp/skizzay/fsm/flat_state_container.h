#pragma once

#include <cassert>
#include <iostream>
#include <skizzay/fsm/concepts.h>
#include <skizzay/fsm/event.h>
#include <skizzay/fsm/node_transition_coordinator.h>
#include <skizzay/fsm/optional_reference.h>
#include <skizzay/fsm/overload.h>
#include <skizzay/fsm/traits.h>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

namespace skizzay::fsm {

namespace flat_state_container_details_ {

template <typename Machine, typename Event, typename EntryCoordinator,
          typename StateContainersVariant>
struct entry_visit_callback {
  void *container = nullptr;
  void (*callback)(void *, Machine &, Event const &, EntryCoordinator const &,
                   StateContainersVariant &) = nullptr;

  explicit operator bool() const noexcept { return nullptr != container; }

  constexpr void fire(Machine &machine, Event const &event,
                      EntryCoordinator const &entry_coordinator,
                      StateContainersVariant &states_container_variant) {
    assert(nullptr != callback &&
           "Entry visit callback encountered null callback");
    std::invoke(callback, container, machine, event, entry_coordinator,
                states_container_variant);
  }

  static entry_visit_callback
  for_bad_state(std::string_view const assertion_message) noexcept {
    std::cerr << assertion_message;
    return {nullptr,
            [](void *, Machine &, Event const &, EntryCoordinator const &,
               StateContainersVariant &) { std::terminate(); }};
  }

  template <typename Container>
  requires contains_v<StateContainersVariant, Container *>
  static entry_visit_callback
  for_entry_of(Container *const container) noexcept {
    assert(nullptr != container &&
           "Container pointer cannot be null for entry");
    return {
        container, [](void *container_ptr, Machine &machine, Event const &event,
                      EntryCoordinator const &entry_coordinator,
                      StateContainersVariant &state_containers_variant) {
          Container *const container = static_cast<Container *>(container_ptr);
          container->on_entry(entry_coordinator, machine, event);
          state_containers_variant = container;
        }};
  }
};

template <typename Machine, typename Event, typename StateContainersVariant>
struct visit_callback {
  void *container = nullptr;
  void (*callback)(void *, Machine &, Event const &,
                   StateContainersVariant &) = nullptr;

  explicit operator bool() const noexcept { return nullptr != container; }

  void fire(Machine &machine, Event const &event,
            StateContainersVariant &state_containers_variant) {
    assert(nullptr != callback && "Visit callback encountered null callback");
    std::invoke(callback, container, machine, event, state_containers_variant);
  }

  static visit_callback
  for_bad_state(std::string_view const assertion_message) noexcept {
    std::cerr << assertion_message;
    return {nullptr,
            [](void *, auto &, auto const &, auto &) { std::terminate(); }};
  }

  template <typename Container>
  requires contains_v<StateContainersVariant, Container *>
  static visit_callback
  for_initial_entry_of(Container *const container) noexcept {
    assert(nullptr != container &&
           "Container pointer cannot be null for entry");
    return {container, [](void *container_ptr, Machine &machine, Event const &,
                          StateContainersVariant &state_containers_variant) {
              Container *const container =
                  static_cast<Container *>(container_ptr);
              container->on_initial_entry(machine);
              state_containers_variant = container;
            }};
  }

  template <typename Container>
  requires contains_v<StateContainersVariant, Container *>
  static visit_callback for_exit_of(Container *const container) noexcept {
    assert(nullptr != container && "Container pointer cannot be null for exit");
    return {container, [](void *container_ptr, Machine &machine, Event const &,
                          StateContainersVariant &state_containers_variant) {
              state_containers_variant = std::monostate{};
              if constexpr (std::same_as<final_exit_event_t, Event>) {
                static_cast<Container *>(container_ptr)->on_final_exit(machine);
              }
            }};
  }
};

template <concepts::state_container... StateContainers> class container {
  using tuple_type = std::tuple<StateContainers...>;
  using variant_type = std::variant<std::monostate, StateContainers *...>;
  tuple_type state_containers_;
  variant_type current_state_container_;

public:
  using states_list_type = concat_t<states_list_t<StateContainers>...>;

  static_assert(0 < length_v<states_list_type>,
                "State container template argument list cannot be empty");
  // static_assert(length_v<states_list_type> ==
  //                   length_v<unique_t<states_list_type>>,
  //               "State containers cannot hold the same state");

  constexpr container() noexcept(
      std::is_nothrow_default_constructible_v<tuple_type>)
      : state_containers_{}, current_state_container_{std::monostate{}} {}

  constexpr explicit container(StateContainers &&...containers) noexcept(
      std::is_nothrow_constructible_v<tuple_type, StateContainers...>)
      : state_containers_{std::forward<StateContainers>(containers)...},
        current_state_container_{std::monostate{}} {}

  template <concepts::state_in<states_list_type> S>
  constexpr bool is() const noexcept {
    return std::visit(
        overload([](std::monostate const) noexcept { return false; },
                 [](concepts::state_container_for<S> auto const
                        *const container) noexcept {
                   return container->template is<S>();
                 },
                 [](auto const *const) noexcept { return false; }),
        current_state_container_);
  }

  template <concepts::state_in<states_list_type> S>
  constexpr optional_reference<S const> current_state() const noexcept {
    return std::visit(
        overload(
            []<typename Container>(Container const *const container) noexcept
            -> optional_reference<S const> {
              if constexpr (contains_v<states_list_t<Container>, S>) {
                return container->template current_state<S>();
              } else {
                return std::nullopt;
              }
            },
            [](std::monostate const) noexcept -> optional_reference<S const> {
              return std::nullopt;
            }),
        current_state_container_);
  }

  template <concepts::state_in<states_list_type> S>
  constexpr S &get() noexcept {
    return std::get<state_container_for_t<tuple_type, S>>(state_containers_)
        .template any_state<S>();
  }

  template <concepts::state_in<states_list_type> S>
  constexpr S const &get() const noexcept {
    return std::get<state_container_for_t<tuple_type, S>>(state_containers_)
        .template get<S>();
  }

  template <concepts::state_in<states_list_type> S, concepts::ancestry Ancestry>
  constexpr auto ancestry_to(Ancestry ancestry) noexcept {
    return std::get<state_container_for_t<tuple_type, S>>(state_containers_)
        .template ancestry_to<S>(ancestry);
  }

  constexpr bool is_active() const noexcept {
    return !std::holds_alternative<std::monostate>(current_state_container_);
  }

  constexpr bool is_inactive() const noexcept {
    return std::holds_alternative<std::monostate>(current_state_container_);
  }

  template <concepts::machine Machine>
  constexpr void on_initial_entry(Machine &machine) {
    std::visit(overload(
                   [&](std::monostate const) noexcept {
                     return visit_callback<Machine, initial_entry_event_t,
                                           variant_type>::
                         for_initial_entry_of(
                             std::addressof(std::get<0>(state_containers_)));
                   },
                   [](auto *const) noexcept {
                     return visit_callback<Machine, initial_entry_event_t,
                                           variant_type>::
                         for_bad_state(
                             "Initial entry into a state while already in a "
                             "valid state");
                   }),
               current_state_container_)
        .fire(machine, initial_entry_event, current_state_container_);
  }

  template <concepts::machine Machine>
  constexpr void on_final_exit(Machine &machine) {
    std::visit(
        overload(
            [](std::monostate const) noexcept {
              return visit_callback<Machine, final_exit_event_t, variant_type>::
                  for_bad_state("on_final_exit invoked when not in a state");
            },
            [&](auto *const container) noexcept {
              return visit_callback<Machine, final_exit_event_t,
                                    variant_type>::for_exit_of(container);
            }),
        current_state_container_)
        .fire(machine, final_exit_event, current_state_container_);
  }

  template <concepts::entry_coordinator EntryCoordinator,
            concepts::machine Machine,
            concepts::event_in<events_list_t<Machine>> Event>
  constexpr void on_entry(EntryCoordinator const &entry_coordinator,
                          Machine &machine, Event const &event) {
    std::visit(
        overload(
            [&](std::monostate const) noexcept {
              constexpr entry_callback_finder<EntryCoordinator, Machine, Event>
                  finder;
              return finder.template find<0>(entry_coordinator,
                                             state_containers_);
            },
            [&]<typename T>(T *const container) {
              if constexpr (EntryCoordinator::template contains_v<T>) {
                return entry_visit_callback<
                    Machine, Event, EntryCoordinator,
                    variant_type>::for_entry_of(container);
              } else {
                return entry_visit_callback<Machine, Event, EntryCoordinator,
                                            variant_type>::
                    for_bad_state(
                        "Attempting to enter state when a different state "
                        "is already active");
              }
            }),
        current_state_container_)
        .fire(machine, event, entry_coordinator, current_state_container_);
  }

  template <typename ParentTransitionCoordinator, typename Machine,
            typename Event>
  constexpr bool
  on_event(ParentTransitionCoordinator &parent_transition_coordinator,
           Machine &machine, Event const &event) {
    auto const [has_been_scheduled, event_handling_result] = std::visit(
        overload(
            [](std::monostate const) noexcept {
              return std::tuple{false, false};
            },
            [&](auto *const container) {
              node_transition_coordinator tc{parent_transition_coordinator,
                                             *this};
              bool const event_handling_result =
                  container->on_event(tc, machine, event);
              return std::tuple{tc.has_been_scheduled(), event_handling_result};
            }),
        current_state_container_);
    if (has_been_scheduled) {
      current_state_container_ = std::monostate{};
    }
    return event_handling_result;
  }

private:
  template <typename EntryCoordinator, typename Machine, typename Event>
  struct entry_callback_finder {
    using result_type =
        entry_visit_callback<Machine, Event, EntryCoordinator, variant_type>;
    using applicable_state_containers_list_type =
        typename EntryCoordinator::applicable_state_containers_list_type<
            tuple_type>;
    template <std::size_t I>
    using state_container_at_t =
        element_at_t<I, applicable_state_containers_list_type>;

    template <std::size_t I>
    requires(
        I <
        length_v<applicable_state_containers_list_type>) constexpr result_type
        find(EntryCoordinator const &entry_coordinator,
             tuple_type &state_containers) const noexcept {
      auto &state_container =
          std::get<state_container_at_t<I>>(state_containers);
      return entry_coordinator.has_scheduled_state(state_container)
                 ? entry_visit_callback<
                       Machine, Event, EntryCoordinator,
                       variant_type>::for_entry_of(&state_container)
                 : this->template find<I + 1>(entry_coordinator,
                                              state_containers);
    }

    template <std::size_t I>
    constexpr result_type find(EntryCoordinator const &,
                               tuple_type &) const noexcept {
      return {};
    }
  };
};
} // namespace flat_state_container_details_

template <concepts::state_container... StateContainers>
using flat_states =
    flat_state_container_details_::container<StateContainers...>;

} // namespace skizzay::fsm
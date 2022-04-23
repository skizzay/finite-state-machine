#pragma once

#include "skizzay/fsm/entry_context.h"
#include "skizzay/fsm/event_context_node.h"
#include "skizzay/fsm/event_transition_context.h"
#include "skizzay/fsm/optional_reference.h"
#include "skizzay/fsm/state_container.h"
#include "skizzay/fsm/state_containers_list.h"
#include "skizzay/fsm/states_list.h"
#include "skizzay/fsm/transition_table.h"
#include "skizzay/fsm/type_list.h"

#include <array>
#include <cstddef>
#include <functional>
#include <tuple>
#include <utility>

namespace skizzay::fsm {

namespace flat_state_container_details_ {

template <concepts::states_list StatesList> struct contains_any_states {
  template <concepts::state_container StateContainer>
  using test = contains_any<StatesList, states_list_t<StateContainer>>;
};

template <concepts::states_list NextStatesList,
          concepts::state_containers_list StateContainersList>
using next_state_containers_indices_t = as_index_sequence_t<
    filter_t<StateContainersList,
             contains_any_states<NextStatesList>::template test>,
    StateContainersList>;

template <concepts::state_container... StateContainers> class container {
  using tuple_type = std::tuple<StateContainers...>;
  template <concepts::state_in<tuple_type> State>
  static constexpr std::size_t const container_index_ =
      index_of_v<tuple_type, state_container_for_t<tuple_type, State>>;

  tuple_type state_containers_;
  std::size_t current_index_ = sizeof...(StateContainers);

  constexpr void reset_index() noexcept {
    current_index_ = sizeof...(StateContainers);
  }

  template <concepts::state State>
  constexpr auto &get_state_container_for() noexcept {
    return std::get<state_container_for_t<tuple_type, State>>(
        state_containers_);
  }

  template <concepts::state State>
  constexpr auto const &get_state_container_for() const noexcept {
    return std::get<state_container_for_t<tuple_type, State>>(
        state_containers_);
  }

  template <typename Context, typename F>
  constexpr auto visit(Context &context, F &&f) {
    return [this](Context &context, F &&f) {
      auto visit_impl = [this]<std::size_t I>(Context &context, F &&f,
                                              auto &impl) {
        if constexpr (I == sizeof...(StateContainers)) {
          // throw
        } else if (I == current_index_) {
          return std::invoke(std::forward<F>(f),
                             std::get<I>(state_containers_));
        } else {
          return impl<I + 1>(context, std::forward<F>(f));
        }
      };
      visit_impl<0>(context, std::forward<F>(f), visit_impl);
    }(context, std::forward<F>(f));
  }

  template <std::size_t N, concepts::entry_context EntryContext>
  constexpr void
  enter_state_container(std::array<std::size_t, N> const &indices,
                        EntryContext &entry_context) {
    constexpr auto const is_invalid_index =
        [](std::size_t const index) noexcept {
          return index == sizeof...(StateContainers);
        };
    auto const end_iter = std::end(indices);
    auto const index_iter =
        std::find_if_not(std::begin(indices), end_iter, is_invalid_index);
    assert(end_iter != index_iter && "Next state container could not be found");
    assert(std::find_if_not(std::next(index_iter), end_iter,
                            is_invalid_index) == end_iter &&
           "Multiple state containers identified for entry");
    constexpr auto const impl =
        [this]<std::size_t I>(std::size_t const index,
                              EntryContext &entry_context) -> void {
      if (I == index) {
        std::get<I>(state_containers_).on_entry(entry_context);
      }
    };
    []<std::size_t... Is>(std::index_sequence<Is...> const,
                          std::size_t const index, auto const &impl,
                          EntryContext &entry_context) {
      ((impl<Is>(index, entry_context)), ...);
    }
    (std::make_index_sequence<N>{}, *index_iter, impl, entry_context);
  }

public:
  using states_list_type = flat_map_t<map_t<tuple_type, states_list_t>>;

  constexpr container() noexcept(
      std::is_nothrow_default_constructible_v<tuple_type>)
      : state_containers_{} {}

  constexpr explicit container(StateContainers &&...containers) noexcept(
      std::is_nothrow_constructible_v<tuple_type, StateContainers...>)
      : state_containers_{std::forward<StateContainers>(containers)...} {}

  template <concepts::state_in<states_list_type> S>
  constexpr bool is() const noexcept {
    return this->template get_state_container_for<S>().template is<S>();
  }

  template <concepts::state_in<states_list_type> S>
  constexpr optional_reference<S const> current_state() const noexcept {
    return this->template get_state_container_for<S>().template state<S>();
  }

  template <concepts::state_in<states_list_type> S>
  constexpr S const &state() const noexcept {
    return this->template get_state_container_for<S>().template state<S>();
  }

  template <concepts::state_in<states_list_type> S>
  constexpr S &state() noexcept {
    return this->template get_state_container_for<S>().template state<S>();
  }

  constexpr bool is_active() const noexcept {
    return sizeof...(StateContainers) == current_index_;
  }

  constexpr bool is_inactive() const noexcept {
    return sizeof...(StateContainers) != current_index_;
  }

  constexpr void on_entry(concepts::initial_entry_context auto &entry_context) {
    assert(is_inactive() && "Conducting initial entry into flat state "
                            "container which is already active");
    std::get<0>(state_containers_).on_entry(entry_context);
    current_index_ = 0;
  }

  constexpr void on_entry(concepts::entry_context auto &entry_context) {
    auto const find_state_container_index =
        []<std::size_t... Is>(
            concepts::entry_context auto const &entry_context,
            std::tuple<StateContainers...> const &state_containers,
            std::index_sequence<Is...> const) noexcept
            ->std::array<std::size_t, sizeof...(Is)> {
      auto const impl =
          []<std::size_t I>(
              concepts::entry_context auto const &entry_context,
              std::tuple<StateContainers...> const &state_containers) noexcept
          -> std::size_t {
        return has_state_scheduled_for_entry(entry_context,
                                             std::get<I>(state_containers))
                   ? I
                   : sizeof...(StateContainers);
      };
      return {{impl<Is>(entry_context, state_containers)...}};
    };
    enter_state_container(
        find_state_container_index(std::as_const(entry_context),
                                   std::as_const(state_containers_)),
        entry_context);
  }

  constexpr bool on_event(concepts::final_exit_event_transition_context auto
                              &final_exit_event_context) {
    assert(is_active() && "Handling a final exit event in a flat state "
                          "container which is inactive");
    bool const result =
        visit([&](concepts::state_container auto &state_container) {
          return state_container.on_event(final_exit_event_context);
        });
    reset_index();
    return result;
  }

  constexpr bool
  on_event(concepts::event_transition_context auto &event_transition_context) {
    event_context_node current_context{*this, event_transition_context};
    bool const result =
        visit([&](concepts::state_container auto &state_container) {
          return state_container.on_event(current_context);
        });
    if (current_context.has_been_scheduled()) {
      reset_index();
    }
    return result;
  }
};

// template <typename Machine, typename Event, typename EntryCoordinator,
//           typename StateContainersVariant>
// struct entry_visit_callback {
//   void *container = nullptr;
//   void (*callback)(void *, Machine &, Event const &, EntryCoordinator const
//   &,
//                    StateContainersVariant &) = nullptr;

//   explicit operator bool() const noexcept { return nullptr != container; }

//   constexpr void fire(Machine &machine, Event const &event,
//                       EntryCoordinator const &entry_coordinator,
//                       StateContainersVariant &states_container_variant) {
//     assert(nullptr != callback &&
//            "Entry visit callback encountered null callback");
//     std::invoke(callback, container, machine, event, entry_coordinator,
//                 states_container_variant);
//   }

//   static entry_visit_callback
//   for_bad_state(std::string_view const assertion_message) noexcept {
//     std::cerr << assertion_message;
//     return {nullptr,
//             [](void *, Machine &, Event const &, EntryCoordinator const &,
//                StateContainersVariant &) { std::terminate(); }};
//   }

//   template <typename Container>
//   requires contains_v<StateContainersVariant, Container *>
//   static entry_visit_callback
//   for_entry_of(Container *const container) noexcept {
//     assert(nullptr != container &&
//            "Container pointer cannot be null for entry");
//     return {
//         container, [](void *container_ptr, Machine &machine, Event const
//         &event,
//                       EntryCoordinator const &entry_coordinator,
//                       StateContainersVariant &state_containers_variant) {
//           Container *const container = static_cast<Container
//           *>(container_ptr); container->on_entry(entry_coordinator, machine,
//           event); state_containers_variant = container;
//         }};
//   }
// };

// template <typename Machine, typename Event, typename StateContainersVariant>
// struct visit_callback {
//   void *container = nullptr;
//   void (*callback)(void *, Machine &, Event const &,
//                    StateContainersVariant &) = nullptr;

//   explicit operator bool() const noexcept { return nullptr != container; }

//   void fire(Machine &machine, Event const &event,
//             StateContainersVariant &state_containers_variant) {
//     assert(nullptr != callback && "Visit callback encountered null
//     callback"); std::invoke(callback, container, machine, event,
//     state_containers_variant);
//   }

//   static visit_callback
//   for_bad_state(std::string_view const assertion_message) noexcept {
//     std::cerr << assertion_message;
//     return {nullptr,
//             [](void *, auto &, auto const &, auto &) { std::terminate(); }};
//   }

//   template <typename Container>
//   requires contains_v<StateContainersVariant, Container *>
//   static visit_callback
//   for_initial_entry_of(Container *const container) noexcept {
//     assert(nullptr != container &&
//            "Container pointer cannot be null for entry");
//     return {container, [](void *container_ptr, Machine &machine, Event const
//     &,
//                           StateContainersVariant &state_containers_variant) {
//               Container *const container =
//                   static_cast<Container *>(container_ptr);
//               container->on_initial_entry(machine);
//               state_containers_variant = container;
//             }};
//   }

//   template <typename Container>
//   requires contains_v<StateContainersVariant, Container *>
//   static visit_callback for_exit_of(Container *const container) noexcept {
//     assert(nullptr != container && "Container pointer cannot be null for
//     exit"); return {container, [](void *container_ptr, Machine &machine,
//     Event const &,
//                           StateContainersVariant &state_containers_variant) {
//               state_containers_variant = std::monostate{};
//               if constexpr (std::same_as<final_exit_event_t, Event>) {
//                 static_cast<Container
//                 *>(container_ptr)->on_final_exit(machine);
//               }
//             }};
//   }
// };

// template <concepts::state_container... StateContainers> class container {
//   using tuple_type = std::tuple<StateContainers...>;
//   using variant_type = std::variant<std::monostate, StateContainers *...>;
//   tuple_type state_containers_;
//   variant_type current_state_container_;

// public:
//   using states_list_type = concat_t<states_list_t<StateContainers>...>;

//   static_assert(0 < length_v<states_list_type>,
//                 "State container template argument list cannot be empty");
//   // static_assert(length_v<states_list_type> ==
//   //                   length_v<unique_t<states_list_type>>,
//   //               "State containers cannot hold the same state");

//   constexpr container() noexcept(
//       std::is_nothrow_default_constructible_v<tuple_type>)
//       : state_containers_{}, current_state_container_{std::monostate{}} {}

//   constexpr explicit container(StateContainers &&...containers) noexcept(
//       std::is_nothrow_constructible_v<tuple_type, StateContainers...>)
//       : state_containers_{std::forward<StateContainers>(containers)...},
//         current_state_container_{std::monostate{}} {}

//   template <concepts::entry_coordinator EntryCoordinator,
//             concepts::machine Machine,
//             concepts::event_in<events_list_t<Machine>> Event>
//   constexpr void on_entry(EntryCoordinator const &entry_coordinator,
//                           Machine &machine, Event const &event) {
//     std::visit(
//         overload(
//             [&](std::monostate const) noexcept {
//               constexpr entry_callback_finder<EntryCoordinator, Machine,
//               Event>
//                   finder;
//               return finder.template find<0>(entry_coordinator,
//                                              state_containers_);
//             },
//             [&]<typename T>(T *const container) {
//               if constexpr (EntryCoordinator::template contains_v<T>) {
//                 return entry_visit_callback<
//                     Machine, Event, EntryCoordinator,
//                     variant_type>::for_entry_of(container);
//               } else {
//                 return entry_visit_callback<Machine, Event, EntryCoordinator,
//                                             variant_type>::
//                     for_bad_state(
//                         "Attempting to enter state when a different state "
//                         "is already active");
//               }
//             }),
//         current_state_container_)
//         .fire(machine, event, entry_coordinator, current_state_container_);
//   }

//   template <typename ParentTransitionCoordinator, typename Machine,
//             typename Event>
//   constexpr bool
//   on_event(ParentTransitionCoordinator &parent_transition_coordinator,
//            Machine &machine, Event const &event) {
//     auto const [has_been_scheduled, event_handling_result] = std::visit(
//         overload(
//             [](std::monostate const) noexcept {
//               return std::tuple{false, false};
//             },
//             [&](auto *const container) {
//               node_transition_coordinator tc{parent_transition_coordinator,
//                                              *this};
//               bool const event_handling_result =
//                   container->on_event(tc, machine, event);
//               return std::tuple{tc.has_been_scheduled(),
//               event_handling_result};
//             }),
//         current_state_container_);
//     if (has_been_scheduled) {
//       current_state_container_ = std::monostate{};
//     }
//     return event_handling_result;
//   }

// private:
//   template <typename EntryCoordinator, typename Machine, typename Event>
//   struct entry_callback_finder {
//     using result_type =
//         entry_visit_callback<Machine, Event, EntryCoordinator, variant_type>;
//     using applicable_state_containers_list_type =
//         typename EntryCoordinator::applicable_state_containers_list_type<
//             tuple_type>;
//     template <std::size_t I>
//     using state_container_at_t =
//         element_at_t<I, applicable_state_containers_list_type>;

//     template <std::size_t I>
//     requires(
//         I <
//         length_v<applicable_state_containers_list_type>) constexpr
//         result_type find(EntryCoordinator const &entry_coordinator,
//              tuple_type &state_containers) const noexcept {
//       auto &state_container =
//           std::get<state_container_at_t<I>>(state_containers);
//       return entry_coordinator.has_scheduled_state(state_container)
//                  ? entry_visit_callback<
//                        Machine, Event, EntryCoordinator,
//                        variant_type>::for_entry_of(&state_container)
//                  : this->template find<I + 1>(entry_coordinator,
//                                               state_containers);
//     }

//     template <std::size_t I>
//     constexpr result_type find(EntryCoordinator const &,
//                                tuple_type &) const noexcept {
//       return {};
//     }
//   };
// };
} // namespace flat_state_container_details_

template <concepts::state_container... StateContainers>
using flat_states =
    flat_state_container_details_::container<StateContainers...>;

} // namespace skizzay::fsm
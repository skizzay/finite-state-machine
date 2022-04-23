#include <skizzay/fsm/flat_state_container.h>

#include "skizzay/fsm/single_state_container.h"
#include "test_objects.h"

#include <catch2/catch.hpp>
#include <cstddef>

using namespace skizzay::fsm;

namespace {
inline constexpr std::size_t num_events = 3;
}

template <std::size_t I>
using target_test_state = test_objects::test_state<I, num_events>;

SCENARIO("flat state container is a state container",
         "[unit][state-container][flat-state-container]") {
  using target_type = flat_states<single_state<target_test_state<0>>,
                                  single_state<target_test_state<1>>>;
  THEN("flat state container is a state container") {
    REQUIRE(is_state_container<target_type>::value);
    REQUIRE(concepts::state_container<target_type>);
  }
}

// SCENARIO("flat state container event handling",
//          "[unit][state-container][flat-state-container]") {
//   GIVEN("two flat states") {
//     using target_type = flat_states<single_state<target_test_state<0>>,
//                                     single_state<target_test_state<1>>>;
//     target_type target;

//     AND_GIVEN("a finite state machine") {
//       fake_machine machine{std::tuple{
//           guarded_transition<
//               target_test_state<0>, target_test_state<0>, test_event<0>,
//               bool (target_test_state<0>::*)(test_event<0> const &)
//                   const noexcept>{&target_test_state<0>::accepts},
//           guarded_transition<
//               target_test_state<0>, target_test_state<1>, test_event<1>,
//               bool (target_test_state<0>::*)(test_event<1> const &)
//                   const noexcept>{&target_test_state<0>::accepts},
//           guarded_transition<
//               target_test_state<0>, target_test_state<2>, test_event<2>,
//               bool (target_test_state<0>::*)(test_event<2> const &)
//                   const noexcept>{&target_test_state<0>::accepts}}};

//       WHEN("initially entered") {
//         target.on_initial_entry(machine);

//         THEN("the container is active") {
//           REQUIRE(target.is_active());
//           REQUIRE_FALSE(target.is_inactive());
//         }

//         THEN("the first state was initially entered") {
//           target_test_state<0> const &s0 =
//               target.current_state<target_test_state<0>>();
//           REQUIRE(1 == s0.initial_entry_count);
//           REQUIRE(0 == s0.total_event_entry_count());
//           REQUIRE(target.is<target_test_state<0>>());

//           AND_THEN("the second state is inactive") {
//             REQUIRE_FALSE(target.is<target_test_state<1>>());
//           }
//         }

//         AND_WHEN("finally exited") {
//           target.on_final_exit(machine);

//           THEN("the container is inactive") {
//             REQUIRE_FALSE(target.is_active());
//             REQUIRE(target.is_inactive());

//             AND_THEN("the first state is no longer current") {
//               REQUIRE_FALSE(
//                   target.current_state<target_test_state<0>>().has_value());
//               REQUIRE_FALSE((target.is<target_test_state<0>>()));
//             }
//           }
//         }

//         AND_WHEN("an event exits the container") {
//           constexpr std::size_t exit_event_id = 2;
//           test_event<exit_event_id> exit_event;
//           fake_transition_coordinator coordinator{machine, exit_event};
//           target_test_state<0> const &s0 =
//               target.current_state<target_test_state<0>>();

//           bool const event_handling_result =
//               target.on_event(coordinator, machine, exit_event);

//           THEN("the event was handled") {
//             REQUIRE(event_handling_result);

//             AND_THEN("the container is inactive") {
//               REQUIRE_FALSE(target.is_active());
//               REQUIRE(target.is_inactive());
//             }

//             AND_THEN("the state exited due to the event") {
//               REQUIRE(0 == s0.final_exit_count);
//               REQUIRE(1 == s0.event_exit_count.at(exit_event_id));
//             }
//           }
//         }

//         AND_WHEN("an event is not triggered") {
//           constexpr std::size_t exit_event_id = 2;
//           test_event<exit_event_id> exit_event{false};
//           fake_transition_coordinator coordinator{machine, exit_event};

//           bool const event_handling_result =
//               target.on_event(coordinator, machine, exit_event);

//           THEN("the event was not handled") {
//             REQUIRE_FALSE(event_handling_result);

//             AND_THEN("the container is active") {
//               REQUIRE(target.is_active());
//               REQUIRE_FALSE(target.is_inactive());
//             }
//           }
//         }

//         AND_WHEN("an event transitions within the container") {
//           constexpr std::size_t exit_event_id = 1;
//           test_event<exit_event_id> exit_event;
//           fake_transition_coordinator coordinator{machine, exit_event};
//           target_test_state<0> const &s0 =
//               target.current_state<target_test_state<0>>();

//           bool const event_handling_result =
//               target.on_event(coordinator, machine, exit_event);

//           THEN("the event was handled") {
//             REQUIRE(event_handling_result);

//             AND_THEN("the state exited due to the event") {
//               REQUIRE(0 == s0.final_exit_count);
//               REQUIRE(1 == s0.event_exit_count.at(exit_event_id));
//             }
//           }

//           AND_WHEN("the other state is entered") {
//             entry_coordinator<states_list<target_test_state<1>>> ec;
//             ec.schedule_entry<target_test_state<1>>();
//             target.on_entry(ec, machine, exit_event);

//             THEN("the container is active") {
//               REQUIRE(target.is_active());
//               REQUIRE_FALSE(target.is_inactive());
//             }

//             THEN("the other state entered due to the event") {
//               target_test_state<1> const &s1 =
//                   target.current_state<target_test_state<1>>();
//               REQUIRE(0 == s1.initial_entry_count);
//               REQUIRE(1 == s1.event_entry_count.at(exit_event_id));
//             }
//           }
//         }
//       }
//     }
//   }
// }
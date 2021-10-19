#include "test_objects.h"
#include <catch.hpp>
#include <skizzay/fsm/entry_coordinator.h>
#include <skizzay/fsm/guarded_transition.h>
#include <skizzay/fsm/orthogonal_state_container.h>
#include <skizzay/fsm/simple_transition.h>
#include <skizzay/fsm/single_state_container.h>

using namespace skizzay::fsm;

namespace {

template <std::size_t Id> using target_test_state = test_state<Id, 6>;
using target_type = orthogonal_states<single_state<target_test_state<0>>,
                                      single_state<target_test_state<1>>>;
} // namespace

SCENARIO("orthogonal state container is a state container",
         "[unit][state-container][orthogonal-state-container]") {
  THEN("orthogonal state container is a state container") {
    REQUIRE(has_states_list_type<target_type>::value);
    REQUIRE(is_state_container<target_type>::value);
    REQUIRE(concepts::state_container<target_type>);
  }
}

SCENARIO("orthogonal state container event handling",
         "[unit][state-container][orthogonal-state-container]") {
  GIVEN("two orthogonal states") {
    target_type target;

    AND_GIVEN("a finite state machine") {
      fake_machine machine{std::tuple{
          simple_transition<target_test_state<0>, target_test_state<0>,
                            test_event<0>>{},
          simple_transition<target_test_state<1>, target_test_state<1>,
                            test_event<1>>{},
          simple_transition<target_test_state<2>, target_test_state<0>,
                            test_event<2>>{},
          simple_transition<target_test_state<2>, target_test_state<1>,
                            test_event<2>>{},
          simple_transition<target_test_state<2>, target_test_state<0>,
                            test_event<3>>{},
          simple_transition<target_test_state<0>, target_test_state<2>,
                            test_event<4>>{},
          simple_transition<target_test_state<1>, target_test_state<2>,
                            test_event<4>>{},
          guarded_transition<
              target_test_state<0>, target_test_state<2>, test_event<5>,
              bool (target_test_state<0>::*)(test_event<5> const &)
                  const noexcept>{&target_test_state<0>::accepts}}};

      WHEN("initially entered") {
        target.on_initial_entry(machine);

        THEN("all states are active") {
          REQUIRE(target.is_active());
          REQUIRE_FALSE(target.is_inactive());
          REQUIRE(target.is<target_test_state<0>>());
          REQUIRE(target.is<target_test_state<1>>());
        }

        AND_WHEN("finally exited") {
          target.on_final_exit(machine);

          THEN("all states are inactive") {
            REQUIRE_FALSE(target.is_active());
            REQUIRE(target.is_inactive());
            REQUIRE_FALSE(target.is<target_test_state<0>>());
            REQUIRE_FALSE(target.is<target_test_state<1>>());
          }
        }
      }

      WHEN("entering from an event that targets each state") {
        constexpr std::size_t entry_event_id = 0;
        test_event<entry_event_id> event;
        entry_coordinator<
            states_list<target_test_state<0>, target_test_state<1>>>
            ec;

        ec.schedule_entry<target_test_state<0>>();
        ec.schedule_entry<target_test_state<1>>();
        target.on_entry(ec, machine, event);

        THEN("both states are event entered") {
          auto const ensure_event_entrance = [&]<std::size_t I>() {
            target_test_state<I> const &s =
                target.template current_state<target_test_state<I>>();
            REQUIRE(1 == s.event_entry_count.at(entry_event_id));
          };
          ensure_event_entrance.operator()<0>();
          ensure_event_entrance.operator()<1>();
        }

        THEN("all states are active") {
          REQUIRE(target.is_active());
          REQUIRE_FALSE(target.is_inactive());
          REQUIRE(target.is<target_test_state<0>>());
          REQUIRE(target.is<target_test_state<1>>());
        }

        AND_WHEN("finally exited") {
          target.on_final_exit(machine);

          THEN("all states are inactive") {
            REQUIRE_FALSE(target.is_active());
            REQUIRE(target.is_inactive());
            REQUIRE_FALSE(target.is<target_test_state<0>>());
            REQUIRE_FALSE(target.is<target_test_state<1>>());
          }
        }
      }

      WHEN("entering from an event that targets only one state") {
        constexpr std::size_t entry_event_id = 3;
        test_event<entry_event_id> event;
        entry_coordinator<states_list<target_test_state<0>>> ec;

        ec.schedule_entry<target_test_state<0>>();
        target.on_entry(ec, machine, event);

        THEN("one state is event entered") {
          target_test_state<0> const &s0 =
              target.template current_state<target_test_state<0>>();
          REQUIRE(0 == s0.initial_entry_count);
          REQUIRE(1 == s0.event_entry_count.at(entry_event_id));

          AND_THEN("the other state is initially entered") {
            target_test_state<1> const &s1 =
                target.template current_state<target_test_state<1>>();
            REQUIRE(1 == s1.initial_entry_count);
            REQUIRE(0 == s1.event_entry_count.at(entry_event_id));
          }
        }

        THEN("all states are active") {
          REQUIRE(target.is_active());
          REQUIRE_FALSE(target.is_inactive());
          REQUIRE(target.is<target_test_state<0>>());
          REQUIRE(target.is<target_test_state<1>>());
        }

        AND_WHEN("finally exited") {
          target.on_final_exit(machine);

          THEN("all states are inactive") {
            REQUIRE_FALSE(target.is_active());
            REQUIRE(target.is_inactive());
            REQUIRE_FALSE(target.is<target_test_state<0>>());
            REQUIRE_FALSE(target.is<target_test_state<1>>());
          }
        }

        AND_WHEN("an event exits both states") {
          constexpr std::size_t exit_event_id = 4;
          test_event<exit_event_id> exit_event;
          fake_transition_coordinator coordinator{machine, exit_event};
          target_test_state<0> const &s0 =
              target.current_state<target_test_state<0>>();
          target_test_state<1> const &s1 =
              target.current_state<target_test_state<1>>();

          bool const event_handling_result =
              target.on_event(coordinator, machine, exit_event);

          THEN("the event was handled") { REQUIRE(event_handling_result); }

          THEN("both states were exited due to the event") {
            REQUIRE(1 == std::get<exit_event_id>(s0.event_exit_count));
            REQUIRE(1 == std::get<exit_event_id>(s1.event_exit_count));

            AND_THEN("neither state was finally exited") {
              REQUIRE(0 == s0.final_exit_count);
              REQUIRE(0 == s1.final_exit_count);
            }
          }

          THEN("all states are inactive") {
            REQUIRE_FALSE(target.is_active());
            REQUIRE(target.is_inactive());
            REQUIRE_FALSE(target.is<target_test_state<0>>());
            REQUIRE_FALSE(target.is<target_test_state<1>>());
          }
        }

        AND_WHEN("an event exits a single states") {
          constexpr std::size_t exit_event_id = 5;
          test_event<exit_event_id> exit_event;
          fake_transition_coordinator coordinator{machine, exit_event};
          target_test_state<0> const &s0 =
              target.current_state<target_test_state<0>>();
          target_test_state<1> const &s1 =
              target.current_state<target_test_state<1>>();

          bool const event_handling_result =
              target.on_event(coordinator, machine, exit_event);

          THEN("the event was handled") { REQUIRE(event_handling_result); }

          THEN("only one state was exited due to event") {
            REQUIRE(1 == std::get<exit_event_id>(s0.event_exit_count));
            REQUIRE(0 == s0.final_exit_count);

            AND_THEN("the other state was finally exited") {
              REQUIRE(0 == std::get<exit_event_id>(s1.event_exit_count));
              REQUIRE(1 == s1.final_exit_count);
            }
          }

          THEN("all states are inactive") {
            REQUIRE_FALSE(target.is_active());
            REQUIRE(target.is_inactive());
            REQUIRE_FALSE(target.is<target_test_state<0>>());
            REQUIRE_FALSE(target.is<target_test_state<1>>());
          }
        }

        AND_WHEN("an event that doesn't exit the container") {
          constexpr std::size_t exit_event_id = 5;
          test_event<exit_event_id> exit_event{false};
          fake_transition_coordinator coordinator{machine, exit_event};
          target_test_state<0> const &s0 =
              target.current_state<target_test_state<0>>();
          target_test_state<1> const &s1 =
              target.current_state<target_test_state<1>>();

          bool const event_handling_result =
              target.on_event(coordinator, machine, exit_event);

          THEN("the event was not handled") {
            REQUIRE_FALSE(event_handling_result);
          }

          THEN("nothing was exited") {
            REQUIRE(0 == std::get<exit_event_id>(s0.event_exit_count));
            REQUIRE(0 == s0.final_exit_count);
            REQUIRE(0 == std::get<exit_event_id>(s1.event_exit_count));
            REQUIRE(0 == s1.final_exit_count);
          }

          THEN("all states are active") {
            REQUIRE(target.is_active());
            REQUIRE_FALSE(target.is_inactive());
            REQUIRE(target.is<target_test_state<0>>());
            REQUIRE(target.is<target_test_state<1>>());
          }
        }
      }
    }
  }
}
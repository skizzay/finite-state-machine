#include "test_objects.h"
#include <catch.hpp>
#include <cstddef>
#include <skizzay/fsm/guarded_transition.h>
#include <skizzay/fsm/hierarchical_state_container.h>
#include <skizzay/fsm/single_state_container.h>

using namespace skizzay::fsm;

namespace {
constexpr std::size_t num_events = 4;

using parent_state_type = test_state<0, num_events>;
using child_state_type = test_state<1, num_events>;
using external_state_type = test_state<2, num_events>;
using target_type =
    hierarchical_states<parent_state_type, single_state<child_state_type>>;
} // namespace

SCENARIO("hierarchical state container is state container",
         "[unit][state-container][hierarchical-state-container]") {
  THEN("simple state container is a state container") {
    REQUIRE(is_state_container<target_type>::value);
    REQUIRE(concepts::state_container<target_type>);
  }
}

SCENARIO("hierarchical state container event handling",
         "[unit][state-container][hierarchical-state-container]") {
  GIVEN("an hierarchical state relationship") {
    target_type target;

    AND_GIVEN("a finite state machine") {
      fake_machine machine{std::tuple{
          guarded_transition<parent_state_type, parent_state_type,
                             test_event<0>,
                             bool (parent_state_type::*)(test_event<0> const &)
                                 const noexcept>{&parent_state_type::accepts},
          guarded_transition<child_state_type, child_state_type, test_event<1>,
                             bool (child_state_type::*)(test_event<1> const &)
                                 const noexcept>{&child_state_type::accepts},
          guarded_transition<parent_state_type, external_state_type,
                             test_event<2>,
                             bool (parent_state_type::*)(test_event<2> const &)
                                 const noexcept>{&parent_state_type::accepts},
          guarded_transition<
              child_state_type, external_state_type, test_event<3>,
              bool (child_state_type::*)(test_event<3> const &) const noexcept>{
              &child_state_type::accepts}}};

      THEN("it is inactive") {
        REQUIRE_FALSE(target.is_active());
        REQUIRE(target.is_inactive());
        REQUIRE_FALSE(target.is<parent_state_type>());
        REQUIRE_FALSE(target.is<child_state_type>());
      }

      WHEN("initially entered") {
        target.on_initial_entry(machine);
        parent_state_type const &parent =
            target.current_state<parent_state_type>();
        child_state_type const &child =
            target.current_state<child_state_type>();

        THEN("it is active") {
          REQUIRE(target.is_active());
          REQUIRE_FALSE(target.is_inactive());
          REQUIRE(target.is<parent_state_type>());
          REQUIRE(target.is<child_state_type>());

          AND_THEN("the parent was initially entered") {
            REQUIRE(1 == parent.initial_entry_count);
          }

          AND_THEN("the child was initially entered") {
            REQUIRE(1 == child.initial_entry_count);
          }
        }

        AND_WHEN("finally exited") {
          target.on_final_exit(machine);

          THEN("it is inactive") {
            REQUIRE_FALSE(target.is_active());
            REQUIRE(target.is_inactive());
            REQUIRE_FALSE(target.is<parent_state_type>());
            REQUIRE_FALSE(target.is<child_state_type>());
          }
        }

        AND_WHEN("an event reentering the parent is triggered") {
          constexpr std::size_t parent_reentry_event_id = 0;
          test_event<parent_reentry_event_id> parent_reentry_event;
          fake_transition_coordinator coordinator{machine,
                                                  parent_reentry_event};

          bool const event_handling_result =
              target.on_event(coordinator, machine, parent_reentry_event);

          THEN("the event was handled") {
            REQUIRE(event_handling_result);

            THEN("it is active") {
              REQUIRE(target.is_active());
              REQUIRE_FALSE(target.is_inactive());
              REQUIRE(target.is<parent_state_type>());
              REQUIRE(target.is<child_state_type>());

              AND_THEN("the parent was reentered due to the event") {
                REQUIRE(1 ==
                        parent.event_reentry_count.at(parent_reentry_event_id));
              }

              AND_THEN("the child was not affected") {
                REQUIRE(0 ==
                        child.event_reentry_count.at(parent_reentry_event_id));
              }
            }

            AND_THEN("the parent state was reentered") {
              REQUIRE(1 ==
                      parent.event_reentry_count.at(parent_reentry_event_id));

              AND_THEN("the child state was unaffected") {
                REQUIRE(0 ==
                        child.event_exit_count.at(parent_reentry_event_id));
                REQUIRE(0 ==
                        child.event_reentry_count.at(parent_reentry_event_id));
              }
            }
          }
        }

        AND_WHEN("an event reentering the child is triggered") {
          constexpr std::size_t child_reentry_event_id = 1;
          test_event<child_reentry_event_id> child_reentry_event;
          fake_transition_coordinator coordinator{machine, child_reentry_event};

          bool const event_handling_result =
              target.on_event(coordinator, machine, child_reentry_event);

          THEN("the event was handled") {
            REQUIRE(event_handling_result);

            AND_THEN("it is active") {
              REQUIRE(target.is_active());
              REQUIRE_FALSE(target.is_inactive());
              REQUIRE(target.is<parent_state_type>());
              REQUIRE(target.is<child_state_type>());
            }

            AND_THEN("the child state exited") {
              REQUIRE(1 ==
                      child.event_reentry_count.at(child_reentry_event_id));

              AND_THEN("the parent state exited") {
                REQUIRE(0 ==
                        parent.event_exit_count.at(child_reentry_event_id));
                REQUIRE(0 ==
                        parent.event_reentry_count.at(child_reentry_event_id));
              }
            }
          }
        }

        AND_WHEN("an event handled by the parent exits the container") {
          constexpr std::size_t parent_exit_event_id = 2;
          test_event<parent_exit_event_id> parent_exit_event;
          fake_transition_coordinator coordinator{machine, parent_exit_event};

          bool const event_handling_result =
              target.on_event(coordinator, machine, parent_exit_event);

          THEN("the event was handled") {
            REQUIRE(event_handling_result);

            AND_THEN("it is inactive") {
              REQUIRE_FALSE(target.is_active());
              REQUIRE(target.is_inactive());
              REQUIRE_FALSE(target.is<parent_state_type>());
              REQUIRE_FALSE(target.is<child_state_type>());
            }

            AND_THEN("the parent state exited") {
              REQUIRE(1 == parent.event_exit_count.at(parent_exit_event_id));

              AND_THEN("the child state exited") {
                REQUIRE(1 == child.final_exit_count);
              }
            }
          }
        }

        AND_WHEN("an event handled by the child exits the container") {
          constexpr std::size_t child_exit_event_id = 3;
          test_event<child_exit_event_id> child_exit_event;
          fake_transition_coordinator coordinator{machine, child_exit_event};

          bool const event_handling_result =
              target.on_event(coordinator, machine, child_exit_event);

          THEN("the event was handled") {
            REQUIRE(event_handling_result);

            AND_THEN("it is inactive") {
              REQUIRE_FALSE(target.is_active());
              REQUIRE(target.is_inactive());
              REQUIRE_FALSE(target.is<parent_state_type>());
              REQUIRE_FALSE(target.is<child_state_type>());
            }

            AND_THEN("the child state exited") {
              REQUIRE(1 == child.event_exit_count.at(child_exit_event_id));

              AND_THEN("the parent state exited") {
                REQUIRE(1 == parent.final_exit_count);
              }
            }
          }
        }
      }
    }
  }
}
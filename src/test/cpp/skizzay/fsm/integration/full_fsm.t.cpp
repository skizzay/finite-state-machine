#include <catch.hpp>
#include <iostream>
#include <skizzay/fsm/concepts.h>
#include <skizzay/fsm/event.h>
#include <skizzay/fsm/finite_state_machine.h>
#include <skizzay/fsm/flat_state_container.h>
#include <skizzay/fsm/hierarchical_state_container.h>
#include <skizzay/fsm/orthogonal_state_container.h>
#include <skizzay/fsm/simple_transition.h>
#include <skizzay/fsm/single_state_container.h>

using namespace skizzay::fsm;

namespace {
template <std::size_t I> struct test_event {};
template <std::size_t I> struct test_state {
  template <concepts::machine_for<test_state<I>> Machine>
  requires concepts::event_in<epsilon_event_t, events_list_t<Machine>> &&
      (0 == I) void on_entry(Machine &machine, initial_entry_event_t const &) {
    machine.on(epsilon_event);
  }
};
} // namespace

SCENARIO("Full finite state machine without guards", "[integration]") {
  GIVEN("a transition table") {
    std::tuple transition_table{
        simple_transition<test_state<0>, test_state<1>, epsilon_event_t>{},
        simple_transition<test_state<1>, test_state<2>, test_event<1>>{},
        simple_transition<test_state<1>, test_state<1>, test_event<2>>{},
        simple_transition<test_state<1>, test_state<8>, test_event<7>>{},
        simple_transition<test_state<3>, test_state<4>, test_event<3>>{},
        simple_transition<test_state<4>, test_state<3>, test_event<4>>{},
        simple_transition<test_state<5>, test_state<6>, test_event<5>>{},
        simple_transition<test_state<6>, test_state<1>, test_event<6>>{},
        simple_transition<test_state<8>, test_state<7>, test_event<8>>{},
        simple_transition<test_state<7>, test_state<1>, test_event<9>>{},
        simple_transition<test_state<2>, test_state<2>, test_event<10>>{},
        simple_transition<test_state<2>, test_state<9>, test_event<11>>{}};

    AND_GIVEN("a root state container") {
      flat_state root{
          single_state{test_state<0>{}}, single_state{test_state<1>{}},
          parent_state_with_external_self_transitions{
              test_state<2>{},
              orthogonal_states{flat_state{single_state{test_state<3>{}},
                                           single_state{test_state<4>{}}},
                                flat_state{single_state{test_state<5>{}},
                                           single_state{test_state<6>{}}},
                                flat_state{single_state{test_state<7>{}},
                                           single_state{test_state<8>{}}}}},
          single_state{test_state<9>{}}};

      AND_GIVEN("a machine") {
        finite_state_machine target{std::move(transition_table),
                                    std::move(root)};

        WHEN("started") {
          target.start();

          THEN("the current state is 1") {
            REQUIRE(target.is<test_state<1>>());
          }

          AND_WHEN("event 2 is fired") {
            target.on(test_event<2>{});

            THEN("the current state is 1") {
              REQUIRE(target.is<test_state<1>>());
            }
          }

          AND_WHEN("event 1 is fired") {
            target.on(test_event<1>{});

            THEN("the current states are 2, 3, 5, & 7") {
              REQUIRE(target.is<test_state<2>>());
              REQUIRE(target.is<test_state<3>>());
              REQUIRE(target.is<test_state<5>>());
              REQUIRE(target.is<test_state<7>>());
            }

            AND_WHEN("event 3 is fired") {
              target.on(test_event<3>{});

              THEN("the current states are 2, 4, 5, & 7") {
                REQUIRE(target.is<test_state<2>>());
                REQUIRE(target.is<test_state<4>>());
                REQUIRE(target.is<test_state<5>>());
                REQUIRE(target.is<test_state<7>>());
              }

              AND_WHEN("event 4 is fired") {
                target.on(test_event<4>{});

                THEN("the current states are 2, 3, 5, & 7") {
                  REQUIRE(target.is<test_state<2>>());
                  REQUIRE(target.is<test_state<3>>());
                  REQUIRE(target.is<test_state<5>>());
                  REQUIRE(target.is<test_state<7>>());
                }
              }
            }

            AND_WHEN("event 5 is fired") {
              target.on(test_event<5>{});

              THEN("the current states are 2, 3, 6, & 7") {
                REQUIRE(target.is<test_state<2>>());
                REQUIRE(target.is<test_state<3>>());
                REQUIRE(target.is<test_state<6>>());
                REQUIRE(target.is<test_state<7>>());
              }

              AND_WHEN("event 6 is fired") {
                target.on(test_event<6>{});

                THEN("the current state is 1") {
                  REQUIRE(target.is<test_state<1>>());
                }
              }
            }
          }

          AND_WHEN("event 7 is fired") {
            target.on(test_event<7>{});

            THEN("the current states are 2, 3, 5, & 8") {
              REQUIRE(target.is<test_state<2>>());
              REQUIRE(target.is<test_state<3>>());
              REQUIRE(target.is<test_state<5>>());
              REQUIRE(target.is<test_state<8>>());
            }
          }
        }
      }
    }
  }
}
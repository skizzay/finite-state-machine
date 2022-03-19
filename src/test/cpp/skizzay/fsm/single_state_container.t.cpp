
#include <skizzay/fsm/single_state_container.h>

#include "skizzay/fsm/state_container.h"
#include "skizzay/fsm/type_list.h"
#include "skizzay/fsm/types.h"
#include "test_objects.h"

#include <catch2/catch.hpp>
#include <memory>
#include <tuple>
#include <vector>

using namespace skizzay::fsm;

namespace {

inline constexpr std::size_t num_events = 4;
template <std::size_t Id>
using target_test_state = test_objects::test_state<Id, num_events>;
using test_event_type = test_objects::test_event<0>;
using test_state_type = target_test_state<0>;
} // namespace

SCENARIO("single state container is a state container",
         "[unit][state-container][single-state-container]") {
  using target_type = single_state<target_test_state<0>>;
  THEN("single state container is a state container") {
    REQUIRE(is_state_container<target_type>::value);
    REQUIRE(concepts::state_container<target_type>);
  }
}

SCENARIO("single state container event handling",
         "[unit][state_container][single_state_container]") {
  using target_type = single_state<test_state_type>;
  using transition_table_type =
      std::tuple<simple_transition<target_test_state<0>, target_test_state<0>,
                                   test_objects::test_event<0>>,
                 simple_transition<target_test_state<0>, target_test_state<1>,
                                   test_objects::test_event<1>>,
                 simple_transition<target_test_state<1>, target_test_state<0>,
                                   test_objects::test_event<1>>,
                 simple_transition<target_test_state<0>, target_test_state<0>,
                                   test_objects::test_event<2>>,
                 simple_transition<target_test_state<0>, target_test_state<1>,
                                   test_objects::test_event<2>>,
                 simple_transition<target_test_state<0>, target_test_state<1>,
                                   test_objects::test_event<3>>,
                 simple_transition<target_test_state<0>, target_test_state<0>,
                                   test_objects::test_event<3>>>;

  GIVEN("a single state container") {
    target_type target;

    THEN("it is inactive") {
      REQUIRE(target.is_inactive());
      REQUIRE_FALSE(target.is_active());
    }

    WHEN("initially entered") {
      test_objects::fake_entry_context<initial_entry_event_t,
                                       states_list_t<transition_table_type>,
                                       events_list_t<transition_table_type>>
          initial_entry_context;
      target.on_entry(initial_entry_context);

      THEN("it has entered initially") {
        REQUIRE(1 == target.state<test_state_type>().initial_entry_count);
      }

      THEN("it is active") {
        REQUIRE(target.is_active());
        REQUIRE_FALSE(target.is_inactive());
      }

      AND_WHEN("finally exited") {
        test_objects::fake_event_transition_context<final_exit_event_t,
                                                    transition_table_type>
            event_transition_context;
        bool const actual = target.on_event(event_transition_context);

        THEN("the event was handled") {
          REQUIRE(actual);

          AND_THEN("it is inactive") {
            REQUIRE_FALSE(target.is_active());
            REQUIRE(target.is_inactive());
          }
        }
      }

      AND_WHEN("a reentering event is raised") {
        test_objects::fake_event_transition_context<test_objects::test_event<0>,
                                                    transition_table_type>
            event_transition_context;
        event_transition_context.e.pass_acceptance = true;
        test_objects::fake_entry_context<test_objects::test_event<0>,
                                         states_list_t<transition_table_type>,
                                         events_list_t<transition_table_type>>
            entry_context;
        entry_context.e = event_transition_context.e;

        bool const actual = target.on_event(event_transition_context);

        THEN("the event was handled") {
          REQUIRE(actual);
          AND_THEN("it is still active") {
            REQUIRE(target.is_active());
            REQUIRE_FALSE(target.is_inactive());
          }

          AND_WHEN("entry is attempted") {
            target.on_entry(entry_context);
            THEN("it was reentered") {
              REQUIRE(1 ==
                      target.state<test_state_type>().event_reentry_count[0]);
            }
          }
        }
      }

      AND_WHEN("an exiting event is raised") {
        test_objects::fake_event_transition_context<test_objects::test_event<1>,
                                                    transition_table_type>
            event_transition_context;
        event_transition_context.e.pass_acceptance = true;

        bool const actual = target.on_event(event_transition_context);

        THEN("the event was handled") {
          REQUIRE(actual);
          AND_THEN("it is inactive") {
            REQUIRE_FALSE(target.is_active());
            REQUIRE(target.is_inactive());
          }

          AND_THEN("the state exited") {
            REQUIRE(1 == target.state<test_state_type>().event_exit_count[1]);
          }
        }
      }

      AND_WHEN("a non-triggering event is raised") {
        test_objects::fake_event_transition_context<test_objects::test_event<1>,
                                                    transition_table_type>
            event_transition_context;
        event_transition_context.e.pass_acceptance = false;

        bool const actual = target.on_event(event_transition_context);

        THEN("the event was not handled") {
          REQUIRE_FALSE(actual);
          AND_THEN("it is still active") {
            REQUIRE(target.is_active());
            REQUIRE_FALSE(target.is_inactive());
          }
        }
      }

      AND_WHEN(
          "an ambiguously triggering event (reenter then exit) is raised") {
        test_objects::fake_event_transition_context<test_objects::test_event<2>,
                                                    transition_table_type>
            event_transition_context;
        event_transition_context.e.pass_acceptance = true;

        REQUIRE_THROWS_AS(target.on_event(event_transition_context),
                          state_transition_ambiguity);

        THEN("state transitition ambiguity has been thrown") { REQUIRE(true); }
      }

      AND_WHEN(
          "an ambiguously triggering event (exit then reenter) is raised") {
        test_objects::fake_event_transition_context<test_objects::test_event<3>,
                                                    transition_table_type>
            event_transition_context;
        event_transition_context.e.pass_acceptance = true;

        REQUIRE_THROWS_AS(target.on_event(event_transition_context),
                          state_transition_ambiguity);

        THEN("state transitition ambiguity has been thrown") { REQUIRE(true); }
      }
    }

    WHEN("entered based upon transition") {
      test_objects::fake_entry_context<test_objects::test_event<1>,
                                       states_list_t<transition_table_type>,
                                       events_list_t<transition_table_type>>
          entry_context;
      target.on_entry(entry_context);

      THEN("it has entered from the transition") {
        REQUIRE(1 == target.state<test_state_type>().event_entry_count[1]);
      }

      THEN("it is active") {
        REQUIRE(target.is_active());
        REQUIRE_FALSE(target.is_inactive());
      }

      AND_WHEN("finally exited") {
        test_objects::fake_event_transition_context<final_exit_event_t,
                                                    transition_table_type>
            event_transition_context;
        bool const actual = target.on_event(event_transition_context);

        THEN("the event was handled") {
          REQUIRE(actual);

          AND_THEN("it is inactive") {
            REQUIRE_FALSE(target.is_active());
            REQUIRE(target.is_inactive());
          }
        }
      }

      AND_WHEN("a reentering event is raised") {
        test_objects::fake_event_transition_context<test_objects::test_event<0>,
                                                    transition_table_type>
            event_transition_context;
        event_transition_context.e.pass_acceptance = true;

        bool const actual = target.on_event(event_transition_context);

        THEN("the event was handled") {
          REQUIRE(actual);
          AND_THEN("it is still active") {
            REQUIRE(target.is_active());
            REQUIRE_FALSE(target.is_inactive());
          }
        }
      }

      AND_WHEN("an exiting event is raised") {
        test_objects::fake_event_transition_context<test_objects::test_event<1>,
                                                    transition_table_type>
            event_transition_context;
        event_transition_context.e.pass_acceptance = true;

        bool const actual = target.on_event(event_transition_context);

        THEN("the event was handled") {
          REQUIRE(actual);
          AND_THEN("it is inactive") {
            REQUIRE_FALSE(target.is_active());
            REQUIRE(target.is_inactive());
          }
        }
      }

      AND_WHEN("a non-triggering event is raised") {
        test_objects::fake_event_transition_context<test_objects::test_event<1>,
                                                    transition_table_type>
            event_transition_context;
        event_transition_context.e.pass_acceptance = false;

        bool const actual = target.on_event(event_transition_context);

        THEN("the event was not handled") {
          REQUIRE_FALSE(actual);
          AND_THEN("it is still active") {
            REQUIRE(target.is_active());
            REQUIRE_FALSE(target.is_inactive());
          }
        }
      }

      AND_WHEN(
          "an ambiguously triggering event (reenter then exit) is raised") {
        test_objects::fake_event_transition_context<test_objects::test_event<2>,
                                                    transition_table_type>
            event_transition_context;
        event_transition_context.e.pass_acceptance = true;

        REQUIRE_THROWS_AS(target.on_event(event_transition_context),
                          state_transition_ambiguity);

        THEN("state transitition ambiguity has been thrown") { REQUIRE(true); }
      }

      AND_WHEN(
          "an ambiguously triggering event (exit then reenter) is raised") {
        test_objects::fake_event_transition_context<test_objects::test_event<3>,
                                                    transition_table_type>
            event_transition_context;
        event_transition_context.e.pass_acceptance = true;

        REQUIRE_THROWS_AS(target.on_event(event_transition_context),
                          state_transition_ambiguity);

        THEN("state transitition ambiguity has been thrown") { REQUIRE(true); }
      }
    }
  }
}

// SCENARIO("single state container event handling",
//          "[unit][state-container][single-state-container]") {
//   using type = target_test_state<0>;
//   using target_type = single_state<type>;
//   fake_machine machine{std::tuple{
//       simple_transition<target_test_state<0>, target_test_state<0>,
//                         test_event<0>>{},
//       simple_transition<target_test_state<1>, target_test_state<0>,
//                         test_event<1>>{},
//       guarded_transition<
//           target_test_state<0>, target_test_state<1>, test_event<2>,
//           bool (target_test_state<0>::*)(test_event<2> const &) const
//           noexcept>{ &target_test_state<0>::accepts}}};

//   GIVEN("a single state container") {
//     target_type target;
//     WHEN("initially entered") {
//       target.on_initial_entry(machine);

//       THEN("the entry callback was triggered") {
//         type const &current_state = target.current_state<type>();
//         REQUIRE(1 == current_state.initial_entry_count);
//       }

//       AND_WHEN("seeing if the container is that type") {
//         bool const actual = target.is<type>();

//         THEN("it's held") { REQUIRE(actual); }
//       }

//       AND_WHEN("handling an external event") {
//         constexpr std::size_t event_id = 2;
//         using event_type = test_event<event_id>;
//         event_type event;
//         fake_transition_coordinator coordinator{machine, event};

//         bool const had_transition =
//             target.on_event(coordinator, machine, event);

//         THEN("the event caused a transition") { REQUIRE(had_transition); }
//       }

//       AND_WHEN("finally exited") {
//         type const &current_state = target.current_state<type>();
//         target.on_final_exit(machine);

//         THEN("the exit callback was triggered") {
//           REQUIRE(1 == current_state.final_exit_count);
//         }

//         AND_WHEN("seeing if the container is that type") {
//           bool const actual = target.is<type>();

//           THEN("it's not held") { REQUIRE_FALSE(actual); }
//         }
//       }
//     }

//     AND_GIVEN("a triggering event") {
//       constexpr std::size_t event_id = 0;
//       using event_type = test_event<event_id>;
//       event_type event;
//       fake_transition_coordinator coordinator{machine, event};

//       WHEN("initially entered") {
//         target.on_initial_entry(machine);

//         AND_WHEN("the event is handled") {
//           bool const on_event_result =
//               target.on_event(coordinator, machine, event);

//           THEN("a transition was triggered") { REQUIRE(on_event_result); }
//           THEN("the state is active") {
//             REQUIRE(target.is_active());
//             REQUIRE_FALSE(target.is_inactive());
//           }
//         }
//       }

//       WHEN("entered") {
//         entry_coordinator<states_list<type>> ec;
//         ec.schedule_entry<type>();
//         target.on_entry(ec, machine, event);
//         THEN("the state is active") {
//           REQUIRE(target.is_active());
//           REQUIRE_FALSE(target.is_inactive());
//         }
//       }
//     }

//     AND_GIVEN("a non-triggering event") {
//       constexpr std::size_t event_id = 2;
//       using event_type = test_event<event_id>;
//       event_type event{false};
//       fake_transition_coordinator coordinator{machine, event};

//       WHEN("initially entered") {
//         target.on_initial_entry(machine);

//         WHEN("the event is handled") {
//           bool const on_event_result =
//               target.on_event(coordinator, machine, event);

//           THEN("a transition was not triggered") {
//             REQUIRE_FALSE(on_event_result);
//           }
//         }
//       }
//     }

//     AND_GIVEN("an unhandled event") {
//       constexpr std::size_t event_id = 1;
//       using event_type = test_event<event_id>;
//       event_type event;
//       fake_transition_coordinator coordinator{machine, event};

//       WHEN("initially entered") {
//         target.on_initial_entry(machine);

//         WHEN("the event is handled") {
//           bool const on_event_result =
//               target.on_event(coordinator, machine, event);

//           THEN("a transition was not triggered") {
//             REQUIRE_FALSE(on_event_result);
//           }
//         }
//       }
//     }
//   }
// }

SCENARIO("single state container state examining",
         "[unit][state-container][single-state-container]") {
  using target_type = single_state<target_test_state<0>>;
  GIVEN("a type held by the container") {
    using type = target_test_state<0>;
    WHEN("seeing if the container holds that type") {
      bool const actual = contains_v<states_list_t<target_type>, type>;

      THEN("it's held") { REQUIRE(actual); }
    }

    AND_GIVEN("an instance of the container") {
      target_type target;

      WHEN("seeing if the container is that type") {
        bool const actual = target.is<type>();

        THEN("it's not held") { REQUIRE_FALSE(actual); }
      }
    }
  }

  GIVEN("a type not held by the container") {
    using type = target_test_state<1>;
    WHEN("seeing if the container holds that type") {
      bool const actual = contains_v<states_list_t<target_type>, type>;

      THEN("it's not held") { REQUIRE_FALSE(actual); }
    }
  }
}
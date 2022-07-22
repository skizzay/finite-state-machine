
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

    THEN("it returns an empty current state") {
      REQUIRE_FALSE(target.current_state().has_value());
    }

    WHEN("initially entered") {
      test_objects::fake_entry_context<initial_entry_event_t,
                                       states_list_t<transition_table_type>,
                                       events_list_t<transition_table_type>>
          initial_entry_context;
      execute_initial_entry(target, initial_entry_context,
                            initial_entry_context);

      THEN("it returns a populated current state") {
        REQUIRE(target.current_state().has_value());
      }

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
        bool const actual = execute_final_exit(target, event_transition_context,
                                               event_transition_context);

        THEN("the event was handled") {
          REQUIRE(actual);

          AND_THEN("it is inactive") {
            REQUIRE_FALSE(target.is_active());
            REQUIRE(target.is_inactive());
          }
        }
      }

      AND_WHEN("a reentering event is raised") {
        test_objects::test_event<0> event{true};
        test_objects::fake_event_transition_context<test_objects::test_event<0>,
                                                    transition_table_type>
            event_transition_context;
        test_objects::fake_entry_context<test_objects::test_event<0>,
                                         states_list_t<transition_table_type>,
                                         events_list_t<transition_table_type>>
            entry_context;

        bool const actual =
            target.on_event(event_transition_context, event,
                            event_transition_context, event_transition_context);

        THEN("the event was handled") {
          REQUIRE(actual);
          AND_THEN("it is still active") {
            REQUIRE(target.is_active());
            REQUIRE_FALSE(target.is_inactive());
          }

          AND_WHEN("entry is attempted") {
            target.on_entry(entry_context, event, entry_context, entry_context);
            THEN("it was reentered") {
              REQUIRE(1 ==
                      target.state<test_state_type>().event_reentry_count[0]);
            }
          }
        }
      }

      AND_WHEN("an exiting event is raised") {
        test_objects::test_event<1> event{true};
        test_objects::fake_event_transition_context<test_objects::test_event<1>,
                                                    transition_table_type>
            event_transition_context;

        bool const actual =
            target.on_event(event_transition_context, event,
                            event_transition_context, event_transition_context);

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
        test_objects::test_event<1> event{false};
        test_objects::fake_event_transition_context<test_objects::test_event<1>,
                                                    transition_table_type>
            event_transition_context;

        bool const actual =
            target.on_event(event_transition_context, event,
                            event_transition_context, event_transition_context);

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
        test_objects::test_event<2> event{true};
        test_objects::fake_event_transition_context<test_objects::test_event<2>,
                                                    transition_table_type>
            event_transition_context;

        REQUIRE_THROWS_AS(target.on_event(event_transition_context, event,
                                          event_transition_context,
                                          event_transition_context),
                          state_transition_ambiguity);

        THEN("state transitition ambiguity has been thrown") { REQUIRE(true); }
      }

      AND_WHEN(
          "an ambiguously triggering event (exit then reenter) is raised") {
        test_objects::test_event<3> event{true};
        test_objects::fake_event_transition_context<test_objects::test_event<3>,
                                                    transition_table_type>
            event_transition_context;

        REQUIRE_THROWS_AS(target.on_event(event_transition_context, event,
                                          event_transition_context,
                                          event_transition_context),
                          state_transition_ambiguity);

        THEN("state transitition ambiguity has been thrown") { REQUIRE(true); }
      }
    }

    WHEN("entered based upon transition") {
      test_objects::test_event<1> event;
      test_objects::fake_entry_context<test_objects::test_event<1>,
                                       states_list_t<transition_table_type>,
                                       events_list_t<transition_table_type>>
          entry_context;
      target.on_entry(entry_context, event, entry_context, entry_context);

      THEN("it returns a populated current state") {
        REQUIRE(target.current_state().has_value());
      }

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
        bool const actual = execute_final_exit(target, event_transition_context,
                                               event_transition_context);

        THEN("the event was handled") {
          REQUIRE(actual);

          AND_THEN("it is inactive") {
            REQUIRE_FALSE(target.is_active());
            REQUIRE(target.is_inactive());
          }
        }
      }

      AND_WHEN("a reentering event is raised") {
        test_objects::test_event<0> event{true};
        test_objects::fake_event_transition_context<test_objects::test_event<0>,
                                                    transition_table_type>
            event_transition_context;

        bool const actual =
            target.on_event(event_transition_context, event,
                            event_transition_context, event_transition_context);

        THEN("the event was handled") {
          REQUIRE(actual);
          AND_THEN("it is still active") {
            REQUIRE(target.is_active());
            REQUIRE_FALSE(target.is_inactive());
          }
        }
      }

      AND_WHEN("an exiting event is raised") {
        test_objects::test_event<1> event{true};
        test_objects::fake_event_transition_context<test_objects::test_event<1>,
                                                    transition_table_type>
            event_transition_context;

        bool const actual =
            target.on_event(event_transition_context, event,
                            event_transition_context, event_transition_context);

        THEN("the event was handled") {
          REQUIRE(actual);
          AND_THEN("it is inactive") {
            REQUIRE_FALSE(target.is_active());
            REQUIRE(target.is_inactive());
          }
        }
      }

      AND_WHEN("a non-triggering event is raised") {
        test_objects::test_event<1> event{false};
        test_objects::fake_event_transition_context<test_objects::test_event<1>,
                                                    transition_table_type>
            event_transition_context;

        bool const actual =
            target.on_event(event_transition_context, event,
                            event_transition_context, event_transition_context);

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
        test_objects::test_event<2> event{true};
        test_objects::fake_event_transition_context<test_objects::test_event<2>,
                                                    transition_table_type>
            event_transition_context;

        REQUIRE_THROWS_AS(target.on_event(event_transition_context, event,
                                          event_transition_context,
                                          event_transition_context),
                          state_transition_ambiguity);

        THEN("state transitition ambiguity has been thrown") { REQUIRE(true); }
      }

      AND_WHEN(
          "an ambiguously triggering event (exit then reenter) is raised") {
        test_objects::test_event<3> event{true};
        test_objects::fake_event_transition_context<test_objects::test_event<3>,
                                                    transition_table_type>
            event_transition_context;

        REQUIRE_THROWS_AS(target.on_event(event_transition_context, event,
                                          event_transition_context,
                                          event_transition_context),
                          state_transition_ambiguity);

        THEN("state transitition ambiguity has been thrown") { REQUIRE(true); }
      }
    }
  }
}

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

SCENARIO("single state container querying",
         "[unit][state-container][single-state-container]") {
  using target_type = single_state<test_state_type>;

  GIVEN("an active single state container") {
    target_type target;
    test_objects::fake_entry_context<initial_entry_event_t,
                                     states_list<test_state_type>,
                                     test_objects::test_events_list<num_events>>
        initial_entry_context;
    execute_initial_entry(target, initial_entry_context, initial_entry_context);

    WHEN("queried that is done after visitation") {
      test_objects::test_query<1> query{0};
      bool const is_done = target.query(query);

      THEN("the result is done") { REQUIRE(is_done); }

      THEN("the state was queried") { REQUIRE(1 == query.states_queried[0]); }
    }

    WHEN("queried that is not done after visitation") {
      test_objects::test_query<1> query;
      bool const is_done = target.query(query);

      THEN("the result is not done") { REQUIRE_FALSE(is_done); }

      THEN("the state was queried") { REQUIRE(1 == query.states_queried[0]); }
    }
  }
}
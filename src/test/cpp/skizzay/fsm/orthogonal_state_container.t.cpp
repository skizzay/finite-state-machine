#include "skizzay/fsm/orthogonal_state_container.h"

#include "skizzay/fsm/single_state_container.h"
#include "skizzay/fsm/type_list.h"
#include "test_objects.h"

#include <catch2/catch.hpp>

using namespace skizzay::fsm;

namespace {

constexpr std::size_t num_events = 6;
template <std::size_t Id>
using target_test_state = test_objects::test_state<Id, num_events>;
using target_type = orthogonal_states<single_state<target_test_state<0>>,
                                      single_state<target_test_state<1>>>;

} // namespace

SCENARIO("orthogonal state container is a state container",
         "[unit][state-container][orthogonal-state-container]") {
  THEN("orthogonal state container is a state container") {
    REQUIRE(is_state_container<target_type>::value);
    REQUIRE(concepts::state_container<target_type>);
  }
}

SCENARIO("orthogonal state container state examination",
         "[unit][state-container][orthogonal-state-container]") {
  GIVEN("a type held by the container") {
    using type_held_by_container = front_t<states_list_t<target_type>>;

    WHEN("seeing if the container holds that type") {
      constexpr bool actual =
          contains_v<states_list_t<target_type>, type_held_by_container>;

      THEN("it's held") { REQUIRE(actual); }
    }
  }

  GIVEN("a type not held by the container") {
    using type_not_held_by_container = target_test_state<3>;

    WHEN("seeing if the container holds that type") {
      constexpr bool actual =
          contains_v<states_list_t<target_type>, type_not_held_by_container>;

      THEN("it's not held") { REQUIRE_FALSE(actual); }
    }
  }
}

SCENARIO("orthogonal state container handling events") {
  GIVEN("an orthogonal state container") {
    target_type target;

    THEN("it is not active") {
      REQUIRE_FALSE(target.is_active());
      REQUIRE(target.is_inactive());
    }

    THEN("both states return an empty current state") {
      REQUIRE_FALSE(target.current_state<target_test_state<0>>().has_value());
      REQUIRE_FALSE(target.is<target_test_state<0>>());
      REQUIRE_FALSE(target.current_state<target_test_state<1>>().has_value());
      REQUIRE_FALSE(target.is<target_test_state<1>>());
    }

    WHEN("initially entered") {
      test_objects::fake_entry_context<
          initial_entry_event_t, states_list_t<target_type>,
          test_objects::test_events_list<num_events>>
          entry_context;

      target.on_entry(entry_context);

      THEN("both states return a populated current state") {
        REQUIRE(target.current_state<target_test_state<0>>().has_value());
        REQUIRE(target.is<target_test_state<0>>());
        REQUIRE(target.current_state<target_test_state<1>>().has_value());
        REQUIRE(target.is<target_test_state<1>>());
      }

      THEN("it is active") {
        REQUIRE(target.is_active());
        REQUIRE_FALSE(target.is_inactive());

        AND_THEN("each substate was entered by initial entry") {
          REQUIRE(target.state<target_test_state<0>>().initial_entry_count ==
                  1);
          REQUIRE(target.state<target_test_state<1>>().initial_entry_count ==
                  1);
        }
      }

      AND_WHEN("finally exited") {
        using event_type = final_exit_event_t;
        test_objects::fake_event_transition_context<
            event_type, std::tuple<>,
            test_objects::test_states_list<num_events, 2>,
            test_objects::test_events_list<num_events>>
            event_transition_context;

        bool const actual = target.on_event(event_transition_context);
        THEN("the event was handled") {
          REQUIRE(actual);

          THEN("it is not active") {
            REQUIRE_FALSE(target.is_active());
            REQUIRE(target.is_inactive());

            AND_THEN("each substate was finally exited") {
              REQUIRE(target.state<target_test_state<0>>().final_exit_count ==
                      1);
              REQUIRE(target.state<target_test_state<1>>().final_exit_count ==
                      1);
            }
          }
        }
      }

      AND_WHEN("one state transitions to an external state") {
        using event_type = test_objects::test_event<0>;
        test_objects::fake_event_transition_context<
            event_type,
            std::tuple<simple_transition<target_test_state<0>,
                                         target_test_state<2>, event_type>>,
            test_objects::test_states_list<num_events, 3>,
            test_objects::test_events_list<num_events>>
            event_transition_context;

        bool const actual = target.on_event(event_transition_context);
        THEN("the event was handled") {
          REQUIRE(actual);
          AND_THEN("it is not active") {
            REQUIRE_FALSE(target.is_active());
            REQUIRE(target.is_inactive());

            AND_THEN("the triggering state was event exited") {
              REQUIRE(
                  target.state<target_test_state<0>>().event_exit_count[0] ==
                  1);

              AND_THEN("the non-triggering state was finally exited") {
                REQUIRE(target.state<target_test_state<1>>().final_exit_count ==
                        1);
              }
            }
          }
        }
      }

      AND_WHEN("both states transition to an external state") {
        using event_type = test_objects::test_event<0>;
        test_objects::fake_event_transition_context<
            event_type,
            std::tuple<simple_transition<target_test_state<0>,
                                         target_test_state<2>, event_type>,
                       simple_transition<target_test_state<1>,
                                         target_test_state<3>, event_type>>,
            test_objects::test_states_list<num_events, 3>,
            test_objects::test_events_list<num_events>>
            event_transition_context;

        bool const actual = target.on_event(event_transition_context);
        THEN("the event was handled") {
          REQUIRE(actual);
          AND_THEN("it is not active") {
            REQUIRE_FALSE(target.is_active());
            REQUIRE(target.is_inactive());

            AND_THEN("both states were event exited") {
              REQUIRE(
                  target.state<target_test_state<0>>().event_exit_count[0] ==
                  1);
              REQUIRE(
                  target.state<target_test_state<1>>().event_exit_count[0] ==
                  1);
            }
          }
        }
      }

      AND_WHEN("no states trigger a transition") {
        using event_type = test_objects::test_event<0>;
        test_objects::fake_event_transition_context<
            event_type,
            std::tuple<simple_transition<target_test_state<2>,
                                         target_test_state<2>, event_type>>,
            test_objects::test_states_list<num_events, 3>,
            test_objects::test_events_list<num_events>>
            event_transition_context;

        bool const actual = target.on_event(event_transition_context);
        THEN("the event was not handled") {
          REQUIRE_FALSE(actual);
          AND_THEN("it is active") {
            REQUIRE(target.is_active());
            REQUIRE_FALSE(target.is_inactive());
          }
        }
      }
    }
  }
}

SCENARIO("entry into orthogonal states",
         "[unit][state-container][orthogonal-state-container]") {
  GIVEN("an orthogonal state container") {
    target_type target;

    WHEN("entering the state container with no contained states scheduled") {
      test_objects::fake_entry_context<
          test_objects::test_event<1>,
          test_objects::test_states_list<num_events, 3>,
          test_objects::test_events_list<num_events>,
          states_list<target_test_state<2>>>
          entry_context;

      target.on_entry(entry_context);

      THEN("both states were initially entered") {
        REQUIRE(target.state<target_test_state<0>>().initial_entry_count == 1);
        REQUIRE(target.state<target_test_state<1>>().initial_entry_count == 1);
      }
    }

    WHEN("entering the state container with both contained states scheduled") {
      constexpr std::size_t event_id = 0;
      test_objects::fake_entry_context<
          test_objects::test_event<event_id>,
          test_objects::test_states_list<num_events, 3>,
          test_objects::test_events_list<num_events>>
          entry_context;

      target.on_entry(entry_context);

      THEN("both states were event entered") {
        REQUIRE(target.state<target_test_state<0>>().initial_entry_count == 0);
        REQUIRE(target.state<target_test_state<1>>().initial_entry_count == 0);
        REQUIRE(
            target.state<target_test_state<0>>().event_entry_count[event_id] ==
            1);
        REQUIRE(
            target.state<target_test_state<1>>().event_entry_count[event_id] ==
            1);
      }
    }

    WHEN("entering the state container with one of the contained states "
         "scheduled") {
      constexpr std::size_t event_id = 0;
      test_objects::fake_entry_context<
          test_objects::test_event<event_id>,
          test_objects::test_states_list<num_events, 3>,
          test_objects::test_events_list<num_events>,
          test_objects::test_states_list<num_events, 1>>
          entry_context;

      target.on_entry(entry_context);

      THEN("the scheduled state was event entered") {
        REQUIRE(target.state<target_test_state<0>>().initial_entry_count == 0);
        REQUIRE(
            target.state<target_test_state<0>>().event_entry_count[event_id] ==
            1);

        AND_THEN("the unscheduled state was initially entered") {
          REQUIRE(target.state<target_test_state<1>>().initial_entry_count ==
                  1);
          REQUIRE(target.state<target_test_state<1>>()
                      .event_entry_count[event_id] == 0);
        }
      }
    }
  }
}

SCENARIO("orthogonal state container querying",
         "[unit][state-container][orthogonal-state-container]") {
  GIVEN("an active orthogonal state container") {
    target_type target;
    test_objects::fake_entry_context<
        initial_entry_event_t, test_objects::test_states_list<num_events, 2>,
        test_objects::test_events_list<num_events>>
        initial_entry_context;
    target.on_entry(initial_entry_context);

    WHEN("queried that is done after visiting first container") {
      test_objects::test_query<length_v<states_list_t<target_type>>> query{0};
      bool const is_done = target.query(query);

      THEN("the result is done") { REQUIRE(is_done); }

      THEN("the first state was queried") {
        REQUIRE(1 == query.states_queried[0]);
      }

      THEN("the second state was not queried") {
        REQUIRE(0 == query.states_queried[1]);
      }
    }

    WHEN("queried that is done after visiting second container") {
      test_objects::test_query<length_v<states_list_t<target_type>>> query{1};
      bool const is_done = target.query(query);

      THEN("the result is done") { REQUIRE(is_done); }

      THEN("the first state was queried") {
        REQUIRE(1 == query.states_queried[0]);
      }

      THEN("the second state was queried") {
        REQUIRE(1 == query.states_queried[1]);
      }
    }

    WHEN("queried that is not done after visitation") {
      test_objects::test_query<length_v<states_list_t<target_type>>> query;
      bool const is_done = target.query(query);

      THEN("the result is not done") { REQUIRE_FALSE(is_done); }

      THEN("the first state was queried") {
        REQUIRE(1 == query.states_queried[0]);
      }

      THEN("the second state was queried") {
        REQUIRE(1 == query.states_queried[1]);
      }
    }
  }
}
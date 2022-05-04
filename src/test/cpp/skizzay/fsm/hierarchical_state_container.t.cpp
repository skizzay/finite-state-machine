#include "skizzay/fsm/hierarchical_state_container.h"

#include <skizzay/fsm/single_state_container.h>
#include <skizzay/fsm/states_list.h>
#include <skizzay/fsm/type_list.h>

#include "test_objects.h"
#include <catch2/catch.hpp>

#include <cstddef>

using namespace skizzay::fsm;

constexpr std::size_t num_events = 4;
constexpr std::size_t parent_id = 0;
constexpr std::size_t child_id = 1;
constexpr std::size_t external_id = 2;

using parent_state_type = test_objects::test_state<parent_id, num_events>;
using child_state_type = test_objects::test_state<child_id, num_events>;
using external_state_type = test_objects::test_state<external_id, num_events>;
using target_type =
    hierarchical_states<parent_state_type, single_state<child_state_type>>;

SCENARIO("hierarchical state container is state container",
         "[unit][state-container][hierarchical-state-container]") {
  THEN("simple state container is a state container") {
    REQUIRE(concepts::state_container<target_type>);
  }
}

SCENARIO("hierarchical state container state examination",
         "[unit][state-container][hierarchical-state-container]") {
  GIVEN("a type held by the container") {
    using type_held_by_container = front_t<states_list_t<target_type>>;

    WHEN("seeing if the container holds that type") {
      constexpr bool actual =
          contains_v<states_list_t<target_type>, type_held_by_container>;

      THEN("it's held") { REQUIRE(actual); }
    }
  }

  GIVEN("a type not held by the container") {
    using type_not_held_by_container = external_state_type;

    WHEN("seeing if the container holds that type") {
      constexpr bool actual =
          contains_v<states_list_t<target_type>, type_not_held_by_container>;

      THEN("it's not held") { REQUIRE_FALSE(actual); }
    }
  }
}

SCENARIO("hierarchical state container entry",
         "[unit][state-container][hierarchical-state-container]") {
  GIVEN("a hierarchical state relationship") {
    target_type target;

    THEN("it is inactive") {
      REQUIRE_FALSE(target.is_active());
      REQUIRE(target.is_inactive());
    }

    THEN("all states return an empty current state") {
      REQUIRE_FALSE(target.current_state<parent_state_type>().has_value());
      REQUIRE_FALSE(target.is<parent_state_type>());
      REQUIRE_FALSE(target.current_state<child_state_type>().has_value());
      REQUIRE_FALSE(target.is<child_state_type>());
    }

    WHEN("initially entered") {
      test_objects::fake_entry_context<
          initial_entry_event_t, states_list_t<target_type>,
          test_objects::test_events_list<num_events>>
          entry_context;

      target.on_entry(entry_context);

      THEN("it is active") {
        REQUIRE(target.is_active());
        REQUIRE_FALSE(target.is_inactive());
      }

      THEN("all states return a valid current state") {
        REQUIRE(target.current_state<parent_state_type>().has_value());
        REQUIRE(target.is<parent_state_type>());
        REQUIRE(target.current_state<child_state_type>().has_value());
        REQUIRE(target.is<child_state_type>());
      }

      THEN("the parent state initially entered") {
        REQUIRE(1 == target.current_state<parent_state_type>()
                         .value()
                         .initial_entry_count);
      }

      THEN("the child state initially entered") {
        REQUIRE(1 == target.current_state<child_state_type>()
                         .value()
                         .initial_entry_count);
      }
    }

    AND_GIVEN("list of next states is the parent only") {
      using next_states_list_type = states_list<parent_state_type>;

      WHEN("entered by an event transitioning to the parent only") {
        constexpr std::size_t const event_id = 0;
        using event_type = test_objects::test_event<event_id>;
        test_objects::fake_entry_context<
            event_type,
            states_list<parent_state_type, child_state_type,
                        external_state_type>,
            test_objects::test_events_list<num_events>, next_states_list_type>
            entry_context;

        target.on_entry(entry_context);

        THEN("it is active") {
          REQUIRE(target.is_active());
          REQUIRE_FALSE(target.is_inactive());
        }

        THEN("all states return a valid current state") {
          REQUIRE(target.current_state<parent_state_type>().has_value());
          REQUIRE(target.is<parent_state_type>());
          REQUIRE(target.current_state<child_state_type>().has_value());
          REQUIRE(target.is<child_state_type>());
        }

        THEN("the parent state event entered") {
          REQUIRE(1 == target.current_state<parent_state_type>()
                           .value()
                           .event_entry_count[event_id]);
          AND_THEN("the parent state did not initially enter") {
            REQUIRE(0 == target.current_state<parent_state_type>()
                             .value()
                             .initial_entry_count);
          }
        }

        THEN("the child state initially entered") {
          REQUIRE(1 == target.current_state<child_state_type>()
                           .value()
                           .initial_entry_count);
          AND_THEN("the child state did not event enter") {
            REQUIRE(0 == target.current_state<child_state_type>()
                             .value()
                             .event_entry_count[event_id]);
          }
        }
      }
    }

    AND_GIVEN("list of next states is the child only") {
      using next_states_list_type = states_list<child_state_type>;

      WHEN("entered by an event transitioning to the child only") {
        constexpr std::size_t const event_id = 0;
        using event_type = test_objects::test_event<event_id>;
        test_objects::fake_entry_context<
            event_type,
            states_list<parent_state_type, child_state_type,
                        external_state_type>,
            test_objects::test_events_list<num_events>, next_states_list_type>
            entry_context;

        target.on_entry(entry_context);

        THEN("it is active") {
          REQUIRE(target.is_active());
          REQUIRE_FALSE(target.is_inactive());
        }

        THEN("all states return a valid current state") {
          REQUIRE(target.current_state<parent_state_type>().has_value());
          REQUIRE(target.is<parent_state_type>());
          REQUIRE(target.current_state<child_state_type>().has_value());
          REQUIRE(target.is<child_state_type>());
        }

        THEN("the parent state initially entered") {
          REQUIRE(1 == target.current_state<parent_state_type>()
                           .value()
                           .initial_entry_count);
          AND_THEN("the parent state did not event enter") {
            REQUIRE(0 == target.current_state<parent_state_type>()
                             .value()
                             .event_entry_count[event_id]);
          }
        }

        THEN("the child state event entered") {
          REQUIRE(1 == target.current_state<child_state_type>()
                           .value()
                           .event_entry_count[event_id]);
          AND_THEN("the child state did not initially enter") {
            REQUIRE(0 == target.current_state<child_state_type>()
                             .value()
                             .initial_entry_count);
          }
        }
      }
    }

    AND_GIVEN("list of next states is the parent and the child") {
      using next_states_list_type =
          states_list<parent_state_type, child_state_type>;

      WHEN("entered by an event transitioning to the parent and the child") {
        constexpr std::size_t const event_id = 0;
        using event_type = test_objects::test_event<event_id>;
        test_objects::fake_entry_context<
            event_type,
            states_list<parent_state_type, child_state_type,
                        external_state_type>,
            test_objects::test_events_list<num_events>, next_states_list_type>
            entry_context;

        target.on_entry(entry_context);

        THEN("it is active") {
          REQUIRE(target.is_active());
          REQUIRE_FALSE(target.is_inactive());
        }

        THEN("all states return a valid current state") {
          REQUIRE(target.current_state<parent_state_type>().has_value());
          REQUIRE(target.is<parent_state_type>());
          REQUIRE(target.current_state<child_state_type>().has_value());
          REQUIRE(target.is<child_state_type>());
        }

        THEN("the parent state event entered") {
          REQUIRE(1 == target.current_state<parent_state_type>()
                           .value()
                           .event_entry_count[event_id]);
          AND_THEN("the parent state did not initially enter") {
            REQUIRE(0 == target.current_state<parent_state_type>()
                             .value()
                             .initial_entry_count);
          }
        }

        THEN("the child state event entered") {
          REQUIRE(1 == target.current_state<child_state_type>()
                           .value()
                           .event_entry_count[event_id]);
          AND_THEN("the child state did not initially enter") {
            REQUIRE(0 == target.current_state<child_state_type>()
                             .value()
                             .initial_entry_count);
          }
        }
      }

      WHEN("entered by an event transitioning to the parent only") {
        constexpr std::size_t const event_id = 0;
        using event_type = test_objects::test_event<event_id>;
        test_objects::fake_entry_context<
            event_type,
            states_list<parent_state_type, child_state_type,
                        external_state_type>,
            test_objects::test_events_list<num_events>, next_states_list_type,
            states_list<parent_state_type>>
            entry_context;

        target.on_entry(entry_context);

        THEN("it is active") {
          REQUIRE(target.is_active());
          REQUIRE_FALSE(target.is_inactive());
        }

        THEN("all states return a valid current state") {
          REQUIRE(target.current_state<parent_state_type>().has_value());
          REQUIRE(target.is<parent_state_type>());
          REQUIRE(target.current_state<child_state_type>().has_value());
          REQUIRE(target.is<child_state_type>());
        }

        THEN("the parent state event entered") {
          REQUIRE(1 == target.current_state<parent_state_type>()
                           .value()
                           .event_entry_count[event_id]);
          AND_THEN("the parent state did not initially enter") {
            REQUIRE(0 == target.current_state<parent_state_type>()
                             .value()
                             .initial_entry_count);
          }
        }

        THEN("the child state initially entered") {
          REQUIRE(1 == target.current_state<child_state_type>()
                           .value()
                           .initial_entry_count);
          AND_THEN("the child state did not event enter") {
            REQUIRE(0 == target.current_state<child_state_type>()
                             .value()
                             .event_entry_count[event_id]);
          }
        }
      }

      WHEN("entered by an event transitioning to the child only") {
        constexpr std::size_t const event_id = 0;
        using event_type = test_objects::test_event<event_id>;
        test_objects::fake_entry_context<
            event_type,
            states_list<parent_state_type, child_state_type,
                        external_state_type>,
            test_objects::test_events_list<num_events>, next_states_list_type,
            states_list<child_state_type>>
            entry_context;

        target.on_entry(entry_context);

        THEN("it is active") {
          REQUIRE(target.is_active());
          REQUIRE_FALSE(target.is_inactive());
        }

        THEN("all states return a valid current state") {
          REQUIRE(target.current_state<parent_state_type>().has_value());
          REQUIRE(target.is<parent_state_type>());
          REQUIRE(target.current_state<child_state_type>().has_value());
          REQUIRE(target.is<child_state_type>());
        }

        THEN("the parent state initially entered") {
          REQUIRE(1 == target.current_state<parent_state_type>()
                           .value()
                           .initial_entry_count);
          AND_THEN("the parent state did not event enter") {
            REQUIRE(0 == target.current_state<parent_state_type>()
                             .value()
                             .event_entry_count[event_id]);
          }
        }

        THEN("the child state event entered") {
          REQUIRE(1 == target.current_state<child_state_type>()
                           .value()
                           .event_entry_count[event_id]);
          AND_THEN("the child state did not initially enter") {
            REQUIRE(0 == target.current_state<child_state_type>()
                             .value()
                             .initial_entry_count);
          }
        }
      }
    }
  }
}

SCENARIO("hierarchical state container event handling and subsequent entry",
         "[unit][state-container][hierarchical-state-container]") {
  GIVEN("a hierarchical state relationship") {
    target_type target;

    THEN("it is inactive") {
      REQUIRE_FALSE(target.is_active());
      REQUIRE(target.is_inactive());
    }

    THEN("all states return an empty current state") {
      REQUIRE_FALSE(target.current_state<parent_state_type>().has_value());
      REQUIRE_FALSE(target.is<parent_state_type>());
      REQUIRE_FALSE(target.current_state<child_state_type>().has_value());
      REQUIRE_FALSE(target.is<child_state_type>());
    }

    WHEN("initially entered") {
      test_objects::fake_entry_context<
          initial_entry_event_t, states_list_t<target_type>,
          test_objects::test_events_list<num_events>>
          entry_context;

      target.on_entry(entry_context);

      THEN("it is active") {
        REQUIRE(target.is_active());
        REQUIRE_FALSE(target.is_inactive());
      }

      THEN("all states return a valid current state") {
        REQUIRE(target.current_state<parent_state_type>().has_value());
        REQUIRE(target.is<parent_state_type>());
        REQUIRE(target.current_state<child_state_type>().has_value());
        REQUIRE(target.is<child_state_type>());
      }

      AND_WHEN("an event is raised but not handled by the container") {
        constexpr std::size_t const event_id = 0;
        using event_type = test_objects::test_event<event_id>;
        test_objects::fake_event_transition_context<
            event_type,
            std::tuple<simple_transition<external_state_type, child_state_type,
                                         event_type>>,
            test_objects::test_states_list<num_events, 2>,
            test_objects::test_events_list<num_events>>
            event_transition_context;

        bool const handled = target.on_event(event_transition_context);
        THEN("the event was not handled") {
          REQUIRE_FALSE(handled);

          AND_THEN("it is active") {
            REQUIRE(target.is_active());
            REQUIRE_FALSE(target.is_inactive());
          }

          AND_THEN("all states return a valid current state") {
            REQUIRE(target.current_state<parent_state_type>().has_value());
            REQUIRE(target.is<parent_state_type>());
            REQUIRE(target.current_state<child_state_type>().has_value());
            REQUIRE(target.is<child_state_type>());
          }
        }
      }

      AND_WHEN("an event triggered by the child exits the container") {
        constexpr std::size_t const event_id = 0;
        using event_type = test_objects::test_event<event_id>;
        test_objects::fake_event_transition_context<
            event_type,
            std::tuple<simple_transition<child_state_type, external_state_type,
                                         event_type>>,
            test_objects::test_states_list<num_events, 2>,
            test_objects::test_events_list<num_events>>
            event_transition_context;

        bool const actual = target.on_event(event_transition_context);
        THEN("the event was handled") {
          REQUIRE(actual);
          AND_THEN("it is inactive") {
            REQUIRE_FALSE(target.is_active());
            REQUIRE(target.is_inactive());
          }

          AND_THEN("the child event exited") {
            REQUIRE(
                1 ==
                target.state<child_state_type>().event_exit_count[event_id]);
          }

          AND_THEN("the parent finally exited") {
            REQUIRE(1 == target.state<parent_state_type>().final_exit_count);
          }
        }
      }

      AND_WHEN("an event triggered by the parent exits the container") {
        constexpr std::size_t const event_id = 0;
        using event_type = test_objects::test_event<event_id>;
        test_objects::fake_event_transition_context<
            event_type,
            std::tuple<simple_transition<parent_state_type, external_state_type,
                                         event_type>>,
            test_objects::test_states_list<num_events, 2>,
            test_objects::test_events_list<num_events>>
            event_transition_context;

        bool const actual = target.on_event(event_transition_context);
        THEN("the event was handled") {
          REQUIRE(actual);
          AND_THEN("it is inactive") {
            REQUIRE_FALSE(target.is_active());
            REQUIRE(target.is_inactive());
          }

          AND_THEN("the child finally exited") {
            REQUIRE(1 == target.state<child_state_type>().final_exit_count);
          }

          AND_THEN("the parent event exited") {
            REQUIRE(
                1 ==
                target.state<parent_state_type>().event_exit_count[event_id]);
          }
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

          AND_THEN("it is not active") {
            REQUIRE_FALSE(target.is_active());
            REQUIRE(target.is_inactive());

            AND_THEN("the child state was finally exited") {
              REQUIRE(target.state<child_state_type>().final_exit_count == 1);

              AND_THEN("the child state is inactive") {
                REQUIRE_FALSE(
                    target.current_state<child_state_type>().has_value());
                REQUIRE_FALSE(target.is<child_state_type>());
              }
            }

            AND_THEN("the parent state was finally exited") {
              REQUIRE(target.state<parent_state_type>().final_exit_count == 1);

              AND_THEN("the parent state is inactive") {
                REQUIRE_FALSE(
                    target.current_state<parent_state_type>().has_value());
                REQUIRE_FALSE(target.is<parent_state_type>());
              }
            }
          }
        }
      }

      AND_WHEN("an event reeentering the parent was triggered") {
        constexpr std::size_t const event_id = 0;
        using event_type = test_objects::test_event<event_id>;
        test_objects::fake_event_transition_context<
            event_type,
            std::tuple<simple_transition<parent_state_type, parent_state_type,
                                         event_type>>,
            test_objects::test_states_list<num_events, 2>,
            test_objects::test_events_list<num_events>>
            event_transition_context;

        bool const actual = target.on_event(event_transition_context);
        THEN("the event was handled") {
          REQUIRE(actual);
          AND_THEN("the child state has finally exited") {
            REQUIRE(target.state<child_state_type>().final_exit_count == 1);

            AND_THEN("the child state is inactive") {
              REQUIRE_FALSE(
                  target.current_state<child_state_type>().has_value());
              REQUIRE_FALSE(target.is<child_state_type>());
            }
          }
        }

        AND_WHEN("entered") {
          test_objects::fake_entry_context<
              event_type,
              states_list<parent_state_type, child_state_type,
                          external_state_type>,
              test_objects::test_events_list<num_events>,
              states_list<parent_state_type>>
              entry_context;

          target.on_entry(entry_context);
          THEN("the parent was event reentered") {
            REQUIRE(1 == target.current_state<parent_state_type>()
                             .value()
                             .event_reentry_count[event_id]);
          }

          THEN("the child was initially entered") {
            REQUIRE(2 == target.current_state<child_state_type>()
                             .value()
                             .initial_entry_count);
          }
        }
      }

      AND_WHEN("an event reeentering the child was triggered") {
        constexpr std::size_t const event_id = 0;
        using event_type = test_objects::test_event<event_id>;
        test_objects::fake_event_transition_context<
            event_type,
            std::tuple<simple_transition<child_state_type, child_state_type,
                                         event_type>>,
            test_objects::test_states_list<num_events, 2>,
            test_objects::test_events_list<num_events>>
            event_transition_context;

        bool const actual = target.on_event(event_transition_context);
        THEN("the event was handled") {
          REQUIRE(actual);
          AND_THEN("the child state has not exited") {
            REQUIRE(target.state<child_state_type>().event_exit_count[0] == 0);

            AND_THEN("the child state is active") {
              REQUIRE(target.current_state<child_state_type>().has_value());
              REQUIRE(target.is<child_state_type>());
            }
          }
        }

        AND_WHEN("entered") {
          test_objects::fake_entry_context<
              event_type,
              states_list<parent_state_type, child_state_type,
                          external_state_type>,
              test_objects::test_events_list<num_events>,
              states_list<child_state_type>>
              entry_context;

          target.on_entry(entry_context);
          THEN("the parent was not reentered") {
            REQUIRE(0 == target.current_state<parent_state_type>()
                             .value()
                             .event_reentry_count[event_id]);
          }

          THEN("the child was reentered") {
            REQUIRE(1 == target.current_state<child_state_type>()
                             .value()
                             .event_reentry_count[event_id]);
          }
        }
      }

      AND_WHEN("an event exiting the child and entering the parent was "
               "triggered") {
        constexpr std::size_t const event_id = 0;
        using event_type = test_objects::test_event<event_id>;
        test_objects::fake_event_transition_context<
            event_type,
            std::tuple<simple_transition<child_state_type, parent_state_type,
                                         event_type>>,
            test_objects::test_states_list<num_events, 2>,
            test_objects::test_events_list<num_events>>
            event_transition_context;

        bool const actual = target.on_event(event_transition_context);
        THEN("the event was handled") {
          REQUIRE(actual);
          AND_THEN("the child state was event exited") {
            REQUIRE(target.state<child_state_type>().event_exit_count[0] == 1);

            AND_THEN("the child state is inactive") {
              REQUIRE_FALSE(
                  target.current_state<child_state_type>().has_value());
              REQUIRE_FALSE(target.is<child_state_type>());
            }
          }
        }

        AND_WHEN("entered") {
          test_objects::fake_entry_context<
              event_type,
              states_list<parent_state_type, child_state_type,
                          external_state_type>,
              test_objects::test_events_list<num_events>,
              states_list<parent_state_type>>
              entry_context;

          target.on_entry(entry_context);
          THEN("the parent was reentered") {
            REQUIRE(1 == target.current_state<parent_state_type>()
                             .value()
                             .event_reentry_count[event_id]);
          }

          THEN("the child was initially entered") {
            REQUIRE(2 == target.current_state<child_state_type>()
                             .value()
                             .initial_entry_count);
          }
        }
      }
    }
  }
}

SCENARIO("hierarchical state container querying",
         "[unit][state-container][single-state-container]") {
  GIVEN("a hierarchical single state container") {
    target_type target;
    test_objects::fake_entry_context<
        initial_entry_event_t, test_objects::test_states_list<num_events, 2>,
        test_objects::test_events_list<num_events>>
        initial_entry_context;
    target.on_entry(initial_entry_context);

    WHEN("queried that is done after visiting parent") {
      test_objects::test_query<length_v<states_list_t<target_type>>> query{
          parent_id};
      bool const is_done = target.query(query);

      THEN("the result is done") { REQUIRE(is_done); }

      THEN("the parent state was queried") {
        REQUIRE(1 == query.states_queried[parent_id]);
      }

      THEN("the child state was not queried") {
        REQUIRE(0 == query.states_queried[child_id]);
      }
    }

    WHEN("queried that is done after visiting child") {
      test_objects::test_query<length_v<states_list_t<target_type>>> query{
          child_id};
      bool const is_done = target.query(query);

      THEN("the result is done") { REQUIRE(is_done); }

      THEN("the parent state was queried") {
        REQUIRE(1 == query.states_queried[parent_id]);
      }

      THEN("the child state was queried") {
        REQUIRE(1 == query.states_queried[child_id]);
      }
    }

    WHEN("queried that is not done after visitation") {
      test_objects::test_query<length_v<states_list_t<target_type>>> query;
      bool const is_done = target.query(query);

      THEN("the result is not done") { REQUIRE_FALSE(is_done); }

      THEN("the parent state was queried") {
        REQUIRE(1 == query.states_queried[parent_id]);
      }

      THEN("the child state was queried") {
        REQUIRE(1 == query.states_queried[child_id]);
      }
    }
  }
}
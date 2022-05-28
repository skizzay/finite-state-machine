#include <skizzay/fsm/state_chart.h>

#include "skizzay/fsm/event_handler.h"
#include "skizzay/fsm/machine.h"
#include "skizzay/fsm/simple_transition.h"
#include "skizzay/fsm/single_state_container.h"
#include "skizzay/fsm/state_queryable.h"
#include "test_objects.h"

#include <catch2/catch.hpp>
#include <tuple>

using namespace skizzay::fsm;

constexpr std::size_t const num_events = 1;

using state_type = test_objects::test_state<0, num_events>;
using state_container_type = single_state<state_type>;

SCENARIO("state chart concepts", "[unit][state-chart]") {
  constexpr std::size_t const event_id = 0;
  using event_type = test_objects::test_event<event_id>;
  using transition_table_type =
      std::tuple<simple_transition<state_type, state_type, event_type>>;
  using target_type = state_chart<state_container_type, transition_table_type>;

  REQUIRE(std::destructible<target_type>);
  REQUIRE(std::copy_constructible<target_type>);
  REQUIRE(concepts::state_queryable<target_type>);
  REQUIRE(concepts::event_handler<target_type>);
  REQUIRE(concepts::machine<target_type>);
}

SCENARIO("state chart starting/stopping", "[unit][state-chart]") {
  constexpr std::size_t const event_id = 0;
  using event_type = test_objects::test_event<event_id>;
  using transition_table_type =
      std::tuple<simple_transition<state_type, state_type, event_type>>;

  GIVEN("a state chart") {
    state_chart target{state_container_type{}, transition_table_type{}};

    THEN("it is not running") {
      REQUIRE(target.is_stopped());
      REQUIRE_FALSE(target.is_running());
      REQUIRE_FALSE(target.is<state_type>());
    }

    WHEN("it is started") {
      target.start();
      // Capturing the state here for examination during requirement checks
      state_type const &state = target.current_state<state_type>().value();

      THEN("it is running") {
        REQUIRE_FALSE(target.is_stopped());
        REQUIRE(target.is_running());
        REQUIRE(target.is<state_type>());
      }

      THEN("the state has been initialized") {
        REQUIRE(1 == state.initial_entry_count);
      }

      AND_WHEN("it is stopped") {
        target.stop();

        THEN("it is not running") {
          REQUIRE(target.is_stopped());
          REQUIRE_FALSE(target.is_running());
          REQUIRE_FALSE(target.is<state_type>());
        }

        THEN("the state has been finalized") {
          REQUIRE(1 == state.final_exit_count);
        }
      }

      AND_WHEN("it is started") {
        REQUIRE_NOTHROW(target.start());

        THEN("it has not thrown") { SUCCEED(); }

        THEN("it is running") {
          REQUIRE_FALSE(target.is_stopped());
          REQUIRE(target.is_running());
          REQUIRE(target.is<state_type>());
        }

        THEN("the state has not been reinitialized") {
          // the count reflects the initialization when the state chart was
          // first started
          REQUIRE(1 == state.initial_entry_count);
        }
      }
    }

    WHEN("it is stopped") {
      REQUIRE_NOTHROW(target.stop());

      THEN("it has not thrown") { SUCCEED(); }

      THEN("it is not running") {
        REQUIRE(target.is_stopped());
        REQUIRE_FALSE(target.is_running());
        REQUIRE_FALSE(target.is<state_type>());
      }
    }
  }
}

SCENARIO("state chart querying", "[unit][state-chart]") {
  constexpr std::size_t const event_id = 0;
  using event_type = test_objects::test_event<event_id>;
  using transition_table_type =
      std::tuple<simple_transition<state_type, state_type, event_type>>;

  GIVEN("a state chart") {
    state_chart target{state_container_type{}, transition_table_type{}};

    WHEN("it is queried") {
      test_objects::test_query<1> query;

      target.query(query);

      THEN("the query callback was not invoked") {
        REQUIRE(0 == query.states_queried[0]);
      }
    }

    WHEN("it is started") {
      target.start();

      AND_WHEN("it is queried") {
        test_objects::test_query<1> query;

        target.query(query);

        THEN("the query callback was invoked") {
          REQUIRE(1 == query.states_queried[0]);
        }
      }
    }
  }
}

SCENARIO("state chart event handling", "[unit][state-chart]") {
  constexpr std::size_t const event_id = 0;
  using event_type = test_objects::test_event<event_id>;
  using transition_table_type =
      std::tuple<simple_transition<state_type, state_type, event_type>>;

  GIVEN("a state chart") {
    state_chart target{state_container_type{}, transition_table_type{}};

    WHEN("an event is posted") {
      bool const handled = target.on(event_type{});

      THEN("it was not handled") {
        REQUIRE_FALSE(handled);
      }
    }

    WHEN("it is started") {
      target.start();

      AND_WHEN("an event is posted") {
        bool const handled = target.on(event_type{});

        THEN("it was handled") { REQUIRE(handled); }

        THEN("the state's event callback was triggered") {
          REQUIRE(1 == target.current_state<state_type>().value().event_reentry_count[event_id]);
        }
      }
    }
  }
}
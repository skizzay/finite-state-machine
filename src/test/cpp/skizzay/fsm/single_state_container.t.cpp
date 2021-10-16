#include "test_objects.h"

#include <catch.hpp>
#include <memory>
#include <skizzay/fsm/ancestors.h>
#include <skizzay/fsm/event.h>
#include <skizzay/fsm/guarded_transition.h>
#include <skizzay/fsm/simple_transition.h>
#include <skizzay/fsm/single_state_container.h>
#include <skizzay/fsm/traits.h>
#include <skizzay/fsm/transition_coordinator.h>
#include <tuple>
#include <vector>

using namespace skizzay::fsm;

namespace {

template <std::size_t Id> using target_test_state = test_state<Id, 3>;

}

SCENARIO("single state container is a state container",
         "[unit][state-container][single-state-container]") {
  using target_type = single_state<target_test_state<0>>;
  THEN("single state container is a state container") {
    REQUIRE(has_states_list_type<target_type>::value);
    REQUIRE(is_state_container<target_type>::value);
    REQUIRE(concepts::state_container<target_type>);
  }
}

SCENARIO("single state container event handling",
         "[unit][state-container][single-state-container]") {
  using type = target_test_state<0>;
  using target_type = single_state<type>;
  fake_machine machine{std::tuple{
      simple_transition<target_test_state<0>, target_test_state<0>,
                        test_event<0>>{},
      simple_transition<target_test_state<1>, target_test_state<0>,
                        test_event<1>>{},
      guarded_transition<
          target_test_state<0>, target_test_state<1>, test_event<2>,
          bool (target_test_state<0>::*)(test_event<2> const &) const noexcept>{
          &target_test_state<0>::accepts}}};

  GIVEN("a single state container") {
    target_type target;
    WHEN("initially entered") {
      target.on_initial_entry(machine);

      THEN("the entry callback was triggered") {
        type const &current_state = target.current_state<type>();
        REQUIRE(1 == current_state.initial_entry_count);
      }

      AND_WHEN("seeing if the container is that type") {
        bool const actual = target.is<type>();

        THEN("it's held") { REQUIRE(actual); }
      }

      AND_WHEN("handling an external event") {
        constexpr std::size_t event_id = 2;
        using event_type = test_event<event_id>;
        event_type event;
        fake_transition_coordinator coordinator{machine, event};

        bool const had_transition =
            target.on_event(coordinator, machine, event);

        THEN("the event caused a transition") { REQUIRE(had_transition); }
      }

      AND_WHEN("finally exited") {
        target.on_final_exit(machine);

        THEN("the exit callback was triggered") {
          type const &current_state = target.current_state<type>();
          REQUIRE(1 == current_state.final_exit_count);
        }

        AND_WHEN("seeing if the container is that type") {
          bool const actual = target.is<type>();

          THEN("it's not held") { REQUIRE_FALSE(actual); }
        }
      }
    }

    AND_GIVEN("a triggering event") {
      constexpr std::size_t event_id = 0;
      using event_type = test_event<event_id>;
      event_type event;
      fake_transition_coordinator coordinator{machine, event};

      WHEN("initially entered") {
        target.on_initial_entry(machine);

        AND_WHEN("the event is handled") {
          bool const on_event_result =
              target.on_event(coordinator, machine, event);

          THEN("a transition was triggered") { REQUIRE(on_event_result); }
          THEN("the state is active") {
            REQUIRE(target.is_active());
            REQUIRE_FALSE(target.is_inactive());
          }
        }
      }

      WHEN("entered") {
        target.on_entry<type>(machine, event);
        THEN("the state is active") {
          REQUIRE(target.is_active());
          REQUIRE_FALSE(target.is_inactive());
        }
      }
    }

    AND_GIVEN("a non-triggering event") {
      constexpr std::size_t event_id = 2;
      using event_type = test_event<event_id>;
      event_type event{false};
      fake_transition_coordinator coordinator{machine, event};

      WHEN("initially entered") {
        target.on_initial_entry(machine);

        WHEN("the event is handled") {
          bool const on_event_result =
              target.on_event(coordinator, machine, event);

          THEN("a transition was not triggered") {
            REQUIRE_FALSE(on_event_result);
          }
        }
      }
    }

    AND_GIVEN("an unhandled event") {
      constexpr std::size_t event_id = 1;
      using event_type = test_event<event_id>;
      event_type event;
      fake_transition_coordinator coordinator{machine, event};

      WHEN("initially entered") {
        target.on_initial_entry(machine);

        WHEN("the event is handled") {
          bool const on_event_result =
              target.on_event(coordinator, machine, event);

          THEN("a transition was not triggered") {
            REQUIRE_FALSE(on_event_result);
          }
        }
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

    AND_GIVEN("an instance of the container") {
      target_type target;

      WHEN("seeing if the container is that type") {
        bool const actual = target.is<type>();

        THEN("it's not held") { REQUIRE_FALSE(actual); }
      }
    }
  }
}
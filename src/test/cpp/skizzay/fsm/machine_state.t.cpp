#include <skizzay/fsm/machine_state.h>

#include "skizzay/fsm/transition.h"
#include "skizzay/fsm/single_state_container.h"
#include "test_objects.h"
#include <catch2/catch.hpp>

using namespace skizzay::fsm;

SCENARIO("machine state is a state container and a state",
         "[unit][machine-state]") {
  using state_type = test_objects::test_state<0, 1>;
  using event_type = test_objects::test_event<0>;
  using transition_table_type =
      std::tuple<simple_transition<state_type, state_type, event_type>>;
  using target_type =
      machine_state<transition_table_type, single_state<state_type>>;

  REQUIRE(concepts::state<target_type>);
  REQUIRE(concepts::state_accessible<target_type>);
  REQUIRE(concepts::snapshottable<target_type>);
  REQUIRE(concepts::state_container<target_type>);
  REQUIRE(concepts::root_state_container<target_type, transition_table_type>);
}
#include <skizzay/fsm/event_transition_context_root.h>

#include "skizzay/fsm/event_transition_context.h"
#include "skizzay/fsm/transition.h"
#include "skizzay/fsm/transition_table.h"
#include "test_objects.h"

#include <catch2/catch.hpp>

using namespace skizzay::fsm;

SCENARIO("Event context root concepts", "[unit][event-context-root]") {
  constexpr std::size_t const num_events = 1;
  using event_type = test_objects::test_event<num_events - 1>;
  using state_type = test_objects::test_state<0, num_events>;
  using transition_table_type =
      std::tuple<simple_transition<state_type, state_type, event_type> &>;
  using target_type = event_transition_context_root<transition_table_type>;
  REQUIRE(concepts::event_transition_context<target_type>);
}
#include <catch.hpp>
#include <skizzay/fsm/simple_transition.h>

using namespace skizzay::fsm;

namespace {

struct timer_expired {};

struct green {};
struct red {};

using target_type = simple_transition<red, green, timer_expired>;

using fake_machine =
    details_::dummy_machine<states_list<red, green>, events_list<timer_expired>,
                            std::tuple<target_type>>;

} // namespace

SCENARIO("simple transition", "[unit][transition]") {
  fake_machine machine;

  GIVEN("a simple transition from red to green") {
    target_type target = {};

    THEN("it models a transition") {
      REQUIRE(
          is_transition_details_::has_current_state_type<target_type>::value);
      REQUIRE(is_transition_details_::has_next_state_type<target_type>::value);
      REQUIRE(is_transition_details_::has_event_type<target_type>::value);
      REQUIRE(is_transition_details_::has_accepts<target_type>::value);
      REQUIRE(is_transition<target_type>::value);
      REQUIRE(concepts::transition<target_type>);

      AND_THEN("it does not model an actionable transtion") {
        REQUIRE_FALSE(is_actionable_transition<target_type>::value);
        REQUIRE_FALSE(concepts::actionable_transition<target_type>);
      }
    }
    WHEN("a timer expired event is queried for acceptance") {
      bool const actual =
          target.accepts(red{}, std::as_const(machine), timer_expired{});

      THEN("the transition accepts the event") { REQUIRE(actual); }
    }
  }
}
#include <skizzay/fsm/transition.h>

#include "test_objects.h"
#include <catch.hpp>

using namespace skizzay::fsm;

namespace {

struct timer_expired {};

struct green {};
struct red {
  bool should_transition_result = true;
  bool should_transition(timer_expired const &) const noexcept {
    return should_transition_result;
  }
};

} // namespace

SCENARIO("actionable and guarded transition", "[unit][transition]") {
  GIVEN("an actionable, guarded transition type from red to green") {
    bool triggered = false;
    auto target = transition_guarded_by_with_action<red, green, timer_expired>(
        &red::should_transition,
        [&triggered](timer_expired const &) noexcept { triggered = true; });

    THEN("it models a transition") {
      REQUIRE(is_transition<decltype(target)>::value);
      REQUIRE(concepts::transition<decltype(target)>);
    }

    WHEN("a timer expired event is queried for acceptance which is set to "
         "pass") {
      timer_expired const event;
      bool const actual = target.is_accepted(red{true}, event);

      THEN("the transition accepts the event") { REQUIRE(actual); }

      AND_WHEN("triggered") {
        target.on_triggered(timer_expired{});
        THEN("the callback was fired") { REQUIRE(triggered); }
      }
    }
    WHEN("a timer expired event is queried for acceptance which is set to "
         "fail") {
      timer_expired const event;
      bool const actual = target.is_accepted(red{false}, event);

      THEN("the transition rejects the event") { REQUIRE_FALSE(actual); }
      THEN("the callback was not fired") { REQUIRE_FALSE(triggered); }
    }
  }
}
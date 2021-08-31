#include <catch.hpp>
#include <skizzay/fsm/action_transition.h>

using namespace skizzay::fsm;

namespace {

struct timer_expired {};

struct green {};
struct red {};

} // namespace

SCENARIO("action transition", "[unit][transition]") {
  GIVEN("an action transition type from red to green") {
    bool triggered = false;
    auto action = [&triggered](timer_expired const &) noexcept {
      triggered = true;
    };
    using target_type =
        action_transition<red, green, timer_expired, decltype(action)>;
    target_type target{action};

    THEN("it models a transition") {
      REQUIRE(is_transition<target_type>::value);
      REQUIRE(concepts::transition<target_type>);

      AND_THEN("it does also model an actionable transtion") {
        REQUIRE(is_actionable_transition<target_type>::value);
        REQUIRE(concepts::actionable_transition<target_type>);
      }
    }

    WHEN("triggered") {
      target.on_triggered(timer_expired{});
      THEN("the callback was fired") { REQUIRE(triggered); }
    }
  }
}
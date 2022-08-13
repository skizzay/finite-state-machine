
#include <skizzay/fsm/transition.h>

#include "test_objects.h"
#include <catch.hpp>

using namespace skizzay::fsm;

namespace {

constexpr std::size_t num_events = 1;
using timer_expired = test_objects::test_event<0>;

using green = test_objects::test_state<0, num_events>;
using red = test_objects::test_state<1, num_events>;

} // namespace

SCENARIO("action transition", "[unit][transition]") {
  GIVEN("an action transition type from red to green") {
    bool triggered = false;
    auto action = [&triggered](timer_expired const &) noexcept {
      triggered = true;
    };
    auto target = action_transition_for<red, green, timer_expired>(std::move(action));

    THEN("it models a transition") {
      REQUIRE(is_transition<decltype(target)>::value);
      REQUIRE(concepts::transition<decltype(target)>);
    }

    WHEN("triggered") {
      target.on_triggered(timer_expired{});
      THEN("the callback was fired") { REQUIRE(triggered); }
    }
  }
}
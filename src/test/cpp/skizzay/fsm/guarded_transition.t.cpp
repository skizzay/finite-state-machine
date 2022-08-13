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

SCENARIO("guarded transition", "[unit][transition]") {

  GIVEN("a guarded transition type from red to green using a pointer to member "
        "function") {
    auto target = transition_guarded_by<red, green, timer_expired>(
        &red::should_transition);

    THEN("it models a transition") {
      REQUIRE(concepts::transition<decltype(target)>);
    }
    WHEN("a timer expired event is queried for acceptance which is set to "
         "pass") {
      bool const actual = target.is_accepted(red{true}, timer_expired{});

      THEN("the transition accepts the event") { REQUIRE(actual); }
    }
    WHEN("a timer expired event is queried for acceptance which is set to "
         "fail") {
      bool const actual = target.is_accepted(red{false}, timer_expired{});

      THEN("the transition rejects the event") { REQUIRE_FALSE(actual); }
    }
  }

  GIVEN("a guarded transition type from red to green using a lambda") {
    auto target = transition_guarded_by<red, green, timer_expired>(
        [](red const &state, timer_expired const &event) noexcept -> bool {
          return state.should_transition(event);
        });

    THEN("it models a transition") {
      REQUIRE(is_transition<decltype(target)>::value);
      REQUIRE(concepts::transition<decltype(target)>);
    }

    WHEN("a timer expired event is queried for acceptance which is set to "
         "pass") {
      bool const actual = target.is_accepted(red{true}, timer_expired{});

      THEN("the transition accepts the event") { REQUIRE(actual); }
    }
    WHEN("a timer expired event is queried for acceptance which is set to "
         "fail") {
      bool const actual = target.is_accepted(red{false}, timer_expired{});

      THEN("the transition rejects the event") { REQUIRE_FALSE(actual); }
    }
  }
}

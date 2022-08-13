#include <skizzay/fsm/transition.h>

#include "skizzay/fsm/accepts.h"
#include "skizzay/fsm/states_list.h"
#include "test_objects.h"
#include <catch.hpp>
#include <tuple>

using namespace skizzay::fsm;

namespace {

struct timer_expired {};

struct green {};
struct red {};

using target_type = simple_transition<red, green, timer_expired>;

} // namespace

SCENARIO("simple transition", "[unit][transition]") {
  test_objects::fake_state_provider<states_list<red, green>> state_provider;

  GIVEN("a simple transition from red to green") {
    target_type target = {};

    THEN("it models a transition") {
      REQUIRE(is_transition<target_type>::value);
      REQUIRE(concepts::transition<target_type>);
    }
    WHEN("a timer expired event is queried for acceptance") {
      bool const actual = skizzay::fsm::accepts(
          std::as_const(target), std::as_const(state_provider).state<red>(),
          timer_expired{}, std::as_const(state_provider));

      THEN("the transition accepts the event") { REQUIRE(actual); }
    }
  }
}
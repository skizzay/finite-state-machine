#include <catch.hpp>
#include <skizzay/fsm/guarded_action_transition.h>

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
    auto action = [&triggered](timer_expired const &) noexcept {
      triggered = true;
    };
    using target_type =
        guarded_action_transition<red, green, timer_expired,
                                  decltype(&red::should_transition),
                                  decltype(action)>;
    using fake_machine = details_::dummy_machine<states_list<red, green>,
                                                 events_list<timer_expired>,
                                                 std::tuple<target_type>>;
    target_type target{&red::should_transition, action};

    THEN("it models a transition") {
      REQUIRE(is_transition<target_type>::value);
      REQUIRE(concepts::transition<target_type>);

      AND_THEN("it does also model an actionable transtion") {
        REQUIRE(is_actionable_transition<target_type>::value);
        REQUIRE(concepts::actionable_transition<target_type>);
      }
    }

    WHEN("a timer expired event is queried for acceptance which is set to "
         "pass") {
      timer_expired const event;
      bool const actual = target.accepts(red{true}, fake_machine{}, event);

      THEN("the transition accepts the event") { REQUIRE(actual); }

      AND_WHEN("triggered") {
        target.on_triggered(timer_expired{});
        THEN("the callback was fired") { REQUIRE(triggered); }
      }
    }
    WHEN("a timer expired event is queried for acceptance which is set to "
         "fail") {
      bool const actual =
          target.accepts(red{false}, fake_machine{}, timer_expired{});

      THEN("the transition rejects the event") { REQUIRE_FALSE(actual); }
      THEN("the callback was not fired") { REQUIRE_FALSE(triggered); }
    }
  }
}
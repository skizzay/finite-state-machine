#include <catch.hpp>
#include <skizzay/fsm/ancestors.h>
#include <skizzay/fsm/guarded_transition.h>

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
    using target_type = guarded_transition<red, green, timer_expired,
                                           decltype(&red::should_transition)>;
    using fake_machine = details_::dummy_machine<states_list<red, green>,
                                                 events_list<timer_expired>,
                                                 std::tuple<target_type>>;
    target_type target{&red::should_transition};

    THEN("it models a transition") {
      REQUIRE(concepts::transition<target_type>);
    }
    WHEN("a timer expired event is queried for acceptance which is set to "
         "pass") {
      bool const actual =
          target.accepts(red{true}, fake_machine{}, timer_expired{});

      THEN("the transition accepts the event") { REQUIRE(actual); }
    }
    WHEN("a timer expired event is queried for acceptance which is set to "
         "fail") {
      bool const actual =
          target.accepts(red{false}, fake_machine{}, timer_expired{});

      THEN("the transition rejects the event") { REQUIRE_FALSE(actual); }
    }
  }

  GIVEN("a guarded transition type from red to green using a lambda") {
    auto predicate = [](red const &state,
                        concepts::machine auto const &,
                        timer_expired const &event) noexcept -> bool {
      return state.should_transition(event);
    };
    using target_type =
        guarded_transition<red, green, timer_expired, decltype(predicate)>;
    using fake_machine = details_::dummy_machine<states_list<red, green>,
                                                 events_list<timer_expired>,
                                                 std::tuple<target_type>>;
    target_type target{predicate};

    THEN("it models a transition") {
      REQUIRE(is_transition<target_type>::value);
      REQUIRE(concepts::transition<target_type>);

      AND_THEN("it does not model an actionable transtion") {
        REQUIRE_FALSE(is_actionable_transition<target_type>::value);
        REQUIRE_FALSE(concepts::actionable_transition<target_type>);
      }
    }

    WHEN("a timer expired event is queried for acceptance which is set to "
         "pass") {
      bool const actual = target.accepts(red{true}, fake_machine{}, timer_expired{});

      THEN("the transition accepts the event") { REQUIRE(actual); }
    }
    WHEN("a timer expired event is queried for acceptance which is set to "
         "fail") {
      bool const actual =
          target.accepts(red{false}, fake_machine{}, timer_expired{});

      THEN("the transition rejects the event") { REQUIRE_FALSE(actual); }
    }
  }
}

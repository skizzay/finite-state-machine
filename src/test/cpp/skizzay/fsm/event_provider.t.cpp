#include "skizzay/fsm/event_provider.h"

#include "test_objects.h"
#include <catch2/catch.hpp>

using namespace skizzay::fsm;

namespace {
struct missing_event_type {
  test_objects::test_event<0> e;

  constexpr test_objects::test_event<0> const &event() const noexcept {
    return e;
  }
};

struct missing_event_member_function {
  using event_type = test_objects::test_event<0>;
};

struct valid_event_provider {
  using event_type = test_objects::test_event<0>;

  event_type e;

  constexpr event_type const &event() const noexcept { return e; }
};
} // namespace

TEST_CASE("missing event type is not an event provider",
          "[unit][event_provider]") {
  REQUIRE_FALSE(is_event_provider<missing_event_type>::value);
  REQUIRE_FALSE(concepts::event_provider<missing_event_type>);
}

TEST_CASE("missing event member function is not an event provider",
          "[unit][event_provider]") {
  REQUIRE_FALSE(is_event_provider<missing_event_member_function>::value);
  REQUIRE_FALSE(concepts::event_provider<missing_event_member_function>);
}

TEST_CASE("valid event provider is an event provider",
          "[unit][event_provider]") {
  REQUIRE(is_event_provider<valid_event_provider>::value);
  REQUIRE(concepts::event_provider<valid_event_provider>);
}
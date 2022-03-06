#include <skizzay/fsm/state_transition_context.h>

#include "skizzay/fsm/events_list.h"
#include "skizzay/fsm/states_list.h"
#include "test_objects.h"
#include <catch2/catch.hpp>

using namespace skizzay::fsm;

namespace {
struct not_event_engine {
  using event_type = test_objects::test_event<0>;
  using states_list_type = states_list<test_objects::test_state<0, 1>>;

  event_type e;
  test_objects::test_state<0, 1> s;

  constexpr event_type const &event() const noexcept { return e; }

  template <concepts::state_in<states_list_type> State>
  constexpr State &state() noexcept {
    return s;
  }

  template <concepts::state_in<states_list_type> State>
  constexpr State const &state() const noexcept {
    return s;
  }
};

struct not_event_provider {
  using states_list_type = states_list<test_objects::test_state<0, 1>>;
  using events_list_type = events_list<test_objects::test_event<0>>;

  test_objects::test_state<0, 1> s;

  template <concepts::state_in<states_list_type> State>
  constexpr State &state() noexcept {
    return s;
  }

  template <concepts::state_in<states_list_type> State>
  constexpr State const &state() const noexcept {
    return s;
  }

  constexpr void
  post_event(concepts::event_in<events_list_type> auto const &) noexcept {}
};

struct not_state_provider {
  using event_type = test_objects::test_event<0>;
  using events_list_type = events_list<test_objects::test_event<0>>;

  event_type e;

  constexpr event_type const &event() const noexcept { return e; }

  constexpr void
  post_event(concepts::event_in<events_list_type> auto const &) noexcept {}
};

struct valid_state_transition_context {
  using event_type = test_objects::test_event<0>;
  using states_list_type = states_list<test_objects::test_state<0, 1>>;
  using events_list_type = events_list<test_objects::test_event<0>>;

  event_type e;
  test_objects::test_state<0, 1> s;

  constexpr event_type const &event() const noexcept { return e; }

  template <std::same_as<test_objects::test_state<0, 1>> State>
  constexpr State &state() noexcept {
    return s;
  }

  template <std::same_as<test_objects::test_state<0, 1>> State>
  constexpr State const &state() const noexcept {
    return s;
  }

  constexpr void
  post_event(concepts::event_in<events_list_type> auto const &) noexcept {}
};
} // namespace

TEST_CASE("not an event engine is not a state transition context",
          "[unit][state_transition_context]") {
  REQUIRE_FALSE(is_state_transition_context<not_event_engine>::value);
  REQUIRE_FALSE(concepts::state_transition_context<not_event_engine>);
}

TEST_CASE("not an event provider is not a state transition context",
          "[unit][state_transition_context]") {
  REQUIRE_FALSE(is_state_transition_context<not_event_provider>::value);
  REQUIRE_FALSE(concepts::state_transition_context<not_event_provider>);
}

TEST_CASE("not a state provider is not a state transition context",
          "[unit][state_transition_context]") {
  REQUIRE_FALSE(is_state_transition_context<not_state_provider>::value);
  REQUIRE_FALSE(concepts::state_transition_context<not_state_provider>);
}

TEST_CASE("valid state transition context is a state transition context",
          "[unit][state_transition_context]") {
  REQUIRE(is_state_transition_context<valid_state_transition_context>::value);
  REQUIRE(concepts::state_transition_context<valid_state_transition_context>);
}

TEST_CASE("valid state transition context for event is a state transition "
          "context when it's an event provider for that event",
          "[unit][state_transition_context]") {
  REQUIRE(is_state_transition_context_for_event<
          valid_state_transition_context,
          typename valid_state_transition_context::event_type>::value);
  REQUIRE(concepts::state_transition_context_for_event<
          valid_state_transition_context,
          typename valid_state_transition_context::event_type>);
}

TEST_CASE("valid state transition context for event is not a state transition "
          "context when it's an event provider for a different event type",
          "[unit][state_transition_context]") {
  REQUIRE_FALSE(is_state_transition_context_for_event<
                valid_state_transition_context,
                test_objects::test_event<100>>::value);
  REQUIRE_FALSE(concepts::state_transition_context_for_event<
                valid_state_transition_context, test_objects::test_event<100>>);
}
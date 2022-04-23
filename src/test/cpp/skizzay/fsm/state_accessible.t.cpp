#include "skizzay/fsm/state_accessible.h"

#include "skizzay/fsm/optional_reference.h"
#include "test_objects.h"
#include <catch2/catch.hpp>

using namespace skizzay::fsm;

struct not_state_queryable {
  test_objects::test_state<0, 1> s;

  template <std::same_as<test_objects::test_state<0, 1>>>
  constexpr test_objects::test_state<0, 1> &state() noexcept {
    return s;
  }

  template <std::same_as<test_objects::test_state<0, 1>>>
  constexpr test_objects::test_state<0, 1> const &state() const noexcept {
    return s;
  }
};

struct valid_state_queryable {
  using states_list_type = states_list<test_objects::test_state<0, 1>>;

  test_objects::test_state<0, 1> s;

  template <typename T> constexpr bool is() const noexcept {
    return std::same_as<T, test_objects::test_state<0, 1>>;
  }

  template <std::same_as<test_objects::test_state<0, 1>> S>
  constexpr optional_reference<S const> current_state() const noexcept {
    return s;
  }
};

struct missing_const_get : valid_state_queryable {
  template <std::same_as<test_objects::test_state<0, 1>>>
  constexpr test_objects::test_state<0, 1> &state() noexcept {
    return s;
  }
};

struct missing_mutable_get : valid_state_queryable {
  template <std::same_as<test_objects::test_state<0, 1>>>
  constexpr test_objects::test_state<0, 1> const &state() const noexcept {
    return s;
  }
};

struct valid_state_accessible : valid_state_queryable {
  template <std::same_as<test_objects::test_state<0, 1>>>
  constexpr test_objects::test_state<0, 1> &state() noexcept {
    return s;
  }

  template <std::same_as<test_objects::test_state<0, 1>>>
  constexpr test_objects::test_state<0, 1> const &state() const noexcept {
    return s;
  }
};

TEST_CASE("non-state queryable is not state accessible",
          "[unit][state_accessible]") {
  REQUIRE_FALSE(concepts::state_accessible<not_state_queryable>);
}

TEST_CASE("missing const state is not state accessible",
          "[unit][state_accessible]") {
  REQUIRE_FALSE(concepts::state_accessible<missing_const_get>);
}

TEST_CASE("missing mutable state is not state accessible",
          "[unit][state_accessible]") {
  REQUIRE_FALSE(concepts::state_accessible<missing_mutable_get>);
}

TEST_CASE("valid state accessible is state accessible",
          "[unit][state_accessible]") {
  REQUIRE(concepts::state_accessible<valid_state_accessible>);
}
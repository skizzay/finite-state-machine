#include "skizzay/fsm/state_accessible.h"

#include "skizzay/fsm/optional_reference.h"
#include "test_objects.h"
#include <catch2/catch.hpp>

using namespace skizzay::fsm;

namespace {
struct not_state_queryable {
  test_objects::test_state<0, 1> state;

  template <std::same_as<test_objects::test_state<0, 1>>>
  constexpr test_objects::test_state<0, 1> &get() noexcept {
    return state;
  }

  template <std::same_as<test_objects::test_state<0, 1>>>
  constexpr test_objects::test_state<0, 1> const &get() const noexcept {
    return state;
  }
};

struct valid_state_queryable {
  using states_list_type = states_list<test_objects::test_state<0, 1>>;

  test_objects::test_state<0, 1> state;

  template <typename T> constexpr bool is() const noexcept {
    return std::same_as<T, test_objects::test_state<0, 1>>;
  }

  template <std::same_as<test_objects::test_state<0, 1>> S>
  constexpr optional_reference<S const> current_state() const noexcept {
    return state;
  }
};

struct missing_const_get : valid_state_queryable {
  template <std::same_as<test_objects::test_state<0, 1>>>
  constexpr test_objects::test_state<0, 1> &get() noexcept {
    return state;
  }
};

struct missing_mutable_get : valid_state_queryable {
  template <std::same_as<test_objects::test_state<0, 1>>>
  constexpr test_objects::test_state<0, 1> const &get() const noexcept {
    return state;
  }
};

struct valid_state_accessible : valid_state_queryable {
  template <std::same_as<test_objects::test_state<0, 1>>>
  constexpr test_objects::test_state<0, 1> &get() noexcept {
    return state;
  }

  template <std::same_as<test_objects::test_state<0, 1>>>
  constexpr test_objects::test_state<0, 1> const &get() const noexcept {
    return state;
  }
};
} // namespace

TEST_CASE("non-state queryable is not state accessible",
          "[unit][state_accessible]") {
  REQUIRE_FALSE(is_state_accessible<not_state_queryable>::value);
  REQUIRE_FALSE(concepts::state_accessible<not_state_queryable>);
}

TEST_CASE("missing const get is not state accessible",
          "[unit][state_accessible]") {
  REQUIRE_FALSE(is_state_accessible<missing_const_get>::value);
  REQUIRE_FALSE(concepts::state_accessible<missing_const_get>);
}

TEST_CASE("missing mutable get is not state accessible",
          "[unit][state_accessible]") {
  REQUIRE_FALSE(is_state_accessible<missing_mutable_get>::value);
  REQUIRE_FALSE(concepts::state_accessible<missing_mutable_get>);
}

TEST_CASE("valid state accessible is state accessible",
          "[unit][state_accessible]") {
  REQUIRE(is_state_accessible<valid_state_accessible>::value);
  REQUIRE(concepts::state_accessible<valid_state_accessible>);
}
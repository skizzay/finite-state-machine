#include "skizzay/fsm/state_queryable.h"

#include "skizzay/fsm/optional_reference.h"
#include "test_objects.h"
#include <catch2/catch.hpp>

using namespace skizzay::fsm;

namespace {
struct missing_states_list_type {
  template <concepts::state> constexpr bool is() const noexcept {
    return false;
  }
  template <concepts::state S>
  constexpr optional_reference<S const> current_state() const noexcept {
    return std::nullopt;
  }
};

struct missing_is_template_member_function {
  using states_list_type = states_list<test_objects::test_state<0, 1>>;
  template <concepts::state S>
  constexpr optional_reference<S const> current_state() const noexcept {
    return std::nullopt;
  }
};

struct missing_current_state_template_member_function {
  using states_list_type = states_list<test_objects::test_state<0, 1>>;
  template <concepts::state> constexpr bool is() const noexcept {
    return false;
  }
};

struct valid_state_queryable {
  using states_list_type = states_list<test_objects::test_state<0, 1>>;
  template <concepts::state> constexpr bool is() const noexcept {
    return false;
  }
  template <concepts::state S>
  constexpr optional_reference<S const> current_state() const noexcept {
    return std::nullopt;
  }
};
} // namespace

TEST_CASE("missing states list type is not state queryable",
          "[unit][state_queryable]") {
  REQUIRE_FALSE(is_state_queryable<missing_states_list_type>::value);
  REQUIRE_FALSE(concepts::state_queryable<missing_states_list_type>);
}

TEST_CASE("missing is member function is not state queryable",
          "[unit][state_queryable]") {
  REQUIRE_FALSE(is_state_queryable<missing_is_template_member_function>::value);
  REQUIRE_FALSE(concepts::state_queryable<missing_is_template_member_function>);
}

TEST_CASE("missing current state member function is not state queryable",
          "[unit][state_queryable]") {
  REQUIRE_FALSE(is_state_queryable<
                missing_current_state_template_member_function>::value);
  REQUIRE_FALSE(concepts::state_queryable<
                missing_current_state_template_member_function>);
}

TEST_CASE("valid state queryable is state queryable",
          "[unit][state_queryable]") {
  REQUIRE(is_state_queryable<valid_state_queryable>::value);
  REQUIRE(concepts::state_queryable<valid_state_queryable>);
}
#include "skizzay/fsm/state_provider.h"

#include "test_objects.h"
#include <catch2/catch.hpp>

using namespace skizzay::fsm;

namespace {
struct missing_states_list_type {
  test_objects::test_state<0, 1> s;

  template <std::same_as<test_objects::test_state<0, 1>>>
  constexpr test_objects::test_state<0, 1> const &state() const noexcept {
    return s;
  }

  template <std::same_as<test_objects::test_state<0, 1>>>
  constexpr test_objects::test_state<0, 1> &state() noexcept {
    return s;
  }
};

struct missing_mutable_state {
  using states_list_type = states_list<test_objects::test_state<0, 1>>;
  test_objects::test_state<0, 1> s;

  template <std::same_as<test_objects::test_state<0, 1>>>
  constexpr test_objects::test_state<0, 1> const &state() const noexcept {
    return s;
  }
};

struct missing_immutable_state {
  using states_list_type = states_list<test_objects::test_state<0, 1>>;
  test_objects::test_state<0, 1> s;

  template <std::same_as<test_objects::test_state<0, 1>>>
  constexpr test_objects::test_state<0, 1> &state() noexcept {
    return s;
  }
};

struct valid_state_provider {
  using states_list_type = states_list<test_objects::test_state<0, 1>>;
  test_objects::test_state<0, 1> s;

  template <std::same_as<test_objects::test_state<0, 1>>>
  constexpr test_objects::test_state<0, 1> const &state() const noexcept {
    return s;
  }

  template <std::same_as<test_objects::test_state<0, 1>>>
  constexpr test_objects::test_state<0, 1> &state() noexcept {
    return s;
  }
};
} // namespace

TEST_CASE("missing states list type is not a state provider",
          "[unit][state_provider]") {
  REQUIRE_FALSE(is_state_provider<missing_states_list_type>::value);
  REQUIRE_FALSE(concepts::state_provider<missing_states_list_type>);
}

TEST_CASE(
    "missing states list type is a state provider of a specific state though",
    "[unit][state_provider]") {
  REQUIRE(is_state_provider_of<missing_states_list_type,
                                     test_objects::test_state<0, 1>>::value);
  REQUIRE(concepts::state_provider_of<missing_states_list_type,
                                            test_objects::test_state<0, 1>>);
}

TEST_CASE("missing mutable state is not a state provider",
          "[unit][state_provider]") {
  REQUIRE_FALSE(is_state_provider<missing_mutable_state>::value);
  REQUIRE_FALSE(concepts::state_provider<missing_mutable_state>);
}

TEST_CASE("missing immutable state is not a state provider",
          "[unit][state_provider]") {
  REQUIRE_FALSE(is_state_provider<missing_immutable_state>::value);
  REQUIRE_FALSE(concepts::state_provider<missing_immutable_state>);
}

TEST_CASE("valid state provider is a state provider",
          "[unit][state_provider]") {
  REQUIRE(is_state_provider<valid_state_provider>::value);
  REQUIRE(concepts::state_provider<valid_state_provider>);
}
#include "skizzay/fsm/state_queryable.h"

#include "skizzay/fsm/is_done.h"
#include "skizzay/fsm/optional_reference.h"
#include "test_objects.h"
#include <catch2/catch.hpp>

using namespace skizzay::fsm;

struct missing_states_list_type {
  template <concepts::state> constexpr bool is() const noexcept {
    return false;
  }
  template <concepts::state S>
  constexpr optional_reference<S const> current_state() const noexcept {
    return std::nullopt;
  }

  template <typename F> constexpr bool query(F &&f) const { return is_done(f); }
};

struct missing_is_template_member_function {
  using states_list_type = states_list<test_objects::test_state<0, 1>>;
  template <concepts::state S>
  constexpr optional_reference<S const> current_state() const noexcept {
    return std::nullopt;
  }

  template <typename F> constexpr bool query(F &&f) const { return is_done(f); }
};

struct missing_current_state_template_member_function {
  using states_list_type = states_list<test_objects::test_state<0, 1>>;
  template <concepts::state> constexpr bool is() const noexcept {
    return false;
  }

  template <typename F> constexpr bool query(F &&f) const { return is_done(f); }
};

struct missing_query {
  using states_list_type = states_list<test_objects::test_state<0, 1>>;
  template <concepts::state> constexpr bool is() const noexcept {
    return false;
  }
  template <concepts::state S>
  constexpr optional_reference<S const> current_state() const noexcept {
    return std::nullopt;
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

  template<typename F>
  constexpr bool query(F &&f) const {
    return is_done(f);
  }
};

TEST_CASE("missing states list type is not state queryable",
          "[unit][state_queryable]") {
  REQUIRE_FALSE(concepts::state_queryable<missing_states_list_type>);
}

TEST_CASE("missing is member function is not state queryable",
          "[unit][state_queryable]") {
  REQUIRE_FALSE(concepts::state_queryable<missing_is_template_member_function>);
}

TEST_CASE("missing current state member function is not state queryable",
          "[unit][state_queryable]") {
  REQUIRE_FALSE(concepts::state_queryable<
                missing_current_state_template_member_function>);
}

TEST_CASE("missing query member function is not state queryable",
          "[unit][state_queryable]") {
  REQUIRE_FALSE(concepts::state_queryable<
                missing_query>);
}

TEST_CASE("valid state queryable is state queryable",
          "[unit][state_queryable]") {
  REQUIRE(concepts::state_queryable<valid_state_queryable>);
}
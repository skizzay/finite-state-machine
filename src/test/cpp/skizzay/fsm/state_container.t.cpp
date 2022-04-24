#include <skizzay/fsm/state_container.h>

#include "skizzay/fsm/event_transition_context.h"
#include "test_objects.h"

#include <catch2/catch.hpp>

using namespace skizzay::fsm;

using test_event_type = test_objects::test_event<0>;
using test_state_type = test_objects::test_state<0, 1>;

struct not_state_accessible {
  test_state_type s;

  template <typename T> constexpr bool is() const noexcept {
    return std::same_as<T, test_state_type>;
  }

  template <std::same_as<test_state_type> S>
  constexpr optional_reference<S const> current_state() const noexcept {
    return s;
  }

  template <std::same_as<test_state_type>>
  constexpr test_state_type &state() noexcept {
    return s;
  }

  template <std::same_as<test_state_type>>
  constexpr test_state_type const &state() const noexcept {
    return s;
  }

  constexpr bool on_event(concepts::event_transition_context auto &) {
    return false;
  }

  constexpr bool is_active() const noexcept { return false; }

  constexpr bool is_inactive() const noexcept { return true; }
};

struct missing_on_event_member_function {
  using states_list_type = states_list<test_state_type>;

  test_state_type s;

  template <typename T> constexpr bool is() const noexcept {
    return std::same_as<T, test_state_type>;
  }

  template <std::same_as<test_state_type> S>
  constexpr optional_reference<S const> current_state() const noexcept {
    return s;
  }

  template <std::same_as<test_state_type>>
  constexpr test_state_type &state() noexcept {
    return s;
  }

  template <std::same_as<test_state_type>>
  constexpr test_state_type const &state() const noexcept {
    return s;
  }

  constexpr bool is_active() const noexcept { return false; }

  constexpr bool is_inactive() const noexcept { return true; }
};

struct missing_is_active_member_function {
  using states_list_type = states_list<test_state_type>;

  test_state_type s;

  template <typename T> constexpr bool is() const noexcept {
    return std::same_as<T, test_state_type>;
  }

  template <std::same_as<test_state_type> S>
  constexpr optional_reference<S const> current_state() const noexcept {
    return s;
  }

  template <std::same_as<test_state_type>>
  constexpr test_state_type &state() noexcept {
    return s;
  }

  template <std::same_as<test_state_type>>
  constexpr test_state_type const &state() const noexcept {
    return s;
  }

  constexpr bool on_event(concepts::event_transition_context auto &) {
    return false;
  }

  constexpr bool is_inactive() const noexcept { return true; }
};

struct missing_is_inactive_member_function {
  using states_list_type = states_list<test_state_type>;

  test_state_type s;

  template <typename T> constexpr bool is() const noexcept {
    return std::same_as<T, test_state_type>;
  }

  template <std::same_as<test_state_type> S>
  constexpr optional_reference<S const> current_state() const noexcept {
    return s;
  }

  template <std::same_as<test_state_type>>
  constexpr test_state_type &state() noexcept {
    return s;
  }

  template <std::same_as<test_state_type>>
  constexpr test_state_type const &state() const noexcept {
    return s;
  }

  constexpr bool on_event(concepts::event_transition_context auto &) {
    return false;
  }

  constexpr bool is_active() const noexcept { return false; }
};

struct valid_state_container {
  using states_list_type = states_list<test_state_type>;

  test_state_type s;

  template <typename T> constexpr bool is() const noexcept {
    return std::same_as<T, test_state_type>;
  }

  template <std::same_as<test_state_type> S>
  constexpr optional_reference<S const> current_state() const noexcept {
    return s;
  }

  template <std::same_as<test_state_type>>
  constexpr test_state_type &state() noexcept {
    return s;
  }

  template <std::same_as<test_state_type>>
  constexpr test_state_type const &state() const noexcept {
    return s;
  }

  constexpr bool on_event(concepts::event_transition_context auto &) {
    return false;
  }

  constexpr void on_entry(concepts::entry_context auto &) {}

  constexpr bool is_active() const noexcept { return false; }

  constexpr bool is_inactive() const noexcept { return true; }
};

TEST_CASE("not state accessible is not a state container",
          "[unit][state_container]") {
  REQUIRE(concepts::event_transition_context<
          is_state_container_details_::fake_event_transition_context>);
  REQUIRE_FALSE(concepts::state_container<not_state_accessible>);
}

TEST_CASE("missing on event member function is not a state container",
          "[unit][state_container]") {
  REQUIRE(concepts::event_transition_context<
          is_state_container_details_::fake_event_transition_context>);
  REQUIRE_FALSE(concepts::state_container<missing_on_event_member_function>);
}

TEST_CASE("missing is active member function is not a state container",
          "[unit][state_container]") {
  REQUIRE(concepts::event_transition_context<
          is_state_container_details_::fake_event_transition_context>);
  REQUIRE_FALSE(concepts::state_container<missing_is_active_member_function>);
}

TEST_CASE("missing is inactive member function is not a state container",
          "[unit][state_container]") {
  REQUIRE(concepts::event_transition_context<
          is_state_container_details_::fake_event_transition_context>);
  REQUIRE_FALSE(concepts::state_container<missing_is_inactive_member_function>);
}

TEST_CASE("valid state container is a state container",
          "[unit][state_container]") {
  REQUIRE(concepts::event_transition_context<
          is_state_container_details_::fake_event_transition_context>);
  REQUIRE(concepts::state_container<valid_state_container>);
}
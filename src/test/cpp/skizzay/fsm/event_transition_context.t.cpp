#include <skizzay/fsm/event_transition_context.h>

#include "test_objects.h"
#include <catch2/catch.hpp>
#include <skizzay/fsm/event_context.h>
#include <skizzay/fsm/simple_transition.h>
#include <skizzay/fsm/states_list.h>
#include <skizzay/fsm/transition_table.h>
#include <skizzay/fsm/type_list.h>
#include <skizzay/fsm/types.h>

#include <tuple>

using namespace skizzay::fsm;

// namespace {
using test_event_type = test_objects::test_event<0>;
using test_state_type = test_objects::test_state<0, 1>;

struct not_move_constructible {
  using event_type = test_event_type;
  using states_list_type = states_list<test_state_type>;
  using events_list_type = events_list<test_event_type>;
  using transition_table_type = std::tuple<
      simple_transition<test_state_type, test_state_type, test_event_type>>;

  not_move_constructible(not_move_constructible const &) = delete;
  not_move_constructible(not_move_constructible &&) = delete;

  void operator=(not_move_constructible const &) = delete;
  void operator=(not_move_constructible &&) = delete;

  event_type e;
  test_objects::test_state<0, 1> s;
  [[no_unique_address]] simple_transition<test_state_type, test_state_type,
                                          test_event_type>
      t;

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

  constexpr std::tuple<
      simple_transition<test_state_type, test_state_type, test_event_type> &>
  get_transitions(test_state_type const &) noexcept {
    return {t};
  }
};

struct not_event_context {
  using states_list_type = states_list<test_state_type>;
  using events_list_type = events_list<test_event_type>;
  using transition_table_type = std::tuple<
      simple_transition<test_state_type, test_state_type, test_event_type>>;

  test_objects::test_state<0, 1> s;
  [[no_unique_address]] simple_transition<test_state_type, test_state_type,
                                          test_event_type>
      t;

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

  constexpr std::tuple<
      simple_transition<test_state_type, test_state_type, test_event_type> &>
  get_transitions(test_state_type const &) noexcept {
    return {t};
  }
};

struct missing_transition_table_type {
  using event_type = test_event_type;
  using states_list_type = states_list<test_state_type>;
  using events_list_type = events_list<test_event_type>;

  event_type e;
  test_objects::test_state<0, 1> s;
  [[no_unique_address]] simple_transition<test_state_type, test_state_type,
                                          test_event_type>
      t;

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

  constexpr std::tuple<
      simple_transition<test_state_type, test_state_type, test_event_type> &>
  get_transitions(test_state_type const &) noexcept {
    return {t};
  }
};

struct missing_get_transitions_member_function {
  using event_type = test_event_type;
  using states_list_type = states_list<test_state_type>;
  using events_list_type = events_list<test_event_type>;
  using transition_table_type = std::tuple<
      simple_transition<test_state_type, test_state_type, test_event_type>>;

  event_type e;
  test_objects::test_state<0, 1> s;
  [[no_unique_address]] simple_transition<test_state_type, test_state_type,
                                          test_event_type>
      t;

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

struct valid_event_transition_context {
  using event_type = test_event_type;
  using states_list_type = states_list<test_state_type>;
  using events_list_type = events_list<test_event_type>;
  using transition_table_type = std::tuple<
      simple_transition<test_state_type, test_state_type, test_event_type>>;

  event_type e;
  test_objects::test_state<0, 1> s;
  [[no_unique_address]] simple_transition<test_state_type, test_state_type,
                                          test_event_type>
      t;

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

  constexpr std::tuple<
      simple_transition<test_state_type, test_state_type, test_event_type> &>
  get_transitions(test_state_type const &) noexcept {
    return {t};
  }
};
// } // namespace

TEST_CASE("not move constructible is not an event transition context",
          "[unit][event_context]") {
  REQUIRE_FALSE(std::move_constructible<not_move_constructible>);
  REQUIRE(concepts::event_context<not_move_constructible>);
  REQUIRE(concepts::transition_table<
          detected_t<transition_table_t, not_move_constructible>>);
  REQUIRE_FALSE(is_event_transition_context<not_move_constructible>::value);
  REQUIRE_FALSE(concepts::event_transition_context<not_move_constructible>);
}

TEST_CASE("not an event context is not an event transition context",
          "[unit][event_context]") {
  REQUIRE(std::move_constructible<not_event_context>);
  REQUIRE_FALSE(concepts::event_context<not_event_context>);
  REQUIRE(concepts::transition_table<
          detected_t<transition_table_t, not_event_context>>);
  REQUIRE_FALSE(is_event_transition_context<not_event_context>::value);
  REQUIRE_FALSE(concepts::event_transition_context<not_event_context>);
}

TEST_CASE("missing transition table is not an event transition context",
          "[unit][event_context]") {
  REQUIRE(std::move_constructible<missing_transition_table_type>);
  REQUIRE(concepts::event_context<missing_transition_table_type>);
  REQUIRE_FALSE(concepts::transition_table<
                detected_t<transition_table_t, missing_transition_table_type>>);
  REQUIRE_FALSE(
      is_event_transition_context<missing_transition_table_type>::value);
  REQUIRE_FALSE(
      concepts::event_transition_context<missing_transition_table_type>);
}

TEST_CASE("missing get transtions member function is not an event context",
          "[unit][event_context]") {
  REQUIRE(std::move_constructible<missing_get_transitions_member_function>);
  REQUIRE(concepts::event_context<missing_get_transitions_member_function>);
  REQUIRE(concepts::transition_table<detected_t<
              transition_table_t, missing_get_transitions_member_function>>);
  REQUIRE_FALSE(is_event_transition_context<
                missing_get_transitions_member_function>::value);
  REQUIRE_FALSE(concepts::event_transition_context<
                missing_get_transitions_member_function>);
}

TEST_CASE("valid event context is an event context", "[unit][event_context]") {
  REQUIRE(std::move_constructible<valid_event_transition_context>);
  REQUIRE(concepts::event_context<valid_event_transition_context>);
  REQUIRE(concepts::transition_table<
          detected_t<transition_table_t, valid_event_transition_context>>);
  REQUIRE(is_event_transition_context<valid_event_transition_context>::value);
  REQUIRE(concepts::event_transition_context<valid_event_transition_context>);
}
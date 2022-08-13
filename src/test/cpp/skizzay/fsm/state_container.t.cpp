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

  template <typename F> constexpr bool query(F &&f) const { return is_done(f); }

  template <std::same_as<test_state_type>>
  constexpr test_state_type &state() noexcept {
    return s;
  }

  template <std::same_as<test_state_type>>
  constexpr test_state_type const &state() const noexcept {
    return s;
  }

  constexpr bool on_event(concepts::event_transition_context auto &,
                          concepts::event auto const &,
                          concepts::event_engine auto,
                          concepts::state_provider auto) {
    return false;
  }

  constexpr void on_entry(concepts::state_schedule auto const &,
                          concepts::event auto const &,
                          concepts::event_engine auto,
                          concepts::state_provider auto) {}

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

  template <typename F> constexpr bool query(F &&f) const { return is_done(f); }

  template <std::same_as<test_state_type>>
  constexpr test_state_type &state() noexcept {
    return s;
  }

  template <std::same_as<test_state_type>>
  constexpr test_state_type const &state() const noexcept {
    return s;
  }

  constexpr void on_entry(concepts::state_schedule auto const &,
                          concepts::event auto const &,
                          concepts::event_engine auto,
                          concepts::state_provider auto) {}

  constexpr bool is_active() const noexcept { return false; }

  constexpr bool is_inactive() const noexcept { return true; }
};

struct missing_on_entry_member_function {
  using states_list_type = states_list<test_state_type>;

  test_state_type s;

  template <typename T> constexpr bool is() const noexcept {
    return std::same_as<T, test_state_type>;
  }

  template <std::same_as<test_state_type> S>
  constexpr optional_reference<S const> current_state() const noexcept {
    return s;
  }

  template <typename F> constexpr bool query(F &&f) const { return is_done(f); }

  template <std::same_as<test_state_type>>
  constexpr test_state_type &state() noexcept {
    return s;
  }

  template <std::same_as<test_state_type>>
  constexpr test_state_type const &state() const noexcept {
    return s;
  }

  constexpr bool on_event(concepts::event_transition_context auto &,
                          concepts::event auto const &,
                          concepts::event_engine auto,
                          concepts::state_provider auto) {
    return false;
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

  template <typename F> constexpr bool query(F &&f) const { return is_done(f); }

  template <std::same_as<test_state_type>>
  constexpr test_state_type &state() noexcept {
    return s;
  }

  template <std::same_as<test_state_type>>
  constexpr test_state_type const &state() const noexcept {
    return s;
  }

  constexpr bool on_event(concepts::event_transition_context auto &,
                          concepts::event auto const &,
                          concepts::event_engine auto,
                          concepts::state_provider auto) {
    return false;
  }

  constexpr void on_entry(concepts::state_schedule auto const &,
                          concepts::event auto const &,
                          concepts::event_engine auto,
                          concepts::state_provider auto) {}

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

  template <typename F> constexpr bool query(F &&f) const { return is_done(f); }

  template <std::same_as<test_state_type>>
  constexpr test_state_type &state() noexcept {
    return s;
  }

  template <std::same_as<test_state_type>>
  constexpr test_state_type const &state() const noexcept {
    return s;
  }

  constexpr bool on_event(concepts::event_transition_context auto &,
                          concepts::event auto const &,
                          concepts::event_engine auto,
                          concepts::state_provider auto) {
    return false;
  }

  constexpr void on_entry(concepts::state_schedule auto const &,
                          concepts::event auto const &,
                          concepts::event_engine auto,
                          concepts::state_provider auto) {}

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

  template <typename F> constexpr bool query(F &&f) const { return is_done(f); }

  test_state_type memento() const
      noexcept(std::is_nothrow_copy_constructible_v<test_state_type>) {
    return s;
  }

  void recover_from_memento(test_state_type &&s) {
    this->s = std::move(s);
  }

  template <std::same_as<test_state_type>>
  constexpr test_state_type &state() noexcept {
    return s;
  }

  template <std::same_as<test_state_type>>
  constexpr test_state_type const &state() const noexcept {
    return s;
  }

  constexpr bool on_event(concepts::event_transition_context auto &,
                          concepts::event auto const &,
                          concepts::event_engine auto,
                          concepts::state_provider auto) {
    return false;
  }

  constexpr void on_entry(concepts::state_schedule auto const &,
                          concepts::event auto const &,
                          concepts::event_engine auto,
                          concepts::state_provider auto) {}

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

TEST_CASE("missing on entry member function is not a state container",
          "[unit][state_container]") {
  REQUIRE(concepts::event_transition_context<
          is_state_container_details_::fake_event_transition_context>);
  REQUIRE_FALSE(concepts::state_container<missing_on_entry_member_function>);
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
  REQUIRE(std::move_constructible<
          is_state_container_details_::fake_event_transition_context>);
  REQUIRE(concepts::state_schedule<
          is_state_container_details_::fake_state_schedule>);
  REQUIRE(concepts::state_provider<
          is_state_container_details_::fake_state_provider>);
  REQUIRE(
      concepts::event_engine<is_state_container_details_::fake_event_engine>);
  REQUIRE(concepts::event<is_state_container_details_::event_type>);
  REQUIRE(concepts::transition_table<transition_table_t<
              is_state_container_details_::fake_event_transition_context>>);
  REQUIRE(concepts::states_list<current_states_list_t<
              is_state_container_details_::fake_event_transition_context>>);
  REQUIRE(
      all_v<current_states_list_t<
                is_state_container_details_::fake_event_transition_context>,
            curry<is_event_transition_context_details_::
                      has_get_transitions_template_member_function,
                  is_state_container_details_::fake_event_transition_context>::
                template type>);
  REQUIRE(
      all_v<current_states_list_t<
                is_state_container_details_::fake_event_transition_context>,
            curry<is_event_transition_context_details_::
                      has_schedule_entry_template_member_function,
                  is_state_container_details_::fake_event_transition_context>::
                template type>);
  using event_type = is_state_container_details_::event_type;
  using state_type = is_state_container_details_::state_type;

  REQUIRE(
      is_event_transition_context_details_::
          has_on_transition_template_member_function<
              is_state_container_details_::fake_event_transition_context,
              simple_transition<state_type, state_type, event_type>>::value);
  REQUIRE(length_v<current_states_list_t<
              is_state_container_details_::fake_event_transition_context>> ==
          1);
  REQUIRE(std::same_as<
          state_type,
          front_t<current_states_list_t<
              is_state_container_details_::fake_event_transition_context>>>);

  REQUIRE(
      is_event_transition_context_details_::
          has_on_transition_template_member_function<
              is_state_container_details_::fake_event_transition_context,
              simple_transition<state_type, state_type, event_type>>::value);
  REQUIRE(all_v<
          map_t<transition_table_t<
                    is_state_container_details_::fake_event_transition_context>,
                std::remove_cvref_t>,
          curry<is_event_transition_context_details_::
                    has_on_transition_template_member_function,
                is_state_container_details_::fake_event_transition_context>::
              template type>);
  REQUIRE(concepts::event_transition_context<
          is_state_container_details_::fake_event_transition_context>);
  REQUIRE(concepts::state_container<valid_state_container>);
}
#include <catch.hpp>
#include <memory>
#include <skizzay/fsm/ancestors.h>
#include <skizzay/fsm/event.h>
#include <skizzay/fsm/simple_transition.h>
#include <skizzay/fsm/single_state_container.h>
#include <skizzay/fsm/traits.h>
#include <tuple>
#include <vector>

using namespace skizzay::fsm;

namespace {
template <std::size_t> struct test_event {};

struct test_state {
  std::size_t initial_entry_count = 0;
  std::size_t final_exit_count = 0;
  std::array<std::size_t, 2> event_entry_count{0, 0};
  std::array<std::size_t, 2> event_reentry_count{0, 0};
  std::array<std::size_t, 2> event_exit_count{0, 0};

  void on_entry(initial_entry_event_t const &) noexcept {
    initial_entry_count += 1;
  }
  template <std::size_t I> void on_entry(test_event<I> const &) noexcept {
    event_entry_count[I] += 1;
  }

  void on_exit(final_exit_event_t const &) noexcept { final_exit_count += 1; }
  template <std::size_t I> void on_exit(test_event<I> const &) noexcept {
    event_exit_count[I] += 1;
  }

  template <std::size_t I> void on_reentry(test_event<I> const &) noexcept {
    event_reentry_count[I] += 1;
  }
};
struct external_state {};

struct fake_machine {
  using states_list_type = states_list<test_state, external_state>;
  void start() noexcept {}
  void stop() noexcept {}
  bool on(concepts::event auto const &) { return true; }
  template <concepts::state... State> bool is() const noexcept { return false; }
};
} // namespace

SCENARIO("single state container is a state container",
         "[unit][state-container][single-state-container]") {
  using target_type = single_state_container<test_state>;
  THEN("single state container is a state container") {
    REQUIRE(has_states_list_type<target_type>::value);
    REQUIRE(
        is_state_container_details_::has_on_initial_entry<target_type>::value);
    REQUIRE(is_state_container_details_::has_on_final_exit<target_type>::value);
    REQUIRE(is_state_container_details_::has_on_exit<target_type>::value);
    REQUIRE(is_state_container_details_::has_on_entry<target_type>::value);
    REQUIRE(is_state_container_details_::has_on_reentry<target_type>::value);
    REQUIRE(is_state_container_details_::has_schedule_acceptable_transitions<
            target_type>::value);
    REQUIRE(is_state_container<target_type>::value);
    REQUIRE(concepts::state_container<target_type>);
  }
}

SCENARIO("single state container event handling",
         "[unit][state-container][single-state-container]") {
  using target_type = single_state_container<test_state>;
  fake_machine machine;

  GIVEN("a single state container") {
    target_type target;
    WHEN("initially entered") {
      target.on_initial_entry(machine);

      THEN("the entry callback was triggered") {
        test_state const &current_state = target.current_state<test_state>();
        REQUIRE(1 == current_state.initial_entry_count);
      }

      AND_WHEN("reentered") {
        target.on_reentry<test_state>(machine, test_event<0>{});

        THEN("the reentry callback was triggered") {
          test_state const &current_state = target.current_state<test_state>();
          REQUIRE(1 == current_state.event_reentry_count[0]);
          REQUIRE(0 == current_state.event_reentry_count[1]);
        }

        AND_WHEN("reentered") {
          target.on_reentry<test_state>(machine, test_event<1>{});

          THEN("the reentry callback was triggered") {
            test_state const &current_state =
                target.current_state<test_state>();
            REQUIRE(1 == current_state.event_reentry_count[0]);
            REQUIRE(1 == current_state.event_reentry_count[1]);
          }
        }
      }
      AND_WHEN("exited") {
        target.on_exit<test_state, external_state>(machine, test_event<0>{});

        THEN("the exit callback was triggered") {
          test_state const &current_state = target.current_state<test_state>();
          REQUIRE(1 == current_state.event_exit_count[0]);
          REQUIRE(0 == current_state.event_exit_count[1]);
        }

        AND_WHEN("entered") {
          target.on_entry<external_state, test_state>(machine, test_event<1>{});

          THEN("the entry callback was triggered") {
            test_state const &current_state =
                target.current_state<test_state>();
            REQUIRE(0 == current_state.event_entry_count[0]);
            REQUIRE(1 == current_state.event_entry_count[1]);
          }
        }
      }
      AND_WHEN("finally exited") {
        target.on_final_exit(machine);

        THEN("the exit callback was triggered") {
          test_state const &current_state = target.current_state<test_state>();
          REQUIRE(1 == current_state.final_exit_count);
        }
      }
    }
  }
}

SCENARIO("single state container state examining",
         "[unit][state-container][single-state-container]") {
  using target_type = single_state_container<test_state>;
  GIVEN("a type held by the container") {
    using type = test_state;
    WHEN("seeing if the container holds that type") {
      bool const actual = contains_v<states_list_t<target_type>, type>;

      THEN("it's held") { REQUIRE(actual); }
    }

    AND_GIVEN("an instance of the container") {
      target_type target;

      WHEN("seeing if the container is that type") {
        bool const actual = target.is<type>();

        THEN("it's held") { REQUIRE(actual); }
      }
    }
  }

  GIVEN("a type not held by the container") {
    using type = external_state;
    WHEN("seeing if the container holds that type") {
      bool const actual = contains_v<states_list_t<target_type>, type>;

      THEN("it's not held") { REQUIRE_FALSE(actual); }
    }

    AND_GIVEN("an instance of the container") {
      target_type target;

      WHEN("seeing if the container is that type") {
        bool const actual = target.is<type>();

        THEN("it's not held") { REQUIRE_FALSE(actual); }
      }
    }
  }
}
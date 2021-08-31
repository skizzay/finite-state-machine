#include <catch.hpp>
#include <skizzay/fsm/flat_state_container.h>
#include <skizzay/fsm/single_state_container.h>

using namespace skizzay::fsm;

namespace {
template <std::size_t> struct test_event {};
template <std::size_t> struct test_state {};

struct fake_machine {
  using states_list_type =
      states_list<test_state<0>, test_state<1>, test_state<2>>;
  void start() noexcept {}
  void stop() noexcept {}
  bool on(concepts::event auto const &) { return true; }
  template <concepts::state... State> bool is() const noexcept { return false; }
};
} // namespace

SCENARIO("flat state container is a state container",
         "[unit][state-container][flat-state-container]") {
  using target_type =
      flat_state<single_state_container<test_state<0>>,
                           single_state_container<test_state<1>>>;
  THEN("flat state container is a state container") {
    REQUIRE(is_state_container<target_type>::value);
    REQUIRE(concepts::state_container<target_type>);
  }
}

SCENARIO("flat state container event handling",
         "[unit][state-container][flat-state-container]") {
  using target_type =
      flat_state<single_state_container<test_state<0>>,
                           single_state_container<test_state<1>>>;
  fake_machine machine;

  GIVEN("a flat state container") {
    target_type target;
    WHEN("initially entered") {
      target.on_initial_entry(machine);

      AND_WHEN("reentered") {
        target.on_reentry<test_state<0>>(machine, test_event<0>{});
      }
      AND_WHEN("exited") {
        target.on_exit<test_state<0>, test_state<2>>(machine, test_event<0>{});
        AND_WHEN("entered") {
          target.on_entry<test_state<0>, test_state<1>>(machine,
                                                        test_event<1>{});
        }
      }
      AND_WHEN("finally exited") { target.on_final_exit(machine); }
    }
  }
}

SCENARIO("flat state container state examining",
         "[unit][state-container][flat-state-container]") {
  using target_type =
      flat_state<single_state_container<test_state<0>>,
                           single_state_container<test_state<1>>>;
  fake_machine machine;

  GIVEN("a type held by the container") {
    using type = test_state<0>;
    WHEN("seeing if the container holds that type") {
      bool const actual = contains_v<states_list_t<target_type>, type>;

      THEN("it's held") { REQUIRE(actual); }
    }

    AND_GIVEN("an instance of the container") {
      target_type target;

      WHEN("initially entered") {
        target.on_initial_entry(machine);

        AND_WHEN("seeing if the container is that type") {
          bool const actual = target.is<type>();

          THEN("it's held") { REQUIRE(actual); }
        }
      }

      WHEN("seeing if the container is that type") {
        bool const actual = target.is<type>();

        THEN("it's not held") { REQUIRE_FALSE(actual); }
      }
    }
  }

  GIVEN("a type not held by the container") {
    using type = test_state<2>;
    WHEN("seeing if the container holds that type") {
      bool const actual = contains_v<states_list_t<target_type>, type>;

      THEN("it's not held") { REQUIRE_FALSE(actual); }
    }
  }
}
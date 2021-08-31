#include "skizzay/fsm/event.h"
#include "skizzay/fsm/exit.h"
#include <catch.hpp>

using skizzay::fsm::epsilon_event;

namespace {

struct simple_state {
  std::size_t exit_count = 0;
};

struct adl_state {
  std::size_t exit_count = 0;
};

void on_exit(auto const &, adl_state &s) noexcept { ++s.exit_count; }

struct member_function_state {
  std::size_t exit_count = 0;

  void on_exit(auto const &) noexcept { ++exit_count; }
};

struct child_state {
  std::size_t exit_count = 0;
};

void on_exit(child_state &child, auto const &, simple_state &parent) {
  ++child.exit_count;
  ++parent.exit_count;
}

} // namespace

SCENARIO("exit callbacks", "[unit][state]") {
  GIVEN("a state without any callbacks") {
    simple_state target;

    WHEN("exited") {
      skizzay::fsm::exit(epsilon_event, target);
      THEN("exit callback was not fired") { REQUIRE(0 == target.exit_count); }
    }
  }

  GIVEN("a state with ADL callbacks") {
    adl_state target;

    WHEN("exited") {
      skizzay::fsm::exit(epsilon_event, target);
      THEN("exit callback was fired") { REQUIRE(1 == target.exit_count); }
    }
  }

  GIVEN("a state with member function callbacks") {
    member_function_state target;

    WHEN("exited") {
      skizzay::fsm::exit(epsilon_event, target);
      THEN("exit callback was fired") { REQUIRE(1 == target.exit_count); }
    }
  }

  GIVEN("a state with parental callbacks") {
    simple_state parent;
    child_state child;

    AND_GIVEN("a grandparent") {
      member_function_state grandparent;

      WHEN("exited") {
        auto target = std::tie(child, parent, grandparent);
        skizzay::fsm::exit(epsilon_event, target);
        THEN("child exit callback was fired") {
          REQUIRE(1 == child.exit_count);
        }
        THEN("parent exit callback was fired") {
          REQUIRE(1 == parent.exit_count);
        }
        THEN("grandparent exit callback was not fired") {
          REQUIRE(0 == grandparent.exit_count);
        }
      }
    }

    WHEN("exited") {
      auto target = std::tie(child, parent);
      skizzay::fsm::exit(epsilon_event, target);
      THEN("child exit callback was fired") { REQUIRE(1 == child.exit_count); }
      THEN("parent exit callback was fired") {
        REQUIRE(1 == parent.exit_count);
      }
    }
  }
}
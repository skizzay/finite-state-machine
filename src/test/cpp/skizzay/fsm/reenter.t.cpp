#include "skizzay/fsm/event.h"
#include <catch.hpp>
#include <skizzay/fsm/reenter.h>

using skizzay::fsm::epsilon_event;

namespace {

struct simple_state {
  std::size_t reentry_count = 0;
};

struct adl_state {
  std::size_t reentry_count = 0;
};

void on_reentry(auto const &, adl_state &s) noexcept { ++s.reentry_count; }

struct member_function_state {
  std::size_t reentry_count = 0;

  void on_reentery(auto const &) noexcept { ++reentry_count; }
};

struct child_state {
  std::size_t reentry_count = 0;

  void on_reentry(simple_state &parent, member_function_state &grandparent) {
    ++reentry_count;
    ++parent.reentry_count;
    ++grandparent.reentry_count;
  }
};

} // namespace

SCENARIO("reenter callbacks", "[unit][state]") {
  GIVEN("a state without any callbacks") {
    simple_state target;

    WHEN("reentered") {
      skizzay::fsm::reenter(epsilon_event, target);
      THEN("reentry callback was not fired") {
        REQUIRE(0 == target.reentry_count);
      }
    }
  }

  GIVEN("a state with ADL callbacks") {
    adl_state target;

    WHEN("reentered") {
      skizzay::fsm::reenter(epsilon_event, target);
      THEN("reentry callback was fired") { REQUIRE(1 == target.reentry_count); }
    }
  }

  GIVEN("a state with member function callbacks") {
    member_function_state target;

    WHEN("reentered") {
      skizzay::fsm::reenter(epsilon_event, target);
      THEN("reentry callback was fired") { REQUIRE(1 == target.reentry_count); }
    }
  }

  GIVEN("a state with parental callbacks") {
    simple_state parent;
    child_state child;

    AND_GIVEN("a grandparent") {
      member_function_state grandparent;

      WHEN("reentered") {
        auto target = std::tie(child, parent, grandparent);
        skizzay::fsm::reenter(epsilon_event, target);
        THEN("child reentry callback was fired") {
          REQUIRE(1 == child.reentry_count);
        }
        THEN("parent reentry callback was fired") {
          REQUIRE(1 == parent.reentry_count);
        }
        THEN("grandparent reentry callback was fired") {
          REQUIRE(1 == grandparent.reentry_count);
        }
      }
    }

    WHEN("reentered") {
      auto target = std::tie(child, parent);
      skizzay::fsm::reenter(epsilon_event, target);
      THEN("child reentry callback was fired") {
        REQUIRE(1 == child.reentry_count);
      }
      THEN("parent reentry callback was fired") {
        REQUIRE(1 == parent.reentry_count);
      }
    }
  }
}
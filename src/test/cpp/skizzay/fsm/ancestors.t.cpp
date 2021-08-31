#include <catch.hpp>
#include <skizzay/fsm/ancestors.h>
#include <skizzay/fsm/concepts.h>
#include <skizzay/fsm/traits.h>

using namespace skizzay::fsm;

namespace {
struct red {};
struct purple {};
struct blue {};
} // namespace

TEST_CASE("ancestors without machine", "[unit][ancestry]") {
  SECTION("no states") {
    using target_type = ancestors<>;
    REQUIRE(is_ancestry<target_type>::value);
    REQUIRE(concepts::ancestry<target_type>);
    REQUIRE_FALSE(is_child_ancestry<target_type>::value);
    REQUIRE_FALSE(concepts::child_ancestry<target_type>);
    REQUIRE_FALSE(is_parent_ancestry<target_type>::value);
    REQUIRE_FALSE(concepts::parent_ancestry<target_type>);
    REQUIRE_FALSE(is_full_ancestry<target_type>::value);
    REQUIRE_FALSE(concepts::full_ancestry<target_type>);
    REQUIRE_FALSE(is_machine_ancestry<target_type>::value);
    REQUIRE_FALSE(concepts::machine_ancestry<target_type>);

    SECTION("first generation") {
      using child_target_type =
          decltype(std::declval<target_type>().with_new_generation(
              std::declval<red &>()));
      REQUIRE(is_child_ancestry<child_target_type>::value);
      REQUIRE(concepts::child_ancestry<child_target_type>);
      REQUIRE_FALSE(is_parent_ancestry<child_target_type>::value);
      REQUIRE_FALSE(concepts::parent_ancestry<child_target_type>);
      REQUIRE_FALSE(is_full_ancestry<child_target_type>::value);
      REQUIRE_FALSE(concepts::full_ancestry<child_target_type>);
      REQUIRE_FALSE(is_machine_ancestry<child_target_type>::value);
      REQUIRE_FALSE(concepts::machine_ancestry<child_target_type>);

      SECTION("second generation") {
        using grandchild_target_type =
            decltype(std::declval<child_target_type>().with_new_generation(
                std::declval<purple &>()));
        REQUIRE(is_child_ancestry<grandchild_target_type>::value);
        REQUIRE(concepts::child_ancestry<grandchild_target_type>);
        REQUIRE(is_parent_ancestry<grandchild_target_type>::value);
        REQUIRE(concepts::parent_ancestry<grandchild_target_type>);
        REQUIRE(is_full_ancestry<grandchild_target_type>::value);
        REQUIRE(concepts::full_ancestry<grandchild_target_type>);
        REQUIRE_FALSE(is_machine_ancestry<grandchild_target_type>::value);
        REQUIRE_FALSE(concepts::machine_ancestry<grandchild_target_type>);
      }
    }
  }
}
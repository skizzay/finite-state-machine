#include <catch.hpp>
#include <skizzay/fsm/traits.h>

using namespace skizzay::fsm;

namespace {
struct undestructible {
  ~undestructible() = delete;
};
} // namespace

TEST_CASE("dummy types", "[unit][traits]") {
  SECTION("event") { REQUIRE(is_event<details_::dummy_event>::value); }
  SECTION("state") {
    REQUIRE(is_state<details_::dummy_state>::value);
    REQUIRE_FALSE(is_state<undestructible>::value);
  }
  SECTION("transition") {
    REQUIRE(is_transition<details_::dummy_transition>::value);
    REQUIRE_FALSE(is_actionable_transition<details_::dummy_transition>::value);
    REQUIRE(is_transition<details_::dummy_action_transition>::value);
    REQUIRE(is_actionable_transition<details_::dummy_action_transition>::value);
  }
  SECTION("machine") {
    REQUIRE(is_machine<details_::dummy_machine<
                states_list<details_::dummy_state>,
                events_list<details_::dummy_event>>>::value);
  }
  SECTION("ancestry") {
    REQUIRE(is_ancestry<details_::dummy_ancestry>::value);
    REQUIRE(is_ancestry<details_::dummy_parent_ancestry>::value);
    REQUIRE(is_parent_ancestry<details_::dummy_parent_ancestry>::value);
    REQUIRE(is_ancestry<details_::dummy_child_ancestry>::value);
    REQUIRE(is_child_ancestry<details_::dummy_child_ancestry>::value);
    REQUIRE(is_ancestry<details_::dummy_full_ancestry>::value);
    REQUIRE(is_child_ancestry<details_::dummy_full_ancestry>::value);
    REQUIRE(is_parent_ancestry<details_::dummy_full_ancestry>::value);
    REQUIRE(is_full_ancestry<details_::dummy_full_ancestry>::value);
    REQUIRE(is_ancestry<details_::dummy_machine_ancestry>::value);
    REQUIRE(is_machine_ancestry<details_::dummy_machine_ancestry>::value);
    SECTION("dummy child ancestor") {
      using target_type = details_::dummy_child_ancestor<details_::dummy_state>;
      REQUIRE(is_ancestry<target_type>::value);
      REQUIRE(is_child_ancestry<target_type>::value);
    }
  }
  SECTION("transition coordinator") {
    REQUIRE(is_transition_coordinator<
            details_::dummy_transition_coordinator>::value);
  }
  SECTION("state container") {
    REQUIRE(has_states_list_type<details_::dummy_state_container>::value);
    REQUIRE(is_state_container_details_::has_on_initial_entry<
            details_::dummy_state_container>::value);
    REQUIRE(is_state_container_details_::has_on_final_exit<
            details_::dummy_state_container>::value);
    REQUIRE(is_state_container_details_::has_on_exit<
            details_::dummy_state_container>::value);
    REQUIRE(is_state_container_details_::has_on_entry<
            details_::dummy_state_container>::value);
    REQUIRE(is_state_container_details_::has_on_reentry<
            details_::dummy_state_container>::value);
    REQUIRE(is_state_container_details_::has_schedule_acceptable_transitions<
            details_::dummy_state_container>::value);
    REQUIRE(is_state_container<details_::dummy_state_container>::value);
  }
}
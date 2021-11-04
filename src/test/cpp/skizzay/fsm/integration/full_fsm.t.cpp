#include "../test_objects.h"
#include <catch.hpp>
#include <iostream>
#include <skizzay/fsm/concepts.h>
#include <skizzay/fsm/event.h>
#include <skizzay/fsm/finite_state_machine.h>
#include <skizzay/fsm/flat_state_container.h>
#include <skizzay/fsm/hierarchical_state_container.h>
#include <skizzay/fsm/orthogonal_state_container.h>
#include <skizzay/fsm/simple_transition.h>
#include <skizzay/fsm/single_state_container.h>

using namespace skizzay::fsm;

namespace {
constexpr std::size_t event_count = 12;
template <std::size_t Id> using target_test_state = test_state<Id, event_count>;

struct initial_state : target_test_state<0> {
  template <concepts::machine Machine>
  void on_entry(Machine &machine, initial_entry_event_t const) noexcept {
    this->target_test_state<0>::on_entry(initial_entry_event);
    machine.on(epsilon_event);
  }
};

} // namespace

SCENARIO("Full finite state machine without guards", "[integration]") {
  GIVEN("a transition table") {
    std::tuple transition_table{
        simple_transition<initial_state, target_test_state<1>,
                          epsilon_event_t>{},
        simple_transition<target_test_state<1>, target_test_state<1>,
                          test_event<0>>{},
        simple_transition<target_test_state<1>, target_test_state<2>,
                          test_event<1>>{},
        simple_transition<target_test_state<20>, target_test_state<21>,
                          test_event<2>>{},
        simple_transition<target_test_state<21>, target_test_state<20>,
                          test_event<3>>{},
        simple_transition<target_test_state<2>, target_test_state<5>,
                          test_event<4>>{},
        simple_transition<target_test_state<30>, target_test_state<31>,
                          test_event<5>>{},
        simple_transition<target_test_state<31>, target_test_state<1>,
                          test_event<6>>{},
        simple_transition<target_test_state<1>, target_test_state<41>,
                          test_event<7>>{},
        simple_transition<target_test_state<41>, target_test_state<40>,
                          test_event<8>>{},
        simple_transition<target_test_state<4>, target_test_state<4>,
                          test_event<9>>{},
        simple_transition<target_test_state<40>, target_test_state<4>,
                          test_event<10>>{},
        simple_transition<target_test_state<40>, target_test_state<5>,
                          test_event<11>>{}};

    AND_GIVEN("a root state container") {
      flat_states root{
          single_state{initial_state{}}, single_state{target_test_state<1>{}},
          orthogonal_states{
              hierarchical_states{
                  target_test_state<2>{},
                  flat_states{single_state{target_test_state<20>{}},
                              single_state{target_test_state<21>{}}}},
              hierarchical_states{
                  target_test_state<3>{},
                  flat_states{single_state{target_test_state<30>{}},
                              single_state{target_test_state<31>{}}}},
              hierarchical_states{
                  target_test_state<4>{},
                  flat_states{single_state{target_test_state<40>{}},
                              single_state{target_test_state<41>{}}}},
          },
          single_state{target_test_state<5>{}}};

      AND_GIVEN("a machine") {
        finite_state_machine target{std::move(transition_table),
                                    std::move(root)};

        THEN("it is stopped") {
          REQUIRE(target.is_stopped());
          REQUIRE_FALSE(target.is_running());
        }

        WHEN("started") {
          target.start();

          THEN("it is running") {
            REQUIRE_FALSE(target.is_stopped());
            REQUIRE(target.is_running());
          }

          THEN("the current state is 1") {
            REQUIRE(target.is<target_test_state<1>>());

            AND_THEN("s1 was entered via epsilon") {
              target_test_state<1> const &s1 =
                  target.current_state<target_test_state<1>>();

              REQUIRE(1 == s1.epsilon_event_entry_count);
            }
          }

          AND_WHEN("e0 is raised") {
            target.on(test_event<0>{});

            THEN("the current state is 1") {
              REQUIRE(target.is<target_test_state<1>>());

              AND_THEN("s1 was reentered for e0") {
                target_test_state<1> const &s1 =
                    target.current_state<target_test_state<1>>();

                REQUIRE(1 == s1.event_reentry_count.at(0));
              }
            }
          }

          AND_WHEN("e1 is raised") {
            target.on(test_event<1>{});

            THEN("the current state is 2") {
              REQUIRE(target.is<target_test_state<2>>());

              AND_THEN("s2 was entered for e1") {
                target_test_state<2> const &s2 =
                    target.current_state<target_test_state<2>>();

                REQUIRE(1 == s2.event_entry_count.at(1));
              }

              AND_THEN("the current state is 20") {
                REQUIRE(target.is<target_test_state<20>>());

                AND_THEN("s20 was initially entered") {
                  target_test_state<20> const &s20 =
                      target.current_state<target_test_state<20>>();

                  REQUIRE(1 == s20.initial_entry_count);
                }
              }
            }

            THEN("the current state is 3") {
              REQUIRE(target.is<target_test_state<3>>());

              AND_THEN("s2 was initially entered") {
                target_test_state<3> const &s3 =
                    target.current_state<target_test_state<3>>();

                REQUIRE(1 == s3.initial_entry_count);
              }

              AND_THEN("the current state is 30") {
                REQUIRE(target.is<target_test_state<30>>());

                AND_THEN("s30 was initially entered") {
                  target_test_state<30> const &s30 =
                      target.current_state<target_test_state<30>>();

                  REQUIRE(1 == s30.initial_entry_count);
                }
              }
            }

            THEN("the current state is 4") {
              REQUIRE(target.is<target_test_state<4>>());

              AND_THEN("s2 was initially entered") {
                target_test_state<4> const &s4 =
                    target.current_state<target_test_state<4>>();

                REQUIRE(1 == s4.initial_entry_count);
              }

              AND_THEN("the current state is 40") {
                REQUIRE(target.is<target_test_state<40>>());

                AND_THEN("s40 was initially entered") {
                  target_test_state<40> const &s40 =
                      target.current_state<target_test_state<40>>();

                  REQUIRE(1 == s40.initial_entry_count);
                }
              }
            }

            AND_WHEN("e2 is raised") {
              target_test_state<20> const &s20 =
                  target.current_state<target_test_state<20>>();
              target.on(test_event<2>{});

              THEN("the current state is 2") {
                REQUIRE(target.is<target_test_state<2>>());

                AND_THEN("s2 was not affected by e2") {
                  target_test_state<2> const &s2 =
                      target.current_state<target_test_state<2>>();

                  REQUIRE(0 == s2.event_entry_count.at(2));
                  REQUIRE(0 == s2.event_reentry_count.at(2));
                  REQUIRE(0 == s2.event_exit_count.at(2));
                }

                AND_THEN("the current state is not 20") {
                  REQUIRE_FALSE(target.is<target_test_state<20>>());

                  AND_THEN("s20 exited for e2") {
                    REQUIRE(1 == s20.event_exit_count.at(2));
                  }
                }

                AND_THEN("the current state is 21") {
                  REQUIRE(target.is<target_test_state<21>>());

                  AND_THEN("s21 was entered for e2") {
                    target_test_state<21> const &s21 =
                        target.current_state<target_test_state<21>>();

                    REQUIRE(1 == s21.event_entry_count.at(2));
                  }
                }
              }

              AND_WHEN("e3 is raised") {
                target_test_state<21> const &s21 =
                    target.current_state<target_test_state<21>>();
                target.on(test_event<3>{});

                THEN("the current state is 2") {
                  REQUIRE(target.is<target_test_state<2>>());

                  AND_THEN("s2 was not affected by e3") {
                    target_test_state<2> const &s2 =
                        target.current_state<target_test_state<2>>();

                    REQUIRE(0 == s2.event_entry_count.at(3));
                    REQUIRE(0 == s2.event_reentry_count.at(3));
                    REQUIRE(0 == s2.event_exit_count.at(3));
                  }

                  AND_THEN("the current state is not 21") {
                    REQUIRE_FALSE(target.is<target_test_state<21>>());

                    AND_THEN("s21 exited for e3") {
                      REQUIRE(1 == s21.event_exit_count.at(3));
                    }
                  }

                  AND_THEN("the current state is 20") {
                    REQUIRE(target.is<target_test_state<20>>());

                    AND_THEN("s20 was entered for e3") {
                      target_test_state<20> const &s20 =
                          target.current_state<target_test_state<20>>();

                      REQUIRE(1 == s20.event_entry_count.at(3));
                    }
                  }
                }
              }
            }

            AND_WHEN("e4 is raised") {
              target_test_state<2> const &s2 =
                  target.current_state<target_test_state<2>>();
              target_test_state<20> const &s20 =
                  target.current_state<target_test_state<20>>();
              target_test_state<3> const &s3 =
                  target.current_state<target_test_state<3>>();
              target_test_state<30> const &s30 =
                  target.current_state<target_test_state<30>>();
              target_test_state<4> const &s4 =
                  target.current_state<target_test_state<4>>();
              target_test_state<40> const &s40 =
                  target.current_state<target_test_state<40>>();
              target.on(test_event<4>{});

              THEN("the current state is 5") {
                REQUIRE(target.is<target_test_state<5>>());

                AND_THEN("s5 was entered for e4") {
                  target_test_state<5> const &s5 =
                      target.current_state<target_test_state<5>>();

                  REQUIRE(1 == s5.event_entry_count.at(4));
                }

                AND_THEN("s2 was exited for e4") {
                  REQUIRE(1 == s2.event_exit_count.at(4));
                }

                AND_THEN("s3 final exited for e4") {
                  REQUIRE(0 == s3.event_exit_count.at(4));
                  REQUIRE(1 == s3.final_exit_count);

                  AND_THEN("s30 final exited for e4") {
                    REQUIRE(0 == s30.event_exit_count.at(4));
                    REQUIRE(1 == s30.final_exit_count);
                  }
                }

                AND_THEN("s4 final exited for e4") {
                  REQUIRE(0 == s4.event_exit_count.at(4));
                  REQUIRE(1 == s4.final_exit_count);

                  AND_THEN("s40 final exited for e4") {
                    REQUIRE(0 == s40.event_exit_count.at(4));
                    REQUIRE(1 == s40.final_exit_count);
                  }
                }

                AND_THEN("the current state is not 20") {
                  REQUIRE_FALSE(target.is<target_test_state<20>>());

                  AND_THEN("s20 final exited for e4") {
                    REQUIRE(0 == s20.event_exit_count.at(4));
                    REQUIRE(1 == s20.final_exit_count);
                  }
                }

                AND_THEN("the current state is not 21") {
                  REQUIRE_FALSE(target.is<target_test_state<21>>());
                }
              }
            }

            AND_WHEN("e5 is raised") {
              target_test_state<30> const &s30 =
                  target.current_state<target_test_state<30>>();
              target.on(test_event<5>{});

              THEN("the current state is 3") {
                REQUIRE(target.is<target_test_state<3>>());

                AND_THEN("s3 was not affected by e5") {
                  target_test_state<3> const &s3 =
                      target.current_state<target_test_state<3>>();

                  REQUIRE(0 == s3.event_entry_count.at(5));
                  REQUIRE(0 == s3.event_reentry_count.at(5));
                  REQUIRE(0 == s3.event_exit_count.at(5));
                }

                AND_THEN("the current state is not 30") {
                  REQUIRE_FALSE(target.is<target_test_state<30>>());

                  AND_THEN("s30 exited for e5") {
                    REQUIRE(1 == s30.event_exit_count.at(5));
                  }
                }

                AND_THEN("the current state is 31") {
                  REQUIRE(target.is<target_test_state<31>>());

                  AND_THEN("s31 was entered for e5") {
                    target_test_state<31> const &s31 =
                        target.current_state<target_test_state<31>>();

                    REQUIRE(1 == s31.event_entry_count.at(5));
                  }
                }
              }

              AND_WHEN("e6 is raised") {
                target_test_state<2> const &s2 =
                    target.current_state<target_test_state<2>>();
                target_test_state<20> const &s20 =
                    target.current_state<target_test_state<20>>();
                target_test_state<3> const &s3 =
                    target.current_state<target_test_state<3>>();
                target_test_state<31> const &s31 =
                    target.current_state<target_test_state<31>>();
                target_test_state<4> const &s4 =
                    target.current_state<target_test_state<4>>();
                target_test_state<40> const &s40 =
                    target.current_state<target_test_state<40>>();
                target.on(test_event<6>{});

                THEN("the current state is 1") {
                  REQUIRE(target.is<target_test_state<1>>());

                  AND_THEN("s1 was entered for e6") {
                    target_test_state<1> const &s1 =
                        target.current_state<target_test_state<1>>();

                    REQUIRE(1 == s1.event_entry_count.at(6));
                  }

                  AND_THEN("s2 final exited for e6") {
                    REQUIRE_FALSE(target.is<target_test_state<2>>());
                    REQUIRE(0 == s2.event_exit_count.at(6));
                    REQUIRE(1 == s2.final_exit_count);

                    AND_THEN("s20 final exited for e6") {
                      REQUIRE_FALSE(target.is<target_test_state<20>>());
                      REQUIRE(0 == s20.event_exit_count.at(6));
                      REQUIRE(1 == s20.final_exit_count);
                    }
                  }

                  AND_THEN("s3 final exited for e6") {
                    REQUIRE_FALSE(target.is<target_test_state<3>>());
                    REQUIRE(0 == s3.event_exit_count.at(6));
                    REQUIRE(1 == s3.final_exit_count);

                    AND_THEN("s31 was exited for e6") {
                      REQUIRE_FALSE(target.is<target_test_state<30>>());
                      REQUIRE(1 == s31.event_exit_count.at(6));
                      REQUIRE(0 == s31.final_exit_count);
                    }
                  }

                  AND_THEN("s4 final exited for e6") {
                    REQUIRE_FALSE(target.is<target_test_state<4>>());
                    REQUIRE(0 == s4.event_exit_count.at(6));
                    REQUIRE(1 == s4.final_exit_count);

                    AND_THEN("s40 final exited for e6") {
                      REQUIRE_FALSE(target.is<target_test_state<40>>());
                      REQUIRE(0 == s40.event_exit_count.at(6));
                      REQUIRE(1 == s40.final_exit_count);
                    }
                  }
                }
              }
            }
          }

          AND_WHEN("e7 is raised") {
            target_test_state<1> const &s1 =
                target.current_state<target_test_state<1>>();
            target.on(test_event<7>{});

            THEN("s1 exited for e7") {
              REQUIRE(0 == s1.final_exit_count);
              REQUIRE(1 == s1.event_exit_count.at(7));
              AND_THEN("s1 is not active") {
                REQUIRE_FALSE(target.is<target_test_state<1>>());
              }
            }

            THEN("s2 was initially entered") {
              target_test_state<2> const &s2 =
                  target.current_state<target_test_state<2>>();

              REQUIRE(1 == s2.initial_entry_count);
              REQUIRE(0 == s2.event_entry_count.at(7));

              AND_THEN("s2 is active") {
                REQUIRE(target.is<target_test_state<2>>());
              }

              AND_THEN("s20 was initially entered") {
                target_test_state<20> const &s20 =
                    target.current_state<target_test_state<20>>();

                REQUIRE(1 == s20.initial_entry_count);
                REQUIRE(0 == s20.event_entry_count.at(7));

                AND_THEN("s20 is active") {
                  REQUIRE(target.is<target_test_state<20>>());
                }
              }
            }

            THEN("s3 was initially entered") {
              target_test_state<3> const &s3 =
                  target.current_state<target_test_state<3>>();

              REQUIRE(1 == s3.initial_entry_count);
              REQUIRE(0 == s3.event_entry_count.at(7));

              AND_THEN("s3 is active") {
                REQUIRE(target.is<target_test_state<3>>());
              }

              AND_THEN("s30 was initially entered") {
                target_test_state<30> const &s30 =
                    target.current_state<target_test_state<30>>();

                REQUIRE(1 == s30.initial_entry_count);
                REQUIRE(0 == s30.event_entry_count.at(7));

                AND_THEN("s30 is active") {
                  REQUIRE(target.is<target_test_state<30>>());
                }
              }
            }

            THEN("s4 was initially entered") {
              target_test_state<4> const &s4 =
                  target.current_state<target_test_state<4>>();

              REQUIRE(1 == s4.initial_entry_count);
              REQUIRE(0 == s4.event_entry_count.at(7));

              AND_THEN("s4 is active") {
                REQUIRE(target.is<target_test_state<4>>());
              }

              AND_THEN("s41 was entered for e7") {
                target_test_state<41> const &s41 =
                    target.current_state<target_test_state<41>>();

                REQUIRE(0 == s41.initial_entry_count);
                REQUIRE(1 == s41.event_entry_count.at(7));

                AND_THEN("s41 is active") {
                  REQUIRE(target.is<target_test_state<41>>());
                }
              }
            }

            AND_WHEN("e8 is raised") {
              target_test_state<4> const &s4 =
                  target.current_state<target_test_state<4>>();
              target_test_state<41> const &s41 =
                  target.current_state<target_test_state<41>>();
              target.on(test_event<8>{});

              THEN("s2 is active") {
                REQUIRE(target.is<target_test_state<2>>());

                AND_THEN("s20 is active") {
                  REQUIRE(target.is<target_test_state<20>>());
                }
              }

              THEN("s3 is active") {
                REQUIRE(target.is<target_test_state<3>>());

                AND_THEN("s30 is active") {
                  REQUIRE(target.is<target_test_state<30>>());
                }
              }

              THEN("s4 was unaffected by e8") {
                REQUIRE(1 == s4.initial_entry_count);
                REQUIRE(0 == s4.event_entry_count.at(8));

                AND_THEN("s4 is active") {
                  REQUIRE(target.is<target_test_state<4>>());
                }

                AND_THEN("s41 was exited for e8") {
                  REQUIRE(0 == s41.final_exit_count);
                  REQUIRE(1 == s41.event_exit_count.at(8));

                  AND_THEN("s41 is not active") {
                    REQUIRE_FALSE(target.is<target_test_state<41>>());
                  }
                }

                AND_THEN("s40 was entered for e8") {
                  target_test_state<40> const &s40 =
                      target.current_state<target_test_state<40>>();

                  REQUIRE(0 == s40.initial_entry_count);
                  REQUIRE(1 == s40.event_entry_count.at(8));

                  AND_THEN("s40 is active") {
                    REQUIRE(target.is<target_test_state<40>>());
                  }
                }
              }

              AND_WHEN("e10 is raised") {
                target_test_state<40> const &s40 =
                    target.current_state<target_test_state<40>>();
                target.on(test_event<10>{});

                THEN("s2 is active") {
                  REQUIRE(target.is<target_test_state<2>>());

                  AND_THEN("s20 is active") {
                    REQUIRE(target.is<target_test_state<20>>());
                  }
                }

                THEN("s3 is active") {
                  REQUIRE(target.is<target_test_state<3>>());

                  AND_THEN("s30 is active") {
                    REQUIRE(target.is<target_test_state<30>>());
                  }
                }

                THEN("s4 was reentered by e10") {
                  REQUIRE(1 == s4.initial_entry_count);
                  REQUIRE(0 == s4.event_entry_count.at(10));
                  REQUIRE(1 == s4.event_reentry_count.at(10));

                  AND_THEN("s4 is active") {
                    REQUIRE(target.is<target_test_state<4>>());
                  }

                  AND_THEN("s40 was exited for e8") {
                    REQUIRE(0 == s40.final_exit_count);
                    REQUIRE(1 == s40.event_exit_count.at(10));

                    AND_THEN("s40 was initially entered") {
                      REQUIRE(1 == s40.initial_entry_count);

                      AND_THEN("s40 is active") {
                        REQUIRE(target.is<target_test_state<40>>());
                      }
                    }
                  }
                }
              }

              AND_WHEN("e11 is raised") {
                target_test_state<2> const &s2 =
                    target.current_state<target_test_state<2>>();
                target_test_state<20> const &s20 =
                    target.current_state<target_test_state<20>>();
                target_test_state<3> const &s3 =
                    target.current_state<target_test_state<3>>();
                target_test_state<30> const &s30 =
                    target.current_state<target_test_state<30>>();
                target_test_state<4> const &s4 =
                    target.current_state<target_test_state<4>>();
                target_test_state<40> const &s40 =
                    target.current_state<target_test_state<40>>();
                target.on(test_event<11>{});

                THEN("s2 final exited") {
                  REQUIRE(1 == s2.final_exit_count);
                  REQUIRE(0 == s2.event_exit_count.at(11));

                  AND_THEN("s20 final exited") {
                    REQUIRE(1 == s20.final_exit_count);
                    REQUIRE(0 == s20.event_exit_count.at(11));

                    AND_THEN("s20 is not active") {
                      REQUIRE_FALSE(target.is<target_test_state<20>>());
                    }
                  }

                  AND_THEN("s2 is not active") {
                    REQUIRE_FALSE(target.is<target_test_state<2>>());
                  }
                }

                THEN("s3 final exited") {
                  REQUIRE(1 == s3.final_exit_count);
                  REQUIRE(0 == s3.event_exit_count.at(11));

                  AND_THEN("s30 final exited") {
                    REQUIRE(1 == s30.final_exit_count);
                    REQUIRE(0 == s30.event_exit_count.at(11));

                    AND_THEN("s30 is not active") {
                      REQUIRE_FALSE(target.is<target_test_state<30>>());
                    }
                  }

                  AND_THEN("s3 is not active") {
                    REQUIRE_FALSE(target.is<target_test_state<3>>());
                  }
                }

                THEN("s4 final exited") {
                  REQUIRE(1 == s4.final_exit_count);
                  REQUIRE(0 == s4.event_exit_count.at(11));

                  AND_THEN("s40 was exited from e11") {
                    REQUIRE(0 == s40.final_exit_count);
                    REQUIRE(1 == s40.event_exit_count.at(11));

                    AND_THEN("s40 is not active") {
                      REQUIRE_FALSE(target.is<target_test_state<40>>());
                    }
                  }

                  AND_THEN("s4 is not active") {
                    REQUIRE_FALSE(target.is<target_test_state<4>>());
                  }
                }

                THEN("s5 is active") {
                  REQUIRE(target.is<target_test_state<5>>());

                  AND_THEN("s5 was entered from e11") {
                    target_test_state<5> const &s5 =
                        target.current_state<target_test_state<5>>();
                    REQUIRE(0 == s5.initial_entry_count);
                    REQUIRE(1 == s5.event_entry_count.at(11));
                  }
                }
              }
            }

            AND_WHEN("e9 is raised") {
              target.on(test_event<9>{});

              THEN("s4 is active") {
                REQUIRE(target.is<target_test_state<4>>());

                AND_THEN("s40 is not active") {
                  REQUIRE_FALSE(target.is<target_test_state<40>>());
                }

                AND_THEN("s41 is active") {
                  REQUIRE(target.is<target_test_state<41>>());
                }
              }
            }
          }

          AND_WHEN("stopped") {
            target.stop();

            THEN("it is stopped") {
              REQUIRE(target.is_stopped());
              REQUIRE_FALSE(target.is_running());
            }
          }
        }
      }
    }
  }
}
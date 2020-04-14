#include "../SBM.h"
#include "build_testing_networks.h"
#include "catch.hpp"

TEST_CASE("Agglomerative merge step - Simple Bipartite", "[SBM]")
{
  auto my_sbm = simple_bipartite();

  const int n_initial_blocks = my_sbm.n_nodes_at_level(1);

  REQUIRE(n_initial_blocks == 6);

  // Run aglomerative merge with best single merge done
  const auto single_merge = agglomerative_merge(&my_sbm,
                                                1,    // block_level,
                                                1,    // n_merges_to_make,
                                                5,    // n_checks_per_block,
                                                0.1); // eps

  // Make sure that we now have one less block than before for each type
  const int new_block_num    = my_sbm.n_nodes_at_level(1);
  const int change_in_blocks = n_initial_blocks - new_block_num;
  REQUIRE(change_in_blocks == 1);

  // Make sure entropy has gone up as we would expect
  REQUIRE(single_merge.entropy_delta > 0);

  // Run again but this time merging the best 2
  auto new_sbm = simple_bipartite();

  const auto double_merge = agglomerative_merge(&new_sbm,
                                                1,    // block_level,
                                                2,    // n_merges_to_make,
                                                5,    // n_checks_per_block,
                                                0.1); // eps

  // Make sure that we now have two fewer blocks per type than before
  REQUIRE(2 == n_initial_blocks - new_sbm.n_nodes_at_level(1));

  // Entropy should up even more with two merges
  REQUIRE(single_merge.entropy_delta < double_merge.entropy_delta);
}

TEST_CASE("Agglomerative merge step - Simple Unipartite", "[SBM]")
{
  const int n_initial_blocks = 4;

  auto my_sbm = simple_unipartite();
  // We need to erase the original block stucture and give every node its own block to have enough blocks for this
  my_sbm.remove_block_levels_above(0);
  my_sbm.initialize_blocks(n_initial_blocks);

  // Run aglomerative merge with best single merge done
  const auto single_merge = agglomerative_merge(&my_sbm,
                                                1,    // block_level,
                                                1,    // n_merges_to_make,
                                                5,    // n_checks_per_block,
                                                0.1); // eps

  // Make sure that we now have one less block than before for each type
  const int new_block_num    = my_sbm.n_nodes_at_level(1);
  const int change_in_blocks = n_initial_blocks - new_block_num;
  REQUIRE(change_in_blocks == 1);

  // Make sure entropy has gone up as we would expect
  REQUIRE(single_merge.entropy_delta > 0);

  // Run again but this time merging the best 2
  auto new_sbm = simple_unipartite();
  new_sbm.remove_block_levels_above(0);
  new_sbm.initialize_blocks(n_initial_blocks);

  const auto double_merge = agglomerative_merge(&new_sbm,
                                                1,    // block_level,
                                                2,    // n_merges_to_make,
                                                5,    // n_checks_per_block,
                                                0.1); // eps

  // Make sure that we now have two fewer blocks per type than before
  REQUIRE(2 == n_initial_blocks - new_sbm.n_nodes_at_level(1));

  // Entropy should up even more with two merges
  REQUIRE(single_merge.entropy_delta < double_merge.entropy_delta);
}

TEST_CASE("Collapse Blocks (no MCMC) - Simple Bipartite", "[SBM]")
{
  auto my_sbm = simple_bipartite();

  auto collapse_to_2_res = my_sbm.collapse_blocks(0,     // node_level,
                                                  2,     // B_end,
                                                  5,     // n_checks_per_block,
                                                  0,     // n_mcmc_sweeps,
                                                  1.1,   // sigma,
                                                  0.01,  // eps,
                                                  true,  // report all steps,
                                                  true); // Allow exhaustive

  // Does the network have the requested number of blocks?
  REQUIRE(my_sbm.n_nodes_at_level(1) == 2);

  // Was entropy delta strictly positive?
  // Make sure entropy has gone up as we would expect
  REQUIRE(collapse_to_2_res.entropy_delta > 0);

  // Are our step reporting vectors the correct size?
  REQUIRE(collapse_to_2_res.merge_steps.size() > 1);
  REQUIRE(collapse_to_2_res.states.size() > 1);

  // Now do a collapse to 3 total groups
  auto collapse_to_3_res = my_sbm.collapse_blocks(0,     // node_level,
                                                  3,     // B_end,
                                                  5,     // n_checks_per_block,
                                                  0,     // n_mcmc_sweeps,
                                                  1.1,   // sigma,
                                                  0.01,  // eps,
                                                  true,  // report all steps,
                                                  true); // Allow exhaustive

  // Does the network have the requested number of blocks?
  REQUIRE(my_sbm.n_nodes_at_level(1) == 3);

  // Was entropy delta strictly positive?
  // Make sure entropy has gone up as we would expect
  REQUIRE(collapse_to_3_res.entropy_delta > 0);

  // Are our step reporting vectors the correct size?
  REQUIRE(collapse_to_3_res.merge_steps.size() > 1);
  REQUIRE(collapse_to_3_res.states.size() > 1);

  // Now do a collapse to 2 total groups but with one merge per step due to a sigma less than 1
  auto collapse_to_2_full_res = my_sbm.collapse_blocks(0,     // node_level,
                                                       2,     // B_end,
                                                       5,     // n_checks_per_block,
                                                       0,     // n_mcmc_sweeps,
                                                       0.8,   // sigma,
                                                       0.01,  // eps,
                                                       true,  // report all steps,
                                                       true); // Allow exhaustive

  // Does the network have the requested number of blocks?
  REQUIRE(my_sbm.n_nodes_at_level(1) == 2);

  // Was entropy delta strictly positive?
  // Make sure entropy has gone up as we would expect
  REQUIRE(collapse_to_2_full_res.entropy_delta > 0);

  // Are our step reporting vectors the correct size?
  REQUIRE(collapse_to_2_full_res.merge_steps.size() == 8 - 2);
  REQUIRE(collapse_to_2_full_res.states.size() == 8 - 2);

  // Can't collapse network to a single block because we have more than 1 type
  REQUIRE_THROWS(my_sbm.collapse_blocks(0,      // node_level,
                                        1,      // B_end,
                                        5,      // n_checks_per_block,
                                        0,      // n_mcmc_sweeps,
                                        0.8,    // sigma,
                                        0.01,   // eps,
                                        true,   // report all steps,
                                        true)); // Allow exhaustive)
}

TEST_CASE("Collapse Blocks (no MCMC) - Simple Unipartite", "[SBM]")
{
  auto my_sbm = simple_unipartite();

  auto collapse_to_2_res = my_sbm.collapse_blocks(0,     // node_level,
                                                  2,     // B_end,
                                                  5,     // n_checks_per_block,
                                                  0,     // n_mcmc_sweeps,
                                                  1.1,   // sigma,
                                                  0.01,  // eps,
                                                  true,  // report all steps,
                                                  true); // Allow exhaustive

  // Does the network have the requested number of blocks?
  REQUIRE(my_sbm.n_nodes_at_level(1) == 2);

  // Was entropy delta strictly positive?
  // Make sure entropy has gone up as we would expect
  REQUIRE(collapse_to_2_res.entropy_delta > 0);

  // Are our step reporting vectors the correct size?
  REQUIRE(collapse_to_2_res.merge_steps.size() > 1);
  REQUIRE(collapse_to_2_res.states.size() > 1);

  // Now do a collapse to 3 total groups
  auto collapse_to_3_res = my_sbm.collapse_blocks(0,     // node_level,
                                                  3,     // B_end,
                                                  5,     // n_checks_per_block,
                                                  0,     // n_mcmc_sweeps,
                                                  1.1,   // sigma,
                                                  0.01,  // eps,
                                                  true,  // report all steps,
                                                  true); // Allow exhaustive

  // Does the network have the requested number of blocks?
  REQUIRE(my_sbm.n_nodes_at_level(1) == 3);

  // Was entropy delta strictly positive?
  // Make sure entropy has gone up as we would expect
  REQUIRE(collapse_to_3_res.entropy_delta > 0);

  // Are our step reporting vectors the correct size?
  REQUIRE(collapse_to_3_res.merge_steps.size() > 1);
  REQUIRE(collapse_to_3_res.states.size() > 1);

  // Now do a collapse to 2 total groups but with one merge per step due to a sigma less than 1
  auto collapse_to_2_full_res = my_sbm.collapse_blocks(0,     // node_level,
                                                       2,     // B_end,
                                                       5,     // n_checks_per_block,
                                                       0,     // n_mcmc_sweeps,
                                                       0.8,   // sigma,
                                                       0.01,  // eps,
                                                       true,  // report all steps,
                                                       true); // Allow exhaustive

  // Does the network have the requested number of blocks?
  REQUIRE(my_sbm.n_nodes_at_level(1) == 2);

  // Was entropy delta strictly positive?
  // Make sure entropy has gone up as we would expect
  REQUIRE(collapse_to_2_full_res.entropy_delta > 0);

  // Are our step reporting vectors the correct size?
  REQUIRE(collapse_to_2_full_res.merge_steps.size() == 6 - 2);
  REQUIRE(collapse_to_2_full_res.states.size() == 6 - 2);
}

TEST_CASE("Collapse Blocks (w/ MCMC) - Simple Bipartite", "[SBM]")
{
  auto my_sbm = simple_bipartite();

  auto collapse_to_2_res = my_sbm.collapse_blocks(0,     // node_level,
                                                  2,     // B_end,
                                                  5,     // n_checks_per_block,
                                                  5,     // n_mcmc_sweeps,
                                                  1.1,   // sigma,
                                                  0.01,  // eps,
                                                  true,  // report all steps,
                                                  true); // Allow exhaustive

  // Does the network have the requested number of blocks or fewer?
  REQUIRE(my_sbm.n_nodes_at_level(1) <= 2);

  // Are our step reporting vectors the correct size?
  REQUIRE(collapse_to_2_res.merge_steps.size() > 1);
  REQUIRE(collapse_to_2_res.states.size() > 1);
}

TEST_CASE("Collapse Blocks (w/ MCMC) - Simple Unipartite", "[SBM]")
{
  auto my_sbm = simple_unipartite();

  auto collapse_to_2_res = my_sbm.collapse_blocks(0,     // node_level,
                                                  2,     // B_end,
                                                  5,     // n_checks_per_block,
                                                  5,     // n_mcmc_sweeps,
                                                  1.1,   // sigma,
                                                  0.01,  // eps,
                                                  true,  // report all steps,
                                                  true); // Allow exhaustive

  // Does the network have the requested number of blocks or fewer?
  REQUIRE(my_sbm.n_nodes_at_level(1) <= 2);

  // Are our step reporting vectors the correct size?
  REQUIRE(collapse_to_2_res.merge_steps.size() > 1);
  REQUIRE(collapse_to_2_res.states.size() > 1);
}
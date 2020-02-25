#include "../SBM.h"
#include "catch.hpp"
#include "network_builders.cpp"
#include "print_helpers.h"

#include <iomanip>

TEST_CASE("Generate Node move proposals", "[SBM]")
{
  double tol    = 0.05;
  double eps    = 0.01;
  SBM    my_SBM = build_simple_SBM();

  NodePtr a1 = my_SBM.get_node_by_id("a1");

  // Initialize a sampler to choose block
  Sampler my_sampler;

  int     num_trials        = 5000;
  int     num_times_no_move = 0;
  NodePtr old_block         = a1->parent;

  // Run multiple trials and of move and see how often a given node is moved
  for (int i = 0; i < num_trials; ++i) {
    // Do move attempt (dry run)
    NodePtr new_block = my_SBM.propose_move(a1, eps, my_SBM.sampler);

    if (new_block->id == old_block->id)
      num_times_no_move++;
  }

  double frac_of_time_no_change = double(num_times_no_move) / double(num_trials);

  // Prob of a1 staying in a1_1 should be approximately (2 + eps)/(6 + 4*eps)
  // Make sure model's decisions to move a1 reflects this.
  double two  = 2;
  double six  = 6;
  double four = 4;

  REQUIRE(
      Approx((two + eps) / (six + four * eps)).epsilon(tol) == frac_of_time_no_change);
}

TEST_CASE("Simple entropy calculation (unipartite)", "[SBM")
{
  SBM unipartite_sbm = build_simple_SBM_unipartite();

  // Hand calculated
  REQUIRE(unipartite_sbm.get_entropy(0) == Approx(6.433708).epsilon(0.1));

  // Move node 4 to group c and compute entropy
  const NodePtr n4 = unipartite_sbm.get_node_by_id("n4", 0);
  n4->set_parent(unipartite_sbm.get_node_by_id("c", 1));

  REQUIRE(unipartite_sbm.get_entropy(0) == Approx(6.321931).epsilon(0.1));
}

TEST_CASE("Simple entropy calculation (bipartite)", "[SBM]")
{
  // Setup simple SBM model
  SBM my_SBM = build_simple_SBM();

  // Compute full entropy at first level of nodes
  double model_entropy = my_SBM.get_entropy(0);

  // Test entropy is near a hand-calculated value
  // Note that this network being so small and having bipartite structure causes the entropy
  // to be negative becuause we're using an entropy approximation function, rather than directly calculating the entropy
  // of the degree corrected model.
  REQUIRE(model_entropy == Approx(-1.420612).epsilon(0.1));

  // Now move node a2 to group a11 and calculate entropy
  NodePtr a2  = my_SBM.get_node_by_id("a2");
  NodePtr a11 = my_SBM.get_node_by_id("a11", 1);

  a2->set_parent(a11);

  // Again hand-calculated and again negative because of approximation
  REQUIRE(my_SBM.get_entropy(0) == Approx(-2.013081).epsilon(0.1));
}

TEST_CASE("Move proposal returns values are correct (simple unipartite)", "[SBM")
{
  SBM unipartite_sbm = build_simple_SBM_unipartite();

  // Propose move of n4 to group c
  const NodePtr n4 = unipartite_sbm.get_node_by_id("n4", 0);
  const NodePtr c  = unipartite_sbm.get_node_by_id("c", 1);

  const auto proposal_results = unipartite_sbm.make_proposal_decision(n4, c, 0.1);

  // Delta from hand calculation
  REQUIRE(proposal_results.entropy_delta == Approx(-0.1117765).epsilon(0.1));

  // Probality of accepting from hand calculation
  // REQUIRE(proposal_results.prob_of_accept == Approx(1.748512).epsilon(0.1));
}

TEST_CASE("Move proposal returns values are correct (simple bipartite)", "[SBM")
{
  // Setup simple SBM model
  SBM my_SBM = build_simple_SBM();

  // Now move node a2 to group a11 and calculate entropy
  NodePtr a2  = my_SBM.get_node_by_id("a2");
  NodePtr a11 = my_SBM.get_node_by_id("a11", 1);

  const auto proposal_results = my_SBM.make_proposal_decision(a2, a11, 0.1);

  // Delta from hand calculation
  REQUIRE(proposal_results.entropy_delta == Approx(-0.5924696).epsilon(0.1));

  // REQUIRE(proposal_results.prob_of_accept == Approx(0.1514709).epsilon(0.1));
}

TEST_CASE("Move proposal entropy delta is correct (Unipartite)", "[SBM]")
{
  int num_sweeps = 2;

  Sampler random(312);

  // Setup a simulated SBM model
  SBM  my_SBM    = build_unipartite_simulated();
  bool all_zeros = true;

  // Give it some random groupings of the correct number of groups
  my_SBM.initialize_blocks(0, 3);

  auto all_nodes = my_SBM.get_level(0);

  for (int i = 0; i < num_sweeps; i++) {
    // Loop through all nodes
    for (auto node_to_move_it = all_nodes->begin();
         node_to_move_it != all_nodes->end();
         node_to_move_it++) {
      const NodePtr     node_to_move      = node_to_move_it->second;
      const std::string pre_move_group_id = node_to_move->parent->id;

      // Calculate current model entropy
      const double pre_entropy = my_SBM.get_entropy(0);

      // Choose random group for node to join
      const NodePtr group_to_move_to = random.sample(my_SBM.get_nodes_of_type_at_level(node_to_move->type, 1));

      // Get move proposal report for move
      const Proposal_Res proposal_vals = my_SBM.make_proposal_decision(node_to_move, group_to_move_to, 0.1);

      const double reported_entropy_delta = proposal_vals.entropy_delta;

      // Move node
      node_to_move->set_parent(group_to_move_to);

      // Take new model entropy
      const double true_delta  = my_SBM.get_entropy(0) - pre_entropy;
      const double differences = true_delta - reported_entropy_delta;
      if (true_delta != 0)
        all_zeros = false;

      // // They should be the same
      const double thresh = 0.1;
      REQUIRE(((differences < thresh) & (differences > -thresh)));

    } // End node loop
  }   // End iteration loop

  REQUIRE(!all_zeros);
}

TEST_CASE("Move proposal entropy delta is correct (Bipartite)", "[SBM]")
{
  int     num_sweeps = 2;
  Sampler random(312);

  // Setup a simulated SBM model
  SBM my_SBM = build_bipartite_simulated();

  // Give it some random groupings of the correct number of groups
  my_SBM.initialize_blocks(0, 3);

  auto all_nodes = my_SBM.get_level(0);

  bool all_zeros = true;

  for (int i = 0; i < num_sweeps; i++) {
    // Loop through all nodes
    for (auto node_to_move_it = all_nodes->begin();
         node_to_move_it != all_nodes->end();
         node_to_move_it++) {
      NodePtr node_to_move = node_to_move_it->second;

      // Calculate current model entropy
      double pre_entropy = my_SBM.get_entropy(0);

      // Make sure that our entropy value is positive as it should be
      REQUIRE(pre_entropy > 0);

      // Choose random group for node to join
      random.sample(my_SBM.get_nodes_of_type_at_level(node_to_move->type, 1));
      NodePtr group_to_move_to = random.sample(my_SBM.get_nodes_of_type_at_level(node_to_move->type, 1));

      // Get move proposal report for move
      Proposal_Res proposal_vals = my_SBM.make_proposal_decision(node_to_move, group_to_move_to, 0.1);

      double reported_entropy_delta = proposal_vals.entropy_delta;

      // Move node
      node_to_move->set_parent(group_to_move_to);

      // Take new model entropy
      double       true_delta  = my_SBM.get_entropy(0) - pre_entropy;
      const double differences = true_delta - reported_entropy_delta;

      if (true_delta != 0)
        all_zeros = false;

  

      // // They should be the same
      const double thresh = 0.1;
      REQUIRE(((differences < thresh) & (differences > -thresh)));
    } // End node loop
  }   // End iteration loop

  REQUIRE(!all_zeros);
}

TEST_CASE("Simple move decision matches brute force calculation", "[SBM]")
{

  // Setup a simulated SBM model with another self-edge
  SBM my_SBM = build_simple_SBM_unipartite();

  const double pre_entropy = my_SBM.get_entropy(0);

  // Grab node to move
  NodePtr n5 = my_SBM.get_node_by_id("n5");

  // Grab the new group for the node
  NodePtr b = my_SBM.get_node_by_id("b", 1);

  const double expected_delta = my_SBM.make_proposal_decision(n5, b, 0.1).entropy_delta;

  n5->set_parent(b);
  const double post_entropy   = my_SBM.get_entropy(0);
  const double reported_delta = post_entropy - pre_entropy;
  REQUIRE(expected_delta == Approx(reported_delta).epsilon(0.1));
}

TEST_CASE("Basic MCMC sweeps", "[SBM]")
{
  // Setup simple SBM model
  SBM my_SBM = build_simple_SBM();

  State_Dump pre_sweep = my_SBM.get_state();

  // Run a few rounds of sweeps of the MCMC algorithm on network
  int num_sweeps = 100;

  // Loop over a few different epsilon values
  std::vector<double> epsilons = { 0.01, 0.9 };
  std::vector<double> avg_num_moves;

  for (double eps : epsilons) {
    int total_num_changes = 0;

    for (int i = 0; i < num_sweeps; i++) {
      int n_changes = my_SBM.mcmc_sweep(0, 1, eps, false, false).sweep_num_nodes_moved[0];
      total_num_changes += n_changes;
    }

    double avg_num_changes = double(total_num_changes) / double(num_sweeps);

    avg_num_moves.push_back(avg_num_changes);
  }

  // Make sure that we have a more move-prone model when we have a high epsilon value...
  REQUIRE(avg_num_moves.at(0) < avg_num_moves.at(epsilons.size() - 1));
}

TEST_CASE("Agglomerative merge steps", "[SBM]")
{
  // Setup simple SBM model
  SBM my_SBM = build_simple_SBM();

  int    num_initial_blocks = my_SBM.get_level(1)->size();
  double initial_entropy    = my_SBM.get_entropy(0);

  // Run aglomerative merge with best single merge done
  Merge_Step single_merge = my_SBM.agglomerative_merge(1, 1, 5, 0.1);

  // Make sure that we now have one less block than before for each type
  int new_block_num    = my_SBM.get_level(1)->size();
  int change_in_blocks = num_initial_blocks - new_block_num;
  REQUIRE(change_in_blocks == 1);

  // Make sure entropy has gone up as we would expect
  REQUIRE(single_merge.entropy_delta > 0);

  // Run again but this time merging the best 2
  SBM new_SBM = build_simple_SBM();

  // Run aglomerative merge with best single merge done
  Merge_Step double_merge = new_SBM.agglomerative_merge(1, 2, 5, 0.1);

  // Make sure that we now have two fewer blocks per type than before
  REQUIRE(2 == num_initial_blocks - new_SBM.get_level(1)->size());

  // Entropy should up even more with two merges
  REQUIRE(single_merge.entropy_delta < double_merge.entropy_delta);
}

TEST_CASE("Agglomerative merging algorithm steps", "[SBM]")
{
  // Setup simple SBM model
  SBM my_SBM = build_simple_SBM();

  int    num_initial_blocks = my_SBM.get_level(1)->size();
  double initial_entropy    = my_SBM.get_entropy(0);

  // Run full agglomerative merging algorithm till we have just 3 blocks left
  auto run_results = my_SBM.collapse_blocks(0, 0, 3, 5, 2, 0.1, false);

  // Make sure that we now have just 3 blocks left
  REQUIRE(my_SBM.get_level(1)->size() == 3);
}

TEST_CASE("One merge at a time agglomerative merging on larger network", "[SBM]")
{
  // Setup simple SBM model
  SBM my_SBM = build_bipartite_simulated();

  const int desired_num_blocks = 6;

  // Run full agglomerative merging algorithm
  const auto run_results = my_SBM.collapse_blocks(0, 0, desired_num_blocks, 5, 0.5, 0.1, true);

  const int num_blocks_removed = my_SBM.get_level(0)->size() - my_SBM.get_level(1)->size();

  // Make sure we have a single step for each block removed.
  REQUIRE(num_blocks_removed == run_results.size());
}

TEST_CASE("Agglomerative merging on larger network", "[SBM]")
{
  // Setup simple SBM model
  SBM my_SBM = build_bipartite_simulated();

  int desired_num_blocks = 4;

  // Run full agglomerative merging algorithm till we have just 3 blocks left
  auto run_results = my_SBM.collapse_blocks(0, 0, desired_num_blocks, 5, 1.5, 0.1, false);

  // Make sure that we have lumped together at least some blocks
  REQUIRE(my_SBM.get_level(1)->size() < my_SBM.get_level(0)->size());
}

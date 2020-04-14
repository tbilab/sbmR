#include "build_testing_networks.h"
#include "catch.hpp"

TEST_CASE("Basic mcmc sweep - Simple Bipartite", "[SBM]")
{
  auto my_sbm = simple_bipartite();

  const int n_sweeps = 2;

  auto sweep_res = my_sbm.mcmc_sweep(n_sweeps,
                                     0.2,    // eps
                                     true,   //variable num blocks
                                     false,  //track pairs
                                     0,      // level
                                     false); //verbose

  REQUIRE(sweep_res.entropy_deltas.size() == n_sweeps);
  REQUIRE(sweep_res.n_nodes_moved.size() == n_sweeps);
}

TEST_CASE("Basic mcmc sweep - Simple Unipartite", "[SBM]")
{
  const int n_sweeps = 2;

  auto my_sbm = simple_unipartite();

  auto sweep_res = my_sbm.mcmc_sweep(n_sweeps,
                                     0.2,    // eps
                                     true,   //variable num blocks
                                     false,  //track pairs
                                     0,      // level
                                     false); //verbose

  REQUIRE(sweep_res.entropy_deltas.size() == n_sweeps);
  REQUIRE(sweep_res.n_nodes_moved.size() == n_sweeps);
}

TEST_CASE("MCMC sweeps w/ varying epsilon - Simple Bipartite", "[SBM]")
{

  const int n_sweeps = 20;

  // Loop over a few different epsilon values
  std::vector<double> epsilons = { 0.01, 0.9 };
  std::vector<double> avg_n_moves;
  avg_n_moves.reserve(n_sweeps);

  for (const auto& epsilon : epsilons) {

    auto my_sbm    = simple_bipartite();
    auto sweep_res = my_sbm.mcmc_sweep(n_sweeps,
                                       epsilon, // eps
                                       true,    // variable num blocks
                                       false,   // track pairs
                                       0,       // level
                                       false);  // verbose
    avg_n_moves.push_back(sweep_res.nodes_moved.size() / double(n_sweeps));
  }

  // Make sure that we have a more move-prone model when we have a high epsilon value...
  REQUIRE(avg_n_moves.at(0) < avg_n_moves.at(1));
}

TEST_CASE("MCMC sweeps w/ varying epsilon - Simple Unipartite", "[SBM]")
{

  const int n_sweeps = 20;

  // Loop over a few different epsilon values
  std::vector<double> epsilons = { 0.01, 0.9 };
  std::vector<double> avg_n_moves;
  avg_n_moves.reserve(n_sweeps);

  for (const auto& epsilon : epsilons) {

    auto my_sbm    = simple_unipartite();
    auto sweep_res = my_sbm.mcmc_sweep(n_sweeps,
                                       epsilon, // eps
                                       true,    // variable num blocks
                                       false,   // track pairs
                                       0,       // level
                                       false);  // verbose
    avg_n_moves.push_back(sweep_res.nodes_moved.size() / double(n_sweeps));
  }

  // Make sure that we have a more move-prone model when we have a high epsilon value...
  REQUIRE(avg_n_moves.at(0) < avg_n_moves.at(1));
}
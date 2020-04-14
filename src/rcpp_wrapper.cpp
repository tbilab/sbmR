#include <RcppCommon.h>

#include "SBM.h"

using Node_Ptr = Node*;

// declaring the specialization
namespace Rcpp {
template <>
SEXP wrap(const Node_Ptr&);
template <>
SEXP wrap(const State_Dump&);
template <>
SEXP wrap(const MCMC_Sweeps&);
template <>
SEXP wrap(const Collapse_Results&);
template <>
SEXP wrap(const Edge_Counts&);
template <>
SEXP wrap(const Block_Counts&);
template <>
SEXP wrap(const Edge_Counts_by_ID&);

// Create and return dump of state as dataframe
inline DataFrame state_to_df(const State_Dump& state)
{
  return DataFrame::create(
      _["id"]               = state.ids,
      _["type"]             = state.types,
      _["parent"]           = state.parents,
      _["level"]            = state.levels,
      _["stringsAsFactors"] = false);
}
}

namespace Rcpp {

// Node pointer just gets ignored
template <>
SEXP wrap(const Node_Ptr& node_ref) { return 0; }

// Let Rcpp know how to convert a state dump object into a dataframe for R
template <>
SEXP wrap(const State_Dump& state) { return state_to_df(state); }

template <>
SEXP wrap(const MCMC_Sweeps& results)
{
  // Check if we have pair tracking information present
  const int n_pairs        = results.block_consensus.node_pairs.size();
  const bool tracked_pairs = n_pairs > 0;

  auto results_df = List::create(
      _["nodes_moved"] = results.nodes_moved,
      _["sweep_info"]  = DataFrame::create(
          _["entropy_delta"]    = results.entropy_deltas,
          _["n_nodes_moved"]    = results.n_nodes_moved,
          _["stringsAsFactors"] = false));

  if (tracked_pairs) {
    // Initialize vectors to hold pair tracking results, if needed.
    auto node_pair       = CharacterVector(n_pairs);
    auto times_connected = IntegerVector(n_pairs);

    int i = 0;
    for (const auto& pair : results.block_consensus.node_pairs) {
      node_pair[i]       = pair.first;
      times_connected[i] = pair.second.times_connected;
      i++;
    }

    results_df["pairing_counts"] = DataFrame::create(_["node_pair"]        = node_pair,
                                                     _["times_connected"]  = times_connected,
                                                     _["stringsAsFactors"] = false);
  }

  return results_df;
}

template <>
SEXP wrap(const Edge_Counts& edge_counts)
{
  const int n = edge_counts.size();

  auto a_blocks = CharacterVector(n);
  auto b_blocks = CharacterVector(n);
  auto counts   = IntegerVector(n);

  int i = 0;

  for (const auto& edge_to_count : edge_counts) {
    a_blocks[i] = edge_to_count.first.first()->id();
    b_blocks[i] = edge_to_count.first.second()->id();
    counts[i]   = edge_to_count.second;
    i++;
  }

  return DataFrame::create(_["block_a"]          = a_blocks,
                           _["block_b"]          = b_blocks,
                           _["n_edges"]          = counts,
                           _["stringsAsFactors"] = false);
}

template <>
SEXP wrap(const Edge_Counts_by_ID& edge_counts)
{
  auto block_ids = CharacterVector(edge_counts.size());
  auto counts    = IntegerVector(edge_counts.size());

  int i = 0;
  for (const auto& block_count : edge_counts) {
    block_ids[i] = block_count.first;
    counts[i]    = block_count.second;
    i++;
  }

  return DataFrame::create(_["block"]   = block_ids,
                           _["n_edges"] = counts);
}

template <>
SEXP wrap(const Block_Counts& block_counts)
{
  return DataFrame::create(_["type_id"]          = block_counts.ids,
                           _["count"]            = block_counts.counts,
                           _["stringsAsFactors"] = false);
}

template <>
SEXP wrap(const Collapse_Results& collapse_results)
{
  const int n_steps             = collapse_results.states.size();
  const bool all_steps_reported = collapse_results.merge_steps.size() != 0;

  List results = List::create(_["entropy_delta"] = collapse_results.entropy_delta,
                              _["final_entropy"] = collapse_results.final_entropy,
                              _["n_blocks"]      = collapse_results.n_blocks);

  auto step_states = List(n_steps);
  for (int i = 0; i < n_steps; i++) {
    step_states[i] = state_to_df(collapse_results.states[i]);
  }
  results["step_states"] = step_states;

  if (all_steps_reported) {
    auto step_merges    = List(n_steps);
    auto entropy_deltas = NumericVector(n_steps);
    auto n_blocks       = IntegerVector(n_steps);

    for (int i = 0; i < n_steps; i++) {
      const auto& step = collapse_results.merge_steps[i];

      entropy_deltas[i] = step.entropy_delta;
      n_blocks[i]       = step.n_blocks;

      step_merges[i] = DataFrame::create(_["from"] = step.merge_from,
                                         _["into"] = step.merge_into);
    }

    results["step_info"] = DataFrame::create(_["n_blocks"]      = n_blocks,
                                             _["entropy_delta"] = entropy_deltas);

    results["step_merges"] = step_merges;
  }

  return results;
}

} // End RCPP namespace

RCPP_MODULE(SBM)
{
  Rcpp::class_<SBM>("SBM")

      .constructor<InOut_String_Vec, // node ids
                   InOut_String_Vec, // node types
                   InOut_String_Vec, // all types
                   int>("Setup network with just nodes loaded")
      // all types
      .constructor<InOut_String_Vec, int>("Setup empty network with no nodes loaded")

      .const_method("n_nodes_at_level", &SBM::n_nodes_at_level,
                    "Returns number of nodes of all types for given level in network")
      .const_method("n_levels", &SBM::n_levels,
                    "Total number of levels. E.g. 2 = data-nodes and single block level")
      .const_method("n_types", &SBM::n_types,
                    "How many node types are in model?")
      .const_method("node_types", &SBM::node_types,
                    "Returns a vector of all the node types in model")
      .const_method("n_edges", &SBM::n_edges,
                    "How many edges are in the network?")
      .const_method("block_counts", &SBM::block_counts,
                    "Gets dataframe of counts of blocks by type.")
      .const_method("state", &SBM::state,
                    "Exports the current state of the network as dataframe with each node as a row and columns for node id, parent id, node type, and node level.")
      .const_method("interblock_edge_counts", &SBM::interblock_edge_counts,
                    "Get dataframe of counts of edges between all unique pairs of blocks in network")
      .const_method("node_to_block_edge_counts", &SBM::node_to_block_edge_counts,
                    "Get dataframe of a node's edge counts to blocks at a given level.")
      .const_method("entropy", &SBM::entropy,
                    "Calculate the degree corrected entropy of current model state at desired level")
      .method("add_node", &SBM::add_node_no_ret,
              "Add a node to the network. Takes the node id (string), the node type (string), and the node level (int). Use level = 0 for data-level nodes.")
      .method("add_edge", &SBM::add_edge,
              "Connects two nodes in network (at level 0) by their ids (string).")
      .method("add_edges", &SBM::add_edges,
              "Takes two character vectors of node ids (string) and connects the nodes with edges in network")
      .method("initialize_blocks", &SBM::initialize_blocks,
              "Adds a desired number of blocks and randomly assigns them for a given level. n_blocks = -1 means every node gets their own block")
      .method("reset_blocks", &SBM::reset_blocks,
              "Erases all block levels in network.")
      .method("update_state", &SBM::update_state,
              "Takes model state export as given by SBM$state() and returns model to specified state. This is useful for resetting model before running various algorithms such as agglomerative merging.")
      .method("mcmc_sweep", &SBM::mcmc_sweep,
              "Runs a single MCMC sweep across all nodes at specified level. Each node is given a chance to move blocks or stay in current block and all nodes are processed in random order. Takes the level that the sweep should take place on (int) and if new blocks blocks can be proposed and empty blocks removed (boolean).")
      .method("collapse_blocks", &SBM::collapse_blocks,
              "Performs agglomerative merging on network, starting with each block has a single node down to one block per node type. Arguments are level to perform merge at (int) and number of MCMC steps to peform between each collapsing to equilibriate block. Returns list with entropy and model state at each merge.");
};

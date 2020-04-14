#pragma once
// Takes a node and a new block along with the containers for the blocks and nodes and
// calculates both the entropy delta of the SBM before and after the proposed move and
// the ratio of the probabilities of moving to the proposed block before the move and
// moving back to the original block after the move.
// #include "calc_edge_entropy.h"

#include "Node.h"
#include "model_helpers.h"

using Node_Edge_Counts = std::map<const Node*, int>;
using Edge_Count       = std::pair<const Node*, int>;

struct Move_Results {
  double entropy_delta  = 0.0;
  double prob_ratio     = 1.0;
  double prob_of_accept = 0.0;
  Move_Results(const double& e, const double& p)
      : entropy_delta(e)
      , prob_ratio(p)
      , prob_of_accept(exp(-e) * p)
  {
  }
};

inline Move_Results get_move_results(const Node* node,
                                     const Node* new_block,
                                     const int n_possible_neighbors,
                                     const double eps = 0.1)
{
  Node* old_block = node->parent();

  // No need to go on if we're "swapping" to the same group
  if (new_block == old_block)
    return Move_Results(0, 1);

  // These stay constant before and after move
  const int block_level     = node->level() + 1;
  const double& node_degree = node->degree();
  const double epsB         = eps * n_possible_neighbors;

  // These will change before and after move
  double new_block_degree = new_block->degree();
  double old_block_degree = old_block->degree();

  // Gather up all the edges for both the node being moved and its old and new blocks
  Edge_Count_Map node_neighbor_counts      = node->gather_neighbors_at_level(block_level);
  Edge_Count_Map new_block_neighbor_counts = new_block->gather_neighbors_at_level(block_level);
  Edge_Count_Map old_block_neighbor_counts = old_block->gather_neighbors_at_level(block_level);

  auto get_block_degree = [&](const Node* block_t) {
    return block_t == old_block
        ? old_block_degree
        : block_t == new_block
            ? new_block_degree
            : block_t->degree();
  };

  // Get pre move entropy partials from new and old blocks
  double pre_move_ent = 0.0;

  for (const auto& new_to_t : new_block_neighbor_counts) {
    const Node* block_t = new_to_t.first;
    const int t_degree  = get_block_degree(block_t);
    const double scalar = block_t == new_block ? 2 : 1;

    pre_move_ent += ent(new_to_t.second, new_block_degree, t_degree) / scalar;
  }

  for (const auto& old_to_t : old_block_neighbor_counts) {
    const Node* block_t = old_to_t.first;

    // Don't double count the old-new edge counts
    if (block_t == new_block) continue;

    const int t_degree  = get_block_degree(block_t);
    const double scalar = block_t == old_block ? 2 : 1;

    pre_move_ent += ent(old_to_t.second, old_block_degree, t_degree) / scalar;
  }

  // Get probability of node moving to new block
  double prob_move_to_new = 0.0;
  for (const auto& node_neighbor_count : node_neighbor_counts) {
    const double t_degree   = get_block_degree(node_neighbor_count.first);
    const double edges_to_t = new_block_neighbor_counts[node_neighbor_count.first];

    prob_move_to_new += node_neighbor_count.second / node_degree * (edges_to_t + eps) / (t_degree + epsB);
  }

  // Update edge count maps for post move
  for (const auto& node_block_count : node_neighbor_counts) {
    const Node* block    = node_block_count.first;
    const int e_to_block = node_block_count.second;

    if (block == new_block) {
      increase_edge_count(new_block_neighbor_counts, new_block, 2 * e_to_block);

      reduce_edge_count(new_block_neighbor_counts, old_block, e_to_block);
      reduce_edge_count(old_block_neighbor_counts, new_block, e_to_block);
    } else if (block == old_block) {
      increase_edge_count(new_block_neighbor_counts, old_block, e_to_block);
      increase_edge_count(old_block_neighbor_counts, new_block, e_to_block);

      reduce_edge_count(old_block_neighbor_counts, old_block, 2 * e_to_block);
    } else {
      increase_edge_count(new_block_neighbor_counts, block, e_to_block);
      reduce_edge_count(old_block_neighbor_counts, block, e_to_block);
    }
  }

  new_block_degree += node_degree;
  old_block_degree -= node_degree;

  // Get post move entropy partials from new and old blocks
  double post_move_ent = 0.0;
  for (const auto& new_to_t : new_block_neighbor_counts) {
    const Node* block_t = new_to_t.first;

    const int t_degree  = get_block_degree(new_to_t.first);
    const double scalar = block_t == new_block ? 2 : 1;

    post_move_ent += ent(new_to_t.second, new_block_degree, t_degree) / scalar;
  }

  for (const auto& old_to_t : old_block_neighbor_counts) {
    const Node* block_t = old_to_t.first;

    // Don't double count the old-new edge counts
    if (block_t == new_block)
      continue;

    const int t_degree  = get_block_degree(block_t);
    const double scalar = block_t == old_block ? 2 : 1;

    post_move_ent += ent(old_to_t.second, old_block_degree, t_degree) / scalar;
  }

  // Get probability of node moving back to original block
  double prob_return_to_old = 0.0;
  for (const auto& node_neighbor_count : node_neighbor_counts) {
    const double t_degree   = get_block_degree(node_neighbor_count.first);
    const double edges_to_t = old_block_neighbor_counts[node_neighbor_count.first];

    prob_return_to_old += node_neighbor_count.second / node_degree * (edges_to_t + eps) / (t_degree + epsB);
  }

  return Move_Results(pre_move_ent - post_move_ent,
                      prob_return_to_old / prob_move_to_new);
}

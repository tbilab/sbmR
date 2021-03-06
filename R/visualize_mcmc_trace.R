#' Trace plot of multiple MCMC sweep sweeps
#'
#' A simple line plot showing both the entropy delta and the number of nodes
#' moved for each sweep. `sbmr::mcmc_sweep()` must be run on object before this function.
#'
#' @family visualizations
#'
#' @inheritParams verify_model
#'
#' @return GGplot line plot with x-axis as sweep number and y-axes as sweep
#'   entropy delta and number of nodes moved during sweep.
#' @export
#'
#' @examples
#'
#' # Simulate a model and run mcmc sweeps on it
#' net <- sim_basic_block_network(n_blocks = 3, n_nodes_per_block = 25) %>%
#'   initialize_blocks(3) %>%
#'   mcmc_sweep(num_sweeps = 75, track_pairs = TRUE)
#'
#' # Plot results
#' visualize_mcmc_trace(net)
#'
visualize_mcmc_trace <- function(sbm){
  UseMethod("visualize_mcmc_trace")
}

visualize_mcmc_trace.default <- function(sbm){
  cat("visualize_mcmc_trace generic")
}

#' @export
visualize_mcmc_trace.sbm_network <- function(sbm){

  sweep_results <- get_sweep_results(sbm)

  N <- nrow(sweep_results$sweep_info)
  sweep_results$sweep_info %>%
    dplyr::mutate(sweep = 1:N) %>%
    dplyr::rename(`entropy delta` = entropy_delta,
                  `number of nodes moved` = n_nodes_moved) %>%
    tidyr::pivot_longer(c(`entropy delta`,`number of nodes moved`)) %>%
    ggplot2::ggplot(ggplot2::aes(x = sweep, y = value)) +
    ggplot2::geom_line() +
    ggplot2::facet_grid(name~., scales = "free_y")
}

utils::globalVariables(c("n_nodes_moved", "entropy delta", "number of nodes moved", "value"))

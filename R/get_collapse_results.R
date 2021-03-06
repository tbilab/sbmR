#' Get collapse results from model
#'
#' Retrieves the dataframe recording the results of running either
#' \code{\link{collapse_blocks}} or \code{\link{collapse_run}} on
#' `sbm_network` object.
#'
#' @inheritParams verify_model
#'
#' @return List with two dataframes. The first telling for all sweeps everytime
#'   a node was moved and what group it was moved to. The second telling for
#'   each sweep the entropy delta and total number of nodes that were moved to
#'   new groups in that sweep. If `track_pairs = TRUE`, then an additional
#'   `pairing_counts` dataframe is added to output.
#' @export
#'
#' @examples
#'
#' # Start with a random network of two blocks with 25 nodes each and
#' # run agglomerative clustering with no intermediate MCMC steps on network
#' my_sbm <- sim_basic_block_network(n_blocks = 2, n_nodes_per_block = 25) %>%
#'   collapse_blocks(num_mcmc_sweeps = 0, sigma = 1.1)
#'
#' # Look at the results of the collapse directly
#' get_collapse_results(my_sbm)
#'
get_collapse_results <- function(sbm){
  UseMethod("get_collapse_results")
}


#' @export
get_collapse_results.sbm_network <- function(sbm){
  collapse_results <- sbm$collapse_results
  if(is.null(collapse_results)) {
    stop("Need to run collapse_blocks() or collapse_run() on network before retrieving collapse results")
  }
  collapse_results
}

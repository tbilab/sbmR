#' Simulate completely random network
#'
#' Generate an edge dataframe from an \href{https://en.wikipedia.org/wiki/Erd%C5%91s%E2%80%93R%C3%A9nyi_model}{Erdos-Renyi network.} An edge will occur
#' between two nodes with a uniform common probability.
#'
#' @param n_nodes Total number of unique nodes to simulate
#' @param prob_of_connection Probability of any given edges between two nodes
#'   occuring
#' @inheritParams sim_sbm_network
#'
#' @return An edge dataframe with node ids for connections in the `from` and
#'   `to` columns
#' @export
#'
#' @examples
#' sim_random_network(n_nodes = 4, prob_of_connection = 0.9)
sim_random_network <- function(n_nodes = 5, prob_of_connection = 0.7, allow_self_connections = FALSE){

  sim_sbm_network(
    group_info = dplyr::tibble(
      group = "node",
      n_nodes = n_nodes
    ),
    connection_propensities = dplyr::tibble(
      group_1 = "node",
      group_2 = "node",
      propensity = prob_of_connection
    ),
    edge_dist = purrr::rbernoulli,
    allow_self_connections = allow_self_connections
  )$edges %>%
    dplyr::select(-connections)
}
% Generated by roxygen2: do not edit by hand
% Please edit documentation in R/visualize_mcmc_sweep.R
\name{visualize_mcmc_trace}
\alias{visualize_mcmc_trace}
\title{Trace plot of multiple MCMC sweep sweeps}
\usage{
visualize_mcmc_trace(sweep_results)
}
\arguments{
\item{sweep_results}{Results of running \code{sbmR::mcmc_sweep()}}
}
\value{
GGplot line plot with x-axis as sweep number and y-axes as sweep
entropy delta and number of nodes moved during sweep.
}
\description{
A simple line plot showing both the entropy delta and the number of nodes
moved for each sweep.
}
\examples{

set.seed(42)

# Start with a random network and assign randomly to 4 blocks
n_blocks <- 3
my_sbm <- create_sbm(sim_basic_block_network(n_blocks = n_blocks, n_nodes_per_block = 15)) \%>\%
  initialize_blocks(num_blocks = n_blocks)

# Run 25 MCMC sweeps
sweep_results <- my_sbm \%>\% mcmc_sweep(num_sweeps = 25, variable_num_blocks = FALSE)

# Plot results
visualize_mcmc_trace(sweep_results)

}
/**
 * @file em_gmm.h
 * @brief Expectation-Maximization Algorithm to fit GMM
 * @author Haoxiang Li
 * @version 1.1
 * @date 2013-12-29
 */
#pragma once

/**
 * @brief Fit Mixture of Gaussian given a set of points
 *
 * @param data pointer to the contiguous memory of data points (num_pts rows -by- dim cols)
 * @param num_pts number of points
 * @param dim dimension of data point
 * @param num_modes number of gaussians
 * @param means mean of the learned GMM, one row for one gaussian 
 * @param diag_covs diagonal covariance  of the learn GM<, one row for one gaussian
 * @param weights weights of the Gaussians
 * @param should_fit_spherical_gaussian whether the learned Gaussians are spherical or not, 
 *        spherical Gaussian has identical variance along all dimensions
 */
void em_gmm(double const *data, long num_pts, long dim, int num_modes,
		double *means, double *diag_covs, double *weights);
double liklihoods_func(double const *data, long num_pts, long dim,
		int num_modes, double const *means, double const *diag_covs,
		double const *weights, double const *matrix);

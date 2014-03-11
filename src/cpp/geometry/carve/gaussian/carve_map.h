#ifndef CARVE_MAP_H
#define CARVE_MAP_H

/**
 * @file carve_map.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 *
 * @section DESCRIPTION
 *
 * The carve_map_t class is defined here.  This class is used
 * to compute the result of a carve map, given a position in
 * continuous 3D space.  A carve map is generated by a single
 * range scan, and assigns a value to all of 3D space.  This
 * value is an estimate of the probability that a given point
 * in space is 'interior'.  A value of 0.5 represents no knowledge
 * of the environment, a value of 1.0 indicates certainty that
 * the point is interior, and a value of 0.0 indicates certainty
 * that the point is exterior.
 *
 * This class requires the Eigen framework.
 */

#include <iostream>
#include <Eigen/Dense>

/**
 * This class represents a mapping from R^3 to [0,1], indicating P(interior)
 */
class carve_map_t
{
	/* parameters */
	private:

		/*---------------------*/
		/* input distributions */
		/*---------------------*/

		/* the gaussian distribution of the sensor position */
		Eigen::Vector3d sensor_mean;
		Eigen::Matrix3d sensor_cov;

		/* the gaussian distribution of the scanpoint position */
		Eigen::Vector3d scanpoint_mean;
		Eigen::Matrix3d scanpoint_cov;

		/*-------------------*/
		/* cached parameters */
		/*-------------------*/

		/* unit vector along mean of ray */
		Eigen::Vector3d ray;

		/* each end of the ray is modeled partly with a
		 * plane in line with one of the dominant axes of
		 * the covariance of each gaussian.  These values indicate
		 * the normals of these planes. */
		Eigen::Vector3d sensor_norm;
		double sensor_dot; /* dot(sensor_norm, ray) */
		double sensor_var; /* marginalized variance of sensor */
		double sensor_neg_inv_sqrt_2v; /* 1 / sqrt(2*var) */
		Eigen::Vector3d scanpoint_norm;
		double scanpoint_dot; /* dot(scanpoint_norm, ray) */
		double scanpoint_var; /* marginalized variance of point */
		double scanpoint_neg_inv_sqrt_2v; /* 1 / sqrt(2*var) */

	/* functions */
	public:

		/*--------------*/
		/* constructors */
		/*--------------*/

		/* Since this class contains Eigen structures, we
		 * need to properly align memory */
		EIGEN_MAKE_ALIGNED_OPERATOR_NEW

		/**
		 * Initializes empty map
		 */
		carve_map_t();

		/**
		 * Initializes carve map based on two gaussian distribs.
		 *
		 * @param s_mean  Mean of sensor position
		 * @param s_cov   Covariance of sensor position
		 * @param p_mean  Mean of scanpoint position
		 * @param p_cov   Covariance of scanpoint position
		 */
		carve_map_t(const Eigen::Vector3d& s_mean,
		            const Eigen::Matrix3d& s_cov,
		            const Eigen::Vector3d& p_mean,
		            const Eigen::Matrix3d& p_cov);

		/**
		 * Initializes this carve map based on two gaussians
		 *
		 * Will initialze all stored parameters based on the
		 * gaussian distributions of the sensor positon and
		 * the scanpoint position in 3D space.
		 *
		 * @param s_mean  Mean of sensor position
		 * @param s_cov   Covariance of sensor position
		 * @param p_mean  Mean of scanpoint position
		 * @param p_cov   Covariance of scanpoint position
		 */
		void init(const Eigen::Vector3d& s_mean,
		          const Eigen::Matrix3d& s_cov,
		          const Eigen::Vector3d& p_mean,
		          const Eigen::Matrix3d& p_cov);

		/*-------------*/
		/* computation */
		/*-------------*/
		
		/**
		 * Computes the value of this carve map at the given loc
		 *
		 * Given a location and a feature size at that location,
		 * will compute the estimate of this carve map for that
		 * location.  The feature size should be a rough distance
		 * estimate of the feature length of the volume being
		 * analyzed.  The result of this computation will be
		 * a probability estimate of whether that volume is
		 * interior.
		 *
		 * 0.0 ............ 0.5 ........... 1.0
		 * exterior       unknown      interior
		 *
		 * @param x      The volume center in 3D space to analyze
		 * @param xsize  The feature length of volume at x
		 *
		 * @return   Returns probability that volume is interior
		 */
		double compute(const Eigen::Vector3d& x,double xsize) const;

		/*-----------*/
		/* debugging */
		/*-----------*/

		/**
		 * Will export a sampling of this mapping to the outstream
		 *
		 * Will provide a sampling of 3D points near and around
		 * the originating ray of this mapping, and write these
		 * to the ASCII stream.  Each line of the stream will
		 * be one sample point, formatted as:
		 *
		 * <x> <y> <z> <f>
		 *
		 * Where <x,y,z> is a 3D position, and <f> is the value
		 * of this mapping at that position.  The first line will
		 * be the sensor mean position and the second line will
		 * be the scanpoint mean position.
		 *
		 * @param os   The output stream to write to
		 */
		void print_sampling(std::ostream& os) const;

	/* helper functions */
	private:

		/**
		 * Finds principal component that best aligns with input
		 *
		 * Given an input vector and an input matrix, will find
		 * the eigenvector of the matrix that is the closest
		 * aligned with the input vector.  This vector will be
		 * an output, with an orientation to be the same as the
		 * input vector (so their dot-product is positive).
		 *
		 * @param eig   Where to store the output vector
		 * @param in    The input vector to analyze
		 * @param M     The input matrix to analyze
		 *
		 * @return      Returns dot(eig, in)
		 */
		static double find_aligned_eig(Eigen::Vector3d& eig,
		                               const Eigen::Vector3d& in,
		                               const Eigen::Matrix3d& M);
};

#endif

#ifndef SYSTEM_PATH_H
#define SYSTEM_PATH_H

/**
 * @file   system_path.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 *
 * @section DESCRIPTION
 *
 * This file contains structs and functions
 * used to manipulate poses of the scan acquisition
 * system.
 *
 * Typically the path information is retrieved from a *.mad
 * file (which is an outdated path format).
 *
 * The system transformations are read in from a hardware
 * xml configuration file, which allows conversion from
 * system common to the location of a specific sensor at
 * each timestep.
 */

#include <string>
#include <map>
#include <Eigen/Core>
#include <Eigen/Dense>
#include <Eigen/Geometry>
#include <geometry/transform.h>

/* the following classes are defined in this file */
class system_path_t;
class pose_t;

/**
 * This class represents a complete system_path: a list of poses
 *
 * Using this list of poses, this classes represents a continuous
 * mapping between timestamps and the 6-degree-of-freedom pose of
 * the system.  From this mapping, the pose of a specific sensor
 * can be determined at any arbitrary timestamp within the
 * duration of a dataset.
 */
class system_path_t
{
	/*** parameters ***/
	private:

		/* this pose list represents the poses in the system_path,
		 * in chronological order (dynamically allocated). */
		pose_t* pl;
		unsigned int pl_size; /* number of poses */

		/* the following map specifies the transforms from
		 * sensor coordinate systems to system common coordinates */
		std::map<std::string, transform_t*> transform_map;

	/*** functions ***/
	public:

		/* constructors */

		/**
		 * Constructs empty system path
		 */
		system_path_t();
		
		/**
		 * Frees all memory and resources
		 */
		~system_path_t();

		/* i/o */
		
		/**
		 * Reads *.mad file and stores results.
		 *
		 * This function will destroy any pose information 
		 * currently in this object.
		 *
		 * @param filename    Local path to file to parse.
		 *
		 * @return      Returns 0 on success, non-zero on failure.
		 */
		int readmad(const std::string& filename);

		/**
		 * Reads in hardware transformations for each sensor
		 *
		 * Will parse the given xml hardware configuration file,
		 * and store the transformations for each sensor
		 * in this object.
		 *
		 * @param  xml     The hardware config xml file
		 *
		 * @return     Returns zero on success, non-zero on failure.
		 */
		int parse_hardware_config(const std::string& xml);

		/**
		 * Clears all information in this structure.
		 *
		 * Resets the value of this structure to the state before
		 * any pose information was read.
		 */
		void clear();

		/* acessors */

		/**
		 * Generates the interpolated pose for specified timestamp.
		 *
		 * Given a timestamp, will compute the interpolate
		 * pose of the system for that time, based on Spherical 
		 * Linear intERPolation (SLERP).  See:
		 *
		 * 	http://en.wikipedia.org/wiki/Slerp
		 *
		 * @param p   Where to store the output pose
		 * @param t   The timestamp in question
		 *
		 * @return    Returns zero on success, non-zero on failure.
		 */
		int compute_pose_at(pose_t& p, double t) const;

		/**
		 * Computes transform of specified sensor at given timestamp
		 *
		 * Given a sensor and a timestamp, will generate an
		 * interpolated transform of the sensor's position and
		 * orientation at that time from sensor coordinates to
		 * world coordinates.  This is done by finding
		 * the interpolated pose of the system using
		 * compute_pose_at(), then pre-applying the sensor->system
		 * transform.
		 *
		 * @param p   Where to store the output transform
		 * @param t   The timestamp of the pose to compute
		 * @param s   The name of the sensor to use
		 *
		 * @return    Returns zero on success, non-zero on failure.
		 */
		int compute_transform_for(transform_t& p, double t,
		                     const std::string& s) const;

	/* helper functions */
	private:

		/**
		 * Computes index of last pose at or before time t.
		 *
		 * Returns the index of the latest pose in pl
		 * that occurred at or before the specified time t.
		 *
		 * @param t  The timstamp to use to look-up pose
		 *
		 * @return   On success, returns non-negative index of 
		 * a pose.   On failure, returns negative value.
		 */
		int closest_index(double t) const;
};

/**
 * Represents a single pose: a location and orientation in space-time.
 */
class pose_t
{
	/* the path can modify poses */
	friend class system_path_t;

	/*** parameters ***/
	public:

		/* for a single pose, we are interested in temporal and 
		 * spatial position */
		double timestamp;

		/* Position of system common specified in ENU coordinates */
		Eigen::Vector3d T; /* meters */
		
		/* The following specify the orientation of the scanner
		 * at this pose as a rotation from system coordinates
		 * to world coordinates */
		Eigen::Quaternion<double> R; /* rotation to world coords */
	
	/*** functions ***/
	public:
	
		/* the pose list is an array that contains eigen 
		 * constructions, so it must be properly aligned */
		EIGEN_MAKE_ALIGNED_OPERATOR_NEW

		/* constructors */
		
		/**
		 * Constructs pose at origin with default orientation
		 */
		pose_t();

		/**
		 * Clears all memory and resources
		 */
		~pose_t();

		/* operators */

		inline pose_t operator = (const pose_t& rhs)
		{
			/* copy params */
			this->timestamp = rhs.timestamp;
			this->T = rhs.T;
			this->R = rhs.R;

			/* return the value of this point */
			return (*this);
		};

		/* geometry */
	
		/**
		 * Computes the squared-distance between two poses
		 *
		 * Compues the squared-distance between this pose
		 * and the argument pose.  Only spatial translation
		 * is considered, not timestamps or rotations values.
		 *
		 * @param other   The other pose to analyze
		 *
		 * @return   Returns the square of the distance (spatial).
		 */
		double dist_sq(const pose_t& other) const;

		/* i/o */

		/**
		 * Prints the pose information for debugging purposes.
		 *
		 * Pose information is printed to stdout as a space-
		 * separated list of values <time> <T> <R>
		 */
		void print() const;

	/* helper functions */
	private:

		/**
		 * Computes rotation quaternion from NED roll, pitch, yaw
		 * 
		 * Assumes roll, pitch, yaw are in NED coordinates
		 * in units of radians.  The resulting quaternion will
		 * be stored in this->R.
		 *
		 * @param roll   The roll value  (rotation about +x)
		 * @param pitch  The pitch value (rotation about +y)
		 * @param yaw    The yaw value   (rotation about +z)
		 */
		void compute_transform_NED(double roll, double pitch,
		                           double yaw);
};

#endif
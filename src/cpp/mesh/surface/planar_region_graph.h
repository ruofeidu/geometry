#ifndef PLANAR_REGION_GRAPH_H
#define PLANAR_REGION_GRAPH_H

/**
 * @file   planar_region_graph.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Represents the neighbor/connectivity info for regions
 *
 * @section DESCRIPTION
 *
 * Planar regions are used to represent subsets of node faces generated
 * from an octree.  This file contains the planar_region_graph_t, which
 * is used to organize all the regions within a model, and provide
 * connectivity information between regions (i.e. which regions are adjacent
 * to which other regions).
 *
 * This class also is used to generate the set of regions from the model.
 * It is assumed that the topology of the octree has been constructed using
 * an octtopo_t object, and that the boundary faces of that topology have
 * been generated with a node_boundary_t object.
 */

#include <geometry/shapes/plane.h>
#include <mesh/surface/node_boundary.h>
#include <mesh/surface/planar_region.h>
#include <string>
#include <map>
#include <float.h>
#include <Eigen/Dense>

/* the following classes are defined in this file */
class planar_region_graph_t;
class planar_region_info_t;
class planar_region_pair_t;

/* the following types are defined in this file */
typedef std::map<node_face_t, planar_region_info_t> regionmap_t;
typedef std::map<node_face_t, node_face_t>          seedmap_t;

/**
 * The planar_region_graph_t represents the set of regions on a model
 *
 * The set of regions on a model are represented by groupings of the
 * node faces within that model.  Each region also has references to
 * the other regions in the model that it is connected to.  Thi
 * connectivity is determined based on the linkages between node faces.
 * That is, if region A contains node 1, region B contains node 2, and
 * nodes 1 and 2 are linked, then regions A and B are also linked.
 *
 * This class is used to generate a set of coalesced regions, so it must
 * be initialized with a set of parameters that describe how to coalesce
 * each region.
 */
class planar_region_graph_t
{
	/* parameters */
	private:

		/**
		 * Represents all the generated regions and their info
		 *
		 * This mapping goes from seed faces to regions and region
		 * info.  Each region is initialized with a face node that
		 * acts as a seed for the region.  As regions get coalesced,
		 * the total number of regions is reduced but the same
		 * seeds are used to represent the regions.
		 *
		 * This can be thought of as the mapping: regions --> faces
		 */
		regionmap_t regions;

		/**
		 * Represents the mapping: faces --> regions
		 *
		 * This structure can be thought of as the inverse mapping
		 * from the regionmap_t, since it allows fast determination
		 * of which region a given node face is in.
		 */
		seedmap_t seeds;

		/* the following parameters are for region coalescing */

		/**
		 * The planarity threshold is used to determine if a
		 * face should be added to a plane.  The originating
		 * nodes for faces have estimates of the planarity of
		 * surfaces in that volume.  This is used to compute
		 * the planarity of each face.
		 *
		 * If a face has a planarity below this threshold, then
		 * it cannot be used to coalesce two regions.
		 */
		double planarity_threshold;

		/**
		 * The distance threshold is used to determine whether
		 * a given face (and its corresponding center position)
		 * are inliers or outliers to a region.
		 *
		 * If a face has a position p with variance v, then
		 * the face will be considered an outlier if the distance
		 * of p to the plane of the region is greater than 
		 * sqrt(v)*distance_threshold.
		 *
		 * Thus, distance_threshold is measured in units of
		 * standard deviations.
		 */
		double distance_threshold;

	/* functions */
	public:

		/*----------------*/
		/* initialization */
		/*----------------*/

		/**
		 * Constructs this object with default parameters
		 */
		planar_region_graph_t();

		/**
		 * Iniitalizes this object based on the given parameters
		 *
		 * This region graph is used to generate a set of coalesced
		 * regions from a set of faces.  In order to coalesce these
		 * regions effectively, it must use the following
		 * parameters.
		 *
		 * Note that this function should be called before calling
		 * populate.  If it is not called, then a set of default
		 * parameters will be used.
		 *
		 * @param planethresh   The planarity threshold [0,1] to use
		 * @param distthresh    The distance threshold to use, in
		 *                      units of standard deviations.
		 */
		void init(double planethresh, double distthresh);

		/*------------*/
		/* processing */
		/*------------*/

		/**
		 * Populates the set of regions from the given set of faces
		 *
		 * Given a set of faces represented by a node boundary
		 * object, will populate the regions in this model.
		 *
		 * @param boundary    The boundary faces of the model
		 *
		 * @return     Returns zero on success, non-zero on failure
		 */
		int populate(const node_boundary_t& boundary);

		/**
		 * Will attempt to coalesce regions in this graph
		 *
		 * This function will join regions in this until no
		 * neighboring regions can join without violating the
		 * error thresholds provided when init() was called.
		 *
		 * @return    Returns zero on success, non-zero on failure.
		 */
		int coalesce_regions();

		/*-----------*/
		/* debugging */
		/*-----------*/

		/**
		 * Writes region geometry to specified Wavefront OBJ file
		 *
		 * Given the location of a .obj file to write to, will
		 * export the node faces for each region to this file.
		 *
		 * @param filename    Where to write the file
		 *
		 * @return     Returns zero on success, non-zero on failure.
		 */
		int writeobj(const std::string& filename) const;

	/* public helper functions */
	public:

		/**
		 * Will compute the planrity estimate for the given face
		 *
		 * Will use the originating octnodes of the specified
		 * face to compute the planarity estimate of that face.
		 * A planarity estimate is a value between 0 and 1, where
		 * 1 is perfectly planar and 0 is not planar at all.
		 *
		 * @param f   The face to analyze
		 *
		 * @return    Returns the planarity estimate of f, [0,1]
		 */
		static double get_face_planarity(const node_face_t& f);

		/**
		 * Computes the position of the center of the face,
		 * assuming alignment with the local isosurface.
		 *
		 * Rather than using the face's built-in get_center()
		 * function, which aligns the center with the octree
		 * grid, this function will compute the center based
		 * on the underlying octdata in order to put the 
		 * face on the isosurface of the probability distribution.
		 *
		 * @param f    The face to analyze
		 * @param p    Where to store the center position
		 */
		 static void get_isosurface_pos(const node_face_t& f,
				 	Eigen::Vector3d& p);

		/**
		 * Computes the variance of the face position along the
		 * normal direction of the face.
		 *
		 * Given a node face to analyze, this function will
		 * determine what the positional variance is for
		 * that face's center point, along the normal of the face.
		 * This computation uses the octdata values in the
		 * originating octnodes that are used to represent this
		 * face.
		 *
		 * @param f     The face to analyze
		 *
		 * @return      Returns the variance in f's center position
		 *              along f's normal vector.
		 */
		 static double get_face_pos_var(const node_face_t& f);

	/* private helper functions */
	private:

		/**
		 * Will compute the best-fit plane for the combination
		 * of the two regions in this pair.
		 *
		 * Will perform PCA analysis on the center points of the
		 * faces in these regions.  Will also compute the
		 * normalized maximal error (in units of standard
		 * deviations) of these center points from the computed
		 * plane.
		 *
		 * These values will be stored in the parameters of this
		 * structure 'plane' and 'max_err' respectively.
		 *
		 * @param pair   The pair of regions to analyze
		 *
		 * @return    Returns zero on success, non-zero on failure.
		 */
		int compute_planefit(planar_region_pair_t& pair) const;
};

/**
 * This class represents a planar region along with its connectivity info
 */
class planar_region_info_t
{
	/* security */
	friend class planar_region_graph_t;

	/* parameters */
	private:

		/**
		 * The region in question
		 */
		planar_region_t region;

		/**
		 * This set represents the seed faces for each region
		 * that neighbors the region represented by this object
		 *
		 * Note that these links are symmetric, and need to be
		 * updated after each coalescion.
		 */
		faceset_t neighbor_seeds;

	/* functions */
	public:

		/**
		 * Constructs empty region
		 */
		planar_region_info_t()
		{ /* no processing required */ };

		/**
		 * Constructs region based on flood-fill operation
		 *
		 * Given necessary face-linkage information, will perform
		 * flood-fill on the given face in order to form a region.
		 *
		 * @param f          The face to use as a seed for this 
		 *                   region
		 * @param boundary   The node_boundary_t that represents
		 *                   face connectivity
		 * @param blacklist  List of face not allowed to be a part
		 *                   of this region (typically because
		 *                   they've already been allocated
		 *                   another region).
		 */
		planar_region_info_t(const node_face_t& f,
				const node_boundary_t& boundary,
				faceset_t& blacklist);

		// TODO functions to merge two regions
};

/**
 * This class is used to represent two neighboring regions for coalescing.
 *
 * Objects of this class contain references to the seeds of two regions,
 * which can be used to compute the merged plane geometry of the two
 * regions.  This can be used to quantify the cost of merging the two
 * regions.
 */
class planar_region_pair_t
{
	/* parameters */
	public:

		/* the two regions to compare, as represented by their
		 * node face seeds */
		node_face_t first;
		node_face_t second;

		/* the following parameters represent the result of
		 * plane fit analysis on the two regions in this pair */
		plane_t plane; /* the computed best-fit plane */
		double max_err; /* normalized maximum error in the fit */

	/* functions */
	public:

		/**
		 * Constructs a default pair
		 */
		planar_region_pair_t()
		{ this->max_err = DBL_MAX; };

		/*-----------*/
		/* operators */
		/*-----------*/

		/**
		 * Sets the value of this pair, given the provided argument
		 */
		inline planar_region_pair_t& operator = (
				const planar_region_pair_t& other)
		{
			/* copy values */
			this->first = other.first;
			this->second = other.second;
			this->plane = other.plane;
			this->max_err = other.max_err;

			/* return the modified result */
			return (*this);
		};

		/**
		 * Determines if this pair is equal to the provided argument
		 *
		 * Pairs are compared based on the 'max_err' parameter.
		 */
		inline bool operator == (
				const planar_region_pair_t& other) const
		{
			return (this->max_err == other.max_err);
		};

		/**
		 * Compares the ordering for two pairs
		 *
		 * Checks if this pair is less than the given argument
		 */
		inline bool operator < (
				const planar_region_pair_t& other) const
		{
			return (this->max_err < other.max_err);
		};
};

#endif

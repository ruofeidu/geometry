#include "tree_exporter.h"
#include <geometry/octree/octree.h>
#include <geometry/octree/octnode.h>
#include <geometry/octree/octdata.h>
#include <stdlib.h>
#include <cmath>
#include <string>
#include <iostream>
#include <fstream>
#include <Eigen/Dense>

/**
 * @file tree_exporter.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 *
 * @section DESCRIPTION
 *
 * This file contains function implementations that are used
 * to export information stored in an octree_t to various
 * formats for visualization purposes.
 */

using namespace std;
using namespace Eigen;

/* function implementations */

/**
 * Helper function used to export leaf center to OBJ file
 *
 * This helper function is recursive, and is called by
 * tree_exporter::export_leafs_to_obj().
 *
 * @param os     The output file stream to write to
 * @param node   The node to analyze and export recursively
 */
void export_leafs_to_obj_recur(ostream& os, const octnode_t* node)
{
	unsigned int i, red, green, blue;
	double p;

	/* check if this node is a leaf (i.e. it has data)*/
	if(node->data != NULL)
	{
		/* get characteristic to visualize */
		p = node->data->get_probability();
		if(p > 1)
			p = 1;
		if(p < 0)
			p = 0;
		
		/* assign colors */
		red = (unsigned int) (255 * (1-p));
		green = (unsigned int) (255 * (1 - (2*fabs(p-0.5))));
		blue = (unsigned int) (255 * p);

		/* export this leaf */
		os << "v " << node->center(0)
		   <<  " " << node->center(1)
		   <<  " " << node->center(2)
		   <<  " " << red
		   <<  " " << green
		   <<  " " << blue
		   << endl;
	}

	/* recurse through the node's children */
	for(i = 0; i < CHILDREN_PER_NODE; i++)
		if(node->children[i] != NULL)
			export_leafs_to_obj_recur(os, node->children[i]);
}

int tree_exporter::export_leafs_to_obj(const string& filename,
                                       const octree_t& tree)
{
	ofstream outfile;
	
	/* open file for writing */
	outfile.open(filename.c_str());
	if(!(outfile.is_open()))
		return -1; /* could not open file */

	/* add some header information */
	outfile << "# This file generated by tree_exporter" << endl
	        << "#" << endl
	        << "# The contents are a list of vertices, which" << endl
		<< "# denote the 3D positions of centers of leaf" << endl
		<< "# nodes of an octree, colored based on the" << endl
		<< "# data stored in that tree." << endl << endl;

	/* export to file */
	export_leafs_to_obj_recur(outfile, tree.get_root());

	/* clean up */
	outfile.close();
	return 0;
}
	
/**
 * Helper function used to export exterior cubes to OBJ file
 *
 * This helper function is recursive, and is called by
 * tree_exporter::export_exterior_cubes_to_obj().
 *
 * @param os     The output file stream to write to
 * @param node   The node to analyze and export recursively
 */
void export_exterior_cubes_to_obj_recur(ostream& os, const octnode_t* node)
{
	unsigned int i;
	double hw;
	int cc[8][3] = { /* this list indicates corner corner pos */
			{ 1, 1, 1},
			{ 1,-1, 1},
			{-1,-1, 1},
			{-1, 1, 1},
			{ 1, 1,-1},
			{ 1,-1,-1},
			{-1,-1,-1},
			{-1, 1,-1}};

	/* check if this node is a leaf (i.e. it has data) */
	if(node->data != NULL && !(node->data->is_interior()))
	{
		/* export cube of this leaf */
	
		/* vertices of cube */
		hw = node->halfwidth;
		for(i = 0; i < 8; i++)
			os << "v " << (node->center(0)+cc[i][0]*hw)
			   <<  " " << (node->center(1)+cc[i][1]*hw)
			   <<  " " << (node->center(2)+cc[i][2]*hw)
			   << endl;
		
		/* faces of the cube */
		os << "f -1 -4 -3 -2" << endl
		   << "f -5 -6 -7 -8" << endl
		   << "f -2 -3 -7 -6" << endl
		   << "f -1 -5 -8 -4" << endl
		   << "f -3 -4 -8 -7" << endl
		   << "f -2 -1 -5 -6" << endl;
	}

	/* recurse through the node's children */
	for(i = 0; i < CHILDREN_PER_NODE; i++)
		if(node->children[i] != NULL)
			export_exterior_cubes_to_obj_recur(os,
					node->children[i]);
}

int tree_exporter::export_exterior_cubes_to_obj(const string& filename,
                                                const octree_t& tree)
{
	ofstream outfile;
	
	/* open file for writing */
	outfile.open(filename.c_str());
	if(!(outfile.is_open()))
		return -1; /* could not open file */

	/* add some header information */
	outfile << "# This file generated by tree_exporter" << endl
	        << "#" << endl
	        << "# The contents are a set of cubes, which" << endl
		<< "# denote the 3D positions of exterior leaf" << endl
		<< "# nodes of an octree, colored based on the" << endl
		<< "# data stored in that tree." << endl << endl;

	/* export to file */
	export_exterior_cubes_to_obj_recur(outfile, tree.get_root());

	/* clean up */
	outfile.close();
	return 0;
}

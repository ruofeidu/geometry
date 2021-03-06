#include "octsurf_run_settings.h"
#include <io/octree/tree_exporter.h>
#include <io/octree/vox_writer.h>
#include <io/octree/sof_io.h>
#include <geometry/octree/octree.h>
#include <geometry/shapes/bloated_fp.h>
#include <mesh/refine/octree_padder.h>
#include <mesh/surface/node_boundary.h>
#include <mesh/floorplan/floorplan.h>
#include <util/error_codes.h>
#include <iostream>
#include <set>

/**
 * @file   main.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Exports octrees (.oct) to various output files
 *
 * @section DESCRIPTION
 *
 * This is the main file for the octree surface reconstruction program
 * (octsurf) which will generate a surface reconstruction of a building
 * interior environment from an octree generated by procarve.
 */

using namespace std;

/*------------------*/
/* function headers */
/*------------------*/

/**
 * This helper function is used to remove parts of the octree
 * that are considered 'explosions'.
 *
 * Explosions are areas that occur far away from any floorplan generation
 * of the environment, and are considered outside of the scan area.  These
 * parts are removed from the octree in order to simplify the geometry.
 *
 * Note that if no floorplans are provided, or an invalid buffer is given,
 * then no trimming will be performed.
 *
 * @param tree   The tree to modify
 * @param args   The settings to use
 * 
 * @return   Returns zero on success, non-zero on failure.
 */
int trim_explosions(octree_t& tree, octsurf_run_settings_t args);

/*--------------------------*/
/* function implementations */
/*--------------------------*/

/**
 * The main function for this program
 */
int main(int argc, char** argv)
{
	octsurf_run_settings_t args;
	node_boundary_t::SEG_SCHEME scheme;
	octree_t tree;
	int ret;

	/* parse the given parameters */
	ret = args.parse(argc, argv);
	if(ret)
	{
		cerr << "[main]\tError " << ret << ": "
		     << "Could not parse parameters" << endl;
		return 1;
	}

	/* initialize */
	ret = tree.parse(args.octfiles[0]);
	if(ret)
	{
		cerr << "[main]\tError " << ret << ": "
		     << "Unable to read octfile." << endl;
		return 2;
	}
				
	/* determine which scheme to use, to see if we need to trim */
	scheme = node_boundary_t::SEG_ALL;
	if(args.export_objects)
		scheme = node_boundary_t::SEG_OBJECTS;
	else if(args.export_room)
		scheme = node_boundary_t::SEG_ROOM;

	/* check for further trimming from input floorplans */
	ret = trim_explosions(tree, args);
	if(ret)
	{
		cerr << "[main]\tError " << ret << ": "
		     << "Unable to trim explosions." << endl;
		return 3;
	}
	octree_padder::pad(tree);

	/* export */
	switch(args.output_format)
	{
		default:
			/* unable to export to this file format */
			cerr << "[main]\tUnknown file extension provided"
			     << " for output file: " << args.outfile
			     << endl;
			break;
		case FORMAT_VOX:
			/* export tree as vox file */
			ret = vox_writer_t::write(args.outfile, tree);
			if(ret)
			{
				cerr << "[main]\tError " << ret << ": "
				     << "Unable to export to vox" << endl;
				return 4;
			}
			break;
		case FORMAT_SOF:
			/* export tree as sof file */
			ret = sof_io::writesof(tree, args.outfile);
			if(ret)
			{
				cerr << "[main]\tError " << ret << ": "
				     << "Unable to export to sof" << endl;
				return 5;
			}
			break;
		case FORMAT_SOG:
			/* export tree as sog file */
			ret = sof_io::writesog(tree, args.outfile);
			if(ret)
			{
				cerr << "[main]\tError " << ret << ": "
				     << "Unable to export to sog" << endl;
				return 6;
			}
			break;
		case FORMAT_PLY:
			/* export ply file of node faces */
			if(args.export_planar)
				ret = tree_exporter::export_planar_mesh(
						args.outfile, tree, scheme,
						args.xml_settings);
			else if(args.export_dense)
				ret = tree_exporter::export_dense_mesh(
						args.outfile, tree, scheme);
			else if(args.export_node_faces)
				ret = tree_exporter::export_node_faces(
					args.outfile, tree, scheme);
			else
				ret = tree_exporter::export_all(
						args.outfile, tree,
						args.xml_settings);
			if(ret)
			{
				cerr << "[main]\tError " << ret << ": "
				     << "Unable to export to ply" << endl;
				return 7;
			}
			break;
		case FORMAT_OBJ:
			/* export basic obj file */
			if(args.export_planar)
				ret = tree_exporter::export_planar_mesh(
						args.outfile, tree, scheme,
						args.xml_settings);
			else if(args.export_dense)
				ret = tree_exporter::export_dense_mesh(
						args.outfile, tree, scheme);
			else if(args.export_node_faces)
				ret = tree_exporter::export_node_faces(
					args.outfile, tree, scheme);
			else if(args.export_regions)
				ret = tree_exporter::export_regions(
						args.outfile, tree, scheme,
						args.xml_settings);
			else if(args.export_obj_leafs)
				ret = tree_exporter::export_leafs_to_obj(
						args.outfile, tree);
			else if(args.export_corners)
				ret = tree_exporter::export_corners_to_obj(
						args.outfile, tree);
			else
				ret = tree_exporter::export_all(
						args.outfile, tree,
						args.xml_settings);
			if(ret)
			{
				cerr << "[main]\tError " << ret << ": "
				     << "Unable to export to obj" << endl;
				return 8;
			}
			break;
		case FORMAT_TXT:
			/* export useful stats to txt */
			ret = tree_exporter::export_stats_to_txt(
					args.outfile, tree);
			if(ret)
			{
				cerr << "[main]\tError " << ret << ": "
				     << "Unable to export to txt" << endl;
				return 9;
			}
			break;
	}

	/* success */
	return 0;
}

int trim_explosions(octree_t& tree, octsurf_run_settings_t args)
{
	set<octdata_t*> whitelist;
	fp::floorplan_t floorplan;
	bloated_fp_t shape;
	size_t fi, num_fps;
	int ret;

	/* check if any floorplans are defined.  If not,
	 * then no filtering will occur */
	if(args.floorplans.empty())
		return 0;

	/* check if the explosion buffer is valid.  If not,
	 * then no filtering will occurr */
	if(args.explosion_buffer < 0.0)
		return 0;
	
	/* iterate over the floorplans given */
	num_fps = args.floorplans.size();
	for(fi = 0; fi < num_fps; fi++)
	{
		/* read in this floorplan */
		ret = floorplan.import_from_fp(args.floorplans[fi]);
		if(ret)
		{
			cerr << "[trim_explosions]\tError " << ret << ": "
			     << "Unable to read floorplan file #" 
			     << (fi+1) << ": "
			     << args.floorplans[fi] << endl << endl;
			return PROPEGATE_ERROR(-1, ret);
		}

		/* populate the 'bloated' floorplan */
		shape.init(floorplan, args.explosion_buffer);

		/* get the whitelist from the bloated floorplan */
		tree.find(shape);

		/* add to total whitelist */
		whitelist.insert(shape.begin(), shape.end());
	}

	/* trim the octree based on the compiled whitelist */
	tree.filter(whitelist);

	/* success */
	return 0;
}

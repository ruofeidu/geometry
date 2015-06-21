#include "generate_scanorama_run_settings.h"
#include <image/scanorama/scanorama_maker.h>
#include <util/tictoc.h>
#include <iostream>
#include <string>

/**
 * @file   main.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Will generate scanorama (.ptx) files from this dataset
 *
 * @section DESCRIPTION
 *
 * This is the main file for the scanorama program.  This program
 * (generate_scanorama) will form scanorama products 
 * using the imported imagery and geometry.
 */

using namespace std;

/*--------------------------*/
/* function implementations */
/*--------------------------*/

/**
 * The main function for this program
 */
int main(int argc, char** argv)
{
	generate_scanorama_run_settings_t args;
	scanorama_maker_t maker;
	tictoc_t clk;
	size_t i, n;
	int ret;

	/* parse the given parameters */
	ret = args.parse(argc, argv);
	if(ret)
	{
		cerr << "[main]\tError " << ret << ": "
		     << "Could not parse parameters" << endl;
		return 1;
	}

	/* initialize the maker object */
	tic(clk);
	ret = maker.init(args.pathfile, args.xml_config, args.modelfile);
	if(ret)
	{
		cerr << "[main]\tError " << ret << ": "
		     << "Could not initialize" << endl;
		return 2;
	}

	/* import all fisheye cameras that are given */
	n = args.fisheye_cam_metafiles.size();
	for(i = 0; i < n; i++)
	{
		/* add this camera */
		ret = maker.add_fisheye_camera(
				args.fisheye_cam_metafiles[i],
				args.fisheye_cam_calibfiles[i],
				args.fisheye_cam_imgdirs[i]);
		if(ret)
		{
			cerr << "[main]\tError " << ret << ": "
			     << "Could not add fisheye camera #" 
			     << i << endl;
			return 3;
		}
	}

	/* import all rectilinear cameras */
	n = args.rectilinear_cam_metafiles.size();
	for(i = 0; i < n; i++)
	{
		/* add this camera */
		ret = maker.add_rectilinear_camera(
				args.rectilinear_cam_metafiles[i],
				args.rectilinear_cam_calibfiles[i],
				args.rectilinear_cam_imgdirs[i]);
		if(ret)
		{
			cerr << "[main]\tError " << ret << ": "
			     << "Could not add rectilinear camera #" 
			     << i << endl;
			return 3;
		}
	}

	toc(clk, "Initialization");

	/* export the scans */
	ret = maker.generate_along_path(args.ptx_outfile, args.spacing_dist,
			args.num_rows, args.num_cols, args.blendwidth,
			args.begin_idx, args.end_idx);
	if(ret)
	{
		cerr << "[main]\tError " << ret << ": "
		     << "Unable to generate scanoramas" << endl;
		return 4;
	}

	/* success */
	return 0;
}

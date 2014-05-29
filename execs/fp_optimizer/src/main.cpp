#include "fpopt_run_settings.h"
#include <mesh/optimize/fp_optimizer.h>
#include <iostream>

/**
 * @file main.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 *
 * @section DESCRIPTION
 *
 * This is the main file for the octree surface reconstruction program
 * (fpopt) which will generate a surface reconstruction of a building
 * interior environment from an octree generated by procarve.
 */

using namespace std;

/* function implementations */

/**
 * The main function for this program
 */
int main(int argc, char** argv)
{
	fpopt_run_settings_t args;
	fp_optimizer_t opt;
	int ret;

	/* parse the given parameters */
	ret = args.parse(argc, argv);
	if(ret)
	{
		cerr << "[main]\tError " << ret << ": "
		     << "Could not parse parameters" << endl;
		return 1;
	}

	/* initialize the optimizer */
	opt.init(args.num_iterations, args.search_range,
	         args.offset_step_coeff);

	/* optimize all given floorplans using the specified octree */
	ret = opt.process_all(args.octfile, 
	                      args.input_fpfiles, args.output_fpfiles);
	if(ret)
	{
		cerr << "[main]\tError " << ret << ": "
		     << "Unable to process floorplans" << endl;
		return 2;
	}

	/* success */
	return 0;
}

#include <iostream>
#include <string>

/**
 * @file main.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 *
 * @section DESCRIPTION
 *
 * This is the main file for the probability-carving program (procarve),
 * which will generate a surface reconstruction of a building interior
 * environment from range scans from a mobile scanning system.
 */

using namespace std;

/* function implementations */

#include <geometry/carve/random_carver.h>

/**
 * The main function for this program
 */
int main()
{
	random_carver_t carver;
	int ret;
	
	/* set input files */
	string dataset = "/home/elturner/Desktop/data/20131204-16/";
	string confile = dataset + "config/backpack_config.xml";
	string madfile = dataset + "Localization/Magneto_TEST_OL_LC_3D.mad";
	string fssfile = dataset + "data/urg/H1214157/urg_H1214157_scandata.fss";
	string dimfile = dataset + "data/d_imager/d_imager_scandata.fss";
	string fpfile  = dataset + "models/floorplan/Magneto_TEST_OL_LC_3D_i40.fp";
	string octfile = dataset + "models/carving/testcarve.oct";

	/* initialize */
	ret = carver.init(madfile, confile, 0.05, 0.003, 2);
	if(ret)
	{
		cerr << "Unable to init carver: " << ret << endl;
		return 1;
	}

	/* process */
	ret = carver.carve(fssfile);
	if(ret)
	{
		cerr << "unable to carve urg: " << ret << endl;
		return 2;
	}

	/* import some floorplans */
	ret = carver.import_fp(fpfile);
	if(ret)
	{
		cerr << "unable to import floorplan: " << ret << endl;
		return 3;
	}

	/* export */
	ret = carver.serialize(octfile);
	if(ret)
	{
		cerr << "unable to export: " << ret << endl;
		return 4;
	}

	/* success */
	return 0;
}

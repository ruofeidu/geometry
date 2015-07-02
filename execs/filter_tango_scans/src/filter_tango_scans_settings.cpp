#include "filter_tango_scans_settings.h"
#include <util/cmd_args.h>
#include <util/tictoc.h>
#include <util/error_codes.h>
#include <iostream>
#include <string>
#include <vector>

/**
 * @file   filter_tango_scans_settings.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Gets user-defined run settings for filter_tango_scans program
 *
 * @section DESCRIPTION
 *
 * This file contains classes used to parse and store
 * user-defined run parameters and settings for the
 * filter_tango_scans program.  This is a wrapper class around
 * cmd_args_t, which is used to parse command-line
 * arguments.
 */

using namespace std;

/* the command-line flags to check for */

#define BEGIN_IDX_FLAG   "-b"
#define END_IDX_FLAG     "-e"

/* the file types to check for */

#define FILE_EXT_DAT     "dat"
#define FILE_EXT_MAD     "mad"
#define FILE_EXT_FSS     "fss"

/*--------------------------*/
/* function implementations */
/*--------------------------*/
		
filter_tango_scans_run_settings_t::filter_tango_scans_run_settings_t()
{
	/* initialize fields to default values */
	this->tangofile        = "";
	this->begin_idx        = 0;
	this->end_idx          = -1;
	this->fss_outfile      = "";
	this->mad_outfile      = "";
}

int filter_tango_scans_run_settings_t::parse(int argc, char** argv)
{
	cmd_args_t args;
	vector<string> files;
	tictoc_t clk;
	int ret;

	/* populate args with what we expect on the command-line */
	tic(clk);
	args.set_program_description("This program converts .dat files "
			"generated during acquisition on the Google Tango "
			"to sensor-agnostic file formats.  Both .fss and "
			".mad files can be extracted from the input .dat "
			"file.");
	args.add(BEGIN_IDX_FLAG, "If specified, then only the subset of"
			" frames starting at this index (inclusive) "
			"will be exported.", true, 1);
	args.add(END_IDX_FLAG, "If specified, then only the subset of "
			"frames before this index (exclusive) will be "
			"exported.", true, 1);
	args.add_required_file_type(FILE_EXT_DAT, 1,
			"The input .dat file generated by the google "
			"tango program.  This file contains both pose "
			"and scan information.");
	args.add_required_file_type(FILE_EXT_MAD, 0,
			"The output .mad file, which represents the "
			"path observed by the tango.  This path tracks "
			"the position of the tango fisheye camera.");
	args.add_required_file_type(FILE_EXT_FSS, 0,
			"The output .fss file, which represents the "
			"scan frames generated by the google tango.");

	/* parse the command-line arguments */
	ret = args.parse(argc, argv);
	if(ret)
	{
		/* unable to parse command-line arguments.  Inform
		 * user and quit. */
		ret = PROPEGATE_ERROR(-1, ret);
		cerr << "[filter_tango_scans_run_settings_t::parse]\t"
		     << "Unable to parse command-line arguments:  "
		     << "Error " << ret << endl;
		return ret;
	}

	/* retrieve specified input file names */
	files.clear(); args.files_of_type(FILE_EXT_DAT, files);
	if(files.size() > 1)
	{
		cerr << "[filter_tango_scans_run_settings::parse]\t"
		     << "Multiple input (.dat) files given, only the "
		     << "first will be parsed." << endl;
	}
	this->tangofile = files[0];

	/* retrieve specified output path file names */
	files.clear(); args.files_of_type(FILE_EXT_MAD, files);
	if(files.empty())
		this->mad_outfile = "";
	else
	{
		/* check for multiple files */
		if(files.size() > 1)
		{
			cerr << "[filter_tango_scans_run_settings::parse]\t"
			     << "Multiple output path (.mad) files given, "
			     << "only the first will be generated." << endl;
		}
		this->mad_outfile = files[0];
	}
	
	/* retrieve specified output scan file names */
	files.clear(); args.files_of_type(FILE_EXT_FSS, files);
	if(files.empty())
		this->fss_outfile = "";
	else
	{
		/* check for multiple files */
		if(files.size() > 1)
		{
			cerr << "[filter_tango_scans_run_settings::parse]\t"
			     << "Multiple output scan (.fss) files given, "
			     << "only the first will be generated." << endl;
		}
		this->fss_outfile = files[0];
	}

	/* check that at least one outfile was specified */
	if(this->mad_outfile.empty() && this->fss_outfile.empty())
	{
		cerr << "[filter_tango_scans_run_settings::parse]\t"
		     << "No output file is specified.  This call won't "
		     << "do anything." << endl;
		return -2;
	}

	/* get the optional arguments */
	if(args.tag_seen(BEGIN_IDX_FLAG))
		this->begin_idx = args.get_val_as<int>(BEGIN_IDX_FLAG);
	else
		this->begin_idx = 0;
	if(args.tag_seen(END_IDX_FLAG))
		this->end_idx = args.get_val_as<int>(END_IDX_FLAG);
	else
		this->end_idx = -1;

	/* we successfully populated this structure, so return */
	toc(clk, "Importing settings");
	return 0;
}

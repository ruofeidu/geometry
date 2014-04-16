#ifndef OCTSURF_RUN_SETTINGS_H
#define OCTSURF_RUN_SETTINGS_H

/**
 * @file   octsurf_run_settings.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Gets user-defined run settings for octsurf program
 *
 * @section DESCRIPTION
 *
 * This file contains classes used to parse and store
 * user-defined run parameters and settings for the 
 * octsurf program.  This is a wrapper class around
 * cmd_args_t, which is used to parse command-line
 * arguments.
 */

#include <string>
#include <vector>

/**
 * This class is used to store run settings for the octsurf program
 */
class octsurf_run_settings_t
{
	/* parameters */
	public:

		/* the following files are necessary for
		 * this program */

		/**
		 * Location of the input .oct files
		 */
		std::vector<std::string> octfiles;

		/**
		 * Location of the output file
		 *
		 * This program supports many different output filetypes,
		 * which are specified by the file extension of this
		 * given file path.
		 */
		std::string outfile;

	/* functions */
	public:

		/**
		 * Creates an empty object
		 */
		octsurf_run_settings_t();

		/**
		 * Parses settings from command-line
		 *
		 * Will parse the command-line arguments to get all
		 * the necessary settings.  This may also include
		 * parsing xml settings files that were passed on
		 * the command-line.
		 *
		 * @param argc  Number of command-line arguments
		 * @param argv  The command-line arguments given to main()
		 *
		 * @return      Returns zero on success, non-zero on failure
		 */
		int parse(int argc, char** argv);
};

#endif
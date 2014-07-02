#!/usr/bin/python

##
# @file chunker.py
# @author Eric Turner <elturner@eecs.berkeley.edu>
# @brief  Calls the scan_chunker program on a dataset
#
# @section DESCRIPTION
#
# Will call the scan_chunker program on a dataset, verifying that all
# input files exist, and that the output directories are valid.
#

# Import standard python libraries
import os
import sys
import shutil
import subprocess

# Get the location of this file
SCRIPT_LOCATION = os.path.dirname(__file__)

# Import our python files
sys.path.append(os.path.join(SCRIPT_LOCATION, '..', 'config'))
sys.path.append(os.path.join(SCRIPT_LOCATION, '..', 'files'))
import config
import dataset_filepaths

# the following indicates the expected location of the scan_chunker
# executable file
CHUNKER_EXE = os.path.abspath(os.path.join(SCRIPT_LOCATION, \
		'..', '..', '..', 'bin', 'scan_chunker'))

# the following indicates the expected location of the settings xml
# file used for the scan_chunker program
SETTINGS_XML = os.path.abspath(os.path.join(SCRIPT_LOCATION, \
		'..', '..', '..', 'config', 'procarve', \
		'procarve_settings.xml'))

##
# The main function of this script, which is used to run scan_chunker
#
# This call will run the scan_chunker program, verifying inputs and
# outputs.
#
# @param dataset_dir  The path to the dataset to process
#
# @return             Returns zero on success, non-zero on failure
def run(dataset_dir):
	
	# determine the expected location of necessary files from
	# within the dataset
	cmfile    = dataset_filepaths.get_carvemap_file(dataset_dir)
	wedgefile = dataset_filepaths.get_wedgefile(dataset_dir)
	chunklist = dataset_filepaths.get_chunklist(dataset_dir)

	# parse the settings file to get the location of the output
	# directory that will house the chunk files
	settings = config.parse_settings_xml(SETTINGS_XML)
	if settings is None:
		print "Error!  Invalid settings file:",SETTINGS_XML
		return -1
	
	# retrieve output directory for chunks
	rel_chunkdir = settings["procarve_chunkdir"]
	chunkdir = os.path.normpath(os.path.join(os.path.dirname( \
			chunklist), rel_chunkdir))

	# check that output directory exists
	if not os.path.exists(chunkdir):
		os.makedirs(chunkdir)
	elif not os.path.isdir(chunkdir):
		print "Error! Output directory is not a directory:",chunkdir
		return -2

	# check that output directory is empty of any .chunk files
	for f in os.listdir(chunkdir):	
		shutil.rmtree(os.path.join(chunkdir, f))

	# prepare the command-line arguments for the chunker code
	args = [CHUNKER_EXE, '-m', cmfile, '-w', wedgefile, \
		'-o', chunklist, '-s', SETTINGS_XML]

	# run the chunker code
	ret = subprocess.call(args, executable=CHUNKER_EXE, \
		cwd=dataset_dir, stdout=None, stderr=None, \
		stdin=None, shell=False)
	if ret != 0:
		print "chunker program returned error",ret
		return -3
	
	# success
	return 0

##
# The main function
#
# This will call the run() function using command-line arguments
#
def main():

	# check command-line arguments
	if len(sys.argv) != 2:
		print ""
		print " Usage:"
		print ""
		print "\t",sys.argv[0],"<path_to_dataset>"
		print ""
		sys.exit(1)

	# run this script with the given arguments
	ret = run(sys.argv[1])
	sys.exit(ret)

##
# Boilerplate code to call main function when used as executable
#
if __name__ == '__main__':
	main()

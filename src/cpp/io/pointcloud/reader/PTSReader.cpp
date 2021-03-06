/*
	PTSReader.cpp

	This class serves as an implementation of the PointCloudReaderImpl for
	reading PTS files.

	The ASCII PTS file format is defined where each point is its own line and 
	follows the form of :

		X Y Z . . . R G B 
*/
#include "PTSReader.h"

/* includes */
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include "PointCloudReader.h"

/* namespaces */
using namespace std;

/* function definitions */
/*
*	The open function.
*
*	This function performs all needed tasks to get the input file ready
*	for reading.  
*
*	Returns true on success and false on error.
*
*	After this function is called, the input file should begin to accept
*	calls to the read_point function.
*/
bool PTSReader::open(const std::string& input_file_name)
{
	/* check if we area already open and then close */
	if(this->is_open())
		this->close();

	/* open the file and return the stream state as the return code */
	_inStream.open(input_file_name);
	return this->is_open();
}

/*
*	The close function.
*
*	This function performs all the needed tasks for closing out the input
*	stream.  
*
*	After this function is called the class should not accept any more 
*	requests to read points	
*/
void PTSReader::close()
{
	_inStream.close();
}

/*
*	Checks if the input file is open and ready to receive read requests 
*
*	Returns true if the input file can receive points for reading and
*	false if it can not.
*/
bool PTSReader::is_open() const
{
	return _inStream.is_open();
}

/*
*	Checks if a specific attribute will be validly returned by the reader
*	
*	Returns true if the attribute is supported and false if it is not
*/
bool PTSReader::supports_attribute(PointCloudReaderImpl::POINT_ATTRIBUTES attribute) const
{
	switch(attribute)
	{
		case POSITION:
		case COLOR:
			return true;
		case POINT_INDEX:
		case TIMESTAMP:
			return false;
	}
	return false;
}

/*
*	The read point function.  
*
*	This is the main workhorse of the class. This function should read the
*	next point from the file and return the relevant data in the passed
*	references
*
*	Which values are actually supported depend on the file type being read
*
*	This function will return true if a point was successfully read from
*	the file and false if a point is unable to be read from the file.
*/
bool PTSReader::read_point(double& x, double& y, double& z,
	unsigned char& r, unsigned char& g, unsigned char& b,
	int& index, double& timestamp)
{
	/* check if the stream can be read from */
	if(!this->is_open())
		return false;

	/* attempt to read a line */
	string line;
	while(_inStream.good() && line.empty())
	{
		getline(_inStream, line);
		if(line.empty())
			continue;
	}
	
	/* we hit the end of the file without getting any lines to read */
	if(line.empty())
		return false;
	
	/* extract the data from the string */
	/* we need to tokenize the string because the PTS format has the position */
	/* up front and the color in the rear */
	stringstream ss(line);
	vector<string> tokens;
	string tok;
	while(ss >> tok)
		tokens.push_back(tok);

	/* check if this has at least 3 elements */
	if(tokens.size() < 3)
		return false;

	/* extract the position */
	ss.str(tokens[0]);
	ss.clear();
	if(!(ss >> x))
		return false;
	ss.str(tokens[1]);
	ss.clear();
	if(!(ss >> y))
		return false;
	ss.str(tokens[2]);
	ss.clear();
	if(!(ss >> z))
		return false;

	/* check if color was given and if so we should copy it out */
	if(tokens.size() >= 6)
	{
		unsigned int color;
		ss.str(tokens[tokens.size()-3]);
		ss.clear();
		if(!(ss >> color))
			return false;
		else
			r = (unsigned char)color;
		ss.str(tokens[tokens.size()-2]);
		ss.clear();
		if(!(ss >> color))
			return false;
		else
			g = (unsigned char)color;
		ss.str(tokens[tokens.size()-1]);
		ss.clear();
		if(!(ss >> color))
			return false;
		else
			b = (unsigned char)color;
	}
	else
	{
		r = g = b = 0;
	}

	/* set other things not supported to zeros */
	timestamp = 0;
	index = 0;

	/* return success */
	return true;
}
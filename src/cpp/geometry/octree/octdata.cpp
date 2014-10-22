#include "octdata.h"
#include <algorithm>
#include <cmath>
#include <stdlib.h>

/**
 * @file octdata.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 *
 * @section DESCRIPTION
 *
 * This file contains implementations for the octdata_t class.  This
 * implimentation of the class stores distribution estimates of
 * probabilistic carvings of each octnode, as well as intersection
 * information from imported floorplans.
 */

using namespace std;
	
/* octdata_t function implementations */

octdata_t::octdata_t()
{
	/* initialize empty octdata object
	 * with no samples */
	this->count       = 0; /* no samples yet */
	this->prob_sum    = 0;
	this->prob_sum_sq = 0;
	this->surface_sum = 0;
	this->corner_sum  = 0;
	this->planar_sum  = 0;
	this->fp_room     = -1; /* no room yet */
	this->is_carved   = false; /* no deterministic carve */
}
		
octdata_t::octdata_t(double prob_samp, double surface_samp,
                     double corner_samp, double planar_samp)
{
	/* initialize octdata object with a single scan sample */
	this->count       = 1; /* just one sample so far */
	this->prob_sum    = prob_samp;
	this->prob_sum_sq = (prob_samp * prob_samp);
	this->surface_sum = surface_samp; 
	this->corner_sum  = corner_samp;
	this->planar_sum  = planar_samp;
	this->fp_room     = -1; /* no room yet */
	this->is_carved   = false; /* not deterministic carve */
}
		
octdata_t::~octdata_t()
{
	/* no explicit calls necessary to free data */
}
		
void octdata_t::merge(octdata_t* p)
{
	/* check validity of input */
	if(p == NULL)
		return; /* no merging with nonexistant data */

	/* merge p's elements with this object */
	this->count       += p->count; /* combine samples */
	this->prob_sum    += p->prob_sum;
	this->prob_sum_sq += p->prob_sum_sq;
	this->surface_sum += p->surface_sum;
	this->corner_sum  += p->corner_sum;
	this->planar_sum  += p->planar_sum;
	this->fp_room     = max(this->fp_room, p->fp_room); /* prefers
	                                                     * valid labels
	                                                     */
	this->is_carved   |= p->is_carved; /* if either is carved, result
	                                    * is carved */
}

octdata_t* octdata_t::clone() const
{
	octdata_t* c;

	/* allocate new memory for data */
	c = new octdata_t();
	
	/* populate new data with current data */
	c->count       = this->count;
	c->prob_sum    = this->prob_sum;
	c->prob_sum_sq = this->prob_sum_sq;
	c->surface_sum = this->surface_sum;
	c->corner_sum  = this->corner_sum;
	c->planar_sum  = this->planar_sum;
	c->fp_room     = this->fp_room;
	c->is_carved   = this->is_carved;

	/* return new data */
	return c;
}
		
void octdata_t::subdivide(unsigned int n)
{
	unsigned int newcount;
	double ratio;

	/* check arguments */
	if(n <= 1 || this->count == 0)
		return; /* do nothing */

	/* get the new count after subdivision */
	newcount = std::max((unsigned int) 1, this->count / n);

	/* compute ratio. This will account for integer division
	 * issues.  We want most of all to keep the ratios of the
	 * floating point values to be the same */
	ratio = ((double) newcount) / ((double) this->count);

	/* update other values */
	this->count        = newcount;
	this->prob_sum    *= ratio;
	this->prob_sum_sq *= ratio;
	this->surface_sum *= ratio;
	this->corner_sum  *= ratio;
	this->planar_sum  *= ratio;
	/* other values don't change */
}

void octdata_t::serialize(ostream& os) const
{
	/* write data to stream in little-endian
	 * binary notation */
	os.write((char*) &(this->count),       sizeof(this->count));
	os.write((char*) &(this->prob_sum),    sizeof(this->prob_sum));
	os.write((char*) &(this->prob_sum_sq), sizeof(this->prob_sum_sq));
	os.write((char*) &(this->surface_sum), sizeof(this->surface_sum));
	os.write((char*) &(this->corner_sum),  sizeof(this->corner_sum));
	os.write((char*) &(this->planar_sum),  sizeof(this->planar_sum));
	os.write((char*) &(this->fp_room),     sizeof(this->fp_room));
}

int octdata_t::parse(istream& is)
{
	/* read data from stream in little-endian
	 * binary notation */
	is.read((char*) &(this->count),       sizeof(this->count));
	is.read((char*) &(this->prob_sum),    sizeof(this->prob_sum));
	is.read((char*) &(this->prob_sum_sq), sizeof(this->prob_sum_sq));
	is.read((char*) &(this->surface_sum), sizeof(this->surface_sum));
	is.read((char*) &(this->corner_sum),  sizeof(this->corner_sum));
	is.read((char*) &(this->planar_sum),  sizeof(this->planar_sum));
	is.read((char*) &(this->fp_room),     sizeof(this->fp_room));

	/* check that parsing was successful */
	if(is.bad())
		return -1; /* failure */
	return 0; /* success */
}
		
void octdata_t::add_sample(double prob, double surf, double corner, 
                           double planar)
{
	this->count       ++; /* update observation count */
	this->prob_sum    += prob; /* add probability observation */
	this->prob_sum_sq += prob*prob; /* square of probability sample */
	this->surface_sum += surf; /* surface point probability obs. */
	this->corner_sum  += corner; /* add corner coefficient obs. */
	this->planar_sum  += planar; /* add planarity observation */
}

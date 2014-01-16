#include "simplify_graph.h"
#include "../structs/cell_graph.h"
#include "../rooms/tri_rep.h"
#include "../util/tictoc.h"
#include "../util/room_parameters.h"
#include "../util/error_codes.h"

int simplify_graph(cell_graph_t& graph, tri_rep_t& trirep, double thresh)
{
	tictoc_t clk;
	int ret;

	/* check valid threshold */
	if(thresh < 0)
		return 0; /* do nothing */

	/* simplify graph using error quadrics */
	tic(clk);
	ret = graph.simplify(trirep, thresh);
	if(ret)
		return PROPEGATE_ERROR(-1, ret);

	/* straighten walls */
	ret = graph.simplify_straights(trirep);
	if(ret)
		return PROPEGATE_ERROR(-2, ret);

	/* remove unnecessary unions */
	// graph.remove_unions_below(MIN_ROOM_PERIMETER);
	ret = trirep.remove_interroom_columns(MIN_ROOM_PERIMETER);
	if(ret)
		return PROPEGATE_ERROR(-3, ret);
	toc(clk, "Simplifying model");

	/* verify that triangulation topology still valid */
	tic(clk);
	ret = trirep.verify();
	if(ret)
		return PROPEGATE_ERROR(-4, ret);
	toc(clk, "Verifying topology");

	/* success */
	return 0;
}
/*------------------------------------------------------------------------------*
 *                       (c)2014, All Rights Reserved.     						*
 *       ___           ___           ___     									*
 *      /__/\         /  /\         /  /\    									*
 *      \  \:\       /  /:/        /  /::\   									*
 *       \  \:\     /  /:/        /  /:/\:\  									*
 *   ___  \  \:\   /  /:/  ___   /  /:/~/:/        								*
 *  /__/\  \__\:\ /__/:/  /  /\ /__/:/ /:/___     UCR DMFB Synthesis Framework  *
 *  \  \:\ /  /:/ \  \:\ /  /:/ \  \:\/:::::/     www.microfluidics.cs.ucr.edu	*
 *   \  \:\  /:/   \  \:\  /:/   \  \::/~~~~ 									*
 *    \  \:\/:/     \  \:\/:/     \  \:\     									*
 *     \  \::/       \  \::/       \  \:\    									*
 *      \__\/         \__\/         \__\/    									*
 *-----------------------------------------------------------------------------*/
/*------------------------------Algorithm Details-------------------------------*
 * Type: Router																	*
 * Name: Fixed Place Router														*
 *																				*
 * Not inferred or detailed in any publications.								*
 * 																				*
 * Details: This router depends on the fixed placer (grissom_fixed_placer.h) to *
 * be used as the placer. It uses this knowledge to know where routing cells	*
 * are and generates routes that take the open routing cells.  Paths are 		*
 * deterministic.																*
 *-----------------------------------------------------------------------------*/
#ifndef DEMO_COMPACTION_ROUTER_H_
#define DEMO_COMPACTION_ROUTER_H_

#include "../Testing/elapsed_timer.h"
#include "post_subprob_compact_router.h"

struct RotuingPoint;

class GrissomFixedPlaceRouter : public PostSubproblemCompactionRouter
{
	public:
		// Constructors
		GrissomFixedPlaceRouter();
		GrissomFixedPlaceRouter(DmfbArch *dmfbArch);
		virtual ~GrissomFixedPlaceRouter();

	protected:
		void computeIndivSupProbRoutes(vector<vector<RoutePoint *> *> *subRoutes, vector<Droplet *> *subDrops, map<Droplet *, vector<RoutePoint *> *> *routes);
};



#endif /* DEMO_COMPACTION_ROUTER_H_ */

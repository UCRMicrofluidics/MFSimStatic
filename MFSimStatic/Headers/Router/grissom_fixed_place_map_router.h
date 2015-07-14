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
 * Name: Fixed Place Map Router													*
 *																				*
 * Not inferred or detailed in any publications.								*
 * 																				*
 * Details: This is the same general algorithm detailed							*
 * in grissom_fixed_place_router.h/cc, but instead of computing the routes each	*
 * routing phase, all possible routes are generated before routing begins and	*
 * stored in a library for later use. This router depends on the fixed placer	*
 * (grissom_fixed_placer.h) to be used as the placer. Paths are deterministic.	*
 *-----------------------------------------------------------------------------*/

#ifndef DEMO_MAP_ROUTER_H_
#define DEMO_MAP_ROUTER_H_

#include "post_subprob_compact_router.h"
#include "../Testing/elapsed_timer.h"


struct RotuingPoint;

class GrissomFixedPlaceMapRouter : public PostSubproblemCompactionRouter
{
	public:
		// Constructors
		GrissomFixedPlaceMapRouter();
		GrissomFixedPlaceMapRouter(DmfbArch *dmfbArch);
		virtual ~GrissomFixedPlaceMapRouter();

		// Methods
		void preRoute(DmfbArch *arch);

	protected:
		void computeIndivSupProbRoutes(vector<vector<RoutePoint *> *> *subRoutes, vector<Droplet *> *subDrops, map<Droplet *, vector<RoutePoint *> *> *routes);

	private:
		// Members
		map<string, vector<pair<int, int> *> *> *routeMap;

		// Methods
};


#endif /* DEMO_MAP_ROUTER_H_ */

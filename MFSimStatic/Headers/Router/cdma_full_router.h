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
 * Name: CDMARouter																*
 *																				*
 * Inferred from the following paper:											*
 * Authors: Kamalesh Singha, Tuhina Samanta, Hafizur Rahaman, and				*
 * Partharasi Dasgupta:															*
 * Method of droplet routing in digital microfluidic biochip. MESA 2012: 251-25	*
 *-----------------------------------------------------------------------------*/

#ifndef HEADERS_ROUTER_CDMA_FULL_ROUTER_H_
#define HEADERS_ROUTER_CDMA_FULL_ROUTER_H_

#include "post_subprob_compact_router.h"

#include "router.h"
#include "lee_router.h"


class CDMAFullRouter : public PostSubproblemCompactionRouter
{
	public:
		// Constructors
		CDMAFullRouter();
		CDMAFullRouter(DmfbArch *dmfbArch);
		virtual ~CDMAFullRouter();

	protected:
		void computeIndivSupProbRoutes(vector<vector<RoutePoint *> *> *subRoutes, vector<Droplet *> *subDrops, map<Droplet *, vector<RoutePoint *> *> *routes);
		void routerSpecificInits();

	private:
		void processNodes();
		// Members
		vector<vector<LeeCell *> *> *board;
};

#endif /* HEADERS_ROUTER_CDMA_FULL_ROUTER_H_ */

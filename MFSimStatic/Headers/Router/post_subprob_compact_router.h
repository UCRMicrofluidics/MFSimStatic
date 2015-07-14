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
 * Name: Post SubProblem Compaction Base Router									*
 *																				*
 * Not inferred or detailed in any publications									*
 * 																				*
 * Details: This class is the sub-class to use for any router which computes	*
 * individual routes for a sub-problem and THEN compacts the sub-problem. This	*
 * is opposed to routers that coordinate multiple droplet movements 			*
 * concurrently. The main structure is already provided such that the			*
 * programmer should only need to re-implement computIndivSubProbRoutes().		*
 *-----------------------------------------------------------------------------*/

#ifndef POST_SUBPROB_COMPACT_ROUTER_H_
#define POST_SUBPROB_COMPACT_ROUTER_H_

#include "../Testing/elapsed_timer.h"
#include "router.h"

struct RotuingPoint;

class PostSubproblemCompactionRouter : public Router
{
	public:
		// Constructors
		PostSubproblemCompactionRouter();
		PostSubproblemCompactionRouter(DmfbArch *dmfbArch);
		virtual ~PostSubproblemCompactionRouter();

		// Methods
		void route(DAG *dag, DmfbArch *arch, vector<ReconfigModule *> *rModules, map<Droplet *, vector<RoutePoint *> *> *routes, vector<vector<int> *> *pinActivations, vector<unsigned long long> *tsBeginningCycle);

	protected:
		// Methods - General
		bool rpInterferesWithRpList(RoutePoint *rp, map<Droplet *, RoutePoint *> *rps, Droplet *d);
		bool rpInterferesWithPersistentModule(RoutePoint *rp);
		bool rpInterferesWithPotentialIoLocations(RoutePoint *rp);
		void eliminateSubRouteDependencies(map<Droplet *, vector<RoutePoint *> *> *routes);
		virtual void computeIndivSupProbRoutes(vector<vector<RoutePoint *> *> *subRoutes, vector<Droplet *> *subDrops, map<Droplet *, vector<RoutePoint *> *> *routes);
		void addSubProbToGlobalRoutes(vector<Droplet *> *subDrops, vector<vector<RoutePoint *> *> *subRoutes, map<Droplet *, vector<RoutePoint *> *> *routes);
		void equalizeGlobalRoutes(map<Droplet *, vector<RoutePoint *> *> *routes);
		virtual void routerSpecificInits();
		void initCellTypeArray();
		void printSubRoutes(vector<vector<RoutePoint *> *> *subRoutes, vector<Droplet *> *subDrops);
		void debugPrintSourceTargetPair(Droplet *d, map<Droplet *, RoutePoint *> *sourceCells, map<Droplet *, RoutePoint *> *targetCells);

		void printNumIntersections(vector<vector<RoutePoint *> *> *subRoutes, DmfbArch *arch);

		// Methods - Droplet/Operation Processing
		void processTimeStep(map<Droplet *, vector<RoutePoint *> *> *routes);
		void processFixedPlaceTS(map<Droplet *, vector<RoutePoint *> *> *routes);
		void processFreePlaceTS(map<Droplet *, vector<RoutePoint *> *> *routes);
		void mergeNodeDroplets(AssayNode *n, map<Droplet *, vector<RoutePoint *> *> *routes);
		Droplet * mergeDroplets(Droplet *d1, Droplet *d2);
		string mergeFluidNames(string n1, string n2);
		string splitFluidNames(string n1, int numSplitDrops);

		RoutePoint * advanceDropInLinMod(RoutePoint *lrp, ReconfigModule *rm, bool *travelingRight, bool *travelingDown);
		RoutePoint * advanceDropIn2ByMod(RoutePoint *lrp, ReconfigModule *rm);
		RoutePoint * advanceDropIn3By3PlusMod(RoutePoint *lrp, ReconfigModule *rm, bool *travelingRight, bool *travelingDown);


		// Members
		vector<vector<ResourceType> *> *cellType;
		vector<AssayNode *> *thisTS;
};


#endif /* POST_SUBPROB_COMPACT_ROUTER_H_ */

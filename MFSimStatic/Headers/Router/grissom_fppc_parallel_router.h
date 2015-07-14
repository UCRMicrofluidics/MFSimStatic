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
 * Name: Parallel Router for Field-Programmable Pin-Constrained Design			*
 *																				*
 * Detailed in the following paper:												*
 * Authors: Dan Grissom and Philip Brisk										*
 * Title: Field Programmable, Pin-Constrained Digital Microfluidic Biochips		*
 * Publication Details: Submitted to TCAD 2014									*
 * 																				*
 * Details: Computes parallel pin activations and converts to droplet routes	*
 * for the field-programmable pin-contrained design.							*
 *-----------------------------------------------------------------------------*/

#ifndef GRISSOM_FPPC_PARALLEL_ROUTER_H
#define GRISSOM_FPPC_PARALLEL_ROUTER_H

#include "../PinMapper/grissom_fppc_pin_mapper.h"
#include "../Models/module_dependence_graph.h"
#include "../Testing/elapsed_timer.h"
#include "router.h"
#include <algorithm>
#include <set>

struct RotuingPoint;

class GrissomFppcParallelRouter : public Router
{
	public:
		// Constructors
		GrissomFppcParallelRouter();
		GrissomFppcParallelRouter(DmfbArch *dmfbArch);
		virtual ~GrissomFppcParallelRouter();

		// Methods
		void route(DAG *dag, DmfbArch *arch, vector<ReconfigModule *> *rModules, map<Droplet *, vector<RoutePoint *> *> *routes, vector<vector<int> *> *pinActivations, vector<unsigned long long> *tsBeginningCycle);
		bool getDropLocationsFromSim() { return true; } // True if droplet routes/locations are to be calculated after the routing stage; False if the locations have already been computed by the router

	protected:
		// Methods
		bool rpInterferesWithRpList(RoutePoint *rp, map<Droplet *, RoutePoint *> *rps, Droplet *d);
		void eliminateSubRouteDependencies(map<Droplet *, vector<RoutePoint *> *> *routes);
		void computeIndivSubProbRouteOffsets(vector<vector<RoutePoint *> *> *subRoutes, vector<Droplet *> *subDrops, map<Droplet *, vector<RoutePoint *> *> *routes);
		void computeIndivSubProbRoutes(vector<vector<RoutePoint *> *> *subRoutes, vector<Droplet *> *subDrops, map<Droplet *, vector<RoutePoint *> *> *routes);
		void addSubProbToGlobalRoutes(vector<Droplet *> *subDrops, vector<vector<RoutePoint *> *> *subRoutes, map<Droplet *, vector<RoutePoint *> *> *routes);
		void processFixPlaceTSQuick(map<Droplet *, vector<RoutePoint *> *> *routes);
		void processFixPlaceTSFull(map<Droplet *, vector<RoutePoint *> *> *routes);
		void equalizeGlobalRoutes(map<Droplet *, vector<RoutePoint *> *> *routes);
		void processTimeStep(map<Droplet *, vector<RoutePoint *> *> *routes);
		virtual void routerSpecificInits();
		void initCellTypeArray();

		// Routing Helper Methods
		int routeFromInputToCentralColumn(IoPort *inPort, int tx, int ty, vector<vector<RoutePoint *> *> *subRoutes);
		int extractDropletFromMixModule(ReconfigModule *rm, vector<vector<RoutePoint *> *> *subRoutes);
		int extractDropletFromSSDModule(ReconfigModule *rm, vector<vector<RoutePoint *> *> *subRoutes);
		void insertDropletIntoMixModule(ReconfigModule *rm, vector<vector<RoutePoint *> *> *subRoutes);
		void insertDropletIntoSSDModule(ReconfigModule *rm, vector<vector<RoutePoint *> *> *subRoutes);
		void splitNodeIntoSecondSSDModule(AssayNode *s, vector<vector<RoutePoint *> *> *subRoutes);
		int routeDropletAlongVerticalColumn(int startY, int endY, int x, vector<vector<RoutePoint *> *> *subRoutes);
		int routeDropletAlongHorizontalRow(int startX, int endX, int y, vector<vector<RoutePoint *> *> *subRoutes);

		int routeFromInputToCentralColumn(IoPort *inPort, int tx, int ty);
		int extractDropletFromMixModule(ReconfigModule *rm);
		int extractDropletFromSSDModule(ReconfigModule *rm);
		void insertDropletIntoMixModule(ReconfigModule *rm);
		void insertDropletIntoSSDModule(ReconfigModule *rm);
		void splitNodeIntoSecondSSDModule(AssayNode *s);
		int routeDropletAlongVerticalColumn(int startY, int endY, int x);
		int routeDropletAlongHorizontalRow(int startX, int endX, int y);

		void forcePinOffAtCycle(int pinNum, unsigned long long cycle);

		// Members
		int centralRoutingColumn;
		vector<vector<ResourceType> *> *cellType;
		vector<AssayNode *> *thisTS;
		vector<vector<int> *> *pinActs; // Just a proxy for the pinActivations passed in to computeIndivSupProbRoutes
		IoPort *washIn;
		IoPort *washOut;
		vector<int> *routeOffsets;
		map<unsigned long long, vector<int> *> *pinsForcedOffAtCycle; // Force the vector of pins off at the given cycle (key)
		vector<set<int> *> mixPairs; // Used during compaction to check which cells need to be cleaned
		vector<vector<vector<pair<int, int> > *> *> *dirtyArray; //3D array (x, y, t) to determine when cells are dirty and by what droplets
		vector<vector<vector<pair<int, int> > *> *> *dropletOrderArray; //3D array (x, y, dropNum) required ordering of functional (non-wash) droplets crossing a cell
		unsigned long long routingCyclesToOverlay;
		unsigned long long firstOverlayCycle;
		bool isInsertingAtOverlayCycle;

		GrissomFppcPinMapper *fppcPinMapper;
		vector<int> *mHoldPins;
		vector<int> *mIOPins;
		vector<int> *ssHoldPins;
		vector<int> *ssIOPins;
		vector<vector<int> *> *mixPins;
		vector<vector<int> *> *pinMapping;

		void addPinAtCycle(int pinNo, int cycleNo, bool activateHolds);

	private:
		RoutePoint *addNewRoutePoint(unsigned dropNum, int x, int y, vector<vector<RoutePoint *> *> *subRoutes);
		unsigned dropNum;
		void compactFppcRoutesWithBegStalls(vector<vector<RoutePoint *> *> *subRoutes);
		int preCompactionRouteLength;
		int postCompactionRouteLength;

};


#endif /* GRISSOM_FPPC_ROUTER_H */


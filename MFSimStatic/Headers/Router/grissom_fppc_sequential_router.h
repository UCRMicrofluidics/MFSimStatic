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
 * Name: Router for Field-Programmable Pin-Constrained Design					*
 *																				*
 * Detailed in the following paper:												*
 * Authors: Dan Grissom and Philip Brisk										*
 * Title: Field Programmable, Pin-Constrained Digital Microfluidic Biochips		*
 * Publication Details: Submitted to DAC 2013									*
 * 																				*
 * Details: Computes pin activations and converts to droplet routes for 		*
 * the field-programmable pin-contrained design.								*
 *-----------------------------------------------------------------------------*/

#ifndef GRISSOM_FPPC_SEQUENTIAL_ROUTER_H
#define GRISSOM_FPPC_SEQUENTIAL_ROUTER_H

#include "../PinMapper/grissom_fppc_pin_mapper.h"
#include "../Models/module_dependence_graph.h"
#include "../Testing/elapsed_timer.h"
#include "router.h"
#include <algorithm>

struct RotuingPoint;

class GrissomFppcSequentialRouter : public Router
{
	public:
		// Constructors
		GrissomFppcSequentialRouter();
		GrissomFppcSequentialRouter(DmfbArch *dmfbArch);
		virtual ~GrissomFppcSequentialRouter();

		// Methods
		void route(DAG *dag, DmfbArch *arch, vector<ReconfigModule *> *rModules, map<Droplet *, vector<RoutePoint *> *> *routes, vector<vector<int> *> *pinActivations, vector<unsigned long long> *tsBeginningCycle);
		bool getDropLocationsFromSim() { return true; } // True if droplet routes/locations are to be calculated after the routing stage; False if the locations have already been computed by the router

	protected:
		// Methods
		bool rpInterferesWithRpList(RoutePoint *rp, map<Droplet *, RoutePoint *> *rps, Droplet *d);
		void eliminateSubRouteDependencies(map<Droplet *, vector<RoutePoint *> *> *routes);
		void computeIndivSubProbRoutes(vector<vector<RoutePoint *> *> *subRoutes, vector<Droplet *> *subDrops, map<Droplet *, vector<RoutePoint *> *> *routes);
		void addSubProbToGlobalRoutes(vector<Droplet *> *subDrops, vector<vector<RoutePoint *> *> *subRoutes, map<Droplet *, vector<RoutePoint *> *> *routes);
		void processFixPlaceTSQuick(map<Droplet *, vector<RoutePoint *> *> *routes);
		void processFixPlaceTSFull(map<Droplet *, vector<RoutePoint *> *> *routes);
		void equalizeGlobalRoutes(map<Droplet *, vector<RoutePoint *> *> *routes);
		void processTimeStep(map<Droplet *, vector<RoutePoint *> *> *routes);
		virtual void routerSpecificInits();
		void initCellTypeArray();

		// Routing Helper Methods
		int routeFromInputToCentralColumn(IoPort *inPort, int tx, int ty);
		int extractDropletFromMixModule(ReconfigModule *rm);
		int extractDropletFromSSDModule(ReconfigModule *rm);
		void insertDropletIntoMixModule(ReconfigModule *rm);
		void insertDropletIntoSSDModule(ReconfigModule *rm);
		void splitNodeIntoSecondSSDModule(AssayNode *s);
		int routeDropletAlongVerticalColumn(int startY, int endY, int x);
		int routeDropletAlongHorizontalRow(int startX, int endX, int y);

		// Members
		int centralRoutingColumn;
		vector<vector<ResourceType> *> *cellType;
		vector<AssayNode *> *thisTS;
		vector<vector<int> *> *pinActs; // Just a proxy for the pinActivations passed in to computeIndivSupProbRoutes
		IoPort *washIn;
		IoPort *washOut;

		GrissomFppcPinMapper *fppcPinMapper;
		vector<int> *mHoldPins;
		vector<int> *mIOPins;
		vector<int> *ssHoldPins;
		vector<int> *ssIOPins;
		vector<vector<int> *> *mixPins;
		vector<vector<int> *> *pinMapping;

		void addPinAtCycle(int pinNo, int cycleNo, bool activateHolds);

};


#endif /* GRISSOM_FPPC_SEQUENTIAL_ROUTER_H */


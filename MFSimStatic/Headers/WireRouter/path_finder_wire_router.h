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
 * Type: Wire Router															*
 * Name: Path-finder Wire Router				 								*
 * 																				*
 * Detailed in the following paper:												*
 * Authors: Jeff McDaniel, Dan Grissom and Philip Brisk							*
 * Title: Multi-terminal PCB Escape Routing for Digital Microfluidic Biochips	*
 * using Negotiated Congestion													*
 * Publication Details: VLSI-SoC, Playa Del Carmen, Mexico, Oct 6-8, 2014		*
 * 																				*
 * Details: Uses a modified version of Path-Finder to compute routes between	*
 * pin groupings, and then between those groupings and an edge.	This is an		*
 * original DMFB algorithm.														*
 *-----------------------------------------------------------------------------*/
#ifndef PATH_FINDER_WIRE_ROUTER_H_
#define PATH_FINDER_WIRE_ROUTER_H_

#include "../Models/diagonal_wire_routing_model.h"
#include "../Models/min_cost_max_flow_graph.h"
#include "../Util/sort.h"
#include "wire_router.h"
#include "paths.h"
#include <queue>

#define kDebug 0
	// kLayerMinimization 1: on 0: off
	// Layer minimization will relax the pin constraint to allow multiple pins
	// constrained to the same wire to route off chip directly if it is cheaper.
#define kLayerMinimization 0
	// kCleanRoute 1: on 0: off
	// Clean Route will use the original method first to determine which routes fit
	// on the layer, then clear the history cost and re-route only those routes in
	// an attempt to clean up the routes. 
#define kCleanRoute 0
	// kCheaperDiagonalCost 1: on 0: off
	// Cheaper Diagonal Cost will make the diagonal edges of the tile cheaper
	// than the orthogonal edges. The goal is to even out the utilization of
	// the corners of the chip.
#define kCheaperDiagonalCost 0
	// kCheaperCornerTiles 1: on 0: off
	// Cheaper Corner tiles will make the corner tiles cheaper to route
	// through. The goal is to even out the utilization of the corners of the
	// chip.
#define kCheaperCornerTiles 0
#define kXTilesCheaper 3
#define kYTilesCheaper 3
#define kCenterTileCost 100

class PathFinderWireRouter : public WireRouter
{
	private:
		// Members
		int maxPinNum;
		vector<vector<Path*> > layers;

		// Constants
		double getKHFac() { return 1.0; } // Value taken from "Architecture and CAD for Deep-Submicron FPGA's", by Vaughn Betz
		double getKPFacIncrease() { return 1.5; } // Value taken from "Architecture and CAD for Deep-Submicron FPGA's", by Vaughn Betz
		unsigned getKMaxIterations() { return 30; } // Value taken from "Architecture and CAD for Deep-Submicron FPGA's", by Vaughn Betz
		int getFirstItPenalty() { return 10000; } // Arbitrary value chose to impose penalty for re-using nodes on first iteration
		bool isSavingBest() { return true; } // Saving best of the kMaxIterations? Or last (false)
		bool isReUsingPinsOnLowerLevels() { return true; } // Once pins routed on layer, pin area can be used for normal routing on lower layers

		// Methods
		void pathfinder(vector<WireRouteNode*> allNodes, map<int, vector<WireRouteNode*>* > pinGroups, WireRouteNode* super_escape,bool allowFails = false);
		void leeMazeRouting(int pin_number,double pfac,WireRouteNode* source, vector<WireRouteNode*> sinks, int iterationNum,Path* new_route);
		vector<WireRouteNode*> fillGrid(int pin_number,double pfac,Path* source, vector<WireRouteNode*>* sinks);
		int isSink(WireRouteNode* node,vector<WireRouteNode*>* sinks);
		bool traceBack(double pfac,WireRouteNode* sink,Path* sources, int iterationNum);
		void append(Path* original, vector<WireRouteNode*>* appended);
		WireRouteNode* findPrevious(WireRouteNode* current_node);
		void clearHistory(vector<WireRouteNode*>* allNodes);
		int historyCost(int old_history,int occupancy);
		void printPath(vector<WireRouteNode*>* path);
		void clearPaths(vector<Path*>* paths, vector<Path*>* best_layer);

		/* Tile Cost Helper function */
		void change_tile_costs(vector<WireRouteNode*>& allNodes);

	protected:
		// Members
		void layeredPathfinder(DiagonalWireRoutingModel* model);
		void convertWireSegments(vector<vector<Path*> >* layers,vector<vector<WireSegment*>*>* wires);

	public:
		// Constructors
		PathFinderWireRouter() {arch = NULL;}
		PathFinderWireRouter(DmfbArch *dmfbArch);
		virtual ~PathFinderWireRouter();

		// Methods
		vector<vector<Path*> >* getLayers() {return &layers;}
		void computeWireRoutes(vector<vector<int> *> *pinActivations, bool isIterative);
		//void computeWireRoutesTest();
};
#endif /* PATH_FINDER_WIRE_ROUTER_H_ */

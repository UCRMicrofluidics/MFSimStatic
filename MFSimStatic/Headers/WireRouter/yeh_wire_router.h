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
 * Authors: 																	*
 * Title: 																		*
 * Publication Details: 														*
 * 																				*
 * Details: Uses a modified version of Path-Finder to compute routes between	*
 * pin groupings, and then between those groupings and an edge.	This is an		*
 * original DMFB algorithm.														*
 *-----------------------------------------------------------------------------*/
#ifndef YEH_WIRE_ROUTER_H_
#define YEH_WIRE_ROUTER_H_

#include "../Models/diagonal_wire_routing_model.h"
#include "../Models/min_cost_max_flow_graph.h"
#include "../Util/sort.h"
#include "wire_router.h"
#include "paths.h"
#include <queue>

#define kDebug 0

class YehWireRouter : public WireRouter
{
	private:
		// Members
		int maxPinNum;
		vector<vector<Path*> > layers;

		// Constants
		double getKHFac() { return 1.0; } // Value taken from "Architecture and CAD for Deep-Submicron FPGA's", by Vaughn Betz
		//double getKPFacIncrease() { return 1.5; } // Value taken from "Architecture and CAD for Deep-Submicron FPGA's", by Vaughn Betz
		unsigned getKMaxIterations() { return 30; } // Value taken from "Architecture and CAD for Deep-Submicron FPGA's", by Vaughn Betz
		int getFirstItPenalty() { return 10000; } // Arbitrary value chose to impose penalty for re-using nodes on first iteration
		bool isSavingBest() { return true; } // Saving best of the kMaxIterations? Or last (false)
		bool isReUsingPinsOnLowerLevels() { return true; } // Once pins routed on layer, pin area can be used for normal routing on lower layers

		// Methods
		//vector<Path> yeh_iccad_routing(vector<WireRouteNode*> allNodes, map<int, vector<WireRouteNode*>* > pinGroups, WireRouteNode* super_escape);
		void yeh_iccad_routing(vector<WireRouteNode*> allNodes, map<int, vector<WireRouteNode*>* > pinGroups, WireRouteNode* super_escape);
		void changeSharing(vector<Path*>* paths,bool allowed);
		void rip_up_route(Path* ripped);

		vector<WireRouteNode*> fillGrid(int pin_number,double pfac,Path* source, vector<WireRouteNode*>* sinks);
		void leeMazeRouting(int pin_number,double pfac,WireRouteNode* source, vector<WireRouteNode*> sinks, int iterationNum,Path* new_path);
		void convertWireSegments(vector<vector<Path*> >* layers,vector<vector<WireSegment*>*>* wires);
		void layeredYeh(DiagonalWireRoutingModel* model);
		int isSink(WireRouteNode* node,vector<WireRouteNode*>* sinks);
		bool traceBack(double pfac,WireRouteNode* sink,Path* sources, int iterationNum);
		void append(Path* original, vector<WireRouteNode*>* appended);
		WireRouteNode* findPrevious(WireRouteNode* current_node);
		void clearHistory(vector<WireRouteNode*>* allNodes);
		vector<int> intersecting_pins(Path* new_path,vector<Path*>* old_paths);
		bool intersects(Path* new_path,vector<Path*>* old_paths);
		int historyCost(int old_history,int occupancy);
		void printPath(vector<WireRouteNode*>* path);

	public:
		// Constructors
		YehWireRouter(DmfbArch *dmfbArch);
		virtual ~YehWireRouter();

		// Methods
		void computeWireRoutes(vector<vector<int> *> *pinActivations);
};
#endif /* YEH_WIRE_ROUTER_H_ */

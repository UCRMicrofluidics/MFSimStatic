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
 * Name: Field Programmable, Pin-Constrained (Design #)2 Wire Router			*
 * 																				*
 * Detailed in the following paper:												*
 * Authors: 																	*
 * Title: 																		*
 * Publication Details: 														*
 * 																				*
 * Details: This algorithm has an explicit foreknowledge of the layout and		*
 * patterns used in the FPPC 2 design and generates a 1-layer wire-routing		*
 * by taking advantage of those facts.											*
 *-----------------------------------------------------------------------------*/
#ifndef GRISSOM_ENHANCED_WIRE_ROUTER_H_
#define GRISSOM_ENHANCED_WIRE_ROUTER_H_

#include "../Models/diagonal_wire_routing_model.h"
#include "wire_segment.h"
#include "wire_router.h"

class EnhancedFPPCWireRouter : public WireRouter
{
	private:
		// Members
		int maxPinNum;

		// Methods
		void routeMixingModules(vector<FixedModule *> *mixers, vector< vector<WireSegment *> *> *wires);
		void routeSSDModules(vector<FixedModule *> *ssdMods, vector< vector<WireSegment *> *> *wires);
		void routePinOptRoutingCells(int verticalColumnIndex, vector< vector<WireSegment *> *> *wires);
		void routeRouteOptRoutingCells(int verticalColumnIndex, vector< vector<WireSegment *> *> *wires);

		// Methods for orthogonal capacity of 2
		void routeMixingModules2(vector<FixedModule *> *mixers, vector< vector<WireSegment *> *> *wires);
		void routePinOptRoutingCells2(int verticalColumnIndex, vector< vector<WireSegment *> *> *wires);

		// Methods for orthogonal capacity of 3
		void routeMixingModules3(vector<FixedModule *> *mixers, vector< vector<WireSegment *> *> *wires);
		void routePinOptRoutingCells3(int verticalColumnIndex, vector< vector<WireSegment *> *> *wires);

	public:
		// Constructors
		EnhancedFPPCWireRouter(DmfbArch *dmfbArch);
		virtual ~EnhancedFPPCWireRouter();

		// Methods
		void computeWireRoutes(vector<vector<int> *> *pinActivations);
};
#endif /* GRISSOM_ENHANCED_FPPC_WIRE_ROUTER_H_ */

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
 * Name: Diagonal Wire Routing Model											*
 * 																				*
 * Detailed in the following paper:												*
 * Authors: 																	*
 * Title: 																		*
 * Publication Details: 														*
 * 																				*
 * Details: Models the wire-routing problem while allowing proper modeling for	*
 * diagonal wire routes. 														*
 *-----------------------------------------------------------------------------*/
#ifndef DIAGONAL_WIRE_ROUTING_MODEL_H_
#define DIAGONAL_WIRE_ROUTING_MODEL_H_

using namespace std;
#include "../Resources/structs.h"
#include "../Testing/claim.h"
#include "dmfb_arch.h"
#include <vector>
#include <limits>

class DmfbArch;

class DiagonalWireRoutingModel
{
	public:
		// Constructors
		DiagonalWireRoutingModel();
		DiagonalWireRoutingModel(DmfbArch *dmfbArch);
		virtual ~DiagonalWireRoutingModel();

		// Methods
		void OutputGraphFile(string name, bool showSuperEscape);

		// Getters/Setters
		vector<WireRouteNode *> *getAllNodes() { return allNodes; }
		map<int, vector<WireRouteNode *> *> *getPinGroups() { return pinGroups; }
		WireRouteNode *getSuperEscape() { return superEscape; }
		int getTileGridSize() { return tileGridSize; }
		int getWireGridXSize() { return wireGridSizeX; }
		int getWireGridYSize() { return wireGridSizeY; }
		//vector<vector<WireRouteNode *> *> * getPins() { return pins; }
		WireRouteNode *getPin(int x, int y);

	private:
		// Members
		map<int, vector<WireRouteNode *> *> *pinGroups; // Map/grouping of all pins needing to be tied together; indexed on shared pin#
		WireRouteNode *superEscape; // All escape nodes pass through this node (Sink)
		vector<vector<WireRouteTile *> *> *model;
		vector<vector<WireRouteNode *> *> *pins;
		vector<WireRouteNode *> *allNodes;
		int oCap; // orthogonal capacity
		int tileGridSize; // size of the wire-routing tile-grid
		int wireGridSizeX;
		int wireGridSizeY;
		static int next_id;
		DmfbArch *arch;

		// Get Constants
		bool isAvoidingIOPorts() { return false; } // If true, will avoid routing off array where IO's exist
		bool isUsingSparseModel() { return false; } // True uses a WireRouteTile with 9 internal nodes; False uses a tile with 13 internal nodes and can handle greater diagonal capacity

		// Getters/Setters
		//int GetInfiniteCapacity() { return std::numeric_limits<int>::max(); }
};


#endif /* DIAGONAL_WIRE_ROUTING_MODEL_H_ */

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
/*--------------------------------File Details----------------------------------*
 * Name: Structures																*
 *																				*
 * Details: This file contains some basic structures used by various classes.	*
 * They are placed here to keep the other files from getting clustered.			*
 *-----------------------------------------------------------------------------*/
#ifndef _STRUCTS_H
#define _STRUCTS_H

#include <ext/slist>
#include <sstream>
#include "enums.h"
#include <vector>
#include <string.h>
#include <list>
#include <map>
#include "../Models/reconfig_module.h"
#include "../Models/fixed_module.h"

using namespace std;

class IoPort;
class AssayNode;
struct WireRouteNode;
class DmfbArch;

/////////////////////////////////////////////////////////////////
// WireRouteNode Structure: Used to represent a wire routing node
// in the wire routing model.
/////////////////////////////////////////////////////////////////
struct WireRouteNode {
	int id;
	int claimedPin; // The pin number which is claiming this node to route through
	int wgX; // Wire Grid X-coordinate
	int wgY; // Wire Grid Y-coordinate
	int originalPinNum; // The pin number originally associated with this node (if a pin node)
	DmfbArch *arch; // Needed for checking distance to edge in comparator
	string name;
	WireRouteNodeType nodeType;
	vector<WireRouteNode *> neighbors;

	int iteration;
	int history_cost;
	int penalty_cost;
	int occupancy;
	int orthogonal_cost;
	int tile_cost;
	WireRouteNode* spawn;

	WireRouteNode() {
		id = -1;
		claimedPin = -1;
		wgX = -1;
		wgY = -1;

		iteration = -1;
		history_cost = 1;
		penalty_cost = 0;
		occupancy = 0;
		orthogonal_cost = 0;
		tile_cost = 0;
		spawn = NULL;
	}

	// Destructor deletes child edges
	~WireRouteNode()
	{
		neighbors.clear();
	}

	void AddUniqueConnection(WireRouteNode *to)
	{
		// Check to make sure not already a connection
		bool found = false;
		for (unsigned i = 0; i < neighbors.size(); i++)
			if (neighbors.at(i) == to)
				found = true;

		if (!found)
		{
			neighbors.push_back(to);
			to->neighbors.push_back(this);
		}
	}

	void RemoveUniqueConnection(WireRouteNode *dest)
	{
		// Remove destination from own neighbors
		for (unsigned i = 0; i < neighbors.size(); i++)
		{
			if (neighbors.at(i) == dest)
			{
				neighbors.erase(neighbors.begin()+i);
				break;
			}
		}

		// Remove self from destination's neighbors
		for (unsigned i = 0; i < dest->neighbors.size(); i++)
		{
			if (dest->neighbors.at(i) == this)
			{
				dest->neighbors.erase(dest->neighbors.begin()+i);
				break;
			}
		}
	}
};

struct WireRouteTile {
	vector<vector<WireRouteNode *> *> *nodCols;

	// Constructor creates new nodes (pins created separately)
	WireRouteTile(int oCap, bool sparseModel) {

		nodCols = new vector<vector<WireRouteNode *> *>();

		int tileSize = ((oCap+2)*2 - 1);

		for (int i = 0; i < tileSize; i++)
		{
			// Create new row and add to rows
			vector<WireRouteNode *> *col = new vector<WireRouteNode *>();
			nodCols->push_back(col);

			// If true, draws the sparse model with smaller diagonal capacity (Design 1)
			// If fale, draws the slightly-more dense model with larger diagonal cap (Design 2)
			//bool sparseModel = false;

			// First and last columns have: oCap edgeNodes and 0 internal nodes
			if (i == 0 || i == tileSize-1)
			{
				for (int j = 0; j < oCap; j++)
				{
					WireRouteNode *n = new WireRouteNode();
					n->nodeType = ESCAPE_WRN; // ID and name will be set externally later
					col->push_back(n);
				}
			}
			else if ((i == 1 || i == tileSize-2) && sparseModel)
			{	// Second and next-to-last rows 0 edge nodes and oCap-1 internal nodes
				for (int j = 0; j < oCap-1; j++)
				{
					WireRouteNode *n = new WireRouteNode();
					n->nodeType = INTERNAL_WRN; // ID and name will be set externally later
					col->push_back(n);
				}
			}
			else if (i % 2 == 0)
			{	// All other EVEN rows have 2 edge nodes and oCap internal nodes sandwiched in between
				for (int j = 0; j < oCap+2; j++)
				{
					WireRouteNode *n = new WireRouteNode();
					if (j == 0 || j == oCap+1)
						n->nodeType = ESCAPE_WRN; // ID and name will be set externally later
					else
						n->nodeType = INTERNAL_WRN; // ID and name will be set externally later
					col->push_back(n);
				}
			}
			else
			{	// All other ODD rows have 0 edge nodes and oCap+1 internal nodes
				for (int j = 0; j < oCap+1; j++)
				{
					WireRouteNode *n = new WireRouteNode();
					n->nodeType = INTERNAL_WRN; // ID and name will be set externally later
					col->push_back(n);
				}
			}
		}
	}
	// Destructor deletes the wire-routing nodes (pins deleted separately)
	~WireRouteTile()
	{
		while (!nodCols->empty())
		{
			vector<WireRouteNode *> *v = nodCols->back();
			nodCols->pop_back();
			v->clear();
			delete v;
		}
	}
};
/* CODE REPRESENTS THE MODEL WHERE THE PERIPHERAL EDGES HAVE CAPACITY (NOT SURE
 * THIS IS CORRECT AS IT DOESN'T SAY THIS IN THE PAPER.
 */
/*
  struct WireRouteTile {
	WireRouteNode *pinNW, *pinNE, *pinSW, *pinSE;
	WireRouteNode *periphInN, *periphInS, *periphInE, *periphInW, *periphOutN, *periphOutS, *periphOutE, *periphOutW;
	WireRouteNode *centerIn, *centerOut;

	// Constructor creates new nodes (pins created separately)
	WireRouteTile() {
		periphInN = new WireRouteNode();
		periphOutN = new WireRouteNode();
		periphInE = new WireRouteNode();
		periphOutE = new WireRouteNode();
		periphInS = new WireRouteNode();
		periphOutS = new WireRouteNode();
		periphInW = new WireRouteNode();
		periphOutW = new WireRouteNode();
		centerIn = new WireRouteNode();
		centerOut = new WireRouteNode();
	}
	// Destructor deletes the wire-routing nodes (pins deleted separately)
	~WireRouteTile() {
		vector<WireRouteNode *> tileNodes;
		tileNodes.push_back(periphInN);
		tileNodes.push_back(periphInS);
		tileNodes.push_back(periphInE);
		tileNodes.push_back(periphInW);
		tileNodes.push_back(periphOutN);
		tileNodes.push_back(periphOutS);
		tileNodes.push_back(periphOutE);
		tileNodes.push_back(periphOutW);
		tileNodes.push_back(centerIn);
		tileNodes.push_back(centerOut);

		while (!tileNodes.empty())
		{
			WireRouteNode *n = tileNodes.back();
			tileNodes.pop_back();
			delete n; // WireRouteNode destructor automatically deletes edges
		}
	}
};
*/




/////////////////////////////////////////////////////////////////
// KamerLlNode: Node for the Kamer Linked-List Implementation
/////////////////////////////////////////////////////////////////
struct KamerLlNode {
	KamerNodeType nodeType;
	string modName; // A name for the associated module for helping link/match in- and out-edges together
	int height;
	int left;
	int right;
	int numInEdges;
	int numOutEdges;
	unsigned long long endTS;

	KamerLlNode *next;
	KamerLlNode *last;
	KamerLlNode *ref;
};

/////////////////////////////////////////////////////////////////
// MaxFreeRect: Dimensions of maximum free rectangle
/////////////////////////////////////////////////////////////////
struct MaxFreeRect {
	int lower;
	int upper;
	int left;
	int right;
	ResourceType resType;
};

/////////////////////////////////////////////////////////////////
// Coord: Simple (x, y) coordinate.
/////////////////////////////////////////////////////////////////
struct Coord {
	int x;
	int y;
};

/////////////////////////////////////////////////////////////////
// AssayPathNode Structure: Used to represent multiple assay
// nodes that are part of the same path (connected by continuous
// edges) and have the same resource type.
/////////////////////////////////////////////////////////////////
struct AssayPathNode {
	vector<AssayNode *> pathNodes;
	ResourceType resType;
	unsigned long long startTS;
	unsigned long long endTS;
	FixedModule *res;
	vector<AssayPathNode *> parents;
	vector<AssayPathNode *> children;
};

/////////////////////////////////////////////////////////////////
// BoardCell: Holds the basic information for a cell to be used
// with various routing algorithms.
/////////////////////////////////////////////////////////////////
struct BoardCell {
	int x;
	int y;
	int S;
	int C;
	//everything below is useful for debugging purposes.
	int route_distance;
	BoardCell* trace;
	vector<BoardCell *> path;
};

/////////////////////////////////////////////////////////////////
// KamerCell: Holds the basic information for a cell to be used
// with the KamerLlPlacer algorithm
/////////////////////////////////////////////////////////////////
struct KamerCell {
	int x;
	int y;
	char status; // 'o' = out-Edge, 'i' = in-edge, 'm' = module/IR (excluding in/out edge area), '-' = free/open, 'f' = forbidden
};

/////////////////////////////////////////////////////////////////
// SoukupCell: Holds the basic information for a cell to be used
// with Soukup's algorithm (maze routing)
/////////////////////////////////////////////////////////////////
struct SoukupCell {
	int x;
	int y;
	int S;
	int C;
	SoukupCell* trace;
};

/////////////////////////////////////////////////////////////////
// LeeCell: Holds the basic information for a cell to be used
// with Lee's algorithm
/////////////////////////////////////////////////////////////////
/*struct LeeCell {
	int x;
	int y;
	int S;
	int C;
	int val;
	LeeCell* trace;
};*/
/////////////////////////////////////////////////////////////////
// RoutePoint: Holds the basic information for a point in a
// droplet's route
/////////////////////////////////////////////////////////////////
struct RoutePoint {
	unsigned long long cycle;
	int x;
	int y;
	DropletStatus dStatus;
	Droplet *droplet;
};

/////////////////////////////////////////////////////////////////
// IoResource Structure: Holds basic info for a Dispense
// resource so the scheduler can tell if a proper Dispense resource
// is ready.
/////////////////////////////////////////////////////////////////
struct IoResource {
	string name;
	IoPort *port;
	unsigned long long lastEndTS; // Used for LS and schedulers that don't look back
	unsigned long long durationInTS;
	list<AssayNode *> schedule;
	map<unsigned long long, bool> isActiveAtTS; // Used for PathSched and schedulers that DO look back
};

/////////////////////////////////////////////////////////////////
// ResHist Structure: Holds info which details available
// resources for a particular time-step
/////////////////////////////////////////////////////////////////
struct ResHist {
	unsigned long long timeStep;
	int availRes[RES_TYPE_MAX+1];
	int dropsInLoc;
	int dropsInStorage;
};

/////////////////////////////////////////////////////////////////
// DropletDependenceVertex Structure: Used to keep track of
// dependencies...helps detect/free deadlock
/////////////////////////////////////////////////////////////////
struct ModuleDependenceVertex
{
	AssayNode *operation;
	vector<ModuleDependenceVertex *> preds;
	vector<ModuleDependenceVertex *> succs;
	int utilNum; // Number used for priorities, ordering and numbering
	bool alreadyChecked;
	bool addedToAnSCC;
	bool isRoutingBufferNode;
	ModuleDependenceVertex *reRouteVertex; // This node must be re-routed to the routing buffer before any node in the external container can be routed

	~ModuleDependenceVertex()
	{
		operation = NULL;
		//preds = succs = NULL;
	}
};

#endif

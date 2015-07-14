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
/*---------------------------Implementation Details-----------------------------*
 * Source: path_finder_wire_router.cc											*
 * Original Code Author(s): Jeffrey McDaniel									*
 * Original Completion/Release Date: December 16, 2013							*
 *																				*
 * Details: N/A																	*
 *																				*
 * Revision History:															*
 * WHO		WHEN		WHAT													*
 * ---		----		----													*
 * FML		MM/DD/YY	One-line description									*
 *-----------------------------------------------------------------------------*/

#include "../../Headers/WireRouter/yeh_wire_router.h"
#include "../../Headers/WireRouter/paths.h"
#include "../../Headers/Util/file_out.h"
#include "../../Headers/Util/sort.h"
#include <algorithm>

///////////////////////////////////////////////////////////////////////////////////
// Constants - Values taken from "Architecture and CAD for Deep-Submicron FPGA's",
// by Vaughn Betz
///////////////////////////////////////////////////////////////////////////////////
//const double PathFinderWireRouter::kHfac = 1;
//const int PathFinderWireRouter::kPfacIncrease = 1.5;
//const unsigned PathFinderWireRouter::kMaxIterations = 30;
//const bool PathFinderWireRouter::SaveBest = true;

///////////////////////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////////////////////
YehWireRouter::YehWireRouter(DmfbArch *dmfbArch)
{
	arch = dmfbArch;
	type = YEH_WR;
}

///////////////////////////////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////////////////////////////
YehWireRouter::~YehWireRouter()
{
}

///////////////////////////////////////////////////////////////////////////////////
// Given the pin-mapping (contained in the DmfbArch), computes the wire routing
// to connect all the pins together and to the output of the device.  Also, has
// access to the pin activations list in case this algorithm needs to modify the
// pin-mapping itself.  This algorithm uses lee's maze routing.
///////////////////////////////////////////////////////////////////////////////////
void YehWireRouter::computeWireRoutes(vector<vector<int> *> *pinActivations)
{
	cout << "Beginning wire routing phase:" << endl;
	layeredYeh(model);

	maxPinNum = 0;
	map<int, vector<WireRouteNode *> *>::iterator groupIt;
	for (groupIt = model->getPinGroups()->begin();groupIt != model->getPinGroups()->end();groupIt++)
		if (maxPinNum == 0 || groupIt->first > maxPinNum)
			maxPinNum = groupIt->first;
	maxPinNum++;

	vector< vector<WireSegment *> *> *wires = arch->getWireRouter()->getWireRoutesPerPin();
	for (int i = 0; i < maxPinNum; i++)
	{
		vector<WireSegment *> *wire = new vector<WireSegment *>();
		wires->push_back(wire);
	}

	////////////////////////////////////////////////
	// Convert results to the actual wire segments//
	////////////////////////////////////////////////
	convertWireSegments(&layers,wires);
}

///////////////////////////////////////////////////////////////////////////////////
// This function takes in the layers that contain paths (the internal representation
// in this wire-router) and converts them to the wire segments needed by the
// framework.
///////////////////////////////////////////////////////////////////////////////////
void YehWireRouter::convertWireSegments(vector<vector<Path*> >* layers, vector<vector<WireSegment*>*>* wires)
{
	int source_pins = 0;
	int sink_pins = 0;
	int num_pins = 0;
	for (unsigned i = 0;i < layers->size();i++)
	{
		for (unsigned j = 0;j < layers->at(i).size();j++) 
		{
			Path* current_path = layers->at(i).at(j);
			int current_pin = current_path->getPinNumber();
			for (unsigned k = 0;k < current_path->pathSize();k++)
			{
				PathSegment* current_segment = current_path->segmentAt(k);
				for (unsigned l = 0;l < current_segment->nextSegmentsSize();l++)
				{
					// Goal: wires->at(i) should contain a vector with all the wire segments for pin i.
					WireRouteNode* source_node = current_segment->getNode();
					WireRouteNode* sink_node = current_segment->nextSegmentsAt(l)->getNode();

					if (!(source_node->nodeType == SUPER_ESCAPE_WRN || sink_node->nodeType == SUPER_ESCAPE_WRN))
					{
						WireSegment *ws = new WireSegment();
						wires->at(current_pin)->push_back(ws);

						ws->segmentType = LINE_WS;
						ws->pinNo = current_pin;
						ws->layer = i;

						ws->sourceWireCellX = source_node->wgX;
						ws->sourceWireCellY = source_node->wgY;

						ws->destWireCellX = sink_node->wgX;
						ws->destWireCellY = sink_node->wgY;
					}
					else
						num_pins++;
				}
			}
		}
	}

	cout << "Total number of pins routed off DMFB: " << num_pins << endl;
}




///////////////////////////////////////////////////////////////////////////////////
// Tests whether new_path intersects with any path contained in old_paths
// vector. This is used to create layers.
// Returns: True if intersection exists, false otherwise
///////////////////////////////////////////////////////////////////////////////////
bool YehWireRouter::intersects(Path* new_path,vector<Path*>* old_paths)
{
	vector<WireRouteNode*> shared_nodes;
	for (unsigned i = 0;i < old_paths->size();i++)
		for (unsigned j = 0;j < old_paths->at(i)->pathSize();j++)
			if (old_paths->at(i)->nodeAt(j)->nodeType != SUPER_ESCAPE_WRN && old_paths->at(i)->nodeAt(j)->occupancy > 1)
				shared_nodes.push_back(old_paths->at(i)->nodeAt(j));

	if (shared_nodes.empty())
		return false;

	for (unsigned i = 0;i < new_path->pathSize();i++)
		if (new_path->nodeAt(i)->occupancy > 0)
			for (unsigned j = 0;j < shared_nodes.size();j++)
				if (new_path->nodeAt(i) == shared_nodes.at(j))
					return true;

	return false;
}

///////////////////////////////////////////////////////////////////////////////////
// Performs Layered Yeh's.
///////////////////////////////////////////////////////////////////////////////////
//vector<vector<Path> > YehWireRouter::layeredYeh(DiagonalWireRoutingModel* model)
void YehWireRouter::layeredYeh(DiagonalWireRoutingModel* model)
{
	vector<WireRouteNode *> allNodes = *(model->getAllNodes());
	map<int, vector<WireRouteNode *> *> pinGroups = *(model->getPinGroups());
	map<int, vector<WireRouteNode *> *> reRouteGroups;
	WireRouteNode* super_escape = model->getSuperEscape();

	int total_paths = pinGroups.size();
	int layer_number = 1;
	int remaining_paths = total_paths;
	while (!pinGroups.empty())
	{
		cout << "Routing Layer " << layer_number;
		remaining_paths = pinGroups.size();
		clearHistory(&allNodes);

		for (unsigned i = 0;i < allNodes.size();i++)
		{
			WireRouteNode* current_node = allNodes.at(i);

			current_node->history_cost = 0;
			current_node->penalty_cost = 0;
			current_node->occupancy = 0;
			current_node->iteration = -1;
			current_node->claimedPin = -1;
			current_node->spawn = NULL;
		}
		yeh_iccad_routing(allNodes,pinGroups,super_escape);
		cout << layers.back().size() << " pins routed." << endl;
		if (layers.back().empty()) {
			cerr << "No new routes routed." << endl;
			return;
		}
		//Sort::sortPathsBySharedPinSize(&new_routes); //TODO: When this has
		//been transferred to pointers, uncomment

		vector<Path*>::iterator path_iter = layers.back().begin();
		int path_count = 1;
		/* Add paths to the current layer */
		for (path_iter; path_iter != layers.back().end(); path_iter++)
		{
			(*path_iter)->addPathSegment(super_escape,(*path_iter)->nodeAt(0));

			// Now convert used pins to internal nodes so lower layers can route through them - DTG
			//cout << "Erasing pin # " << (*path_iter).getPinNumber() << endl;
			vector<WireRouteNode *> *pinsToErase = pinGroups.at((*path_iter)->getPinNumber());
			for (int i = 0; i < pinsToErase->size(); i++) 
			{
				pinsToErase->at(i)->nodeType = INTERNAL_WRN;
			}

			pinGroups.erase((*path_iter)->getPinNumber());

			path_count++;
		}

		layer_number++;
	}
}

void YehWireRouter::rip_up_route(Path* ripped) {
	for (unsigned i = 0;i < ripped->pathSize();i++) {
		WireRouteNode* current_node = ripped->nodeAt(i);
		/* Using an occupancy of '2' increases the history cost of the entire path */
		current_node->history_cost = historyCost(current_node->history_cost,2);
		current_node->penalty_cost = 0;
		current_node->occupancy = 0;
		current_node->iteration = -1;
		current_node->claimedPin = -1;
		current_node->spawn = NULL;
	}
}

void YehWireRouter::changeSharing(vector<Path*>* paths,bool allowed) {
	for (unsigned i = 0;i < paths->size();i++) {
		if (allowed) { paths->at(i)->allowSharing(); }
		else { paths->at(i)->disallowSharing(); }
	}
}

///////////////////////////////////////////////////////////////////////////////////
// Tests whether new_path intersects with any path contained in old_paths
// vector. 
// Returns: vector of intersecting indices
///////////////////////////////////////////////////////////////////////////////////
vector<int> YehWireRouter::intersecting_pins(Path* new_path,vector<Path*>* old_paths)
{
	vector<int> intersecting_pins;
	vector<WireRouteNode*> shared_nodes;
	for (unsigned i = 0;i < new_path->pathSize();i++) {
		if (new_path->nodeAt(i)->nodeType != SUPER_ESCAPE_WRN && new_path->nodeAt(i)->occupancy > 1) {
			shared_nodes.push_back(new_path->nodeAt(i));
		}
	}
	if (shared_nodes.empty()) {
		return intersecting_pins;
	}

	for (unsigned i = 0;i < old_paths->size();i++) {
		bool not_intersecting = true;
		for (unsigned j = 0;j < old_paths->at(i)->pathSize() && not_intersecting;j++) {
			for (unsigned k = 0;k < shared_nodes.size() && not_intersecting;k++) {
				if (old_paths->at(i)->nodeAt(j) == shared_nodes.at(k)) {
					intersecting_pins.push_back(old_paths->at(i)->getPinNumber());
					not_intersecting = false;
				}
			}
		}
	}
	return intersecting_pins;
}

///////////////////////////////////////////////////////////////////////////////////
// This function calls Wire routing algorithm from "Volgate-Aware Chip-Level
// Design for Reliability-Driven Pin-Constrained EWOD Chips" by Yeh, Chang,
// Huang, and Ho.
// Needs to be routed on a single layer.
// 	1. Attempt to route each wire.
// 	2. If wire_i conflicts with wire_j which has already routed.
// 		1. Rip up wire_j, and add it to the black list
// 		2. Complete routing wire_i
// 		3. Attemp to route wire_j again
// 		4. If routing fails on a black listed route, algorithm fails.
///////////////////////////////////////////////////////////////////////////////////
//vector<Path> YehWireRouter::yeh_iccad_routing(vector<WireRouteNode*> allNodes, map<int, vector<WireRouteNode*>* > pinGroups, 
void YehWireRouter::yeh_iccad_routing(vector<WireRouteNode*> allNodes, map<int, vector<WireRouteNode*>* > pinGroups, 
		WireRouteNode* super_escape) 
{
	vector<Path*> paths;
	int failed_routes = pinGroups.size();

	map<int, vector<WireRouteNode*>* >::iterator group_it;
	unsigned i = 0;
	int ten_percent = pinGroups.size() > 10 ? pinGroups.size() / 10 : 1;
	int percent = 0;
	for (group_it = pinGroups.begin();group_it != pinGroups.end(); group_it++)
	{
		if (percent >= ten_percent) {
			cout << "." << flush;
			percent = 0;
		}
		percent++;

		vector<WireRouteNode *> pinGroup = *(group_it->second);
		int pinNum = pinGroup.front()->originalPinNum;

		//Attempt to route the pin without intersections
		changeSharing(&paths,false/*disAllows Sharing*/);
		/* 0 pfac, 0 iteration */
		Path* new_path = new Path();
		leeMazeRouting(pinNum,0,super_escape,pinGroup,0,new_path);

		super_escape->claimedPin = -1;
		bool pinFailed = false;
		if (new_path->empty()) {
			bool attemptReroute = true;
			vector<int> black_listed_pins;
			while (attemptReroute) {
				// Reroute the current pin, allowing intersections
				changeSharing(&paths,true/*Allows Sharing*/);
				leeMazeRouting(pinNum,0,super_escape,pinGroup,0,new_path);
				// Find which pins it intersects with.
				vector<int> pin_conflicts = intersecting_pins(new_path,&paths);
				// Add pins it intersects with to black list
				for (unsigned i = 0;i < pin_conflicts.size() && !pinFailed;i++) {
					for (unsigned j = 0;j < black_listed_pins.size() && !pinFailed;j++) {
						if (pin_conflicts.at(i) == black_listed_pins.at(j)) {
							pinFailed = true; //This pin fails
							attemptReroute = false;
							pin_conflicts.clear();
						}
					}
					if (!pinFailed) { 
						black_listed_pins.push_back(pin_conflicts.at(i)); 
					}
				}
				if (!pinFailed) {
					// Rip up all pins it intersected with
					for (unsigned i = 0;i < pin_conflicts.size();i++) {
						vector<Path*>::iterator path_it = paths.begin();
						for (/*path_it*/;path_it != paths.end();path_it++) {
							if ((*path_it)->getPinNumber() == pin_conflicts.at(i)) {
								int pin = (*path_it)->getPinNumber();
								(*path_it)->rip_up();
								Path* erased_path = (*path_it);
								paths.erase(path_it);
								delete erased_path;
								if (path_it == paths.end()) { path_it--; }
							}
						}
					}
					changeSharing(&paths,false/*disAllows Sharing*/);
					// Re route all pins (in order)
					for (unsigned i = 0;i < pin_conflicts.size();i++) {
						int reroute_pin = pin_conflicts.at(i);
						vector<WireRouteNode*> reRouteGroup = *(pinGroups.at(reroute_pin));
						Path* reRoutePath = new Path();
						leeMazeRouting(reroute_pin,0,super_escape,reRouteGroup,0,reRoutePath);
						if (!reRoutePath->empty()) {
							reRoutePath->setCost(reRoutePath->pathSize());
							reRoutePath->setSharedNodes(0);
							reRoutePath->setPinNumber(reroute_pin);
							reRoutePath->disallowSharing();
							paths.push_back(reRoutePath);
						} else {
					 		// Any that fail get pushed to later layer
							// By not adding them back to paths, they should be
							// pushed to later
						}
					}
					delete new_path;
					new_path = new Path();
					leeMazeRouting(pinNum,0,super_escape,pinGroup,0,new_path);
					if (new_path->empty()) { attemptReroute = true; }
					else { attemptReroute = false; }
				}
			}
		}
		new_path->setCost(new_path->pathSize());
		new_path->setSharedNodes(0);
		new_path->setPinNumber(pinNum);
		new_path->disallowSharing(); //Simply sets iteration to 1 for all nodes on the path
		paths.push_back(new_path);
	}
	cout << endl; //Outputs a new line after the "."'s
	layers.push_back(paths);
}

///////////////////////////////////////////////////////////////////////////////////
// This function prints the contents of the path.
///////////////////////////////////////////////////////////////////////////////////
void YehWireRouter::printPath(vector<WireRouteNode*>* path)
{
	for (unsigned i = 0;i < path->size()-1;i++)
		cout << "(" << path->at(i)->wgX << ","	<< path->at(i)->wgY << ") -> ";

	cout << "(" << path->at(path->size()-1)->wgX << "," << path->at(path->size()-1)->wgY << ")\n";
}

///////////////////////////////////////////////////////////////////////////////////
// Perfroms Lee's maze routing algorithm.
// 1. While sinks remain unfound.
// 		* Fill the grid from the source to the closest sink
// 		* Traceback from the sink found to the current path (originally just the source)
///////////////////////////////////////////////////////////////////////////////////
//Path YehWireRouter::leeMazeRouting(int pin_number,double pfac,WireRouteNode* source, vector<WireRouteNode*> sinks, int iterationNum)
void YehWireRouter::leeMazeRouting(int pin_number,double pfac,WireRouteNode* source, vector<WireRouteNode*> sinks, int iterationNum,Path* new_path)
{
	for (unsigned i = 0;i < sinks.size();i++) {
		WireRouteNode* current = sinks.at(i);
		current->iteration = -1;
	}
	WireRouteNode* sink_found;
	vector<WireRouteNode*> nodes_found;
	new_path->addPathSegment(source);
	bool failed = false;
	int sink_number = 1;
	int max_sinks = sinks.size();
	while (!sinks.empty() && !failed)
	{
		nodes_found = fillGrid(pin_number,pfac,new_path,&sinks);
		if (!nodes_found.empty())
		{
			sink_found = nodes_found.back();
			nodes_found.pop_back();
			sink_found->claimedPin = pin_number;
			if (!traceBack(pfac,sink_found,new_path, iterationNum))
			{
				failed = true;
			}

			//Remove sink found from list of sinks.
			for (unsigned i = 0;i < sinks.size();i++)
			{
				if (sinks.at(i) == sink_found)
				{
					vector<WireRouteNode*>::iterator it = sinks.begin()+i;
					sinks.erase(it);
				}
			}
			//Clear the iterations of nodes found.
			for (unsigned i = 0;i < nodes_found.size();i++)
			{
				nodes_found.at(i)->iteration = -1;//Clear the iteration found at.
				nodes_found.at(i)->spawn = NULL;//Clear the spawning node.
			}
		} else {
			failed = true;
		}
		sink_number++;
	}
	if (failed) {
		for (unsigned i = 0;i < nodes_found.size();i++) {
			nodes_found.at(i)->iteration = -1;//Clear the iteration found at.
			nodes_found.at(i)->spawn = NULL;//Clear the spawning node.
		}
		// clear the path to return an empty path
		delete new_path;
		new_path = new Path();
	}
}

///////////////////////////////////////////////////////////////////////////////////
// Appends the nodes in appended vector to the path original
///////////////////////////////////////////////////////////////////////////////////
void YehWireRouter::append(Path* original, vector<WireRouteNode*>* appended)
{
	if (!original || !appended)
		return;

	for (int i = appended->size()-2;i >= 0;i--)
		original->addPathSegment(appended->at(i),appended->at(i+1));
}

///////////////////////////////////////////////////////////////////////////////////
// Traces back from the sink that was found to the current path.
// Returns:
// 	On success: a vector of the nodes on the path from sink, to one of the
// 			sources, with source being the last element.
// 	On failure: an empty vector.
///////////////////////////////////////////////////////////////////////////////////
bool YehWireRouter::traceBack(double pfac,WireRouteNode* sink,Path* sources, int iterationNum)
{
	vector<WireRouteNode*>* path = new vector<WireRouteNode*>;
	path->push_back(sink);
	bool source_found = false;
	WireRouteNode* current_node = sink;
	WireRouteNode* next_node = NULL;
	while (!source_found)
	{
		next_node = findPrevious(current_node);
		if (next_node == NULL)
		{
			std::cerr << "Error: Pathfinder: Trace back failed.\n";
			return false;
		}
		PathSegment* source_segment = sources->findSegment(next_node);
		if (source_segment) {
			source_found = true;
		}

		path->push_back(next_node);
		if (next_node->claimedPin != sink->claimedPin)
		{
			sources->addSharedPin(next_node->claimedPin);
			next_node->occupancy = next_node->occupancy + 1;
		}
		next_node->claimedPin = sink->claimedPin;

		if (iterationNum == 0)
		{
			next_node->penalty_cost = getFirstItPenalty() + pfac * next_node->occupancy;
		}
		else
		{
			next_node->penalty_cost = pfac * next_node->occupancy;
		}

		current_node = next_node;
	}
	sources->removeSuper();

	append(sources,path);
	return true;
}

///////////////////////////////////////////////////////////////////////////////////
// Finds the previous node in the trace back. Traces back to the node that
// spawned the current node during the grid fill step
///////////////////////////////////////////////////////////////////////////////////
WireRouteNode* YehWireRouter::findPrevious(WireRouteNode* current_node)
{
	WireRouteNode* return_node = NULL;
	if (current_node->spawn)
		return_node = current_node->spawn;
	else
	{
		std::cerr << "Error: Pathfinder: No previous node can be found.\n";
		return_node = NULL;
	}
	return return_node;
}

///////////////////////////////////////////////////////////////////////////////////
//Comparison operator for priority queues.
///////////////////////////////////////////////////////////////////////////////////
struct CompareNodes
{
	bool operator()(WireRouteNode* lhs, WireRouteNode* rhs) {
		//true if lhs > rhs
		int lhs_cost = lhs->history_cost + lhs->penalty_cost + lhs->iteration;
		int rhs_cost = rhs->history_cost + rhs->penalty_cost + rhs->iteration;
		return lhs_cost > rhs_cost;
	}
};

///////////////////////////////////////////////////////////////////////////////////
// Fills the grid from the set of sources until a sink is found.
// 	2. Expand the nodes one at time, adding new nodes to a priority queue based
// 	on their cost. Continues until any sink is found.
// 		Cost = history_cost + penalty_cost + iteration
// On success returns the nodes found, with the sink found being the last node
// in the vector. On failure returns an empty vector*/
//
// Returns:
// On success: a vector of the WireRouteNodes visited during the fill, with the
// 			sink found as the last element.
// 	On Failure: an empty vector, with all visited WireRouteNodes iteration
// 			value reset to 0 (as if they were never visited).*/
///////////////////////////////////////////////////////////////////////////////////
vector<WireRouteNode*> YehWireRouter::fillGrid(int pin_number,double pfac,Path* sources,vector<WireRouteNode*>* sinks)
{
	vector<WireRouteNode*> searched_nodes;

	/* Set up the expanding wavefront */
	priority_queue<WireRouteNode*, vector<WireRouteNode*>, CompareNodes>* wavefront =
			new priority_queue<WireRouteNode*, vector<WireRouteNode*>, CompareNodes>;
	for (unsigned i = 0;i < sources->pathSize();i++)
	{
		wavefront->push(sources->nodeAt(i));
		searched_nodes.push_back(sources->nodeAt(i));
		sources->nodeAt(i)->iteration = 1;
	}
	int min_distance = -1;
	WireRouteNode* closest_node;
	while (!wavefront->empty())
	{
		WireRouteNode* current_node = wavefront->top();
		wavefront->pop();
		if (!current_node)
			std::cerr << "Error: Current node does not exist.\n";

		for (unsigned i = 0;i < current_node->neighbors.size();i++)
		{
			WireRouteNode* expanse = current_node->neighbors.at(i);
			if ( (expanse->nodeType != SUPER_ESCAPE_WRN) )
			{
				if (expanse->iteration <= 0)
				{
					/* Expand the node */
					int sink_index = isSink(expanse,sinks);
					if (sink_index >= 0)
					{
						expanse->iteration = current_node->iteration + 1;
						expanse->spawn = current_node;
						searched_nodes.push_back(sinks->at(sink_index));
						return searched_nodes;
					}
					else
					{
						if (expanse->nodeType != PIN_WRN)
						{
							expanse->iteration = current_node->iteration + 1;
							expanse->spawn = current_node;
							wavefront->push(expanse);
							searched_nodes.push_back(expanse);
						}
					}
				}
			}
		}
	}
	// Sink node could not be found, Yeh's will attempt to rip and reroute
	// Clear the searched nodes since sink was not found
	for (unsigned i = 0;i < searched_nodes.size();i++)
	{
		searched_nodes.at(i)->iteration = -1;//Clear the iteration found at.
		searched_nodes.at(i)->spawn = NULL;
	}
	searched_nodes.clear();

	return searched_nodes;
}

///////////////////////////////////////////////////////////////////////////////////
// Returns:
//	On success: the index of the sink found in the sinks vector
//	On failure: -1
///////////////////////////////////////////////////////////////////////////////////
int YehWireRouter::isSink(WireRouteNode* node,vector<WireRouteNode*>* sinks)
{
	for (unsigned i = 0;i < sinks->size();i++)
		if (sinks->at(i) == node)
			return i;
	return -1;
}

///////////////////////////////////////////////////////////////////////////////////
// Returns a new history cost based on the old_history, and the occupancy of the
// node.
///////////////////////////////////////////////////////////////////////////////////
int YehWireRouter::historyCost(int old_history,int occupancy)
{
	if (occupancy > 1)
		return old_history + ((occupancy - 1) * getKHFac());
	else
		return old_history >= 1? old_history:1;
}

///////////////////////////////////////////////////////////////////////////////////
// Clears the history cost of allNodes vector.
///////////////////////////////////////////////////////////////////////////////////
void YehWireRouter::clearHistory(vector<WireRouteNode*>* allNodes)
{
	for (unsigned i = 0;i < allNodes->size();i++)
		allNodes->at(i)->history_cost = 0;
}

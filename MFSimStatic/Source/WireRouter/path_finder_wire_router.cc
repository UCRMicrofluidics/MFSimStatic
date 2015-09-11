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

#include "../../Headers/WireRouter/path_finder_wire_router.h"
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
PathFinderWireRouter::PathFinderWireRouter(DmfbArch *dmfbArch)
{
	arch = dmfbArch;
	type = PATH_FINDER_WR;
}

///////////////////////////////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////////////////////////////
PathFinderWireRouter::~PathFinderWireRouter()
{
	//cout << "DESTROYING PATH FINDER WIRE ROUTER..." << endl;
	/*unsigned max_layers = layers.size();
	int i = 0;
	while(!layers.empty()) {
		vector<Path*>* current_layer = &layers.back();
		layers.pop_back();
		unsigned max_paths = current_layer->size();
		int j = 0;
		while(!current_layer->empty()) {
			Path* current = current_layer->back();
			current_layer->pop_back();
			current = NULL;
			j++;
		}
		i++;
	}*/
	for(int i = 0; i < layers.size(); ++i)
	{
		for(int j = 0; j < layers.at(i).size(); ++j)
		{
			delete layers.at(i).at(j);
		}
		layers.at(i).clear();
	}
}

///////////////////////////////////////////////////////////////////////////////////
// Given the pin-mapping (contained in the DmfbArch), computes the wire routing
// to connect all the pins together and to the output of the device.  Also, has
// access to the pin activations list in case this algorithm needs to modify the
// pin-mapping itself.  This algorithm uses lee's maze routing.
///////////////////////////////////////////////////////////////////////////////////
void PathFinderWireRouter::computeWireRoutes(vector<vector<int> *> *pinActivations, bool isIterative)
{
	cout << "Beginning wire routing phase:" << endl;
	layeredPathfinder(model);

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
	if (!isIterative)
		convertWireSegments(&layers,wires);
}

//void PathFinderWireRouter::computeWireRoutesTest()
//{
//	cout << "Beginning wire routing phase:" << endl;
//	layeredPathfinder(model);
//
//	maxPinNum = 0;
//	map<int, vector<WireRouteNode *> *>::iterator groupIt;
//	for (groupIt = model->getPinGroups()->begin();groupIt != model->getPinGroups()->end();groupIt++)
//		if (maxPinNum == 0 || groupIt->first > maxPinNum)
//			maxPinNum = groupIt->first;
//	maxPinNum++;
//
//	vector< vector<WireSegment *> *> *wires = arch->getWireRouter()->getWireRoutesPerPin();
//	for (int i = 0; i < maxPinNum; i++)
//	{
//		vector<WireSegment *> *wire = new vector<WireSegment *>();
//		wires->push_back(wire);
//	}
//
//	////////////////////////////////////////////////
//	// Convert results to the actual wire segments//
//	////////////////////////////////////////////////
//	//convertWireSegments(&layers,wires);
//}
///////////////////////////////////////////////////////////////////////////////////
// This function takes in the layers that contain paths (the internal representation
// in this wire-router) and converts them to the wire segments needed by the
// framework.
///////////////////////////////////////////////////////////////////////////////////
void PathFinderWireRouter::convertWireSegments(vector<vector<Path*> >* layers, vector<vector<WireSegment*>*>* wires)
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

	if (kLayerMinimization)
		cout << "Total number of pins routed off DMFB w/ layer minimization: " << num_pins << endl;
	else
		cout << "Total number of pins routed off DMFB: " << num_pins << endl;
}




///////////////////////////////////////////////////////////////////////////////////
// Tests whether new_path intersects with any path contained in old_paths
// vector. This is used to create layers.
// Returns: True if intersection exists, false otherwise
///////////////////////////////////////////////////////////////////////////////////
bool intersects(Path* new_path,vector<Path*>* old_paths)
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

void PathFinderWireRouter::change_tile_costs(vector<WireRouteNode*>& allNodes) {
	int tile_x_size = (this->getNumHorizTracks() * 2) + 3;
	int tile_y_size = (this->getNumVertTracks() * 2) + 3;
	int overlap_x_nodes = kXTilesCheaper - 1;
	int overlap_y_nodes = kYTilesCheaper - 1;
	int x_cheaper_nodes = (tile_x_size * kXTilesCheaper) - overlap_x_nodes;
	int y_cheaper_nodes = (tile_x_size * kYTilesCheaper) - overlap_y_nodes;
	int center_high_x = model->getWireGridXSize() - x_cheaper_nodes;
	int center_high_y = model->getWireGridYSize() - y_cheaper_nodes;
	int center_low_x = x_cheaper_nodes;
	int center_low_y = y_cheaper_nodes;
	for (unsigned i = 0;i < allNodes.size();i++) {
		WireRouteNode* current_node = allNodes.at(i);
		bool center_x = center_low_x < current_node->wgX  && current_node->wgX < center_high_x;
		bool center_y = center_low_y < current_node->wgY  && current_node->wgY < center_high_y;
		bool super_sink = current_node->wgX == -1 && current_node->wgY == -1;
		if ((center_x || center_y) && !super_sink) {
			current_node->tile_cost = kCenterTileCost;
		} else {
			current_node->tile_cost = 0;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////
// Performs Layered pathfinder.
// 1. Clear the history cost of all nodes.
// 2. Perfrom pathfinder to route all of the pins, minimizing node sharing.
// 3. Sort the routes by the number of pins they share (least to most).
// 4. Add paths to the current layer.
// 5. Push intersecting paths to the next layer.
// 6. Clean up the current layer (running pathfinder again)
///////////////////////////////////////////////////////////////////////////////////
//vector<vector<Path> > PathFinderWireRouter::layeredPathfinder(DiagonalWireRoutingModel* model)
void PathFinderWireRouter::layeredPathfinder(DiagonalWireRoutingModel* model)
{
	vector<WireRouteNode *> allNodes = *(model->getAllNodes());
	map<int, vector<WireRouteNode *> *> pinGroups = *(model->getPinGroups());
	map<int, vector<WireRouteNode *> *> reRouteGroups;
	WireRouteNode* super_escape = model->getSuperEscape();

	if (kCheaperCornerTiles) {
		change_tile_costs(allNodes);
	}

	int total_paths = pinGroups.size();
	int layer_number = 1;
	int remaining_paths = total_paths;
	while (!pinGroups.empty())
	{
		cout << "Routing Layer " << layer_number;
		remaining_paths = pinGroups.size();
		clearHistory(&allNodes);

		// Timer code for individual layers
		char timerName[1024];
		sprintf(timerName,"Layer %d",layer_number);
		ElapsedTimer sTime(timerName);
		sTime.startTimer();
		pathfinder(allNodes,pinGroups,super_escape);
		// Timer code for individual layers
		sTime.endTimer();
		cout << endl;
		sTime.printElapsedTime();

		if (layers.back().empty())
			return;

		Sort::sortPathsBySharedPinSize(&layers.back());

		vector<Path*>::iterator path_iter = (layers.back()).begin();
		/* Add paths to the current layer, then clean (if kCleanRoute is set true). */
		for (path_iter; path_iter != (layers.back()).end(); path_iter++)
		{
			if (!kLayerMinimization)
				(*path_iter)->addPathSegment(super_escape,(*path_iter)->nodeAt(0));

			if (kCleanRoute)
			{
				int pin_number = (*path_iter)->getPinNumber();
				reRouteGroups.insert(pair<int,vector<WireRouteNode*>*>(pin_number,pinGroups.at(pin_number)));
			}
			else
			{
				// Now convert used pins to internal nodes so lower layers can route through them - DTG
				vector<WireRouteNode *> *pinsToErase = pinGroups[(*path_iter)->getPinNumber()];
				for (int i = 0; i < pinsToErase->size(); i++)
					pinsToErase->at(i)->nodeType = INTERNAL_WRN;

				pinGroups.erase((*path_iter)->getPinNumber());
			}
		}
		if (kCleanRoute)
		{
			cerr << "Cleaning up layer " << layer_number << "..." << endl;
			layers.pop_back();

			clearHistory(&allNodes);
			pathfinder(allNodes,reRouteGroups,super_escape);

			Sort::sortPathsBySharedPinSize(&layers.back());
			vector<Path*>::iterator path_iter = layers.back().begin();
			for (path_iter; path_iter != layers.back().end(); path_iter++)
			{
				if (!kLayerMinimization)
					(*path_iter)->addPathSegment(super_escape,(*path_iter)->nodeAt(0));

				// Now convert used pins to internal nodes so lower layers can route through them - DTG
				vector<WireRouteNode *> *pinsToErase = pinGroups[(*path_iter)->getPinNumber()];
				for (int i = 0; i < pinsToErase->size(); i++)
					pinsToErase->at(i)->nodeType = INTERNAL_WRN;

				pinGroups.erase((*path_iter)->getPinNumber());
			}
			reRouteGroups.clear();
		}

		layer_number++;
		cout << layers.back().size() << " pins routed." << endl;
	}
}

///////////////////////////////////////////////////////////////////////////////////
// Tests whether new_path intersects with any path contained in old_paths
// vector.
// Returns: vector of intersecting indices
///////////////////////////////////////////////////////////////////////////////////
vector<int> intersecting_paths(Path* new_path,vector<Path>* old_paths)
		{
	vector<int> intersecting_indices;
	vector<WireRouteNode*> shared_nodes;
	for (unsigned i = 0;i < new_path->pathSize();i++) {
		if (new_path->nodeAt(i)->nodeType != SUPER_ESCAPE_WRN && new_path->nodeAt(i)->occupancy > 1) {
			shared_nodes.push_back(new_path->nodeAt(i));
		}
	}
	if (shared_nodes.empty())
		return intersecting_indices;

	for (unsigned i = 0;i < old_paths->size();i++) {
		bool not_intersecting = true;
		for (unsigned j = 0;j < old_paths->at(i).pathSize() && not_intersecting;j++) {
			for (unsigned k = 0;k < shared_nodes.size() && not_intersecting;k++) {
				if (old_paths->at(i).nodeAt(j) == shared_nodes.at(k)) {
					intersecting_indices.push_back(i);
					not_intersecting = false;
				}
			}
		}
	}
	return intersecting_indices;
		}

///////////////////////////////////////////////////////////////////////////////////
// This function calls path-finder on each of the pinGroups
///////////////////////////////////////////////////////////////////////////////////
//vector<Path> PathFinderWireRouter::pathfinder(vector<WireRouteNode*> allNodes, map<int, vector<WireRouteNode*>* > pinGroups,
void PathFinderWireRouter::pathfinder(vector<WireRouteNode*> allNodes, map<int, vector<WireRouteNode*>* > pinGroups,
		WireRouteNode* super_escape,bool allowFails)
{
	vector<Path*> paths;
	vector<Path*> best_layer; // Save the best layer results
	vector<Path*> current_layer;
	unsigned iteration = 0;
	int failed_routes = pinGroups.size();
	double pfac = 0.5;


	// TODO: DTG - Add sorting function to sort pinGroups (closest to edge --> farthest from edge)
	// Get sorted pin-groups (copy to vector first)
	vector<vector<WireRouteNode*>* > *sortedPinGroups = new vector< vector<WireRouteNode*>* >();
	map<int, vector<WireRouteNode *> *>::iterator groupTestIt;
	for (groupTestIt = pinGroups.begin(); groupTestIt != pinGroups.end(); groupTestIt++)
		sortedPinGroups->push_back(groupTestIt->second);

	//Sort::sortPinGroupsByAvgMinDistToEdge(sortedPinGroups);
	//Sort::sortPinGroupsByPinGroupSize(sortedPinGroups);
	Sort::sortPinGroupsByPinGroupArea(sortedPinGroups);

	while (iteration < getKMaxIterations() && failed_routes > 0)
	{
		// Output progress every 10%
		if (getKMaxIterations() < 10)
			cout << "." << flush;
		else if (iteration % (getKMaxIterations()/10) == 0)
			cout << "." << flush;

		// Save the best (make sure has the most pins and no routes intersect)
		if (isSavingBest())
		{
			vector<Path*>::iterator path_iter = paths.begin();
			for (path_iter; path_iter != paths.end(); path_iter++)
				if (!intersects((*path_iter),&current_layer)) {
					current_layer.push_back((*path_iter));
				} else {
					if (allowFails) {
						current_layer.push_back((*path_iter));
						cerr << "Pin " << (*path_iter)->getPinNumber() << " fails." << endl;
					}
				}

			if (current_layer.size() > best_layer.size())
			{
				//(ZZ): Change made to prevent memory leaks (9/11/2014)
				while(!best_layer.empty())
				{
					delete best_layer.back();
					best_layer.pop_back();
				}
				//End of changes
				best_layer = current_layer;
			}
			current_layer.clear();


		}

		failed_routes = 0;
		//Rip up all existing routes, set history costs, clear penalty costs
		int shared_nodes = 0;
		for (unsigned i = 0;i < allNodes.size();i++)
		{
			WireRouteNode* current_node = allNodes.at(i);
			if (current_node->occupancy > 1)
				shared_nodes++;

			if (current_node->nodeType != SUPER_ESCAPE_WRN)
				current_node->history_cost = historyCost(current_node->history_cost,current_node->occupancy);
			else
				current_node->history_cost = 0;

			current_node->penalty_cost = 0;
			current_node->occupancy = 0;
			current_node->iteration = -1;
			current_node->claimedPin = -1;
			current_node->spawn = NULL;

			//(ZZ): Change made to prevent memory leaks
			clearPaths(&paths, &best_layer);
			//End of changes
			paths.clear();

		}

		//Route everything, one route at a time, updating occupancy throughout.
		for (int i = 0; i < sortedPinGroups->size(); i++)
		{
			vector<WireRouteNode *> pinGroup = *(sortedPinGroups->at(i));
			int pinNum = pinGroup.front()->originalPinNum;

			Path* new_path = new Path();
			leeMazeRouting(pinNum,pfac,super_escape,pinGroup, iteration,new_path);
			super_escape->claimedPin = -1;
			int shared = 0;
			if (new_path->empty())
			{
				//(ZZ): Changed to prevent memory Leaks (9/11/2014)
				delete new_path;
				// end of changes
				failed_routes++;
				std::cerr << "Error: Pathfinder: Could not find a route for pin group " << pinNum << ".\n";
			}
			else
			{
				for (unsigned i = 0;i < new_path->pathSize();i++)
					if (new_path->nodeAt(i)->nodeType != SUPER_ESCAPE_WRN && new_path->nodeAt(i)->occupancy > 1)
						shared++;

				if (shared >= 1)
					failed_routes++;

				new_path->setCost(new_path->pathSize());
				new_path->setSharedNodes(shared);
				new_path->setPinNumber(pinNum);
				paths.push_back(new_path);
			}
		}

		//Increase iteration, increase the penalty for sharing a node
		iteration++;
		pfac = pfac * getKPFacIncrease();
	}

	delete sortedPinGroups;

	// Just make sure we output the same number of "."s for progress indicator
	while (iteration < getKMaxIterations())
	{
		if (getKMaxIterations() < 10)
			cout << "." << flush;
		else if (iteration % (getKMaxIterations()/10) == 0)
			cout << "." << flush;
		iteration++;
	}

	// Output the best layer results
	if (allowFails) {
		layers.push_back(paths);
	}
	if (best_layer.empty())
		layers.push_back(paths);
	else
	{
		layers.push_back(best_layer);
		//(ZZ): Changed to prevent memory leaks (9/11/2014)
		clearPaths(&paths, &best_layer);
		// End of changes
	}
}

///////////////////////////////////////////////////////////////////////////////////
// This function prints the contents of the path.
///////////////////////////////////////////////////////////////////////////////////
void PathFinderWireRouter::printPath(vector<WireRouteNode*>* path)
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
void PathFinderWireRouter::leeMazeRouting(int pin_number,double pfac,WireRouteNode* source, vector<WireRouteNode*> sinks, int iterationNum,Path* new_path)
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
		new_path->clear();
	}
}

///////////////////////////////////////////////////////////////////////////////////
// Appends the nodes in appended vector to the path original
///////////////////////////////////////////////////////////////////////////////////
void PathFinderWireRouter::append(Path* original, vector<WireRouteNode*>* appended)
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
bool PathFinderWireRouter::traceBack(double pfac,WireRouteNode* sink,Path* sources, int iterationNum)
{
	vector<WireRouteNode*> path;
	path.push_back(sink);
	bool source_found = false;
	WireRouteNode* current_node = sink;
	WireRouteNode* next_node = NULL;
	while (!source_found)
	{
		next_node = findPrevious(current_node);
		if (next_node == NULL)
		{
			std::cerr << "Error: Pathfinder: Trace back failed.\n";
			//TODO: clear claimedPins
			return false;
		}
		PathSegment* source_segment = sources->findSegment(next_node);
		if (source_segment) {
			source_found = true;
		}

		path.push_back(next_node);
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
	if (!kLayerMinimization)
	{
		sources->removeSuper();
	}

	append(sources,&path);
	//Added to prevent memory leaks (ZZ) 9/24/2014
	path.clear();
	//end of changes
	return true;
}

///////////////////////////////////////////////////////////////////////////////////
// Finds the previous node in the trace back. Traces back to the node that
// spawned the current node during the grid fill step
///////////////////////////////////////////////////////////////////////////////////
WireRouteNode* PathFinderWireRouter::findPrevious(WireRouteNode* current_node)
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

		// IF kCheaperDiagonalCost
		// ELSE orthogonal_cost should always be 0, so does nothing.
		lhs_cost += lhs->orthogonal_cost;
		rhs_cost += rhs->orthogonal_cost;

		// IF kCheaperCornerTiles
		// ELSE tile_cost should always be 0, so does nothing.
		lhs_cost += lhs->tile_cost;
		rhs_cost += rhs->tile_cost;

		return lhs_cost > rhs_cost;
	}
};

//TODO: debugging only
int sink_distance(WireRouteNode* n1,vector<WireRouteNode*>* sinks) {
	int min_distance = -1;
	for (unsigned i = 0;i < sinks->size();i++) {
		int dist = distance(n1,sinks->at(i));
		if (min_distance < 0 || dist < min_distance) {
			min_distance = dist;
		}
	}
	return min_distance;
}

int distance(WireRouteNode* n1,WireRouteNode* n2) {
	int dist = abs(n2->wgX - n1->wgX) + abs(n2->wgY - n1->wgY);
	return dist;
}
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
vector<WireRouteNode*> PathFinderWireRouter::fillGrid(int pin_number,double pfac,Path* sources,vector<WireRouteNode*>* sinks)
{
	vector<WireRouteNode*> searched_nodes;

	/* Set up the expanding wavefront */
	priority_queue<WireRouteNode*, vector<WireRouteNode*>, CompareNodes> wavefront;
	for (unsigned i = 0;i < sources->pathSize();i++)
	{
		wavefront.push(sources->nodeAt(i));
		searched_nodes.push_back(sources->nodeAt(i));
		sources->nodeAt(i)->iteration = 1;
	}
	int min_distance = -1;
	WireRouteNode* closest_node;
	while (!wavefront.empty())
	{
		WireRouteNode* current_node = wavefront.top();
		wavefront.pop();
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
							if (!kCheaperDiagonalCost) {
								wavefront.push(expanse);
							} else {
								int delta_x = std::abs(expanse->wgX - current_node->wgX);
								int delta_y = std::abs(expanse->wgY - current_node->wgY);
								if (delta_x == 0 || delta_y == 0) { //Orthogonal
									expanse->orthogonal_cost = 1;
								} else {
									expanse->orthogonal_cost = 0;
								}
								wavefront.push(expanse);
							}
							searched_nodes.push_back(expanse);
						}
					}
				}
			}
		}
	}
	std::cerr << "Error: Pathfinder: Sink node could not be found. " << endl;
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
int PathFinderWireRouter::isSink(WireRouteNode* node,vector<WireRouteNode*>* sinks)
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
int PathFinderWireRouter::historyCost(int old_history,int occupancy)
{
	if (occupancy > 1)
		return old_history + ((occupancy - 1) * getKHFac());
	else
		return old_history >= 1? old_history:1;
}

///////////////////////////////////////////////////////////////////////////////////
// Clears the history cost of allNodes vector.
///////////////////////////////////////////////////////////////////////////////////
void PathFinderWireRouter::clearHistory(vector<WireRouteNode*>* allNodes)
{
	for (unsigned i = 0;i < allNodes->size();i++)
		allNodes->at(i)->history_cost = 0;
}

///////////////////////////////////////////////////////////////////////////////
// Clears all but the elements in best_layer from paths
///////////////////////////////////////////////////////////////////////////////
void PathFinderWireRouter::clearPaths(vector<Path*>* paths, vector<Path*>* best_layer)
{
	for(int i = 0; i < paths->size(); ++i)
	{
		bool good = true;
		for(int j = 0; j < best_layer->size(); ++j)
		{
			if(paths -> at(i) == best_layer -> at(j))
			{
				good = false;
				break;
			}

		}
		if(good)
		{
			delete paths -> at(i);
		}
	}
}

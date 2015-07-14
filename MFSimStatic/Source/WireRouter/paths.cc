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
 * Source: paths.cc																*
 * Original Code Author(s): Jeff McDaniel										*
 * Original Completion/Release Date: 1/24/2014									*
 *																				*
 * Details: N/A																	*
 *																				*
 * Revision History:															*
 * WHO		WHEN		WHAT													*
 * ---		----		----													*
 * FML		MM/DD/YY	One-line description									*
 *-----------------------------------------------------------------------------*/

#include "../../Headers/WireRouter/paths.h"
#include <iostream>

///////////////////////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////////////////////
PathSegment::PathSegment(WireRouteNode* new_node)
{
	node = new_node;
}

///////////////////////////////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////////////////////////////
PathSegment::~PathSegment() { }

///////////////////////////////////////////////////////////////////////////////////
// Adds a segment to the path segment's next segments.
///////////////////////////////////////////////////////////////////////////////////
void PathSegment::addSegment(PathSegment* new_segment)
{
	next_segments.push_back(new_segment);
}

///////////////////////////////////////////////////////////////////////////////////
// Finds the next segment.
///////////////////////////////////////////////////////////////////////////////////
PathSegment* PathSegment::nextSegmentsAt(unsigned index)
{
	if (index >= next_segments.size())
		return NULL;
	return next_segments.at(index);
}

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////////////////////
Path::Path()
{
	cost = 0;
	shared_nodes = 0;
	pin_number = 0;
}

///////////////////////////////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////////////////////////////
Path::~Path()
{
	while (!path.empty()) {
		PathSegment* current = path.back();
		path.pop_back();
		delete current;
	}
}
		
void Path::clear() {
	while (!path.empty()) {
		PathSegment* current = path.back();
		path.pop_back();
		delete current;
	}
}

///////////////////////////////////////////////////////////////////////////////////
// Adds a path segment to the path.
///////////////////////////////////////////////////////////////////////////////////
void Path::addPathSegment(WireRouteNode* new_node,WireRouteNode* parent)
{
	PathSegment* new_segment = new PathSegment(new_node);
	if (parent != NULL) {
		PathSegment* parent_segment = findSegment(parent);
		if (parent_segment) {
			parent_segment->addSegment(new_segment);
		}
	}
	path.push_back(new_segment);
}

///////////////////////////////////////////////////////////////////////////////////
// Find a segment in the path.
///////////////////////////////////////////////////////////////////////////////////
PathSegment* Path::findSegment(WireRouteNode* key)
{
	for (unsigned i = 0;i < path.size();i++)
		if (path.at(i)->getNode() == key)
			return path.at(i);
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////
// Removes the super escape node from the path.
///////////////////////////////////////////////////////////////////////////////////
void Path::removeSuper()
{
    for (std::vector<PathSegment*>::iterator iter = path.begin();iter != path.end();iter++)
    {
        if ( (*iter)->getNode()->nodeType == SUPER_ESCAPE_WRN)
        {
            (*iter)->getNode()->iteration = -1;
            path.erase(iter);
			if (iter == path.end())
				return;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////
// Prints the path.
///////////////////////////////////////////////////////////////////////////////////
void Path::printPath()
{
	if (path.empty())
	{
		std::cerr << "Path empty.\n";
		return;
	}
	vector<PathSegment*> current_level;
	vector<PathSegment*> next_level;
	next_level.push_back(path.at(0));
	while (!next_level.empty())
	{
		current_level = next_level;
		next_level.clear();
		for(unsigned i = 0;i < current_level.size();i++)
		{
			WireRouteNode* current_node = current_level.at(i)->getNode();
			for (unsigned j = 0;j < current_level.at(i)->nextSegmentsSize();j++)
				next_level.push_back(current_level.at(i)->nextSegmentsAt(j));

			std::cout << "(" << current_node->wgX 
					  << "," << current_node->wgY << ") "
					  << pin_number << "\t";
		}
		current_level.clear();
		std::cout << endl;
	}
}

///////////////////////////////////////////////////////////////////////////////////
// Finds the segment at the index in the path.
///////////////////////////////////////////////////////////////////////////////////
PathSegment* Path::segmentAt(unsigned index)
{
	if (index >= path.size())
		return NULL;
	return path.at(index);
}

///////////////////////////////////////////////////////////////////////////////////
// Finds the node at the index in the path.
///////////////////////////////////////////////////////////////////////////////////
WireRouteNode* Path::nodeAt(unsigned index)
{
	if (index >= path.size())
		return NULL;
	return path.at(index)->getNode();
}

///////////////////////////////////////////////////////////////////////////////////
// Adds a shared pin to the path.
///////////////////////////////////////////////////////////////////////////////////
void Path::addSharedPin(int new_pin)
{
	for (unsigned i = 0;i < shared_pins.size();i++)
		if (shared_pins.at(i) == new_pin)
			return;
	shared_pins.push_back(new_pin);
}

///////////////////////////////////////////////////////////////////////////////////
// Sets the iteration found at to 1. This tricks the maze routing algorithm
// into thinking the node has already been expanded and does not add it to the
// expanding wavefront, essentially preventing sharing on this path.
///////////////////////////////////////////////////////////////////////////////////
void Path::disallowSharing() {
	for (unsigned i = 0;i < path.size();i++) {
		WireRouteNode* current_node = path.at(i)->getNode();
		current_node->iteration = 1;
	}
}

///////////////////////////////////////////////////////////////////////////////////
// Sets the iteration found at to -1. This reverses the trick of
// disallowSharing() to once again allow sharing, essentially restoring the
// state of the nodes along the path.
///////////////////////////////////////////////////////////////////////////////////
void Path::allowSharing() {
	for (unsigned i = 0;i < path.size();i++) {
		WireRouteNode* current_node = path.at(i)->getNode();
		current_node->iteration = -1;
	}
}

///////////////////////////////////////////////////////////////////////////////////
// Finds the shared pin at the index in the path.
///////////////////////////////////////////////////////////////////////////////////
int Path::sharedPinsAt(unsigned index)
{
	if (index >= shared_pins.size())
		return -1;
	return shared_pins.at(index);
}

int historyCost(int old_history,int occupancy)
{
	if (occupancy > 1)
		return old_history + (occupancy - 1);
	else
		return old_history >= 1? old_history:1;
}

void Path::rip_up() {
	pin_number = 0;
	cost = 0;
	shared_nodes = 0;
	for (unsigned i = 0;i < pathSize();i++) {
		WireRouteNode* current_node = nodeAt(i);
		/* Using an occupancy of '2' increases the history cost of the entire path */
		current_node->history_cost = historyCost(current_node->history_cost,2);
		current_node->penalty_cost = 0;
		current_node->occupancy = 0;
		current_node->iteration = -1;
		current_node->claimedPin = -1;
		current_node->spawn = NULL;
	}
	path.clear();
	shared_pins.clear();
}

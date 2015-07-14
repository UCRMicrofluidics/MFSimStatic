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
 * Source: paths.h																*
 * Original Code Author(s): Jeff McDaniel										*
 * Original Completion/Release Date: 1/24/2014 									*
 *																				*
 * Details: N/A																	*
 *																				*
 * Revision History:															*
 * WHO		WHEN		WHAT													*
 * ---		----		----													*
 * FML		MM/DD/YY	One-line description									*
 *-----------------------------------------------------------------------------*/

#ifndef _PATHFINDER_PATHS_H_
#define _PATHFINDER_PATHS_H_

#include <vector>
#include "../Resources/structs.h"

class PathSegment
{
	private:
		WireRouteNode* node;
		std::vector<PathSegment*> next_segments;
	public:
		PathSegment(WireRouteNode* new_node);
		~PathSegment();

		/* Mutators */
		void setNode(WireRouteNode* new_node) {node = new_node;}
		void addSegment(PathSegment* new_segment);

		/* Accessors */
		inline WireRouteNode* getNode() {return node;}
		inline unsigned nextSegmentsSize() {return next_segments.size();}
		PathSegment* nextSegmentsAt(unsigned index);
};

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

class Path
{
	private: 
		std::vector<int> shared_pins;
		int pin_number;
		int cost;
		int shared_nodes;
		std::vector<PathSegment*> path;

	public:
		Path();
		Path(Path& copy_from);
		~Path();
		
		/* Mutators */
		inline void setPinNumber(int new_pin_number) {pin_number = new_pin_number;}
		inline void setCost(int new_cost) {cost = new_cost;}
		inline void setSharedNodes(int new_shared_nodes) {shared_nodes = new_shared_nodes;}
		inline void addSharedNodes(int added_nodes = 1) {shared_nodes+=added_nodes;}
		void addPathSegment(WireRouteNode* new_node,WireRouteNode* parent = NULL);
		void addSharedPin(int new_pin);
		void clear();
		void disallowSharing();
		void allowSharing();
		void rip_up();

		/* Accessors */
		inline unsigned sharedPinsSize() { return shared_pins.size(); }
		int sharedPinsAt(unsigned index);
		inline int getPinNumber() { return pin_number; }
		inline bool empty() { return path.empty(); }
		inline int getCost() { return cost; }
		inline int getSharedNodes() { return shared_nodes; }
		void removeSuper();
		inline unsigned pathSize() { return path.size(); }
		PathSegment* segmentAt(unsigned index);
		WireRouteNode* nodeAt(unsigned index);
		PathSegment* findSegment(WireRouteNode* key);

		/* Print Functions */
		void printPath();
};

#endif // _PATHFINDER_PATHS_H_

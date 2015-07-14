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
/*--------------------------------Class Details---------------------------------*
 * Name: Sort	 																*
 *																				*
 * Details: This class contains a number of various sorting	methods used		*
 * throughout the simulator that are called statically.  The purpose of this	*
 * class is so that the same sorting methods don't have to be re-written in 	*
 * several classes and to keep the rest of the source code clear of these		*
 * sorting algorithms...basically to keep the other files clean.				*
 *-----------------------------------------------------------------------------*/
#ifndef _SORT_H
#define _SORT_H

#include "../Models/fixed_module.h"
#include "../Models/assay_node.h"
#include "../Resources/structs.h"
#include "../WireRouter/paths.h"
#include <algorithm>
#include <vector>
#include <list>

struct VirtualDroplet;
class DmfbArch; // DTG

class Sort
{
	public:
		// Constructors
		Sort();
		virtual ~Sort();

		// Assay Node Sorting Methods
		static void sortNodesByStartTS(list<AssayNode *> *l);
		static void sortNodesByStartTSThenStorageFirst(list<AssayNode *> *l);
		static void sortNodesByPriorityHiFirst(list<AssayNode *> *l);
		static void sortNodesByPriorityLoFirst(list<AssayNode *> *l);
		static void sortNodesByPriorityLoFirst(vector<AssayNode *> *v);
		static void sortNodesByLimitedResources(list<AssayNode *> *l);
		static void sortNodesById(vector<AssayNode *> *v);
		static void sortNodesByModuleThenStartTS(vector<AssayNode*> *v);
		static void sortNodesByStartThenEndTS(vector<AssayNode *> *v);
		static void sortNodesByLatestTSThenStorage(vector<AssayNode *> *v);
		static void sortNodesByLength(vector<AssayNode *> *v);

		// Various Other Sorting Methods
		static void sortReconfigModsByStartThenEndTS(vector<ReconfigModule *> *v);
		static void sortRoutesByLength(vector<vector<RoutePoint *> *> *v, vector<Droplet *> *vd);
		static void sortPopBySchedTimes(vector< map<AssayNode *, unsigned> *> *pop, vector<unsigned> *times);
		static void sortDropletsInDecManhattanDist(vector<Droplet *> *routingThisTS, map<Droplet *, SoukupCell *> *sourceCells, map<Droplet *, SoukupCell *> *targetCells);
		static void sortPathNodesByStartTS(list<AssayPathNode *> *l);
		static void sortPathsBySharedPinSize(vector<Path *> *v);
		static void sortFixedModulesFromTopToBottom(vector<FixedModule*> *v);
		static void sortPortsNtoSthenPos(vector<IoPort *> *v);
		static void sortPortsStoNthenPos(vector<IoPort *> *v);
		static void sortModulesFromTopToBot(vector<ReconfigModule *> *v);
		static void sortModulesFromBotToTop(vector<ReconfigModule *> *v);
		static void sortFppcNodesInIncreasingRouteDistance(vector<AssayNode *> *v);

		static void sortPinGroupsByAvgMinDistToEdge(vector<vector<WireRouteNode*>* > *v);
		static void sortPinGroupsByPinGroupSize(vector<vector<WireRouteNode*>* > *v);
		static void sortPinGroupsByPinGroupArea(vector<vector<WireRouteNode*>* > *v);

};
#endif

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
 * Source: sort.cc																*
 * Original Code Author(s): Dan Grissom											*
 * Original Completion/Release Date: October 7, 2012							*
 *																				*
 * Details: N/A																	*
 *																				*
 * Revision History:															*
 * WHO		WHEN		WHAT													*
 * ---		----		----													*
 * FML		MM/DD/YY	One-line description									*
 *-----------------------------------------------------------------------------*/
#include "../../Headers/Util/sort.h"
#include "../../Headers/WireRouter/wire_router.h" // DTG

///////////////////////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////////////////////
Sort::Sort() {}

///////////////////////////////////////////////////////////////////////////////////
// Deconstructor
///////////////////////////////////////////////////////////////////////////////////
Sort::~Sort(){}

/////////////////////////////////////////////////////////////////
// Sorts nodes by starting time-step, from least to greatest
/////////////////////////////////////////////////////////////////
bool sNodesByStartTS(AssayNode *a1, AssayNode *a2) { return (a1->GetStartTS() < a2->GetStartTS()); }
bool sNodesByStartThenEndTS(AssayNode *a1, AssayNode *a2)
{
	if (a1->GetStartTS() == a2->GetStartTS())
		return (a1->GetEndTS() < a2->GetEndTS());
	else
		return (a1->GetStartTS() < a2->GetStartTS());
}
bool sPathNodesByStartTS(AssayPathNode *a1, AssayPathNode *a2) { return (a1->startTS < a2->startTS); }
/////////////////////////////////////////////////////////////////
// Sorts nodes by length, from shortest to longest
/////////////////////////////////////////////////////////////////
bool sNodesByLength(AssayNode *a1, AssayNode *a2) { return (a1->GetEndTS() - a1->GetStartTS() < a2->GetEndTS() - a2->GetStartTS()); }
/////////////////////////////////////////////////////////////////
// Sorts integers at derefrenced address in decreasing order
/////////////////////////////////////////////////////////////////
bool sDecreasingInts(int *i1, int *i2) { return ((*i1) > (*i2)); }

/////////////////////////////////////////////////////////////////
// Sorts by starting TS.  If a tie, puts the storage-holders
// first.
/////////////////////////////////////////////////////////////////
bool sNodesByStartTSThenStorageFirst(AssayNode *a1, AssayNode *a2)
{
	if (a1->GetStartTS() == a2->GetStartTS())
	{
		if (a1->GetType() == STORAGE_HOLDER && a2->GetType() != STORAGE_HOLDER)
			return true;
		else
			return false;
	}
	else
		return (a1->GetStartTS() < a2->GetStartTS());
}

/////////////////////////////////////////////////////////////////
// Sorts nodes by reconfig. module, and then by starting time-step,
// from least to greatest
/////////////////////////////////////////////////////////////////
bool sNodesByModuleThenStartTS(AssayNode *a1, AssayNode *a2)
{
	//if (a1->GetReconfigMod() != a2->GetReconfigMod())
	//	return (a1->GetReconfigMod() < a2->GetReconfigMod());
	//else
	//	return (a1->GetStartTS() < a2->GetStartTS());


	//;return true;

	if (a1->GetReconfigMod()->getId() == a2->GetReconfigMod()->getId())
		return (a1->GetStartTS() < a2->GetStartTS());
	else
		return (a1->GetReconfigMod()->getId() < a2->GetReconfigMod()->getId());
}

/////////////////////////////////////////////////////////////////
// Sorts nodes by priority, but puts outputs at the very front
// b/c they can ALWAYS go and free up system resources...not matter
// what their priority is.  HiFirst puts the higher numbers in front,
// while LoFirst puts the lower numbers first
/////////////////////////////////////////////////////////////////
bool sNodesByPriorityHiFirst(AssayNode *a1, AssayNode *a2)
{
	if (a1->GetType() == OUTPUT && a2->GetType() != OUTPUT)
		return true;
	else if (a1->GetType() != OUTPUT && a2->GetType() == OUTPUT)
		return false;
	return (a1->GetPriority() > a2->GetPriority());
}
bool sNodesByPriorityLoFirst(AssayNode *a1, AssayNode *a2)
{
	if (a1->GetType() == OUTPUT && a2->GetType() != OUTPUT)
		return true;
	else if (a1->GetType() != OUTPUT && a2->GetType() == OUTPUT)
		return false;
	return (a1->GetPriority() < a2->GetPriority());
}

///////////////////////////////////////////////////////////////
// Sort heat and detects to front of list b/c they have more
// stringent resource demands and should be processed first
///////////////////////////////////////////////////////////////
bool sNodesByLimitedResources(AssayNode *a1, AssayNode *a2)
{
	if ((a1->GetType() == HEAT || a1->GetType() == DETECT) && !(a2->GetType() == HEAT || a2->GetType() == DETECT))
		return true;
	else
		return false;

}

///////////////////////////////////////////////////////////////
// Shortest id first
///////////////////////////////////////////////////////////////
bool sNodesById(AssayNode *a1, AssayNode *a2) { return (a1->getId() < a2->getId()); }

///////////////////////////////////////////////////////////////
// Longest routes first
///////////////////////////////////////////////////////////////
bool sRoutesByLength(vector<RoutePoint *> *r1, vector<RoutePoint *> *r2)
{
	return r1->size() > r2->size();
}

///////////////////////////////////////////////////////////////
// Latest ending time-steps first, then sort the storage nodes
// to the end.
// *****Changed to:
// If is a storage node and not changing modules from parent
// node's module, then sorted to front
///////////////////////////////////////////////////////////////
bool sNodesByLatestTSThenStorage(AssayNode *a1, AssayNode *a2)
{
	if (a1->GetType() == STORAGE && a2->GetType() != STORAGE)
		return true;
	else if (a1->GetType() != STORAGE && a2->GetType() == STORAGE)
		return false;
	else if (a1->GetType() == STORAGE && a2->GetType() == STORAGE)
	{
		ReconfigModule *rm1 = a1->GetReconfigMod();
		ReconfigModule *rm2 = a2->GetReconfigMod();

		if (rm1->getTY() == rm2->getTY() && rm1->getLX() == rm2->getLX())
			return true;
		else
			return false;
	}
	else // Non-storage nodes
		return false;
	/*if (a1->GetEndTS() != a2->GetEndTS())
		return a1->GetEndTS() > a2->GetEndTS();
	else if (a1->GetType() == STORAGE && a2->GetType() != STORAGE)
		return false;
	else
		return true;*/
}
/////////////////////////////////////////////////////////////////
// Sorts reconfigurable modules by starting time-step, and then
// by ending time-step, from least to greatest
/////////////////////////////////////////////////////////////////
bool sReconfigModsByStartThenEndTS(ReconfigModule *r1, ReconfigModule *r2)
{
	if (r1->getStartTS() == r2->getStartTS())
		return (r1->getEndTS() < r2->getEndTS());
	else
		return (r1->getStartTS() < r2->getStartTS());
}

///////////////////////////////////////////////////////////////////////////////////
// Sorts paths based on shared pin size...least to greatest.
///////////////////////////////////////////////////////////////////////////////////
bool sPathsBySharedPinSize(Path *p1, Path *p2)
{
	return p1->sharedPinsSize() < p2->sharedPinsSize();
}

///////////////////////////////////////////////////////////////////////////////////
// Sorts pin groups based on their average minimum distance to an edge of the DMFB.
///////////////////////////////////////////////////////////////////////////////////
bool sPinGroupsByAvgMinDistToEdge(vector<WireRouteNode *> *pg1, vector<WireRouteNode *> *pg2)
{
	// Get arch and return if either group is empty
	DmfbArch *a = NULL;
	if (!pg1->empty() && !pg2->empty())
		a = pg1->front()->arch;
	else if (pg1->empty())
		return true;
	else
		return false;

	// Get edge extremes
	int wgXMax = a->getWireRouter()->getModel()->getWireGridXSize()-1;
	int wgYMax = a->getWireRouter()->getModel()->getWireGridYSize()-1;

	// Compute Averages
	double avg1 = 0;
	for (unsigned i = 0; i < pg1->size(); i++)
	{
		WireRouteNode *p = pg1->at(i);
		avg1 += min( min(p->wgX, p->wgY), min(wgXMax - p->wgX, wgYMax - p->wgY) );
	}
	avg1 = avg1 / (double)pg1->size();
	double avg2 = 0;
	for (unsigned i = 0; i < pg2->size(); i++)
	{
		WireRouteNode *p = pg2->at(i);
		avg2 += min( min(p->wgX, p->wgY), min(wgXMax - p->wgX, wgYMax - p->wgY) );
	}
	avg2 = avg2 / (double)pg2->size();

	// Output comparison
	if (avg1 == avg2)
		return pg1->front()->originalPinNum < pg2->front()->originalPinNum; // If same, order by pin number
	return (avg1 < avg2); // Else, order by smallest distance first
}

///////////////////////////////////////////////////////////////////////////////////
// Sorts pin groups based on their number of pins being shared. Least number of
// pins in a group is sorted toward the front.
///////////////////////////////////////////////////////////////////////////////////
bool sPinGroupsByPinGroupSize(vector<WireRouteNode *> *pg1, vector<WireRouteNode *> *pg2)
{
	return pg1->size() < pg2->size();
}

///////////////////////////////////////////////////////////////////////////////////
// Sort pin groups based on their area (bounding box).
///////////////////////////////////////////////////////////////////////////////////
bool sPinGroupsByPinGroupArea(vector<WireRouteNode *> *pg1, vector<WireRouteNode *> *pg2)
{
	// Get arch and return if either group is empty
	DmfbArch *a = NULL;
	if (!pg1->empty() && !pg2->empty())
		a = pg1->front()->arch;
	else if (pg1->empty())
		return true;
	else
		return false;

	// Get edge extremes
	int wgXMax = a->getWireRouter()->getModel()->getWireGridXSize()-1;
	int wgYMax = a->getWireRouter()->getModel()->getWireGridYSize()-1;

	int xMin1 = -1;
	int xMax1 = -1;
	int yMin1 = -1;
	int yMax1 = -1;
	int xMin2 = -1;
	int xMax2 = -1;
	int yMin2 = -1;
	int yMax2 = -1;
	int area1 = -1;
	int area2 = -1;

	// Compute Averages and extreme points
	double avg1 = 0;
	for (unsigned i = 0; i < pg1->size(); i++)
	{
		WireRouteNode *p = pg1->at(i);
		avg1 += min( min(p->wgX, p->wgY), min(wgXMax - p->wgX, wgYMax - p->wgY) );

		if (xMin1 == -1 || p->wgX < xMin1)
			xMin1 = p->wgX;
		if (xMax1 == -1 || p->wgX > xMax1)
			xMax1 = p->wgX;
		if (yMin1 == -1 || p->wgY < yMin1)
			yMin1 = p->wgX;
		if (yMax1 == -1 || p->wgY > yMax1)
			yMax1 = p->wgX;
	}
	avg1 = avg1 / (double)pg1->size();
	area1 = (xMax1 - xMin1 + 1) * (yMax1 - yMin1 + 1);

	double avg2 = 0;
	for (unsigned i = 0; i < pg2->size(); i++)
	{
		WireRouteNode *p = pg2->at(i);
		avg2 += min( min(p->wgX, p->wgY), min(wgXMax - p->wgX, wgYMax - p->wgY) );

		if (xMin2 == -1 || p->wgX < xMin2)
			xMin2 = p->wgX;
		if (xMax2 == -1 || p->wgX > xMax2)
			xMax2 = p->wgX;
		if (yMin2 == -1 || p->wgY < yMin2)
			yMin2 = p->wgX;
		if (yMax2 == -1 || p->wgY > yMax2)
			yMax2 = p->wgX;
	}
	avg2 = avg2 / (double)pg2->size();
	area2 = (xMax2 - xMin2 + 1) * (yMax2 - yMin2 + 1);

	// Output comparison
	if (area1 == area2)
		return (avg1 < avg2); // If same area, order by smallest distance first
	else
		return area1 < area2; // Do ones that take up least amount of space first

	//if (avg1 == avg2)
	//	return pg1->front()->originalPinNum < pg2->front()->originalPinNum; // If same, order by pin number
	//return (avg1 < avg2); // Else, order by smallest distance first
}

///////////////////////////////////////////////////////////////////////////////////
// Sorts the fixed modules based on their location on the DMFB, from top to bottom.
// In event of tie (same height), choose one of left.
///////////////////////////////////////////////////////////////////////////////////
bool sFixedModulesFromTopToBottom(FixedModule *fm1, FixedModule *fm2)
{
	if (fm1->getTY() == fm2->getTY())
		return fm1->getLX() < fm2->getLX();
	else
		return fm1->getTY() < fm2->getTY();
}

///////////////////////////////////////////////////////////////////////////////////
// Sorts the modules based on their location on the DMFB, from top to bottom.
// In event of tie (same height), choose one of left.
///////////////////////////////////////////////////////////////////////////////////
bool sModulesFromTopToBot(ReconfigModule *rm1, ReconfigModule *rm2)
{
	if (rm1->getTY() == rm2->getTY())
		return rm1->getLX() < rm2->getLX();
	else
		return rm1->getTY() < rm2->getTY();
}
bool sModulesFromBotToTop(ReconfigModule *rm1, ReconfigModule *rm2)
{
	if (rm1->getTY() == rm2->getTY())
		return rm1->getLX() < rm2->getLX();
	else
		return rm1->getTY() > rm2->getTY();
}

///////////////////////////////////////////////////////////////////////////////////
// Sorts the ports by DMFB side and then position.
///////////////////////////////////////////////////////////////////////////////////
bool sPortsNtoSthenPos(IoPort *p1, IoPort *p2)
{
	if (p1->getSide() == p2->getSide())
		return p1->getPosXY() < p2->getPosXY();
	else
		return p1->getSide() < p2->getSide();
}
bool sPortsStoNthenPos(IoPort *p1, IoPort *p2)
{
	if (p1->getSide() == p2->getSide())
		return p1->getPosXY() < p2->getPosXY();
	else
		return p1->getSide() > p2->getSide();
}

///////////////////////////////////////////////////////////////////////////////////
// This function assumes an FPPC architecture is being used sorts nodes in
// decreasing routing distance from source. This is specifically designed for the
// FPPC2 (and it's a quick, non-comprehensive optimization), but shouldn't break
// on the original FPPC layout.
///////////////////////////////////////////////////////////////////////////////////
bool sFppcNodesInIncreasingRouteDistance(AssayNode *n1, AssayNode *n2)
{
	// Get a reconfigurable module....
	ReconfigModule *rm;
	if (n1->GetReconfigMod())
		rm = n1->GetReconfigMod();
	else if (n2->GetReconfigMod())
		rm = n2->GetReconfigMod();
	else if (n1->GetChildren().at(0)->GetReconfigMod())
		rm = n1->GetChildren().at(0)->GetReconfigMod();
	else if (n2->GetChildren().at(0)->GetReconfigMod())
		rm = n2->GetChildren().at(0)->GetReconfigMod();
	else
		claim(false, "Could not find a suitable module in sFppcNodesInDecreasingRouteDistance to compute the central routing column index.");

	//...and then compute the central routing channel location
	int crcIndex = 0; // central routing column index
	if (rm->getResourceType() == SSD_RES)
		crcIndex = rm->getLX() - 2;
	else if (rm->getResourceType() == BASIC_RES)
		crcIndex = rm->getRX() + 2;
	else
		claim(false, "Unknown module type in sFppcNodesInDecreasingRouteDistance.");

	int d1 = 0;
	int d2 = 0;

	int x = 0;
	int y = 0;

	int ioPenalty = 500; // I/Os should be routed last, let things in modules be routed first

	// Only looking at N/S...E/W for original FPPC not supported
	if (n1->GetType() == DISPENSE)
	{
		AssayNode *c = n1->GetChildren().front();
		if (n1->GetIoPort()->getSide() == NORTH || n1->GetIoPort()->getSide() == SOUTH)
			x = n1->GetIoPort()->getPosXY();
		d1 = abs(crcIndex-x) + ioPenalty;
	}
	else
	{
		rm = n1->GetReconfigMod();
		AssayNode *c = n1->GetChildren().front();
		ReconfigModule *crm = c->GetReconfigMod();
		d1 = abs(crm->getBY());

	}

	if (n2->GetType() == DISPENSE)
	{
		AssayNode *c = n2->GetChildren().front();
		if (n2->GetIoPort()->getSide() == NORTH || n2->GetIoPort()->getSide() == SOUTH)
			x = n2->GetIoPort()->getPosXY();
		d2 = abs(crcIndex-x) + ioPenalty;
	}
	else
	{
		rm = n2->GetReconfigMod();
		AssayNode *c = n2->GetChildren().front();
		ReconfigModule *crm = c->GetReconfigMod();
		d2 = abs(crm->getBY());

	}

	return d1 < d2;
}
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
// Wrapper functions for sorts
/////////////////////////////////////////////////////////////////
void Sort::sortNodesByStartTS(list<AssayNode*>* l) { l->sort(sNodesByStartTS); }
void Sort::sortNodesByStartTSThenStorageFirst(list<AssayNode*>* l) { l->sort(sNodesByStartTSThenStorageFirst); }
void Sort::sortNodesByPriorityHiFirst(list<AssayNode*>* l) { l->sort(sNodesByPriorityHiFirst); }
void Sort::sortNodesByPriorityLoFirst(list<AssayNode*>* l) { l->sort(sNodesByPriorityLoFirst); }
void Sort::sortNodesByPriorityLoFirst(vector<AssayNode*>* v) { sort(v->begin(), v->end(), sNodesByPriorityLoFirst); }
void Sort::sortNodesByLimitedResources(list<AssayNode*>* l) { l->sort(sNodesByLimitedResources); }
void Sort::sortNodesById(vector<AssayNode*>* v) { sort(v->begin(), v->end(), sNodesById); }
void Sort::sortNodesByModuleThenStartTS(vector<AssayNode*>* v) { sort(v->begin(), v->end(), sNodesByModuleThenStartTS); }
void Sort::sortNodesByStartThenEndTS(vector<AssayNode *>* v) { sort(v->begin(), v->end(), sNodesByStartThenEndTS); }
void Sort::sortNodesByLatestTSThenStorage(vector<AssayNode *> *v) { sort(v->begin(), v->end(), sNodesByLatestTSThenStorage); }
void Sort::sortPathNodesByStartTS(list<AssayPathNode*>* l) { l->sort(sPathNodesByStartTS); }
void Sort::sortNodesByLength(vector<AssayNode*>* v) { sort(v->begin(), v->end(), sNodesByLength); }
void Sort::sortReconfigModsByStartThenEndTS(vector<ReconfigModule *>* v) { sort(v->begin(), v->end(), sReconfigModsByStartThenEndTS); }
void Sort::sortPathsBySharedPinSize(vector<Path *> *v) { sort(v->begin(), v->end(), sPathsBySharedPinSize); }
void Sort::sortPinGroupsByAvgMinDistToEdge(vector<vector<WireRouteNode*>* > *v) { sort(v->begin(), v->end(), sPinGroupsByAvgMinDistToEdge); }
void Sort::sortPinGroupsByPinGroupSize(vector<vector<WireRouteNode*>* > *v) { sort(v->begin(), v->end(), sPinGroupsByPinGroupSize); }
void Sort::sortPinGroupsByPinGroupArea(vector<vector<WireRouteNode*>* > *v) { sort(v->begin(), v->end(), sPinGroupsByPinGroupArea); }
void Sort::sortFixedModulesFromTopToBottom(vector<FixedModule*> *v) { sort(v->begin(), v->end(), sFixedModulesFromTopToBottom); }
void Sort::sortPortsNtoSthenPos(vector<IoPort *> *v) { sort(v->begin(), v->end(), sPortsNtoSthenPos); }
void Sort::sortPortsStoNthenPos(vector<IoPort *> *v) { sort(v->begin(), v->end(), sPortsStoNthenPos); }
void Sort::sortModulesFromTopToBot(vector<ReconfigModule *> *v) { sort(v->begin(), v->end(), sModulesFromTopToBot); }
void Sort::sortModulesFromBotToTop(vector<ReconfigModule *> *v) { sort(v->begin(), v->end(), sModulesFromTopToBot); }
void Sort::sortFppcNodesInIncreasingRouteDistance(vector<AssayNode *> *v) { sort(v->begin(), v->end(), sFppcNodesInIncreasingRouteDistance); }

void Sort::sortRoutesByLength(vector<vector<RoutePoint *> *> * v, vector<Droplet *> *vd)
{
	map<vector<RoutePoint *> *, Droplet *> link;
	for (unsigned i = 0; i < v->size(); i++)
		link[v->at(i)] = vd->at(i);
	sort(v->begin(), v->end(), sRoutesByLength);
	vd->clear();
	for (unsigned i = 0; i < v->size(); i++)
		vd->push_back(link[v->at(i)]);
}
void Sort::sortPopBySchedTimes(vector< map<AssayNode*, unsigned> *> *pop, vector<unsigned> * times)
{
	map<unsigned *, map<AssayNode*, unsigned> *> link;
	for (unsigned i = 0; i < pop->size(); i++)
		link[&(times->at(i))] = pop->at(i);

	for (unsigned i = 0; i < times->size(); i++)
		cout << times->at(i) << "(" << pop->at(i) << ")-";
	cout << endl;

	sort(times->begin(), times->end());



	pop->clear();
	for (unsigned i = 0; i < times->size(); i++)
		pop->push_back(link[&(times->at(i))]);

	for (unsigned i = 0; i < times->size(); i++)
		cout << times->at(i) << "(" << pop->at(i) << ")-";
	cout << endl;
	exit(1);
}

///////////////////////////////////////////////////////////////////////////////////////
// Sorts routingThisTS in decreasing order, based on the manhattan distance between
// the corresponding source and target cells.
///////////////////////////////////////////////////////////////////////////////////////
void Sort::sortDropletsInDecManhattanDist(vector<Droplet *> *routingThisTS, map<Droplet *, SoukupCell *> *sourceCells, map<Droplet *, SoukupCell *> *targetCells)
{
	vector<int *> distances;
	map<int *, Droplet *> link;
	for (unsigned i = 0; i < routingThisTS->size(); i++)
	{
		SoukupCell *s = sourceCells->at(routingThisTS->at(i));
		SoukupCell *t = targetCells->at(routingThisTS->at(i));
		int *manhattanDist = new int();
		*manhattanDist = abs(s->x - t->x) + abs(s->y - t->y);
		distances.push_back(manhattanDist);
		link[manhattanDist] = routingThisTS->at(i);
	}

	//for (int i = 0; i < routingThisTS->size(); i++)
	//	cout << "D" << routingThisTS->at(i)->getId() << ": " << *distances.at(i) << endl;

	sort(distances.begin(), distances.end(), sDecreasingInts);

	routingThisTS->clear();
	for (unsigned i = 0; i < distances.size(); i++)
		routingThisTS->push_back(link[distances.at(i)]);

	//for (int i = 0; i < routingThisTS->size(); i++)
	//	cout << "D" << routingThisTS->at(i)->getId() << ": " << *distances.at(i) << endl;


	while (!distances.empty())
	{
		int *i = distances.back();
		distances.pop_back();
		delete i;
	}
}

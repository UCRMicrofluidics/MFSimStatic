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
 * Source: router.cc															*
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
#include "../../Headers/Router/router.h"
#include "../../Headers/Router/cho_router.h"
#include <climits>

///////////////////////////////////////////////////////////////////////////////////
// Constructors
///////////////////////////////////////////////////////////////////////////////////
Router::Router()
{
	arch = NULL;
	hasExecutableSyntesisMethod = true;
}
Router::Router(DmfbArch *dmfbArch)
{
	arch = dmfbArch;
	hasExecutableSyntesisMethod = true;
}

///////////////////////////////////////////////////////////////////////////////////
// Deconstructor
///////////////////////////////////////////////////////////////////////////////////
Router::~Router()
{

}

///////////////////////////////////////////////////////////////////////////////////
// Generic route function inherited by implemented routers.
///////////////////////////////////////////////////////////////////////////////////
void Router::route(DAG *dag, DmfbArch *arch, vector<ReconfigModule *> *rModules, map<Droplet *, vector<RoutePoint *> *> *routes, vector<vector<int> *> *pinActivations, vector<unsigned long long> *tsBeginningCycle)
{
	claim(false, "No valid router was selected for the synthesis process or no method for 'route()' was implemented for the selected router.\n");
}

///////////////////////////////////////////////////////////////////////////////
// Sets parameters for router
///////////////////////////////////////////////////////////////////////////////
void Router::setRoutingParams(CompactionType ct, ProcessEngineType pet)
{
	compactionType = ct;
	processEngineType = pet;
}

///////////////////////////////////////////////////////////////////////////////
// Performs any one-time setup operations that need to be done to operate the
// router.  These operations should not be considered in routing timings.
///////////////////////////////////////////////////////////////////////////////
void Router::preRoute(DmfbArch *arch)
{
}
///////////////////////////////////////////////////////////////////////////////
// Performs any one-time cleanup operations specific to this router
///////////////////////////////////////////////////////////////////////////////
void Router::postRoute()
{
}

///////////////////////////////////////////////////////////////////////////////
// Prints out routes in vector of routes.
///////////////////////////////////////////////////////////////////////////////
void Router::printRoutes(vector<vector<RoutePoint *> *> *routes)
{
	for (unsigned i = 0; i < routes->size(); i++)
	{
		cout << "SubRoute " << i << ": ";
		for (unsigned j = 0; j < routes->at(i)->size(); j++)
		{
			if (routes->at(i)->at(j))
				cout << "(" << routes->at(i)->at(j)->x << "," << routes->at(i)->at(j)->y << ",C" << routes->at(i)->at(j)->cycle << ")-->";
			else
				cout << "(S,S,Cxxx)-->";
		}
		cout << endl;
	}
	cout << "-------------------------------------------------------------------------" << endl;
}

///////////////////////////////////////////////////////////////////////////////////
// Examines all the routes to see if any droplet is occupying the cell at the given
// XY coordinates.  Returns -1 if cell is free of droplets.
///////////////////////////////////////////////////////////////////////////////////
int Router::dropletOccupyingCell(int x, int y, map<Droplet *, vector<RoutePoint *> *> *routes, vector<vector<RoutePoint *> *> *subRoutes, vector<Droplet *> *subDrops)
{
	int id = -1;
	map<Droplet *, vector<RoutePoint *> *>::iterator it = routes->begin();
	for (; it != routes->end(); it++)
	{
		if (!it->second->empty() && it->second->back()->x == x && it->second->back()->y == y && !(it->second->back()->dStatus == DROP_OUTPUT || it->second->back()->dStatus == DROP_MERGING))
		{
			id = it->first->id;
			break;
		}
	}

	// If droplet has already been moved this routing phase, take into account
	for (unsigned q = 0; q < subDrops->size(); q++)
	{
		//Droplet *drop = subDrops->at(q);
		//vector<RoutePoint*> *debugRoute = subRoutes->at(q);
		//RoutePoint * debugRp = debugRoute->back();

		if (!subRoutes->at(q)->empty())
		{
			if (subDrops->at(q)->id != id && subRoutes->at(q)->back()->x == x && subRoutes->at(q)->back()->y == y)
				id = subDrops->at(q)->id;
			else if (subDrops->at(q)->id == id && !(subRoutes->at(q)->back()->x == x && subRoutes->at(q)->back()->y == y))
				id = -1;
		}
	}
	return id;
}

///////////////////////////////////////////////////////////////////////////////
// Given two droplet locations, returns true if one droplet is inside the
// interference region (IR) of the other; returns false, otherwise.
///////////////////////////////////////////////////////////////////////////////
bool Router::doesInterfere(RoutePoint *r1, RoutePoint *r2)
{
	return (abs(r1->x - r2->x) <= 1 && abs(r1->y - r2->y) <= 1);
}

///////////////////////////////////////////////////////////////////////
// Compacts a list of subRoutes so that they can be routed concurrently.
// subDrops contains the droplets and subRoutes contains the routes to
// be compacted.  The indices for subDrops/subRoutes MUST match (i.e.
// subRoutes[x] is the subRoute for the droplet at subDrops[x].
// When passing in subRoutes, each subRoute contains a number of routing
// points that does not consider any other subRoute (i.e. subRoute[x][i],
// which represents droplet x's position at time i, could be the same
// location as subRoute[y][i]). Compaction will prevent droplets from
// being in the same location at the same time, adding stalls when
// necessary, so that subRoutes[x][i] is compatible with subRoutes[y][j],
// for all x, y, i and j (i.e. no unintentional mixing occurs).

// IN SHORT: after this function is called, the second index of subRoutes
// can properly be viewed as the cycle number for this sub-problem b/c
// the compaction methods use static/dynamic rules to ensure no droplet
// enters the interference region (IR) of any other droplet at any time.
///////////////////////////////////////////////////////////////////////
void Router::compactRoutes(vector<Droplet *> *subDrops, vector<vector<RoutePoint *> *> *subRoutes)
{
	Sort::sortRoutesByLength(subRoutes, subDrops); // Route longer paths first

	// Ensure that all routes' last point is labeled appropriately
	for (unsigned i = 0; i < subRoutes->size(); i++)
		if (subRoutes->at(i)->size() > 0)
			if (subRoutes->at(i)->back()->dStatus != DROP_OUTPUT)
				subRoutes->at(i)->back()->dStatus = DROP_WAIT;


	if (compactionType == BEG_COMP)
		compactRoutesWithBegStalls(subRoutes);
	else if (compactionType == MID_COMP)
		compactRoutesWithMidStalls(subRoutes);
	else if (compactionType == CHO_COMP)
		((ChoRouter*)this)->compactRoutesByTimingAndFaultTolerance(subDrops, subRoutes);
	else if (compactionType == DYN_COMP)
		compactRoutesWithDynamicProgramming(subDrops, subRoutes);
	else if (compactionType == INHERENT_COMP) // Ben's addition
	{ /*Do nothing...compaction already done internally by router.*/ }
	else
		decompact(subRoutes);
}

///////////////////////////////////////////////////////////////////////////////
// Flattens the droplet routes so that they are routed one-by-one, instead
// of concurrently.
///////////////////////////////////////////////////////////////////////////////
void Router::decompact(vector<vector<RoutePoint *> *> *subRoutes)
{
	// Now add stalls to any droplets that had to wait to be routed/decompacted
	unsigned long long numStalls = 0;
	for (unsigned i = 0; i < subRoutes->size(); i++)
	{
		vector<RoutePoint *> *subRoute = subRoutes->at(i);
		if (subRoute->size() > 0)
		{
			for (unsigned j = 0; j < numStalls; j++)
				subRoute->insert(subRoute->begin(), NULL);
			numStalls = subRoute->size();
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
// Given a vector full of sequential routes, compacts them such that each
// equal index of any subRoute can be executed at the same time.  Adds any
// necessary stalls at the beginning of the route.
//
// Compacts the routes by combining the routes from longest to shortest.
// If route being placed interferes with a route that is already placed, the
// route being placed will be delayed by 1 cycle until it can be placed.
///////////////////////////////////////////////////////////////////////////////
void Router::compactRoutesWithBegStalls(vector<vector<RoutePoint *> *> *subRoutes)
{
	//if (startingTS == 33)
	//	printRoutes(subRoutes);

	int longestRoute = 0;
	if (subRoutes->size() > 0)
		longestRoute = subRoutes->at(0)->size();

	int numStallsToPrepend = 1;
	for (unsigned i = 0; i < subRoutes->size(); i++)
	{
		vector<RoutePoint *> *subRoute = subRoutes->at(i);
		RoutePoint *destPt = NULL;
		if (subRoute->size() > 0)
			destPt = subRoute->back();

		// Check entire route
		bool isInterference = false;
		int j = 0; // The index used to traverse a specific route/cycle

		//printRoutes(subRoutes);

		int numStallsInserted = 0;

		// If not outputting, check the max length b/c droplet will remain at destination.
		int cyclesToCheck = max((int)subRoute->size(), longestRoute);
		unsigned reachedDestinationAtIteration = 0;

		while (j != cyclesToCheck && subRoute->size() > 0)
		{
			int debugSubRouteNum = -1;
			RoutePoint *debugSubRouteRp = NULL;
			//printRoutes(subRoutes);

			RoutePoint *rp = NULL;
			if (j <= subRoute->size()-1)
				rp = subRoute->at(j);
			else
				rp = subRoute->back();

			if (rp)
			{
				// Check against the previous routes that have been compacted
				for (int k = 0; k < i; k++)
				{
					vector<RoutePoint *> *pastRoute = subRoutes->at(k);

					RoutePoint *rpLc = NULL; // This route's last cycle
					if (j > 0 && j <= subRoute->size()-1)
						rpLc = subRoute->at(j-1);
					else
						rpLc = subRoute->back();

					RoutePoint *rpNc = NULL; // This route's next cycle
					if (j+1 <= subRoute->size()-1)
						rpNc = subRoute->at(j+1);
					else
						rpNc = subRoute->back();

					RoutePoint *prp = NULL; // Past route's current cycle
					if (j <= pastRoute->size()-1)
						prp = pastRoute->at(j);
					else if (pastRoute->back()->dStatus != DROP_OUTPUT)
						prp = pastRoute->back();

					RoutePoint *prpLc = NULL; // Past route's last cycle
					if (j > 0 && j <= pastRoute->size()-1)
						prpLc = pastRoute->at(j-1);
					else if (pastRoute->back()->dStatus != DROP_OUTPUT)
						prpLc = pastRoute->back();
					else if (j <= pastRoute->size()+1)
						prpLc = pastRoute->back();

					RoutePoint *prpNc = NULL; // Past route's next cycle
					if (j+1 <= pastRoute->size()-1)
						prpNc = pastRoute->at(j+1);
					else if (pastRoute->back()->dStatus != DROP_OUTPUT)
						prpNc = pastRoute->back();

					// Check dynamic droplet rules so this and last droplet locations don't interfere
					if (prp && doesInterfere(rp, prp) && !(doesInterfere(rp, destPt) && prp->dStatus == DROP_WAIT))
						isInterference = true;
					if (prpLc && doesInterfere(rp, prpLc) && !(doesInterfere(rp, destPt) && prpLc->dStatus == DROP_WAIT)) // DTG, is this if-statement necessary??
						isInterference = true;

					if (prp && rpNc && doesInterfere(rpNc, prp) && !(doesInterfere(rpNc, destPt) && prp->dStatus == DROP_WAIT))
						isInterference = true;
					if (prpNc && doesInterfere(rp, prpNc) && !(doesInterfere(rp, destPt) && prpNc->dStatus == DROP_WAIT))
						isInterference = true;
					if (prp && rpLc && doesInterfere(rpLc, prp) && !(doesInterfere(rpLc, destPt) && prp->dStatus == DROP_WAIT))
						isInterference = true;
					if (prpLc && rpLc && doesInterfere(rpLc, prpLc) && !(doesInterfere(rpLc, destPt) && prpLc->dStatus == DROP_WAIT))
						isInterference = true;

					// Get debug info if found interference for possible display to user
					if (isInterference)
					{
						// DEBUG
						//if (startingTS == 89 && i == 3 /*#18 local*/&& k == 1 /*#22 other*/)
						//{
						//	cout << "Droplet\t" << "Last\t\t" << "Curr\t" << "Next\t" << "(TS " << startingTS << ", position " << j << ")" << endl;
						//	cout << "Other " << k << "\t" << Analyze::GetFormattedCell(prpLc) << "\t\t" << Analyze::GetFormattedCell(prp) << "\t" << Analyze::GetFormattedCell(prpNc) << endl;
						//	cout << "Local " << i << "\t" << Analyze::GetFormattedCell(rpLc) << "\t\t" << Analyze::GetFormattedCell(rp) << "\t" << Analyze::GetFormattedCell(rpNc) << endl << endl;
						//	int foo = 4 + 5;
						//}

						debugSubRouteNum = k;
						debugSubRouteRp = prp;
						break;
					}
				}
			}
			if (isInterference)
			{	// Add a few stalls at the beginning and try again
				for (int m = 0; m < numStallsToPrepend; m++)
					subRoute->insert(subRoute->begin(), NULL);
				isInterference = false;
				numStallsInserted += numStallsToPrepend;
				j = 0;

				if (subRoute->at(subRoute->size()-1)->dStatus != DROP_OUTPUT)
					cyclesToCheck++;
				reachedDestinationAtIteration = 0;
			}
			else
			{
				// If current route point has reached it's destination, then only check the next cycle or two
				// b/c it will be off the DMFB (in the case of an output) or at its destination and should no
				// longer cause interference
				if (rp && rp == destPt && reachedDestinationAtIteration == 0)
					reachedDestinationAtIteration = j;
				else if (rp && rp == destPt && reachedDestinationAtIteration + 2 == j)
					break;

				j++; // Increment local cycle
			}

			// Sanity check; ensure we're not in an infinite loop
			int stallInsertionLimit = 1000;
			if (numStallsInserted >= stallInsertionLimit)
			{
				stringstream msg;
				msg << "The stall insertion limit of " << stallInsertionLimit << " has been reached during route compaction." << endl;
				msg << "This likely indicates a problem with the individual route generation, such as blockages not being computed properly." << endl;
				msg << "The problem manifests with SubRoute #" << i << " of " << subRoutes->size();
				msg << ", specifically when the droplet at location (" << rp->x << ", " << rp->y << ")";
				msg << " interferes with the droplet in SubRoute #" << debugSubRouteNum << " at location (";
				if (debugSubRouteRp)
					msg << debugSubRouteRp->x << ", " << debugSubRouteRp->y << ")." << endl;
				else
					msg << "NULL)." << endl;
				msg << "A sub-route dump has been generated just above this.  It is recommended that you step through the compaction code to see where the uncompactable droplet interference is taking place to help debug your router." << endl;

				printRoutes(subRoutes);
				claim(false, &msg);

			}
		}
		if (j > longestRoute)
			longestRoute = j;
	}
	//printRoutes(subRoutes);

	// Power/energy computation code
	/*double electrodePitch = 2.54; //mm
	//double voltage = 13; //V
	double voltage = 35; //V (also tried 50V)

	double velocity = 0.005 * voltage*voltage + (0.0358*voltage) - 0.9103; // mm/s
	double cycleTime = electrodePitch / velocity;

	int numDroplets = subRoutes->size();

	int maxRouteCycles = 0;
	for (int i = 0; i < subRoutes->size(); i++)
		if (subRoutes->at(i)->size() > maxRouteCycles)
			maxRouteCycles = subRoutes->at(i)->size();

	double time = (double)((double)maxRouteCycles * cycleTime);
	double cumElectrodeActivationTime = (double)(time * (double)numDroplets);
	double energy = (voltage*voltage) / 1000000000 * time;

	if (startingTS == 0)
		cout << "Pitch: " << electrodePitch << "mm, Volt: " << voltage << "V, Vel: " << velocity << "mm/s, CycleTime: " << cycleTime << "s" << endl;

	if (maxRouteCycles > 0)
	{
		cout << "SUB-PROBLEM " << this->startingTS << " -- ";
		cout << "Time: " << (double)((double)maxRouteCycles * cycleTime) << "s, Energy: " << energy << "J" << endl;
	}*/

}

///////////////////////////////////////////////////////////////////////////////
// Given a vector full of sequential routes, compacts them such that each
// equal index of any subRoute can be executed at the same time.  Adds any
// necessary stalls in the middle of the route.
//
// Compacts the routes by combining the routes from longest to shortest.
// If route being placed interferes with a route that is already placed, the
// route being placed will be delayed by 1 cycle until it can be placed.
///////////////////////////////////////////////////////////////////////////////
void Router::compactRoutesWithMidStalls(vector<vector<RoutePoint *> *> *subRoutes)
{
	//printRoutes(subRoutes);

	bool deadlock = false;
	int numStallsToPrepend = 1;
	int insertionOffset = 2; // # of cycles to offset the insertion of stalls before actual interference

	int longestRoute = 0;
	if (subRoutes->size() > 0)
		longestRoute = subRoutes->at(0)->size();

	for (unsigned i = 0; i < subRoutes->size(); i++)
	{
		if (deadlock)
			break;

		vector<RoutePoint *> *subRoute = subRoutes->at(i);
		RoutePoint *destPt = NULL;
		if (subRoute->size() > 0)
			destPt = subRoute->back();
		int numStallsAdded = 0;

		// Check entire route
		bool isInterference = false;
		int j = 0; // The index used to traverse a specific route/cycle

		// If not outputting, check the max length b/c droplet will remain at destination.
		// If is an output, then only check its own length b/c it will be off the DMFB.
		int cyclesToCheck = max((int)subRoute->size(), longestRoute);
		if (subRoute->size() > 0 && subRoute->at(subRoute->size()-1)->dStatus == DROP_OUTPUT)
			cyclesToCheck = (int)subRoute->size()+2;

		while (j != cyclesToCheck && subRoute->size() > 0)
		{
			RoutePoint *rp = NULL;
			if (j <= subRoute->size()-1)
				rp = subRoute->at(j);
			else
				rp = subRoute->back();

			int insertionIndex = -1;

			if (rp)
			{
				// Check against the previous routes that have been compacted
				for (int k = 0; k < i; k++)
				{
					vector<RoutePoint *> *pastRoute = subRoutes->at(k);

					RoutePoint *rpLc = NULL; // This route's last cycle
					if (j > 0 && j <= subRoute->size()-1)
						rpLc = subRoute->at(j-1);
					else
						rpLc = subRoute->back();

					RoutePoint *rpNc = NULL; // This route's next cycle
					if (j+1 <= subRoute->size()-1)
						rpNc = subRoute->at(j+1);
					else
						rpNc = subRoute->back();

					RoutePoint *prp = NULL; // Past route's current cycle
					if (j <= pastRoute->size()-1)
						prp = pastRoute->at(j);
					else if (pastRoute->back()->dStatus != DROP_OUTPUT)
						prp = pastRoute->back();

					RoutePoint *prpLc = NULL; // Past route's last cycle
					if (j > 0 && j <= pastRoute->size()-1)
						prpLc = pastRoute->at(j-1);
					else if (pastRoute->back()->dStatus != DROP_OUTPUT)
						prpLc = pastRoute->back();
					else if (j <= pastRoute->size()+1)
						prpLc = pastRoute->back();

					RoutePoint *prpNc = NULL; // Past route's next cycle
					if (j+1 <= pastRoute->size()-1)
						prpNc = pastRoute->at(j+1);
					else if (pastRoute->back()->dStatus != DROP_OUTPUT)
						prpNc = pastRoute->back();

					// Check dynamic droplet rules so this and last droplet locations don't interfere
					if (prp && doesInterfere(rp, prp) && !(doesInterfere(rp, destPt) && prp->dStatus == DROP_WAIT))
					{
						isInterference = true;
						break;
					}
					if (prpLc && doesInterfere(rp, prpLc) && !(doesInterfere(rp, destPt) && prpLc->dStatus == DROP_WAIT)) // DTG, is this if-statement necessary??
					{
						isInterference = true;
						break;
					}
					if (prp && rpNc && doesInterfere(rpNc, prp) && !(doesInterfere(rpNc, destPt) && prp->dStatus == DROP_WAIT))
					{
						isInterference = true;
						break;
					}
					if (prpNc && doesInterfere(rp, prpNc) && !(doesInterfere(rp, destPt) && prpNc->dStatus == DROP_WAIT))
					{
						isInterference = true;
						break;
					}
					if (prp && rpLc && doesInterfere(rpLc, prp) && !(doesInterfere(rpLc, destPt) && prp->dStatus == DROP_WAIT))
					{
						isInterference = true;
						break;
					}
					if (prpLc && rpLc && doesInterfere(rpLc, prpLc) && !(doesInterfere(rpLc, destPt) && prpLc->dStatus == DROP_WAIT))
					{
						isInterference = true;
						break;
					}
				}
			}
			if (isInterference)
			{	// Add a few stalls at the beginning and try again

				numStallsAdded++;
				if (numStallsAdded >= 100)
				{
					deadlock = true;
					break;
				}

				insertionIndex = j; // Tells where the interference is

				if ((insertionIndex - insertionOffset <= 0) || (insertionIndex - insertionOffset >= subRoute->size()-1))
					for (int m = 0; m < numStallsToPrepend; m++)
						subRoute->insert(subRoute->begin(), NULL);
				else
				{
					for (int m = 0; m < numStallsToPrepend; m++)
					{
						//cout << subRoute->size() << endl;
						RoutePoint *rpLast = subRoute->at(insertionIndex-insertionOffset);
						if (rpLast)
						{
							RoutePoint *rpStall = new RoutePoint();
							rpStall->x = rpLast->x;
							rpStall->y = rpLast->y;
							rpStall->dStatus = DROP_INT_WAIT;
							subRoute->insert(subRoute->begin()+insertionIndex-1, rpStall);
						}
						else
							subRoute->insert(subRoute->begin()+insertionIndex-1, NULL);
					}
				}
				isInterference = false;
				j = 0;
			}
			else
				j++;
		}
		if (j > longestRoute)
			longestRoute = j;
	}

	// If found a deadlock (inserting stalls in middle did not work), remove all inserts and try
	// adding stalls to beginning.
	if (deadlock)
	{
		//cout << "TRY WITH STALLS AT BEGINNING." << endl;

		for (unsigned i = 0; i < subRoutes->size(); i++)
		{
			vector<RoutePoint *> *subRoute = subRoutes->at(i);
			for (int j = subRoute->size()-1; j >=0; j--)
			{
				if (subRoute->at(j) == NULL || subRoute->at(j)->dStatus == DROP_INT_WAIT)
				{
					RoutePoint *rp = subRoutes->at(i)->at(j);
					subRoutes->at(i)->erase(subRoutes->at(i)->begin()+j);
					if (rp)
						delete rp;
				}
			}
		}
		compactRoutesWithBegStalls(subRoutes);
	}
	//printRoutes(subRoutes);
}

///////////////////////////////////////////////////////////////////////////////
// Given the droplet locations in routes, computes which cells are dirty at
// any particular time
///////////////////////////////////////////////////////////////////////////////
void Router::computeDirtyCells(map<Droplet *, vector<RoutePoint *> *> *routes, vector<vector<RoutePoint*> *> *dirtyCells)
{
	vector<vector<RoutePoint*> *> *dropletsPerCycle = new vector<vector<RoutePoint*> *>();
	vector<vector<RoutePoint*> *> *washDropletsPerCycle = new vector<vector<RoutePoint*> *>();


	// Find the max cycle
	unsigned long long maxCycle = 0;
	map<Droplet *, vector<RoutePoint *> *>::iterator it = routes->begin();
	for (; it != routes->end(); it++)
	{
		// If route has routing points
		vector<RoutePoint *> *route = it->second;
		if (route->size() > 0)
		{
			// Make sure we have the corresponding vectors in dirtyCells
			if (route->back()->cycle > maxCycle)
			{
				maxCycle = route->back()->cycle;
				for (unsigned i = dirtyCells->size(); i <= maxCycle; i++)
				{
					dirtyCells->push_back(new vector<RoutePoint *>());
					dropletsPerCycle->push_back(new vector<RoutePoint *>());
					washDropletsPerCycle->push_back(new vector<RoutePoint *>());
				}
			}

			// Then, add locations to (wash)dropletsPerCycle vectors
			for (unsigned i = 0; i < route->size(); i++)
			{
				RoutePoint *rp = route->at(i);
				rp->droplet = it->first; // rp->droplet member was added for this function so not already set in individual routers

				if (rp->droplet->isWashDroplet)
					washDropletsPerCycle->at(rp->cycle)->push_back(rp);
				else
					dropletsPerCycle->at(rp->cycle)->push_back(rp);
			}
		}
	}

	// Test output
	//for (unsigned i = 0; i < washDropletsPerCycle->size(); i++)
	//	for (unsigned j = 0; j < washDropletsPerCycle->at(i)->size(); j++)
	//		cout << "C" << i << " D" <<  washDropletsPerCycle->at(i)->at(j)->droplet->id << ": " << washDropletsPerCycle->at(i)->at(j)->x << ", " << washDropletsPerCycle->at(i)->at(j)->y << endl;

	// Create empty array (NULL means cell is clean)
	vector<vector<RoutePoint *> *> *array = new vector<vector<RoutePoint *> *>();
	for (int x = 0; x < arch->getNumCellsX(); x++)
	{
		array->push_back(new vector<RoutePoint *>());
		for (int y = 0; y < arch->getNumCellsY(); y++)
			array->back()->push_back(NULL);
	}

	// Compute dirty cells
	set<RoutePoint *> currentDirtyCells;

	for (unsigned long long i = 0; i <= maxCycle; i++)
	{
		// Check for interference
		for (unsigned j = 0; j < dropletsPerCycle->at(i)->size(); j++)
		{
			RoutePoint *rp = dropletsPerCycle->at(i)->at(j);
			RoutePoint *lastRp = array->at(rp->x)->at(rp->y);

			if (rp == NULL)
				exit(0);
				//cout << "BAR" << endl;

			// Contamination just occurred
			if (lastRp != NULL && lastRp->droplet != rp->droplet)
			{
				stringstream msg;
				msg << "Contamination occurred at cycle " << i << ". Residue from droplet " << lastRp->droplet->id;
				msg << " was not cleaned before droplet " << rp->droplet->id << " crossed cell (" << rp->x << ", " << rp->y << ")" << endl;
				//cout << msg.str();
				//claim(false, msg.str());
			}
			else if (lastRp == NULL)
			{
				array->at(rp->x)->at(rp->y) = rp; // New dirty cell
				currentDirtyCells.insert(rp); // Add to current set
			}
		}

		// Now clean the array cell at each wash droplet's location
		for (unsigned j = 0; j < washDropletsPerCycle->at(i)->size(); j++)
		{
			RoutePoint *rp = washDropletsPerCycle->at(i)->at(j);
			RoutePoint *lastRp = array->at(rp->x)->at(rp->y);
			if (lastRp != NULL)
				currentDirtyCells.erase(lastRp); // Remove old cell from dirty list

			array->at(rp->x)->at(rp->y) = NULL; // Clean cell
		}

		// Now add dirty cells to vector
		set<RoutePoint *>::iterator setIt = currentDirtyCells.begin();
		//cout << "Cycle " << i << ": " << endl;
		for (; setIt != currentDirtyCells.end(); setIt++)
		{
			//cout << "\tD" << (*setIt)->droplet->id << "(" << (*setIt)->x << ", " << (*setIt)->y << ")" << endl;
			dirtyCells->at(i)->push_back(*setIt);
		}
	}

	// Now do last cycle
	//set<RoutePoint *>::iterator setIt = currentDirtyCells.begin();
	//for (; setIt != currentDirtyCells.end(); setIt++)
	//	dirtyCells->at(maxCycle)->push_back(*setIt);


	// Cleanup
	while (!dropletsPerCycle->empty())
	{
		vector<RoutePoint *> *v = dropletsPerCycle->back();
		dropletsPerCycle->pop_back();
		dropletsPerCycle->clear();
		delete v;
	}
	delete dropletsPerCycle;

	while (!washDropletsPerCycle->empty())
	{
		vector<RoutePoint *> *v = washDropletsPerCycle->back();
		washDropletsPerCycle->pop_back();
		washDropletsPerCycle->clear();
		delete v;
	}
	delete washDropletsPerCycle;

	while (!array->empty())
	{
		vector<RoutePoint *> *v = array->back();
		array->pop_back();
		array->clear();
		delete v;
	}
	delete array;
}

///////////////////////////////////////////////////////////////////////////////
// Gets the pin-activations from the droplet locations. Assumes that the
// electrode underneath any droplet is activated.
///////////////////////////////////////////////////////////////////////////////
void Router::setPinActivationsFromDropletMotion(map<Droplet *, vector<RoutePoint *> *> *routes, vector<vector<int> *> *pinActivations)
{
	PinMapper *pm = arch->getPinMapper();
	vector<vector<int> *> *pinMapping = pm->getPinMapping();

	// Find the max cycle
	unsigned long long maxCycle = 0;
	map<Droplet *, vector<RoutePoint *> *>::iterator it = routes->begin();
	for (; it != routes->end(); it++)
		if (it->second->size() > 0)
			if (it->second->back()->cycle > maxCycle)
				maxCycle = it->second->back()->cycle;

	/////////////////////// DEBUG PRINT
	/*int d = 0;
	for (it = routes->begin(); it != routes->end(); it++)
	{
		vector<RoutePoint *> *route = it->second;
		cout << "Route #" << d << " size(" << route->size() << "): " << endl;
		for (int i = 0; i < route->size(); i++)
		{
			if (i == 279)
				cout <<"BREAK";
			RoutePoint *dbg = route->at(i);
			cout << "i=" << i << ", c=" << dbg->cycle << " | ";
		}
		d++;
		cout << endl;
	}*/
	///////////////////// END DEBUG PRINT

	// Create a pin-activation vector for each cycle
	for (unsigned long long i = 0; i <= maxCycle+1; i++)
		pinActivations->push_back(new vector<int>);

	// Now, add pin under each droplet to pin-activation list
	for (it = routes->begin(); it != routes->end(); it++)
	{
		vector<RoutePoint *> *route = it->second;
		for (unsigned long long i = 0; i < route->size(); i++)
		{
			//RoutePoint *dbg = route->at(i);
			//cout << "DEBUG : i=" << i << ", c=" << dbg->cycle << ", x=" << dbg->x << ", y=" << dbg->y << endl;
			pinActivations->at(route->at(i)->cycle)->push_back(pinMapping->at(route->at(i)->x)->at(route->at(i)->y));
		}
	}

	// Debug print
	/*for (unsigned long long i = 0; i < pinActivations->size(); i++)
	{
		cout << i << ": ";
		for (int j = 0; j < pinActivations->at(i)->size(); j++)
			cout << pinActivations->at(i)->at(j) << ", ";
		cout << endl;
	}*/
}


///////////////////////////////////////////////////////////////////////////////
// Now that the pin activations have been computed, simulate droplet motion
// from these pin activations. This is not close to a perfect simulation...it
// can't handle complex cases, but should work well for the simple cases of moving
// droplets, simple splits and merges
///////////////////////////////////////////////////////////////////////////////
bool revSort (int i,int j) { return (j<i); }
void Router::simulateDropletMotion(map<Droplet *, vector<RoutePoint *> *> *routes, vector<vector<int> *> *pinActivations)
{
	PinMapper *pm = arch->getPinMapper();
	vector<vector<int> *> *pinMapping = pm->getPinMapping();

	// Get the min I/O pin # so we know when to input/output a droplet
	/*int minIoPin = arch->getIoPorts()->at(0)->getPinNo();
	for (int i = 0; i < arch->getIoPorts()->size(); i++)
		if (arch->getIoPorts()->at(i)->getPinNo() < minIoPin)
			minIoPin = arch->getIoPorts()->at(i)->getPinNo();*/
	int minIoPin = min(pm->getInputPins()->front(), pm->getOutputPins()->front());


	for (unsigned c = 0; c < cycle; c++)
	{
		/*if (c == 4125)
		{
			cout << "Exiting early simulation loop at cycle " << c << "...." << endl;
			return;
		}*/
		vector<int> *pins = pinActivations->at(c);
		sort(pins->begin(), pins->end(), revSort); // Get I/O out there first

		// Remove duplicates
		if (pins->size() > 1)
			for (unsigned i = pins->size()-1; i > 0; i--)
				if (pins->at(i) == pins->at(i-1))
					pins->erase(pins->begin()+i);


		// Debug print
		/*cout << "C" << c << ": ";
		for (int p = 0; p < pins->size(); p++)
			cout << pins->at(p) << "--";
		cout << endl;*/

		// Check the I/O pins (should be at front of pins list b/c of last sort)
		for (unsigned p = 0; p < pins->size(); p++)
		{
			// If I/O pin, handle accordingly
			if (pins->at(p) >= minIoPin)
			{
				for (unsigned i = 0; i < arch->getIoPorts()->size(); i++)
				{
					if (arch->getIoPorts()->at(i)->getPinNo() == pins->at(p))
					{
						// Get port and location
						IoPort *port = arch->getIoPorts()->at(i);
						int px;
						int py;
						if (port->getSide() == NORTH)
						{
							px = port->getPosXY();
							py = 0;
						}
						if (port->getSide() == SOUTH)
						{
							px = port->getPosXY();
							py = arch->getNumCellsY()-1;
						}
						if (port->getSide() == EAST)
						{
							px = arch->getNumCellsX()-1;
							py = port->getPosXY();
						}
						if (port->getSide() == WEST)
						{
							px = 0;
							py = port->getPosXY();
						}

						// If input Port, create new droplet
						if (port->isAnInput())
						{
							Droplet *d = new Droplet();
							d->isWashDroplet = port->isWashPort();
							d->uniqueFluidName = port->getPortName();
							//d->volume = n->GetVolume();
							(*routes)[d] = new vector<RoutePoint *>();

							// Create new route point and add it to droplet list
							RoutePoint *rp = new RoutePoint();
							rp->cycle = c;
							if (!d->isWashDroplet)
								rp->dStatus = DROP_NORMAL;
							else
								rp->dStatus = DROP_WASH;
							rp->x = px;
							rp->y = py;
							(*routes)[d]->push_back(rp);
						}
						else // If output, seal off the droplet as being output
						{
							map<Droplet *, vector<RoutePoint *> *>::iterator dropIt = routes->begin();
							for (; dropIt != routes->end(); dropIt++)
							{
								vector<RoutePoint *> *route = dropIt->second;
								RoutePoint *lrp = route->back(); // Last route point

								if (lrp->x == px && lrp->y == py && lrp->cycle == c-1)
									lrp->dStatus = DROP_OUTPUT;
							}
						}
						break;
					}
				}
			}
			else
				break; // No more I/O pins
		}


		// Check each droplet to see if a pin is adjacent to a droplet
		map<Droplet *, vector<RoutePoint *> *>::iterator dropIt = routes->begin();
		for (; dropIt != routes->end(); dropIt++)
		{
			vector<RoutePoint *> *route = dropIt->second;
			RoutePoint *lrp = route->back(); // Last route point

			if (lrp->dStatus != DROP_OUTPUT && lrp->dStatus != DROP_MERGING && lrp->dStatus != DROP_DRIFT)
			{
				// Check each cell around against the activated pins to see if this droplet will move
				vector<RoutePoint *> next; // Next droplet locations
				for (int y = lrp->y-1; y <= lrp->y+1; y++)
				{
					for (int x = lrp->x-1; x <= lrp->x+1; x++)
					{
						if (x >= 0 && x < arch->getNumCellsX() && y >= 0 && y < arch->getNumCellsY()) // On the board
						{
							if (!(lrp->x == x && lrp->y == y)) // Don't look at the middle for now
							{
								for (unsigned p = 0; p < pins->size(); p++) // Compare to the pins being activated this cycle
								{
									if (pinMapping->at(x)->at(y) == pins->at(p)) // If a match, add to next droplet locations
									{
										RoutePoint *rp = new RoutePoint();
										rp->cycle = c;
										if (!dropIt->first->isWashDroplet)
											rp->dStatus = DROP_NORMAL;
										else
											rp->dStatus = DROP_WASH;
										rp->x = x;
										rp->y = y;
										next.push_back(rp);
									}
								}
							}
						}
					}
				}

				// If just one adjacent electrode in the next list, move to this cell
				// (ignore stretching over multiple elecs for now)
				if (next.size() == 1)
					route->push_back(next.back());
				else if (next.size() > 1) // If more, create new droplets
				{
					route->push_back(next.front());
					for (unsigned n = 1; n < next.size(); n++)
					{
						Droplet *d = new Droplet();
						d->uniqueFluidName = dropIt->first->uniqueFluidName;
						(*routes)[d] = new vector<RoutePoint *>();
						(*routes)[d]->push_back(next.at(n));
					}
				}
				else // No adjacent electrodes activated
				{
					bool stationary = false;
					for (unsigned p = 0; p < pins->size(); p++)
					{
						if (pinMapping->at(lrp->x)->at(lrp->y) == pins->at(p)) // If a match, add to next droplet locations
						{
							stationary = true;
							break;
						}
					}

					// If found stationary, add routepoint for the droplet to stay still
					if (stationary)
					{
						// Make sure a route point doesn't exist already (could have just been dispensed)
						if (lrp->cycle != c)
						{
							RoutePoint *rp = new RoutePoint();
							rp->cycle = c;
							rp->dStatus = DROP_WAIT;
							rp->x = lrp->x;
							rp->y = lrp->y;
							route->push_back(rp);
						}
					}
					else
					{
						cout << "Drop drift error at (" << dropIt->second->back()->x << ", " << dropIt->second->back()->y << ") at cycle " << c << endl;
						cout << "Activated pin #s: ";
						for (unsigned p = 0; p < pins->size(); p++)
							cout << pins->at(p) << ", ";
						cout << endl;
						claim(false, "Drift detected. Droplet on board with no underlying or adjacent electrodes activated.");

						//return;
					}
				}
			}
		}

		//cout << "# droplets = " << routes->size() << endl;
		//if (c == 782)
		//	cout << "Breaking at " << c << endl;

		// Now, check to see if any droplets have just reached the same cell;
		// If so, merge them into one droplet
		for (dropIt = routes->begin(); dropIt != routes->end(); dropIt++)
		{
			vector<RoutePoint *> *route = dropIt->second;
			RoutePoint *lrp0 = route->back(); // Last route point
			if (lrp0->dStatus == DROP_NORMAL || lrp0->dStatus == DROP_WAIT)
			{
				map<Droplet *, vector<RoutePoint *> *>::iterator dropIt2 = routes->begin();
				for (; dropIt2 != routes->end(); dropIt2++)
				{
					vector<RoutePoint *> *route2 = dropIt2->second;
					RoutePoint *lrp2 = route2->back(); // Last route point

					if(lrp0 != lrp2 && lrp0->cycle == lrp2->cycle &&
							lrp0->x == lrp2->x && lrp0->y == lrp2->y &&
							(lrp2->dStatus == DROP_NORMAL || lrp2->dStatus == DROP_WAIT))
					{
						route2->pop_back();
						delete lrp2;
						if (route2->size() > 0)
							route2->back()->dStatus = DROP_MERGING;
					}

				}
			}
		}

	}
}

///////////////////////////////////////////////////////////////////////////////
// Function written by Michael Albertson.
// Performs route compaction using a dynamic programming technique.
///////////////////////////////////////////////////////////////////////////////
void Router::compactRoutesWithDynamicProgramming(vector<Droplet *> *subDrops, vector<vector<RoutePoint *> *> *subRoutes)
{
//	cout<<"Entered the dynamic programming compaction." << endl;
	vector<vector<int> *>* c = new vector<vector<int>*>();
	vector<vector<string>*>* pi = new vector<vector<string>*>();
	int longestRoute = 0;
	if(subRoutes->size() > 0 && subRoutes->at(0)->size() > 0)
		longestRoute = subRoutes->at(0)->size();

	//Initialize the two dynamic programming matrices used by this algorithm
	for(int i = 0; i < 3 * longestRoute; i++)
	{
		vector<int>* row = new vector<int>();
		vector<string>* piRow = new vector<string>();
		for(int j = 0; j < 2 * longestRoute; j++)
		{
			row->push_back(INT_MAX);
			piRow->push_back("");
		}
		c->push_back(row);
		pi->push_back(piRow);
	}
	if(c->size() > 0)
		c->at(0)->at(0) = 0;

	for(int m = 0; m < subRoutes->size(); m++)
	{
		//convert the first route to a motion string
		string ms1 = "x";
		RoutePoint *s = NULL,
				*d = NULL;
		for(int i = 1; i < subRoutes->at(m)->size(); i++)
		{
			if(subRoutes->at(m)->at(i-1) != NULL)
			{
				s = subRoutes->at(m)->at(i-1);
			}
			if(subRoutes->at(m)->at(i) != NULL)
			{
				d = subRoutes->at(m)->at(i);
			}

			//Append an ex for every stall
			if((s==NULL && d==NULL) ||
			   (subRoutes->at(m)->at(i-1) == NULL && d != NULL) ||
			   (s != NULL && subRoutes->at(m)->at(i) == NULL) ||
			   (s->x == d->x && s->y == d->y))
			{
				ms1 += "x";
			}
			//Otherwise append the motion taken moving from source to destination
			else if(d->x == s->x)
			{
				ms1 += ((s->y - d->y) == -1) ? "u" : "d";
			}
			else
			{
				ms1 += ((s->x - d->x) == -1) ? "r" : "l";
			}
		}
		if(ms1.compare("x") == 0)
			ms1="";

		RoutePoint* target1 = NULL;
		if (subRoutes->at(m)->size() > 0)
			target1 = subRoutes->at(m)->back();

		for(unsigned int k = 0; k < m; k++)
		{
			if(m == k || subRoutes->at(m)->size() == 0 || subRoutes->at(k)->size() == 0)
				continue;
			//convert the other route to a string
			string ms2 = "";

			for(int i = 1; i < subRoutes->at(k)->size(); i++)
			{
				if(subRoutes->at(k)->at(i-1) != NULL)
				{
					s = subRoutes->at(k)->at(i-1);
				}
				if(subRoutes->at(k)->at(i) != NULL)
				{
					d = subRoutes->at(k)->at(i);
				}

				if((s==NULL && d==NULL) ||
				   (subRoutes->at(k)->at(i-1) == NULL && d != NULL) ||
				   (s != NULL && subRoutes->at(k)->at(i) == NULL) ||
				   (s->x == d->x && s->y == d->y))
				{
					ms2 += "x";
				}
				else if(s->x == d->x)
				{
					ms2 += (s->y - d->y == -1) ? "u" : "d";
				}
				else
				{
					ms2 += ((s->x - d->x) == -1) ? "r" : "l";
				}
			}

			RoutePoint* target2 = NULL;
			if(subRoutes->at(k)->size() > 0)
				target2 = subRoutes->at(k)->back();

			bool sameTarget = target1 && target2 && target1->x == target2->x && target1->y == target2->y;
			bool dispensed = false;
			bool isInterference = false;
			bool wasInterference = false;
			int size_diff = (ms1.length() > ms2.length()) ? ms1.length() - ms2.length() : 0;
			//If the routes both have the same target, add stalls to the end of the route to prevent interference in the target region.
			if(sameTarget)
				ms2 += "xx";

			//cout<<"ms2: " << ms2 << endl;
			//Setting both before and after points to the first non-null point in the route.
			RoutePoint* p11 = new RoutePoint();
			RoutePoint* p12 = new RoutePoint();
			RoutePoint* source = new RoutePoint();
			for(int i = 0; i < subRoutes->at(m)->size(); i++)
			{
				if(subRoutes->at(m)->at(i) != NULL)
				{
					p11->x = subRoutes->at(m)->at(i)->x;
					p11->y = subRoutes->at(m)->at(i)->y;
					p12->x = p11->x;
					p12->y = p11->y;
					source->x = p11->x;
					source->y = p11->y;
					break;
				}
			}

			//Calculate the first non-null point in the other route.
			RoutePoint* oldP2 = new RoutePoint();
			for(int i = 0; i < subRoutes->at(k)->size(); i++)
			{
				if(subRoutes->at(k)->at(i) != NULL)
				{
					oldP2->x = subRoutes->at(k)->at(i)->x;
					oldP2->y = subRoutes->at(k)->at(i)->y;
					break;
				}
			}

			for(int i = 0; i < ms1.length(); i++)
			{
				//This is an attempt to add more memory to the matrix in case three times the length
				//of the longest routest route is not enough.
				if(i >= c->size())
				{
					c->push_back(new vector<int>());
					pi->push_back(new vector<string>());
				}

				//Set from as to.
				p11->x = p12->x;
				p11->y = p12->y;

				//Determine if the droplet for ms1 has been dispensed yet.
				if(!dispensed && (ms1[i] != 'x'|| (i < ms1.length() - 1 && ms1[i+1] != 'x')))
					dispensed = true;

				//Update the to point if a motion is the current character in the string.
				if(ms1[i] == 'u')
					p12->y++;
				else if (ms1[i] == 'd')
					p12->y--;
				else if (ms1[i] == 'l')
					p12->x--;
				else if (ms1[i] == 'r')
					p12->x++;

				//Reset the points for the other route.
				RoutePoint* p21 = new RoutePoint();
				RoutePoint* p22 = new RoutePoint();
				p21->x = oldP2->x;
				p21->y = oldP2->y;
				p22->x = oldP2->x;
				p22->y = oldP2->y;

				for(int j = 0; j < ms2.length() + size_diff; j++)
				{
					//Skip the first non-movement of each route.
					if(i == 0 && j == 0)
					{
						pi->at(i)->at(j) = "x";
						continue;
					}

					//Attempt to compensate for overflowing the array for the j dimension.
					if(j >= c->at(i)->size())
					{
						c->at(i)->push_back(INT_MAX);
						pi->at(i)->push_back("");
					}

					//set from as to for the other route.
					p21->x = p22->x;
					p21->y = p22->y;

					//Update the to point if the current character in ms2 is a motion.
					if(j < ms2.length())
					{
						if(ms2[j] == 'u')
							p22->y++;
						if(ms2[j] == 'd')
							p22->y--;
						if(ms2[j] == 'l')
							p22->x--;
						if(ms2[j] == 'r')
							p22->x++;
					}

					//Determine if the two motions under consideration interfere with eachother.
					isInterference = (doesInterfere(p11, p21) || doesInterfere(p11,p22) ||
							doesInterfere(p12,p21) || doesInterfere(p12,p22)) &&
							!(sameTarget && p22->x == target2->x && p22->y == target2->y && p21->x == p22->x && p21->y == p22->y);

					if(!dispensed)
					{
						isInterference &= (abs(p21->x - source->x) < 2 && abs(p21->y - source->y) < 2 || abs(p22->x - source->x) < 2 && abs(p22->y - source->y) < 2);
					}

					if((!(isInterference || wasInterference) || !dispensed) && i <= j)
					{
						wasInterference = false;

						//update c
						int cidec = (i!=0) ? c->at(i-1)->at(j) : INT_MAX;
						int cjdec = (j!=0) ? c->at(i)->at(j-1) : INT_MAX;
						int cijdec= (i!=0 && j!=0) ? c->at(i-1)->at(j-1) : INT_MAX;

						int minimum = min(cidec, min(cjdec,cijdec));

						int result = 0;
						if(minimum == INT_MAX)
							result = INT_MAX;
						else
							result = minimum + 1;

						c->at(i)->at(j) = result;

						//update pi
						if(cijdec == minimum && cijdec != INT_MAX)
						{
							pi->at(i)->at(j) = pi->at(i-1)->at(j-1) + ms1[i];
						}
						else if(cidec == minimum && cidec != INT_MAX)
						{
							pi->at(i)->at(j) = pi->at(i-1)->at(j) + ms1[i];
						}
						else if(cjdec == minimum && cjdec != INT_MAX)
						{
							pi->at(i)->at(j) = pi->at(i)->at(j-1) + ((i != ms1.length() - 1) ? "x" : "");
						}
						else
						{
						}
					}
					else
					{
						c->at(i)->at(j) = INT_MAX;
						wasInterference = isInterference;
					}
				}

				delete p21;
				delete p22;
			}
			//overwrite the first route with its compacted version at pi[i][j]
			string pi_result = "";
			int i = 0, j = 0, compLength = 0;
			//Check for the solution in the optimal location within the matrix
			if(ms2.length() > ms1.length() && pi->at(ms1.length() - 1)->at(ms1.length() - 1).compare("") != 0)// && !conflict)
			{
				i = ms1.length() - 1;
				j = i;
			}
			//If there is no solution in that location, then traverse the matrix to find an optimal solution
			else
			{
				while(true)
				{
					int ciInc = (i + 1 < c->size() && j < c->at(i+1)->size()) ? c->at(i + 1)->at(j) : INT_MAX;
					int cjInc = (j + 1 < c->at(0)->size()) ? c->at(i)->at(j + 1) : INT_MAX;
					int cijInc = (i + 1 < c->size() && j + 1 < c->at(0)->size()) ? c->at(i + 1)->at(j + 1) : INT_MAX;

					if(cijInc == compLength + 1)
					{
						i++;
						j++;
						compLength++;
					}
					else if(cjInc == compLength + 1)
					{
						j++;
						compLength++;
					}
					else if(ciInc == compLength + 1)
					{
						i++;
						compLength++;
					}
					else
						break;
				}
				//If no optimal solution is found, prepend a stall and try again.
				if(i < ms1.length() - 1)
				{
					ms1 = "x" + ms1;
					k--;
					continue;
				}
			}

			if(pi->at(i)->at(j).compare("") != 0)
				pi_result = pi->at(i)->at(j);

			if(i < ms1.length() - 1 && j == ms2.length() - 1)
			{
				for(; i < ms1.length(); i++)
					pi_result += ms1[i];
			}

			if(pi_result.compare("") != 0 && pi_result.length() >= ms1.length())
				ms1 = pi_result;

			//reset the c matrix and pi
			for(unsigned int i = 0; i < c->size(); i++)
				for(int j = 0; j < c->at(i)->size(); j++)
				{
					c->at(i)->at(j) = INT_MAX;
					pi->at(i)->at(j) = "";
				}
			if(c->size() > 0)
				c->at(0)->at(0) = 0;

			delete p11;
			delete p12;
			delete oldP2;
		}

		//convert the output string to a route point vector
		int cyclesWaited = 0;
		for(int i = 0; i < ms1.length() && ms1[i] == 'x'; i++) cyclesWaited++;

		vector<RoutePoint*> *route = subRoutes->at(m);
		for(int i = 0, j = 0; i < ms1.length() && j < route->size(); i++)
		{
			if(ms1[i] == 'x' && cyclesWaited > 1)
			{
				route->insert(route->begin() + j, NULL);
				continue;
			}
			else
			{
				j++;
			}
		}
//		printRoutes(subRoutes);
	}
//	printRoutes(subRoutes);

	//delete used memory
	for(int i = 0; i < c->size(); i++)
	{
		delete c->at(i);
		delete pi->at(i);
	}

	delete c;
	delete pi;
//	cout << "Exiting dynamic programming compaction." << endl;
}

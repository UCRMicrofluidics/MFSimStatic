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
 * Source: post_subprob_compact_router.cc										*
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
#include "../../Headers/Router/post_subprob_compact_router.h"

///////////////////////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////////////////////
PostSubproblemCompactionRouter::PostSubproblemCompactionRouter()
{
	cellType = NULL;
	thisTS = new vector<AssayNode *>();
}
PostSubproblemCompactionRouter::PostSubproblemCompactionRouter(DmfbArch *dmfbArch)
{
	cellType = NULL;
	arch = dmfbArch;
	thisTS = new vector<AssayNode *>();
}

///////////////////////////////////////////////////////////////////////////////////
// Deconstructor
///////////////////////////////////////////////////////////////////////////////////
PostSubproblemCompactionRouter::~PostSubproblemCompactionRouter()
{
	///////////////////////////////////////////////////////////////////////
	// More clean-up; Free the arrays being used to keep track of cell
	// types and blockages
	///////////////////////////////////////////////////////////////////////
	if (cellType)
	{
		while (!cellType->empty())
		{
			vector<ResourceType> * col = cellType->back();
			cellType->pop_back();
			col->clear();
			delete col;
		}
		delete cellType;
	}

	if (thisTS)
	{
		thisTS->clear();
		delete thisTS;
	}
}

///////////////////////////////////////////////////////////////////////
// This function performs any one-time initializations that the router
// needs that are specific to a particular router.
///////////////////////////////////////////////////////////////////////
void PostSubproblemCompactionRouter::routerSpecificInits()
{
}

///////////////////////////////////////////////////////////////////////
// Initializes the array that is used by the processing engine to
// determine if a cell is equipped with a heater/detector.
///////////////////////////////////////////////////////////////////////
void PostSubproblemCompactionRouter::initCellTypeArray()
{
	// Create a 2D-array which tells if a cell is augmented with a heater or detector
	cellType = new vector<vector<ResourceType> *>();
	for (int x = 0; x < arch->getNumCellsX(); x++)
	{
		vector<ResourceType> *typeCol = new vector<ResourceType>();
		for (int y = 0; y < arch->getNumCellsY(); y++)
			typeCol->push_back(BASIC_RES);
		cellType->push_back(typeCol);
	}

	for (unsigned i = 0; i < arch->getExternalResources()->size(); i++)
	{
		FixedModule *fm = arch->getExternalResources()->at(i);
		for (int x = fm->getLX(); x <= fm->getRX(); x++)
		{
			for (int y = fm->getTY(); y <= fm->getBY(); y++)
			{
				if (fm->getResourceType() == H_RES && cellType->at(x)->at(y) == D_RES)
					cellType->at(x)->at(y) = DH_RES;
				else if (fm->getResourceType() == H_RES)
					cellType->at(x)->at(y) = H_RES;
				else if (fm->getResourceType() == D_RES && cellType->at(x)->at(y) == H_RES)
					cellType->at(x)->at(y) = DH_RES;
				else if (fm->getResourceType() == D_RES)
					cellType->at(x)->at(y) = D_RES;
				else
					claim(false, "Unsupported cell-augmentation.");
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////
// After droplets are compacted, some droplets will get to their destinations
// first, at which point the router stops adding routing points. Thus,
// these droplets will disappear in the output b/c they finished first.
// Each droplet must be accounted for each cycle, so this function
// examines the global routes and adds routing points to any droplets that
// reached their destinations earlier than the latest droplet so the droplet
// is accounted for each cycle. Droplets that are "caught up" or "equalized"
// simply remain in their last known position. If a droplet was just
// output, then we ensure here that it has been marked properly so
// it will disappear from the DMFB array.
///////////////////////////////////////////////////////////////////////
void PostSubproblemCompactionRouter::equalizeGlobalRoutes(map<Droplet *, vector<RoutePoint *> *> *routes)
{
	for (unsigned i = 0; i < thisTS->size(); i++)
		if (thisTS->at(i)->GetType() == OUTPUT)
			if (thisTS->at(i)->droplets.size() > 0)
				(*routes)[thisTS->at(i)->droplets.front()]->back()->dStatus = DROP_OUTPUT;

	map<Droplet *, vector<RoutePoint *> *>::iterator dropIt = routes->begin();
	for (; dropIt != routes->end(); dropIt++)
	{
		if (!dropIt->second->empty())
		{
			RoutePoint *lrp = dropIt->second->back();
			if (!(lrp->dStatus == DROP_OUTPUT || lrp->dStatus == DROP_MERGING))
			{
				for (unsigned c = lrp->cycle+1; c < routeCycle; c++)
				{
					RoutePoint *nrp = new RoutePoint();
					nrp->cycle = c;
					nrp->dStatus = DROP_WAIT;
					nrp->x = lrp->x;
					nrp->y = lrp->y;
					(*routes)[dropIt->first]->push_back(nrp);
					lrp = nrp;
				}
			}
		}
	}
	cycle = routeCycle;
}

///////////////////////////////////////////////////////////////////////
// This function handles the processing of a time-step.  Most routing
// methods just compute droplet routes from one I/O or module to the
// next; however, this is really incomplete b/c the droplet must be
// accounted for and, in a sense, "routed" (or "processed") inside the
// module.
//
// Thus, this function calls a method to process the time-step so that
// we can generate a COMPLETE simulation such that the results could
// be used to run an actual assay on a DMFB...ensuring that a droplet's
// exact location is accounted for during each cycle.
///////////////////////////////////////////////////////////////////////
void PostSubproblemCompactionRouter::processTimeStep(map<Droplet *, vector<RoutePoint *> *> *routes)
{
	if (processEngineType == FIXED_PE)
		processFixedPlaceTS(routes);
	else if (processEngineType == FREE_PE)
		processFreePlaceTS(routes);
	else /* Do not process - will probably result in some kind of errors */
		claim(false, "No processing engine to process droplets/operations in modules during time-steps.");


	startingTS++; // Time-step is complete; Advance to the next time-step
	cycle += cyclesPerTS; // Advance cycle number by whole time-step
}

///////////////////////////////////////////////////////////////////////
// Divides the fluid names intelligently, maintaining the correct
// composition.
///////////////////////////////////////////////////////////////////////
string PostSubproblemCompactionRouter::splitFluidNames(string n1, int numSplitDrops)
{
	map<string, float> fluids;
	string delimiter = "||";
	string tokenDel = " parts ";
	size_t pos = -1;
	string token;
	string newName = n1;

	// Grab each fluid part
	while ((pos = newName.find(delimiter)) != string::npos)
	{
		token = newName.substr(0, pos);
	    size_t pos2 = token.find(tokenDel); // Will always have
	    float vol = (float)atof(token.substr(0, pos2).c_str());
	    string name = token.substr(pos2 + tokenDel.length(), token.length()-1);
    	fluids[name] = vol / (float)numSplitDrops;
	    newName.erase(0, pos + delimiter.length());
	}
	// Grab last fluid part
	token = newName;
	pos = token.find(tokenDel); // Will always have
	float vol = (float)atof(token.substr(0, pos).c_str());
	string name = token.substr(pos + tokenDel.length(), token.length()-1);
	fluids[name] = vol / (float)numSplitDrops;

	// Generate the new composition from the individual fluid parts
    newName = "";
	map<string, float>::iterator fluidIt = fluids.begin();
	for (; fluidIt != fluids.end(); fluidIt++)
	{
		if (fluidIt != fluids.begin())
			newName = newName + "||";
		stringstream ss;
		ss << fluidIt->second;
		newName = newName + ss.str() + " parts " + fluidIt->first;
	}
	return newName;
}

///////////////////////////////////////////////////////////////////////
// Combines the two fluid names intelligently, maintaining the correct
// composition.
///////////////////////////////////////////////////////////////////////
string PostSubproblemCompactionRouter::mergeFluidNames(string n1, string n2)
{
	map<string, float> fluids;
	string delimiter = "||";
	string tokenDel = " parts ";
	size_t pos = -1;
	string token;
	string newName = (n1 + "||" + n2);

	// Grab each fluid part
	while ((pos = newName.find(delimiter)) != string::npos)
	{
		token = newName.substr(0, pos);
	    size_t pos2 = token.find(tokenDel); // Will always have
	    float vol = (float)atof(token.substr(0, pos2).c_str());
	    string name = token.substr(pos2 + tokenDel.length(), token.length()-1);

	    if (fluids.find(name) == fluids.end())
	    	fluids[name] = vol;
	    else
	    	fluids[name] = fluids[name] + vol;
	    newName.erase(0, pos + delimiter.length());
	}
	// Grab last fluid part
	token = newName;
	pos = token.find(tokenDel); // Will always have
	float vol = (float)atof(token.substr(0, pos).c_str());
	string name = token.substr(pos + tokenDel.length(), token.length()-1);
    if (fluids.find(name) == fluids.end())
    	fluids[name] = vol;
    else
    	fluids[name] = fluids[name] + vol;

    // Generate the new composition from the individual fluid parts
    newName = "";
	map<string, float>::iterator fluidIt = fluids.begin();
	for (; fluidIt != fluids.end(); fluidIt++)
	{
		if (fluidIt != fluids.begin())
			newName = newName + "||";
		stringstream ss;
		ss << fluidIt->second;
		newName = newName + ss.str() + " parts " + fluidIt->first;

	}
	return newName;
}
///////////////////////////////////////////////////////////////////////
// This function merges two droplets together.  It updates the volume,
// unique fluid name (composition description) and returns the droplet
// with the lowest ID.
//
// Assumes that each fluid is formatted as: floatVolume parts fluidName
// with "||" as a delimiter.
// Ex: "10 parts water||3.4 parts glucose||2.1 parts coca-cola"
///////////////////////////////////////////////////////////////////////
Droplet * PostSubproblemCompactionRouter::mergeDroplets(Droplet *d1, Droplet *d2)
{
	float volume = d1->volume + d2->volume;
	string composition = mergeFluidNames(d1->uniqueFluidName, d2->uniqueFluidName);

	if (d1->id < d2->id)
	{
		d1->volume = volume;
		d1->uniqueFluidName = composition;
		return d1;
	}
	else
	{
		d2->volume = volume;
		d2->uniqueFluidName = composition;
		return d2;
	}
}

///////////////////////////////////////////////////////////////////////
// This function takes all the droplets in a node and merges them into
// one droplet.  Thus, before the function is called, the node should
// have 2+ droplets in its droplet list; upon returning, the node will
// have only one droplet (keeps the droplet with the lowest ID).
//
// The new droplet will have the appropriate volume and unique fluid
// name (including correct breakdown of mixture composition)
///////////////////////////////////////////////////////////////////////
void PostSubproblemCompactionRouter::mergeNodeDroplets(AssayNode *n, map<Droplet *, vector<RoutePoint *> *> *routes)
{
	// Merge the droplets into one droplet
	RoutePoint dest;
	dest.x = (*routes)[n->GetDroplets().back()]->back()->x;
	dest.y = (*routes)[n->GetDroplets().back()]->back()->y;

	// Get the minimum droplet ID and the cycle at which all droplets are merged
	int minDropletIdIndex = -1;
	vector<int> arrivalTimes;
	for (unsigned d = 0; d < n->GetDroplets().size(); d++)
	{
		if (minDropletIdIndex == -1 || n->GetDroplets().at(d)->id < n->GetDroplets().at(minDropletIdIndex)->id)
			minDropletIdIndex = d;

		vector<RoutePoint *> *route = (*routes)[n->GetDroplets().at(d)];
		for (int r = route->size()-1; r >= 0; r--)
		{
			if (!(route->at(r)->x == dest.x && route->at(r)->y == dest.y))
			{
				if (r+1 <= route->size()-1)
					arrivalTimes.push_back(route->at(r+1)->cycle);
				else
					arrivalTimes.push_back(route->at(r)->cycle);
				break;
			}
		}
	}
	std::sort(arrivalTimes.begin(), arrivalTimes.end()); // Sort earliest-->latest
	claim(arrivalTimes.size() >= 2, "Mix operation must mix at least 2 droplets.");

	// Delete all redundant route points for merged route droplets and mark
	// interfering droplets as "merging" for the interference checker
	for (unsigned d = 0; d < n->GetDroplets().size(); d++)
	{
		// For the droplets we're not keeping (only keep min ID when merging)
		if (d != minDropletIdIndex)
		{
			vector<RoutePoint *> *terminatingDropletRoute = (*routes)[n->GetDroplets().at(d)];
			RoutePoint *lastRp = NULL;
			if (terminatingDropletRoute->size() > 0)
				lastRp = terminatingDropletRoute->back();

			// Delete redundant route points
			while (lastRp && lastRp->cycle > arrivalTimes.back() && (dest.x == lastRp->x && dest.y == lastRp->y))
			{
				RoutePoint * rpToDelete = terminatingDropletRoute->back();
				terminatingDropletRoute->pop_back();
				delete rpToDelete;

				// Get next route point
				if (terminatingDropletRoute->size() > 0)
					lastRp = terminatingDropletRoute->back();
				else
					lastRp = NULL;
			}

			// Now, mark interfering droplet that is terminating as merging and record the merging cycles
			int rearIndex = 0;
			int begCycle = -1;
			int endCycle = -1;
			while (doesInterfere(&dest, terminatingDropletRoute->at(terminatingDropletRoute->size()-1-rearIndex)))
			{
				RoutePoint *mergingRp = terminatingDropletRoute->at(terminatingDropletRoute->size()-1-rearIndex);
				if (mergingRp->cycle+2 >= arrivalTimes.at(1))
				{
					mergingRp->dStatus = DROP_MERGING;

					if (begCycle == -1 || mergingRp->cycle < begCycle)
						begCycle = mergingRp->cycle;
					if (endCycle == -1 || mergingRp->cycle > endCycle)
						endCycle = mergingRp->cycle;
				}
				else
					break;

				rearIndex++;
			}

			// Now, mark the interfering droplet that is continuing as merging during the appropriate cycles
			vector<RoutePoint *> *continuingDropletRoute = (*routes)[n->GetDroplets().at(minDropletIdIndex)];
			for (int r = continuingDropletRoute->size()-1; r >= 0; r--)
			{
				RoutePoint *rp = continuingDropletRoute->at(r);

				if (rp->cycle >= begCycle && rp->cycle <= endCycle)
					rp->dStatus = DROP_MERGING;

				if (rp->cycle <= begCycle)
					break;
			}
		}
	}

	// Now, combine droplets and droplet properties
	Droplet *drop = n->GetDroplets().back();
	n->droplets.pop_back();

	while(!n->GetDroplets().empty())
	{
		Droplet *d2 = n->droplets.back();
		drop = mergeDroplets(drop, d2);
		n->droplets.pop_back();
	}
	n->droplets.push_back(drop);
}

///////////////////////////////////////////////////////////////////////
// Handles the processing of droplets inside a module during a time-
// step (TS). This draws droplets moving around the module during the TS.
// This function is required to run an actual assay as the droplet locations
// inside the module cannot be ignored in real life.
//
// This function assumes several things.  It assumes the module is at
// least 3 cells tall and 3 cells wide.
//
// For mixes, it essentially causes a droplet to travel around the
// perimeter, clock-wise, until a few cycles before the end of the
// time-step, at which point it will stop at one of the deemed exit-cells
// in the top-right or bottom-right cells of the module.
//
// Heats/detects will travel clock-wise around the perimeter until they
// find a cell equipped with a heater/detector; if one of these devices
// is not on the perimeter (but rather in the center of the module),
// it will not be found. A few cycles before the end of the time-step,
// the droplet will move to one of the deemed exit-cells in the top-right
// or bottom-right.
//
// Splits, will split the droplet into two and then move the two droplets
// into one of the deemed exit-cells in the top- or bottom- right cells
// in the module
//
// Storage go directly to the exit-cells in the top- or bottom- right
// cells of the module.
//
// ONE EXCEPTION that applies to all of the descriptions above: if a
// droplet's next destination is the same module location as the
// current module it is in, it will not position itself in one of the
// exit-cells on the right, as mentioned.  INSTEAD, it will position
// itself in one of the entry cells in the top- or bottom- LEFT cells
// of the module.
///////////////////////////////////////////////////////////////////////
void PostSubproblemCompactionRouter::processFixedPlaceTS(map<Droplet *, vector<RoutePoint *> *> *routes)
{
	///////////////////////////////////////////////////////////////////
	// LONG version, actually moves the droplets around in the chambers.
	// Possibly leave disabled when doing route timings as no other works
	// include the processing time in route construction.
	///////////////////////////////////////////////////////////////////
	RoutePoint *nrp = NULL;
	RoutePoint *lrp = NULL;
	for (unsigned i = 0; i < thisTS->size(); i++)
	{
		AssayNode *n = thisTS->at(i);

		if (!(n->GetType() == DISPENSE || n->GetType() == OUTPUT))
		{
			unsigned long long tsCycle = cycle;

			bool stayingInModule = false;

			if (n->children.front()->reconfigMod && n->reconfigMod->getLX() == n->children.front()->reconfigMod->getLX() && n->reconfigMod->getTY() == n->children.front()->reconfigMod->getTY())
				stayingInModule = true;

			// Here, figure out what kind of node it is and then move the droplets around as necessary
			if (n->GetType() == MIX)
			{
				// Merge the droplets together
				if (n->GetDroplets().size() >= 2)
					mergeNodeDroplets(n, routes);

				Droplet *drop = n->GetDroplets().back();

				// Compute whether to leave out of top or bottom exit
				ReconfigModule *rm = n->GetReconfigMod();
				int exitY; // Tells whether droplet is exiting from the TOP-right or BOTTOM-right
				AssayNode *child = n->children.front();
				if (child->GetType() == OUTPUT)
				{
					if (child->GetIoPort()->getSide() == NORTH)
						exitY = rm->getTY();
					else if (child->GetIoPort()->getSide() == SOUTH)
						exitY = rm->getBY();
					else
					{
						if (child->GetIoPort()->getPosXY() <= rm->getTY()+1)
							exitY = rm->getTY();
						else
							exitY = rm->getBY();
					}
				}
				else
				{
					ReconfigModule *nRm = n->children.front()->GetReconfigMod(); // Next module
					if (nRm->getTY() < rm->getTY())
						exitY = rm->getTY();
					else
						exitY = rm->getBY();
				}

				lrp = (*routes)[drop]->back(); // Last route point
				while (tsCycle < cycle + cyclesPerTS)
				{
					ReconfigModule *rm = n->GetReconfigMod();
					nrp = new RoutePoint(); // This route point
					nrp->dStatus = DROP_PROCESSING;
					int numCellsInCirc = rm->getNumCellsInCirc();
					if (stayingInModule && (lrp->x == rm->getLX() && lrp->y == rm->getBY()) && (tsCycle + numCellsInCirc > cycle + cyclesPerTS) && startingTS+1 == n->endTimeStep )
					{	// Cannot make another full rotation before TS is done so just stay here in BL and take position for next TS
						nrp->x = lrp->x;
						nrp->y = lrp->y;
					}
					else if (!stayingInModule && (lrp->x == rm->getRX() && lrp->y == exitY) && (tsCycle + numCellsInCirc > cycle + cyclesPerTS) && startingTS+1 == n->endTimeStep )
					{	// Cannot make another full rotation before TS is done so just stay here in TR and take position to leave module
						nrp->x = lrp->x;
						nrp->y = lrp->y;
					}
					else
					{	// ***This won't work on linear arrays
						if (lrp->x == rm->getLX()) // If in left column
						{
							if (lrp->y > rm->getTY()) // Go north if not at north border
							{
								nrp->x = lrp->x;
								nrp->y = lrp->y-1;
							}
							else // Else turn toward east
							{
								nrp->x = lrp->x+1;
								nrp->y = lrp->y;
							}
						}
						else if (lrp->y == rm->getTY()) // If in top row
						{
							if (lrp->x < rm->getRX()) // Go east if not at east border
							{
								nrp->x = lrp->x+1;
								nrp->y = lrp->y;
							}
							else // Else turn toward south
							{
								nrp->x = lrp->x;
								nrp->y = lrp->y+1;
							}
						}
						else if (lrp->x == rm->getRX()) // If in right column
						{
							if (lrp->y < rm->getBY()) // Go south if not at south border
							{
								nrp->x = lrp->x;
								nrp->y = lrp->y+1;
							}
							else // Else turn toward west
							{
								nrp->x = lrp->x-1;
								nrp->y = lrp->y;
							}
						}
						else if (lrp->y == rm->getBY()) // If in bottom row
						{
							if (lrp->x > rm->getLX()) // Go west if not at left border
							{
								nrp->x = lrp->x-1;
								nrp->y = lrp->y;
							}
							else // Else toward north
							{
								nrp->x = lrp->x;
								nrp->y = lrp->y-1;
							}
						}
						else
							claim(false, "Unhandled processing decision for router.");
					}
					nrp->cycle = tsCycle++;
					(*routes)[drop]->push_back(nrp);
					lrp = nrp;
				}
			}
			else if (n->GetType() == DILUTE)
			{
				// Merge the droplets together
				if (n->GetDroplets().size() >= 2)
					mergeNodeDroplets(n, routes);

				Droplet *drop = n->GetDroplets().back();

				ReconfigModule *rm = n->GetReconfigMod();
				int modHeight = abs(rm->getBY() - rm->getTY() + 1);
				lrp = (*routes)[drop]->back(); // Last route point

				// Process droplet for duration of time-step
				while (tsCycle < cycle + cyclesPerTS)
				{
					ReconfigModule *rm = n->GetReconfigMod();
					nrp = new RoutePoint(); // This route point
					nrp->dStatus = DROP_PROCESSING;
					int numCellsInCirc = rm->getNumCellsInCirc();

					if ((lrp->x == rm->getRX() && lrp->y == rm->getBY()) && (tsCycle + numCellsInCirc + (modHeight-1) > cycle + cyclesPerTS) && startingTS+1 == n->endTimeStep )
					{	// Cannot make another full rotation before TS is done so just stay here in BR and take position to leave module

						// Split droplets if still only 1 droplet
						if (n->GetDroplets().size() <= 1)
						{
							Droplet *splitDrop1 = n->GetDroplets().back();
							Droplet *splitDrop2 = new Droplet();
							splitDrop1->uniqueFluidName = splitDrop2->uniqueFluidName = splitFluidNames(splitDrop1->uniqueFluidName, n->GetChildren().size());
							splitDrop1->volume = splitDrop2->volume = splitDrop1->volume / n->GetChildren().size();
							(*routes)[splitDrop2] = new vector<RoutePoint *>();
							n->addDroplet(splitDrop2);
							lrp = (*routes)[splitDrop1]->back();

							for (int yPos = lrp->y-1; yPos >= n->reconfigMod->getTY(); yPos--)
							{
								nrp = new RoutePoint();
								nrp->x = lrp->x;
								nrp->y = lrp->y;
								nrp->dStatus = DROP_SPLITTING;
								nrp->cycle = tsCycle;
								(*routes)[splitDrop1]->push_back(nrp);
								nrp = new RoutePoint();
								nrp->x = lrp->x;
								nrp->y = yPos;
								nrp->dStatus = DROP_SPLITTING;
								nrp->cycle = tsCycle++;
								(*routes)[splitDrop2]->push_back(nrp);
							}
						}
						else // Droplets have been split; hold in place
						{
							unsigned long long splitCycle;
							for (unsigned di = 0; di < n->droplets.size(); di++)
							{
								Droplet *drop = n->droplets.at(di);
								lrp = (*routes)[drop]->back();
								splitCycle = tsCycle;
								while (splitCycle < cycle + cyclesPerTS)
								{

									nrp = new RoutePoint();
									nrp->x = lrp->x;
									nrp->y = lrp->y;
									nrp->dStatus = DROP_PROCESSING;
									nrp->cycle = splitCycle++;
									(*routes)[drop]->push_back(nrp);
									lrp = nrp;
								}
							}
							tsCycle = splitCycle;
						}
					}
					else
					{	// ***This won't work on linear arrays
						if (lrp->x == rm->getLX()) // If in left column
						{
							if (lrp->y > rm->getTY()) // Go north if not at north border
							{
								nrp->x = lrp->x;
								nrp->y = lrp->y-1;
							}
							else // Else turn toward east
							{
								nrp->x = lrp->x+1;
								nrp->y = lrp->y;
							}
						}
						else if (lrp->y == rm->getTY()) // If in top row
						{
							if (lrp->x < rm->getRX()) // Go east if not at east border
							{
								nrp->x = lrp->x+1;
								nrp->y = lrp->y;
							}
							else // Else turn toward south
							{
								nrp->x = lrp->x;
								nrp->y = lrp->y+1;
							}
						}
						else if (lrp->x == rm->getRX()) // If in right column
						{
							if (lrp->y < rm->getBY()) // Go south if not at south border
							{
								nrp->x = lrp->x;
								nrp->y = lrp->y+1;
							}
							else // Else turn toward west
							{
								nrp->x = lrp->x-1;
								nrp->y = lrp->y;
							}
						}
						else if (lrp->y == rm->getBY()) // If in bottom row
						{
							if (lrp->x > rm->getLX()) // Go west if not at left border
							{
								nrp->x = lrp->x-1;
								nrp->y = lrp->y;
							}
							else // Else toward north
							{
								nrp->x = lrp->x;
								nrp->y = lrp->y-1;
							}
						}
						else
							claim(false, "Unhandled processing decision for router.");
						nrp->cycle = tsCycle++;
						(*routes)[drop]->push_back(nrp);
						lrp = nrp;
					}

				}
			}
			else if (n->GetType() == HEAT || n->GetType() == DETECT)
			{
				Droplet *drop = n->GetDroplets().back();
				lrp = (*routes)[drop]->back(); // Last route point
				ResourceType rt;
				if (n->GetType() == HEAT)
					rt = H_RES;
				else
					rt = D_RES;

				while (tsCycle < cycle + cyclesPerTS)
				{
					// Compute whether to leave out of top or bottom exit
					ReconfigModule *rm = n->GetReconfigMod();
					int exitY; // Tells whether droplet is exiting from the TOP-right or BOTTOM-right
					AssayNode *child = n->children.front();
					if (child->GetType() == OUTPUT)
					{
						if (child->GetIoPort()->getSide() == NORTH)
							exitY = rm->getTY();
						else if (child->GetIoPort()->getSide() == SOUTH)
							exitY = rm->getBY();
						else
						{
							if (child->GetIoPort()->getPosXY() <= rm->getTY()+1)
								exitY = rm->getTY();
							else
								exitY = rm->getBY();
						}
					}
					else
					{
						ReconfigModule *nRm = n->children.front()->GetReconfigMod(); // Next module
						if (nRm->getTY() < rm->getTY())
							exitY = rm->getTY();
						else
							exitY = rm->getBY();
					}

					nrp = new RoutePoint(); // This route point
					nrp->dStatus = DROP_PROCESSING;
					int numCellsInCirc = rm->getNumCellsInCirc();
					if (((cellType->at(lrp->x)->at(lrp->y) == rt || cellType->at(lrp->x)->at(lrp->y) == DH_RES) && (startingTS+1 != n->endTimeStep || (startingTS+1 == n->endTimeStep && (tsCycle + numCellsInCirc < cycle + cyclesPerTS))))
							|| (stayingInModule && (lrp->x == rm->getLX() && lrp->y == rm->getBY()) && (tsCycle + numCellsInCirc > cycle + cyclesPerTS) && startingTS+1 == n->endTimeStep)
							|| (!stayingInModule && (lrp->x == rm->getRX() && lrp->y == exitY) && (tsCycle + numCellsInCirc > cycle + cyclesPerTS) && startingTS+1 == n->endTimeStep) )
					{	// Cannot make another full rotation before TS is done so just stay here
						nrp->x = lrp->x;
						nrp->y = lrp->y;
					}
					else
					{	// ***This won't work on linear arrays
						if (lrp->x == rm->getLX()) // If in left column
						{
							if (lrp->y > rm->getTY()) // Go north if not at north border
							{
								nrp->x = lrp->x;
								nrp->y = lrp->y-1;
							}
							else // Else turn toward east
							{
								nrp->x = lrp->x+1;
								nrp->y = lrp->y;
							}
						}
						else if (lrp->y == rm->getTY()) // If in top row
						{
							if (lrp->x < rm->getRX()) // Go east if not at east border
							{
								nrp->x = lrp->x+1;
								nrp->y = lrp->y;
							}
							else // Else turn toward south
							{
								nrp->x = lrp->x;
								nrp->y = lrp->y+1;
							}
						}
						else if (lrp->x == rm->getRX()) // If in right column
						{
							if (lrp->y < rm->getBY()) // Go south if not at south border
							{
								nrp->x = lrp->x;
								nrp->y = lrp->y+1;
							}
							else // Else turn toward west
							{
								nrp->x = lrp->x-1;
								nrp->y = lrp->y;
							}
						}
						else if (lrp->y == rm->getBY()) // If in bottom row
						{
							if (lrp->x > rm->getLX()) // Go west if not at left border
							{
								nrp->x = lrp->x-1;
								nrp->y = lrp->y;
							}
							else // Else toward north
							{
								nrp->x = lrp->x;
								nrp->y = lrp->y-1;
							}
						}
						else
							claim(false, "Unhandled processing decision for router.");
					}
					nrp->cycle = tsCycle++;
					(*routes)[drop]->push_back(nrp);
					lrp = nrp;
				}
			}
			else if (n->GetType() == STORAGE)
			{
				// If we are breaking the 2 droplet rule, we cannot move to exit cell b/c there might already be a droplet there
				bool dropsMustRemainStatic = false;
				if (n->GetReconfigMod()->getNumDrops() > 2)
					dropsMustRemainStatic = true;

				Droplet *drop = n->GetDroplets().back();
				lrp = (*routes)[drop]->back(); // Last route point

				while (tsCycle < cycle + cyclesPerTS)
				{
					nrp = new RoutePoint(); // next route point
					nrp->cycle = tsCycle++;
					nrp->dStatus = DROP_PROCESSING;

					// If leaving module after TS, move to right and store, else keep on left
					if (!stayingInModule && lrp->x != n->reconfigMod->getRX() && !dropsMustRemainStatic)
					{
						nrp->x = lrp->x+1;
						nrp->y = lrp->y;
					}
					else if (stayingInModule && n->children.front()->type != STORAGE && lrp->y != n->reconfigMod->getBY() && tsCycle > cycle+2 && !dropsMustRemainStatic)
					{
						nrp->x = lrp->x;
						nrp->y = lrp->y+1;
					}
					else
					{
						nrp->x = lrp->x;
						nrp->y = lrp->y;
					}
					(*routes)[drop]->push_back(nrp);
					lrp = nrp;
				}
			}
			else if (n->GetType() == SPLIT)
			{
				// Only split at beginning of TS
				if (n->startTimeStep == startingTS)
				{
					Droplet *drop = n->GetDroplets().back();
					Droplet *drop2 = new Droplet();

					drop->uniqueFluidName = drop2->uniqueFluidName = splitFluidNames(drop->uniqueFluidName, n->GetChildren().size());
					drop->volume = drop2->volume = drop->volume / n->GetChildren().size();
					(*routes)[drop2] = new vector<RoutePoint *>();
					n->addDroplet(drop2);
					lrp = (*routes)[drop]->back();

					for (int yPos = lrp->y-1; yPos >= n->reconfigMod->getTY(); yPos--)
					{
						nrp = new RoutePoint();
						nrp->x = lrp->x;
						nrp->y = lrp->y;
						nrp->dStatus = DROP_SPLITTING;
						nrp->cycle = tsCycle;
						(*routes)[drop]->push_back(nrp);
						nrp = new RoutePoint();
						nrp->x = lrp->x;
						nrp->y = yPos;
						nrp->dStatus = DROP_SPLITTING;
						nrp->cycle = tsCycle++;
						(*routes)[drop2]->push_back(nrp);
					}
				}
				unsigned long long splitCycle;
				for (unsigned d = 0; d < n->droplets.size(); d++)
				{
					Droplet *drop = n->droplets.at(d);
					lrp = (*routes)[drop]->back();
					splitCycle = tsCycle;
					while (splitCycle < cycle + cyclesPerTS)
					{
						// Move to right
						if (lrp->x != n->reconfigMod->getRX())
						{
							nrp = new RoutePoint(); // next route point
							nrp->cycle = splitCycle++;
							nrp->dStatus = DROP_PROCESSING;
							nrp->x = lrp->x+1;
							nrp->y = lrp->y;
							(*routes)[drop]->push_back(nrp);
							lrp = nrp;
						}
						else
						{
							nrp = new RoutePoint();
							nrp->x = lrp->x;
							nrp->y = lrp->y;
							nrp->dStatus = DROP_PROCESSING;
							nrp->cycle = splitCycle++;
							(*routes)[drop]->push_back(nrp);
							lrp = nrp;
						}
					}
				}
				tsCycle = splitCycle;
			}
			else
				claim(false, "Unsupported opearation encountered while routing.");
		}
	}
}

///////////////////////////////////////////////////////////////////////
// Handles the processing of droplets inside a module during a time-
// step (TS). This is the long version, which draws droplets moving
// around the module during the TS. A full processing (as opposed to
// a quick processing) is required to run an actual assay as the droplet
// locations inside the module cannot be ignored in real life.
//
// This function is meant to be used for free placements and can handle
// modules of different sizes and shapes, assuming they are some sort
// of rectangle
//
// For mixes (2 by X) mixers, it causes a droplet to travel around the
// perimeter, clock-wise, until the time-step is complete. For (3+ by X)
// mixers, performs a zig-zag until the end of the time-step. For
// (1 by X) mixers (linear mixers), traverses the module back and forth.
//
// Heats/detects will zig-zag through their module until they find a cell
// equipped with a heater/detector; heat/detect modules with a single cell
// will just remain there.
//
// Splits assume at least a (3 by 1) array and will split the droplet
// into two and leave the droplets there until the end of the time-step.
//
// Storage will assume it is given a (1 by 1) module (single-cell module)
// and remain in its location until the end of the time-step.
///////////////////////////////////////////////////////////////////////
void PostSubproblemCompactionRouter::processFreePlaceTS(map<Droplet *, vector<RoutePoint *> *> *routes)
{
	RoutePoint *nrp = NULL;
	RoutePoint *lrp = NULL;
	for (unsigned i = 0; i < thisTS->size(); i++)
	{
		AssayNode *n = thisTS->at(i);

		if (!(n->GetType() == DISPENSE || n->GetType() == OUTPUT))
		{
			unsigned long long tsCycle = cycle;

			// Here, figure out what kind of node it is and then move the droplets around as necessary
			if (n->GetType() == MIX)
			{

				// Merge the droplets together
				if (n->GetDroplets().size() >= 2)
					mergeNodeDroplets(n, routes);

				Droplet *drop = n->GetDroplets().back();

				// Create route point for each cycle of the time-step
				lrp = (*routes)[drop]->back(); // Last route point
				bool travelingRight = true;
				bool travelingDown = true;
				while (tsCycle < cycle + cyclesPerTS)
				{
					ReconfigModule *rm = n->GetReconfigMod();
					int modWidth = rm->getRX() - rm->getLX() + 1;
					int modHeight = rm->getBY() - rm->getTY() + 1;

					// If module has exactly 1 cell in any dimension (linear array), travel back and forth
					if (modWidth == 1 || modHeight == 1)
					{
						// Horizontal linear array
						nrp = advanceDropInLinMod(lrp, rm, &travelingRight, &travelingDown);
						nrp->cycle = tsCycle++;
						(*routes)[drop]->push_back(nrp);
						lrp = nrp;
					}
					else if (modWidth == 2 || modHeight == 2)
					{
						// Module has exactly 2 cells in any dimension, travel circumference
						nrp = advanceDropIn2ByMod(lrp, rm);
						nrp->cycle = tsCycle++;
						(*routes)[drop]->push_back(nrp);
						lrp = nrp;
					}
					else
					{
						// Module has >2 cells in all dimensions, do zig-zag
						nrp = advanceDropIn3By3PlusMod(lrp, rm, &travelingRight, &travelingDown);
						nrp->cycle = tsCycle++;
						(*routes)[drop]->push_back(nrp);
						lrp = nrp;
					}
				}
			}
			else if (n->GetType() == DILUTE)
			{
				// Merge the droplets together
				if (n->GetDroplets().size() >= 2)
					mergeNodeDroplets(n, routes);

				Droplet *drop = n->GetDroplets().back();

				// Create route point for each cycle of the time-step
				lrp = (*routes)[drop]->back(); // Last route point
				bool travelingRight = true;
				bool travelingDown = true;
				while (tsCycle < cycle + cyclesPerTS)
				{
					ReconfigModule *rm = n->GetReconfigMod();
					int modWidth = rm->getRX() - rm->getLX() + 1;
					int modHeight = rm->getBY() - rm->getTY() + 1;

					if (
							( ((lrp->x == rm->getLX() || lrp->x == rm->getRX()) && (tsCycle + modWidth + 2 /*(split cycles)*/ > cycle + cyclesPerTS)) ||
							  ((lrp->y == rm->getTY() || lrp->y == rm->getBY()) && (tsCycle + modHeight + 2 /*(split cycles)*/ > cycle + cyclesPerTS))
							)
							&& startingTS+1 == n->endTimeStep && n->GetDroplets().size() <= 1
						)
					{
						// Split the droplet
						Droplet *splitDrop1 = n->GetDroplets().back();
						Droplet *splitDrop2 = new Droplet();
						splitDrop1->uniqueFluidName = splitDrop2->uniqueFluidName = splitFluidNames(splitDrop1->uniqueFluidName, n->GetChildren().size());
						splitDrop1->volume = splitDrop2->volume = drop->volume / n->GetChildren().size();

						(*routes)[splitDrop2] = new vector<RoutePoint *>();
						n->addDroplet(splitDrop2);
						lrp = (*routes)[splitDrop1]->back();

						// Proceess split part of dilution
						for (int yPos = 1; yPos <= 2; yPos++)
						{
							nrp = new RoutePoint();
							nrp->x = lrp->x;
							nrp->y = lrp->y;
							nrp->dStatus = DROP_SPLITTING;
							nrp->cycle = tsCycle;
							(*routes)[splitDrop1]->push_back(nrp);
							nrp = new RoutePoint();

							// Compute split direction depending on current location in module
							if (lrp->x == rm->getLX() && modWidth >= 3) // Split to right
							{
								nrp->x = lrp->x + yPos;
								nrp->y = lrp->y;
							}
							else if (lrp->x == rm->getRX() && modWidth >= 3) // Split to left
							{
								nrp->x = lrp->x - yPos;
								nrp->y = lrp->y;
							}
							else if (lrp->y == rm->getBY() && modHeight >= 3) // Split upward
							{
								nrp->x = lrp->x;
								nrp->y = lrp->y - yPos;
							}
							else if (lrp->y == rm->getTY() && modHeight >= 3) // Split downward
							{
								nrp->x = lrp->x;
								nrp->y = lrp->y + yPos;
							}

							nrp->dStatus = DROP_SPLITTING;
							nrp->cycle = tsCycle++;
							(*routes)[splitDrop2]->push_back(nrp);
						}
					}
					else if (n->GetDroplets().size() > 1) // Droplets have been split; hold in place
					{
						unsigned long long splitCycle;
						for (unsigned di = 0; di < n->droplets.size(); di++)
						{
							Droplet *drop = n->droplets.at(di);
							lrp = (*routes)[drop]->back();
							splitCycle = tsCycle;
							while (splitCycle < cycle + cyclesPerTS)
							{

								nrp = new RoutePoint();
								nrp->x = lrp->x;
								nrp->y = lrp->y;
								nrp->dStatus = DROP_PROCESSING;
								nrp->cycle = splitCycle++;
								(*routes)[drop]->push_back(nrp);
								lrp = nrp;
							}
						}
						tsCycle = splitCycle;
					}
					else
					{
						// If module has exactly 1 cell in any dimension (linear array), travel back and forth
						if (modWidth == 1 || modHeight == 1)
						{
							// Horizontal linear array
							nrp = advanceDropInLinMod(lrp, rm, &travelingRight, &travelingDown);
							nrp->cycle = tsCycle++;
							(*routes)[drop]->push_back(nrp);
							lrp = nrp;
						}
						else if (modWidth == 2 || modHeight == 2)
						{
							// Module has exactly 2 cells in any dimension, travel circumference
							nrp = advanceDropIn2ByMod(lrp, rm);
							nrp->cycle = tsCycle++;
							(*routes)[drop]->push_back(nrp);
							lrp = nrp;
						}
						else
						{
							// Module has >2 cells in all dimensions, do zig-zag
							nrp = advanceDropIn3By3PlusMod(lrp, rm, &travelingRight, &travelingDown);
							nrp->cycle = tsCycle++;
							(*routes)[drop]->push_back(nrp);
							lrp = nrp;
						}
					}
				}
			}
			else if (n->GetType() == HEAT || n->GetType() == DETECT)
			{
				Droplet *drop = n->GetDroplets().back();
				lrp = (*routes)[drop]->back(); // Last route point
				ResourceType rt;
				if (n->GetType() == HEAT)
					rt = H_RES;
				else
					rt = D_RES;

				bool travelingRight = true;
				bool travelingDown = true;
				while (tsCycle < cycle + cyclesPerTS)
				{
					// Compute whether to leave out of top or bottom exit
					ReconfigModule *rm = n->GetReconfigMod();
					//int numCellsInCirc = rm->getNumCellsInCirc();

					// If have found an appropriate resource, just stay here
					if ( cellType->at(lrp->x)->at(lrp->y) == rt || cellType->at(lrp->x)->at(lrp->y) == DH_RES )
					{
						nrp = new RoutePoint();
						nrp->dStatus = DROP_PROCESSING;
						nrp->x = lrp->x;
						nrp->y = lrp->y;
					}
					else // Still trying to find the external resource
					{
						int modWidth = rm->getRX() - rm->getLX() + 1;
						int modHeight = rm->getBY() - rm->getTY() + 1;

						// If module has exactly 1 cell in any dimension (linear array), travel back and forth
						if (modWidth == 1 || modHeight == 1)
							nrp = advanceDropInLinMod(lrp, rm, &travelingRight, &travelingDown);
						else if (modWidth == 2 || modHeight == 2) // If module has exactly 2 cells in any dimension, travel circumference
							nrp = advanceDropIn2ByMod(lrp, rm);
						else // If module has >2 cells in all dimensions, do zig-zag
							nrp = advanceDropIn3By3PlusMod(lrp, rm, &travelingRight, &travelingDown);
					}
					nrp->cycle = tsCycle++;
					(*routes)[drop]->push_back(nrp);
					lrp = nrp;
				}
			}
			else if (n->GetType() == STORAGE)
			{
				Droplet *drop = n->GetDroplets().back();
				lrp = (*routes)[drop]->back(); // Last route point

				while (tsCycle < cycle + cyclesPerTS)
				{
					nrp = new RoutePoint(); // next route point
					nrp->cycle = tsCycle++;
					nrp->dStatus = DROP_PROCESSING;
					nrp->x = lrp->x;
					nrp->y = lrp->y;
					(*routes)[drop]->push_back(nrp);
					lrp = nrp;
				}
			}
			else if (n->GetType() == SPLIT)
			{
				// Only split at beginning of TS
				if (n->startTimeStep == startingTS)
				{
					Droplet *drop = n->GetDroplets().back();
					lrp = (*routes)[drop]->back(); // Last route point

					ReconfigModule *rm = n->GetReconfigMod();
					int modWidth = rm->getRX() - rm->getLX() + 1;
					int modHeight = rm->getBY() - rm->getTY() + 1;

					claim(modWidth >= 3 || modHeight >= 3, "Module to perform split operatoin must be at least 3 cells in one dimension.");


					// If modWidth is at least 3, position at left-most side...
					if (modWidth >= 3)
					{
						while (lrp->x > rm->getLX())
						{
							nrp = new RoutePoint(); // next route point
							nrp->cycle = tsCycle++;
							nrp->dStatus = DROP_PROCESSING;
							nrp->x = lrp->x-1;
							nrp->y = lrp->y;
							(*routes)[drop]->push_back(nrp);
							lrp = nrp;
						}
					}
					else // If modHeight is at least 3, position at bottom-most side...
					{
						while (lrp->y < rm->getBY())
						{
							nrp = new RoutePoint(); // next route point
							nrp->cycle = tsCycle++;
							nrp->dStatus = DROP_PROCESSING;
							nrp->x = lrp->x;
							nrp->y = lrp->y+1;
							(*routes)[drop]->push_back(nrp);
							lrp = nrp;
						}
					}

					// Then do split
					Droplet *drop2 = new Droplet();
					drop->uniqueFluidName = drop2->uniqueFluidName = splitFluidNames(drop->uniqueFluidName, n->GetChildren().size());
					drop->volume = drop2->volume = drop->volume / n->GetChildren().size();
					(*routes)[drop2] = new vector<RoutePoint *>();
					n->addDroplet(drop2);

					// If modWidth is at least 3, split to right
					if (modWidth >= 3)
					{
						for (int xPos = lrp->x+1; xPos <= n->reconfigMod->getRX(); xPos++)
						{
							nrp = new RoutePoint();
							nrp->x = lrp->x;
							nrp->y = lrp->y;
							nrp->dStatus = DROP_SPLITTING;
							nrp->cycle = tsCycle;
							(*routes)[drop]->push_back(nrp);
							nrp = new RoutePoint();
							nrp->x = xPos;
							nrp->y = lrp->y;
							nrp->dStatus = DROP_SPLITTING;
							nrp->cycle = tsCycle++;
							(*routes)[drop2]->push_back(nrp);
						}
					}
					else // If modHeight is at least 3, split upward
					{
						for (int yPos = lrp->y-1; yPos >= n->reconfigMod->getTY(); yPos--)
						{
							nrp = new RoutePoint();
							nrp->x = lrp->x;
							nrp->y = lrp->y;
							nrp->dStatus = DROP_SPLITTING;
							nrp->cycle = tsCycle;
							(*routes)[drop]->push_back(nrp);
							nrp = new RoutePoint();
							nrp->x = lrp->x;
							nrp->y = yPos;
							nrp->dStatus = DROP_SPLITTING;
							nrp->cycle = tsCycle++;
							(*routes)[drop2]->push_back(nrp);
						}
					}
				}

				// Now allow droplets to wait where they are until time-step is complete
				unsigned long long splitCycle;
				for (unsigned d = 0; d < n->droplets.size(); d++)
				{
					Droplet *drop = n->droplets.at(d);
					lrp = (*routes)[drop]->back();
					splitCycle = tsCycle;
					while (splitCycle < cycle + cyclesPerTS)
					{
						nrp = new RoutePoint();
						nrp->x = lrp->x;
						nrp->y = lrp->y;
						nrp->dStatus = DROP_PROCESSING;
						nrp->cycle = splitCycle++;
						(*routes)[drop]->push_back(nrp);
						lrp = nrp;
					}
				}
				tsCycle = splitCycle;
			}
			else
				claim(false, "Unsupported opearation encountered while routing.");
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////
// Advances droplet one cell in a linear array for droplet processing.  Droplet
// oscillates back and forth.
///////////////////////////////////////////////////////////////////////////////////
RoutePoint * PostSubproblemCompactionRouter::advanceDropInLinMod(RoutePoint *lrp, ReconfigModule *rm, bool *travelingRight, bool *travelingDown)
{
	int modWidth = rm->getRX() - rm->getLX() + 1;
	RoutePoint *nrp = new RoutePoint();

	// Horizontal linear array
	if (modWidth == 1)
	{
		// Go down and then up
		if (*travelingDown)
		{
			if (lrp->y == rm->getBY())
			{
				nrp->y = lrp->y-1;
				*travelingDown = false;
			}
			else
				nrp->y = lrp->y+1;
			nrp->x = lrp->x;
		}
		else
		{
			if (lrp->y == rm->getTY())
			{
				nrp->y = lrp->y+1;
				*travelingDown = true;
			}
			else
				nrp->y = lrp->y-1;
			nrp->x = lrp->x;
		}
	}
	else // Vertical linear array
	{
		// Go right and then left
		if (*travelingRight)
		{
			if (lrp->x == rm->getRX())
			{
				nrp->x = lrp->x-1;
				*travelingRight = false;
			}
			else
				nrp->x = lrp->x+1;
			nrp->y = lrp->y;
		}
		else
		{
			if (lrp->x == rm->getLX())
			{
				nrp->x = lrp->x+1;
				*travelingRight = true;
			}
			else
				nrp->x = lrp->x-1;
			nrp->y = lrp->y;
		}
	}
	nrp->dStatus = DROP_PROCESSING;
	return nrp;
}

///////////////////////////////////////////////////////////////////////////////////
// Advances droplet one cell in a module with at least one dimension of exactly
// two cells.  Droplet travels around array in clockwise manner.
///////////////////////////////////////////////////////////////////////////////////
RoutePoint * PostSubproblemCompactionRouter::advanceDropIn2ByMod(RoutePoint *lrp, ReconfigModule *rm)
{
	RoutePoint *nrp = new RoutePoint();

	if (lrp->x == rm->getLX()) // If in left column
	{
		if (lrp->y > rm->getTY()) // Go north if not at north border
		{
			nrp->x = lrp->x;
			nrp->y = lrp->y-1;
		}
		else // Else turn toward east
		{
			nrp->x = lrp->x+1;
			nrp->y = lrp->y;
		}
	}
	else if (lrp->y == rm->getTY()) // If in top row
	{
		if (lrp->x < rm->getRX()) // Go east if not at east border
		{
			nrp->x = lrp->x+1;
			nrp->y = lrp->y;
		}
		else // Else turn toward south
		{
			nrp->x = lrp->x;
			nrp->y = lrp->y+1;
		}
	}
	else if (lrp->x == rm->getRX()) // If in right column
	{
		if (lrp->y < rm->getBY()) // Go south if not at south border
		{
			nrp->x = lrp->x;
			nrp->y = lrp->y+1;
		}
		else // Else turn toward west
		{
			nrp->x = lrp->x-1;
			nrp->y = lrp->y;
		}
	}
	else if (lrp->y == rm->getBY()) // If in bottom row
	{
		if (lrp->x > rm->getLX()) // Go west if not at left border
		{
			nrp->x = lrp->x-1;
			nrp->y = lrp->y;
		}
		else // Else toward north
		{
			nrp->x = lrp->x;
			nrp->y = lrp->y-1;
		}
	}
	nrp->dStatus = DROP_PROCESSING;
	return nrp;
}

///////////////////////////////////////////////////////////////////////////////////
// Advances droplet one cell in a module in which both dimensions are at least
// 3 cells.  Droplet travels around module in zig-zag pattern.
///////////////////////////////////////////////////////////////////////////////////
RoutePoint * PostSubproblemCompactionRouter::advanceDropIn3By3PlusMod(RoutePoint *lrp, ReconfigModule *rm, bool *travelingRight, bool *travelingDown)
{
	RoutePoint *nrp = new RoutePoint();

	if ((lrp->x == rm->getRX() && *travelingRight) || (lrp->x == rm->getLX() && !(*travelingRight)))
	{	// Just hit left/right side, go up or down and then switch left/right direction

		if (*travelingDown && lrp->y == rm->getBY())
		{	// Hit bottom side, reverse to up
			*travelingDown = !(*travelingDown);
			nrp->y = lrp->y-1;
		}
		else if (*travelingDown)
		{
			nrp->y = lrp->y+1;
		}
		else if (!(*travelingDown) && lrp->y == rm->getTY())
		{
			*travelingDown = !(*travelingDown);
			nrp->y = lrp->y+1;
		}
		else if (!(*travelingDown))
			nrp->y = lrp->y-1;

		nrp->x = lrp->x;
		*travelingRight = !(*travelingRight);
	}
	else if (*travelingRight)
	{
		nrp->x = lrp->x+1;
		nrp->y = lrp->y;
	}
	else
	{
		nrp->x = lrp->x-1;
		nrp->y = lrp->y;
	}
	nrp->dStatus = DROP_PROCESSING;
	return nrp;
}


///////////////////////////////////////////////////////////////////////////////////
// After the individual routes have been computed and then compacted, this function
// takes the results of the sub-problem from subRoutes and adds them back to the
// global routes for the entire assay.
///////////////////////////////////////////////////////////////////////////////////
void PostSubproblemCompactionRouter::addSubProbToGlobalRoutes(vector<Droplet *> *subDrops, vector<vector<RoutePoint *> *> *subRoutes, map<Droplet *, vector<RoutePoint *> *> *routes)
{
	routeCycle = cycle;
	while (!subDrops->empty())
	{
		Droplet *d = subDrops->back();
		subDrops->pop_back();
		vector<RoutePoint *> *sr = subRoutes->back();
		subRoutes->pop_back();
		int delay = 0;
		while (!sr->empty())
		{
			RoutePoint *rp = sr->front();
			sr->erase(sr->begin());
			if (rp == NULL)
			{
				delay++;
				if (!(*routes)[d]->empty())
				{
					RoutePoint *lrp = (*routes)[d]->back();
					RoutePoint *nrp = new RoutePoint();
					nrp->cycle = lrp->cycle+1;
					nrp->dStatus = DROP_WAIT;
					nrp->x = lrp->x;
					nrp->y = lrp->y;
					(*routes)[d]->push_back(nrp);
				}
			}
			else
			{
				rp->cycle += delay;
				(*routes)[d]->push_back(rp);
			}
		}
		if ((*routes)[d]->back()->cycle >= routeCycle)
			routeCycle = (*routes)[d]->back()->cycle+1;
		delete sr;
	}
}

///////////////////////////////////////////////////////////////////////////////////
// Problems can arise later on in the compactor when individual routes are compacted
// together.  The main problem arises when one droplet's source is in the IR of
// another droplet's target.  Thus, the order in which droplets are compacted
// matters and can possibly solve this problem.  However, in the case of a dependency
// cycle, at least one droplet will need to be moved out of the way.  This function
// takes a more aggressive and less computationaly complex approach by removing
// all droplet-route dependencies.  If a source is in the IR of another droplet's
// target, the droplet with the conflicting source is simply moved out of the way
// before we compute the individual routes in computeIndivSupProbRoutes() so that
// it has a new source.  This funciton simply tries to move the droplet x=5 cells
// in each cardinal direction until it finds a clear path/destination for the
// conflicting source droplet; if it cannot find a clear path/dest, it reports a
// failure.
//
// WARNING: This solution is not guaranteed in that it is possible that there may
// not be a free block of cells in the immediate vicinity.  However, if we keep
// distance between modules, it should almost always work.  Even without space
// between modules, it should usually work given the module placement isn't
// extremely compact.
///////////////////////////////////////////////////////////////////////////////////
void PostSubproblemCompactionRouter::eliminateSubRouteDependencies(map<Droplet *, vector<RoutePoint *> *> *routes)
{
	// Get the nodes that need to be routed and sort
	vector<AssayNode *> routableThisTS;
	for (unsigned i = 0; i < thisTS->size(); i++)
		if (thisTS->at(i)->GetType() != DISPENSE && thisTS->at(i)->GetStartTS() == startingTS)
			routableThisTS.push_back(thisTS->at(i));

	// Gather the source and destination cells of each routable droplet this TS
	// For now, assume a non-io source is at the location of its last route point; non-io destination is bottom-left
	map<Droplet *, RoutePoint *> *sourceCells = new map<Droplet *, RoutePoint *>();
	map<Droplet *, RoutePoint *> *targetCells = new map<Droplet *, RoutePoint *>();
	vector<Droplet *> *routingThisTS = new vector<Droplet *>();

	for (unsigned i = 0; i < routableThisTS.size(); i++)
	{
		AssayNode *n = routableThisTS.at(i);
		for (unsigned p = 0; p < n->GetParents().size(); p++)
		{
			routeCycle = cycle;// DTG added to compact
			AssayNode *par = n->GetParents().at(p);
			Droplet *pd = par->GetDroplets().back();

			// If more than one parent droplet...remove...but then add to beginning so
			// can examine each split drop w/o actually removing and keeping routing order intact.
			if (par->droplets.size() > 1)
			{
				par->droplets.pop_back();
				par->droplets.insert(par->droplets.begin(), pd);
			}
			if (n->GetReconfigMod())
				n->GetReconfigMod()->incNumDrops(); // MUST clear this before leaving function b/c computeIndivSubRoutes() counts on it being untouched

			// First get sources
			RoutePoint *s = new RoutePoint();
			if (par->GetType() == DISPENSE)
			{
				if (par->GetIoPort()->getSide() == NORTH)
				{
					s->x = par->GetIoPort()->getPosXY();
					s->y = 0;
				}
				else if (par->GetIoPort()->getSide() == SOUTH)
				{
					s->x = par->GetIoPort()->getPosXY();
					s->y = arch->getNumCellsY()-1;

				}
				else if (par->GetIoPort()->getSide() == EAST)
				{
					s->x = arch->getNumCellsX()-1;
					s->y = par->GetIoPort()->getPosXY();
				}
				else if (par->GetIoPort()->getSide() == WEST)
				{
					s->x = 0;
					s->y = par->GetIoPort()->getPosXY();
				}
			}
			else
			{
				s->x = (*routes)[pd]->back()->x;
				s->y = (*routes)[pd]->back()->y; // last route point
			}
			sourceCells->insert(pair<Droplet *, RoutePoint *>(pd, s));

			// Now get targets
			RoutePoint *t = new RoutePoint();
			if (n->GetType() == OUTPUT)
			{
				if (n->GetIoPort()->getSide() == NORTH)
				{
					t->x = n->GetIoPort()->getPosXY();
					t->y = 0;
				}
				else if (n->GetIoPort()->getSide() == SOUTH)
				{
					t->x = n->GetIoPort()->getPosXY();
					t->y = arch->getNumCellsY()-1;
				}
				else if (n->GetIoPort()->getSide() == EAST)
				{
					t->x = arch->getNumCellsX()-1;
					t->y = n->GetIoPort()->getPosXY();
				}
				else if (n->GetIoPort()->getSide() == WEST)
				{
					t->x = 0;
					t->y = n->GetIoPort()->getPosXY();
				}
			}
			else // DTG, this will need to be adjusted for storage etc., when/if more than one destination in a module
			{
				// Original two lines - 6/13/2013 - DTG
				//t->x = n->GetReconfigMod()->getLX();
				//t->y = n->GetReconfigMod()->getBY();

				if (n->GetType() == STORAGE && n->GetReconfigMod()->getNumDrops() == 2)
				{	// Top-Left if second Storage drop
					t->x = n->GetReconfigMod()->getLX();
					t->y = n->GetReconfigMod()->getTY();
				}
				else if (n->GetType() == STORAGE && n->GetReconfigMod()->getNumDrops() == 3)
				{	// Bottom-right if third Storage drop
					t->x = n->GetReconfigMod()->getRX();
					t->y = n->GetReconfigMod()->getBY();
				}
				else if (n->GetType() == STORAGE && n->GetReconfigMod()->getNumDrops() == 4)
				{	// Top-right if fourth Storage drop
					t->x = n->GetReconfigMod()->getRX();
					t->y = n->GetReconfigMod()->getTY();
				}
				else if (n->GetType() == STORAGE && n->GetReconfigMod()->getNumDrops() > 4)
					claim(false, "Router is not currently designed to handle more than 4 storage droplets per module. Please reduce the number of storage drops per module or update the Roy Router source code.");
				else
				{	// Bottom-Left, else
					t->x = n->GetReconfigMod()->getLX();
					t->y = n->GetReconfigMod()->getBY();
				}

			}
			targetCells->insert(pair<Droplet *, RoutePoint *>(pd, t));

			// Debug print
			//debugPrintSourceTargetPair(pd, sourceCells, targetCells);
			//cout << "S/T Pair: Route d" << pd->getId() << " (" << s->x << ", " << s->y << ")-->(" << t->x << ", " << t->y << ")" << endl;
		}

	}

	// Now, clear the droplet count in the modules so can be used later
	for (unsigned i = 0; i < routableThisTS.size(); i++)
		if (routableThisTS.at(i)->GetReconfigMod())
			routableThisTS.at(i)->GetReconfigMod()->resetNumDrops();

	// Now that we've gotten the source/target information, check to see if there is a conflict
	map<Droplet *, RoutePoint *>::iterator srcIt = sourceCells->begin();
	for (; srcIt != sourceCells->end(); srcIt++)
	{
		RoutePoint *s = sourceCells->find(srcIt->first)->second;
		Droplet *d = sourceCells->find(srcIt->first)->first;

		// Do not move droplet if already at destination
		if (!(s->x == targetCells->find(d)->second->x && s->y == targetCells->find(d)->second->y))
		{
			// If we find an interference with another droplet's target, we must move the source location
			map<Droplet *, RoutePoint *>::iterator tgtIt = targetCells->begin();
			for (; tgtIt != targetCells->end(); tgtIt++)
			{
				RoutePoint *t = targetCells->find(tgtIt->first)->second;

				// If the source/target pair is not same route and interferes with each other, try moving the source
				if (doesInterfere(s, t) && (srcIt->first != tgtIt->first))
				{
					bool reRouted = false;
					int maxRerouteDist = 5; // Try moving up to x=5 cells in each direction

					/////////////////////////
					// Try LEFT/WEST
					/////////////////////////
					if (!reRouted)
					{
						int dist = 1;
						int newXDim; // Mod
						// While less than max reroute distance, hasn't already been re-reouted, and reroute is on DMFB
						while (dist <= maxRerouteDist && !reRouted && (newXDim = s->x - dist) >= 0) // MOD
						{
							RoutePoint *sDelta = new RoutePoint();
							sDelta->x = newXDim;
							sDelta->y = s->y;

							if (!rpInterferesWithRpList(sDelta, sourceCells, d))
							{
								if (!rpInterferesWithRpList(sDelta, targetCells, d))
								{
									if(!rpInterferesWithPersistentModule(sDelta))
									{
										unsigned long long int c = (*routes)[d]->back()->cycle;
										// Add new re-route to routes
										for (int i = 1; i <= dist; i++)
										{
											RoutePoint *newRp = new RoutePoint();
											newRp->x = s->x - i; // MOD
											newRp->y = s->y;	// MOD
											newRp->dStatus = DROP_NORMAL;
											newRp->cycle = c + i;
											(*routes)[d]->push_back(newRp);
										}
										(*routes)[d]->back()->dStatus = DROP_WAIT; // Set last location as waiting
										// Update s within our sources list
										s->x = (*routes)[d]->back()->x;
										s->y = (*routes)[d]->back()->y;
										s->dStatus = (*routes)[d]->back()->dStatus;
										reRouted = true;
										routeCycle = routeCycle + dist;
										equalizeGlobalRoutes(routes); // Equalize the routes so all droplets will appear in simulation during re-route cycles
									}
									else // If hit module, cannot take this direction
										break;
								}
							}
							else // If hit another source, then cannot take this direction
								break;
							dist++;
						}
					}
					/////////////////////////
					// Try RIGHT/EAST
					/////////////////////////
					if (!reRouted)
					{
						int dist = 1;
						int newXDim;
						// While less than max reroute distance, hasn't already been re-reouted, and reroute is on DMFB
						while (dist <= maxRerouteDist && !reRouted && (newXDim = s->x + dist) < arch->getNumCellsX())
						{
							RoutePoint *sDelta = new RoutePoint();
							sDelta->x = newXDim;
							sDelta->y = s->y;

							if (!rpInterferesWithRpList(sDelta, sourceCells, d))
							{
								if (!rpInterferesWithRpList(sDelta, targetCells, d))
								{
									if(!rpInterferesWithPersistentModule(sDelta))
									{
										unsigned long long int c = (*routes)[d]->back()->cycle;
										// Add new re-route to routes
										for (int i = 1; i <= dist; i++)
										{
											RoutePoint *newRp = new RoutePoint();
											newRp->x = s->x + i;
											newRp->y = s->y;
											newRp->dStatus = DROP_NORMAL;
											newRp->cycle = c + i;
											(*routes)[d]->push_back(newRp);
										}
										(*routes)[d]->back()->dStatus = DROP_WAIT; // Set last location as waiting
										// Update s within our sources list
										s->x = (*routes)[d]->back()->x;
										s->y = (*routes)[d]->back()->y;
										s->dStatus = (*routes)[d]->back()->dStatus;
										reRouted = true;
										routeCycle = routeCycle + dist;
										equalizeGlobalRoutes(routes); // Equalize the routes so all droplets will appear in simulation during re-route cycles
									}
									else // If hit module, cannot take this direction
										break;
								}
							}
							else // If hit another source, then cannot take this direction
								break;
							dist++;
						}
					}
					/////////////////////////
					// Try UP/NORTH
					/////////////////////////
					if (!reRouted)
					{
						int dist = 1;
						int newYDim;
						// While less than max reroute distance, hasn't already been re-reouted, and reroute is on DMFB
						while (dist <= maxRerouteDist && !reRouted && (newYDim = s->y - dist) >= 0)
						{
							RoutePoint *sDelta = new RoutePoint();
							sDelta->x = s->x;
							sDelta->y = newYDim;

							if (!rpInterferesWithRpList(sDelta, sourceCells, d))
							{
								if (!rpInterferesWithRpList(sDelta, targetCells, d))
								{
									if(!rpInterferesWithPersistentModule(sDelta))
									{
										unsigned long long int c = (*routes)[d]->back()->cycle;
										// Add new re-route to routes
										for (int i = 1; i <= dist; i++)
										{
											RoutePoint *newRp = new RoutePoint();
											newRp->x = s->x;
											newRp->y = s->y - i;
											newRp->dStatus = DROP_NORMAL;
											newRp->cycle = c + i;
											(*routes)[d]->push_back(newRp);
										}
										(*routes)[d]->back()->dStatus = DROP_WAIT; // Set last location as waiting
										// Update s within our sources list
										s->x = (*routes)[d]->back()->x;
										s->y = (*routes)[d]->back()->y;
										s->dStatus = (*routes)[d]->back()->dStatus;
										reRouted = true;
										routeCycle = routeCycle + dist;
										equalizeGlobalRoutes(routes); // Equalize the routes so all droplets will appear in simulation during re-route cycles
									}
									else // If hit module, cannot take this direction
										break;
								}
							}
							else // If hit another source, then cannot take this direction
								break;
							dist++;
						}
					}
					/////////////////////////
					// Try DOWN/SOUTH
					/////////////////////////
					if (!reRouted)
					{
						int dist = 1;
						int newYDim;
						// While less than max reroute distance, hasn't already been re-reouted, and reroute is on DMFB
						while (dist <= maxRerouteDist && !reRouted && (newYDim = s->y + dist) < arch->getNumCellsY())
						{
							RoutePoint *sDelta = new RoutePoint();
							sDelta->x = s->x;
							sDelta->y = newYDim;

							if (!rpInterferesWithRpList(sDelta, sourceCells, d))
							{
								if (!rpInterferesWithRpList(sDelta, targetCells, d))
								{
									if(!rpInterferesWithPersistentModule(sDelta))
									{
										unsigned long long int c = (*routes)[d]->back()->cycle;
										// Add new re-route to routes
										for (int i = 1; i <= dist; i++)
										{
											RoutePoint *newRp = new RoutePoint();
											newRp->x = s->x;
											newRp->y = s->y + i;
											newRp->dStatus = DROP_NORMAL;
											newRp->cycle = c + i;
											(*routes)[d]->push_back(newRp);
										}
										(*routes)[d]->back()->dStatus = DROP_WAIT; // Set last location as waiting
										// Update s within our sources list
										s->x = (*routes)[d]->back()->x;
										s->y = (*routes)[d]->back()->y;
										s->dStatus = (*routes)[d]->back()->dStatus;
										reRouted = true;
										routeCycle = routeCycle + dist;
										equalizeGlobalRoutes(routes); // Equalize the routes so all droplets will appear in simulation during re-route cycles
									}
									else // If hit module, cannot take this direction
										break;
								}
							}
							else // If hit another source, then cannot take this direction
								break;
							dist++;
						}
					}

					// If was re-rotued, good..exit loop.  Else, exit and inform user.
					if (reRouted)
						break; // no need to compare to more sources...it is safe
					else
						claim(false, "Droplet dependency was not fixed;  Attempt to break dependency by moving a source to free cells was a failure.");
				}
			}
		}
	}

	// Cleanup
	map<Droplet *, RoutePoint *>::iterator it;
	for (it = sourceCells->begin(); it != sourceCells->end(); it++)
	{
		RoutePoint *rp = it->second;
		it->second = NULL;
		delete rp;
	}
	sourceCells->clear();
	delete sourceCells;

	for (it = targetCells->begin(); it != targetCells->end(); it++)
	{
		RoutePoint *rp = it->second;
		it->second = NULL;
		delete rp;
	}
	targetCells->clear();
	delete targetCells;

	delete routingThisTS;
}

///////////////////////////////////////////////////////////////////////////////////
// Ensures that the given route point doesn't interfere with any of the route points
// in the map/list, besides the route point that corresponds to the same droplet
// (we are either comparing a source to the complete list of sources or complete
// list of targets)
///////////////////////////////////////////////////////////////////////////////////
bool PostSubproblemCompactionRouter::rpInterferesWithRpList(RoutePoint *rp, map<Droplet *, RoutePoint *> *rps, Droplet *d)
{
	// Search through entire list
	map<Droplet *, RoutePoint *>::iterator rpIt = rps->begin();
	for (; rpIt != rps->end(); rpIt++)
	{
		RoutePoint *rpCompare = rps->find(rpIt->first)->second;
		Droplet *dCompare = rpIt->first;
		if (d != dCompare && doesInterfere(rp, rpCompare))
			return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////
// Ensures that the given route point doesn't interfere with any of the persisting
// modules at the current time-step.
///////////////////////////////////////////////////////////////////////////////////
bool PostSubproblemCompactionRouter::rpInterferesWithPersistentModule(RoutePoint *rp)
{
	for (unsigned i = 0; i < thisTS->size(); i++)
	{
		AssayNode *n = thisTS->at(i);

		// Check all cells in all persistent modules (modules still executing; not starting/finishing this TS)
		if (n->type != DISPENSE && n->type != OUTPUT && startingTS > n->startTimeStep && startingTS < n->endTimeStep)
		{
			ReconfigModule *rm = n->reconfigMod;
			for (int x = rm->getLX(); x <= rm->getRX(); x++)
			{
				for (int y = rm->getTY(); y <= rm->getBY(); y++)
				{
					RoutePoint modPoint;
					modPoint.x = x;
					modPoint.y = y;
					if (doesInterfere(rp, &modPoint))
						return true;
				}
			}
		}
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////
// Ensures that the given route point doesn't interfere with any possible I/O ports.
// That is, returns true if the route point is in the outer two cells of the DMFB
// on any side.  This function is used to help keep the perimeter clear for droplets
// to be input/output.
///////////////////////////////////////////////////////////////////////////////////
bool PostSubproblemCompactionRouter::rpInterferesWithPotentialIoLocations(RoutePoint *rp)
{
	if (rp->x < 2 || rp->y < 2 || rp->x > arch->getNumCellsX() - 3 || rp->y > arch->getNumCellsY() - 3)
		return true;
	else
		return false;
}

///////////////////////////////////////////////////////////////////////////////////
// Computes the individual subroutes for a sub-problem. A new sub-route is created
// for each sub-rotue and added to subRoutes; also, the corresponding droplet is
// added to subDrops (corresponding routes and droplets must share the same index
// in subRoutes and subDrops).
//
// This function is called each time-step that droplets are being routed; it computes
// routes for a single sub-problem.
//
// Upon beginning this function, subRotues and subDrops is empty.  Upon exiting
// this funciton, subDrops is filled with a droplet for each droplet being routed
// during this time-step/sub-problem AND subRoutes is filled with a non-compacted
// route for each corresponding droplet in subDrops.  The routes are computed
// in isolation to be compacted later.
//
// This is the main function to be re-written by other routing algorithms as
// the individual routes are typically computed in various ways.
///////////////////////////////////////////////////////////////////////////////////
void PostSubproblemCompactionRouter::computeIndivSupProbRoutes(vector<vector<RoutePoint *> *> *subRoutes, vector<Droplet *> *subDrops, map<Droplet *, vector<RoutePoint *> *> *routes)
{
	claim(false, "No valid router was selected for the synthesis process or no method for 'computeIndivSupProbRoutes()' was implemented for the selected router.\n");
}

///////////////////////////////////////////////////////////////////////////////////
// This function is the main public function called.  It fills the "routes" and
// "tsBeginningCycle" data structures and contains the code for the main routing flow.
///////////////////////////////////////////////////////////////////////////////////
void PostSubproblemCompactionRouter::route(DAG *dag, DmfbArch *arch, vector<ReconfigModule *> *rModules, map<Droplet *, vector<RoutePoint *> *> *routes, vector<vector<int> *> *pinActivations, vector<unsigned long long> *tsBeginningCycle)
{
	cyclesPerTS = arch->getSecPerTS() * arch->getFreqInHz();
	startingTS = 0; // The TS that is getting ready to begin (droplets are being routed to)
	routeCycle = 0;
	cycle = 0;

	// Copy all nodes to a new list to be sorted
	vector<AssayNode *> nodes;// = new vector<AssayNode *>();
	for (unsigned i = 0; i < dag->getAllNodes().size(); i++)
		nodes.push_back(dag->getAllNodes().at(i));
	Sort::sortNodesByStartThenEndTS(&nodes);

	initCellTypeArray();
	routerSpecificInits();

	///////////////////////////////////////////////////////////////////////
	// This is the main loop. Each iteration of this loop solves one
	// time-step (routing sub-problem)
	///////////////////////////////////////////////////////////////////////
	while(!nodes.empty())
	{
		int j = 0;

		while ( j < nodes.size() && nodes.at(j)->GetStartTS() <= startingTS && nodes.at(j)->GetEndTS() >= startingTS && j < nodes.size())
			thisTS->push_back(nodes.at(j++));

		//if (startingTS >= 67)
		//{
		//	cout << "DebugPrint: Routing to TS " << startingTS << "." << endl; // DTG Debug Print
		//return;
		//}

		///////////////////////////////////////////////////////////////////////
		// First create any new droplets
		///////////////////////////////////////////////////////////////////////
		for (unsigned i = 0; i < thisTS->size(); i++)
		{
			AssayNode *n = thisTS->at(i);
			if (n->GetType() == DISPENSE && n->GetStartTS() == startingTS)
			{	// Create new droplets to be input soon
				Droplet *d = new Droplet();
				stringstream ss;
				ss << n->GetVolume() << " parts " << n->GetPortName();
				d->uniqueFluidName =  ss.str();
				d->volume = n->GetVolume();
				(*routes)[d] = new vector<RoutePoint *>();
				n->addDroplet(d);
			}
		}

		///////////////////////////////////////////////////////////////////////
		// Then, get the initial individual routes to be compacted later
		///////////////////////////////////////////////////////////////////////
		routeCycle = cycle;
		vector<vector<RoutePoint *> *> *subRoutes = new vector<vector<RoutePoint *> *>(); // subProblem routes
		vector<Droplet *> *subDrops = new vector<Droplet *>(); // corresponding subProblem droplets
		// If not a binder (which can take advantage of intra-module syncing to avoid dependencies), then eliminate dependencies
		if (!(getPastPlacerType() == GRISSOM_LE_B || getPastPlacerType() == GRISSOM_PATH_B))
			eliminateSubRouteDependencies(routes); // Optional; ensures that no source is in the IR of a target (moves the source out of way)
		computeIndivSupProbRoutes(subRoutes, subDrops, routes);
		//printSubRoutes(subRoutes, subDrops);

		///////////////////////////////////////////////////////////////////////
		// Then, compact and do maintenance on the routes
		///////////////////////////////////////////////////////////////////////
		compactRoutes(subDrops, subRoutes); // Now, do route COMPACTION
		addSubProbToGlobalRoutes(subDrops, subRoutes, routes); // Add sub-rotues to routes for entire simulation
		equalizeGlobalRoutes(routes); // Now, add cycles for the droplets that were waiting first so they look like they were really waiting there
		tsBeginningCycle->push_back(cycle); // Add cycle so we now when time-step begins
		processTimeStep(routes); // Now that routing is done, process the time-step

		///////////////////////////////////////////////////////////////////////
		// Cleanup
		///////////////////////////////////////////////////////////////////////
		for (int i = nodes.size()-1; i >= 0; i--)
			if (nodes.at(i)->endTimeStep <= startingTS)
				if (!(nodes.at(i)->startTimeStep == nodes.at(i)->endTimeStep && nodes.at(i)->startTimeStep == startingTS)) // Don't delete instant operations that are not yet processed
					nodes.erase(nodes.begin() + i);

		while (!subRoutes->empty())
		{
			vector<RoutePoint *> * v = subRoutes->back();
			subRoutes->pop_back();
			delete v; // Individual RoutePoints are deleted later by the Util Class
		}
		delete subRoutes;
		delete subDrops;
		thisTS->clear();
	}
}

///////////////////////////////////////////////////////////////////////////////////
// Debug function to print the computed subroutes
///////////////////////////////////////////////////////////////////////////////////
void PostSubproblemCompactionRouter::printSubRoutes(vector<vector<RoutePoint *> *> *subRoutes, vector<Droplet *> *subDrops)
{
	cout << "Routing to TS " << startingTS << ":" << endl;
	for (unsigned i = 0; i < subDrops->size(); i++)
	{
		Droplet *d = subDrops->at(i);
		vector<RoutePoint *> *route = subRoutes->at(i);

		cout << d->id << ": ";
		for (unsigned j = 0; j < route->size(); j++)
			cout << "(" << route->at(j)->x << ", " << route->at(j)->y << ") ";
		cout << endl;
	}
	cout << endl;
}

///////////////////////////////////////////////////////////////////////////////////
// Prints out source-target pair for the given droplet.
///////////////////////////////////////////////////////////////////////////////////
void PostSubproblemCompactionRouter::debugPrintSourceTargetPair(Droplet *d, map<Droplet *, RoutePoint *> *sourceCells, map<Droplet *, RoutePoint *> *targetCells)
{
	RoutePoint *s = sourceCells->at(d);
	RoutePoint *t = targetCells->at(d);
	cout << "S/T Pair: Route d" << d->getId() << " (" << s->x << ", " << s->y << ")-->(" << t->x << ", " << t->y << ")" << endl;
}

///////////////////////////////////////////////////////////////////////////////////
// Given the routes, counts and lists the number of intersections
///////////////////////////////////////////////////////////////////////////////////
void PostSubproblemCompactionRouter::printNumIntersections(vector<vector<RoutePoint *> *> *subRoutes, DmfbArch *arch)
{
	// Create a board to count number of times a droplet crossed a cell
	vector<int> column(arch->getNumCellsY(), 0);
	vector< vector<int> > board(arch->getNumCellsX(), column);

	int numberDropletMovements = 0;
	int compactedRouteLength = 0;

	// Increment cell contamination count
	//typedef map<Droplet *, vector<RoutePoint *> *>::iterator route_it;
	for(unsigned i = 0; i < subRoutes->size(); i++)
	{
		vector<RoutePoint*> * route = subRoutes->at(i);
		for(unsigned j = 0; j < route->size() ; j++)
		{
			if (route->at(j))
			{
				if (!(route->at(j) == route->back() && route->at(j)->dStatus != DROP_OUTPUT))
				{
					int x = route->at(j)->x;
					int y = route->at(j)->y;
					board[x][y]++;
				}
				numberDropletMovements++; // Increment instead of just add whole size b/c we don't want to count NULLs
			}
		}
		if (route->size() > compactedRouteLength)
			compactedRouteLength = route->size();
		//total_route_length += route->size();
	}

	int total_crossings = 0;
	for(unsigned i = 0; i < board.size() ; i++)
		for(unsigned j = 0; j < board[i].size() ; j++)
			if(board[i][j] > 1)
				total_crossings += board[i][j] - 1;

	// Easy-to-read output
	cout << "Routing to TS " << startingTS << ":" << endl;
	cout << "Total Crossings: " << total_crossings <<endl;
	cout << "Number of droplet movements: " << numberDropletMovements << endl;
	cout << "Compacted Route Length: " << compactedRouteLength << endl << endl;

	// CSV Compatible output
	//cout << "TS,# Crossings, # Drop Movements, Compacted Route Length" << endl;
	//cout << startingTS << "," << total_crossings << "," << numberDropletMovements << "," << compactedRouteLength << endl;
}

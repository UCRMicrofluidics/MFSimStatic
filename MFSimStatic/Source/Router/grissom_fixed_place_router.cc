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
 * Source: grissom_fixed_place_router.cc										*
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
#include "../../Headers/Router/grissom_fixed_place_router.h"

///////////////////////////////////////////////////////////////////////////////////
// Constructors
///////////////////////////////////////////////////////////////////////////////////
GrissomFixedPlaceRouter::GrissomFixedPlaceRouter()
{
	claim(false, "Invalid constructor used for Router variant.  Must use form that accepts DmfbArch.\n");
}
GrissomFixedPlaceRouter::GrissomFixedPlaceRouter(DmfbArch *dmfbArch)
{
	arch = dmfbArch;
}

///////////////////////////////////////////////////////////////////////////////////
// Deconstructor
///////////////////////////////////////////////////////////////////////////////////
GrissomFixedPlaceRouter::~GrissomFixedPlaceRouter()
{
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
//
// This specific function routes droplets to the left until it can go north/south,
// then routes either north/south until it gets to the proper row, then routes
// east/west to the target cell.
///////////////////////////////////////////////////////////////////////////////////
void GrissomFixedPlaceRouter::computeIndivSupProbRoutes(vector<vector<RoutePoint *> *> *subRoutes, vector<Droplet *> *subDrops, map<Droplet *, vector<RoutePoint *> *> *routes)
{
	RoutePoint *lrp = NULL;
	RoutePoint *nrp = NULL;

	// Add blockages for the modules on the board this time-step
	bool cellIsBlocked[arch->getNumCellsX()][arch->getNumCellsY()];
	// First, reset board for new route.....
	for (int x = 0; x < arch->getNumCellsX(); x++)
		for (int y = 0; y < arch->getNumCellsY(); y++)
			cellIsBlocked[x][y] = false;
	/*for (int j = 0; j < persistingModules->size(); j++)
	{
		ReconfigModule *pm = persistingModules->at(j);
		for (int x = pm->getLX()-1; x <= pm->getRX()+1; x++)
			for (int y = pm->getTY()-1; y <= pm->getBY()+1; y++)
				if (x >= 0 && y >= 0 && x < arch->getNumCellsX() && y < arch->getNumCellsY())
					cellIsBlocked[x][y] = true;
	}*/
	for (unsigned i = 0; i < thisTS->size(); i++)
	{
		AssayNode *n = thisTS->at(i);
		if (!(n->GetType() == DISPENSE || n->GetType() == OUTPUT))
		{	// Block all new reconfigurable areas from being used for routing
			ReconfigModule *rm = n->GetReconfigMod();
			for (int x = rm->getLX()-1; x <= rm->getRX()+1; x++)
				for (int y = rm->getTY()-1; y <= rm->getBY()+1; y++)
					cellIsBlocked[x][y] = true;
		}
	}

	// Get the nodes that need to be routed and sort
	vector<AssayNode *> routableThisTS;
	for (unsigned i = 0; i < thisTS->size(); i++)
		if (thisTS->at(i)->GetType() != DISPENSE && thisTS->at(i)->GetStartTS() == startingTS)
			routableThisTS.push_back(thisTS->at(i));
	Sort::sortNodesByLatestTSThenStorage(&routableThisTS);
	while (!routableThisTS.empty())
	{
		//bool wasRouted = true;
		AssayNode *n = routableThisTS.front();
		routableThisTS.erase(routableThisTS.begin());
		for (unsigned p = 0; p < n->GetParents().size(); p++)
		{
			routeCycle = cycle;// DTG added to compact
			AssayNode *par = n->GetParents().at(p);
			Droplet *pd = par->GetDroplets().back();
			par->droplets.pop_back();
			n->addDroplet(pd);
			subDrops->push_back(pd);
			vector<RoutePoint *> *sr = new vector<RoutePoint *>();
			subRoutes->push_back(sr);
			int numCellsOut = 2; // Number of cells the droplet must travel to get in/out of the module to its syncing point

			int tx;// target x
			int ty; // target y

			// Route to an output, assumes already in some module
			if (n->type == OUTPUT)
			{
				lrp = (*routes)[pd]->back(); // last route point

				if (n->ioPort->getSide() == NORTH || n->ioPort->getSide() == SOUTH)
				{
					tx = n->ioPort->getPosXY();
					if (n->ioPort->getSide() == NORTH)
						ty = 0;
					else
						ty = arch->getNumCellsY()-1;

					for (int k = 0; k < numCellsOut; k++)
					{
						nrp = new RoutePoint(); // next route point
						nrp->cycle = routeCycle++;
						nrp->dStatus = DROP_NORMAL;
						nrp->x = lrp->x+1;
						nrp->y = lrp->y;
						sr->push_back(nrp);
						lrp = nrp;
					}
					// Route along the Y axis to the North/South border
					int dirY = 0;
					if (ty > lrp->y)
						dirY = 1;
					else
						dirY = -1;
					while (lrp->y != ty)
					{
						nrp = new RoutePoint(); // next route point
						nrp->cycle = routeCycle++;
						nrp->dStatus = DROP_NORMAL;
						nrp->x = lrp->x;
						nrp->y = lrp->y+dirY;
						sr->push_back(nrp);
						lrp = nrp;
					}

					// Then router along the X axis to the actual port
					int dirX = 0;
					if (tx > lrp->x)
						dirX = 1;
					else
						dirX = -1;
					while (lrp->x != tx)
					{
						nrp = new RoutePoint(); // next route point
						nrp->cycle = routeCycle++;
						nrp->dStatus = DROP_NORMAL;
						nrp->x = lrp->x+dirX;
						nrp->y = lrp->y;
						sr->push_back(nrp);
						lrp = nrp;
					}
					lrp->dStatus = DROP_OUTPUT;
				}
				else if (n->ioPort->getSide() == EAST || n->ioPort->getSide() == WEST)
				{
					// Get the direction to leave the module
					int dirY;
					ReconfigModule *rMod = par->reconfigMod;
					if (lrp->y == rMod->getBY())
						dirY = 1; // At the bottom, go South to get out
					else if (lrp->y == rMod->getTY())
						dirY = -1;
					else
						claim(false, "Droplet is not in a module exit, but is trying to exit the module.");

					ty = n->ioPort->getPosXY();
					if (n->ioPort->getSide() == EAST)
						tx = arch->getNumCellsX()-1;
					else
						tx = 0;

					for (int k = 0; k < numCellsOut; k++)
					{
						nrp = new RoutePoint(); // next route point
						nrp->cycle = routeCycle++;
						nrp->dStatus = DROP_NORMAL;
						nrp->x = lrp->x;
						nrp->y = lrp->y+dirY;
						sr->push_back(nrp);
						lrp = nrp;
					}
					// Then router along the X axis to the actual port
					int dirX = 0;
					if (tx > lrp->x)
						dirX = 1;
					else
						dirX = -1;
					while (lrp->x != tx)
					{
						nrp = new RoutePoint(); // next route point
						nrp->cycle = routeCycle++;
						nrp->dStatus = DROP_NORMAL;
						nrp->x = lrp->x+dirX;
						nrp->y = lrp->y;
						sr->push_back(nrp);
						lrp = nrp;
					}
					// Route along the Y axis to the North/South border
					dirY = 0;
					if (ty > lrp->y)
						dirY = 1;
					else
						dirY = -1;
					while (lrp->y != ty)
					{
						nrp = new RoutePoint(); // next route point
						nrp->cycle = routeCycle++;
						nrp->dStatus = DROP_NORMAL;
						nrp->x = lrp->x;
						nrp->y = lrp->y+dirY;
						sr->push_back(nrp);
						lrp = nrp;
					}
					lrp->dStatus = DROP_OUTPUT;
				}
				else
					claim(false, "Invalid direction for IoSide.");
			}
			else // Routing from input/module to module
			{
				bool stayingInModule = false;
				// Give it a starting point if coming from a Dispense
				if (par->GetType() == DISPENSE)
				{
					nrp = new RoutePoint();
					nrp->cycle = routeCycle++;
					nrp->dStatus = DROP_NORMAL;
					if (par->GetIoPort()->getSide() == NORTH)
						{ nrp->x = par->GetIoPort()->getPosXY(); nrp->y = 0; }
					else if (par->GetIoPort()->getSide() == SOUTH)
						{ nrp->x = par->GetIoPort()->getPosXY(); nrp->y = arch->getNumCellsY()-1; }
					else if (par->GetIoPort()->getSide() == EAST)
						{ nrp->x = arch->getNumCellsX()-1; nrp->y = par->GetIoPort()->getPosXY(); }
					else if (par->GetIoPort()->getSide() == WEST)
						{ nrp->x = 0; nrp->y = par->GetIoPort()->getPosXY(); }
					sr->push_back(nrp);
					lrp = nrp;
				}
				else // Else, it already has a starting point, get it out of the module and onto the routing cells, if changing modules
				{
					lrp = (*routes)[pd]->back(); // last route point

					// If the droplet is already in the same module
					stayingInModule = (lrp->x >= n->reconfigMod->getLX()) && (lrp->x <= n->reconfigMod->getRX()) && (lrp->y >= n->reconfigMod->getTY()) && (lrp->y <= n->reconfigMod->getBY());
					// Splits must leave module b/c no easy way to choose a specific droplet
					if (par->type == SPLIT || par->type == DILUTE)
						stayingInModule = false;

					if (!stayingInModule)
					{
						int dirY;
						ReconfigModule *rMod = par->reconfigMod;
						if (lrp->y == rMod->getBY())
							dirY = 1; // At the bottom, go South to get out
						else if (lrp->y == rMod->getTY())
							dirY = -1;
						else
							claim(false, "Droplet is not in a module exit, but is trying to exit the module.");

						// Now, go the first few cells to get out of the last module and onto the routing cells
						for (int k = 0; k < numCellsOut; k++)
						{
							nrp = new RoutePoint(); // next route point
							nrp->cycle = routeCycle++;
							nrp->dStatus = DROP_NORMAL;
							nrp->x = lrp->x;
							nrp->y = lrp->y+dirY;
							sr->push_back(nrp);
							lrp = nrp;
						}
					}
				}

				// Now, DO THE ROUTING
				ReconfigModule *rm = n->GetReconfigMod();
				int dirEnterModY = -1; // By default, we enter from South, so must travel North (-1 on the Y-axis)
				int fx; // final x in module...used so drop doesn't have to leave module if not its next module location is exact same
				int fy; // final y in module
				tx = fx = rm->getLX();
				ty = rm->getBY()+numCellsOut; // for non-storage modules, always enter at bottom
				fy = rm->getBY();

				if (n->type == STORAGE) // Storage droplets enter at the top or bottom
				{
					int dropIdTop = dropletOccupyingCell(rm->getLX(), rm->getTY(), routes, subRoutes, subDrops);
					int dropIdBot = dropletOccupyingCell(rm->getLX(), rm->getBY(), routes, subRoutes, subDrops);

					if (dropIdTop < 0 || dropIdTop == pd->id)
					{
						ty = rm->getTY()-numCellsOut;
						fy = rm->getTY();
						dirEnterModY = 1;
					}
					else if (dropIdBot < 0 || dropIdBot == pd->id)
					{
						ty = rm->getBY()+numCellsOut;
						fy = rm->getBY();
					}
					else
						claim(false, "Storage module is already full with other droplets.");
				}

				if (!stayingInModule)
				{
					while(!(lrp->x == tx && lrp->y == ty))
					{
						int xDiff = tx - lrp->x;
						int yDiff = ty - lrp->y;

						int dirX = 0;
						int dirY = 0;

						if (xDiff > 0)
							dirX = 1;
						else if (xDiff < 0)
							dirX = -1;

						if (yDiff > 0)
							dirY = 1;
						else if (yDiff < 0)
							dirY = -1;

						bool canTravelNS = false;
						if (lrp->x >= 0 && lrp->x < arch->getNumCellsX() && lrp->y+dirY >= 0 && lrp->y+dirY < arch->getNumCellsY())
							canTravelNS = !cellIsBlocked[lrp->x][lrp->y+dirY];

						// First, go all the way to the y-coord (destination row)
						if (lrp->y != ty)
						{
							if (canTravelNS)
							{	// If we can go NS, do it!
								nrp = new RoutePoint(); // next route point
								nrp->cycle = routeCycle++;
								nrp->dStatus = DROP_NORMAL;
								nrp->x = lrp->x;
								nrp->y = lrp->y+dirY;
								sr->push_back(nrp);
								lrp = nrp;
							}
							else
							{	// Else, travel east/west until we hit a column where we can travel to the y-coord
								if (dirX == 0)
									dirX = -1;

								while (!canTravelNS)
								{
									nrp = new RoutePoint(); // next route point
									nrp->cycle = routeCycle++;
									nrp->dStatus = DROP_NORMAL;
									nrp->x = lrp->x+dirX;
									nrp->y = lrp->y;
									sr->push_back(nrp);
									lrp = nrp;

									canTravelNS = false;
									if (lrp->x >= 0 && lrp->x < arch->getNumCellsX() && lrp->y+dirY >= 0 && lrp->y+dirY < arch->getNumCellsY())
										canTravelNS = !cellIsBlocked[lrp->x][lrp->y+dirY];
								}
							}
						}
						else
						{	// Go the final distance in the x-direction to the target
							nrp = new RoutePoint(); // next route point
							nrp->cycle = routeCycle++;
							nrp->dStatus = DROP_NORMAL;
							nrp->x = lrp->x+dirX;
							nrp->y = lrp->y;
							sr->push_back(nrp);
							lrp = nrp;
						}
					}
					// Now, go the final few cells to get the droplet into the actual module area
					for (int k = 0; k < numCellsOut; k++)
					{
						nrp = new RoutePoint(); // next route point
						nrp->cycle = routeCycle++;
						nrp->dStatus = DROP_NORMAL;
						nrp->x = lrp->x;
						nrp->y = lrp->y+dirEnterModY;
						sr->push_back(nrp);
						lrp = nrp;
					}
				}
			}
		}
	}
}

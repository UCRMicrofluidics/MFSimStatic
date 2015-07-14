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
 * Source: grissom_fixed_place_map_router.cc									*
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
#include "../../Headers/Router/grissom_fixed_place_map_router.h"

///////////////////////////////////////////////////////////////////////////////////
// Constructors
///////////////////////////////////////////////////////////////////////////////////
GrissomFixedPlaceMapRouter::GrissomFixedPlaceMapRouter()
{
	claim(false, "Invalid constructor used for Router variant.  Must use form that accepts DmfbArch.\n");
}
GrissomFixedPlaceMapRouter::GrissomFixedPlaceMapRouter(DmfbArch *dmfbArch)
{
	routeMap = new map<string, vector<pair<int, int> *> *>();
	arch = dmfbArch;
}

///////////////////////////////////////////////////////////////////////////////////
// Deconstructor
///////////////////////////////////////////////////////////////////////////////////
GrissomFixedPlaceMapRouter::~GrissomFixedPlaceMapRouter()
{
	if (routeMap)
	{
		map<string, vector<pair<int, int> *> *>::iterator it = routeMap->begin();
		for (; it != routeMap->end(); it++)
		{
			vector<pair<int, int> *> *route = it->second;
			while (!route->empty())
			{
				pair<int, int> *point = route->back();
				route->pop_back();
				delete point;
			}
			delete route;
		}
		routeMap->clear();
		delete routeMap;
	}
}

///////////////////////////////////////////////////////////////////////////////////
// This function creates a map of routes from one module to another.  It computes
// routes in the same way that GrissomFixedPlaceRouter::computeIndivSupProbRoutes()
// computes routes.
//
// Note: This function was written overnight before a deadline as a new idea, so
// the code is quite inelegant and brute...but it works :-)
///////////////////////////////////////////////////////////////////////////////////
void GrissomFixedPlaceMapRouter::preRoute(DmfbArch *arch)
{
	ElapsedTimer rmcTime("Route Map Construction Time");
	rmcTime.startTimer();

	// Block cells permanantly for module placements
	vector<FixedModule *> modLocations;
	int modWidth = 4;//2;
	int modHeight = 3;//5;
	int modIntWidth = modWidth + 2; // For interference region
	int modIntHeight = modHeight + 2; // For interference region
	int maxX = arch->getNumCellsX();
	int maxY = arch->getNumCellsY();
	int x = 1;
	int y = 1;
	while (y + modIntHeight < maxY)
	{
		if ( (x + modIntWidth <= maxX) && (y + modIntHeight <= maxY) )
		{
			//FixedModule *fm = new FixedModule(GENERAL, x+1, y+1, x+modWidth, y+modHeight); // DTG
			FixedModule *fm = new FixedModule(BASIC_RES, x+1, y+1, x+modWidth, y+modHeight);
			modLocations.push_back(fm);
		}
		x = x + modIntWidth +1;
		if (x + modIntWidth >= maxX)
		{
			x = 1;
			y = y + modIntHeight + 1;
		}
	}

	int colRouteMult = modIntWidth + 1; // If a droplet has an x % colRouteMult = 0, is in a column


	//map<string, vector<pair<int, int> *> *> *routeMap = new map<string, vector<pair<int, int> *> *>();
	//stringstream ss;
	//ss.str("");
	//ss << x1 << "-" << y1 << "-" << x2 << "-" << y2;

	bool startTop;
	bool endTop;

	// Create a route from each input to a module and each module to an output
	for (unsigned i = 0; i < arch->getIoPorts()->size(); i++)
	{

		IoPort *p = arch->getIoPorts()->at(i);

		if (p->isAnInput())
		{
			int sx;
			int sy;
			if (p->getSide() == NORTH)
				{ sx = p->getPosXY(); sy = 0; }
			else if (p->getSide() == SOUTH)
				{ sx = p->getPosXY(); sy = arch->getNumCellsY()-1; }
			else if (p->getSide() == EAST)
				{ sx = arch->getNumCellsX()-1; sy = p->getPosXY(); }
			else if (p->getSide() == WEST)
				{ sx = 0; sy = p->getPosXY(); }

			//pair<int, int> *coord = new pair<int, int>(x, y);


			for (unsigned j = 0; j < modLocations.size(); j++)
			{
				for (int k = 0; k < 2; k++)
				{
					vector<pair<int, int> *> * newRoute = new vector<pair<int, int> *>();
					newRoute->push_back(new pair<int, int>(sx, sy));

					int tx = modLocations.at(j)->getLX(); // Target x, y coordinate
					int ty;
					x = sx;
					y = sy;
					if (k == 0)
					{
						endTop = false;
						ty = modLocations.at(j)->getBY()+2; // Two below Bottom
					}
					else
					{
						endTop = true;
						ty = modLocations.at(j)->getTY()-2; // Two above top
					}

					// If starting on the top or bottom, we need to get to a column
					if (p->getSide() == NORTH || p->getSide() == SOUTH)
					{
						// If starting out to east of target, go west until first col (if not already in a column)
						if (x >= tx && (x % colRouteMult != 0))
						{
							int nextCol = (x / colRouteMult)*colRouteMult;
							while (x != nextCol)
							{
								x--;
								newRoute->push_back(new pair<int, int>(x, y));
							}
						}
						else if ((x % colRouteMult) != 0)
						{
							int nextCol = (x / colRouteMult)*colRouteMult + colRouteMult;
							while (x != nextCol)
							{
								x++;
								newRoute->push_back(new pair<int, int>(x, y));
							}
						}
					}
					// Now that we're in the column, go north/south to proper row
					if (y > ty) // We are south of target, go north
					{
						while (y != ty)
						{
							y--;
							newRoute->push_back(new pair<int, int>(x, y));
						}
					}
					else if (y < ty)
					{
						while (y != ty)
						{
							y++;
							newRoute->push_back(new pair<int, int>(x, y));
						}
					}
					// Now go the final x distance
					if (x > tx) // We are south of target, go north
					{
						while (x != tx)
						{
							x--;
							newRoute->push_back(new pair<int, int>(x, y));
						}
					}
					else if (x < tx)
					{
						while (x != tx)
						{
							x++;
							newRoute->push_back(new pair<int, int>(x, y));
						}
					}

					// Now go last 2 cells to get into module
					if (endTop)
					{
						y++;
						newRoute->push_back(new pair<int, int>(x, y));
						y++;
						newRoute->push_back(new pair<int, int>(x, y));
					}
					else
					{
						y--;
						newRoute->push_back(new pair<int, int>(x, y));
						y--;
						newRoute->push_back(new pair<int, int>(x, y));
					}

					stringstream coord;
					coord << sx << "-" << sy << "-" << x << "-" << y;
					//string coordStr = coord.str();

					//cout << coord.str() << " added: ";
					//for (int m = 0; m < newRoute->size(); m++)
					//	cout << "(" << newRoute->at(m)->first << "," << newRoute->at(m)->second << ")-->";
					//cout << endl;

					routeMap->insert(pair<string, vector<pair<int, int> *> *>(coord.str(), newRoute));
				}
			}
		}
		else
		{
			// If an output port.....
			int tx;
			int ty;
			if (p->getSide() == NORTH)
				{ tx = p->getPosXY(); ty = 0; }
			else if (p->getSide() == SOUTH)
				{ tx = p->getPosXY(); ty = arch->getNumCellsY()-1; }
			else if (p->getSide() == EAST)
				{ tx = arch->getNumCellsX()-1; ty = p->getPosXY(); }
			else if (p->getSide() == WEST)
				{ tx = 0; ty = p->getPosXY(); }


			for (unsigned j = 0; j < modLocations.size(); j++)
			{
				for (int k = 0; k < 2; k++)
				{
					vector<pair<int, int> *> * newRoute = new vector<pair<int, int> *>();

					int sx = modLocations.at(j)->getRX(); // Target x, y coordinate
					int sy;

					if (k == 0)
					{
						startTop = false;
						sy = modLocations.at(j)->getBY(); // Two below Bottom
					}
					else
					{
						startTop = true;
						sy = modLocations.at(j)->getTY(); // Two above top
					}

					newRoute->push_back(new pair<int, int>(sx, sy));
					x = sx;
					y = sy;

					// If north/south output, go out module into column
					if (p->getSide() == NORTH || p->getSide() == SOUTH)
					{
						x++;
						newRoute->push_back(new pair<int, int>(x, y));
						x++;
						newRoute->push_back(new pair<int, int>(x, y));

						// First, go all the way to top/bottom of array
						if (p->getSide() == NORTH)
						{
							while (y != 0)
							{
								y--;
								newRoute->push_back(new pair<int, int>(x, y));
							}
						}
						else // going to bottom
						{
							while (y != arch->getNumCellsY()-1)
							{
								y++;
								newRoute->push_back(new pair<int, int>(x, y));
							}
						}

						// Then, go left or right to port
						if (x < tx)
						{
							while (x != tx)
							{
								x++;
								newRoute->push_back(new pair<int, int>(x, y));
							}
						}
						else if (x > tx)
						{
							while (x != tx)
							{
								x--;
								newRoute->push_back(new pair<int, int>(x, y));
							}
						}
					}
					else // if output is east/west, then go out of module top/bottom into row
					{
						if (startTop)
						{
							y--;
							newRoute->push_back(new pair<int, int>(x, y));
							y--;
							newRoute->push_back(new pair<int, int>(x, y));
						}
						else
						{
							y++;
							newRoute->push_back(new pair<int, int>(x, y));
							y++;
							newRoute->push_back(new pair<int, int>(x, y));
						}

						// First, go all the way to left/right of array
						if (p->getSide() == WEST)
						{
							while (x != 0)
							{
								x--;
								newRoute->push_back(new pair<int, int>(x, y));
							}
						}
						else // going to east
						{
							while (x != arch->getNumCellsX()-1)
							{
								x++;
								newRoute->push_back(new pair<int, int>(x, y));
							}
						}

						// Then, go up/down to port
						if (y < ty)
						{
							while (y != ty)
							{
								y++;
								newRoute->push_back(new pair<int, int>(x, y));
							}
						}
						else if (y > ty)
						{
							while (y != ty)
							{
								y--;
								newRoute->push_back(new pair<int, int>(x, y));
							}
						}
						stringstream coord;
						coord << sx << "-" << sy << "-" << x << "-" << y;
						routeMap->insert(pair<string, vector<pair<int, int> *> *>(coord.str(), newRoute));
					}
				}
			}
		}
	}

	// Now create routes from any module to any module
	//for (int i = 0; i < modLocations.size(); i++)
		//cout << modLocations.at(i)->getLX() << "--" << modLocations.at(i)->getTY() << endl;

	for (unsigned i = 0; i < modLocations.size(); i++)
	{
		FixedModule *startMod = modLocations.at(i);
		for (int j = 0; j < 2; j++)  // Two exits
		{

			// From startMod to every other module
			for (unsigned k = 0; k < modLocations.size(); k++)
			{
				FixedModule *stopMod = modLocations.at(k);

				//if (startMod == stopMod)
				//{
				//}
				//else
				if (true)
				{
					for (int l = 0; l < 2; l++)  // Two entrances
					{
						int sx = startMod->getRX();
						int sy;
						if (j == 0)
						{
							startTop = false;
							sy = startMod->getBY(); // Two below Bottom
						}
						else
						{
							startTop = true;
							sy = startMod->getTY(); // Two above top
						}
						int tx = stopMod->getLX();
						int ty;
						if (l == 0)
						{
							endTop = false;
							ty = stopMod->getBY()+2; // Two below Bottom
						}
						else
						{
							endTop = true;
							ty = stopMod->getTY()-2; // Two above top
						}

						x = sx;
						y = sy;

						vector<pair<int, int> *> * newRoute = new vector<pair<int, int> *>();
						newRoute->push_back(new pair<int, int>(sx, sy));

						// Route out of modules
						if (startTop)
						{
							y--;
							newRoute->push_back(new pair<int, int>(x, y));
							y--;
							newRoute->push_back(new pair<int, int>(x, y));
						}
						else
						{
							y++;
							newRoute->push_back(new pair<int, int>(x, y));
							y++;
							newRoute->push_back(new pair<int, int>(x, y));
						}

						// If starting out to east of target, go west until first col (if not already in a column)
						if (x >= tx && (x % colRouteMult != 0))
						{
							int nextCol = (x / colRouteMult)*colRouteMult;
							while (x != nextCol)
							{
								x--;
								newRoute->push_back(new pair<int, int>(x, y));
							}
						}
						else if ((x % colRouteMult) != 0)
						{
							int nextCol = (x / colRouteMult)*colRouteMult + colRouteMult;
							while (x != nextCol)
							{
								x++;
								newRoute->push_back(new pair<int, int>(x, y));
							}
						}
						// Now that we're in the column, go north/south to proper row
						if (y > ty) // We are south of target, go north
						{
							while (y != ty)
							{
								y--;
								newRoute->push_back(new pair<int, int>(x, y));
							}
						}
						else if (y < ty)
						{
							while (y != ty)
							{
								y++;
								newRoute->push_back(new pair<int, int>(x, y));
							}
						}
						// Now go the final x distance
						if (x > tx) // We are south of target, go north
						{
							while (x != tx)
							{
								x--;
								newRoute->push_back(new pair<int, int>(x, y));
							}
						}
						else if (x < tx)
						{
							while (x != tx)
							{
								x++;
								newRoute->push_back(new pair<int, int>(x, y));
							}
						}

						// Now go last 2 cells to get into module
						if (endTop)
						{
							y++;
							newRoute->push_back(new pair<int, int>(x, y));
							y++;
							newRoute->push_back(new pair<int, int>(x, y));
						}
						else
						{
							y--;
							newRoute->push_back(new pair<int, int>(x, y));
							y--;
							newRoute->push_back(new pair<int, int>(x, y));
						}

						stringstream coord;
						coord << sx << "-" << sy << "-" << x << "-" << y;
						//string coordStr = coord.str();

						//cout << coord.str() << " added: ";
						//for (int m = 0; m < newRoute->size(); m++)
						//	cout << "(" << newRoute->at(m)->first << "," << newRoute->at(m)->second << ")-->";
						//cout << endl;

						routeMap->insert(pair<string, vector<pair<int, int> *> *>(coord.str(), newRoute));
					}
				}
			}
		}
	}

	// DEBUG PRINT ROUTE LIBRARY
	/*map<string, vector<pair<int, int> *> *>::iterator libIt = routeMap->begin();
	for (; libIt != routeMap->end(); libIt++)
	{
		cout << libIt->first << ": ";
		for (int i = 0; i < libIt->second->size(); i++)
			cout << "(" << libIt->second->at(i)->first << "," << libIt->second->at(i)->second << ")-->";
		cout << endl;
	}*/

	rmcTime.endTimer();
	rmcTime.printElapsedTime();
}


///////////////////////////////////////////////////////////////////////////////////
// Computes the individual subroutes for a sub-problem. A new sub-route is created
// for each sub-rotue and added to subRoutes; also, the corresponding droplet is
// added to subDrops (corresponding routes and droplets must share the same index
// in subRoutes and subDrops).
//
// The only difference between this class and the GrissomFixedPlaceRouter class
// is that this class creates the routes in a map first, instead of on the fly.
//
// See PostSubproblemCompactRouter::computeIndivSupProbRoutes() for more details.
///////////////////////////////////////////////////////////////////////////////////
void GrissomFixedPlaceMapRouter::computeIndivSupProbRoutes(vector<vector<RoutePoint *> *> *subRoutes, vector<Droplet *> *subDrops, map<Droplet *, vector<RoutePoint *> *> *routes)
{
	RoutePoint *lrp = NULL;
	RoutePoint *nrp = NULL;

	// Get the nodes that need to be routed and sort
	vector<AssayNode *> routableThisTS;
	for (unsigned i = 0; i < thisTS->size(); i++)
		if (thisTS->at(i)->GetType() != DISPENSE && thisTS->at(i)->GetStartTS() == startingTS)
			routableThisTS.push_back(thisTS->at(i));

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
				}
				else if (n->ioPort->getSide() == EAST || n->ioPort->getSide() == WEST)
				{
					ty = n->ioPort->getPosXY();
					if (n->ioPort->getSide() == EAST)
						tx = arch->getNumCellsX()-1;
					else
						tx = 0;
				}
				else
					claim(false, "Invalid direction for IoSide.");
				stringstream coord;
				coord << lrp->x << "-" << lrp->y << "-" << tx << "-" << ty;
				vector<pair<int, int> *> *libRoute = (*routeMap)[coord.str()];
				for (unsigned lr = 1; lr < libRoute->size(); lr++)
				{
					nrp = new RoutePoint(); // next route point
					nrp->cycle = routeCycle++;
					nrp->dStatus = DROP_NORMAL;
					nrp->x = libRoute->at(lr)->first;
					nrp->y = libRoute->at(lr)->second;
					sr->push_back(nrp);
					lrp = nrp;
				}
				lrp->dStatus = DROP_OUTPUT;
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
				}

				// Now, DO THE ROUTING
				ReconfigModule *rm = n->GetReconfigMod();
				//int dirEnterModY = -1; // By default, we enter from South, so must travel North (-1 on the Y-axis)
				int fx; // final x in module...used so drop doesn't have to leave module if not its next module location is exact same
				int fy; // final y in module
				//tx = fx = rm->getLX();
				fx = rm->getLX();
				//ty = rm->getBY()+numCellsOut; // for non-storage modules, always enter at bottom
				fy = rm->getBY();

				if (n->type == STORAGE) // Storage droplets enter at the top or bottom
				{
					int dropIdTop = dropletOccupyingCell(rm->getLX(), rm->getTY(), routes, subRoutes, subDrops);
					int dropIdBot = dropletOccupyingCell(rm->getLX(), rm->getBY(), routes, subRoutes, subDrops);

					if (dropIdTop < 0 || dropIdTop == pd->id)
						fy = rm->getTY();
					else if (dropIdBot < 0 || dropIdBot == pd->id)
						fy = rm->getBY();
					else
						claim(false, "Storage module is already full with other droplets.");
				}

				if (!stayingInModule)
				{
					stringstream coord;
					coord << lrp->x << "-" << lrp->y << "-" << fx << "-" << fy;
					vector<pair<int, int> *> *libRoute = (*routeMap)[coord.str()];
					for (unsigned lr = 1; lr < libRoute->size(); lr++)
					{
						nrp = new RoutePoint(); // next route point
						nrp->cycle = routeCycle++;
						nrp->dStatus = DROP_NORMAL;
						nrp->x = libRoute->at(lr)->first;
						nrp->y = libRoute->at(lr)->second;
						sr->push_back(nrp);
						lrp = nrp;
					}
				}
			}
		}
	}
}

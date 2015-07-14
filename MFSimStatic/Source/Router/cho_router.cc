/*------------------------------------------------------------------------------*
 *                       (c)2012, All Rights Reserved.     						*
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
 * Source: cho_router.cc														*
 * Original Code Author(s): Mark Louton											*
 * Original Completion/Release Date: May 31, 2013								*
 *																				*
 * Details: N/A																	*
 *																				*
 * Revision History:															*
 * WHO		WHEN		WHAT													*
 * ---		----		----													*
 * DTG		10/03/13	Small changes to blockage-grid and new droplet RP gen.	*
 *-----------------------------------------------------------------------------*/
#include "../../Headers/Router/cho_router.h"

bool sDecreasingInts2(int *i1, int *i2) { return ((*i1) > (*i2)); }

///////////////////////////////////////////////////////////////////////////////////
// Constructors
///////////////////////////////////////////////////////////////////////////////////
ChoRouter::ChoRouter()
{
	board = NULL;
	claim(false, "Invalid constructor used for Router variant.  Must use form that accepts DmfbArch.\n");
}
ChoRouter::ChoRouter(DmfbArch *dmfbArch)
{
	board = NULL;
	arch = dmfbArch;
}

///////////////////////////////////////////////////////////////////////////////////
// Deconstructor
///////////////////////////////////////////////////////////////////////////////////
ChoRouter::~ChoRouter()
{
	if (board)
	{
		while (!board->empty())
		{
			vector<BoardCell1*> *v = board->back();
			board->pop_back();
			while (!v->empty())
			{
				BoardCell1 *c = v->back();
				v->pop_back();
				delete c;
			}
			delete v;
		}
		delete board;
	}
}

///////////////////////////////////////////////////////////////////////////////////
// Initializes the board full of board cells
///////////////////////////////////////////////////////////////////////////////////
void ChoRouter::routerSpecificInits()
{
	// Create a 2D-array of Board cells
	board = new vector<vector<BoardCell1 *> *>();
	for (int x = 0; x < arch->getNumCellsX(); x++)
	{
		vector<BoardCell1 *> *col = new vector<BoardCell1 *>();
		for (int y = 0; y < arch->getNumCellsY(); y++)
		{
			BoardCell1 *c = new BoardCell1();
			c->x = x;
			c->y = y;
			c->type = Empty;
			c->distance = getInfinityVal();
			c->visited = false;
			c->prev = NULL;
			col->push_back(c);
		}
		board->push_back(col);
	}
}

///////////////////////////////////////////////////////////////////////////////////////
// Sorts routingThisTS in decreasing order, based on the bypassibility between the
// corresponding source and target cells.
///////////////////////////////////////////////////////////////////////////////////////
void ChoRouter::sortDropletsInDecBypassability(vector<Droplet *> *routingThisTS, map<Droplet *, BoardCell1 *> *sourceCells, map<Droplet *, BoardCell1 *> *targetCells, int numCellsX, int numCellsY)
{
	vector<int *> area;
	map<int *, Droplet *> link;
	int h_up = 0;
	int h_down = 0;
	int v_left = 0;
	int v_right = 0;
	for (unsigned int i = 0; i < routingThisTS->size(); i++)
	{
		//BoardCell1 *s = sourceCells->at(routingThisTS->at(i));
		BoardCell1 *t = targetCells->at(routingThisTS->at(i));
		int *bypassibility = new int();
		//set the bypassibility directions
		if(t->x-2 >= 0)
		{
			if(t->y-2 >= 0 && t->y+2 < numCellsY)
				v_left = 1;
		}
		if(t->x+2 <= numCellsX)
		{
			if(t->y-2 >= 0 && t->y+2 < numCellsY)
				v_right = 1;
		}
		if(t->y-2 >= 0)
		{
			if(t->x-2 >= 0 && t->x+2 < numCellsX)
				h_down = 1;
		}
		if(t->y+2 <= numCellsY)
		{
			if(t->x-2 >= 0 && t->x+2 < numCellsX)
				h_up = 1;
		}
		//set the bypassibility
		if(h_up || h_down)
		{
			if(v_left || v_right)
				*bypassibility = 3;
			else
				*bypassibility = 2;
		}
		else if(v_left || v_right)
			*bypassibility = 2;
		else
			*bypassibility = 1;

		//add found bypassibility to set
		area.push_back(bypassibility);
		link[bypassibility] = routingThisTS->at(i);
	}

	sort(area.begin(), area.end(), sDecreasingInts2);

	routingThisTS->clear();
	for (unsigned int i = 0; i < area.size(); i++)
		routingThisTS->push_back(link[area.at(i)]);

	while (!area.empty())
	{
		int *i = area.back();
		area.pop_back();
		delete i;
	}
}

///////////////////////////////////////////////////////////////////////////////////
// Computes the individual subroutes for a sub-problem. A new sub-route is created
// for each sub-route and added to subRoutes; also, the corresponding droplet is
// added to subDrops (corresponding routes and droplets must share the same index
// in subRoutes and subDrops).
//
// This algorithm uses Dijkstra's algorithm to compute individual paths.
//
// See PostSubproblemCompactRouter::computeIndivSupProbRoutes() for more details.
///////////////////////////////////////////////////////////////////////////////////
void ChoRouter::computeIndivSupProbRoutes(vector<vector<RoutePoint *> *> *subRoutes, vector<Droplet *> *subDrops, map<Droplet *, vector<RoutePoint *> *> *routes)
{
	//eliminateSubRouteDependencies(routes); // Ensures that no source is in the IR of a target (moves the source out of way) - used in lieu of concession

	// Get the nodes that need to be routed and sort
	vector<AssayNode *> routableThisTS;
	for (unsigned int i = 0; i < thisTS->size(); i++)
		if (thisTS->at(i)->GetType() != DISPENSE && thisTS->at(i)->GetStartTS() == startingTS)
			routableThisTS.push_back(thisTS->at(i));

	// Gather the source and destination cells of each routable droplet this TS
	map<Droplet *, BoardCell1 *> *sourceCells = new map<Droplet *, BoardCell1 *>();
	map<Droplet *, BoardCell1 *> *targetCells = new map<Droplet *, BoardCell1 *>();
	set<Droplet *> dropsBeingOutput;
	vector<Droplet *> *routingThisTS = new vector<Droplet *>();
	BoardCell1 *s = NULL;
	BoardCell1 *t = NULL;
	for (unsigned int i = 0; i < routableThisTS.size(); i++)
	{
		AssayNode *n = routableThisTS.at(i);
		for (unsigned int p = 0; p < n->GetParents().size(); p++)
		{
			routeCycle = cycle;// DTG added to compact
			AssayNode *par = n->GetParents().at(p);
			Droplet *pd = par->GetDroplets().back();
			routingThisTS->push_back(pd);
			par->droplets.pop_back();
			n->addDroplet(pd);
			if (n->GetReconfigMod())
				n->GetReconfigMod()->incNumDrops();

			// First get sources
			s = NULL;
			if (par->GetType() == DISPENSE)
			{
				if (par->GetIoPort()->getSide() == NORTH)
					s = board->at(par->GetIoPort()->getPosXY())->at(0);
				else if (par->GetIoPort()->getSide() == SOUTH)
					s = board->at(par->GetIoPort()->getPosXY())->at(arch->getNumCellsY()-1);
				else if (par->GetIoPort()->getSide() == EAST)
					s = board->at(arch->getNumCellsX()-1)->at(par->GetIoPort()->getPosXY());
				else if (par->GetIoPort()->getSide() == WEST)
					s = board->at(0)->at(par->GetIoPort()->getPosXY());
			}
			else
				s = board->at((*routes)[pd]->back()->x)->at((*routes)[pd]->back()->y); // last route point
			sourceCells->insert(pair<Droplet *, BoardCell1 *>(pd, s));

			// Now get targets
			t = NULL;
			if (n->GetType() == OUTPUT)
			{
				if (n->GetIoPort()->getSide() == NORTH)
					t = board->at(n->GetIoPort()->getPosXY())->at(0);
				else if (n->GetIoPort()->getSide() == SOUTH)
					t = board->at(n->GetIoPort()->getPosXY())->at(arch->getNumCellsY()-1);
				else if (n->GetIoPort()->getSide() == EAST)
					t = board->at(arch->getNumCellsX()-1)->at(n->GetIoPort()->getPosXY());
				else if (n->GetIoPort()->getSide() == WEST)
					t = board->at(0)->at(n->GetIoPort()->getPosXY());
				dropsBeingOutput.insert(pd); // Add to set of droplets being output
			}
			else // DTG, this will need to be adjusted for storage etc., when/if more than one destination in a module
			{
				if (n->GetType() == STORAGE && n->GetReconfigMod()->getNumDrops() > 1)
					t = board->at(n->GetReconfigMod()->getLX())->at(n->GetReconfigMod()->getTY()); // Top-Left if second Storage drop
				else
					t = board->at(n->GetReconfigMod()->getLX())->at(n->GetReconfigMod()->getBY()); // Bottom-Left, else
			}
			targetCells->insert(pair<Droplet *, BoardCell1 *>(pd, t));
		}
	}

	// Sort in decreasing order of bypassibility
	sortDropletsInDecBypassability(routingThisTS, sourceCells, targetCells, arch->getNumCellsX(), arch->getNumCellsY());

	//route each droplet
	for (unsigned int i = 0; i < routingThisTS->size(); i++)
	{
		bool routeComplete = false;
		bool noRoute = false;
		int nodesVisited = 0;
		int numCells = 0;		//number of cells on the board
		routeCycle = cycle; // DTG added for compaction
		Droplet *d = routingThisTS->at(i);
		subDrops->push_back(d);
		vector<RoutePoint *> *sr = new vector<RoutePoint *>();
		subRoutes->push_back(sr);

		//BoardCell pos;
		s = sourceCells->at(d);
		t = targetCells->at(d);

		// reset board for new route.....
		vector<vector<BoardCell1> >grid;
		for (int x = 0; x < arch->getNumCellsX(); x++)
		{
			vector<BoardCell1> col;
			for (int y = 0; y < arch->getNumCellsY(); y++)
			{
				BoardCell1 c;
				c.x = x;
				c.y = y;
				c.type = Empty;
				c.distance = getInfinityVal();
				c.visited = false;
				c.prev = NULL;
				col.push_back(c);

				numCells++;
			}
			grid.push_back(col);
		}
		// add blockages for modules on the board
		for(unsigned int i = 0; i < thisTS->size(); i++)
		{
			AssayNode *n = thisTS->at(i);
			if (!(n->GetType() == DISPENSE || n->GetType() == OUTPUT))
			{
				// Block all new reconfigurable areas from being used for routing
				ReconfigModule *rm = n->GetReconfigMod();
				if (n->startTimeStep < startingTS && n->endTimeStep > startingTS)
				{
					for (int x = rm->getLX()-1; x <= rm->getRX()+1; x++)
						for (int y = rm->getTY()-1; y <= rm->getBY()+1; y++)
							if (x >= 0 && y >= 0 && x < arch->getNumCellsX() && y < arch->getNumCellsY())
								grid[x][y].type = Blocked;
				}
			}
		}
		// add blockages for other droplet sources/targets
		for(unsigned int j = 0; j < routingThisTS->size(); j++)
		{
			Droplet *d2 = routingThisTS->at(j);
			if(d != d2)	//if another droplet
			{
				BoardCell1 *c = sourceCells->at(d2);		//add blockage for sources
				if( c != s ) 		// Don't block its own source
				{
					for(int x = c->x-1; x <= c->x+1; x++)
					{
						for(int y = c->y-1; y <= c->y+1; y++)
						{
							if(x >= 0 && y >= 0 && x < arch->getNumCellsX() && y < arch->getNumCellsY()) // must be on the board
							{
								if(!(*routes)[d2]->empty()) // Don't mark as blockage for dispense droplets; they wait in reservoir
									grid[x][y].type = Blocked;
							}
						}
					}
				}

				c = targetCells->at(d2);	//add blockage for targets
				if(c != t)			//don't block its own target
				{
					for(int x = c->x-1; x <= c->x+1; x++)
					{
						for(int y = c->y-1; y <= c->y+1; y++)
						{
							if(x >= 0 && y >= 0 && x < arch->getNumCellsX() && y < arch->getNumCellsY()) // must be on the board
								if (dropsBeingOutput.count(d2) == 0) // Don't mark as blockage for output dorplets; they go into reservoir
									grid[x][y].type = Blocked;
						}
					}
				}
			}
		}	//finish blockages from other droplet sources/targets
		// ensure own target are not considered blockages
		for (int x = t->x-1; x <= t->x+1; x++)
			for (int y = t->y-1; y <= t->y+1; y++)
				if (x >= 0 && y >= 0 && x < arch->getNumCellsX() && y < arch->getNumCellsY()) // On board?
					if (dropsBeingOutput.count(d) == 0) // Don't mark as free when outputting droplet b/c can cause conflict in some cases
						grid[x][y].type = Empty;

		if(s != t)		//if not already at target, route it
		{
			//Add initial input position if necessary
			/*if((*routes)[d]->empty())
			{
				RoutePoint *rp = new RoutePoint();
				rp->cycle = routeCycle++;
				rp->dStatus = DROP_NORMAL;
				rp->x = s->x;
				rp->y = s->y;
				sr->push_back(rp);
			}*/

			//set source as initial node (distance = 0)
			grid[s->x][s->y].distance = 0;

			//in while loop, construct the shortest path
			while(!routeComplete && !noRoute)
			{
				//find node with shortest distance
				int minX = 0;
				int minY = 0;
				int minDist = getInfinityVal();
				for(int x = 0; x < arch->getNumCellsX(); x++)
				{
					for (int y = 0; y < arch->getNumCellsY(); y++)
					{
						if(grid[x][y].visited == false && grid[x][y].type == Empty)
						{
							if(grid[x][y].distance < minDist)
							{
								minX = x;
								minY = y;
								minDist = grid[x][y].distance;
							}
						}
					}
				}

				//for node, update all unvisited neighbors if is a shorter path
				if(minX-1 >= 0 && minX-1 < arch->getNumCellsX() )
				{
					//if cell to west is closer after the current cell is found, update the cell
					if(grid[minX-1][minY].visited == false && grid[minX-1][minY].distance > (grid[minX][minY].distance + 1)
							&& grid[minX-1][minY].type == Empty)
					{
						grid[minX-1][minY].distance = grid[minX][minY].distance + 1;
						grid[minX-1][minY].prev = &grid[minX][minY];
					}
				}
				if(minY+1 >= 0 && minY+1 < arch->getNumCellsY() )
				{
					//if cell to north is closer after the current cell is found, update the cell
					if(grid[minX][minY+1].visited == false && grid[minX][minY+1].distance > (grid[minX][minY].distance + 1)
							&& grid[minX][minY+1].type == Empty)
					{
						grid[minX][minY+1].distance = grid[minX][minY].distance + 1;
						grid[minX][minY+1].prev = &grid[minX][minY];
					}
				}
				if(minX+1 >= 0 && minX+1 < arch->getNumCellsX() )
				{
					//if cell to east is closer after the current cell is found, update the cell
					if(grid[minX+1][minY].visited == false && grid[minX+1][minY].distance > (grid[minX][minY].distance + 1)
							&& grid[minX+1][minY].type == Empty)
					{
						grid[minX+1][minY].distance = grid[minX][minY].distance + 1;
						grid[minX+1][minY].prev = &grid[minX][minY];
					}
				}
				if(minY-1 >= 0 && minY-1 < arch->getNumCellsY() )
				{
					//if cell to south is closer after the current cell is found, update the cell
					if(grid[minX][minY-1].visited == false && grid[minX][minY-1].distance > (grid[minX][minY].distance + 1)
							&& grid[minX][minY-1].type == Empty)
					{
						grid[minX][minY-1].distance = grid[minX][minY].distance + 1;
						grid[minX][minY-1].prev = &grid[minX][minY];
					}
				}

				//set current node as visited
				grid[minX][minY].visited = true;
				nodesVisited++;

				if(nodesVisited > numCells)		//no route was found
				{
					noRoute = true;

				}
				if(minX == t->x && minY == t->y)	//have reached target
				{
					routeComplete = true;

					//use target's previous node to find the shortest path

					BoardCell1 * pos = &grid[t->x][t->y];
					stack<BoardCell1 *> tempS;
					while(pos != NULL)		//put nodes in order of visitation
					{
						tempS.push(pos);
						pos = pos->prev;
					}
					while(!tempS.empty())	//load route into the subroutes structure
					{
						BoardCell1 * pos2;
						pos2 = tempS.top();
						RoutePoint *rp2 = new RoutePoint();
						rp2->cycle = routeCycle++;
						rp2->dStatus = DROP_NORMAL;
						rp2->x = pos2->x;
						rp2->y = pos2->y;
						sr->push_back(rp2);
						tempS.pop();
					}

					// Set last droplet as output if true
					if (dropsBeingOutput.count(d) > 0)
						sr->back()->dStatus = DROP_OUTPUT;
				}

			}	//end construct shortest path while loop
		}	//finish current droplet
	}

	delete routingThisTS;
	delete sourceCells;
	delete targetCells;
}

///////////////////////////////////////////////////////////////////////////////////
//Gets the sources and targets of all droplets for the compaction process
///////////////////////////////////////////////////////////////////////////////////
void ChoRouter::compactorGetSourcesTargets(vector<Droplet *> *subDrops, vector<vector<RoutePoint *> *> *subRoutes, map<Droplet *, BoardCell1 *> *sourceCells, map<Droplet *, BoardCell1 *> *targetCells)
{
	BoardCell1 *s = NULL;
	BoardCell1 *t = NULL;
	for (unsigned int i = 0; i < subRoutes->size(); i++)
	{
		s = NULL;
		t = NULL;

		if(subRoutes->at(i)->size() == 0)
			continue;
		//get source - is first route point
		BoardCell1 hs;
		hs.x = subRoutes->at(i)->front()->x;
		hs.y = subRoutes->at(i)->front()->y;

		s = &hs;
		sourceCells->insert(pair<Droplet *, BoardCell1 *>(subDrops->at(i), s));

		//get target - is last route point
		BoardCell1 ht;
		ht.x = subRoutes->at(i)->front()->x;
		ht.y = subRoutes->at(i)->front()->y;

		t = &ht;
		targetCells->insert(pair<Droplet *, BoardCell1 *>(subDrops->at(i), t));
	}
}

///////////////////////////////////////////////////////////////////////////////////
//Compacts the route several times either by using timing constraints or fault tolerance
//constraints.
///////////////////////////////////////////////////////////////////////////////////
void ChoRouter::compactRoutesByTimingAndFaultTolerance(vector<Droplet *> *subDrops, vector<vector<RoutePoint *> *> *subRoutes)
{
	//maximum number of times to go through the loop
	const int maxNumIterations = 5;

	int iterations = 0;
	bool routeLonger = false;

	//compaction loop
	while(iterations < maxNumIterations)
	{
		iterations++;
		//cout << "Iteration: " << iterations << endl;

		Sort::sortRoutesByLength(subRoutes, subDrops); // Route longer paths first

		// Ensure that all routes' last point is labeled appropriately
		for (unsigned int i = 0; i < subRoutes->size(); i++)
			if (subRoutes->at(i)->size() > 0)
				subRoutes->at(i)->back()->dStatus = DROP_WAIT;

		// Gather the source and destination cells of each droplet
		map<Droplet *, BoardCell1 *> *sourceCells = new map<Droplet *, BoardCell1 *>();
		map<Droplet *, BoardCell1 *> *targetCells = new map<Droplet *, BoardCell1 *>();
		vector<Droplet *> *routingThisTS = new vector<Droplet *>();

		//get the sources and targets of the droplets
		compactorGetSourcesTargets(subDrops, subRoutes, sourceCells, targetCells);

		BoardCell1 *s = NULL;
		BoardCell1 *t = NULL;

		//route for timing constraints

		//create new vectors for the time constraint
		vector<Droplet *> *subDropsTC = new vector<Droplet *>();
		vector<vector<RoutePoint *> *> *subRoutesTC = new vector<vector<RoutePoint *> *>();

		//create the grid for the fault tolerance
		vector<vector<BoardCell2> > ftgrid;
		int numCells = 0;		//number of cells on the board

		for (int x = 0; x < arch->getNumCellsX(); x++)
		{
			vector<BoardCell2> col;
			for (int y = 0; y < arch->getNumCellsY(); y++)
			{
				BoardCell2 c;
				c.x = x;
				c.y = y;
				c.type = Empty;
				c.occupation = 4;
				c.distance = getInfinityVal();
				c.visited = false;
				c.prev = NULL;
				col.push_back(c);

				numCells++;
			}
			ftgrid.push_back(col);
		}

		//route each droplet
		for(unsigned int di = 0; di < subDrops->size(); di++)
		{
			if(subRoutes->at(di)->size() == 0)
				continue;
			else if(sourceCells->size() < di || targetCells->size() < di)
				continue;

			bool routeComplete = false;
			bool noRoute = false;
			int nodesVisited = 0;
			numCells = 0;
			routeCycle = cycle; // DTG added for compaction
			Droplet *d = subDrops->at(di);
			subDropsTC->push_back(d);
			vector<RoutePoint *> *sr = new vector<RoutePoint *>();
			subRoutesTC->push_back(sr);

			BoardCell1 hs;
			hs.x = subRoutes->at(di)->front()->x;
			hs.y = subRoutes->at(di)->front()->y;
			s = &hs;
			BoardCell1 ht;
			ht.x = subRoutes->at(di)->front()->x;
			ht.y = subRoutes->at(di)->front()->y;
			t = &ht;

			// reset board for new route.....
			vector<vector<BoardCell1> >grid;
			for (int x = 0; x < arch->getNumCellsX(); x++)
			{
				vector<BoardCell1> col;
				for (int y = 0; y < arch->getNumCellsY(); y++)
				{
					BoardCell1 c;
					c.x = x;
					c.y = y;
					c.type = Empty;
					c.distance = getInfinityVal();
					c.visited = false;
					c.prev = NULL;
					col.push_back(c);

					numCells++;
				}
				grid.push_back(col);
			}
			// add blockages for modules on the board
			for(unsigned int i = 0; i < thisTS->size(); i++)
			{
				AssayNode *n = thisTS->at(i);
				if (!(n->GetType() == DISPENSE || n->GetType() == OUTPUT))
				{
					// Block all new reconfigurable areas from being used for routing
					ReconfigModule *rm = n->GetReconfigMod();
					if (n->startTimeStep < startingTS && n->endTimeStep > startingTS)
					{
						for (int x = rm->getLX()-1; x <= rm->getRX()+1; x++)
							for (int y = rm->getTY()-1; y <= rm->getBY()+1; y++)
								if (x >= 0 && y >= 0 && x < arch->getNumCellsX() && y < arch->getNumCellsY())
									grid[x][y].type = Blocked;
					}
				}
			}
			// add blockages for other droplet sources/targets
			for(unsigned int j = 0; j < subDrops->size(); j++)
			{
				if(subRoutes->at(j)->size() == 0)
					continue;
				Droplet *d2 = subDrops->at(j);
				if(d != d2)	//if another droplet
				{
					BoardCell1 sc;
					sc.x = subRoutes->at(j)->front()->x;
					sc.y = subRoutes->at(j)->front()->y;
					BoardCell1 *c = &sc;		//add blockage for sources
					if( c != s ) 		// Don't block its own source
					{
						for(int x = c->x-1; x <= c->x+1; x++)
						{
							for(int y = c->y-1; y <= c->y+1; y++)
							{
								if(x >= 0 && y >= 0 && x < arch->getNumCellsX() && y < arch->getNumCellsY()) // must be on the board
								{
									grid[x][y].type = Blocked;
								}
							}
						}
					}

					BoardCell1 tc;
					tc.x = subRoutes->at(j)->front()->x;
					tc.y = subRoutes->at(j)->front()->y;
					c = &tc;	//add blockage for targets
					if(c != t)			//don't block its own target
					{
						for(int x = c->x-1; x <= c->x+1; x++)
						{
							for(int y = c->y-1; y <= c->y+1; y++)
							{
								if(x >= 0 && y >= 0 && x < arch->getNumCellsX() && y < arch->getNumCellsY()) // must be on the board
									grid[x][y].type = Blocked;
							}
						}
					}
				}
			}	//finish blockages from other droplet sources/targets

			if(s != t)		//if not already at target, route it
			{
				//set source as initial node (distance = 0)
				grid[s->x][s->y].distance = 0;

				//in while loop, construct the shortest path
				while(!routeComplete && !noRoute)
				{
					//find node with shortest distance
					int minX = 0;
					int minY = 0;
					int minDist = getInfinityVal();
					for(int x = 0; x < arch->getNumCellsX(); x++)
					{
						for (int y = 0; y < arch->getNumCellsY(); y++)
						{
							if(grid[x][y].visited == false && grid[x][y].type == Empty)
							{
								if(grid[x][y].distance < minDist)
								{
									minX = x;
									minY = y;
									minDist = grid[x][y].distance;
								}
							}
						}
					}

					//for node, update all unvisited neighbors if is a shorter path
					if(minX-1 >= 0 && minX-1 < arch->getNumCellsX() )
					{
						//if cell to west is closer after the current cell is found, update the cell
						if(grid[minX-1][minY].visited == false && grid[minX-1][minY].distance > grid[minX][minY].distance + 1
								&& grid[minX-1][minY].type == Empty)
						{
							grid[minX-1][minY].distance = grid[minX][minY].distance + 1;
							grid[minX-1][minY].prev = &grid[minX][minY];
						}
					}
					if(minY+1 >= 0 && minY+1 < arch->getNumCellsY() )
					{
						//if cell to north is closer after the current cell is found, update the cell
						if(grid[minX][minY+1].visited == false && grid[minX][minY+1].distance > grid[minX][minY].distance + 1
								&& grid[minX][minY+1].type == Empty)
						{
							grid[minX][minY+1].distance = grid[minX][minY].distance + 1;
							grid[minX][minY+1].prev = &grid[minX][minY];
						}
					}
					if(minX+1 >= 0 && minX+1 < arch->getNumCellsX() )
					{
						//if cell to east is closer after the current cell is found, update the cell
						if(grid[minX+1][minY].visited == false && grid[minX+1][minY].distance > grid[minX][minY].distance + 1
								&& grid[minX+1][minY].type == Empty)
						{
							grid[minX+1][minY].distance = grid[minX][minY].distance + 1;
							grid[minX+1][minY].prev = &grid[minX][minY];
						}
					}
					if(minY-1 >= 0 && minY-1 < arch->getNumCellsY() )
					{
						//if cell to south is closer after the current cell is found, update the cell
						if(grid[minX][minY-1].visited == false && grid[minX][minY-1].distance > grid[minX][minY].distance + 1
								&& grid[minX][minY-1].type == Empty)
						{
							grid[minX][minY-1].distance = grid[minX][minY].distance + 1;
							grid[minX][minY-1].prev = &grid[minX][minY];
						}
					}

					//set current node as visited
					grid[minX][minY].visited = true;
					nodesVisited++;

					if(minX == t->x && minY == t->y)	//have reached target
					{
						routeComplete = true;

						//use target's previous node to find the shortest path
						BoardCell1 * pos = &grid[t->x][t->y];
						stack<BoardCell1 *> tempS;
						int tcRLength = 0;
						while(pos != NULL)		//put nodes in order of visitation
						{
							tempS.push(pos);
							pos = pos->prev;
							tcRLength++;
						}
						if(tcRLength > subRoutes->at(di)->size())
							routeLonger = true;
						else
						{
							int ind = 0;
							//subRoutes->erase(subRoutes->begin()+di);  //erase current route and add in new route
							while(!tempS.empty())	//load route into the subroutes structure
							{
								//cout << "added route" << endl;
								BoardCell1 * pos2;
								pos2 = tempS.top();
								RoutePoint *rp2 = new RoutePoint();
								rp2->cycle = routeCycle++;
								rp2->dStatus = DROP_NORMAL;
								rp2->x = pos2->x;
								rp2->y = pos2->y;
								sr->push_back(rp2);
								tempS.pop();
								if(ind != 0)
								{
									subRoutes->at(di)->at(ind) = rp2;
								}
								ind++;
							}
						}
					}
					if(nodesVisited >= numCells)		//no route was found
					{
						noRoute = true;
					}

				}	//end construct shortest path while loop
			}	//finish routing current droplet
			if(!routeLonger)
			{
				//after routed, compact droplet
				// Ensure that all routes' last point is labeled appropriately
				for (unsigned int i = 0; i < subRoutes->size(); i++)
					if (subRoutes->at(i)->size() > 0)
						subRoutes->at(i)->back()->dStatus = DROP_WAIT;

				int longestRoute = 0;
				if (subRoutes->size() > 0)
					longestRoute = subRoutes->at(0)->size();

				int numStallsToPrepend = 1;

				vector<RoutePoint *> *subRoute = subRoutesTC->at(di);
				RoutePoint *destPt = NULL;
				if (subRoute->size() > 0)
					destPt = subRoute->back();

				// Check entire route
				bool isInterference = false;
				unsigned int j = 0; // The index used to traverse a specific route/cycle

				while (j != max((int)subRoute->size(), longestRoute) && subRoute->size() > 0)
				{
					RoutePoint *rp = NULL;
					if (j <= subRoute->size()-1)
						rp = subRoute->at(j);
					else
						rp = subRoute->back();

					if (rp)
					{
						// Check against the previous routes that have been compacted
						for(unsigned int k = 0; k < di; k++)
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
							else
								prp = pastRoute->back();

							RoutePoint *prpLc = NULL; // Past route's last cycle
							if (j > 0 && j <= pastRoute->size()-1)
								prpLc = pastRoute->at(j-1);
							else
								prpLc = pastRoute->back();

							RoutePoint *prpNc = NULL; // Past route's next cycle
							if (j+1 <= pastRoute->size()-1)
								prpNc = pastRoute->at(j+1);
							else
								prpNc = pastRoute->back();

							// Check dynamic droplet rules so this and last droplet locations don't interfere
							if (prp && doesInterfere(rp, prp) && !(doesInterfere(rp, destPt) && prp->dStatus == DROP_WAIT))
							{
								isInterference = true;
								break;
							}
							if (prpLc && doesInterfere(rp, prpLc) && !(doesInterfere(rp, destPt) && prpLc->dStatus == DROP_WAIT))
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
						for (int m = 0; m < numStallsToPrepend; m++)
							subRoute->insert(subRoute->begin(), NULL);
						isInterference = false;
						j = 0;
					}
					else
						j++;
				}
			}	//finish compaction for timing constraint
			else
				continue;
			//} //finsih routing droplets

			delete routingThisTS;
			delete sourceCells;
			delete targetCells;

			//route for fault tolerance
			// Gather the source and destination cells of each droplet
			sourceCells = new map<Droplet *, BoardCell1 *>();
			targetCells = new map<Droplet *, BoardCell1 *>();
			routingThisTS = new vector<Droplet *>();

			compactorGetSourcesTargets(subDrops, subRoutes, sourceCells, targetCells);

			s = NULL;
			t = NULL;

			if(subRoutes->at(di)->size() == 0)
				continue;

			routeComplete = false;
			noRoute = false;
			nodesVisited = 0;
			numCells = 0;
			routeCycle = cycle; // DTG added for compaction
			Droplet *d_2 = subDrops->at(di);
			subDropsTC->push_back(d_2);
			vector<RoutePoint *> *sr_2 = new vector<RoutePoint *>();
			subRoutesTC->push_back(sr_2);

			//BoardCell pos;
			BoardCell1 hs2;
			hs2.x = subRoutes->at(di)->front()->x;
			hs2.y = subRoutes->at(di)->front()->y;
			s = &hs2;
			BoardCell1 ht2;
			ht2.x = subRoutes->at(di)->front()->x;
			ht2.y = subRoutes->at(di)->front()->y;
			t = &ht2;

			// reset board for new route.....
			for(unsigned int x = 0; x < ftgrid.size(); x++)
			{
				for(unsigned int y = 0; y < ftgrid[0].size(); y++)
				{
					ftgrid[x][y].x = x;
					ftgrid[x][y].y = y;
					ftgrid[x][y].type = Empty;
					ftgrid[x][y].distance = getInfinityVal();
					ftgrid[x][y].visited = false;
					ftgrid[x][y].prev = NULL;

					numCells++;
				}
			}
			// add blockages for modules on the board
			for(unsigned int i = 0; i < thisTS->size(); i++)
			{
				AssayNode *n = thisTS->at(i);
				if (!(n->GetType() == DISPENSE || n->GetType() == OUTPUT))
				{
					// Block all new reconfigurable areas from being used for routing
					ReconfigModule *rm = n->GetReconfigMod();
					if (n->startTimeStep < startingTS && n->endTimeStep > startingTS)
					{
						for (int x = rm->getLX()-1; x <= rm->getRX()+1; x++)
							for (int y = rm->getTY()-1; y <= rm->getBY()+1; y++)
								if (x >= 0 && y >= 0 && x < arch->getNumCellsX() && y < arch->getNumCellsY())
									ftgrid[x][y].type = Blocked;
					}
				}
			}
			// add blockages for other droplet sources/targets
			for(unsigned int j = 0; j < subDrops->size(); j++)
			{
				if(subRoutes->at(j)->size() == 0)
					continue;
				Droplet *d2 = subDrops->at(j);
				if(d != d2)	//if another droplet
				{
					BoardCell1 sc;
					sc.x = subRoutes->at(j)->front()->x;
					sc.y = subRoutes->at(j)->front()->y;
					BoardCell1 *c = &sc;		//add blockage for sources
					if( c != s ) 		// Don't block its own source
					{
						for(int x = c->x-1; x <= c->x+1; x++)
						{
							for(int y = c->y-1; y <= c->y+1; y++)
							{
								if(x >= 0 && y >= 0 && x < arch->getNumCellsX() && y < arch->getNumCellsY()) // must be on the board
								{
									ftgrid[x][y].type = Blocked;
								}
							}
						}
					}

					BoardCell1 tc;
					tc.x = subRoutes->at(j)->front()->x;
					tc.y = subRoutes->at(j)->front()->y;
					c = &tc;	//add blockage for targets
					if(c != t)			//don't block its own target
					{
						for(int x = c->x-1; x <= c->x+1; x++)
						{
							for(int y = c->y-1; y <= c->y+1; y++)
							{
								if(x >= 0 && y >= 0 && x < arch->getNumCellsX() && y < arch->getNumCellsY()) // must be on the board
									ftgrid[x][y].type = Blocked;
							}
						}
					}
				}
			}	//finish blockages from other droplet sources/targets

			if(s != t)		//if not already at target, route it
			{
				//set source as initial node (distance = 0)
				ftgrid[s->x][s->y].distance = 0;

				//in while loop, construct the shortest path
				while(!routeComplete && !noRoute)
				{
					//find node with shortest distance
					int minX = 0;
					int minY = 0;
					int minDist = getInfinityVal();
					for(int x = 0; x < arch->getNumCellsX(); x++)
					{
						for (int y = 0; y < arch->getNumCellsY(); y++)
						{
							if(ftgrid[x][y].visited == false && ftgrid[x][y].type == Empty)
							{
								if(ftgrid[x][y].distance < minDist)
								{
									minX = x;
									minY = y;
									minDist = ftgrid[x][y].distance;
								}
							}
						}
					}

					//for node, update all unvisited neighbors if is a shorter path
					if(minX-1 >= 0 && minX-1 < arch->getNumCellsX() )
					{
						//if cell to west is closer after the current cell is found, update the cell
						if(ftgrid[minX-1][minY].visited == false && ftgrid[minX-1][minY].distance > ftgrid[minX][minY].distance + ftgrid[minX-1][minY].occupation
								&& ftgrid[minX-1][minY].type == Empty)
						{
							ftgrid[minX-1][minY].distance = ftgrid[minX][minY].distance + ftgrid[minX-1][minY].occupation;
							ftgrid[minX-1][minY].prev = &ftgrid[minX][minY];
						}
					}
					if(minY+1 >= 0 && minY+1 < arch->getNumCellsY() )
					{
						//if cell to north is closer after the current cell is found, update the cell
						if(ftgrid[minX][minY+1].visited == false && ftgrid[minX][minY+1].distance > ftgrid[minX][minY].distance + ftgrid[minX][minY+1].occupation
								&& ftgrid[minX][minY+1].type == Empty)
						{
							ftgrid[minX][minY+1].distance = ftgrid[minX][minY].distance + ftgrid[minX][minY+1].occupation;
							ftgrid[minX][minY+1].prev = &ftgrid[minX][minY];
						}
					}
					if(minX+1 >= 0 && minX+1 < arch->getNumCellsX() )
					{
						//if cell to east is closer after the current cell is found, update the cell
						if(ftgrid[minX+1][minY].visited == false && ftgrid[minX+1][minY].distance > ftgrid[minX][minY].distance + ftgrid[minX+1][minY].occupation
								&& ftgrid[minX+1][minY].type == Empty)
						{
							ftgrid[minX+1][minY].distance = ftgrid[minX][minY].distance + ftgrid[minX+1][minY].occupation;
							ftgrid[minX+1][minY].prev = &ftgrid[minX][minY];
						}
					}
					if(minY-1 >= 0 && minY-1 < arch->getNumCellsY() )
					{
						//if cell to south is closer after the current cell is found, update the cell
						if(ftgrid[minX][minY-1].visited == false && ftgrid[minX][minY-1].distance > ftgrid[minX][minY-1].distance + ftgrid[minX][minY-1].occupation
								&& ftgrid[minX][minY-1].type == Empty)
						{
							ftgrid[minX][minY-1].distance = ftgrid[minX][minY].distance + ftgrid[minX][minY-1].occupation;
							ftgrid[minX][minY-1].prev = &ftgrid[minX][minY];
						}
					}

					//set current node as visited
					ftgrid[minX][minY].visited = true;
					nodesVisited++;

					if(minX == t->x && minY == t->y)	//have reached target
					{
						routeComplete = true;

						//use target's previous node to find the shortest path
						BoardCell2 * pos = &ftgrid[t->x][t->y];
						stack<BoardCell2 *> tempS;
						int tcRLength = 0;
						while(pos != NULL)		//put nodes in order of visitation
						{
							tempS.push(pos);
							pos = pos->prev;
							tcRLength++;
						}
						int ind = 0;
						while(!tempS.empty())	//load route into the subroutes structure
						{
							BoardCell2 * pos2;
							pos2 = tempS.top();
							RoutePoint *rp2 = new RoutePoint();
							rp2->cycle = routeCycle++;
							rp2->dStatus = DROP_NORMAL;
							rp2->x = pos2->x;
							rp2->y = pos2->y;
							sr_2->push_back(rp2);
							tempS.pop();
							if(ind != 0)
							{
								subRoutes->at(di)->at(ind) = rp2;
							}
							//since using route, add to ft grid to update occupancy
							if(ftgrid[pos2->x][pos2->y].occupation == 4)
								ftgrid[pos2->x][pos2->y].occupation = 3;
							else if(ftgrid[pos2->x][pos2->y].occupation == 3)
								ftgrid[pos2->x][pos2->y].occupation = 1;

							ind++;
						}
					}
					if(nodesVisited >= numCells)		//no route was found
					{
						noRoute = true;
					}

				}	//end construct shortest path while loop
			}	//finish routing current droplet
			//after routed, compact droplet
			// Ensure that all routes' last point is labeled appropriately
			for (unsigned int i = 0; i < subRoutes->size(); i++)
				if (subRoutes->at(i)->size() > 0)
					subRoutes->at(i)->back()->dStatus = DROP_WAIT;

			int longestRoute = 0;
			if (subRoutes->size() > 0)
				longestRoute = subRoutes->at(0)->size();

			int numStallsToPrepend = 1;

			vector<RoutePoint *> *subRoute = subRoutesTC->at(di);

			RoutePoint *destPt = NULL;
			if (subRoute->size() > 0)
				destPt = subRoute->back();

			// Check entire route
			bool isInterference = false;
			unsigned int j = 0; // The index used to traverse a specific route/cycle

			while (j != max((int)subRoute->size(), longestRoute) && subRoute->size() > 0)
			{
				RoutePoint *rp = NULL;
				if (j <= subRoute->size()-1)
					rp = subRoute->at(j);
				else
					rp = subRoute->back();

				if (rp)
				{
					// Check against the previous routes that have been compacted
					for(unsigned int k = 0; k < di; k++)
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
						else
							prp = pastRoute->back();

						RoutePoint *prpLc = NULL; // Past route's last cycle
						if (j > 0 && j <= pastRoute->size()-1)
							prpLc = pastRoute->at(j-1);
						else
							prpLc = pastRoute->back();

						RoutePoint *prpNc = NULL; // Past route's next cycle
						if (j+1 <= pastRoute->size()-1)
							prpNc = pastRoute->at(j+1);
						else
							prpNc = pastRoute->back();

						// Check dynamic droplet rules so this and last droplet locations don't interfere
						if (prp && doesInterfere(rp, prp) && !(doesInterfere(rp, destPt) && prp->dStatus == DROP_WAIT))
						{
							isInterference = true;
							break;
						}
						if (prpLc && doesInterfere(rp, prpLc) && !(doesInterfere(rp, destPt) && prpLc->dStatus == DROP_WAIT))
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
					for (int m = 0; m < numStallsToPrepend; m++)
						subRoute->insert(subRoute->begin(), NULL);
					isInterference = false;
					j = 0;
				}
				else
					j++;
			}

		} //finsih routing droplets

		delete routingThisTS;
		delete sourceCells;
		delete targetCells;
		//} //finish routing for all droplets
	}//finish while loop, compaction complete

	//final stage compaction
	int longestRoute = 0;
	if (subRoutes->size() > 0)
		longestRoute = subRoutes->at(0)->size();

	int numStallsToPrepend = 2;
	for(unsigned int i = 0; i < subRoutes->size(); i++)
	{
		vector<RoutePoint *> *subRoute = subRoutes->at(i);
		RoutePoint *destPt = NULL;
		if (subRoute->size() > 0)
			destPt = subRoute->back();

		// Check entire route
		bool isInterference = false;
		int isInterfereEnd = 0;
		unsigned int j = 0; // The index used to traverse a specific route/cycle

		while (j != max((int)subRoute->size(), longestRoute) && subRoute->size() > 0)
		{
			RoutePoint *rp = NULL;
			if (j <= subRoute->size()-1)
				rp = subRoute->at(j);
			else
				rp = subRoute->back();

			if (rp)
			{
				// Check against the previous routes that have been compacted
				for(unsigned int k = 0; k < i; k++)
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
					else
						prp = pastRoute->back();

					RoutePoint *prpLc = NULL; // Past route's last cycle
					if (j > 0 && j <= pastRoute->size()-1)
						prpLc = pastRoute->at(j-1);
					else
						prpLc = pastRoute->back();

					RoutePoint *prpNc = NULL; // Past route's next cycle
					if (j+1 <= pastRoute->size()-1)
						prpNc = pastRoute->at(j+1);
					else
						prpNc = pastRoute->back();

					// Check dynamic droplet rules so this and last droplet locations don't interfere
					if (prp && doesInterfere(rp, prp) && !(doesInterfere(rp, destPt) && prp->dStatus == DROP_WAIT))
					{
						isInterference = true;
						break;
					}
					if (prpLc && doesInterfere(rp, prpLc) && !(doesInterfere(rp, destPt) && prpLc->dStatus == DROP_WAIT))
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
					if(rpNc && prpLc && doesInterfere(prpLc, rpNc) && doesInterfere(prpLc, destPt) && !(doesInterfere(rpNc, destPt) && prpLc->dStatus == DROP_WAIT))
					{
						isInterfereEnd = 1;
						break;
					}
				}
			}
			if (isInterference || isInterfereEnd == 1)
			{	// Add a few stalls at the beginning and try again
				if(isInterference)
				{
					for (int m = 0; m < numStallsToPrepend; m++)

					{
						subRoute->insert(subRoute->begin(), NULL);
					}
				}
				else
				{
					subRoute->insert(subRoute->begin(), NULL);
				}
				isInterference = false;
				isInterfereEnd = 2;
				j = 0;
			}
			else
				j++;
		}
		if (j > longestRoute)
			longestRoute = j;
	}

}

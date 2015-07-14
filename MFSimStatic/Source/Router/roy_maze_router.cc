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
 * Source: roy_maze_router.cc													*
 * Original Code Author(s): Dan Grissom, Robert Doherty							*
 * Original Completion/Release Date: October 7, 2012							*
 *																				*
 * Details: N/A																	*
 *																				*
 * Revision History:															*
 * WHO		WHEN		WHAT													*
 * ---		----		----													*
 * FML		MM/DD/YY	One-line description									*
 *-----------------------------------------------------------------------------*/
#include "../../Headers/Router/roy_maze_router.h"

///////////////////////////////////////////////////////////////////////////////////
// Constructors
///////////////////////////////////////////////////////////////////////////////////
RoyMazeRouter::RoyMazeRouter()
{
	board = NULL;
	claim(false, "Invalid constructor used for Router variant.  Must use form that accepts DmfbArch.\n");
}
RoyMazeRouter::RoyMazeRouter(DmfbArch *dmfbArch)
{
	board = NULL;
	arch = dmfbArch;
}

///////////////////////////////////////////////////////////////////////////////////
// Deconstructor
///////////////////////////////////////////////////////////////////////////////////
RoyMazeRouter::~RoyMazeRouter()
{
	if (board)
	{
		while (!board->empty())
		{
			vector<SoukupCell*> *v = board->back();
			board->pop_back();
			while (!v->empty())
			{
				SoukupCell *c = v->back();
				v->pop_back();
				delete c;
			}
			delete v;
		}
		delete board;
	}
}

///////////////////////////////////////////////////////////////////////////////////
// Initializes the board full of soukop cells
///////////////////////////////////////////////////////////////////////////////////
void RoyMazeRouter::routerSpecificInits()
{
	// Create a 2D-array of Soukup cells
	board = new vector<vector<SoukupCell *> *>();
	for (int x = 0; x < arch->getNumCellsX(); x++)
	{
		vector<SoukupCell *> *col = new vector<SoukupCell *>();
		for (int y = 0; y < arch->getNumCellsY(); y++)
		{
			SoukupCell *c = new SoukupCell();
			c->C = NotReached;
			c->S = OtherLayer; // What to initialize to?
			c->x = x;
			c->y = y;
			c->trace = NULL;
			col->push_back(c);
		}
		board->push_back(col);
	}
}

///////////////////////////////////////////////////////////////////////////////////
// Returns true if moving from curr to next will get a droplet closer to target.
///////////////////////////////////////////////////////////////////////////////////
bool RoyMazeRouter::towardTarget(SoukupCell *curr, SoukupCell *next, SoukupCell *target)
{
	if(abs(target->x - next->x) < abs(target->x - curr->x))
		return true;
	else if(abs(target->y - next->y) < abs(target->y - curr->y) )
		return true;
	else
		return false;
}

///////////////////////////////////////////////////////////////////////////////////
// Tells the direction that needs to be traveled when tracing back from next
// to curr (the opposite direction traveled when going from curr to next)
///////////////////////////////////////////////////////////////////////////////////
int RoyMazeRouter::getTraceBackDir(SoukupCell* curr, SoukupCell* next) {
	if(curr->x > next->x)
		return RightT; // Right
	else if(curr->x < next->x)
		return LeftT; // Left
	else if(curr->y > next->y)
		return DownT; // Down (grid Y-values are flipped
	else if(curr->y < next->y)
		return UpT; // Up
	else
		return OtherLayer;// Just some value
}

///////////////////////////////////////////////////////////////////////////////////
// Prints out the blockages, as well as Source/target for the soukup board
///////////////////////////////////////////////////////////////////////////////////
void RoyMazeRouter::debugPrintSoukupBoard()
{
	cout << "Soukup Board Status:" << endl;

	// Print X-coordinates
	cout << "\\ ";
	for (int x = 0; x < arch->getNumCellsX(); x++)
		cout << x % 10 << " ";
	cout << endl;

	for (int y = 0; y < arch->getNumCellsY(); y++)
	{
		for (int x = 0; x < arch->getNumCellsX(); x++)
		{
			// Print Y-coordinates
			if (x == 0)
				cout << y % 10;

			if (board->at(x)->at(y)->S == Blockage)
				cout << " B";
			else if (board->at(x)->at(y)->S == StartPoint)
				cout << " S";
			else if (board->at(x)->at(y)->S == TargetPoint)
				cout << " T";
			else
				cout << " -";
		}
		cout << endl;
	}
	cout << endl;
}

///////////////////////////////////////////////////////////////////////////////////
// Prints out source-target pair for the given droplet.
///////////////////////////////////////////////////////////////////////////////////
void RoyMazeRouter::debugPrintSourceTargetPair(Droplet *d, map<Droplet *, SoukupCell *> *sourceCells, map<Droplet *, SoukupCell *> *targetCells)
{
	SoukupCell *s = sourceCells->at(d);
	SoukupCell *t = targetCells->at(d);
	cout << "S/T Pair: Route d" << d->getId() << " (" << s->x << ", " << s->y << ")-->(" << t->x << ", " << t->y << ")" << endl;
}

///////////////////////////////////////////////////////////////////////////////////
// Computes the individual subroutes for a sub-problem. A new sub-route is created
// for each sub-route and added to subRoutes; also, the corresponding droplet is
// added to subDrops (corresponding routes and droplets must share the same index
// in subRoutes and subDrops).
//
// This algorithm uses Soukup's maze routing algorithm to compute individual paths.
//
// See PostSubproblemCompactRouter::computeIndivSupProbRoutes() for more details.
///////////////////////////////////////////////////////////////////////////////////
void RoyMazeRouter::computeIndivSupProbRoutes(vector<vector<RoutePoint *> *> *subRoutes, vector<Droplet *> *subDrops, map<Droplet *, vector<RoutePoint *> *> *routes)
{
	bool debugPrint = false;
	//eliminateSubRouteDependencies(routes); // Optional; ensures that no source is in the IR of a target (moves the source out of way)

	// Get the nodes that need to be routed and sort
	vector<AssayNode *> routableThisTS;
	for (unsigned i = 0; i < thisTS->size(); i++)
		if (thisTS->at(i)->GetType() != DISPENSE && thisTS->at(i)->GetStartTS() == startingTS)
			routableThisTS.push_back(thisTS->at(i));

	// Gather the source and destination cells of each routable droplet this TS
	// For now, assume a non-io source is at the location of its last route point; non-io destination is bottom-left
	map<Droplet *, SoukupCell *> *sourceCells = new map<Droplet *, SoukupCell *>();
	map<Droplet *, SoukupCell *> *targetCells = new map<Droplet *, SoukupCell *>();
	set<Droplet *> dropsBeingOutput;
	set<ReconfigModule *> storageModsWithMultDrops; // Storage modules with multiple droplets
	vector<Droplet *> *routingThisTS = new vector<Droplet *>();
	SoukupCell *s = NULL;
	SoukupCell *t = NULL;
	for (unsigned i = 0; i < routableThisTS.size(); i++)
	{
		AssayNode *n = routableThisTS.at(i);
		for (unsigned p = 0; p < n->GetParents().size(); p++)
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
			sourceCells->insert(pair<Droplet *, SoukupCell *>(pd, s));

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
				if (n->GetType() == STORAGE && n->GetReconfigMod()->getNumDrops() == 2)
				{
					t = board->at(n->GetReconfigMod()->getLX())->at(n->GetReconfigMod()->getTY()); // Top-Left if second Storage drop
					storageModsWithMultDrops.insert(n->GetReconfigMod());
				}
				else if (n->GetType() == STORAGE && n->GetReconfigMod()->getNumDrops() == 3)
				{
					t = board->at(n->GetReconfigMod()->getRX())->at(n->GetReconfigMod()->getBY()); // Bottom-right if third Storage drop
					storageModsWithMultDrops.insert(n->GetReconfigMod());
				}
				else if (n->GetType() == STORAGE && n->GetReconfigMod()->getNumDrops() == 4)
				{
					t = board->at(n->GetReconfigMod()->getRX())->at(n->GetReconfigMod()->getTY()); // Top-right if fourth Storage drop
					storageModsWithMultDrops.insert(n->GetReconfigMod());
				}
				else if (n->GetType() == STORAGE && n->GetReconfigMod()->getNumDrops() > 4)
					claim(false, "Roy Router is not currently designed to handle more than 4 storage droplets per module. Please reduce the number of storage drops per module or update the Roy Router source code.");
				else
					t = board->at(n->GetReconfigMod()->getLX())->at(n->GetReconfigMod()->getBY()); // Bottom-Left, else
			}
			targetCells->insert(pair<Droplet *, SoukupCell *>(pd, t));

			//debugPrintSourceTargetPair(pd, sourceCells, targetCells);
		}
	}

	// Check the droplets in "storage modules with multiple droplets"
	// Ensures that droplets if one of the droplet's last routing points
	// is the other's target, that they switch targets so the droplet(s)
	// already at a valid destination does not have to move.
	set<ReconfigModule *>::iterator modIt = storageModsWithMultDrops.begin();
	for (; modIt != storageModsWithMultDrops.end(); modIt++)
	{
		ReconfigModule *rm = *modIt;

		// Now we know what module has 2+ droplets, need to search possible
		// nodes to find droplets
		vector<Droplet *> moduleDroplets;
		for (unsigned i = 0; i < routableThisTS.size(); i++)
			if (routableThisTS.at(i)->GetReconfigMod() == rm)
				moduleDroplets.push_back(routableThisTS.at(i)->GetDroplets().front());

		// Now, examine these droplets - brute force fine since number is low
		for (unsigned i = 0; i < moduleDroplets.size()/2+1; i++)
		{
			for (unsigned j = i+1; j < moduleDroplets.size(); j++)
			{
				Droplet *d1 = moduleDroplets.at(i);
				Droplet *d2 = moduleDroplets.at(j);
				RoutePoint *rp1 = (*routes)[d1]->back(); // Last rp for d1
				RoutePoint *rp2 = (*routes)[d2]->back(); // Last rp for d2
				SoukupCell *t1 = targetCells->at(d1);
				SoukupCell *t2 = targetCells->at(d2);

				if ((rp1->x == t2->x && rp1->y == t2->y) || (rp2->x == t1->x && rp2->y == t1->y))
				{
					targetCells->erase(d1);
					targetCells->erase(d2);
					targetCells->insert(pair<Droplet *, SoukupCell *>(d1, t2));
					targetCells->insert(pair<Droplet *, SoukupCell *>(d2, t1));
				}

			}
		}
	}

	// Now, sort in decreasing order of manhattan distance
	Sort::sortDropletsInDecManhattanDist(routingThisTS, sourceCells, targetCells);

	// Debug Print
	if (debugPrint)
	{
		cout << "Printing the source/target pairs for the sub-problem preceding TS " << startingTS << ": " << endl;
		for (unsigned i = 0; i < routingThisTS->size(); i++)
			debugPrintSourceTargetPair(routingThisTS->at(i), sourceCells, targetCells);
	}

	// Now, do Soukop's algorithm on each droplet
	for (unsigned i = 0; i < routingThisTS->size(); i++)
	{
		routeCycle = cycle; // DTG added for compaction
		Droplet *d = routingThisTS->at(i);
		subDrops->push_back(d);
		vector<RoutePoint *> *sr = new vector<RoutePoint *>();
		subRoutes->push_back(sr);

		s = sourceCells->at(d);
		t = targetCells->at(d);

		if (s != t)
		{
			// If the main route's container is empty, then add initial input location point
			if ((*routes)[d]->empty())
			{
				RoutePoint *rp = new RoutePoint();
				rp->cycle = routeCycle++;
				rp->dStatus = DROP_NORMAL;
				rp->x = s->x;
				rp->y = s->y;
				sr->push_back(rp);
			}

			// First, reset board for new route.....
			for (int x = 0; x < arch->getNumCellsX(); x++)
			{
				for (int y = 0; y < arch->getNumCellsY(); y++)
				{
					SoukupCell *c = board->at(x)->at(y);
					c->C = NotReached;
					c->S = OtherLayer; // What to initialize with?
					c->trace = NULL;
				}
			}
			// .....then add blockages for modules .....
			//persistingModules = new vector <ReconfigModule *>();
			for (unsigned i = 0; i < thisTS->size(); i++)
			{
				AssayNode *n = thisTS->at(i);
				if (!(n->GetType() == DISPENSE || n->GetType() == OUTPUT))
				{	// Block all new reconfigurable areas from being used for routing
					ReconfigModule *rm = n->GetReconfigMod();
					if (n->startTimeStep < startingTS && n->endTimeStep > startingTS)
					{
						//persistingModules->push_back(rm);
						//for (int j = 0; j < persistingModules->size(); j++)
						//{
						//	ReconfigModule *pm = persistingModules->at(j);
						for (int x = rm->getLX()-1; x <= rm->getRX()+1; x++)
							for (int y = rm->getTY()-1; y <= rm->getBY()+1; y++)
								if (x >= 0 && y >= 0 && x < arch->getNumCellsX() && y < arch->getNumCellsY())
									board->at(x)->at(y)->S = 7;
						//}
					}
				}
			}
			// .....then add blockages for other droplet sources/targets.....
			for (unsigned j = 0; j < routingThisTS->size(); j++)
			{
				Droplet *d2 = routingThisTS->at(j);
				if (d != d2)
				{
					SoukupCell *c = sourceCells->at(d2);
					//SoukupCell *c2 = targetCells->at(d2);
					if ( c != s ) // Don't block self source
						for (int x = c->x-1; x <= c->x+1; x++)
							for (int y = c->y-1; y <= c->y+1; y++)
								if (x >= 0 && y >= 0 && x < arch->getNumCellsX() && y < arch->getNumCellsY()) // On board?
									//if ( !(abs(x - t->x) <= 1 && abs(y - t->y) <= 1) ) // Don't block own target
									if (!(*routes)[d2]->empty()) // Don't mark as blockage for dispense droplets; they wait in reservoir
										board->at(x)->at(y)->S = Blockage;
					/*for (int x = c->x-1; x <= c->x+1; x++)
						for (int y = c->y-1; y <= c->y+1; y++)
							if (x >= 0 && y >= 0 && x < arch->getNumCellsX() && y < arch->getNumCellsY()) // On board?
								if ( !(abs(x - s->x) <= 1 && abs(y - s->y) <= 1) ) // Don't block self
									if ( !(abs(x - t->x) <= 1 && abs(y - t->y) <= 1) ) // Don't block self
										if (!(*routes)[d2]->empty()) // Don't mark as blockage for dispense droplets; they wait in reservoir
											board->at(x)->at(y)->S = Blockage;*/

					c = targetCells->at(d2);
					if (c != t)
						for (int x = c->x-1; x <= c->x+1; x++)
							for (int y = c->y-1; y <= c->y+1; y++)
								if (x >= 0 && y >= 0 && x < arch->getNumCellsX() && y < arch->getNumCellsY()) // On board?
									//if ( !(abs(x - t->x) <= 1 && abs(y - t->y) <= 1) ) // Don't block self's target
									//if ( !(abs(x - s->x) <= 1 && abs(y - s->y) <= 1) ) // Don't block self's source
									if (dropsBeingOutput.count(d2) == 0) // Don't mark as blockage for output dorplets; they go into reservoir
										board->at(x)->at(y)->S = Blockage;

				}
			}
			// ....then, ensure own source and target are not considered blockages
			/*for (int x = s->x-1; x <= s->x+1; x++)
				for (int y = s->y-1; y <= s->y+1; y++)
					if (x >= 0 && y >= 0 && x < arch->getNumCellsX() && y < arch->getNumCellsY() && board->at(x)->at(y)->S != StartPoint) // On board?
							board->at(x)->at(y)->S = OtherLayer;*/
			for (int x = t->x-1; x <= t->x+1; x++)
				for (int y = t->y-1; y <= t->y+1; y++)
					if (x >= 0 && y >= 0 && x < arch->getNumCellsX() && y < arch->getNumCellsY() && board->at(x)->at(y)->S != TargetPoint) // On board?
						board->at(x)->at(y)->S = OtherLayer;



			// ...and Finally, wet appropriate info for own source/target
			s->S = StartPoint;
			t->S = TargetPoint;

			if (debugPrint)
				debugPrintSoukupBoard();

			// Now that the board has been initialized, do Soukup's Algorithm (Robert's code)
			stack<SoukupCell*> RN; // new reach set
			stack<SoukupCell*> RO; // old reach set
			RN.push(s);	// push start onto new reach set
			s->C = LineReached; // set cell C to 2 (reached line search)


			int Rn = 0;
			int Ro = 0;
			bool routeFound = false;
			while( !RN.empty() && !routeFound)
			{
				Rn++;
				// 2 - move RN into RO and reverse order; basically just recreating order of RN in RO?  If so, same as popping to a temp stack and then popping to RO
				stack<SoukupCell *> tmpStack;
				while (!RN.empty())
				{
					tmpStack.push(RN.top());
					RN.pop();
				}
				while (!tmpStack.empty())
				{
					RO.push(tmpStack.top());
					tmpStack.pop();
				}

				//RN.empty();	// empty RN


				while( !RO.empty() && !routeFound)
				{
					Ro++;
					SoukupCell *currCell = RO.top();
					SoukupCell *preNeighborCurr = currCell;
					RO.pop();

					// 3 - find neighbors
					vector<SoukupCell*> neighbors;
					//= currCell->getNeighbors( 0, arch->getNumCellsX() - 1, 0, arch->getNumCellsY() - 1, &board);
					int cx = currCell->x; // Current x,y
					int cy = currCell->y;
					if (cx+1 >= 0 && cx+1 < arch->getNumCellsX() && cy >= 0 && cy < arch->getNumCellsY()) // Right neighbor
						neighbors.push_back(board->at(cx+1)->at(cy));
					if (cx-1 >= 0 && cx-1 < arch->getNumCellsX() && cy >= 0 && cy < arch->getNumCellsY()) // Left neighbor
						neighbors.push_back(board->at(cx-1)->at(cy));
					if (cx >= 0 && cx < arch->getNumCellsX() && cy+1 >= 0 && cy+1 < arch->getNumCellsY()) // Bottom neighbor
						neighbors.push_back(board->at(cx)->at(cy+1));
					if (cx >= 0 && cx < arch->getNumCellsX() && cy-1 >= 0 && cy-1 < arch->getNumCellsY()) // Top neighbor
						neighbors.push_back(board->at(cx)->at(cy-1));


					bool goto3 = false;
					// Check all neighbors' C and S values
					for(unsigned int k = 0; k < neighbors.size(); k++)
					{
						if (goto3)
							break;
						currCell = preNeighborCurr;
						//cout << "Neighbors" << endl;
						SoukupCell *nc = neighbors.at(k);
						if(nc->C == LineReached || nc->C == LeeReached || nc->S == Blockage)
						{ // C = 2 or S = 7
							// do nothing
						}
						else if(nc->S == TargetPoint)
						{	// S = 6
							// go to 8 - connection has been found; trace back to starting point through S

							// Reconstructs route and adds to sub-routes
							SoukupCell* trace = currCell;	// working Cell
							vector< SoukupCell* > tracePath; // return vector of int pairs for coordinates
							tracePath.push_back(nc);

							while( trace->S != StartPoint )
							{ // not the start cell
								tracePath.push_back(trace);

								if( trace->S == UpT ) // UP
									trace = board->at(trace->x)->at(trace->y-1);
								else if( trace->S == DownT ) // DOWN
									trace = board->at(trace->x)->at(trace->y+1);
								else if( trace->S == RightT ) // RIGHT
									trace = board->at(trace->x+1)->at(trace->y);
								else if( trace->S == LeftT ) // LEFT
									trace = board->at(trace->x-1)->at(trace->y);
								else
								{
									cerr << "Bad Path" << endl;
									exit(1);
								}
							}

							while (!tracePath.empty())
							{
								RoutePoint* rp = new RoutePoint();
								rp->cycle = routeCycle++;
								rp->dStatus = DROP_NORMAL;
								rp->x = tracePath.back()->x;
								rp->y = tracePath.back()->y;
								sr->push_back(rp);
								tracePath.pop_back();
							}
							goto3 = true;
							routeFound = true;

							// Set last droplet as output if true
							if (dropsBeingOutput.count(d) > 0)
								sr->back()->dStatus = DROP_OUTPUT;
							//cout << "SOUKUP ROUTE FOUND." << endl;
							//return;
						}
						else if( nc->C <= 1 && towardTarget(currCell, nc, t))
						{ // C <= 1
							// 5 - move RN into RO  after last unused entry
							for( unsigned int l = 0; l < RN.size(); l++ )
							{
								RO.push(RN.top());
								RN.pop(); // Do we remove here???
							}

							// Push current cell onto stack in case need to examine final neighbors later
							RO.push(preNeighborCurr);

							bool goto6 = true;
							while( goto6 )
							{
								// 6
								RO.push(nc);
								nc->C = LineReached;
								if(nc->S <= 4)
									nc->S = getTraceBackDir(currCell, nc); // set S to traceback code


								// 7 - find next neighbor in same direction
								SoukupCell* next = NULL;
								int nextX;
								int nextY;
								if( nc->S == UpT ) // Traceback is up, means we just went DOWN
								{
									nextX = nc->x;
									nextY = nc->y+1;
								}
								else if( nc->S == DownT ) // Traceback is down, means we just went UP
								{
									nextX = nc->x;
									nextY = nc->y-1;
								}
								else if( nc->S == RightT ) // Traceback is right, means we just went LEFT
								{
									nextX = nc->x-1;
									nextY = nc->y;
								}
								else if( nc->S == LeftT ) // Traceback is left, means we just went RIGHT
								{
									nextX = nc->x+1;
									nextY = nc->y;
								}

								if (nextX >= 0 && nextX < arch->getNumCellsX() && nextY >= 0 && nextY < arch->getNumCellsY())
									next = board->at(nextX)->at(nextY); // Check the bounds and grab next if on board


								if( next == NULL )
								{
									goto6 = false;
									goto3 = true;
								}
								else
								{
									if( next->C == LineReached || next->S == Blockage )
									{
										goto6 = false;
										goto3 = true;
									}
									else if( next->S == TargetPoint )
									{
										// Reconstructs route and adds to sub-routes
										SoukupCell* trace = nc;	// working Cell
										vector< SoukupCell* > tracePath; // return vector of int pairs for coordinates
										tracePath.push_back(next);

										while( trace->S != StartPoint )
										{ // not the start cell
											tracePath.push_back(trace);

											if( trace->S == UpT ) // UP
												trace = board->at(trace->x)->at(trace->y-1);
											else if( trace->S == DownT ) // DOWN
												trace = board->at(trace->x)->at(trace->y+1);
											else if( trace->S == RightT ) // RIGHT
												trace = board->at(trace->x+1)->at(trace->y);
											else if( trace->S == LeftT ) // LEFT
												trace = board->at(trace->x-1)->at(trace->y);
											else
											{
												cerr << "Bad Path" << endl;
												exit(1);
											}
										}

										while (!tracePath.empty())
										{
											RoutePoint* rp = new RoutePoint();
											rp->cycle = routeCycle++;
											rp->dStatus = DROP_NORMAL;
											rp->x = tracePath.back()->x;
											rp->y = tracePath.back()->y;
											sr->push_back(rp);
											tracePath.pop_back();
										}
										goto6 = false;
										goto3 = true;
										routeFound = true;

										// Set last droplet as output if true
										if (dropsBeingOutput.count(d) > 0)
											sr->back()->dStatus = DROP_OUTPUT;

										//cout << "SOUKUP ROUTE FOUND." << endl;
									}
									else if(!towardTarget(nc, next, t)) //not closer to target
									{
										goto6 = false; // Goto 3 (don't iterate through rest of neighbors??)
										goto3 = true;
									}
									else // DTG added to keep route moving
									{
										currCell = nc;
										nc = next;
									}
								}
							} // while goto 6

						} // if
						else if(nc->C == NotReached)
						{ // C == 0
							nc->C = LeeReached;	// set C = 1
							RN.push(nc);	// push neighbor onto RN
							if(nc->S <= 4) // set S to trace back
								nc->S = getTraceBackDir(currCell, nc); // set S to traceback code
						}

					} // for check neighbors
					if( Ro > 100000 ) break;
				} // while RO not empty
				if( Rn > 100000 ) break;
			} // while RN not empty

			if (!routeFound)
			{
				stringstream msg;

				cout << "Printing the source/target pairs for the sub-problem preceding TS " << startingTS << ": " << endl;
				for (unsigned i = 0; i < routingThisTS->size(); i++)
					debugPrintSourceTargetPair(routingThisTS->at(i), sourceCells, targetCells);
				debugPrintSoukupBoard();

				msg << "TS " << startingTS << ": Roy Maze Router could not compute path for droplet " << d->getId();
				msg << " from (" << s->x << ", " << s->y << ") to (" << t->x << ", " << t->y << ")" << endl;

				claim(routeFound, &msg);
			}

		}
	}
	delete routingThisTS;
	delete sourceCells;
	delete targetCells;
}

/*
 * lee_router.cc
 *
 *  
 */

#include <vector>
#include <algorithm>
#include "../../Headers/Router/lee_router.h"
#include <set>


//int k_paths = 5;
const int BLOCK_VAL = 10000;

///////////////////////////////////////////////////////////////////////////////////
// Constructors
///////////////////////////////////////////////////////////////////////////////////
LeeRouter::LeeRouter()
{
	board = NULL;
	claim(false, "Invalid constructor used for Router variant.  Must use form that accepts DmfbArch.\n");
}
LeeRouter::LeeRouter(DmfbArch *dmfbArch)
{
	board = NULL;
	arch = dmfbArch;
}

///////////////////////////////////////////////////////////////////////////////////
// Deconstructor
///////////////////////////////////////////////////////////////////////////////////
LeeRouter::~LeeRouter()
{
	//cerr << "Destructor" << endl;
	if (board)
	{
		while (!board->empty())
		{
			vector<LeeCell*> *v = board->back();
			board->pop_back();
			while (!v->empty())
			{
				LeeCell *c = v->back();
				v->pop_back();
				delete c;
			}
			delete v;
		}
		delete board;
	}
}

///////////////////////////////////////////////////////////////////////////////////
// Initializes the board full of LeeCells instead of Soukup cells for Lee's
///////////////////////////////////////////////////////////////////////////////////
void LeeRouter::routerSpecificInits()
{
	// Create a 2D-array of Lee cells
	board = new vector<vector<LeeCell *> *>();
	for (int x = 0; x < arch->getNumCellsX(); x++)
	{
		vector<LeeCell *> *col = new vector<LeeCell *>();
		for (int y = 0; y < arch->getNumCellsY(); y++)
		{
			LeeCell *c = new LeeCell();
			c->val = 0;
			c->x = x;
			c->y = y;
			col->push_back(c);
		}
		board->push_back(col);
	}
}


///////////////////////////////////////////////////////////////////////////////////
//void LeeRouter::computeIndivSupProbRoutes(vector<vector<RoutePoint *> *> *subRoutes, vector<Droplet *> *subDrops, map<Droplet *, vector<RoutePoint *> *> *routes)
void LeeRouter::computeIndivSupProbRoutes(vector<vector<RoutePoint *> *> *subRoutes, vector<Droplet *> *subDrops, map<Droplet *, vector<RoutePoint *> *> *routes)
{
	// Get the nodes that need to be routed and sort

	//eliminateSubRouteDependencies(routes);
	vector<AssayNode *> routableThisTS;
	for (unsigned i = 0; i < thisTS->size(); i++)
		if (thisTS->at(i)->GetType() != DISPENSE && thisTS->at(i)->GetStartTS() == startingTS)
			routableThisTS.push_back(thisTS->at(i));

	// Gather the source and destination cells of each routable droplet this TS
	// For now, assume a non-io source is at the location of its last route point; non-io destination is bottom-left
	map<Droplet *, LeeCell *> *sourceCells = new map<Droplet *, LeeCell *>();
	map<Droplet *, LeeCell *> *targetCells = new map<Droplet *, LeeCell *>();
	set<ReconfigModule *> storageModsWithMultDrops;
	vector<Droplet *> *routingThisTS = new vector<Droplet *>();
	vector<LeeCell*> blockCells;
	LeeCell *s = NULL;
	LeeCell *t = NULL;
	//int score_temp = 0;
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
			sourceCells->insert(pair<Droplet *, LeeCell *>(pd, s));
			//blockCells.push_back(s);

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
			}
			else
			{
				if (n->GetType() == STORAGE && n->GetReconfigMod()->getNumDrops() > 1)
				{
					//cout << "Storage!"  << endl;
					t = board->at(n->GetReconfigMod()->getLX())->at(n->GetReconfigMod()->getTY()); // Top-Left if second Storage drop
					storageModsWithMultDrops.insert(n->GetReconfigMod());
				}
				else
					t = board->at(n->GetReconfigMod()->getLX())->at(n->GetReconfigMod()->getBY()); // Bottom-Left, else// DTG, this will need to be adjusted for storage etc., when/if more than one destination in a module
			}
			//t = board->at(n->GetReconfigMod()->getLX())->at(n->GetReconfigMod()->getBY()); // last route point
			targetCells->insert(pair<Droplet *, LeeCell *>(pd, t));
			//blockCells.push_back(t);
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
				LeeCell *t1 = targetCells->at(d1);
				LeeCell *t2 = targetCells->at(d2);

				if ((rp1->x == t2->x && rp1->y == t2->y) || (rp2->x == t1->x && rp2->y == t1->y))
				{
					cout << "Switched!"  << endl;
					targetCells->erase(d1);
					targetCells->erase(d2);
					targetCells->insert(pair<Droplet *, LeeCell *>(d1, t2));
					targetCells->insert(pair<Droplet *, LeeCell *>(d2, t1));
				}

			}
		}
	}



	// Now, do Lee's Algorithm
	for (unsigned i = 0; i < routingThisTS->size(); i++)
	{
		stack<LeeCell*> *stack_route = new stack<LeeCell*>();
		vector<stack <LeeCell> > list_of_routes;
		//routeCycle = cycle; // DTG added for compaction
		routeCycle = cycle;
		Droplet *d = routingThisTS->at(i);
		subDrops->push_back(d);
		vector<RoutePoint *> *sr = new vector<RoutePoint *>();
		//subRoutes->push_back(sr);
		s = sourceCells->at(d);
		s->val = 0;
		t = targetCells->at(d);


		if ((*routes)[d]->empty())
		{
			RoutePoint *rp = new RoutePoint();
			rp->cycle = routeCycle;
			routeCycle++;
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
				LeeCell *c = board->at(x)->at(y);
				c->x = x;
				c->y = y;
				c->val = 0;
				//c->C = NotReached;
				c->block = false; // What to initialize with?
				//c->trace = NULL;
			}
		}

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
								board->at(x)->at(y)->block = true;
					//}
				}
			}
		}


		//Blockages for other droplets
		for (unsigned j = 0; j < routingThisTS->size(); j++)
		{
			Droplet *d2 = routingThisTS->at(j);
			if (d != d2)
			{
				LeeCell *c = sourceCells->at(d2);
				for (int x = c->x-1; x <= c->x+1; x++)
					for (int y = c->y-1; y <= c->y+1; y++)
						if (x >= 0 && y >= 0 && x < arch->getNumCellsX() && y < arch->getNumCellsY()) // On board?
								//if ( !(abs(x - s->x) <= 1 && abs(y - s->y) <= 1) ) // Don't block self
									//if ( !(abs(x - t->x) <= 1 && abs(y - t->y) <= 1) ) // Don't block self
										if (!(*routes)[d2]->empty()) // Don't mark as blockage for dispense droplets; they wait in reservoir
										{
											//LeeCell * block_cell = new LeeCell();
											//block_cell->x = d2->
											//evaluated->push_back(*d2);
											//LeeCell* tempBlock = new LeeCell;
											//tempBlock->x = x;
											//tempBlock->y = y;
											//blockCells.push_back(tempBlock);
											board->at(x)->at(y)->block = true;
											//board->at(x)->at(y)->score = 100000;
										}

				c = targetCells->at(d2);
				for (int x = c->x-1; x <= c->x+1; x++)
					for (int y = c->y-1; y <= c->y+1; y++)
						if (x >= 0 && y >= 0 && x < arch->getNumCellsX() && y < arch->getNumCellsY())
						{// On board?
							//if ( !(abs(x - t->x) <= 1 && abs(y - t->y) <= 1) ) // Don't block self
								//if ( !(abs(x - s->x) <= 1 && abs(y - s->y) <= 1) ) // Don't block self
									//{
									//LeeCell* tempBlock = new LeeCell;
									//tempBlock->x = x;
									//tempBlock->y = y;
									//blockCells.push_back(tempBlock);
									//}
									board->at(x)->at(y)->block = true;
							//board->at(x)->at(y)->score = 100000;
							//evaluated->push_back(c);
							//blockCells.push_back(c);
						}
				//LeeCell * c = targetCells->at(d2);
				//blockCells.push_back(c);

			}
		}

		for (int x = t->x-1; x <= t->x+1; x++)
			for (int y = t->y-1; y <= t->y+1; y++)
				if (x >= 0 && y >= 0 && x < arch->getNumCellsX() && y < arch->getNumCellsY() && !(x == t->x && y == t->y)) // On board?
					board->at(x)->at(y)->block = false;


		mark_cells(sourceCells->at(d), targetCells->at(d)); //JUST MARK THE NEXT sourceCell THEN FIND ROUTE AND ADD TO ROUTE LIST.

		//LeeCell sourceCell = *(sourceCells->at(d));
		LeeCell *currCell = (targetCells->at(d));




		/*if(currCell->val == 0)
		{
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


											if (x == currCell->x && y == currCell->y)
												cout << " T";
											else if (x == sourceCell.x && y == sourceCell.y )
												cout << " S";
											else if (board->at(x)->at(y)->block)
													cout << " B";
											else
												cout << " -";
										}
										cout << endl;
									}
									cout << endl;

			cout << routeCycle << endl;
			exit(0);
		}*/

		if(currCell->val <0)
		{
			cerr << "Lee Router could not find a valid path at cycle "  << routeCycle << endl;
			exit(1);
		}

		while( currCell->val >= 0)
		{

			stack_route->push(currCell);
			//sr->push_back(rp);
			if(currCell->val == 0)// towardTarget(currCell, nc, t))
			{
				//stack_route.push(currCell);
				break;
			}
			// 3 - find neighbors
			vector<LeeCell*> neighbors;
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
			int flag = 0;
			for(unsigned int k = 0; k < neighbors.size(); k++)
			{
				//next_cell = neighbors[k];
				//cerr << next_cell->val << endl;
				//if(neighbors[k]->val == -1 && neighbors[k] != sourceCells->at(d))
				//{

				//}
				if(((currCell->val > neighbors[k]->val || neighbors[k]->val ==0) && (neighbors[k]->x >=0 && neighbors[k]->y >=0
						&& neighbors[k]->x < arch->getNumCellsX() && neighbors[k]->y < arch->getNumCellsY())) && neighbors[k]->val != -1 )
				{
					currCell = (neighbors[k]);
					flag = 1;
				}
			}
			neighbors.erase(neighbors.begin(), neighbors.end());
			if(flag == 0)
			{
				cerr <<"Lee router could not route at cycle " << routeCycle << endl;
				exit(1);
			}

		}
		//list_of_routes.push_back(stack_route);

		//int rand_path = 0;
		//stack <LeeCell> add_route = list_of_routes[rand_path];
		//cerr << "add_route size = " << add_route.size() << endl;

		//		cout << "Stack size = "  << stack_route->size() << endl;
		if(stack_route->size() == 0)
		{
			cerr << "Lee router could not find a valid path."  << endl;
			exit(1);
		}
		while(!stack_route->empty())
		{
			RoutePoint* rp = new RoutePoint();
			rp->cycle = routeCycle;
			routeCycle++;
			rp->dStatus = DROP_NORMAL;
			rp->x = stack_route->top()->x;
			rp->y = stack_route->top()->y;
			//cerr << "This is the next route point =  " << stack_route.top()->x << " , " << stack_route.top()->y << endl;
			sr->push_back(rp);
			stack_route->pop();
		}
		//cout << sr->back()->x << " "  << sr->back()->y << endl;
		subRoutes->push_back(sr);
	}

	//cerr << "world" << endl;
	delete routingThisTS;
	delete sourceCells;
	delete targetCells;
	//cerr << "function done" << endl;
}


//MARK CELLS FUNCTION

void LeeRouter::mark_cells(LeeCell * source, LeeCell * target)
{

	if(source->x == target->x && source->y == target->y)
		return;

	int i = 0;
	//int size;
	source->val = 0;
	LeeCell* currCell = source;
	vector<LeeCell* > neighbors;
	vector<LeeCell *> new_neighbors;


	//cout << "Source = "  << source->x << " " << source->y << endl;

	//Initially populate neighbors
	if (currCell->x+1 >= 0 && currCell->x+1 < arch->getNumCellsX() && currCell->y >= 0 && currCell->y < arch->getNumCellsY()) // Right neighbor
		neighbors.push_back(board->at(currCell->x+1)->at(currCell->y));
	if (currCell->x-1 >= 0 && currCell->x-1 < arch->getNumCellsX() && currCell->y >= 0 && currCell->y < arch->getNumCellsY()) // Left neighbor
		neighbors.push_back(board->at(currCell->x-1)->at(currCell->y));
	if (currCell->x >= 0 && currCell->x < arch->getNumCellsX() && currCell->y+1 >= 0 && currCell->y+1 < arch->getNumCellsY()) // Bottom neighbor
		neighbors.push_back(board->at(currCell->x)->at(currCell->y+1));
	if (currCell->x >= 0 && currCell->x < arch->getNumCellsX() && currCell->y-1 >= 0 && currCell->y-1 < arch->getNumCellsY()) // Top neighbor
		neighbors.push_back(board->at(currCell->x)->at(currCell->y-1));




	while(1)
	{


		++i;
		//Assign values to neighbors and adds them to new neighbors
		for (unsigned k = 0; k < neighbors.size(); ++k)
		{
			if(!neighbors[k]->block)// && neighbors[k]->val != 0)// && neighbors[k]->val == 0)
			{
				if((/*neighbors[k]->val > i ||*/ neighbors[k]->val >= 0))// && (!(neighbors[k]->x == source->x && neighbors[k]->y == source->y)))
				{
					neighbors[k]->val = i;
					if (neighbors[k]->x+1 >= 0 && neighbors[k]->x+1 < arch->getNumCellsX() && neighbors[k]->y >= 0 && neighbors[k]->y < arch->getNumCellsY() && board->at(neighbors[k]->x+1)->at(neighbors[k]->y)->val == 0 && (!(neighbors[k]->x + 1 == source->x && neighbors[k]->y == source->y))) // Right neighbor
					{
						board->at(neighbors[k]->x+1)->at(neighbors[k]->y)->val = i+1;
						new_neighbors.push_back(board->at(neighbors[k]->x+1)->at(neighbors[k]->y));
					}
					if (neighbors[k]->x-1 >= 0 && neighbors[k]->x-1 < arch->getNumCellsX() && neighbors[k]->y >= 0 && neighbors[k]->y < arch->getNumCellsY()&& board->at(neighbors[k]->x-1)->at(neighbors[k]->y)->val == 0 && (!(neighbors[k]->x -1 == source->x && neighbors[k]->y == source->y))) // Left neighbor
					{
						board->at(neighbors[k]->x-1)->at(neighbors[k]->y)->val = i+1;
						new_neighbors.push_back(board->at(neighbors[k]->x-1)->at(neighbors[k]->y));
					}
					if (neighbors[k]->x >= 0 && neighbors[k]->x < arch->getNumCellsX() && neighbors[k]->y+1 >= 0 && neighbors[k]->y+1 < arch->getNumCellsY()&& board->at(neighbors[k]->x)->at(neighbors[k]->y + 1)->val == 0 && (!(neighbors[k]->x == source->x && neighbors[k]->y+1 == source->y))) // Bottom neighbor
					{
						board->at(neighbors[k]->x)->at(neighbors[k]->y+1)->val = i+1;
						new_neighbors.push_back(board->at(neighbors[k]->x)->at(neighbors[k]->y+1));
					}
					if (neighbors[k]->x >= 0 && neighbors[k]->x < arch->getNumCellsX() && neighbors[k]->y-1 >= 0 && neighbors[k]->y-1 < arch->getNumCellsY()&& board->at(neighbors[k]->x)->at(neighbors[k]->y - 1)->val == 0 && (!(neighbors[k]->x == source->x && neighbors[k]->y-1 == source->y))) // Top neighbor
					{
						board->at(neighbors[k]->x)->at(neighbors[k]->y - 1)->val = i+1;
						new_neighbors.push_back(board->at(neighbors[k]->x)->at(neighbors[k]->y-1));
					}
					//new_neighbors.push_back(neighbors[k]);
				}
			}
			//do not add blocked neighbors to new neighbors list
			else if(neighbors[k]->block)
				neighbors[k]->val = -1;
			//currCell = neighbors[k];
			//neighbors.erase(neighbors.begin() + k);
		}


		if(new_neighbors.empty())
			break;



		for(int j = neighbors.size() - 1; j >= 0; --j)
		{
			//cout << "(" << neighbors[j]->x  << ", "  << neighbors[j]->y << ")" << "   val = "  << neighbors[j]->val << endl;
			neighbors.pop_back();
		}


		neighbors = new_neighbors;
		for(int j = new_neighbors.size() - 1; j >=0; j--)
			new_neighbors.pop_back();

	}

}




/*
 * A_Star_Router.h
 *
 *  Created on: Feb 26, 2013
 *      Author: Chris Jaress
 */


#include "../../Headers/Router/a_star_router.h"
#include <stack>
#include <set>



///////////////////////////////////////////////////////////////////////////////////
// Constructors
///////////////////////////////////////////////////////////////////////////////////
AStarRouter::AStarRouter()
{
	board = NULL;
	claim(false, "Invalid constructor used for Router variant.  Must use form that accepts DmfbArch.\n");
}
AStarRouter::AStarRouter(DmfbArch *dmfbArch)
{
	board = NULL;
	arch = dmfbArch;
}

void AStarRouter::printBlockages()
{
	cout << "---------------------------------------" << endl;
	for(int i = 0; i < arch->getNumCellsY(); ++i )
	{
		cout << i%10 << " ";
		for(int k = 0; k < arch->getNumCellsX(); ++k)
		{
			if(board->at(k)->at(i)->block)
				cout << " B ";
			else
				cout << " - ";
		}
		cout << endl;

	}
	cout << endl << "---------------------------------------------";
}

///////////////////////////////////////////////////////////////////////////////////
// Deconstructor
///////////////////////////////////////////////////////////////////////////////////
AStarRouter::~AStarRouter()
{
	//cerr << "Destructor" << endl;
	if (board)
	{
		while (!board->empty())
		{
			vector<StarCell*> *v = board->back();
			board->pop_back();
			while (!v->empty())
			{
				StarCell *c = v->back();
				v->pop_back();
				delete c;
			}
			delete v;
		}
		delete board;
	}
}

///////////////////////////////////////////////////////////////////////////////////
// Initializes the board full of StarCells instead of Soukup cells for Lee's
///////////////////////////////////////////////////////////////////////////////////
void AStarRouter::routerSpecificInits()
{
	// Create a 2D-array of Lee cells
	board = new vector<vector<StarCell *> *>();
	for (int x = 0; x < arch->getNumCellsX(); x++)
	{
		vector<StarCell *> *col = new vector<StarCell *>();
		for (int y = 0; y < arch->getNumCellsY(); y++)
		{
			StarCell *c = new StarCell();
			c->x = x;
			c->y = y;
			c->block = false;
			c->came_from = NULL;
			c->score = 0;
			col->push_back(c);
		}
		board->push_back(col);
	}
}

///////////////////////////////////////////////////////////////////////////////////
// Returns true if moving from curr to next will get a droplet closer to target.
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
// Tells the direction that needs to be traveled when tracing back from next
// to curr (the opposite direction traveled when going from curr to next)
//	DO NOT NEED FOR LEE'S
///////////////////////////////////////////////////////////////////////////////////
/*int LeeRouter::getTraceBackDir(StarCell* curr, StarCell* next) {
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

*/
///////////////////////////////////////////////////////////////////////////////////
//void LeeRouter::computeIndivSupProbRoutes(vector<vector<RoutePoint *> *> *subRoutes, vector<Droplet *> *subDrops, map<Droplet *, vector<RoutePoint *> *> *routes)
void AStarRouter::computeIndivSupProbRoutes(vector<vector<RoutePoint *> *> *subRoutes, vector<Droplet *> *subDrops, map<Droplet *, vector<RoutePoint *> *> *routes)
{
	// Get the nodes that need to be routed and sort
	//eliminateSubRouteDependencies(routes);
	vector<AssayNode *> routableThisTS;
	for (unsigned i = 0; i < thisTS->size(); i++)
		if (thisTS->at(i)->GetType() != DISPENSE && thisTS->at(i)->GetStartTS() == startingTS)
			routableThisTS.push_back(thisTS->at(i));

	// Gather the source and destination cells of each routable droplet this TS
	// For now, assume a non-io source is at the location of its last route point; non-io destination is bottom-left
	map<Droplet *, StarCell *> *sourceCells = new map<Droplet *, StarCell *>();
	map<Droplet *, StarCell *> *targetCells = new map<Droplet *, StarCell *>();
	set<ReconfigModule *> storageModsWithMultDrops;
	vector<Droplet *> *routingThisTS = new vector<Droplet *>();
	vector<StarCell*> blockCells;
	StarCell *s = NULL;
	StarCell *t = NULL;
	int score_temp = 0;
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
			sourceCells->insert(pair<Droplet *, StarCell *>(pd, s));
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
//				if(pd->getId() == 7)
//				{
//					cout << pd->getId() << endl;
//					cout << routeCycle << endl;
//					cout << n->GetReconfigMod()->getNumDrops() << endl;
//				}
//				else if(pd->getId() == 15)
//				{
//					cout << pd->getId() << endl;
//					cout << routeCycle << endl;
//					cout << n->GetReconfigMod()->getNumDrops() << endl;
//				}
					//cout << n->GetReconfigMod()->getNumDrops() << endl;
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
			targetCells->insert(pair<Droplet *, StarCell *>(pd, t));
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
					StarCell *t1 = targetCells->at(d1);
					StarCell *t2 = targetCells->at(d2);

					if ((rp1->x == t2->x && rp1->y == t2->y) || (rp2->x == t1->x && rp2->y == t1->y))
					{
						//cout << "Switched!"  << endl;
						targetCells->erase(d1);
						targetCells->erase(d2);
						targetCells->insert(pair<Droplet *, StarCell *>(d1, t2));
						targetCells->insert(pair<Droplet *, StarCell *>(d2, t1));
					}

				}
			}
		}




	for(unsigned i = 0; i < routingThisTS->size(); ++i)
	{
		vector<StarCell*> *open_set = new vector<StarCell*>();
		vector<StarCell* > *evaluated = new vector<StarCell*>();
		vector<RoutePoint *> *sr = new vector<RoutePoint *>();
		vector<StarCell*> neighbors;// = new vector<StarCell*>();
		score_temp = 0;
		routeCycle = cycle;
		//subRoutes->push_back(sr);


		//stack<StarCell*> stack_route;
		Droplet *d = routingThisTS->at(i);
		//cerr << "Got passed here!" << endl;
		subDrops->push_back(d);
		//vector<StarCell*> blockCells;

		//Loop used to block off cells that have a droplet in them



		s = sourceCells->at(d);
		s->score = 0;
		t = targetCells->at(d);

		if ((*routes)[d]->empty())
		{
			RoutePoint *rp = new RoutePoint();
			rp->cycle = routeCycle;
			//routeCycle++;
			rp->dStatus = DROP_NORMAL;
			rp->x = s->x;
			rp->y = s->y;
			sr->push_back(rp);
		}

		//Reset board for new route
		for (int x = 0; x < arch->getNumCellsX(); x++)
		{
			for (int y = 0; y < arch->getNumCellsY(); y++)
			{
				StarCell *c = board->at(x)->at(y);
				c->came_from = NULL;
				c->score = 0;
				c->block = false;
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
						StarCell *c = sourceCells->at(d2);
						for (int x = c->x-1; x <= c->x+1; x++)
							for (int y = c->y-1; y <= c->y+1; y++)
								if (x >= 0 && y >= 0 && x < arch->getNumCellsX() && y < arch->getNumCellsY()) // On board?
									//if ( !(abs(x - s->x) <= 1 && abs(y - s->y) <= 1) ) // Don't block self
										//if ( !(abs(x - t->x) <= 1 && abs(y - t->y) <= 1) ) // Don't block self
											if (!(*routes)[d2]->empty()) // Don't mark as blockage for dispense droplets; they wait in reservoir
											{
												//StarCell * block_cell = new StarCell();
												//block_cell->x = d2->
												//evaluated->push_back(*d2);
												//StarCell* tempBlock = new StarCell;
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
															//StarCell* tempBlock = new StarCell;
															//tempBlock->x = x;
															//tempBlock->y = y;
															//blockCells.push_back(tempBlock);
														//}
															board->at(x)->at(y)->block = true;
															//board->at(x)->at(y)->score = 100000;
															//evaluated->push_back(c);
															//blockCells.push_back(c);
									}
					//StarCell * c = targetCells->at(d2);
					//blockCells.push_back(c);

					}
					}

//		if(routeCycle >=2000 && routeCycle < 3000)// 2120)
//		{
//			cout << routeCycle << endl;
		//printBlockages();
//			//exit(1);
//		}


		//Blockages for modules

		StarCell * currCell = s;
		open_set->push_back(s);
		//while loop to go through A*
		while(!open_set->empty())
		{
			currCell = score_min(open_set);

			if(currCell == t)
			{
				sr = find_path(s,t);

			}
			remove_from_open_set(open_set, currCell);
			evaluated->push_back(currCell);
			int cx = currCell->x; // Current x,y
			int cy = currCell->y;
			//grab the neighbor cells to find best possible
			if (cx+1 >= 0 && cx+1 < arch->getNumCellsX() && cy >= 0 && cy < arch->getNumCellsY()) // Right neighbor
				neighbors.push_back(board->at(cx+1)->at(cy));
			if (cx-1 >= 0 && cx-1 < arch->getNumCellsX() && cy >= 0 && cy < arch->getNumCellsY()) // Left neighbor
				neighbors.push_back(board->at(cx-1)->at(cy));
			if (cx >= 0 && cx < arch->getNumCellsX() && cy+1 >= 0 && cy+1 < arch->getNumCellsY()) // Bottom neighbor
				neighbors.push_back(board->at(cx)->at(cy+1));
			if (cx >= 0 && cx < arch->getNumCellsX() && cy-1 >= 0 && cy-1 < arch->getNumCellsY()) // Top neighbor
				neighbors.push_back(board->at(cx)->at(cy-1));
			for(unsigned k = 0; k < neighbors.size(); ++k)
			{
				neighbors[k]->score = currCell->score+1;
				//if(board->at(neighbors[k]->x)->at(neighbors[k]->y)->block)
				if(neighbors[k]->block)
				{
					neighbors[k]->score = 100000;
					//evaluated->push_back(neighbors[k]);
				}
				score_temp = neighbors[k]->score + 1;
				if(is_in_evaluated_set(evaluated , neighbors[k]))
				{
					if(score_temp >= neighbors[k]->score)
					{

					}
					else
						cerr << "Error " << endl;
				}
				else if(!is_in_evaluated_set(open_set,neighbors[k]) || score_temp < neighbors[k]->score)// && (!find(blockCells.begin(),blockCells.end(),neighbors[k]) || neighbors[k] == t))
					{
						//cerr << "Got passed here!" << endl;
						neighbors[k]->came_from = currCell;
						neighbors[k]->score = score_temp;
						if(!is_in_evaluated_set(open_set, neighbors[k]))
							open_set->push_back(neighbors[k]);
					}
//				neighbors[k]->score = currCell->score + 1;
			}
			neighbors.erase(neighbors.begin(), neighbors.end());

		}
		subRoutes->push_back(sr);
		//cerr << "Finished" << endl;

	}



	// Now, A_Star search algorithm

	delete routingThisTS;
	delete sourceCells;
	delete targetCells;



}

void AStarRouter::remove_from_open_set(vector<StarCell*> *open_set, StarCell* current_node)
{
	for(unsigned i = 0; i< open_set->size(); ++i)
		if(((*open_set)[i])->x == current_node->x && ((*open_set)[i])->y == current_node->y)
		{
			open_set->erase(open_set->begin() + i);
		}
}

vector<RoutePoint*> * AStarRouter::find_path(StarCell* previous_node, StarCell* current_node)
{
	stack<StarCell*> stack_points;
	StarCell * next = current_node;
	vector<RoutePoint *> *sr = new vector<RoutePoint *>();
	while(next->x != previous_node->x || next->y != previous_node->y)
	{
		//RoutePoint* rp = new RoutePoint();
		//rp->cycle = routeCycle++;
		//rp->dStatus = DROP_NORMAL;
		//rp->x = next->x;
		//rp->y = next->y;
		stack_points.push(next);
		next = next->came_from;
		//sr->push_back(rp);
	}
	stack_points.push(next);
	while(!stack_points.empty())
	{
		RoutePoint* rp = new RoutePoint();
		rp->cycle = routeCycle;
		routeCycle++;
		rp->dStatus = DROP_NORMAL;
		rp->x = stack_points.top()->x;
		rp->y = stack_points.top()->y;
		sr->push_back(rp);
		stack_points.pop();
	}
	return sr;
}

StarCell* AStarRouter::score_min(vector<StarCell*> *curr_set)
{
	StarCell* min = new StarCell();
	min->score = -1;
	//cerr << curr_set->size();
	for(unsigned i = 0; i < curr_set->size(); ++i)
		if(min->score > (*curr_set)[i]->score || min->score == -1)
			min = (*curr_set)[i];
	return min;
}

bool AStarRouter::is_in_evaluated_set(vector<StarCell*> *open_set, StarCell* in_question)
{
	for(unsigned i = 0; i< open_set->size(); ++i)
	if(((*open_set)[i])->x == in_question->x && ((*open_set)[i])->y == in_question->y)
	{
		return true;
	}
	return false;
}

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
 * Source: synthesis.cc															*
 * Original Code Author(s): Masruba Tasnim										*
 * Release Date: January 6, 2015												*
 *																				*
 * Details: N/A																	*
 *																				*
 * Revision History:															*
 * WHO		WHEN		WHAT													*
 * ---		----		----													*
 * FML		MM/DD/YY	One-line description									*
 *-----------------------------------------------------------------------------*/

#include "../../Headers/Router/cdma_full_router.h"

#include <algorithm>
#include <vector>
#include <set>
using namespace std;



///////////////////////////////////////////////////////////////////////////////////
// Constructors
///////////////////////////////////////////////////////////////////////////////////
CDMAFullRouter::CDMAFullRouter()
{
	board = NULL;
	claim(false, "Invalid constructor used for Router variant.  Must use form that accepts DmfbArch.\n");
}
CDMAFullRouter::CDMAFullRouter(DmfbArch *dmfbArch)
{
	board = NULL;
	arch = dmfbArch;
}

///////////////////////////////////////////////////////////////////////////////////
// Deconstructor
///////////////////////////////////////////////////////////////////////////////////
CDMAFullRouter::~CDMAFullRouter()
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


// NOTE: unnamed namespace is used so that these CDMA-Router specific things are not exposed externally to other routers and/or codes.
namespace {

#define STALL_COST 1
#define USED_CELL_COST 2
#define NEW_CELL_COST 5
bool debugPrint = false;

struct DropletSourceTarget {
	Droplet* d;  // The corresponding droplet
	int sx, sy;  // (x,y) of source
	int tx, ty;  // (x,y) of target
};

bool SortByManhattanDistance(const DropletSourceTarget& a, const DropletSourceTarget& b) {
	int mda = abs(a.tx - a.sx) + abs(a.ty - a.sy);
	int mdb = abs(b.tx - b.sx) + abs(b.ty - b.sy);
	return mda < mdb;
}


// board size: nx*ny
int nx, ny;

// Smart way to navigate to 4 neighbors of a cell.
// consistent with: enum Direction { EAST, WEST, NORTH, SOUTH, ON_TARGET, DIR_UNINIT };
int dx[] = {1, -1,  0,  0};
int dy[] = {0,  0, -1,  1};

// Opposite direction({E, W, N, S}) == {W, E, S, N}
int oppoDir[] = {1, 0, 3, 2};

enum CellState { CELL_NONE = 0, CELL_HAS_DROPLET = 1, CELL_IR = 2 };

// State parameters for the dijkstra.
struct XYC {
	int x;  // x of cells
	int y;  // y of cells
	int c;  // cycle
	XYC(int x1=0, int y1=0, int c1=0) : x(x1), y(y1), c(c1) {}
};

// This 'lt' operator is needed as pair<int, XYC> is used as a 'key' in a set.
bool operator<(const XYC& a, const XYC& b) {
	if (a.c != b.c) return a.c < b.c;
	if (a.x != b.x) return a.x < b.x;
	return a.y < b.y;
}

int INF = 100000000;

enum DirState { DISPENSE_STATE = -1, STALL_STATE = -2, UNVISITED = -10};
// State information: has 'cost' and the 'direction' element.
struct Info {
	// {0,1,2,3} == EWNS, -1 == root/dispense, -2 == STALL (stay in the same place), -10 == unvisited
	int dir;   // the last direction to come in this slot.
	int cost;  // minimum cost for coming to this state.

	Info(int dir1 = UNVISITED, int cost1 = INF) : dir(dir1), cost(cost1) {}  // -10 == unvisited
};

// Returns true if a droplet can go to (x, y) at cycle-c.
// 'gstates' is the global concurrent routing state containing already-calculated-droplet-routes.
bool CanMoveDropletAt(const vector<vector<map<int, CellState> > >& gstates, int x, int y, int c) {
	claim(gstates[x][y].empty() || gstates[x][y].begin()->first >= 0, "Negative cycle-point found!");

	map<int, CellState>::const_iterator it;
	// check with {c-1, c, c+1} -- dynamic+static fluidic constraints
	for (int collision_cycle = max(c-1, 0); collision_cycle <= c+1; ++collision_cycle) {
		it = gstates[x][y].find(collision_cycle);
		if (it != gstates[x][y].end() && it->second != CELL_NONE) return false;
	}
	return true;
}

bool CanMoveDropletAt(const vector<vector<map<int, CellState> > >& gstates, const XYC& state) {
	return CanMoveDropletAt(gstates, state.x, state.y, state.c);
}

// Returns the current state cost of state[x][y][c].
// If this state was never visited before, it returns infinity.
int GetStateCost(const vector<vector<map<int, Info> > >& state_info, const XYC& state) {
	map<int, Info>::const_iterator it = state_info[state.x][state.y].find(state.c);
	if (it == state_info[state.x][state.y].end()) return INF;
	return it->second.cost;
}

}  // unnamed namespace



///////////////////////////////////////////////////////////////////////////////////
// Initializes the board full of LeeCells instead of Soukup cells for Lee's
///////////////////////////////////////////////////////////////////////////////////
void CDMAFullRouter::routerSpecificInits()
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
void CDMAFullRouter::computeIndivSupProbRoutes(vector<vector<RoutePoint *> *> *subRoutes, vector<Droplet *> *subDrops,
		map<Droplet *, vector<RoutePoint *> *> *routes) {
	if (debugPrint) printf("\nstartingTS = %d\n", startingTS);
	// Masruba: Reused some implementations from LeeRouter.
	/* Start: implementation taken from LeeRouter */
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
					t = board->at(n->GetReconfigMod()->getLX())->at(n->GetReconfigMod()->getBY()); // Bottom-Left,
			}
			targetCells->insert(pair<Droplet *, LeeCell *>(pd, t));
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
					if (debugPrint) cout << "Switched!"  << endl;
					targetCells->erase(d1);
					targetCells->erase(d2);
					targetCells->insert(pair<Droplet *, LeeCell *>(d1, t2));
					targetCells->insert(pair<Droplet *, LeeCell *>(d2, t1));
				}

			}
		}
	}
	/* End: implementation taken from LeeRouter */

	// Construct 's-t pairs' from 'sourceCells' and 'targetCells'.
	vector<DropletSourceTarget> stpairs;
	for (unsigned i = 0; i < routingThisTS->size(); i++) {
		Droplet* d = routingThisTS->at(i);

		DropletSourceTarget stpair;
		stpair.d = d;
		stpair.sx = sourceCells->at(d)->x;
		stpair.sy = sourceCells->at(d)->y;
		stpair.tx = targetCells->at(d)->x;
		stpair.ty = targetCells->at(d)->y;

		stpairs.push_back(stpair);
	}

	// Sort the pairs by increasing Manhattan distance.
	sort(stpairs.begin(), stpairs.end(), SortByManhattanDistance);

	// Board sizes.
	nx = arch->getNumCellsX();  // domain of x: {0, ..., nx-1}
	ny = arch->getNumCellsY();  // domain of y: {0, ..., ny-1}

	// global states containing all routes -- everything is clean at first.
	vector<vector<map<int, CellState> > > gstates(nx);  // gstates[x][y][c] represents state of cell(x,y) at cycle(c).
	vector<vector<bool> > gused(nx);  // gused[x][y] represents whether cell(x,y) is used so far.
	for (int x = 0; x < nx; ++x) {
		gstates[x].resize(ny);
		gused[x].resize(ny);
		for (int y = 0; y < ny; ++y) {
			gstates[x][y].clear();
			claim(gused[x][y] == false, "gused is not false!");
		}
	}

	// *** Parameters for this Router ***
	// costs for Dijkstra:
	int stall_cost = STALL_COST;
	int used_cell_cost = USED_CELL_COST;
	int new_cell_cost = NEW_CELL_COST;

	// Initialize latest-arrival-cycle.
	int latest_arrival_cycle = -1;

	// Route each s-t pair, concurrently. But calculate the routes serially.
	for (int i = 0; i < stpairs.size(); ++i) {
		DropletSourceTarget* stpair = &stpairs[i];
		Droplet* d = stpair->d;
		if (debugPrint) printf("Calculating route for source-target pair with droplet id #%d, fluid: %s\n", d->getId(), d->getComposition().c_str());

		// Determine whether the source is a DISPENSE assay node.
		bool dispense_src = false;
		{
			// Get the *global* route for this droplet.
			vector<RoutePoint*>* route = (*routes)[d];
			claim(route != NULL, "Route for the source-droplet is null!");

			// If completely new route, then DISPENSE.
			if (route->empty()) dispense_src = true;
		}

		/* Start: implementation taken from LeeRouter */
		// For reusing: blocking the 'ReconfigModule' codes.
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

		// NOTE: implementation taken from LeeRouter.
		// Block all new reconfigurable areas from being used for routing
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

		// Blockages for other droplets
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
							//}
							board->at(x)->at(y)->block = true;
							//board->at(x)->at(y)->score = 100000;
							//evaluated->push_back(c);
						}
				//LeeCell * c = targetCells->at(d2);
			}
		}
		for (int x = stpair->tx-1; x <= stpair->tx+1; x++)
			for (int y = stpair->ty-1; y <= stpair->ty+1; y++)
				if (x >= 0 && y >= 0 && x < arch->getNumCellsX() && y < arch->getNumCellsY() /* && !(x == stpair->tx && y == stpair->ty) */ ) // On board?
					board->at(x)->at(y)->block = false;
		/* End: Taken from Lee Router */

		// Create state for this new path.
		// states[x][y][c] represents state of cell(x,y) at cycle(c).
		// Info contains 'cost' and 'direction'.
		vector<vector<map<int, Info> > > states(nx);
		for (int x = 0; x < nx; ++x) {
			states[x].resize(ny);
			for (int y = 0; y < ny; ++y) {
				states[x][y].clear();
			}
		}

		int tc = -1;

		// Insert the 'Start' cell at the first possible 'cycle'.
		// NOTE: for DISPENSE node: we can try in many cycle offsets, starting at c=0.
		//       for non-DISPENSE nodes, we must start at c=0. Because we cannot make it to disappear for the cycle offsets [0, some-positive-c] cycles.
		for (int c = 0;; ++c) {
			// only 1 chance for non-DISPENSE! no 'hidden' stalls possible!
			if (!dispense_src && c > 0) break;

			int cost = c * stall_cost;
			claim(c < 1000, "Failed to route even after adding 999 stalls in the beginning");
			if (!CanMoveDropletAt(gstates, stpair->sx, stpair->sy, c)) continue;

			// Reset state for this path.
			for (int x = 0; x < nx; ++x) {
				for (int y = 0; y < ny; ++y) {
					states[x][y].clear();
				}
			}

			// using set as a min-priority-queue
			// - easier to 'erase' existing item -- needed for 'DECREASE_KEY'.
			set<pair<int /* cost */, XYC /* state */> > pq;

			// Add the initial source for Dijkstra.
			pq.insert(make_pair(cost, XYC(stpair->sx, stpair->sy, c)));
			//states[stpair->sx][stpair->sy][c] = Info(-1, cost);
			states[stpair->sx][stpair->sy][c] = Info(DISPENSE_STATE, cost);

			// shortest-path
			while (!pq.empty()) {
				claim(pq.size() <= 10000000, "PQ got too big!");

				int cost = pq.begin()->first;
				XYC xyc = pq.begin()->second;
				pq.erase(pq.begin());

				// Check if OUTPUT is reached.
				if (xyc.x == stpair->tx && xyc.y == stpair->ty) {
					tc = xyc.c;
					break;
				}

				///////////////////
				// 5 moves total:
				///////////////////

				// stall move (prefer stall to moving backwards (i.e. for the same 'cost'))
				XYC next(xyc.x, xyc.y, xyc.c + 1);
				if (CanMoveDropletAt(gstates, next.x, next.y, next.c)) {
					int old_cost = GetStateCost(states, next);
					int new_cost = cost + stall_cost;

					// DECREASE_KEY
					if (new_cost < old_cost) {
						pq.erase(make_pair(old_cost, next));
						pq.insert(make_pair(new_cost, next));
						//states[next.x][next.y][next.c] = Info(-2 /* stall */, new_cost);
						states[next.x][next.y][next.c] = Info(STALL_STATE /* stall */, new_cost);
					}
				}

				// 4 directional moves
				for (int i = 0; i < 4; ++i) {
					XYC next(xyc.x + dx[i], xyc.y + dy[i], xyc.c + 1);
					if (0 <= next.x && next.x < nx && 0 <= next.y && next.y < ny &&
							CanMoveDropletAt(gstates, next.x, next.y, next.c) &&
							board->at(next.x)->at(next.y)->block == false) {
						int old_cost = GetStateCost(states, next);

						// calculate new cost
						int new_cost = cost + (gused[next.x][next.y] ? used_cell_cost : new_cell_cost);

						// DECREASE_KEY
						if (new_cost < old_cost) {
							pq.erase(make_pair(old_cost, next));
							pq.insert(make_pair(new_cost, next));
							states[next.x][next.y][next.c] = Info(i /* non-stall dir */, new_cost);
						}
					}
				}
			}
			if (tc != -1) break;
		}
		claim(tc != -1, "Target not reached!");

		// Update latest-arrival-cycle.
		latest_arrival_cycle = max(latest_arrival_cycle, tc);

		// construct the path (backtrack from the target using the 'direction' information in states[x][y][c]).
		XYC cur(stpair->tx, stpair->ty, tc);
		vector<XYC> path;
		path.push_back(cur);
		while (states[cur.x][cur.y][cur.c].dir != DISPENSE_STATE) {
			if (states[cur.x][cur.y][cur.c].dir == STALL_STATE) {
				--cur.c;
			}
			else {
				claim(0 <= states[cur.x][cur.y][cur.c].dir && states[cur.x][cur.y][cur.c].dir < 4, "stall or directional");
				int rd = oppoDir[states[cur.x][cur.y][cur.c].dir];
				cur.x += dx[rd];
				cur.y += dy[rd];
				--cur.c;
			}
			path.push_back(cur);
		}

		//construct subroute
		reverse(path.begin(), path.end());

		vector<RoutePoint*>* subRoute = new vector<RoutePoint*>();
		for (int j = 0; j < path.size(); ++j) {
			RoutePoint* rp = new RoutePoint;
			rp->cycle = cycle + path[j].c;
			rp->x = path[j].x;
			rp->y = path[j].y;
			rp->droplet = d;
			if (j > 0 && path[j].x == path[j-1].x && path[j].y == path[j-1].y) {
				rp->dStatus = DROP_WAIT;   // STALL
			}
			else {
				rp->dStatus = DROP_NORMAL;  // non-STALL directional movement
			}
			subRoute->push_back(rp);

			// update global states
			for (int irx = path[j].x-1; irx <= path[j].x + 1; ++irx) {
				for (int iry = path[j].y-1; iry <= path[j].y + 1; ++iry) {
					if (0 <= irx && irx < nx && 0 <= iry && iry < ny) {
						gstates[irx][iry][path[j].c] = CELL_IR;
					}
				}
			}
			gstates[path[j].x][path[j].y][path[j].c] = CELL_HAS_DROPLET;

			// update 'gused' that the cell in (x, y) is used.
			gused[path[j].x][path[j].y] = true;
		}

		// Add the subRoute to 'subRoutes'.
		subRoutes->push_back(subRoute);

		// Add the droplet to the 'subDrops'.
		subDrops->push_back(d);

		// Update 'routeCycle'
		routeCycle = cycle + latest_arrival_cycle;
	}

	// Calculate and output the unique cell used count.
	int cells_used = 0;
	for (int x = 0; x < nx; ++x) {
		for (int y = 0; y < ny; ++y) {
			if (gused[x][y]) ++cells_used;
		}
	}

	if (debugPrint)
	{
		printf("Number of unique cells used this TS: %d\n", cells_used);

		// Also, draw a grid of the used cells, if enabled.
		// # means used
		// . means unused
		bool draw_used_cell_grid = true;
		if (cells_used > 0 && draw_used_cell_grid) {
			for (int y = 0; y < ny; ++y) {
				for (int x = 0; x < nx; ++x) {
					printf("%c", gused[x][y] ? '#' : '.');
				}
				printf("\n");
			}
		}

		printf("\nSTALL COST = %d USED_CELL_COST = %d NEW_CELL_COST %d\n", stall_cost, used_cell_cost, new_cell_cost);
	}

	delete routingThisTS;
	delete sourceCells;
	delete targetCells;
}


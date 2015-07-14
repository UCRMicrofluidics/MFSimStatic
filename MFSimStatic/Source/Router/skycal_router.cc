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
 * Source: skycal_router.cc														*
 * Original Code Author(s): Calvin Phung, Skyler Windh							*
 * Original Completion/Release Date: April 23, 2014								*
 *																				*
 * Details: N/A																	*
 *																				*
 * Revision History:															*
 * WHO		WHEN		WHAT													*
 * ---		----		----													*
 * FML		MM/DD/YY	One-line description									*
 *-----------------------------------------------------------------------------*/

#include "../../Headers/Router/skycal_router.h"
#include <queue>
#include <ctime>

//#define DEBUG_SKYCAL_OUTPUT
#define DEBUG_SKYCAL_PROGRESS_CHECK

#define WEIGHTS_ON_TIME
//#define WEIGHTS_CONGESTION

///////////////////////////////////////////////////////////////////////
// Utility global functions
///////////////////////////////////////////////////////////////////////
Direction leftTurn(Direction dir)
{
	switch (dir)
	{
	case WEST:
		return SOUTH;
	case EAST:
		return NORTH;
	case NORTH:
		return WEST;
	case SOUTH:
		return EAST;
	case DIR_UNINIT:
		return DIR_UNINIT;
	default:
		return DIR_UNINIT;
	}
}

Direction rightTurn(Direction dir)
{
	switch (dir)
	{
	case WEST:
		return NORTH;
	case EAST:
		return SOUTH;
	case NORTH:
		return EAST;
	case SOUTH:
		return WEST;
	case DIR_UNINIT:
		return DIR_UNINIT;
	default:
		return DIR_UNINIT;
	}
}

Direction fullTurn(Direction dir)
{
	switch (dir)
	{
	case WEST:
		return EAST;
	case EAST:
		return WEST;
	case NORTH:
		return SOUTH;
	case SOUTH:
		return NORTH;
	case DIR_UNINIT:
		return DIR_UNINIT;
	default:
		return DIR_UNINIT;
	}
}

string toString(Direction d)
{
	switch (d)
	{
	case DIR_UNINIT:
		return "None";
	case EAST:
		return "East";
	case WEST:
		return "West";
	case NORTH:
		return "North";
	case SOUTH:
		return "South";
	default:
		return "";
	}
}

string toString(Turning t)
{
	switch (t)
	{
	case NO_FACE:
		return "No Face";
	case FORWARD1:
		return "Forward1x";
	case FORWARD2:
		return "Forward2x";
	case FORWARD3:
		return "Forward3x";
	case TURN_LEFT:
		return "Left Turn";
	case TURN_RIGHT:
		return "Right Turn";
	case FULL_TURN:
		return "Full Turn";
	default:
		return "";
	}
}

// The DmfbSimVisualizer requires the start time steps to be sorted in ascending order
// to draw the interference regions of the modules at the right time.
bool compareStartTS(ReconfigModule *rm1, ReconfigModule *rm2)
{
	if (!rm1)
		return true;
	if (!rm2)
		return false;
	return rm1->getStartTS() < rm2->getStartTS();
}

///////////////////////////////////////////////////////////////////////
// Constructors
///////////////////////////////////////////////////////////////////////
SkyCalRouter::SkyCalRouter()
: Router(NULL), Scheduler(), washEnabled(true), cells(), executionTable(), growthFactors(), numDroplets(0), availableArea(0),
  noDelta(0), underDeadlockThresholdCount(0), dropProgress(),
  twoWeightBest1(TWO_WEIGHT_BEST1), twoWeightBest2(TWO_WEIGHT_BEST2),
  threeWeightBest1(THREE_WEIGHT_BEST1), threeWeightBest2(THREE_WEIGHT_BEST2), threeWeightBest3(THREE_WEIGHT_BEST3),
  assayComplete(false)
{
}

SkyCalRouter::SkyCalRouter(DmfbArch *arch)
: Router(arch), Scheduler(), washEnabled(true), cells(), executionTable(), growthFactors(), numDroplets(0), availableArea(0),
  noDelta(0), underDeadlockThresholdCount(0),
  twoWeightBest1(TWO_WEIGHT_BEST1), twoWeightBest2(TWO_WEIGHT_BEST2),
  threeWeightBest1(THREE_WEIGHT_BEST1), threeWeightBest2(THREE_WEIGHT_BEST2), threeWeightBest3(THREE_WEIGHT_BEST3),
  assayComplete(false)
{
}

///////////////////////////////////////////////////////////////////////
// Determines how deep a cell is within modules
// Uses a wave technique which is M * O(N^2), where N is the number of
// cells and M depends on the number of required re-updates
// *** TODO: Should probably not only be done in initialization and be dynamic in runtime. This could slow down the process greatly though.
///////////////////////////////////////////////////////////////////////
void SkyCalRouter::computeCellDepth()
{
	for (int i = 0; i < cells.size(); ++i)
		for (int j = 0; j < cells[i].size(); ++j)
		{
			if (cells[i][j].modules.size() > 0 || cells[i][j].outs.size() > 0)
				cells[i][j].depth = arch->getNumCellsX() * arch->getNumCellsY();
			else
				cells[i][j].depth = 0;
		}

	bool change = true;
	while (change)
	{
		change = false;
		for (int x = 0; x < cells.size(); ++x)
			for (int y = 0; y < cells[x].size(); ++y)
			{
				int left = cells[max(x - 1, 0)][y].depth;
				int right = cells[min(x + 1, arch->getNumCellsX() - 1)][y].depth;
				int top = cells[x][max(y - 1, 0)].depth;
				int bottom = cells[x][min(y + 1, arch->getNumCellsY() - 1)].depth;

				int nbest = min(min(left, right), min(top, bottom));
				if (nbest < cells[x][y].depth - 1)
				{
					cells[x][y].depth = nbest + 1;
					change = true;
				}
			}
	}
}

///////////////////////////////////////////////////////////////////////
// Initializes the array that is used by the processing engine to
// determine if a cell is equipped with a heater/detector.
///////////////////////////////////////////////////////////////////////
// *** TODO: We should be able to invoke the base function getAvailResources instead
//			There is an issue with initializing availRes for some reason
//			so we do it in this function instead
void SkyCalRouter::initCellTypeArray()
{
	numDroplets = 0;

	// Create a 2D-array which tells if a cell is augmented with a heater or detector
	for (int x = 0; x < arch->getNumCellsX(); x++)
	{
		vector<CellInfo> infoCol;
		for (int y = 0; y < arch->getNumCellsY(); y++)
		{
			CellInfo cell;
			cell.type = BASIC_RES;
			cell.depth = 0;
			cell.modules = vector<FixedModule *>();
			cell.contaminator = "";
			infoCol.push_back(cell);
		}
		cells.push_back(infoCol);
	}

	for (int i = 0; i < outRes->size(); ++i)
	{
		IoResource *res = outRes->at(i);
		IoPort *port = res->port;

		int posXY = port->getPosXY();
		int left = 0;
		int right = 0;
		int top = 0;
		int bottom = 0;
			switch (port->getSide())
			{
			case NORTH:
				left = max(posXY - 1, 0);
				right = min(posXY + 1, arch->getNumCellsX() - 1);
				top = 0;
				bottom = min(1, arch->getNumCellsY() - 1);
				break;
			case EAST:
				left = max(arch->getNumCellsX() - 2, 0);
				right = arch->getNumCellsX() - 1;
				top = max(posXY - 1, 0);
				bottom = min(posXY + 1, arch->getNumCellsY() - 1);
				break;
			case SOUTH:
				left = max(posXY - 1, 0);
				right = min(posXY + 1, arch->getNumCellsX() - 1);
				top = max(arch->getNumCellsY() - 2, 0);
				bottom = arch->getNumCellsY() - 1;
				break;
			case WEST:
				left = 0;
				right = min(1, arch->getNumCellsX() - 1);
				top = max(posXY - 1, 0);
				bottom = min(posXY + 1, arch->getNumCellsY() - 1);
				break;
			default:
				break;
			}

		for (int x = left; x <= right; ++x)
			for (int y = top; y <= bottom; ++y)
			{
				cells[x][y].outs.push_back(res);

				/*
				// Set module depth for each cell that belongs to an out resource
				int leftdist = port->getSide() != WEST ? x - left + 1 : arch->getNumCellsX();
				int rightdist = port->getSide() != EAST ? right - x + 1 : arch->getNumCellsX();
				int topdist = port->getSide() != NORTH ? y - top + 1 : arch->getNumCellsY();
				int bottomdist = port->getSide() != SOUTH ? bottom - y + 1 : arch->getNumCellsY();

				int depth = min(min(leftdist, rightdist), min(topdist, bottomdist));

				cells.at(x).at(y).depth += depth;
				*/
			}
	}

	// Initialize available resources and set up cell type 2-D map.
	for (int i = 0; i < arch->getExternalResources()->size(); ++i)
	{
		FixedModule *fm = arch->getExternalResources()->at(i);
		fm->setBusy(false);
		ResourceType conclusion = UNKNOWN_RES;
		for (int x = fm->getLX(); x <= fm->getRX(); x++)
		{
			for (int y = fm->getTY(); y <= fm->getBY(); y++)
			{
				if (fm->getResourceType() == H_RES && cells.at(x).at(y).type == D_RES)
				{
					cells.at(x).at(y).type = DH_RES;
					conclusion = DH_RES;
				}
				else if (fm->getResourceType() == H_RES)
				{
					cells.at(x).at(y).type = H_RES;
					conclusion = H_RES;
				}
				else if (fm->getResourceType() == D_RES && cells.at(x).at(y).type == H_RES)
				{
					cells.at(x).at(y).type = DH_RES;
					conclusion = DH_RES;
				}
				else if (fm->getResourceType() == D_RES)
				{
					cells.at(x).at(y).type = D_RES;
					conclusion = D_RES;
				}
				else
					claim(false, "Unsupported cell-augmentation.");
			}

		}
		// For each module,
		// Set up depth region along with interference which is 1 along the sides
		int edgeL = fm->getLX() - 1;
		int edgeR = fm->getRX() + 1;
		int edgeT = fm->getTY() - 1;
		int edgeB = fm->getBY() + 1;
		for (int x = edgeL; x <= edgeR; x++)
		{
			for (int y = edgeT; y <= edgeB; y++)
			{
				if (x < 0 || x >= arch->getNumCellsX() || y < 0 || y >= arch->getNumCellsY())
					claim(false, "Initialization failed: point location outside of architecture");
				cells.at(x).at(y).modules.push_back(fm);

				/*
				// Set module depth for each cell that belongs to a module
				int leftdist = x - edgeL + 1;
				int rightdist = edgeR - x + 1;
				int topdist = y - edgeT + 1;
				int bottomdist = edgeB - y + 1;
				int depth = min(min(leftdist, rightdist), min(topdist, bottomdist));

				cells.at(x).at(y).depth += depth;
				*/
			}
		}
		availRes[conclusion]++;
	}

	computeCellDepth();

#ifdef DEBUG_SKYCAL_OUTPUT
	for (int x = 0; x < cells.size(); ++x)
	{
		for (int y = 0; y < cells[x].size(); ++y)
		{
			cout << cells[x][y].depth << " ";
		}
		cout << endl;
	}
#endif

	computeAvailableArea();
	recomputationNecessary = false;
}

// Sets up potential growth factors by tracing the sum of the number of forward edges
// backwards. This is under the assumption that a droplet exists on the board
// based on each forward edge.
//
// *** TODO: Sets are expensive to store at every assay node. A pointer design is set up
//			to only store sets at every important node (splits and end nodes). This
//			accurately captures growth of converging paths. However, we still store
//			an integer value for growthFactors to save not only memory, but also
//			the idea that droplets can all potentially merge into one. Therefore,
//			a combination of limiting execution based on growth factors and
//			a current droplet count limiter should be done.
void SkyCalRouter::setGrowthFactors(DAG *dag)
{
	list<vector<int> *> growthList;
	map<AssayNode *, vector<int> *> growthSet; // Provides information for each node about their potential droplet growth
	queue<AssayNode *> frontier;

	int c = 0;
	for (int i = 0; i < dag->getAllNodes().size(); ++i)
	{
		AssayNode *n = dag->getAllNodes().at(i);
		if (n->GetChildren().size() <= 0)
		{
			growthList.push_back(new vector<int>(1));
			growthList.back()->at(0) = c;
			growthSet[n] = growthList.back();
			++c;

			for (int j = 0; j < n->GetParents().size(); ++j)
				frontier.push(n->GetParents().at(j));
		}
	}

	while (!frontier.empty())
	{
		AssayNode *n = frontier.front();
		frontier.pop();
		const map<AssayNode *, vector<int> *>::iterator res = growthSet.find(n);
		if (res != growthSet.end())
			continue;

		for (int i = 0; i < n->GetChildren().size(); ++i)
		{
			const map<AssayNode *, vector<int> *>::iterator it = growthSet.find(n->GetChildren().at(i));
			if (it == growthSet.end())
				continue;
		}

		if (n->GetChildren().size() > 1)
		{
			growthList.push_back(new vector<int>());
			growthSet[n] = growthList.back();
			for (int i = 0; i < n->GetChildren().size(); ++i)
			{
				vector<int> result(c);
				vector<int>::iterator result_it;
				result_it = set_union(growthSet[n]->begin(), growthSet[n]->end(),
						growthSet[n->GetChildren().at(i)]->begin(), growthSet[n->GetChildren().at(i)]->end(),
						result.begin());
				result.resize(result_it - result.begin());
				growthSet[n]->resize(result.size());
				copy(result.begin(), result.end(), growthSet[n]->begin());
			}
		}
		else if (n->GetChildren().size() == 1)
			growthSet[n] = growthSet[n->GetChildren().at(0)];
		else
			claim(false, "Error, case that should never occur happened - initializing Growth Factors");

		growthFactors[n] = growthSet[n]->size();
		for (int j = 0; j < n->GetParents().size(); ++j)
			frontier.push(n->GetParents().at(j));
	}

	for (list<vector<int> *>::iterator it = growthList.begin(); it != growthList.end(); ++it)
		delete *it;

#ifdef DEBUG_SKYCAL_OUTPUT
	cout << "----- GROWTH FACTOR -----\n";
	for (int i = 0; i < dag->getAllNodes().size(); ++i)
	{
		AssayNode *n = dag->getAllNodes().at(i);
		cout << n->id << ": " << growthFactors[n] << endl;
	}
#endif
}

void SkyCalRouter::initWashAssay()
{
	partitions.clear();
	contaminationMap.clear();
	contaminationQueue.clear();

	washOps = new DAG();

	int num = 0;

	partitionWidth = 0;
	partitionHeight = 0;
	for (int x = 0; x + MIN_PARTITION_WIDTH - 1 < arch->getNumCellsX(); x += MIN_PARTITION_WIDTH)
	{
		++partitionWidth;
		for (int y = 0; y + MIN_PARTITION_HEIGHT - 1 < arch->getNumCellsY(); y += MIN_PARTITION_HEIGHT)
		{
			// Only need to figure out what the height is once
			if (x == 0)
				++partitionHeight;

			int lx = x;
			int ty = y;
			int rx = x + MIN_PARTITION_WIDTH - 1;
			int by = y + MIN_PARTITION_HEIGHT - 1;
			if (rx + MIN_PARTITION_WIDTH >= arch->getNumCellsX())
				rx = arch->getNumCellsX() - 1;
			if (by + MIN_PARTITION_HEIGHT >= arch->getNumCellsY())
				by = arch->getNumCellsY() - 1;
			ReconfigModule *part = new ReconfigModule(UNKNOWN_RES, lx, ty, rx, by);
			part->setTileNum(num);

			++num;
			stringstream ss;
			ss << num;
			AssayNode *n = washOps->AddWashNode("WASH" + ss.str());
			n->reconfigMod = part;
			n->priority = 0;
			//n->portName = "water";
			n->portName = washPort->getPortName(); // DTG
			n->ioPort = washPort;
			n->status = SCHEDULED;
			growthFactors[n] = 1;
			partitions.push_back(part);
			contaminationMap.push_back(map<int, list<RoutePoint *> >());
			contaminationQueue.push_back(list<int>());
		}
	}
}

///////////////////////////////////////////////////////////////////////
// Utility class functions
///////////////////////////////////////////////////////////////////////

void SkyCalRouter::setBestWeights()
{
	//cout << " BEST ";
	twoWeightBest1 = TWO_WEIGHT_BEST1C;
	twoWeightBest2 = TWO_WEIGHT_BEST2C;
	threeWeightBest1 = THREE_WEIGHT_BEST1C;
	threeWeightBest2 = THREE_WEIGHT_BEST2C;
	threeWeightBest3 = THREE_WEIGHT_BEST3C;
}

void SkyCalRouter::setImprovedWeights()
{
	//cout << " IMPROVED ";
	twoWeightBest1 = TWO_WEIGHT_BEST1B;
	twoWeightBest2 = TWO_WEIGHT_BEST2B;
	threeWeightBest1 = THREE_WEIGHT_BEST1B;
	threeWeightBest2 = THREE_WEIGHT_BEST2B;
	threeWeightBest3 = THREE_WEIGHT_BEST3B;
}

void SkyCalRouter::setNeutralWeights()
{
	//cout << " NEUTRAL ";
	twoWeightBest1 = TWO_WEIGHT_BEST1A;
	twoWeightBest2 = TWO_WEIGHT_BEST2A;
	threeWeightBest1 = THREE_WEIGHT_BEST1A;
	threeWeightBest2 = THREE_WEIGHT_BEST2A;
	threeWeightBest3 = THREE_WEIGHT_BEST3A;
}

int SkyCalRouter::getPartitionIndex(RoutePoint *p) const
{
	int partX = p->x / MIN_PARTITION_WIDTH;
	int partY = p->y / MIN_PARTITION_HEIGHT;

	if (partX >= partitionWidth)
		partX = partitionWidth - 1;
	if (partY >= partitionHeight)
		partY = partitionHeight - 1;

	return partY + partX * partitionHeight;
}

void SkyCalRouter::getPartitionRect(int part, int & lx, int & rx, int & ty, int & by) const
{
	int partX = part / partitionHeight;
	int partY = part - partX * partitionHeight;

	lx = partX * MIN_PARTITION_WIDTH;
	rx = lx + MIN_PARTITION_WIDTH - 1;
	ty = partY * MIN_PARTITION_HEIGHT;
	by = ty + MIN_PARTITION_HEIGHT - 1;
	if (rx + MIN_PARTITION_WIDTH >= arch->getNumCellsX())
		rx = arch->getNumCellsX() - 1;
	if (by + MIN_PARTITION_HEIGHT >= arch->getNumCellsY())
		by = arch->getNumCellsY() - 1;
}

bool SkyCalRouter::areaIsClean(AssayNode* n, int lx, int rx, int ty, int by) const
{
	for (int x = lx; x <= rx; ++x)
		for (int y = ty; y <= by; ++y)
		{
			bool safe = false;
			if (cells[x][y].contaminator == "")
				safe = true;
			else
			{
				for (int i = 0; i < n->GetDroplets().size(); ++i)
				{
					if (cells[x][y].contaminator == n->GetDroplets()[i]->uniqueFluidName)
					{
						safe = true;
						break;
					}
				}
			}
			if (!safe)
				return false;
		}
	return true;
}

void SkyCalRouter::removeContaminator(RoutePoint *p, Droplet *d)
{
	if (d->hp > 0)
		cells[p->x][p->y].contaminator = "";
	d->hp -= 1;
}

void SkyCalRouter::updateContaminationAt(RoutePoint *p, Droplet *d)
{
	if (!washEnabled)
		return;

	int partIndex = getPartitionIndex(p);

	//if (d->uniqueFluidName != "water")
	if (d->uniqueFluidName != washPort->getPortName())
	{
		// This is for ordinary droplets that like to infect cells
		if (cells[p->x][p->y].contaminator == "")
		{
			int ref = dropReference[d->uniqueFluidName];

			if (contaminationMap[partIndex][ref].size() <= 0)
				contaminationQueue[partIndex].push_back(ref);
			contaminationMap[partIndex][ref].push_back(p);
			cells[p->x][p->y].contaminator = d->uniqueFluidName;
		}
	}
	else
	{
		// This is for wash droplets that clean cells

		if (cells[p->x][p->y].contaminator != "")
			removeContaminator(p, d);
	}
}

RoutePoint *SkyCalRouter::getNextWashDropletDest(AssayNode *n, Droplet *d, map<Droplet *, vector<RoutePoint *> *> *routes)
{
	int partIndex = n->GetReconfigMod()->getTileNum();

	RoutePoint *s = NULL;
	if ((*routes)[d]->size() > 0)
		s = (*routes)[d]->back();
	if (s != NULL && s->cycle != cycle - 1)
		s = NULL;

	while (contaminationQueue[partIndex].size() > 0)
	{
		int ref = contaminationQueue[partIndex].front();

		if (dropProgress[d].inProgress && dropProgress[d].isLate(cycle))
		{
			contaminationQueue[partIndex].pop_front();
			contaminationQueue[partIndex].push_back(ref);
			ref = contaminationQueue[partIndex].front();
			dropProgress[d].inProgress = false;
		}

		while (contaminationMap[partIndex][ref].size() > 0)
		{
			RoutePoint *dest = contaminationMap[partIndex][ref].front();
			if (cells[dest->x][dest->y].contaminator != "" && (s == NULL || s->x != dest->x || s->y != dest->y))
			{
				return dest;
			}
			else
			{
				dropProgress[d].inProgress = false;
				contaminationMap[partIndex][ref].pop_front();
			}
		}
		contaminationQueue[partIndex].pop_front();
	}
	return NULL;
}

// Returns the next point based on the direction passed in and the current point
// WARNING: allocates the point on the heap
RoutePoint *SkyCalRouter::step(RoutePoint *p, Direction dir)
{
	RoutePoint *ret = NULL;
	switch (dir)
	{
	case NORTH:
		ret = new RoutePoint();
		ret->x = p->x;
		ret->y = p->y - 1;
		break;
	case EAST:
		ret = new RoutePoint();
		ret->x = p->x + 1;
		ret->y = p->y;
		break;
	case SOUTH:
		ret = new RoutePoint();
		ret->x = p->x;
		ret->y = p->y + 1;
		break;
	case WEST:
		ret = new RoutePoint();
		ret->x = p->x - 1;
		ret->y = p->y;
		break;
	case DIR_UNINIT:
		ret = new RoutePoint();
		ret->x = p->x;
		ret->y = p->y;
		break;
	default:
		ret = new RoutePoint();
		ret->x = p->x;
		ret->y = p->y;
		break;
	}
	return ret;
}

// Returns the direction assuming the points from and to are adjacent to each other
Direction SkyCalRouter::determineDirection(RoutePoint *from, RoutePoint *to)
{
	if (from->x < to->x)
		return EAST;
	else if (from->x > to->x)
		return WEST;
	else if (from->y < to->y)
		return NORTH;
	else if (from->y > to->y)
		return SOUTH;
	else //if (from->x == to->x && from->y == to->y)
		return DIR_UNINIT;
}

// Returns the turn based on the droplet's previous movement and the next potential point
// *** TODO: On a droplet mix, we shouldn't consider the lowest id droplet direction. Otherwise, the mixing droplet starts with a temporary
// forward boost.
Turning SkyCalRouter::determineDropletTurn(Droplet *d, map<Droplet *, vector<RoutePoint *> *> *routes, RoutePoint *next)
{
	vector<RoutePoint *> *route = (*routes)[d];

	if (route->size() <= 0)
		return NO_FACE;
	else if (route->size() == 1)
	{
		Direction dir1 = determineDirection(route->at(route->size() - 1), next);
		return FORWARD1;
	}
	else if (route->size() == 2)
	{
		Direction dir2 = determineDirection(route->at(route->size() - 2), route->at(route->size() - 1));
		Direction dir1 = determineDirection(route->at(route->size() - 1), next);

		if (dir1 == DIR_UNINIT)
			return NO_FACE;
		else
		{
			if (dir2 == DIR_UNINIT)
				return FORWARD1;
			else if (dir1 == dir2)
				return FORWARD2;
			else if (leftTurn(dir1) == dir2)
				return TURN_LEFT;
			else if (rightTurn(dir1) == dir2)
				return TURN_RIGHT;
			else //if (fullTurn(dir1) == dir2)
				return FULL_TURN;
		}
	}
	else
	{
		Direction dir3 = determineDirection(route->at(route->size() - 3), route->at(route->size() - 2));
		Direction dir2 = determineDirection(route->at(route->size() - 2), route->at(route->size() - 1));
		Direction dir1 = determineDirection(route->at(route->size() - 1), next);

		if (dir1 == DIR_UNINIT)
			return NO_FACE;
		else
		{
			if (dir2 == DIR_UNINIT)
				return FORWARD1;
			else if (dir1 == dir2)
			{
				if (dir3 == DIR_UNINIT || dir2 != dir3)
					return FORWARD2;
				else
					return FORWARD3;
			}
			else if (leftTurn(dir1) == dir2)
				return TURN_LEFT;
			else if (rightTurn(dir1) == dir2)
				return TURN_RIGHT;
			else //if (fullTurn(dir1) == dir2)
				return FULL_TURN;
		}
	}
	claim(false, "Should've returned a value in: determineDropletTurn");
}

// Returns the mixing degree for a certain direction between a droplet's previous moves and the next point
float SkyCalRouter::computeMixingDegree(Droplet *d, map<Droplet *, vector<RoutePoint *> *> *routes, RoutePoint *next)
{
	Turning result = determineDropletTurn(d, routes, next);
	switch (result)
	{
	case FORWARD1:
		return -0.29;
	case FORWARD2:
		return -0.58;
	case FORWARD3:
		return -0.87;
	case TURN_LEFT:
		return -0.10;
	case TURN_RIGHT:
		return -0.10;
	case FULL_TURN:
		return 0.50;
	case NO_FACE:
		return 0.00;
	}
}

// Looks for the closest available module for a certain droplet to perform a detect/heat operation.
// It first finds an available module that is specialized for either detect/heat depending on the operation.
// If none exist, then it finds an available module that can do both detect/heat.
// It returns NULL if no module is found.
FixedModule *SkyCalRouter::findModule(Droplet *d, AssayNode *n, map<Droplet *, vector<RoutePoint *> *> *routes)
{
	vector<FixedModule *> modules;

	// Find all available modules
	for (int i = 0; i < arch->getExternalResources()->size(); i++)
	{
		FixedModule *fm = arch->getExternalResources()->at(i);

		ResourceType resType = n->type == DETECT ? D_RES : H_RES;
		if (fm->getResourceType() == resType && !fm->getReady())
			modules.push_back(fm);
	}
	if (modules.size() <= 0)
	{
		// If no specialized module are available, find DETECT/HEAT modules
		for (int i = 0; i < arch->getExternalResources()->size(); i++)
		{
			FixedModule *fm = arch->getExternalResources()->at(i);
			if (fm->getResourceType() == DH_RES && !fm->getReady())
			{
				modules.push_back(fm);
			}
		}
	}

	RoutePoint *p = (*routes)[d]->back();
	RoutePoint *q = new RoutePoint();

	FixedModule *best = NULL;
	int bestValue = arch->getNumCellsX() * arch->getNumCellsY(); // Infinity
	for (int i = 0; i < modules.size(); ++i)
	{
		q->x = modules[i]->getLX();
		q->y = modules[i]->getBY();

		int score = manhattanDist(p, q);
		if (score < bestValue)
		{
			best = modules[i];
			bestValue = score;
		}
	}
	delete q;

	return best;
}

// Looks for the closest output port, if waste is set, it will only look for waste ports
IoResource *SkyCalRouter::findOutputPort(Droplet *d, AssayNode *n, map<Droplet *, vector<RoutePoint *> *> *routes, bool waste)
{
	RoutePoint *p = (*routes)[d]->back();
	RoutePoint *q = new RoutePoint();

	IoResource *best = NULL;
	int bestValue = arch->getNumCellsX() * arch->getNumCellsY(); // Infinity
	for (int i = 0; i < outRes->size(); i++)
	{
		IoResource *dr = outRes->at(i);
		if ((!waste && n->GetPortName() == dr->name) || (waste && dr->name == "waste"))
		{
			IoPort *port = dr->port;

			// Obtain IO Port's dispense location.
			if (port->getSide() == NORTH)
			{
				q->x = port->getPosXY();
				q->y = 0;
			}
			else if (port->getSide() == SOUTH)
			{
				q->x = port->getPosXY();
				q->y = arch->getNumCellsY()-1;
			}
			else if (port->getSide() == EAST)
			{
				q->x = arch->getNumCellsX()-1;
				q->y = port->getPosXY();
			}
			else if (port->getSide() == WEST)
			{
				q->x = 0;
				q->y = port->getPosXY();
			}
			else
				claim(false, "Output port has no assigned side\n");

			int score = manhattanDist(p, q);
			if (score < bestValue)
			{
				best = dr;
				bestValue = score;
			}
		}
	}
	delete q;
	return best;
}

// Checks whether the module is clear of unwanted droplets. Any droplets
// in the operation for node n are ignored.
bool SkyCalRouter::moduleIsClear(FixedModule *fm, AssayNode *n, map<Droplet *, vector<RoutePoint *> *> *routes)
{
	int edgeL = fm->getLX() - 1;
	int edgeR = fm->getRX() + 1;
	int edgeT = fm->getTY() - 1;
	int edgeB = fm->getBY() + 1;
	for (int x = edgeL; x <= edgeR; x++)
		for (int y = edgeT; y <= edgeB; y++)
		{
			for (map<Droplet *, vector<RoutePoint *> *>::iterator it = routes->begin(); it != routes->end(); ++it)
			{
				if(!n->isMaintainingDroplet(it->first))
				{
					vector<RoutePoint *> *route = it->second;
					if (route->size() > 0)
					{
						RoutePoint *t = route->back();
						if (t->cycle == cycle || t->cycle == cycle - 1)
							if (t->x >= edgeL && t->x <= edgeR && t->y >= edgeT && t->y <= edgeB)
								return false;
					}
				}
			}
		}
	return true;
}

// Returns true whether the current cell location is available to the droplets in node n.
// i.e. if the cell has a module that's busy and it is not node n's module, it is not available
// i.e. if the cell is near an output port thats been active, it is not available if node n
//		does not have the output port as its resource.
bool SkyCalRouter::cellIsAvailable(int x, int y, AssayNode *n)
{
	CellInfo cell = cells.at(x).at(y);

	for (int i = 0; i < cell.modules.size(); ++i)
	{
		if (cell.modules[i]->getReady())
		{
			if (n->type != WASH)
			{
				// The module becomes busy for the assay when it is cleaned, so don't go into it until it's done
				if (n->boundedExternalRes != cell.modules[i] || !cell.modules[i]->getBusy())
					return false;
			}
			else
			{
				// If the module is busy, don't have to clean it
				if (cell.modules[i]->getBusy())
					return false;
			}
		}
	}

	bool designatedPortCell = false;
	for (int i = 0; i < cell.outs.size(); ++i)
	{
		if (n->ioPort == cell.outs[i]->port || n->type == WASH)
		{
			designatedPortCell = true;
			break;
		}
	}

	if (!designatedPortCell)
	{
		for (int i = 0; i < cell.outs.size(); ++i)
		{
			IoResource *res = cell.outs[i];

			for (list<AssayNode *>::iterator it = res->schedule.begin(); it != res->schedule.end(); ++it)
			{
				// Possible that an assay is set to COMPLETE before and after makes a difference?
				// No, Synchronous is still achieved here due to feasibility checking
				// previous cycle to not activate an unwanted mix near output ports
				AssayNode *sched = *it;

				if (sched->status != COMPLETE)
					return false;
			}
		}
	}
	return true;
}

bool SkyCalRouter::willContaminate(RoutePoint *p, AssayNode *n)
{

	// Check around the point we're heading to has no contamination
	if (n->type != WASH)
	{
		/*
		 // For not allowing droplets to be adjacent to contaminated cell
		int edgeL = max(0, p->x - 1);
		int edgeR = min(arch->getNumCellsX() - 1, p->x + 1);
		int edgeT = max(0, p->y - 1);
		int edgeB = min(arch->getNumCellsY() - 1, p->y + 1);
		*/
		// For allowing droplets to be adjacent
		int edgeL = max(0, p->x);
		int edgeR = min(arch->getNumCellsX() - 1, p->x);
		int edgeT = max(0, p->y);
		int edgeB = min(arch->getNumCellsY() - 1, p->y);
		for (int x = edgeL; x <= edgeR; x++)
			for (int y = edgeT; y <= edgeB; y++)
			{
				bool available = false;
				for (int i = 0; i < n->GetDroplets().size(); ++i)
				{
					if (n->GetDroplets()[i]->uniqueFluidName.find(cells[x][y].contaminator) != string::npos) // includes "" for not contaminated cell
					{
						available = true;
						break;
					}
				}
				if (!available)
					return true;
			}
	}
	return false;
}

// Checks whether point for droplet d will be in any adjacent cell to another droplet.
// This requires an exception for any droplet we need to mix with if provided in the f parameter
//
// If strict is set to true, then droplets will force itself to leave the module as soon as possible.
//
// Also now checks contamination for a cell we are moving to.
//
// *** TODO: A Wash droplet needs to clean a module just set to busy..., but it has to get clear of the module, this is a MUST FIX HIGH PRIORITY
bool SkyCalRouter::checkFeasibility(RoutePoint *p, Droplet *d, AssayNode* n, map<Droplet *, vector<RoutePoint *> *> *routes, bool strict)
{
	if (p->x < 0 || p->x >= arch->getNumCellsX() || p->y < 0 || p->y >= arch->getNumCellsY())
		return false;

	for (map<Droplet *, vector<RoutePoint *> *>::iterator it = routes->begin(); it != routes->end(); ++it)
	{
		// Don't check for conflicts between droplets in the same Assay as mix to avoid checking droplets that merge
		if (d != it->first && (n->GetType() != MIX || !n->isMaintainingDroplet(it->first)))
		{
			vector<RoutePoint *> *route = it->second;
			RoutePoint *t1 = NULL;
			RoutePoint *t2 = NULL;

			RoutePoint *t = NULL;
			if (route->size() > 0)
			{
				t = route->at(route->size() - 1);
				if (t->dStatus != DROP_MERGING && (t->dStatus != DROP_OUTPUT && t->dStatus != DROP_WASTE))
				{
					if (t->cycle == cycle)
						t1 = t;
					else if (t->cycle == cycle - 1)
						t2 = t;
				}
			}
			if (route->size() > 1)
			{
				t = route->at(route->size() - 2);
				if (t->dStatus != DROP_MERGING && (t->dStatus != DROP_OUTPUT && t->dStatus != DROP_WASTE))
				{
					if (t->cycle == cycle - 1)
						t2 = t;
				}
			}
			if ((t1 && doesInterfere(p, t1)) || (t2 && doesInterfere(p, t2)))
				return false;
		}
	}

	if (willContaminate(p, n))
		return false;

	// Check for busy modules, if it is part of a busy module, it is not a feasible location
	// unless the module is our target.
	if ((*routes)[d]->size() > 0)
	{
		RoutePoint *cur = (*routes)[d]->back();

		if (!cellIsAvailable(p->x, p->y, n))
		{
			// If we are in here, that means the target cell is where we don't want to go.
			// However, if the cell we are standing on is also where we don't want to go,
			// we compare which cell is best to leave the module we're on.
			if (!cellIsAvailable(cur->x, cur->y, n))
			{
				CellInfo *cellFrom = &cells[cur->x][cur->y];
				CellInfo *cellTo = &cells[cur->x][cur->y];
				return strict ? cellTo->depth < cellFrom->depth : cellTo->depth <= cellFrom->depth;
			}
			return false;
		}
	}
	else
	{
		if (!cellIsAvailable(p->x, p->y, n))
			return false;
	}

	return true;
}

// Returns the absolute difference between two points.
int SkyCalRouter::manhattanDist(RoutePoint *p1, RoutePoint *p2)
{
	return abs(p2->x - p1-> x) + abs(p2->y - p1->y);
}

// Selects a random Move with specified weights given a list of moves, it only considers the top 3 moves.
// Moves of equal value are not equally considered.
int SkyCalRouter::selectRandomMove(vector<Move> *moves)
{
#ifdef WEIGHTS_CONGESTION
	// Uses adaptive weights depending on the board congestion.
	// Comment this out if this is not desired
	if (boardIsVeryCongested() || underDeadlockThresholdCount > 10)
	{
		setNeutralWeights();
	}
	else if (boardIsCongested() || underDeadlockThresholdCount > 2)
	{
		setImprovedWeights();
	}
	else
	{
		setBestWeights();
	}
#endif
	/*
	vector<int> mergedMoves;
	mergedMoves.push_back(0);
	for (int i = 1; i < moves->size(); ++i)
	{
		if (moves->at(i).value != moves->at(i - 1).value)
		{
			mergedMoves.push_back(i);
		}
	}

	int selectedIndex = -1;
	if (mergedMoves.size() <= 1)
		selectedIndex = 0;
	else if (mergedMoves.size() == 2)
	{
		int r = rand() % 100 + 1;
		if (r <= twoWeightBest1)
			selectedIndex = 0;
		else
			selectedIndex = 1;
	}
	else
	{
		int r = rand() % 100 + 1;
		if (r <= threeWeightBest1)
			selectedIndex = 0;
		else if (r <= threeWeightBest2)
			selectedIndex = 1;
		else
			selectedIndex = 2;
	}
	mergedMoves.push_back(moves->size());

	for (int i = 0; i < moves->size(); ++i)
	{
		if (finalIndex != i)
			cout << "(";
			else cout << "*";

		cout << toString(moves->at(i).dir) << "," << moves->at(i).value;

		if (finalIndex != i)
			cout << ") ";
			else cout << "* ";
	}
	cout << endl;

	int finalIndex = mergedMoves[selectedIndex] + rand() % (mergedMoves[selectedIndex + 1] - mergedMoves[selectedIndex]);
	return finalIndex;
	*/
	int selectedIndex = -1;
	if (moves->size() <= 1)
		selectedIndex = 0;
	else if (moves->size() == 2)
	{
		int r = rand() % 100 + 1;
		if (r <= twoWeightBest1)
			selectedIndex = 0;
		else
			selectedIndex = 1;
	}
	else
	{
		int r = rand() % 100 + 1;
		if (r <= threeWeightBest1)
			selectedIndex = 0;
		else if (r <= threeWeightBest2)
			selectedIndex = 1;
		else
			selectedIndex = 2;
	}
	return selectedIndex;
}

// Determines whether the board is congested. Since the number of droplets on a given rectangular
// board can fit up to approximate half the area. We set half of half the area as a strict limit
bool SkyCalRouter::boardIsCongested()
{
	return numDroplets >= availableArea / 8;
}
bool SkyCalRouter::boardIsVeryCongested()
{
	return numDroplets >= availableArea / 6;
}

// Determines whether a Dispense assay should be delayed if other operations are not complete.
// This prevents dispensed droplets to wait around for other sibling operations to finish.
bool SkyCalRouter::delayDispense(AssayNode *n)
{
	bool goExecute = true;
	for (int i = 0; i < n->GetChildren().size(); ++i)
	{
		AssayNode *m = n->GetChildren()[i];
		for (int j = 0; j < m->GetParents().size(); ++j)
		{
			AssayNode *n2 = m->GetParents()[j];
			if (n2 != n && n2->type != DISPENSE && n2->status != COMPLETE)
			{
				return true;
			}
		}
	}
	return false;
}

// Computes the area area for droplets to move around. It takes a look at the area of the board
// and the active fixed modules.
void SkyCalRouter::computeAvailableArea()
{
	int totalArea = arch->getNumCellsX() * arch->getNumCellsY();

	vector< vector<bool> > busyMap = vector< vector<bool> >(arch->getNumCellsX(), vector<bool>(arch->getNumCellsY(), false));
	for (int i = 0; i < arch->getExternalResources()->size(); i++)
	{
		FixedModule *fm = arch->getExternalResources()->at(i);

		if (fm->getReady())
		{
			// Fill in busy cells including interference region around busy modules
			int left = max(0, fm->getLX() - 1);
			int top = max(0, fm->getTY() - 1);
			int right = min(arch->getNumCellsX() - 1, fm->getRX() + 1);
			int bottom = min(arch->getNumCellsY() - 1, fm->getBY() + 1);
			for (int x = left; x <= right; x++)
				for (int y = top; y <= bottom; y++)
				{
					busyMap[x][y] = true;
				}
		}
	}

	for (int i = 0; i < outRes->size(); ++i)
	{
		IoResource *res = outRes->at(i);
		//for (list<AssayNode *>::iterator it = res->schedule.begin(); it != res->schedule.end(); ++it)
		//{
		//	AssayNode *sched = *it;
		//
		//	if (sched->status != COMPLETE)

		bool used = false;
		for (list<AssayNode *>::iterator it = res->schedule.begin(); it != res->schedule.end(); ++it)
		{
			// Possible that an assay is set to COMPLETE before and after makes a difference?
			// No, Synchronous is still achieved here due to feasibility checking
			// previous cycle to not activate an unwanted mix near output ports
			AssayNode *sched = *it;

			if (sched->status != COMPLETE)
			{
				used = true;
				break;
			}
		}
		if (used)
			{
				IoPort *port = res->port;

				int posXY = port->getPosXY();
				int left = 0;
				int right = 0;
				int top = 0;
				int bottom = 0;
					switch (port->getSide())
					{
					case NORTH:
						left = max(posXY - 1, 0);
						right = min(posXY + 1, arch->getNumCellsX() - 1);
						top = 0;
						bottom = min(1, arch->getNumCellsY() - 1);
						break;
					case EAST:
						left = max(arch->getNumCellsX() - 2, 0);
						right = arch->getNumCellsX() - 1;
						top = max(posXY - 1, 0);
						bottom = min(posXY + 1, arch->getNumCellsY() - 1);
						break;
					case SOUTH:
						left = max(posXY - 1, 0);
						right = min(posXY + 1, arch->getNumCellsX() - 1);
						top = max(arch->getNumCellsY() - 2, 0);
						bottom = arch->getNumCellsY() - 1;
						break;
					case WEST:
						left = 0;
						right = min(1, arch->getNumCellsX() - 1);
						top = max(posXY - 1, 0);
						bottom = min(posXY + 1, arch->getNumCellsY() - 1);
						break;
					default:
						break;
					}

				for (int x = left; x <= right; ++x)
					for (int y = top; y <= bottom; ++y)
					{
						busyMap[x][y] = true;
					}
		//		break;
			}
		//}
	}

	/*
	 // Output the array of busy cells
	for (int i = 0; i < busyMap.size(); ++i)
	{
		for (int j = 0; j < busyMap[i].size(); ++j)
		{
			if (busyMap[i][j])
				--totalArea;
			cout << busyMap[i][j] << " ";
		}
		cout << endl;
	}
	*/
	availableArea = totalArea;
}

// Inserts the list of possible moves for a specific droplet. It ensures that a move does not interfere with another droplet
// and that it only selects the possible list of moves to leave a busy module.
void SkyCalRouter::insertFeasibleMoves(vector<Move> *moves, Droplet *d, AssayNode *n, map<Droplet *, vector<RoutePoint *> *> *routes)
{
	RoutePoint *p = (*routes)[d]->back();

	// Insert directions such that equally weighted moves
	// later on will not be fixed in this order
	vector<Direction> temp(5);
	temp[0] = DIR_UNINIT; // Don't Move
	temp[1] = NORTH;
	temp[2] = EAST;
	temp[3] = SOUTH;
	temp[4] = WEST;
	vector<Direction> directions;
	while (temp.size() > 0)
	{
		int r = rand() % temp.size();
		directions.push_back(temp[r]);
		temp[r] = temp[temp.size() - 1];
		temp.pop_back();
	}

	// Insert all available moves
	for (int dir = 0; dir < directions.size(); ++dir)
	{
		RoutePoint *nloc = step(p, directions[dir]); // Note: this allocates on heap
		if (!checkFeasibility(nloc, d, n, routes, true))
			delete nloc;
		else
			moves->push_back(Move(directions[dir], nloc));
	}
	// If no possible moves, reinsert with more flexibility on leaving busy modules
	if (moves->size() <= 0)
	{
		for (int dir = 0; dir < directions.size(); ++dir)
		{
			RoutePoint *nloc = step(p, directions[dir]);
			if (!checkFeasibility(nloc, d, n, routes, false))
				delete nloc;
			else
				moves->push_back(Move(directions[dir], nloc));
		}
	}
	// If no possible moves, reinsert the "Don't Move" as the only possibility
	if (moves->size() <= 0)
	{
		claim(true, "Impossible move\n");
		moves->push_back(Move(DIR_UNINIT, step(p, DIR_UNINIT)));
	}
}

///////////////////////////////////////////////////////////////////////
// Functions which involve operations on a droplet
// Each updates routes to reflect the current cycle for a droplet
// Each check for possible moves in each direction including itself
// Some return true whether the performance is done
///////////////////////////////////////////////////////////////////////
// Determines whether the output resource can a droplet without interfering with anything,
// if possible, updating routes for the droplet being dispensed and return true
bool SkyCalRouter::performDispense(Droplet *d, AssayNode* n, map<Droplet *, vector<RoutePoint *> *> *routes)
{
	RoutePoint *s = new RoutePoint();
	// Now do Left-Edge binding for inputs
	for (int i = 0; i < dispRes->size(); i++)
	{
		IoResource *dr = dispRes->at(i);
		if (n->GetPortName() == dr->name) // No need for a check
		{
			dr->schedule.push_back(n);
			n->ioPort = dr->port;
		}
	}

	RoutePoint temp;
	// Obtain IO Port's dispense location.
	if (n->GetIoPort()->getSide() == NORTH)
	{
		s->x = n->GetIoPort()->getPosXY();
		s->y = 0;
	}
	else if (n->GetIoPort()->getSide() == SOUTH)
	{
		s->x = n->GetIoPort()->getPosXY();
		s->y = arch->getNumCellsY()-1;
	}
	else if (n->GetIoPort()->getSide() == EAST)
	{
		s->x = arch->getNumCellsX()-1;
		s->y = n->GetIoPort()->getPosXY();
	}
	else if (n->GetIoPort()->getSide() == WEST)
	{
		s->x = 0;
		s->y = n->GetIoPort()->getPosXY();
	}
	else
	{
		if ((*routes)[d]->size() <= 0)
			cerr << "WARNING: WE GOING TO CRASH!!!\n";
		s->x = (*routes)[d]->back()->x;
		s->y = (*routes)[d]->back()->y; // last route point
	}

	// Check area around dispenser for contamination (feasibility) so a droplet
	// is not surrounded by contamination such that it can't move for wash droplets
	// to clean it up. This resolves a possible deadlock.
	bool feasible = true;
	int left = max(s->x - 1, 0);
	int right = min(s->x + 1, arch->getNumCellsX() - 1);
	int top = max(s->y - 1, 0);
	int bottom = min(s->y + 1, arch->getNumCellsY() - 1);
	for (int x = left; x <= right; ++x)
		for (int y = top; y <= bottom; ++y)
		{
			RoutePoint p;
			p.x = x;
			p.y = y;

			if (p.x == s->x && p.y == s->y)
			{
				// *** UPDATED TO RESOLVE DEADLOCK
				if (!checkFeasibility(&p, d, n, routes, true) ||
						(n->type != WASH && cells[x][y].contaminator != ""))
				{
					feasible = false;
					break;
				}
			}
			else
			{
				if (n->type != WASH && cells[x][y].contaminator != "")
				//if (willContaminate(&p, n))
				{
					feasible = false;
					break;
				}

			}
		}

	// Dispense the droplet if there is no interference
	if (feasible)
	{
#ifdef DEBUG_SKYCAL_OUTPUT
		cout << "IO: " << n->GetIoPort()->getPortName() << " at " << s->x << "," << s->y << endl;
#endif
		s->cycle = cycle;
		if (n->type == WASH)
			s->dStatus = DROP_WASH;
		(*routes)[d]->push_back(s);
		if (dropReference.find(d->uniqueFluidName) == dropReference.end())
			dropReference[d->uniqueFluidName] = dropReference.size();
		updateContaminationAt(s, d);

		d->isWashDroplet = n->GetIoPort()->isWashPort();
		//d->uniqueFluidName = n->GetIoPort()->getPortName();
		return true;
	}
	delete s;
	++noDelta;
	return false;
}

// Attempts to move the droplet towards the specified goal. It selects the best three moves that do not interfere with anything
// and randomly selects from the three.
void SkyCalRouter::performGoalMove(Droplet *d, RoutePoint *goal, AssayNode* n, map<Droplet *, vector<RoutePoint *> *> *routes)
{
	// Do not move if droplet is already at destination
	// also assumes d1 and d2 have a route point
	RoutePoint *p = (*routes)[d]->back();
	RoutePoint *dest = goal;
	if (p->x == dest->x && p->y == dest->y)
		return;

	// Populate the list of moves
	vector<Move> moves;
	insertFeasibleMoves(&moves, d, n, routes);

	// Assign values of each move based on a chosen heuristic
	for (int i = 0; i < moves.size(); ++i)
	{
		moves[i].value = manhattanDist(moves[i].result, dest);
	}
	sort(moves.begin(), moves.end());

	if (!dropProgress[d].inProgress)
	{
		dropProgress[d].beginCycle = cycle;
		dropProgress[d].expectedEndCycle = cycle + arch->getNumCellsX() + arch->getNumCellsY();
		dropProgress[d].inProgress = true;
	}
#ifdef WEIGHTS_ON_TIME
	if (dropProgress[d].isOnTime(cycle))
		setBestWeights();
	else if (dropProgress[d].isLate(cycle))
		setImprovedWeights();
	else
		setNeutralWeights();
#endif
	// Randomly select one of the first three candidates with a weight
	int selectedIndex = selectRandomMove(&moves);

	Move chosenMove = moves[selectedIndex];
	// Clean up rejected moves
	moves[selectedIndex] = moves[moves.size() - 1];
	moves.pop_back();
	for (int i = 0; i < moves.size(); ++i)
		delete moves[i].result;

	chosenMove.result->cycle = cycle;
	if (n->type == WASH)
		chosenMove.result->dStatus = DROP_WASH;
	(*routes)[d]->push_back(chosenMove.result);
	if (chosenMove.dir == DIR_UNINIT)
		++noDelta;
	updateContaminationAt(chosenMove.result, d);

#ifdef DEBUG_SKYCAL_OUTPUT
	cout << d->uniqueFluidName << " " << p->x << "," << p->y << " to " << chosenMove.result->x << "," << chosenMove.result->y << " (GOAL " << dest->x << "," << dest->y << ")" << endl;
#endif
}

// Attempts to move the droplet towards another droplet. It selects the best three moves that do not interfere with anything
// and randomly selects from the three.
// *** TODO: If two droplets are next to each other, they should automatically converge if they can. They shouldn't go further away at all
void SkyCalRouter::performMergeMove(Droplet *d1, Droplet *d2, AssayNode* n, map<Droplet *, vector<RoutePoint *> *> *routes)
{
	// Do not move if droplet is already at destination
	// also assumes d1 and d2 have a route point
	RoutePoint *p = (*routes)[d1]->back();
	RoutePoint *dest = (*routes)[d2]->back();
	if (p->x == dest->x && p->y == dest->y)
		return;

	// Populate the list of moves
	vector<Move> moves;
	insertFeasibleMoves(&moves, d1, n, routes);

	// Assign values of each move based on a chosen heuristic
	for (int i = 0; i < moves.size(); ++i)
	{
		moves[i].value = manhattanDist(moves[i].result, dest);
	}
	sort(moves.begin(), moves.end());

	if (!dropProgress[d1].inProgress)
	{
		dropProgress[d1].beginCycle = cycle;
		dropProgress[d1].expectedEndCycle = cycle + (arch->getNumCellsX() + arch->getNumCellsY()) / 2;
		dropProgress[d1].inProgress = true;
	}
#ifdef WEIGHTS_ON_TIME
	if (dropProgress[d1].isOnTime(cycle))
		setBestWeights();
	else if (dropProgress[d1].isLate(cycle))
		setImprovedWeights();
	else
		setNeutralWeights();
#endif
	// Randomly select one of the first three candidates with a weight
	int selectedIndex = selectRandomMove(&moves);

	Move chosenMove = moves[selectedIndex];
	// Clean up rejected moves
	moves[selectedIndex] = moves[moves.size() - 1];
	moves.pop_back();
	for (int i = 0; i < moves.size(); ++i)
		delete moves[i].result;

	chosenMove.result->cycle = cycle;
	(*routes)[d1]->push_back(chosenMove.result);
	if (chosenMove.dir == DIR_UNINIT)
		++noDelta;
	updateContaminationAt(chosenMove.result, d1);

#ifdef DEBUG_SKYCAL_OUTPUT
	cout << d1->uniqueFluidName << " " << p->x << "," << p->y << " to " << chosenMove.result->x << "," << chosenMove.result->y << " (FIND PARTNER)"<< endl;
#endif
}

// Attempts to perform mix for the specified droplet without interfering with anything. It hopes to do mix
// as best as it can when determining the best three moves. This is done by evaluating the benefits of moving
// forward, turning 90 degrees, turning 180 degrees, and not moving. It returns true if the mix is successful.
bool SkyCalRouter::performMixMove(Droplet *d, AssayNode* n, map<Droplet *, vector<RoutePoint *> *> *routes)
{
	RoutePoint *p = (*routes)[d]->back();

	// Populate the list of moves
	vector<Move> moves;
	insertFeasibleMoves(&moves, d, n, routes);

	// Assign values of each move based on a chosen heuristic
	for (int i = 0; i < moves.size(); ++i)
	{
		moves[i].value = computeMixingDegree(d, routes, moves[i].result);
	}
	sort(moves.begin(), moves.end());

	if (!dropProgress[d].inProgress)
	{
		dropProgress[d].beginCycle = cycle;
		dropProgress[d].expectedEndCycle = cycle + 300;
		dropProgress[d].inProgress = true;

		// For the first time around in a mix, just don't move.
		for (int i = 0; i < moves.size(); ++i)
			delete moves[i].result;
		performNoMove(d, n, routes);
		return false;
	}
#ifdef WEIGHTS_ON_TIME
	if (dropProgress[d].isOnTime(cycle))
		setBestWeights();
	else if (dropProgress[d].isLate(cycle))
		setImprovedWeights();
	else
		setNeutralWeights();
#endif
	// Randomly select one of the first three candidates with a weight
	int selectedIndex = selectRandomMove(&moves);

	Move chosenMove = moves[selectedIndex];
	// Clean up rejected moves
	moves[selectedIndex] = moves[moves.size() - 1];
	moves.pop_back();
	for (int i = 0; i < moves.size(); ++i)
		delete moves[i].result;

	Turning turn = determineDropletTurn(d, routes, chosenMove.result);
	chosenMove.result->cycle = cycle;
	(*routes)[d]->push_back(chosenMove.result);
	if (chosenMove.dir == DIR_UNINIT)
		++noDelta;
	updateContaminationAt(chosenMove.result, d);

	d->completion -= chosenMove.value;
	if( d->completion < 0 ) d->completion = 0;

#ifdef DEBUG_SKYCAL_OUTPUT
	cout << d->uniqueFluidName << " " << p->x << "," << p->y << " to " << chosenMove.result->x << "," << chosenMove.result->y << " (" << toString(turn) << ") Progress: " << d->completion << "%\n";
#endif
	return (d->completion >= 100.0);
}

// Attempts to perform split of the specified droplet by determining whether it can split vertically or horizontally without
// interfering with anything else. If successful, it returns true.
//
// The split operation is strict such that the intended operation does not waste time delaying a busy module. It must
// leave the busy module before splitting if this happens.
bool SkyCalRouter::performSplitMove(Droplet *d, AssayNode* n, map<Droplet *, vector<RoutePoint *> *> *routes)
{
	RoutePoint *p = (*routes)[d]->back();

	RoutePoint *up = step(p, NORTH);
	RoutePoint *down = step(p, SOUTH);
	RoutePoint *left = step(p, WEST);
	RoutePoint *right = step(p, EAST);;
	if (checkFeasibility(left, d, n, routes, true) && checkFeasibility(right, d, n, routes, true))
	{
		delete up; delete down;

		Droplet *drop = n->GetDroplets().back();
		Droplet *drop2 = new Droplet();
		drop2->uniqueFluidName = drop->uniqueFluidName;
		drop2->volume = drop->volume / 2;
		(*routes)[drop2] = new vector<RoutePoint *>();
		n->addDroplet(drop2);
		++numDroplets;

		left->cycle = cycle;
		(*routes)[drop2]->push_back(left);
		updateContaminationAt(left, drop2);

		right->cycle = cycle;
		(*routes)[drop]->push_back(right);
		updateContaminationAt(right, drop);

#ifdef DEBUG_SKYCAL_OUTPUT
		cout << d->uniqueFluidName << " " << p->x << "," << p->y << " to " << left->x << "," << left->y << " and " << right->x << "," << right->y << " (SPLIT)"<< endl;
#endif
		return true;
	}
	else if (checkFeasibility(up, d, n, routes, true) && checkFeasibility(down, d, n, routes, true))
	{
		delete left; delete right;

		Droplet *drop = n->GetDroplets().back();
		Droplet *drop2 = new Droplet();
		drop2->uniqueFluidName = drop->uniqueFluidName;
		drop2->volume = drop->volume / 2;
		(*routes)[drop2] = new vector<RoutePoint *>();
		n->addDroplet(drop2);
		++numDroplets;

		up->cycle = cycle;
		(*routes)[drop2]->push_back(up);
		updateContaminationAt(up, drop2);

		down->cycle = cycle;
		(*routes)[drop]->push_back(down);
		updateContaminationAt(down, drop);

#ifdef DEBUG_SKYCAL_OUTPUT
		cout << d->uniqueFluidName << " " << p->x << "," << p->y << " to " << up->x << "," << up->y << " and " << down->x << "," << down->y << " (SPLIT)"<< endl;
#endif
		return true;
	}
	else
	{
		delete up; delete down; delete left; delete right;
		performUnweightedMove(d, n, routes);
		return false;
	}
}

// Performs a move in any random direction (staying still included)
void SkyCalRouter::performUnweightedMove(Droplet *d, AssayNode* n, map<Droplet *, vector<RoutePoint *> *> *routes)
{
	RoutePoint *p = (*routes)[d]->back();

	// Populate the list of moves
	vector<Move> moves;
	insertFeasibleMoves(&moves, d, n, routes);

	int selectedIndex = rand() % moves.size();

	Move chosenMove = moves[selectedIndex];
	// Clean up rejected moves
	moves[selectedIndex] = moves[moves.size() - 1];
	moves.pop_back();
	for (int i = 0; i < moves.size(); ++i)
		delete moves[i].result;

	chosenMove.result->cycle = cycle;
	(*routes)[d]->push_back(chosenMove.result);
	if (n->type == WASH)
		chosenMove.result->dStatus = DROP_WASH;
	if (chosenMove.dir == DIR_UNINIT)
		++noDelta;
	updateContaminationAt(chosenMove.result, d);

#ifdef DEBUG_SKYCAL_OUTPUT
	cout << d->uniqueFluidName << " " << p->x << "," << p->y << " to " << chosenMove.result->x << "," << chosenMove.result->y << " (RANDOM)\n";
#endif
}

// Used to periodically update routes for active droplets that just don't move
void SkyCalRouter::performNoMove(Droplet *d, AssayNode* n, map<Droplet *, vector<RoutePoint *> *> *routes)
{
	// Sit still
	RoutePoint *mp = (*routes)[d]->back();
	RoutePoint *np = new RoutePoint();
	np->x = mp->x;
	np->y = mp->y;
	np->cycle = cycle;
	if (n->type == WASH)
		np->dStatus = DROP_WASH;
	(*routes)[d]->push_back(np);
	updateContaminationAt(np, d);

#ifdef DEBUG_SKYCAL_OUTPUT
	cout << d->uniqueFluidName << " " << np->x << "," << np->y << " (NO MOVE)" << endl;
#endif
}

void SkyCalRouter::performNoMoveUnforced(Droplet *d, AssayNode* n, map<Droplet *, vector<RoutePoint *> *> *routes)
{
	RoutePoint *p = (*routes)[d]->back();

	// Populate the list of moves
	vector<Move> moves;
	insertFeasibleMoves(&moves, d, n, routes);

	int selectedIndex = -1;
	for (int i = 0; i < moves.size(); ++i)
	{
		if (moves[i].dir == DIR_UNINIT)
		{
			selectedIndex = i;
			break;
		}
	}
	if (selectedIndex < 0)
		selectedIndex = rand() % moves.size();

	Move chosenMove = moves[selectedIndex];
	// Clean up rejected moves
	moves[selectedIndex] = moves[moves.size() - 1];
	moves.pop_back();
	for (int i = 0; i < moves.size(); ++i)
		delete moves[i].result;

	chosenMove.result->cycle = cycle;
	(*routes)[d]->push_back(chosenMove.result);
	if (n->type == WASH)
		chosenMove.result->dStatus = DROP_WASH;
	if (chosenMove.dir == DIR_UNINIT)
		++noDelta;
	updateContaminationAt(chosenMove.result, d);

	#ifdef DEBUG_SKYCAL_OUTPUT
		cout << d->uniqueFluidName << " " << p->x << "," << p->y << " to " << chosenMove.result->x << "," << chosenMove.result->y << " (NO MOVE UNFORCED)\n";
	#endif
}

///////////////////////////////////////////////////////////////////////
// The following function processes a specific Assay for a given cycle
///////////////////////////////////////////////////////////////////////
// The massive function is organized as follows:
//
// if (n == OPERATION_TYPE1)
// {
//    switch (n->status)
//	  {
//    case ASSAY_STATUS1:
//		...
//    case ASSAY_STATUSN:
//    }
// }
// ...
// else if (n == OPERATION_TYPEN)
// {
//    switch (n->status)
//	  {
//    case ASSAY_STATUS1:
//		...
//    c	ase ASSAY_STATUSN:
//    }
// }
//
// The function executes a cycle of an Assay based on its status and type of operation.
// The function will return the Assay's status for determining its progress.
// (i.e. a COMPLETE return value means the Assay's operation is done.)
//
// Currently, Assays progress in these states:
// SCHEDULED -> EXECUTING -> COMPLETE
//  (init)   ->   (op)    ->  (wait)
AssayNodeStatus SkyCalRouter::processAssay(AssayNode* n, bool allowProcessing,
		DAG *dag, DmfbArch *arch,
		vector<ReconfigModule *> *rModules,
		map<Droplet *, vector<RoutePoint *> *> *routes,
		vector<vector<int> *> *pinActivations,
		vector<unsigned long long> *tsBeginningCycle)
{
	// Determines whether the assay has been executed this cycle already, if so, skip this function call
	if (executionTable.find(n) != executionTable.end())
		return n->status;
	executionTable[n] = true; // Adds this assay to the table, the boolean value is redundant

	// combo Allows immediate execution of another case statement if necessary for certain
	// operation types and status so we can do it all in one cycle.
	bool combo = true;
	while (combo)
	{
		combo = false;
		if (n->GetType() == DISPENSE)
		{
			switch (n->status)
			{
			case UNBOUND_UNSCHED:
				// Should never occur
				break;
			case SCHEDULED:
			{
				// Create new droplets to be input soon, then go to dispensing the droplet
				Droplet *d = new Droplet();
				d->uniqueFluidName = n->GetPortName();
				d->volume = n->GetVolume();
				//d->isWashDroplet = n->GetIoPort()->isWashPort();
				(*routes)[d] = new vector<RoutePoint *>();
				n->addDroplet(d);
				n->status = EXECUTING;

				combo = true;
				break;
			}
			case BOUND:
				// Used as a hashtag for candidateOps efficiency
				break;
			case EXECUTING:
			{
				// Dispense a droplet assuming there is only one droplet in a Dispense Assay Node
				Droplet *d = n->GetDroplets().back();

				if (allowProcessing && !delayDispense(n) && performDispense(d, n, routes))
				{
					++numDroplets;
					n->status = COMPLETE;
					n->startTimeStep = cycle / 100;
					n->endTimeStep = cycle / 100;
					n->seconds = n->endTimeStep - n->startTimeStep;
				}

				break;
			}
			case ROUTED:
				// Unused
				break;
			case COMPLETE:
			{
				for (int i = 0; i < n->GetDroplets().size(); ++i)
				{
						Droplet *d = n->GetDroplets().at(i);
						performUnweightedMove(d, n, routes);
				}
				break;
			}
			default:
				break;
			}
		}
		else if (n->GetType() == MIX)
		{
			switch (n->status)
			{
			case UNBOUND_UNSCHED:
				// Should never occur
				break;
			case SCHEDULED:
			{
				// *** TODO: What happens if 3 or more droplets in a MIX operation? How do we do manhattan then? Center of Mass?
				// Send 2 droplets toward each other, does not handle more than that, maybe that should be considered...
				for (int i = 0; i <= 1; ++i)
				{
					Droplet *d1 = NULL; // Droplet we are considering to move
					Droplet *d2 = NULL; // Droplet we are moving towards
					if (i == 0)
					{
						d1 = n->GetDroplets().at(0);
						d2 = n->GetDroplets().at(1);
					}
					else
					{
						d1 = n->GetDroplets().at(1);
						d2 = n->GetDroplets().at(0);
					}
					// Send droplet d1 towards d2's location
					performMergeMove(d1, d2, n, routes);
				}

				// Set next state to mix the droplets when they have merged
				bool merged = false;
				if (true)
				{
					Droplet *d1 = n->GetDroplets().at(0);
					Droplet *d2 = n->GetDroplets().at(1);

					RoutePoint *p = (*routes)[d1]->back();
					RoutePoint *dest = (*routes)[d2]->back();
					if (p->x == dest->x && p->y == dest->y)
					{
						merged = true;
						dropProgress[d1].inProgress = false;
						dropProgress[d2].inProgress = false;
					}
				}

				if (merged)
				{
					// Merge the droplets into one, to avoid making a new copy, use the droplet with the lowest
					// id and remove any other droplet from existence.
					Droplet *drop = n->GetDroplets().back();
					n->droplets.pop_back();
					while(!n->GetDroplets().empty())
					{
						Droplet *d2 = n->droplets.back();
						if (d2->id < drop->id)
						{
							d2->volume += drop->volume;
							d2->uniqueFluidName += ("+" + drop->uniqueFluidName);
							(*routes)[drop]->back()->dStatus = DROP_MERGING;
							drop = d2;
						}
						else
						{
							drop->volume += n->GetDroplets().back()->volume;
							drop->uniqueFluidName += ( "+" + n->GetDroplets().back()->uniqueFluidName);
							(*routes)[d2]->back()->dStatus = DROP_MERGING;
						}
						n->droplets.pop_back();
						--numDroplets;
					}
					drop->completion = 0.0;
					n->droplets.push_back(drop);
					if (dropReference.find(drop->uniqueFluidName) == dropReference.end())
						dropReference[drop->uniqueFluidName] = dropReference.size();
					updateContaminationAt((*routes)[drop]->back(), drop);

					n->startTimeStep = cycle / 100;
					n->status = EXECUTING;
				}

				break;
			}
			case BOUND:
				// Used as a hashtag for candidateOps efficiency
				break;
			case EXECUTING:
			{
				// Perform mix procedure
				Droplet *d = n->GetDroplets().back(); // Droplet we are considering to move

				if (performMixMove(d, n, routes))
				{
					dropProgress[d].inProgress = false;

					n->status = COMPLETE;
					n->endTimeStep = cycle / 100;
					n->seconds = n->endTimeStep - n->startTimeStep;
				}
				break;
			}
			case ROUTED:
				break;
			case COMPLETE:
			{
				for (int i = 0; i < n->GetDroplets().size(); ++i)
				{
					Droplet *d = n->GetDroplets().at(i);
					performUnweightedMove(d, n, routes);
				}
				break;
			}
			default:
				break;
			}
		}
		else if (n->GetType() == OUTPUT)
		{
			switch (n->status)
			{
			case UNBOUND_UNSCHED:
				break;
			case SCHEDULED:
			{
				// Find the closest IO port to assign
				Droplet *d = n->GetDroplets().back();
				IoResource *dr = findOutputPort(d, n, routes);
				if (dr)
				{
					dr->schedule.push_back(n);
					n->ioPort = dr->port;
				}
				else
					claim(false, "No Output Port exists");
				/*
				// Now do left-edge binding on output
				for (int i = 0; i < outRes->size(); i++)
				{
					IoResource *dr = outRes->at(i);
					if (n->GetPortName() == dr->name) // No need for a check
					{
						dr->schedule.push_back(n);
						n->ioPort = dr->port;
					}
				}
				*/
				recomputationNecessary = true;
				n->status = EXECUTING;
				combo = true;
				break;
			}
			case BOUND:
				break;
			case EXECUTING:
			{
				for (int i = 0; i < n->GetDroplets().size(); ++i)
				{
					Droplet *d = n->GetDroplets().at(i);

					RoutePoint *p = (*routes)[d]->back();
					RoutePoint *dest = new RoutePoint();
					// Obtain IO Port's dispense location.
					if (n->GetIoPort()->getSide() == NORTH)
					{
						dest->x = n->GetIoPort()->getPosXY();
						dest->y = 0;
					}
					else if (n->GetIoPort()->getSide() == SOUTH)
					{
						dest->x = n->GetIoPort()->getPosXY();
						dest->y = arch->getNumCellsY()-1;
					}
					else if (n->GetIoPort()->getSide() == EAST)
					{
						dest->x = arch->getNumCellsX()-1;
						dest->y = n->GetIoPort()->getPosXY();
					}
					else if (n->GetIoPort()->getSide() == WEST)
					{
						dest->x = 0;
						dest->y = n->GetIoPort()->getPosXY();
					}
					else
					{
						if ((*routes)[d]->size() <= 0)
							cerr << "WARNING: WE GOING TO CRASH!!!\n";
						dest->x = (*routes)[d]->back()->x;
						dest->y = (*routes)[d]->back()->y; // last route point
					}

					// If droplet is at the output terminal, remove it from the list.
					// Otherwise, move it towards output terminal.
					if (p->x == dest->x && p->y == dest->y)
					{
						dropProgress[d].inProgress = false;

						p->dStatus = DROP_OUTPUT;
						n->eraseFromDropList(d);
						--numDroplets;

#ifdef DEBUG_SKYCAL_OUTPUT
						cout << d->uniqueFluidName << " " << p->x << "," << p->y << " -- OUTPUTTING" << endl;
#endif
					}
					else
					{
						performGoalMove(d, dest, n, routes);
					}
					delete dest;
				}
				if (n->GetDroplets().size() <= 0)
				{
					n->status = COMPLETE;
					recomputationNecessary = true;
					n->startTimeStep = cycle / 100;
					n->endTimeStep = cycle / 100;
					n->seconds = n->endTimeStep - n->startTimeStep;
				}
				break;
			}
			case ROUTED:
				break;
			case COMPLETE:
			{
				for (int i = 0; i < n->GetDroplets().size(); ++i)
				{
					Droplet *d = n->GetDroplets().at(i);
					performUnweightedMove(d, n, routes);
				}
				break;
			}
			default:
				break;
			}
		}
		else if (n->GetType() == DETECT)
		{
			switch (n->status)
			{
			case UNBOUND_UNSCHED:
				break;
			case SCHEDULED:
			{
				// Find a free DETECT module available
				if (n->boundedResType == UNKNOWN_RES)
				{
					Droplet *d = n->GetDroplets().back();
					FixedModule *module = findModule(d, n, routes);
					if (module)
					{
						n->boundedResType = module->getResourceType();
						availRes[module->getResourceType()]--;

						n->boundedExternalRes = module;
						n->startCycle = cycle;
						module->setReady(true);
						recomputationNecessary = true;
						combo = true;
					}
					else
						performUnweightedMove(d, n, routes);
				}
				else if (!n->boundedExternalRes->getBusy())
				{
					Droplet *d = n->GetDroplets().back();
					FixedModule *module = n->boundedExternalRes;
					int left = max(0, module->getLX());
					int top = max(0, module->getTY());
					int right = min(arch->getNumCellsX() - 1, module->getRX());
					int bottom = min(arch->getNumCellsY() - 1, module->getBY());
					if (areaIsClean(n, left, right, top, bottom))
					{
						module->setBusy(true);
						combo = true;
					}
					else
						performUnweightedMove(d, n, routes);
				}
				else
				{
					Droplet *d = n->GetDroplets().back();
					RoutePoint *p = (*routes)[d]->back();
					RoutePoint *dest = new RoutePoint();
					dest->x = n->GetBoundedExternalRes()->getLX();
					dest->y = n->GetBoundedExternalRes()->getBY();
					if (p->x == dest->x && p->y == dest->y)
					{
						dropProgress[d].inProgress = false;

						// If droplet is at the detect goal, begin executing the module
						// as long as other droplets.
						if (moduleIsClear(n->GetBoundedExternalRes(), n, routes))
						{
							d->completion = 0.0;
							n->startTimeStep = cycle / 100;
							n->status = EXECUTING;
							combo = true;
						}
						else
						{
							performNoMove(d, n, routes);
							++noDelta;
						}
					}
					else
					{
						performGoalMove(d, dest, n, routes);
					}
					delete dest;
				}
				break;
			}
			case BOUND:
				break;
			case EXECUTING:
			{
				Droplet *d = n->GetDroplets().back();
				performNoMove(d, n, routes);

				RoutePoint *p = (*routes)[d]->back();
				p->dStatus = DROP_PROCESSING;

				double conv = n->seconds / (arch->getSecPerTS() / cyclesPerTS);
				d->completion += (1 / conv) * 100;

				if (d->completion >= 100.0)
				{
					n->endTimeStep = cycle / cyclesPerTS;
					n->seconds = n->endTimeStep - n->startTimeStep;
					n->status = COMPLETE;
					n->cycles = cycle - n->startCycle + 1;
					FixedModule *fm = n->boundedExternalRes;

					// Set up a reconfigurable module for the resource so it draws it on the visualizer
					ReconfigModule *rm = new ReconfigModule(D_RES, fm->getLX(), fm->getTY(), fm->getRX(), fm->getBY());
					rm->boundNode = n;
					rm->startTimeStep = n->startCycle;
					rm->endTimeStep = cycle;
					rModules->push_back(rm);

					availRes[n->boundedResType]++;
					n->boundedExternalRes->setBusy(false);
					n->boundedExternalRes->setReady(false);
					recomputationNecessary = true;
					n->boundedExternalRes = NULL;
				}

				break;
			}
			case ROUTED:
				break;
			case COMPLETE:
			{
				for (int i = 0; i < n->GetDroplets().size(); ++i)
				{
					Droplet *d = n->GetDroplets().at(i);
					performUnweightedMove(d, n, routes);
				}
				break;
			}
			default:
				break;
			}
		}
		else if (n->GetType() == HEAT)
		{
			// *** TODO: Fill this in when DETECT is finalized
			switch (n->status)
			{
			case UNBOUND_UNSCHED:
				break;
			case SCHEDULED:
				break;
			case BOUND:
				break;
			case EXECUTING:
				break;
			case ROUTED:
				break;
			case COMPLETE:
				break;
			default:
				break;
			}
		}
		else if (n->GetType() == SPLIT)
		{
			switch (n->status)
			{
			case UNBOUND_UNSCHED:
				break;
			case SCHEDULED:
			{
				n->status = EXECUTING;
				combo = true;
				break;
			}
			case BOUND:
				break;
			case EXECUTING:
			{
				Droplet *d = n->GetDroplets().back();
				if (allowProcessing)
				{
					if (performSplitMove(d, n, routes)) // numDroplets is updated in here.
					{
						n->startTimeStep = cycle / 100;
						n->endTimeStep = cycle / 100;
						n->seconds = n->endTimeStep - n->startTimeStep;
						n->status = COMPLETE;
					}
				}
				else
				{
					performUnweightedMove(d, n, routes);
				}

				break;
			}
			case ROUTED:
				break;
			case COMPLETE:
			{
				for (int i = 0; i < n->GetDroplets().size(); ++i)
				{
					Droplet *d = n->GetDroplets().at(i);
					performUnweightedMove(d, n, routes);
				}
				break;
			}
			default:
				break;
			}
		}
		else if (n->GetType() == WASH)
		{
			if (n->status == SCHEDULED)
			{
				Droplet *d = new Droplet();
				d->uniqueFluidName = n->GetPortName();
				d->volume = n->GetVolume();
				(*routes)[d] = new vector<RoutePoint *>();
				n->addDroplet(d);
				n->status = EXECUTING;
				combo = true;
			}
			else
			{
				Droplet *d = n->GetDroplets().back();
				RoutePoint *dest = NULL;

				if (d->hp > 0)
				{
					dest = getNextWashDropletDest(n, d, routes);
					if (dest != NULL)
					{
						// Go clean
						if ((*routes)[d]->size() <= 0 || (*routes)[d]->back()->dStatus == DROP_WASTE)
						{
							if (performDispense(d, n, routes))
							{
								++numDroplets;
								n->ioPort = NULL;
							}
						}
						else
						{
							performGoalMove(d, dest, n, routes);
						}
					}
					else
					{
						// No more places to clean
						if ((*routes)[d]->size() <= 0 || (*routes)[d]->back()->dStatus == DROP_WASTE)
						{
							if (assayComplete)
							{
								// *** MERGE WITH DAN
								// Deals with the case where the wash droplet is not on the board,
								// but still exists and the assay is finished.
								// so we need to erase this special case having no need to
								// find an output reservoir, but exists in the droplet list
								n->eraseFromDropList(d);
								if (n->GetDroplets().size() <= 0)
								{
									n->status = COMPLETE;

									recomputationNecessary = true;
									n->startTimeStep = cycle / 100;
									n->endTimeStep = cycle / 100;
									n->seconds = n->endTimeStep - n->startTimeStep;
								}
							}
						}
						else
						{
							if (!assayComplete)
							{
								//performNoMove(d, n, routes);
								performUnweightedMove(d, n, routes);
								/*
								int lx; int rx; int ty; int by;
								getPartitionRect(n->reconfigMod->getTileNum(), lx, rx, ty, by);

								RoutePoint *s = (*routes)[d]->back();
								int midx = (lx + rx) / 2;
								int midy = (ty + by) / 2;
								dest = new RoutePoint();
								dest->x = midx;
								dest->y = midy;

								if (s->x == dest->x && s->y == dest->y)
								{
									performNoMoveUnforced(d, n, routes);
									dropProgress[d].inProgress = false;
								}
								else
									performGoalMove(d, dest, n, routes);
								delete dest;
								*/
							}
							else
							{
								if (n->ioPort == NULL)
								{
									// Find the closest IO port to assign
									IoResource *dr = findOutputPort(d, n, routes, true);
									if (dr)
									{
										dr->schedule.push_back(n);
										n->ioPort = dr->port;
									}
									else
										claim(false, "No Waste Port exists");
								}

								for (int i = 0; i < n->GetDroplets().size(); ++i)
								{
									d = n->GetDroplets().at(i);

									RoutePoint *p = (*routes)[d]->back();
									dest = new RoutePoint();
									// Obtain IO Port's dispense location.
									if (n->GetIoPort()->getSide() == NORTH)
									{
										dest->x = n->GetIoPort()->getPosXY();
										dest->y = 0;
									}
									else if (n->GetIoPort()->getSide() == SOUTH)
									{
										dest->x = n->GetIoPort()->getPosXY();
										dest->y = arch->getNumCellsY()-1;
									}
									else if (n->GetIoPort()->getSide() == EAST)
									{
										dest->x = arch->getNumCellsX()-1;
										dest->y = n->GetIoPort()->getPosXY();
									}
									else if (n->GetIoPort()->getSide() == WEST)
									{
										dest->x = 0;
										dest->y = n->GetIoPort()->getPosXY();
									}
									else
									{
										if ((*routes)[d]->size() <= 0)
											cerr << "WARNING: WE GOING TO CRASH!!!\n";
										dest->x = (*routes)[d]->back()->x;
										dest->y = (*routes)[d]->back()->y; // last route point
									}

									// If droplet is at the output terminal, remove it from the list.
									// Otherwise, move it towards output terminal.
									if (p->x == dest->x && p->y == dest->y)
									{
										p->dStatus = DROP_WASTE;
										n->eraseFromDropList(d);
										n->ioPort = NULL;
										--numDroplets;
	#ifdef DEBUG_SKYCAL_OUTPUT
										cout << d->uniqueFluidName << " " << p->x << "," << p->y << " -- WASTING" << endl;
	#endif
									}
									else
									{
										performGoalMove(d, dest, n, routes);
									}

									delete dest;
								}
								if (n->GetDroplets().size() <= 0)
								{
									n->status = COMPLETE;

									recomputationNecessary = true;
									n->startTimeStep = cycle / 100;
									n->endTimeStep = cycle / 100;
									n->seconds = n->endTimeStep - n->startTimeStep;
								}
							}
						}
					}
				}
				else
				{
					if (n->ioPort == NULL)
					{
						// Find the closest IO port to assign
						IoResource *dr = findOutputPort(d, n, routes, true);
						if (dr)
						{
							dr->schedule.push_back(n);
							n->ioPort = dr->port;
						}
						else
							claim(false, "No Waste Port exists");
					}

					for (int i = 0; i < n->GetDroplets().size(); ++i)
					{
						d = n->GetDroplets().at(i);

						RoutePoint *p = (*routes)[d]->back();
						dest = new RoutePoint();
						// Obtain IO Port's dispense location.
						if (n->GetIoPort()->getSide() == NORTH)
						{
							dest->x = n->GetIoPort()->getPosXY();
							dest->y = 0;
						}
						else if (n->GetIoPort()->getSide() == SOUTH)
						{
							dest->x = n->GetIoPort()->getPosXY();
							dest->y = arch->getNumCellsY()-1;
						}
						else if (n->GetIoPort()->getSide() == EAST)
						{
							dest->x = arch->getNumCellsX()-1;
							dest->y = n->GetIoPort()->getPosXY();
						}
						else if (n->GetIoPort()->getSide() == WEST)
						{
							dest->x = 0;
							dest->y = n->GetIoPort()->getPosXY();
						}
						else
						{
							if ((*routes)[d]->size() <= 0)
								cerr << "WARNING: WE GOING TO CRASH!!!\n";
							dest->x = (*routes)[d]->back()->x;
							dest->y = (*routes)[d]->back()->y; // last route point
						}

						// If droplet is at the output terminal, remove it from the list.
						// Otherwise, move it towards output terminal.
						if (p->x == dest->x && p->y == dest->y)
						{
							p->dStatus = DROP_WASTE;
							d->hp = Droplet::HP_INIT;
							n->ioPort = NULL;
							--numDroplets;
#ifdef DEBUG_SKYCAL_OUTPUT
							cout << d->uniqueFluidName << " " << p->x << "," << p->y << " -- WASTING" << endl;
#endif
						}
						else
						{
							performGoalMove(d, dest, n, routes);
						}

						delete dest;
					}
				}
			}
		}
	}
	return n->status;
}

///////////////////////////////////////////////////////////////////////
// The preroute, route, and postroute functions
///////////////////////////////////////////////////////////////////////
void SkyCalRouter::route(DAG *dag, DmfbArch *arch,
		vector<ReconfigModule *> *rModules,
		map<Droplet *, vector<RoutePoint *> *> *routes,
		vector<vector<int> *> *pinActivations,
		vector<unsigned long long> *tsBeginningCycle) {

	// Get first wash port
	washPort = NULL;
	for (unsigned i = 0; i < arch->getIoPorts()->size(); i++)
	{
		IoPort *p = arch->getIoPorts()->at(i);
		if (p->isAnInput() && p->isWashPort())
		{
			washPort = p;
			break;
		}
	}
	claim(washPort != NULL, "There is no valid input wash port on the DMFB. Please add one in the DMFB architecture input file.");

	int seed = time(NULL);
	//seed = 1374260671;
	//seed = 1394814266; // Deadlock issue for wash if wash drops don't move
	//seed = 1394814688; // Deadlock issue for wash, congested weights
	//seed = 1394868396; // Deadlock issue for wash, heavily congested board in vitro

	//seed = 1396583646; // investigating
	srand(seed);

	this->arch = arch;

	// Initialize Module Resources and IO resources
	for (int i = 0; i < dag->getAllNodes().size(); i++)
	{
		AssayNode *n = dag->getAllNodes().at(i);
		n->boundedResType = UNKNOWN_RES;
		n->boundedExternalRes = NULL;
	}
	resetIoResources(arch);

	dropProgress.clear();

	if (washEnabled)
	{
		initWashAssay();
		for (int i = 0; i < washOps->getAllNodes().size(); ++i)
			candidateOps->push_back(washOps->getAllNodes().at(i));
	}

	// Initialize candidateOps
	commissionDAG(dag);

	Priority::setAsCritPathDist(dag, arch);

	if (washEnabled)
	{
		for (int i = 0; i < washOps->getAllNodes().size(); ++i)
			washOps->getAllNodes().at(i)->priority = 999999;
	}
	Sort::sortNodesByPriorityLoFirst(candidateOps); // MLS_INC

	// Used to notify a setting that modules are set by cycles, not time steps
	//rModules->push_back(NULL); // DTG done with router type now

	// Initialize Status for all nodes to be undefined
	for (int i = 0; i < dag->getAllNodes().size(); i++)
	{
		AssayNode *n = dag->getAllNodes().at(i);
		n->status = UNBOUND_UNSCHED;

#ifdef DEBUG_SKYCAL_OUTPUT
		cout << i << ": " << n->priority << endl;
#endif
	}
	// Bound is used to insert an Assay into candidateOps only once
	for (list<AssayNode*>::iterator it = candidateOps->begin(); it != candidateOps->end(); it++)
	{
		AssayNode *cur = *it;
		cur->status = BOUND;
	}
	// Fake scheduled for dispense nodes not in candidateOps
	// any scheduled nodes that are parents of candidateOps will be processed each cycle.
	// similarly, candidateOps who are sink nodes and are scheduled will be processed each cycle.
	for (int i = 0; i < dag->getAllNodes().size(); i++)
	{
		AssayNode *n = dag->getAllNodes().at(i);
		if (n->GetParents().size() <= 0)
			n->status = SCHEDULED;
	}

	// Initialize 2-D grid architecture with heaters/detects/ect.
	initCellTypeArray();
	setGrowthFactors(dag);

	assayComplete = false;

	cyclesPerTS = arch->getSecPerTS() * arch->getFreqInHz();
	startingTS = 0; // The TS that is getting ready to begin (droplets are being routed to)
	routeCycle = 0;
	cycle = 0;

#ifdef DEBUG_SKYCAL_PROGRESS_CHECK
	int lastCycle = -1;
	int dangerNumCycles = 10000;
	int growthValuePrevious = -1;
	int availableAreaPrevious = -1;
#endif
	///////////////////////////////////////////////////////////////////////
	// This is the main loop. Each iteration of this loop solves one
	// time-step (routing sub-problem)
	///////////////////////////////////////////////////////////////////////

	noDelta = 0; //reset noDelta value
	int numDropletsPrevious = 0;
	while(moreNodesToSchedule())
	{
#ifdef DEBUG_SKYCAL_OUTPUT
		cout << "*************** Cycle: " << cycle << endl;
#endif
		executionTable.clear();

		if (recomputationNecessary)
		{
			computeAvailableArea();
			recomputationNecessary = false;
		}

		noDelta = 0;
		int progress = 0;
		int growthValue = 0;
		list<AssayNode*> newOps;
		bool washOpsOnly = true;
		for (list<AssayNode*>::iterator it = candidateOps->begin(); it != candidateOps->end();)
		{
			AssayNode *cur = *it;

			// This check is to prevent droplet congestion on the board.
			// It is used to avoid dispensing and splitting.
			bool allowProcessing = true;
			int gf = growthFactors[cur];
			if (growthValue >= availableArea / 8)
				allowProcessing = false;
			else
				growthValue += gf;
			progress += gf;

			if (cur->status == BOUND)
			{
				// This case deals with nodes with children and are not roots
				// Roots are executed by intermediate nodes
				bool predecessorsComplete = true;
				for (int p = 0; p < cur->GetParents().size(); p++)
				{
					AssayNode *par = cur->GetParents().at(p);
					if (par->type != WASH)
						washOpsOnly = false;

					AssayNodeStatus stat = processAssay(par, allowProcessing, dag, arch,
							rModules, routes, pinActivations, tsBeginningCycle);
					//cout << "optype: " << par->GetType() << " " << par->id << " " << stat << endl;

					if (stat != COMPLETE)
						predecessorsComplete = false;
				}
				// If all predecessors are complete, that means the current node can start to execute
				if (predecessorsComplete)
				{
					cur->status = SCHEDULED;
					executionTable[cur] = false; // Avoid critical paths falling behind to execute twice due to Child node in candidateOps

					// Transfer a droplet to next node (Only split would have more than one droplet at completion stage)
					for (int p = 0; p < cur->GetParents().size(); p++)
					{
						AssayNode *par = cur->GetParents().at(p);
						Droplet* pd = par->droplets.back();
						par->droplets.pop_back();

						cur->addDroplet(pd);
					}

					// Remove the current node from candidate ops and add its successor
					if (cur->GetChildren().size() > 0)
					{
						it++;
						candidateOps->remove(cur);
						for (int s = 0; s < cur->GetChildren().size(); ++s)
						{
							AssayNode *succ = cur->GetChildren().at(s);

							// Ensure we don't add a successor twice.
							if (succ->status != BOUND)
							{
								newOps.push_back(succ);
								succ->status = BOUND;
							}
						}
					}
					else it++;
				}
				else it++;
			}
			else //if (cur->status != BOUND)
			{
				if (cur->type != WASH)
					washOpsOnly = false;

				// This case deals with sink nodes which are nodes with no children
				AssayNodeStatus stat = processAssay(cur, allowProcessing, dag, arch,
						rModules, routes, pinActivations, tsBeginningCycle);

				if (stat == COMPLETE)
				{
					it++;
					candidateOps->remove(cur);
				}
				else it++;
			}
		}
		if (washOpsOnly)
			assayComplete = true;

		// Add the new operations to candidateOps processing the current iteration of candidateOps
		for (list<AssayNode *>::iterator it = newOps.begin(); it != newOps.end(); it++)
			candidateOps->push_back(*it);
		if (newOps.size() > 0)
			Sort::sortNodesByPriorityLoFirst(candidateOps); // MLS_INC

#ifdef DEBUG_SKYCAL_OUTPUT
		cout << "Growth/Limit Value: " << growthValue << " " << availableArea / 8 << endl;
#endif
#ifdef DEBUG_SKYCAL_PROGRESS_CHECK
		if (lastCycle + dangerNumCycles < cycle || growthValuePrevious != growthValue || availableAreaPrevious != availableArea)
		{
			cout << "***************\n";
			cout << "*************** Cycle: " << cycle << endl;
			int count = 0;
			for (map<AssayNode *, bool>::iterator it = executionTable.begin(); it != executionTable.end(); ++it)
				if (it->second)
				{
					cout << "ID: " << it->first->id << " - " << it->first->name << endl;
					++count;
				}
			cout << "Progress: " << progress << endl;
			cout << "Growth/Limit Value: " << growthValue << " " << availableArea / 8 << endl;
			cout << "Number of Ops: " << count << endl;
			cout << "Number of Droplets: " << numDropletsPrevious << " to " << numDroplets << endl;


			if (lastCycle + dangerNumCycles < cycle)
				cout << "WARNING: 10000 CYCLES HAVE PASSED." << endl;
			lastCycle = cycle;
			cout << "*************** Cycle: " << cycle << endl;
		}
		growthValuePrevious = growthValue;
		availableAreaPrevious = availableArea;
#endif

		/*
		int previousThresholdCount = underDeadlockThresholdCount;
		if( numDropletsPrevious && (1 -(noDelta / (double)numDropletsPrevious)) < deadlockThreshold )
		{
			++underDeadlockThresholdCount;
		}else{
			// we had a move that made progress, but don't want to completely reset
			// underThrsholdCount in case it was a local minimum
			underDeadlockThresholdCount /= 2;
		}
		//claim(underDeadlockThresholdCount < 100, "Probable Deadlock\n");
		if (previousThresholdCount != underDeadlockThresholdCount)
		{
			cout << "No Delta:" << noDelta << " | Percentage moving: "
					<< (numDropletsPrevious == 0 ? 0 : (1 -(noDelta / (double)numDropletsPrevious)) ) * 100
					<< " | No Droplets: " << numDroplets
					<< " | Cycles Under Threshold: " << underDeadlockThresholdCount << endl;
		}
		numDropletsPrevious = numDroplets;
		*/
		++cycle;
		//if (cycle == 6500) // Used to terminate early and debug
		//	break;
	}

	sort(rModules->begin(), rModules->end(), compareStartTS);

	/*
	//Post-Assay wash-droplet cleaning
	EulerGraph g(arch->getNumCellsX(), arch->getNumCellsY());
	list<int> washPath = g.getEulerPath();

	Droplet *washDrop = new Droplet();
	washDrop->uniqueFluidName = "Water";
	washDrop->volume = 10;
	(*routes)[washDrop] = new vector<RoutePoint *>();

	for( list<int>::iterator it = washPath.begin(); it != washPath.end(); ++it )
	{
		int pos = *it;
		RoutePoint* ret = new RoutePoint();
		ret->x = pos % arch->getNumCellsX(); //get x coord from index
		ret->y = pos / arch->getNumCellsX(); //get y coord from index
		ret->dStatus = DROP_WASH;
		(*routes)[washDrop]->push_back(ret);
		ret->cycle = cycle;
		cycle++;
	}
	*/

	// To get simulation to run, we do this such that one route is the entire flow
	for (unsigned i = 0; i < cycle; i += cyclesPerTS)
	{
		tsBeginningCycle->push_back(i);
	}

	cout << "Total Cycles: " << cycle << endl;
	cout << "Seed: " << seed << endl;
}

void SkyCalRouter::preRoute(DmfbArch *arch)
{
}

void SkyCalRouter::postRoute()
{
}

///////////////////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////////////////
SkyCalRouter::~SkyCalRouter()
{
}

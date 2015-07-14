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
 * Source: bioroute_router.cc													*
 * Original Code Author(s): Benjamin Preciado									*
 * Original Completion/Release Date: October 7, 2012							*
 *																				*
 * Details: N/A																	*
 *																				*
 * Revision History:															*
 * WHO		WHEN		WHAT													*
 * ---		----		----													*
 * FML		MM/DD/YY	One-line description									*
 *-----------------------------------------------------------------------------*/

#include "../../Headers/Router/bioroute_router.h"
#include <iostream>
#include <string>
#include <queue>
#include <ctime>
#include <map>
#include <limits>
#include <algorithm>

void Node::clearLast()
{
    for (unsigned i = 0; i < last.size(); i++)
        while (!last.at(i).empty())
            last.at(i).pop_back();
    while (!last.empty())
        last.pop_back();
}

void Node::pushLast(vector< vector<int> > H)
{
    for (unsigned i = 0; i < H.size(); i++)
    {
        vector<int> temp = H.at(i);
        last.push_back(temp);
    }
}

void Node::updateHistory(int h)
{
    for (unsigned i = 0; i < last.size(); i++)
    {
        vector<int> l = last.at(i);
        l.push_back(h);
        history.push_back(l);
    }
}

void Node::clearHistory()
{
    for (unsigned i = 0; i < history.size(); i++)
        while (!history.at(i).empty())
            history.at(i).pop_back();
    while (!history.empty())
        history.pop_back();
}

void Network::reset()
{
    for (unsigned i = 0; i < vertices.size(); i++)
    {
        Node V = vertices.at(i);
        V.clearLast();
        V.clearHistory();
        V.setComplete(false);
        V.setSum(0);
        vertices.at(i) = V;
    }
    for (unsigned i = 0; i < pathInjuncts.size(); i++)
    	for (unsigned j = 0; j < pathInjuncts.at(i).size(); j++)
    		pathInjuncts.at(i).pop_back();
    for (unsigned i = 0; i < pathInjuncts.size(); i++)
    	pathInjuncts.pop_back();
}

void Network::removeNode(int GCx, int GCy)
{
	int removalInjunct = -1;
	for (unsigned i = 0; i < vertices.size(); i++)
	{
		Node Nd = vertices[i];
		if (GCx == Nd.getGCx() && GCy == Nd.getGCy())
			removalInjunct = i;
	}

	vector<int> neighborInjuncts = neighborMap[removalInjunct];
	for (unsigned j = 0; j < neighborInjuncts.size(); j++)
	{
		int injunct = neighborInjuncts[j];
		vector<int> oldInjuncts = neighborMap[injunct];

		vector<int> newInjuncts;
		for (unsigned k = 0; k < oldInjuncts.size(); k++)
		{
			int oldinjunct = oldInjuncts[k];
			if (oldinjunct != removalInjunct)
				newInjuncts.push_back(oldinjunct);
		}
		neighborMap.erase(injunct);
		neighborMap[injunct] = newInjuncts;
	}
	neighborMap.erase(removalInjunct);
}

bool Network::detonate()
{
    reset();
	if (vertices.size() == 0)
		return false;

	//set injunction to injunct at the source vertex
    int injunction = -1;
    for (unsigned i = 0; i < vertices.size(); i++)
        if (vertices.at(i).isSource())
        {
            injunction = i;
            break;
        }
    if (injunction == -1) return false;

    //initialize the source vertex with appropriate  values
    Node S = vertices.at(injunction);
    int C = S.getCost();
    vector< vector<int> > H = S.getHistory();
    vector<int> temp;
    temp.push_back(injunction);
    H.push_back(temp);
    S.setHistory(H);
    S.setSum(C);
    S.setComplete(true);
    vertices.at(injunction) = S;

    //set injunction to injunct at the sink vertex
    injunction = -1;
    for (unsigned i = 0; i < vertices.size(); i++)
        if (vertices.at(i).isSink())
        {
            injunction = i;
            break;
        }
    if (injunction == -1) return false;

    int checker = -1; //use this to determine if there is no path from source to sink
    S = vertices.at(injunction); // 0 : S is always the sink, renewed at the end of this while loop
    while (!S.isComplete())
    {
        // test to see if this loop has executed enough times to indicate failure
    	checker++;
        if (checker > vertices.size() * 2)
        	return false;

        // gather the injuncts of vertices that have 'finished'
        vector<int> finished;
        for (unsigned i = 0; i < vertices.size(); i++)
        {
            Node finCandidate = vertices.at(i);
            if (finCandidate.isComplete())
                finished.push_back(i);
        }

        // reset some of the values of the 'unfinished' vertices
        for (unsigned i = 0; i < vertices.size(); i++)
        {
            Node resetCandidate = vertices.at(i);
            if (!resetCandidate.isComplete())
            {
                resetCandidate.setSum(0);
                resetCandidate.clearLast();
                resetCandidate.clearHistory();
                vertices.at(i) = resetCandidate;
            }
        }

        vector<int> unfinishedNeighbors;
        for (unsigned i = 0; i < finished.size(); i++)	//iterate through the finished vertices
        {
            int fin = finished.at(i);
            Node F = vertices.at(fin);
            H = F.getHistory();
            int Fs = F.getSum();

            vector<int> neighbors;
            if (neighborMap.count(fin) != 0)
            	neighbors = neighborMap.at(fin);

            for (unsigned j = 0; j < neighbors.size(); j++)
            {
                int n = neighbors.at(j);
                Node unfinCandidate = vertices.at(n);

                if (!unfinCandidate.isComplete())
                {
                    int ufc = unfinCandidate.getCost();
                    int ufs = unfinCandidate.getSum();
                    int temp = Fs + ufc;
                    if (temp <= ufs || ufs == 0)
                    {
                        unfinCandidate.setSum(temp);
                        if (temp != ufs) unfinCandidate.clearLast();
                        unfinCandidate.pushLast(H);
                        vertices.at(n) = unfinCandidate;
                    }
                    bool include = true;
                    for (unsigned k = 0; k < unfinishedNeighbors.size(); k++)
                    {
                        int m = unfinishedNeighbors.at(k);
                        if (n == k) include = false;
                    }
                    if (include) unfinishedNeighbors.push_back(n);
                }
            }
        }


        int min = -1;
        int updateIndex = -1;
        for (unsigned i = 0; i < unfinishedNeighbors.size(); i++)
        {
            int j = unfinishedNeighbors.at(i);
            Node tM = vertices.at(j);
            int tMs = tM.getSum();
            if (tMs < min || min == -1)
            {
                min = tMs;
                updateIndex = j;
            }
        }
        if (updateIndex != -1)
        {
            Node updateNode = vertices.at(updateIndex);
            updateNode.updateHistory(updateIndex);
            updateNode.setComplete(true);
            vertices.at(updateIndex) = updateNode;
        }
        S = vertices.at(injunction);
    }

    pathInjuncts = S.getHistory();
    return true;
}

BioRouter::BioRouter()
{
	claim(false, "Invalid constructor used for Router variant.  Must use form that accepts DmfbArch.\n");
}
BioRouter::BioRouter(DmfbArch *dmfbArch)
{
	arch = dmfbArch;
	tMax = 1.5 * arch->getNumCellsX() + 1.5 * arch->getNumCellsY();
	srand(time(NULL));
}


BioRouter::~BioRouter()
{

}

///////////////////////////////////////////////////////////////////////////////////
// Determines whether or not a cell satisfies the dimensions of the array.
///////////////////////////////////////////////////////////////////////////////////
bool BioRouter::cellIsBoundedByArray(Cell * c)
{
	if ((c->x <= arch->getNumCellsX() - 1)
			&& (c->y <= arch->getNumCellsY() - 1)
			&& (c->x >= 0)
			&& (c->y >= 0))
		return true;
	else return false;
}

///////////////////////////////////////////////////////////////////////////////////
// Returns the bounding box of a net n.
///////////////////////////////////////////////////////////////////////////////////
Box BioRouter::boundingBoxOfNet(Net * n)
{
	int LX, RX, TY, BY;
	if (n->source.x < n->target.x) { LX = n->source.x; RX = n->target.x; }
	else { LX = n->target.x; RX = n->source.x; }
	if (n->source.y < n->target.y) { TY = n->source.y; BY = n->target.y; }
	else { TY = n->target.y; BY = n->source.y; }

	return Box(LX, RX, TY, BY);
}

///////////////////////////////////////////////////////////////////////////////////
// Returns the bounding box of a global cell gc
///////////////////////////////////////////////////////////////////////////////////
Box BioRouter::boundingBoxOfGlobalCell(globalCell * gc)
{
	int LX, RX, TY, BY;
	LX = gc->LX; RX = gc->RX; TY = gc->TY; BY = gc->BY;
	return Box(LX, RX, TY, BY);
}

///////////////////////////////////////////////////////////////////////////////////
// Determines whether or not two boxes intersect OR are adjacent to each other.
///////////////////////////////////////////////////////////////////////////////////
bool BioRouter::boxesTouch(Box * b1, Box * b2)
{
	bool potentialViolation = false;
	bool violation = false;

	// Compare the horizontal intervals.  If they overlap, then the
	// boxes may intersect if they aren't displaced enough vertically.
	for (int i = b1->LX - 1; i <= b1->RX + 1; i++)
		if ((i >= b2->LX) && (i <= b2->RX ))
		{
			potentialViolation = true;
			break;
		}
	// Compare the vertical intervals if the horizontal intervals overlapped.
	if (potentialViolation)
		for (int i = b1->TY - 1; i <= b1->BY + 1; i++)
			if ((i >= b2->TY) && (i <= b2->BY))
			{
				violation = true;
				break;
			}
	return violation;
}

///////////////////////////////////////////////////////////////////////////////////
// Determines whether or not a cell c lies within the boundary of a box, b.
///////////////////////////////////////////////////////////////////////////////////
bool BioRouter::boxContainsCell(Box * b, Cell * c)
{
	if ((c->x >= b->LX) && (c->x <= b->RX)
			&& (c->y >= b->TY) && (c->y <= b->BY))
		return true;
	else return false;
}

///////////////////////////////////////////////////////////////////////////////////
// Returns the overlapping and neighboring cells of boxes b1 and b2 in the form of
// a vector.
///////////////////////////////////////////////////////////////////////////////////
vector<Cell> BioRouter::touchingCells(Box * b1, Box * b2)
{
	vector<Cell> touchingCells;

	for (int i = b1->LX; i <= b1->RX; i++)
	{
		for (int j = b1->TY; j <= b1->BY; j++)
		{
			Cell ci(i,j);
			for (int m = -1; m <= 1; m++)
			{
				for (int n = -1; n <= 1; n++)
				{
					if (m == 0 && n == 0) continue;
					int x = ci.x + m, y = ci.y + n;
					Cell co(x,y);
					if (!cellIsBoundedByArray(&co))
							continue;
					if (boxContainsCell(b2, &co))
						touchingCells.push_back(co);
				}
			}
		}
	}

	for (int i = b2->LX; i <= b2->RX; i++)
	{
		for (int j = b2->TY; j <= b2->BY; j++)
		{
			Cell ci(i,j);
			if (boxContainsCell(b1, &ci))
				continue;
			for (int m = -1; m <= 1; m++)
			{
				for (int n = -1; n <= 1; n++)
				{
					if (m == 0 && n == 0) continue;
					int x = ci.x + m, y = ci.y + n;
					Cell co(x,y);
					if (!cellIsBoundedByArray(&co)) continue;
					if (boxContainsCell(b1, &co))
						touchingCells.push_back(co);
				}
			}
		}
	}
	return touchingCells;
}

///////////////////////////////////////////////////////////////////////////////////
// Computes the idle interval of cell c for net n.
///////////////////////////////////////////////////////////////////////////////////
timeInterval BioRouter::idleInterval(Cell * c, Net * n)
{
	int t1 = manhattanDistance(&(n->source), c);
	int t2 = (tMax - manhattanDistance(c, &(n->target)));
	return timeInterval(t1, t2);
}

///////////////////////////////////////////////////////////////////////////////////
// Computes the violation interval of cell c for net n.
///////////////////////////////////////////////////////////////////////////////////
timeInterval BioRouter::violationInterval(Cell * c, Net * n)
{
	int t1 = manhattanDistance(&(n->source), c) - 1;
	int t2 = (tMax - manhattanDistance(c, &(n->target))) + 1;
	return timeInterval(t1, t2);
}

///////////////////////////////////////////////////////////////////////////////////
// Determines whether or not two time intervals intersect.
///////////////////////////////////////////////////////////////////////////////////
bool BioRouter::intervalViolation(timeInterval dT1, timeInterval dT2)
{
	for (int i = dT1.t1; i <= dT1.t2; i++)
		for (int j = dT2.t1; j <= dT2.t2; j++)
		{
			if (i == j) return true;
		}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////
// Computes the Manhattan distance between two cells.
///////////////////////////////////////////////////////////////////////////////////
int BioRouter::manhattanDistance(Cell * C1, Cell * C2)
{
	//Determine the bounding box.
	Net N(*C1, *C2);
	Box B = boundingBoxOfNet(&N);
	claim(((B.LX <= B.RX) && (B.TY <= B.BY)), "in BioRouter::manhattanDistance : fail LX <= RX and TY <= BY");

	//Sum the horizontal and vertical distances.
	int addendA = B.RX - B.LX, addendB = B.BY - B.TY;
	return addendA + addendB;
}

///////////////////////////////////////////////////////////////////////////////////
// These functions provide the means for sorting a vector of nets in order of
// criticality.
///////////////////////////////////////////////////////////////////////////////////
void BioRouter::sort(vector<Net> * a, update DIR)
{
	mergesort(a, 0, a->size() - 1, DIR);
}

void BioRouter::mergesort(vector<Net> *a, int low,int high, update DIR)
{
    int pivot;
    if(low<high)
    {
        pivot=(low+high)/2;
        mergesort(a,low,pivot, DIR);
        mergesort(a,pivot+1,high, DIR);
        merge(a,low,pivot,high, DIR);
    }
}
void BioRouter::merge(vector<Net> * a, int low, int pivot, int high, update DIR)
{
    int h,i,j,k;
    Net b[50];
    h=low;
    i=low;
    j=pivot+1;

    while((h<=pivot)&&(j<=high))
    {
    	if (DIR == DOWN)
    	{
			if(criticality(& a->at(h)) >= criticality(& a->at(j)))
			{
				b[i]=a->at(h);
				h++;
			}
			else
			{
				b[i]=a->at(j);
				j++;
			}
    	}
        else if (DIR == UP)
        {
			if(criticality(& a->at(h)) <= criticality(& a->at(j)))
			{
				b[i]=a->at(h);
				h++;
			}
			else
			{
				b[i]=a->at(j);
				j++;
			}
        }
        i++;
    }
    if(h>pivot)
    {
        for(k=j; k<=high; k++)
        {
            b[i]=a->at(k);
            i++;
        }
    }
    else
    {
        for(k=h; k<=pivot; k++)
        {
            b[i]=a->at(k);
            i++;
        }
    }
    for(k=low; k<=high; k++) a->at(k)=b[k];
}

///////////////////////////////////////////////////////////////////////////////////
// Computes the criticality of a net n.
///////////////////////////////////////////////////////////////////////////////////
double BioRouter::criticality(Net * n)
{
	// Determine the bounding box of n.
	Box B = boundingBoxOfNet(n);
	claim(((B.LX <= B.RX) && (B.TY <= B.BY)), "in BioRouter::criticality : fail LX <= RX and TY <= BY");

	// Create a vector of cells within the bounding box, and a vector of their
	// corresponding violation intervals so that they can be iterated through.
	vector<Cell> Ca;
	vector<timeInterval> Vi;
	for (int i = B.LX; i <= B.RX; i++)
	{
		for (int j = B.TY; j <= B.BY; j++)
		{
			Cell C(i,j);
			Ca.push_back(C);
			timeInterval V = violationInterval(&C, n);
			Vi.push_back(V);
		}
	}

	// Compute the numerator of criticality.
	double numerator = 0;
	for (unsigned i = 0; i < nets.size(); i++)
	{
		Net N = nets.at(i);				// 'N' iterates through all nets (member 'nets').
		for (unsigned j = 0; j < Ca.size(); j++)
		{
			Cell C = Ca.at(j);					// 'C' iterates through all cells of the bounding box of net n ('Ca').
			int T1 = Vi.at(j).t1, T2 = Vi.at(j).t2;
			for (int k = T1; k <= T2; k++)		// 'k' iterates through the violation interval of net n at cell C.
				numerator += available(&C, &N, k);
		}
	}

	// Compute the denominator of criticality.
	double denominator = 0;
	for (unsigned j = 0; j < Ca.size(); j++)
	{
		Cell C = Ca.at(j);						// 'C' iterates through all cells of the bounding box of net n ('Ca').
		int T1 = Vi.at(j).t1 + 1, T2 = Vi.at(j).t2 - 1;
		for (int k = T1; k <= T2; k++)			// 'k' iterates through the idle interval of net n at cell C.
			denominator += available(&C, n, k);
	}

	return (numerator / denominator);
}

///////////////////////////////////////////////////////////////////////////////////
// Computes the availability of cell c to net n at time t.
// Return value 1 means available, 0 means not available.
///////////////////////////////////////////////////////////////////////////////////
int BioRouter::available(Cell * c, Net * n, int t)
{
	// If the cell is not free, we can know in advance that it's not available.
	if (!cellIsFree(c, n))
		return 0;

	//Determine if t lies in the idle interval.
	timeInterval di = idleInterval(c, n);
	if (di.t1 <= di.t2 && t >= di.t1 && t <= di.t2) return 1;
	else return 0;
}

///////////////////////////////////////////////////////////////////////////////////
// Determines whether a cell is free for a net n by consulting cellIsBlocked.
// A cell may be blocked, but if the cell lies within the reconfigurable
// module that houses the target of the net, we should regard the cell as usable
// for the droplet of that net.
///////////////////////////////////////////////////////////////////////////////////
bool BioRouter::cellIsFree(Cell * c, Net * n)
{
	Box B;
	Cell t = n->target;
	Cell s = n->source;
	for (unsigned i = 0; i < thisTS->size(); i++)
	{
		AssayNode *n = thisTS->at(i);
		if (!(n->GetType() == DISPENSE || n->GetType() == OUTPUT))
		{
			ReconfigModule *rm = n->GetReconfigMod();
			B = Box(rm->getLX() - getHCellsBetweenModIR(), rm->getRX() + getHCellsBetweenModIR(), rm->getTY() - getVCellsBetweenModIR(), rm->getBY() + getVCellsBetweenModIR());
			if (boxContainsCell(&B, c) && boxContainsCell(&B, &t))
				return true;
			if (boxContainsCell(&B, c) && boxContainsCell(&B, &s))
				return true;
		}
	}

	if (cellIsBlocked[c->x][c->y])
		return false;
	else return true;
}

///////////////////////////////////////////////////////////////////////////////////
// Determines whether or not two nets n1 and n2 are independent of each other.
///////////////////////////////////////////////////////////////////////////////////
bool BioRouter::independent(Net * n1, Net * n2)
{
	// Determine the bounding boxes of the nets.
	Box b1 = boundingBoxOfNet(n1), b2 = boundingBoxOfNet(n2);

	// If they aren't overlapped or adjacent, we know the nets are independent.
	if (!boxesTouch(&b1, &b2)) return true;

	// Gather the cells that are shared or adjacent.
	vector<Cell> touchCells = touchingCells(&b1, &b2);

	claim(!touchCells.empty(), "in BioRouter::independent : 'touchCells' is empty after an affirmative result from boxesTouch");

	// Test each of the cells to see if any fails to be violation-free.
	for (unsigned i = 0; i < touchCells.size(); i++)
	{
		Cell c = touchCells.at(i);
		if (violationFree(n1, n2, &c))
			continue;
		else return false;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////
// Determines whether or not a cell c is violation-free for two nets n1 and n2.
///////////////////////////////////////////////////////////////////////////////////
bool BioRouter::violationFree(Net * n1, Net * n2, Cell * c)
{
	Box b1 = boundingBoxOfNet(n1), b2 = boundingBoxOfNet(n2);

	claim((boxContainsCell(&b1,c) || (boxContainsCell(&b2,c))), "in BioRouter::violationFree : failure of cell to exist in either bounding box");

	if (boxContainsCell(&b1, c))
		if (!unidirectionCheck(c, &b2, n1, n2))
			return false;
	if (boxContainsCell(&b2, c))
		if (!unidirectionCheck(c, &b1, n2, n1))
			return false;
	return true;
}

///////////////////////////////////////////////////////////////////////////////////
// This is a helper function for violationFree.
// Arguments are the cell c, the *other* box, the net of the bounding box of c,
// and finally the net of the *other* box.
///////////////////////////////////////////////////////////////////////////////////
bool BioRouter::unidirectionCheck(Cell * c, Box * b, Net * n1, Net * n2)
{
	// Gather the neighboring cells of c inclusive that belong to the other box, b.
	vector<Cell> nCells;
	for (int i = -1; i <= 1; i++)
	{
		for (int j = -1; j <= 1; j++)
		{
			int x = c->x + i, y = c->y + j;
			Cell nc(x,y);
			if (!cellIsBoundedByArray(&nc) || !cellIsFree(&nc, n2) || !boxContainsCell(b, &nc))
				continue;
			else nCells.push_back(nc);
		}
	}

	timeInterval Vi = violationInterval(c, n1);
	while (!nCells.empty())
	{
		Cell nc = nCells.at(nCells.size()-1);
		timeInterval Di = idleInterval(&nc, n2);
		claim((Di.sensible() && Vi.sensible()), "in BioRouter::unidirectionCheck : interval sensibility failure");
		if (intervalViolation(Vi, Di))
			return false;
		nCells.pop_back();
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////
// Populates cellIsBlocked, a 2-D vector indicating which cells are occupied by
// reconfigurable modules and their specified surrounding regions.
///////////////////////////////////////////////////////////////////////////////////
void BioRouter::establishBlocks()
{
	for (int x = 0; x < arch->getNumCellsX(); x++)
	{
		vector<bool> horizontal;
		cellIsBlocked.push_back(horizontal);
		for (int y = 0; y < arch->getNumCellsY(); y++)
		{
			bool vertical = false;
			cellIsBlocked[x].push_back(false);
		}
	}
	for (unsigned i = 0; i < thisTS->size(); i++)
	{
		AssayNode *n = thisTS->at(i);
		if (!(n->GetType() == DISPENSE || n->GetType() == OUTPUT))
		{
			ReconfigModule *rm = n->GetReconfigMod();
			for (int x = rm->getLX()-getHCellsBetweenModIR(); x <= rm->getRX()+getHCellsBetweenModIR(); x++)
				for (int y = rm->getTY() - getVCellsBetweenModIR(); y <= rm->getBY() + getVCellsBetweenModIR(); y++)
					cellIsBlocked[x][y] = true;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////
// Populates members sources, targets, nets, attributes, networks, routeStatus
///////////////////////////////////////////////////////////////////////////////////
void BioRouter::establishResources(vector<vector<RoutePoint *> *> *subRoutes, vector<Droplet *> *subDrops, map<Droplet *, vector<RoutePoint *> *> *routes)
{
	vector<AssayNode *> routableThisTS;
	for (unsigned i = 0; i < thisTS->size(); i++)
		if (thisTS->at(i)->GetType() != DISPENSE && thisTS->at(i)->GetStartTS() == startingTS)
			routableThisTS.push_back(thisTS->at(i));
	Sort::sortNodesByLatestTSThenStorage(&routableThisTS);

	while (!routableThisTS.empty())
	{
		AssayNode *n = routableThisTS.front();
		routableThisTS.erase(routableThisTS.begin());
		for (unsigned p = 0; p < n->GetParents().size(); p++)
		{
			AssayNode *par = n->GetParents().at(p);
			Droplet *pd = par->GetDroplets().back();
			par->droplets.pop_back();
			n->addDroplet(pd);
			subDrops->push_back(pd);
			vector<RoutePoint *> *sr = new vector<RoutePoint *>();
			subRoutes->push_back(sr);
			vector<int> attribute;

			Cell source;
			if (par->GetType() == DISPENSE)
			{
				attribute.push_back(CREATIVE);
				if (par->GetIoPort()->getSide() == NORTH)
					{ source.x = par->GetIoPort()->getPosXY(); source.y = 0; }
				else if (par->GetIoPort()->getSide() == SOUTH)
					{ source.x = par->GetIoPort()->getPosXY(); source.y = arch->getNumCellsY()-1; }
				else if (par->GetIoPort()->getSide() == EAST)
					{ source.x = arch->getNumCellsX()-1; source.y = par->GetIoPort()->getPosXY(); }
				else if (par->GetIoPort()->getSide() == WEST)
					{ source.x = 0; source.y = par->GetIoPort()->getPosXY(); }
				else
					claim(false, "Invalid direction for IoSide.");
			}
			else
			{
				RoutePoint * lrp = (*routes)[pd]->back();
				source.x = lrp->x;
				source.y = lrp->y;
			}

			sources.push_back(source);

			Cell target;
			if (n->type == OUTPUT)
			{
				attribute.push_back(DISMISSIVE);
				if (n->ioPort->getSide() == NORTH || n->ioPort->getSide() == SOUTH)
				{
					target.x = n->ioPort->getPosXY();
					if (n->ioPort->getSide() == NORTH)
						target.y = 0;
					else
						target.y = arch->getNumCellsY()-1;
				}
				else if (n->ioPort->getSide() == EAST || n->ioPort->getSide() == WEST)
				{
					target.y = n->ioPort->getPosXY();
					if (n->ioPort->getSide() == EAST)
						target.x = arch->getNumCellsX()-1;
					else
						target.x = 0;
				}
			}
			else if (n->type == STORAGE)
			{
				attribute.push_back(POSSESSIVE);
				ReconfigModule *rm = n->GetReconfigMod();
				int ty;
				int dropIdTop = dropletOccupyingCell(rm->getLX(), rm->getTY(), routes, subRoutes, subDrops);
				int dropIdBot = dropletOccupyingCell(rm->getLX(), rm->getBY(), routes, subRoutes, subDrops);

				if (dropIdTop < 0 || dropIdTop == pd->getId())
					ty = rm->getTY();
				else if (dropIdBot < 0 || dropIdBot == pd->getId())
					ty = rm->getBY();
				else
					claim(false, "Storage module is already full with other droplets.");
				target.x = rm->getLX();
				target.y = ty;
			}
			else
			{
				ReconfigModule *rMod = n->reconfigMod;
				target.x = rMod->getLX();
				target.y = rMod->getBY();
			}
			targets.push_back(target);

			attributes.push_back(attribute);
		}
	}

	claim(sources.size() == targets.size(), "in BioRouter::establishResources : sources and targets are inconsistent");

	for (unsigned i = 0; i < sources.size(); i++)
	{
		Net n;
		n.source = sources.at(i);
		n.target = targets.at(i);

		nets.push_back(n);
	}

	if (!(sources.size() == targets.size() && targets.size() == nets.size() && nets.size() == subRoutes->size() && subRoutes->size() == attributes.size()))
		claim(false, "in BioRouter::establishResources : resources are inconsistent");

	for (unsigned i = 0; i < sources.size(); i++)
	{
		Network N;
		networks.push_back(N);
	}

	for (unsigned i = 0; i < sources.size(); i++)
		routeStatus.push_back(true);
}

///////////////////////////////////////////////////////////////////////////////////
// Returns a vector of independent nets.
///////////////////////////////////////////////////////////////////////////////////
vector<Net> BioRouter::independentNets()
{
	//gather all unrouted nets and sort by criticality
	vector<Net> netCritic;
	for (unsigned i = 0; i < nets.size(); i++)
	{
		Net alpha = nets[i];
		if (alpha.global == UNROUTED)
		{
			alpha.netsInjunction = i;
			netCritic.push_back(alpha);
		}
	}
	sort(& netCritic, DOWN);

	//create the dependence map
	map< int, vector<int> > dependenceMap;
	if (netCritic.size() != 0)
	{
		for (unsigned i = 0; i < netCritic.size()-1; i++)
		{
			Net alpha = netCritic[i];
			int ainjunction = alpha.netsInjunction;
			for (unsigned j = i+1; j < netCritic.size(); j++)
			{
				Net beta = netCritic[j];
				int binjunction = beta.netsInjunction;
				if (!independent(&alpha, &beta))
				{
					dependenceMap[ainjunction].push_back(binjunction);
					dependenceMap[binjunction].push_back(ainjunction);
				}
			}
		}
	}

	//Discard dependent sets in the way of MWIS.
	for (unsigned i = 0; i < netCritic.size(); i++)
	{
		Net alpha = netCritic[i];
		if (alpha.independence == INCLUDED) //inclusion is default
		{
			int ainjunction = alpha.netsInjunction;
			vector<int> depend = dependenceMap[ainjunction];
			for (unsigned j = 0; j < netCritic.size(); j++)
			{
				Net beta = netCritic[j];
				int binjunction = beta.netsInjunction;
				for (unsigned k = 0; k < depend.size(); k++)
				{
					int cinjunction = depend[k];
					if (binjunction == cinjunction)
						netCritic[j].independence = DISCARDED;
				}
			}
		}
	}

	vector<Net> independents;
	for (unsigned i = 0; i < netCritic.size(); i++)
	{
		Net alpha = netCritic[i];
		if (alpha.independence == INCLUDED)
			independents.push_back(alpha);
	}
	return independents;
}

///////////////////////////////////////////////////////////////////////////////////
// Constructs the global array 2-D vector, members of which represent 3x3 cell structures
///////////////////////////////////////////////////////////////////////////////////
void BioRouter::constructGlobalArray(GlobalArray * GC)
{
	int maxx = arch->getNumCellsX(), maxy = arch->getNumCellsY();
	claim(maxx > 2 && maxy > 2, "in BioRouter::constructGlobalArray : the dimensions of the microfluidic array are too small");
	int currx = 2, curry = 2;
	while (true)
	{
		vector<globalCell> vertical;
		while (true)
		{
			globalCell gc(currx - 2, currx, curry - 2, curry);
			vertical.push_back(gc);
			if (curry + 3 >= maxy)
				break;
			curry += 3;
		}
		if (curry != maxy - 1)
		{
			globalCell gc(currx - 2, currx, curry + 1, maxy - 1);
			vertical.push_back(gc);
		}
		GC->globalCells.push_back(vertical);
		if (currx + 3 >= maxx)
			break;
		currx += 3;
		curry = 2;
	}
	if (currx != maxx - 1)
	{
		curry = 2;
		vector<globalCell> vertical;
		while (true)
		{
			globalCell gc(currx + 1, maxx - 1, curry - 2, curry);
			vertical.push_back(gc);
			if (curry + 3 >= maxy)
				break;
			curry += 3;
		}
		if (curry != maxy - 1)
		{
			globalCell gc(currx + 1, maxx - 1, curry + 1, maxy - 1);
			vertical.push_back(gc);
		}
		GC->globalCells.push_back(vertical);
	}

	for (unsigned i = 0; i < GC->globalCells.size(); i++)
	{
		for (unsigned j = 0; j < GC->globalCells.at(i).size(); j++)
		{
			GC->globalCells.at(i).at(j).cost = 0;
			GC->globalCells.at(i).at(j).previous = 0;
			GC->globalCells.at(i).at(j).capacity = 0;
		}
	}

	for (unsigned i = 0; i < GC->globalCells.size(); i++)
	{
		vector<bool> inc;
		GC->inclusionMatrix.push_back(inc);
		for (unsigned j = 0; j < GC->globalCells.at(i).size(); j++)
		{
			GC->inclusionMatrix.at(i).push_back(false);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////
// This is a helper function for calculateCapacities, for sorting time intervals
///////////////////////////////////////////////////////////////////////////////////
bool intervalSort(const timeInterval & lhs, const timeInterval & rhs)
{
	if (lhs.t1 < rhs.t1) return true;
	else return false;
}

///////////////////////////////////////////////////////////////////////////////////
// Sets the capacities for each global cell in the global array GC
///////////////////////////////////////////////////////////////////////////////////
void BioRouter::calculateCapacities(GlobalArray * GC)
{
	for (unsigned i = 0; i < GC->globalCells.size(); i++)
	{
		for (unsigned j = 0; j < GC->globalCells.at(i).size(); j++)
		{
			globalCell gc = GC->globalCells.at(i).at(j);
			list<timeInterval> T;
			for (int x = gc.LX; x <= gc.RX; x++)
			{
				for (int y = gc.TY; y <= gc.BY; y++)
				{
					Cell c(x,y);
					for (unsigned k = 0; k < nets.size(); k++)
					{
						Net n = nets[k];
						timeInterval t = idleInterval(&c, &n);
						if (t.sensible())
							T.push_back(t);
					}
				}
			}
			T.sort(intervalSort);

			vector<timeInterval> dilation;
			list<timeInterval>::iterator it = T.begin();
			timeInterval t1 = *it;
			int min = t1.t1, max = t1.t2;
			timeInterval d(min, max);
			if (!T.empty())
			{
				while (it != T.end())
				{
					t1 = *it;
					it++;
					if (it == T.end()) break;
					timeInterval t2 = *it;
					if (intervalViolation(t1, t2) || intervalViolation(t2, d))
					{
						if (t2.t2 > max)
						{
							max = t2.t2;
							d = timeInterval(min, max);
						}
					}
					else
					{
						dilation.push_back(d);
						min = t2.t1;
						max = t2.t2;
						d = timeInterval(min, max);
					}
				}
				dilation.push_back(d);
			}
			else
			{
				d = timeInterval(0,0);
				dilation.push_back(d);
			}

			int LX = gc.LX, RX = gc.RX, TY = gc.TY, BY = gc.BY;
			int dX = RX-LX, dY = BY-TY;
			int div;
			if (dX == 2 || dY == 2) div = 5;
			else if (dX == 1 || dY == 1) div = 4;
			else div = 3;
			double u = 0;
			for (unsigned m = 0; m < dilation.size(); m++)
			{
				timeInterval t = dilation[m];
				double s = t.span();
				u += floor((s+2)/div);
			}
			GC->globalCells.at(i).at(j).capacity = u;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////
// Sets the cost for each global cell gc in the global array GC
///////////////////////////////////////////////////////////////////////////////////
void BioRouter::calculateCosts(GlobalArray * GC, vector<Net> * N)
{
	for (unsigned i = 0; i < GC->globalCells.size(); i++)
	{
		for (unsigned j = 0; j < GC->globalCells.at(i).size(); j++)
		{
			globalCell gc = GC->globalCells.at(i).at(j);
			int max = 0;
			for (int x = gc.LX; x <= gc.RX; x++)
			{
				for (int y = gc.TY; y <= gc.BY; y++)
				{
					Cell c(x,y);
					int tally = 0;
					for (unsigned k = 0; k < N->size(); k++)
					{
						Net n = N->at(k);
						timeInterval t = idleInterval(&c, &n);
						if (t.sensible())
							tally++;
					}
					if (tally >= max) max = tally;
				}
			}
			int independentUse = max;
			int previous = gc.previous;
			int independentSize = N->size();
			int cost = 0;

			if (previous != 0)
				cost = independentSize - independentUse;
			else if (previous == 0)
				cost = 1 + (independentSize - independentUse);
			GC->globalCells.at(i).at(j).cost = cost;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////
// Develops a network for the net N according to the global array GC
///////////////////////////////////////////////////////////////////////////////////
void BioRouter::networkize(Network * Nw, GlobalArray * GC, Net * N)
{
	for (unsigned i = 0; i < GC->inclusionMatrix.size(); i++)
	{
		for (unsigned j = 0; j < GC->inclusionMatrix.at(i).size(); j++)
			GC->inclusionMatrix[i][j] = false;
	}

	for (unsigned i = 0; i < GC->globalCells.size(); i++)
	{
		for (unsigned j = 0; j < GC->globalCells.at(i).size(); j++)
		{
			globalCell gc = GC->globalCells[i][j];
			Box b = boundingBoxOfNet(N);
			for (int x = gc.LX; x <= gc.RX; x++)
			{
				for (int y = gc.TY; y <= gc.BY; y++)
				{
					Cell c(x,y);
					timeInterval t = idleInterval(&c, N);
					if (t.sensible() && boxContainsCell(&b, &c) && cellIsFree(&c, N))
					{
						GC->inclusionMatrix[i][j] = true;
					}
				}
			}
		}
	}

	vector< vector<int> > index;
	int injunction = 1;
	for (unsigned i = 0; i < GC->inclusionMatrix.size(); i++)
	{
		vector<int> vertical;
		index.push_back(vertical);
		for (unsigned j = 0; j < GC->inclusionMatrix.at(i).size(); j++)
		{
			bool state = GC->inclusionMatrix[i][j];
			if (state == true)
			{
				index[i].push_back(injunction);
				injunction++;
			}
			else index[i].push_back(0);
		}
	}

	Cell source = N->source;
	Cell target = N->target;
	for (unsigned i = 0; i < index.size(); i++)
	{
		for (unsigned j = 0; j < index[i].size(); j++)
		{
			if (index[i][j] == 0) continue;
			else
			{
				globalCell gc = GC->globalCells[i][j];
				Box bc = boundingBoxOfGlobalCell(&gc);
				double cost = gc.cost;
				Node NodeToAdd(cost, i, j);
				if (boxContainsCell(&bc, &source))
					NodeToAdd.declareSource();
				if (boxContainsCell(&bc, &target))
					NodeToAdd.declareSink();
				Nw->pushNode(NodeToAdd);
			}
		}
	}

	for (unsigned i = 0; i < index.size(); i++)
	{
		for (unsigned j = 0; j < index[i].size(); j++)
		{
			if (index[i][j] == 0) continue;
			else
			{
				int xmod1 = i - 1;
				int xmod2 = i + 1;
				int ymod1 = j - 1;
				int ymod2 = j + 1;
				int maxX = index.size();
				int maxY = index.at(0).size();

				if (xmod1 >= 0)
				{
					if (index[xmod1][j] != 0)
						Nw->pushEdge(index[i][j] - 1, index[xmod1][j] - 1);
				}
				if (xmod2 < maxX)
				{
					if (index[xmod2][j] != 0)
						Nw->pushEdge(index[i][j] - 1, index[xmod2][j] - 1);
				}
				if (ymod1 >= 0)
				{
					if (index[i][ymod1] != 0)
						Nw->pushEdge(index[i][j] - 1, index[i][ymod1] - 1);
				}
				if (ymod2 < maxY)
				{
					if (index[i][ymod2] != 0)
						Nw->pushEdge(index[i][j] - 1, index[i][ymod2] - 1);
				}
			}
		}
	}
}

void BioRouter::updateGlobalArray(GlobalArray * GC, Network * N, update dir)
{
	vector< vector<int> > paths = N->getPaths();
	vector<int> path = paths[0];
	vector<Node> vertices = N->getVertices();
	for (unsigned j = 0; j < path.size(); j++)
	{
		int injunct = path[j];
		Node Nd = vertices[injunct];
		int xinjunct = Nd.getGCx();
		int yinjunct = Nd.getGCy();
		double prev = GC->globalCells[xinjunct][yinjunct].previous;
		if (dir == UP) prev++;
		else if (dir == DOWN) prev--;
		GC->globalCells[xinjunct][yinjunct].previous = prev;
	}
}

///////////////////////////////////////////////////////////////////////////////////
// Determines whether or not a cell lies in a global path - used in detailed routing
//to follow the general path established in global routing
///////////////////////////////////////////////////////////////////////////////////
bool BioRouter::cellIsInPath(Cell * c, Network * Nw, GlobalArray * GC)
{
	vector<int> path = Nw->getPaths().at(0);
	for (unsigned a = 0; a < path.size(); a++)
	{
		Node Nd = Nw->getVertices().at(path[a]);
		int nx = Nd.getGCx(), ny = Nd.getGCy();
		globalCell ngc = GC->globalCells[nx][ny];
		Box B(ngc.LX, ngc.RX, ngc.TY, ngc.BY);
		if (boxContainsCell(&B, c))
			return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////
// Determines the cost to move from previous if arrived at pArrival to next if
// arrives at nArrival, by consulting the graph
///////////////////////////////////////////////////////////////////////////////////
double BioRouter::costToMove(vector< vector< cellNode> > * graph, Cell * next, int nArrival, Cell * previous, int pArrival)
{
	int nx = next->x, ny = next->y;
	int px = previous->x, py = previous->y;

	double cost = 0;

	int lower = pArrival + 1, upper = nArrival - 1;

	for (int k = lower; k <= upper; k++)
	{
		double s = graph->at(px).at(py).s[k];
		double factor = s / 27;
		double H = graph->at(px).at(py).currentHFP[k] * graph->at(px).at(py).previousHFP[k];
		double product = factor * H;

		cost += product;
	}

	double s = graph->at(nx).at(ny).s[nArrival];
	double factor = s/27;
	double H = graph->at(nx).at(ny).currentHFP[nArrival] * graph->at(nx).at(ny).previousHFP[nArrival];
	double product = factor * H;

	cost += product;
	return cost;
}

///////////////////////////////////////////////////////////////////////////////////
// Strategically restricts fan-out so that droplets tend to move towards targets,
// however, discards are provided in order to enable handling of cases when the
// restriction prevents a solution.
///////////////////////////////////////////////////////////////////////////////////
void BioRouter::gathering(vector<Cell> * candidates, vector<Cell> * discarded, vector<Cell> * happened, GlobalArray * GC, Net N, Cell current, int iterationCount, bool specialIndividual)
{
	int injunct = N.netsInjunction;
	vector<Cell> originals;
	int x = current.x, y = current.y;
	for (int dy = -1; dy <= 1; dy++)
	{
		for (int dx = 1; dx >= -1; dx--)
		{
			if (abs(dx) == abs(dy)) continue;
			Cell candidate(x+dx, y+dy);
			int ix = candidate.x, iy = candidate.y;
			if (!cellIsBoundedByArray(&candidate)) continue;

			bool progress = false;
			for (unsigned z = 0; z < happened->size(); z++)
			{
				if (candidate == happened->at(z))
				{
					progress = true;
					break;
				}
			}
			if (progress) continue;
			if (iterationCount == 0 && !specialIndividual)
			{
				Network Nw = networks[injunct];
				if (cellIsInPath(&candidate, &Nw, GC))
				{
					if (cellIsFree(&candidate, &N))
					{
						candidates->push_back(candidate);
						originals.push_back(candidate);
					}
				}
			}
			else if (cellIsFree(&candidate, &N))
			{
				candidates->push_back(candidate);
				originals.push_back(candidate);
			}
		}
	}

	//remove candidates that result in moving away from the target, unless this would result in no candidates
	//if moving away from the target is necessary, ensure that it's not frivolous by only including if there's
	//a blocked neighboring cell that's closer to the target than the candidate

	Cell tgt = N.target;
	vector<Cell> improvedCandidates;
	for (unsigned z = 0; z < candidates->size(); z++)
	{
		Cell candidate = candidates->at(z);
		if (manhattanDistance(&candidate, &tgt) < manhattanDistance(&current, &tgt))
			improvedCandidates.push_back(candidate);
	}
	bool frivolousCheck = false;
	if (!improvedCandidates.empty())
		* candidates = improvedCandidates;
	else
		frivolousCheck = true;

	if (frivolousCheck)
	{
		while (!improvedCandidates.empty()) improvedCandidates.pop_back();
		for (unsigned z = 0; z < candidates->size(); z++)
		{
			Cell candidate = candidates->at(z);
			int ix = candidate.x, iy = candidate.y;
			bool closerBlockedNeighbor = false;
			for (int dx = -1; dx <= 1; dx++)
			{
				for (int dy = -1; dy <= 1; dy++)
				{
					Cell neighbor(ix+dx, iy+dy);
					if (candidate == neighbor) continue;
					if ((manhattanDistance(&neighbor, &tgt) < manhattanDistance(&candidate, &tgt))
							&& cellIsBlocked[neighbor.x][neighbor.y])
						closerBlockedNeighbor = true;
				}
			}
			if (closerBlockedNeighbor)
				improvedCandidates.push_back(candidate);
		}
	}
	if (!improvedCandidates.empty()) * candidates = improvedCandidates;

	//gather the cells that didn't make the cut into a separate vector
	for (unsigned e = 0; e < originals.size(); e++)
	{
		Cell E = originals[e];
		bool collect = true;
		for (unsigned f = 0; f < candidates->size(); f++)
		{
			Cell F = candidates->at(f);
			if (E == F) collect = false;
		}
		if (collect) discarded->push_back(E);
	}
}

void BioRouter::cleanUp()
{
	while (!sources.empty())
		sources.pop_back();
	while (!targets.empty())
		targets.pop_back();
	while (!nets.empty())
		nets.pop_back();
	while (!attributes.empty())
		attributes.pop_back();
	while (!networks.empty())
		networks.pop_back();
	while (!routeStatus.empty())
		routeStatus.pop_back();

	while (!cellIsBlocked.empty())
	{
		int position = cellIsBlocked.size()-1;
		while (!cellIsBlocked[position].empty())
		{
			cellIsBlocked[position].pop_back();
		}
		cellIsBlocked.pop_back();
	}
}

///////////////////////////////////////////////////////////////////////////////////
// Implementation of flow-based algorithm.
///////////////////////////////////////////////////////////////////////////////////
void BioRouter::computeIndivSupProbRoutes(vector<vector<RoutePoint *> *> *subRoutes, vector<Droplet *> *subDrops, map<Droplet *, vector<RoutePoint *> *> *routes)
{
	cout << "Routing time-step " << startingTS;
	//cout << "Global Routing" << "...";

	establishBlocks();
	establishResources(subRoutes, subDrops, routes);
	GlobalArray GC;
	constructGlobalArray(&GC);
	calculateCapacities(&GC);

	vector<Net> independents = independentNets();
	while (!independents.empty())
	{
		calculateCosts(&GC, &independents);

		//correlate independent nets with their injunctions into 'nets'
		vector<int> netInjunction;
		for (unsigned i = 0; i < independents.size(); i++)
			for (unsigned j = 0; j < nets.size(); j++)
				if (independents[i] == nets[j])
					netInjunction.push_back(j);

		claim((independents.size() == netInjunction.size()), "error: local netInjunction is inconsistent with independents");

		// apply the minimum cost algorithm for each independent net
		for (unsigned i = 0; i < netInjunction.size(); i++)
		{
			networkize(& networks[netInjunction[i]], &GC, & independents[i]);
			if (networks[netInjunction[i]].detonate())
			{
				Network Nw = networks[netInjunction[i]];
				vector< vector<int> > paths = Nw.getPaths();
				updateGlobalArray(& GC, & Nw, UP);
			}
			else
				routeStatus[netInjunction[i]] = false;
		}

		// the 'global' member of each corresponding fundamental net must be set to ROUTED_BC before reiteration
		for (unsigned i = 0; i < independents.size(); i++)
		{
			Net alpha = independents[i];
			int injunction = alpha.netsInjunction;
			nets[injunction].global = ROUTED_BC;
		}
		independents = independentNets();
	}

	bool roundRobin;
	do
	{
		roundRobin = true;
		for (unsigned i = 0; i < GC.globalCells.size(); i++)
		{
			for (unsigned j = 0; j < GC.globalCells.at(i).size(); j++)
			{
				double cap = GC.globalCells[i][j].capacity;
				double prev = GC.globalCells[i][j].previous;
				if (prev > cap)
				{
					cout << "error: capacity violation at (" << i << ", " << j << ")" << endl << endl;

					roundRobin = false;

					// gather the nets that use the problematic node in their route and sort by criticality
					vector<Net> fixers;
					for (unsigned k = 0; k < networks.size(); k++)
					{
						if (routeStatus[k] == false) continue;

						Network Nw = networks[k];
						vector< vector<int> > paths = Nw.getPaths();
						vector<int> path = paths[0];
						vector<Node> vertices = Nw.getVertices();
						for (unsigned l = 0; l < path.size(); l++)
						{
							int vinjunct = path[l];
							Node Nd = vertices[vinjunct];
							int GCx = Nd.getGCx(); int GCy = Nd.getGCy();
							if (i == GCx && j == GCy)
							{
								Net Nt = nets[k];
								Nt.netsInjunction = k;
								fixers.push_back(Nt);
							}
						}
					}
					sort (& fixers, DOWN);

					claim(fixers.size() > 0, "error: global routing error: no problematic nets found after alleged capacity violation");

					// acquire the one with lowest criticality
					Net Nt = fixers.back();

					// remove the problematic node from the net's corresponding network
					int ninjunct = Nt.netsInjunction;
					Network Nw = networks[ninjunct];
					Nw.removeNode(i, j);

					if (Nw.detonate())
					{
						// undo the effects of the previous network and apply the new
						Network Nx = networks[ninjunct];
						updateGlobalArray(& GC, & Nx, DOWN);
						networks[ninjunct] = Nw;
						updateGlobalArray(& GC, & Nw, UP);
					}
					else
					{
						if (!(fixers.size() > 1))
						{
							Network Nx = networks[ninjunct];
							updateGlobalArray(& GC, & Nx, DOWN);
							routeStatus[ninjunct] = false;
						}
						else
						{
							Net Nt2 = fixers.at(fixers.size()-2);

							int ninjunct2 = Nt2.netsInjunction;
							Network Nw2 = networks[ninjunct2];
							Nw2.removeNode(i, j);

							if (Nw2.detonate())
							{
								// undo the effects of the previous network and apply the new
								Network Nx2 = networks[ninjunct2];
								updateGlobalArray(& GC, & Nx2, DOWN);
								networks[ninjunct2] = Nw2;
								updateGlobalArray(& GC, & Nw2, UP);
							}
							else
							{
								Network Nx = networks[ninjunct];
								updateGlobalArray(& GC, & Nx, DOWN);
								routeStatus[ninjunct] = false;
							}
						}
					}
				}
			}
		}
	} while (!roundRobin);

	// check the networks that resulted from global routing for occupancy violations - if they exist, regard the corresponding net as failed
	// to be handled in detailed routing

	for (unsigned pos = 0; pos < nets.size(); pos++)
	{
		if (routeStatus[pos] == false) continue;

		//create an inclusion matrix initialized to 0
		vector< vector<int> > iMatrix;
		for (int i = 0; i < arch->getNumCellsX(); i++)
		{
			vector<int> horizontal;
			for (int j = 0; j < arch->getNumCellsY(); j++)
				horizontal.push_back(0);
			iMatrix.push_back(horizontal);
		}

		Network N = networks[pos];
		Net Nt = nets[pos];
		Cell src = sources[pos];
		Cell tgt = targets[pos];
		vector<Node> vertices = N.getVertices();
		vector< vector<int> > paths = N.getPaths();
		vector<int> path = paths[0];

		for (unsigned pathPos = 0; pathPos < path.size(); pathPos++)
		{
			Node Nd = vertices[path[pathPos]];
			globalCell gc = GC.globalCells[Nd.getGCx()][Nd.getGCy()];
			Box B = boundingBoxOfGlobalCell(&gc);
			for (int i = B.LX; i <= B.RX; i++)
			{
				for (int j = B.TY; j <= B.BY; j++)
				{
					Cell c(i,j);
					if (cellIsFree(&c, &Nt))
						iMatrix[i][j] = 1;
				}
			}
		}

		list<Cell> mass;
		for (unsigned z = 0; z < iMatrix.at(0).size(); z++)
		{
			for (unsigned w = 0; w < iMatrix.size(); w++)
			{
				if (iMatrix[w][z] == 1)
				{
					Cell C(w,z);
					mass.push_back(C);
				}
			}
		}

		list<Cell> expansion;
		for (list<Cell>::iterator i = mass.begin(); i != mass.end(); i++)
		{
			Cell C = *i;
			if (C == src)
			{
				expansion.push_back(C);
				mass.erase(i);
				break;
			}
		}

		bool cont = false;
		bool breaker = false;
		while (!cont)
		{
			cont = true;
			breaker = false;
			for (list<Cell>::iterator i = expansion.begin(); i != expansion.end(); i++)
			{
				for (list<Cell>::iterator j = mass.begin(); j != mass.end(); j++)
				{
					Cell C1 = *i;
					Cell C2 = *j;

					int dx = C1.x - C2.x, dy = C1.y - C2.y;
					if (!(abs(dx) == 1 && abs(dy) == 1) && (abs(dx) <= 1 && abs(dy) <= 1))
					{
						expansion.push_back(C2);
						mass.erase(j);
						cont = false;
						breaker = true;
					} if (breaker) break;
				} if (breaker) break;
			}
		}

		bool routeExists = true;
		for (list<Cell>::iterator i = mass.begin(); i != mass.end(); i++)
		{
			Cell C = *i;
			if (C == tgt) routeExists = false;
		}

		if (!routeExists) routeStatus[pos] = false;
	}

	//cout << "completed!" << endl;

	/////////////////////////////////////////////////////////////////DETAILED ROUTING/////////////////////////////////////////////////////////////////////////////////////

	//controller
	bool stopping = false;

	//cout << "Detailed Routing" << "...";

	//make sure each net knows its correct location in the vector
	for (unsigned i = 0; i < nets.size(); i++)
		nets[i].netsInjunction = i;

	//gather the nets that were successful in global routing
	vector<Net> successors;
	for (unsigned i = 0; i < nets.size(); i++)
		if (routeStatus[i] == true)
			successors.push_back(nets[i]);

	//if there were no successful nets in global routing
	//detailed routing must begin with all failed nets
	bool specialIndividual = false;
	if (successors.empty())
	{
		for (unsigned i = 0; i < nets.size(); i++)
			if (routeStatus[i] == false)
			{
				routeStatus[i] = true;
				successors.push_back(nets[i]);
			}
		specialIndividual = true;
	}

	sort(&successors, DOWN);

	//push the sorted nets onto a list (makes popping from the front possible)
	list<Net> E;
	for (unsigned j = 0; j < successors.size(); j++)
		E.push_back(successors[j]);

	//generate the graph
	vector< vector< cellNode> > graph;
	for (int i = 0; i < arch->getNumCellsX(); i++)
	{
		vector<cellNode> column;
		graph.push_back(column);
		for (int j = 0; j < arch->getNumCellsY(); j++)
		{
			cellNode cN(nets.size(), tMax);
			graph[i].push_back(cN);
		}
	}

	//create a tree for each net
	vector< vector<Cell> > trees;
	for (unsigned i = 0; i < nets.size(); i++)
	{
		vector<Cell> tree;
		trees.push_back(tree);
	}

	//these provide a means of rotation for failed nets via lexiconographic permutation
	vector< vector<int> > failuresMemory;
	vector<int> init;
	failuresMemory.push_back(init);

	int maximumIterationLimit = 50;//50
	int iterationCount = 0;

	while (!E.empty() && iterationCount < maximumIterationLimit)
	{
		bool fanExpansion = false;
		bool loopSkipper = false;
		double binaryMultiple = 2;//1
		while (!E.empty())
		{
			//take the first node from E
			list<Net>::iterator eat = E.begin();
			Net N = *eat;
			E.pop_front();
			int injunct = N.netsInjunction;

			//performs rip-up
			vector<Cell> tree = trees[injunct];
			for (unsigned i = 0; i < tree.size(); i++)
			{
				Cell c = tree[i];
				int x = c.x; int y = c.y;
				int arr = c.arrival;
				int dep = c.departure;
				int lower, upper;
				if (arr > 0) lower = arr-1; else lower = arr;
				if (dep < tMax) upper = dep+1; else upper = dep;
				for (int dx = -1; dx <= 1; dx++)
				{
					for (int dy = -1; dy <= 1; dy++)
					{
						Cell n(x+dx, y+dy);
						if (!cellIsBoundedByArray(&n)) continue;
						int ix = n.x; int iy = n.y;
						for (int j = lower; j <= upper; j++)
						{
							if (j < 0 || j > tMax) continue;
							graph[ix][iy].s[j]--;
							graph[ix][iy].currentHFP[j]--;
						}
					}
				}
			}
			while (!tree.empty()) tree.pop_back();
			trees[injunct] = tree;

			//push the source node into the priority queue and set the arrival to 0.
			priority_queue<Cell, vector<Cell>, compareCell> Q;
			Cell current = N.source;
			current.arrival = 0;
			current.cost = 0;
			current.history = 0;
			Q.push(current);

			//you'll want to record the histories
			vector< vector<Cell> > histories;
			vector<Cell> original;
			original.push_back(current);
			histories.push_back(original);
			vector<int> freeInjuncts;

			loopSkipper = false;
			int targetLooper = 0;
			if (iterationCount > maximumIterationLimit/2 || fanExpansion) binaryMultiple *= 2;
			//while the target is not reached
			Cell tgt = N.target;
			while (current != tgt)
			{
				targetLooper++;
				if (Q.empty()) { fanExpansion = true; loopSkipper = true; break; }
				//remove the node with the lowest cost from Q
				current = Q.top(); Q.pop();

				//gather fan-out nodes
				vector<Cell> candidates;
				vector<Cell> discarded;
				vector<Cell> happened = histories[current.history];
				gathering(&candidates, &discarded, &happened, &GC, N, current, iterationCount, specialIndividual);
				if ((fanExpansion && targetLooper < int(binaryMultiple))
						|| (iterationCount > maximumIterationLimit/2 && targetLooper < int(binaryMultiple)))
				{
					while (!discarded.empty())
					{
						candidates.push_back(discarded.back());
						discarded.pop_back();
					}
				}

				if (candidates.empty()) freeInjuncts.push_back(current.history);

				//for each node, gather the arrival times that enable the droplet to reach its target within tMax
				vector< vector<int> > arrivalTimes;
				for (unsigned a = 0; a < candidates.size(); a++)
				{
					vector<int> atimes;
					Cell candidate = candidates[a];
					int arr = current.arrival;
					int dist = manhattanDistance(&candidate, &tgt);
					int rem = tMax - arr;
					int span = rem - dist;
					for (int b = arr+1; b <= arr + span; b++)
						atimes.push_back(b);
					arrivalTimes.push_back(atimes);
				}

				//test the arrival times for each candidate node to determine which result in low costs
				vector< vector<int> > itimes;
				for (unsigned a = 0; a < candidates.size(); a++)
				{
					vector<int> temp;
					itimes.push_back(temp);

					Cell candidate = candidates[a];
					vector<int> atimes = arrivalTimes[a];

					double running = numeric_limits<double>::max();
					bool landing = false;
					for (unsigned b = 0; b < atimes.size(); b++)
					{
						int t = atimes[b];
						double cost = costToMove(&graph, &candidate, t, &current, current.arrival);
						if (cost < running)
						{
							running = cost;
							while (!itimes[a].empty()) itimes[a].pop_back();
							itimes[a].push_back(t);
						}
						else if (cost == running)
						{
							if (landing)
							{
								itimes[a].push_back(t);
								landing = false;
							}
						}
						else if (cost > running)
							landing = true;
					}
				}

				//create cells that have these arrivals and push them to Q after modifying their costs and associated histories appropriately

				for (unsigned a = 0; a < candidates.size(); a++)
				{
					for (unsigned b = 0; b < itimes[a].size(); b++)
					{
						Cell toModify = candidates[a];
						toModify.arrival = itimes[a][b];
						toModify.cost = current.cost + costToMove(&graph, &toModify, toModify.arrival, &current, current.arrival);
						if (a == candidates.size()-1 && b == itimes[a].size()-1)
						{
							toModify.history = current.history;
							histories[current.history].push_back(toModify);
						}
						else
						{
							vector<Cell> toUpdate = histories[current.history];
							if (!freeInjuncts.empty()) toModify.history = freeInjuncts.back();
							else toModify.history = histories.size();
							toUpdate.push_back(toModify);
							if (!freeInjuncts.empty()) histories[freeInjuncts.back()] = toUpdate;
							else histories.push_back(toUpdate);
							if (!freeInjuncts.empty()) freeInjuncts.pop_back();
						}
						Q.push(toModify);
					}
				}
			} //ends reach of target

			if (loopSkipper)
			{
				E.push_back(N);
				continue;
			}

			//loop from the target node to the source in the routing tree, updating the departures
			trees[injunct] = histories[current.history];
			tree = trees[injunct];

			while (tree.back() != N.target)
				tree.pop_back();

			if (tree.size() == 1)
			{
				Cell pathNode = tree[0];
				pathNode.departure = tMax;
				tree[0] = pathNode;
			}

			else
			{
				for (int z = tree.size()-1; z >= 1; z--)
				{
					Cell pathNode = tree[z];
					if (z == tree.size() - 1)
					{
						pathNode.departure = tMax;
						tree[z] = pathNode;
					}
					int postArrival = pathNode.arrival;
					Cell priorNode = tree[z-1];
					priorNode.departure = postArrival;
					tree[z-1] = priorNode;
				}
			}
			trees[injunct] = tree;

			//loop again, this time around updating the usage and fluidic penalties
			for (int z = tree.size()-1; z >= 0; z--)
			{
				Cell pathNode = tree[z];
				int ix = pathNode.x, iy = pathNode.y;
				int arr = pathNode.arrival;
				int dep = pathNode.departure;
				int lower;
				if (arr != 0) lower = arr - 1;
				else lower = arr;
				int upper;
				if (!(dep >= tMax)) upper = dep + 1;
				else upper = dep;

				for (int dx = -1; dx <= 1; dx++)
				{
					for (int dy = -1; dy <= 1; dy++)
					{
						Cell nc(ix+dx, iy+dy);
						if (!cellIsBoundedByArray(&nc))
							continue;
						int ux = nc.x, uy = nc.y;
						for (int ts = lower; ts <= upper; ts++)
						{
							graph[ux][uy].s[ts]++;
							graph[ux][uy].currentHFP[ts]++;
							if (dx == 0 && dy == 0) graph[ux][uy].previousHFP[ts]++;
						}
					}
				}
			}
		} // ends while(!E.empty())

		//Determine which nets failed and add them to E (if this is the first pass, include the failed nets from global routing).
		vector<int> failures; //injuncts to failed nets
		for (unsigned a = 0; a < trees.size(); a++)
		{
			vector<Cell> tree = trees[a];
			if (tree.empty() && iterationCount == 0) continue;
			claim(!tree.empty(), "error: one of the routing trees is empty!");

			bool breaker = false;
			for (unsigned b = 0; b < tree.size(); b++)
			{
				Cell pathNode = tree[b];
				int ix = pathNode.x, iy = pathNode.y;
				int arr = pathNode.arrival;
				int dep = pathNode.departure;
				if (arr > 0) arr -= 1;
				timeInterval vInterval(arr, dep);
				for (int dx = -1; dx <= 1; dx++)
				{
					for (int dy = -1; dy <= 1; dy++)
					{
						if (dx == 0 && dy == 0) continue;
						Cell neighborNode(ix+dx, iy+dy);
						if (!cellIsBoundedByArray(&neighborNode)) continue;

						for (unsigned c = 0; c < nets.size(); c++)
						{
							if (c == a) continue;
							vector<Cell> tree2 = trees[c];
							if (tree2.empty() && iterationCount == 0) continue;
							claim(!tree2.empty(), "error: one of the routing trees is empty!");
							int arr2, dep2;
							for (unsigned d = 0; d < tree2.size(); d++)
							{
								Cell pathNode2 = tree2[d];
								if (pathNode2 == neighborNode)
								{
									arr2 = pathNode2.arrival;
									dep2 = pathNode2.departure;
									timeInterval iInterval(arr2, dep2-1);
									if (intervalViolation(vInterval, iInterval))
									{
										Cell tget = nets[a].target;
										int ix2 = neighborNode.x, iy2 = neighborNode.y;
										if ((nets[a].target == nets[c].target) && ((ix == tget.x && iy == tget.y) || (ix2 == tget.x && iy2 == tget.y))) continue;
										failures.push_back(a);
										breaker = true;
									} if (breaker) break;
								} if (breaker) break;
							} if (breaker) break;
						} if (breaker) break;
					} if (breaker) break;
				} if (breaker) break;
			}
		}

		if (iterationCount == 0)
			for (unsigned a = 0; a < routeStatus.size(); a++)
				if (routeStatus[a] == false)
					failures.push_back(a);

		vector<int>::iterator failuresBegin = failures.begin();
		vector<int>::iterator failuresEnd = failures.end();
		std::sort(failuresBegin, failuresEnd);

		if (iterationCount != 0)
		{
			for (unsigned i = 0; i < failuresMemory.size(); i++)
			{
				vector<int> test = failuresMemory[i];
				vector<int>::iterator testBegin = test.begin();
				vector<int>::iterator testEnd = test.end();
				std::sort(testBegin, testEnd);
				if (failures == test)
				{
					test = failuresMemory[i];
					testBegin = test.begin();
					testEnd = test.end();
					if (next_permutation(testBegin, testEnd))
					{
						failures = test;
						failuresMemory[i] = failures;
					}
					else
					{
						failuresMemory[i] = failures;
					}
					break;
				}
				else if (i == failuresMemory.size()-1)
				{
					failuresMemory.push_back(failures);
					i = failuresMemory.size();
				}
			}
		}

		vector<Net> failedNets;
		for (unsigned z = 0; z < failures.size(); z++)
		{
			int finjunct = failures[z];
			Net N = nets[finjunct];
			failedNets.push_back(N);
		}

		//sort(&failedNets, DOWN);  //undoes the effects of randomization above
		for (unsigned j = 0; j < failedNets.size(); j++)
			E.push_back(failedNets[j]);

		iterationCount++;
	}

	claim ((iterationCount < maximumIterationLimit), "error: detailed routing failed for maximum iteration limit");
	//cout << "completed!" << endl;

	// finally, using the routing trees, perform the actions required to plug into Dan's framework

	RoutePoint *lrp = NULL; //last route point
	RoutePoint *nrp = NULL; //next route point

	int numberOfSubProblems = nets.size();
	for (int iteration = 0; iteration < numberOfSubProblems; iteration++)
	{
		routeCycle = cycle;
		Droplet * pd = subDrops->at(iteration);
		vector< RoutePoint *> * sr = subRoutes->at(iteration);
		Cell source = sources.at(iteration);
		Cell target = targets.at(iteration);
		vector<int> attribute = attributes.at(iteration);

		bool initialize = false;
		for (unsigned i = 0; i < attribute.size(); i++)
		{
			if (attribute.at(i) == CREATIVE)
			{
				initialize = true;
				break;
			}
		}
		if (initialize)
		{
			nrp = new RoutePoint();
			nrp->cycle = routeCycle++;
			nrp->dStatus = DROP_NORMAL;
			nrp->x = source.x;
			nrp->y = source.y;
			sr->push_back(nrp);
			lrp = nrp;
		}
		else
		{
			lrp = (*routes)[pd]->back();
		}

		vector<Cell> tree = trees[iteration];
		for (unsigned i = 0; i < tree.size(); i++)
		{
			Cell step = tree[i];
			int arr = step.arrival;
			int dep = step.departure;

			if (i == tree.size() - 1) dep = arr + 1;
			for (int t = arr; t < dep; t++)
			{
				if (i == 0 && t == arr && initialize) continue;
				nrp = new RoutePoint();
				nrp->cycle = routeCycle++;
				if (t == arr)
					nrp->dStatus = DROP_NORMAL;
				else nrp->dStatus = DROP_WAIT;
				for (unsigned j = 0; j < attribute.size(); j++)
				{
					if (attribute.at(j) == DISMISSIVE)
					{
						nrp->dStatus = DROP_OUTPUT;
						break;
					}
				}
				nrp->x = step.x;
				nrp->y = step.y;
				sr->push_back(nrp);
				lrp = nrp;
			}
		}
	}
	cleanUp();
	cout << " - Complete." << endl;
}

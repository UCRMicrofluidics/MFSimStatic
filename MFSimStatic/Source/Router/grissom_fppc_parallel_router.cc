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
 * Source: grissom_fppc_router.cc											*
 * Original Code Author(s): Dan Grissom											*
 * Original Completion/Release Date: December 11, 2012							*
 *																				*
 * Details: N/A																	*
 *																				*
 * Revision History:															*
 * WHO		WHEN		WHAT													*
 * ---		----		----													*
 * FML		MM/DD/YY	One-line description									*
 *-----------------------------------------------------------------------------*/
#include "../../Headers/Router/grissom_fppc_parallel_router.h"

///////////////////////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////////////////////
GrissomFppcParallelRouter::GrissomFppcParallelRouter()
{
	thisTS = new vector<AssayNode *>();
	cellType = new vector<vector<ResourceType> *>();
	fppcPinMapper = NULL;
	mHoldPins = NULL;
	mIOPins = NULL;
	ssHoldPins = NULL;
	ssIOPins = NULL;
	mixPins = NULL;
	pinMapping = NULL;
	centralRoutingColumn = -1;
	routeOffsets = new vector<int>();
	pinsForcedOffAtCycle = new map<unsigned long long, vector<int> *>();
	dirtyArray = new vector<vector<vector<pair<int, int> > *> *>();
	dropletOrderArray = new vector<vector<vector<pair<int, int> > *> *>();

}
GrissomFppcParallelRouter::GrissomFppcParallelRouter(DmfbArch *dmfbArch)
{
	arch = dmfbArch;
	thisTS = new vector<AssayNode *>();
	cellType = new vector<vector<ResourceType> *>();
	fppcPinMapper = NULL;
	mHoldPins = NULL;
	mIOPins = NULL;
	ssHoldPins = NULL;
	ssIOPins = NULL;
	mixPins = NULL;
	pinMapping = NULL;
	centralRoutingColumn = -1;
	washIn = NULL;
	washOut = NULL;
	routeOffsets = new vector<int>();
	pinsForcedOffAtCycle = new map<unsigned long long, vector<int> *>();
	dirtyArray = new vector<vector<vector<pair<int, int> > *> *>();
	dropletOrderArray = new vector<vector<vector<pair<int, int> > *> *>();
}

///////////////////////////////////////////////////////////////////////////////////
// Deconstructor
///////////////////////////////////////////////////////////////////////////////////
GrissomFppcParallelRouter::~GrissomFppcParallelRouter()
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

	while (!dirtyArray->empty())
	{
		vector<vector<pair<int, int> > *> *vv = dirtyArray->back();
		dirtyArray->pop_back();
		while (!vv->empty())
		{
			vector<pair<int, int> > *v = vv->back();
			vv->pop_back();
			v->clear();
			delete v;
		}
		delete vv;
	}
	delete dirtyArray;

	while (!dropletOrderArray->empty())
	{
		vector<vector<pair<int, int> > *> *vv = dropletOrderArray->back();
		dropletOrderArray->pop_back();
		while (!vv->empty())
		{
			vector<pair<int, int> > *v = vv->back();
			vv->pop_back();
			v->clear();
			delete v;
		}
		delete vv;
	}
	delete dropletOrderArray;

	delete routeOffsets;

	map<unsigned long long, vector<int> *>::iterator mapIt = pinsForcedOffAtCycle->begin();
	for (; mapIt != pinsForcedOffAtCycle->end(); mapIt++)
	{
		vector<int> *v = mapIt->second;
		v->clear();
		delete v;
	}
	pinsForcedOffAtCycle->clear();
	delete pinsForcedOffAtCycle;
	// Do not need to delete all the variables initialized as NULL in constructor
	// because they are only referenced here and will be deleted elsewhere.
}

///////////////////////////////////////////////////////////////////////
// This function performs any one-time initializations that the router
// needs that are specific to a particular router.
///////////////////////////////////////////////////////////////////////
void GrissomFppcParallelRouter::routerSpecificInits()
{
//	// Compute the routing column
//	// routingColumn = mixModule br + 2
//
//
//
//	// These dimensions must correspond with the same ones in pin_constrained_placer_1.cc
//	int mWidth = 4; // md = mix/heat/detect
//	int mHeight = 2;
//	int ssWidth = 1; // md = split/store
//	int ssHeight = 1;
//	routingColumn = 3 + mWidth;
//
//	// Create a 2D-array which tells if a cell is augmented with a heater or detector
//	pinMapping = new vector<vector<int> *>();
//	for (int x = 0; x < arch->getNumCellsX(); x++)
//	{
//		vector<int> *pinCol = new vector<int>();
//		for (int y = 0; y < arch->getNumCellsY(); y++)
//			pinCol->push_back(0);
//		pinMapping->push_back(pinCol);
//	}
//
//
//	// Now, set-up the actual mapping for the array
//	int pinNo = 1;
//	// 1-2-3 for 2 horizontal paths
//	for (int x = 0; x < arch->getNumCellsX(); x++)
//	{
//		pinMapping->at(x)->at(0) = (x % 3) + pinNo;
//		pinMapping->at(x)->at(arch->getNumCellsY()-1) = (x % 3) + pinNo;
//	}
//	pinNo += 3;
//
//	// 4-5-6 for 3 vertical paths
//	for (int y = 1; y < arch->getNumCellsY()-1; y++)
//	{
//		pinMapping->at(0)->at(y) = ((y-1) % 3) + pinNo;
//		pinMapping->at(mWidth + 3)->at(y) = ((y-1) % 3) + pinNo;
//		pinMapping->at(arch->getNumCellsX()-1)->at(y) = ((y-1) % 3) + pinNo;
//	}
//	pinNo += 3;
//
//	// 7-8-9-10-11-12-13 for mixers (2x4 mixers)
//	// 14+ for all other independent pins
//	// First calculate mix/detect resources
//	int x = 2; // Always start here
//	int y = 2; // Always start here
//
//	// While there is enough room for this module
//	int modStartPinNo = pinNo;
//	pinNo += (mWidth*mHeight-1);
//	while (y + mHeight + 2 <= arch->getNumCellsY())
//	{
//		int modPinNo = modStartPinNo;
//
//		// The bottom row, next to right-most cell in module is independent;
//		// Otherwise, give each module a tied pin number
//		for (int modY = y; modY < y+mHeight; modY++)
//		{
//			for (int modX = x; modX < x+mWidth; modX++)
//			{
//				if (!(modX == x+mWidth-2 && modY == y+mHeight-1))
//					pinMapping->at(modX)->at(modY) = modPinNo++;
//				else
//				{
//					pinMapping->at(modX)->at(modY) = pinNo;
//					mHoldPins.push_back(pinNo);
//					pinNo++;
//				}
//			}
//		}
//
//		// Also, set I/O cell to bottom-right
//		pinMapping->at(x+mWidth)->at(y+mHeight-1) = pinNo;
//		mIOPins.push_back(pinNo);
//		pinNo++;
//
//		y = y + mHeight + 1;
//	}
//
//	// Grab the mixer pins; start with pin to left of hold pin and travel clockwise
//	// Just base off first module, whose top-left coordinate is (2,2)
//	int mx = 2+mWidth-2;
//	int my = 2+mHeight-1;
//	while (mx > 2) // Left
//		mixPins.push_back(pinMapping->at(--mx)->at(my));
//	while (my > 2) // Up
//		mixPins.push_back(pinMapping->at(mx)->at(--my));
//	while (mx < 2+mWidth-1) // Right
//		mixPins.push_back(pinMapping->at(++mx)->at(my));
//	while (my < 2+mHeight-1) // Down
//		mixPins.push_back(pinMapping->at(mx)->at(++my));
//
//	// Now, compute the number of storage/split modules we can fit
//	int numSsModSpots = ((arch->getNumCellsY() - 3) / (ssHeight + 1));
//	x = mWidth + 5; // Starting location for split/store modules
//	y = 2; // Starting location for split/store modules
//
//	for (int i = 1; i <= numSsModSpots; i++)
//	{
//		pinMapping->at(x-1)->at(y) = pinNo;
//		ssIOPins.push_back(pinNo);
//		pinNo++;
//		for (int modY = y; modY < y+ssHeight; modY++)
//		{
//			for (int modX = x; modX < x+ssWidth; modX++)
//			{
//				pinMapping->at(modX)->at(modY) = pinNo;
//				ssHoldPins.push_back(pinNo);
//				pinNo++;
//			}
//		}
//
//		y = y + ssHeight + 1;
//	}
//
//	// Set pin numbers for the I/O ports
//	for (int i = 0; i < arch->getIoPorts()->size(); i++)
//		arch->getIoPorts()->at(i)->setPinNo(pinNo++);


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
void GrissomFppcParallelRouter::processTimeStep(map<Droplet *, vector<RoutePoint *> *> *routes)
{
//	if (processEngineType == FIXED_QUICK_PE)
//		processFixPlaceTSQuick(routes);
//	else if (processEngineType == FIXED_FULL_PE)
//		processFixPlaceTSFull(routes);
//	else { /* Do not process - will probably result in some kind of errors */ }
//
//	startingTS++; // Time-step is complete; Advance to the next time-step
//	cycle += cyclesPerTS; // Advance cycle number by whole time-step

	GrissomFppcPinMapper *gfpm = (GrissomFppcPinMapper *)arch->getPinMapper();
	vector<int> *ssHoldPins = gfpm->getSsHoldPins();
	vector<vector<int> *> *mixPins = gfpm->getMixPins();
	unsigned long long tsEndCycle = cycle + cyclesPerTS;

	while (cycle < tsEndCycle)
	{
		// If can make another revolution around the module, do it!
		if (cycle < tsEndCycle - mixPins->back()->size() - 2)
		{
			for (unsigned i = 0; i < mixPins->back()->size(); i++)
			{
				// Activate mix pins for each mix group
				for (unsigned j = 0; j < mixPins->size(); j++)
					addPinAtCycle(mixPins->at(j)->at(i), cycle, false);

				// Also, activate the S/S pins
				for (unsigned j = 0; j < ssHoldPins->size(); j++)
					addPinAtCycle(ssHoldPins->at(j), cycle, false);
				cycle++;
			}
			addPinAtCycle(-1, cycle++, true);// activate all holds to complete revolution
		}
		else // Cause all to hold and wait for TS to end
			addPinAtCycle(-1, cycle++, true);
	}
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
/*void GrissomFppcParallelRouter::computeIndivSupProbRoutes(vector<vector<RoutePoint *> *> *subRoutes, vector<Droplet *> *subDrops, map<Droplet *, vector<RoutePoint *> *> *routes)
{
	//claim(false, "No valid router was selected for the synthesis process or no method for 'computeIndivSupProbRoutes()' was implemented for the selected router.\n");
	ModuleDependenceGraph mdg;

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
			AssayNode *par = n->GetParents().at(p);
			mdg.AddDependency(par, n);
		}
	}

	//mdg.PrintDependencies();
	mdg.FindAllConnectedComponents();
	//mdg.PrintConnectedComponents();
	mdg.FindAndResolveDependencies();
	mdg.RevTopSortSCCs();
	//mdg.PrintConnectedComponents();

	// Now, do actual routing of droplets
	vector<vector<ModuleDependenceVertex *> *> cc = mdg.getConnectedComponents();
	for (unsigned i = 0; i < cc.size(); i++)
	{
		// Determine if there is a re-route that needs to take place
		AssayNode *rb = NULL; // Routing buffer node (created if necessary)
		AssayNode *rrNode = NULL; // Re-routed node (created if necessary)

		for (unsigned j = 0; j < cc.at(i)->size(); j++)
		{
			if (cc.at(i)->at(j)->reRouteVertex)
			{
				// Create a new routing-buffer (rb) node with rb module and reRouteVertex-node parent
				// and add to front of connected components so it's done first

				rb = new AssayNode(GENERAL); // Will call it a general op
				rb->startTimeStep = startingTS;
				rb->endTimeStep = startingTS;
				rb->status = BOUND;
				FixedModule *fm = arch->getPinMapper()->getAvailRes()->at(RB_RES)->front();
				rb->reconfigMod = new ReconfigModule(RB_RES, fm->getLX(), fm->getTY(), fm->getRX(), fm->getBY());
				rb->reconfigMod->setTileNum(fm->getTileNum());
				rb->parents.push_back(cc.at(i)->at(j)->reRouteVertex->operation);
				rrNode = rb->parents.back();
				ModuleDependenceVertex *mdv = new ModuleDependenceVertex();
				mdv->operation = rb;
				cc.at(i)->insert(cc.at(i)->begin(), mdv);

				//cout << "TS " << startingTS << " (cycle " << cycle << "): Rerouting " << rb->parents.back()->name << " to avoid routing deadlock." << endl;
				break;
			}
		}

		for (unsigned j = 0; j < cc.at(i)->size(); j++)
		{
			ModuleDependenceVertex *mdv = cc.at(i)->at(j);
			AssayNode *n = mdv->operation;
			//bool isReRouting = false;
			//if (mdv->reRouteVertex && n == mdv->reRouteVertex->operation)
			//	isReRouting = true; // Does node need to be re-routed to avoid deadlock

			if (n->startTimeStep == startingTS && n->status != ROUTED)
			{
				n->status = ROUTED;
				int tx;
				int ty;
				int x;
				int y;
				int topY = 0;
				int botY = arch->getNumCellsY()-1;
				int leftX = 0;
				int rightX = arch->getNumCellsX()-1;

				for (unsigned p = 0; p < n->GetParents().size(); p++)
				{
					// If routing to a module, first job is to get the droplet to the routingColumn.....
					if (n->GetType() != OUTPUT)
					{
						AssayNode *par = n->GetParents().at(p);

						// If the parent node is the re-routed node, then we start at the routing buffer
						if (par && rrNode && par->reconfigMod && rrNode->reconfigMod &&
								par->reconfigMod->getTY() == rrNode->reconfigMod->getTY() &&
								par->reconfigMod->getLX() == rrNode->reconfigMod->getLX() &&
								n != rb)
						{
							par = rb;
						}

						// If destination is not same module as source, then route
						if (!(par->type != DISPENSE && n->boundedResType == par->boundedResType && n->reconfigMod->getTileNum() == par->reconfigMod->getTileNum()))
						{
							tx = centralRoutingColumn;
							ty = n->reconfigMod->getBY();

							if (par->GetType() == DISPENSE)
							{
								IoPort *port = par->GetIoPort();
								y = routeFromInputToCentralColumn(port, tx, ty);
								x = tx;
							}
							else // Must get the parent droplet out of its module and into the routing column
							{
								// If departing from a basic resource on the left
								if (par->boundedResType == BASIC_RES)
								{
									x = centralRoutingColumn;
									y = extractDropletFromMixModule(par->GetReconfigMod());
								}
								else // Departing from a SS_RES/RB_RES on the right
								{
									x = centralRoutingColumn;
									y = extractDropletFromSSDModule(par->GetReconfigMod());
								}
							}

							////////////////////////////////////////////////////
							// Okay, now we are somewhere in the routing column, just go up or down till get to right row
							y = routeDropletAlongVerticalColumn(y, ty, centralRoutingColumn);

							////////////////////////////////////////////////////
							// Now, have the droplet enter the module to left if
							// mixer; to right if split/store module
							if (n->boundedResType == BASIC_RES)
								insertDropletIntoMixModule(n->GetReconfigMod());
							else // Routing to SS_RES
							{
								insertDropletIntoSSDModule(n->GetReconfigMod());
								// Now, if a SPLIT, need to route in, split, and then route to any children STORAGE nodes
								if (n->GetType() == SPLIT)
									splitNodeIntoSecondSSDModule(n);
							}
						}
					}
					else // If routing to OUTPUT node
					{
						////////////////////////////////////////////////////
						// Must First get the parent droplet out of its module
						// and into the routing column
						AssayNode *par = n->GetParents().at(0); // Output nodes only have one parent
						if (par->boundedResType == BASIC_RES) // If departing from a basic resource on the left
						{
							x = centralRoutingColumn;
							y = extractDropletFromMixModule(par->GetReconfigMod());
						}
						else // Departing from a SS_RES on the right
						{
							x = centralRoutingColumn;
							y = extractDropletFromSSDModule(par->GetReconfigMod());
						}

						////////////////////////////////////////////////////
						// Now that we are in the routing column, route to output port
						IoPort *port = n->GetIoPort();

						////////////////////////////////////////////////////
						// First get to the top/bottom row
						if (port->getSide() == WEST || port->getSide() == EAST)
						{
							if (port->getSide() == WEST)
								tx = leftX;
							else
								tx = rightX;
							ty = port->getPosXY();

							int northFirstDist = y + ty;
							int southFirstDist = (arch->getNumCellsY() - y) + (arch->getNumCellsY() - ty);

							if (northFirstDist >= southFirstDist)
								y = routeDropletAlongVerticalColumn(y, botY, centralRoutingColumn);
							else
								y = routeDropletAlongVerticalColumn(y, topY, centralRoutingColumn);
						}
						else if (port->getSide() == NORTH || port->getSide() == SOUTH)
						{
							tx = port->getPosXY();
							if (port->getSide() == NORTH)
								ty = topY;
							else
								ty = botY;
							y = routeDropletAlongVerticalColumn(y, ty, centralRoutingColumn);
						}

						////////////////////////////////////////////////////
						// Now, go right or left to get to the get to the N/S port or the W/E column
						x = routeDropletAlongHorizontalRow(x, tx, y);

						////////////////////////////////////////////////////
						// Now, either output drop to N/S port or travel N/S to get to W/E port and output there
						if (port->getSide() == NORTH || port->getSide() == SOUTH)
							addPinAtCycle(port->getPinNo(), cycle++, true);
						else
						{
							y = routeDropletAlongVerticalColumn(y, ty, x);
							addPinAtCycle(port->getPinNo(), cycle++, true);
						}
					}
				}
			}
		}
	}
}*/

///////////////////////////////////////////////////////////////////////////////////
// Creates and adds a new route point to sub-routes at the given index (dropNum).
///////////////////////////////////////////////////////////////////////////////////
RoutePoint *GrissomFppcParallelRouter::addNewRoutePoint(unsigned dropNum, int x, int y, vector<vector<RoutePoint *> *> *subRoutes)
{
	RoutePoint *rp = new RoutePoint();
	rp->x = x;
	rp->y = y;

	while (dropNum >= subRoutes->size())
		subRoutes->push_back(new vector<RoutePoint*>());
	subRoutes->at(dropNum)->push_back(rp);

	return rp;
}

///////////////////////////////////////////////////////////////////////////////////
// Given an input port and the target XY coordinates, actiavtes the pins to get the
// droplet from the input to the top or bottom of the central routing column.
// Returns y coordinate to tell if droplet is at top/bottom of column).
///////////////////////////////////////////////////////////////////////////////////
int GrissomFppcParallelRouter::routeFromInputToCentralColumn(IoPort *inPort, int tx, int ty, vector<vector<RoutePoint *> *> *subRoutes)
{
	vector<vector<int> *> *pinMapping = arch->getPinMapper()->getPinMapping();
	int x;
	int y;
	int topY = 0;
	int botY = arch->getNumCellsY()-1;
	int leftX = 0;
	int rightX = arch->getNumCellsX()-1;
	int routingColumn = tx;


	//addPinAtCycle(inPort->getPinNo(), cycle, false);

	////////////////////////////////////////////////////
	// First get to the top/bottom row
	if (inPort->getSide() == WEST || inPort->getSide() == EAST)
	{
		if (inPort->getSide() == WEST)
			x = leftX;
		else
			x = rightX;
		y = inPort->getPosXY();
		//addPinAtCycle(pinMapping->at(x)->at(y), cycle++, true);
		addNewRoutePoint(dropNum, x, y, subRoutes);

		int northFirstDist = inPort->getPosXY() + ty;
		int southFirstDist = (arch->getNumCellsY() - inPort->getPosXY()) + (arch->getNumCellsY() - ty);
		if (northFirstDist >= southFirstDist)
			y = routeDropletAlongVerticalColumn(y, botY, x, subRoutes);
		else
			y = routeDropletAlongVerticalColumn(y, topY, x, subRoutes);
	}
	else if (inPort->getSide() == NORTH || inPort->getSide() == SOUTH)
	{
		x = inPort->getPosXY();
		if (inPort->getSide() == NORTH)
			y = topY;
		else
			y = botY;
		//addPinAtCycle(pinMapping->at(x)->at(y), cycle++, true);
		addNewRoutePoint(dropNum, x, y, subRoutes);
	}

	////////////////////////////////////////////////////
	// Now, go right or left to get to the routing column
	x = routeDropletAlongHorizontalRow(x, routingColumn, y, subRoutes);

	return y;
}

///////////////////////////////////////////////////////////////////////////////////
// Extracts a droplet from the given mix/basic module into the vertical routing
// column. Returns the y dimension that the droplet ends up at.
///////////////////////////////////////////////////////////////////////////////////
int GrissomFppcParallelRouter::extractDropletFromMixModule(ReconfigModule *rm, vector<vector<RoutePoint *> *> *subRoutes)
{
	claim(rm->getResourceType() == BASIC_RES, "FPPC router expecting to extract droplet from a basic mixing module but given a module of a different type.");
	// Activate the SS_RES holders to keep them in place
	/*for (unsigned k = 0; k < ssHoldPins->size(); k++)
		addPinAtCycle(ssHoldPins->at(k), cycle, false);
	// Also activate the bottom-right mixer pins
	for (unsigned k = 0; k < mixPins->size(); k++)
		addPinAtCycle(mixPins->at(k)->back(), cycle, false);
	cycle++;

	// Activate the SS_RES holders...
	for (unsigned k = 0; k < ssHoldPins->size(); k++)
		addPinAtCycle(ssHoldPins->at(k), cycle, false);
	// then, activate all the holders except the mix module we are departing
	// from...activate it's I/O pin instead
	for (unsigned k = 0; k < mHoldPins->size(); k++)
	{
		if (k == rm->getTileNum())
			addPinAtCycle(mIOPins->at(k), cycle, false);
		else
			addPinAtCycle(mHoldPins->at(k), cycle, false);
	}
	cycle++;

	// Now, activate all the holders and the pin to get the droplet into the street
	addPinAtCycle(pinMapping->at(centralRoutingColumn)->at(rm->getBY()), cycle++, true);*/

	addNewRoutePoint(dropNum, rm->getRX()-1, rm->getBY(), subRoutes);
	addNewRoutePoint(dropNum, rm->getRX(), rm->getBY(), subRoutes);
	addNewRoutePoint(dropNum, rm->getRX()+1, rm->getBY(), subRoutes);
	addNewRoutePoint(dropNum, rm->getRX()+2, rm->getBY(), subRoutes);
	return rm->getBY();
}

///////////////////////////////////////////////////////////////////////////////////
// Extracts a droplet from the given SSD module into the vertical routing
// column. Returns the y dimension that the droplet ends up at.
///////////////////////////////////////////////////////////////////////////////////
int GrissomFppcParallelRouter::extractDropletFromSSDModule(ReconfigModule *rm, vector<vector<RoutePoint *> *> *subRoutes)
{
	claim(rm->getResourceType() == SSD_RES, "FPPC router expecting to extract droplet from a SSD module but given a module of a different type.");
	// Activate all the mix holders; then activate all the split/store holds
	// except the s/s module we are departing from...activate it's I/O pin instead
	/*for (unsigned k = 0; k < mHoldPins->size(); k++)
		addPinAtCycle(mHoldPins->at(k), cycle, false);
	for (unsigned k = 0; k < ssHoldPins->size(); k++)
	{
		if (k == rm->getTileNum())
			addPinAtCycle(ssIOPins->at(k), cycle, false);
		else
			addPinAtCycle(ssHoldPins->at(k), cycle, false);
	}
	cycle++;

	// Now, Activate all m holds, and all s/s holds, except the s/s module we
	// are departing from...just activate the pin to get the droplet into the street
	for (unsigned k = 0; k < mHoldPins->size(); k++)
		addPinAtCycle(mHoldPins->at(k), cycle, false);
	for (unsigned k = 0; k < ssHoldPins->size(); k++)
		if (k != rm->getTileNum())
			addPinAtCycle(ssHoldPins->at(k), cycle, false);
	addPinAtCycle(pinMapping->at(centralRoutingColumn)->at(rm->getBY()), cycle, false);
	cycle++;*/

	addNewRoutePoint(dropNum, rm->getLX(), rm->getBY(), subRoutes);
	addNewRoutePoint(dropNum, rm->getLX()-1, rm->getBY(), subRoutes);
	addNewRoutePoint(dropNum, rm->getLX()-2, rm->getBY(), subRoutes);

	return rm->getBY();
}

///////////////////////////////////////////////////////////////////////////////////
// Inserts a droplet into the given mix/basic module from the vertical routing
// column.
///////////////////////////////////////////////////////////////////////////////////
void GrissomFppcParallelRouter::insertDropletIntoMixModule(ReconfigModule *rm, vector<vector<RoutePoint *> *> *subRoutes)
{
	claim(rm->getResourceType() == BASIC_RES, "FPPC router expecting to insert droplet into a basic mixing module but given a module of a different type.");
	/*addPinAtCycle(mIOPins->at(rm->getTileNum()), cycle++, true);

	//addPinAtCycle(pinMapping->at(n->GetReconfigMod()->getRX())->at(n->GetReconfigMod()->getBY()), cycle++, true);
	// Activate the SS_RES holders to keep them in place, and also activate the bottom-right mixer pin
	for (unsigned k = 0; k < ssHoldPins->size(); k++)
		addPinAtCycle(ssHoldPins->at(k), cycle, false);
	// Also activate the bottom-right mixer pins
	for (unsigned k = 0; k < mixPins->size(); k++)
		addPinAtCycle(mixPins->at(k)->back(), cycle, false);
	cycle++;
	addPinAtCycle(-1, cycle++, true);*/

	addNewRoutePoint(dropNum, rm->getRX()+1, rm->getBY(), subRoutes);
	addNewRoutePoint(dropNum, rm->getRX(), rm->getBY(), subRoutes);
	RoutePoint *rp = addNewRoutePoint(dropNum, rm->getRX()-1, rm->getBY(), subRoutes);
	rp->dStatus = DROP_WAIT;

}

///////////////////////////////////////////////////////////////////////////////////
// Inserts a droplet into the given SSD module into the vertical routing
// column. Returns the y dimension that the droplet ends up at.
///////////////////////////////////////////////////////////////////////////////////
void GrissomFppcParallelRouter::insertDropletIntoSSDModule(ReconfigModule *rm, vector<vector<RoutePoint *> *> *subRoutes)
{
	claim(rm->getResourceType() == SSD_RES, "FPPC router expecting to insert droplet into a SSD module but given a module of a different type.");
	/*addPinAtCycle(ssIOPins->at(rm->getTileNum()), cycle++, true);
	addPinAtCycle(-1, cycle++, true);*/

	addNewRoutePoint(dropNum, rm->getLX()-1, rm->getBY(), subRoutes);
	RoutePoint *rp = addNewRoutePoint(dropNum, rm->getLX(), rm->getBY(), subRoutes);
	rp->dStatus = DROP_WAIT;
}

///////////////////////////////////////////////////////////////////////////////////
// This funciton should be called once the droplet that is about to be split has
// already been routed to an SSD module. At that point, this function will split
// the droplet into 2 and cause the 2nd droplet to be routed to a second SSD
// module.
///////////////////////////////////////////////////////////////////////////////////
void GrissomFppcParallelRouter::splitNodeIntoSecondSSDModule(AssayNode *s, vector<vector<RoutePoint *> *> *subRoutes)
{
	claim(s->GetType() == SPLIT, "FPPC router expecting to split a droplet, but droplet is not originating from a split node.");
	AssayNode *c0;
	AssayNode *c1;

	int x = centralRoutingColumn;
	int y = s->reconfigMod->getTY();

	if (s->reconfigMod->getTY() == s->GetChildren().at(0)->reconfigMod->getTY())
	{
		c0 = s->GetChildren().at(0);
		c1 = s->GetChildren().at(1);
	}
	else if (s->reconfigMod->getTY() == s->GetChildren().at(1)->reconfigMod->getTY())
	{
		c0 = s->GetChildren().at(1);
		c1 = s->GetChildren().at(0);
	}
	else
		claim(false, "A split's immediate storage children are in different modules than the split. Binder should not allow this.");

	////////////////////////////////////////////////////
	// One droplet stays in c0's module, move other droplet out and to c1
	//addPinAtCycle(ssIOPins->at(s->GetReconfigMod()->getTileNum()), cycle++, true);
	//addPinAtCycle(pinMapping->at(x)->at(y), cycle++, true);
	//dropNum++; // New droplet b/c of split
	RoutePoint *rp = addNewRoutePoint(dropNum, s->GetReconfigMod()->getLX()-1, y, subRoutes);
	rp->dStatus = DROP_SPLITTING;
	addNewRoutePoint(dropNum, x, y, subRoutes);


	int tx = centralRoutingColumn;
	int ty = c1->reconfigMod->getBY();

	////////////////////////////////////////////////////
	// Okay, now we are somewhere in the routing column,
	// just go up or down till get to right row
	while (y < ty)
		addNewRoutePoint(dropNum, x, ++y, subRoutes);
		//addPinAtCycle(pinMapping->at(x)->at(++y), cycle++, true);
	while (y > ty)
		addNewRoutePoint(dropNum, x, --y, subRoutes);
		//addPinAtCycle(pinMapping->at(x)->at(--y), cycle++, true);

	////////////////////////////////////////////////////
	// Finally, route droplet into the SS_RES module
	//addPinAtCycle(ssIOPins->at(c1->GetReconfigMod()->getTileNum()), cycle++, true);
	//addPinAtCycle(-1, cycle++, true);

	addNewRoutePoint(dropNum, c1->GetReconfigMod()->getLX()-1, y, subRoutes);
	rp = addNewRoutePoint(dropNum, c1->GetReconfigMod()->getLX(), y, subRoutes);
	rp->dStatus = DROP_WAIT;
}


///////////////////////////////////////////////////////////////////////////////////
// Adds the pin to the list of pins to be activated at the specified cycle
// If activateHolds is true, also adds to pins for the module holds.
// If pass pinNo==-1, won't add to list...this is a way to simply activate all
// the holders.
///////////////////////////////////////////////////////////////////////////////////
void GrissomFppcParallelRouter::addPinAtCycle(int pinNo, int cycleNo, bool activateHolds)
{
	GrissomFppcPinMapper *gfpm = (GrissomFppcPinMapper *)arch->getPinMapper();
	vector<int> *ssHoldPins = gfpm->getSsHoldPins();
	vector<int> *mHoldPins = gfpm->getMHoldPins();

	while (pinActs->size() <= cycleNo)
		pinActs->push_back(new vector<int>());

	if (pinNo >= 0)
		pinActs->at(cycleNo)->push_back(pinNo);

	if (activateHolds)
	{
		for (unsigned i = 0; i < mHoldPins->size(); i++)
			pinActs->at(cycleNo)->push_back(mHoldPins->at(i));
		for (unsigned i = 0; i < ssHoldPins->size(); i++)
			pinActs->at(cycleNo)->push_back(ssHoldPins->at(i));
	}
}

///////////////////////////////////////////////////////////////////////////////////
// Given a start y position, routes a droplet along the vertical routing column
// (given by the x coordinate) till it gets to the end y position.
///////////////////////////////////////////////////////////////////////////////////
int GrissomFppcParallelRouter::routeDropletAlongVerticalColumn(int startY, int endY, int x, vector<vector<RoutePoint *> *> *subRoutes)
{
	int y = startY;
	while (y < endY)
		addNewRoutePoint(dropNum, x, ++y, subRoutes);
		//addPinAtCycle(pinMapping->at(x)->at(++y), cycle++, true);
	while (y > endY)
		addNewRoutePoint(dropNum, x, --y, subRoutes);
		//addPinAtCycle(pinMapping->at(x)->at(--y), cycle++, true);
	return y;
}

///////////////////////////////////////////////////////////////////////////////////
// Given a start x position, routes a droplet along the horizontal routing column
// (given by the y coordinate) till it gets to the end x position.
///////////////////////////////////////////////////////////////////////////////////
int GrissomFppcParallelRouter::routeDropletAlongHorizontalRow(int startX, int endX, int y, vector<vector<RoutePoint *> *> *subRoutes)
{
	int x = startX;
	while (x < endX)
		addNewRoutePoint(dropNum, ++x, y, subRoutes);
		//addPinAtCycle(pinMapping->at(++x)->at(y), cycle++, true);
	while (x > endX)
		addNewRoutePoint(dropNum, --x, y, subRoutes);
		//addPinAtCycle(pinMapping->at(--x)->at(y), cycle++, true);
	return x;
}

///////////////////////////////////////////////////////////////////////////////////
// This function is the main public function called.  It fills the "routes" and
// "tsBeginningCycle" data structures and contains the code for the main routing flow.
///////////////////////////////////////////////////////////////////////////////////
void GrissomFppcParallelRouter::route(DAG *dag, DmfbArch *arch, vector<ReconfigModule *> *rModules, map<Droplet *, vector<RoutePoint *> *> *routes, vector<vector<int> *> *pinActivations, vector<unsigned long long> *tsBeginningCycle)
{
	// Initializations
	fppcPinMapper = (GrissomFppcPinMapper *)arch->getPinMapper();
	mHoldPins = fppcPinMapper->getMHoldPins();
	mIOPins = fppcPinMapper->getMIOPins();
	ssHoldPins = fppcPinMapper->getSsHoldPins();
	ssIOPins = fppcPinMapper->getSsIOPins();
	mixPins = fppcPinMapper->getMixPins();
	pinMapping = fppcPinMapper->getPinMapping();
	centralRoutingColumn = fppcPinMapper->getRoutingColumn();
	//cout << "All nodes: " << dag->getAllNodes().size() << endl;
	pinActs = pinActivations;
	cyclesPerTS = arch->getSecPerTS() * arch->getFreqInHz();
	startingTS = 0; // The TS that is getting ready to begin (droplets are being routed to)
	routeCycle = 0;
	cycle = 0;
	preCompactionRouteLength = 0;
	postCompactionRouteLength = 0;

	// Copy all nodes to a new list to be sorted
	vector<AssayNode *> nodes;// = new vector<AssayNode *>();
	for (unsigned i = 0; i < dag->getAllNodes().size(); i++)
		nodes.push_back(dag->getAllNodes().at(i));
	Sort::sortNodesByStartThenEndTS(&nodes);

	// Init dirty cells array
	for (int x = 0; x < arch->getNumCellsX(); x++)
	{
		dirtyArray->push_back(new vector<vector<pair<int, int> > *>());
		dropletOrderArray->push_back(new vector<vector<pair<int, int> > *>());
		for (int y = 0; y < arch->getNumCellsY(); y++)
		{
			dirtyArray->back()->push_back(new vector<pair<int, int> >());
			dropletOrderArray->back()->push_back(new vector<pair<int, int> >());
		}
	}

	//initCellTypeArray();
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

		//cout << "DebugPrint: Routing to TS " << startingTS << "." << endl; // DTG Debug Print

		///////////////////////////////////////////////////////////////////////
		// First create any new droplets
		///////////////////////////////////////////////////////////////////////
		/*for (int i = 0; i < thisTS->size(); i++)
		{
			AssayNode *n = thisTS->at(i);
			if (n->GetType() == DISPENSE && n->GetStartTS() == startingTS)
			{	// Create new droplets to be input soon
				Droplet *d = new Droplet();
				d->uniqueFluidName = n->GetPortName();
				d->volume = n->GetVolume();
				(*routes)[d] = new vector<RoutePoint *>();
				n->addDroplet(d);
			}
		}*/

		///////////////////////////////////////////////////////////////////////
		// Then, get the initial individual routes to be compacted later
		///////////////////////////////////////////////////////////////////////
		routeCycle = cycle;
		vector<vector<RoutePoint *> *> *subRoutes = new vector<vector<RoutePoint *> *>(); // subProblem routes
		vector<Droplet *> *subDrops = new vector<Droplet *>(); // corresponding subProblem droplets
		//eliminateSubRouteDependencies(routes); // Optional; ensures that no source is in the IR of a target (moves the source out of way)
		computeIndivSubProbRouteOffsets(subRoutes, subDrops, routes);
		computeIndivSubProbRoutes(subRoutes, subDrops, routes);

		///////////////////////////////////////////////////////////////////////
		// Then, compact and do maintenance on the routes
		///////////////////////////////////////////////////////////////////////
		//compactRoutes(subDrops, subRoutes); // Now, do route COMPACTION
		//addSubProbToGlobalRoutes(subDrops, subRoutes, routes); // Add sub-rotues to routes for entire simulation
		//equalizeGlobalRoutes(routes); // Now, add cycles for the droplets that were waiting first so they look like they were really waiting there

		if (performWash())
		{
			cycle = firstOverlayCycle;
			if (isInsertingAtOverlayCycle)
				cycle++;
		}

		//cycle = firstOverlayCycle;

		tsBeginningCycle->push_back(cycle); // Add cycle so we now when time-step begins
		processTimeStep(routes); // Now that routing is done, process the time-step


		///////////////////////////////////////////////////////////////////////
		// Cleanup
		///////////////////////////////////////////////////////////////////////
		while (!mixPairs.empty())
		{
			set<int> *s = mixPairs.back();
			mixPairs.pop_back();
			s->clear();
			delete s;
		}
		for (int i = nodes.size()-1; i >= 0; i--)
		{
			if (nodes.at(i)->endTimeStep <= startingTS)
				nodes.erase(nodes.begin() + i);
		}
		while (!subRoutes->empty())
		{
			vector<RoutePoint *> * v = subRoutes->back();
			subRoutes->pop_back();

			// These route-points are not needed later as they will be generating by the pin activations
			// and were just used for compaction
			while (!v->empty())
			{
				RoutePoint *rp = v->back();
				v->pop_back();
				delete rp;
			}
			delete v;
		}
		delete subRoutes;
		delete subDrops;
		thisTS->clear();
		startingTS++;
	}

	// Make sure compaction did not force any pins needed off (b/c of module extraction) to on
	map<unsigned long long, vector<int>* >::iterator mapIt = pinsForcedOffAtCycle->begin();
	for (; mapIt != pinsForcedOffAtCycle->end(); mapIt++)
	{
		unsigned long long cycle = mapIt->first;
		vector<int> *remove = mapIt->second;
		vector<int> *pins = pinActs->at(cycle);

		for (int r = 0; r < remove->size(); r++)
		{
			int remPin = remove->at(r);
			for (int p = pins->size()-1; p >= 0; p--)
			{
				int pin = pins->at(p);
				if (pin == remPin)
				{
					//cout << "\tDeleting pin " << pin << " at cycle " << cycle << endl;
					pins->erase(pins->begin()+p);
				}
			}
		}
	}

	cout << "Compaction reduced routing time from " << preCompactionRouteLength << " cycles to " << postCompactionRouteLength << " cycles." << endl;
	//cout << "DEBUG: Exiting pre-maturely." << endl;
	//exit(0);
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
void GrissomFppcParallelRouter::computeIndivSubProbRouteOffsets(vector<vector<RoutePoint *> *> *subRoutes, vector<Droplet *> *subDrops, map<Droplet *, vector<RoutePoint *> *> *routes)
{
	//bool computeOffsets = true;

	dropNum = 0;
	// DTGWD
	// Get first wash input/output
	for (unsigned i = 0; i < arch->getIoPorts()->size(); i++)
	{
		IoPort *p = arch->getIoPorts()->at(i);
		if (p->isAnInput() && p->isWashPort() && washIn == NULL)
			washIn = p;
		if (!p->isAnInput() && p->isWashPort() && washOut == NULL)
			washOut = p;

		if (washIn && washOut)
			break;
	}
	claim(washIn != NULL && washOut != NULL, "Must have a valid input and output port on the DMFB to perform washing. Please add one in the DMFB architecture input file.");
	claim((washIn->getSide() == NORTH && washOut->getSide() == SOUTH) || (washIn->getSide() == SOUTH && washOut->getSide() == NORTH),
			"Wash ports must be on North & South sides of DMFB and must be on opposite ends.");

	//claim(false, "No valid router was selected for the synthesis process or no method for 'computeIndivSupProbRoutes()' was implemented for the selected router.\n");
	ModuleDependenceGraph mdg;

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
			AssayNode *par = n->GetParents().at(p);
			mdg.AddDependency(par, n);
		}
	}

	//mdg.PrintDependencies();
	mdg.FindAllConnectedComponents();
	//mdg.PrintConnectedComponents();
	mdg.FindAndResolveDependencies();
	mdg.RevTopSortSCCs();
	//mdg.PrintConnectedComponents();

	// Now, do actual routing of droplets
	vector<vector<ModuleDependenceVertex *> *> cc = mdg.getConnectedComponents();

	for (unsigned i = 0; i < cc.size(); i++)
	{
		// Determine if there is a re-route that needs to take place
		AssayNode *rb = NULL; // Routing buffer node (created if necessary)
		AssayNode *rrNode = NULL; // Re-routed node (created if necessary)

		for (unsigned j = 0; j < cc.at(i)->size(); j++)
		{
			if (cc.at(i)->at(j)->reRouteVertex)
			{
				// Create a new routing-buffer (rb) node with rb module and reRouteVertex-node parent
				// and add to front of connected components so it's done first

				rb = new AssayNode(GENERAL); // Will call it a general op
				rb->startTimeStep = startingTS;
				rb->endTimeStep = startingTS;
				rb->status = BOUND;
				FixedModule *fm = arch->getPinMapper()->getAvailRes()->at(RB_RES)->front();
				rb->reconfigMod = new ReconfigModule(RB_RES, fm->getLX(), fm->getTY(), fm->getRX(), fm->getBY());
				rb->reconfigMod->setTileNum(fm->getTileNum());
				rb->parents.push_back(cc.at(i)->at(j)->reRouteVertex->operation);
				rrNode = rb->parents.back();
				ModuleDependenceVertex *mdv = new ModuleDependenceVertex();
				mdv->operation = rb;
				cc.at(i)->insert(cc.at(i)->begin(), mdv);

				//cout << "TS " << startingTS << " (cycle " << cycle << "): Rerouting " << rb->parents.back()->name << " to avoid routing deadlock." << endl;
				break;
			}
		}

		for (unsigned j = 0; j < cc.at(i)->size(); j++)
		{
			ModuleDependenceVertex *mdv = cc.at(i)->at(j);
			AssayNode *n = mdv->operation;
			//bool isReRouting = false;
			//if (mdv->reRouteVertex && n == mdv->reRouteVertex->operation)
			//	isReRouting = true; // Does node need to be re-routed to avoid deadlock

			if (n->startTimeStep == startingTS && n->status != ROUTED)
			{
				n->status = ROUTED;
				int tx;
				int ty;
				int x;
				int y;
				int topY = 0;
				int botY = arch->getNumCellsY()-1;
				int leftX = 0;
				int rightX = arch->getNumCellsX()-1;

				// DTGWD
				vector<IoPort *> dirtyPorts;
				vector<ReconfigModule *> dirtyModules;


				Sort::sortFppcNodesInIncreasingRouteDistance(&n->parents);
				for (unsigned p = 0; p < n->GetParents().size(); p++)
				{
					if (n->GetParents().size() >= 2)
					{
						if (p == 0)
							mixPairs.push_back(new set<int>());

						mixPairs.back()->insert(dropNum);
					}

					// If routing to a module, first job is to get the droplet to the routingColumn.....
					if (n->GetType() != OUTPUT)
					{
						AssayNode *par = n->GetParents().at(p);

						// If the parent node is the re-routed node, then we start at the routing buffer
						if (par && rrNode && par->reconfigMod && rrNode->reconfigMod &&
								par->reconfigMod->getTY() == rrNode->reconfigMod->getTY() &&
								par->reconfigMod->getLX() == rrNode->reconfigMod->getLX() &&
								n != rb)
						{
							par = rb;
						}

						// If destination is not same module as source, then route
						if (!(par->type != DISPENSE && n->boundedResType == par->boundedResType && n->reconfigMod->getTileNum() == par->reconfigMod->getTileNum()))
						{
							tx = centralRoutingColumn;
							ty = n->reconfigMod->getBY();

							if (par->GetType() == DISPENSE)
							{
								IoPort *port = par->GetIoPort();
								y = routeFromInputToCentralColumn(port, tx, ty, subRoutes);
								x = tx;
								dirtyPorts.push_back(port); // DTGWD
							}
							else // Must get the parent droplet out of its module and into the routing column
							{
								// If departing from a basic resource on the left
								if (par->boundedResType == BASIC_RES)
								{
									x = centralRoutingColumn;
									y = extractDropletFromMixModule(par->GetReconfigMod(), subRoutes);
								}
								else // Departing from a SS_RES/RB_RES on the right
								{
									x = centralRoutingColumn;
									y = extractDropletFromSSDModule(par->GetReconfigMod(), subRoutes);
								}
								dirtyModules.push_back(par->GetReconfigMod()); // DTGWD
							}

							////////////////////////////////////////////////////
							// Okay, now we are somewhere in the routing column, just go up or down till get to right row
							y = routeDropletAlongVerticalColumn(y, ty, centralRoutingColumn, subRoutes);

							////////////////////////////////////////////////////
							// Now, have the droplet enter the module to left if
							// mixer; to right if split/store module
							if (n->boundedResType == BASIC_RES)
								insertDropletIntoMixModule(n->GetReconfigMod(), subRoutes);
							else // Routing to SS_RES
							{
								insertDropletIntoSSDModule(n->GetReconfigMod(), subRoutes);
								// Now, if a SPLIT, need to route in, split, and then route to any children STORAGE nodes
								if (n->GetType() == SPLIT)
									splitNodeIntoSecondSSDModule(n, subRoutes);
							}
							dropNum++;
						}
					}
					else // If routing to OUTPUT node
					{
						////////////////////////////////////////////////////
						// Must First get the parent droplet out of its module
						// and into the routing column
						AssayNode *par = n->GetParents().at(0); // Output nodes only have one parent
						if (par->boundedResType == BASIC_RES) // If departing from a basic resource on the left
						{
							x = centralRoutingColumn;
							y = extractDropletFromMixModule(par->GetReconfigMod(), subRoutes);
						}
						else // Departing from a SS_RES on the right
						{
							x = centralRoutingColumn;
							y = extractDropletFromSSDModule(par->GetReconfigMod(), subRoutes);
						}
						dirtyModules.push_back(par->GetReconfigMod()); // DTGWD

						////////////////////////////////////////////////////
						// Now that we are in the routing column, route to output port
						IoPort *port = n->GetIoPort();
						dirtyPorts.push_back(port); // DTGWD

						////////////////////////////////////////////////////
						// First get to the top/bottom row
						if (port->getSide() == WEST || port->getSide() == EAST)
						{
							if (port->getSide() == WEST)
								tx = leftX;
							else
								tx = rightX;
							ty = port->getPosXY();

							int northFirstDist = y + ty;
							int southFirstDist = (arch->getNumCellsY() - y) + (arch->getNumCellsY() - ty);

							if (northFirstDist >= southFirstDist)
								y = routeDropletAlongVerticalColumn(y, botY, centralRoutingColumn, subRoutes);
							else
								y = routeDropletAlongVerticalColumn(y, topY, centralRoutingColumn, subRoutes);
						}
						else if (port->getSide() == NORTH || port->getSide() == SOUTH)
						{
							tx = port->getPosXY();
							if (port->getSide() == NORTH)
								ty = topY;
							else
								ty = botY;
							y = routeDropletAlongVerticalColumn(y, ty, centralRoutingColumn, subRoutes);
						}

						////////////////////////////////////////////////////
						// Now, go right or left to get to the get to the N/S port or the W/E column
						x = routeDropletAlongHorizontalRow(x, tx, y, subRoutes);

						////////////////////////////////////////////////////
						// Now, either output drop to N/S port or travel N/S to get to W/E port and output there
						if (port->getSide() == NORTH || port->getSide() == SOUTH)
							subRoutes->at(dropNum)->back()->dStatus = DROP_OUTPUT;
							//	addPinAtCycle(port->getPinNo(), cycle++, true);
						else
						{
							y = routeDropletAlongVerticalColumn(y, ty, x, subRoutes);
							//addPinAtCycle(port->getPinNo(), cycle++, true);
							subRoutes->at(dropNum)->back()->dStatus = DROP_OUTPUT;
						}
						dropNum++;
					}
				}

				/////////////////////////////////////////////////////////
				// Now do wash droplet routing - DTGWD
				//Sort::sortPortsNtoSthenPos(&dirtyPorts);
				//Sort::sortModulesFromTopToBot(&dirtyModules);

				/*cout << "At " << startingTS << " clean: " << endl;
				for (unsigned p = 0; p < dirtyPorts.size(); p++)
					cout << "\t" << dirtyPorts.at(p)->getPortName() << endl;
				for (unsigned m = 0; m < dirtyModules.size(); m++)
					cout << "\tMod(" << dirtyModules.at(m)->getRX() << ", " << dirtyModules.at(m)->getBY() << ")" << endl;
				*/

				if (performWash())
				{
					x = washIn->getPosXY();
					Direction startingSide = washIn->getSide();
					if (washIn->getSide() == NORTH)
					{
						Sort::sortModulesFromTopToBot(&dirtyModules);
						Sort::sortPortsNtoSthenPos(&dirtyPorts);
						y = 0;
					}
					else if (washIn->getSide() == SOUTH)
					{
						Sort::sortModulesFromBotToTop(&dirtyModules);
						Sort::sortPortsStoNthenPos(&dirtyPorts);
						y = arch->getNumCellsY()-1;
					}
					else
						claim(false, "Wash Droplets for original FPPC-DMFB (w/ East/West ports) not yet supported. Please use enhanced FPPC2-DMFB instead.");

					// Activate input pin
					//addPinAtCycle(washIn->getPinNo(), cycle, false);
					//addPinAtCycle(pinMapping->at(x)->at(y), cycle++, true);
					addNewRoutePoint(dropNum, x, y, subRoutes);

					// First, clean any of the electrodes on the starting horizontal bus made dirty by I/O operations
					unsigned p = 0;
					for (; p < dirtyPorts.size(); p++)
					{
						IoPort *port = dirtyPorts.at(p);
						claim(!(port->getSide() == WEST || port->getSide() == EAST), "Wash Droplets for original FPPC-DMFB (w/ East/West ports) not yet supported. Please use enhanced FPPC2-DMFB instead.");
						if (port->getSide() != startingSide)
							break;

						x = routeDropletAlongHorizontalRow(x, port->getPosXY(), y, subRoutes);
					}
					x = routeDropletAlongHorizontalRow(x, centralRoutingColumn, y, subRoutes);

					// Now traverse along vertical rotuing channel, cleaning any dirty modules along the way
					for (unsigned m = 0; m < dirtyModules.size(); m++)
					{
						ReconfigModule *dm = dirtyModules.at(m);

						y = routeDropletAlongVerticalColumn(y, dm->getBY(), x, subRoutes);

						if (dm->getResourceType() == BASIC_RES)
						{
							insertDropletIntoMixModule(dm, subRoutes);

							// Clean module by performing 1 complete cycle
							/*for (unsigned i = 0; i < mixPins->back()->size(); i++)
							{
								// Activate mix pins for each mix group
								for (unsigned j = 0; j < mixPins->size(); j++)
									addPinAtCycle(mixPins->at(j)->at(i), cycle, false);

								// Also, activate the S/S pins
								for (unsigned j = 0; j < ssHoldPins->size(); j++)
									addPinAtCycle(ssHoldPins->at(j), cycle, false);
								cycle++;
							}
							addPinAtCycle(-1, cycle++, true);// activate all holds to complete revolution*/

							// Assumes 4x2 module
							addNewRoutePoint(dropNum, dm->getRX()-2, dm->getBY(), subRoutes);
							addNewRoutePoint(dropNum, dm->getRX()-3, dm->getBY(), subRoutes);
							addNewRoutePoint(dropNum, dm->getRX()-3, dm->getTY(), subRoutes);
							addNewRoutePoint(dropNum, dm->getRX()-2, dm->getTY(), subRoutes);
							addNewRoutePoint(dropNum, dm->getRX()-1, dm->getTY(), subRoutes);
							addNewRoutePoint(dropNum, dm->getRX(), dm->getTY(), subRoutes);
							addNewRoutePoint(dropNum, dm->getRX(), dm->getBY(), subRoutes);
							//addNewRoutePoint(dropNum, dm->getRX()-1, dm->getBY(), subRoutes);

							extractDropletFromMixModule(dm, subRoutes);
						}
						else if (dm->getResourceType() == SSD_RES)
						{
							insertDropletIntoSSDModule(dm, subRoutes);
							extractDropletFromSSDModule(dm, subRoutes);
						}
						else
							claim(false, "Cannot clean unsupported module type.");
					}

					// Route to opposite horizontal channel
					if (startingSide == NORTH)
						y = routeDropletAlongVerticalColumn(y, arch->getNumCellsY()-1, x, subRoutes);
					else
						y = routeDropletAlongVerticalColumn(y, 0, x, subRoutes);

					// Finally, clean any of the electrodes on the opposite horizontal bus made dirty by I/O operations
					for (; p < dirtyPorts.size(); p++)
					{
						IoPort *port = dirtyPorts.at(p);
						claim(!(port->getSide() == WEST || port->getSide() == EAST), "Wash Droplets for original FPPC-DMFB (w/ East/West ports) not yet supported. Please use enhanced FPPC2-DMFB instead.");
						x = routeDropletAlongHorizontalRow(x, port->getPosXY(), y, subRoutes);
					}
					x = routeDropletAlongHorizontalRow(x, washOut->getPosXY(), y, subRoutes);
					//addPinAtCycle(washOut->getPinNo(), cycle++, true); // Output droplet
					subRoutes->at(dropNum)->back()->dStatus = DROP_OUTPUT;

					// Set status of all routing points except last
					for (unsigned i = 0; i < subRoutes->at(dropNum)->size(); i++)
						if (subRoutes->at(dropNum)->at(i)->dStatus != DROP_OUTPUT)
							subRoutes->at(dropNum)->at(i)->dStatus = DROP_WASH;

					dropNum++;
				}
			}
		}
	}

	// Remove empty routes so we don't get compaction errors
	/*for (int i = subRoutes->size()-1; i >= 0; i--) // Needs to be an int (warning) for comparison to work
	{
		vector<RoutePoint *> *r = subRoutes->at(i);
		if (r->empty())
		{
			subRoutes->erase(subRoutes->begin() + i);
			delete r;
		}
	}*/


	// Debug print
	unsigned length = 0;
	/*cout << "TS " << startingTS << ": " << endl;

	for (int i = 0; i < mixPairs.size(); i++)
	{
		cout << "Mix Drops: ";
		set<int>::iterator mixIt = mixPairs.at(i)->begin();
		for (; mixIt != mixPairs.at(i)->end(); mixIt++)
		{
			cout << *mixIt << ", ";
		}
		cout << endl;
	}*/

	for (unsigned i = 0; i < subRoutes->size(); i++)
	{
		vector<RoutePoint *> *route = subRoutes->at(i);
		/*cout << "Route " << i << ": ";
		for (unsigned j = 0; j < route->size(); j++)
		{
			RoutePoint *rp = route->at(j);
			cout << "(" << rp->x << ", " << rp->y;
			if (rp->dStatus == DROP_WAIT)
				cout << ", W)->";
			else if (rp->dStatus == DROP_SPLITTING)
				cout << ", S)->";
			else if (rp->dStatus == DROP_OUTPUT)
				cout << ", O)->";
			else if (rp->dStatus == DROP_WASH)
				cout << ", C)->";
			else
				cout << ")->";
		}
		cout << endl;*/
		length += route->size();
	}
	//cout << "Sequential Length: " << length << endl << endl;
	preCompactionRouteLength += length;

	compactFppcRoutesWithBegStalls(subRoutes);

	// Debug print
	length = 0;
	//cout << "TS " << startingTS << ": " << endl;
	for (unsigned i = 0; i < subRoutes->size(); i++)
	{
		vector<RoutePoint *> *route = subRoutes->at(i);
		/*cout << "Route " << i << ": ";
		for (unsigned j = 0; j < route->size(); j++)
		{
			RoutePoint *rp = route->at(j);
			if (!rp)
				cout << "(----)->";
			else
			{
				cout << "(" << rp->x << ", " << rp->y;
				if (rp->dStatus == DROP_WAIT)
					cout << ", W)->";
				else if (rp->dStatus == DROP_SPLITTING)
					cout << ", S)->";
				else if (rp->dStatus == DROP_OUTPUT)
					cout << ", O)->";
				else if (rp->dStatus == DROP_WASH)
					cout << ", C)->";
				else
					cout << ")->";
			}
		}
		cout << endl;*/
		if (route->size() > length)
			length = route->size();
	}
	//cout << "Parallel Length: " << length << endl << endl;
	postCompactionRouteLength += length;
	//cout << "-----------------------------------------------------------------" << endl;

	// Add offsets
	routeOffsets->clear();
	vector<unsigned> hasInputStart;
	bool hasModStart = false;
	for (unsigned i = 0; i < subRoutes->size(); i++)
	{
		int offset = 0;
		vector<RoutePoint *> *route = subRoutes->at(i);
		//cout << "Route " << i << " off: ";
		for (unsigned j = 0; j < route->size(); j++)
		{
			if (!route->at(j))
				offset++;
			else
			{
				if (route->at(j)->y == 0 || route->at(j)->y == arch->getNumCellsY()-1)
					hasInputStart.push_back(i);
				else
					hasModStart = true;
				break;
			}
		}
		routeOffsets->push_back(offset);
		//cout << routeOffsets->at(i) << endl;
	}
	//cout << endl;

	if (!hasInputStart.empty() && hasModStart)
	{
		//cout << "Updated offsets: " << endl;
		for (unsigned i = 0; i < hasInputStart.size(); i++)
		{
			routeOffsets->at(hasInputStart.at(i)) = routeOffsets->at(hasInputStart.at(i)) - 1;
			//cout << "Route " << hasInputStart.at(i) << " off: ";
			//cout << routeOffsets->at(hasInputStart.at(i)) << endl;
		}
	}

}

///////////////////////////////////////////////////////////////////////////////
// Given a vector full of sequential routes, compacts them such that each
// equal index of any subRoute can be executed at the same time.  Adds any
// necessary stalls at the beginning of the route.
//
// Compacts the routes by combining the routes from longest to shortest.
// If route being placed interferes with a route that is already placed, the
// route being placed will be delayed by 1 cycle until it can be placed.
///////////////////////////////////////////////////////////////////////////////
void GrissomFppcParallelRouter::compactFppcRoutesWithBegStalls(vector<vector<RoutePoint *> *> *subRoutes)
{
	//if (startingTS == 33)
	//	printRoutes(subRoutes);

	// Clear arrays before compaction
	for (int x = 0; x < arch->getNumCellsX(); x++)
	{
		for (int y = 0; y < arch->getNumCellsY(); y++)
		{
			dirtyArray->at(x)->at(y)->clear();
			dropletOrderArray->at(x)->at(y)->clear();
		}
	}

	// Compute preferred ordering based on compaction ordering
	/*for (unsigned i = 0; i < subRoutes->size(); i++)
	{
		vector<RoutePoint *> *subRoute = subRoutes->at(i);
		for (unsigned j = 0; j < subRoute->size(); j++)
		{
			RoutePoint *rp = subRoute->at(j);
			dropletOrderArray->at(rp->x)->at(rp->y)->push_back(i);
		}
	}*/

	firstOverlayCycle = 0; // First cycle that can be overlaid with operation processing
	unsigned subProblemLength = 0;
	int longestRoute = 0;
	int sequentialLength = 0;
	for (unsigned i = 0; i < subRoutes->size(); i++)
	{
		sequentialLength += subRoutes->at(i)->size();
		if (subRoutes->at(i)->size() > longestRoute)
			longestRoute = subRoutes->at(i)->size();
	}
	//if (subRoutes->size() > 0)
	//	longestRoute = subRoutes->at(0)->size();

	vector<bool> mixCleanPerCycle; // Vector of cycles telling if mixers are being cleaned at that cycle
	for (int i = 0; i < sequentialLength; i++)
		mixCleanPerCycle.push_back(false);

	int numStallsToPrepend = 1;
	for (unsigned i = 0; i < subRoutes->size(); i++)
	{
		bool isWashDrop = false;

		vector<RoutePoint *> *subRoute = subRoutes->at(i);
		RoutePoint *destPt = NULL;
		if (subRoute->size() > 0)
			destPt = subRoute->back();

		// Check entire route
		bool isInterference = false;
		int j = 0; // The index used to traverse a specific route/cycle

		//printRoutes(subRoutes);

		int numStallsInserted = 0;

		// If not outputting, check the max length b/c droplet will remain at destination.
		int cyclesToCheck = max((int)subRoute->size(), longestRoute);
		unsigned reachedDestinationAtIteration = 0;

		while (j != cyclesToCheck && subRoute->size() > 0)
		{
			int debugSubRouteNum = -1;
			RoutePoint *debugSubRouteRp = NULL;
			//printRoutes(subRoutes);

			int routeIndex;
			RoutePoint *rp = NULL;
			if (j <= subRoute->size()-1)
			{
				rp = subRoute->at(j);
				routeIndex = j;
			}
			else
			{
				rp = subRoute->back();
				routeIndex = subRoute->size()-1;
			}

			if (rp)
			{
				if (rp->dStatus == DROP_WASH)
					isWashDrop = true;

				// Check against the previous routes that have been compacted
				for (int k = 0; k < i; k++)
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
					else if (pastRoute->back()->dStatus != DROP_OUTPUT)
						prp = pastRoute->back();

					RoutePoint *prpLc = NULL; // Past route's last cycle
					if (j > 0 && j <= pastRoute->size()-1)
						prpLc = pastRoute->at(j-1);
					else if (pastRoute->back()->dStatus != DROP_OUTPUT)
						prpLc = pastRoute->back();
					else if (j <= pastRoute->size()+1)
						prpLc = pastRoute->back();

					RoutePoint *prpNc = NULL; // Past route's next cycle
					if (j+1 <= pastRoute->size()-1)
						prpNc = pastRoute->at(j+1);
					else if (pastRoute->back()->dStatus != DROP_OUTPUT)
						prpNc = pastRoute->back();

					// Check dynamic droplet rules so this and last droplet locations don't interfere
					if (prp && doesInterfere(rp, prp) && !(doesInterfere(rp, destPt) && prp->dStatus == DROP_WAIT))
						isInterference = true;
					if (prpLc && doesInterfere(rp, prpLc) && !(doesInterfere(rp, destPt) && prpLc->dStatus == DROP_WAIT)) // DTG, is this if-statement necessary??
						isInterference = true;

					if (prp && rpNc && doesInterfere(rpNc, prp) && !(doesInterfere(rpNc, destPt) && prp->dStatus == DROP_WAIT))
						isInterference = true;
					if (prpNc && doesInterfere(rp, prpNc) && !(doesInterfere(rp, destPt) && prpNc->dStatus == DROP_WAIT))
						isInterference = true;
					if (prp && rpLc && doesInterfere(rpLc, prp) && !(doesInterfere(rpLc, destPt) && prp->dStatus == DROP_WAIT))
						isInterference = true;
					if (prpLc && rpLc && doesInterfere(rpLc, prpLc) && !(doesInterfere(rpLc, destPt) && prpLc->dStatus == DROP_WAIT))
						isInterference = true;

					// If this is a wash droplet cleaning a mixing module, make sure no other mix module is
					// currently being cleaned or undesired splits/drifts will occur
					// Examine route of wash droplets to determine if they are cleaning mixers
					if (isWashDrop && !isInterference && performWash())
					{
						if (rp && rp->y > 0 && rp->y < arch->getNumCellsY()-1 && rp->x < centralRoutingColumn && mixCleanPerCycle.at(routeIndex) == true)
							isInterference = true;
					}
					else if (!isWashDrop && !isInterference)
					{
						// Droplets should not be insert/extracted to/from mix module if mixers are active with other I/O or cleaning
						if (rp && rp->y > 0 && rp->y < arch->getNumCellsY()-1 && rp->x < centralRoutingColumn && rp->x >= centralRoutingColumn-3)
							if (mixCleanPerCycle.at(routeIndex) == true)
								isInterference = true;
							else if (routeIndex > 0 && mixCleanPerCycle.at(routeIndex-1))
								isInterference = true;
					}


					// Now, check to make sure that a droplet is not hitting a cell that is dirty
					// b/c of another droplet (that it's not about to mix with); cleaning droplets
					// must get there first
					if (!isWashDrop && !isInterference && performWash())
					{
						vector<pair<int, int> > * dirty = dirtyArray->at(rp->x)->at(rp->y);

						// If there are dirty cells, need to mark as clean;
						if (!dirty->empty())
						{
							for (int d = 0; d < dirty->size(); d+=2)
							{
								int dropNumber = dirty->at(d).first;
								int d1 = dirty->at(d).second;
								int d2 = dirty->at(d+1).second;

								// If this cell is dirty
								if (j >= d1 && j <= d2)
								{
									if (i == dropNumber)
										break;
									else
									{
										bool compatibleMix = false;
										for (int m = 0; m < mixPairs.size(); m++)
										{
											set<int> *mixPair = mixPairs.at(m);
											if (mixPair->count(i) > 0 && mixPair->count(dropNumber) > 0)
											{
												compatibleMix = true;
												break;
											}
										}
										if (!compatibleMix)
										{
											isInterference = true;
											break;
										}
									}
								}
							}
						}
					}

					// Check that order is being maintained, which will prevent contamination
					if (performWash())
					{
						vector<pair<int, int> > *cellDropletOrder = dropletOrderArray->at(rp->x)->at(rp->y);
						for (int o = 0; o < cellDropletOrder->size(); o++)
						{
							pair<int, int> dropPos = cellDropletOrder->at(o);

							if (dropPos.first == i)
								break; // No interference if we reach the matching droplet record
							else if (dropPos.second < j)
								break; // If prev. compacted droplet passes at an earlier time, then no interference
							else
							{	// Else, is interference if not about to mix with droplet
								for (int m = 0; m < mixPairs.size(); m++)
								{
									set<int> *mixPair = mixPairs.at(m);
									if (!(mixPair->count(i) > 0 && mixPair->count(dropPos.first) > 0))
										isInterference = true;
								}
							}
						}
					}

					// Get debug info if found interference for possible display to user
					if (isInterference)
					{
						// DEBUG
						//if (startingTS == 89 && i == 3 /*#18 local*/&& k == 1 /*#22 other*/)
						//{
						//	cout << "Droplet\t" << "Last\t\t" << "Curr\t" << "Next\t" << "(TS " << startingTS << ", position " << j << ")" << endl;
						//	cout << "Other " << k << "\t" << Analyze::GetFormattedCell(prpLc) << "\t\t" << Analyze::GetFormattedCell(prp) << "\t" << Analyze::GetFormattedCell(prpNc) << endl;
						//	cout << "Local " << i << "\t" << Analyze::GetFormattedCell(rpLc) << "\t\t" << Analyze::GetFormattedCell(rp) << "\t" << Analyze::GetFormattedCell(rpNc) << endl << endl;
						//	int foo = 4 + 5;
						//}

						debugSubRouteNum = k;
						debugSubRouteRp = prp;
						break;
					}
				}
			}
			if (isInterference)
			{	// Add a few stalls at the beginning and try again
				for (int m = 0; m < numStallsToPrepend; m++)
					subRoute->insert(subRoute->begin(), NULL);
				isInterference = false;
				numStallsInserted += numStallsToPrepend;
				j = 0;

				if (subRoute->at(subRoute->size()-1)->dStatus != DROP_OUTPUT)
					cyclesToCheck++;
				reachedDestinationAtIteration = 0;
			}
			else
			{
				// If current route point has reached it's destination, then only check the next cycle or two
				// b/c it will be off the DMFB (in the case of an output) or at its destination and should no
				// longer cause interference
				if (rp && rp == destPt && reachedDestinationAtIteration == 0)
					reachedDestinationAtIteration = j;
				else if (rp && rp == destPt && reachedDestinationAtIteration + 2 == j)
					break;

				j++; // Increment local cycle
			}

			// Sanity check; ensure we're not in an infinite loop
			int stallInsertionLimit = 1000;
			if (numStallsInserted >= stallInsertionLimit)
			{
				stringstream msg;
				msg << "The stall insertion limit of " << stallInsertionLimit << " has been reached during route compaction." << endl;
				msg << "This likely indicates a problem with the individual route generation, such as blockages not being computed properly." << endl;
				msg << "The problem manifests with SubRoute #" << i << " of " << subRoutes->size();
				msg << ", specifically when the droplet at location (" << rp->x << ", " << rp->y << ")";
				msg << " interferes with the droplet in SubRoute #" << debugSubRouteNum << " at location (";
				if (debugSubRouteRp)
					msg << debugSubRouteRp->x << ", " << debugSubRouteRp->y << ")." << endl;
				else
					msg << "NULL)." << endl;
				msg << "A sub-route dump has been generated just above this.  It is recommended that you step through the compaction code to see where the uncompactable droplet interference is taking place to help debug your router." << endl;

				printRoutes(subRoutes);
				claim(false, &msg);

			}
		}
		if (j > longestRoute)
			longestRoute = j;


		// Remove droplets from ordering
		/*int nextDropOnThisCell = dropletOrderArray->at(rp->x)->at(rp->y)->front();
		if (nextDropOnThisCell != i)
		{
			for (int m = 0; m < mixPairs.size(); m++)
			{
				set<int> *mixPair = mixPairs.at(m);
				if (!(mixPair->count(i) > 0 && mixPair->count(nextDropOnThisCell) > 0))
					isInterference = true;
			}
		}*/

		// Add compacted route to cell/droplet ordering
		for (unsigned k = 0; k < subRoute->size(); k++)
		{
			RoutePoint *rp = subRoute->at(k);
			if (rp)
				dropletOrderArray->at(rp->x)->at(rp->y)->push_back(make_pair(i, k));
		}


		// Determine how much of routes can be overlayed with operations
		if (subRoute->size() > subProblemLength)
			subProblemLength = subRoute->size();
		int firstCycleWithNoMoreMix = 0;
		for (int k = subRoute->size()-1; k >= 0; k--)
		{
			RoutePoint *rp = subRoute->at(k);
			firstCycleWithNoMoreMix = k;
			// If not in horizontal column
			if (rp && rp->y > 0 && rp->y < arch->getNumCellsY()-1)
			{
				// If in mix module or mix I/O
				if (rp->x < centralRoutingColumn)
					break;
			}
		}
		if (firstCycleWithNoMoreMix > firstOverlayCycle)
			firstOverlayCycle = firstCycleWithNoMoreMix;

		// If wash droplet, update dirty/clean cells and if in mixer
		if (subRoute->at(subRoute->size()-2)->dStatus == DROP_WASH)
		{

			for (unsigned k = 0; k < subRoute->size(); k++)
			{
				RoutePoint *rp = subRoute->at(k);

				// Examine route of wash droplets to determine if they are cleaning mixers
				if (rp && rp->y > 0 && rp->y < arch->getNumCellsY()-1 && rp->x < centralRoutingColumn)
					mixCleanPerCycle.at(k) = true;

				if (rp)
				{
					// Mark as clean
					vector<pair<int, int> > * dirty = dirtyArray->at(rp->x)->at(rp->y);

					// If there are dirty cells, need to mark as clean;
					if (!dirty->empty())
					{
						for (int d = 0; d < dirty->size(); d+=2)
						{
							int dropNumber = dirty->at(d).first;

							int d1 = dirty->at(d).second;
							int d2 = dirty->at(d+1).second;

							if (dropNumber != dirty->at(d+1).first)
							{
								cout << "Cell (" << rp->x << ", " << rp->y << "): ";
								for (int i = 0; i < dirty->size(); i+=2)
								{
									cout << "D" << dropNumber << "/" << dirty->at(d+1).first << "(";
									cout << d1 << ", ";
									cout << d2 << "), ";
								}
								cout << endl;

								claim(dropNumber == dirty->at(d+1).first, "Droplet numbers in clean array must match.");
							}

							if (k >= d1 && k <= d2)
							{
								// If occurs after last dirty record
								dirty->at(d+1) = make_pair(dropNumber, k-1);
							}
						}
					}
				}
			}
		}
		else
		{
			// Mark as dirty
			for (unsigned k = 0; k < subRoute->size(); k++)
			{
				//if (j == 8)
				//	cout << j << endl;

				RoutePoint *rp = subRoute->at(k);

				// Examine route of normal droplets to determine if they are entering/exiting mix modules
				if (rp && rp->y > 0 && rp->y < arch->getNumCellsY()-1 && rp->x < centralRoutingColumn && rp->x > centralRoutingColumn-3)
					mixCleanPerCycle.at(k) = true;

				if (rp)
				{
					vector<pair<int, int> > * dirty = dirtyArray->at(rp->x)->at(rp->y);

					if (dirty->empty())
					{
						// If never marked dirty, mark dirty from now till end
						dirty->push_back(make_pair(i, k));
						dirty->push_back(make_pair(i, sequentialLength));
					}
					else
					{
						for (int d = 0; d < dirty->size(); d+=2)
						{
							int dropNumber = dirty->at(d).first;
							if (dropNumber != dirty->at(d+1).first)
								claim(dropNumber == dirty->at(d+1).first, "Droplet numbers in clean array must match.");
							int d1 = dirty->at(d).second;
							int d2 = dirty->at(d+1).second;

							if (k < d1)
							{
								dirty->at(d) = make_pair(i, k);
								dirty->at(d+1) = make_pair(i, d2);
								break;
							}
							else if (k > d2 && d == dirty->size()-2)
							{
								// If occurs after last dirty record
								dirty->push_back(make_pair(i, k));
								dirty->push_back(make_pair(i, sequentialLength));
								break;
							}
						}
					}
				}
			}
		}
	}

	routingCyclesToOverlay = subProblemLength - firstOverlayCycle;
	//cout << "SubProbLength (" << subProblemLength << ") - tsBegin(" << firstOverlayCycle << ") = #overlayCycles (" <<routingCyclesToOverlay << ")" << endl;

	// DEBUG Dirty Array Printout
	/*for (int y = 0; y < arch->getNumCellsY(); y++)
	{
		for (int x = 0; x < arch->getNumCellsX(); x++)
		{
			if (!dirtyArray->at(x)->at(y)->empty())
			{
				cout << "Cell (" << x << ", " << y << "): ";
				for (int i = 0; i < dirtyArray->at(x)->at(y)->size(); i+=2)
				{
					cout << "D" << dirtyArray->at(x)->at(y)->at(i).first << "(";
					cout << dirtyArray->at(x)->at(y)->at(i).second << ", ";
					cout << dirtyArray->at(x)->at(y)->at(i+1).second << "), ";
				}
				cout << endl;
			}
		}
	}*/

	//printRoutes(subRoutes);

	// Power/energy computation code
	/*double electrodePitch = 2.54; //mm
	//double voltage = 13; //V
	double voltage = 35; //V (also tried 50V)

	double velocity = 0.005 * voltage*voltage + (0.0358*voltage) - 0.9103; // mm/s
	double cycleTime = electrodePitch / velocity;

	int numDroplets = subRoutes->size();

	int maxRouteCycles = 0;
	for (int i = 0; i < subRoutes->size(); i++)
		if (subRoutes->at(i)->size() > maxRouteCycles)
			maxRouteCycles = subRoutes->at(i)->size();

	double time = (double)((double)maxRouteCycles * cycleTime);
	double cumElectrodeActivationTime = (double)(time * (double)numDroplets);
	double energy = (voltage*voltage) / 1000000000 * time;

	if (startingTS == 0)
		cout << "Pitch: " << electrodePitch << "mm, Volt: " << voltage << "V, Vel: " << velocity << "mm/s, CycleTime: " << cycleTime << "s" << endl;

	if (maxRouteCycles > 0)
	{
		cout << "SUB-PROBLEM " << this->startingTS << " -- ";
		cout << "Time: " << (double)((double)maxRouteCycles * cycleTime) << "s, Energy: " << energy << "J" << endl;
	}*/

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
void GrissomFppcParallelRouter::computeIndivSubProbRoutes(vector<vector<RoutePoint *> *> *subRoutes, vector<Droplet *> *subDrops, map<Droplet *, vector<RoutePoint *> *> *routes)
{
	isInsertingAtOverlayCycle = false;
	dropNum = 0;
	int startCycle = cycle;
	int maxCycle = cycle;
	if (performWash())
		firstOverlayCycle = startCycle + firstOverlayCycle;
	else
		firstOverlayCycle = startCycle + preCompactionRouteLength;

	//cout << "First Overlay cycle: " << firstOverlayCycle << endl;

	// DTGWD
	// Get first wash input/output
	for (unsigned i = 0; i < arch->getIoPorts()->size(); i++)
	{
		IoPort *p = arch->getIoPorts()->at(i);
		if (p->isAnInput() && p->isWashPort() && washIn == NULL)
			washIn = p;
		if (!p->isAnInput() && p->isWashPort() && washOut == NULL)
			washOut = p;

		if (washIn && washOut)
			break;
	}
	claim(washIn != NULL && washOut != NULL, "Must have a valid input and output port on the DMFB to perform washing. Please add one in the DMFB architecture input file.");
	claim((washIn->getSide() == NORTH && washOut->getSide() == SOUTH) || (washIn->getSide() == SOUTH && washOut->getSide() == NORTH),
			"Wash ports must be on North & South sides of DMFB and must be on opposite ends.");

	//claim(false, "No valid router was selected for the synthesis process or no method for 'computeIndivSupProbRoutes()' was implemented for the selected router.\n");
	ModuleDependenceGraph mdg;

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
			AssayNode *par = n->GetParents().at(p);
			mdg.AddDependency(par, n);
		}
	}

	//mdg.PrintDependencies();
	mdg.FindAllConnectedComponents();
	//mdg.PrintConnectedComponents();
	mdg.FindAndResolveDependencies();
	mdg.RevTopSortSCCs();
	//mdg.PrintConnectedComponents();

	vector<vector<ModuleDependenceVertex *> *> cc = mdg.getConnectedComponents();

	// Re-init all nodes to "bound" since they were set in computeIndivSubProbRouteOffsets()
	for (unsigned i = 0; i < cc.size(); i++)
		for (unsigned j = 0; j < cc.at(i)->size(); j++)
			cc.at(i)->at(j)->operation->status = BOUND;

	// Now, do actual routing of droplets
	for (unsigned i = 0; i < cc.size(); i++)
	{
		// Determine if there is a re-route that needs to take place
		AssayNode *rb = NULL; // Routing buffer node (created if necessary)
		AssayNode *rrNode = NULL; // Re-routed node (created if necessary)

		for (unsigned j = 0; j < cc.at(i)->size(); j++)
		{
			if (cc.at(i)->at(j)->reRouteVertex)
			{
				// Create a new routing-buffer (rb) node with rb module and reRouteVertex-node parent
				// and add to front of connected components so it's done first

				rb = new AssayNode(GENERAL); // Will call it a general op
				rb->startTimeStep = startingTS;
				rb->endTimeStep = startingTS;
				rb->status = BOUND;
				FixedModule *fm = arch->getPinMapper()->getAvailRes()->at(RB_RES)->front();
				rb->reconfigMod = new ReconfigModule(RB_RES, fm->getLX(), fm->getTY(), fm->getRX(), fm->getBY());
				rb->reconfigMod->setTileNum(fm->getTileNum());
				rb->parents.push_back(cc.at(i)->at(j)->reRouteVertex->operation);
				rrNode = rb->parents.back();
				ModuleDependenceVertex *mdv = new ModuleDependenceVertex();
				mdv->operation = rb;
				cc.at(i)->insert(cc.at(i)->begin(), mdv);

				//cout << "TS " << startingTS << " (cycle " << cycle << "): Rerouting " << rb->parents.back()->name << " to avoid routing deadlock." << endl;
				break;
			}
		}

		for (unsigned j = 0; j < cc.at(i)->size(); j++)
		{
			ModuleDependenceVertex *mdv = cc.at(i)->at(j);
			AssayNode *n = mdv->operation;
			//bool isReRouting = false;
			//if (mdv->reRouteVertex && n == mdv->reRouteVertex->operation)
			//	isReRouting = true; // Does node need to be re-routed to avoid deadlock

			if (n->startTimeStep == startingTS && n->status != ROUTED)
			{
				n->status = ROUTED;
				int tx;
				int ty;
				int x;
				int y;
				int topY = 0;
				int botY = arch->getNumCellsY()-1;
				int leftX = 0;
				int rightX = arch->getNumCellsX()-1;

				// DTGWD
				vector<IoPort *> dirtyPorts;
				vector<ReconfigModule *> dirtyModules;

				for (unsigned p = 0; p < n->GetParents().size(); p++)
				{
					cycle = startCycle + routeOffsets->at(dropNum); // Add offset from compaction step

					// If routing to a module, first job is to get the droplet to the routingColumn.....
					if (n->GetType() != OUTPUT)
					{
						AssayNode *par = n->GetParents().at(p);

						// If the parent node is the re-routed node, then we start at the routing buffer
						if (par && rrNode && par->reconfigMod && rrNode->reconfigMod &&
								par->reconfigMod->getTY() == rrNode->reconfigMod->getTY() &&
								par->reconfigMod->getLX() == rrNode->reconfigMod->getLX() &&
								n != rb)
						{
							par = rb;
						}

						// If destination is not same module as source, then route
						if (!(par->type != DISPENSE && n->boundedResType == par->boundedResType && n->reconfigMod->getTileNum() == par->reconfigMod->getTileNum()))
						{
							tx = centralRoutingColumn;
							ty = n->reconfigMod->getBY();

							if (par->GetType() == DISPENSE)
							{
								IoPort *port = par->GetIoPort();
								y = routeFromInputToCentralColumn(port, tx, ty);
								x = tx;
								dirtyPorts.push_back(port); // DTGWD
							}
							else // Must get the parent droplet out of its module and into the routing column
							{
								// If departing from a basic resource on the left
								if (par->boundedResType == BASIC_RES)
								{
									x = centralRoutingColumn;
									y = extractDropletFromMixModule(par->GetReconfigMod());
								}
								else // Departing from a SS_RES/RB_RES on the right
								{
									x = centralRoutingColumn;
									y = extractDropletFromSSDModule(par->GetReconfigMod());
								}
								dirtyModules.push_back(par->GetReconfigMod()); // DTGWD
							}

							////////////////////////////////////////////////////
							// Okay, now we are somewhere in the routing column, just go up or down till get to right row
							y = routeDropletAlongVerticalColumn(y, ty, centralRoutingColumn);

							////////////////////////////////////////////////////
							// Now, have the droplet enter the module to left if
							// mixer; to right if split/store module
							if (n->boundedResType == BASIC_RES)
								insertDropletIntoMixModule(n->GetReconfigMod());
							else // Routing to SS_RES
							{
								insertDropletIntoSSDModule(n->GetReconfigMod());
								// Now, if a SPLIT, need to route in, split, and then route to any children STORAGE nodes
								if (n->GetType() == SPLIT)
									splitNodeIntoSecondSSDModule(n);
							}
							if (cycle > maxCycle)
								maxCycle = cycle;
							dropNum++;
						}
					}
					else // If routing to OUTPUT node
					{
						////////////////////////////////////////////////////
						// Must First get the parent droplet out of its module
						// and into the routing column
						AssayNode *par = n->GetParents().at(0); // Output nodes only have one parent
						if (par->boundedResType == BASIC_RES) // If departing from a basic resource on the left
						{
							x = centralRoutingColumn;
							y = extractDropletFromMixModule(par->GetReconfigMod());
						}
						else // Departing from a SS_RES on the right
						{
							x = centralRoutingColumn;
							y = extractDropletFromSSDModule(par->GetReconfigMod());
						}
						dirtyModules.push_back(par->GetReconfigMod()); // DTGWD

						////////////////////////////////////////////////////
						// Now that we are in the routing column, route to output port
						IoPort *port = n->GetIoPort();
						dirtyPorts.push_back(port); // DTGWD

						////////////////////////////////////////////////////
						// First get to the top/bottom row
						if (port->getSide() == WEST || port->getSide() == EAST)
						{
							if (port->getSide() == WEST)
								tx = leftX;
							else
								tx = rightX;
							ty = port->getPosXY();

							int northFirstDist = y + ty;
							int southFirstDist = (arch->getNumCellsY() - y) + (arch->getNumCellsY() - ty);

							if (northFirstDist >= southFirstDist)
								y = routeDropletAlongVerticalColumn(y, botY, centralRoutingColumn);
							else
								y = routeDropletAlongVerticalColumn(y, topY, centralRoutingColumn);
						}
						else if (port->getSide() == NORTH || port->getSide() == SOUTH)
						{
							tx = port->getPosXY();
							if (port->getSide() == NORTH)
								ty = topY;
							else
								ty = botY;
							y = routeDropletAlongVerticalColumn(y, ty, centralRoutingColumn);
						}

						////////////////////////////////////////////////////
						// Now, go right or left to get to the get to the N/S port or the W/E column
						x = routeDropletAlongHorizontalRow(x, tx, y);

						////////////////////////////////////////////////////
						// Now, either output drop to N/S port or travel N/S to get to W/E port and output there
						if (port->getSide() == NORTH || port->getSide() == SOUTH)
						{
							//addPinAtCycle(port->getPinNo(), cycle++, true);
							addPinAtCycle(port->getPinNo(), cycle, cycle < firstOverlayCycle);
							cycle++;
						}
						else
						{
							y = routeDropletAlongVerticalColumn(y, ty, x);
							//addPinAtCycle(port->getPinNo(), cycle++, true);
							addPinAtCycle(port->getPinNo(), cycle, cycle < firstOverlayCycle);
							cycle++;
						}
						if (cycle > maxCycle)
							maxCycle = cycle;
						dropNum++;
					}
				}

				/////////////////////////////////////////////////////////
				// Now do wash droplet routing - DTGWD
				//Sort::sortPortsNtoSthenPos(&dirtyPorts);
				//Sort::sortModulesFromTopToBot(&dirtyModules);

				/*cout << "At " << startingTS << " clean: " << endl;
				for (unsigned p = 0; p < dirtyPorts.size(); p++)
					cout << "\t" << dirtyPorts.at(p)->getPortName() << endl;
				for (unsigned m = 0; m < dirtyModules.size(); m++)
					cout << "\tMod(" << dirtyModules.at(m)->getRX() << ", " << dirtyModules.at(m)->getBY() << ")" << endl;
				*/

				if (performWash())
				{
					cycle = startCycle + routeOffsets->at(dropNum);

					x = washIn->getPosXY();
					Direction startingSide = washIn->getSide();
					if (washIn->getSide() == NORTH)
					{
						Sort::sortModulesFromTopToBot(&dirtyModules);
						Sort::sortPortsNtoSthenPos(&dirtyPorts);
						y = 0;
					}
					else if (washIn->getSide() == SOUTH)
					{
						Sort::sortModulesFromBotToTop(&dirtyModules);
						Sort::sortPortsStoNthenPos(&dirtyPorts);
						y = arch->getNumCellsY()-1;
					}
					else
						claim(false, "Wash Droplets for original FPPC-DMFB (w/ East/West ports) not yet supported. Please use enhanced FPPC2-DMFB instead.");

					// Activate input pin
					addPinAtCycle(washIn->getPinNo(), cycle, false);
					//addPinAtCycle(pinMapping->at(x)->at(y), cycle++, true);
					addPinAtCycle(pinMapping->at(x)->at(y), cycle, cycle < firstOverlayCycle);
					cycle++;

					// First, clean any of the electrodes on the starting horizontal bus made dirty by I/O operations
					unsigned p = 0;
					for (; p < dirtyPorts.size(); p++)
					{
						IoPort *port = dirtyPorts.at(p);
						claim(!(port->getSide() == WEST || port->getSide() == EAST), "Wash Droplets for original FPPC-DMFB (w/ East/West ports) not yet supported. Please use enhanced FPPC2-DMFB instead.");
						if (port->getSide() != startingSide)
							break;

						x = routeDropletAlongHorizontalRow(x, port->getPosXY(), y);
					}
					x = routeDropletAlongHorizontalRow(x, centralRoutingColumn, y);

					// Now traverse along vertical rotuing channel, cleaning any dirty modules along the way
					for (unsigned m = 0; m < dirtyModules.size(); m++)
					{
						ReconfigModule *dm = dirtyModules.at(m);

						y = routeDropletAlongVerticalColumn(y, dm->getBY(), x);

						if (dm->getResourceType() == BASIC_RES)
						{
							insertDropletIntoMixModule(dm);

							// Clean module by performing 1 complete cycle
							for (unsigned i = 0; i < mixPins->back()->size(); i++)
							{
								// Activate mix pins for each mix group
								for (unsigned j = 0; j < mixPins->size(); j++)
									addPinAtCycle(mixPins->at(j)->at(i), cycle, false);

								for (unsigned j = 0; j < mHoldPins->size(); j++)
									forcePinOffAtCycle(mHoldPins->at(j), cycle); // Make sure pin is forced off these cycles

								// Also, activate the S/S pins
								for (unsigned j = 0; j < ssHoldPins->size(); j++)
									addPinAtCycle(ssHoldPins->at(j), cycle, false);
								cycle++;
							}
							addPinAtCycle(-1, cycle++, true);// activate all holds to complete revolution

							extractDropletFromMixModule(dm);
						}
						else if (dm->getResourceType() == SSD_RES)
						{
							insertDropletIntoSSDModule(dm);
							extractDropletFromSSDModule(dm);
						}
						else
							claim(false, "Cannot clean unsupported module type.");
					}

					// Route to opposite horizontal channel
					if (startingSide == NORTH)
						y = routeDropletAlongVerticalColumn(y, arch->getNumCellsY()-1, x);
					else
						y = routeDropletAlongVerticalColumn(y, 0, x);

					// Finally, clean any of the electrodes on the opposite horizontal bus made dirty by I/O operations
					for (; p < dirtyPorts.size(); p++)
					{
						IoPort *port = dirtyPorts.at(p);
						claim(!(port->getSide() == WEST || port->getSide() == EAST), "Wash Droplets for original FPPC-DMFB (w/ East/West ports) not yet supported. Please use enhanced FPPC2-DMFB instead.");
						x = routeDropletAlongHorizontalRow(x, port->getPosXY(), y);
					}
					x = routeDropletAlongHorizontalRow(x, washOut->getPosXY(), y);
					//addPinAtCycle(washOut->getPinNo(), cycle++, true); // Output droplet
					addPinAtCycle(washOut->getPinNo(), cycle, cycle < firstOverlayCycle);
					cycle++;

					dropNum++;
					if (cycle > maxCycle)
						maxCycle = cycle;
				}
			}
		}

		// If we have any cyles with no pins, activate all holders (this is a quick bug fix)
		for (unsigned long long c = startCycle; c < maxCycle; c++)
		{
			if (pinActs->at(c)->empty())
				addPinAtCycle(-1, c, true);
			//for (unsigned j = 0; j < mHoldPins->size(); j++)
			//	addPinAtCycle(mHoldPins->at(j), cycle); // Make sure pin is forced off these cycles

			// Also, activate the S/S pins
			//for (unsigned j = 0; j < ssHoldPins->size(); j++)
			//	addPinAtCycle(ssHoldPins->at(j), cycle, false);
		}

		cycle = maxCycle;
		//cout << "Latest route ends at cycle " << cycle << "." << endl;
	}
}


///////////////////////////////////////////////////////////////////////////////////
// Given an input port and the target XY coordinates, actiavtes the pins to get the
// droplet from the input to the top or bottom of the central routing column.
// Returns y coordinate to tell if droplet is at top/bottom of column).
///////////////////////////////////////////////////////////////////////////////////
int GrissomFppcParallelRouter::routeFromInputToCentralColumn(IoPort *inPort, int tx, int ty)
{
	vector<vector<int> *> *pinMapping = arch->getPinMapper()->getPinMapping();
	int x;
	int y;
	int topY = 0;
	int botY = arch->getNumCellsY()-1;
	int leftX = 0;
	int rightX = arch->getNumCellsX()-1;
	int routingColumn = tx;


	addPinAtCycle(inPort->getPinNo(), cycle, false);

	////////////////////////////////////////////////////
	// First get to the top/bottom row
	if (inPort->getSide() == WEST || inPort->getSide() == EAST)
	{
		if (inPort->getSide() == WEST)
			x = leftX;
		else
			x = rightX;
		y = inPort->getPosXY();
		//addPinAtCycle(pinMapping->at(x)->at(y), cycle++, true);
		addPinAtCycle(pinMapping->at(x)->at(y), cycle, cycle < firstOverlayCycle);
		cycle++;


		int northFirstDist = inPort->getPosXY() + ty;
		int southFirstDist = (arch->getNumCellsY() - inPort->getPosXY()) + (arch->getNumCellsY() - ty);
		if (northFirstDist >= southFirstDist)
			y = routeDropletAlongVerticalColumn(y, botY, x);
		else
			y = routeDropletAlongVerticalColumn(y, topY, x);
	}
	else if (inPort->getSide() == NORTH || inPort->getSide() == SOUTH)
	{
		x = inPort->getPosXY();
		if (inPort->getSide() == NORTH)
			y = topY;
		else
			y = botY;
		//addPinAtCycle(pinMapping->at(x)->at(y), cycle++, true);
		addPinAtCycle(pinMapping->at(x)->at(y), cycle, cycle < firstOverlayCycle);
		cycle++;
	}

	////////////////////////////////////////////////////
	// Now, go right or left to get to the routing column
	x = routeDropletAlongHorizontalRow(x, routingColumn, y);

	return y;
}

///////////////////////////////////////////////////////////////////////////////////
// Extracts a droplet from the given mix/basic module into the vertical routing
// column. Returns the y dimension that the droplet ends up at.
///////////////////////////////////////////////////////////////////////////////////
int GrissomFppcParallelRouter::extractDropletFromMixModule(ReconfigModule *rm)
{
	claim(rm->getResourceType() == BASIC_RES, "FPPC router expecting to extract droplet from a basic mixing module but given a module of a different type.");
	// Activate the SS_RES holders to keep them in place
	for (unsigned k = 0; k < ssHoldPins->size(); k++)
		addPinAtCycle(ssHoldPins->at(k), cycle, false);
	// Also activate the bottom-right mixer pins
	for (unsigned k = 0; k < mixPins->size(); k++)
		addPinAtCycle(mixPins->at(k)->back(), cycle, false);
	for (unsigned k = 0; k < mHoldPins->size(); k++)
		forcePinOffAtCycle(mHoldPins->at(k), cycle); // Make sure pin is forced off these cycles
	cycle++;

	// Activate the SS_RES holders...
	for (unsigned k = 0; k < ssHoldPins->size(); k++)
		addPinAtCycle(ssHoldPins->at(k), cycle, false);
	// then, activate all the holders except the mix module we are departing
	// from...activate it's I/O pin instead
	for (unsigned k = 0; k < mHoldPins->size(); k++)
	{
		if (k == rm->getTileNum())
		{
			addPinAtCycle(mIOPins->at(k), cycle, false);
			forcePinOffAtCycle(mHoldPins->at(k), cycle-1); // Make sure pin is forced off these cycles
			forcePinOffAtCycle(mHoldPins->at(k), cycle);
		}
		else
			addPinAtCycle(mHoldPins->at(k), cycle, false);
	}
	cycle++;

	// Now, activate all the holders and the pin to get the droplet into the street
	//addPinAtCycle(pinMapping->at(centralRoutingColumn)->at(rm->getBY()), cycle++, true);
	addPinAtCycle(pinMapping->at(centralRoutingColumn)->at(rm->getBY()), cycle, cycle < firstOverlayCycle);
	cycle++;

	return rm->getBY();
}

///////////////////////////////////////////////////////////////////////////////////
// Extracts a droplet from the given SSD module into the vertical routing
// column. Returns the y dimension that the droplet ends up at.
///////////////////////////////////////////////////////////////////////////////////
int GrissomFppcParallelRouter::extractDropletFromSSDModule(ReconfigModule *rm)
{
	claim(rm->getResourceType() == SSD_RES, "FPPC router expecting to extract droplet from a SSD module but given a module of a different type.");
	// Activate all the mix holders; then activate all the split/store holds
	// except the s/s module we are departing from...activate it's I/O pin instead
	if (cycle < firstOverlayCycle)
	{
		for (unsigned k = 0; k < mHoldPins->size(); k++)
			addPinAtCycle(mHoldPins->at(k), cycle, false);
	}
	for (unsigned k = 0; k < ssHoldPins->size(); k++)
	{
		if (k == rm->getTileNum())
		{
			addPinAtCycle(ssIOPins->at(k), cycle, false);
			forcePinOffAtCycle(ssHoldPins->at(k), cycle); // Make sure pin is forced off these cycles
			forcePinOffAtCycle(ssHoldPins->at(k), cycle+1);
		}
		else
			addPinAtCycle(ssHoldPins->at(k), cycle, false);
	}
	cycle++;

	// Now, Activate all m holds, and all s/s holds, except the s/s module we
	// are departing from...just activate the pin to get the droplet into the street
	if (cycle < firstOverlayCycle)
	{
		for (unsigned k = 0; k < mHoldPins->size(); k++)
			addPinAtCycle(mHoldPins->at(k), cycle, false);
	}
	for (unsigned k = 0; k < ssHoldPins->size(); k++)
		if (k != rm->getTileNum())
			addPinAtCycle(ssHoldPins->at(k), cycle, false);
	addPinAtCycle(pinMapping->at(centralRoutingColumn)->at(rm->getBY()), cycle, false);
	cycle++;

	return rm->getBY();
}

///////////////////////////////////////////////////////////////////////////////////
// Inserts a droplet into the given mix/basic module into the vertical routing
// column.
///////////////////////////////////////////////////////////////////////////////////
void GrissomFppcParallelRouter::insertDropletIntoMixModule(ReconfigModule *rm)
{
	claim(rm->getResourceType() == BASIC_RES, "FPPC router expecting to insert droplet into a basic mixing module but given a module of a different type.");
	addPinAtCycle(mIOPins->at(rm->getTileNum()), cycle++, true);

	//addPinAtCycle(pinMapping->at(n->GetReconfigMod()->getRX())->at(n->GetReconfigMod()->getBY()), cycle++, true);
	// Activate the SS_RES holders to keep them in place, and also activate the bottom-right mixer pin
	for (unsigned k = 0; k < ssHoldPins->size(); k++)
		addPinAtCycle(ssHoldPins->at(k), cycle, false);
	// Also activate the bottom-right mixer pins
	for (unsigned k = 0; k < mixPins->size(); k++)
		addPinAtCycle(mixPins->at(k)->back(), cycle, false);
	for (unsigned k = 0; k < mHoldPins->size(); k++)
		forcePinOffAtCycle(mHoldPins->at(k), cycle); // Make sure pin is forced off these cycles
	cycle++;
	addPinAtCycle(-1, cycle++, true);

	// If is inserting at first overlay cycle, will need to delay start of time-step later
	if (firstOverlayCycle == cycle-1)
		isInsertingAtOverlayCycle = true;
}

///////////////////////////////////////////////////////////////////////////////////
// Inserts a droplet into the given SSD module into the vertical routing
// column. Returns the y dimension that the droplet ends up at.
///////////////////////////////////////////////////////////////////////////////////
void GrissomFppcParallelRouter::insertDropletIntoSSDModule(ReconfigModule *rm)
{
	claim(rm->getResourceType() == SSD_RES, "FPPC router expecting to insert droplet into a SSD module but given a module of a different type.");
	//addPinAtCycle(ssIOPins->at(rm->getTileNum()), cycle++, true);
	addPinAtCycle(ssIOPins->at(rm->getTileNum()), cycle, cycle < firstOverlayCycle);
	cycle++;
	forcePinOffAtCycle(ssHoldPins->at(rm->getTileNum()), cycle-1); // Make sure pin is forced off these cycles
	//addPinAtCycle(-1, cycle++, true);
	addPinAtCycle(-1, cycle, cycle < firstOverlayCycle);
	cycle++;
}

///////////////////////////////////////////////////////////////////////////////////
// This funciton should be called once the droplet that is about to be split has
// already been routed to an SSD module. At that point, this function will split
// the droplet into 2 and cause the 2nd droplet to be routed to a second SSD
// module.
///////////////////////////////////////////////////////////////////////////////////
void GrissomFppcParallelRouter::splitNodeIntoSecondSSDModule(AssayNode *s)
{
	claim(s->GetType() == SPLIT, "FPPC router expecting to split a droplet, but droplet is not originating from a split node.");
	AssayNode *c0;
	AssayNode *c1;

	int x = centralRoutingColumn;
	int y = s->reconfigMod->getTY();

	if (s->reconfigMod->getTY() == s->GetChildren().at(0)->reconfigMod->getTY())
	{
		c0 = s->GetChildren().at(0);
		c1 = s->GetChildren().at(1);
	}
	else if (s->reconfigMod->getTY() == s->GetChildren().at(1)->reconfigMod->getTY())
	{
		c0 = s->GetChildren().at(1);
		c1 = s->GetChildren().at(0);
	}
	else
		claim(false, "A split's immediate storage children are in different modules than the split. Binder should not allow this.");

	////////////////////////////////////////////////////
	// One droplet stays in c0's module, move other droplet out and to c1
	addPinAtCycle(ssIOPins->at(s->GetReconfigMod()->getTileNum()), cycle++, true);
	addPinAtCycle(pinMapping->at(x)->at(y), cycle++, true);

	int tx = centralRoutingColumn;
	int ty = c1->reconfigMod->getBY();

	////////////////////////////////////////////////////
	// Okay, now we are somewhere in the routing column,
	// just go up or down till get to right row
	while (y < ty)
		addPinAtCycle(pinMapping->at(x)->at(++y), cycle++, true);
	while (y > ty)
		addPinAtCycle(pinMapping->at(x)->at(--y), cycle++, true);

	////////////////////////////////////////////////////
	// Finally, route droplet into the SS_RES module
	addPinAtCycle(ssIOPins->at(c1->GetReconfigMod()->getTileNum()), cycle++, true);
	forcePinOffAtCycle(ssHoldPins->at(c1->GetReconfigMod()->getTileNum()), cycle-1); // Make sure pin is forced off these cycles
	addPinAtCycle(-1, cycle++, true);
}


///////////////////////////////////////////////////////////////////////////////////
// Given a start y position, routes a droplet along the vertical routing column
// (given by the x coordinate) till it gets to the end y position.
///////////////////////////////////////////////////////////////////////////////////
int GrissomFppcParallelRouter::routeDropletAlongVerticalColumn(int startY, int endY, int x)
{
	int y = startY;
	while (y < endY)
	{
		//addPinAtCycle(pinMapping->at(x)->at(++y), cycle++, true);
		addPinAtCycle(pinMapping->at(x)->at(++y), cycle, cycle < firstOverlayCycle);
		cycle++;
	}
	while (y > endY)
	{
		//addPinAtCycle(pinMapping->at(x)->at(--y), cycle++, true);
		addPinAtCycle(pinMapping->at(x)->at(--y), cycle, cycle < firstOverlayCycle);
		cycle++;
	}
	return y;
}

///////////////////////////////////////////////////////////////////////////////////
// Given a start x position, routes a droplet along the horizontal routing column
// (given by the y coordinate) till it gets to the end x position.
///////////////////////////////////////////////////////////////////////////////////
int GrissomFppcParallelRouter::routeDropletAlongHorizontalRow(int startX, int endX, int y)
{
	int x = startX;
	while (x < endX)
	{
		//addPinAtCycle(pinMapping->at(++x)->at(y), cycle++, true);
		addPinAtCycle(pinMapping->at(++x)->at(y), cycle, cycle < firstOverlayCycle);
		cycle++;
	}
	while (x > endX)
	{
		//addPinAtCycle(pinMapping->at(--x)->at(y), cycle++, true);
		addPinAtCycle(pinMapping->at(--x)->at(y), cycle, cycle < firstOverlayCycle);
		cycle++;
	}
	return x;
}

///////////////////////////////////////////////////////////////////////////////////
// Given a cycle and pin number, adds the pin to a vector to ensure that this pin
// is forced to be deactivated at the given cycle. This is to ensure that no other
// concurrent droplets activate hold/pins when a concurrent droplet is being
// extracted from a module (causing a split).
///////////////////////////////////////////////////////////////////////////////////
void GrissomFppcParallelRouter::forcePinOffAtCycle(int pinNum, unsigned long long cycle)
{
	vector<int> *pins = NULL;
	map<unsigned long long, vector<int>* >::iterator it = pinsForcedOffAtCycle->find(cycle);
	if (it == pinsForcedOffAtCycle->end())
	{
		pins = new vector<int>();
		pinsForcedOffAtCycle->insert(pair<unsigned long long, vector<int>*>(cycle, pins));
	}
	else
		pins = it->second;
	pins->push_back(pinNum);
}

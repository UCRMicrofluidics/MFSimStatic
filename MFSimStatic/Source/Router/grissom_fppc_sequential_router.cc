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
 * Source: grissom_fppc_sequential_router.cc											*
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
#include "../../Headers/Router/grissom_fppc_sequential_router.h"

///////////////////////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////////////////////
GrissomFppcSequentialRouter::GrissomFppcSequentialRouter()
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
}
GrissomFppcSequentialRouter::GrissomFppcSequentialRouter(DmfbArch *dmfbArch)
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
}

///////////////////////////////////////////////////////////////////////////////////
// Deconstructor
///////////////////////////////////////////////////////////////////////////////////
GrissomFppcSequentialRouter::~GrissomFppcSequentialRouter()
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

	// Do not need to delete all the variables initialized as NULL in constructor
	// because they are only referenced here and will be deleted elsewhere.
}

///////////////////////////////////////////////////////////////////////
// This function performs any one-time initializations that the router
// needs that are specific to a particular router.
///////////////////////////////////////////////////////////////////////
void GrissomFppcSequentialRouter::routerSpecificInits()
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
void GrissomFppcSequentialRouter::processTimeStep(map<Droplet *, vector<RoutePoint *> *> *routes)
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
/*void GrissomFppcSequentialRouter::computeIndivSupProbRoutes(vector<vector<RoutePoint *> *> *subRoutes, vector<Droplet *> *subDrops, map<Droplet *, vector<RoutePoint *> *> *routes)
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
// Given an input port and the target XY coordinates, actiavtes the pins to get the
// droplet from the input to the top or bottom of the central routing column.
// Returns y coordinate to tell if droplet is at top/bottom of column).
///////////////////////////////////////////////////////////////////////////////////
int GrissomFppcSequentialRouter::routeFromInputToCentralColumn(IoPort *inPort, int tx, int ty)
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
		addPinAtCycle(pinMapping->at(x)->at(y), cycle++, true);

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
		addPinAtCycle(pinMapping->at(x)->at(y), cycle++, true);
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
int GrissomFppcSequentialRouter::extractDropletFromMixModule(ReconfigModule *rm)
{
	claim(rm->getResourceType() == BASIC_RES, "FPPC router expecting to extract droplet from a basic mixing module but given a module of a different type.");
	// Activate the SS_RES holders to keep them in place
	for (unsigned k = 0; k < ssHoldPins->size(); k++)
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
	addPinAtCycle(pinMapping->at(centralRoutingColumn)->at(rm->getBY()), cycle++, true);

	return rm->getBY();
}

///////////////////////////////////////////////////////////////////////////////////
// Extracts a droplet from the given SSD module into the vertical routing
// column. Returns the y dimension that the droplet ends up at.
///////////////////////////////////////////////////////////////////////////////////
int GrissomFppcSequentialRouter::extractDropletFromSSDModule(ReconfigModule *rm)
{
	claim(rm->getResourceType() == SSD_RES, "FPPC router expecting to extract droplet from a SSD module but given a module of a different type.");
	// Activate all the mix holders; then activate all the split/store holds
	// except the s/s module we are departing from...activate it's I/O pin instead
	for (unsigned k = 0; k < mHoldPins->size(); k++)
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
	cycle++;

	return rm->getBY();
}

///////////////////////////////////////////////////////////////////////////////////
// Inserts a droplet into the given mix/basic module into the vertical routing
// column.
///////////////////////////////////////////////////////////////////////////////////
void GrissomFppcSequentialRouter::insertDropletIntoMixModule(ReconfigModule *rm)
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
	cycle++;
	addPinAtCycle(-1, cycle++, true);
}

///////////////////////////////////////////////////////////////////////////////////
// Inserts a droplet into the given SSD module into the vertical routing
// column. Returns the y dimension that the droplet ends up at.
///////////////////////////////////////////////////////////////////////////////////
void GrissomFppcSequentialRouter::insertDropletIntoSSDModule(ReconfigModule *rm)
{
	claim(rm->getResourceType() == SSD_RES, "FPPC router expecting to insert droplet into a SSD module but given a module of a different type.");
	addPinAtCycle(ssIOPins->at(rm->getTileNum()), cycle++, true);
	addPinAtCycle(-1, cycle++, true);
}

///////////////////////////////////////////////////////////////////////////////////
// This funciton should be called once the droplet that is about to be split has
// already been routed to an SSD module. At that point, this function will split
// the droplet into 2 and cause the 2nd droplet to be routed to a second SSD
// module.
///////////////////////////////////////////////////////////////////////////////////
void GrissomFppcSequentialRouter::splitNodeIntoSecondSSDModule(AssayNode *s)
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
	addPinAtCycle(-1, cycle++, true);
}


///////////////////////////////////////////////////////////////////////////////////
// Adds the pin to the list of pins to be activated at the specified cycle
// If activateHolds is true, also adds to pins for the module holds.
// If pass pinNo==-1, won't add to list...this is a way to simply activate all
// the holders.
///////////////////////////////////////////////////////////////////////////////////
void GrissomFppcSequentialRouter::addPinAtCycle(int pinNo, int cycleNo, bool activateHolds)
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
int GrissomFppcSequentialRouter::routeDropletAlongVerticalColumn(int startY, int endY, int x)
{
	int y = startY;
	while (y < endY)
		addPinAtCycle(pinMapping->at(x)->at(++y), cycle++, true);
	while (y > endY)
		addPinAtCycle(pinMapping->at(x)->at(--y), cycle++, true);
	return y;
}

///////////////////////////////////////////////////////////////////////////////////
// Given a start x position, routes a droplet along the horizontal routing column
// (given by the y coordinate) till it gets to the end x position.
///////////////////////////////////////////////////////////////////////////////////
int GrissomFppcSequentialRouter::routeDropletAlongHorizontalRow(int startX, int endX, int y)
{
	int x = startX;
	while (x < endX)
		addPinAtCycle(pinMapping->at(++x)->at(y), cycle++, true);
	while (x > endX)
		addPinAtCycle(pinMapping->at(--x)->at(y), cycle++, true);
	return x;
}

///////////////////////////////////////////////////////////////////////////////////
// This function is the main public function called.  It fills the "routes" and
// "tsBeginningCycle" data structures and contains the code for the main routing flow.
///////////////////////////////////////////////////////////////////////////////////
void GrissomFppcSequentialRouter::route(DAG *dag, DmfbArch *arch, vector<ReconfigModule *> *rModules, map<Droplet *, vector<RoutePoint *> *> *routes, vector<vector<int> *> *pinActivations, vector<unsigned long long> *tsBeginningCycle)
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

	// Copy all nodes to a new list to be sorted
	vector<AssayNode *> nodes;// = new vector<AssayNode *>();
	for (unsigned i = 0; i < dag->getAllNodes().size(); i++)
		nodes.push_back(dag->getAllNodes().at(i));
	Sort::sortNodesByStartThenEndTS(&nodes);

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
		computeIndivSubProbRoutes(subRoutes, subDrops, routes);

		///////////////////////////////////////////////////////////////////////
		// Then, compact and do maintenance on the routes
		///////////////////////////////////////////////////////////////////////
		//compactRoutes(subDrops, subRoutes); // Now, do route COMPACTION
		//addSubProbToGlobalRoutes(subDrops, subRoutes, routes); // Add sub-rotues to routes for entire simulation
		//equalizeGlobalRoutes(routes); // Now, add cycles for the droplets that were waiting first so they look like they were really waiting there
		tsBeginningCycle->push_back(cycle); // Add cycle so we now when time-step begins
		processTimeStep(routes); // Now that routing is done, process the time-step


		///////////////////////////////////////////////////////////////////////
		// Cleanup
		///////////////////////////////////////////////////////////////////////
		for (int i = nodes.size()-1; i >= 0; i--)
		{
			if (nodes.at(i)->endTimeStep <= startingTS)
				nodes.erase(nodes.begin() + i);
		}
		while (!subRoutes->empty())
		{
			vector<RoutePoint *> * v = subRoutes->back();
			subRoutes->pop_back();
			delete v; // Individual RoutePoints are deleted later by the Util Class
		}
		delete subRoutes;
		delete subDrops;
		thisTS->clear();
		startingTS++;
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
void GrissomFppcSequentialRouter::computeIndivSubProbRoutes(vector<vector<RoutePoint *> *> *subRoutes, vector<Droplet *> *subDrops, map<Droplet *, vector<RoutePoint *> *> *routes)
{
	// DTGWD
	// Get first wash input/output
	//if (performWash())
	//{
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
	//}
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
							addPinAtCycle(port->getPinNo(), cycle++, true);
						else
						{
							y = routeDropletAlongVerticalColumn(y, ty, x);
							addPinAtCycle(port->getPinNo(), cycle++, true);
						}
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
					addPinAtCycle(washIn->getPinNo(), cycle, false);
					addPinAtCycle(pinMapping->at(x)->at(y), cycle++, true);

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
					addPinAtCycle(washOut->getPinNo(), cycle++, true); // Output droplet
				}
			}
		}
	}
}



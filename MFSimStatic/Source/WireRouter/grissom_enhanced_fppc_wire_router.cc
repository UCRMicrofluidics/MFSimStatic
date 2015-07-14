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
 * Source: grissom_enhanced_fppc_wire_router.cc											*
 * Original Code Author(s): Dan Grissom											*
 * Original Completion/Release Date: February 5, 2014							*
 *																				*
 * Details: N/A																	*
 *																				*
 * Revision History:															*
 * WHO		WHEN		WHAT													*
 * ---		----		----													*
 * FML		MM/DD/YY	One-line description									*
 *-----------------------------------------------------------------------------*/

#include "../../Headers/WireRouter/grissom_enhanced_fppc_wire_router.h"
#include "../../Headers/Util/sort.h"

///////////////////////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////////////////////
EnhancedFPPCWireRouter::EnhancedFPPCWireRouter(DmfbArch *dmfbArch)
{
	arch = dmfbArch;
	type = ENHANCED_FPPC_WR;
}

///////////////////////////////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////////////////////////////
EnhancedFPPCWireRouter::~EnhancedFPPCWireRouter()
{
}

///////////////////////////////////////////////////////////////////////////////////
// Given the pin-mapping (contained in the DmfbArch), computes the wire routing
// to connect all the pins together and to the output of the device.  Also, has
// access to the pin activations list in case this algorithm needs to modify the
// pin-mapping itself.  This algorithm uses lee's maze routing.
///////////////////////////////////////////////////////////////////////////////////
void EnhancedFPPCWireRouter::computeWireRoutes(vector<vector<int> *> *pinActivations)
{
	if (!((numHorizTracks == 2 && numVertTracks == 2) || (numHorizTracks == 3 && numVertTracks == 3)))
		claim(false, "The number or horizontal/vertical tracks for the FPPC2 router must be 2 or 3." );

	cout << "Beginning wire routing phase:" << endl;

	// Get max pin num
	maxPinNum = 0;
	map<int, vector<WireRouteNode *> *>::iterator groupIt;
	for (groupIt = model->getPinGroups()->begin();groupIt != model->getPinGroups()->end();groupIt++)
		if (maxPinNum == 0 || groupIt->first > maxPinNum)
			maxPinNum = groupIt->first;

	// Create empty wire vectors
	vector< vector<WireSegment *> *> *wires = arch->getWireRouter()->getWireRoutesPerPin();
	for (int i = 0; i <= maxPinNum; i++)
	{
		vector<WireSegment *> *wire = new vector<WireSegment *>();
		wires->push_back(wire);
	}

	// FPPC2 Design only has basic resources (mixers), ssd (split/store/detect) and rb (routing buffer) modules
	vector<vector<FixedModule *> *> * availRes = arch->getPinMapper()->getAvailRes();
	vector<FixedModule *> *mixers = new vector<FixedModule *>();
	vector<FixedModule *> *SSDsAndRBs = new vector<FixedModule *>();
	for (unsigned i = 0; i < availRes->at(BASIC_RES)->size(); i++)
		mixers->push_back(availRes->at(BASIC_RES)->at(i));
	for (unsigned i = 0; i < availRes->at(SSD_RES)->size(); i++)
		SSDsAndRBs->push_back(availRes->at(SSD_RES)->at(i));
	for (unsigned i = 0; i < availRes->at(RB_RES)->size(); i++)
		SSDsAndRBs->push_back(availRes->at(RB_RES)->at(i));
	int mixWidth = mixers->front()->getRX() - mixers->front()->getLX() + 1;

	// First, compute routing column, as done in pin-mapper
	int routingColumn = mixWidth + 2;
	int mid = arch->getNumCellsX() / 2;
	if (mid > routingColumn)
		routingColumn = mid;

	////////////////////////////////////////////////
	// Generate wire-routings
	////////////////////////////////////////////////
	routeMixingModules(mixers, wires);
	routeSSDModules(SSDsAndRBs, wires);

	// If pins are being shared in first row as a 3-phase routing bus, then must to pin-optimized wire routing
	if (arch->getPinMapper()->getType() == ENHANCED_FPPC_PIN_OPT_PM)
		routePinOptRoutingCells(routingColumn, wires);
	else if (arch->getPinMapper()->getType() == ENHANCED_FPPC_ROUTE_OPT_PM)
		routeRouteOptRoutingCells(routingColumn, wires);
	else
		claim(false, "Unknown/incompatible pin-mapping type in EnhancedFPPCWireRouter.");
}




///////////////////////////////////////////////////////////////////////////////////
// Performs wire routing for the routing cells. This is the pin-optimized version
// which routes each of the 3 routing channels (top/bottom horizontal; middle
// vertical) as a 3-phase bus.
///////////////////////////////////////////////////////////////////////////////////
void EnhancedFPPCWireRouter::routePinOptRoutingCells(int verticalColumnIndex, vector< vector<WireSegment *> *> *wires)
{
	if (numHorizTracks == 2)
		routePinOptRoutingCells2(verticalColumnIndex, wires);
	else if (numHorizTracks == 3)
		routePinOptRoutingCells3(verticalColumnIndex, wires);
}

///////////////////////////////////////////////////////////////////////////////////
// This function performs all the wire routing for the mixers.
///////////////////////////////////////////////////////////////////////////////////
void EnhancedFPPCWireRouter::routeMixingModules(vector<FixedModule *> *mixers, vector< vector<WireSegment *> *> *wires)
{
	if (numHorizTracks == 2)
		routeMixingModules2(mixers, wires);
	else if (numHorizTracks == 3)
		routeMixingModules3(mixers, wires);
}






///////////////////////////////////////////////////////////////////////////////////
// Performs wire routing for the routing cells. This is the pin-optimized version
// which routes each of the 3 routing channels (top/bottom horizontal; middle
// vertical) as a 3-phase bus.
///////////////////////////////////////////////////////////////////////////////////
void EnhancedFPPCWireRouter::routePinOptRoutingCells2(int verticalColumnIndex, vector< vector<WireSegment *> *> *wires)
{
	int phase = 3; // Number of electrodes (phase) between matching pins on BUS
	int d = 2; // Delta - number of wire-routing coordinates from orthogonal link to ortho. link
	//vector<vector<WireRouteNode *> *> * pins = model->getPmodel->getPins();
	WireRouteNode *pTr = NULL;
	WireRouteNode *pEscape = NULL;
	WireRouteNode *p = NULL;
	int x;
	int y = 0;

	////////////////////////////////////////////
	// Route the top and bottom horizontal channel from
	// right size of DMFB
	////////////////////////////////////////////
	for (int i = 0; i < 2; i++)
	{
		pTr = model->getPin(arch->getNumCellsX()-1, y); // Right-most pin
		pEscape = model->getPin(arch->getNumCellsX(), y); // Pin off DMFB to right
		////////////////////////////////////////////
		// Far-right pin (and all connecting pins) - Route from above
		x = arch->getNumCellsX()-1;
		p = pTr;
		wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 1, pEscape->wgX, pEscape->wgY-(2*d), p->wgX, p->wgY-(2*d)));


		while (x >= 0)
		{
			WireRouteNode *nP = model->getPin(x, y); // Next-pin
			wires->at(nP->originalPinNum)->push_back(new WireSegment(nP->originalPinNum, 1, p->wgX, p->wgY-(2*d), nP->wgX, nP->wgY-(2*d)));
			wires->at(nP->originalPinNum)->push_back(new WireSegment(nP->originalPinNum, 1, nP->wgX, nP->wgY-(2*d), nP->wgX, nP->wgY));
			x-=phase;
			p = nP;
		}
		////////////////////////////////////////////
		// 2nd pin from far-right (and all connecting pins)
		x = arch->getNumCellsX()-2;
		p = pEscape; // "p" will represent current starting spot in this while-loop

		while (x >= 0)
		{
			WireRouteNode *t = model->getPin(x, y); // Target
			//WireRouteNode *a = model->getPin(x+1, y); // Avoid this pin

			// From current point 'p' to avoid pin 'a', and then route around 'a'
			//wires->at(t->originalPinNum)->push_back(new WireSegment(t->originalPinNum, 0, p->wgX, p->wgY-d, a->wgX+d, a->wgY-d)); // To 'a'
			//wires->at(t->originalPinNum)->push_back(new WireSegment(t->originalPinNum, 0, a->wgX+d, a->wgY-d, a->wgX+d, a->wgY+d)); // Down below 'a'
			//wires->at(t->originalPinNum)->push_back(new WireSegment(t->originalPinNum, 0, a->wgX+d, a->wgY+d, a->wgX-d, a->wgY+d)); // Past 'a'
			//wires->at(t->originalPinNum)->push_back(new WireSegment(t->originalPinNum, 0, a->wgX-d, a->wgY+d, a->wgX-d, a->wgY-d)); // Up above 'a'

			// Over to the pin and then down to connect to it
			wires->at(t->originalPinNum)->push_back(new WireSegment(t->originalPinNum, 0, p->wgX, p->wgY-d, t->wgX, t->wgY-d));
			wires->at(t->originalPinNum)->push_back(new WireSegment(t->originalPinNum, 0, t->wgX, t->wgY-d, t->wgX, t->wgY));

			x-=phase;
			p = t;
		}
		////////////////////////////////////////////
		// 3rd pin from far-right (and all connecting pins)
		x = arch->getNumCellsX()-3;
		p = pEscape;

		while (x >= 0)
		{
			WireRouteNode *t = model->getPin(x, y); // Target

			// Over to the target pin and then up to connect to it
			wires->at(t->originalPinNum)->push_back(new WireSegment(t->originalPinNum, 1, p->wgX, p->wgY+(1*d), t->wgX, t->wgY+(1*d)));
			wires->at(t->originalPinNum)->push_back(new WireSegment(t->originalPinNum, 1, t->wgX, t->wgY+(1*d), t->wgX, t->wgY));

			x-=phase;
			p = t;
		}

		// Now change y to do the bottom row
		y = arch->getNumCellsY()-1;
	}

	////////////////////////////////////////////
	// Route the middle vertical channel from
	// right size of DMFB
	////////////////////////////////////////////
	pEscape = model->getPin(arch->getNumCellsX(), 1); // Pin off DMFB to right
	////////////////////////////////////////////
	// 2nd pin from top (and all connecting pins) - Route from right
	x = verticalColumnIndex;
	y = 2;
	WireRouteNode *topP = model->getPin(x, 1); // Top vertical BUS pin
	p = model->getPin(x, y); // Top vertical BUS pin

	// Route over
	wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, pEscape->wgX, pEscape->wgY-d, topP->wgX-(2*d), topP->wgY-d));
	wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, topP->wgX-(2*d), topP->wgY-d, topP->wgX-(2*d), topP->wgY));
	//wires->at(topP->originalPinNum)->push_back(new WireSegment(topP->originalPinNum, 0, topP->wgX-(2*d), topP->wgY, topP->wgX, topP->wgY));

	p = topP; // Start at p
	//y+=phase;

	while (y <= arch->getNumCellsY()-2)
	{
		WireRouteNode *nP = model->getPin(x, y); // Next-pin
		wires->at(nP->originalPinNum)->push_back(new WireSegment(nP->originalPinNum, 0, p->wgX-(2*d), p->wgY, nP->wgX-(2*d), nP->wgY));
		wires->at(nP->originalPinNum)->push_back(new WireSegment(nP->originalPinNum, 0, nP->wgX-(2*d), nP->wgY, nP->wgX, nP->wgY));
		y+=phase;
		p = nP;
	}

	////////////////////////////////////////////
	// Top-most in vertical BUS (and all connecting pins)
	y = 1;
	topP = model->getPin(x, y); // Top vertical BUS pin

	// Route over
	wires->at(topP->originalPinNum)->push_back(new WireSegment(topP->originalPinNum, 0, pEscape->wgX, pEscape->wgY, topP->wgX, topP->wgY));
	wires->at(topP->originalPinNum)->push_back(new WireSegment(topP->originalPinNum, 0, topP->wgX, topP->wgY, topP->wgX-d, topP->wgY));
	y+=phase;
	p = topP;

	while (y <= arch->getNumCellsY()-2)
	{
		WireRouteNode *t = model->getPin(x, y); // Target
		WireRouteNode *a = model->getPin(x, y-2); // Avoid this pin

		// From current point 'p' to avoid pin 'a', and then route around 'a'
		wires->at(t->originalPinNum)->push_back(new WireSegment(t->originalPinNum, 0, p->wgX-d, p->wgY, a->wgX-d, a->wgY-d)); // To 'a'
		wires->at(t->originalPinNum)->push_back(new WireSegment(t->originalPinNum, 0, a->wgX-d, a->wgY-d, a->wgX+d, a->wgY-d)); // Right of 'a'
		wires->at(t->originalPinNum)->push_back(new WireSegment(t->originalPinNum, 0, a->wgX+d, a->wgY-d, a->wgX+d, a->wgY+d)); // Past 'a'
		wires->at(t->originalPinNum)->push_back(new WireSegment(t->originalPinNum, 0, a->wgX+d, a->wgY+d, a->wgX-d, a->wgY+d)); // Left of 'a'

		// Over to the pin and then down to connect to it
		wires->at(t->originalPinNum)->push_back(new WireSegment(t->originalPinNum, 0, a->wgX-d, a->wgY+d, t->wgX-d, t->wgY));
		wires->at(t->originalPinNum)->push_back(new WireSegment(t->originalPinNum, 0, t->wgX-d, t->wgY, t->wgX, t->wgY));

		y+=phase;
		p = t;
	}

	////////////////////////////////////////////
	// 3rd pin from top (and all connecting pins)
	y = 3;
	p = model->getPin(x, y); // Top vertical BUS pin

	wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, pEscape->wgX, pEscape->wgY+d, topP->wgX+(2*d), topP->wgY+d));
	wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, topP->wgX+(2*d), topP->wgY+d, p->wgX+(2*d), p->wgY));
	wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, p->wgX+(2*d), p->wgY, p->wgX, p->wgY));
	y+=phase;

	while (y <= arch->getNumCellsY()-2)
	{
		WireRouteNode *t = model->getPin(x, y); // Target

		// Over to the target pin and then up to connect to it
		wires->at(t->originalPinNum)->push_back(new WireSegment(t->originalPinNum, 0, p->wgX+(2*d), p->wgY, t->wgX+(2*d), t->wgY));
		wires->at(t->originalPinNum)->push_back(new WireSegment(t->originalPinNum, 0, t->wgX+(2*d), t->wgY, t->wgX, t->wgY));

		y+=phase;
		p = t;
	}

}

///////////////////////////////////////////////////////////////////////////////////
// This function performs all the wire routing for the mixers.
///////////////////////////////////////////////////////////////////////////////////
void EnhancedFPPCWireRouter::routeMixingModules2(vector<FixedModule *> *mixers, vector< vector<WireSegment *> *> *wires)
{
	// Sort so modules are in order from top to bottom
	Sort::sortFixedModulesFromTopToBottom(mixers);

	//vector<vector<WireRouteNode *> *> * pins = model->getPins();

	// First, wire up all mixers and corresponding mixer I/O
	// Mixers are 4wide x 2tall (so there are 4 columns) and an entrance to the right of
	// the bottom right electrode in 5th column
	while (!mixers->empty())
	{
		// Route each group of 4 mix modules (can do less than 4 on last group)
		vector<FixedModule *> group;
		for (unsigned i = 0; i < 4; i++)
		{
			if (mixers->empty())
				break;

			group.push_back(mixers->front());
			mixers->erase(mixers->begin());
		}

		// Connect all of the mixers pins together
		for (unsigned i = 0; i < group.size(); i++)
		{
			FixedModule *m = group.at(i); // Module of interest
			FixedModule *mBot = group.back(); // Bottom module
			int hOff = 0; // Start height offset
			int d = 2; // Delta - number of wire-routing coordinates from orthogonal link to ortho. link

			////////////////////////////////////////////
			// WIRING FROM TOP DOWN
			////////////////////////////////////////////
			////////////////////////////////////////////
			// Column 1 - Top Pin
			WireRouteNode *p = model->getPin(m->getLX(), m->getTY());
			//WireRouteNode *e = model->getPin(-1, m->getTY()); // "E"scape pin (edge pin)
			WireRouteNode *p2;
			hOff = p->wgY;
			if (i == 0)
			{
				// Connect to edge
				//wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, -1, -1, 0, p->wgY));
				wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, 0, p->wgY, p->wgX, p->wgY));

				// Draw main vertical line to bottom pin
				p2 = model->getPin(mBot->getLX(),mBot->getTY());
				if (group.size() > 1)
					wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, p->wgX+d, p->wgY, p->wgX+d, p2->wgY));
			}
			// Draw lip to right to connect to main vertical line
			if (group.size() > 1)
				wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, p->wgX, p->wgY, p->wgX+d, p->wgY));

			////////////////////////////////////////////
			// Column 1 - Bottom Pin
			p = model->getPin(m->getLX(), m->getBY());
			if (i == 0)
			{
				// Connect to edge
				//wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, -1, -1, 0, p->wgY));
				wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, 0, p->wgY, p->wgX, p->wgY));

				// Draw main vertical line to bottom pin
				p2 = model->getPin(mBot->getLX(), mBot->getBY());
				wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, p->wgX-d, p->wgY, p->wgX-d, p2->wgY));
			}
			// Draw lip to right to connect to main vertical line
			if (i != 0)
				wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, p->wgX, p->wgY, p->wgX-d, p->wgY));

			////////////////////////////////////////////
			// Column 2 - Bottom Pin
			hOff-=d;
			p = model->getPin(m->getLX()+1, m->getBY());
			if (i == 0)
			{
				// Connect to edge
				//wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, -1, -1, 0, hOff));
				wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, 0, hOff, p->wgX-d, hOff));

				// Draw main vertical line to bottom pin
				p2 = model->getPin(mBot->getLX()+1, mBot->getBY());
				wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, p->wgX-d, hOff, p->wgX-d, p2->wgY));
			}
			// Draw lip to right to connect to main vertical line
			wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, p->wgX, p->wgY, p->wgX-d, p->wgY));

			////////////////////////////////////////////
			// Column 2 - Top Pin
			hOff-=d;
			p = model->getPin(m->getLX()+1, m->getTY());
			if (i == 0)
			{
				// Connect to edge
				//wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, -1, -1, 0, hOff));
				wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, 0, hOff, p->wgX+d, hOff));

				// Draw main vertical line to bottom pin
				p2 = model->getPin(mBot->getLX()+1, mBot->getTY());
				wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, p->wgX+d, hOff, p->wgX+d, p2->wgY));
			}
			// Draw lip to right to connect to main vertical line
			wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, p->wgX, p->wgY, p->wgX+d, p->wgY));

			////////////////////////////////////////////
			// Column 3 - Bottom pin for top 2 mixers
			hOff-=d;
			if (i == 0)
			{
				// If there is a 2nd mixer, do it (the lower one) first
				if (group.size() > 1)
				{
					FixedModule *m2 = group.at(1);
					p = model->getPin(m2->getLX()+2, m2->getBY());

					// Connect to edge
					wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 1, 0, hOff, p->wgX-(1*d), hOff));

					// Draw main vertical line to pin
					wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 1, p->wgX-(1*d), hOff, p->wgX-(1*d), p->wgY));
					// Draw lip to right to connect to vertical line
					wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 1, p->wgX-(1*d), p->wgY, p->wgX, p->wgY));
					//hOff-=d;
				} // DTG NOW

				// Top Mixer
				p = model->getPin(m->getLX()+2, m->getBY());

				// Connect to edge
				//wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, -1, -1, 0, hOff));
				wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, 0, hOff, p->wgX-d, hOff));

				// Draw main vertical line to pin
				wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, p->wgX-d, hOff, p->wgX-d, p->wgY));
				// Draw lip to right to connect to vertical line
				wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, p->wgX-d, p->wgY, p->wgX, p->wgY));
			}
			else if (group.size() > 1)
				hOff-=d;

			////////////////////////////////////////////
			// Column 3 - Top pin
			hOff-=d;
			p = model->getPin(m->getLX()+2, m->getTY());
			if (i == 0)
			{
				// Connect to edge
				//wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, -1, -1, 0, hOff));
				wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, 0, hOff, p->wgX+d, hOff));

				// Draw main vertical line to bottom pin
				p2 = model->getPin(mBot->getLX()+2, mBot->getTY());
				wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, p->wgX+d, hOff, p->wgX+d, p2->wgY));
			}
			// Draw lip to right to connect to main vertical line
			wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, p->wgX, p->wgY, p->wgX+d, p->wgY));

			////////////////////////////////////////////
			// Column 4 - Bottom Pin
			hOff-=d;
			p = model->getPin(m->getLX()+3, m->getBY());
			if (i == 0)
			{
				// Connect to edge
				//wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, -1, -1, 0, hOff));
				wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, 0, hOff, p->wgX-d, hOff));

				// Draw main vertical line to bottom pin
				p2 = model->getPin(mBot->getLX()+3, mBot->getBY());
				wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, p->wgX-d, hOff, p->wgX-d, p2->wgY));
			}
			// Draw lip to right to connect to main vertical line
			wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, p->wgX, p->wgY, p->wgX-d, p->wgY));

			////////////////////////////////////////////
			// Column 4 - Top Pin
			//hOff-=d;
			p = model->getPin(m->getLX()+3, m->getTY());
			if (i == 0)
			{
				// Connect to edge
				//wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, -1, -1, 0, hOff));
				wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 1, 0, hOff, p->wgX-d, hOff));

				// Draw main vertical line to bottom pin
				p2 = model->getPin(mBot->getLX()+3, mBot->getTY());
				wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 1, p->wgX-d, hOff, p->wgX-d, p2->wgY));
			}
			// Draw lip to right to connect to main vertical line
			wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 1, p->wgX-d, p->wgY, p->wgX, p->wgY));
			// DTG NOW


			////////////////////////////////////////////
			// Column 5 - Mixer I/O (Bottom) pin for top 2 mixers
			hOff-=d;
			if (i == 0)
			{
				// If there is a 2nd mixer, do it (the lower one) first
				if (group.size() > 1)
				{
					FixedModule *m2 = group.at(1);
					p = model->getPin(m2->getLX()+4, m2->getBY());

					// Connect to edge
					//wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, -1, -1, 0, hOff));
					wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, 0, hOff, p->wgX-(2*d), hOff));

					// Draw main vertical line to pin
					wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, p->wgX-(2*d), hOff, p->wgX-(2*d), p->wgY));
					// Draw lip to right to connect to vertical line
					wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, p->wgX-(2*d), p->wgY, p->wgX, p->wgY));
					hOff-=d;
				}

				// Top Mixer
				p = model->getPin(m->getLX()+4, m->getBY());

				// Connect to edge
				//wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, -1, -1, 0, hOff));
				wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, 0, hOff, p->wgX-d, hOff));

				// Draw main vertical line to pin
				wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, p->wgX-d, hOff, p->wgX-d, p->wgY));
				// Draw lip to right to connect to vertical line
				wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, p->wgX-d, p->wgY, p->wgX, p->wgY));
			}
			else if (group.size() > 1)
				hOff-=d;


			////////////////////////////////////////////
			// WIRING FROM BOTTOM UP
			////////////////////////////////////////////
			hOff = model->getPin(mBot->getLX(), mBot->getBY())->wgY;

			////////////////////////////////////////////
			// Column 3 - Bottom pin for top 2 mixers
			if (i == 0)
			{
				// If there is a 3rd mixer, do it (the upper one) first
				if (group.size() > 2)
				{
					hOff+=d;
					FixedModule *m3 = group.at(2);
					p = model->getPin(m3->getLX()+2, m3->getBY());

					// Connect to edge
					//wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, -1, -1, 0, hOff));
					wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, 0, hOff, p->wgX-(1*d), hOff));

					// Draw main vertical line to pin
					wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, p->wgX-(1*d), hOff, p->wgX-(1*d), p->wgY));
					// Draw lip to right to connect to vertical line
					wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, p->wgX-(1*d), p->wgY, p->wgX, p->wgY));
				}

				// If there is a 4th mixer, do it (the lower one) last
				if (group.size() > 3)
				{
					hOff+=d;
					FixedModule *m4 = group.at(3);
					p = model->getPin(m4->getLX()+2, m4->getBY());

					// Connect to edge
					//wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, -1, -1, 0, hOff));
					wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, 0, hOff, p->wgX, hOff));

					// Draw main vertical line to pin
					wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, p->wgX, hOff, p->wgX, p->wgY));
					// Draw lip to right to connect to vertical line
					//wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, p->wgX-d, p->wgY, p->wgX, p->wgY));
				}
			}
			else
				for (unsigned i = 2; i < group.size(); i++)
					hOff+=d;

			////////////////////////////////////////////
			// Column 5 - Mixer I/O (Bottom) pin for bottom 2 mixers
			if (i == 0)
			{
				// If there is a 3rd mixer, do it (the upper one) first
				if (group.size() > 2)
				{
					hOff+=d;
					FixedModule *m3 = group.at(2);
					p = model->getPin(m3->getLX()+4, m3->getBY());

					// Connect to edge
					//wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, -1, -1, 0, hOff));
					wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, 0, hOff, p->wgX-(2*d), hOff));

					// Draw main vertical line to pin
					wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, p->wgX-(2*d), hOff, p->wgX-(2*d), p->wgY));
					// Draw lip to right to connect to vertical line
					wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, p->wgX-(2*d), p->wgY, p->wgX, p->wgY));
				} // DTG NOW

				// If there is a 4th mixer, do it (the lower one) last
				if (group.size() > 3)
				{
					hOff+=d;
					FixedModule *m4 = group.at(3);
					p = model->getPin(m4->getLX()+4, m4->getBY());

					// Connect to edge
					//wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, -1, -1, 0, hOff));
					wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, 0, hOff, p->wgX, hOff));

					// Draw main vertical line to pin
					wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, p->wgX, hOff, p->wgX, p->wgY));
					// Draw lip to right to connect to vertical line
					//wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, p->wgX-d, p->wgY, p->wgX, p->wgY));
				}
			}
			else
				for (unsigned i = 2; i < group.size(); i++)
					hOff+=d;
		}
	}

}






///////////////////////////////////////////////////////////////////////////////////
// This function performs all the wire routing for the SSD modules.
///////////////////////////////////////////////////////////////////////////////////
void EnhancedFPPCWireRouter::routeSSDModules(vector<FixedModule *> *ssdMods, vector< vector<WireSegment *> *> *wires)
{
	int d = 2; // Delta - number of wire-routing coordinates from orthogonal link to ortho. link
	//vector<vector<WireRouteNode *> *> * pins = model->getPins();

	// Sort so modules are in order from top to bottom
	Sort::sortFixedModulesFromTopToBottom(ssdMods);

	for (unsigned i = 0; i < ssdMods->size(); i++)
	{
		FixedModule *m = ssdMods->at(i);
		WireRouteNode *p = model->getPin(m->getLX(), m->getTY());
		WireRouteNode *pR = model->getPin(arch->getNumCellsX(), m->getTY()); // Pin off DMFB to right
		WireRouteNode *pL = model->getPin(m->getLX()-1, m->getTY()); // I/O pin for SSD module

		////////////////////////////////////////////
		// Wire SSD pin
		//wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, -1, -1, pR->wgX, pR->wgY));
		wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, pR->wgX, pR->wgY, p->wgX, p->wgY));

		////////////////////////////////////////////
		// Wire SSD I/O pin above SSD pin
		//wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, -1, -1, pR->wgX, pR->wgY-d));
		wires->at(pL->originalPinNum)->push_back(new WireSegment(pL->originalPinNum, 0, pR->wgX, pR->wgY-d, pL->wgX, pL->wgY-d));
		wires->at(pL->originalPinNum)->push_back(new WireSegment(pL->originalPinNum, 0, pL->wgX, pL->wgY-d, pL->wgX, pL->wgY));
	}
}

///////////////////////////////////////////////////////////////////////////////////
// Performs wire routing for the routing cells. This is the routing-optimized version
// which routes each pin directly to provide the maximum flexibility.
///////////////////////////////////////////////////////////////////////////////////
void EnhancedFPPCWireRouter::routeRouteOptRoutingCells(int verticalColumnIndex, vector< vector<WireSegment *> *> *wires)
{
	int d = 2; // Delta - number of wire-routing coordinates from orthogonal link to ortho. link

	for (unsigned i = 0; i < arch->getNumCellsX(); i++)
	{
		// Wire top horizontal row
		WireRouteNode *bp = model->getPin(i, -1);
		WireRouteNode *rp = model->getPin(i, 0);
		wires->at(rp->originalPinNum)->push_back(new WireSegment(rp->originalPinNum, 0, bp->wgX, bp->wgY, rp->wgX, rp->wgY));

		// Wire top horizontal row
		rp = model->getPin(i, arch->getNumCellsY()-1);
		bp = model->getPin(i, arch->getNumCellsY());
		wires->at(rp->originalPinNum)->push_back(new WireSegment(rp->originalPinNum, 0, bp->wgX, bp->wgY, rp->wgX, rp->wgY));
	}

	for (unsigned i = 1; i < arch->getNumCellsY()-1; i++)
	{
		WireRouteNode *bp = model->getPin(arch->getNumCellsX(), i);
		WireRouteNode *rp = model->getPin(verticalColumnIndex, i);

		// For first row and all even rows - can route directly
		if (i == 1 || i % 2 == 0)
			wires->at(rp->originalPinNum)->push_back(new WireSegment(rp->originalPinNum, 0, bp->wgX, bp->wgY, rp->wgX, rp->wgY)); // Wire directly from right side
		else
		{
			// Wire from right side, just below pin
			WireRouteNode *bp = model->getPin(arch->getNumCellsX(), i);
			WireRouteNode *rp = model->getPin(verticalColumnIndex, i);
			wires->at(rp->originalPinNum)->push_back(new WireSegment(rp->originalPinNum, 0, bp->wgX, bp->wgY+d, rp->wgX, rp->wgY+d));
			wires->at(rp->originalPinNum)->push_back(new WireSegment(rp->originalPinNum, 0, rp->wgX, rp->wgY+d, rp->wgX, rp->wgY));
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////
// Performs wire routing for the routing cells. This is the pin-optimized version
// which routes each of the 3 routing channels (top/bottom horizontal; middle
// vertical) as a 3-phase bus.
///////////////////////////////////////////////////////////////////////////////////
void EnhancedFPPCWireRouter::routePinOptRoutingCells3(int verticalColumnIndex, vector< vector<WireSegment *> *> *wires)
{
	int phase = 3; // Number of electrodes (phase) between matching pins on BUS
	int d = 2; // Delta - number of wire-routing coordinates from orthogonal link to ortho. link
	//vector<vector<WireRouteNode *> *> * pins = model->getPmodel->getPins();
	WireRouteNode *pTr = NULL;
	WireRouteNode *pEscape = NULL;
	WireRouteNode *p = NULL;
	int x;
	int y = 0;

	////////////////////////////////////////////
	// Route the top and bottom horizontal channel from
	// right size of DMFB
	////////////////////////////////////////////
	for (int i = 0; i <2; i++)
	{
		pTr = model->getPin(arch->getNumCellsX()-1, y); // Right-most pin
		pEscape = model->getPin(arch->getNumCellsX(), y); // Pin off DMFB to right
		////////////////////////////////////////////
		// Far-right pin (and all connecting pins) - Route from above
		x = arch->getNumCellsX()-1;
		p = pTr;
		wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, pEscape->wgX, pEscape->wgY-(2*d), p->wgX, p->wgY-(2*d)));


		while (x >= 0)
		{
			WireRouteNode *nP = model->getPin(x, y); // Next-pin
			wires->at(nP->originalPinNum)->push_back(new WireSegment(nP->originalPinNum, 0, p->wgX, p->wgY-(2*d), nP->wgX, nP->wgY-(2*d)));
			wires->at(nP->originalPinNum)->push_back(new WireSegment(nP->originalPinNum, 0, nP->wgX, nP->wgY-(2*d), nP->wgX, nP->wgY));
			x-=phase;
			p = nP;
		}
		////////////////////////////////////////////
		// 2nd pin from far-right (and all connecting pins)
		x = arch->getNumCellsX()-2;
		p = pEscape; // "p" will represent current starting spot in this while-loop

		while (x >= 0)
		{
			WireRouteNode *t = model->getPin(x, y); // Target
			WireRouteNode *a = model->getPin(x+1, y); // Avoid this pin

			// From current point 'p' to avoid pin 'a', and then route around 'a'
			wires->at(t->originalPinNum)->push_back(new WireSegment(t->originalPinNum, 0, p->wgX, p->wgY-d, a->wgX+d, a->wgY-d)); // To 'a'
			wires->at(t->originalPinNum)->push_back(new WireSegment(t->originalPinNum, 0, a->wgX+d, a->wgY-d, a->wgX+d, a->wgY+d)); // Down below 'a'
			wires->at(t->originalPinNum)->push_back(new WireSegment(t->originalPinNum, 0, a->wgX+d, a->wgY+d, a->wgX-d, a->wgY+d)); // Past 'a'
			wires->at(t->originalPinNum)->push_back(new WireSegment(t->originalPinNum, 0, a->wgX-d, a->wgY+d, a->wgX-d, a->wgY-d)); // Up above 'a'

			// Over to the pin and then down to connect to it
			wires->at(t->originalPinNum)->push_back(new WireSegment(t->originalPinNum, 0, a->wgX-d, a->wgY-d, t->wgX, t->wgY-d));
			wires->at(t->originalPinNum)->push_back(new WireSegment(t->originalPinNum, 0, t->wgX, t->wgY-d, t->wgX, t->wgY));

			x-=phase;
			p = t;
		}
		////////////////////////////////////////////
		// 3rd pin from far-right (and all connecting pins)
		x = arch->getNumCellsX()-3;
		p = pEscape;

		while (x >= 0)
		{
			WireRouteNode *t = model->getPin(x, y); // Target

			// Over to the target pin and then up to connect to it
			wires->at(t->originalPinNum)->push_back(new WireSegment(t->originalPinNum, 0, p->wgX, p->wgY+(2*d), t->wgX, t->wgY+(2*d)));
			wires->at(t->originalPinNum)->push_back(new WireSegment(t->originalPinNum, 0, t->wgX, t->wgY+(2*d), t->wgX, t->wgY));

			x-=phase;
			p = t;
		}

		// Now change y to do the bottom row
		y = arch->getNumCellsY()-1;
	}

	////////////////////////////////////////////
	// Route the middle vertical channel from
	// right size of DMFB
	////////////////////////////////////////////
	pEscape = model->getPin(arch->getNumCellsX(), 1); // Pin off DMFB to right
	////////////////////////////////////////////
	// 2nd pin from top (and all connecting pins) - Route from right
	x = verticalColumnIndex;
	y = 2;
	WireRouteNode *topP = model->getPin(x, 1); // Top vertical BUS pin
	p = model->getPin(x, y); // Top vertical BUS pin

	// Route over
	wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, pEscape->wgX, pEscape->wgY-d, topP->wgX-(2*d), topP->wgY-d));
	wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, topP->wgX-(2*d), topP->wgY-d, topP->wgX-(2*d), topP->wgY));
	//wires->at(topP->originalPinNum)->push_back(new WireSegment(topP->originalPinNum, 0, topP->wgX-(2*d), topP->wgY, topP->wgX, topP->wgY));

	p = topP; // Start at p
	//y+=phase;

	while (y <= arch->getNumCellsY()-2)
	{
		WireRouteNode *nP = model->getPin(x, y); // Next-pin
		wires->at(nP->originalPinNum)->push_back(new WireSegment(nP->originalPinNum, 0, p->wgX-(2*d), p->wgY, nP->wgX-(2*d), nP->wgY));
		wires->at(nP->originalPinNum)->push_back(new WireSegment(nP->originalPinNum, 0, nP->wgX-(2*d), nP->wgY, nP->wgX, nP->wgY));
		y+=phase;
		p = nP;
	}

	////////////////////////////////////////////
	// Top-most in vertical BUS (and all connecting pins)
	y = 1;
	topP = model->getPin(x, y); // Top vertical BUS pin

	// Route over
	wires->at(topP->originalPinNum)->push_back(new WireSegment(topP->originalPinNum, 0, pEscape->wgX, pEscape->wgY, topP->wgX, topP->wgY));
	wires->at(topP->originalPinNum)->push_back(new WireSegment(topP->originalPinNum, 0, topP->wgX, topP->wgY, topP->wgX-d, topP->wgY));
	y+=phase;
	p = topP;

	while (y <= arch->getNumCellsY()-2)
	{
		WireRouteNode *t = model->getPin(x, y); // Target
		WireRouteNode *a = model->getPin(x, y-2); // Avoid this pin

		// From current point 'p' to avoid pin 'a', and then route around 'a'
		wires->at(t->originalPinNum)->push_back(new WireSegment(t->originalPinNum, 0, p->wgX-d, p->wgY, a->wgX-d, a->wgY-d)); // To 'a'
		wires->at(t->originalPinNum)->push_back(new WireSegment(t->originalPinNum, 0, a->wgX-d, a->wgY-d, a->wgX+d, a->wgY-d)); // Right of 'a'
		wires->at(t->originalPinNum)->push_back(new WireSegment(t->originalPinNum, 0, a->wgX+d, a->wgY-d, a->wgX+d, a->wgY+d)); // Past 'a'
		wires->at(t->originalPinNum)->push_back(new WireSegment(t->originalPinNum, 0, a->wgX+d, a->wgY+d, a->wgX-d, a->wgY+d)); // Left of 'a'

		// Over to the pin and then down to connect to it
		wires->at(t->originalPinNum)->push_back(new WireSegment(t->originalPinNum, 0, a->wgX-d, a->wgY+d, t->wgX-d, t->wgY));
		wires->at(t->originalPinNum)->push_back(new WireSegment(t->originalPinNum, 0, t->wgX-d, t->wgY, t->wgX, t->wgY));

		y+=phase;
		p = t;
	}

	////////////////////////////////////////////
	// 3rd pin from top (and all connecting pins)
	y = 3;
	p = model->getPin(x, y); // Top vertical BUS pin

	wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, pEscape->wgX, pEscape->wgY+d, topP->wgX+(2*d), topP->wgY+d));
	wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, topP->wgX+(2*d), topP->wgY+d, p->wgX+(2*d), p->wgY));
	wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, p->wgX+(2*d), p->wgY, p->wgX, p->wgY));
	y+=phase;

	while (y <= arch->getNumCellsY()-2)
	{
		WireRouteNode *t = model->getPin(x, y); // Target

		// Over to the target pin and then up to connect to it
		wires->at(t->originalPinNum)->push_back(new WireSegment(t->originalPinNum, 0, p->wgX+(2*d), p->wgY, t->wgX+(2*d), t->wgY));
		wires->at(t->originalPinNum)->push_back(new WireSegment(t->originalPinNum, 0, t->wgX+(2*d), t->wgY, t->wgX, t->wgY));

		y+=phase;
		p = t;
	}

}

///////////////////////////////////////////////////////////////////////////////////
// This function performs all the wire routing for the mixers.
///////////////////////////////////////////////////////////////////////////////////
void EnhancedFPPCWireRouter::routeMixingModules3(vector<FixedModule *> *mixers, vector< vector<WireSegment *> *> *wires)
{
	// Sort so modules are in order from top to bottom
	Sort::sortFixedModulesFromTopToBottom(mixers);

	//vector<vector<WireRouteNode *> *> * pins = model->getPins();

	// First, wire up all mixers and corresponding mixer I/O
	// Mixers are 4wide x 2tall (so there are 4 columns) and an entrance to the right of
	// the bottom right electrode in 5th column
	while (!mixers->empty())
	{
		// Route each group of 4 mix modules (can do less than 4 on last group)
		vector<FixedModule *> group;
		for (unsigned i = 0; i < 4; i++)
		{
			if (mixers->empty())
				break;

			group.push_back(mixers->front());
			mixers->erase(mixers->begin());
		}

		// Connect all of the mixers pins together
		for (unsigned i = 0; i < group.size(); i++)
		{
			FixedModule *m = group.at(i); // Module of interest
			FixedModule *mBot = group.back(); // Bottom module
			int hOff = 0; // Start height offset
			int d = 2; // Delta - number of wire-routing coordinates from orthogonal link to ortho. link

			////////////////////////////////////////////
			// WIRING FROM TOP DOWN
			////////////////////////////////////////////
			////////////////////////////////////////////
			// Column 1 - Top Pin
			WireRouteNode *p = model->getPin(m->getLX(), m->getTY());
			//WireRouteNode *e = model->getPin(-1, m->getTY()); // "E"scape pin (edge pin)
			WireRouteNode *p2;
			hOff = p->wgY;
			if (i == 0)
			{
				// Connect to edge
				//wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, -1, -1, 0, p->wgY));
				wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, 0, p->wgY, p->wgX, p->wgY));

				// Draw main vertical line to bottom pin
				p2 = model->getPin(mBot->getLX(),mBot->getTY());
				if (group.size() > 1)
					wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, p->wgX+d, p->wgY, p->wgX+d, p2->wgY));
			}
			// Draw lip to right to connect to main vertical line
			if (group.size() > 1)
				wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, p->wgX, p->wgY, p->wgX+d, p->wgY));

			////////////////////////////////////////////
			// Column 1 - Bottom Pin
			p = model->getPin(m->getLX(), m->getBY());
			if (i == 0)
			{
				// Connect to edge
				//wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, -1, -1, 0, p->wgY));
				wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, 0, p->wgY, p->wgX, p->wgY));

				// Draw main vertical line to bottom pin
				p2 = model->getPin(mBot->getLX(), mBot->getBY());
				wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, p->wgX-d, p->wgY, p->wgX-d, p2->wgY));
			}
			// Draw lip to right to connect to main vertical line
			if (i != 0)
				wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, p->wgX, p->wgY, p->wgX-d, p->wgY));

			////////////////////////////////////////////
			// Column 2 - Bottom Pin
			hOff-=d;
			p = model->getPin(m->getLX()+1, m->getBY());
			if (i == 0)
			{
				// Connect to edge
				//wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, -1, -1, 0, hOff));
				wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, 0, hOff, p->wgX-d, hOff));

				// Draw main vertical line to bottom pin
				p2 = model->getPin(mBot->getLX()+1, mBot->getBY());
				wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, p->wgX-d, hOff, p->wgX-d, p2->wgY));
			}
			// Draw lip to right to connect to main vertical line
			wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, p->wgX, p->wgY, p->wgX-d, p->wgY));

			////////////////////////////////////////////
			// Column 2 - Top Pin
			hOff-=d;
			p = model->getPin(m->getLX()+1, m->getTY());
			if (i == 0)
			{
				// Connect to edge
				//wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, -1, -1, 0, hOff));
				wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, 0, hOff, p->wgX+d, hOff));

				// Draw main vertical line to bottom pin
				p2 = model->getPin(mBot->getLX()+1, mBot->getTY());
				wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, p->wgX+d, hOff, p->wgX+d, p2->wgY));
			}
			// Draw lip to right to connect to main vertical line
			wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, p->wgX, p->wgY, p->wgX+d, p->wgY));

			////////////////////////////////////////////
			// Column 3 - Bottom pin for top 2 mixers
			hOff-=d;
			if (i == 0)
			{
				// If there is a 2nd mixer, do it (the lower one) first
				if (group.size() > 1)
				{
					FixedModule *m2 = group.at(1);
					p = model->getPin(m2->getLX()+2, m2->getBY());

					// Connect to edge
					//wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, -1, -1, 0, hOff));
					wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, 0, hOff, p->wgX-(2*d), hOff));

					// Draw main vertical line to pin
					wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, p->wgX-(2*d), hOff, p->wgX-(2*d), p->wgY));
					// Draw lip to right to connect to vertical line
					wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, p->wgX-(2*d), p->wgY, p->wgX, p->wgY));
					hOff-=d;
				}

				// Top Mixer
				p = model->getPin(m->getLX()+2, m->getBY());

				// Connect to edge
				//wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, -1, -1, 0, hOff));
				wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, 0, hOff, p->wgX-d, hOff));

				// Draw main vertical line to pin
				wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, p->wgX-d, hOff, p->wgX-d, p->wgY));
				// Draw lip to right to connect to vertical line
				wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, p->wgX-d, p->wgY, p->wgX, p->wgY));
			}
			else if (group.size() > 1)
				hOff-=d;

			////////////////////////////////////////////
			// Column 3 - Top pin
			hOff-=d;
			p = model->getPin(m->getLX()+2, m->getTY());
			if (i == 0)
			{
				// Connect to edge
				//wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, -1, -1, 0, hOff));
				wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, 0, hOff, p->wgX+d, hOff));

				// Draw main vertical line to bottom pin
				p2 = model->getPin(mBot->getLX()+2, mBot->getTY());
				wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, p->wgX+d, hOff, p->wgX+d, p2->wgY));
			}
			// Draw lip to right to connect to main vertical line
			wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, p->wgX, p->wgY, p->wgX+d, p->wgY));

			////////////////////////////////////////////
			// Column 4 - Bottom Pin
			hOff-=d;
			p = model->getPin(m->getLX()+3, m->getBY());
			if (i == 0)
			{
				// Connect to edge
				//wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, -1, -1, 0, hOff));
				wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, 0, hOff, p->wgX-d, hOff));

				// Draw main vertical line to bottom pin
				p2 = model->getPin(mBot->getLX()+3, mBot->getBY());
				wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, p->wgX-d, hOff, p->wgX-d, p2->wgY));
			}
			// Draw lip to right to connect to main vertical line
			wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, p->wgX, p->wgY, p->wgX-d, p->wgY));

			////////////////////////////////////////////
			// Column 4 - Top Pin
			hOff-=d;
			p = model->getPin(m->getLX()+3, m->getTY());
			if (i == 0)
			{
				// Connect to edge
				//wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, -1, -1, 0, hOff));
				wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, 0, hOff, p->wgX+d, hOff));

				// Draw main vertical line to bottom pin
				p2 = model->getPin(mBot->getLX()+3, mBot->getTY());
				wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, p->wgX+d, hOff, p->wgX+d, p2->wgY));
			}
			// Draw lip to right to connect to main vertical line
			wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, p->wgX, p->wgY, p->wgX+d, p->wgY));


			////////////////////////////////////////////
			// Column 5 - Mixer I/O (Bottom) pin for top 2 mixers
			hOff-=d;
			if (i == 0)
			{
				// If there is a 2nd mixer, do it (the lower one) first
				if (group.size() > 1)
				{
					FixedModule *m2 = group.at(1);
					p = model->getPin(m2->getLX()+4, m2->getBY());

					// Connect to edge
					//wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, -1, -1, 0, hOff));
					wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, 0, hOff, p->wgX-(2*d), hOff));

					// Draw main vertical line to pin
					wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, p->wgX-(2*d), hOff, p->wgX-(2*d), p->wgY));
					// Draw lip to right to connect to vertical line
					wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, p->wgX-(2*d), p->wgY, p->wgX, p->wgY));
					hOff-=d;
				}

				// Top Mixer
				p = model->getPin(m->getLX()+4, m->getBY());

				// Connect to edge
				//wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, -1, -1, 0, hOff));
				wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, 0, hOff, p->wgX-d, hOff));

				// Draw main vertical line to pin
				wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, p->wgX-d, hOff, p->wgX-d, p->wgY));
				// Draw lip to right to connect to vertical line
				wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, p->wgX-d, p->wgY, p->wgX, p->wgY));
			}
			else if (group.size() > 1)
				hOff-=d;


			////////////////////////////////////////////
			// WIRING FROM BOTTOM UP
			////////////////////////////////////////////
			hOff = model->getPin(mBot->getLX(), mBot->getBY())->wgY;

			////////////////////////////////////////////
			// Column 3 - Bottom pin for top 2 mixers
			if (i == 0)
			{
				// If there is a 3rd mixer, do it (the upper one) first
				if (group.size() > 2)
				{
					hOff+=d;
					FixedModule *m3 = group.at(2);
					p = model->getPin(m3->getLX()+2, m3->getBY());

					// Connect to edge
					//wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, -1, -1, 0, hOff));
					wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, 0, hOff, p->wgX-(2*d), hOff));

					// Draw main vertical line to pin
					wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, p->wgX-(2*d), hOff, p->wgX-(2*d), p->wgY));
					// Draw lip to right to connect to vertical line
					wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, p->wgX-(2*d), p->wgY, p->wgX, p->wgY));
				}

				// If there is a 4th mixer, do it (the lower one) last
				if (group.size() > 3)
				{
					hOff+=d;
					FixedModule *m4 = group.at(3);
					p = model->getPin(m4->getLX()+2, m4->getBY());

					// Connect to edge
					//wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, -1, -1, 0, hOff));
					wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, 0, hOff, p->wgX-d, hOff));

					// Draw main vertical line to pin
					wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, p->wgX-d, hOff, p->wgX-d, p->wgY));
					// Draw lip to right to connect to vertical line
					wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, p->wgX-d, p->wgY, p->wgX, p->wgY));
				}
			}
			else
				for (unsigned i = 2; i < group.size(); i++)
					hOff+=d;

			////////////////////////////////////////////
			// Column 5 - Mixer I/O (Bottom) pin for bottom 2 mixers
			if (i == 0)
			{
				// If there is a 3rd mixer, do it (the upper one) first
				if (group.size() > 2)
				{
					hOff+=d;
					FixedModule *m3 = group.at(2);
					p = model->getPin(m3->getLX()+4, m3->getBY());

					// Connect to edge
					//wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, -1, -1, 0, hOff));
					wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, 0, hOff, p->wgX-(2*d), hOff));

					// Draw main vertical line to pin
					wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, p->wgX-(2*d), hOff, p->wgX-(2*d), p->wgY));
					// Draw lip to right to connect to vertical line
					wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, p->wgX-(2*d), p->wgY, p->wgX, p->wgY));
				}

				// If there is a 4th mixer, do it (the lower one) last
				if (group.size() > 3)
				{
					hOff+=d;
					FixedModule *m4 = group.at(3);
					p = model->getPin(m4->getLX()+4, m4->getBY());

					// Connect to edge
					//wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, -1, -1, 0, hOff));
					wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, 0, hOff, p->wgX-d, hOff));

					// Draw main vertical line to pin
					wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, p->wgX-d, hOff, p->wgX-d, p->wgY));
					// Draw lip to right to connect to vertical line
					wires->at(p->originalPinNum)->push_back(new WireSegment(p->originalPinNum, 0, p->wgX-d, p->wgY, p->wgX, p->wgY));
				}
			}
			else
				for (unsigned i = 2; i < group.size(); i++)
					hOff+=d;
		}
	}

}

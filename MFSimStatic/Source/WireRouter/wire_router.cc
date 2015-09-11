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
 * Source: wire_router.cc														*
 * Original Code Author(s): Dan Grissom											*
 * Original Completion/Release Date: April 1, 2013								*
 *																				*
 * Details: N/A																	*
 *																				*
 * Revision History:															*
 * WHO		WHEN		WHAT													*
 * ---		----		----													*
 * FML		MM/DD/YY	One-line description									*
 *-----------------------------------------------------------------------------*/
#include "../../Headers/WireRouter/wire_router.h"
#include <iostream>

///////////////////////////////////////////////////////////////////////////////////
// Constructors
///////////////////////////////////////////////////////////////////////////////////
WireRouter::WireRouter()
{
	wireRoutesPerPin = new vector<vector<WireSegment *> *>();
	//pinCoords = new vector<vector<Coord> *>();
	numHorizTracks = 0;
	numVertTracks = 0;
	type = NONE_WR;
	hasExecutableSyntesisMethod = true;
	model = NULL;
}
WireRouter::WireRouter(DmfbArch *dmfbArch)
{
	arch = dmfbArch;
	wireRoutesPerPin = new vector<vector<WireSegment *> *>();
	//pinCoords = new vector<vector<Coord> *>();
	numHorizTracks = 0;
	numVertTracks = 0;
	type = NONE_WR;
	hasExecutableSyntesisMethod = true;
	model = NULL;
}

///////////////////////////////////////////////////////////////////////////////////
// Set number of tracks and then create the wire-routing model
///////////////////////////////////////////////////////////////////////////////////
void WireRouter::setNumTracksAndCreateModel(int horiz, int vert)
{
	numHorizTracks = horiz;
	numVertTracks = vert;

	claim(numHorizTracks > 1, "Number of horizontal wire routing tracks must be greater than 1.");
	claim(numVertTracks > 1, "Number of vertical wire routing tracks must be greater than 1.");

	// Create model from architecture and output graph for debugging purposes
	model = new DiagonalWireRoutingModel(arch);
	//model->OutputGraphFile("Output/WireFlowGraph",false);
}

///////////////////////////////////////////////////////////////////////////////////
// Deconstructor
///////////////////////////////////////////////////////////////////////////////////
WireRouter::~WireRouter()
{
	while (wireRoutesPerPin->size() > 0)
	{
		vector<WireSegment *> *pinWires = wireRoutesPerPin->back();
		wireRoutesPerPin->pop_back();
		while (pinWires->size() > 0)
		{
			WireSegment *ws = pinWires->back();
			pinWires->pop_back();
			delete ws;
		}
		delete pinWires;
	}
	delete wireRoutesPerPin;

	if (model)
		delete model;

	/*if (pinCoords)
	{
		while (pinCoords->size() > 0)
		{
			vector<Coord> *coords = pinCoords->back();
			pinCoords->pop_back();
			coords->clear();
			delete coords;
		}
		delete pinCoords;
	}*/
}

///////////////////////////////////////////////////////////////////////////////////
// Given the pin-mapping (contained in the DmfbArch), computes (re-organizes) the
// electrode coordinates to which each pin must connect.
///////////////////////////////////////////////////////////////////////////////////
/*void WireRouter::computePinCoords()
{
	vector<vector<int> *> *pinMap = arch->getPinMapper()->getPinMapping();

	// Populate pinCoords so each index is the pin number and the item at the index
	// is a vector or coordinates corresponding to that pin number.
	for (int x = 0; x < arch->getNumCellsX(); x++)
	{
		for (int y = 0; y < arch->getNumCellsY(); y++)
		{
			Coord c;
			c.x = x;
			c.y = y;
			int pinNo = pinMap->at(x)->at(y);

			if (pinNo >= 0)
			{
				// Create the necessary arrays, if haven't yet
				for (int i = pinCoords->size(); i <= pinNo; i++)
					pinCoords->push_back(new vector<Coord>());

				pinCoords->at(pinNo)->push_back(c);
			}
		}
	}

}*/

///////////////////////////////////////////////////////////////////////////////////
// Given the pin-mapping (contained in the DmfbArch), computes the wire routing
// to connect all the pins together and to the output of the device.  Also, has
// access to the pin activations list in case this algorithm needs to modify the
// pin-mapping itself.
///////////////////////////////////////////////////////////////////////////////////
void WireRouter::computeWireRoutes(vector<vector<int> *> *pinActivations, bool isIterative)
{
	//computePinCoords();
	//cout << "No wire-router implemented; no wire routes generated." << endl;
	claim(false, "No valid wire-router was selected for the synthesis process or no method for 'computeWireRoutes()' was implemented for the selected wire router.\n");

}

///////////////////////////////////////////////////////////////////////////////////
// This method returns layers of containing wire networks (Paths). This header is
// here to support templating in classes such as combined_wire_rotuer.h.
///////////////////////////////////////////////////////////////////////////////////
vector<vector<Path*> >* WireRouter::getLayers()
{
	claim(false, "No valid wire-router was selected for the synthesis process or no method for 'getLayers()' was implemented for the selected wire router.\n");
}

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
/*------------------------------Algorithm Details-------------------------------*
 * Type: Wire Router															*
 * Name: General Base Wire Router												*
 *																				*
 * Not inferred or detailed in any publications									*
 * 																				*
 * Details: This is a base wire-router class which contains some basic members	*
 * and functions useful to all wire-routers.  It must be inherited from when	*
 * creating a new wire-router in order to keep the structure of the simulator	*
 * and to keep the code clean and modular.										*
 *-----------------------------------------------------------------------------*/
#ifndef WIRE_ROTUER_H_
#define WIRE_ROTUER_H_

#include "../Models/diagonal_wire_routing_model.h"
#include "../Models/dmfb_arch.h"
#include "../Resources/structs.h"
#include "wire_segment.h"
#include <vector>

using std::vector;

class DiagonalWireRoutingModel;
class DmfbArch;

class WireRouter
{
	public:
		// Constructors
		WireRouter();
		WireRouter(DmfbArch *dmfbArch);
		virtual ~WireRouter();

		// Getters/Setters
		vector<vector<WireSegment *> *> *getWireRoutesPerPin() { return wireRoutesPerPin; }
		void setNumTracksAndCreateModel(int horiz, int vert);
		DiagonalWireRoutingModel* getModel() {return model;}
		int getNumHorizTracks() { return numHorizTracks; }
		int getNumVertTracks() { return numVertTracks; }
		void setArch(DmfbArch *da) { arch = da; }
		WireRouteType getType() { return type; }
		bool hasExecutableSynthMethod() { return hasExecutableSyntesisMethod; }
		void setHasExecutableSynthMethod(bool hasMethod) { hasExecutableSyntesisMethod = hasMethod; }

		// Methods
		virtual void computeWireRoutes(vector<vector<int> *> *pinActivations);

	protected:
		// Members
		DmfbArch *arch;
		DiagonalWireRoutingModel *model;
		vector<vector<WireSegment *> *> *wireRoutesPerPin; 	//wireRoutesPerPin->at(i) contains vector with all wire segments for pin i
		//vector<vector<Coord> *> *pinCoords; // List of coordinates at each pin (array index)
		bool hasExecutableSyntesisMethod; // Tells if method contains code to execute, or if it is just a shell

		// Methods
		void computePinCoords();
		WireRouteType type;
		int numHorizTracks;
		int numVertTracks;

};
#endif /* WIRE_ROUTING_H_ */

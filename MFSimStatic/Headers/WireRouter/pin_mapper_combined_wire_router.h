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
 * Name: Combined Wire Router													*
 * 																				*
 * Detailed in the following paper:												*
 * Authors: 																	*
 * Title: 																		*
 * Publication Details: 														*
 * 																				*
 * Details: 																	*
 *-----------------------------------------------------------------------------*/

#ifndef COMBINEDWIREROUTER_H_
#define COMBINEDWIREROUTER_H_

#include "path_finder_wire_router.h"
#include "../Models/dmfb_arch.h"



template<class WireRouterType>
class CombinedWireRouter :  public WireRouterType
{
	public:
		CombinedWireRouter(DmfbArch *dmfbArch) : WireRouterType(dmfbArch) {}
		virtual ~CombinedWireRouter();
		//CombinedWireRouter(DmfbArch *dmfbArch) : WDmfbArch *dmfbArch);
		//virtual void computeWireRoutesTest();
		virtual void clearLayers();
		virtual void resetWireRoutesPerPin();
		virtual void setNumTracks(int horiz, int vert);
		virtual void recreateModel();
};
template<class WireRouterType>
CombinedWireRouter<WireRouterType>::~CombinedWireRouter()
{
}

/*
template<class WireRouterType >
void CombinedWireRouter<WireRouterType>::computeWireRoutesTest()
{
	//cout << "Beginning wire routing phase:" << endl;
	//layers.resize(0);
	WireRouterType::layeredPathfinder(WireRouterType::model);

	//cout << "layer size = " << layers.at(0).size() << endl << flush;

	WireRouterType::maxPinNum = 0;
	map<int, vector<WireRouteNode *> *>::iterator groupIt;
	for (groupIt = WireRouterType::model->getPinGroups()->begin();groupIt != WireRouterType::model->getPinGroups()->end();groupIt++)
		if (WireRouterType::maxPinNum == 0 || groupIt->first > WireRouterType::maxPinNum)
			WireRouterType::maxPinNum = groupIt->first;
	WireRouterType::maxPinNum++;

	vector< vector<WireSegment *> *> *wires = WireRouterType::arch->getWireRouter()->getWireRoutesPerPin();
	for (int i = 0; i < WireRouterType::maxPinNum; i++)
	{
		vector<WireSegment *> *wire = new vector<WireSegment *>();
		wires->push_back(wire);
	}

	////////////////////////////////////////////////
	// Convert results to the actual wire segments//
	////////////////////////////////////////////////
	for(int i = 0; i < layers.size(); ++i)
	{
		for(int j = 0; j < layers[i].size(); ++j)
		{
			layers[i][j].printPath();
		}
	}
	//convertWireSegments(&layers,wires);
	//layers.clear();
}
 */




template<class WireRouterType>
void CombinedWireRouter<WireRouterType>::resetWireRoutesPerPin()
{
	while (WireRouterType::wireRoutesPerPin->size() > 0)
	{
		vector<WireSegment *> *pinWires = WireRouterType::wireRoutesPerPin->back();
		WireRouterType::wireRoutesPerPin->pop_back();
		while (pinWires->size() > 0)
		{
			WireSegment *ws = pinWires->back();
			pinWires->pop_back();
			delete ws;
		}
		delete pinWires;
	}
}
template<class WireRouterType>
void CombinedWireRouter<WireRouterType>::clearLayers()
{
	for(int i = 0; i < WireRouterType::getLayers()->size(); ++i)
	{
		for(int j = 0; j < WireRouterType::getLayers()->at(i).size(); ++j)
		{
			delete WireRouterType::getLayers()->at(i).at(j);
			//WireRouterType::getLayers().at(i).at(j) = NULL;
		}
		WireRouterType::getLayers()->at(i).clear();
	}
	WireRouterType::getLayers()->clear();
}
template<class WireRouterType>
void CombinedWireRouter<WireRouterType>::setNumTracks(int horiz, int vert)
{
	WireRouterType::numHorizTracks = horiz;
	WireRouterType::numVertTracks = vert;

	claim(WireRouterType::numHorizTracks > 1, "Number of horizontal wire routing tracks must be greater than 1.");
	claim(WireRouterType::numVertTracks > 1, "Number of vertical wire routing tracks must be greater than 1.");

}
template<class WireRouterType>
void CombinedWireRouter<WireRouterType>::recreateModel()
{
	delete WireRouterType::model;
	WireRouterType::model = new DiagonalWireRoutingModel(WireRouterType::arch);
}

#endif /* COMBINEDWIREROUTER_H_ */

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
 * Type: Pin Mapper																*
 * Name: Combined Pin Mapper Wire Router										*
 * 																				*
 * Details: This pin mapper serves as an abstract base class for pin mappers	*
 * that use a min cost max flow graph to generate a pin mapping	and also		*
 * perform wire routing															*
 * 																				*
 * Note:																		*
 * The following must be implemented in a derived class							*
 * constructMCMF() -> determines how the min cost max flow graph should be		*
 * constructed (see switchingAwarePMWR for sample) 								*
 * addEdgeBasedOnConstraints() -> determines how an edge should be added to a	*
 * compatibility graph (see switchingawarePMWR for sample)						*
 */

#ifndef COMBINED_PIN_MAPPER_WIRE_ROUTER_H_
#define COMBINED_PIN_MAPPER_WIRE_ROUTER_H_

#include "../WireRouter/pin_mapper_combined_wire_router.h"
#include "flow_network_pin_mapper.h"
#include <vector>
#include <string>
#include <map>
#include <sstream>
#include <iostream>

using std::vector;
using std::string;
using std::map;


template<class WireRouterType>
class CombinedPinMapper : public FlowNetworkPinMapper
{
protected:
	//Wire Router
	CombinedWireRouter<WireRouterType>* WireRtr;
	vector<string> unaddressedElectrodes;

	//Holds boundary information for a rectangle (used in bounding box calculations)
	struct Rectangle
	{
		int left, right, top, bottom;
	};

	// Holds network flow information about a particular edge of a mcmf graph
	struct electrodeFlow
	{
		string e;
		int net;
		long long cost;
	};

	//Comparison function for sorting
	struct eFlowsComp
	{
		bool operator() (const electrodeFlow & ef1, const electrodeFlow & ef2 )
		{
			return ef1.cost > ef2.cost;
		}
	};

	virtual MinCostMaxFlowGraph constructMCMF() = 0;
	virtual void addEdgeBasedOnConstraints(CliquePinMapper::Graph & g, long long seq1index, long long seq2index, string s1, string s2, long long size) = 0;
	int calculateClosestElectrode(int e, const vector<string> & p);
	double calcHPWL(int e1, int e2);
	double calcHPWL2(int e1, int e2);
	double  distance(int x1, int y1, int x2, int y2 );
	bool rectangleIntersect(const Rectangle & a, const Rectangle & b) {
		return (a.left <= b.right &&
	          b.left <= a.right &&
	          a.top <= b.bottom &&
	          b.top <= a.bottom); }
	Rectangle getBoundingBox(Path* p);
	double calcIntersects(int e1, int e2);

public:
	virtual void processAndAddressElectrodes();
	virtual void routeCurrentElectrodes();
	virtual void resetAllContainers();
};


//////////////////////////////////////////////////////////////////////////////////////////////////
// Calculates the half perimeter wire length (extension) used as a constraint by the pin mapper //
//////////////////////////////////////////////////////////////////////////////////////////////////
template< class WireRouterType>
double CombinedPinMapper<WireRouterType>::calcHPWL(int e1, int e2)
{
	int e1Row = e1 / arch ->getNumCellsX();
	int e1Col = e1 % arch ->getNumCellsX();
	int e2Row = e2 / arch ->getNumCellsX();
	int e2Col = e2 % arch ->getNumCellsX();

	if(e1Row > e2Row)
	{
		if(e1Col > e2Col)
		{
			return ((e1Row - e2Row) + (e1Col - e2Col));
		}
		else if(e1Col < e2Col)
		{
			return ((e1Row - e2Row) + (e2Col - e1Col));
		}
		else
		{
			return (e1Row - e2Row);
		}
	}
	else if(e1Row < e2Row)
	{
		if(e1Col > e2Col)
		{
			return ((e2Row - e1Row) + (e1Col - e2Col));
		}
		else if(e1Col < e2Col)
		{
			return ((e2Row - e1Row) * (e2Col - e1Col));
		}
		else
		{
			return (e2Row - e1Row);
		}
	}
	else
	{
		if(e1Col > e2Col)
		{
			return (e1Col - e2Col);
		}
		else
		{
			return (e2Col - e1Col);
		}
	}
}

//////////////////////////////////
//TODO(ZZ)
/////////////////////////////////
template< class WireRouterType>
double CombinedPinMapper<WireRouterType>::calcIntersects(int e1, int e2)
{
	double result = 0;
	int e1Row = e1 / arch ->getNumCellsX();
	int e1Col = e1 % arch ->getNumCellsX();
	int e2Row = e2 / arch ->getNumCellsX();
	int e2Col = e2 % arch ->getNumCellsX();

	Rectangle box;
	WireRouteNode * e1Node = WireRtr->getModel()->getPin(e1Col, e1Row);
	WireRouteNode * e2Node = WireRtr->getModel()->getPin(e2Col, e2Row);
	int x1 = e1Node ->wgX;
	int y1 = e1Node ->wgY;
	int x2 = e2Node ->wgX;
	int y2 = e2Node ->wgY;
	if(x1 < x2)
	{
		box.left = x1;
		box.right = x2;
	}
	else
	{
		box.left = x2;
		box.right = x1;
	}
	if(y1 < y2)
	{
		box.top = y1;
		box.bottom = y2;
	}
	else
	{
		box.top = y2;
		box.bottom = y1;
	}
	cout << "Howdy" << endl;
	vector<double> results(WireRtr->getLayers()->size(), 0 );
	//cout << "Howdy2" << endl;
	for(int i = 0; i < WireRtr->getLayers()->size(); ++i)
	{
		//cout << "Howdy3" << endl;
		for(int j = 0; j < WireRtr->getLayers()->at(i).size(); ++j)
		{
			if(WireRtr->getLayers()->at(i).at(j)->pathSize() == 0)
				continue;
			if(rectangleIntersect(box, getBoundingBox(WireRtr->getLayers()->at(i).at(j))))
			{
				++results[i];
			}
		}
	}
	//cout << "Howdy4" << endl;
	double total = 0;
	for(int i = 0; i < results.size(); ++i)
	{
		total += results[i];
	}
	//TODO: box.top may equal box.bottom here... same with box.left and box.right
	return total / results.size();
}
///////////////////////////////////////////////////////////////////////////////////////
// Uses distance formula to find closest electrode in vector p to a given electrode e
///////////////////////////////////////////////////////////////////////////////////////
template< class WireRouterType>
int CombinedPinMapper<WireRouterType>::calculateClosestElectrode(int e, const vector<string> & p)
{
		int e1Row = e / arch ->getNumCellsX();
		int e1Col = e % arch ->getNumCellsX();
		double minDist =  arch ->getNumCellsX() *  arch ->getNumCellsX() +  arch ->getNumCellsY() * arch ->getNumCellsY();
		int minIdx = -1;
		for(unsigned i = 0; i < p.size(); ++i)
		{
			int index = atoi(p[i].c_str());
			int e2Row = index / arch->getNumCellsX();
			int e2Col = index % arch ->getNumCellsX();
			double d = distance(e1Row, e1Col, e2Row, e2Col);
			if(d < minDist)
			{
				minDist = d;
				minIdx = index;
			}
		}
		return minIdx;
}


/////////////////////////////////////////////////////////
// Implementation of distance formula
// We can implement a shortcut here if necessarry
//////////////////////////////////////////////////////////
template< class WireRouterType>
double  CombinedPinMapper<WireRouterType>::distance(int x1, int y1, int x2, int y2 )
{
	return sqrt( pow(x2 - x1, 2) + pow(y2 - y1, 2));
}



////////////////////////////////////////////////////////
// TODO(ZZ)
////////////////////////////////////////////////////////
template< class WireRouterType>
typename CombinedPinMapper<WireRouterType>::Rectangle CombinedPinMapper<WireRouterType>::getBoundingBox(Path* p)
{
	Rectangle R;

	if(p->pathSize() == 0)
	{
		//cout << "Hello0" << endl;
		R.top = -1;
		R.bottom = -1;
		R.left = -1;
		R.right = -1;
		return R;
	}
	else if( p->pathSize() == 1)
	{
		//cout << "Hello" << endl;
		R.top = p->nodeAt(0)->wgY;
		R.bottom = R.top;
		R.left = p->nodeAt(0)->wgX;
		R.right = R.left;
		//cout << "Hello2" << endl;
		return R;
	}
	//cout << "Hello3" << endl;
	int Y1 = p->nodeAt(0)->wgY;
	int Y2 = p->nodeAt(1)->wgY;
	int X1 = p->nodeAt(0)->wgX;
	int X2 = p->nodeAt(1)->wgX;
	//cout << "Hello4" << endl;
	if(Y1 < Y2)
	{
		R.top = Y1;
		R.bottom = Y2;
	}
	else
	{
		R.top = Y2;
		R.bottom = Y1;
	}
	if(X1 < X2)
	{
		R.left = X1;
		R.right = X2;
	}
	else
	{
		R.left = X2;
		R.right = X1;
	}

	if(p->pathSize() == 2)
	{
		//cout << "Hello5" << endl;
		return R;
	}


	//cout << "Hello6" << endl;
	for(int i = 2; i < p->pathSize(); ++i )
	{
		//cout << i << " " << p->pathSize() << endl;
		//cout << p->nodeAt(i) << endl;
		int X = p->nodeAt(i)->wgX;
		int Y = p->nodeAt(i)->wgY;
		//cout << "Hello8" << endl;
		if(X < R.left)
			R.left = X;
		else if(X > R.right)
			R.right = X;
		if(Y < R.top)
			R.top = Y;
		else if(Y > R.bottom)
			R.bottom = Y;
	}

	return R;
}

//////////////////////////////////////////////////////////////////////////////
// This method addresses electrodes based on constraints					//
//////////////////////////////////////////////////////////////////////////////
template< class WireRouterType >
void CombinedPinMapper<WireRouterType>::processAndAddressElectrodes()
{
	unsigned N = currentPins.size();
	unsigned num_nodes = unaddressedElectrodes.size() + N + 2;
	MinCostMaxFlowGraph mcmf(constructMCMF());

	// Calculate flow network result
	pair<long long, long long> result = mcmf.GetMaxFlow(0,1);

	// If there is no flow, but still unaddressed electrodes, add a new pin and assign an
	// arbitrary electrode to that pin.
	if(result.first <= 0 && !unaddressedElectrodes.empty())
	{
		for(unsigned i = 0; i < unaddressedElectrodes.size(); ++i)// = unaddressedElectrodes.begin(); it != unaddressedElectrodes.end(); ++it)
		{
				//Pick an electrode to assign to a new pin
				currentPins.push_back(vector<string>());
				currentPins[currentPins.size() - 1].push_back(*unaddressedElectrodes.begin() );
				int index = atoi(unaddressedElectrodes.begin() -> c_str());
				vector<int> seq(tAMatrix[index]);
				PinActivationSeqs.push_back(seq);
				addressedElectrodes.insert(make_pair(*unaddressedElectrodes.begin(), currentPins.size() - 1));
				unaddressedElectrodes.erase(unaddressedElectrodes.begin());
				break;
		}
		return;
	}


	//vector<pair<string, pair<int, long long> > > eFlows;
	vector<electrodeFlow> eFlows;

	// Populate eFlows with flow information
	for(unsigned i = N + 2; i < num_nodes; ++i)
	{
		//For each current pin
		for(unsigned j = 2; j < N + 2; ++j)
		{
			// If there is flow from the electrode to the pin
			if(mcmf.getFlow(i, j) > 0)
			{

				//string idx = unaddressedElectrodes[i - N - 2];
				electrodeFlow eFlow;
				eFlow.e = unaddressedElectrodes[i - N - 2];
				eFlow.net = j - 2;
				eFlow.cost = mcmf.getCost(i,j);
				//eFlows.push_back(make_pair(idx, make_pair(j - 2,  mcmf.getCost(i,j))));
				eFlows.push_back(eFlow);

			}
		}
	}

	// sort eflows in ascending order of cost
	sort(eFlows.begin(), eFlows.end(), eFlowsComp());

	for(unsigned i = 0; i < eFlows.size(); ++i)
	{
		//Test wire and escape routing
		//if(checkAgainstMasterRoute(atoi(eFlows[i].first.c_str()), eFlows[i].second.first, false, false))
		//{

			int idx = atoi(eFlows[i].e.c_str());
			// Get the electrode's activation sequence
			vector<int> seq(tAMatrix[idx]);
			// Merge the activation sequence of the pin and the electrode
			// This is the new pin activation sequence after addressing it
			// with the new electrode
			PinActivationSeqs[eFlows[i].net] = seqCompare(seq,PinActivationSeqs[eFlows[i].net]);
			// Add the electrode to the list of addressed electrodes
			addressedElectrodes.insert(make_pair(eFlows[i].e, eFlows[i].net));
			// Update the pin information about the addition of the new electrode
			currentPins[eFlows[i].net].push_back(eFlows[i].e);
			// This electrode has been addressed, so erase it from the unaddressed list
			unaddressedElectrodes.erase(find(unaddressedElectrodes.begin(), unaddressedElectrodes.end(), eFlows[i].e));

			//Update the master route
			//recalculateMasterRoute();
			//Alternate
			routeCurrentElectrodes();
		//}
	}


}

////////////////////////////////////////////////////////////////////
// Performs wire routing according to the current pin mapping
////////////////////////////////////////////////////////////////////
template< class WireRouterType >
void CombinedPinMapper<WireRouterType>::routeCurrentElectrodes()
{

		//WireRtr->clearLayers();
		WireRtr->resetWireRoutesPerPin();
		setPinMapping();
		WireRtr->recreateModel();
		WireRtr ->computeWireRoutes(NULL, true);
		//WireRtr->clearLayers();
}

///////////////////////////////////////////////////////////////
// Clears out data in all major containers used by Pin Mapper
////////////////////////////////////////////////////////////////
template< class WireRouterType >
void CombinedPinMapper<WireRouterType>::resetAllContainers()
{
	addressedElectrodes.clear();
	currentPins.clear();
	unaddressedElectrodes.clear();
	PinActivationSeqs.clear();
	cliques.clear();
	WireRtr->resetWireRoutesPerPin();
	WireRtr->clearLayers();
	//WireRtr->recreateModel();
	trivialElectrodes.clear();
}


#endif /* COMBINED_PIN_MAPPER_WIRE_ROUTER_H_ */

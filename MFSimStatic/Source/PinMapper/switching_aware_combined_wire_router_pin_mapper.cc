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
 * Source: switching_aware_combined_wire_router_pin_mapper.cc												*
 * Original Code Author(s): Zach Zimmerman										*
 * Original Completion/Release Date: 10/20/2014									*
 *																				*
 * Details: 																	*
 * Revision History:															*
 * WHO		WHEN		WHAT													*
 * ---		----		----													*
 * FML		MM/DD/YY	One-line description									*
 *-----------------------------------------------------------------------------*/

#include "../../Headers/Util/file_out.h"
#include "../../Headers/Util/file_in.h"
#include "../../Headers/PinMapper/switching_aware_combined_wire_router_pin_mapper.h"
#include "../../Headers/Models/min_cost_max_flow_graph.h"
#include "../../Headers/WireRouter/pin_mapper_combined_wire_router.h"
#include "../../Headers/WireRouter/path_finder_wire_router.h"
#include<string>
#include<set>
#include<algorithm>
#include<limits>


///////////////////////////////////////////////
//Constructors/Destructor
///////////////////////////////////////////////
SwitchingAwarePMWR::SwitchingAwarePMWR()
{
	arch = NULL;
	isCombinedWireRotuer = true;
}
SwitchingAwarePMWR::SwitchingAwarePMWR(DmfbArch *dmfbArch)
{
	arch = dmfbArch;

	// Get the original wire-route type (from user) for compatibility checking
	//WireRouteType originalWrt = dmfbArch->getWireRouter()->getType();

	// *** Both of these work, just change to match in constructor too
	//WireRtr = new CombinedWireRouter<YehWireRouter>(dmfbArch);
	WireRtr = new CombinedWireRouter<PathFinderWireRouter>(dmfbArch);

	WireRtr->setArch(dmfbArch);
	//dmfbArch->setWireRouter(WireRtr); // DTG - Arch's internal WR not used by this file so commented b/c line messes up compatibility checking
	isCombinedWireRotuer = true;
}

SwitchingAwarePMWR::~SwitchingAwarePMWR() { }

//////////////////////////////////////////////////////////////////////////////////////
// Main point of execution, calls other methods to assist in calculating result		//
//////////////////////////////////////////////////////////////////////////////////////
void SwitchingAwarePMWR::setMapPostRoute(vector<vector<int> *> *pinActivations, map<Droplet *, vector<RoutePoint *> *> *routes)
{
	// If the number of tracks were specified (via the EntireFlow, or at some other point), grab those values and
	// save them in the local WireRtr
	if (arch->getWireRouter()->getNumHorizTracks() > 0 && arch->getWireRouter()->getNumVertTracks() > 0)
		WireRtr->setNumTracksAndCreateModel(arch->getWireRouter()->getNumHorizTracks(), arch->getWireRouter()->getNumVertTracks());
	// TODO: Find a way to specify these at top level main. For now, if they are greater than 0,
	// then we are probably running Synthesis::EntireFlow(), which allows us to set them before the
	// combined pin-mapping/wire-routing stage. If they aren't set, hardcode for now b/c the framework
	// does not currently support setting the track numbers before the Synthesis::WireRoute() method is
	// called.
	else if (!(WireRtr->getNumHorizTracks() > 0 && WireRtr->getNumVertTracks() > 0))
	{
		cerr << "WARNING: SwitchingAware Pin-mapper Wire-Router hardcoded to 3 horizontal/vertical wire-routing tracks because there is currently no framework support to programmatically set the number of tracks for a combined pin-mapper/wire-router. Continuing..." << endl;
		const int _horiz = 3, _vert = 3; // ZZ Hardcode
		WireRtr->setNumTracksAndCreateModel(_horiz, _vert); // ZZ Hardcode
	}

	numGVatIndex = vector<int>( pinActivations -> size(), 0);
	//P_MAX is the maximum allowed pins, our pin constraint is currently picked arbitrarily
	const int P_MAX = 285;
	pre_num_activations = 0;
	post_num_activations = 0;
	if (debugPrint()) cout << "[SwitchingAwarePMWR] Initializing Matrix and calculating interference..." << endl;
	vector< vector<int> > AMatrix = initActivationMatrix(pinActivations);
	int pinsBefore = arch->getPinMapper()->getNumUniquePins();
	int elecsBefore = arch->getPinMapper()->getNumElectrodes();

	// transpose the matrix for much easier access to electrode activation seqs
	initTranspose(AMatrix);

	//Set up value for RAU calculation at end
	for(unsigned i = 0; i < AMatrix.size(); ++i)
	{
		for(unsigned j = 0; j < AMatrix[i].size(); ++j)
		{
			if(AMatrix[i][j] == 1)
				pre_num_activations++;
		}
	}

	// Calculate the lower bound on switching, this is the minimum number of times
	// we could switch given perfect circumstances
	int switchingLowerBound = calculateSwitchingLowerBound();

	precomputeAllSwitches();

	//Upper bound of switching is the number of cycles in the assay
	//This is generous, this upper bound will likely never be a solution
	int swtichingUpperBound = AMatrix.size();

	//Set up variables to help with binary search functionality
	int currentSTLowerLimit = switchingLowerBound;
	int currentSTUpperLimit = swtichingUpperBound;

	// set our maximum allowed switched to this lower bound
	STMax = (currentSTUpperLimit + currentSTLowerLimit) / 2;
	int lastSucceededSTMax = -1;
	int iterations = 0;
	bool satisfied = false;
	bool binarySearchDone = false;
	int runResult;
	// Keep going until we meet constraint
	while(!satisfied)
	{
		runResult = fullRun(AMatrix, P_MAX, false);
		if(runResult == -1)
		{
			cerr << "[SwitchingAwarePMWR] Error: aborting..." << endl;
			return;
		}
		if((runResult == 2 || runResult == 0) && (currentPins.size() <= P_MAX))
		{
			//Set up next iteration of binary search, or stop if end is reached
			int newSTMax = (currentSTLowerLimit + STMax) / 2;
			currentSTUpperLimit = STMax;
			if(STMax == newSTMax)
			{
				binarySearchDone = true;
			}
			lastSucceededSTMax = STMax;
			STMax = newSTMax;
		}

		if(runResult == 2 && !binarySearchDone)
		{
			if (debugPrint()) cout << "[SwitchingAwarePMWR] Can make route in worst case with constraint = " << lastSucceededSTMax << endl;
			if (debugPrint()) cout << "[SwitchingAwarePMWR] Tightening constraint to " << STMax << " and restarting..." << endl;
		}
		else if(runResult != 1 && binarySearchDone)
		{
			satisfied = true;
		}
		else if (runResult != 1 && currentPins.size() <= P_MAX)
		{
			if (debugPrint()) cout << "[SwitchingAwarePMWR] Found a solution with STMax = " << lastSucceededSTMax << ", but we can still attempt to tighten constraint..." << endl;
			if (debugPrint()) cout << "[SwitchingAwarePMWR] Tightening constraint to " << STMax << " and restarting..." << endl;
		}
		else
		{
			if(STMax >= AMatrix.size())
			{
				if (debugPrint()) cerr << "[SwitchingAwarePMWR] Error: Failed to find a viable mapping after reaching upper bound, try lowering pin constraint... Aborting" << endl;
				break;
			}
			if (debugPrint()) cout << "[SwitchingAwarePMWR] Did not meet constraint with STMax = " << STMax << endl;


			//Loosen upper bound
			int newSTMax = (currentSTUpperLimit + STMax) / 2;
			currentSTLowerLimit = STMax;
			if(STMax == newSTMax)
			{
				binarySearchDone = true;
				if(lastSucceededSTMax == -1)
				{
					// We don't have any successes to go back to, we fail to map.
					if (debugPrint()) cout << "[SwitchingAwarePMWR] Unable to find solution with P_MAX = " << P_MAX << endl;
					break;
				}
				else
				{
					//We failed here, but we have a previous success.
					//cout << "Last succeeded: " << lastSucceededSTMax << endl;
					if (debugPrint()) cout << "[SwitchingAwarePMWR] setting upper bound to last success value, " << lastSucceededSTMax << " and restarting..." << endl;
					STMax = lastSucceededSTMax;
				}
			}
			else
			{
				if (debugPrint()) cout << "[SwitchingAwarePMWR] loosening upper bound to " << newSTMax << " and restarting..." << endl;
				STMax = newSTMax;
			}
		}
	}

	if(!satisfied)
	{
		return;
	}

	//do final pin mapping and wire routing based on constraint found previously
	//In some cases the final run may not be needed ( (EX) edge case where the return is zero for the most recent call to fullRun() )

	fullRun(AMatrix, P_MAX, true);
	WireRtr->clearLayers();
	WireRtr->resetWireRoutesPerPin();
	setPinMapping();
	WireRtr->recreateModel();
	WireRtr->computeWireRoutes(pinActivations, false);
	setPinActivations(pinActivations);

	if (debugPrint()) cout << "Entire electrode array size ( num electrodes * num activations) = " << (long long) addressedElectrodes.size() * (long long) pinActivations -> size() << endl;
	if (debugPrint()) cout << "[SwitchingAwarePMWR] Addressed and routed " << currentPins.size() << " pins." << endl;

	cout << "[SwitchingAwarePMWR] Total redundant activations: " << post_num_activations - pre_num_activations << endl;
	cout << "[SwitchingAwarePMWR] Total initial activations: " << pre_num_activations << endl;
	cout << "[SwitchingAwarePMWR] Total post activations: " << post_num_activations << endl;
	cout << "[SwitchingAwarePMWR] Number of electrodes reduced from " << elecsBefore << " to " << arch->getPinMapper()->getNumElectrodes() << endl;
	cout << "[SwitchingAwarePMWR] Number of pins reduced from " << pinsBefore << " to " << arch->getPinMapper()->getNumUniquePins() << endl;
	cout << "[SwitchingAwarePMWR] " << flush;
	calculateSwitchingResults();

	for(int i = 0; i < PinActivationSeqs.size(); ++i)
	{
		addGroundVectorsToEntireAssay(i);
	}
	long long numGVs = 0;
	for(int i = 0; i < numGVatIndex.size(); ++i)
	{
		numGVs += numGVatIndex.at(i);
	}
	if (debugPrint()) cout << "[SwitchingAwarePMWR] total gvs that would be inserted: " << numGVs << endl;

}
/*
/////////////////////////////////////////////////////////////////////////
// Sets the pin mapping result.
//
/////////////////////////////////////////////////////////////////////////
void SwitchingAwarePMWR::setPinMapping(const map<string,int> & addressedElectrodes, int pinNo)
{
	stringstream ss;

	int numCols = arch -> getNumCellsX();

	for(unsigned i = 0; i  < pinMapping -> size(); ++i)
	{
		for(unsigned j = 0; j < pinMapping -> at(i) -> size(); ++j)
		{
			pinMapping -> at(i) -> at(j) = -1;
		}
	}

	for(map<string,int>::const_iterator i = addressedElectrodes.begin(); i != addressedElectrodes.end(); ++i)
	{
		ss << i -> first;
		int electrode;
		ss >> electrode;
		ss.clear();
		int erow = electrode / numCols;
		int ecol = electrode % numCols;
		if(trivialElectrodes[electrode])
			pinMapping->at(ecol)->at(erow) = -1;
		else
			pinMapping->at(ecol)->at(erow) = i->second;

	}

	// Set pin numbers for the I/O ports
	for (unsigned i = 0; i < arch->getIoPorts()->size(); i++)
	{
		if (arch->getIoPorts()->at(i)->isAnInput())
			inputPins->push_back(pinNo);
		else
			outputPins->push_back(pinNo);
		arch->getIoPorts()->at(i)->setPinNo(pinNo++);
	}

	sort(inputPins->begin(), inputPins->end());
	sort(outputPins->begin(), outputPins->end());

	//computeAvailResources();

}
 */
///////////////////////////////////////////////////////////////////////////////////////
// Calculates the theoretical fewest number of switches (individually addressed pins)
///////////////////////////////////////////////////////////////////////////////////////
int SwitchingAwarePMWR::calculateSwitchingLowerBound()
{
	int LowerBound = 0;
	timesSwitched = vector<int>(tAMatrix.size());
	for(unsigned i = 0; i < tAMatrix.size(); ++i)
	{
		timesSwitched[i] = getNumSwitches(tAMatrix[i]);
		if(timesSwitched[i] > LowerBound)
		{
			LowerBound = timesSwitched[i];
		}
	}
	return LowerBound;
}
/*
/////////////////////////////////////////////////////////////////////////////////////////
// Transposes the activation matrix for easier access to electrode activation sequences
//////////////////////////////////////////////////////////////////////////////////////////
void SwitchingAwarePMWR::transposeAMatrix(const vector<vector<int> > & AMatrix)
{
	if(AMatrix.size() == 0)
		return;
	tAMatrix = vector<vector<int> >(AMatrix[0].size());
	for(unsigned i = 0; i < AMatrix.size(); ++i)
	{
		for(unsigned j = 0; j < AMatrix[i].size(); ++j)
		{
			tAMatrix[j].push_back(AMatrix[i][j]);
		}
	}
}*/
/*
/////////////////////////////////////////////////////////////////
// Gets the number of switches in an activation sequence
//////////////////////////////////////////////////////////////////
int SwitchingAwarePMWR::getNumSwitches(const vector<int> & seq)
{
	int result = 0;
	if(seq.size() == 0 || seq.size() == 1)
	{
		return 0;
	}
	int test = 0;
	while(seq[test] == 2) //TODO(ZZ):Uninitialized Access
	{
		test++;
	}
	int prevIdx = test;
	for(unsigned i = test + 1; i <  seq.size(); ++i)
	{

		if(seq[i] == 2)
		{
			continue;
		}
		if(seq[prevIdx] != seq[i])
		{
			result++;
		}
		prevIdx = i;
	}
	return result;
}
 */
/*
////////////////////////////////////////////////////////////////////
// Performs wire routing according to the current pin mapping
////////////////////////////////////////////////////////////////////
void SwitchingAwarePMWR::routeCurrentElectrodes()
{
	WireRtr->resetWireRoutesPerPin();
	setPinMapping(addressedElectrodes, currentPins.size());
	WireRtr->recreateModel();
	WireRtr ->computeWireRoutesTest();
	WireRtr->clearLayers();
}
 */
/*
///////////////////////////////////////////////////////////////////////////////////////
// Calculates the half perimiter wire length used as a constraint by the pin mapper
///////////////////////////////////////////////////////////////////////////////////////
double SwitchingAwarePMWR::calcHPWL(int e1, int e2)
{
	int e1Row = e1 / arch ->getNumCellsX();
	int e1Col = e1 % arch ->getNumCellsX();
	int e2Row = e2 / arch ->getNumCellsX();
	int e2Col = e2 % arch ->getNumCellsX();

	if(e1Row > e2Row)
	{
		if(e1Col > e2Col)
		{
			return ((e1Row - e2Row) * (e1Col - e2Col)) / 2;
		}
		else if(e1Col < e2Col)
		{
			return ((e1Row - e2Row) * (e2Col - e1Col)) / 2;
		}
		else
		{
			return (e2Row - e1Row) / 2;
		}
	}
	else if(e1Row < e2Row)
	{
		if(e1Col > e2Col)
		{
			return ((e2Row - e1Row) * (e1Col - e2Col)) / 2;
		}
		else if(e1Col < e2Col)
		{
			return ((e2Row - e1Row) * (e2Col - e1Col)) / 2;
		}
		else
		{
			return (e1Row - e2Row) / 2;
		}
	}
	else
	{
		if(e1Col > e2Col)
		{
			return (e2Col - e1Col) / 2;
		}
		else
		{
			return (e1Col - e2Col) / 2;
		}
	}
}
///////////////////////////////////////////////////////////////////////////////////////
// Uses distance formula to find closest electrode in vector p to a given electrode e
///////////////////////////////////////////////////////////////////////////////////////
int SwitchingAwarePMWR::calculateClosestElectrode(int e, const vector<string> & p)
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
//////////////////////////////////////////////////////////
double  SwitchingAwarePMWR::distance(int x1, int y1, int x2, int y2 )
{
	return sqrt( pow(x2 - x1, 2) + pow(y2 - y1, 2));
}*/

//////////////////////////////////////////////////////////////////////////////
//Precompute switches between all pairs of electrodes, this is a tremendous //
//time saver when creating the compatability graph                          //
//////////////////////////////////////////////////////////////////////////////
void SwitchingAwarePMWR::precomputeAllSwitches()
{
	allSwitches = vector<vector<int> >(tAMatrix.size(), vector<int>(tAMatrix.size()));
	for(unsigned i = 0; i < tAMatrix.size(); ++i)
	{
		for(unsigned j = i; j < tAMatrix.size(); ++j)
		{
			if(i == j)
			{
				allSwitches[i][j] = 0;
			}
			else
			{
				allSwitches[i][j] = getNumSwitches(seqCompare(tAMatrix[i], tAMatrix[j]));
				allSwitches[j][i] = allSwitches[i][j];
			}
		}
	}
}

/*
///////////////////////////////////////////////////////////////////////
// Calculates and outputs some metrics on effectiveness of algorithm
///////////////////////////////////////////////////////////////////////
void SwitchingAwarePMWR::calculateSwitchingResults()
{
	vector<int> switches;
	for(unsigned i = 0; i < PinActivationSeqs.size(); ++i)
	{
		switches.push_back(getNumSwitches(PinActivationSeqs[i]));
	}
	sort(switches.begin(),switches.end());
	long long minVal = *switches.begin();
	long long maxVal = switches[switches.size() - 1];

	double median;
	double mean;
	if(switches.size() % 2 == 0)
	{
		median = (switches[switches.size() / 2] + switches[(switches.size() / 2) + 1]) / (double) 2;
	}
	else
	{
		median = switches[(switches.size() / 2) + 1];
	}
	long long sum = 0;
	for(unsigned i = 0; i < switches.size(); ++i)
	{
		sum += switches[i];
	}
	mean = sum / (double) switches.size();


	cout << "Switching Results:" << endl;
	cout << "Max switches on one pin = " << maxVal << endl;
	cout << "Min switches on one pin = " << minVal << endl;
	cout << "Median switches = " << median << endl;
	cout << "Mean switches = " << mean << endl;
	cout << "Total Switches across all pins = " << sum << endl;
}
 */
/*
///////////////////////////////////////////////////////////////
// Clears out data in all major containers used by Pin Mapper
////////////////////////////////////////////////////////////////
void SwitchingAwarePMWR::resetAllContainers()
{
	//MasterRoute.clear();
	addressedElectrodes.clear();
	currentPins.clear();
	unaddressedElectrodes.clear();
	PinActivationSeqs.clear();
	cliques.clear();
	WireRtr->resetWireRoutesPerPin();
	WireRtr->clearLayers();
	trivialElectrodes.clear();
}
 */

///////////////////////////////////////////////////////////////////////////////
// Computes a full iteration of the algorithm and returns a status           //
// Algorithm:																 //
//		1. compute compatability graph and perform coloring                  //
//		2. Identify a maximum clique of incompatible electrodes              //
//      3. Route the electrodes in the clique                                //
//		4. Calculate min cost max flow of remaining electrodes w.r.t		 //
//		wire length and map/route electrodes with flow                       //
// Input:																	 //
//		AMatrix is the precomputed activation matrix for the assay			 //
//		P_MAX is the maximum pin count allowed							     //
//		finalRun determines weather early escapes are allowed				 //
// Return:																	 //
//		1 if computed pin count will exceed constraint if we keep going      //
//		2 if computed pin count will meet constraint if we keep going		 //
//		0 if full run was completed											 //
///////////////////////////////////////////////////////////////////////////////
int SwitchingAwarePMWR::fullRun(const vector<vector<int> >& AMatrix, int P_MAX, bool finalRun)
{
	resetAllContainers();
	stringstream SS;
	if(finalRun && debugPrint())
		cout << "[SwitchingAwarePMWR] Final Run..." << endl;
	if (debugPrint()) cout << "[SwitchingAwarePMWR] Generating compatibility graph...." << endl;
	Graph coloringGraph = convertMatrixToGraph(AMatrix.size());
	if (debugPrint()) cout << "[SwitchingAwarePMWR] Coloring.... "<< endl;
	coloringGraph.dsatur();


	if (debugPrint()) cout << "[SwitchingAwarePMWR] Converting to cliques..." << endl;
	cliques = convertToCliques(coloringGraph);


	if(cliques.empty())
	{
		cerr << "[SwitchingAwarePMWR] Error: Empty Graph... Aborting" << endl;
		return -1;
	}

	for(unsigned i = 0; i < tAMatrix.size(); ++i)
	{
		string idx;
		SS << i;
		SS >> idx;
		SS.clear();
		if(!trivialElectrodes[i])
			unaddressedElectrodes.push_back(idx);
	}

	// Initial setup of flow problem (Each electrode in the largest clique must be addressed
	// by its own pin (they are all incompatible).
	vector<string> clique = findAndDeleteMaxClique();
	for(unsigned i = 0; i < clique.size(); ++i)
	{
		int index = atoi(clique[i].c_str());
		if(trivialElectrodes[index])
		{
			continue;
		}
		currentPins.push_back(vector<string>());

		//Assign this electrode to that pin
		currentPins[currentPins.size() - 1].push_back(clique[i]);


		// Obtain this electrode's activation sequence...
		//since this electrode is the only one assigned to the pin for now
		//the pin's activation sequence is equal to this electrode's activation sequence
		PinActivationSeqs.push_back(tAMatrix[index]);


		//Add this electrode to the set of addressed electrodes, along with its corresponding pin
		addressedElectrodes.insert(make_pair(clique[i], currentPins.size() - 1));

		unaddressedElectrodes.erase(find(unaddressedElectrodes.begin(), unaddressedElectrodes.end(), clique[i])); //(clique[i]));

		//Set the produced route as the master
		//recalculateMasterRoute();
		routeCurrentElectrodes();


		//If we have exceeded our alloted pin count, there is no point in continuing with this round.
		if(currentPins.size() > P_MAX && !finalRun)
		{
			if (debugPrint()) cout << "Current Pin Count Exceeds constraint... Restarting..." << endl;
			return 1;
		}
		if(currentPins.size() + unaddressedElectrodes.size() <= P_MAX && !finalRun)
		{
			if (debugPrint()) cout << "Can make route in worst case... Tightening constraint..." << endl;
			return 2;
		}
		//}
	}

	// Continue to next electrodes until we run out
	if (debugPrint()) cout << "[SwitchingAwarePMWR] Addressing Electrodes..." << endl;
	while(!unaddressedElectrodes.empty())
	{

		processAndAddressElectrodes();

		if (debugPrint()) cout << "[SwitchingAwarePMWR] electrodes left = " << unaddressedElectrodes.size() << endl;
		if(currentPins.size() > P_MAX && !finalRun)
		{
			if (debugPrint()) cout << "[SwitchingAwarePMWR] Current Pin Count Exceeds constraint... Restarting..." << endl;
			return 1;
		}
		if(currentPins.size() + unaddressedElectrodes.size() <= P_MAX && !finalRun)
		{
			if (debugPrint()) cout << "[SwitchingAwarePMWR] RESTARTING" << endl;
			//cout << "Can make route in worst case with constraint = " << STMax << endl << "Tightening constraint and restarting..." << endl;
			//satisfiedJump = true;
			return 2;
		}

	}

	return 0;

}

/////////////////////////////////////////////////////////////////////////////////
// Constructs a min cost max flow graph with HPWL-extention as a cost function //
/////////////////////////////////////////////////////////////////////////////////
MinCostMaxFlowGraph SwitchingAwarePMWR::constructMCMF()
{
	unsigned N = currentPins.size();
	//Number of nodes in flow network is unaddressed electrodes + current number of pins
	// + source + sink
	unsigned num_nodes = unaddressedElectrodes.size() + N + 2;
	MinCostMaxFlowGraph mcmf(num_nodes);
	//Add edges to sink node (1) from every current pin with 1 unit cap and 0 cost
	for(unsigned i = 0; i < N; ++i)
	{
		mcmf.AddEdge(i + 2, 1, 1, 0);
	}
	//Add edges from source node (0) to every unaddressed electrode with 1 unit cap and 0 cost
	for(unsigned i = 0; i < unaddressedElectrodes.size(); ++i)
	{
		mcmf.AddEdge(0, N + i + 2, 1, 0);
	}
	stringstream SS;
	for(unsigned i = 0; i < unaddressedElectrodes.size(); ++i)
	{
		//bool incomp = false;
		int idx = atoi(unaddressedElectrodes[i].c_str());
		for(unsigned j = 0; j < currentPins.size(); ++j)
		{
			vector<int> result(seqCompare(PinActivationSeqs[j], tAMatrix[idx]));
			if(!result.empty())
			{
				int _e_ = calculateClosestElectrode(i, currentPins[j]);
				long long cost = /*(calcIntersects(i, _e_)  + 1) */ calcHPWL(i,_e_ );
				//cout << "cost = " << cost << endl;
				mcmf.AddEdge(N + i + 2, j + 2, 1, cost);
			}
		}
	}
	return mcmf;
}

//////////////////////////////////////////////////////////////////////////////////////
// Called by convertMatrixToGraph(), adds edges only between compatible electrodes  //
// so that when the graph is colored it produces cliques of incompatible electrodes //
//////////////////////////////////////////////////////////////////////////////////////
void SwitchingAwarePMWR::addEdgeBasedOnConstraints(CliquePinMapper::Graph & g, long long seq1index, long long seq2index, string s1, string s2, long long size)
{
	bool match = false;
	for(unsigned i = 0; i < size; ++i)
	{
		if(allSwitches[seq2index][seq1index] > STMax)
		{

			match = true;
			break;
		}
		if( (tAMatrix[seq2index][i] == 0 && tAMatrix[seq1index][i] == 1) || (tAMatrix[seq2index][i] == 1 && tAMatrix [seq1index][i] == 0))
		{

			match = true;
			break;
		}
	}
	if(match)
	{

		match = false;
	}
	else
	{
		//We need cliques that contain mutually incompatible electrodes
		//We need to add edges between COMPATIBLE electrodes, in order for
		// the coloring to produce such cliques
		g.add_edge(s1, s2);
	}

}



/*
 * stringstream SS;


		cout << "[SwitchingAwarePMWR] Generating compatibility graph...." << endl;
		Graph coloringGraph = convertMatrixToGraph(AMatrix.size());


		cout << "[SwitchingAwarePMWR] Coloring.... "<< endl;
		coloringGraph.dsatur();


		cout << "[SwitchingAwarePMWR] Converting to cliques..." << endl;
		cliques = convertToCliques(coloringGraph);


		if(cliques.empty())
		{
			cerr << "[SwitchingAwarePMWR] Error: Empty Graph... Aborting" << endl;
			return;
		}

		for(unsigned i = 0; i < tAMatrix.size(); ++i)
		{
				string idx;
				SS << i;
				SS >> idx;
				SS.clear();
				if(!trivialElectrodes[i])
					unaddressedElectrodes.push_back(idx);
		}

		// Initial setup of flow problem (Each electrode in the largest clique must be addressed
		// by its own pin (they are all incompatible).
		clique = findAndDeleteMaxClique();

		for(unsigned i = 0; i < clique.size(); ++i)
		{
			//determine the electrode number asociated with this particular entry
			// in the clique
			int index = atoi(clique[i].c_str());
			if(trivialElectrodes[index])
			{
				continue;
			}

			//Make sure these can be routed (TODO: Right now route will always succeed, we have no condition of a failed route currently)
			//if(checkAgainstMasterRoute(index, currentPins.size() - 1, true, true))
			//{


				//Setup an empty pin
				currentPins.push_back(vector<string>());

				//Assign this electrode to that pin
				currentPins[currentPins.size() - 1].push_back(clique[i]);


				// Obtain this electrode's activation sequence...
				//since this electrode is the only one assigned to the pin for now
				//the pin's activation sequence is equal to this electrode's activation sequence
				PinActivationSeqs.push_back(tAMatrix[index]);


				//Add this electrode to the set of addressed electrodes, along with its corresponding pin
				addressedElectrodes.insert(make_pair(clique[i], currentPins.size() - 1));

				unaddressedElectrodes.erase(find(unaddressedElectrodes.begin(), unaddressedElectrodes.end(), clique[i])); //(clique[i]));

				//Set the produced route as the master
				//recalculateMasterRoute();
				routeCurrentElectrodes();

				//If we have exceeded our alloted pin count, there is no point in continuing with this round.
				if(currentPins.size() > P_MAX)
				{
					cout << "Current Pin Count Exceeds constraint... Restarting..." << endl;
					goto Unsatisfied;
				}
				if(currentPins.size() + unaddressedElectrodes.size() <= P_MAX)
				{
					cout << "Can make route in worst case... Tightening constraint..." << endl;
					satisfiedJump = true;
					goto Satisfied;
				}
			//}
		}

		// Continue to next electrodes until we run out
		cout << "[SwitchingAwarePMWR] Addressing Electrodes..." << endl;
		while(!unaddressedElectrodes.empty())
		{

			processAndAddressElectrodes();
			if(currentPins.size() > P_MAX)
			{
				cout << "Current Pin Count Exceeds constraint... Restarting..." << endl;
				goto Unsatisfied;
			}
			if(currentPins.size() + unaddressedElectrodes.size() <= P_MAX)
			{
				cout << "Can make route in worst case with constraint = " << STMax << "Tightening constraint and restarting..." << endl;
				satisfiedJump = true;
				goto Satisfied;
			}
		}


		cout << "Mapped and routed DMFB with " << currentPins.size() << " pins..." << endl;
 */

///////////////////////////////////////////////////////////////////////////////////////////////
// The following code is UNFINISHED but would be used in the case that you have a fail
// condition for wire routing, it saves the result of wire routing so you can perform
// checks on the route, instead of updating instantly like the current implementation
//////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////
//
//
///////////////////////////
/*
void SwitchingAwarePMWR::recalculateMasterRoute()
{
	if(MasterRouteCheck.empty())
	{
		cerr << "Error: checkAgainstMasterRoute() was not called before recalculateMasterRoute(), aborting... error inevitable" << endl;
	}
	MasterRoute = MasterRouteCheck;
	MasterRouteCheck.clear();
}
 */
////////////////////////
//
///////////////////////
/*
bool SwitchingAwarePMWR::checkAgainstMasterRoute(const int & eNum, const int & netNum, bool newPin, bool newPinOpt)
{
	WireRtr->resetWireRoutesPerPin();
	map<string, int> tempAddressedElectrodes(addressedElectrodes);
	vector<vector<string> > tempCurrentPins(currentPins);
	stringstream SS;
	string index;
	SS << eNum;
	SS >> index;
	if(newPin)
	{
		tempCurrentPins.push_back(vector<string>());
		if(newPinOpt)
		{
			tempAddressedElectrodes.insert(make_pair(index, tempCurrentPins.size() -1));
			tempCurrentPins[tempCurrentPins.size() -1].push_back(index);
		}
		else
		{
			tempAddressedElectrodes.insert(make_pair(*unaddressedElectrodes.begin(), tempCurrentPins.size() - 1));
			tempCurrentPins[tempCurrentPins.size() -1].push_back(*unaddressedElectrodes.begin());
		}
	}
	else
	{
		tempCurrentPins[netNum].push_back(index);
		tempAddressedElectrodes.insert(make_pair(index, netNum));
	}
	setPinMapping(tempAddressedElectrodes, tempCurrentPins.size());
	//DiagonalWireRoutingModel* m = new DiagonalWireRoutingModel(arch);
	//PinMapperWireRouter* pmwr = WireRtr;

	//WireRtr = new PinMapperWireRouter();
	setupWireRouterAndComputeRoutes();
	DiagonalWireRoutingModel* m = WireRtr->getModel();
	if(WireRtr->modelExists)
		delete m;
	WireRtr->setNumTracksAndCreateModel(3,3);
	WireRtr->modelExists = true;
	WireRtr ->computeWireRoutesTest();


	MasterRouteCheck = WireRtr->layers;

	WireRtr->layers.clear();

	//delete m;
	return true;
}
 */
/*
void SwitchingAwarePMWR::setupWireRouterAndComputeRoutes()
{

	if(WireRtr->modelExists)
	{
		DiagonalWireRoutingModel* m = WireRtr->getModel();
		delete m;
		WireRtr->modelExists = false;
	}
	WireRtr->setNumTracksAndCreateModel(3,3);
	WireRtr->modelExists = true;
	WireRtr ->computeWireRoutesTest();
}
 */

/*
void SwitchingAwarePMWR::printMasterRoute(int index1, int index2)
{
	if(index1 == -1)
	{
		for(unsigned i = 0; i < MasterRoute.size(); ++i)
		{
			cout << "Layer " <<  i << "..." << endl;
			for(unsigned j = 0; j < MasterRoute[i].size(); ++j)
			{
				MasterRoute[i][j].printPath();
			}
		}

	}
	else
	{
		MasterRoute[index1][index2].printPath();
	}

}
 */

/*
void debugPaths(long long n, PinMapperWireRouter* pmwr)
{
	WireRouteNode* wrn = new WireRouteNode();
	pmwr->layers.push_back(vector<Path*>());
	for(unsigned k = 0; k < 10; ++k)
	{
		Path * p = new Path();
		for(long long i = 0; i < n; ++i )
		{
			p->addPathSegment(wrn);
		}
		pmwr->layers.at(0).push_back(p);
		cout << "Created " << pmwr->layers.at(0).back()->pathSize() << " path segments" << endl;
	}
	//cout << "num paths "pmwr->layers.size() << endl;
	pmwr->clearLayers();
	cout << "Cleared Layers" << endl;

}

 */


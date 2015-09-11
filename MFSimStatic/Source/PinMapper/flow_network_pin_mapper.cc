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
 * Source: flow_network_pin_mapper.cc											*
 * Original Code Author(s): Zach Zimmerman										*
 * Original Completion/Release Date: 10/15/2014									*
 *																				*
 * Details: The following must be implemented in a derived class				*
 * constructMCMF() -> determines how the min cost max flow graph should be		*
 * constructed (see switchingAwarePMWR for sample) 								*
 * addEdgeBasedOnConstraints() -> determines how an edge should be added to a	*
 * compatibility graph (see switchingawarePMWR for sample)						*
 * 																				*
 * Revision History:															*
 * WHO		WHEN		WHAT													*
 * ---		----		----													*
 * FML		MM/DD/YY	One-line description									*
 *-----------------------------------------------------------------------------*/






#include "../../Headers/PinMapper/flow_network_pin_mapper.h"
#include <map>
#include <set>

//////////////////////////////////////////////////////////////////////////////////////
// Sets the pin activations to match up with a new pin mapping (not individually	//
// addressed anymore)																//
//////////////////////////////////////////////////////////////////////////////////////
void FlowNetworkPinMapper::setPinActivations(vector<vector<int> *>* pinActivations)
{
	set<int> allPins;
	char * string1 = new char[10];
	//Reassign Pin Activations Based on new mapping
	for (int i = 0; i < pinActivations->size(); ++i)
	{
		set<int> activatedThisCycle;

		for (int j = 0; j < pinActivations->at(i)->size(); ++j )
		{
			if(!trivialElectrodes[pinActivations->at(i) ->at(j)])
			{
				int pin = addressedElectrodes[Util::itoa(pinActivations -> at(i) -> at(j), string1, 10)];
				activatedThisCycle.insert(pin);
				allPins.insert(pin);
			}
		}
		pinActivations->at(i)->clear(); // Clear out the old individually addressable pins
		set<int>::iterator pinIt = activatedThisCycle.begin();
		for (; pinIt != activatedThisCycle.end(); pinIt++)
		{
			pinActivations->at(i)->push_back(*pinIt);
			post_num_activations += currentPins[*pinIt].size();
		}
	}
}
///////////////////////////////////////////////////////////////////
// Gets the number of switches in an activation sequence		//
//////////////////////////////////////////////////////////////////
int FlowNetworkPinMapper::getNumSwitches(const vector<int> & seq)
{
	//cout << "E" << flush << endl;
	int result = 0;
	if(seq.size() == 0 || seq.size() == 1)
	{
		return 0;
	}
	int test = 0;
	while(seq[test] == 2)
	{
		test++;
	}
	int prevIdx = test;
	for(int i = test + 1; i <  seq.size(); ++i)
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

//////////////////////////////////////////////////////////////////////////
// Calculates and prints switching results based on new pin activation  //
// sequences															//
//////////////////////////////////////////////////////////////////////////
void FlowNetworkPinMapper::calculateSwitchingResults()
{
	vector<int> switches;
	for(int i = 0; i < PinActivationSeqs.size(); ++i)
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
	for(int i = 0; i < switches.size(); ++i)
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

///////////////////////////////////////////////////////////////////////////
// Converts an activation matrix to a compatibility graph, calling       //
// addEdgeBasedOnContraints() to determine where to add edges			 //
///////////////////////////////////////////////////////////////////////////
CliquePinMapper::Graph FlowNetworkPinMapper::convertMatrixToGraph(const int & size)
{
	Graph g;
	string s1, s2;
	//bool match2 = false;
	char * string1 = new char[10];
	char * string2 = new char[10];
	//int size = matrix.size();

	// Get number of electrodes
	//int numElectrodes = arch -> getNumCellsY() * arch -> getNumCellsX();

	bool match;

	//Generate a map of each electrode number to wheter or not it is a trivial case
	// i.e. the electrode never being on during the entire simulation.
	for (int i = 0; i < tAMatrix.size(); ++i)
	{
		match = true;
		for( int j = 0; j < tAMatrix[i].size(); ++j)
		{
			if(tAMatrix[i][j] == 1)
			{
				match = false;
				break;
			}
		}
		if(match)
		{
			trivialElectrodes.insert(make_pair(i, true));
		}
		else
		{
			trivialElectrodes.insert(make_pair(i, false));
		}
	}

	// Add a node for each non trivial electrode
	for (unsigned i = 0; i < tAMatrix.size(); ++i)
	{
		if(!trivialElectrodes[i])
			g.add_node( Util::itoa(i, string1, 10));
	}


	for(unsigned j = 0; j < tAMatrix.size(); ++j)
	{
		if(trivialElectrodes[j])
			continue;
		s1 = Util::itoa(j,string1,10);

		for(unsigned k = j + 1; k < tAMatrix.size(); ++k )
		{
			if(trivialElectrodes[k])
				continue;
			s2 = Util::itoa(k,string2,10);

			addEdgeBasedOnConstraints(g, j, k, s1, s2, size);
		}
	}

	//cleanup memory
	delete [] string1;
	delete [] string2;
	return g;

}

/////////////////////////////////////////////////////////////////
// Converts a colored compatibility graph to a list of cliques //
/////////////////////////////////////////////////////////////////
vector<vector< string > > FlowNetworkPinMapper::convertToCliques(CliquePinMapper::Graph & g)
{
	vector<vector<string> > cliques(g.find_max_color() + 1);
	for(map< string,int >::iterator i = g.coloring.begin(); i != g.coloring.end(); i++) {
		cliques[(*i).second].push_back((*i).first);
	}
	return cliques;
}

////////////////////////////////////////////////////////////////////////
// returns and deletes the maximum size clique in the list of cliques //
////////////////////////////////////////////////////////////////////////
vector<string> FlowNetworkPinMapper::findAndDeleteMaxClique()
{
	if(cliques.size() != 0)
	{
		int maxLen = 0;
		int maxIdx = 0;
		for(int i = 0; i < cliques.size(); ++i)
		{
			if(cliques[i].size() > maxLen)
			{
				maxLen = cliques[i].size();
				maxIdx = i;
			}
		}
		vector<string> v(cliques[maxIdx]);
		cliques.erase(cliques.begin() + maxIdx);
		return v;
	}
	cerr << "[FlowNetworkPinMapper] ERROR: Cannot find max clique, no more cliques available.." << endl;
	return vector<string>();
}

////////////////////////////////////////////////////////////////
// Sets the pin mapping based on computed pin assignments     //
////////////////////////////////////////////////////////////////
void FlowNetworkPinMapper::setPinMapping()
{
	stringstream ss;
	long long pinNo = currentPins.size();
	int numCols = arch -> getNumCellsX();

	for(int i = 0; i  < pinMapping -> size(); ++i)
	{
		for(int j = 0; j < pinMapping -> at(i) -> size(); ++j)
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

	computeAvailResources();


}

//////////////////////////////////////////////////////////////////////////////////
// This method compares two compatable activation sequences and returns the 	//
// sequence that is the combination of the two inputs, that is, all necessary   //
// "don't cares" are converted to an "off" or "on" signal						//
//////////////////////////////////////////////////////////////////////////////////
vector<int> FlowNetworkPinMapper::seqCompare(const vector<int> & s1, const vector<int> & s2)
{
	if(s1.size() != s2.size())
		{
			return vector<int>();
		}
		vector<int> result(s1.size());

		for (unsigned i = 0; i < s1.size(); ++i)
		{
			//cout << "i = " << i << flush << endl;
			if(s1[i] == 0 && (s2[i] == 2 || s2[i] == 0))
			{
				result[i] = 0;
			}
			else if(s1[i] == 1 && (s2[i] == 2 || s2[i] == 1))
			{
				result[i] = 1;
			}
			else if(s1[i] == 2 && s2[i] == 0)
			{
				result[i] = 0;
			}
			else if(s1[i] == 2 && s2[i] == 1)
			{
				result[i] = 1;
			}
			else if(s1[i] == 2 && s2[i] == 2)
			{
				result[i] = 2;
			}
			else
			{
				//cerr << "[SwitchingAwarePMWR] InternalError: Incompatible Sequences, incorrect output likely" << endl;
				return vector<int>();
			}
		}
		return result;
}


////////////////////////////////////////////////////////////////////////////////
// Computes min cost max flow and addresses electrodes based on constraints   //
// provided in constructMCMF()												  //
////////////////////////////////////////////////////////////////////////////////
void FlowNetworkPinMapper::processAndAddressElectrodes()
{
	int N = currentPins.size();
	//Number of nodes in flow network is size of clique + current number of pins
	// + source + sink
	int num_nodes = max_clique.size() + N + 2;
/*		MinCostMaxFlowGraph mcmf(num_nodes);
		//Add edges to sink node (1) from every current pin with 1 unit cap and 0 cost
		for(int i = 0; i < N; ++i)
		{
			 mcmf.AddEdge(i + 2, 1, 1, 0);
		}
		//Add edges from source node (0) to every electrode in the clique with 1 unit cap and 0 cost
		for(int i = 0; i < max_clique.size(); ++i)
		{
			mcmf.AddEdge(0, N + i + 2, 1, 0);
		}
		//Add edges from the clique electrodes node to current pins
		// only if they are compatible, with 1 unit cap and RAUek + RAUpi cost
		for(int i = 0; i < max_clique.size(); ++i)
		{
			int idx = atoi(max_clique[i].c_str());
			for(int j = 0; j < PinActivationSeqs.size(); ++j)
			{
				bool incomp = false;
				// RAUek is the number of redundant activations incurred in the max_clique[i]
				// electrode activation sequence when addressing it with pin j
				int RAUek = 0;
				// RAUek is the number of redundant activations incurred in pin j's
				// activation sequence when addressing it with the max_clique[i]
				int RAUpi = 0;

				for(int k = 0; k < PinActivationSeqs[j].size(); ++k)
				{
					if(PinActivationSeqs[j][k] == 1 && tAMatrix[idx][k] == 2)
					{
						RAUek++;
					}
					else if(PinActivationSeqs[j][k] == 2 && tAMatrix[idx][k] == 1)
					{

						RAUpi += currentPins[j].size();

					}
					else if(PinActivationSeqs[j][k] == 1 && tAMatrix[idx][k] == 0)
					{
						incomp = true;
						break;
					}
					else if(PinActivationSeqs[j][k] == 0 && tAMatrix[idx][k] == 1)
					{
						incomp = true;
						break;
					}
				}
				if(!incomp)
				{
					mcmf.AddEdge(N + i + 2, j + 2, 1, RAUek + RAUpi);
				}
			}
		}*/


		MinCostMaxFlowGraph mcmf(constructMCMF());
		// Calculate flow network result
		pair<long long, long long> result = mcmf.GetMaxFlow(0,1);
		//cout << "Cost = " << result.second << endl;
		//cout << "Flow = " << result.first << endl;
		//totalRAU += result.second;
		// Setup a copy of the max_clique in order to determine unaddressable electrodes
		vector<string> remaining(max_clique);

		//Address Electrodes based on flow
		// For each electrode in the clique
		for(int i = N + 2; i < num_nodes; ++i)
		{
			//For each current pin
			for(int j = 2; j < N + 2; ++j)
			{
				// If there is flow from the electrode to the pin
				if(mcmf.getFlow(i, j) > 0)
				{


					int idx = atoi(max_clique[i - N - 2].c_str());
					// Get the electrode's activation sequence
					/*for(int k = 0; k < AMatrix.size(); ++k)
					{
							seq[k] = AMatrix[k][idx];

					}*/
					vector<int> seq(tAMatrix.at(idx));
					// Merge the activation sequence of the pin and the electrode
					// This is the new pin activation sequence after addressing it
					//with the new electrode
					PinActivationSeqs[j-2] = seqCompare(seq,PinActivationSeqs[j - 2]);
					// Add the electrode to the list of addressed electrodes
					addressedElectrodes.insert(make_pair(max_clique[i - N - 2], j - 2));
					// Update the pin information about the addition of the new electrode
					currentPins[j - 2].push_back(max_clique[i - N - 2]);
					// This electrode has been addressed, so erase it from the unaddressed list
					remaining.erase(find(remaining.begin(), remaining.end(),max_clique[i - N - 2]));


				}
			}
		}

		//Process Electrodes in clique that did not flow and were not addressed
		// by adding a new pin for each
		for(int i = 0; i < remaining.size(); ++i)
		{
			currentPins.push_back(vector<string>());
			currentPins[currentPins.size() - 1].push_back(remaining[i]);

			int index = atoi(remaining[i].c_str());
			vector<int> seq(tAMatrix.at(index));
			/*for(int j = 0; j < AMatrix.size(); ++j)
			{
				seq[j] = AMatrix[j][index];
			}*/
			PinActivationSeqs.push_back(seq);
			addressedElectrodes.insert(make_pair(remaining[i], currentPins.size() - 1));
		}
}
void FlowNetworkPinMapper::addGroundVectorsToEntireAssay(int seqIdx)
{
	GroundVector_r = 2;
	GroundVector_c = 4;
	if(GroundVector_r <= 1)
	{
		cerr << "[ReliabilityAwarePinMapper] InternalError: num Consecutive Activations Threshold(GroundVector_r) must be 2 or greater..." << endl << flush;
		return;
	}
	int numConsecActivations = 0;
	// If we get to the last entry and there is no conflict
	// the last entry does not matter because it is the end of the
	// assay execution
	vector<int> seq(PinActivationSeqs[seqIdx]);
	//for(int i = 0; i < currentPins[seqIdx].size(); ++i)
	//{
		//if(seqIdx == 19)
		//{
	//		cout << currentPins[seqIdx][i] << endl;
		//}
//	}
	if(seq.size() <= 1)
	{
		cerr << "[ReliabilityAwarePinMapper] InternalError: insufficient activation sequence size (size < 2)..." << endl << flush;
		return;
	}

	for(int i = 0; i < seq.size() - 1; ++i)
	{
		//cout << seq[i];
		if(seq[i] == 1)
		{
			//cout << "seq[" << i + 1 << "] = " << seq[i + 1] << endl;
			numConsecActivations++;
		}
		else
		{
			numConsecActivations = 0;
		}
		if(numConsecActivations == GroundVector_c)
		{
			if(!criticalTimesteps.empty() && criticalTimesteps[i] && criticalTimesteps[i + 1])
			{
				//cout << "criticalTimesteps[" << i << "] = " << criticalTimesteps[i] << endl;
				//cout << "criticalTimesteps[" << i + 1 << "] = " << criticalTimesteps[i + 1] << endl;
				//We cannot insert a ground vector here as it violates a critical operation
				// (if i happens to be the end of one critical operation and i + 1 the beginning
				// of another, technically we CAN add a ground vector in that case, but this algorithm
				// will not do that)
				return;
			}
			else
			{
				//We need to insert up to GroundVector_c Ground Vectors
				if(seq[i + 1] != 0)
				{
					if( numGVatIndex[i] < GroundVector_r)
						numGVatIndex[i] = GroundVector_r;
					//insertGroundVectors(i + 1, GroundVector_c, pinActivations, matrix);
					//GVtemp += GroundVector_c;
				}
				else
				{
					int remainingGroundVectors = GroundVector_r;
					for(int j = i + 1 ; j < i + GroundVector_r + 1; ++j)
					{
						if(j >= seq.size())
						{
							//Do I need to insert the ground vectors at the end?
							//Does it not matter?
							break;
						}
						if(seq[j] != 0)
						{
							if(numGVatIndex[i] < remainingGroundVectors)
							{
								numGVatIndex[i] = remainingGroundVectors;
							}
							//insertGroundVectors(j, remainingGroundVectors, pinActivations, matrix);
							break;
						}
						else
						{
							--remainingGroundVectors;
						}

					}
				}
			}
			numConsecActivations = 0;
		}
	}
}

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
 * Source: power_clique_pin_mapper.cc													*
 * Original Code Author(s): Zach Zimmerman										*
 * Original Completion/Release Date: 7/10/2014									*
 *																				*
 * Details: Uses itoa() which is not supported by some C++ compilers, and is	*
 * not part of the ISO C++ standard.											*						*
 * Revision History:															*
 * WHO		WHEN		WHAT													*
 * ---		----		----													*
 * FML		MM/DD/YY	One-line description									*
 *-----------------------------------------------------------------------------*/

#include "../../Headers/PinMapper/power_clique_pin_mapper.h"
#include "../../Headers/Models/min_cost_max_flow_graph.h"
#include<string>
#include<set>
#include<algorithm>

PowerAwarePinMapper::PowerAwarePinMapper()
{
	arch = NULL;
	totalRAU = 0;
}
PowerAwarePinMapper::PowerAwarePinMapper(DmfbArch *dmfbArch)
{
	arch = dmfbArch;
	totalRAU = 0;
}

//////////////////////////////////////////////////////////////////////////////////////
// Main thread of execution, calls other methods to assist in calculating result	//
// Creates cliques of mutually incompatible electrode activation sequences			//
// then uses a flow network to calculate the optimal addressing scheme for			//
//	each clique in order of the size of each clique.								//
//////////////////////////////////////////////////////////////////////////////////////
void PowerAwarePinMapper::setMapPostRoute(vector<vector<int> *> *pinActivations, map<Droplet *, vector<RoutePoint *> *> *routes)
{
	stringstream _SS;
	numGVatIndex = vector<int>( pinActivations -> size(), 0);
	pre_num_activations = 0;
	post_num_activations = 0;
	int pinsBefore = arch->getPinMapper()->getNumUniquePins();
	int elecsBefore = arch->getPinMapper()->getNumElectrodes();
	if (debugPrint()) cout << "[PowerAwarePinMapper] Initializing Matrix and calculating interference..." << endl;
	vector< vector<int> > AMatrix = initActivationMatrix(pinActivations);
	initTranspose(AMatrix);
	for(int i = 0; i < AMatrix.size(); ++i)
	{
		for(int j = 0; j < AMatrix[i].size(); ++j)
		{
				if(AMatrix[i][j] == 1)
					pre_num_activations++;
		}
	}
	if (debugPrint()) cout << "[PowerAwarePinMapper] Convert to graph...." << endl;
	Graph coloringGraph = convertMatrixToGraph(AMatrix.size());
	if (debugPrint()) cout << "[PowerAwarePinMapper] Coloring.... "<< endl;
	coloringGraph.dsatur();
	if (debugPrint()) cout << "[PowerAwarePinMapper] Converting to cliques..." << endl;
	cliques = convertToCliques(coloringGraph);

	if(cliques.empty())
	{
		cerr << "[PowerAwarePinMapper] Error: Empty Graph... Aborting" << endl;
		return;
	}

	stringstream SS;

	// Initial setup of flow problem (Each electrode in the largest clique must be addressed
	// by its own pin (they are all incompatible).
	max_clique = findAndDeleteMaxClique();
	for(int i = 0; i < max_clique.size(); ++i)
	{
		//determine the electrode number asociated with this particular entry
		// in the clique
		int index = atoi(max_clique[i].c_str());;
		if(trivialElectrodes[index])
		{
			continue;
		}
		//Setup an empty pin
		currentPins.push_back(vector<string>());
		//Assign this electrode to that pin
		currentPins[currentPins.size() - 1].push_back(max_clique[i]);
		//Setup empty activation sequence
		vector<int> seq(AMatrix.size());
		// Obtain this electrode's activation sequence
		for(int j = 0; j < AMatrix.size(); ++j)
		{
			seq[j] = AMatrix[j][index];
		}
		//since this electrode is the only one assigned to the pin for now
		//the pin's activation sequence is equal to this electrode's activation sequence
		PinActivationSeqs.push_back(seq);
		//Add this electrode to the set of addressed electrodes, along with its corresponding pin
		addressedElectrodes.insert(make_pair(max_clique[i], currentPins.size() - 1));
	}
	// Continue to next cliques until we run out
	if (debugPrint()) cout << "[PowerAwarePinMapper] Addressing pins..." << endl;
	while(!cliques.empty())
	{
		// Get the next largest clique
		max_clique = findAndDeleteMaxClique();
		// Construct flow network and calculate flow result, address electrodes in clique
		// According to flow result
		processAndAddressElectrodes();
	}
	/*
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
	*/
	setPinActivations(pinActivations);
	setPinMapping();
	if (debugPrint()) cout << "Entire electrode array size ( num electrodes * num activations) = " << addressedElectrodes.size() * pinActivations -> size() << endl;
	if (debugPrint()) cout << "[PowerAwarePinMapper] Addressed " << currentPins.size() << " pins." << endl;
	cout << "[PowerAwarePinMapper] Total redundant activations: " << post_num_activations - pre_num_activations << endl;
	cout << "[PowerAwarePinMapper] Total intitial activations: " << pre_num_activations << endl;
	cout << "[PowerAwarePinMapper] Total post-mapping activations: " << post_num_activations << endl;
	cout << "[PowerAwarePinMapper] Number of electrodes reduced from " << elecsBefore << " to " << arch->getPinMapper()->getNumElectrodes() << endl;
	cout << "[PowerAwarePinMapper] Number of pins reduced from " << pinsBefore << " to " << arch->getPinMapper()->getNumUniquePins() << endl;
	if (debugPrint()) cout << "[PowerAwarePinMapper] ";
	if (debugPrint()) calculateSwitchingResults();

	for(int i = 0; i < PinActivationSeqs.size(); ++i)
	{
		addGroundVectorsToEntireAssay(i);
	}
	long long numGVs = 0;
	for(int i = 0; i < numGVatIndex.size(); ++i)
	{
		numGVs += numGVatIndex.at(i);
	}
	if (debugPrint()) cout << "[PowerAwarePMWR] total gvs that would be inserted: " << numGVs << endl;
}
/*
/////////////////////////////////////////////////////////////////////////
// Sets the pin mapping result.
//
/////////////////////////////////////////////////////////////////////////
void PowerAwarePinMapper::setPinMapping(map<string,int> addressedElectrodes, int pinNo)
{
	stringstream ss;

	int numCols = arch -> getNumCellsX();

	for(int i = 0; i  < pinMapping -> size(); ++i)
	{
		for(int j = 0; j < pinMapping -> at(i) -> size(); ++j)
		{
			pinMapping -> at(i) -> at(j) = -1;
		}
	}

	for(map<string,int>::iterator i = addressedElectrodes.begin(); i != addressedElectrodes.end(); ++i)
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

/////////////////////////////////////////////////////////////////////////
// This is identical to CliquePinMapper::convertMatrixToGraph() except //
// it adds the opposite edges to the result, i.e adds an edge between  //
// compatible electrodes instead of incompatible.											   //
/////////////////////////////////////////////////////////////////////////
PowerAwarePinMapper::Graph PowerAwarePinMapper::convertMatrixToGraph(const int & size)
{
	Graph g;
	string s1, s2;
	bool nomatch = false;
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
	for (int i = 0; i < tAMatrix.size(); ++i)
	{
		if(!trivialElectrodes[i])
			g.add_node( Util::itoa(i, string1, 10));
	}


	for(int j = 0; j < tAMatrix.size(); ++j)
	{
		if(trivialElectrodes[j])
			continue;
		s1 = Util::itoa(j,string1,10);

		for(int k = j + 1; k < tAMatrix.size(); ++k )
		{
			if(trivialElectrodes[k])
				continue;
			s2 = Util::itoa(k,string2,10);


			//Warmup sequence of 200 can be hardcoded out? (if application permits)
			for(int i = 0; i < size; ++i)
			{
				if( (tAMatrix[j][i] == 0 && tAMatrix[k][i] == 1) || (tAMatrix[j][i] == 1 && tAMatrix [k][i] == 0))
				{

					nomatch = true;
					break;
				}
			}
			if(nomatch)
			{
				nomatch = false;
			}
			else
			{
				//Add edge only if edges are compatible
				g.add_edge(s1, s2);
			}
		}
	}

	//cleanup memory
	delete [] string1;
	delete [] string2;
	return g;

}


int PowerAwarePinMapper::getNumSwitches(const vector<int> & seq)
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

void PowerAwarePinMapper::calculateSwitchingResults()
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


/////////////////////////////////////////////////////////////////////////
// This method converts a colored graph to an easy to access list of   //
// cliques.															   //
/////////////////////////////////////////////////////////////////////////
vector<vector< string > > PowerAwarePinMapper::convertToCliques(PowerAwarePinMapper::Graph & g)
{
	vector<vector<string> > cliques(g.find_max_color() + 1);
	for(map< string,int >::iterator i = g.coloring.begin(); i != g.coloring.end(); i++) {
		cliques[(*i).second].push_back((*i).first);
	}
	return cliques;
}
/////////////////////////////////////////////////////////////////////////
// The method determines the clique of maximum length from the list of //
// cliques, removes it from the list and returns it.				   //
/////////////////////////////////////////////////////////////////////////
vector<string> PowerAwarePinMapper::findAndDeleteMaxClique()
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
	cerr << "[PowerAwarePinMapper] ERROR: Cannot find max clique, no more cliques available.." << endl;
	return vector<string>();

}
//////////////////////////////////////////////////////////////////////////////
// This method constructs a flow network of electrodes in the clique and	//
// the current pins available to address new electrodes. This flow network	//
// is solved and the flow result is used to address the electrodes in		//
// the provided clique. Electrodes that cannot be addressed are handled by	//
// adding an additional pin for each unaddressable electrode at this step.	//
//////////////////////////////////////////////////////////////////////////////
void PowerAwarePinMapper::processAndAddressMaxClique(vector<string> max_clique, const vector<vector<int> > & AMatrix)
{
	int N = currentPins.size();
	//Number of nodes in flow network is size of clique + current number of pins
	// + source + sink
	int num_nodes = max_clique.size() + N + 2;
	MinCostMaxFlowGraph mcmf(num_nodes);
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
				if(PinActivationSeqs[j][k] == 1 && AMatrix[k][idx] == 2)
				{
					RAUek++;
				}
				else if(PinActivationSeqs[j][k] == 2 && AMatrix[k][idx] == 1)
				{

					RAUpi += currentPins[j].size();

				}
				else if(PinActivationSeqs[j][k] == 1 && AMatrix[k][idx] == 0)
				{
					incomp = true;
					break;
				}
				else if(PinActivationSeqs[j][k] == 0 && AMatrix[k][idx] == 1)
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
	}
	// Calculate flow network result
	pair<long long, long long> result = mcmf.GetMaxFlow(0,1);
	cout << "Cost = " << result.second << endl;
	cout << "Flow = " << result.first << endl;
	totalRAU += result.second;
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

				vector<int> seq(AMatrix.size());
				int idx = atoi(max_clique[i - N - 2].c_str());
				// Get the electrode's activation sequence
				for(int k = 0; k < AMatrix.size(); ++k)
				{
						seq[k] = AMatrix[k][idx];

				}
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
		vector<int> seq(AMatrix.size());
		int index = atoi(remaining[i].c_str());
		for(int j = 0; j < AMatrix.size(); ++j)
		{
			seq[j] = AMatrix[j][index];
		}
		PinActivationSeqs.push_back(seq);
		addressedElectrodes.insert(make_pair(remaining[i], currentPins.size() - 1));
	}
}

//////////////////////////////////////////////////////////////////////////////////
// This method compares two compatable activation sequences and returns the 	//
// sequence that is the combination of the two inputs, that is, all necessary    //
// "don't cares" are converted to an "off" or "on" signal						//
//////////////////////////////////////////////////////////////////////////////////
vector<int> PowerAwarePinMapper::seqCompare(const vector<int> & s1, const vector<int> & s2)
{
	if(s1.size() != s2.size())
	{
		cerr << "[PowerAwarePinMapper] InternalError: MisSized Sequences, incorrect output likely" << endl;
		return vector<int>();
	}
	vector<int> result(s1.size());
	for (int i = 0; i < s1.size(); ++i)
	{
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
			cerr << "[PowerAwarePinMapper] InternalError: Incompatible Sequences, incorrect output likely" << endl;
			return vector<int>();
		}
	}
	return result;
}

*/

MinCostMaxFlowGraph PowerAwarePinMapper::constructMCMF()
{
	int N = currentPins.size();
	//Number of nodes in flow network is size of clique + current number of pins
	// + source + sink
	int num_nodes = max_clique.size() + N + 2;
	MinCostMaxFlowGraph mcmf(num_nodes);
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
	}
	return mcmf;
}
void PowerAwarePinMapper::addEdgeBasedOnConstraints(CliquePinMapper::Graph & g, long long seq1index, long long seq2index, string s1, string s2, long long size)
{
	bool match = false;
	for(int i = 0; i < size; ++i)
	{
		if( (tAMatrix[seq1index][i] == 0 && tAMatrix[seq2index][i] == 1) || (tAMatrix[seq1index][i] == 1 && tAMatrix [seq2index][i] == 0))
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
		//Add edge only if edges are compatible
		g.add_edge(s1, s2);
	}
}
/*
int PowerAwarePinMapper::getRAUpi(int pinIdx, int elecIdx, const vector<vector<int> > & AMatrix) const
{
	int result = 0;
	vector<string> electrodes(currentPins[pinIdx]);
	for(int i = 0; i < electrodes.size(); ++i)
	{
		int idx = atoi(electrodes[i].c_str());
		for(int j = 0; j < AMatrix.size(); ++j)
		{
			if(AMatrix[j][idx] == 2AMatrix[j][elexIdx])
		}
	}

	return result;
}
*/

/*for (int i = 0; i < cliques.size(); ++i)
	{
		if(cliques[i].size() == 0)
		{
			cerr << "Zero item clique..." << endl;
			continue;
		}
		if(cliques[i].size() == 1)
		{
			cout << "One item clique..." << endl;
			continue;
		}
		bool match = true;
		bool match2 = true;
		cout << "Clique #" << i << endl;
		for(int j = 0; j < cliques[i].size(); ++j)
		{

			cout << "e" << cliques[i][j] << " : ";
			for(int k = i + 1; k < cliques[i].size(); ++k)
			{
				//int val;
				for(int l = 0; l < AMatrix.size(); ++l)
				{
					if((AMatrix[l][atoi(cliques[i][j].c_str())] == 0 && AMatrix[l][atoi(cliques[i][k].c_str())] == 1) || (AMatrix[l][atoi(cliques[i][j].c_str())] == 0 && AMatrix[l][atoi(cliques[i][k].c_str())] == 1))
					{
						break;
					}
					match = false;
				}
				if(!match)
				{
					//cout << "(" << cliques[i][j] << ", " << cliques[i][k] << ") are Incompatible" << endl;
					//match = false;
					//break;

				}
				else
				{
					match2 = false;
					//cerr << "(" << cliques[i][j] << ", " << cliques[i][k] << ") are Compatible" << endl;
					match = false;
					break;
				}

										//cout << val << " " << cliques[i][j] << " " << endl;
				//cout << AMatrix[k][atoi(cliques[i][j].c_str())];
			}

			if(!match2)
			{
				cout << "Failed" << endl;
			}
			else
				cout << "Pass" << endl;

		}
		cout << endl;
	}
	/*cout << "MATRIX" << endl << endl;
	for(int i = 200; i < AMatrix.size(); ++i)
	{
		for(int j = 0; j < AMatrix[i].size(); ++j)
		{
			cout << AMatrix[i][j];
		}
		cout << endl;
	}
	for(int j = 0; j < trivialElectrodes.size(); ++j)
			{
				if(trivialElectrodes[j])
				{
					cout << "T";
				}
				else
					cout << "F";
			}
			cout << endl;*/

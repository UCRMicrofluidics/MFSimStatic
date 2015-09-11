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
 * Source: reliability_aware_pin_mapper.cc													*
 * Original Code Author(s): Zach Zimmerman										*
 * Original Completion/Release Date: 											*
 *																				*
 * Details: Uses itoa() which is not supported by some C++ compilers, and is	*
 * not part of the ISO C++ standard. Does not actually add ground vectors to	*
 * the assay yet.																*
 * Revision History:															*
 * WHO		WHEN		WHAT													*
 * ---		----		----													*
 * FML		MM/DD/YY	One-line description									*
 *-----------------------------------------------------------------------------*/

#include "../../Headers/PinMapper/reliability_aware_pin_mapper.h"
#include "../../Headers/Models/min_cost_max_flow_graph.h"
#include "../../Headers/Router/router.h"
#include<string>
#include<set>
#include<algorithm>

ReliabilityAwarePinMapper::ReliabilityAwarePinMapper()
{
	arch = NULL;
}
ReliabilityAwarePinMapper::ReliabilityAwarePinMapper(DmfbArch *dmfbArch)
{
	arch = dmfbArch;

}

//////////////////////////////////////////////////////////////////////////////////////
// Main thread of execution, calls other methods to assist in calculating result	//
// Creates cliques of mutually incompatible electrode activation sequences			//
// then uses a flow network to calculate the optimal addressing scheme for			//
//	each clique in order of the size of each clique.								//
//////////////////////////////////////////////////////////////////////////////////////
void ReliabilityAwarePinMapper::setMapPostRoute(vector<vector<int> *> *pinActivations, map<Droplet *, vector<RoutePoint *> *> *routes)
{
	GroundVector_r = 2;
	GroundVector_c = 4;
	numGVatIndex = vector<int>( pinActivations -> size(), 0);
	//Initialize critical timesteps to all non-critical
	//This must be changed if an assay has critical timings
	for(int i = 0; i < pinActivations -> size(); ++i)
	{
		criticalTimesteps.insert(make_pair(i, false));
	}



	stringstream _SS;
	set<int> allPins;
	pre_num_activations = 0;
	post_num_activations = 0;
	int pinsBefore = arch->getPinMapper()->getNumUniquePins();
	int elecsBefore = arch->getPinMapper()->getNumElectrodes();
	if (debugPrint()) cout<< "[ReliabilityAwarePinMapper] Initializing Matrix and calculating interference..." << endl;
	vector< vector<int> > AMatrix = initActivationMatrix(pinActivations);
	initTranspose(AMatrix);
	/*ofstream outfile("BOARD.txt");
	for(int i = 0; i < AMatrix.size(); ++i)
	{
		printBoardAtActivation(i, AMatrix, outfile);
	}
	outfile.close();
*/
	//for(int i = 200; i < 400; ++i)
	//{
	//	cout << tAMatrix.at(62).at(i) <<flush;
	//}
	//cout << endl;
	for(int i = 0; i < AMatrix.size(); ++i)
	{
		for(int j = 0; j < AMatrix[i].size(); ++j)
		{
				if(AMatrix[i][j] == 1)
					pre_num_activations++;
		}
	}

	if (debugPrint()) cout<< "[ReliabilityAwarePinMapper] Convert to graph...." << endl;
	Graph coloringGraph = convertMatrixToGraph(AMatrix.size());
	if (debugPrint()) cout<< "[ReliabilityAwarePinMapper] Coloring.... "<< endl;
	coloringGraph.dsatur();
	if (debugPrint()) cout<< "[ReliabilityAwarePinMapper] Converting to cliques..." << endl;
	cliques = convertToCliques(coloringGraph);

	if(cliques.empty())
	{
		cerr << "[ReliabilityAwarePinMapper] Error: Empty Graph... Aborting" << endl;
		return;
	}

	stringstream SS;

	// Initial setup of flow problem (Each electrode in the largest clique must be addressed
	// by its own pin (they are all incompatible).
	vector<string> clique = findAndDeleteMaxClique();
	for(int i = 0; i < clique.size(); ++i)
	{
		//determine the electrode number asociated with this particular entry
		//in the clique
		int index = atoi(clique[i].c_str());

		//Setup an empty pin
		currentPins.push_back(vector<string>());
		//Assign this electrode to that pin
		currentPins[currentPins.size() - 1].push_back(clique[i]);
		//Setup empty activation sequence
		vector<int> seq(tAMatrix.at(index));
		// Obtain this electrode's activation sequence
		//for(int j = 0; j < AMatrix.size(); ++j)
		//{
		//	seq[j] = AMatrix[j][index];
		//}
		//since this electrode is the only one assigned to the pin for now
		//the pin's activation sequence is equal to this electrode's activation sequence
		PinActivationSeqs.push_back(seq);
		//Add this electrode to the set of addressed electrodes, along with its corresponding pin
		addressedElectrodes.insert(make_pair(clique[i], currentPins.size() - 1));
	}
	// Continue to next cliques until we run out
	while(!cliques.empty())
	{
		//vector<string> max_clique;
		// Get the next largest clique
		max_clique = findAndDeleteMaxClique();
		//for(int i = 0; i < max_clique.size(); ++i)
		//{
			//if(trivialElectrodes[atoi(max_clique[i].c_str())])
			//{
				//cout << max_clique[i] << endl;
			//}
		//}
		// Construct flow network and calculate flow result, address electrodes in clique
		// According to flow result
		//processAndAddressMaxClique(max_clique, pinActivations, AMatrix);
		processAndAddressMaxClique(pinActivations, AMatrix);
	}

	/*char * string1 = new char[10];
	cout << "Pin Activations size = " << pinActivations ->size();
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
			postNumActivations += currentPins[*pinIt].size();
		}
	}
	cout << "Pin Activations size = " << pinActivations ->size() << endl;*/
	setPinActivations(pinActivations);
	int totalGVs = 0;
	//We assume nothing needs to be done after last activation, so we do not consider it
	// This guarantees error-free array access.
	for(int i = 0, k = 0; i < pinActivations -> size() - 1; ++i, ++k)
	{
		totalGVs += numGVatIndex[k];
		int count = 0;
		/*if(numGVatIndex[k] > 0)
		{
			cout << "Inserting " << numGVatIndex[k] << " GVs at timestep " << k << endl;
		}*/
		for(int j = 0; j < numGVatIndex[k]; ++j)
		{
			pinActivations->insert(pinActivations->begin() + i + 1, new vector<int>());
			count++;
		}
		i += count;
	}
	//cout << "Pin Activations size = " << pinActivations ->size() << endl;
	//setPinMapping(addressedElectrodes, currentPins.size());
	setPinMapping();
	//cout << "addressedElectrodes.at(62) = " << addressedElectrodes.at("62") << endl;
	//for(int i = 0; i < currentPins[19].size(); ++i)
	//{
	//	cout << currentPins[19][i] << endl;
	//}

	rerouteDroplets(routes);
	//Analyze::AnalyzeRoutes(arch, routes);
	//for(int i = 200; i < 400 ; ++i)
	//	{
	//		cout << PinActivationSeqs[19][i];
	//	}
	//	cout << endl;
	//routes->clear();
	//Router* router = new Router(arch);
	//router -> simulateDropletMotion(routes, pinActivations);
	//delete router;

	if (debugPrint()) cout<< "Entire electrode array size ( num electrodes * num activations) = " << addressedElectrodes.size() * pinActivations -> size() << endl;
	if (debugPrint()) cout<< "[ReliabilityAwarePinMapper] Addressed " << currentPins.size() << " pins." << endl;
	cout << "[ReliabilityAwarePinMapper] Total redundant activations: " << post_num_activations - pre_num_activations << endl;
	cout << "[ReliabilityAwarePinMapper] Total intitial activations: " << pre_num_activations << endl;
	cout << "[ReliabilityAwarePinMapper] Total post activations: " << post_num_activations << endl;
	cout << "[ReliabilityAwarePinMapper] Number of electrodes reduced from " << elecsBefore << " to " << arch->getPinMapper()->getNumElectrodes() << endl;
	cout << "[ReliabilityAwarePinMapper] Number of pins reduced from " << pinsBefore << " to " << arch->getPinMapper()->getNumUniquePins() << endl;
	if (debugPrint()) cout<< "[ReliabilityAwarePinMapper] Total GVs Inserted: " << totalGVs << endl;
	if (debugPrint()) cout<< "[ReliabilityAwarePinMapper]";
	if (debugPrint()) calculateSwitchingResults();

	//TODO::Once Ground Vectors are inserted, droplet route must be changed for sim
	// to make sense.
}
/*
/////////////////////////////////////////////////////////////////////////
// Sets the pin mapping result.										   //
/////////////////////////////////////////////////////////////////////////
void ReliabilityAwarePinMapper::setPinMapping(map<string,int> addressedElectrodes, int pinNo)
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
// compatible electrodes instead of incompatible.					   //
/////////////////////////////////////////////////////////////////////////
ReliabilityAwarePinMapper::Graph ReliabilityAwarePinMapper::convertMatrixToGraph(const int & size)
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
*/
/*ReliabilityAwarePinMapper::Graph ReliabilityAwarePinMapper::convertMatrixToGraph(const vector< vector<int> > &matrix)
{
	Graph g;
	string s1, s2;
	bool match2 = false;
	char * string1 = new char[10];
	char * string2 = new char[10];
	int size = matrix.size();

	// Get number of electrodes
	int numElectrodes = arch -> getNumCellsY() * arch -> getNumCellsX();


	bool match;

	//Generate a map of each electrode number to wheter or not it is a trivial case
	// i.e. the electrode never being on during the entire simulation.
	for (int i = 0; i < numElectrodes; ++i)
	{
		match = true;
		for( int j = 0; j < matrix.size(); ++j)
		{
			if(matrix[j][i] == 1)
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
	for (int i = 0; i < numElectrodes; ++i)
	{
		if(!trivialElectrodes[i])
			g.add_node( Util::itoa(i, string1, 10));
	}


	for(int j = 0; j < numElectrodes; ++j)
	{
		if(trivialElectrodes[j])
			continue;
		s1 = Util::itoa(j,string1,10);

		for(int k = j + 1; k < numElectrodes; ++k )
		{
			if(trivialElectrodes[k])
				continue;
			s2 = Util::itoa(k,string2,10);


			//Warmup sequence of 200 can be hardcoded out? (if application permits)
			for(int i = 0; i < size; ++i)
			{

				if( (matrix[i][j] == 0 && matrix[i][k] == 1) || (matrix[i][j] == 1 && matrix [i][k] == 0))
				{
					match2 = true;
					break;
				}


			}
			if(match2)
			{
				match2 = false;
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

}*/
/*
/////////////////////////////////////////////////////////////////////////
// This method converts a colored graph to an easy to access list of   //
// cliques.															   //
/////////////////////////////////////////////////////////////////////////
vector<vector< string > > ReliabilityAwarePinMapper::convertToCliques(ReliabilityAwarePinMapper::Graph & g)
{
	vector<vector<string> > cliques(g.find_max_color() + 1);
	cerr << cliques.size() << endl;
	for(map< string,int >::iterator i = g.coloring.begin(); i != g.coloring.end(); i++) {
		cliques[(*i).second].push_back((*i).first);
	}
	return cliques;
}
/////////////////////////////////////////////////////////////////////////
// The method determines the clique of maximum length from the list of //
// cliques, removes it from the list and returns it.				   //
/////////////////////////////////////////////////////////////////////////

vector<string> ReliabilityAwarePinMapper::findAndDeleteMaxClique()
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
	cerr << "[ReliabilityAwarePinMapper] ERROR: Cannot find max clique, no more cliques available.." << endl;
	return vector<string>();

}
*/
//////////////////////////////////////////////////////////////////////////////
// This method constructs a flow network of electrodes in the clique and	//
// the current pins available to address new electrodes. This flow network	//
// is solved and the flow result is used to address the electrodes in		//
// the provided clique. Electrodes that cannot be addressed are handled by	//
// adding an additional pin for each unaddressable electrode at this step.	//
//////////////////////////////////////////////////////////////////////////////
void ReliabilityAwarePinMapper::processAndAddressMaxClique(vector<vector<int> *> *pinActivations, vector< vector<int> > & AMatrix)
{

		int N = currentPins.size();
		//Number of nodes in flow network is size of clique + current number of pins
		// + source + sink
		int num_nodes = max_clique.size() + N + 2;
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
				//cout << "j - 2 = " << j - 2 << endl;
				// If there is flow from the electrode to the pin
				if(mcmf.getFlow(i, j) > 0)
				{


					int idx = atoi(max_clique[i - N - 2].c_str());
					vector<int> seq(tAMatrix.at(idx));
					// Merge the activation sequence of the pin and the electrode
					// This is the new pin activation sequence after addressing it
					//with the new electrode
					PinActivationSeqs[j-2] = seqCompare(seq,PinActivationSeqs[j - 2]);
					// Add the electrode to the list of addressed electrodes
					addressedElectrodes.insert(make_pair(max_clique[i - N - 2], j - 2));
					// Update the pin information about the addition of the new electrode
					currentPins[j - 2].push_back(max_clique[i - N - 2]);
			//		cout << "addressed electrode: " << max_clique[i - N - 2] << endl;
					// This electrode has been addressed, so erase it from the unaddressed list
					remaining.erase(find(remaining.begin(), remaining.end(),max_clique[i - N - 2]));

					addGroundVectorsToEntireAssay(j-2);

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
			PinActivationSeqs.push_back(seq);
			addressedElectrodes.insert(make_pair(remaining[i], currentPins.size() - 1));
			addGroundVectorsToEntireAssay(PinActivationSeqs.size() - 1);
		}
}
/*
//////////////////////////////////////////////////////////////////////////////////
// This method compares two compatable activation sequences and returns the 	//
// sequence that is the combination of the two inputs, that is, all necessary    //
// "don't cares" are converted to an "off" or "on" signal						//
//////////////////////////////////////////////////////////////////////////////////
vector<int> ReliabilityAwarePinMapper::seqCompare(const vector<int> & s1, const vector<int> & s2)
{
	if(s1.size() != s2.size())
	{
		cerr << "[ReliabilityAwarePinMapper] InternalError: MisSized Sequences, incorrect output likely" << endl;
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
			//cerr << "[ReliabilityAwarePinMapper] InternalError: Incompatible Sequences, incorrect output likely" << endl;
			return vector<int>();
		}
	}
	return result;
}
*/
//////////////////////////////////////////////////////////////////////////////////////////
//Calculates the number of ground vectors that must be inserted to address a paticular	//
//sequence, also determines if a ground vector is insertable based on critical			//
//timesteps of an assay. This method has not been extensively tested. But does produce	//
//correct output when no timesteps are set to be critical								//
//////////////////////////////////////////////////////////////////////////////////////////
bool ReliabilityAwarePinMapper::calculateGroundVectors(const vector<int> & seq, int & GV)
{
	int numConsecActivations = 0;
	// If we get to the last entry and there is no conflict
	// the last entry does not matter because it is the end of the
	// assay execution
	int GVtemp = 0;
	for(int i = 0; i < seq.size() - 1; ++i)
	{
		//cout << seq[i];
		if(seq[i] == 1)
		{
			numConsecActivations++;
		}
		else
		{
			numConsecActivations = 0;
		}
		if(numConsecActivations == GroundVector_r)
		{
			if(criticalTimesteps[i] && criticalTimesteps[i + 1])
			{
				//cout << "criticalTimesteps[" << i << "] = " << criticalTimesteps[i] << endl;
				//cout << "criticalTimesteps[" << i + 1 << "] = " << criticalTimesteps[i + 1] << endl;
				//We cannot insert a ground vector here as it violates a critical operation
				// (if i happens to be the end of one critical operation and i + 1 the beginning
				// of another, technically we CAN add a ground vector in that case, but this algorithm
				// will not do that)
				return true;
			}
			else
			{
				//We need to insert up to GroundVector_c Ground Vectors
				if(seq[i + 1] != 0)
				{

					GVtemp += GroundVector_c;
					/*for(int k = 0; k < GroundVector_c; ++k)
					{
						cout << "0";
					}*/
				}
				else
				{
					int remainingGroundVectors = GroundVector_c;
					for(int j = i + 1 ; j < i + GroundVector_c + 1; ++j)
					{
						if(j >= seq.size())
						{
							//Do I need to insert the ground vectors at the end?
							//Does it not matter?
							break;
						}
						if(seq[j] != 0)
						{
							GVtemp += remainingGroundVectors;
							/*for(int k = 0; k < remainingGroundVectors; ++k)
							{
								cout << "0";
							}*/
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
	GV = GVtemp;
	//cout << seq[seq.size() - 1] << endl;
	return false;
}
/*
void ReliabilityAwarePinMapper::addGroundVectorsToEntireAssay(int seqIdx)
{
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
		if(numConsecActivations == GroundVector_r)
		{
			if(criticalTimesteps[i] && criticalTimesteps[i + 1])
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
					if( numGVatIndex[i] < GroundVector_c)
						numGVatIndex[i] = GroundVector_c;
					//insertGroundVectors(i + 1, GroundVector_c, pinActivations, matrix);
					//GVtemp += GroundVector_c;
				}
				else
				{
					int remainingGroundVectors = GroundVector_c;
					for(int j = i + 1 ; j < i + GroundVector_c + 1; ++j)
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
*/
/*
int ReliabilityAwarePinMapper::getNumSwitches(const vector<int> & seq)
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
*/
void ReliabilityAwarePinMapper::rerouteDroplets(map<Droplet*, vector<RoutePoint* > * > * routes)
{
	//map<Droplet*, vector<RoutePoint* > * > * newRoutes = new map<Droplet*, vector<RoutePoint* > * >();
	map<Droplet*, vector<RoutePoint* > * >::iterator routeIt = routes -> begin();
	if (debugPrint()) cout<< "[ReliabilityAwarePinMapper]" << " Total Non GV cycles = " << numGVatIndex.size() << endl;
	for(; routeIt != routes -> end(); ++routeIt)
	{
		//cout << "Vector size init = " << routeIt -> second -> size() << endl << endl;
		int sizeInit = routeIt -> second -> size();
		for(int i = 0, k = 0, n = 0; i < sizeInit; ++i)
		{
			//cout << "Vector size = " << routeIt -> second -> size() << endl;
			//cout << "i = " << i << endl;
			//cout << "k = " << k << endl << flush;
			RoutePoint* curr = routeIt -> second -> at(i + k);

			long long cycle =  curr -> cycle - k;
			//cout << cycle << endl;
			//cout << "curr -> cycle = " << curr -> cycle << endl;
			//cout << "cycle = " << cycle << endl;
			int numInserting = numGVatIndex[cycle];
			//cout << "Inserting " << numInserting << " GVs at cycle " << curr -> cycle << ", previous position " << cycle << endl;
			vector<RoutePoint*> insertions(numInserting);
			for( int j = 0; j < numInserting; ++j)
			{
					insertions[j] = new RoutePoint();
					insertions[j] -> cycle = cycle + k + j + 1;
					insertions[j] -> droplet = curr -> droplet;
					insertions[j] -> x = curr -> x;
					insertions[j] -> y = curr -> y;
					insertions[j] -> dStatus = DROP_WAIT;
			}


			if(numInserting > 0)
			{
				//cout << curr -> cycle << endl;
				insertions.back()->dStatus = curr -> dStatus;
				curr -> dStatus = DROP_WAIT;
				routeIt -> second -> insert(routeIt -> second -> begin() + i + k + 1, insertions.begin(), insertions.end());
				//cout << "Inserted " << numInserting << " GVs at cycle " << curr -> cycle << ", previous position " << cycle << endl;
				//for(int m = 0; m < insertions.size(); ++m)
				//{
				//	cout << "Insertion " << m + 1 << " cycle is " << insertions.at(m)->cycle << endl;
				//}

				k += numInserting;
				for(vector<RoutePoint* >::iterator it = routeIt -> second -> begin() + i + k + 1; it != routeIt -> second -> end(); ++it)
				{
					((*it) -> cycle) += numInserting;
				}
				n++;

			}
			//cout << "Done..." << endl << endl;
		}
		//cout << "Droplet Done..." << endl;
	}
}


/*
void ReliabilityAwarePinMapper::calculateSwitchingResults()
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
*/

MinCostMaxFlowGraph ReliabilityAwarePinMapper::constructMCMF()
{
	//Weight of constraints
	const int ALPHA = 10;
	const int BETA = 1;
	//Ground Vector function
	//f(r) = c;
	GroundVector_r = 2;
	GroundVector_c = 4;

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
	// only if they are compatible, with 1 unit cap and ALPHA * AUekpi + BETA * GVekpi cost
	for(int i = 0; i < max_clique.size(); ++i)
	{
		int idx = atoi(max_clique[i].c_str());
		vector<int> sequence(tAMatrix.at(idx));
		//for(int k = 0; k < AMatrix.size(); ++k)
		//{
		//	sequence[k] = AMatrix[k][idx];
		//}
		for(int j = 0; j < PinActivationSeqs.size(); ++j)
		{
			bool incomp = false;
			int AUekpi = 0;
			int GVekpi = 0;
			vector<int> result(seqCompare(PinActivationSeqs[j], sequence));
			if(result.empty())
			{
				incomp = true;
			}
			else
			{
				incomp = calculateGroundVectors(result, GVekpi);
				//cout << "incomp = " << incomp << endl;
				if (incomp)
					break;
				for(int k = 0; k < result.size(); ++k)
				{
					if(result[k] == 1)
					{
						++AUekpi;
					}
				}
				AUekpi *= PinActivationSeqs.size() + 1;

			}
			if(!incomp)
			{
				//cout << "AUekpi = " << AUekpi << " GVekpi = " << GVekpi << endl;
				mcmf.AddEdge(N + i + 2, j + 2, 1, ALPHA * AUekpi + BETA * GVekpi);
			}

		}
	}
	return mcmf;
}
void ReliabilityAwarePinMapper::addEdgeBasedOnConstraints(CliquePinMapper::Graph & g, long long seq1index, long long seq2index, string s1, string s2, long long size)
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




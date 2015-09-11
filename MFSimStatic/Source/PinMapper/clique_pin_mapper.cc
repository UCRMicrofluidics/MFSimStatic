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
 * Source: clique_pin_mapper.cc													*
 * Original Code Author(s): Zach Zimmerman										*
 * Original Completion/Release Date: 5/6/2014									*
 *																				*
 * Details: The graph could be optimized by changing the way nodes are added in *
 * convertMatrixToGraph(), but it wouldn't be a simple undertaking				*
 *																				*
 * Revision History:															*
 * WHO		WHEN		WHAT													*
 * ---		----		----													*
 * FML		MM/DD/YY	One-line description									*
 *-----------------------------------------------------------------------------*/
#include "../../Headers/PinMapper/clique_pin_mapper.h"
#include "../../Headers/Util/sort.h"
#include<sstream>
#include<stdlib.h>
#include <set>

using std::cerr;
using std::cout;
using std::endl;
using std::ifstream;
using std::string;



///////////////////////////////////////////////////////////////////////////////////
// Constructors
///////////////////////////////////////////////////////////////////////////////////
CliquePinMapper::CliquePinMapper()
{
	arch = NULL;
}
CliquePinMapper::CliquePinMapper(DmfbArch *dmfbArch)
{
	RAUs = 0;
	arch = dmfbArch;
}

///////////////////////////////////////////////////////////////////////////////////
// Deconstructor
///////////////////////////////////////////////////////////////////////////////////
CliquePinMapper::~CliquePinMapper()
{

}


///////////////////////////////////////////////////////////////////////////////////
// This function should be overridden; it is called just after routing.
///////////////////////////////////////////////////////////////////////////////////
void CliquePinMapper::setMapPostRoute(vector<vector<int> *> *pinActivations, map<Droplet *, vector<RoutePoint *> *> *routes)
{
	// Variables for statistics
	int post_num_activations = 0;
	int pre_num_activations = 0;

	set<int> allPins;
	int pinsBefore = arch->getPinMapper()->getNumUniquePins();
	int elecsBefore = arch->getPinMapper()->getNumElectrodes();

	vector< vector<int> > AMatrix = initActivationMatrix(pinActivations);
	for(int i = 0; i < AMatrix.size(); ++i)
	{
		for(int j = 0; j < AMatrix[i].size(); ++j)
		{
			if(AMatrix[i][j] == 1)
				pre_num_activations++;
		}
	}
	initTranspose(AMatrix);
	Graph coloringGraph = convertMatrixToGraph(AMatrix.size());

	//cerr << "CliquePinMapper: coloring...\n";
	coloringGraph.dsatur();


	vector<int> pinSizes(coloringGraph.find_max_color() + 1, 0);
	if (debugPrint()) cout << "MaxColor = " << coloringGraph.find_max_color() << endl;
	//coloringGraph.print_coloring();

	for (map<string, int >::iterator graph_it = coloringGraph.coloring.begin(); graph_it != coloringGraph.coloring.end(); graph_it++)
	{
		pinSizes[graph_it->second]++;
	}

	setPinMapping(coloringGraph);

	char * string1 = new char[10];
	vector<vector<int> > pinActivationSeqs(coloringGraph.find_max_color() + 1, vector<int>(pinActivations->size(), 0));
	//Reassign Pin Activations Based on new mapping
	for (size_t i = 0; i < pinActivations->size(); ++i)
	{
		set<int> activatedThisCycle;

		for (size_t j = 0; j < pinActivations->at(i)->size(); ++j )
		{
			if(!trivialElectrodes[pinActivations->at(i) ->at(j)])
			{
				int pin = coloringGraph.coloring.at(Util::itoa(pinActivations->at(i)->at(j), string1, 10));
				activatedThisCycle.insert(pin);
				allPins.insert(pin);
			}
		}
		pinActivations->at(i)->clear(); // Clear out the old individually addressable pins
		set<int>::iterator pinIt = activatedThisCycle.begin();
		for (; pinIt != activatedThisCycle.end(); pinIt++)
		{
			//cout << *pinIt << " ";
			pinActivations->at(i)->push_back(*pinIt);
			pinActivationSeqs.at(*pinIt).at(i) = 1;
			post_num_activations += pinSizes[*pinIt];
		}

	}
	//cout << "Entire electrode array size ( num electrodes * num activations) = " << tAMatrix.size() * pinActivations -> size() << endl;
	if (debugPrint()) cout << "[CliquePinMapper] Addressed " << pinSizes.size() << " pins." << endl;
	if (debugPrint()) cout << "[CliquePinMapper] Total redundant activations: " << post_num_activations - pre_num_activations << endl;
	if (debugPrint()) cout << "[CliquePinMapper] Total initial activations: " << pre_num_activations << endl;
	if (debugPrint()) cout << "[CliquePinMapper] Total post activations: " << post_num_activations << endl;
	cout << "[CliquePinMapper] Number of electrodes reduced from " << elecsBefore << " to " << arch->getPinMapper()->getNumElectrodes() << endl;
	cout << "[CliquePinMapper] Number of pins reduced from " << pinsBefore << " to " << arch->getPinMapper()->getNumUniquePins() << endl;

	if (debugPrint()) cout << "[CliquePinMapper] switching results:" << endl;
	if (debugPrint()) calculateSwitchingResults(pinActivationSeqs);

	numGVatIndex = vector<int>(pinActivations->size(), 0);
	for(int i = 0; i < pinActivationSeqs.size(); ++i)
	{
		addGroundVectorsToEntireAssay2(i, pinActivationSeqs);
	}
	long long numGVs = 0;
	for(int i = 0; i < numGVatIndex.size(); ++i)
	{
		numGVs += numGVatIndex.at(i);
	}
	if (debugPrint()) cout << "[CliquePinMapper] total gvs that would be inserted: " << numGVs << endl;


	// DTG Test
	/*for (size_t i = 0; i < pinActivations->size(); ++i)
	{
		cout << "Cycle " << i << ": ";
		for (size_t j = 0; j < pinActivations->at(i)->size(); ++j )
			cout << pinActivations->at(i)->at(j) << ", ";
		cout << endl;
	}*/

	//resizeDmfb();
	delete [] string1;

	//cerr << "CliquePinMapper: finished." << endl;
}
///////////////////////////////////////////////////////////////////////
// This method transposes the activation matrix so that is easier to //
// traverse.														 //
///////////////////////////////////////////////////////////////////////
void CliquePinMapper::initTranspose(const vector<vector<int> > & matrix)
{
	if(matrix.size() == 0)
		return;
	tAMatrix = vector<vector<int> >(matrix[0].size());
	for(int i = 0; i < matrix.size(); ++i)
	{
		for(int j = 0; j < matrix[i].size(); ++j)
		{
			tAMatrix[j].push_back(matrix[i][j]);
		}
	}
}

// Debug Method
void CliquePinMapper::printBoardAtActivation(int n, const vector< vector< int> > &AMatrix, ofstream &output)
{
	//ofstream output("BoardLayouts.txt");
	output << "Activation #" << n << endl << endl;
	int numRows = arch -> getNumCellsY();
	int numCols = arch -> getNumCellsX();
	for(int i = 0; i < numRows; ++i)
	{
		for(int j = 0; j < numCols; ++j)
		{
			int x = AMatrix[n][i * numCols + j];
			if(x == 2)
			{
				output << "X ";
			}
			else if (x == 1)
				output << "1 ";
			else if (x == 0)
				output << "0 ";
			else
				output << "? ";
		}
		output << endl;

	}
	output << endl;

}

void CliquePinMapper::addGroundVectorsToEntireAssay2(int seqIdx, vector<vector<int> > & PinActivationSeqs)
{
	int GroundVector_r = 4;
	int GroundVector_c = 2;
	if(GroundVector_r <= 1)
	{
		cerr << "[ReliabilityAwarePinMapper] InternalError: num Consecutive Activations Threshold(GroundVector_r) must be 2 or greater..." << endl << flush;
		return;
	}
	int numConsecActivations = 0;
	//vector< vector<int> > AMatrix = initActivationMatrix(pinActivations);
	//tAMatrix.clear();
	//initTranspose(AMatrix);

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
			//if(!criticalTimesteps.empty() && criticalTimesteps[i] && criticalTimesteps[i + 1])
			//{
			//cout << "criticalTimesteps[" << i << "] = " << criticalTimesteps[i] << endl;
			//cout << "criticalTimesteps[" << i + 1 << "] = " << criticalTimesteps[i + 1] << endl;
			//We cannot insert a ground vector here as it violates a critical operation
			// (if i happens to be the end of one critical operation and i + 1 the beginning
			// of another, technically we CAN add a ground vector in that case, but this algorithm
			// will not do that)
			//return;
			//}
			//else
			//{
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
			//}
			numConsecActivations = 0;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////
// This function creates and returns a matrix initialized to the pin activations
// specified by the router. Takes into account interference regions
///////////////////////////////////////////////////////////////////////////////////
vector<vector<int> > CliquePinMapper::initActivationMatrix(const vector<vector<int> *> *pinActivations)
{
	int numActivations = pinActivations -> size();
	if( numActivations == 0)
		return vector<vector<int> >(numActivations);

	//Get number of Electrodes
	int numElectrodes = arch ->getNumCellsX() * arch -> getNumCellsY();

	//Initialize Matrix to all "Don't Care" values
	vector<vector<int> > AMatrix(numActivations, vector<int>(numElectrodes, 2));

	//Populate Matrix with appropriate values
	for (size_t i = 0; i < pinActivations->size(); ++i)
	{
		for (size_t j = 0; j < pinActivations->at(i)->size(); ++j )
		{
			AMatrix[i][pinActivations->at(i)->at(j)] = 1;
			setInterferenceRegion(AMatrix, i, pinActivations->at(i)->at(j), false);
		}

	}
	return AMatrix;
}

int CliquePinMapper::getNumSwitches2(const vector<int> & seq)
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

void CliquePinMapper::calculateSwitchingResults(vector<vector<int> > PinActivationSeqs)
{
	vector<int> switches;
	for(int i = 0; i < PinActivationSeqs.size(); ++i)
	{
		switches.push_back(getNumSwitches2(PinActivationSeqs[i]));
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
///////////////////////////////////////////////////////////////////////////////////
// This function sets the interference region of a particular electrode at a
// particular activation time step. This function accounts for direct neighbors
// it also accounts for lookahead and lookbehind in the direction of a droplet's movement.
///////////////////////////////////////////////////////////////////////////////////
void CliquePinMapper::setInterferenceRegion(vector<vector<int> > &matrix, int timestep, int electrode, bool lookahead)
{
	int numColumns = arch -> getNumCellsX();
	int numRows = arch -> getNumCellsY();
	int column = electrode % numColumns;
	int row = electrode / numColumns;

	//Check standard interior case where droplet is not located on the border of the chip
	if(row != 0 && row != numRows - 1 && column != 0 && column != numColumns - 1)
	{
		if(matrix[timestep][electrode - numColumns - 1] == 2)
			matrix[timestep][electrode - numColumns - 1] = 0;

		if(matrix[timestep][electrode - numColumns] == 2)
			matrix[timestep][electrode - numColumns] = 0;

		if(matrix[timestep][electrode - numColumns + 1] == 2)
			matrix[timestep][electrode - numColumns + 1] = 0;

		if(matrix[timestep][electrode + 1] == 2)
			matrix[timestep][electrode + 1] = 0;

		if(matrix[timestep][electrode + numColumns + 1] == 2)
			matrix[timestep][electrode + numColumns + 1] = 0;

		if(matrix[timestep][electrode + numColumns] == 2)
			matrix[timestep][electrode + numColumns] = 0;

		if(matrix[timestep][electrode + numColumns - 1] == 2)
			matrix[timestep][electrode + numColumns - 1] = 0;

		if(matrix[timestep][electrode - 1] == 2)
			matrix[timestep][electrode - 1] = 0;



		if(timestep != matrix.size() - 1 && !lookahead)
		{
			if(matrix[timestep + 1][electrode + 1] == 1)
			{
				setInterferenceRegion(matrix, timestep, electrode + 1, true);
				//matrix[timestep][electrode + 1]
			}
			else if(matrix[timestep + 1][electrode - 1] == 1)
			{
				setInterferenceRegion(matrix, timestep, electrode - 1, true);
			}
			else if(matrix[timestep + 1][electrode + numColumns] == 1)
			{
				setInterferenceRegion(matrix, timestep, electrode + numColumns, true);
			}
			else if(matrix[timestep + 1][electrode - numColumns] == 1)
			{
				setInterferenceRegion(matrix, timestep, electrode - numColumns, true);
			}
		}


		if(timestep != 0 && !lookahead)
		{
			if(matrix[timestep - 1][electrode + 1] == 1)
			{
				setInterferenceRegion(matrix, timestep, electrode + 1, true);
				//matrix[timestep][electrode + 1]
			}
			else if(matrix[timestep - 1][electrode - 1] == 1)
			{
				setInterferenceRegion(matrix, timestep, electrode - 1, true);
			}
			else if(matrix[timestep - 1][electrode + numColumns] == 1)
			{
				setInterferenceRegion(matrix, timestep, electrode + numColumns, true);
			}
			else if(matrix[timestep - 1][electrode - numColumns] == 1)
			{
				setInterferenceRegion(matrix, timestep, electrode - numColumns, true);
			}
		}



	}
	//Check case where droplet is located on the top edge of the chip
	else if(row == 0 && column != 0 && column != numColumns - 1)
	{
		if(matrix[timestep][electrode + 1] == 2)
			matrix[timestep][electrode + 1] = 0;

		if(matrix[timestep][electrode + numColumns + 1] == 2)
			matrix[timestep][electrode + numColumns + 1] = 0;

		if(matrix[timestep][electrode + numColumns] == 2)
			matrix[timestep][electrode + numColumns] = 0;

		if(matrix[timestep][electrode + numColumns - 1] == 2)
			matrix[timestep][electrode + numColumns - 1] = 0;

		if(matrix[timestep][electrode - 1] == 2)
			matrix[timestep][electrode - 1] = 0;


		if(timestep != matrix.size() - 1 && !lookahead)
		{
			if(matrix[timestep + 1][electrode + 1] == 1)
			{
				setInterferenceRegion(matrix, timestep, electrode + 1, true);
				//matrix[timestep][electrode + 1]
			}
			else if(matrix[timestep + 1][electrode - 1] == 1)
			{
				setInterferenceRegion(matrix, timestep, electrode - 1, true);
			}
			else if(matrix[timestep + 1][electrode + numColumns] == 1)
			{
				setInterferenceRegion(matrix, timestep, electrode + numColumns, true);
			}
		}

		if(timestep != 0 && !lookahead)
		{
			if(matrix[timestep - 1][electrode + 1] == 1)
			{
				setInterferenceRegion(matrix, timestep, electrode + 1, true);
				//matrix[timestep][electrode + 1]
			}
			else if(matrix[timestep - 1][electrode - 1] == 1)
			{
				setInterferenceRegion(matrix, timestep, electrode - 1, true);
			}
			else if(matrix[timestep - 1][electrode + numColumns] == 1)
			{
				setInterferenceRegion(matrix, timestep, electrode + numColumns, true);
			}
		}
	}
	//Check case where droplet is located on the bottom edge of the chip
	else if(row == numRows - 1 && column != 0 && column != numColumns - 1)
	{
		if(matrix[timestep][electrode - numColumns - 1] == 2)
			matrix[timestep][electrode - numColumns - 1] = 0;

		if(matrix[timestep][electrode - numColumns] == 2)
			matrix[timestep][electrode - numColumns] = 0;

		if(matrix[timestep][electrode - numColumns + 1] == 2)
			matrix[timestep][electrode - numColumns + 1] = 0;

		if(matrix[timestep][electrode + 1] == 2)
			matrix[timestep][electrode + 1] = 0;

		if(matrix[timestep][electrode - 1] == 2)
			matrix[timestep][electrode - 1] = 0;

		if(timestep != matrix.size() - 1 && !lookahead)
		{
			if(matrix[timestep + 1][electrode + 1] == 1)
			{
				setInterferenceRegion(matrix, timestep, electrode + 1, true);
				//matrix[timestep][electrode + 1]
			}
			else if(matrix[timestep + 1][electrode - 1] == 1)
			{
				setInterferenceRegion(matrix, timestep, electrode - 1, true);
			}
			else if(matrix[timestep + 1][electrode - numColumns] == 1)
			{
				setInterferenceRegion(matrix, timestep, electrode - numColumns, true);
			}
		}

		if(timestep != 0 && !lookahead)
		{
			if(matrix[timestep - 1][electrode + 1] == 1)
			{
				setInterferenceRegion(matrix, timestep, electrode + 1, true);
				//matrix[timestep][electrode + 1]
			}
			else if(matrix[timestep - 1][electrode - 1] == 1)
			{
				setInterferenceRegion(matrix, timestep, electrode - 1, true);
			}
			else if(matrix[timestep - 1][electrode - numColumns] == 1)
			{
				setInterferenceRegion(matrix, timestep, electrode - numColumns, true);
			}
		}

	}
	//Check case where droplet is located on the left edge of the chip
	else if(row != 0 && row != numRows - 1 && column == 0)
	{
		if(matrix[timestep][electrode - numColumns] == 2)
			matrix[timestep][electrode - numColumns] = 0;

		if(matrix[timestep][electrode - numColumns + 1] == 2)
			matrix[timestep][electrode - numColumns + 1] = 0;

		if(matrix[timestep][electrode + 1] == 2)
			matrix[timestep][electrode + 1] = 0;

		if(matrix[timestep][electrode + numColumns + 1] == 2)
			matrix[timestep][electrode + numColumns + 1] = 0;

		if(matrix[timestep][electrode + numColumns] == 2)
			matrix[timestep][electrode + numColumns] = 0;

		if(timestep != matrix.size() - 1 && !lookahead)
		{
			if(matrix[timestep + 1][electrode + 1] == 1)
			{
				setInterferenceRegion(matrix, timestep, electrode + 1, true);
				//matrix[timestep][electrode + 1]
			}
			else if(matrix[timestep + 1][electrode + numColumns] == 1)
			{
				setInterferenceRegion(matrix, timestep, electrode + numColumns, true);
			}
			else if(matrix[timestep + 1][electrode - numColumns] == 1)
			{
				setInterferenceRegion(matrix, timestep, electrode - numColumns, true);
			}
		}

		if(timestep != 0 && !lookahead)
		{
			if(matrix[timestep - 1][electrode + 1] == 1)
			{
				setInterferenceRegion(matrix, timestep, electrode + 1, true);
				//matrix[timestep][electrode + 1]
			}
			else if(matrix[timestep - 1][electrode + numColumns] == 1)
			{
				setInterferenceRegion(matrix, timestep, electrode + numColumns, true);
			}
			else if(matrix[timestep - 1][electrode - numColumns] == 1)
			{
				setInterferenceRegion(matrix, timestep, electrode - numColumns, true);
			}
		}
	}
	//Check case where droplet is located on the right edge of the chip
	else if(row != 0 && row != numRows - 1 && column == numColumns - 1)
	{
		if(matrix[timestep][electrode - numColumns - 1] == 2)
			matrix[timestep][electrode - numColumns - 1] = 0;

		if(matrix[timestep][electrode - numColumns] == 2)
			matrix[timestep][electrode - numColumns] = 0;

		if(matrix[timestep][electrode + numColumns] == 2)
			matrix[timestep][electrode + numColumns] = 0;

		if(matrix[timestep][electrode + numColumns - 1] == 2)
			matrix[timestep][electrode + numColumns - 1] = 0;

		if(matrix[timestep][electrode - 1] == 2)
			matrix[timestep][electrode - 1] = 0;

		if(timestep != matrix.size() - 1 && !lookahead)
		{
			if(matrix[timestep + 1][electrode - 1] == 1)
			{
				setInterferenceRegion(matrix, timestep, electrode - 1, true);
			}
			else if(matrix[timestep + 1][electrode + numColumns] == 1)
			{
				setInterferenceRegion(matrix, timestep, electrode + numColumns, true);
			}
			else if(matrix[timestep + 1][electrode - numColumns] == 1)
			{
				setInterferenceRegion(matrix, timestep, electrode - numColumns, true);
			}
		}


		if(timestep != 0 && !lookahead)
		{
			if(matrix[timestep - 1][electrode - 1] == 1)
			{
				setInterferenceRegion(matrix, timestep, electrode - 1, true);
			}
			else if(matrix[timestep - 1][electrode + numColumns] == 1)
			{
				setInterferenceRegion(matrix, timestep, electrode + numColumns, true);
			}
			else if(matrix[timestep - 1][electrode - numColumns] == 1)
			{
				setInterferenceRegion(matrix, timestep, electrode - numColumns, true);
			}
		}
	}
	//Check case where droplet is located on the upper left corner of the chip
	else if(row == 0 && column == 0)
	{
		if(matrix[timestep][electrode + 1] == 2)
			matrix[timestep][electrode + 1] = 0;

		if(matrix[timestep][electrode + numColumns + 1] == 2)
			matrix[timestep][electrode + numColumns + 1] = 0;

		if(matrix[timestep][electrode + numColumns] == 2)
			matrix[timestep][electrode + numColumns] = 0;

		if(timestep != matrix.size() - 1 && !lookahead)
		{
			if(matrix[timestep + 1][electrode + 1] == 1)
			{
				setInterferenceRegion(matrix, timestep, electrode + 1, true);
				//matrix[timestep][electrode + 1]
			}
			else if(matrix[timestep + 1][electrode + numColumns] == 1)
			{
				setInterferenceRegion(matrix, timestep, electrode + numColumns, true);
			}
		}

		if(timestep != 0 && !lookahead)
		{
			if(matrix[timestep - 1][electrode + 1] == 1)
			{
				setInterferenceRegion(matrix, timestep, electrode + 1, true);
				//matrix[timestep][electrode + 1]
			}
			else if(matrix[timestep - 1][electrode + numColumns] == 1)
			{
				setInterferenceRegion(matrix, timestep, electrode + numColumns, true);
			}
		}

	}
	//Check case where droplet is located on the upper right corner of the chip
	else if(row == 0 && column == numColumns - 1)
	{
		if(matrix[timestep][electrode + numColumns] == 2)
			matrix[timestep][electrode + numColumns] = 0;

		if(matrix[timestep][electrode + numColumns - 1] == 2)
			matrix[timestep][electrode + numColumns - 1] = 0;

		if(matrix[timestep][electrode - 1] == 2)
			matrix[timestep][electrode - 1] = 0;

		if(timestep != matrix.size() - 1 && !lookahead)
		{
			if(matrix[timestep + 1][electrode - 1] == 1)
			{
				setInterferenceRegion(matrix, timestep, electrode - 1, true);
			}
			else if(matrix[timestep + 1][electrode + numColumns] == 1)
			{
				setInterferenceRegion(matrix, timestep, electrode + numColumns, true);
			}
		}


		if(timestep != 0 && !lookahead)
		{
			if(matrix[timestep - 1][electrode - 1] == 1)
			{
				setInterferenceRegion(matrix, timestep, electrode - 1, true);
			}
			else if(matrix[timestep - 1][electrode + numColumns] == 1)
			{
				setInterferenceRegion(matrix, timestep, electrode + numColumns, true);
			}
		}

	}
	//Check case where droplet is located on the lower left corner of the chip
	else if(row == numRows - 1 && column == 0)
	{
		if(matrix[timestep][electrode - numColumns] == 2)
			matrix[timestep][electrode - numColumns] = 0;

		if(matrix[timestep][electrode - numColumns + 1] == 2)
			matrix[timestep][electrode - numColumns + 1] = 0;

		if(matrix[timestep][electrode + 1] == 2)
			matrix[timestep][electrode + 1] = 0;

		if(timestep != matrix.size() - 1 && !lookahead)
		{
			if(matrix[timestep + 1][electrode + 1] == 1)
			{
				setInterferenceRegion(matrix, timestep, electrode + 1, true);
				//matrix[timestep][electrode + 1]
			}
			else if(matrix[timestep + 1][electrode - numColumns] == 1)
			{
				setInterferenceRegion(matrix, timestep, electrode - numColumns, true);
			}
		}

		if(timestep != 0 && !lookahead)
		{
			if(matrix[timestep - 1][electrode + 1] == 1)
			{
				setInterferenceRegion(matrix, timestep, electrode + 1, true);
				//matrix[timestep][electrode + 1]
			}
			else if(matrix[timestep - 1][electrode - numColumns] == 1)
			{
				setInterferenceRegion(matrix, timestep, electrode - numColumns, true);
			}
		}
	}
	//Check case where droplet is located on the lower right corner of the chip
	else if(row == numRows - 1 && column == numColumns - 1)
	{
		if(matrix[timestep][electrode - numColumns - 1] == 2)
			matrix[timestep][electrode - numColumns - 1] = 0;

		if(matrix[timestep][electrode - numColumns] == 2)
			matrix[timestep][electrode - numColumns] = 0;

		if(matrix[timestep][electrode - 1] == 2)
			matrix[timestep][electrode - 1] = 0;

		if(timestep != matrix.size() - 1 && !lookahead)
		{
			if(matrix[timestep + 1][electrode - 1] == 1)
			{
				setInterferenceRegion(matrix, timestep, electrode - 1, true);
			}
			else if(matrix[timestep + 1][electrode - numColumns] == 1)
			{
				setInterferenceRegion(matrix, timestep, electrode - numColumns, true);
			}
		}
		if(timestep != 0 && !lookahead)
		{
			if(matrix[timestep - 1][electrode - 1] == 1)
			{
				setInterferenceRegion(matrix, timestep, electrode - 1, true);
			}
			else if(matrix[timestep  - 1][electrode - numColumns] == 1)
			{
				setInterferenceRegion(matrix, timestep, electrode - numColumns, true);
			}
		}

	}
}

///////////////////////////////////////////////////////////////////////////////////
// This function takes an activation matrix created by CliquePinMapper::initActivationMatrix()
// and converts it to a Graph. Every electrode has a node and an edge occurs if the column
// of the activation matrix corresponding to a node can be matched with the column of
// the other node. Matching is O(n^2 * m) where n is the number of electrodes and m is
// the number of activations
///////////////////////////////////////////////////////////////////////////////////
CliquePinMapper::Graph CliquePinMapper::convertMatrixToGraph(const int & size)
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

	// Add a node for each electrode
	for (int i = 0; i < tAMatrix.size(); ++i)
	{
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
				g.add_edge(s1, s2);
				nomatch = false;
			}
		}
	}

	//cleanup memory
	delete [] string1;
	delete [] string2;
	return g;

}
/*
CliquePinMapper::Graph CliquePinMapper::convertMatrixToGraph(const vector< vector<int> > &matrix)
{
		Graph g;
		string s1, s2;
		bool nomatch = false;
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

		// Add a node for each electrode
		for (int i = 0; i < numElectrodes; ++i)
		{
				g.add_node( Util::itoa(i, string1, 10));
		}


		//Compare each non-trivial electrode's activations over the course of the assay and add an edge iff
		//they do not match, i.e. one electrode's is active at time step x and the other must be off
		//num = 0;
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
						if((matrix[i][j] == 0 && matrix[i][k] == 1) || (matrix[i][j] == 1 && matrix [i][k] == 0))
						{
								nomatch = true;
								break;
						}
					}
					if(nomatch)
					{
						g.add_edge(s1, s2);
						nomatch = false;
					}

			}
		}
		//cleanup memory
		delete [] string1;
		delete [] string2;
		return g;

}
 */
////////////////////////////////////////////////////////////////////////////////////
// This function sets the pin mapping for use with a wire router. It uses the
// coloring of the graph generated by CliquePinMapper::convertMatrixToGraph()
////////////////////////////////////////////////////////////////////////////////////
void CliquePinMapper::setPinMapping(Graph &coloringGraph)
{
	stringstream ss;

	int pinNo = coloringGraph.find_max_color();
	//cerr << "max color is: " << pinNo << endl;
	int numCols = arch -> getNumCellsX();
	//cerr << "numcols:" << numCols << endl;
	map<string, vector<string> > graph = coloringGraph.graph;


	//Iterate through the graph assigning electrodes the pin number
	// matching the color of their node.
	for (map<string, vector<string> >::iterator graph_it = graph.begin(); graph_it != graph.end(); graph_it++)
	{
		int color = coloringGraph.coloring.at((*graph_it).first);
		ss << (*graph_it).first;
		int electrode;
		ss >> electrode;
		ss.clear();
		//cerr << "Electrode #: " << electrode << endl;
		int erow = electrode / numCols;
		int ecol = electrode % numCols;
		//cerr << "Setting pinMapping[" << erow << "][" << ecol << "] = " << color << endl;
		if(trivialElectrodes[electrode])
			pinMapping->at(ecol)->at(erow) = -1;
		else
		{
			pinMapping->at(ecol)->at(erow) = color;
		}
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



//////////////////////////////////////////////////////////////////////////////////////////////
// The following methods are used by the nested graph class...
//////////////////////////////////////////////////////////////////////////////////////////////
//Coloring Algorithm
void CliquePinMapper::Graph::dsatur() {
	vector<string> todo;
	string max_degree = "error";
	int degree = -1;
	//find maximal degree vertex to color first and color with 0
	for(map< string, vector<string> >::iterator i = graph.begin(); i != graph.end(); i++) {
		if(degree < (int)(*i).second.size()) {
			degree = (*i).second.size();
			max_degree = (*i).first;
		}
	}
	coloring[max_degree] = 0;

	//populate the to do list with the remaining vertices
	for(map< string, vector<string> >::iterator i = graph.begin(); i != graph.end(); i++) {
		if((*i).first != max_degree) {
			coloring[(*i).first] = -1;
			todo.push_back((*i).first);
		}
	}

	while(!todo.empty()) {
		int pos = -1;
		int saturation = -1;
		string saturation_name = "error";
		vector<int> saturation_colors;
		//find the vertex with the highest saturation level
		for(unsigned i=0; i<todo.size(); i++) {
			int internal = 0;
			vector<int> internal_colors;
			//iterate over to do vertices neighbors and count how many are colroed
			for(unsigned j=0; j<graph[todo[i]].size(); j++) {
				//a lack of color is denoted with -1, so check it has an actual color
				if(coloring[graph[todo[i]][j]] != -1) {
					internal += 1;
					internal_colors.push_back(coloring[graph[todo[i]][j]]);
				}
			}
			if(saturation < internal) {
				saturation = internal;
				saturation_name = todo[i];
				saturation_colors = internal_colors;
				pos = i;
			}
		}
		//we now know the highest saturated vertex, so remove it from the to do list
		todo.erase(todo.begin()+pos);
		int max_color = 0;
		int done = 0;
		//find the lowest possible value color that isn't used in a neighbor
		while(!done) {
			done = 1;
			for(unsigned i=0; i<saturation_colors.size(); i++) {
				if(saturation_colors[i] == max_color) {
					max_color += 1;
					done = 0;
				}
			}
		}
		coloring[saturation_name] = max_color;
	}
}

//Adds an edge between two nodes
void CliquePinMapper::Graph::add_edge(string source,string sink) {
	map<string,vector<string> >::iterator source_node;
	map<string,vector<string> >::iterator sink_node;
	// Find each node. find will insert the node if it does not exist in the
	// graph
	source_node = graph.insert(pair<string,vector<string> >(source,vector<string>())).first;
	sink_node = graph.insert(pair<string,vector<string> >(sink,vector<string>())).first;
	// Add the opposite node to the edge list for each node
	(*source_node).second.push_back(sink);
	(*sink_node).second.push_back(source);
}


//Prints the coloring of the graph
void CliquePinMapper::Graph::print_coloring() {
	std::cerr << "----------DSatur Colorings----------" << endl;
	for(map< string,int >::iterator i = coloring.begin(); i != coloring.end(); i++) {
		std::cerr << (*i).first << " " << (*i).second << endl;
	}
}


//Returns the chromatic number of the graph
int CliquePinMapper::Graph::find_max_color() {
	map<string,int>::iterator color_it = coloring.begin();
	int max_color = 0;
	for (/*color_it*/;color_it != coloring.end();color_it++) {
		if ((*color_it).second > max_color) {
			max_color = (*color_it).second;
		}
	}
	return max_color;
}



const string ColorArray[114] = {"aliceblue","antiquewhite",
		"aquamarine","azure","beige","bisque","blanchedalmond","blue",
		"blueviolet","brown","brown1","brown2","brown3","brown4", //14
		"burlywood","burlywood1","burlywood2","burlywood3","burlywood4",
		"cadetblue","cadetblue1","cadetblue2","cadetblue3","cadetblue4",
		"chartreuse","chartreuse1","chartreuse2","chartreuse3","chartreuse4",
		"chocolate","chocolate1","chocolate2","chocolate3","chocolate4",
		"coral","coral1","coral2","coral3","coral4",
		"cornflowerblue","cornsilk","cornsilk1","cornsilk2","cornsilk3",
		"cornsilk4","crimson","cyan","cyan1","cyan2",
		"cyan3","cyan4","darkgoldenrod","darkgoldenrod1","darkgoldenrod2",
		"darkgoldenrod3","darkgoldenrod4","darkgreen","darkkhaki","darkolivegreen",
		"darkolivegreen1","darkolivegreen2","darkolivegreen3","darkolivegreen4","darkorange", //64
		"darkorange1","darkorange2","darkorange3","darkorange4","darkorchid",
		"darkorchid1","darkorchid2","darkorchid3","darkorchid4","darksalmon",
		"darkseagreen","darkseagreen1","darkseagreen2","darkseagreen3","darkseagreen4",
		"darkslateblue","darkslategray","darkslategray1","darkslategray2","darkslategray3",
		"darkslategray4","darkslategrey","darkturquoise","darkviolet","deeppink",
		"deeppink1","deeppink2","deeppink3","deeppink4","deepskyblue",
		"deepskyblue1","deepskyblue2","deepskyblue3","deepskyblue4","dimgray",
		"dimgrey","dodgerblue","dodgerblue1","dodgerblue2","dodgerblue3",
		"dodgerblue4","firebrick","firebrick1","firebrick2","firebrick3",
		"firebrick4","floralwhite","forestgreen","gainsboro","ghostwhie"};

//Returns a color
string CliquePinMapper::Graph::get_color_string(int color,int max_color) {
	return ColorArray[color];
	const int MaxColor = 1023;
	color = color * (MaxColor / max_color);
	std::stringstream color_changer;
	color_changer << "0x" << std::hex << color;
	return color_changer.str();
}


//Outputs the graph to a dot file for use with GraphViz software
void CliquePinMapper::Graph::write_graph(string graph_name, bool colored) {
	if (graph_name.empty()) {
		graph_name = "colored_graph";
	}
	string filename = graph_name+".dot";
	ofstream outfile(filename.c_str());
	if (!outfile.is_open()) {
		cerr << "Error: Unable to open \"" << filename << "\"." << endl;
		return;
	}
	outfile << "graph " << graph_name << " {\n";
	//int max_color = find_max_color();
	map <string,vector<string> >::iterator graph_it;
	graph_it = graph.begin();
	for (/*graph_it*/;graph_it != graph.end();graph_it++) {
		outfile << (*graph_it).first << "[label=\"" << (*graph_it).first << "\\n"
				<< coloring.at((*graph_it).first) << "\"";
		/*if (colored) {
			outfile << " style=filled fillcolor=\"";
			outfile << get_color_string(coloring.at((*graph_it).first),max_color);
			outfile << "\"";
		}*/
		outfile << "];\n";
	}
	graph_it = graph.begin();
	for (/*graph_it*/;graph_it != graph.end();graph_it++) {
		string start_node = (*graph_it).first;
		vector<string> connections = (*graph_it).second;
		for (unsigned i = 0;i < connections.size();i++) {
			if (connections.at(i) < start_node) {
				outfile << start_node << " -- " << connections.at(i) << endl;
			}
		}
	}
	outfile << "}" << endl;
	outfile.close();
}


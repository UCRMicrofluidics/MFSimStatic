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
 * Name: Flow Network Pin Mapper												*
 * 																				*
 * Details: This pin mapper serves as an abstract base class for pin mappers	*
 * that use a min cost max flow graph to generate a pin mapping					*
 *-----------------------------------------------------------------------------*/
#ifndef FLOW_NETWORK_PIN_MAPPER_H_
#define FLOW_NETWORK_PIN_MAPPER_H_

#include "clique_pin_mapper.h"
#include "../Models/min_cost_max_flow_graph.h"

class FlowNetworkPinMapper : public CliquePinMapper
{
private:

protected:
	long long post_num_activations;
	long long pre_num_activations;
	vector<string> max_clique;
	map<string,int> addressedElectrodes;
	vector<vector<string> > currentPins;
	vector<vector<int> > PinActivationSeqs;
	vector<vector<string> > cliques;
	virtual MinCostMaxFlowGraph constructMCMF() = 0;
	virtual void addEdgeBasedOnConstraints(CliquePinMapper::Graph & g, long long seq1index, long long seq2index, string s1, string s2, long long size) = 0;
	virtual void setPinActivations(vector<vector<int> *>* pinActivations);
	virtual CliquePinMapper::Graph convertMatrixToGraph(const int & size);
	virtual vector<vector< string > > convertToCliques(CliquePinMapper::Graph & g);
	virtual vector<string> findAndDeleteMaxClique();
	virtual void setPinMapping();
	vector<int> seqCompare(const vector<int> & s1, const vector<int> & s2);
	virtual void processAndAddressElectrodes();
	void calculateSwitchingResults();
	int getNumSwitches(const vector<int> & seq);
	void addGroundVectorsToEntireAssay(int seqIdx);
	vector<int> numGVatIndex;
	int GroundVector_r;

	int GroundVector_c;
	map<int,bool> criticalTimesteps;
};



#endif /* FLOW_NETWORK_PIN_MAPPER_H_ */

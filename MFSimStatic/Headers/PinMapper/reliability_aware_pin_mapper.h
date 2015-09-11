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
 * Name: Reliability Aware Pin Mapper											*
 *																				*
 * Inferred from the following paper:											*
 * Authors:	Tsung-Wei Huang, Tsung-Yi Ho, Krishnendu Chakrabarty				*
 * Title: Reliability-oriented broadcast electrode-addressing for				*
 * pin-constrained digital microfluidic biochips								*
 * Publication Details: ICCAD 2011: 448-455
 *																				*
 * Details:																		*
 *-----------------------------------------------------------------------------*/

#ifndef RELIABILITY_AWARE_PIN_MAPPER_H_
#define RELIABILITY_AWARE_PIN_MAPPER_H_

#include "flow_network_pin_mapper.h"
#include <vector>

class ReliabilityAwarePinMapper : public FlowNetworkPinMapper
{
public:

	ReliabilityAwarePinMapper();
	ReliabilityAwarePinMapper(DmfbArch *dmfbArch);
	void setMapPostRoute(vector<vector<int>* >* pinActivations, map<Droplet *, vector<RoutePoint *> *> *routes);
	void processAndAddressMaxClique(vector<vector<int> *> *pinActivations, vector< vector<int> > & AMatrix);
	bool calculateGroundVectors(const vector<int> & seq, int & GV);
	//void addGroundVectorsToEntireAssay(int seqIdx);
	//void calculateSwitchingResults();
	//int getNumSwitches(const vector<int> & seq);
	void rerouteDroplets(map<Droplet*, vector<RoutePoint* > * > * routes);
	virtual MinCostMaxFlowGraph constructMCMF();
	virtual void addEdgeBasedOnConstraints(CliquePinMapper::Graph & g, long long seq1index, long long seq2index, string s1, string s2, long long size);
private:
	//TODO: This needs to be filled somehow


};

#endif /* RELIABILITY_AWARE_PIN_MAPPER_H_ */

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
 * Name: Pin Mapper with Wire Router											*
 *																				*
 * Inferred from the following paper:											*
 * Authors: Shang-Tsung Yu, Sheng-Han Yeh, Tsung-Yi Ho							*
 * Title: Reliability-Driven Chip-Level Design for High-Frequency Digital		*
 * Microfluidic Biochips														*
 * Publication Details: IEEE Trans. on CAD of Integrated Circuits and			*
 * Systems 34(4): 529-539 (2015)												*
 * 																				*
 * Details:																		*
 *-----------------------------------------------------------------------------*/

#ifndef SWITCHING_AWARE_PM_WR_H_
#define SWITCHING_AWARE_PM_WR_H_

#include "clique_pin_mapper.h"
#include "../WireRouter/yeh_wire_router.h"
#include "../WireRouter/path_finder_wire_router.h"
#include <vector>
#include <set>
#include "../WireRouter/paths.h"
#include "wire_router_combined_pin_mapper.h"

// *** Both of these work, just change to match in constructor too
//class SwitchingAwarePMWR : public CombinedPinMapper<YehWireRouter>
class SwitchingAwarePMWR : public CombinedPinMapper<PathFinderWireRouter>
{
public:

	SwitchingAwarePMWR();
	SwitchingAwarePMWR(DmfbArch *dmfbArch);
	virtual ~SwitchingAwarePMWR();

	virtual void setMapPostRoute(vector<vector<int> *> *pinActivations, map<Droplet *, vector<RoutePoint *> *> *routes);
	int calculateSwitchingLowerBound();
	void precomputeAllSwitches();
	int fullRun(const vector<vector<int> >& AMatrix, int P_MAX, bool finalRun);
	MinCostMaxFlowGraph constructMCMF();
	void addEdgeBasedOnConstraints(CliquePinMapper::Graph & g, long long seq1index, long long seq2index, string s1, string s2, long long size);
private:
	vector<int> timesSwitched;
	vector<vector<int> > allSwitches;
	int STMax;

};

#endif /* SWITCHING_AWARE_PM_WR_H_ */

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
 * Type: Scheduler																*
 * Name: Force-Directed List Scheduler											*
 *																				*
 * Detailed in the following paper:												*
 * Authors: Kenneth O' Neal, Dan Grissom and Philip Brisk						*
 * Title: Force-directed list scheduling for digital microfluidic biochips		*
 * Publication Details: In Proc. VLSI-SOC, Santa Cruz, CA, 2012					*
 * 																				*
 * Details: Uses force based calculations to determine priorities and then uses	*
 * list scheduling to do the final scheduling									*
 *-----------------------------------------------------------------------------*/

#ifndef FDL_SCHEDULER_H_
#define FDL_SCHEDULER_H_


#include "../../Headers/Scheduler/scheduler.h"
#include "../Scheduler/priority.h"
#include "../Resources/structs.h"
#include <string.h>
#include <math.h>
#include "../Scheduler/list_scheduler.h"

#include <iostream>
#include <algorithm>
#include <functional>
#include <vector>
#include <time.h>

struct n_vals
{
	AssayNode* n;
	vector<float> poss_TS;
	float prior_val;
};

struct n_forces
{
	AssayNode* n;
	float force;
	unsigned TS;
};

class AssayNode;

class FDLScheduler : public Scheduler
{


	private:

	// Methods
	IoResource * getReadyDispenseWell(string fluidName, unsigned long long schedTS);
	bool moreNodesToSchedule();


	unsigned uniform_deviate ( int seed );
	void setCritPathDist(DAG *dag, DmfbArch *arch);
	void recursiveCPD(DmfbArch *arch, AssayNode *node, unsigned childDist);
	void reset_priority(DAG *dag);
	void setAsLongestPathDist(DAG *dag,DmfbArch *arch);
	void setAsLongestPathDistT(DAG *dag, DmfbArch * arch, float max_latency);
	void recursiveLPD(AssayNode *node, unsigned childDist,DmfbArch *arch);
	void recursiveLPDT(AssayNode *node, unsigned childDist, DmfbArch * arch, float max_latency);
	vector< pair< AssayNode *, float> > ASAP(DAG * dag, DmfbArch *arch);
	vector< pair< AssayNode *, float> > ALAP(DAG * dag, float max_latency, DmfbArch *arch);
	vector< n_vals* > DG_Inputs(vector< pair< AssayNode *, float> >, vector< pair< AssayNode *, float> >);
	pair< vector<unsigned>, vector<float> > DG_evaluate(vector<n_vals*>);
	vector <n_forces*> force_calc (vector < n_vals* > np, pair <vector<unsigned>, vector<float> > DG);
	void Dag_reinitialize(DAG* dag);
	void commissionDAG(DAG *dag);



	//constructors
	public:
	FDLScheduler();
	virtual ~FDLScheduler();


	//methods
	unsigned long long schedule(DmfbArch *arch, DAG *dag);
	unsigned long long list_schedule(DmfbArch *arch, DAG *dag);
};


#endif /* FDL_SCHEDULER_H_ */

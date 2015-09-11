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
 * Name: Genetic Path Scheduler													*
 *																				*
 * Detailed in the following paper:												*
 * Authors: Kenneth O'Neal														*
 *-----------------------------------------------------------------------------*/
#ifndef _GENET_PATH_SCHEDULER_H
#define _GENET_PATH_SCHEDULER_H

#include "scheduler.h"
#include "priority.h"
#include <string.h>
#include <map>
#include "scheduler.h"
#include <math.h>

#include <iostream>
#include <algorithm>
#include <functional>
#include <vector>
#include <time.h>
#include "path_scheduler.h"
#include "../Testing/elapsed_timer.h"

class AssayNode;

class GenetPathScheduler : public Scheduler{

private:
//GENET PRIVATES
	// Variables
	map<AssayNode*, unsigned>* chromosome;
	map<AssayNode*, unsigned> mutationset; //top percentage for mutation (subset of chromosome)
	map<AssayNode*, unsigned> crossover_set;
	map<AssayNode*, unsigned> reproduce_set;

	// "Hard-coded" Constants
	unsigned get_init_pop()	{ return 100; } //sets initial population size
	unsigned get_num_generations() { return 1000; } // sets the number of generations
	unsigned get_rep_amnt()	{ return (get_init_pop() * .125); } //sets percentage of population to be reproduced
	unsigned get_cross_amnt() { return (get_init_pop() * .625); } //sets percentage of population to be crossed over
	unsigned get_mut_amnt() { return (get_init_pop() * .25); } //sets percentage of population to be mutated

	// Methods
	IoResource * getReadyDispenseWell(string fluidName, unsigned long long schedTS);
	bool moreNodesToSchedule();

	vector< map<AssayNode*, unsigned> *> * reproduction(vector< map<AssayNode*, unsigned> *> * popA, vector<unsigned> * TS);
	void setAsRand(vector< map<AssayNode*, unsigned> *>::iterator it, vector< map<AssayNode*, unsigned> *> * pop);
	vector< map<AssayNode*, unsigned> *> * crossover(vector< map<AssayNode*, unsigned> *> * popA, DAG * dag);
	vector< map<AssayNode*, unsigned> *> * mutation(DAG * dag);
	vector< map<AssayNode*, unsigned> *> *initialize_population(DAG * dag, int A);
	map<AssayNode*, unsigned> *rand_key( DAG *dag);
	vector< map<AssayNode*, unsigned> *> * return_pop();
	vector< map<AssayNode*, unsigned> *> * get_pop();
	unsigned uniform_deviate ( int seed );
	void Dag_reinitialize(DAG* dag);
	void commissionDAG(DAG *dag);

/* ///PATH PRIVATES
	bool assayComplete;

protected:
//PATH PROTECTED
		// Members
		map<unsigned long long, ResHist*> *availResAtTS; // Contains the available resources for specific TS
		map<unsigned long long, list<AssayNode*>* > *indefStorage; // Contains leader nodes that are being stored indefinitely


		// Methods
		ResHist *createNewResHist(unsigned long long ts);
		ResHist *getResHistAtTS(unsigned long long ts);
		int getNumNewIndefStorDrops(unsigned long long ts);
		int getNumIndefStorDropsBefore(unsigned long long ts);
		list<AssayNode *> *getIndefStorDropsBefore(unsigned long long ts);
		bool resTypeExists(ResourceType rt);
		int getNumFreeModules(unsigned long long ts, int runningStorageDrops);
		bool resTypeIsAvailAtTS(unsigned long long ts, ResourceType rt, int runningStorageDrops);
		bool canStoreDropsAtTS(unsigned long long ts, int numDrops);
		void setActiveAtTS(IoResource *input, unsigned long long ts);
		void setInactiveAtTS(IoResource *input, unsigned long long ts);
		void setPotentialRes(vector<ResourceType> *potRes, OperationType ot);

		virtual IoResource * getReadyDispenseWell(string fluidName, unsigned long long schedTS, unsigned long long dagStartTS);
//GENET PROTECTED

 */
public:
//GENET PUBLIC
	//constructors
	GenetPathScheduler();
	virtual ~GenetPathScheduler();

	//methods
	unsigned long long schedule(DmfbArch *arch, DAG *dag);
//PATH PUBLIC
};
#endif



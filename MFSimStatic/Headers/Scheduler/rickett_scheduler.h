/*------------------------------------------------------------------------------*
 *                       (c)2013, All Rights Reserved.     						*
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
 * Name: Rickett's Genetic-Algorithm Based Scheduler							*
 *																				*
 * Inferred from the following paper:											*
 * Authors: Andrew Ricketts, Keven Irick, N. Vijaykrishnan and Mary Jane Irwin	*
 * Title: Priority scheduling in digital microfluidics-based biochips			*
 * Publication Details: In Proc. DATE, Munich, Germany, 2006					*
 * 																				*
 * Details: Genetic algorithm was inferred from the above paper.  This			*
 * algorithm considers input reservoirs in it's computations and was originally	*
 * shown to produce better results on the in-vitro diagnostics assays			*
 *-----------------------------------------------------------------------------*/

#ifndef RICKETT_SCHEDULER_H_
#define RICKETT_SCHEDULER_H_

#include "../Scheduler/priority.h"
#include "../Resources/structs.h"
#include "scheduler.h"
#include <string.h>
#include <math.h>
#include "../Scheduler/list_scheduler.h"

#include <iostream>
#include <algorithm>
#include <functional>
#include <vector>
#include <time.h>
#include "../Testing/elapsed_timer.h"

class AssayNode;

class RickettScheduler : public Scheduler
{


private:
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
	void setAsRand(vector< map<AssayNode*, unsigned> *>::iterator  pop);
	vector< map<AssayNode*, unsigned> *> * crossover(vector< map<AssayNode*, unsigned> *> * popA, DAG * dag);
	vector< map<AssayNode*, unsigned> *> * mutation(DAG * dag);
	vector< map<AssayNode*, unsigned> *> *initialize_population(DAG * dag, int A);
	map<AssayNode*, unsigned> *rand_key( DAG *dag);
	vector< map<AssayNode*, unsigned> *> * return_pop();
	vector< map<AssayNode*, unsigned> *> * get_pop();
	unsigned uniform_deviate ( int seed );
	void Dag_reinitialize(DAG* dag);
	void commissionDAG(DAG *dag);

public:
	//constructors
	RickettScheduler();
	virtual ~RickettScheduler();

	//methods
	unsigned long long schedule(DmfbArch *arch, DAG *dag);
};
#endif /* RICKETT_SCHEDULER_H_ */

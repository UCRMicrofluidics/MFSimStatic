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
 * Name: FPPC Path Scheduler													*
 *																				*
 * Derived from work in the following paper:									*
 * Authors: Dan Grissom and Philip Brisk										*
 * Title: Path scheduling on digital microfluidic biochips						*
 * Publication Details: In Proc. DAC, San Francisco, CA, 2012					*
 * 																				*
 * Details: Schedules assays by 1 path-at-a-time instead of 1 node-at-a-time 	*
 * (e.g. list-scheduler). This scheduler is best suited for assays with high	*
 * fan-out and fast dispense operations and is compatible with the FPPC devices	*
 * which have SSD modules.														*
 *-----------------------------------------------------------------------------*/
#ifndef _FPPC_PATH_SCHEDULER_H
#define _FPPC_PATH_SCHEDULER_H

#include "scheduler.h"
#include "priority.h"
#include <string.h>
#include <map>

//#include <vector>

class GrissomFppcPathScheduler : public Scheduler{
    protected:
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
		//int getNumFreeModules(unsigned long long ts, int runningStorageDrops);
		bool resTypeIsAvailAtTS(unsigned long long ts, OperationType ot, ResourceType rt, int runningStorageDrops);
		bool canStoreDropsAtTS(unsigned long long ts, int numDrops);
		void setActiveAtTS(IoResource *input, unsigned long long ts);
		void setInactiveAtTS(IoResource *input, unsigned long long ts);
		void setPotentialRes(vector<ResourceType> *potRes, OperationType ot);

		virtual IoResource * getReadyDispenseWell(string fluidName, unsigned long long schedTS, unsigned long long dagStartTS);

    public:
		// Constructors
		GrissomFppcPathScheduler();
        ~GrissomFppcPathScheduler ();

        // Methods
        unsigned long long schedule(DmfbArch *arch, DAG *dag);
};
#endif

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
 * Name: General Base Scheduler													*
 *																				*
 * Not inferred or detailed in any publications									*
 * 																				*
 * Details: This is a base scheduler class which contains some basic members	*
 * and functions useful to all schedulers.  It must be inherited from when		*
 * creating a new scheduler in order to keep the structure of the simulator		*
 * and to keep the code clean and modular.										*
 *-----------------------------------------------------------------------------*/
#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include "../Models/dmfb_arch.h"
#include <string.h>
#include "../Resources/structs.h"
#include "../Testing/claim.h"
#include "../Resources/enums.h"

class DmfbArch;
class DAG;

class Scheduler
{
	protected:
		// Variables
		int availRes[RES_TYPE_MAX+1]; // Tells the available module resource availability (not dispense/output info)
		vector<IoResource*> *dispRes;
		vector<IoResource*> *outRes;
		list<AssayNode*> *candidateOps; // Holds AssayNodes that can be bound now or very soon
		list<AssayNode*> *unfinishedOps; // To be used while scheduling
		list<AssayNode*> *executingOps; // To be used while executing (in maintainSchedule() function)
		bool internalPriorities; // True if list scheduler will set priorities itself; false if an external class/scheduler already set the priorities
		int maxStoragePerModule; // Number of droplets a single module is able to store
		SchedulerType type; // Specific scheduler type of this instance of scheduler
		bool hasExecutableSyntesisMethod; // Tells if method contains code to execute, or if it is just a shell

		// Methods
		int getAvailResources(DmfbArch *arch);
		void resetIoResources(DmfbArch *arch);
		void commissionDAG(DAG *dag);
		bool moreNodesToSchedule();

	public:
		// Constructors
		Scheduler();
		virtual ~Scheduler();

		// Methods
		virtual unsigned long long schedule(DmfbArch *arch, DAG *dag);

		// Getters/Setters
		void setPrioritiesExternally() { internalPriorities = false; }
		int getMaxStoragePerModule() { return maxStoragePerModule; }
		void setMaxStoragePerModule(int drops) { maxStoragePerModule = drops; }
		SchedulerType getType() { return type; }
		void setType(SchedulerType st) { type = st; }
		bool hasExecutableSynthMethod() { return hasExecutableSyntesisMethod; }
		void setHasExecutableSynthMethod(bool hasMethod) { hasExecutableSyntesisMethod = hasMethod; }

};
#endif /* SCHEDULER_H_ */

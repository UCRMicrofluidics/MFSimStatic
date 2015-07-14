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
 * Type: Placer																	*
 * Name: General Base Placer													*
 *																				*
 * Not inferred or detailed in any publications									*
 * 																				*
 * Details: This is a base placer class which contains some basic members		*
 * and functions useful to all placers.  It must be inherited from when			*
 * creating a new placer in order to keep the structure of the simulator		*
 * and to keep the code clean and modular.										*
 *-----------------------------------------------------------------------------*/
#ifndef PLACER_H_
#define PLACER_H_

#include "../Models/dmfb_arch.h"
#include "../Testing/claim.h"

class DAG;

class Placer
{
	public:
		// Constructors
		Placer();
		virtual ~Placer();

		// Methods
		virtual void place(DmfbArch *arch, DAG *dag, vector<ReconfigModule *> *rModules);

		// Getters/Setters
		int getMaxStoragePerModule() { return maxStoragePerModule; } // Not sure what class this info should really belong to, will hard-code for now
		void setMaxStoragePerModule(int drops) { maxStoragePerModule = drops; }
		int getHCellsBetweenModIR() { return hCellsBetweenModIR; }
		void setHCellsBetweenModIR(int cells) { hCellsBetweenModIR = cells; }
		int getVCellsBetweenModIR() { return vCellsBetweenModIR; }
		void setVCellsBetweenModIR(int cells) { vCellsBetweenModIR = cells; }
		SchedulerType getPastSchedType() { return pastSchedType; }
		PlacerType getType() { return type; }
		void setPastSchedType(SchedulerType st) { pastSchedType = st; }
		void setType(PlacerType pt) { type = pt; }
		bool hasExecutableSynthMethod() { return hasExecutableSyntesisMethod; }
		void setHasExecutableSynthMethod(bool hasMethod) { hasExecutableSyntesisMethod = hasMethod; }

	protected:
		// Members
		int maxStoragePerModule; // Number of droplets a single module is able to store
		int hCellsBetweenModIR; // # of cells, on x-axis, between module interference regions; should be non-negative
		int vCellsBetweenModIR; // # of cells, on y-axis, between module interference regions; should be non-negative
		vector<IoResource*> *dispRes; // Dispense reservoir resources
		vector<IoResource*> *outRes; // Output reservoir resources
		vector<FixedModule *> availRes[RES_TYPE_MAX+1]; // Tells the module resource availability (not dispense/output info)
		SchedulerType pastSchedType;
		PlacerType type;
		bool hasExecutableSyntesisMethod; // Tells if method contains code to execute, or if it is just a shell

		// Methods
		void resetIoResources(DmfbArch *arch);
		void getAvailResources(DmfbArch *arch);
		void bindInputsLE(list<AssayNode *> *inputNodes);
		void bindOutputsLE(list<AssayNode *> *outputNodes);

};
#endif /* PLACER_H_ */

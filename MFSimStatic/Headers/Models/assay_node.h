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
/*--------------------------------Class Details---------------------------------*
 * Name: AssayNode (Assay Node)													*
 *																				*
 * Details: The basic node for the DAG.  Each AssayNode represents an operation	*
 * in an assay (DAG). Thus, the AssayNode has many members to be able to 		*
 * represent any operation (e.g. ioPort, reading, storageOps, etc.).			*
 *-----------------------------------------------------------------------------*/
#ifndef _ASSAY_NODE_H
#define _ASSAY_NODE_H

using namespace std;
#include "../Models/reconfig_module.h"
#include "../Models/fixed_module.h"
#include "../Models/io_port.h"
#include "droplet.h"
#include <iostream>
#include <stdlib.h>
#include "entity.h"
#include "../Resources/enums.h"
#include <vector>
#include <string>
#include "dag.h"
#include <list>

class DAG;
class ReconfigModule;

class AssayNode : public Entity
{
	protected:
		// Variables
		ReconfigModule *reconfigMod; // describes the location at which the module has been placed (non DISPENSE/OUTPUT nodes)
		unsigned long long startTimeStep; // inclusive
		unsigned long long endTimeStep; // exclusive, so if it starts on 1 and the endTimeStep is 3, the operation is 2 cycles
		int numDrops;// mix: # drops before mix; split: # drops after split; store: # drops during
		IoPort *ioPort; // The IO Port the node is bound to if is a DISPENSE/OUTPUT node
		double reading; // Used for detect node sensor readings
		unsigned long long startCycle;
		ResourceType boundedResType;
		list<AssayNode*> storageOps;
		vector<AssayNode*> children;
		vector<AssayNode*> parents;
		vector<Droplet *> droplets;
		AssayNodeStatus status;
		AssayNode *transfer;
		OperationType type;
		static int next_id;
		unsigned priority;
		unsigned cycles;
		string portName;
		unsigned order;
		double seconds;
		double volume;
		string name;
		DAG *dag;
		FixedModule *boundedExternalRes;

	public:
		// Constructors
		AssayNode(OperationType opType);
		virtual ~AssayNode();

		//Getters/Setters
		OperationType GetType() { return type; }
		AssayNodeStatus GetStatus() { return status; }
		void SetStatus(AssayNodeStatus Stat) { status = Stat; }
		vector<AssayNode*> GetChildren() { return children; }
		vector<AssayNode*> GetParents() { return parents; }
		vector<Droplet *> GetDroplets() { return droplets; }
		string GetPortName() { return portName; }
		void SetStartCycle(unsigned long long cycle) { startCycle = cycle; }
		unsigned GetPriority() { return priority; }
		void SetPriority(unsigned prior) { priority = prior; }
		string GetName() { return name; }
		unsigned GetNumCycles() { return cycles; }
		unsigned long long GetStartCycle() { return startCycle; }
		unsigned long long GetStartTS() { return startTimeStep; }
		unsigned long long GetEndTS() { return endTimeStep; }
		double GetReading() { return reading; }
		string Print();
		string GetSummary();
		DAG *GetDAG() { return dag; }
		double GetNumSeconds() { return seconds; }
		void SetNumCycles (unsigned numCycles) { cycles = numCycles; }
		ReconfigModule *GetReconfigMod() { return reconfigMod; }
		IoPort *GetIoPort() { return ioPort; }
		double GetVolume () { return volume; }
		FixedModule *GetBoundedExternalRes() { return boundedExternalRes; }

		// Utility
		bool isMaintainingDroplet(Droplet * drop);
		void eraseFromDropList(Droplet *drop);
		void addDroplet(Droplet *drop) { droplets.push_back(drop); }

		// Friend Classes
		friend class LeftEdgeBinder;
		friend class Priority;
		friend class Analyze;
		friend class FileOut;
		friend class FileIn;
		friend class DAG;

		friend class RealTimeEvalListScheduler;
		friend class GrissomFppcPathScheduler;
		friend class GrissomFppcScheduler;
		friend class RickettScheduler;
		friend class GenetScheduler;
		friend class ListScheduler;
		friend class PathScheduler;
		friend class FDLScheduler;

		friend class GrissomFppcLEBinder;
		friend class GrissomPathBinder;
		friend class GrissomLEBinder;
		friend class KamerLlPlacer;
		friend class SAPlacer;
		friend class Placer;

		friend class PostSubproblemCompactionRouter;
		friend class GrissomFixedPlaceMapRouter;
		friend class GrissomFppcSequentialRouter;
		friend class GrissomFppcParallelRouter;
		friend class GrissomFixedPlaceRouter;
		friend class RoyMazeRouter;
		friend class AStarRouter;
		friend class BioRouter;
		friend class ChoRouter;
		friend class LeeRouter;
		friend class SkyCalRouter;

		friend class Test;
};
#endif

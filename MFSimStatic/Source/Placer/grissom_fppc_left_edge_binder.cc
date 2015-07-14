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
/*---------------------------Implementation Details-----------------------------*
 * Source: grissom_fppc_left_edge_binder.cc										*
 * Original Code Author(s): Dan Grissom											*
 * Original Completion/Release Date: December 11, 2012							*
 *																				*
 * Details: N/A																	*
 *																				*
 * Revision History:															*
 * WHO		WHEN		WHAT													*
 * ---		----		----													*
 * FML		MM/DD/YY	One-line description									*
 *-----------------------------------------------------------------------------*/
#include "../../Headers/Placer/grissom_fppc_left_edge_binder.h"
#include "../../Headers/Util/sort.h"

///////////////////////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////////////////////
GrissomFppcLEBinder::GrissomFppcLEBinder()
{
}

///////////////////////////////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////////////////////////////
GrissomFppcLEBinder::~GrissomFppcLEBinder()
{
}

///////////////////////////////////////////////////////////////////////////////////
// Places the scheduled DAG (contained by synthesis) according to the demo
// placement algorithm.

// This algorithm works more like a binder as it assumes that the module locations
// are essentially fixed each time-cycle. Thus, the placer is really just binding
// scheduled operations to available work areas.
///////////////////////////////////////////////////////////////////////////////////
void GrissomFppcLEBinder::place(DmfbArch *arch, DAG *schedDag, vector<ReconfigModule *> *rModules)
{
	getAvailResources(arch);
	resetIoResources(arch);

	int maxStoragePerModule = 1; // Hardcoded for this placer...will always be one for Design #1

	int INPUT_RES = RES_TYPE_MAX + 1;
	int OUTPUT_RES = RES_TYPE_MAX + 2;

	vector< list<AssayNode*> * > opsByResType; // Operations distinguished by operation types
	list<AssayNode*> storeList;
	for (int i = 0; i <= RES_TYPE_MAX + 2; i++)// +2 for inputs and outputs
		opsByResType.push_back(new list<AssayNode*>());
	for (unsigned i = 0; i < schedDag->getAllNodes().size(); i++)
	{
		AssayNode *n = schedDag->getAllNodes().at(i);
		if (n->GetType() == DISPENSE)
			opsByResType.at(INPUT_RES)->push_back(n);
		else if (n->GetType() == OUTPUT)
			opsByResType.at(OUTPUT_RES)->push_back(n);
		else if (n->GetType() != STORAGE)
			opsByResType.at(n->boundedResType)->push_back(n);
		else if (n->GetType() == STORAGE)
		{
			n->boundedResType = SSD_RES; // Can hardcode b/c this particular design will always use this resource to store
			opsByResType.at(n->boundedResType)->push_back(n);
			//storeList.push_back(n);
		}
	}

	for (int i = 0; i <= RES_TYPE_MAX; i++)
		Sort::sortNodesByStartTSThenStorageFirst(opsByResType.at(i));
		//Sort::sortNodesByStartTS(opsByResType.at(i));

	// Do Left-Edge binding for modules
	for (int i = 0; i <= RES_TYPE_MAX; i++)
	{
		ResourceType rt = (ResourceType)i;
		for (unsigned j = 0; j < availRes[i].size(); j++)
		{
			FixedModule *fm = availRes[i].at(j);
			//modSchedule[fm] = sched;

			unsigned long long lastEnd = 0;
			list<AssayNode *> scheduled;
			list<AssayNode *>::iterator it = opsByResType.at(rt)->begin();
			for (; it != opsByResType.at(rt)->end(); it++)
			{
				AssayNode *n = *it;

				// Handle splits later
				if (n->GetStartTS() >= lastEnd && n->type != SPLIT)
				{
					scheduled.push_back(n);
					lastEnd = n->GetEndTS();
					n->status = BOUND;
					n->reconfigMod = new ReconfigModule(fm->getResourceType(), fm->getLX(), fm->getTY(), fm->getRX(), fm->getBY());
					n->reconfigMod->startTimeStep = n->startTimeStep;
					n->reconfigMod->endTimeStep = n->endTimeStep;
					n->reconfigMod->boundNode = n;
					n->reconfigMod->setTileNum(fm->getTileNum());
					//n->reconfigMod->linkedModule = NULL;
					rModules->push_back(n->reconfigMod);
				}
			}
			while (!scheduled.empty())
			{
				opsByResType.at(rt)->remove(scheduled.front());
				scheduled.pop_front();
			}
		}
	}

	// Since splits are "instantaneous", bind splits to same module
	// as one of their children as a post processing step
	for (unsigned i = 0; i < schedDag->splits.size(); i++)
	{
		AssayNode *s = schedDag->splits.at(i);
		AssayNode *sc = s->children.front();
		ReconfigModule *scMod = sc->reconfigMod;

		s->status = BOUND;
		s->reconfigMod = new ReconfigModule(scMod->resourceType, scMod->getLX(), scMod->getTY(), scMod->getRX(), scMod->getBY());
		s->reconfigMod->startTimeStep = s->startTimeStep;
		s->reconfigMod->endTimeStep = s->endTimeStep;
		s->reconfigMod->boundNode = s;
		s->reconfigMod->setTileNum(scMod->getTileNum());
		rModules->push_back(s->reconfigMod);
	}

	/////////////////////////////////////////////////////////////
	// Now do simple Left-Edge binding for inputs/outputs
	bindInputsLE(opsByResType.at(INPUT_RES));
	bindOutputsLE(opsByResType.at(OUTPUT_RES));

	{	// Sanity check: All nodes should be bound by now
		stringstream msg;
		msg << "ERROR. All nodes were not bound during Left-Edge Bind (Grissom FPPC Placer)" << endl;
		msg << "There is probably a problem with the schedule." << endl;
		msg << "Try increasing the number of resources and re-scheduling." << endl;
		bool allBound = true;
		for (unsigned i = 0; i < schedDag->allNodes.size(); i++)
		{
			if (schedDag->allNodes.at(i)->GetStatus() != BOUND)
			{
				cout << "Unbound Node: ";
				schedDag->allNodes.at(i)->Print(); // Debugging
				cout << endl;
				allBound = false;
			}
		}
		claim(allBound, &msg);
	}

	// Cleanup
	while (!opsByResType.empty())
	{
		list<AssayNode*> *l = opsByResType.back();
		opsByResType.pop_back();
		delete l;
	}
}

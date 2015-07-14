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
 * Source: grissom_left_edge_binder.cc												*
 * Original Code Author(s): Dan Grissom											*
 * Original Completion/Release Date: October 7, 2012							*
 *																				*
 * Details: N/A																	*
 *																				*
 * Revision History:															*
 * WHO		WHEN		WHAT													*
 * ---		----		----													*
 * FML		MM/DD/YY	One-line description									*
 *-----------------------------------------------------------------------------*/
#include "../../Headers/Placer/grissom_left_edge_binder.h"
#include "../../Headers/Util/sort.h"

///////////////////////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////////////////////
GrissomLEBinder::GrissomLEBinder()
{
}

///////////////////////////////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////////////////////////////
GrissomLEBinder::~GrissomLEBinder()
{
}

///////////////////////////////////////////////////////////////////////////////////
// Places the scheduled DAG (contained by synthesis) according to the demo
// placement algorithm.

// This algorithm works more like a binder as it assumes that the module locations
// are essentially fixed each time-cycle. Thus, the placer is really just binding
// scheduled operations to available work areas.
///////////////////////////////////////////////////////////////////////////////////
void GrissomLEBinder::place(DmfbArch *arch, DAG *schedDag, vector<ReconfigModule *> *rModules)
{
	/////////////////////////////////////////////////////////////
	// Delete old Storage Holder Nodes if any exist - are
	// created later by binder as necessary (previously created
	// by some schedulers)
	while (!schedDag->storageHolders.empty())
	{
		AssayNode *sh = schedDag->storageHolders.back();
		schedDag->storageHolders.pop_back();
		delete sh;
	}

	getAvailResources(arch);
	resetIoResources(arch);

	unsigned long long maxTS = 0;
	int maxStoragePerModule = getMaxStoragePerModule();
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
		{
			opsByResType.at(OUTPUT_RES)->push_back(n);
			if (n->endTimeStep > maxTS)
				maxTS = n->endTimeStep;
		}
		else if (n->GetType() != STORAGE)
			opsByResType.at(n->boundedResType)->push_back(n);
		else if (n->GetType() == STORAGE)
			storeList.push_back(n);
	}
	for (unsigned i = 0; i < schedDag->getAllStorageHolders().size(); i++)
	{
		AssayNode *n = schedDag->getAllStorageHolders().at(i);
		opsByResType.at(n->boundedResType)->push_back(n);
	}
	for (int i = 0; i <= RES_TYPE_MAX + 2; i++)
		Sort::sortNodesByStartTSThenStorageFirst(opsByResType.at(i));

	/////////////////////////////////////////////////////////////
	// Initialize the structure that tells when fixed-modules have
	// gaps in their schedules.
	int numTiles = 0;
	for (int i = 0; i <= RES_TYPE_MAX; i++)
		numTiles += availRes[i].size();
	vector<map<unsigned long long, int> *> freeTsPerFixedMod; // # droplets can be stored for particular TS for each module
	for (int i = 0; i < numTiles; i++)
	{
		freeTsPerFixedMod.push_back(new map<unsigned long long, int>);
		for (unsigned long long j = 0; j < maxTS; j++)
			freeTsPerFixedMod.back()->insert(make_pair(j, maxStoragePerModule));
	}

	/////////////////////////////////////////////////////////////
	// Do Left-Edge binding for modules
	map<FixedModule *, vector<AssayNode *> *> modSchedule;
	for (int i = 0; i <= RES_TYPE_MAX; i++)
	{
		ResourceType rt = (ResourceType)i;
		for (unsigned j = 0; j < availRes[i].size(); j++)
		{
			vector<AssayNode *> * sched = new vector<AssayNode *>();
			FixedModule *fm = availRes[i].at(j);
			modSchedule[fm] = sched;

			unsigned long long lastEnd = 0;
			list<AssayNode *> scheduled;
			list<AssayNode *>::iterator it = opsByResType.at(rt)->begin();
			for (; it != opsByResType.at(rt)->end(); it++)
			{
				AssayNode *n = *it;
				if (n->GetStartTS() >= lastEnd)
				{
					sched->push_back(n);
					scheduled.push_back(n);
					lastEnd = n->GetEndTS();
					n->status = BOUND;
					n->reconfigMod = new ReconfigModule(fm->getResourceType(), fm->getLX(), fm->getTY(), fm->getRX(), fm->getBY());
					n->reconfigMod->startTimeStep = n->startTimeStep;
					n->reconfigMod->endTimeStep = n->endTimeStep;
					n->reconfigMod->boundNode = n;
					n->reconfigMod->tiledNum = fm->getTileNum();
					rModules->push_back(n->reconfigMod);

					// Erase time-steps from modules availability stucture
					for (unsigned long long ts = n->startTimeStep; ts < n->endTimeStep; ts++)
						freeTsPerFixedMod.at(fm->getTileNum())->erase(ts);
				}
			}
			while (!scheduled.empty())
			{
				opsByResType.at(rt)->remove(scheduled.front());
				scheduled.pop_front();
			}
		}
	}

	/////////////////////////////////////////////////////////////
	// Now create new storageHolder nodes based on examining
	// storage nodes
	Sort::sortNodesByStartTS(&storeList);
	list<AssayNode *>::iterator snIt = storeList.begin();
	for (; snIt != storeList.end(); snIt++)
	{
		AssayNode *sn = *snIt;
		for (unsigned ts = sn->startTimeStep; ts < sn->endTimeStep; ts++)
		{
			// Find first available fixed-module location to storage-holders
			bool spaceFound = false;
			for (int i = 0; i <= RES_TYPE_MAX; i++)
			{
				for (unsigned j = 0; j < availRes[i].size(); j++)
				{
					FixedModule *fm = availRes[i].at(j);

					map<unsigned long long, int> *freeTs = freeTsPerFixedMod.at(fm->getTileNum());
					if (freeTs->find(ts) != freeTs->end() && freeTs->find(ts)->second > 0)
					{
						int numDropsStoredAtTS = freeTs->find(ts)->second;

						// If module not used yet this TS, add to storageHolders
						if (numDropsStoredAtTS == maxStoragePerModule)
						{
							AssayNode *sh = schedDag->AddStorageHolderNode();
							sh->status = BOUND;
							sh->boundedResType = fm->getResourceType();
							sh->reconfigMod = new ReconfigModule(fm->getResourceType(), fm->getLX(), fm->getTY(), fm->getRX(), fm->getBY());
							sh->reconfigMod->startTimeStep = ts;
							sh->reconfigMod->endTimeStep = ts+1;
							sh->reconfigMod->boundNode = sh;
							sh->reconfigMod->tiledNum = fm->getTileNum();
							sh->startTimeStep = ts;
							sh->endTimeStep = ts + 1;
							sh->cycles = (sh->GetEndTS()-sh->GetStartTS())* (arch->getFreqInHz() * arch->getSecPerTS());
							rModules->push_back(sh->reconfigMod);
						}

						// If last droplet, remove record of available storage from data-structure
						if (numDropsStoredAtTS > 1)
							freeTs->find(ts)->second = numDropsStoredAtTS-1;
						else
							freeTs->erase(freeTs->find(ts));

						spaceFound = true;
						break; // Examine next time-step
					}
				}
				if (spaceFound)
					break;
			}
			claim(spaceFound, "Error: No storage holder created during left-edge binding. This means there was not enough room for storage at the requested time-step and there is *probably* something wrong with the schedule.");
		}
	}

	// Sort the lists and copy
	Sort::sortNodesByModuleThenStartTS(&(schedDag->storageHolders)); // This may be unnecessary
	list<AssayNode*> holdList;
	for (unsigned i = 0; i < schedDag->storageHolders.size(); i++)
		holdList.push_back(schedDag->storageHolders.at(i));

	//for (list<AssayNode *>::iterator it = holdList.begin(); it != holdList.end(); it++)
	//	cout << "M (" << (*it)->reconfigMod->getLX() << ", " << (*it)->reconfigMod->getTY() << ")\tStart: " << (*it)->startTimeStep << "-" << (*it)->endTimeStep << endl;

	/////////////////////////////////////////////////////////////
	// Do Left-Edge binding of individual storage nodes into
	// storage-holder scheduled nodes. Assumes scheduler will
	// always produce a list of storage holders of 1 TS in length
	while (!storeList.empty())
	{
		//cout << "Num Storage: " << storeList.size() << endl; // DTG Debug

		AssayNode *sNode = storeList.front();
		storeList.pop_front();

		unsigned long long runningEnd = 0;
		bool scheduled = false;
		bool split = false;
		list<AssayNode *> holdersFull;
		list<AssayNode *>::iterator it = holdList.begin();
		for(; it != holdList.end(); it++)
		{
			//cout << "Num Storage_Holders: " << holdList.size() << endl; // DTG Debug

			AssayNode *sHolder = *it;
			if (sNode->GetStatus() != BOUND)
			{
				if (sHolder->GetStartTS() == sNode->GetStartTS()
						//&& sHolder->GetEndTS() <= sNode->GetEndTS()
						&& sHolder->storageOps.size() < maxStoragePerModule)
				{
					sNode->status = BOUND;
					sNode->reconfigMod = sHolder->reconfigMod;
					sNode->boundedResType = sHolder->boundedResType;
					sHolder->storageOps.push_back(sNode);
					sHolder->numDrops++;
					runningEnd = sHolder->endTimeStep;
					scheduled = true;
					if (sHolder->storageOps.size() >= maxStoragePerModule)
						holdersFull.push_back(sHolder);
				}
			}
			else if (scheduled) /*sNode->status == BOUND*/
			{
				if (sHolder->GetStartTS() == runningEnd
						&& sHolder->storageOps.size() < maxStoragePerModule
						&& sHolder->reconfigMod == sNode->reconfigMod)
				{
					sHolder->storageOps.push_back(sNode);
					sHolder->numDrops++;
					runningEnd = sHolder->endTimeStep;
					if (sHolder->storageOps.size() >= maxStoragePerModule)
						holdersFull.push_back(sHolder);
				}
				else
					split = true;
			}

			// If we're at the end of the list and node hasn't been fully sched, split
			it++;
			if (sNode->endTimeStep != runningEnd && it == holdList.end())
				split = true;
			it--;

			if (split)
			{
				AssayNode *storeSecPart = schedDag->AddStorageNode();
				storeSecPart->startTimeStep = runningEnd;
				storeSecPart->endTimeStep = sNode->endTimeStep;
				sNode->endTimeStep = runningEnd;
				storeSecPart->status = SCHEDULED;
				schedDag->InsertNode(sNode, sNode->children.front(), storeSecPart);
				storeList.remove(sNode);
				storeList.push_front(storeSecPart);
				break;
			}
			if (runningEnd == sNode->endTimeStep)
			{
				storeList.remove(sNode);
				break;
			}
		}
		while (!holdersFull.empty())
		{
			holdList.remove(holdersFull.front());
			holdersFull.pop_front();
		}
	}

	/////////////////////////////////////////////////////////////
	// Now do simple Left-Edge binding for inputs/outputs
	bindInputsLE(opsByResType.at(INPUT_RES));
	bindOutputsLE(opsByResType.at(OUTPUT_RES));

	{	// Sanity check: All nodes should be bound by now
		stringstream msg;
		msg << "ERROR. All nodes were not bound during Left-Edge Bind (Grissom Fixed Placer)" << endl;
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

	/////////////////////////////////////////////////////////////
	// Cleanup
	while (!opsByResType.empty())
	{
		list<AssayNode*> *l = opsByResType.back();
		opsByResType.pop_back();
		delete l;
	}
	map<FixedModule *, vector<AssayNode *> *>::iterator it = modSchedule.begin();
	for (; it != modSchedule.end(); it++)
		delete it->second;
	modSchedule.clear();

}

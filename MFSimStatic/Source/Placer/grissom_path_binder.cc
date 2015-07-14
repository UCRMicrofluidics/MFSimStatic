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
 * Source: grissom_path_binder.cc												*
 * Original Code Author(s): Dan Grissom											*
 * Original Completion/Release Date: February 13, 2013							*
 *																				*
 * Details: N/A																	*
 *																				*
 * Revision History:															*
 * WHO		WHEN		WHAT													*
 * ---		----		----													*
 * FML		MM/DD/YY	One-line description									*
 *-----------------------------------------------------------------------------*/

#include "../../Headers/Placer/grissom_path_binder.h"
#include "../../Headers/Util/sort.h"

///////////////////////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////////////////////
GrissomPathBinder::GrissomPathBinder()
{
}

///////////////////////////////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////////////////////////////
GrissomPathBinder::~GrissomPathBinder()
{
}

/////////////////////////////////////////////////////////////////
// Computes the Manhattan distance between the top/left corners
// of the two modules.
/////////////////////////////////////////////////////////////////
int GrissomPathBinder::computeManDist(FixedModule *m1, ReconfigModule *m2)
{
	if (m2 == NULL) // Unsure
		return 16000;
	else
		return (abs(m1->getLX()-m2->getLX()) + abs(m1->getTY() - m2->getTY()));
}

/////////////////////////////////////////////////////////////////
// Creates new storageHolder node in given DAG with specified
// start/stop times and binds node to a new reconfigModule at
// the given fixed module location
/////////////////////////////////////////////////////////////////
AssayNode *GrissomPathBinder::addAndBindNewSHNode(DmfbArch *arch, DAG *schedDag, unsigned long long startTS, unsigned long long endTS, FixedModule *fm)
{
	AssayNode *shNew = schedDag->AddStorageHolderNode();
	shNew->boundedResType = fm->getResourceType();
	shNew->startTimeStep = startTS;
	shNew->endTimeStep = endTS;
	shNew->cycles = (shNew->GetEndTS()-shNew->GetStartTS()) * (arch->getFreqInHz() * arch->getSecPerTS());
	shNew->reconfigMod = new ReconfigModule(fm->getResourceType(), fm->getLX(), fm->getTY(), fm->getRX(), fm->getBY());
	shNew->reconfigMod->boundNode = shNew;
	shNew->reconfigMod->startTimeStep = shNew->startTimeStep;
	shNew->reconfigMod->endTimeStep = shNew->endTimeStep;
	shNew->reconfigMod->tiledNum = fm->getTileNum();
	shNew->status = BOUND;
	return shNew;
}

AssayNode *GrissomPathBinder::addAndBindNewSnodeToSHnode(DAG *schedDag, AssayNode *sh)
{
	AssayNode *sNew = schedDag->AddStorageNode();
	sNew->startTimeStep = sh->startTimeStep;
	sNew->endTimeStep = sh->endTimeStep;
	addAndBindSnodeToSHnode(sNew, sh);
	return sNew;
}

/////////////////////////////////////////////////////////////////
// Add and bind storage node to storage holder node
/////////////////////////////////////////////////////////////////
void GrissomPathBinder::addAndBindSnodeToSHnode(AssayNode *s, AssayNode *sh)
{
	s->boundedResType = sh->boundedResType;
	s->reconfigMod = sh->reconfigMod;
	s->status = BOUND;
	sh->storageOps.push_back(s);
	sh->numDrops++;
}


///////////////////////////////////////////////////////////////////////////////////
// Places the scheduled DAG (contained by synthesis) according to the demo
// placement algorithm.

// This algorithm works more like a binder as it assumes that the module locations
// are essentially fixed each time-cycle. Thus, the placer is really just binding
// scheduled operations to available work areas.
///////////////////////////////////////////////////////////////////////////////////
void GrissomPathBinder::place(DmfbArch *arch, DAG *schedDag, vector<ReconfigModule *> *rModules)
{
	/////////////////////////////////////////////////////////////
	// Initializations
	getAvailResources(arch);
	resetIoResources(arch);
	unsigned long long maxTS = 0;
	int INPUT_RES = 0;
	int OUTPUT_RES = 1;
	int maxStoragePerModule = getMaxStoragePerModule();
	set<AssayPathNode *> leaderNodes;
	set<AssayPathNode *> pathNodes;
	list<AssayNode*> storeList;
	vector<list<AssayNode *> *> ioOps;
	for (int i = 0; i < 2; i++) // 2 for inputs and outputs
		ioOps.push_back(new list<AssayNode*>());
	int numTiles = 0;
	for (int i = 0; i <= RES_TYPE_MAX; i++)
		numTiles += availRes[i].size();

	/////////////////////////////////////////////////////////////
	// Get initial path leader nodes
	for (unsigned i = 0; i < schedDag->allNodes.size(); i++)
	{
		AssayNode *n = schedDag->allNodes.at(i);
		unsigned long long start= 1000000000;
		if (n->type != DISPENSE && n->type != OUTPUT && n->type != STORAGE)
		{
			bool allParentsDispense = true;
			for (unsigned j = 0; j < n->parents.size(); j++)
			{
				if (n->parents.at(j)->type != DISPENSE)
					allParentsDispense = false;
			}
			if (allParentsDispense)
			{
				AssayPathNode *pn = new AssayPathNode();
				pn->pathNodes.push_back(n);
				pn->startTS = n->startTimeStep;
				pn->endTS = n->endTimeStep;
				pn->resType = n->boundedResType;
				n->status = BOUND; // For now
				leaderNodes.insert(pn);
				pathNodes.insert(pn);
			}
		}
		else if (n->type == STORAGE)
			storeList.push_back(n);
		else if (n->type == DISPENSE)
			ioOps.at(INPUT_RES)->push_back(n);
		else if (n->type == OUTPUT)
			ioOps.at(OUTPUT_RES)->push_back(n);
	}

	/////////////////////////////////////////////////////////////
	// Create new graph with path-nodes
	while (!leaderNodes.empty())
	{
		AssayPathNode *pn = *leaderNodes.begin();
		leaderNodes.erase(pn);

		AssayNode *cn = pn->pathNodes.front(); // Current node; will only have 1 at this point

		// Keep storage nodes isolated for now
		if (cn->type == STORAGE)
		{
			cn = cn->children.front(); // Storage only has 1 child and will not be another storage

			// Add a new path node if the child node has not already been bound
			if (cn->status != BOUND)
			{
				cn->status = BOUND;
				AssayPathNode *npn = new AssayPathNode(); // New Path Node
				npn = new AssayPathNode(); // New Path Node
				npn->pathNodes.push_back(cn);
				npn->startTS = cn->startTimeStep;
				npn->endTS = cn->endTimeStep;
				npn->resType = cn->boundedResType;
				pn->children.push_back(npn);
				npn->parents.push_back(pn);
				leaderNodes.insert(npn);
				pathNodes.insert(npn);
			}
		}
		else // Traverse path
		{

			bool continuePath = true;
			while (continuePath)
			{
				continuePath = false; // Continue if we find an eligible child

				// Find all elibible/ineligible children for path binding
				vector <AssayNode *> ineligibleChildren;
				vector <AssayNode *> eligibleChildren;
				for (unsigned i = 0; i < cn->children.size(); i++)
				{
					AssayNode *child = cn->children.at(i);

					// Ignore children that are already bound, they are a part of another PathNode
					if (child->status != BOUND && child->type != OUTPUT)
					{
						if (child->type != STORAGE && child->boundedResType == pn->resType)
						{
							eligibleChildren.push_back(cn->children.at(i));
							continuePath = true;
						}
						else
							ineligibleChildren.push_back(cn->children.at(i));
					}
				}


				if (eligibleChildren.size() + ineligibleChildren.size() > 0)
				{
					// If more than one child (split), add first eligible child to the path;
					// add rest to ineligible list (these will not be considered as path
					// children since they would be mid-path insertions
					if (cn->children.size() > 1)
					{
						for (unsigned i = 0; i < eligibleChildren.size(); i++)
						{
							if (i == 0)
							{
								cn = eligibleChildren.at(i);
								cn->status = BOUND;
								pn->pathNodes.push_back(cn);
								pn->endTS = cn->endTimeStep;
							}
							else
								ineligibleChildren.push_back(eligibleChildren.at(i));
						}
						for (unsigned i = 0; i < ineligibleChildren.size(); i++)
						{
							AssayPathNode *npn = new AssayPathNode(); // New Path Node
							AssayNode *ic = ineligibleChildren.at(i);
							npn->pathNodes.push_back(ic);
							npn->startTS = ic->startTimeStep;
							npn->endTS = ic->endTimeStep;
							npn->resType = ic->boundedResType;
							leaderNodes.insert(npn);
							pathNodes.insert(npn);
						}
					}
					else if (cn->children.size() == 1) // If 1 child, can be considered part of path or new child
					{
						if (eligibleChildren.size() > 0) // Add to path
						{
							cn = eligibleChildren.front();
							cn->status = BOUND;
							pn->pathNodes.push_back(cn);
							pn->endTS = cn->endTimeStep;
						}
						else // Create new child path node
						{
							AssayPathNode *npn = new AssayPathNode(); // New Path Node
							AssayNode *ic = ineligibleChildren.front();
							npn->pathNodes.push_back(ic);
							npn->startTS = ic->startTimeStep;
							npn->endTS = ic->endTimeStep;
							npn->resType = ic->boundedResType;
							pn->children.push_back(npn);
							npn->parents.push_back(pn);
							leaderNodes.insert(npn);
							pathNodes.insert(npn);
						}
					}

					if (cn->type == OUTPUT)
						continuePath = false;
				}
			}
		}
	}

	/////////////////////////////////////////////////////////////
	// Sort AssayPathNodes by resource-type
	vector< list<AssayPathNode*> * > opsByResType; // Operations distinguished by operation types
	for (int i = 0; i <= RES_TYPE_MAX + 2; i++)// +2 for inputs and outputs
		opsByResType.push_back(new list<AssayPathNode*>());
	set<AssayPathNode *>::iterator it = pathNodes.begin();
	for (; it != pathNodes.end(); it++)
	{
		AssayPathNode *n = *it;

		if (n->endTS > maxTS)
			maxTS = n->endTS;

		if (n->pathNodes.front()->type != STORAGE)
			opsByResType.at(n->resType)->push_back(n);
	}

	/////////////////////////////////////////////////////////////
	// Initialize the structure that tells when fixed-modules have
	// gaps in their schedules and associated storageHolder boundary
	// structure, as well as storage and storage-holder (per fixed
	// module) resources
	vector<map<unsigned long long, int> *> freeTsPerFixedMod; // # droplets can be stored for particular TS for each module
	vector<vector<AssayNode *> *> sHoldersPerFixMod; // StorageHolders nodes for each fixed module location
	vector<vector<AssayNode *> *> storagePerFixMod; // Storage nodes for each fixed module location
	for (int i = 0; i < numTiles; i++)
	{
		freeTsPerFixedMod.push_back(new map<unsigned long long, int>);
		for (unsigned long long j = 0; j < maxTS; j++)
			freeTsPerFixedMod.back()->insert(make_pair(j, maxStoragePerModule));
		sHoldersPerFixMod.push_back(new vector<AssayNode *>());
		storagePerFixMod.push_back(new vector<AssayNode *>());
	}

	/////////////////////////////////////////////////////////////
	// Sort nodes by start time
	for (unsigned i = 0; i < opsByResType.size(); i++)
		Sort::sortPathNodesByStartTS(opsByResType.at(i));
	Sort::sortNodesByStartTS(&storeList);

	/////////////////////////////////////////////////////////////
	// Do Left-Edge binding for modules
	map<FixedModule *, vector<AssayPathNode *> *> modSchedule;
	for (int i = 0; i <= RES_TYPE_MAX; i++)
	{
		ResourceType rt = (ResourceType)i;
		for (unsigned j = 0; j < availRes[i].size(); j++)
		{
			vector<AssayPathNode *> * sched = new vector<AssayPathNode *>();
			FixedModule *fm = availRes[i].at(j);
			modSchedule[fm] = sched;

			unsigned long long lastEnd = 0;
			list<AssayPathNode *> scheduled;
			list<AssayPathNode *>::iterator it = opsByResType.at(rt)->begin();
			for (; it != opsByResType.at(rt)->end(); it++)
			{
				AssayPathNode *pn = *it;
				if (pn->startTS >= lastEnd)
				{
					sched->push_back(pn);
					scheduled.push_back(pn);
					lastEnd = pn->endTS;
					pn->res = fm;

					// Now, bind the individual nodes in the path
					for (unsigned k = 0; k < pn->pathNodes.size(); k++)
					{
						AssayNode *n = pn->pathNodes.at(k);
						n->status = BOUND;
						n->reconfigMod = new ReconfigModule(fm->getResourceType(), fm->getLX(), fm->getTY(), fm->getRX(), fm->getBY());
						n->reconfigMod->startTimeStep = n->startTimeStep;
						n->reconfigMod->endTimeStep = n->endTimeStep;
						n->reconfigMod->boundNode = n;
						n->reconfigMod->tiledNum = fm->getTileNum();
						rModules->push_back(n->reconfigMod);
					}

					// Erase time-steps from modules availability stucture
					for (unsigned long long ts = pn->startTS; ts < pn->endTS; ts++)
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
	// DEBUG PRINT
	/*cout << "ASSAY PATH NODES:" << endl;
	int pathNo = 1;
	for (it = pathNodes.begin(); it != pathNodes.end(); it++)
	{
		AssayPathNode *pn = *it;
		cout << "PathNode #" << pathNo++ << " [" << pn->startTS << "," << pn->endTS << ") ";
		if (pn->res != NULL)
			cout << "TN " << pn->res->getTileNum() << ": LxTy(" << pn->res->getLX() << ", " << pn->res->getTY() << ")" << endl;

		for (int i = 0; i < pn->pathNodes.size(); i++)
			{ cout << "\t"; pn->pathNodes.at(i)->Print(); }
	}*/
	// END DEBUG PRINT
	/////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////
	// DEBUG PRINT
	/*cout << "FINE-GRAINED RESOURCE FREEDOM PER FIX-MOD/TS:" << endl;
	for (int i = 0; i <= RES_TYPE_MAX; i++)
	{
		for (int j = 0; j < availRes[i].size(); j++)
		{
			FixedModule *fm = availRes[i].at(j);
			map<unsigned long long, int> *freeTs = freeTsPerFixedMod.at(fm->getTileNum());
			map<unsigned long long, int>::iterator freeTsIt = freeTs->begin();
			cout << "TN " << fm->getTileNum() << ": LxTy(" << fm->getLX() << ", " << fm->getTY() << "):" << endl << "\tTS(Storable Droplets): ";
			for (; freeTsIt != freeTs->end(); freeTsIt++)
				cout << freeTsIt->first << "(" << freeTsIt->second << "), ";
			cout << endl;
		}
	}*/
	// END DEBUG PRINT
	/////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////
	// DEBUG OUTPUT
	//schedDag->OutputGraphFile("Output/temp1", true, true);
	// END DEBUG OUTPUT
	/////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////
	// Bind the storage nodes to a particular module HERE
	/////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////
	while (!storeList.empty())
	{
		AssayNode *sn = storeList.front();
		storeList.pop_front();
		sn->status = BOUND;

		// Check all mods to see if storage op will fit
		vector<FixedModule *> potentialMods;
		int maxRun = 0;
		for (int i = 0; i <= RES_TYPE_MAX; i++)
		{
			for (unsigned j = 0; j < availRes[i].size(); j++)
			{
				int run = 0;
				FixedModule *fm = availRes[i].at(j);
				map<unsigned long long, int> *freeTs = freeTsPerFixedMod.at(fm->getTileNum());

				for (unsigned ts = sn->startTimeStep; ts < sn->endTimeStep; ts++)
				{
					if (freeTs->find(ts) != freeTs->end() && freeTs->find(ts)->second > 0)
						run++;
					else
						break;
				}

				// Save if longest run and clear any shorter runs
				if (run == maxRun)
					potentialMods.push_back(fm);
				else if (run > maxRun)
				{
					maxRun = run;
					potentialMods.clear();
					potentialMods.push_back(fm);
				}
			}
		}

		// Choose module to bind node to
		FixedModule *selection;
		int minDist = 16000; // Large number
		for (unsigned i = 0; i < potentialMods.size(); i++)
		{
			//int dist = computeManDist(potentialMods.at(i), sn->parents.front()->reconfigMod);
			int dist = min(computeManDist(potentialMods.at(i), sn->parents.front()->reconfigMod), computeManDist(potentialMods.at(i), sn->children.front()->reconfigMod));
			if (dist <= minDist)
			{
				minDist = dist;
				selection = potentialMods.at(i);
			}
		}

		// If splitting, create new storage node and add to list to be bound
		if (maxRun < sn->endTimeStep - sn->startTimeStep)
		{
			AssayNode *newStoreNode = schedDag->AddStorageNode();
			newStoreNode->endTimeStep = sn->endTimeStep;
			sn->endTimeStep = sn->startTimeStep + maxRun;
			newStoreNode->startTimeStep = sn->endTimeStep;
			schedDag->InsertNode(sn, sn->children.front(), newStoreNode);
			storeList.push_front(newStoreNode);
		}


		// Update storage module-selection data-structure
		storagePerFixMod.at(selection->getTileNum())->push_back(sn);

		// Update freeTsPerFixedMod structure for selection
		map<unsigned long long, int> *freeTs = freeTsPerFixedMod.at(selection->getTileNum());
		for (unsigned long long i = sn->startTimeStep; i < sn->endTimeStep; i++)
		{
			if (freeTs->find(i) != freeTs->end())
			{
				if (freeTs->find(i)->second > 0)
					freeTs->find(i)->second = freeTs->find(i)->second - 1;
				else
					freeTs->erase(freeTs->find(i));
			}
		}

	}

	/////////////////////////////////////////////////////////////
	// DEBUG OUTPUT
	//schedDag->OutputGraphFile("Output/temp2", true, true);
	// END DEBUG OUTPUT
	/////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////
	// Delete old Storage Holder Nodes - Don't need them - & then
	// create new storage holder nodes based on storage bindings
	// from above
	while (!schedDag->storageHolders.empty())
	{
		AssayNode *sh = schedDag->storageHolders.back();
		schedDag->storageHolders.pop_back();
		delete sh;
	}

	/////////////////////////////////////////////////////////////
	// DEBUG PRINT
	/*cout << "STORES PER MODULE:" << endl;
	for (int i = 0; i < numTiles; i++)
	{
		vector<AssayNode *> *stores = storagePerFixMod.at(i);
		Sort::sortNodesByStartThenEndTS(stores);
		cout << "TN" << i << ": ";
		for (int j = 0; j < stores->size(); j++)
		{
			AssayNode *s = stores->at(j);
			cout << "S" << s->id << "[" << s->startTimeStep << ", " << s->endTimeStep << "), ";
		}
		cout << endl;
	}*/
	// END DEBUG PRINT
	/////////////////////////////////////////////////////////////



	/////////////////////////////////////////////////////////////
	// Now do Left-Edge binding for storage-holder nodes, creating
	// new storage-holder nodes if necessary
	for (int i = 0; i <= RES_TYPE_MAX; i++)
	{
		for (unsigned j = 0; j < availRes[i].size(); j++)
		{
			FixedModule *fm = availRes[i].at(j);
			vector<AssayNode *> *sHolds = sHoldersPerFixMod.at(fm->getTileNum());
			vector<AssayNode *> *stores = storagePerFixMod.at(fm->getTileNum());

			// Examine stores in order of start
			Sort::sortNodesByStartThenEndTS(stores);

			for (unsigned k = 0; k < stores->size(); k++)
			{
				AssayNode *s = stores->at(k);

				// No StorageHolders yet; create new StorageHolder nodes that overlap perfectly with Storage node
				if (sHolds->size() == 0)
				{
					AssayNode *shNewNode = addAndBindNewSHNode(arch, schedDag, s->startTimeStep, s->endTimeStep, fm);
					rModules->push_back(shNewNode->reconfigMod);
					sHolds->push_back(shNewNode);
					addAndBindSnodeToSHnode(s, shNewNode);
				}
				// There are StorageHolders, so we must determine if this storage node overlaps any
				// existing and create new SH nodes and split this storage node, as necessary
				else
				{
					unsigned long long runningStart = s->startTimeStep;
					int shIndex = 0; // sHolds index

					// While we have not bound all of the current storage node
					while (runningStart != s->endTimeStep)
					{
						AssayNode *sh = sHolds->at(shIndex);
						// if whole or part of S pre (should bring runningStart to beginning of a sh node...or finish binding of node)
						//     	create new pre sh node
						//		runningStart = end of new pre sh node
						if (runningStart < sh->startTimeStep)
						{
							AssayNode *shPre = addAndBindNewSHNode(arch, schedDag, s->startTimeStep, min(s->endTimeStep, sh->startTimeStep), fm);
							rModules->push_back(shPre->reconfigMod);
							sHolds->insert(sHolds->begin()+shIndex, shPre);

							// If S node overlapped a SH node, split S node and adjust TS info
							if (shPre->endTimeStep < s->endTimeStep)
							{
								AssayNode *sPre = addAndBindNewSnodeToSHnode(schedDag, shPre);
								s->startTimeStep = sPre->endTimeStep;
								schedDag->InsertNode(s->parents.front(), s, sPre);
							}
							else // No overlap, just add to S
								addAndBindSnodeToSHnode(s, shPre);

							runningStart = shPre->endTimeStep;
						}
						// else if S starts at beginning of SH
						//		if S ends in middle of SH
						//			split SH at S.end and create new SH from S.end to SH.end
						//		else if S.end > OR == SH.end
						//			running end = SH.end
						//		add S to SH first half (original)
						else if (runningStart == sh->startTimeStep)
						{
							if (s->endTimeStep < sh->endTimeStep)
							{
								AssayNode *shEnd = addAndBindNewSHNode(arch, schedDag, s->endTimeStep, sh->endTimeStep, fm);
								rModules->push_back(shEnd->reconfigMod);
								sHolds->insert(sHolds->begin()+shIndex+1, shEnd);

								sh->endTimeStep = s->endTimeStep;
								sh->reconfigMod->endTimeStep = sh->endTimeStep;

								// Already bound storage nodes must be broken up and added
								list<AssayNode *>::iterator soIt= sh->storageOps.begin();
								for (; soIt != sh->storageOps.end(); soIt++)
								{
									AssayNode *oldStore = *soIt;
									oldStore->endTimeStep = shEnd->startTimeStep;

									AssayNode *oldStoreEnd = addAndBindNewSnodeToSHnode(schedDag, shEnd);
									schedDag->InsertNode(oldStore, oldStore->children.front(), oldStoreEnd);
								}

								runningStart = s->endTimeStep;
							}
							else //if (s->endTimeStep (> || ==) sh->endTimeStep)
								runningStart = sh->endTimeStep;

							addAndBindSnodeToSHnode(s, sh);
							shIndex--;
						}
						// else if S starts in middle
						//		split SH and add to 2nd half
						else if (runningStart > sh->startTimeStep && runningStart < sh->endTimeStep)
						{
							AssayNode *shEnd = addAndBindNewSHNode(arch, schedDag, runningStart, min(s->endTimeStep, sh->endTimeStep), fm);
							rModules->push_back(shEnd->reconfigMod);
							sHolds->insert(sHolds->begin()+shIndex+1, shEnd);

							if (shEnd->endTimeStep < s->endTimeStep)
							{
								AssayNode *sBeg = addAndBindNewSnodeToSHnode(schedDag, shEnd);
								s->startTimeStep = sBeg->endTimeStep;
								schedDag->InsertNode(s->parents.front(), s, sBeg);
							}
							else
								addAndBindSnodeToSHnode(s, shEnd);



							// Already bound storage nodes must be broken up and added
							list<AssayNode *>::iterator soIt= sh->storageOps.begin();
							for (; soIt != sh->storageOps.end(); soIt++)
							{
								AssayNode *oldStore = *soIt;
								oldStore->endTimeStep = shEnd->startTimeStep;

								AssayNode *oldStoreEnd = addAndBindNewSnodeToSHnode(schedDag, shEnd);
								schedDag->InsertNode(oldStore, oldStore->children.front(), oldStoreEnd);
							}


							// Completely encompassed by SH - must create one more SH
							if (s->endTimeStep < sh->endTimeStep)
							{
								AssayNode *shEnd2 = addAndBindNewSHNode(arch, schedDag, shEnd->endTimeStep, sh->endTimeStep, fm);
								rModules->push_back(shEnd2->reconfigMod);
								sHolds->insert(sHolds->begin()+shIndex+2, shEnd2);

								// Already bound storage nodes must be broken up and added
								soIt= sh->storageOps.begin();
								for (; soIt != sh->storageOps.end(); soIt++)
								{
									AssayNode *oldStore = *soIt;
									AssayNode *oldStore3 = addAndBindNewSnodeToSHnode(schedDag, shEnd2);
									schedDag->InsertNode(oldStore->children.front(), oldStore->children.front()->children.front(), oldStore3);
								}

							}
							runningStart = shEnd->endTimeStep;
							sh->endTimeStep = shEnd->startTimeStep;
							sh->reconfigMod->endTimeStep = sh->endTimeStep;
							shIndex--;
						}
						// else if whole node starts after SH and shIndex +1 == sHolds.size()
						//		create new post sh node
						// 		runningStart = end of new post sh node (s->end)
						//
						else if (runningStart >= sh->endTimeStep && shIndex+1 == sHolds->size())
						{
							AssayNode *shPost = addAndBindNewSHNode(arch, schedDag, runningStart, s->endTimeStep, fm);
							rModules->push_back(shPost->reconfigMod);
							sHolds->insert(sHolds->end(), shPost);

							if (runningStart > s->startTimeStep)
							{
								AssayNode *sPost = addAndBindNewSnodeToSHnode(schedDag, shPost);
								s->endTimeStep = sPost->startTimeStep;
								schedDag->InsertNode(s, s->children.front(), sPost);
								s = sPost; // For the sake of exiting the loop
							}
							else
								addAndBindSnodeToSHnode(s, shPost);

							runningStart = shPost->endTimeStep;
							shIndex--;
						}




						shIndex++;
					}
				}
			}
		}
	}

	/////////////////////////////////////////////////////////////
	// Now do simple Left-Edge binding for inputs/outputs
	bindInputsLE(ioOps.at(INPUT_RES));
	bindOutputsLE(ioOps.at(OUTPUT_RES));

	{	// Sanity check: All nodes should be bound by now
		stringstream msg;
		msg << "ERROR. All nodes were not bound during Path Binding...there is probably a problem with the schedule." << ends;
		msg << "ERROR. All nodes were not bound during Path Binding." << endl;
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

			/*for (int j = 0; j < schedDag->allNodes.at(i)->children.size(); j++)
			{
				if (schedDag->allNodes.at(i)->endTimeStep != schedDag->allNodes.at(i)->children.at(j)->startTimeStep)
				{
					cout << "P->C pair does not have contiguous time-steps: ";
					schedDag->allNodes.at(i)->Print();
					schedDag->allNodes.at(i)->children.at(j)->Print();
					allBound = false;
				}
			}*/

		}
		claim(allBound, &msg);
	}

	/////////////////////////////////////////////////////////////
	// Cleanup
	/////////////////////////////////////////////////////////////
	while (!sHoldersPerFixMod.empty())
	{
		vector<AssayNode *> *v = sHoldersPerFixMod.back();
		v->clear();
		sHoldersPerFixMod.pop_back();
		delete v;
	}
	/*while (!sHoldBoundaries.empty())
	{
		vector<unsigned long long> *v = sHoldBoundaries.back();
		v->clear();
		sHoldBoundaries.pop_back();
		delete v;
	}*/
	while (!freeTsPerFixedMod.empty())
	{
		map<unsigned long long, int> *m = freeTsPerFixedMod.back();
		m->clear();
		freeTsPerFixedMod.pop_back();
		delete m;
	}
	while (!opsByResType.empty())
	{
		list<AssayPathNode*> *l = opsByResType.back();
		l->clear();
		opsByResType.pop_back();
		delete l;
	}
	while (!ioOps.empty())
	{
		list<AssayNode*> *l = ioOps.back();
		l->clear();
		ioOps.pop_back();
		delete l;
	}
	map<FixedModule *, vector<AssayPathNode *> *>::iterator msIt = modSchedule.begin();
	for (; msIt != modSchedule.end(); msIt++)
		delete msIt->second;
	modSchedule.clear();

	/////////////////////////////////////////////////////////////
	// DEBUG OUTPUT
	//schedDag->OutputGraphFile("Output/temp3", true, true);
	// END DEBUG OUTPUT
	/////////////////////////////////////////////////////////////

	//cout << "GRISSOM PATH BINDER UNDER CONSTRUCTION\nExiting.";
	//exit(1);
}




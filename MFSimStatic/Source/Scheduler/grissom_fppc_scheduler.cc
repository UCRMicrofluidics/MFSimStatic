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
 * Source: grissom_fppc_scheduler.cc									*
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
#include "../../Headers/Scheduler/grissom_fppc_scheduler.h"
#include "../../Headers/Util/sort.h"

///////////////////////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////////////////////
GrissomFppcScheduler::GrissomFppcScheduler()
{
}

///////////////////////////////////////////////////////////////////////////////////
// Deconstructor
///////////////////////////////////////////////////////////////////////////////////
GrissomFppcScheduler::~GrissomFppcScheduler()
{
}

/////////////////////////////////////////////////////////////////
// Tells whether a dispense well containing the specified fluid
// is available at the current timestep
/////////////////////////////////////////////////////////////////
IoResource * GrissomFppcScheduler::getReadyDispenseWell(string fluidName, unsigned long long schedTS)
{
	for (unsigned i = 0; i < dispRes->size(); i++)
	{
		IoResource *dr = dispRes->at(i);
		if (strcmp(Util::StringToUpper(fluidName).c_str(), Util::StringToUpper(dr->name).c_str()) == 0)
		{
			if (dr->lastEndTS + dr->durationInTS <= schedTS)
				return dr;
		}
	}
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////
// Schedules the DAG (contained by synthesis) according to the list scheduling
// algorithm.
///////////////////////////////////////////////////////////////////////////////////
unsigned long long GrissomFppcScheduler::schedule(DmfbArch *arch, DAG *dag)
{
	int maxStoragePerModule = 1; // This will not change, so we hard code instead of pull from command line
	int numModules = getAvailResources(arch);
	int maxLocDrops = numModules * maxStoragePerModule;
	int dropsInLoc = 0;
	int dropsInStorage = 0;
	int numStorageModules = 0;
	resetIoResources(arch);

	if (internalPriorities)
	{
		Priority::setAsCritPathDist(dag, arch);
		//Priority::setAsNumIndPaths(dag);
		//Priority::setAsLongestPathDist(dag);
		//Priority::setAsNumIndPaths(dag);
	}


	commissionDAG(dag);


	unsigned long long schedTS = 0;


	while(moreNodesToSchedule())
	{
		//cout << "DB: Scheduling Time Step " << schedTS << endl << "----------------------------" << endl;
		//cout << "\tlocDrops " << dropsInLoc << " -- stDrops " << dropsInStorage << " -- stChams " << numStorageModules << endl;

		// If any ops just finished, their droplets go back into "storage" until otherwise claimed
		vector<AssayNode*> finishedOps;
		vector<AssayNode*> scheduledOps;
		list<AssayNode*>::iterator	it = unfinishedOps->begin();

		// Determine which ops just finished so we can reclaim resources and account for droplets
		for (; it != unfinishedOps->end(); it++)
		{
			AssayNode *node = *it;

			if (node->GetEndTS() == schedTS)
			{
				finishedOps.push_back(node);
				if (node->boundedResType == SSD_RES)
				{
					if (node->type == SPLIT)
						availRes[SSD_RES] = availRes[SSD_RES] + 2; // Split takes two SS modules with PC Design #1
					else
						availRes[SSD_RES]++;
				}
				else if (node->boundedResType == BASIC_RES)
					availRes[BASIC_RES]++;
				else if (node->boundedResType == D_RES)
					availRes[D_RES]++;
				else if (node->boundedResType == H_RES)
					availRes[H_RES]++;
				else if (node->boundedResType == DH_RES)
					availRes[DH_RES]++;

				for (unsigned i = 0; i < node->GetChildren().size(); i++)
				{
					if (node->GetChildren().at(i)->GetType() != OUTPUT)
						dropsInStorage++;
				}
			}
		}
		for (unsigned i = 0; i < finishedOps.size(); i++)
			unfinishedOps->remove(finishedOps.at(i));

		// Arrange by priority
		//Sort::sortNodesByPriorityHiFirst(candidateOps); // MLS_DEC
		Sort::sortNodesByPriorityLoFirst(candidateOps); // MLS_INC

		// Now, see if there is an operation we can schedule
		it = candidateOps->begin();
		for (; it != candidateOps->end(); it++)
		{
			AssayNode *n = *it;

			// TODO: Add dilution support for this device
			claim(n->type != DILUTE, "The Field-programmable Pin-constrained device does not currently support dilution operations.");

			int netStorageDropsGain = 0;
			int netLocDropsGain = 0;

			// Determine what the net gain in droplets would be by scheduling this node at this TS
			bool parentsDone = true;
			for (unsigned p = 0; p < n->GetParents().size(); p++)
			{
				AssayNode *par = n->GetParents().at(p);
				if (par->GetType() == DISPENSE)
				{
					//netStorageDropsGain++;// Pretend like this came from storage for a moment to cancel out later
					if (!getReadyDispenseWell(par->GetPortName(), schedTS))
						parentsDone = false;
				}
				else
				{
					// droplets being output next are NEVER considered as in the system/storage
					if (n->GetType() != OUTPUT)
					{
						netStorageDropsGain--;
						netLocDropsGain--;
					}
					if (!(par->GetStatus() == SCHEDULED && par->GetEndTS() <= schedTS))
						parentsDone = false;
				}

			}
			for (unsigned c = 0; c < n->GetChildren().size(); c++)
				if (n->GetChildren().at(c)->GetType() != OUTPUT)
					netLocDropsGain++;

			// If the proposed operation doesn't cause too many droplets/chambers, schedule it
			numStorageModules = ceil(((double)dropsInStorage + (double)netStorageDropsGain)/ (double)maxStoragePerModule);
			if (parentsDone && dropsInLoc + netLocDropsGain <= maxLocDrops &&
					(unfinishedOps->size() + 1) + numStorageModules <= numModules)
			{
				// Determine if there is an applicable, available resource
				bool canSchedule = false;
				if (n->type == MIX && (availRes[BASIC_RES] /*+ availRes[D_RES] + availRes[H_RES] + availRes[DH_RES]*/) > 0)
				{
					canSchedule = true;
					if (availRes[BASIC_RES] > 0)
					{
						availRes[BASIC_RES]--;
						n->boundedResType = BASIC_RES;
					}
				}
				else if (n->type == SPLIT && availRes[SSD_RES] >= 2)
				{
					canSchedule = true;
					availRes[SSD_RES] = availRes[SSD_RES] - 2;
					n->boundedResType = SSD_RES;
				}
				else if (n->type == DETECT && availRes[SSD_RES] > 0) //(availRes[D_RES] + availRes[DH_RES]) > 0)
				{	// Assume SSD_RES can also do detection
					canSchedule = true;
					availRes[SSD_RES]--;
					n->boundedResType = SSD_RES;
				}
				else if (n->type == HEAT && (availRes[H_RES] + availRes[DH_RES]) > 0)
				{
					canSchedule = true;
					if (availRes[H_RES] > 0)
					{
						availRes[H_RES]--;
						n->boundedResType = H_RES;
					}
					else
					{
						availRes[DH_RES]--;
						n->boundedResType = DH_RES;
					}
				}
				else if (n->type == OUTPUT)
					canSchedule = true;

				if (canSchedule)
				{
					dropsInLoc += netLocDropsGain;
					dropsInStorage += netStorageDropsGain;
					n->startTimeStep = schedTS;
					n->endTimeStep = schedTS + ceil((double)n->cycles/(double)arch->getFreqInHz()/arch->getSecPerTS());
					n->status = SCHEDULED;
					if (n->GetType() != OUTPUT)
						unfinishedOps->push_back(n);
					else
						n->endTimeStep = schedTS+1;
					scheduledOps.push_back(n);

					//cout << "DEBUG: " << n->name << ": [" << n->startTimeStep << ", " << n->endTimeStep << "]" << endl;// dtg debug

					// Update any dispense parents & insert any necessary storage nodes into the DAG
					vector<AssayNode*> pInsert;
					vector<AssayNode*> sInsert;

					for (unsigned p = 0; p < n->GetParents().size(); p++)
					{
						AssayNode *parent = n->GetParents().at(p);

						// If parent is SPLIT, convert to two stores by making the Split "instantaneous" so we can reserve two modules for it
						if (parent->type == SPLIT)
							parent->endTimeStep = parent->startTimeStep;

						if (parent->GetType() == DISPENSE)
						{
							IoResource *dr = getReadyDispenseWell(parent->GetPortName(), schedTS);
							parent->startTimeStep = schedTS - dr->durationInTS;
							parent->endTimeStep = parent->startTimeStep + dr->durationInTS;
							parent->status = SCHEDULED;
							dr->lastEndTS = parent->endTimeStep;
						}

						if (parent->endTimeStep < schedTS)
						{
							AssayNode *store = dag->AddStorageNode();
							store->status = SCHEDULED;
							store->startTimeStep = parent->endTimeStep;
							store->endTimeStep = schedTS;
							store->boundedResType = SSD_RES; // Always in this configuration
							pInsert.push_back(parent); // Insert later so we don't mess up for loop
							sInsert.push_back(store);
							//dag->InsertNode(parent, n, store);
						}
					}

					// Now do actual insert of any necessary storage nodes
					for (unsigned s = 0; s < pInsert.size(); s++)
						dag->InsertNode(pInsert.at(s), n, sInsert.at(s));

					// If each child's parents are all scheduled, we can add it to the candidate list
					for (unsigned c = 0; c < n->GetChildren().size(); c++)
					{
						bool canAddChild = true;
						AssayNode *child = n->GetChildren().at(c);
						for (unsigned p = 0; p < child->GetParents().size(); p++)
						{
							if (child->GetParents().at(p)->GetType() != DISPENSE && child->GetParents().at(p)->GetStatus() != SCHEDULED)
								canAddChild = false;
						}
						if (canAddChild)
							candidateOps->push_back(child);
					}
				}
			}
		}
		// Remove scheduled ops from the candidate list
		for (unsigned i = 0; i < scheduledOps.size(); i++)
			candidateOps->remove(scheduledOps.at(i));

		// Create nodes for storage
		int ar[RES_TYPE_MAX+1];
		for (int i = 0; i <= RES_TYPE_MAX; i++)
			ar[i] = availRes[i];

		int dis = dropsInStorage;
		while (dis > 0)
		{
			// Assign resource type
			if (ar[SSD_RES] > 0)
				ar[SSD_RES]--;
			else
			{
				stringstream msg;
				msg << "Cannot schedule assay using this method because there are no more SSD modules remaining for the remaining " << dis << " storage operations at TS " << schedTS << "." << endl;
				claim(false, &msg);
			}

			if (dis >= maxStoragePerModule)
				dis -= maxStoragePerModule;
			else
				dis -= dis;
		}


		schedTS++;

		// If we failed to schedule any operations, and there are more left,
		// then LS cannot schedule this assay given the current priorities
		int freeModules = 0;
		for (int i = 0; i <= RES_TYPE_MAX; i++)
			freeModules += ar[i];
		if (unfinishedOps->empty() && moreNodesToSchedule() && freeModules == 0)
		{
			if (internalPriorities)
				cout << "LS FAIL, returning: " << schedTS << endl;
			return 1000000;
		}

		//if (schedTS > 1000)
		//	return 10000;

		//cout << "storage: " << dropsInStorage << endl; // dtg debug
		//cout << "loc: " << dropsInLoc << endl; // dtg debug
	}

	if (internalPriorities)
		cout << "LS Time: " << schedTS << endl;

	return schedTS;
}


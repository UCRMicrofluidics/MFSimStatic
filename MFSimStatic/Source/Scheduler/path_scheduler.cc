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
 * Source: path_scheduler.cc													*
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
#include "../../Headers/Scheduler/path_scheduler.h"
#include "../../Headers/Util/sort.h"

///////////////////////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////////////////////
PathScheduler::PathScheduler()
{
	availResAtTS = new map<unsigned long long, ResHist*> ();
	//dispHistories = new vector<IoHist*>();
	indefStorage = new map<unsigned long long, list<AssayNode*>*> ();

	dispRes = new vector<IoResource*>();
	outRes = new vector<IoResource*>();
	candidateOps = new list<AssayNode*>();
	unfinishedOps = new list<AssayNode*>();
	executingOps = new list<AssayNode*>();
	internalPriorities = true;
}

///////////////////////////////////////////////////////////////////////////////////
// Deconstructor
///////////////////////////////////////////////////////////////////////////////////
PathScheduler::~PathScheduler()
{
	while (!availResAtTS->empty())
	{
		ResHist *rh = availResAtTS->begin()->second;
		availResAtTS->erase(availResAtTS->begin()->first);
		delete rh;
	}
	delete availResAtTS;
	/*while (!dispHistories->empty())
	 {
	 IoHist *ih = dispHistories->back();
	 dispHistories->pop_back();
	 delete ih;
	 }*/
	//delete dispHistories;
	while (!indefStorage->empty())
	{
		list<AssayNode*> *v = indefStorage->begin()->second;
		indefStorage->erase(indefStorage->begin()->first);
		v->clear();
		delete v;
	}
}

/////////////////////////////////////////////////////////////////
// Create a new ResHist element with initialized values as if
// the chip is empty at that ts (time-step)
/////////////////////////////////////////////////////////////////
ResHist * PathScheduler::createNewResHist(unsigned long long ts)
{
	ResHist *rh = new ResHist();
	rh->timeStep = ts;
	rh->dropsInLoc = 0;
	rh->dropsInStorage = 0;

	for (int i = 0; i <= RES_TYPE_MAX; i++)
		rh->availRes[i] = availRes[i];

	return rh;
}

/////////////////////////////////////////////////////////////////
// Gets the ResHist element at the specified time-step.  If one
// doesn't exist, it creates a new one and adds it to the history
// map.
/////////////////////////////////////////////////////////////////
ResHist * PathScheduler::getResHistAtTS(unsigned long long ts)
{
	map<unsigned long long, ResHist*>::iterator rhIt;
	rhIt = availResAtTS->find(ts);
	if (rhIt != availResAtTS->end())
		return rhIt->second;
	else
	{
		ResHist *rh = createNewResHist(ts);
		availResAtTS->insert(pair<unsigned long long, ResHist*> (ts, rh));
		return rh;
	}
}

/////////////////////////////////////////////////////////////////
// Gets the number of droplets (as of the specified time-step)
// that are being stored indefinitely starting at that cycle
// (does not include others starting before or after the
// specified TS
/////////////////////////////////////////////////////////////////
int PathScheduler::getNumNewIndefStorDrops(unsigned long long ts)
{
	map<unsigned long long, list<AssayNode*>*>::iterator it;
	it = indefStorage->find(ts);
	if (it != indefStorage->end())
		return it->second->size();
	else
		return 0;
}

/////////////////////////////////////////////////////////////////
// Gets the number of droplets that are being indefinitely
// stored up to the specified TS (but not including the new
// droplets being indefinitely stored at ts)
/////////////////////////////////////////////////////////////////
int PathScheduler::getNumIndefStorDropsBefore(unsigned long long ts)
{
	int numDrops = 0;
	map<unsigned long long, list<AssayNode*>*>::iterator it = indefStorage->begin();
	for (; it != indefStorage->end(); it++)
		if (it->first < ts)
			numDrops += it->second->size();

	return numDrops;
}
list<AssayNode *> *PathScheduler::getIndefStorDropsBefore(unsigned long long ts)
{
	list<AssayNode *> *storing = new list<AssayNode *>();
	map<unsigned long long, list<AssayNode*>*>::iterator it = indefStorage->begin();
	list<AssayNode*>::iterator it2;
	for (; it != indefStorage->end(); it++)
		if (it->first < ts)
			for (it2 = it->second->begin(); it2 != it->second->end(); it2++)
				storing->push_back(*it2);

	return storing;
}

/////////////////////////////////////////////////////////////////
// Tells if a certain resource/chamber type exists on the LoC
// (this is more of a physical property of the LoC...it does not
// tell if a resource type is actually available at a specific
// time-step...use another function for that)
/////////////////////////////////////////////////////////////////
bool PathScheduler::resTypeExists(ResourceType rt)
{
	return availRes[rt] > 0;
}


/////////////////////////////////////////////////////////////////
// Tells whether a dispense well containing the specified fluid
// is available at the current timestep (meaning it is free for
// a few TS before now, depending on how long it takes for the
// input to dispense onto the chip.
/////////////////////////////////////////////////////////////////
IoResource * PathScheduler::getReadyDispenseWell(string fluidName, unsigned long long schedTS, unsigned long long dagStartTS)
{
	for (unsigned i = 0; i < dispRes->size(); i++)
	{
		IoResource *dr = dispRes->at(i);
		if (strcmp(Util::StringToUpper(fluidName).c_str(), Util::StringToUpper(dr->name).c_str()) == 0)
		{
			bool canDispense = true;
			if (dr->durationInTS > schedTS || schedTS - dr->durationInTS < dagStartTS)
				return NULL;
			for (unsigned long long j = schedTS - dr->durationInTS; j < schedTS; j++)
			{
				if (dr->isActiveAtTS.find(j) != dr->isActiveAtTS.end())
				{ // Something is already dispensing at time j
					if (dr->isActiveAtTS.find(j)->second == true)
					{
						canDispense = false;
						break;
					}
				}
			}
			if (canDispense)
				return dr;
		}
	}
	return NULL;
}

/////////////////////////////////////////////////////////////////
// Given a time-step and a number of running drops being stored
// (i.e. the number of drops being stored in the current path
// that haven't been committed to history elements yet), gives
// the number of free chambers
/////////////////////////////////////////////////////////////////
int PathScheduler::getNumFreeModules(unsigned long long ts, int runningStorageDrops)
{
	ResHist *rh = getResHistAtTS(ts);
	int numStorageModules = ceil(((double) rh->dropsInStorage + (double) runningStorageDrops) / (double) getMaxStoragePerModule());
	int freeChambers = 0;
	for (int i = 0; i <= RES_TYPE_MAX; i++)
		freeChambers += rh->availRes[i];
	return (freeChambers - numStorageModules);
}

/////////////////////////////////////////////////////////////////
// Given a time-step, a number of running drops being stored
// (i.e. the number of drops being stored in the current path
// that haven't been committed to history elements yet), and a
// resource type, tells if the resource type is free for that
// time-step
/////////////////////////////////////////////////////////////////
bool PathScheduler::resTypeIsAvailAtTS(unsigned long long ts, ResourceType rt, int runningStorageDrops)
{
	if (getNumFreeModules(ts, runningStorageDrops) > 0
			&& getResHistAtTS(ts)->availRes[rt] > 0)
		return true;
	else
		return false;
}

/////////////////////////////////////////////////////////////////
// Given a number of droplets to store, tells if we have enough
// room to store everything at the specified time-step
/////////////////////////////////////////////////////////////////
bool PathScheduler::canStoreDropsAtTS(unsigned long long ts, int numDrops)
{
	if (getNumFreeModules(ts, numDrops) >= 0)
		return true;
	else
		return false;
}

/////////////////////////////////////////////////////////////////
// Sets the previous time-steps as active (does not add anything
// to the input-resource's schedule
/////////////////////////////////////////////////////////////////
void PathScheduler::setActiveAtTS(IoResource *input, unsigned long long ts)
{
	map<unsigned long long, bool>::iterator it;
	for (unsigned i = ts - input->durationInTS; i < ts; i++)
	{
		it = input->isActiveAtTS.find(i);
		if (it == input->isActiveAtTS.end())
			input->isActiveAtTS.insert(pair<unsigned long long, bool> (i, true));
		else if (it->second == false)
			input->isActiveAtTS[i] = true;
		else
		{
			// Sanity check: Must inactive
			stringstream msg;
			msg << "ERROR: " << input->name
					<< " is already active at time-step " << i << ends;
			claim(false, &msg);
		}
	}
}

/////////////////////////////////////////////////////////////////
// Sets the previous time-steps as un-active (does not remove
// anything from the input-resource's schedule
/////////////////////////////////////////////////////////////////
void PathScheduler::setInactiveAtTS(IoResource *input, unsigned long long ts)
{
	map<unsigned long long, bool>::iterator it;
	for (unsigned i = ts - input->durationInTS; i < ts; i++)
	{
		it = input->isActiveAtTS.find(i);
		if (it == input->isActiveAtTS.end() || it->second == false)
		{
			// Sanity check: Must be active
			stringstream msg;
			msg << "ERROR: " << input->name << " is already in-active at time-step " << i << endl;
			claim(false, &msg);
		}
		else
			input->isActiveAtTS[i] = false;
	}
}

/////////////////////////////////////////////////////////////////
// Fills the potRes vector with the resource types that can
// can perform the operation ot.  Only considers resource types
// on this specific LoC.
/////////////////////////////////////////////////////////////////
void PathScheduler::setPotentialRes(vector<ResourceType> *potRes, OperationType ot)
{
	if ((ot == SPLIT || ot == MIX || ot == DILUTE))
	{
		if (resTypeExists(BASIC_RES))
			potRes->push_back(BASIC_RES);
		if (resTypeExists(H_RES))
			potRes->push_back(H_RES);
		if (resTypeExists(D_RES))
			potRes->push_back(D_RES);
		if (resTypeExists(DH_RES))
			potRes->push_back(DH_RES);
	}
	else if (ot == DETECT)
	{
		if (resTypeExists(D_RES))
			potRes->push_back(D_RES);
		if (resTypeExists(DH_RES))
			potRes->push_back(DH_RES);
	}
	else if (ot == HEAT)
	{
		if (resTypeExists(H_RES))
			potRes->push_back(H_RES);
		if (resTypeExists(DH_RES))
			potRes->push_back(DH_RES);
	}
}

/////////////////////////////////////////////////////////////////
// Schedules the DAG given the provided DmfbArch.
// *WARNING: This will break a bit if a node has 2+ dispense
// parents of the same exact fluid name
/////////////////////////////////////////////////////////////////
unsigned long long PathScheduler::schedule(DmfbArch *arch, DAG *dag)
{
	//long statPathNo = 0;
	unsigned long long schedTS = 0;
	unsigned long long nextTSCycleEnd = 0;

	getAvailResources(arch);
	resetIoResources(arch);


	// Set priorities and do initializations
	if (internalPriorities)
	{
		//Priority::setAsCritPathDist(dag, this->vl);
		Priority::setAsNumIndPaths(dag);
		//Priority::setAsNumIndThenCritPath(dag, this->vl);
	}

	for (unsigned i = 0; i < dag->allNodes.size(); i++)// Sort nodes by priority so we address the nodes with the least splits first
		Sort::sortNodesByPriorityLoFirst(&(dag->allNodes.at(i)->children));

	commissionDAG(dag); // All nodes initially placed in candidate list are leaders

	//int pathNum = 1;
	while (moreNodesToSchedule())
	{
		Sort::sortNodesByPriorityLoFirst(candidateOps);

		vector<AssayNode*> path;
		vector<AssayNode*> pathIns;
		vector<AssayNode*> pathOuts;
		vector<IoResource*> pathInsRes; // Holds specific io-res used, in case there are multiple of the same type

		AssayNode *n = candidateOps->front();
		if (n->boundedResType != UNKNOWN_RES)
			schedTS = n->startTimeStep + 1; // Already tried scheduling, try starting later
		else
		{
			schedTS = 0;

			// Cannot start path until after last scheduled parent
			for (unsigned p = 0; p < n->parents.size(); p++)
				if (n->parents.at(p)->GetStatus() == SCHEDULED)
					if (n->parents.at(p)->endTimeStep > schedTS)
						schedTS = n->parents.at(p)->endTimeStep;
		}

		unsigned long long earliestScheduableTS = schedTS;
		bool continuePath = true;
		bool mustRestartPathLater = false;
		int runDropsInStorage = getNumIndefStorDropsBefore(schedTS);
		runDropsInStorage += getNumNewIndefStorDrops(schedTS);
		while (continuePath)
		{
			// If there is an unscheduled parent that is not part of the path, we're done with path
			for (unsigned p = 0; p < n->parents.size(); p++)
				if (n->parents.at(p)->GetStatus() != SCHEDULED   &&   n->parents.at(p)->GetType() != DISPENSE)
					if (path.size() == 0 || (path.size() > 0 && n->parents.at(p) != path.back()))
						continuePath = false;
			if (!continuePath)
				break;

			// Find the next gap that can fit the current node on this path for each potential resource type
			unsigned long long duration = ceil((double) n->cycles / (double) arch->getFreqInHz() / arch->getSecPerTS());
			ResourceType earliestRT = UNKNOWN_RES;
			unsigned long long earliestTS = 0;
			unsigned long long beginTS = schedTS;
			//int runDropsInStorage = getNumIndefStorDropsBefore(schedTS);
			//runDropsInStorage += getNumNewIndefStorDrops(schedTS);
			int beginStDrops = runDropsInStorage;
			int endStDrops = beginStDrops;
			bool nodeReadyToSched = true;
			list<IoResource*> bestActivatedIo;
			list<AssayNode*> bestAddedPar;
			///////////////////////////////////////////////////////
			// Make sure the proper resources are available
			///////////////////////////////////////////////////////
			vector<ResourceType> potResTypes;
			setPotentialRes(&potResTypes, n->type);
			for (int i = 0; i < potResTypes.size(); i++)
			{
				ResourceType rt = potResTypes.at(i);
				unsigned long long consecRun = 0;
				bool gapIsFound = false;
				runDropsInStorage = beginStDrops;
				schedTS = beginTS;
				list<IoResource*> recentlyActivatedIo;
				list<AssayNode*> recentlyAddedPar;
				int dropsStoring = 0;
				//bool isStoringDrop = false; // Used in case we have to store droplet before finding a gap
				for (unsigned j = 0; j < n->parents.size(); j++)
					if (n->parents.at(j)->GetStatus() == SCHEDULED)
						dropsStoring++;
						//isStoringDrop = true; // The parent is already part of a path and has been storing

				///////////////////////////////////////////////////////
				// Find a gap for this node/resource
				///////////////////////////////////////////////////////
				while (!gapIsFound)
				{
					// If there is a free chamber and it can be of the type we're looking for, then we can consider
					if (resTypeIsAvailAtTS(schedTS, rt, runDropsInStorage))
					{
						consecRun++;

						// If is 1st TS of operation, need to check if all inputs are ready
						if (consecRun == 1)
						{
							bool nodeReadyToSched = true;

							for (unsigned p = 0; p < n->parents.size(); p++)
							{
								AssayNode *par = n->parents.at(p);
								if (par->GetType() == DISPENSE)
								{
									IoResource *dispWell = NULL;
									if ((dispWell = getReadyDispenseWell(par->GetPortName(), schedTS, earliestScheduableTS)) != NULL)
									{
										setActiveAtTS(dispWell, schedTS); // Since could be 2+ dispense parents
										recentlyActivatedIo.push_back(dispWell);
										recentlyAddedPar.push_back(par);
									}
									else
										nodeReadyToSched = false;
								}
								else if (par->endTimeStep > schedTS)
									nodeReadyToSched = false;
							}

							if (nodeReadyToSched)
								runDropsInStorage -= dropsStoring;
							else
							{	// Restore IoResource changes
								while (!recentlyActivatedIo.empty())
								{
									setInactiveAtTS(recentlyActivatedIo.front(), schedTS);
									recentlyActivatedIo.pop_front();
								}
								recentlyAddedPar.clear();
								consecRun = 0;
							}

						}
						if (consecRun == duration)
						{
							gapIsFound = true;

							// Must reset activations in case another resource-type overlaps
							list<IoResource*>::iterator ioIt = recentlyActivatedIo.begin();
							for (; ioIt != recentlyActivatedIo.end(); ioIt++)
								setInactiveAtTS((*ioIt), (schedTS - duration + 1));

							// If we got a better result from this resource type, then save results
							if (earliestRT == UNKNOWN_RES || (schedTS - duration + 1) < earliestTS)
							{
								earliestRT = rt;
								earliestTS = schedTS - duration + 1; // earliest end
								endStDrops = runDropsInStorage;

								// Save for activation later if remains the best
								bestActivatedIo.swap(recentlyActivatedIo);
								bestAddedPar.swap(recentlyAddedPar);
								recentlyActivatedIo.clear();
								recentlyAddedPar.clear();
								break; // from while(!gapIsFound)
							}
						}
					}
					else //if (path.size() > 0) // If not leader
					{	// Path leader will never need to account for an extra storage droplet here
						if (/*!isStoringDrop*/ dropsStoring == 0 && path.size() > 0)
						{
							runDropsInStorage++;
							//isStoringDrop = true;
							dropsStoring++;
						}

						while (!recentlyActivatedIo.empty())
						{
							setInactiveAtTS(recentlyActivatedIo.front(), schedTS - consecRun);
							recentlyActivatedIo.pop_front();
						}
						recentlyAddedPar.clear();

						if (canStoreDropsAtTS(schedTS, runDropsInStorage))
							consecRun = 0; // Start the run over
						else
							mustRestartPathLater = true; // Can't find a gap b/c ran out of storage
					}
					if (!mustRestartPathLater)
					{
						schedTS++;
						runDropsInStorage += getNumNewIndefStorDrops(schedTS);
					}
					else
						break; // from while(!gapIsFound);
				} // End while(!gapIsFound)
			} // End for(all potential resources)

			if (mustRestartPathLater)
				break; // from while(continuePath)

			////////////////////////////////////////////////////////////////
			// If we made it here, we successfully found a valid spot, for
			// the current AssayNode (n)...add to path.
			////////////////////////////////////////////////////////////////
			// Set and add dispense parent resources properly
			while (!bestAddedPar.empty())
			{
				IoResource * bDisp = bestActivatedIo.front();
				AssayNode * bPar = bestAddedPar.front();
				bPar->startTimeStep = earliestTS - bDisp->durationInTS;
				bPar->endTimeStep = earliestTS;
				setActiveAtTS(bDisp, earliestTS);
				pathIns.push_back(bPar);
				pathInsRes.push_back(bDisp);
				bestAddedPar.pop_front();
				bestActivatedIo.pop_front();
			}

			// Save schedule for node
			n->boundedResType = earliestRT;
			n->startTimeStep = earliestTS;
			n->endTimeStep = earliestTS + duration;
			path.push_back(n);

			// Obtain next path node
			AssayNode *nextPathNode = NULL;
			for (unsigned i = 0; i < n->children.size(); i++)
			{
				if (n->children.at(i)->GetStatus() != SCHEDULED && n->children.at(i)->GetType() != OUTPUT)
				{
					nextPathNode = n->children.at(i);
					break;
				}
			}

			// Now add any outputs to path, if applicable, as well as new storage if unaccounted-for splits
			schedTS = earliestTS + duration - 1;
			runDropsInStorage = endStDrops;
			for (unsigned i = 0; i < n->children.size(); i++)
			{
				AssayNode *c = n->children.at(i);
				if (c->GetType() == OUTPUT)
				{
					c->startTimeStep = earliestTS + duration;
					c->endTimeStep = c->startTimeStep + 1;
					pathOuts.push_back(c);
				}
				if (!(c == nextPathNode || c->GetStatus() == SCHEDULED || c->GetType() == OUTPUT))
					runDropsInStorage++; // New indefinite storage
			}

			if (nextPathNode)
				n = nextPathNode;
			else
				break; // No valid child, path is done...break from while(continuePath)
		} // while (continuePath)

		///////////////////////////////////////////////////////
		// At this point, we either throw away the path b/c it
		// must be scheduled later, or submit/finalize the path
		///////////////////////////////////////////////////////
		if (mustRestartPathLater)
		{ 	// Throw away path
			for (unsigned pi = 0; pi < pathIns.size(); pi++)
				setInactiveAtTS(pathInsRes.at(pi), pathIns.at(pi)->endTimeStep);
		}
		else
		{ 	// Submit path
			//statPathNo++;
			//if (statPathNo % 100)
			//cout << "P# " << statPathNo << endl; // DTG DEBUG
			candidateOps->remove(path.front());
			for (unsigned po = 0; po < pathOuts.size(); po++)
			{
				pathOuts.at(po)->status = SCHEDULED;
				//cout << "DEBUG: Output scheduled: "; // DTG debug
				//pathOuts.at(po)->Print(); // DTG debug
			}
			for (unsigned pi = 0; pi < pathIns.size(); pi++)
			{
				pathIns.at(pi)->status = SCHEDULED; // Easier to schedule all before next step
				pathInsRes.at(pi)->schedule.push_back(pathIns.at(pi));
				//cout << "DEBUG: Input scheduled: "; // DTG debug
				//pathIns.at(pi)->Print(); // DTG debug
			}
			for (unsigned pi = 0; pi < path.size(); pi++)
				path.at(pi)->status = SCHEDULED; // easier to do in advanced

			for (unsigned pi = 0; pi < path.size(); pi++)
			{
				AssayNode *pathNode = path.at(pi);
				pathNode->status = SCHEDULED;

				// Insert indefinite storage for any hanging children
				for (unsigned pc = 0; pc < pathNode->GetChildren().size(); pc++)
				{ // Add to indefStorage for the children
					AssayNode *cNode = pathNode->GetChildren().at(pc);
					map<unsigned long long, list<AssayNode*>*>::iterator cIt;
					if (cNode->GetStatus() != SCHEDULED)
					{
						if ((pathNode->GetType() == SPLIT || pathNode->GetType() == DILUTE) && cNode->GetType() != OUTPUT)
							candidateOps->push_back(cNode); // Is a new leader
						cIt = indefStorage->find(pathNode->endTimeStep);
						if (cIt == indefStorage->end())
						{
							list<AssayNode *> * listStorage = new list<AssayNode *> ();
							listStorage->push_back(cNode);
							indefStorage->insert(pair<unsigned long long, list<AssayNode*>*> (pathNode->endTimeStep,listStorage));
						}
						else
							cIt->second->push_back(cNode);
					}

				}

				// Remove all parents from the indefStorage list
				for (unsigned pp = 0; pp < pathNode->GetParents().size(); pp++)
				{	// Only do if not a dispense
					AssayNode *pNode = pathNode->GetParents().at(pp);

					map<unsigned long long, list<AssayNode*>*>::iterator pIt;
					pIt = indefStorage->find(pNode->endTimeStep);

					if (pIt != indefStorage->end())
					{
						list<AssayNode*>::iterator nIt = pIt->second->begin();
						for (; nIt != pIt->second->end(); nIt++)
						{
							if ((*nIt) == pathNode)
							{
								pIt->second->erase(nIt);
								break;
							}
						}
					}
				}

				// Update history elements available resources and storage
				for (unsigned long long dur = pathNode->startTimeStep; dur < pathNode->endTimeStep; dur++)
					getResHistAtTS(dur)->availRes[pathNode->boundedResType]--;
				for (unsigned pp = 0; pp < pathNode->GetParents().size(); pp++)
				{
					AssayNode *pNode = pathNode->GetParents().at(pp);
					for (unsigned long long sts = pNode->endTimeStep; sts < pathNode->startTimeStep; sts++) // storage time-step
						getResHistAtTS(sts)->dropsInStorage++;
				}
			}
		}
		/*stringstream ssOut;
		ssOut << "output/partialPathSched" << pathNum;
		dag->OutputGraphFile(ssOut.str(), true, true); // DTG DEBUG
		cout << "Path " << pathNum++ << endl;*/

	} // while (moreNodesToSchedule())


	// If we have transfer-outs, make sure they're scheduled at very end of
	// DAG so we can insert storage nodes between them and their parent
	unsigned maxOutTS = 0;
	for (unsigned i = 0; i < dag->tails.size(); i++)
		if (maxOutTS < dag->tails.at(i)->startTimeStep && dag->tails.at(i)->parents.front()->GetType() != DISPENSE)
			maxOutTS = dag->tails.at(i)->startTimeStep;
	unsigned minInTS = maxOutTS;
	for (unsigned i = 0; i < dag->heads.size(); i++)
		if (minInTS > dag->heads.at(i)->startTimeStep && dag->heads.at(i)->children.front()->GetType() != OUTPUT)
			minInTS = dag->heads.at(i)->startTimeStep;
	if (minInTS < 1)
		minInTS = 1;

	map<unsigned long long, int> dropsInStorageAtTS;
	map<unsigned long long, int>::iterator disIt;

	// The drops in storage seems to be a little off here b/c it is perhaps a bit
	// conservative when reserving space for storage (reserves more than necessary);
	// Thus, we will reset these values here and then reset according to actual
	// storage nodes generated.
	//map<unsigned long long, ResHist*>::iterator rhIt = availResAtTS->begin();
	//for (; rhIt != availResAtTS->end(); rhIt++)
	//	rhIt->second->dropsInStorage = 0;

	// Add individual storage nodes
	vector<AssayNode *> storeInsert;
	vector<AssayNode *> childInsert;
	for (unsigned i = 0; i < dag->allNodes.size(); i++)
	{
		AssayNode *p = dag->allNodes.at(i);
		for (unsigned j = 0; j < p->children.size(); j++)
		{
			AssayNode *c = p->children.at(j);
			if (p->endTimeStep < c->startTimeStep)
			{
				AssayNode *s = dag->AddStorageNode();
				s->status = SCHEDULED;
				s->startTimeStep = p->endTimeStep;
				s->endTimeStep = c->startTimeStep;
				storeInsert.push_back(s);
				childInsert.push_back(c);
				//dag->InsertNode(p, c, s); // Must insert later b/c will affect for() loop looking at children

				// Add actual droplets in storage for each TS so we can make storage holder nodes soon
				//for (unsigned long long k = s->startTimeStep; k < s->endTimeStep; k++)
				//{
				//	rhIt = availResAtTS->find(k);
				//	rhIt->second->dropsInStorage++;
				//	//cout << "Drops in storage at TS " << k << " = " << rhIt->second->dropsInStorage << endl;
				//}

			}
		}
		// Now, insert node into DAG (can't do earlier or will impact loop
		for (unsigned j = 0; j < storeInsert.size(); j++)
			dag->InsertNode(p, childInsert.at(j), storeInsert.at(j));
		storeInsert.clear();
		childInsert.clear();
	}



	// Add storage holder nodes for binding
	map<unsigned long long, ResHist*>::iterator rhIt = availResAtTS->begin();
	rhIt = availResAtTS->begin();
	for (; rhIt != availResAtTS->end(); rhIt++)
	{
		ResHist *rh = rhIt->second;
		if (rh->timeStep >= minInTS && rh->timeStep < maxOutTS)
		{
			int dis = rh->dropsInStorage;
			while (dis > 0)
			{
				//AssayNode *node = dag->AddStorageHolderNode();
				//node->startTimeStep = rh->timeStep;
				//node->endTimeStep = rh->timeStep + 1;
				//node->cycles = (node->GetEndTS()-node->GetStartTS())* (arch->getFreqInHz() * arch->getSecPerTS());
				if (dis >= getMaxStoragePerModule())
					dis -= getMaxStoragePerModule();
				else
					dis -= dis;

				// Assign resource type
				if (rh->availRes[BASIC_RES] > 0)
				{
					rh->availRes[BASIC_RES]--;
					//node->boundedResType = BASIC_RES;
				}
				else if (rh->availRes[H_RES] > 0)
				{
					rh->availRes[H_RES]--;
					//node->boundedResType = H_RES;
				}
				else if (rh->availRes[D_RES] > 0)
				{
					rh->availRes[D_RES]--;
					//node->boundedResType = D_RES;
				}
				else
				{
					rh->availRes[DH_RES]--;
					//node->boundedResType = DH_RES;
				}
			}
		}
	}

	if (internalPriorities)
		cout << "PS TIME: " << maxOutTS + 1 << endl;

	return (maxOutTS + 1);
}

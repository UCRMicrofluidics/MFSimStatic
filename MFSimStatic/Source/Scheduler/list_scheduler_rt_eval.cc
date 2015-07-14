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
 * Source: list_scheduler_rt_eval.cc											*
 * Original Code Author(s): Dan Grissom											*
 * Original Completion/Release Date: November 5, 2013							*
 *																				*
 * Details: N/A																	*
 *																				*
 * Revision History:															*
 * WHO		WHEN		WHAT													*
 * ---		----		----													*
 * FML		MM/DD/YY	One-line description									*
 *-----------------------------------------------------------------------------*/
#include "../../Headers/Scheduler/list_scheduler_rt_eval.h"
#include "../../Headers/Util/sort.h"
#include <stdlib.h>
#include <sys/time.h>

///////////////////////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////////////////////
RealTimeEvalListScheduler::RealTimeEvalListScheduler()
{
}

///////////////////////////////////////////////////////////////////////////////////
// Deconstructor
///////////////////////////////////////////////////////////////////////////////////
RealTimeEvalListScheduler::~RealTimeEvalListScheduler()
{
}

/////////////////////////////////////////////////////////////////
// Tells whether a dispense well containing the specified fluid
// is available at the current timestep
/////////////////////////////////////////////////////////////////
IoResource * RealTimeEvalListScheduler::getReadyDispenseWell(string fluidName, unsigned long long schedTS)
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
unsigned long long RealTimeEvalListScheduler::schedule(DmfbArch *arch, DAG *dag)
{
	// There can be a variation on operations, typically 10% of the operation, so we reduce
	// the time-step length to allow for easier scheduling and flexibility for variable-length
	// operation scheduling.
	arch->setSecPerTS(arch->getSecPerTS() / 10.0);
	//arch->setSecPerTS(arch->getSecPerTS() / arch->getFreqInHz()); // Min-theoretical time-step length

	{
		stringstream msg("");
		msg << "The seconds-per-timestep (" << arch->getSecPerTS() << ") must be greater than or equal to the frequency period (";
		msg <<  arch->getFreqInHz() << "Hz = " << (1.0 / arch->getFreqInHz()) << "s)";
		claim (arch->getSecPerTS() >= (1.0 / arch->getFreqInHz()), &msg);
	}


	// Set random seed
	unsigned int seed = time(0);
	cout << "Random seed for Real-time evaluation list scheduler: " << seed << endl;
	//unsigned int seed = 1383780608;
	//cout << "Hard-coded seed for Real-time evaluation list scheduler: " << seed << endl;
	srand(seed);


	// Test for generating random numbers according to Gaussian Dist. (Passed)
    /*int numQuantiles = 60;
	int dist[60] = {0};
    int midIndex = 30; // First positive quantile
    float quantileSize = 1.0 / 10.0; // For normal dist, stdDev == 1

	// Generate 1000 random variables according to http://www.design.caltech.edu/erik/Misc/Gaussian.html
	for (int i = 0; i < 1000; i++)
	{
        float x1, x2, w, y1, y2;

        do {
                x1 = 2.0 * (((float)rand()/(1.0+(float)RAND_MAX))) - 1.0;
                x2 = 2.0 * (((float)rand()/(1.0+(float)RAND_MAX))) - 1.0;
                w = x1 * x1 + x2 * x2;
        } while ( w >= 1.0 );

        w = sqrt( (-2.0 * log( w ) ) / w );
        y1 = x1 * w;
        y2 = x2 * w;


        // Add y1
        bool isNeg = false;
        if (y1 < 0)
        {
        	isNeg = true;
        	y1 = y1*-1.0; // Get abs value
        }
        for (int q = 0; q < numQuantiles - midIndex; q++)
        {
        	//cout << y1 << "<?" << (((float)(q+1))*quantileSize) << endl;
        	if (y1 < (((float)(q+1))*quantileSize))
        	{
        		if (isNeg)
        			dist[midIndex-q]++;
        		else
        			dist[midIndex+q]++;
        		break;
        	}
        }

        // Add y2
        isNeg = false;
        if (y2 < 0)
        {
        	isNeg = true;
        	y2 = y2*-1.0; // Get abs value
        }
        for (int q = 0; q < numQuantiles - midIndex; q++)
        {
        	//cout << y2 << "<?" << (((float)(q+1))*quantileSize) << endl;
        	if (y2 < (((float)(q+1))*quantileSize))
        	{
        		if (isNeg)
        			dist[midIndex-q]++;
        		else
        			dist[midIndex+q]++;
        		break;
        	}
        }
	}

	// Now test by printing out distribution
	for (int i = 0; i < numQuantiles; i++)
	{
		cout << i << " ";
		for (int j = 0; j < dist[i]; j++)
			cout << "*";
		cout << endl;
	}*/



	// Now, for all dilutes and mixes, we must add/subtract some variable length of time
	// Generate a random variable using the polar-form of the Box-Muller transformation, which
	// abides by the normal distribution, then multiply by sigma and add mu to get gaussian
	// distribution
	for (unsigned m = 0; m < dag->mixes.size(); m++)
	{
		// Computer average/standard deviation
		AssayNode *mixNode = dag->mixes.at(m);
		float average = (float)mixNode->seconds; // mu == expected operation time
		float stdDev = average / 10.0; // sigma == 10% expected operation time

		// Generate random numbers (can pick y1 or y2)
        float x1, x2, w, y1, y2;
        do {
                x1 = 2.0 * (((float)rand()/(1.0+(float)RAND_MAX))) - 1.0;
                x2 = 2.0 * (((float)rand()/(1.0+(float)RAND_MAX))) - 1.0;
                w = x1 * x1 + x2 * x2;
        } while ( w >= 1.0 );
        w = sqrt( (-2.0 * log( w ) ) / w );
        y1 = x1 * w;
        y2 = x2 * w;

        float adjustedTime = average + (stdDev * y1);
        mixNode->seconds = adjustedTime;
        mixNode->SetNumCycles( (unsigned)ceil((double)arch->getFreqInHz() * mixNode->GetNumSeconds()) );
	}
	for (unsigned d = 0; d < dag->dilutes.size(); d++)
	{
		// Computer average/standard deviation
		AssayNode *diluteNode = dag->dilutes.at(d);
		float average = (float)diluteNode->seconds; // mu == expected operation time
		float stdDev = average / 10.0; // sigma == 10% expected operation time

		// Generate random numbers (can pick y1 or y2)
        float x1, x2, w, y1, y2;
        do {
                x1 = 2.0 * (((float)rand()/(1.0+(float)RAND_MAX))) - 1.0;
                x2 = 2.0 * (((float)rand()/(1.0+(float)RAND_MAX))) - 1.0;
                w = x1 * x1 + x2 * x2;
        } while ( w >= 1.0 );
        w = sqrt( (-2.0 * log( w ) ) / w );
        y1 = x1 * w;
        y2 = x2 * w;

        float adjustedTime = average + (stdDev * y1);
        diluteNode->seconds = adjustedTime;
        diluteNode->SetNumCycles( (unsigned)ceil((double)arch->getFreqInHz() * diluteNode->GetNumSeconds()) );
	}





	int numModules = getAvailResources(arch);
	int maxLocDrops = ((numModules-1)*getMaxStoragePerModule()) + 1;
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
				if (node->boundedResType == BASIC_RES)
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
			numStorageModules = ceil(((double)dropsInStorage + (double)netStorageDropsGain)/ (double)getMaxStoragePerModule());
			if (parentsDone && dropsInLoc + netLocDropsGain <= maxLocDrops &&
					(unfinishedOps->size() + 1) + numStorageModules <= numModules)
			{
				// Determine if there is an applicable, available resource
				bool canSchedule = false;
				if ((n->type == SPLIT) && (availRes[BASIC_RES] + availRes[D_RES] + availRes[H_RES] + availRes[DH_RES]) > 0)
				{
					canSchedule = true;
					if (availRes[BASIC_RES] > 0)
					{
						availRes[BASIC_RES]--;
						n->boundedResType = BASIC_RES;
					}
					else if (availRes[H_RES] > 0)
					{
						availRes[H_RES]--;
						n->boundedResType = H_RES;
					}
					else if (availRes[D_RES] > 0)
					{
						availRes[D_RES]--;
						n->boundedResType = D_RES;
					}
					else
					{
						availRes[DH_RES]--;
						n->boundedResType = DH_RES;
					}
				}
				else if ((n->type == DETECT || n->type == MIX || n->type == DILUTE ) && (availRes[D_RES] + availRes[DH_RES]) > 0)
				{
					canSchedule = true;
					if (availRes[D_RES] > 0)
					{
						availRes[D_RES]--;
						n->boundedResType = D_RES;
					}
					else
					{
						availRes[DH_RES]--;
						n->boundedResType = DH_RES;
					}
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
			//AssayNode *node = dag->AddStorageHolderNode();
			//node->startTimeStep = schedTS;
			//node->endTimeStep = schedTS + 1;
			//node->cycles = (node->GetEndTS()-node->GetStartTS())* (arch->getFreqInHz() * arch->getSecPerTS());
			if (dis >= getMaxStoragePerModule())
				dis -= getMaxStoragePerModule();
			else
				dis -= dis;

			// Reserve a resource type, but don't need to assign here (will do in placer/binder)
			if (ar[BASIC_RES] > 0)
			{
				ar[BASIC_RES]--;
				//node->boundedResType = BASIC_RES;
			}
			else if (ar[H_RES] > 0)
			{
				ar[H_RES]--;
				//node->boundedResType = H_RES;
			}
			else if (ar[D_RES] > 0)
			{
				ar[D_RES]--;
				//node->boundedResType = D_RES;
			}
			else
			{
				ar[DH_RES]--;
				//node->boundedResType = DH_RES;
			}
		}


		schedTS++;

		// If we failed to schedule any operations, and there are more left,
		// then LS cannot schedule this assay given the current priorities
		int freeModules = 0;
		for (int i = 0; i <= RES_TYPE_MAX; i++)
			freeModules += ar[i];
		if (unfinishedOps->empty() && moreNodesToSchedule() && freeModules == 0)
		{
			claim(!internalPriorities, "List Scheduler failed to produce a valid schedule.");
			return 1000000;
		}

		//if (schedTS > 1000)
		//	return 10000;

		//cout << "storage: " << dropsInStorage << endl; // dtg debug
		//cout << "loc: " << dropsInLoc << endl; // dtg debug
	}

	if (internalPriorities)
	{
		cout << "LS Time: " << schedTS-1 << " time-steps (" << (double)(schedTS-1) * arch->getSecPerTS() << "s";
		cout << " with " << arch->getSecPerTS() << "s time-steps)"<< endl;
	}

	return schedTS;
}


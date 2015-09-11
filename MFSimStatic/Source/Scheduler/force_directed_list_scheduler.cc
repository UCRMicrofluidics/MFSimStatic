/*------------------------------------------------------------------------------*
 *                       (c)2014, All Rights Reserved.     						*
 *       ___           ___           ___     									*
 *      /__/\         /  /\         /  /\    									*
 *      \  \:\       /  /:/        /  /::\   									*
 *       \  \:\     /  /:/        /  /:/\:\  									*
 *   ___  \  \:\   /  /:/  ___   /  /:/~/:/        								*
 *  /__/\  \__\:\ /__/:/  /  /\ /__/:/ /:/___    UCR DMFB Synthesis Framework	*
 *  \  \:\ /  /:/ \  \:\ /  /:/ \  \:\/:::::/    www.microfluidics.cs.ucr.edu	*
 *   \  \:\  /:/   \  \:\  /:/   \  \::/~~~~ 									*
 *    \  \:\/:/     \  \:\/:/     \  \:\     									*
 *     \  \::/       \  \::/       \  \:\    									*
 *      \__\/         \__\/         \__\/    									*
 *-----------------------------------------------------------------------------*/
/*---------------------------Implementation Details-----------------------------*
 * Source: force_directed_list_scheduler.cc										*
 * Original Code Author(s): Kenneth O' Neal										*
 * Original Completion/Release Date: October 7, 2012							*
 *																				*
 * Details: N/A																	*
 *																				*
 * Revision History:															*
 * WHO		WHEN		WHAT													*
 * ---		----		----													*
 * FML		MM/DD/YY	One-line description									*
 *-----------------------------------------------------------------------------*/

#include "../../Headers/Scheduler/force_directed_list_scheduler.h"

///////////////////////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////////////////////
FDLScheduler::FDLScheduler(){}

///////////////////////////////////////////////////////////////////////////////////
// Deconstructor
///////////////////////////////////////////////////////////////////////////////////
FDLScheduler::~FDLScheduler() {}

unsigned maxv = 0;

///////////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////////
void FDLScheduler::reset_priority(DAG * dag)
{
	for (unsigned i = 0; i < dag->allNodes.size(); i++)
		dag->allNodes.at(i)->priority = 0;
}

///////////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////////
void FDLScheduler::setCritPathDist(DAG *dag, DmfbArch *arch)
{
	reset_priority(dag);
	for (unsigned i = 0; i < dag->tails.size(); i++)
	{
		recursiveCPD(arch, dag->tails.at(i), 0);
	}
}

///////////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////////
void FDLScheduler::recursiveCPD(DmfbArch *arch, AssayNode *node, unsigned childDist)
{

	if (node->GetType() == DISPENSE)
	{
		node->cycles = arch->getIoPort(node->portName)->getTimeInSec() * arch->getFreqInHz();
	}

	unsigned temp = max(node->priority, childDist + node->cycles);
	node->priority = temp;
	if(temp > maxv)
	{
		maxv = temp;
	}

	for (unsigned i = 0; i < node->parents.size(); i++)
		recursiveCPD(arch, node->parents.at(i), node->priority);

}

///////////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////////
void recursive_priori(unsigned prior, AssayNode* node)
{
	node->SetPriority(prior);
	for (unsigned i = 0; i < node->GetParents().size(); i++)
		recursive_priori(node->GetPriority(), node->GetParents().at(i));
}

///////////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////////
void FDLScheduler::setAsLongestPathDist(DAG *dag, DmfbArch *arch)
{
	reset_priority(dag);
	for (unsigned i = 0; i < dag->heads.size(); i++)
		recursiveLPD(dag->heads.at(i), 0, arch);
}

///////////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////////
void FDLScheduler::recursiveLPD(AssayNode *node, unsigned childDist, DmfbArch *arch)
{
	if(node->GetType() == OUTPUT)
	{
		node->priority = (max(node->priority, childDist + 1));
	}
	else
		node->priority = (max((double)node->priority, (double)childDist + ceil((float)node->cycles/(float)arch->getFreqInHz()/arch->getSecPerTS())));
	for (unsigned i = 0; i < node->children.size(); i++)
		recursiveLPD(node->children.at(i), node->priority, arch);
}

///////////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////////
vector< pair<AssayNode* , float > >  FDLScheduler::ASAP(DAG * dag, DmfbArch *arch)
{
	Dag_reinitialize(dag);

	for(unsigned i = 0; i < dag->heads.size(); i++) //schedule all input nodes first
	{
		dag->heads.at(i)->status = SCHEDULED;
		dag->heads.at(i)->startTimeStep = 0;
		dag->heads.at(i)->endTimeStep = 1;
	}
	vector<AssayNode*> unfinished;
	for(unsigned i =0; i<dag->allNodes.size(); i++) // nodes not finished
	{
		if(!(dag->allNodes.at(i)->status == SCHEDULED))
		{
			unfinished.push_back(dag->allNodes.at(i));
		}
	}
	unsigned TS2 = 0;
	while(!unfinished.empty()) //while nodes to schedule
	{
		for(unsigned i = 0; i<dag->allNodes.size(); i++) // for every node left to schedule
		{
			bool status = true;
			if(!(dag->allNodes.at(i)->status == SCHEDULED))
			{
				for(unsigned j = 0; j<dag->allNodes.at(i)->parents.size(); j++) //for all of the current nodes parents
				{
					//if parents not finished, set flag indicating to not schedule current node
					if(!(dag->allNodes.at(i)->parents.at(j)->status == SCHEDULED && dag->allNodes.at(i)->parents.at(j)->endTimeStep < TS2))
					{
						status = false;
					}
				}
				if(status == true)
				{
					dag->allNodes.at(i)->status = SCHEDULED;
					dag->allNodes.at(i)->startTimeStep = TS2;
					dag->allNodes.at(i)->endTimeStep = TS2 + ceil((float)dag->allNodes.at(i)->cycles/(float)arch->getFreqInHz()/arch->getSecPerTS());

					for(unsigned k = 0; k< unfinished.size(); k++)
					{
						if(unfinished.at(k)->name == dag->allNodes.at(i)->name)
						{
							unfinished.erase(unfinished.begin()+k);
						}
					}
				}
				//flag not set, so schedule

			}

		}
		//increment current timestep
		TS2++;
	}

	vector< pair < AssayNode*, float > > ASAP_vals; //push back node and schedule time into array for future use.
	for(unsigned i = 0; i<dag->allNodes.size();i++)
	{
		ASAP_vals.push_back(pair< AssayNode*, float > (dag->allNodes.at(i), dag->allNodes.at(i)->startTimeStep));
	}


	return ASAP_vals;


}

///////////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////////
void FDLScheduler::setAsLongestPathDistT(DAG *dag, DmfbArch * arch, float max_latency)
{
	reset_priority(dag);
	for (unsigned i = 0; i < dag->tails.size(); i++)
		recursiveLPDT(dag->tails.at(i), 0, arch, max_latency);
}

///////////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////////
void FDLScheduler::recursiveLPDT(AssayNode *node, unsigned childDist, DmfbArch * arch, float max_latency)
{
	if(node->GetType() == OUTPUT)
	{
		node->priority = (max(node->priority, childDist + 1));
	}
	else
		node->priority = (max((double)node->priority, (double)childDist + ceil((float)node->cycles/(float)arch->getFreqInHz()/arch->getSecPerTS())));
	for (unsigned i = 0; i < node->parents.size(); i++)
		recursiveLPDT(node->parents.at(i), node->priority, arch, max_latency);

}

///////////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////////
vector< pair<AssayNode* , float> >  FDLScheduler::ALAP(DAG * dag, float max_latency, DmfbArch *arch)
{
	Dag_reinitialize(dag);

	for(unsigned i = 0; i < dag->tails.size(); i++)//schedule tails (outputs) first
	{
		dag->tails.at(i)->status = SCHEDULED;
		dag->tails.at(i)->startTimeStep = 0;
		dag->tails.at(i)->endTimeStep = 1;
	}
	vector<AssayNode*> Not_finished;
	for(unsigned i =0; i<dag->allNodes.size(); i++)//nodes not finished yet
	{
		if(!(dag->allNodes.at(i)->status == SCHEDULED))
		{
			Not_finished.push_back(dag->allNodes.at(i));
		}
	}
	unsigned TS = 0;
	while(!Not_finished.empty()) // while nodes to schedule
	{
		for(unsigned i = 0; i<dag->allNodes.size(); i++) // for each node left to schedule
		{
			bool status = true;
			if(!(dag->allNodes.at(i)->status == SCHEDULED))
			{
				for(unsigned j = 0; j<dag->allNodes.at(i)->children.size(); j++) // for all the children at the current node
				{
					//if not finished set flag
					if(!(dag->allNodes.at(i)->children.at(j)->status == SCHEDULED && dag->allNodes.at(i)->children.at(j)->endTimeStep < TS) )
					{
						status = false;
					}
				}
				//flag not set, so can schedule
				if(status == true)
				{
					dag->allNodes.at(i)->status = SCHEDULED;
					dag->allNodes.at(i)->startTimeStep = TS;
					dag->allNodes.at(i)->endTimeStep = TS + ceil((float)dag->allNodes.at(i)->cycles/(float)arch->getFreqInHz()/arch->getSecPerTS());
					//Not_finished.erase(Not_finished.begin()+i);
					for(unsigned k = 0; k< Not_finished.size(); k++)
					{
						if(Not_finished.at(k)->name == dag->allNodes.at(i)->name)
						{
							Not_finished.erase(Not_finished.begin()+k);
						}
					}
				}
			}
		}
		TS++;
	}

	unsigned Maxts = 0;
	for(unsigned i = 0; i<dag->allNodes.size();i++)//find max time step
	{
		if(dag->allNodes.at(i)->endTimeStep > Maxts){
			Maxts = dag->allNodes.at(i)->startTimeStep;//16 bet so far

		}

	}
	vector< pair < AssayNode*, float > > ALAP_vals;
	for(unsigned i = 0; i<dag->allNodes.size();i++)
	{
		//schedule time is MaxTS  - start time to account for reverse scheduling
		ALAP_vals.push_back(pair< AssayNode*, float > (dag->allNodes.at(i), (Maxts - dag->allNodes.at(i)->startTimeStep)));
	}
	return ALAP_vals;
}

///////////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////////
vector<n_vals*> FDLScheduler::DG_Inputs(vector< pair< AssayNode *, float> > ASAP_vals, vector< pair< AssayNode *, float> > ALAP_vals)
{
	vector< pair<AssayNode*, vector<float> > > node_posTS; // pair of assayNode, pair of float =prior value, vector<float> = all possible TS
	vector<pair<AssayNode*, float> > prior_vec;
	vector<n_vals*> vals;
	float priori = 0.0;
	vector<float> TS;
	for(unsigned i = 0; i < ASAP_vals.size(); i++) //for each asap val
	{
		for(unsigned j = 0; j< ALAP_vals.size(); j++) //for each alap val
		{
			if(ALAP_vals.at(j).first->name == ASAP_vals.at(i).first->name) // if asap node is same as alap node
			{
				if(ALAP_vals.at(j).second == ASAP_vals.at(i).second) // and if asap val is same as alap val
				{
					priori = 1.0; //priority is one because only one poss TS for that node 1/1 = 1
					TS.push_back(ASAP_vals.at(i).second); // timestep is timestep of current nodes
					n_vals* temp1 = new n_vals; //create new nval and set values
					temp1->n = ASAP_vals.at(i).first;
					temp1->poss_TS = TS;
					temp1->prior_val = priori;
					vals.push_back(temp1);
				}
				else
				{
					float slack = fabs(ALAP_vals.at(j).second - ASAP_vals.at(j).second); //else DG val is 1/number of timesteps +1
					priori = 1 / (slack+1);
				}
				unsigned k_low = ASAP_vals.at(i).second;
				unsigned k_high = ALAP_vals.at(j).second;
				if(k_low > k_high) //if asap timestep value is lower than alap value, switch them
				{
					unsigned temp_k = k_low;;
					k_low = k_high;
					k_high = temp_k;
				}
				TS.clear();
				for(unsigned k = k_low; k<= k_high; k++ )
				{
					TS.push_back(k); //push back timestep value for every timestep in range.
				}
				n_vals* temp2 = new n_vals; //delete this after each iteration of while loops in schedule()
				temp2->n = ASAP_vals.at(i).first;
				temp2->poss_TS = TS;
				temp2->prior_val = priori;
				vals.push_back(temp2); //push back node, possts vector, priority value association
			}
		}
	}
	return vals;
}

///////////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////////
pair< vector<unsigned>, vector<float> > FDLScheduler::DG_evaluate(vector<n_vals*> temp)
{
	vector< pair< unsigned, float> > DG_val;
	vector <unsigned> TS;
	vector <float> prior;
	for(unsigned i = 0; i< temp.size(); i++) //for every node in dg values
	{
		for(unsigned j = 0; j<temp.at(i)->poss_TS.size(); j++) //for every possible time step of each node
		{
			vector< unsigned >::iterator it = find(TS.begin(), TS.end(), temp.at(i)->poss_TS.at(j)); //checking to see if this timestep has been visited yet

			if (it != TS.end())
			{
				// found it
				int it_val = it-TS.begin();
				prior.at(it_val) = prior.at(it_val) + temp.at(i)->prior_val; //priority is priority is sum of old value and this new priority
			}
			else
			{
				// doesn't exist
				TS.push_back(temp.at(i)->poss_TS.at(j)); //put it onto visited list with prior equal to dg val from input
				prior.push_back(temp.at(i)->prior_val);
			}
		}

	}
	pair < vector<unsigned>, vector<float> > DG = pair < vector<unsigned>, vector<float> >(TS, prior); //return pair of timesteps and final DG_vals
	return DG;
}
float differ = 0.0;
float IFD = 0.0;

///////////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////////
vector <n_forces*> FDLScheduler::force_calc (vector < n_vals* > np, pair <vector<unsigned>, vector<float> > DG)
{
	float self_force;
	//float pred_force;
	float succ_force;
	//float succ2_force;
	float DG_val;
	float DG_val2;
	vector<n_forces*> force;
	//float dif = 0.0;
	for(unsigned j = 0; j < np.size(); j++) // for every possible node
	{
		for(unsigned k = 0; k < np.at(j)->poss_TS.size(); k++ ) // for every possible TS for that node
		{


			vector< unsigned >::iterator it = find(DG.first.begin(), DG.first.end(), np.at(j)->poss_TS.at(0));// at the timestep in DG_graph
			vector< unsigned >::iterator it3;
			if (it != DG.first.end()) //if you found it
			{
				vector<unsigned> ::iterator it2 = DG.first.begin();
				int temp = it-it2; //temp is value representing from beginning of dg to timestep found (it)
				DG_val = DG.second.at(temp); //DG_val is value for TS in graph

				self_force = DG_val * np.at(j)->prior_val;

			}

			else //if you didnt find it you've got a problem
			{
				// doesn't exist
				cerr<<"....................TS location not found........Exiting\n\n";
				exit(1); //this should never happen, error!
			}
			if(np.at(j)->poss_TS.size() > 1 )
			{
				if(it3 != DG.first.end())
				{
					it3 = find(DG.first.begin(), DG.first.end(), np.at(j)->poss_TS.at(1));
					vector<unsigned> :: iterator it4 = DG.first.begin();
					int temp2 = it3-it4;
					DG_val2 = DG.second.at(temp2);
					succ_force = DG_val2 * np.at(j)->prior_val;
				}
				else //if you didnt find it you've got a problem
				{
					// doesn't exist
					cerr<<"....................TS location not found........Exiting\n\n";
					exit(1); //this should never happen, error!
				}

			}
			else succ_force = self_force;

			unsigned TS = np.at(j)->poss_TS.at(k); //TS for force_val is current TS being looked at for current node
			AssayNode* n;
			n = np.at(j)->n;

			n_forces* val = new n_forces; // delete this after each iteration of while loop in schedule()

			val->TS = TS; // set time step for current node
			val->force = (self_force + succ_force);//succ_force+self_force;//+pred_force;//self_force+pred_force+succ_force;//force is sum of self force, pred force, succ force
			val->n = n;
			force.push_back(val);

		}
	}

	return force;

}

/////////////////////////////////////////////////////////////////////////////////////
// Since FDLScheduler calls ListScheduler multiple times, it must reinitialize
// the DAG each time it generates a schedule.
/////////////////////////////////////////////////////////////////////////////////////
void FDLScheduler::Dag_reinitialize(DAG * dag)
{
	for (unsigned i = 0; i < dag->allNodes.size(); i++)
	{
		AssayNode *n = dag->allNodes.at(i);
		n->boundedResType = UNKNOWN_RES;
		n->startTimeStep = 0;
		n->endTimeStep = 0;
		n->SetStatus(UNBOUND_UNSCHED);
	}

	vector<AssayNode*> storage;
	for (int i = dag->allNodes.size()-1; i >= 0; i--)
	{
		if (dag->allNodes.at(i)->type == STORAGE)
		{
			storage.push_back(dag->allNodes.at(i));
			dag->allNodes.erase(dag->allNodes.begin()+i);
		}
	}
	dag->storage.clear();
	for (unsigned i = 0; i < dag->storageHolders.size(); i++)
	{
		AssayNode *n = dag->storageHolders.at(i);
		delete n;
	}
	dag->storageHolders.clear();

	for (int i = storage.size()-1; i >= 0; i--)
	{
		AssayNode *s = storage.at(i);
		AssayNode *p = s->parents.front();
		AssayNode *c = s->children.front();

		// Remove the storage from the parent
		for (unsigned k = 0; k <= p->children.size(); k++)
		{
			if (p->children.at(k) == s)
			{
				p->children.erase(p->children.begin()+k);
				break;
			}
		}
		p->children.push_back(c);

		// Remove the storage from the child
		for (unsigned k = 0; k <= c->parents.size(); k++)
		{
			if (c->parents.at(k) == s)
			{
				c->parents.erase(c->parents.begin()+k);
				break;
			}
		}
		c->parents.push_back(p);
		delete s;
	}
	storage.clear();
}



/////////////////////////////////////////////////////////////////////////////////////
// Generate a schedule based on a number of generations with reproductions, cross-
// overs and mutations.
/////////////////////////////////////////////////////////////////////////////////////
float dg_eval_total= 0.0;
float dg_in_total = 0.0;
float force_total = 0.0;
float differ2 = 0.0;
unsigned long long FDLScheduler::schedule(DmfbArch *arch, DAG *dag)
{
	/////////////////////////////////////////////////START of SCHEDULING//////////////////////////////////////////////

	maxv = 0;
	//setCritPathDist(dag, arch);
	reset_priority(dag);

	//assigns priorities for list_scheduler so that
	vector< pair<AssayNode*, float >  > asap_vals = ASAP(dag, arch);
	float max_v = 0.0;
	reset_priority(dag);
	Dag_reinitialize(dag);
	vector< pair<AssayNode*, float> > alap_vals = ALAP(dag, max_v, arch);



	unsigned count = 0;
	vector< pair< AssayNode*, unsigned > > priority_list;
	//while there are nodes left to schedule, calculate DG_graphs, and forces so you can choose best node to schedule at this timestep
	while(!asap_vals.empty())
	{
		float worst_force = 0.0;
		unsigned eraser = 0;
		vector<n_vals* >temp;

		// perform DG_input (feeds values so DG graphs can be calculated, and gets probability values for forces
		temp = DG_Inputs(asap_vals, alap_vals);



		// perform DG_evaluate (returns the node and its associated DG graph value)
		pair < vector<unsigned>, vector<float> > dg = DG_evaluate(temp);


		vector<n_forces*> tempor;

		//perform force calculation for every single node and every possible TS of that node
		tempor = force_calc (temp, dg); //calculate the forces

		for(unsigned i = 0; i< tempor.size(); i++) //find the worst force (highest concurrency amount)(node we want to schedule)
		{
			if(tempor.at(i)->force > worst_force)
			{
				worst_force = tempor.at(i)->force;
				eraser = i;
			}
		}
		pair<AssayNode*, unsigned> t2 = pair<AssayNode*, unsigned>(tempor.at(eraser)->n, count); //take node we want to schedule and give it a priority value
		priority_list.push_back(t2); //push it onto priority list so that we can set priorities for list scheduling
		for(unsigned i = 0; i < asap_vals.size(); i++) //if your asap val is the same as your tempor val
		{
			if(asap_vals.at(i).first->name == tempor.at(eraser)->n->name)
			{
				//erase it from your asap and alap vals, this will make it so that each iteration of while loop will re evaluate the DG graphs and the forces
				asap_vals.erase(asap_vals.begin()+i);
				alap_vals.erase(alap_vals.begin()+i);//from merger of two for loops
				//cout<<"i asap is:"<<i<<endl;
			}
		}
		count++;
		//delete temp and tempor (*n_vals, * forces


		while (!temp.empty())//delete memory allocate in DG graph calculations, DG_eval()
		{
			delete temp.back();
			temp.pop_back();
		}
		while(!tempor.empty()) //delete memeory allocated in force calculations, force_calc()
		{
			delete tempor.back();
			tempor.pop_back();
		}
	}
	for(unsigned i = 0; i<priority_list.size(); i++) //set priorities
	{
		priority_list.at(i).first->priority = priority_list.at(i).second;//new 4/12
	}


	ListScheduler ls;
	ls.setMaxStoragePerModule(getMaxStoragePerModule());
	ls.setPrioritiesExternally(); //let list scheduler know priorities already set
	Dag_reinitialize(dag);
	int TS_time = ls.schedule(arch, dag); //schedule
	cout<< "FDLS2 TIME: " << TS_time << "TS" << endl;

	return TS_time;
	//////////////////////////END of SCHEDULING///////////////////////////////////////////////
}

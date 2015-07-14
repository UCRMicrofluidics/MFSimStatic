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
 * Source: genet_scheduler.cc													*
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

#include "../../Headers/Scheduler/genet_scheduler.h"

///////////////////////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////////////////////
GenetScheduler::GenetScheduler()
{
	chromosome = NULL;
	srand ( (unsigned)time ( NULL ) );
}

///////////////////////////////////////////////////////////////////////////////////
// Deconstructor
///////////////////////////////////////////////////////////////////////////////////
GenetScheduler::~GenetScheduler() { }

/////////////////////////////////////////////////////////////////////////////////////
// Establishes node priorities as the random values generated in GA.
/////////////////////////////////////////////////////////////////////////////////////
void setAsRand(vector< map<AssayNode*, unsigned> *>::iterator it, vector< map<AssayNode*, unsigned> *> * pop)
{
	map<AssayNode*, unsigned> *chrom = (*it);
	map<AssayNode*, unsigned>::iterator it2 = chrom->begin();

	//for every node in gene, priority = priority of gene
	while (it2 != chrom->end())
	{
		AssayNode *n = it2->first;
		unsigned priority = it2->second;
		n->SetPriority(priority);
		it2++;
	}
}

/////////////////////////////////////////////////////////////////////////////////////
// Creates the node to random number asociation. Called rand key because the keys are randomly generated
// as specified by the genetic algorithm
/////////////////////////////////////////////////////////////////////////////////////
map<AssayNode*,  unsigned> * GenetScheduler::rand_key(DAG *dag)
{
	//generate vector full of nodes. Nodes to be paired with random numbers
	vector<AssayNode*> node_list;
	for( unsigned i = 0; i<dag->getAllNodes().size(); i++)
		node_list.push_back(dag->getAllNodes().at(i));

	//generate vector full of random numbers equal in size to vector full of nodes.
	vector<unsigned> rands;
	map<AssayNode*, unsigned>  *kn_assoc = new map<AssayNode*, unsigned>();

	for( unsigned i = 0; i<node_list.size(); i++)
		rands.push_back(rand() % 10000);

	//shuffles the random numbers, now that I have changed random number generator it is not
	//necessary but ensures good randomization
	random_shuffle ( rands.begin(), rands.end());

	//inserts random numbers with nodes as pairs into a map
	for (unsigned j = 0; j<node_list.size(); j++)
		kn_assoc->insert( pair<AssayNode*, unsigned>(node_list[j], rands[j]) );

	return kn_assoc;
}

/////////////////////////////////////////////////////////////////////////////////////
//initialize the population with A values. Currently set to 20
/////////////////////////////////////////////////////////////////////////////////////
vector< map<AssayNode*, unsigned> *> * GenetScheduler::initialize_population(DAG * dag, int A)
{
	//first, reinitialize dag
	Dag_reinitialize(dag);

	//pushes back the maps generated in rand_key onto a vector so that an initial population can be made.
	vector< map<AssayNode*, unsigned> *> * init_pop = new vector< map<AssayNode*, unsigned> *> ;
	for (int i = 0; i < A; i++)
		init_pop->push_back(rand_key(dag));

	return init_pop;
}

/////////////////////////////////////////////////////////////////////////////////////
// Calls mutation creating a small randomly generated population this is of the
// purpose to not converge on a single value too soon, with the idea that reintroducing
// randomply generated values will allow some backtracking which could possibly lead
// to a better overall path later on
/////////////////////////////////////////////////////////////////////////////////////
vector< map<AssayNode*, unsigned> *> * GenetScheduler::mutation(DAG * dag)
{
	unsigned size = get_mut_amnt();

	//get_mut_amt returns the percentage of mutations quickly
	if(size <1)
		size = 1;

	vector< map<AssayNode*, unsigned> *> *  mutate_pop = new vector< map<AssayNode*, unsigned> *> ;
	mutate_pop = initialize_population(dag, size);

	return mutate_pop;
}

/////////////////////////////////////////////////////////////////////////////////////
// Calls crossover creating a small crossedover population. The crossover is done
// by taking two random chromosomes of the population, crossing over their
// priority values (random keys) and creating a new chromosome from them called an
// offspring
/////////////////////////////////////////////////////////////////////////////////////
vector< map<AssayNode*, unsigned> *> * GenetScheduler::crossover(vector< map<AssayNode*, unsigned> *> * popA, DAG * dag)
{
	vector< map<AssayNode*, unsigned> *> * cross_pop = new vector< map<AssayNode*, unsigned> *> ;

	int cross_size = get_cross_amnt();

	//for the number of times specified in get_cross_amnt() which is a percentage of intitil pop size
	for (int number = 0; number < cross_size; number ++)
	{

		unsigned range = ( popA->size());
		vector< map<AssayNode*, unsigned> *>::iterator it = popA->begin();
		vector< map<AssayNode*, unsigned> *>::iterator it_Cross = popA->begin();

		//generate random numbers that are modded by the range(so not bigger than size of population)
		/* these random numbers are going to be used as indexes into the population so that I can choose
		 * two random chromosomes to crossover as specified in the paper
		 */
		unsigned rand1 = rand() % range;
		unsigned rand2 = rand() % range;

		//makes sure random numbers are not equal to each other
		while(rand1 == rand2 or rand2 >= range)
			rand2 = rand() % range ;

		while(rand1 == rand2 or rand1 >= range )
			rand1 = rand() % range;

		int count = 0;
		int count2 = 0;
		int count3 = 0;
		int count4 = 0;
		int count5 = 0;
		int count6 = 0;
		vector<unsigned> prior_1;
		vector<unsigned> prior_2;


		//getting map vals(rand_keys) at rand location rand1 from first copy of population set
		//storing in vector so that maps can be rewritten with crossover ratio .7 to .3
		while (it != popA->end() )
		{
			map<AssayNode*, unsigned> *chrom = (*it);
			map<AssayNode*, unsigned>::iterator it2 = chrom->begin();


			while (it2 != chrom->end())
			{
				//if count is iterated to the first random value, push back onto a temp vector the priority stored
				//at the iterator location in the initial population.
				if(count == rand1)
				{
					prior_1.push_back(it2->second);
					count3++;
				}
				else
					count2++;

				it2++;

			}
			if(count != rand1)
				it++;

			count++;

		}

		while (it_Cross != popA->end())
		{
			map<AssayNode*, unsigned> *chrom2 = (*it_Cross);
			map<AssayNode*, unsigned>::iterator it3 = chrom2->begin();

			while (it3 != chrom2->end())
			{
				//if count is iterated to the second random value, push back onto a temp vector the priority stored
				//at the second iterator location in the initial population.
				if(count4 == rand2)
				{
					prior_2.push_back(it3->second);
					count6++;
				}
				else
					count5++;
				it3++;
			}
			if(count4 != rand2)
				it_Cross++;

			count4++;

		}

		//rewriting vectors with crossed values.
		vector<unsigned> cross_p1 = prior_1;
		vector<unsigned> cross_p2 = prior_2;

		//crossing the values in the vectors so that the random key maps can generate a new random association
		//for a new crossedover chromosome
		for (unsigned i = 0; i< prior_1.size(); i++)
		{
			if ( ( ( i+1 ) % 4 ) == 0)
				cross_p1[i] = prior_2[i];
			else
				cross_p1[i] = prior_1[i];
		}

		//end crossing *note* I should probably remove the two maps being crossed after populating vectors, and then return a new
		//population with the new crossedover map pushed onto the back. Yah.. do that. Check... did that.
		//attempting to make new population containing the crossed value.

		//creates the map using the vector of crossed over priorities directly above
		map<AssayNode*, unsigned>  *map_add = new map<AssayNode*, unsigned>();
		for (unsigned j = 0; j<cross_p1.size(); j++)
			map_add->insert( pair<AssayNode*, unsigned>(dag->getAllNodes().at(j), cross_p1[j]) );

		//pushes the map onto a crossedover population
		cross_pop->push_back(map_add);

		//outputting new population
	}
	return cross_pop; //returns the crossed over population
}

/////////////////////////////////////////////////////////////////////////////////////
// Calls reproduction, creating a small reproduction population
/////////////////////////////////////////////////////////////////////////////////////
vector< map<AssayNode*, unsigned> *> * GenetScheduler::reproduction(vector< map<AssayNode*, unsigned> *> * popA, vector<unsigned> * TS )
{

	int foo = 0;
	foo = foo + 5;
	vector<int> * min_locs = new vector<int>();
	vector<int> * temp = new vector<int>();
	vector< map<AssayNode*, unsigned> *> * rep_pop = new vector< map<AssayNode*, unsigned> *>() ;

	//make temp a copy of the Timestep vector
	for(unsigned i = 0; i< TS->size(); i++)
		temp->push_back(TS->at(i));

	//rep size is the value associates with the top .125 percent of the initial populaiton
	//see get_rep_amnt();
	unsigned rep_size = get_rep_amnt();


	//for rep_size number of times
	//Sort::sortPopBySchedTimes(popA, TS);

	for( unsigned k = 0; k< rep_size; k++)
	{
		//int max_loc = 0;
		int max_val = 0;
		int min_loc = 0;
		int j = 0;
		vector<int>::iterator it = temp->begin();
		int min_val = *it;
		//for the size of temp (initial population)
		while(it != temp->end())
		{
			if(*it < min_val)
			{
				min_val = *it;
				min_loc = j;
			}
			j++;
			it++;
		}
		min_locs->push_back(min_loc);
		rep_pop->push_back(popA->at(min_loc));
		temp->at(min_loc) = max_val;
		//using the same value as before iterate through and find the minimum value(shortest time step)
		//push this shortest time step onto a vector, create a new population returning rep_size number of
		//chromosomes representing the most min_timesteps of the population

		//need to delete temp, min_locs here

	}


	delete temp;
	delete min_locs;
	return rep_pop;

}

/////////////////////////////////////////////////////////////////////////////////////
// Since GenetScheduler calls ListScheduler multiple times, it must reinitialize
// the DAG each time it generates a schedule.
/////////////////////////////////////////////////////////////////////////////////////
void GenetScheduler::Dag_reinitialize(DAG * dag)
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
unsigned long long GenetScheduler::schedule(DmfbArch *arch, DAG *dag)
{
	int num_generations = get_num_generations();
	int count = 0;

	vector< map<AssayNode*, unsigned> *> * init_pop = NULL;
	vector< map<AssayNode*, unsigned> *> * rep_pop = NULL;
	vector< map<AssayNode*, unsigned> *> * cross_pop = NULL;
	vector< map<AssayNode*, unsigned> *> * mut_pop = NULL;
	//vector< map<AssayNode*, unsigned> *> * new_pop = new vector< map<AssayNode*, unsigned> *>;
	vector<unsigned> *tempor = new vector<unsigned>;

	ListScheduler ls;
	ls.setMaxStoragePerModule(getMaxStoragePerModule());
	ls.setPrioritiesExternally(); // GenetScheduler will set the priorities itself

	int pop_size = 20;
	Dag_reinitialize(dag); //reinitialize the dag so that it does not contain storage nodes
	init_pop = initialize_population(dag, pop_size ); //create the initial population

	// Generations loop
	while(count < num_generations)
	{
		//turn into proper schedules:
		vector< map<AssayNode*, unsigned> *>::iterator iter = init_pop->begin();
		//int inner_count = init_pop->size();
		int T = 0;
		tempor->clear();

		//for each chromosome in the population
		while (iter != init_pop->end())
		{

			//reinitialize dag
			Dag_reinitialize(dag);
			map<AssayNode*, unsigned> *chrom = (*iter);
			int dag_count = 0;

			//for each node in the dag of each chromosome
			for( map<AssayNode*, unsigned>::iterator it2 = chrom->begin(); it2 != chrom->end(); it2++)
			{
				//the priority is equal to the value at the rand key
				//AssayNode *n = it2->first;
				unsigned priority = it2->second;
				dag->getAllNodes().at(dag_count)->SetPriority(priority); //changed to this from line below
				dag_count++;
			}

			//TS_time is equal to the scheduled value in list_sheduler::schedule
			//done using a "     " approach
			int TS_time = ls.schedule(arch, dag);
			tempor->push_back(TS_time);
			T++;
			iter++;
		}


		cross_pop = crossover(init_pop, dag);
		mut_pop = mutation(dag);
		rep_pop = reproduction(init_pop, tempor);
		//cout<<"init_pop size = "<<init_pop->size()<<endl;
		//deleting init pop entries not in rep_pop

		while(!init_pop->empty())
		{
			bool isMin = false;
			vector < map<AssayNode*, unsigned> * > ::reverse_iterator iter;
			for(iter = init_pop->rbegin(); iter != init_pop->rend(); iter++)
			{
				for(unsigned j = 0; j<rep_pop ->size(); j++)
				{
					if(rep_pop->at(j) == *iter)
					{
						//should delete value at popA->at(count);
						isMin = true;
						//it++;
						break;

						//popA->erase(it);
					}

				}
				if(!isMin)
				{
					delete init_pop->back();
				}
				init_pop->pop_back();
				//cout<<"count = "<<count<<flush<<endl;

			}
		}

		//delete init_pop ends here

		// init_pop clear just in case
		init_pop->clear();


		//inserts into new_pop all of the subset populations from reproduction, crossover and mutation
		init_pop->insert( init_pop->end(), rep_pop->begin(), rep_pop->end() );
		init_pop->insert( init_pop->end(), cross_pop->begin(), cross_pop->end() );
		init_pop->insert( init_pop->end(), mut_pop->begin(), mut_pop->end() );

		delete rep_pop;
		delete mut_pop;
		delete cross_pop;

		//init_pop = new_pop;
		count ++;
	}

	int less = tempor->at(0);
	int less_loc = 0;
	//finds the shortest schedule time and the location of that schedule in the population so that
	//it can be used later.
	for(unsigned val = 0; val< tempor->size(); val++)
	{
		if(tempor->at(val) < less)
		{
			less = tempor->at(val);
			less_loc = val;

		}
	}
	//outputs the information for visual confirmation


	//setting iterator to location of shortest schedule in population
	vector< map<AssayNode*, unsigned> *>::iterator iter = init_pop->begin()+less_loc;
	//reinitializing dag to ensure priorities reset and storage nodes removed
	Dag_reinitialize(dag);
	//setting second iterator to beggining of chromosome
	map<AssayNode*, unsigned> *chrom = (*iter);
	int dag_count = 0;

	//calling schedule on the shortest evaluated time in the final population

	////iterating throug chromsome to set priorities in dag so that I can call list_scheduler::schedule
	for( map<AssayNode*, unsigned>::iterator it2 = chrom->begin(); it2 != chrom->end(); it2++)
	{
		//the priority is equal to the value at the rand key
		//AssayNode *n = it2->first;
		unsigned priority = it2->second;
		dag->getAllNodes().at(dag_count)->SetPriority(priority); //changed to this from line below
		dag_count++;
	}

	//TS_time is equal to the scheduled value in list_sheduler::schedule
	//done using a "     " approach
	int TS_time = ls.schedule(arch, dag);
	cout<< "GA Time: " << TS_time << " TS" << endl;
	delete tempor;

	//delete init_pop
	init_pop->erase(init_pop->begin()); // Something wrong with this...is duplicate
	while(!init_pop->empty())
	{
		map<AssayNode*, unsigned> * del = init_pop->back();
		init_pop->pop_back();

		if (del)
			delete del;

		//delete init_pop->back();
		//init_pop->pop_back();
	}


	//end delete init_pop

	return (unsigned long long)TS_time;
}

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
 * Source: analyze.cc															*
 * Original Code Author(s): Dan Grissom											*
 * Original Completion/Release Date: October 17, 2013							*
 *																				*
 * Details: N/A																	*
 *																				*
 * Revision History:															*
 * WHO		WHEN		WHAT													*
 * ---		----		----													*
 * FML		MM/DD/YY	One-line description									*
 *-----------------------------------------------------------------------------*/
#include "../../Headers/Util/analyze.h"
#include "../../Headers/Util/sort.h"
#include <algorithm>

///////////////////////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////////////////////
Analyze::Analyze() {}

///////////////////////////////////////////////////////////////////////////////////
// Deconstructor
///////////////////////////////////////////////////////////////////////////////////
Analyze::~Analyze(){}



///////////////////////////////////////////////////////////////
// Does basic rule checking, trying to validate that the data
// input actually forms a DAG and checks for potential mistakes.
// Also, once it has validated the DAG, it does a breadth-first
// ordering of the DAG for later processing.
///////////////////////////////////////////////////////////////
string Analyze::AnalyzeSchedule(DAG *dag, DmfbArch *arch, Scheduler *scheduler)
{
	cout << "Analyzing schedule for errors...";

	string failTag = "***FAIL - ";
	stringstream rs(""); // return string


	rs << "//////////////////////////////////////////////////////" << endl;
	rs << "// NODE SUMMARY" << endl;
	rs << "//////////////////////////////////////////////////////" << endl;
	rs << "Number Total Nodes: " << dag->allNodes.size() << endl;
	rs << "Number Dispense Nodes: " << dag->heads.size() << endl;
	rs << "Number Output Nodes: " << dag->tails.size() << endl;
	rs << "Number Mix Nodes: " << dag->mixes.size() << endl;
	rs << "Number Dilute Nodes: " << dag->dilutes.size() << endl;
	rs << "Number Splits Nodes: " << dag->splits.size() << endl;
	rs << "Number Detects Nodes: " << dag->detects.size() << endl;
	rs << "Number Heats Nodes: " << dag->heats.size() << endl;
	rs << "Number Storage Nodes: " << dag->storage.size() << endl;
	rs << "Number Storage Holder Nodes: " << dag->storageHolders.size() << endl;
	rs << "Number Other Nodes: " << dag->others.size() << endl << endl;

	rs << "//////////////////////////////////////////////////////" << endl;
	rs << "// ERROR SUMMARY" << endl;
	rs << "//////////////////////////////////////////////////////" << endl;



	int numSplitChildren = 0;
	int numMixParents = 0;
	int maxTS = 0;

	// Make sure no heads have parents and one child
	for (unsigned i = 0; i < dag->heads.size(); i++)
	{
		AssayNode *h = dag->heads.at(i);

		if (h->GetParents().size() != 0)
		{
			stringstream ss;
			ss << failTag << "A head (DISPENSE/TRANSFER_IN) node has a parent.";
			rs << ss.str() << endl;
			cerr << endl << ss.str();
		}
		if (h->GetChildren().size() == 0)

		{
			stringstream ss;
			ss << failTag << "A head (DISPENSE/TRANSFER_IN) node has no children.";
			rs << ss.str() << endl;
			cerr << endl << ss.str();
		}
	}

	// Make sure no tails have children and one parent
	for (unsigned i = 0; i < dag->tails.size(); i++)
	{
		AssayNode *t = dag->tails.at(i);

		if (t->endTimeStep > maxTS)
			maxTS = t->endTimeStep;

		if (t->GetChildren().size() != 0)
		{
			stringstream ss;
			ss << failTag << "A tail (OUTPUT/TRANSFER_OUT) node has a child." ;
			rs << ss.str() << endl;
			cerr << endl << ss.str();
		}
		if (t->GetParents().size() == 0)
		{
			stringstream ss;
			ss << failTag << "A tail (OUTPUT/TRANSFER_OUT) node has no parents.";
			rs << ss.str() << endl;
			cerr << endl << ss.str();
		}

	}

	// Make sure mixes have 1 output
	for (unsigned i = 0; i < dag->mixes.size(); i++)
	{
		AssayNode *m = dag->mixes.at(i);
		numMixParents += m->parents.size();

		if (m->GetParents().size() != (unsigned)m->numDrops)
		{
			stringstream ss;
			ss << failTag << "A MIX node has " << m->GetParents().size() << " parents...should have " << m->numDrops << " parents.";
			rs << ss.str() << endl;
			cerr << endl << ss.str();
		}
		if (m->GetChildren().size() != 1)
		{
			stringstream ss;
			ss << failTag << "A MIX node has " << m->GetChildren().size() << " children...should have 1 child.";
			rs << ss.str() << endl;
			cerr << endl << ss.str();
		}
	}

	// Make sure dilutes have same number of inputs and outputs
	for (unsigned i = 0; i < dag->dilutes.size(); i++)
	{
		AssayNode *d = dag->dilutes.at(i);
		numMixParents += d->parents.size();
		numSplitChildren += d->children.size();

		if (d->GetParents().size() != (unsigned)d->numDrops)
		{
			stringstream ss;
			ss << failTag << "A DILUTE node has " << d->GetParents().size() << " parents...should have " << d->numDrops << " parents.";
			rs << ss.str() << endl;
			cerr << endl << ss.str();
		}
		if (d->GetChildren().size() != (unsigned)d->numDrops)
		{
			stringstream ss;
			ss << failTag << "A DILUTE node has " << d->GetChildren().size() << " children...should have " << d->numDrops << " children.";
			rs << ss.str() << endl;
			cerr << endl << ss.str();
		}
	}

	// Make sure splits have 1 input and 'numDropsAfterSplit' outputs
	for (unsigned i = 0; i < dag->splits.size(); i++)
	{
		AssayNode *s = dag->splits.at(i);
		numSplitChildren += s->numDrops;

		if (s->GetChildren().size() != (unsigned)s->numDrops)
		{
			stringstream ss;
			ss << failTag << "A SPLIT node has " << s->GetChildren().size() << " children...should have " << s->numDrops << " children.";
			rs << ss.str() << endl;
			cerr << endl << ss.str();
		}
		if (s->GetParents().size() != 1)
		{
			stringstream ss;
			ss << failTag << "A SPLIT node has " << s->GetParents().size() << " parents...should have 1 parent.";
			rs << ss.str() << endl;
			cerr << endl << ss.str();
		}
	}

	// Make sure all other nodes (GENERAL/FTSplit) have exactly 1 input and 1 output
	for (unsigned i = 0; i < dag->others.size(); i++)
	{
		AssayNode *o = dag->others.at(i);

		if (o->GetChildren().size() != 1)
		{
			stringstream ss;
			ss << failTag << "A General/FTSPlit node has " << o->GetChildren().size() << " children...should have 1 child.";
			rs << ss.str() << endl;
			cerr << endl << ss.str();
		}
		if (o->GetParents().size() != 1)
		{
			stringstream ss;
			ss << failTag << "A General/FTSplit node has " << o->GetParents().size() << " parents...should have 1 parent.";
			rs << ss.str() << endl;
			cerr << endl << ss.str();
		}
	}

	for (unsigned i = 0; i < dag->heats.size(); i++)
	{
		AssayNode *h = dag->heats.at(i);

		if (h->GetChildren().size() != 1)
		{
			stringstream ss;
			ss << failTag << "A heat node has " << h->GetChildren().size() << " children...should have 1 child.";
			rs << ss.str() << endl;
			cerr << endl << ss.str();
		}
		if (h->GetParents().size() != 1)
		{
			stringstream ss;
			ss << failTag << "A heat node has " << h->GetParents().size() << " parents...should have 1 parent.";
			rs << ss.str() << endl;
			cerr << endl << ss.str();
		}
	}

	for (unsigned i = 0; i < dag->detects.size(); i++)
	{
		AssayNode *d = dag->detects.at(i);

		if (d->GetChildren().size() != (unsigned)d->numDrops)
		{
			stringstream ss;
			ss << failTag << "A detect node has " << d->GetChildren().size() << " children...should have " << d->numDrops << " child(ren).";
			rs << ss.str() << endl;
			cerr << endl << ss.str();
		}
		if (d->GetParents().size() != (unsigned)d->numDrops)
		{
			stringstream ss;
			ss << failTag << "A detect node has " << d->GetParents().size() << " parents...should have " << d->numDrops << " parent(s).";
			rs << ss.str() << endl;
			cerr << endl << ss.str();
		}
	}

	// #Dispenses - #Mixes + #Splits - #Outputs should == 0
	if (dag->heads.size() - (numMixParents - dag->mixes.size()) + (numSplitChildren - dag->splits.size()) - dag->tails.size() != 0)
	{
		stringstream ss;
		ss << failTag << "#Dispenses - #Mixes + #Splits - #Outputs should = 0";
		rs << ss.str() << endl;
		cerr << endl << ss.str();
	}


	///////////////////////////////////////////////////////////////
	// Make sure we haven't allocated more resources than we have available at any time-step
	// Start by creating an empty list
	vector<ResHist *> resHist;
	for (int i = 0; i < maxTS; i++)
	{
		ResHist *rh = new ResHist();

		for (int j = 0; j <= RES_TYPE_MAX; j++)
		{
			rh->availRes[j] = arch->getPinMapper()->getAvailResCount()->at(j);
		}
		resHist.push_back(rh);
	}

	//for (int j = 0; j <= RES_TYPE_MAX; j++)
	//	cout << "Res " << j << " = " << arch->getPinMapper()->getAvailResCount()->at(j) << endl;

	// Update and check resource usage
	for (unsigned i = 0; i < dag->allNodes.size(); i++)
	{
		AssayNode *n = dag->allNodes.at(i);

		if (!(n->type == DISPENSE || n->type == OUTPUT))
		{
			if (n->type == STORAGE)
			{
				for (unsigned j = n->startTimeStep; j < n->endTimeStep; j++)
					resHist.at(j)->dropsInStorage++;
			}
			else if (n->boundedResType != UNKNOWN_RES)
			{
				for (unsigned j = n->startTimeStep; j < n->endTimeStep; j++)
				{
					resHist.at(j)->availRes[n->boundedResType]--;

					if (resHist.at(j)->availRes[n->boundedResType] < 0)
					{
						stringstream ss;
						ss << failTag << "A node (" << n->GetName() << ") is bound to a resource type that is not available;";
						ss << "Too many nodes bound to Resource Type " << n->boundedResType << " at TS " << j;
						rs << ss.str() << endl;
						cerr << endl << ss.str();
						//break;
					}
				}
			}
			else
			{
				stringstream ss;
				ss << failTag << "A node (" << n->GetName() << ") has no resource type associated with it.";
				rs << ss.str() << endl;
				cerr << endl << ss.str();
			}
		}
	}

	if (scheduler->getType() == FPPC_S || scheduler->getType() == FPPC_PATH_S)
	{
		for (int i = 0; i < maxTS; i++)
		{
			if (resHist.at(i)->availRes[SSD_RES] < resHist.at(i)->dropsInStorage)
			{
				stringstream ss;
				ss << failTag << "Attempting to store " <<  resHist.at(i)->dropsInStorage << " droplets in ";
				ss << resHist.at(i)->availRes[SSD_RES] << " SSD modules at TS " << i << ".";
				rs << ss.str() << endl;
				cerr << endl << ss.str();
			}
		}
	}

	// Clean up the ResHist structure
	while (!resHist.empty())
	{
		ResHist *rh = resHist.back();
		resHist.pop_back();
		delete rh;
	}


	// Re-init all node orders to 0
	for (unsigned i = 0; i < dag->allNodes.size(); i++)
		dag->allNodes.at(i)->order = 0;

	//////////////////////////////////////////////////////////////
	// Do topological sort to get ordering and determine if DAG
	// First create a mapping of the edges
	map<AssayNode*, vector<AssayNode*> *> edges;
	for (unsigned i = 0; i < dag->allNodes.size(); i++)
	{
		vector<AssayNode*> * childEdges = new vector<AssayNode*>();
		AssayNode *node = dag->allNodes.at(i);
		for (unsigned j = 0; j < node->children.size(); j++)
			childEdges->push_back(node->children.at(j));
		edges[node] = childEdges;
	}

	// Now perform algorithm as seen at
	// http://en.wikipedia.org/wiki/Topological_sorting for alg.
	vector<AssayNode*> L;
	vector<AssayNode*> S;
	for (unsigned i = 0; i < dag->heads.size(); i++)
		S.push_back(dag->heads.at(i));
	while (!S.empty())
	{
		AssayNode *n = S.back();
		S.pop_back();
		L.push_back(n);

		vector<AssayNode*> *ms = edges.find(n)->second;
		while (!ms->empty())
		{
			AssayNode *m = ms->back();
			ms->pop_back();

			bool mIsReady = true;
			map<AssayNode*, vector<AssayNode*> *>::iterator it = edges.begin();
			for (; it != edges.end(); it++)
			{
				vector<AssayNode*> *es = it->second;
				for (unsigned i = 0; i < es->size(); i++)
				{
					if (es->at(i) == m)
					{
						mIsReady = false;
						break;
					}
				}
			}
			if (mIsReady)
				S.push_back(m);
		}
	}
	for (unsigned i = 0; i < L.size(); i++)
		L.at(i)->order = i + 1;

	while(edges.empty())
	{
		delete edges.begin()->second;
		edges.erase(edges.begin());
	}

	//////////////////////////////////////////////////////////////
	// Now that we have an order, make sure monotonic (that is,
	// no children have a lower order than parents...should help
	// prevent cycles); also check that child nodes start at end
	// of parent nodes and that there are no gaps in the schedule.
	for (unsigned i = 0; i < dag->allNodes.size(); i++)
	{
		AssayNode *n = dag->allNodes.at(i);
		for (unsigned j = 0; j < n->children.size(); j++)
		{
			AssayNode *c = n->children.at(j);

			// Check order
			if ( !((c->order > n->order) && n->order != 0) )
			{
				stringstream ss;
				ss << failTag << "Cycle detected: Node " << n->order << " (" << n->name << ") points to higher/equal node, Node " << c->order << " (" << c->name << ")";
				rs << ss.str() << endl;
				cerr << endl << ss.str();
			}

			// Ensure that there is no gap in successive nodes
			if (n->endTimeStep != c->startTimeStep)
			{
				stringstream ss;
				ss << failTag << "Parent node (" << n->name << ") ends at TS " << n->endTimeStep << ", but has a child node (" << c->name << ") ";
				ss << "that begins at TS " << c->startTimeStep << ". Please ensure the schedule leaves no gaps in time.";
				rs << ss.str() << endl;
				cerr << endl << ss.str();
			}
		}
	}

	cout << "Done." << endl << endl;
	return rs.str();
}

///////////////////////////////////////////////////////////////
// Analyzes the placement for the following basic errors:
//  1.) No two modules can use the same cells at the same time.
//  2.) No two modules can use cells directly adjacent to one another at the same time (i.e., there must
//      be an interference region between modules)
//
//  *3.) In general, a module *should* not be placed in such a way that its cells or IR covers the cell
//       directly adjacent to an I/O port.  This is treated as a WARNING, as there are cases where this
//       could be okay (e.g. a module can block an output port when no droplet is being output for the entire
//       duration of the module's existence).
//
//	4.) Ensure that nodes and modules are properly bound:
//		  a.) All nodes are bound to a module/ioPort
//		  b.) All non-dispense modules are bound to a node
//		  c.) No two modules are bound to a single node.
///////////////////////////////////////////////////////////////
string Analyze::AnalyzePlacement(DmfbArch *arch, vector<ReconfigModule *> *rModules)
{
	cout << "Analyzing placement for errors...";

	string failTag = "***FAIL - ";
	string warnTag = "WARNING - ";
	stringstream rs(""); // return string

	// Create a map of the outputs
	map<string, bool> outputsAtCellMap;
	for (unsigned i = 0; i < arch->getIoPorts()->size(); i++)
		outputsAtCellMap[GetFormattedIOCell(arch->getIoPorts()->at(i), arch)] = true;

	// Create a 2D-array of Soukup cells
	vector<vector<ReconfigModule *> *> *board = new vector<vector<ReconfigModule *> *>();
	for (int x = 0; x < arch->getNumCellsX(); x++)
	{
		vector<ReconfigModule *> *col = new vector<ReconfigModule *>();
		for (int y = 0; y < arch->getNumCellsY(); y++)
			col->push_back(NULL);
		board->push_back(col);
	}

	// Sort modules by time
	Sort::sortReconfigModsByStartThenEndTS(rModules);

	// First, set all nodes bound to module as unbound to make sure only bound to a single module
	for (unsigned i = 0; i < rModules->size(); i++)
	{
		ReconfigModule *ra = rModules->at(i);
		AssayNode *boundNode = ra->getBoundNode();

		// 4b.) All modules must be bound to a node
		if (!boundNode)
		{
			stringstream ss("");
			ss << failTag << "The reconfigurable module (id " << ra->getId() << ") with bound node name \"" << ra->getBoundNode()->name << "\", ";
			ss << "occurring from TS " << ra->getStartTS() << " - " << ra->getEndTS();
			ss << " with TL, BR (x,y) coordinates of (" << ra->getLX() << "," << ra->getTY() << "), (" << ra->getRX() << "," << ra->getBY() << ")";
			ss << " does not have a assay node bound to it.  All modules should have a node bound to them.";
			cerr << endl << ss.str();
			rs << ss.str();
		}
		else if (boundNode->status != BOUND) // All nodes should be marked as BOUND
		{
			stringstream ss("");
			ss << failTag << "The reconfigurable module (id " << ra->getId() << ") with bound node name \"" << ra->getBoundNode()->name << "\", ";
			ss << "occurring from TS " << ra->getStartTS() << " - " << ra->getEndTS();
			ss << " with TL, BR (x,y) coordinates of (" << ra->getLX() << "," << ra->getTY() << "), (" << ra->getRX() << "," << ra->getBY() << ")";
			ss << " has a node that was not properly marked as bound.";
			cerr << endl << ss.str();
			rs << ss.str();
		}
		else if (boundNode->type == STORAGE_HOLDER && boundNode->storageOps.size() <= 0) // STORAGE_HOLDER nodes should have storage operations
		{
			stringstream ss("");
			ss << failTag << "The reconfigurable module (id " << ra->getId() << ") with bound node name \"" << ra->getBoundNode()->name << "\", ";
			ss << "occurring from TS " << ra->getStartTS() << " - " << ra->getEndTS();
			ss << " with TL, BR (x,y) coordinates of (" << ra->getLX() << "," << ra->getTY() << "), (" << ra->getRX() << "," << ra->getBY() << ")";
			ss << " is bound to a STORAGE_HOLDER node with no storage operations bound to it; this is probably an error.";
			cerr << endl << ss.str();
			rs << ss.str();
		}

		if (boundNode)
			boundNode->status = UNBOUND_UNSCHED; // Mark as unscheduled for now to ensure multiple modules are not bound to it

	}


	bool outputWarning = false;

	for (unsigned i = 0; i < rModules->size(); i++)
	{
		ReconfigModule *ra = rModules->at(i);

		// 4c.) No two modules are bound to a single node.
		if (ra->getBoundNode()->status == BOUND)
		{
			stringstream ss("");
			ss << failTag << "The reconfigurable module (id " << ra->getId() << ") ";
			ss << "occurring from TS " << ra->getStartTS() << " - " << ra->getEndTS();
			ss << " with TL, BR (x,y) coordinates of (" << ra->getLX() << "," << ra->getTY() << "), (" << ra->getRX() << "," << ra->getBY() << ")";
			ss << " is bound to the node with name \"" << ra->getBoundNode()->name << "\", ";
			ss << " however, this node as already been bound.";
			cerr << endl << ss.str();
			rs << ss.str();
		}

		bool foundIOBlockingModule = false;
		bool foundOverlappingModule = false;
		bool foundAdjacentModule = false;

		// Examine interference regions (for some checks) as well
		int modLeft = ra->getLX()-1 >= 0 ? ra->getLX()-1 : ra->getLX();
		int modRight = ra->getRX()+1 <= arch->getNumCellsX()-1 ? ra->getRX()+1 : ra->getRX();
		int modTop = ra->getTY()-1 >= 0 ? ra->getTY()-1 : ra->getTY();
		int modBottom = ra->getBY()+1 <= arch->getNumCellsY()-1 ? ra->getBY()+1 : ra->getBY();

		for (int y = modTop; y <= modBottom; y++)
		{
			for (int x = modLeft; x <= modRight; x++)
			{
				// If examining actual module (not just IR)
				if (x >= ra->getLX() && x <= ra->getRX() && y >= ra->getTY() && y <= ra->getBY())
				{
					// If there was something there in the past...
					if (board->at(x)->at(y))
					{
						ReconfigModule *raOld = board->at(x)->at(y);

						// ... 1.) Check if there is an overlapping module already placed at same time/place
						if (ra->getStartTS() < raOld->getEndTS() && !foundOverlappingModule)
						{
							foundOverlappingModule = true;
							stringstream ss;
							ss << failTag << "At location " << GetFormattedCell(x, y) << ",";
							ss << " the reconfigurable module (id " << ra->getId() << ") with name \"" << ra->getBoundNode()->name << "\", ";
							ss << "occurring from TS " << ra->getStartTS() << " - " << ra->getEndTS();
							ss << " with TL, BR (x,y) coordinates of (" << ra->getLX() << "," << ra->getTY() << "), (" << ra->getRX() << "," << ra->getBY() << ")";
							ss << " interferes with the module (id " << raOld->getId() << ") with name\"" << raOld->getBoundNode()->name << "\", ";
							ss << " occurring from TS "  << raOld->getStartTS() << " - " << raOld->getEndTS();
							ss << " with TL, BR (x,y) coordinates of (" << raOld->getLX() << "," << raOld->getTY() << "), (" << raOld->getRX() << "," << raOld->getBY() << ").";
							ss << " Please check your placer for placement errors/bugs.";
							cerr << endl << ss.str();
							rs << ss.str();
						}
					}
					board->at(x)->at(y) = ra;
				}
				else // If examining module's IR
				{
					// If there was something there in the past...
					if (board->at(x)->at(y))
					{
						ReconfigModule *raOld = board->at(x)->at(y);

						// ... 2.) Check if there is a module directly adjacent at same time (could cause interference during processing)
						if (ra->getStartTS() < raOld->getEndTS() && !foundAdjacentModule)
						{
							foundAdjacentModule = true;
							stringstream ss;
							ss << failTag << "At location " << GetFormattedCell(x, y) << ",";
							ss << " the reconfigurable module (id " << ra->getId() << ") with name \"" << ra->getBoundNode()->name << "\", ";
							ss << "occurring from TS " << ra->getStartTS() << " - " << ra->getEndTS();
							ss << " with TL, BR (x,y) coordinates of (" << ra->getLX() << "," << ra->getTY() << "), (" << ra->getRX() << "," << ra->getBY() << ")";
							ss << " is directly adjacent to the module (id " << raOld->getId() << ") with name \"" << raOld->getBoundNode()->name << "\", ";
							ss << " occurring from TS "  << raOld->getStartTS() << " - " << raOld->getEndTS();
							ss << " with TL, BR (x,y) coordinates of (" << raOld->getLX() << "," << raOld->getTY() << "), (" << raOld->getRX() << "," << raOld->getBY() << ").";
							ss << " Please check your placer for placement errors/bugs.";
							cerr << endl << ss.str();
							rs << ss.str();
						}
					}
				}

				// *3.) Next, perform warning check to see if the module's IR interferes with an I/O port (including IR)
				if (outputsAtCellMap.find(GetFormattedCell(x, y)) != outputsAtCellMap.end() && !foundIOBlockingModule)
				{
					foundIOBlockingModule = outputWarning = true;
					stringstream ss;
					ss << warnTag << "The reconfigurable module (id " << ra->getId() << ") with name \"" << ra->getBoundNode()->name << "\", ";
					ss << "occurring from TS " << ra->getStartTS() << " - " << ra->getEndTS();
					ss << " with TL, BR (x,y) coordinates of (" << ra->getLX() << "," << ra->getTY() << "), (" << ra->getRX() << "," << ra->getBY() << ")";
					ss << " has a cell or IR cell that interferes with the I/O port at " << GetFormattedCell(x, y) << ", which may cause an unresolvable routing blockage.";
					//cerr << endl << ss.str();
					rs << ss.str() << endl;
				}
			}
		}
	}

	// Cleanup
	while (!board->empty())
	{
		vector<ReconfigModule*> *v = board->back();
		board->pop_back();
		v->clear();
		delete v;
	}
	delete board;

	// Final output
	if (outputWarning)
		cout << endl << "Discovered several warnings; please check the output file for more details." << endl << "Done." << endl << endl;
	else if (rs.str().length() != 0)
		cout << endl << "Done." << endl << endl;
	else
		cout << "Done." << endl << endl;

	// Return results
	if (rs.str().length() == 0)
		return "No failures/warnings discovered.";
	else
		return rs.str();
}

///////////////////////////////////////////////////////////////
// Performs sanity checks and analysis on routes to make sure
// that the router followed all the conventions for a valid
// visualization and output. Checks that:
//  1.) All droplets marked as output are next to an output port
//  2.) All consecutive routing points are no more than 1
//      orthogonal cell away (no diagonals allowed for now)
//  3.) There are no gaps in the route (missing cycles)
//  4.) No two droplets interfere except when merging/splitting
///////////////////////////////////////////////////////////////
string Analyze::AnalyzeRoutes(DmfbArch *arch, map<Droplet *, vector<RoutePoint *> *> *routes)
{
	cout << "Analyzing routes for errors...";

	string failTag = "***FAIL - ";
	stringstream rs(""); // return string

	// Create a map of the outputs
	map<string, bool> outputsAtCellMap;
	for (unsigned i = 0; i < arch->getIoPorts()->size(); i++)
		if (!arch->getIoPorts()->at(i)->isInput)
			outputsAtCellMap[GetFormattedIOCell(arch->getIoPorts()->at(i), arch)] = true;

	// Check droplet termination statuses and get simulation length
	unsigned lastCycle = 0;
	unsigned firstCycle = 0;
	map<Droplet *, vector<RoutePoint *> *>::iterator routesIt = routes->begin();
	for (; routesIt != routes->end(); routesIt++)
	{

		RoutePoint *lastRp = routesIt->second->back();

		// 1.) Check that all droplets marked as output are next to an output port...
		if (lastRp->dStatus == DROP_OUTPUT)
		{
			if (outputsAtCellMap.find(GetFormattedCell(lastRp)) == outputsAtCellMap.end())
			{
				stringstream ss("");
				ss << failTag << "Droplet " << routesIt->first->getId() << " said to output at cell ";
				ss << GetFormattedCell(lastRp) << ", but there is no adjacent output port.";
				rs << ss.str();
				cerr << endl << ss.str();
			}
		}
		else if (lastRp->dStatus != DROP_MERGING) // ...and that rest are marked as merging
		{
			stringstream ss("");
			ss << failTag << "Droplet " << routesIt->first->getId() << " @ Cycle " << lastRp->cycle;
			ss << " @ cell " << GetFormattedCell(lastRp);
			ss << " is not properly terminated with a 'DROP_OUTPUT' or 'DROP_MERGING' status. ";
			ss << "Please ensure the last RoutePoint is properly marked.";
			rs << ss.str();
			cerr << endl << ss.str();
		}

		// Get total cycles
		if (lastRp->cycle > lastCycle)
			lastCycle = lastRp->cycle;

		// Get starting cycle
		RoutePoint *firstRp = routesIt->second->front();
		if (firstRp->cycle < firstCycle || firstCycle == 0)
			firstCycle = firstRp->cycle;

	}

	// Create a vector (of length = to number of cycles in simulation) for each droplet
	//int numDrops = routes->size();
	map<unsigned, vector<pair<RoutePoint *, int> > *> dropletLocAtCycleMap;
	//for (int i = 0; i < numDrops; i++)
		//globallyIndexedRoutes.push_back(new vector<RoutePoint *>(lastCycle+1));

	// Now, populate the globallyIndexdRoutes vector with routing data
	vector<Droplet *> droplets;
	int globalIndex = 0;
	for (routesIt = routes->begin(); routesIt != routes->end(); routesIt++)
	{
		Droplet *d = routesIt->first;
		vector<RoutePoint *> *r = routesIt->second;
		droplets.push_back(d);

		for (unsigned i = 0; i < r->size(); i++)
		{
			RoutePoint * rp = r->at(i);
			if (i > 0)
			{
				RoutePoint * rpLast = r->at(i-1);

				// Check for null routing points (partially condition #3)
				if (!rp)
				{
					stringstream ss("");
					ss << failTag << "Found a NULL routing point for Droplet " << d->getId();
					if (rpLast)
						ss << ". Previous routing point location: " << GetFormattedCell(rpLast) << " at cycle " << rpLast->cycle;
					ss << ". Please ensure there are no NULL routing points in the final route.";
					rs << ss.str();
					cerr << endl << ss.str();
				}
				else if (rpLast)
				{
					// 2.) Check that there are no gaps in the route (missing cycles)
					if (!ValidRoutingMove(rpLast, rp))
					{
						stringstream ss("");
						ss << failTag << "Droplet " << d->getId();
						ss << " does not make a valid move from cycle " << rpLast->cycle << "-" << rp->cycle;
						ss << ", from " << GetFormattedCell(rpLast) << "-->" << GetFormattedCell(rp);
						ss << ". Please ensure that droplets move one orthogonal cell, at most, each cycle.";
						rs << ss.str();
						cerr << endl << ss.str();
					}

					// 3.) There are no missing cycles in consecutive routing points
					if (rp->cycle != rpLast->cycle+1)
					{
						stringstream ss("");
						ss << failTag << "Droplet " << d->getId();
						ss << " has consecutive routing points with non-consecutive cycle numbers: " << rpLast->cycle << " to " << rp->cycle;
						ss << ", when moving from " << GetFormattedCell(rpLast) << "-->" << GetFormattedCell(rp);
						ss << ". Please ensure that consecutive routing points increment by one cycle.";
						rs << ss.str();
						cerr << endl << ss.str();
					}
				}
			}
			// Add each routing point to the globallyIndexedRoutes vector
			if (rp)
			{
				if (dropletLocAtCycleMap.find(rp->cycle) == dropletLocAtCycleMap.end())
					dropletLocAtCycleMap[rp->cycle] = new vector<pair<RoutePoint*, int> >();
				dropletLocAtCycleMap[rp->cycle]->push_back(make_pair(rp, d->getId()));
			}
		}
		globalIndex++;
	}


	// Now, check for droplet interference amongst all droplets
	for (unsigned i = firstCycle; i <= lastCycle; i++)
	{
		//cout << i << endl;
		vector<pair<RoutePoint *, int> > *lastCycleRps = dropletLocAtCycleMap.find(i-1) == dropletLocAtCycleMap.end() ? NULL : dropletLocAtCycleMap[i-1];
		vector<pair<RoutePoint *, int> > *thisCycleRps = dropletLocAtCycleMap.find(i) == dropletLocAtCycleMap.end() ? NULL : dropletLocAtCycleMap[i];
		vector<pair<RoutePoint *, int> > *nextCycleRps = dropletLocAtCycleMap.find(i+1) == dropletLocAtCycleMap.end() ? NULL : dropletLocAtCycleMap[i+1];

		// Check each RoutePoint this cycle with every other this cycle, last cycle and next cycle
		for (unsigned j = 0; j < thisCycleRps->size(); j++)
		{
			pair<RoutePoint *, int> thisRp = thisCycleRps->at(j);

			// Check against last cycle for all other routes
			if (lastCycleRps)
			{
				for (unsigned l = 0; l < lastCycleRps->size(); l++)
				{
					pair<RoutePoint *, int> lastRp = lastCycleRps->at(l);
					if (thisRp.second != lastRp.second) // Don't compare same droplet's own history
					{
						if (DoesInterfere(thisRp.first, lastRp.first))
						{
							stringstream ss;
							ss << failTag << "Droplet " << thisRp.second << " at cycle " << thisRp.first->cycle << " at " << GetFormattedCell(thisRp.first);
							ss << " interferes with droplet " << lastRp.second << " at cycle " << lastRp.first->cycle << " at " << GetFormattedCell(lastRp.first);
							ss << ". Please make sure routing and compaction are performed properly.";
							rs << ss.str();
							cerr << endl << ss.str();
						}
					}
				}
			}

			// Check against this cycle for all other routes
			if (thisCycleRps->size() > 1)
			{
				for (unsigned t = 0; t < thisCycleRps->size(); t++)
				{
					pair<RoutePoint *, int> otherRp = thisCycleRps->at(t);
					if (thisRp.second != otherRp.second) // Don't compare same droplet's own history
					{
						if (DoesInterfere(thisRp.first, otherRp.first))
						{
							stringstream ss;
							ss << failTag << "Droplet " << thisRp.second << " at cycle " << thisRp.first->cycle << " at " << GetFormattedCell(thisRp.first);
							ss << " interferes with droplet " << otherRp.second << " at cycle " << otherRp.first->cycle << " at " << GetFormattedCell(otherRp.first);
							ss << ". Please make sure routing and compaction are performed properly.";
							rs << ss.str();
							cerr << endl << ss.str();
						}
					}
				}
			}

			// Check against next cycle for all other routes
			if (nextCycleRps)
			{
				for (unsigned n = 0; n < nextCycleRps->size(); n++)
				{
					pair<RoutePoint *, int> nextRp = nextCycleRps->at(n);
					if (thisRp.second != nextRp.second) // Don't compare same droplet's own history
					{
						if (DoesInterfere(thisRp.first, nextRp.first))
						{
							stringstream ss;
							ss << failTag << "Droplet " << thisRp.second << " at cycle " << thisRp.first->cycle << " at " << GetFormattedCell(thisRp.first);
							ss << " interferes with droplet " << nextRp.second << " at cycle " << nextRp.first->cycle << " at " << GetFormattedCell(nextRp.first);
							ss << ". Please make sure routing and compaction are performed properly.";
							rs << ss.str();
							cerr << endl << ss.str();
						}
					}
				}
			}
		}
	}

	// Cleanup
	while (!dropletLocAtCycleMap.empty())
	{
		vector<pair<RoutePoint *, int> > *v = dropletLocAtCycleMap.begin()->second;
		dropletLocAtCycleMap.erase(dropletLocAtCycleMap.begin());
		v->clear();
		delete v;
	}


	if (rs.str().length() == 0)
	{
		cout << "Done." << endl << endl;
		return "No failures discovered.";
	}
	else
	{
		cout << endl << "Done." << endl << endl;
		return rs.str();
	}
}

///////////////////////////////////////////////////////////////
// Gathers the statistics of the droplets being input/output of
// the system such as the amount of fluids input and the
// concentrations of the droplets being output from the system.
// Outputs the errors to cerr and returns a string to be written
// to file.
///////////////////////////////////////////////////////////////
string Analyze::AnalyzeDropletConcentrationAndIO(DAG *dag, DmfbArch *arch, map<Droplet *, vector<RoutePoint *> *> *routes)
{
	cout << "Analyzing droplet I/O and concentrations for errors...";

	stringstream rs;

	// Analyze and report concerning input droplets
	rs << "//////////////////////////////////////////////////////" << endl;
	rs << "// INPUT DROPLETS" << endl;
	rs << "//////////////////////////////////////////////////////" << endl;
	map<string, float> inFluids;
	int dropsIn = 0;
	int volIn = 0.0;
	for (unsigned i = 0; i < dag->getAllInputNodes().size(); i++)
	{
		// Print out the details of the input droplet
		AssayNode *inputNode = dag->getAllInputNodes().at(i);
		string name = Util::StringToLower(inputNode->GetPortName());
		rs << inputNode->volume << " Unit droplet input @ TS " << inputNode->startTimeStep << " @ cell ";
		rs << GetFormattedIOCell(inputNode->ioPort, arch) << endl;
		rs << "\t100%\t" << name << endl;

		// Now save for equilibrium checking (did we output everything we input?
		if (inFluids.find(name) == inFluids.end())
			inFluids[name] = inputNode->volume;
		else
			inFluids[name] = inFluids[name] + inputNode->volume;

		dropsIn++;
		volIn += inputNode->volume;
	}
	rs << endl << "SUMMARY: " << dropsIn << " total droplets (totaling " << volIn << " units) were input." << endl << endl;

	// Analyze and report concerning output droplets
	rs << "//////////////////////////////////////////////////////" << endl;
	rs << "// OUTPUT DROPLETS" << endl;
	rs << "// (Absolute Parts / Normalized Parts / Percentage)" << endl;
	rs << "//////////////////////////////////////////////////////" << endl;
	map<string, float> outFluids;
	int dropsOut = 0;
	int volOut = 0.0;
	stringstream ssErrorDrops(""); // Details for droplets not properly output
	map<Droplet *, vector<RoutePoint *> *>::iterator routeIt = routes->begin();
	for (; routeIt != routes->end(); routeIt++)
	{
		Droplet *d = routeIt->first;
		vector<RoutePoint*> * r = routeIt->second;
		RoutePoint *lastRp = r->back();
		stringstream ssDrop("");

		// Determine if the droplet was terminated properly (output or merged)
		bool properlyTerminated = false;
		if (lastRp->dStatus == DROP_OUTPUT || lastRp->dStatus == DROP_MERGING)
			properlyTerminated = true;

		// Examine the droplets routes and obtain composition details and last position/time
		ssDrop << d->getVolume() << " Unit droplet (#" << d->getId() << ") @ Cycle " << lastRp->cycle << " @ cell (" << lastRp->x << ", " << lastRp->y << "):" << endl;//-- " << d->getComposition() << endl;

		map<string, float> fluids;
		string delimiter = "||";
		string tokenDel = " parts ";
		size_t pos = -1;
		string token;
		string newName = d->getComposition();
		float totalVolume = 0;
		float minVolume = d->getVolume();

		// Grab each fluid part
		while ((pos = newName.find(delimiter)) != string::npos)
		{
			token = newName.substr(0, pos);
			size_t pos2 = token.find(tokenDel); // Will always have
			float vol = (float)atof(token.substr(0, pos2).c_str());
			totalVolume += vol;
			string name = Util::StringToLower(token.substr(pos2 + tokenDel.length(), token.length()-1));

			// Add to local fluids list
			if (fluids.find(name) == fluids.end())
				fluids[name] = vol;
			else
				fluids[name] = fluids[name] + vol;

			// Add to global fluids output list
			if (lastRp->dStatus == DROP_OUTPUT)
			{
				if (outFluids.find(name) == outFluids.end())
					outFluids[name] = vol;
				else
					outFluids[name] = outFluids[name] + vol;
			}
			if (vol < minVolume)
				minVolume = vol;
			newName.erase(0, pos + delimiter.length());
		}
		// Grab last fluid part
		token = newName;
		pos = token.find(tokenDel); // Will always have
		float vol = (float)atof(token.substr(0, pos).c_str());
		totalVolume += vol;
		string name = Util::StringToLower(token.substr(pos + tokenDel.length(), token.length()-1));

		// Add to local fluids list
		if (fluids.find(name) == fluids.end())
			fluids[name] = vol;
		else
			fluids[name] = fluids[name] + vol;

		// Add to global fluids output list
		if (lastRp->dStatus == DROP_OUTPUT)
		{
			if (outFluids.find(name) == outFluids.end())
				outFluids[name] = vol;
			else
				outFluids[name] = outFluids[name] + vol;
		}

		if (vol < minVolume)
			minVolume = vol;

		// Print the composition from the individual fluid parts for output droplets
		stringstream ss;
		ss.precision(2);
		map<string, float>::iterator fluidIt = fluids.begin();
		float totalPercentage = 0.0;
		float totalParts = 0.0;
		float totalNormalizedParts = 0.0;
		for (; fluidIt != fluids.end(); fluidIt++)
		{
			totalParts += fluidIt->second;
			totalNormalizedParts += fluidIt->second/minVolume;
			totalPercentage += fluidIt->second/d->getVolume()*100;
			ss << "\t" << fixed << fluidIt->second << "p / " << fluidIt->second/minVolume << "np / " << fluidIt->second/d->getVolume()*100 << "%   " << fluidIt->first << endl;
		}
		ss << "SUM\t" << fixed << totalParts << "p / " << totalNormalizedParts << "np / " << totalPercentage << "%" << endl;
		ssDrop << ss.str();

		// Output if an output droplet; send to error output if improperly terminated
		if (lastRp->dStatus == DROP_OUTPUT)
		{
			rs << ssDrop.str() << endl;
			dropsOut++;
			volOut += totalParts;
		}
		else if (!properlyTerminated)
			ssErrorDrops << ssDrop.str() << endl;
	}
	rs << endl << "SUMMARY: " << dropsOut << " total droplets (totaling " << volOut << " units) were output." << endl << endl;

	// Print erroneous droplets, if necessary
	if (ssErrorDrops.str().length() > 0)
	{
		stringstream ssErr("");
		ssErr << "//////////////////////////////////////////////////////" << endl;
		ssErr << "// ERRONEOUS DROPLETS DISCOVERED" << endl;
		ssErr << "// These droplets end at the shown cycle, but are not marked as Output or Merged." << endl;
		ssErr << "//////////////////////////////////////////////////////" << endl;
		ssErr << ssErrorDrops.str();
		rs << ssErr.str();
		//cerr << ssErr.str() << endl; // Will let the route analyzer do the cerr for this info
	}


	// Report concerning input/output comparisons
	rs << "//////////////////////////////////////////////////////" << endl;
	rs << "// DROPLET I/O SANITY CHECK" << endl;
	rs << "//////////////////////////////////////////////////////" << endl;

	// Check the input fluids against the output fluids
	map<string, float>::iterator fluidIt = inFluids.begin();
	for (; fluidIt != inFluids.end(); fluidIt++)
	{
		string name = fluidIt->first;
		float inVol = fluidIt->second;

		if (outFluids.find(name) == outFluids.end())
		{
			stringstream ss("");
			ss <<"***FAIL - " << inVol << " units of " << name << " input but never output." << endl;
			cerr << ss.str() << endl;
			rs << ss.str() << endl;
		}
		else
		{
			float outVol = outFluids.find(name)->second;

			// Floats can be a bit off b/c of the computation, so do it based on percentage; if
			// < .01% difference, we say it is the same volume
			if ((max(inVol, outVol) - min(inVol, outVol))/inVol < .0001 )//inVol == outVol)
				rs << "PASS - " << inVol << " units of " << name << " properly input and output." << endl;
			else
			{
				stringstream ss("");
				ss <<"***FAIL - " << inVol << " units of " << name << " input but " << outVol << " units output." << endl;
				cerr << ss.str() << endl;
				rs << ss.str() << endl;
			}
			outFluids.erase(outFluids.find(name));
		}
	}

	// Now, for sanity, check that there are no fluids that were output without being input
	fluidIt = outFluids.begin();
	for (; fluidIt != outFluids.end(); fluidIt++)
	{
		string name = fluidIt->first;
		float outVol = fluidIt->second;

		stringstream ss("");
		ss <<"***FAIL - " << outVol << " units of " << name << " output but never input." << endl;
		cerr << ss.str() << endl;
		rs << ss.str() << endl;
	}

	cout << "Done." << endl;

	return rs.str();
}

///////////////////////////////////////////////////////////////
// Checks if two routing points constitute a valid move; that
// that is, that they are exactly 1 or less cells away in any
// direction cumulatively (at this point diagonal movements
// are not allowed)
///////////////////////////////////////////////////////////////
bool Analyze::ValidRoutingMove(RoutePoint *rpStart, RoutePoint *rpEnd)
{
	//if (!rpStart || !rpEnd) // If either is null, then assume it is okay
	//	return true;
	if (abs(rpEnd->x - rpStart->x) + abs(rpEnd->y - rpStart->y) <= 1)
		return true;
	else
		return false;
}


///////////////////////////////////////////////////////////////
// Checks if two routing points interfere with each other.
// Considers locations and statuses.
///////////////////////////////////////////////////////////////
bool Analyze::DoesInterfere(RoutePoint *rp1, RoutePoint *rp2)
{
	if (!rp1 || !rp2) // If either is null, then assume error already reported and don't report as interference error
		return false;
	else if (abs(rp2->x - rp1->x) > 1 || abs(rp2->y - rp1->y) > 1) // 2+ cells away in any direction
		return false;
	else if (rp1->dStatus == DROP_MERGING || rp2->dStatus == DROP_MERGING || rp1->dStatus == DROP_SPLITTING || rp2->dStatus == DROP_SPLITTING) // In interference region but marked as splitting/merging
		return false; // This is a bit conservative and could theoretically be cheated, but should be fine
	else // In interference region w/ no excuses
		return true;
}

///////////////////////////////////////////////////////////////
// Prints the XY coordinate of the adjacent cell in the form "(x, y)"
///////////////////////////////////////////////////////////////
string Analyze::GetFormattedIOCell(IoPort *port, DmfbArch *arch)
{
	stringstream ss("");
	if (port->getSide() == NORTH)
		ss << "(" << port->posXY << ", 0)";
	else if (port->getSide() == SOUTH)
		ss << "(" << port->posXY << ", " << arch->getNumCellsY()-1 << ")";
	else if (port->getSide() == WEST)
		ss << "(0, " << port->posXY << ")";
	else if (port->getSide() == EAST)
		ss << "(" << arch->getNumCellsX()-1 << ", " << port->posXY << ")";

	return ss.str();
}

///////////////////////////////////////////////////////////////
// Prints the XY coordinate of the cell in the form "(x, y)"
///////////////////////////////////////////////////////////////
string Analyze::GetFormattedCell(RoutePoint *rp)
{
	stringstream ss("");
	if (rp)
		ss << "(" << rp->x << ", " << rp->y << ")";
	else
		ss << "(NULL)";
	return ss.str();
}

///////////////////////////////////////////////////////////////
// Prints the XY coordinate of the cell in the form "(x, y)"
///////////////////////////////////////////////////////////////
string Analyze::GetFormattedCell(int x, int y)
{
	stringstream ss("");
	ss << "(" << x << ", " << y << ")";
	return ss.str();
}

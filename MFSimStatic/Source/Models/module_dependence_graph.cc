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
 * Source: module_dependence_graph.cc											*
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

#include "../../Headers/Models/module_dependence_graph.h"

ModuleDependenceGraph::ModuleDependenceGraph()
//: vLoc(virtloc)
{


}

ModuleDependenceGraph::~ModuleDependenceGraph()
{
	/*for(unsigned i = 0; i < vertices.size(); i++) {
		ModuleDependenceVertex *v = vertices[i];
		v->preds.clear();
		v->succs.clear();
	}*/
	while(vertices.size() > 0) {
		ModuleDependenceVertex *v = vertices[vertices.size()-1];
		vertices.pop_back();
		delete v;
	}

	while (connectedComponents.size() > 0)
	{
		vector<ModuleDependenceVertex *> *v = connectedComponents.back();
		connectedComponents.pop_back();
		v->clear();
		delete v;
	}
	while (ccHeads.size() > 0)
	{
		vector<ModuleDependenceVertex *> *v = ccHeads.back();
		ccHeads.pop_back();
		v->clear();
		delete v;
	}
	while (ccTails.size() > 0)
	{
		vector<ModuleDependenceVertex *> *v = ccTails.back();
		ccTails.pop_back();
		v->clear();
		delete v;
	}
	while (stronglyConnComps.size() > 0)
	{
		vector<ModuleDependenceVertex *> *v = stronglyConnComps.back();
		stronglyConnComps.pop_back();
		v->clear();
		delete v;
	}
}

void ModuleDependenceGraph::RecursiveComponentLabel(ModuleDependenceVertex *v, int componentNum)
{
	if (v->utilNum < 0)
	{
		v->utilNum = componentNum;

		// Add vertex to appropriate lists
		connectedComponents.at(componentNum)->push_back(v);
		if (v->preds.size() == 0)
			ccHeads.at(componentNum)->push_back(v);
		if (v->succs.size() == 0)
			ccTails.at(componentNum)->push_back(v);

		// Recursively look through all parents/children
		for (unsigned p = 0; p < v->preds.size(); p++)
			RecursiveComponentLabel(v->preds.at(p), componentNum);
		for (unsigned s = 0; s < v->succs.size(); s++)
			RecursiveComponentLabel(v->succs.at(s), componentNum);
	}
	else
		return;
}

void ModuleDependenceGraph::FindAllConnectedComponents()
{
	// Set/Reset all component numbers
	int label = -1;
	for (unsigned i = 0; i < vertices.size(); i++)
		vertices.at(i)->utilNum = label;

	for (unsigned i = 0; i < vertices.size(); i++)
	{
		if (vertices.at(i)->utilNum < 0)
		{
			label++;
			connectedComponents.push_back(new vector<ModuleDependenceVertex *>());
			ccHeads.push_back(new vector<ModuleDependenceVertex *>());
			ccTails.push_back(new vector<ModuleDependenceVertex *>());
			RecursiveComponentLabel(vertices.at(i), label);
		}
	}

}

/////////////////////////////////////////////////////////////
// Sorts the connected components via a topological
// sort, which gives a valid order to route droplets.  Given
// the nature of the problem, droplets must be routed in
// reverse topological order.
/////////////////////////////////////////////////////////////
void ModuleDependenceGraph::RevTopSortSCCs()
{
	for (unsigned c = 0; c < connectedComponents.size(); c++)
	{
		vector<ModuleDependenceVertex *> *cc = connectedComponents.at(c);
		vector<ModuleDependenceVertex *> *cch = ccHeads.at(c);

		// Re-init all node orders to 0
		for (unsigned i = 0; i < cc->size(); i++)
			cc->at(i)->utilNum = 0;

		//////////////////////////////////////////////////////////////
		// Do topological sort to get ordering and determine if DAG
		// First create a mapping of the edges
		map<ModuleDependenceVertex*, vector<ModuleDependenceVertex*> *> edges;
		for (unsigned i = 0; i < cc->size(); i++)
		{
			vector<ModuleDependenceVertex*> * childEdges = new vector<ModuleDependenceVertex*>();
			ModuleDependenceVertex *node = cc->at(i);
			for (unsigned j = 0; j < node->succs.size(); j++)
				childEdges->push_back(node->succs.at(j));
			edges[node] = childEdges;
		}

		// Now perform algorithm as seen at
		// http://en.wikipedia.org/wiki/Topological_sorting for alg.
		vector<ModuleDependenceVertex*> L;
		vector<ModuleDependenceVertex*> S;
		for (unsigned i = 0; i < cch->size(); i++)
			S.push_back(cch->at(i));
		while (!S.empty())
		{
			ModuleDependenceVertex *n = S.back();
			S.pop_back();
			L.push_back(n);

			vector<ModuleDependenceVertex*> *ms = edges.find(n)->second;
			while (!ms->empty())
			{
				ModuleDependenceVertex *m = ms->back();
				ms->pop_back();

				bool mIsReady = true;
				map<ModuleDependenceVertex*, vector<ModuleDependenceVertex*> *>::iterator it = edges.begin();
				for (; it != edges.end(); it++)
				{
					vector<ModuleDependenceVertex*> *es = it->second;
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
			L.at(i)->utilNum = i + 1;

		while(edges.empty())
		{
			delete edges.begin()->second;
			edges.erase(edges.begin());
		}

		//////////////////////////////////////////////////////////////
		// Now that we have an order, make sure monotonic (that is,
		// no children have a lower order than parents...should help
		// prevent cycles)
		for (unsigned i = 0; i < cc->size(); i++)
		{
			ModuleDependenceVertex *n = cc->at(i);
			for (unsigned j = 0; j < n->succs.size(); j++)
			{
				ModuleDependenceVertex *ch = n->succs.at(j);
				{
					stringstream msg;
					msg << "ERROR. Cycle detected: Node " << n->utilNum << " (" << n->operation->GetName() << ") points to higher/equal node, Node " << ch->utilNum << " (" << ch->operation->GetName() << ")" << endl;
					if (!((ch->utilNum > n->utilNum) && n->utilNum != 0))
					{
						//PrintConnectedComponents();
						//PrintDependencies();
						claim((ch->utilNum > n->utilNum) && n->utilNum != 0, &msg);
					}
				}

			}
		}

		// Must route in reverse topological order so
		// put ordered nodes back in connected components list
		cc->clear();
		while (!L.empty())
		{
			cc->push_back(L.back());
			L.pop_back();
		}
	}
}

/////////////////////////////////////////////////////////////
// Finds the strongly connected components (i.e. cycles) of
// the graph.  Does so by using algorith detailed at:
// http://en.wikipedia.org/wiki/Path-based_strong_component_algorithm
/////////////////////////////////////////////////////////////
void ModuleDependenceGraph::RecursiveSccSearch(ModuleDependenceVertex *v, stack<ModuleDependenceVertex *> *S, stack<ModuleDependenceVertex *> *P)
{
	// (1) Set the preorder number of v to C, and increment C.
	v->utilNum = order++;

    // (2) Push v onto S and also onto P.
	S->push(v);
	P->push(v);

    // (3) For each edge from v to a neighboring vertex w:
    //       If the preorder number of w has not yet been assigned, recursively search w;
    //       Otherwise, if w has not yet been assigned to a strongly connected component:
    //         Repeatedly pop vertices from P until the top element of P has a preorder number less than or equal to the preorder number of w.
    for (unsigned i = 0; i < v->succs.size(); i++)
    {
    	ModuleDependenceVertex *w = v->succs.at(i);
    	if (w->utilNum < 0)
    		RecursiveSccSearch(w, S, P);
    	else if (!w->addedToAnSCC)
    		while (P->size() > 0 && P->top()->utilNum > w->utilNum)
    			P->pop();
    }

	// (4) If v is the top element of P:
    //       Pop vertices from S until v has been popped, and assign the popped vertices to a new component.
    //       Pop v from P.
    if (!P->empty() && v == P->top())
    {
    	vector<ModuleDependenceVertex *> *ssc = new vector<ModuleDependenceVertex *>();
    	while (S->top() != v)
    	{
    		ssc->push_back(S->top());
    		S->top()->addedToAnSCC = true;
    		S->pop();
    	}
    	ssc->push_back(S->top());
    	S->top()->addedToAnSCC = true;
    	S->pop();
    	P->pop();
    	stronglyConnComps.push_back(ssc);
    }

}

/////////////////////////////////////////////////////////////////////////////////
// Searches the connected components for strongly connected components (SSRs).
// A SSR with more than one node is a cycle.  Resolve by adding "routing buffer"
// nodes.
/////////////////////////////////////////////////////////////////////////////////
void ModuleDependenceGraph::FindAndResolveDependencies()
{
	// Variables for RecursiveSccSearch
	order = 0;
	stack<ModuleDependenceVertex *> S;
	stack<ModuleDependenceVertex *> P;

	for (unsigned i = 0; i < connectedComponents.size(); i++)
	{
		vector<ModuleDependenceVertex *> *cc = connectedComponents.at(i);
		// Reset nodes as not-visited
		for (unsigned j = 0; j < cc->size(); j++)
		{
			ModuleDependenceVertex *v = cc->at(j);
			v->alreadyChecked = false;
			v->addedToAnSCC = false; // Used to tell if assigned to a cycle already
			v->isRoutingBufferNode = false; // Used to tell if vertex is representing a transient routing buffer node
			v->utilNum = -1; // preorder number for this search
			v->reRouteVertex = NULL;
		}

		for (unsigned j = 0; j < cc->size(); j++)
		{
			ModuleDependenceVertex *v = cc->at(j);
			if (v->utilNum < 0)
				RecursiveSccSearch(v, &S, &P);
		}
	}

	// Remove SSCs with only 1 node b/c they are not cycles, as we are interested in
	for (int i = stronglyConnComps.size() - 1; i >= 0; i--)
	{
		if (stronglyConnComps.at(i)->size() <= 1)
		{
			vector<ModuleDependenceVertex *> * vec = stronglyConnComps.at(i);
			stronglyConnComps.erase(stronglyConnComps.begin()+i);
			vec->clear();
			delete vec;
		}
	}

	//////////////////////////////////////////////////////////////////////
	// Convert SCC into a CC (remove cycle) for routing buffer
	if (stronglyConnComps.size() > 0)
	{
		//cout << stronglyConnComps.size() << "cycle(s) found..." << endl;
		for (unsigned i = 0; i < stronglyConnComps.size(); i++)
		{
			// Print cycles for debuging purposes
			/*cout << "C " << i << ": ";
			for (unsigned j  = 0; j < stronglyConnComps.at(i)->size(); j++)
			{
				PrintVertex(stronglyConnComps.at(i)->at(j));
				cout << "-->";
			}
			cout << endl;*/

			//////////////////////////////////////////////////////////////////////
			// Insert routing buffer into graph (re-route first component/droplet)
			ModuleDependenceVertex *rr = stronglyConnComps.at(i)->front();
			ModuleDependenceVertex *b = stronglyConnComps.at(i)->back();

			// Set re-route node in all nodes (b/c we don't know which one will be done first)
			for (unsigned j = 0; j < stronglyConnComps.at(i)->size(); j++)
				stronglyConnComps.at(i)->at(j)->reRouteVertex = rr;

			// Remove links between front and back of SCC to eliminate cycle
			for (int j = rr->succs.size()-1; j >= 0; j--)
				if (rr->succs.at(j) == b)
					rr->succs.erase(rr->succs.begin()+j);
			for (int j = b->preds.size()-1; j >= 0; j--)
				if (b->preds.at(j) == rr)
					b->preds.erase(b->preds.begin()+j);

			// Now, add new routing vertex and link to back of cycle
			ModuleDependenceVertex *rv = new ModuleDependenceVertex();
			rv->isRoutingBufferNode = true;
			rv->operation = NULL;
			rv->reRouteVertex = rr;
			rv->succs.push_back(b); // Successor is back of cycle
			b->preds.push_back(rv);
			vertices.push_back(rv);
		}
	}
}

void ModuleDependenceGraph::PrintDependencies()
{
	cout << "Found " << vertices.size() << " unique locations/vertices." << endl;
	for (unsigned i = 0; i < vertices.size(); i++)
	{
		ModuleDependenceVertex *f = vertices.at(i);
		if (f->isRoutingBufferNode)
			cout << "RouteBuffer-->" << endl;
		else if (f->operation->GetType() == DISPENSE || f->operation->GetType() == OUTPUT)
		{
			cout << f->operation->GetName() << ": P[" << f->operation->GetPortName();
			if (f->operation->GetIoPort()->getSide() == NORTH)
				cout << ", NORTH";
			else if (f->operation->GetIoPort()->getSide() == SOUTH)
				cout << ", SOUTH";
			else if (f->operation->GetIoPort()->getSide() == EAST)
				cout << ", EAST";
			else if (f->operation->GetIoPort()->getSide() == WEST)
				cout << ", WEST";
			cout << ", " << f->operation->GetIoPort()->getPosXY() << "]-->" << endl;
		}
		else if (f->operation->GetType() == STORAGE)
			cout << "STOR_" << f->operation->getId() << ": M[" << f->operation->GetReconfigMod()->getLX() << ", " << f->operation->GetReconfigMod()->getTY() << "]-->" << endl;
		else
			cout << f->operation->GetName() << ": M[" << f->operation->GetReconfigMod()->getLX() << ", " << f->operation->GetReconfigMod()->getTY() << "]-->" << endl;


		if (f->succs.size() == 0)
			cout << "\t**NONE**" << endl;
		for (unsigned s = 0; s < f->succs.size(); s++)
		{
			ModuleDependenceVertex *t = f->succs.at(s);

			if (t->operation->GetType() == OUTPUT)
			{
				cout << "\t" << t->operation->GetName() << ": P[" << t->operation->GetPortName();
				if (t->operation->GetIoPort()->getSide() == NORTH)
					cout << ", NORTH";
				else if (t->operation->GetIoPort()->getSide() == SOUTH)
					cout << ", SOUTH";
				else if (t->operation->GetIoPort()->getSide() == EAST)
					cout << ", EAST";
				else if (t->operation->GetIoPort()->getSide() == WEST)
					cout << ", WEST";
				cout << ", " << t->operation->GetIoPort()->getPosXY() << "]" << endl;
			}
			else if (t->operation->GetType() == STORAGE)
				cout << "\t" << "STOR_" << t->operation->getId() << ": M[" << t->operation->GetReconfigMod()->getLX() << ", " << t->operation->GetReconfigMod()->getTY() << "]" << endl;
			else if (t->operation->GetType() == SPLIT || t->operation->GetType() == DILUTE)
			{
				cout << "\t" << t->operation->GetName() << ": M[" << t->operation->GetReconfigMod()->getLX() << ", " << t->operation->GetReconfigMod()->getTY() << "]" << endl;
				cout << "\t\t" << "STOR_" << t->operation->GetChildren().at(0)->getId() << ": M[" << t->operation->GetChildren().at(0)->GetReconfigMod()->getLX() << ", " << t->operation->GetChildren().at(0)->GetReconfigMod()->getTY() << "]" << endl;
				cout << "\t\t" << "STOR_" << t->operation->GetChildren().at(1)->getId() << ": M[" << t->operation->GetChildren().at(1)->GetReconfigMod()->getLX() << ", " << t->operation->GetChildren().at(1)->GetReconfigMod()->getTY() << "]" << endl;
			}
			else
				cout << "\t" << t->operation->GetName() << ": M[" << t->operation->GetReconfigMod()->getLX() << ", " << t->operation->GetReconfigMod()->getTY() << "]" << endl;
		}
	}
	cout << endl;
}

void ModuleDependenceGraph::PrintVertex(ModuleDependenceVertex *v)
{
	if (v->operation->GetType() == DISPENSE || v->operation->GetType() == OUTPUT)
	{
		cout << v->operation->GetName() << ": P[" << v->operation->GetPortName();
		if (v->operation->GetIoPort()->getSide() == NORTH)
			cout << ", NORTH";
		else if (v->operation->GetIoPort()->getSide() == SOUTH)
			cout << ", SOUTH";
		else if (v->operation->GetIoPort()->getSide() == EAST)
			cout << ", EAST";
		else if (v->operation->GetIoPort()->getSide() == WEST)
			cout << ", WEST";
		cout << ", " << v->operation->GetIoPort()->getPosXY() << "]";
	}
	else if (v->operation->GetType() == STORAGE)
		cout << "STOR_" << v->operation->getId() << ": M[" << v->operation->GetReconfigMod()->getLX() << ", " << v->operation->GetReconfigMod()->getTY() << "]";
	else
		cout << v->operation->GetName() << ": M[" << v->operation->GetReconfigMod()->getLX() << ", " << v->operation->GetReconfigMod()->getTY() << "]";
}

void ModuleDependenceGraph::PrintConnectedComponents()
{
	if (connectedComponents.size() == 0)
		return;

	cout << "Printing Connected Components (unordered)..." << endl;
	for (unsigned i = 0; i < connectedComponents.size(); i++)
	{
		cout << "CC" << i << ": ";
		for (unsigned j = 0; j < connectedComponents.at(i)->size(); j++)
		{
			PrintVertex(connectedComponents.at(i)->at(j));
			cout << "---";
		}
		cout << endl;
		cout << "\tCC_Heads" << i << ": ";
		for (unsigned j = 0; j < ccHeads.at(i)->size(); j++)
		{
			PrintVertex(ccHeads.at(i)->at(j));
			cout << "---";
		}
		cout << endl;
		cout << "\tCC_Tails" << i << ": ";
		for (unsigned j = 0; j < ccTails.at(i)->size(); j++)
		{
			PrintVertex(ccTails.at(i)->at(j));
			cout << "---";
		}
		cout << endl;
	}
}

/////////////////////////////////////////////////////////////
// Given two nodes, tells whether they match locations or
// not by examining the modules they occupy.  If looking at
// a SPLIT node, must examine its immediate storage successors
// too!
/////////////////////////////////////////////////////////////
bool ModuleDependenceGraph::ModuleLocationsMatch(AssayNode *n1, AssayNode *n2)
{
	// Dispenses and outputs will not depend on each other
	if (n1->GetType() == DISPENSE || n1->GetType() == OUTPUT || n2->GetType() == DISPENSE || n2->GetType() == OUTPUT)
		return false;

	ReconfigModule *rm1 = n1->GetReconfigMod();
	ReconfigModule *rm2 = n2->GetReconfigMod();

	// If the two direct modules match
	if (rm1->getTY() == rm2->getTY() && rm1->getLX() == rm2->getLX())
		return true;

	// If n1's split children match with n2's location
	if (n1->GetType() == SPLIT || n1->GetType() == DILUTE)
	{
		ReconfigModule *srm0 = n1->GetChildren().at(0)->GetReconfigMod(); // Storage-module child
		ReconfigModule *srm1 = n1->GetChildren().at(1)->GetReconfigMod();

		if (rm2->getTY() == srm0->getTY() && rm2->getLX() == srm0->getLX())
			return true;
		if (rm2->getTY() == srm1->getTY() && rm2->getLX() == srm1->getLX())
			return true;
	}

	// If n2's split children match with n1's location
	if (n2->GetType() == SPLIT || n2->GetType() == DILUTE)
	{
		ReconfigModule *srm0 = n2->GetChildren().at(0)->GetReconfigMod(); // Storage-module child
		ReconfigModule *srm1 = n2->GetChildren().at(1)->GetReconfigMod();

		if (rm1->getTY() == srm0->getTY() && rm1->getLX() == srm0->getLX())
			return true;
		if (rm1->getTY() == srm1->getTY() && rm1->getLX() == srm1->getLX())
			return true;
	}

	// If n1's split children match with n2's split children
	if ((n1->GetType() == SPLIT || n1->GetType() == DILUTE) && (n2->GetType() == SPLIT || n2->GetType() == DILUTE))
	{
		ReconfigModule *srm0 = n1->GetChildren().at(0)->GetReconfigMod(); // Storage-module child
		ReconfigModule *srm1 = n1->GetChildren().at(1)->GetReconfigMod();
		ReconfigModule *srm2 = n2->GetChildren().at(0)->GetReconfigMod();
		ReconfigModule *srm3 = n2->GetChildren().at(1)->GetReconfigMod();

		if (srm0->getTY() == srm2->getTY() && srm0->getLX() == srm2->getLX())
			return true;
		if (srm0->getTY() == srm3->getTY() && srm0->getLX() == srm3->getLX())
			return true;
		if (srm1->getTY() == srm2->getTY() && srm1->getLX() == srm2->getLX())
			return true;
		if (srm1->getTY() == srm3->getTY() && srm1->getLX() == srm3->getLX())
			return true;
	}
	return false;
}


/////////////////////////////////////////////////////////////
// Adds dependency to DDG
/////////////////////////////////////////////////////////////
void ModuleDependenceGraph::AddDependency(AssayNode *from, AssayNode *to)
{
	// If either are NULL, or we have a storage with a split parent - no dependency (these storages are lumped in with the split
	if (from == NULL || to == NULL || (to->GetType() == STORAGE && (to->GetParents().front()->GetType() == SPLIT || to->GetParents().front()->GetType() == DILUTE)))
	{
		//cerr << "NULL PROBLEM WITH AddDependency\n";
		//exit(0);
		return;//no dependency
	}

	ModuleDependenceVertex *fromVertex = NULL;
	ModuleDependenceVertex *toVertex = NULL;

	//Search for vertices
	bool foundFrom = false;
	bool foundTo = false;

	for(unsigned i = 0; i < vertices.size(); i++)
	{
		ModuleDependenceVertex *v = vertices[i];
		//ReconfigModule *frm = from->GetReconfigMod();
		//ReconfigModule *trm = to->GetReconfigMod();
		//ReconfigModule *vrm = v->operation->GetReconfigMod();


		if (ModuleLocationsMatch(v->operation, from))
		{
			fromVertex = v;
			foundFrom = true;

			// If the found vertex represents a previously-routed AssayNode, replace
			// with new AssayNode b/c we always want the AssayNode that is being routed this TS
			if (v->operation->GetStartTS() < from->GetStartTS())
				v->operation = from;
		}
		else if (ModuleLocationsMatch(v->operation, to))
		{
			toVertex = v;
			foundTo = true;

			// If the found vertex represents a previously-routed AssayNode, replace
			// with new AssayNode b/c we always want the AssayNode that is being routed this TS
			if (v->operation->GetStartTS() < to->GetStartTS())
				v->operation = to;
		}
		if (foundFrom && foundTo)
			break;
	}
	if (!foundFrom)
	{
		fromVertex = new ModuleDependenceVertex();
		fromVertex->operation = from;
		//fromVertex->preds = NULL;
		fromVertex->alreadyChecked = false;
		vertices.push_back(fromVertex);
	}
	if (!foundTo)
	{
		toVertex = new ModuleDependenceVertex();
		toVertex->operation = to;
		//toVertex->succs = NULL;
		toVertex->alreadyChecked = false;
		vertices.push_back(toVertex);
	}

	fromVertex->succs.push_back(toVertex);
	toVertex->preds.push_back(fromVertex);

}


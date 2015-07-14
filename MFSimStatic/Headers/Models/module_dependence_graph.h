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
/*--------------------------------Class Details---------------------------------*
 * Name: ModuleDependenceGraph													*
 *																				*
 * Details: A graph structure used to determine if there are dependencies		*
 * between modules during routing.												*
 *-----------------------------------------------------------------------------*/

#ifndef _MODULE_DEPENDENCE_GRAPH_H
#define _MODULE_DEPENDENCE_GRAPH_H

#include "../Testing/claim.h"

// Classes defined in the file


// STL-based locally defined data types
using namespace std;
using namespace __gnu_cxx;

#include <sstream>
#include <stack>
#include "../Resources/structs.h"
#include "assay_node.h"

struct ModuleDependenceVertex;
class AssayNode;

class ModuleDependenceGraph
{
	private:
		vector<ModuleDependenceVertex *> vertices;
		vector<vector<ModuleDependenceVertex *> *> stronglyConnComps;
		vector<vector<ModuleDependenceVertex *> *> connectedComponents;
		vector<vector<ModuleDependenceVertex *> *> ccHeads;
		vector<vector<ModuleDependenceVertex *> *> ccTails;
		int order;

		bool ModuleLocationsMatch(AssayNode *n1, AssayNode *n2);
		void RecursiveComponentLabel(ModuleDependenceVertex *v, int componentNum);
		void RecursiveSccSearch(ModuleDependenceVertex *v, stack<ModuleDependenceVertex *> *S, stack<ModuleDependenceVertex *> *P);

	public:
		ModuleDependenceGraph();
		virtual ~ModuleDependenceGraph();
		void AddDependency(AssayNode *from, AssayNode *to);
		void PrintDependencies();
		void PrintConnectedComponents();
		void PrintVertex(ModuleDependenceVertex *v);
		void FindAllConnectedComponents();
		void FindAndResolveDependencies();
		void RevTopSortSCCs();

		vector<vector<ModuleDependenceVertex *> *> getConnectedComponents() { return connectedComponents; }
};

#endif

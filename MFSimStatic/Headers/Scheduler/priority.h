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
 * Name: Priority 																*
 *																				*
 * Details: Contains several different methods for setting the priorities of a 	*
 * DAG.																			*
 *-----------------------------------------------------------------------------*/
#ifndef _PRIORITY_H
#define _PRIORITY_H

#include "../Models/dmfb_arch.h"
#include <functional>
#include "../Models/dag.h"

struct CompareNode : public std::binary_function<AssayNode*, AssayNode*, bool>
{
    bool operator()(AssayNode* lhs, AssayNode* rhs) const
    {
        return lhs->GetPriority() < rhs->GetPriority();
    }
};

class Priority{
    protected:
		// Methods
		static void recursiveCPD(DmfbArch *arch, AssayNode *node, unsigned childDist);
		static void recursiveLPD(AssayNode *node, unsigned childDist);
		static void recursiveNIP(AssayNode *node);
		static void resetPriorities(DAG *dag);
    public:
		// Constructors
		Priority();
        ~Priority ();

        // Methods
		static void debugPrintPriorities(DAG *dag);
        static void setAsCritPathDist(DAG *dag, DmfbArch *arch);
        static void setAsLongestPathDist(DAG *dag);
        static void setAsNumIndPaths(DAG *dag);
        //static void setAsNumIndThenCritPath(DAG *dag, VirtualLoC *vl);
};
#endif

// For debugging priorities
/*for (int i = 0; i < 10; i++)
{
	int num = rand() % dag->allNodes.size();
	cout << "pushing " << dag->allNodes.at(num)->GetName() << " with priority " << dag->allNodes.at(num)->GetPriority() << endl;
	pq.push(dag->allNodes.at(num));
}
cout << "PQ ------------------ PQ" << endl;
while (!pq.empty())
{
	cout << "at top: " << pq.top()->GetName() << " with priority " << pq.top()->GetPriority() << endl;
	pq.pop();
}*/

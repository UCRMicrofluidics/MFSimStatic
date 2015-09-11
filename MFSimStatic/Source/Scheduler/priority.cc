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
 * Source: priority.cc															*
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
#include "../../Headers/Scheduler/priority.h"

///////////////////////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////////////////////
Priority::Priority(){}

///////////////////////////////////////////////////////////////////////////////////
// Deconstructor
///////////////////////////////////////////////////////////////////////////////////
Priority::~Priority() {}

///////////////////////////////////////////////////////////////////
// Reset priorities; set duration of dispenses to vl dispense times
///////////////////////////////////////////////////////////////////
void Priority::resetPriorities(DAG *dag)
{
	for (int i = 0; i < dag->allNodes.size(); i++)
		dag->allNodes.at(i)->priority = 0;
}

///////////////////////////////////////////////////////////////////
// Reset priorities; set duration of dispenses to vl dispense times
///////////////////////////////////////////////////////////////////
void Priority::debugPrintPriorities(DAG *dag)
{
	for (unsigned i = 0; i < dag->allNodes.size(); i++)
		cout << dag->allNodes.at(i)->name << " has priority of " << dag->allNodes.at(i)->priority << endl;
}

/*void Priority::setAsNumIndThenCritPath(DAG *dag, VirtualLoC *vl)
{
	unsigned long long p1Mult = 1000000; // Multiplier for priority 1
	setAsNumIndPaths(dag);
	vector<unsigned long long> p1Values;

	for (int i = 0; i < dag->getAllNodes().size(); i++)
		p1Values.push_back(dag->getAllNodes().at(i)->priority * p1Mult);

	setAsCritPathDist(dag, vl);

	for (int i = 0; i < dag->getAllNodes().size(); i++)
	{
		//cout << dag->getAllNodes().at(i)->name << " " << dag->getAllNodes().at(i)->priority;
		dag->getAllNodes().at(i)->priority += p1Values.at(i);
		//cout << " --> " << dag->getAllNodes().at(i)->priority << endl;
	}



	//dag->getAllNodes()
	//for (int i = 0; dag->getAllNodes()->)
}*/

///////////////////////////////////////////////////////////////////
// Sets each node's priority to the number of cycles of the combined
// duration of all it's successor nodes in the following path.  If
// there is a branch below, the node will have the value of the longest
// path.
///////////////////////////////////////////////////////////////////
void Priority::setAsCritPathDist(DAG *dag, DmfbArch *arch)
{
	resetPriorities(dag);
	for (unsigned i = 0; i < dag->tails.size(); i++)
		recursiveCPD(arch, dag->tails.at(i), 0);
}
void Priority::recursiveCPD(DmfbArch *arch, AssayNode *node, unsigned childDist)
{
	if (node->GetType() == DISPENSE)
	{
		IoPort *iop = arch->getIoPort(node->portName);
		stringstream ss;
		ss << "No input available in given architecture with fluid-type of " << node->portName << endl;
		claim (iop, &ss);
		node->cycles = arch->getIoPort(node->portName)->getTimeInSec() * arch->getFreqInHz();
	}

	node->priority = max(node->priority, childDist + node->cycles);
	for (unsigned i = 0; i < node->parents.size(); i++)
		recursiveCPD(arch, node->parents.at(i), node->priority);
}

///////////////////////////////////////////////////////////////////
// Sets each node's priority to the max number of nodes it must go
// through to reach the output.
///////////////////////////////////////////////////////////////////
void Priority::setAsLongestPathDist(DAG *dag)
{
	resetPriorities(dag);
	for (unsigned i = 0; i < dag->tails.size(); i++)
		recursiveLPD(dag->tails.at(i), 0);
}
void Priority::recursiveLPD(AssayNode *node, unsigned childDist)
{
	node->priority = max(node->priority, childDist + 1);
	for (unsigned i = 0; i < node->parents.size(); i++)
		recursiveLPD(node->parents.at(i), node->priority);
}

///////////////////////////////////////////////////////////////////
// Sets each node's priority to the number of independent paths
// that lead to outputs
//
// ****TODO: For now, does not take into account paths that split and
// then reconverge...will deal with this later
///////////////////////////////////////////////////////////////////
void Priority::setAsNumIndPaths(DAG *dag)
{
	resetPriorities(dag);
	for (unsigned i = 0; i < dag->tails.size(); i++)
		recursiveNIP(dag->tails.at(i));
}
void Priority::recursiveNIP(AssayNode *node)
{
	if (node->type == OUTPUT)
		node->priority = 1;
	else
	{
		node->priority = 0;
		for (unsigned i = 0; i < node->children.size(); i++)
			node->priority += node->children.at(i)->priority;
	}
	for (unsigned i = 0; i < node->parents.size(); i++)
		recursiveNIP(node->parents.at(i));
}

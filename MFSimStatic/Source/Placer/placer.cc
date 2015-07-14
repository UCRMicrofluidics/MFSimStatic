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
 * Source: placer.cc															*
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
#include "../../Headers/Placer/placer.h"
#include "../../Headers/Util/sort.h"
#include <math.h>

///////////////////////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////////////////////
Placer::Placer()
{
	dispRes = new vector<IoResource*>();
	outRes = new vector<IoResource*>();
	hasExecutableSyntesisMethod = true;
}

///////////////////////////////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////////////////////////////
Placer::~Placer()
{
	for (int i = 0; i <= RES_TYPE_MAX; i++)
		availRes[i].clear(); // Don't delete them here, will be deleted in pin-mapper
	while (!dispRes->empty())
	{
		IoResource *dr = dispRes->back();
		dispRes->pop_back();
		delete dr;
	}
	while (!outRes->empty())
	{
		IoResource *oR = outRes->back();
		outRes->pop_back();
		delete oR;
	}
	delete dispRes;
	delete outRes;
}

///////////////////////////////////////////////////////////////////////////////////
// Generic function called to do placement.
///////////////////////////////////////////////////////////////////////////////////
void Placer::place(DmfbArch *arch, DAG *dag, vector<ReconfigModule *> *rModules)
{
	claim(false, "No valid placer was selected for the synthesis process or no method for 'place()' was implemented for the selected placer.\n");
}

///////////////////////////////////////////////////////////////////////////////////
// Resets the resource reservoirs (for binding/placement), according to the
// architecture, so they can be used for placement.
///////////////////////////////////////////////////////////////////////////////////
void Placer::resetIoResources(DmfbArch *arch)
{
	// Now reset dispense/output resources
	while (!dispRes->empty())
	{
		IoResource *r = dispRes->back();
		dispRes->pop_back();
		delete r;
	}
	while (!outRes->empty())
	{
		IoResource *r = outRes->back();
		outRes->pop_back();
		delete r;
	}
	for (unsigned i = 0; i < arch->getIoPorts()->size(); i++)
	{
		IoPort *iop = arch->getIoPorts()->at(i);
		IoResource *ir = new IoResource();
		ir->port = iop;
		ir->name = iop->getPortName();
		ir->lastEndTS = 0;
		ir->durationInTS = ceil((double)iop->getTimeInSec()/((double)arch->getSecPerTS()));

		if (iop->isAnInput())
			dispRes->push_back(ir);
		else
			outRes->push_back(ir);
	}
}

///////////////////////////////////////////////////////////////////////////////////
// Copies resources for local use that were determined by the resource allocator.
///////////////////////////////////////////////////////////////////////////////////
void Placer::getAvailResources(DmfbArch *arch)
{
	// Copy the module resources into the local array (Temp, DTG)
	for (int i = 0; i <= RES_TYPE_MAX; i++)
		for (unsigned j = 0; j < arch->getPinMapper()->getAvailRes()->at(i)->size(); j++)
			availRes[i].push_back(arch->getPinMapper()->getAvailRes()->at(i)->at(j));
}

///////////////////////////////////////////////////////////////////////////////////
// Binds the given input nodes to input reservoirs using a simple left-edge binding
// technique.
///////////////////////////////////////////////////////////////////////////////////
void Placer::bindInputsLE(list<AssayNode *> *inputNodes)
{
	Sort::sortNodesByStartTS(inputNodes);
	for (unsigned i = 0; i < dispRes->size(); i++)
	{
		unsigned long long lastEnd = 0;
		list<AssayNode *> scheduled;
		IoResource *dr = dispRes->at(i);
		list<AssayNode *>::iterator it = inputNodes->begin();
		for (; it != inputNodes->end(); it++)
		{
			AssayNode *n = *it;
			if (n->GetPortName() == dr->name && n->GetStartTS() >= lastEnd)
			{
				dr->schedule.push_back(n);
				scheduled.push_back(n);
				lastEnd = n->GetEndTS();
				n->status = BOUND;
				n->ioPort = dr->port;
			}
		}
		while (!scheduled.empty())
		{
			inputNodes->remove(scheduled.front());
			scheduled.pop_front();
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////
// Binds the given output nodes to output reservoirs using a simple left-edge
// binding technique.
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////////
void Placer::bindOutputsLE(list<AssayNode *> *outputNodes)
{
	Sort::sortNodesByStartTS(outputNodes);
	for (unsigned i = 0; i < outRes->size(); i++)
	{
		list<AssayNode *> scheduled;
		IoResource *outR = outRes->at(i);
		list<AssayNode *>::iterator it = outputNodes->begin();
		for (; it != outputNodes->end(); it++)
		{
			AssayNode *n = *it;
			if (n->GetPortName() == outR->name)
			{
				outR->schedule.push_back(n);
				scheduled.push_back(n);
				n->status = BOUND;
				n->ioPort = outR->port;
			}
		}
		while (!scheduled.empty())
		{
			outputNodes->remove(scheduled.front());
			scheduled.pop_front();
		}
	}
}

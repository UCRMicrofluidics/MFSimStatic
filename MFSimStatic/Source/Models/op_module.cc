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
 * Source: op_module.cc															*
 * Original Code Author(s): Benjamin Preciado									*
 * Original Completion/Release Date: October 7, 2012							*
 *																				*
 * Details: N/A																	*
 *																				*
 * Revision History:															*
 * WHO		WHEN		WHAT													*
 * ---		----		----													*
 * FML		MM/DD/YY	One-line description									*
 *-----------------------------------------------------------------------------*/
#include "../../Headers/Models/op_module.h"
#include <iostream>
using namespace std;

int OpModule::next_id = 1;

///////////////////////////////////////////////////////////////////////////////////
// Constructors
///////////////////////////////////////////////////////////////////////////////////
OpModule::OpModule(OperationType oType, int lx, int ty, int rx, int by)
{
	id = next_id++;
	operationType = oType;
	leftX = lx;
	topY = ty;
	rightX = rx;
	bottomY = by;
}
OpModule::OpModule(ResourceType rType, int lx, int ty, int rx, int by)
{
	id = next_id++;
	boundedResType = rType;
	leftX = lx;
	topY = ty;
	rightX = rx;
	bottomY = by;
}
OpModule::OpModule(OperationType oType, ResourceType rType, int lx, int ty, int rx, int by, unsigned long long start, unsigned long long end)
{
	id = next_id++;
	operationType = oType;
	boundedResType = rType;
	leftX = lx;
	topY = ty;
	rightX = rx;
	bottomY = by;
	startTS = start;
	endTS = end;
}

///////////////////////////////////////////////////////////////////////////////////
// Deconstructor
///////////////////////////////////////////////////////////////////////////////////
OpModule::~OpModule()
{
}

///////////////////////////////////////////////////////////////////////////////////
// Operator overload for '='
///////////////////////////////////////////////////////////////////////////////////
OpModule OpModule::operator=(const OpModule& opM) const
{
	return OpModule(opM.boundedResType, opM.leftX, opM.topY, opM.rightX, opM.bottomY);
}

///////////////////////////////////////////////////////////////////////////////////
// Displays info about the OpModule
///////////////////////////////////////////////////////////////////////////////////
void OpModule::display()
{
	cout << boundedResType << " / " << leftX << " " << topY << " " << rightX << " " << bottomY << " / ";
	cout << startTS << " " << endTS << endl;
}

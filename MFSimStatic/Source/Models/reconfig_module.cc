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
 * Source: reconfig_module.cc													*
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
#include "../../Headers/Models/reconfig_module.h"
//#include "../../Headers/Models/assay_node.h"

int ReconfigModule::next_id = 1;

///////////////////////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////////////////////
ReconfigModule::ReconfigModule(ResourceType rType, int lx, int ty, int rx, int by)
{
	id = next_id++;
	resourceType = rType;
	leftX = lx;
	topY = ty;
	rightX = rx;
	bottomY = by;
	numDrops = 0;
	tiledNum = -1;
}

///////////////////////////////////////////////////////////////////////////////////
// Deconstructor
///////////////////////////////////////////////////////////////////////////////////
ReconfigModule::~ReconfigModule()
{
}

///////////////////////////////////////////////////////////////////////////////////
// Gets the number of cells within a module
///////////////////////////////////////////////////////////////////////////////////
int ReconfigModule::getNumCellsInModule()
{
	return ((bottomY-topY+1)*(rightX-leftX+1));
}

/////////////////////////////////////////////////////////
// Gets the number of cells in the circumference of the
// module
/////////////////////////////////////////////////////////
int ReconfigModule::getNumCellsInCirc()
{
	return ((2*((bottomY-topY+1)+(rightX-leftX-1))));
}

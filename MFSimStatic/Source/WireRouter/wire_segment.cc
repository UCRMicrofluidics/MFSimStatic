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
 * Source: wire_segment.cc														*
 * Original Code Author(s): Dan Grissom											*
 * Original Completion/Release Date: April 1, 2013								*
 *																				*
 * Details: N/A																	*
 *																				*
 * Revision History:															*
 * WHO		WHEN		WHAT													*
 * ---		----		----													*
 * FML		MM/DD/YY	One-line description									*
 *-----------------------------------------------------------------------------*/
#include "../../Headers/WireRouter/wire_segment.h"

#include <cmath>
#include <math.h>
using std::abs;


int WireSegment::next_id = 1;

///////////////////////////////////////////////////////////////////////////////////
// Constructors
///////////////////////////////////////////////////////////////////////////////////
WireSegment::WireSegment()
{
	id = next_id++;

	pinNo -1;
	layer -1;
	segmentType = LINE_WS;

	// Start/stop relative locations
	sourceWireCellX = -1;
	sourceWireCellY = -1;
	destWireCellX = -1;
	destWireCellY = -1;

	// Arc parameters
	startAngle = -1;
	arcAngle = -1;
}

WireSegment::WireSegment(int pin, int lay, int beginX, int beginY, int endX, int endY)
{
	id = next_id++;

	pinNo = pin;
	layer = lay;
	segmentType = LINE_WS;

	// Start/stop relative locations
	sourceWireCellX = beginX;
	sourceWireCellY = beginY;
	destWireCellX = endX;
	destWireCellY = endY;

	// Arc parameters
	startAngle = -1;
	arcAngle = -1;
}
///////////////////////////////////////////////////////////////////////////////////
// Deconstructor
///////////////////////////////////////////////////////////////////////////////////
WireSegment::~WireSegment()
{

}

///////////////////////////////////////////////////////////////////////////////////
// Computes the length of the wire segment based on the pythagorean theorem. Length
// units are wire-cell units
///////////////////////////////////////////////////////////////////////////////////
double WireSegment::getLengthInWireCells()
{
	// This case routes off the device, don't count this
	if (destWireCellX == -1 || destWireCellY == -1 || sourceWireCellX == -1 || sourceWireCellY == -1)
		return 0;

	double a = abs((double)destWireCellX - (double)sourceWireCellX);
	double b = abs((double)destWireCellY - (double)sourceWireCellY);
	double length = sqrt(pow(a, 2.0) + pow(b, 2.0));
	return length;
}


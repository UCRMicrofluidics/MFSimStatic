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
 * Source: pin_mapper.cc														*
 * Original Code Author(s): Dan Grissom											*
 * Original Completion/Release Date: December 11, 2012							*
 *																				*
 * Details: N/A																	*
 *																				*
 * Revision History:															*
 * WHO		WHEN		WHAT													*
 * ---		----		----													*
 * FML		MM/DD/YY	One-line description									*
 *-----------------------------------------------------------------------------*/
#include "../../Headers/PinMapper/pin_mapper.h"
#include "../../Headers/Testing/claim.h"
#include <set>

///////////////////////////////////////////////////////////////////////////////////
// Constructors
///////////////////////////////////////////////////////////////////////////////////
PinMapper::PinMapper()
{
	arch = NULL;
	pinMapping = new vector<vector<int> *>();
	inputPins = new vector<int>();
	outputPins = new vector<int>();
	specialPurposePins = new vector<int>();
	availResCount = new vector<int>();
	for (int i = 0; i <= RES_TYPE_MAX; i++)
		availResCount->push_back(0);
	availRes = new vector<vector<FixedModule *> *>();
	for (int i = 0; i <= RES_TYPE_MAX; i++)
		availRes->push_back(new vector<FixedModule *>());
	hasExecutableSyntesisMethod = true;
}
PinMapper::PinMapper(DmfbArch *dmfbArch)
{
	arch = dmfbArch;
	pinMapping = new vector<vector<int> *>();
	inputPins = new vector<int>();
	outputPins = new vector<int>();
	specialPurposePins = new vector<int>();
	availResCount = new vector<int>();
	for (int i = 0; i <= RES_TYPE_MAX; i++)
		availResCount->push_back(0);
	availRes = new vector<vector<FixedModule *> *>();
	for (int i = 0; i <= RES_TYPE_MAX; i++)
		availRes->push_back(new vector<FixedModule *>());
	hasExecutableSyntesisMethod = true;
}

///////////////////////////////////////////////////////////////////////////////////
// Deconstructor
///////////////////////////////////////////////////////////////////////////////////
PinMapper::~PinMapper()
{
	while (pinMapping->size() > 0)
	{
		vector<int> *col = pinMapping->back();
		pinMapping->pop_back();
		delete col;
	}
	delete pinMapping;

	while (availRes->size() > 0)
	{
		vector<FixedModule *> *resources = availRes->back();
		availRes->pop_back();
		while(resources->size() > 0)
		{
			FixedModule *fm = resources->back();
			resources->pop_back();
			if (fm)
				delete fm;
		}
		delete resources;
	}
	delete availRes;

	delete availResCount;
	delete inputPins;
	delete outputPins;
	delete specialPurposePins;
}

///////////////////////////////////////////////////////////////
// Initializes the pin-mapping to a blank array of the size
// of the dimensions already specified. Array is filled with
// -1's, which denotes that the electrodes are not mapped yet.
///////////////////////////////////////////////////////////////
void PinMapper::initPinMapping()
{
	claim(arch->getNumCellsX() > 0 && arch->getNumCellsY() > 0, "DMFB dimensions must be set before initializing pin map.");

	// First, delete the current mapping...
	for (unsigned i = 0; i < pinMapping->size(); i++)
	{
		vector<int> *col = pinMapping->back();
		pinMapping->pop_back();
		delete col;
	}

	// Then clear the i/o pins
	inputPins->clear();
	outputPins->clear();

	// ... then create a 2D-array which holds the pin numbers for each electrode
	for (int x = 0; x < arch->getNumCellsX(); x++)
	{
		vector<int> *pinCol = new vector<int>();
		for (int y = 0; y < arch->getNumCellsY(); y++)
			pinCol->push_back(-1);
		pinMapping->push_back(pinCol);
	}
}

///////////////////////////////////////////////////////////////
// Prints the pin-mapping to the console.
///////////////////////////////////////////////////////////////
void PinMapper::printPinMapping()
{
	cout << "Printing Pin-mapping:" << endl << endl;
	for (int y = 0; y < arch->getNumCellsY(); y++)
	{
		for (int x = 0; x < arch->getNumCellsX(); x++)
		{
			if (pinMapping->at(x)->at(y) >= 0)
				cout << pinMapping->at(x)->at(y);
			else
				cout << "-";

			if (x == arch->getNumCellsX()-1)
				cout << endl;
			else
				cout << "\t";
		}
	}
}

///////////////////////////////////////////////////////////////
// Examine the DMFB and return the number of unique pins.
///////////////////////////////////////////////////////////////
int PinMapper::getNumUniquePins()
{
	set<int> pins;
	for (unsigned x = 0; x < pinMapping->size(); x++)
		for (unsigned y = 0; y < pinMapping->at(x)->size(); y++)
			if (pinMapping->at(x)->at(y) >= 0)
				pins.insert(pinMapping->at(x)->at(y));
	return pins.size();
}

///////////////////////////////////////////////////////////////
// Examine the DMFB and return the number of active electrodes
// (some can just be space).
///////////////////////////////////////////////////////////////
int PinMapper::getNumElectrodes()
{
	int e = 0;
	for (unsigned x = 0; x < pinMapping->size(); x++)
		for (unsigned y = 0; y < pinMapping->at(x)->size(); y++)
			if (pinMapping->at(x)->at(y) >= 0)
				e++;
	return e;
}

///////////////////////////////////////////////////////////////////////////////////
// This function should be overridden to create a custom pin-mapping.
///////////////////////////////////////////////////////////////////////////////////
void PinMapper::setCustomMapping()
{
	claim(false, "No Pin-mapper has been selected.");
}

///////////////////////////////////////////////////////////////////////////////////
// This function should be overridden; it is called just before scheduling.
///////////////////////////////////////////////////////////////////////////////////
void PinMapper::setMapPreSched()
{
	initPinMapping();
	setCustomMapping();
}

///////////////////////////////////////////////////////////////////////////////////
// This function should be overridden; it is called just after routing.
///////////////////////////////////////////////////////////////////////////////////
void PinMapper::setMapPostRoute(vector<vector<int> *> *pinActivations)
{
	initPinMapping();
	setCustomMapping();
}

///////////////////////////////////////////////////////////////////////////////////
// This function should be overridden; it is called to flatten all of the special-
// purpose pin vectors for a particular pin-mapper into a single vector that
// can be written to file. It is arranged such that a single number dictating
// directly the length of the next sub-list exists before each sub-list.
//
// NOTE: Order MUST match with unflattenSpecialPurposePins().
///////////////////////////////////////////////////////////////////////////////////
void PinMapper::flattenSpecialPurposePins()
{
	claim(false, "No Pin-mapper has been selected.");
}

///////////////////////////////////////////////////////////////////////////////////
// This function should be overridden; it is called to un-flatten the single
// special-purpose pin vector into all of the special-purpose pin vectors
// (for reading from file). It is arranged such that a single number dictating
// directly the length of the next sub-list exists before each sub-list.
//
// NOTE: Order MUST match with flattenSpecialPurposePins().
///////////////////////////////////////////////////////////////////////////////////
void PinMapper::unflattenSpecialPurposePins()
{
	claim(false, "No Pin-mapper has been selected.");
}

///////////////////////////////////////////////////////////////////////////////////
// This method should be called after pin-mapping to resize the DMFB. Essentially,
// the pin-mapping could have made a large chunk of the DMFB unnecessary (that is,
// it removed the electrodes). This function will remove any rows or columns of
// unused electrodes from the edges that don't require I/Os to be moved.
///////////////////////////////////////////////////////////////////////////////////
void PinMapper::resizeDmfb()
{
	//printPinMapping();
	// DMFB min/max values to be resized (start with current)
	int leftX = 0;
	int rightX = arch ->getNumCellsX() - 1;
	int topY = 0;
	int botY = arch->getNumCellsY() - 1;

	// Sort ports by side
	vector<IoPort *> northPorts;
	vector<IoPort *> southPorts;
	vector<IoPort *> westPorts;
	vector<IoPort *> eastPorts;
	int northPortMin = -1;
	int southPortMin = -1;
	int westPortMin = -1;
	int eastPortMin = -1;
	int northPortMax = -1;
	int southPortMax = -1;
	int westPortMax = -1;
	int eastPortMax = -1;

	for (unsigned i = 0; i < arch->getIoPorts()->size(); i++)
	{
		IoPort *p = arch->getIoPorts()->at(i);
		if (p->getSide() == NORTH)
		{
			northPorts.push_back(p);
			if (northPortMin == -1 || p->getPosXY() < northPortMin)
				northPortMin = p->getPosXY();
			if (northPortMax == -1 || p->getPosXY() > northPortMax)
				northPortMax = p->getPosXY();
		}
		else if (p->getSide() == SOUTH)
		{
			southPorts.push_back(p);
			if (southPortMin == -1 || p->getPosXY() < southPortMin)
				southPortMin = p->getPosXY();
			if (southPortMax == -1 || p->getPosXY() > southPortMax)
				southPortMax = p->getPosXY();
		}
		else if (p->getSide() == WEST)
		{
			westPorts.push_back(p);
			if (westPortMin == -1 || p->getPosXY() < westPortMin)
				westPortMin = p->getPosXY();
			if (westPortMax == -1 || p->getPosXY() > westPortMax)
				westPortMax = p->getPosXY();
		}
		else if (p->getSide() == EAST)
		{
			eastPorts.push_back(p);
			if (eastPortMin == -1 || p->getPosXY() < eastPortMin)
				eastPortMin = p->getPosXY();
			if (eastPortMax == -1 || p->getPosXY() > eastPortMax)
				eastPortMax = p->getPosXY();
		}
	}

	vector<vector<int> *> * pm = arch->getPinMapper()->getPinMapping();

	// Can remove rows on bottom
	if (southPorts.size() == 0)
	{
		for (int y = arch->getNumCellsY() - 1; y >= 0; y--)
		{
			bool removeRow = true;
			for (int x = 0; x < arch->getNumCellsX(); x++)
			{
				if (pm->at(x)->at(y) > 0)
				{
					removeRow = false;
					break;
				}
			}

			if (removeRow && eastPortMax < y && westPortMax < y)
				botY--;
			else
				break;
		}
	}

	// Can remove rows on top
	if (northPorts.size() == 0)
	{
		for (int y = 0; y < arch->getNumCellsY(); y++)
		{
			bool removeRow = true;
			for (int x = 0; x < arch->getNumCellsX(); x++)
			{
				if (pm->at(x)->at(y) > 0)
				{
					removeRow = false;
					break;
				}
			}

			if (removeRow && eastPortMin > y && westPortMin > y)
				topY++;
			else
				break;
		}
	}

	// Can remove columns on right
	if (eastPorts.size() == 0)
	{
		for (int x = arch->getNumCellsX() - 1; x >= 0; x--)
		{
			bool removeCol = true;
			for (int y = 0; y < arch->getNumCellsY(); y++)
			{
				if (pm->at(x)->at(y) > 0)
				{
					removeCol = false;
					break;
				}
			}

			if (removeCol && northPortMax < x && southPortMax < x)
				rightX--;
			else
				break;
		}
	}

	// Can remove columns on left
	if (westPorts.size() == 0)
	{
		for (int x = 0; x < arch->getNumCellsX(); x++)
		{
			bool removeCol = true;
			for (int y = 0; y < arch->getNumCellsY(); y++)
			{
				if (pm->at(x)->at(y) > 0)
				{
					removeCol = false;
					break;
				}
			}

			if (removeCol && northPortMin > x && southPortMin > x)
				topY++;
			else
				break;
		}
	}

	cout << "New X " << leftX << "-" << rightX << endl;
	cout << "New Y " << topY << "-" << botY << endl;
	cout << "Implement new function in architecture to resize the DMFB." << endl;

	//arch->
}

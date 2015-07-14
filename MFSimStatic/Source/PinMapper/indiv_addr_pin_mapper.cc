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
 * Source: indiv_addr_pin_mapper.cc														*
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
#include "../../Headers/PinMapper/indiv_addr_pin_mapper.h"
#include "../../Headers/Util/sort.h"

///////////////////////////////////////////////////////////////////////////////////
// Constructors
///////////////////////////////////////////////////////////////////////////////////
IndivAddrPinMapper::IndivAddrPinMapper()
{
	arch = NULL;
}
IndivAddrPinMapper::IndivAddrPinMapper(DmfbArch *dmfbArch)
{
	arch = dmfbArch;
}

///////////////////////////////////////////////////////////////////////////////////
// Deconstructor
///////////////////////////////////////////////////////////////////////////////////
IndivAddrPinMapper::~IndivAddrPinMapper()
{

}

///////////////////////////////////////////////////////////////////////////////////
// This function sets each pin to be individually controlled.  It also calls the
// function which computes the resources.
///////////////////////////////////////////////////////////////////////////////////
void IndivAddrPinMapper::setCustomMapping()
{
	int pinNo = 0;
	for (int y = 0; y < arch->getNumCellsY(); y++)
		for (int x = 0; x < arch->getNumCellsX(); x++)
			pinMapping->at(x)->at(y) = pinNo++;

	// Set pin numbers for the I/O ports
	for (unsigned i = 0; i < arch->getIoPorts()->size(); i++)
	{
		if (arch->getIoPorts()->at(i)->isAnInput())
			inputPins->push_back(pinNo);
		else
			outputPins->push_back(pinNo);
		arch->getIoPorts()->at(i)->setPinNo(pinNo++);
	}

	sort(inputPins->begin(), inputPins->end());
	sort(outputPins->begin(), outputPins->end());

	computeAvailResources();
}

///////////////////////////////////////////////////////////////////////////////////
// This function should be overridden; it is called just before scheduling.
///////////////////////////////////////////////////////////////////////////////////
void IndivAddrPinMapper::setMapPreSched()
{
	initPinMapping();
	setCustomMapping();
}

///////////////////////////////////////////////////////////////////////////////////
// This function should be overridden; it is called just after routing.
///////////////////////////////////////////////////////////////////////////////////
void IndivAddrPinMapper::setMapPostRoute(vector<vector<int> *> *pinActivations)
{
	// Do nothing...mapping set at beginning.
}

///////////////////////////////////////////////////////////////////////////////////
// This function should be overridden; it is called to flatten all of the special-
// purpose pin vectors for a particular pin-mapper into a single vector that
// can be written to file. It is arranged such that a single number dictating
// directly the length of the next sub-list exists before each sub-list.
//
// NOTE: Order MUST match with unflattenSpecialPurposePins().
///////////////////////////////////////////////////////////////////////////////////
void IndivAddrPinMapper::flattenSpecialPurposePins()
{
	// Just write the I/O pins b/c there are no other special purpose pins
	specialPurposePins->clear();
	specialPurposePins->push_back(inputPins->size());
	for (unsigned i = 0; i < inputPins->size(); i++)
		specialPurposePins->push_back(inputPins->at(i));
	specialPurposePins->push_back(outputPins->size());
	for (unsigned i = 0; i < outputPins->size(); i++)
		specialPurposePins->push_back(outputPins->at(i));
}

///////////////////////////////////////////////////////////////////////////////////
// This function should be overridden; it is called to un-flatten the single
// special-purpose pin vector into all of the special-purpose pin vectors
// (for reading from file). It is arranged such that a single number dictating
// directly the length of the next sub-list exists before each sub-list.
//
// NOTE: Order MUST match with flattenSpecialPurposePins().
///////////////////////////////////////////////////////////////////////////////////
void IndivAddrPinMapper::unflattenSpecialPurposePins()
{
	if (specialPurposePins->size() <= 0)
		return;

	inputPins->clear();
	outputPins->clear();

	int i = 0;
	int slSize = specialPurposePins->at(i++);
	int slStart = i;

	for (; i < slStart+slSize; i++)
		inputPins->push_back(specialPurposePins->at(i));
	slSize = specialPurposePins->at(i++);
	slStart = i;
	for (; i < slStart+slSize; i++)
		outputPins->push_back(specialPurposePins->at(i));
}

///////////////////////////////////////////////////////////////////////////////////
// Computes the available resources based on the provided resource-allocation type.
///////////////////////////////////////////////////////////////////////////////////
int IndivAddrPinMapper::computeAvailResources()
{
	if (resAllocType == GRISSOM_FIX_0_RA)
		return computeAvailResourcesForGrissomFixedPlacer(0);
	else if (resAllocType == GRISSOM_FIX_1_RA)
		return computeAvailResourcesForGrissomFixedPlacer(1);
	else if (resAllocType == GRISSOM_FIX_2_RA)
		return computeAvailResourcesForGrissomFixedPlacer(2);
	else if (resAllocType == GRISSOM_FIX_3_RA)
		return computeAvailResourcesForGrissomFixedPlacer(3);
	else if (resAllocType == INHERIT_RA)
		return 0;
	else
	{
		claim(false, "An invalid resource-allocation type was specified to the scheduler.");
		return 0; // Unreachable, Eliminates Warning
	}
}

///////////////////////////////////////////////////////////////////////////////////
// Resets the available resources for scheduling so the scheduler knows how many
// modules it can schedule at any given time.
//
// Making some assumptions about how resources are divvied out
// Assume we use all 4x3 (Horiz x Vert) modules that cannot be rotated.
// Assume there is an interference region surrounding all modules.
// Assume we must maintain at least one cell between all placed modules.
// Assume that if a heater/detector overlaps a module, it is a heating/detecting module
// Given these assumptions, we compute the number of available resources for scheduling:
//
// Creates a horizontal routing path in-between every 'horizRouteSpacing' vertical modules
///////////////////////////////////////////////////////////////////////////////////
int IndivAddrPinMapper::computeAvailResourcesForGrissomFixedPlacer(int horizRouteSpacing)
{
	int modWidth = 4;//4;
	int modHeight = 3;//3;
	int modIntWidth = modWidth + 2; // For interference region
	int modIntHeight = modHeight + 2; // For interference region


	for (int i = 0; i <= RES_TYPE_MAX; i++)
		availResCount->at(i) = 0;

	int maxX = arch->getNumCellsX();
	int maxY = arch->getNumCellsY();
	int x;
	int y;


	// Create a map of the fixed-module layout
	ResourceType cellType[maxX][maxY];
	for (x = 0; x < maxX; x++)
		for (y = 0; y < maxY; y++)
			cellType[x][y] = BASIC_RES;
	for (unsigned i = 0; i < arch->getExternalResources()->size(); i++)
	{
		FixedModule *fm = arch->getExternalResources()->at(i);
		for (x = fm->getLX(); x <= fm->getRX(); x++)
		{
			for (y = fm->getTY(); y <= fm->getBY(); y++)
			{
				claim(x < maxX && y < maxY, "A fixed module has coordinates that do not fit on the array.");
				claim(cellType[x][y] == BASIC_RES, "A cell has already been augmented with a fixed module; cannot augment with two fixed modules.");
				if (fm->getResourceType() == H_RES)
					cellType[x][y] = H_RES;
				else if (fm->getResourceType() == D_RES)
					cellType[x][y] = D_RES;
				else
					claim(false, "Unknown type of fixed resource.");
			}
		}
	}

	x = 1;
	y = 1;
	int tileNum = 0;
	int numModsY = 0;
	while (y + modIntHeight < maxY)
	{
		if ( (x + modIntWidth <= maxX) && (y + modIntHeight <= maxY) )
		{
			bool canHeat = false;
			bool canDetect = false;

			for (int xMod = x+1; xMod < x+1+modWidth; xMod++)
			{
				for (int yMod = y+1; yMod < y+1+modHeight; yMod++)
				{
					if (cellType[xMod][yMod] == D_RES)
						canDetect = true;
					else if (cellType[xMod][yMod] == H_RES)
						canHeat = true;
				}
			}

			if (canDetect && canHeat)
			{
				availResCount->at(DH_RES)++;
				FixedModule *fm = new FixedModule(DH_RES, x+1, y+1, x+modWidth, y+modHeight);
				availRes->at(DH_RES)->push_back(fm);
				availRes->at(DH_RES)->back()->setTileNum(tileNum++);
			}
			else if (canDetect)
			{
				availResCount->at(D_RES)++;
				FixedModule *fm = new FixedModule(D_RES, x+1, y+1, x+modWidth, y+modHeight);
				availRes->at(D_RES)->push_back(fm);
				availRes->at(D_RES)->back()->setTileNum(tileNum++);
			}
			else if (canHeat)
			{
				availResCount->at(H_RES)++;
				FixedModule *fm = new FixedModule(H_RES, x+1, y+1, x+modWidth, y+modHeight);
				availRes->at(H_RES)->push_back(fm);
				availRes->at(H_RES)->back()->setTileNum(tileNum++);
			}
			else
			{
				availResCount->at(BASIC_RES)++;
				FixedModule *fm = new FixedModule(BASIC_RES, x+1, y+1, x+modWidth, y+modHeight);
				availRes->at(BASIC_RES)->push_back(fm);
				availRes->at(BASIC_RES)->back()->setTileNum(tileNum++);
			}
		}
		x = x + modIntWidth +1;
		if (x + modIntWidth >= maxX)
		{
			x = 1;
			//y = y + modIntHeight + 1;

			numModsY++;
			if (horizRouteSpacing == 0 || numModsY % horizRouteSpacing != 0)
				y = y + modHeight + 1;
			else
				y = y + modIntHeight + 1;
		}
	}

	//cout << availRes[DH_RES] << " DH" << endl;
	//cout << availRes[D_RES] << " D" << endl;
	//cout << availRes[H_RES] << " H" << endl;
	//cout << availRes[BASIC_RES] << " G" << endl;

	int numModules = 0;
	for (int i = 0; i <= RES_TYPE_MAX; i++)
		numModules += availResCount->at(i);
	return numModules;
}

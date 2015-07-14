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
 * Source: grissom_fppc_pin_mapper.cc											*
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
#include "../../Headers/PinMapper/grissom_fppc_pin_mapper.h"
#include "../../Headers/Testing/claim.h"

///////////////////////////////////////////////////////////////////////////////////
// Constructors
///////////////////////////////////////////////////////////////////////////////////
GrissomFppcPinMapper::GrissomFppcPinMapper()
{
	arch = NULL;
	mHoldPins = new vector<int>();
	mIOPins = new vector<int>();
	ssHoldPins = new vector<int>();
	ssIOPins = new vector<int>();
	mixPins = new vector<vector<int> *>();
}
GrissomFppcPinMapper::GrissomFppcPinMapper(DmfbArch *dmfbArch)
{
	arch = dmfbArch;
	mHoldPins = new vector<int>();
	mIOPins = new vector<int>();
	ssHoldPins = new vector<int>();
	ssIOPins = new vector<int>();
	mixPins = new vector<vector<int> *>();
}

///////////////////////////////////////////////////////////////////////////////////
// Deconstructor
///////////////////////////////////////////////////////////////////////////////////
GrissomFppcPinMapper::~GrissomFppcPinMapper()
{
	delete mHoldPins;
	delete mIOPins;
	delete ssHoldPins;
	delete ssIOPins;

	while (!mixPins->empty())
	{
		vector<int> *v = mixPins->back();
		mixPins->pop_back();
		v->clear();
		delete v;
	}
	delete mixPins;
}

///////////////////////////////////////////////////////////////////////////////////
// Generates a pin-mapping for the FPPC design. Also, generates a list of available
// resources for synthesis based on the pin-mapping.
//
// This allocates resources based on the specific pre-designed layout of the
// Field Programmable Pin-Constrained Design (FPPC).  This layout calls for mix
// modules, modules which can perform split, storage and detect (SSD) operations,
// and a buffer module which is used to resolve droplet dependencies during routing.
// All mix modules are 2hx4w with a I/O cell to the bottom right. All SSD modules are
// 1hx1w with a I/O cell to the left. Mix modules and stacked in single column and
// SSD modules are stacked in a single column to the right of the mix modules with
// a vertical routing channel in between the two stacks of modules. The two stacks
// are of equal height and routing cells surrounding each stack.

// Example: On a 12x21 array of electrodes, this layout calls for 6 Mix modules,
// 8 SSD modules, and 1 buffer module.
//
// Given these details, we set the number of available resources for scheduling:
///////////////////////////////////////////////////////////////////////////////////
void GrissomFppcPinMapper::setCustomMapping()
{
	claim(resAllocType == PC_INHERENT_RA, "Grissom FPPC Pin-mapper uses an internal resource-allocation method; Select the Pin-Constrained Inherent Resource Allocation Type");

	// These dimensions must correspond with the same ones in pin_constrained_placer_1.cc
	int mWidth = getMixWidth(); // md = mix/heat/detect
	int mHeight = getMixHeight();
	int ssWidth = 1; // md = split/store
	int ssHeight = 1;
	routingColumn = 3 + mWidth;

	// Check that min dimensions are okay to have at least a few modules on the board
	claim((mWidth + 2) + (ssWidth + 2) + 3 <= arch->getNumCellsX(), "Width of array must be larger to fit this design/layout.");
	claim(mHeight*2 + 5 <= arch->getNumCellsY(), "Height of array must be larger to stack at least 2 mix modules so this design/layout can fit.");


	// Now, set-up the actual mapping for the array
	int pinNo = 1;
	// 1-2-3 for 2 horizontal paths
	for (int x = 0; x < arch->getNumCellsX(); x++)
	{
		pinMapping->at(x)->at(0) = (x % 3) + pinNo;
		pinMapping->at(x)->at(arch->getNumCellsY()-1) = (x % 3) + pinNo;
	}
	pinNo += 3;

	// 4-5-6 for 3 vertical paths
	for (int y = 1; y < arch->getNumCellsY()-1; y++)
	{
		pinMapping->at(0)->at(y) = ((y-1) % 3) + pinNo;
		pinMapping->at(mWidth + 3)->at(y) = ((y-1) % 3) + pinNo;
		pinMapping->at(arch->getNumCellsX()-1)->at(y) = ((y-1) % 3) + pinNo;
	}
	pinNo += 3;

	// 7-8-9-10-11-12-13 for mixers (2x4 mixers)
	// 14+ for all other independent pins
	// First calculate mix/detect resources
	int x = 2; // Always start here
	int y = 2; // Always start here

	// While there is enough room for this module
	int tileNum = 0;
	int modStartPinNo = pinNo;
	pinNo += (mWidth*mHeight-1);
	while (y + mHeight + 2 <= arch->getNumCellsY())
	{
		int modPinNo = modStartPinNo;

		// Add to the module count and create an instance
		availResCount->at(BASIC_RES) = availResCount->at(BASIC_RES) + 1;
		availRes->at(BASIC_RES)->push_back(new FixedModule(BASIC_RES, x, y, x+mWidth-1, y+mHeight-1));
		availRes->at(BASIC_RES)->back()->setTileNum(tileNum++);

		// The bottom row, next to right-most cell in module is independent;
		// Otherwise, give each module a tied pin number
		for (int modY = y; modY < y+mHeight; modY++)
		{
			for (int modX = x; modX < x+mWidth; modX++)
			{
				if (!(modX == x+mWidth-2 && modY == y+mHeight-1))
					pinMapping->at(modX)->at(modY) = modPinNo++;
				else
				{
					pinMapping->at(modX)->at(modY) = pinNo;
					mHoldPins->push_back(pinNo);
					pinNo++;
				}
			}
		}

		// Also, set I/O cell to bottom-right
		pinMapping->at(x+mWidth)->at(y+mHeight-1) = pinNo;
		mIOPins->push_back(pinNo);
		pinNo++;

		y = y + mHeight + 1;
	}

	// Grab the mixer pins; start with pin to left of hold pin and travel clockwise
	// Just base off first module, whose top-left coordinate is (2,2)
	int mx = 2+mWidth-2;
	int my = 2+mHeight-1;
	mixPins->push_back(new vector<int>());
	while (mx > 2) // Left
		mixPins->back()->push_back(pinMapping->at(--mx)->at(my));
	while (my > 2) // Up
		mixPins->back()->push_back(pinMapping->at(mx)->at(--my));
	while (mx < 2+mWidth-1) // Right
		mixPins->back()->push_back(pinMapping->at(++mx)->at(my));
	while (my < 2+mHeight-1) // Down
		mixPins->back()->push_back(pinMapping->at(mx)->at(++my));

	// Now, compute the number of storage/split modules we can fit
	int numSsModSpots = ((arch->getNumCellsY() - 3) / (ssHeight + 1));
	int skipMod = (numSsModSpots / 2) + 1; // skip a module and put a routing buffer module in the middle
	x = mWidth + 5; // Starting location for split/store modules
	y = 2; // Starting location for split/store modules
	tileNum = 0;

	for (int i = 1; i <= numSsModSpots; i++)
	{
		// Add to the module count and create an instance
		if (i == skipMod)
		{
			availResCount->at(RB_RES) = availResCount->at(RB_RES) + 1;
			availRes->at(RB_RES)->push_back(new FixedModule(RB_RES, x, y, x+ssWidth-1, y+ssHeight-1));
			availRes->at(RB_RES)->back()->setTileNum(tileNum++);
		}
		else
		{
			availResCount->at(SSD_RES) = availResCount->at(SSD_RES) + 1;
			availRes->at(SSD_RES)->push_back(new FixedModule(SSD_RES, x, y, x+ssWidth-1, y+ssHeight-1));
			availRes->at(SSD_RES)->back()->setTileNum(tileNum++);
			arch->getExternalResources()->push_back(new FixedModule(D_RES, x, y, x+ssWidth-1, y+ssHeight-1)); // Adds fixed detector at location for graphic
		}


		pinMapping->at(x-1)->at(y) = pinNo;
		ssIOPins->push_back(pinNo);
		pinNo++;
		for (int modY = y; modY < y+ssHeight; modY++)
		{
			for (int modX = x; modX < x+ssWidth; modX++)
			{
				pinMapping->at(modX)->at(modY) = pinNo;
				ssHoldPins->push_back(pinNo);
				pinNo++;
			}
		}

		y = y + ssHeight + 1;
	}

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
}

///////////////////////////////////////////////////////////////////////////////////
// Generates a pin-mapping for the FPPC2 design. Also, generates a list of available
// resources for synthesis based on the pin-mapping.
//
// This allocates resources based on the specific pre-designed layout of the
// enhanced Field Programmable Pin-Constrained Design.  This layout calls for mix
// modules, modules which can perform split, storage and detect (SSD) operations,
// and a buffer module which is used to resolve droplet dependencies during routing.
// All mix modules are 2hx4w with a I/O cell to the bottom right. All SSD modules are
// 1hx1w with a I/O cell to the left. Mix modules and stacked in single column and
// SSD modules are stacked in a single column to the right of the mix modules with
// a vertical routing channel in between the two stacks of modules. The two stacks
// are of equal height and routing cells surrounding each stack.
//
// This funciton sets out a mapping similar to setCustomMapping (the original
// FPPC), but adds some extra spacing between each group of 4 mixers, and requires
// that each group of 4 mixers be wired separately.  Also, the wiring for the
// routing buses is different:
// Pin-optimized - 2 horizontal and 1 vertical 3-phase bus
// Route-optimized - Individually addressable horizontal/vertical busses
///////////////////////////////////////////////////////////////////////////////////
void GrissomFppcPinMapper::setEnhancedCustomMapping()
{
	claim(resAllocType == PC_INHERENT_RA, "Grissom FPPC Pin-mapper uses an internal resource-allocation method; Select the Pin-Constrained Inherent Resource Allocation Type");

	// These dimensions must correspond with the same ones in pin_constrained_placer_1.cc
	int mWidth = getMixWidth(); // md = mix/heat/detect
	int mHeight = getMixHeight();
	int ssWidth = 1; // md = split/store
	int ssHeight = 1;
	routingColumn = 2 + mWidth;

	// Set offset to center routing column
	int mid = arch->getNumCellsX() / 2;
	int offset = 0;
	if (mid > routingColumn)
	{
		offset = mid - routingColumn;
		routingColumn = mid;
	}
	else
		offset = 0;

	// Check that min dimensions are okay to have at least a few modules on the board
	claim((mWidth + 2) + (ssWidth + 2) + 1 <= arch->getNumCellsX(), "Width of array must be larger to fit this design/layout.");
	claim(mHeight*2 + 6 <= arch->getNumCellsY(), "Height of array must be larger to stack at least 2 mix modules so this design/layout can fit.");


	// Now, set-up the actual mapping for the array
	int pinNo = 1;

	// Set pin numbers for the three busses
	if (type == ENHANCED_FPPC_ROUTE_OPT_PM)
	{
		// Individually addressable
		for (int i = 0; i < arch->getNumCellsX(); i++)
			pinMapping->at(i)->at(0) = pinNo++;
		for (int i = 1; i < arch->getNumCellsY()-1; i++)
			pinMapping->at(routingColumn)->at(i) = pinNo++;
		for (int i = 0; i < arch->getNumCellsX(); i++)
			pinMapping->at(i)->at(arch->getNumCellsY()-1) = pinNo++;
	}
	else if (type == ENHANCED_FPPC_PIN_OPT_PM)
	{
		int phase = 3; // Using a 3-phase bus

		// Each of the 3 routing channels is its own 3-phase bus
		for (int x = 0; x < arch->getNumCellsX(); x++)
			pinMapping->at(x)->at(0) = (x % phase) + pinNo;
		pinNo += phase;
		for (int y = 1; y < arch->getNumCellsY()-1; y++)
			pinMapping->at(routingColumn)->at(y) = ((y-1) % 3) + pinNo;
		pinNo += 3;
		for (int x = 0; x < arch->getNumCellsX(); x++)
			pinMapping->at(x)->at(arch->getNumCellsY()-1) = (x % phase) + pinNo;
		pinNo += phase;
	}
	else
		claim(false, "Unrecognized pin-mapping type.");

	// First calculate mix/detect resources
	int x = 1 + offset; // Always start here
	int y = 3; // Always start here
	vector<pair<int, int> > mixerHolds;
	vector<pair<int, int> >	mixerIOs;

	// While there is enough room for this module, map shared pins for modules
	int tileNum = 0;
	int modStartPinNo = pinNo;
	pinNo += (mWidth*mHeight-1);
	while (y + mHeight + 2 <= arch->getNumCellsY())
	{
		int modPinNo = modStartPinNo;

		// Add to the module count and create an instance
		availResCount->at(BASIC_RES) = availResCount->at(BASIC_RES) + 1;
		availRes->at(BASIC_RES)->push_back(new FixedModule(BASIC_RES, x, y, x+mWidth-1, y+mHeight-1));
		availRes->at(BASIC_RES)->back()->setTileNum(tileNum++);

		// The bottom row, next to right-most cell in module is independent;
		// Otherwise, give each module a tied pin number
		for (int modY = y; modY < y+mHeight; modY++)
		{
			for (int modX = x; modX < x+mWidth; modX++)
			{
				if (!(modX == x+mWidth-2 && modY == y+mHeight-1))
					pinMapping->at(modX)->at(modY) = modPinNo++;
				else
					mixerHolds.push_back(make_pair(modX, modY));
			}
		}

		// Also, set I/O cell to bottom-right
		mixerIOs.push_back(make_pair(x+mWidth, y+mHeight-1));

		// If starting new group of 4, add to mixPins
		if (tileNum % 4 == 1)
		{
			// Grab the mixer pins; start with pin to left of hold pin and travel clockwise
			// Just base off first module, whose top-left coordinate is (1,3)
			int mx = x+mWidth-2;
			int my = y+mHeight-1;
			mixPins->push_back(new vector<int>());
			while (mx > x) // Left
				mixPins->back()->push_back(pinMapping->at(--mx)->at(my));
			while (my > y) // Up
				mixPins->back()->push_back(pinMapping->at(mx)->at(--my));
			while (mx < x+mWidth-1) // Right
				mixPins->back()->push_back(pinMapping->at(++mx)->at(my));
			while (my < y+mHeight-1) // Down
				mixPins->back()->push_back(pinMapping->at(mx)->at(++my));
		}

		// Increment y to next potential module location
		y = y + mHeight + 1;

		// If at end of a group of 4...
		if (tileNum % 4 == 0)
		{
			y+=2;
			modStartPinNo = pinNo;
			// Increment pin number if more room
			if (y + mHeight + 2 <= arch->getNumCellsY())
				pinNo += (mWidth*mHeight-1);
		}
	}

	// Now that the shared mixing pins have been mapped, map the hold and I/O pins for mixers
	for (unsigned i = 0; i < mixerHolds.size(); i++)
	{
		pinMapping->at(mixerHolds.at(i).first)->at(mixerHolds.at(i).second) = pinNo;
		mHoldPins->push_back(pinNo++);
	}
	for (unsigned i = 0; i < mixerIOs.size(); i++)
	{
		pinMapping->at(mixerIOs.at(i).first)->at(mixerIOs.at(i).second) = pinNo;
		mIOPins->push_back(pinNo++);
	}

	// Now, compute the number of storage/split modules we can fit
	vector<pair<int, int> > ssHolds;
	vector<pair<int, int> >	ssIOs;
	int numSsModSpots = ((arch->getNumCellsY() - 4) / (ssHeight + 1));
	int skipMod = (numSsModSpots / 2) + 1; // skip a module and put a routing buffer module in the middle
	x = mWidth + 4 + offset; // Starting location for split/store modules
	y = 3; // Starting location for split/store modules
	tileNum = 0;

	for (int i = 1; i <= numSsModSpots; i++)
	{
		// Add to the module count and create an instance
		if (i == skipMod)
		{
			availResCount->at(RB_RES) = availResCount->at(RB_RES) + 1;
			availRes->at(RB_RES)->push_back(new FixedModule(RB_RES, x, y, x+ssWidth-1, y+ssHeight-1));
			availRes->at(RB_RES)->back()->setTileNum(tileNum++);
		}
		else
		{
			availResCount->at(SSD_RES) = availResCount->at(SSD_RES) + 1;
			availRes->at(SSD_RES)->push_back(new FixedModule(SSD_RES, x, y, x+ssWidth-1, y+ssHeight-1));
			availRes->at(SSD_RES)->back()->setTileNum(tileNum++);
			arch->getExternalResources()->push_back(new FixedModule(D_RES, x, y, x+ssWidth-1, y+ssHeight-1)); // Adds fixed detector at location for graphic
		}

		// Save cell locations to add pin-numbers with proper ordering later
		ssIOs.push_back(make_pair(x-1, y));
		for (int modY = y; modY < y+ssHeight; modY++)
			for (int modX = x; modX < x+ssWidth; modX++)
				ssHolds.push_back(make_pair(modX, modY));
		y = y + ssHeight + 1;
	}

	// Set the pin numbers for the SSD & RB hold/IO cells
	for (unsigned i = 0; i < ssIOs.size(); i++)
	{
		pinMapping->at(ssIOs.at(i).first)->at(ssIOs.at(i).second) = pinNo;
		ssIOPins->push_back(pinNo++);
	}
	for (unsigned i = 0; i < ssHolds.size(); i++)
	{
		pinMapping->at(ssHolds.at(i).first)->at(ssHolds.at(i).second) = pinNo;
		ssHoldPins->push_back(pinNo++);
	}

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
}

///////////////////////////////////////////////////////////////////////////////////
// This function should be overridden; it is called just before scheduling.
///////////////////////////////////////////////////////////////////////////////////
void GrissomFppcPinMapper::setMapPreSched()
{
	initPinMapping();

	if (type == ORIGINAL_FPPC_PM)
		setCustomMapping();
	else
		setEnhancedCustomMapping();
}

///////////////////////////////////////////////////////////////////////////////////
// This function should be overridden; it is called just after routing.
///////////////////////////////////////////////////////////////////////////////////
void GrissomFppcPinMapper::setMapPostRoute(vector<vector<int> *> *pinActivations)
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
void GrissomFppcPinMapper::flattenSpecialPurposePins()
{
	// Just write the I/O pins b/c there are no other special purpose pins
	specialPurposePins->clear();
	specialPurposePins->push_back(inputPins->size());
	for (unsigned i = 0; i < inputPins->size(); i++)
		specialPurposePins->push_back(inputPins->at(i));

	specialPurposePins->push_back(outputPins->size());
	for (unsigned i = 0; i < outputPins->size(); i++)
		specialPurposePins->push_back(outputPins->at(i));
	specialPurposePins->push_back(mHoldPins->size());
	for (unsigned i = 0; i < mHoldPins->size(); i++)
		specialPurposePins->push_back(mHoldPins->at(i));
	specialPurposePins->push_back(mIOPins->size());
	for (unsigned i = 0; i < mIOPins->size(); i++)
		specialPurposePins->push_back(mIOPins->at(i));
	specialPurposePins->push_back(ssHoldPins->size());
	for (unsigned i = 0; i < ssHoldPins->size(); i++)
		specialPurposePins->push_back(ssHoldPins->at(i));
	specialPurposePins->push_back(ssIOPins->size());
	for (unsigned i = 0; i < ssIOPins->size(); i++)
		specialPurposePins->push_back(ssIOPins->at(i));

	// Get total number of mixing pins
	int numMixPins = 0;
	for (unsigned i = 0; i < mixPins->size(); i++)
		numMixPins+=mixPins->at(i)->size();
	specialPurposePins->push_back(numMixPins);
	for (unsigned i = 0; i < mixPins->size(); i++)
		for (unsigned j = 0; j < mixPins->at(i)->size(); j++)
			specialPurposePins->push_back(mixPins->at(i)->at(j));
	specialPurposePins->push_back(routingColumn);

}

///////////////////////////////////////////////////////////////////////////////////
// This function should be overridden; it is called to un-flatten the single
// special-purpose pin vector into all of the special-purpose pin vectors
// (for reading from file). It is arranged such that a single number dictating
// directly the length of the next sub-list exists before each sub-list.
//
// NOTE: Order MUST match with flattenSpecialPurposePins().
///////////////////////////////////////////////////////////////////////////////////
void GrissomFppcPinMapper::unflattenSpecialPurposePins()
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
	slSize = specialPurposePins->at(i++);
	slStart = i;
	for (; i < slStart+slSize; i++)
		mHoldPins->push_back(specialPurposePins->at(i));
	slSize = specialPurposePins->at(i++);
	slStart = i;
	for (; i < slStart+slSize; i++)
		mIOPins->push_back(specialPurposePins->at(i));
	slSize = specialPurposePins->at(i++);
	slStart = i;
	for (; i < slStart+slSize; i++)
		ssHoldPins->push_back(specialPurposePins->at(i));
	slSize = specialPurposePins->at(i++);
	slStart = i;
	for (; i < slStart+slSize; i++)
		ssIOPins->push_back(specialPurposePins->at(i));
	slSize = specialPurposePins->at(i++);
	slStart = i;
	int count = 0;
	for (; i < slStart+slSize; i++)
	{
		if (count++ % (getMixWidth()*getMixHeight()-1) == 0)
		{
			mixPins->push_back(new vector<int>());
		}
		mixPins->back()->push_back(specialPurposePins->at(i));
	}
	routingColumn = specialPurposePins->at(i++);
}

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
 * Source: pin_constrained_pin_mapping_test_generator.cc						*
 * Original Code Author(s): Dan Grissom											*
 * Original Completion/Release Date: April, 30, 2014							*
 *																				*
 * Details: N/A																	*
 *																				*
 * Revision History:															*
 * WHO		WHEN		WHAT													*
 * ---		----		----													*
 * FML		MM/DD/YY	One-line description									*
 *-----------------------------------------------------------------------------*/
#include "../../Headers/Testing/pin_constrained_pin_mapping_test_generator.h"
#include "../../Headers/Util/sort.h"

///////////////////////////////////////////////////////////////////////////////////
// Constructors
///////////////////////////////////////////////////////////////////////////////////
PCPinMappingTestGenerator::PCPinMappingTestGenerator()
{
	arch = NULL;
}
PCPinMappingTestGenerator::PCPinMappingTestGenerator(DmfbArch *dmfbArch)
{
	arch = dmfbArch;
}

///////////////////////////////////////////////////////////////////////////////////
// Deconstructor
///////////////////////////////////////////////////////////////////////////////////
PCPinMappingTestGenerator::~PCPinMappingTestGenerator()
{

}

///////////////////////////////////////////////////////////////////////////////////
// This function sets each pin according to the pin-constrained generator type.
///////////////////////////////////////////////////////////////////////////////////
void PCPinMappingTestGenerator::setCustomMapping()
{
	if (type == JETC_2D_MESH_TPM)
		JetcMapping();
	else if (type == JETC2_2D_MESH_TPM)
		JetcMapping2();
	else
		claim(false, "Unknown pin-constrained pin-mapping test generator type.");
}

///////////////////////////////////////////////////////////////////////////////////
// This function should be overridden; it is called just before scheduling.
///////////////////////////////////////////////////////////////////////////////////
void PCPinMappingTestGenerator::setMapPreSched()
{
	initPinMapping();
	setCustomMapping();
}

///////////////////////////////////////////////////////////////////////////////////
// This function should be overridden; it is called just after routing.
///////////////////////////////////////////////////////////////////////////////////
void PCPinMappingTestGenerator::setMapPostRoute(vector<vector<int> *> *pinActivations, map<Droplet *, vector<RoutePoint *> *> *routes)
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
void PCPinMappingTestGenerator::flattenSpecialPurposePins()
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
void PCPinMappingTestGenerator::unflattenSpecialPurposePins()
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

void PCPinMappingTestGenerator::JetcMapping()
{
	// For JETC PinMapping
	int pinNo = 0;

	// Hard-coded Original 10x10 Tile
	int tileSize = 10;
	bool tile [10][10] =
	{
			{1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
			{1, 0, 0, 1, 0, 1, 0, 0, 1, 0 },
			{1, 0, 1, 1, 1, 1, 1, 0, 1, 0 },
			{1, 1, 1, 1, 1, 1, 1, 1, 1, 0 },
			{1, 0, 1, 1, 1, 1, 1, 0, 1, 0 },
			{1, 1, 1, 1, 1, 1, 1, 1, 1, 0 },
			{1, 0, 1, 1, 1, 1, 1, 0, 1, 0 },
			{1, 0, 0, 1, 0, 1, 0, 0, 1, 0 },
			{1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
			{1, 0, 0, 0, 0, 0, 0, 0, 1, 0 }
	};

	// Hard-coded Optimized 10x10 Tile
	/*int tileSize = 10;
	bool tile [10][10] =
	{
			{1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
			{1, 0, 0, 1, 0, 1, 0, 0, 1, 0 },
			{1, 0, 0, 1, 0, 1, 0, 0, 1, 0 },
			{1, 1, 1, 1, 1, 1, 1, 1, 1, 0 },
			{1, 0, 0, 1, 1, 1, 0, 0, 1, 0 },
			{1, 1, 1, 1, 1, 1, 1, 1, 1, 0 },
			{1, 0, 0, 1, 0, 1, 0, 0, 1, 0 },
			{1, 0, 0, 1, 0, 1, 0, 0, 1, 0 },
			{1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
			{1, 0, 0, 0, 0, 0, 0, 0, 1, 0 }
	};*/

	int numModsX = arch->getNumCellsX() / tileSize;
	int numModsY = arch->getNumCellsY() / tileSize;

	for (int ty = 0; ty < numModsY; ty++)
	{
		for (int tx = 0; tx < numModsX; tx++)
		{
			int xOff = tx * tileSize;
			int yOff = ty * tileSize;

			for (int y = 0; y < tileSize; y++)
			{
				for (int x = 0; x < tileSize; x++)
				{
					if (tile[x][y])
						pinMapping->at(xOff + x)->at(yOff + y) = pinNo++;
					else
						pinMapping->at(xOff + x)->at(yOff + y) = -1;
				}
			}
		}
	}

	pinNo = setIoPins(pinNo);
}

void PCPinMappingTestGenerator::JetcMapping2()
{
	// For JETC PinMapping
	int pinNo = 0;

	// Hard-coded Tile
	/*int tileSize = 11;
	bool tile [11][11] =
	{
			{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
			{1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0 },
			{1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0 },
			{1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0 },
			{1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0 },
			{1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0 },
			{1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0 },
			{1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0 },
			{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
			{1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0 },
			{1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0 }
	};*/

	/*int tileSize = 13;
	bool tile [13][13] =
	{
			{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
			{1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0 },
			{1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0 },
			{1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0 },
			{1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0 },
			{1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0 },
			{1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0 },
			{1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0 },
			{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
			{1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0 },
			{1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0 },
			{1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0 },
			{1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0 }
	};*/

	int tileSize = 8;
	bool tile [8][8] =
	{
			{1, 1, 1, 1, 1, 1, 1, 1 },
			{1, 0, 1, 0, 1, 0, 1, 0 },
			{1, 1, 1, 1, 1, 1, 1, 0 },
			{1, 0, 1, 0, 1, 0, 1, 0 },
			{1, 1, 1, 1, 1, 1, 1, 0 },
			{1, 0, 1, 0, 1, 0, 1, 0 },
			{1, 1, 1, 1, 1, 1, 1, 1 },
			{1, 0, 0, 0, 0, 0, 1, 0 }
	};

	int numModsX = arch->getNumCellsX() / tileSize;
	int numModsY = arch->getNumCellsY() / tileSize;

	for (int ty = 0; ty < numModsY; ty++)
	{
		for (int tx = 0; tx < numModsX; tx++)
		{
			int xOff = tx * tileSize;
			int yOff = ty * tileSize;

			for (int y = 0; y < tileSize; y++)
			{
				for (int x = 0; x < tileSize; x++)
				{
					if (tile[x][y])
						pinMapping->at(xOff + x)->at(yOff + y) = pinNo++;
					else
						pinMapping->at(xOff + x)->at(yOff + y) = -1;
				}
			}
		}
	}

	pinNo = setIoPins(pinNo);
}

///////////////////////////////////////////////////////////////////////////////////
// Sets the pin numbers for the I/O pins
///////////////////////////////////////////////////////////////////////////////////
int PCPinMappingTestGenerator::setIoPins(int startingPinNo)
{
	int pinNo = startingPinNo;
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

	return pinNo;
}

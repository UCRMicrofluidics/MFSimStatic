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
/*------------------------------Algorithm Details-------------------------------*
 * Type: Pin Mapping Test Generator												*
 * Name:  Pin Constrained Pin Mapping Test Generator							*
 *																				*
 * Details: Computes pin mapping for pin-constrained devices that have been 	*
 * previously described (or are being tested/researched) in published works.	*
 * The primary purpose of this class is to have a unified, clean place to 		*
 * implement prior and new pin-constrained designs so that they can easily be	*
 * passed to the wire router for testing.										*
 *-----------------------------------------------------------------------------*/
#ifndef PIN_CONSTRAINED_PIN_MAPPING_TEST_GENERATOR_H_
#define PIN_CONSTRAINED_PIN_MAPPING_TEST_GENERATOR_H_

#include "../Models/dmfb_arch.h"
#include "../PinMapper/pin_mapper.h"

class PCPinMappingTestGenerator : public PinMapper
{
	public:
		// Constructors
		PCPinMappingTestGenerator();
		PCPinMappingTestGenerator(DmfbArch *dmfbArch);
		virtual ~PCPinMappingTestGenerator();

		// Methods
		void setCustomMapping();
		void setMapPreSched();
		void setMapPostRoute(vector<vector<int> *> *pinActivations);
		void flattenSpecialPurposePins();
		void unflattenSpecialPurposePins();
		void setGeneratorTestType(PinConstrainedPinMapTestGeneratorType t) { type = t; }

		// Getters/Setters

		// Pin-constrained design test cases
		void JetcMapping();
		void JetcMapping2();

	protected:
		// Methods

		int setIoPins(int startingPinNo);
		// Members
		DmfbArch *arch;

	private:
		PinConstrainedPinMapTestGeneratorType type;
};
#endif /* PIN_CONSTRAINED_PIN_MAPPING_TEST_GENERATOR_H_ */

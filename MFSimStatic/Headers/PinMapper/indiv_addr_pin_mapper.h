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
 * Type: Pin Mapper																*
 * Name: Individually-Addressable Pin Mapper									*
 *																				*
 * Detailed in the following paper:												*
 * Authors: Dan Grissom and Philip Brisk										*
 * Title: Field Programmable, Pin-Constrained Digital Microfluidic Biochips		*
 * Publication Details: Submitted to DAC 2013									*
 * 																				*
 * Details: Computes pin mapping for individually-addressable DMFBs.			*
 *-----------------------------------------------------------------------------*/
#ifndef INDIV_ADDR_PIN_MAPPER_H_
#define INDIV_ADDR_PIN_MAPPER_H_

#include "../Models/dmfb_arch.h"
#include "pin_mapper.h"

class IndivAddrPinMapper : public PinMapper
{
	public:
		// Constructors
		IndivAddrPinMapper();
		IndivAddrPinMapper(DmfbArch *dmfbArch);
		virtual ~IndivAddrPinMapper();

		// Methods
		void setCustomMapping();
		void setMapPreSched();
		void setMapPostRoute(vector<vector<int> *> *pinActivations, map<Droplet *, vector<RoutePoint *> *> *routes);
		void flattenSpecialPurposePins();
		void unflattenSpecialPurposePins();

		// Getters/Setters


	protected:
		// Methods
		int computeAvailResources();
		int computeAvailResourcesForGrissomFixedPlacer(int horizRouteSpacing);
		int computeAvailResourcesForGrissomFixedPlacer2();

		// Members
		DmfbArch *arch;

};
#endif /* INDIV_ADDR_PIN_MAPPER_H_ */

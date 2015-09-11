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
 * Name: Field-Programmable Pin-Constrained Pin Mapper							*
 *																				*
 * Detailed in the following paper:												*
 * Authors: Dan Grissom and Philip Brisk										*
 * Title: Field Programmable, Pin-Constrained Digital Microfluidic Biochips		*
 * Publication Details: Submitted to DAC 2013									*
 * 																				*
 * Details: Computes pin mapping for the field-programmable pin-constrained		*
 * DMFB as detailed in the above paper. There are two columns of resources; 	*
 * one with mixers and one with split/store/detect modules						*
 *-----------------------------------------------------------------------------*/
#ifndef GRISSOM_FPPC_PIN_MAPPER_H_
#define GRISSOM_FPPC_PIN_MAPPER_H_

#include "../Models/dmfb_arch.h"
#include "pin_mapper.h"

class GrissomFppcPinMapper : public PinMapper
{
	public:
		// Constructors
		GrissomFppcPinMapper();
		GrissomFppcPinMapper(DmfbArch *dmfbArch);
		virtual ~GrissomFppcPinMapper();

		// Methods
		void setCustomMapping();
		void setEnhancedCustomMapping();
		void setMapPreSched();
		void setMapPostRoute(vector<vector<int> *> *pinActivations, map<Droplet *, vector<RoutePoint *> *> *routes);
		void flattenSpecialPurposePins();
		void unflattenSpecialPurposePins();

		// Getters/Setters
		vector<int> *getMHoldPins() { return mHoldPins; }
		vector<int> *getMIOPins() { return mIOPins; }
		vector<int> *getSsHoldPins() { return ssHoldPins; }
		vector<int> *getSsIOPins() { return ssIOPins; }
		vector<vector<int> *> *getMixPins() { return mixPins; }
		int getRoutingColumn() { return routingColumn; }


	protected:
		// Methods

		// Members
		DmfbArch *arch;
		vector<int> *mHoldPins; // mixer hold pins
		vector<int> *mIOPins; // mixer I/O pins
		vector<int> *ssHoldPins; // split/store hold pins
		vector<int> *ssIOPins; // split/store I/O pins
		vector<vector<int> *> *mixPins; // Mixing pins, starts just after the module hold pin and goes clockwise
		int routingColumn;

	private:
		// Constants
		int getMixWidth() { return 4; }
		int getMixHeight() { return 2; }

};
#endif /* GRISSOM_FPPC_PIN_MAPPER_H_ */

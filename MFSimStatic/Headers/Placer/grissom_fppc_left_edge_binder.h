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
 * Type: Placer																	*
 * Name: Binder for Field-Programmable Pin-Constrained Design					*
 * 																				*
 * Detailed in the following paper:												*
 * Authors: Dan Grissom and Philip Brisk										*
 * Title: Field Programmable, Pin-Constrained Digital Microfluidic Biochips		*
 * Publication Details: Submitted to DAC 2013									*
 * 																				*
 * Details: Modules are placed as specified in GrissomFppcScheduler. Assign		*
 * modules to nodes.															*
 *-----------------------------------------------------------------------------*/
#ifndef GRISSOM_FPPC_PLACER_H
#define GRISSOM_FPPC_PLACER_H

#include "../Resources/enums.h"
#include "placer.h"
#include <vector>

class FixedModule;

class GrissomFppcLEBinder : public Placer
{
	protected:
		// Methods

	public:
		// Constructors
		GrissomFppcLEBinder();
		virtual ~GrissomFppcLEBinder();

		// Methods
		void place(DmfbArch *arch, DAG *schedDag, vector<ReconfigModule *> *rModules);

};
#endif /* GRISSOM_FPPC_PLACER_H */

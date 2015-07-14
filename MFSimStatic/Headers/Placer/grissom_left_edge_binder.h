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
 * Name: Grissom's Left Edge Binder												*
 * 																				*
 * Detailed in the following paper:												*
 * Authors: Dan Grissom and Philip Brisk										*
 * Title: Fast online synthesis of generally programmable digital microfluidic	*
 * 			biochips															*
 * Publication Details: In Proc. ESWEEK (CODES+ISSS), Tampere, Finland, 2012	*
 * 																				*
 * Details: Provided a set of fixed module locations, binds scheduled			*
 * operations to particular location according to the Left Edge binding			*
 * algorithm.																	*
 *-----------------------------------------------------------------------------*/
#ifndef GRISSOM_LEFT_EDGE_BINDER_H_
#define GRISSOM_LEFT_EDGE_BINDER_H_

#include "../Resources/enums.h"
#include "placer.h"
#include <vector>

class FixedModule;

class GrissomLEBinder : public Placer
{
	protected:
		// Methods

	public:
		// Constructors
		GrissomLEBinder();
		virtual ~GrissomLEBinder();

		// Methods
		void place(DmfbArch *arch, DAG *schedDag, vector<ReconfigModule *> *rModules);
};
#endif /* GRISSOM_LEFT_EDGE_BINDER_H_ */

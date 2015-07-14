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
 * Type: Scheduler																*
 * Name: List Scheduler for Field Programmable Pin-Constrained Design			*
 *																				*
 * Detailed in the following paper:												*
 * Authors: Dan Grissom and Philip Brisk										*
 * Title: Field Programmable, Pin-Constrained Digital Microfluidic Biochips		*
 * Publication Details: Submitted to DAC 2013									*
 * 																				*
 * Details: List scheduling algorithm was inferred from Su's JETC journal. This	*
 * implementation is the basic list-scheduling and does not include the			*
 * "modified" part of Su's algorithm.  That is, there is no	rescheduling step.	*
 * This version has a new module type and schedules slightly different in that	*
 * it cannot schedule a split
 *-----------------------------------------------------------------------------*/
#ifndef GRISSOM_FPPC_SCHEDULER_H
#define GRISSOM_FPPC_SCHEDULER_H

#include "scheduler.h"
#include "priority.h"
#include "../Util/util.h"
#include <math.h>

class AssayNode;

class GrissomFppcScheduler : public Scheduler
{
	public:
		// Constructors
		GrissomFppcScheduler();
		virtual ~GrissomFppcScheduler();

		// Methods
		unsigned long long schedule(DmfbArch *arch, DAG *dag);

	private:
		// Methods
		IoResource * getReadyDispenseWell(string fluidName, unsigned long long schedTS);
};
#endif /* GRISSOM_FPPC_SCHEDULER_H */

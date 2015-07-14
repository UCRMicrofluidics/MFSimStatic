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
 * Name: Real-time Mix Evaluation List Scheduler								*
 *																				*
 * Inferred from the following paper:											*
 * Authors: 																	*
 * Title: 																		*
 * Publication Details: 														*
 *																				*
 * Detailed in the following paper:												*
 * Authors: Dan Grissom and Philip Brisk										*
 * Title: 																		*
 * Publication Details: 														*
 * 																				*
 * Details: List scheduling algorithm was inferred from Su's JETC journal. This	*
 * implementation is the basic list-scheduling and does not include the			*
 * "modified" part of Su's algorithm.  That is, there is no	rescheduling step.	*
 * This version is similar to the original list scheduling implementation, but	*
 * treats mixes and dilutes as detectable operations which must use a detect	*
 * module; also, they have some probability to end a little early or late, 		*
 * depending on some probability.
 *-----------------------------------------------------------------------------*/
#ifndef _LIST_SCHEDULER_RT_EVAL_H
#define _LIST_SCHEDULER_RT_EVAL_H

#include "scheduler.h"
#include "priority.h"
#include "../Util/util.h"
#include <math.h>

class AssayNode;

class RealTimeEvalListScheduler : public Scheduler
{
	public:
		// Constructors
		RealTimeEvalListScheduler();
		virtual ~RealTimeEvalListScheduler();

		// Methods
		unsigned long long schedule(DmfbArch *arch, DAG *dag);

	private:
		// Variables

		// Methods
		IoResource * getReadyDispenseWell(string fluidName, unsigned long long schedTS);

};
#endif /* LIST_SCHEDULER_H_ */

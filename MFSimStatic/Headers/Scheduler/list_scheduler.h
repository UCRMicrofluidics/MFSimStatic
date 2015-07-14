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
 * Name: List Scheduler															*
 *																				*
 * Inferred from the following paper:											*
 * Authors: Fei Su and Krishnendu Chakrabarty									*
 * Title: High-Level Synthesis of Digital Microfluidic Biochips					*
 * Publication Details: In JETC, Vol. 3, No. 4, Article 16, Jan 2008			*
 *																				*
 * Detailed in the following paper:												*
 * Authors: Dan Grissom and Philip Brisk										*
 * Title: Fast online synthesis of generally programmable digital microfluidic	*
 * 			biochips															*
 * Publication Details: In Proc. ESWEEK (CODES+ISSS), Tampere, Finland, 2012	*
 * 																				*
 * Details: List scheduling algorithm was inferred from Su's JETC journal. This	*
 * implementation is the basic list-scheduling and does not include the			*
 * "modified" part of Su's algorithm.  That is, there is no	rescheduling step.	*
 * The version presented here is detailed fully in Grissom's CODES+ISSS paper.	*
 *-----------------------------------------------------------------------------*/
#ifndef _LIST_SCHEDULER_H
#define _LIST_SCHEDULER_H

#include "scheduler.h"
#include "priority.h"
#include "../Util/util.h"
#include <math.h>

class AssayNode;

class ListScheduler : public Scheduler
{
	public:
		// Constructors
		ListScheduler();
		virtual ~ListScheduler();

		// Methods
		unsigned long long schedule(DmfbArch *arch, DAG *dag);

	private:
		// Variables

		// Methods
		IoResource * getReadyDispenseWell(string fluidName, unsigned long long schedTS);

};
#endif /* LIST_SCHEDULER_H_ */

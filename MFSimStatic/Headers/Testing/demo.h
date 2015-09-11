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
/*--------------------------------Class Details---------------------------------*
 * Name: Demo (Demo)															*
 *																				*
 * Details: This class contains several functions of popular benchmarks that	*
 * can be run in a hard-coded environment (i.e., from main()). PLEASE NOTE that	*
 * these methods are only meant to act as a DEMO and do not demonstrate all		*
 * capabilities of the system. ALSO, please note that not all combinations of	*
 * schedulers, placers, routers, benchmarks, architecture files, etc. are		*
 * compatible with one another; please keep this in mind as you try your own 	*
 * combination of settings and encounter unknown errors. We make no guarantee	*
 * of every combination and remind you that this is not a commercial product,	*
 * but a free, opnen-source research tool.
 *-----------------------------------------------------------------------------*/

#ifndef _DEMO_H
#define _DEMO_H

#include "../../Headers/Resources/enums.h"
using namespace std;
#include <iostream>
#include <stdlib.h>
#include <sstream>

class Demo
{
    public:
        // Constructors
		Demo();
        virtual ~Demo();

        // General
        static void FieldProgrammablePinConstrainedDMFB(bool isParallel, bool performWash, bool useFPPCWireRouter, CommonBenchmarkType benchmark);
        static void ProgrammableDMFB(bool isParallel, bool useVirtualTopology, PinMapType pmType, bool performWireRouting, CommonBenchmarkType benchmark);
        static void IndividuallyAddressableDMFB(bool isParallel, bool useVirtualTopology, bool performWireRouting, CommonBenchmarkType benchmark);
        static void CliquePartitionedDMFB(bool isParallel, bool useVirtualTopology, bool performWireRouting, CommonBenchmarkType benchmark);
        static void WireRoutingCase(int orthogonalCapacity, int testNum);
        static void PinMapWireRouteFlow(PinMapType pmt, WireRouteType wrt, int oCap, int testNum);
        static void CopyArchFile(string fileName);
        static void MySandboxCode();
};

#endif /* _DEMO_H */

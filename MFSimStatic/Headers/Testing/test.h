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
 * Name: Test (Test)															*
 *																				*
 * Details: This class contains several functions of popular benchmarks that	*
 * were created by hand (manually programmed by creating nodes and edges). 		*
 * The resultant DAGs can be used for testing and the timings can be modified 	*
 * by changing the source code for a particular assay function.					*
 *-----------------------------------------------------------------------------*/
#ifndef _TEST_H
#define _TEST_H

using namespace std;
#include "../../Headers/Models/assay_node.h"
#include "../../Headers/Resources/enums.h"
#include "../../Headers/Models/dag.h"
#include <stdlib.h>
#include <stdio.h>
#include<list>
#include <map>
#include <set>

class Test
{
    public:
        // Constructors
		Test();
        virtual ~Test ();

        // Benchmarks from http://people.ee.duke.edu/~fs/Benchmark.pdf
        static DAG *Create_B1_PCRMix(double mult, int repeat);
        static DAG *Create_B2_InVitroDiag_1(double mult, int repeat);
        static DAG *Create_B2_InVitroDiag_2(double mult, int repeat);
        static DAG *Create_B2_InVitroDiag_3(double mult, int repeat);
        static DAG *Create_B2_InVitroDiag_4(double mult, int repeat);
        static DAG *Create_B2_InVitroDiag_5(double mult, int repeat);
        static DAG *Create_B3_Protein(double mult, int repeat);
        static void JETC2014_Tests();
        static void MinCostMaxFlow();

        // DAG2014 Functions
        static void DAC2014RecoveryDagGenerator(string inputDagFile, string inputArchFile, string outputFile, SchedulerType st, ResourceAllocationType rat, PinMapType pmt, int maxStorageDropsPerMod, int numFaults);
        static DAG *ScheduleAndReturnDag(string inputDagFile, string inputArchFile, string outputFile, SchedulerType st, ResourceAllocationType rat, PinMapType pmt, int maxStorageDropsPerMod);
        static void RecursiveAncestorFind(set<string> *ancestors, AssayNode *n);
        static bool IsInSet(AssayNode *n, set<AssayNode*> *nodeSet) { return nodeSet->find(n) != nodeSet->end();};
        static bool IsInSet(string name, set<string> *nodeSet) { return nodeSet->find(name) != nodeSet->end();};

        // Wire Routing Tests
        static void WireRouteIndivAddr(int numCellsX, int numCellsY, WireRouteType wrt, int numHorizTracks, int numVertTracks);
        static void WireRouteDacOrigFppc(int numCellsX, int numCellsY, WireRouteType wrt, int numHorizTracks, int numVertTracks);
        static void WireRouteTcadEnhancedFppc(bool isPinOptimized, int numCellsX, int numCellsY, WireRouteType wrt, int numHorizTracks, int numVertTracks);
        static void WireRouteJetcVirtTop(int numModsX, int numModsY, WireRouteType wrt, int numHorizTracks, int numVertTracks);
        static void WireRouteJetc2VirtTop(int numModsX, int numModsY, WireRouteType wrt, int numHorizTracks, int numVertTracks);
};
#endif

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
/*---------------------------Implementation Details-----------------------------*
 * Source: test.cc																*
 * Original Code Author(s): Dan Grissom											*
 * Original Completion/Release Date: October 7, 2012							*
 *																				*
 * Details: N/A																	*
 *																				*
 * Revision History:															*
 * WHO		WHEN		WHAT													*
 * ---		----		----													*
 * FML		MM/DD/YY	One-line description									*
 *-----------------------------------------------------------------------------*/
#include "../../Headers/Testing/test.h"
#include "../../Headers/Models/min_cost_max_flow_graph.h"
#include "../../Headers/Util/sort.h"
#include "../../Headers/Util/file_in.h"
#include "../../Headers/Util/file_out.h"
#include <fstream>
#include <sys/time.h>


#include "../../Headers/Scheduler/force_directed_list_scheduler.h"
#include "../../Headers/Scheduler/list_scheduler_rt_eval.h"
#include "../../Headers/Scheduler/grissom_fppc_scheduler.h"
#include "../../Headers/Scheduler/rickett_scheduler.h"
#include "../../Headers/Scheduler/genet_scheduler.h"
#include "../../Headers/Scheduler/list_scheduler.h"
#include "../../Headers/Scheduler/path_scheduler.h"
#include "../../Headers/Scheduler/scheduler.h"
#include "../../Headers/PinMapper/indiv_addr_pin_mapper.h"
#include "../../Headers/PinMapper/grissom_fppc_pin_mapper.h"
#include "../../Headers/Testing/pin_constrained_pin_mapping_test_generator.h"

///////////////////////////////////////////////////////////////////////////
// This test performs wire-routing on an individually addressable DMFB
// given several parameters.
///////////////////////////////////////////////////////////////////////////
void Test::WireRouteIndivAddr(int numCellsX, int numCellsY, WireRouteType wrt, int numHorizTracks, int numVertTracks)
{
	DmfbArch *a = new DmfbArch();
	a->numCellsX = numCellsX;
	a->numCellsY = numCellsY;
	PinMapper *pm = new IndivAddrPinMapper(a);
	pm->setType(INDIVADDR_PM);
	pm->setArch(a);
	pm->setResAllocType(INHERIT_RA);
	a->setPinMapper(pm);
	pm->setMapPreSched();

	stringstream fileName("");
	fileName << "Output/IndivArch_" << a->numCellsX << "x" << a->numCellsY << ".txt";

	FileOut::WriteArchForWireRouting(a, fileName.str());
	Synthesis::WireRoute(fileName.str(), wrt, numHorizTracks, numVertTracks);
}

///////////////////////////////////////////////////////////////////////////
// This test performs wire-routing on the JETC 2014/GLSVI 2012 DMFB virtual
// topology (10x10 module-tiles which mimic a 2D-mesh network and can be
// tiled) given several parameters.
///////////////////////////////////////////////////////////////////////////
void Test::WireRouteJetcVirtTop(int numModsX, int numModsY, WireRouteType wrt, int numHorizTracks, int numVertTracks)
{
	int modDim = 10;
	DmfbArch *a = new DmfbArch();
	a->numCellsX = numModsX * modDim;
	a->numCellsY = numModsY * modDim;
	PCPinMappingTestGenerator *pm = new PCPinMappingTestGenerator(a);
	pm->setType(INDIVADDR_PM);
	pm->setGeneratorTestType(JETC_2D_MESH_TPM);
	pm->setArch(a);
	pm->setResAllocType(INHERIT_RA);
	a->setPinMapper(pm);
	pm->setMapPreSched();

	stringstream fileName("");
	fileName << "Output/JetcArch_" << a->numCellsX << "x" << a->numCellsY << ".txt";

	FileOut::WriteArchForWireRouting(a, fileName.str());
	Synthesis::WireRoute(fileName.str(), wrt, numHorizTracks, numVertTracks);
}
///////////////////////////////////////////////////////////////////////////
// This test performs wire-routing on the JETC 2014/GLSVI 2012 DMFB virtual
// topology (11x11 module-tiles which mimic a 2D-mesh network and can be
// tiled) given several parameters.
///////////////////////////////////////////////////////////////////////////
void Test::WireRouteJetc2VirtTop(int numModsX, int numModsY, WireRouteType wrt, int numHorizTracks, int numVertTracks)
{
	int modDim = 8;
	DmfbArch *a = new DmfbArch();
	a->numCellsX = numModsX * modDim;
	a->numCellsY = numModsY * modDim;
	PCPinMappingTestGenerator *pm = new PCPinMappingTestGenerator(a);
	pm->setType(INDIVADDR_PM);
	pm->setGeneratorTestType(JETC2_2D_MESH_TPM);
	pm->setArch(a);
	pm->setResAllocType(INHERIT_RA);
	a->setPinMapper(pm);
	pm->setMapPreSched();

	stringstream fileName("");
	fileName << "Output/Jetc2Arch_" << a->numCellsX << "x" << a->numCellsY << ".txt";

	FileOut::WriteArchForWireRouting(a, fileName.str());
	Synthesis::WireRoute(fileName.str(), wrt, numHorizTracks, numVertTracks);
}

///////////////////////////////////////////////////////////////////////////
// This test performs wire-routing on the original FPPC topology (described
// in DAC 2013), given several parameters.
///////////////////////////////////////////////////////////////////////////
void Test::WireRouteDacOrigFppc(int numCellsX, int numCellsY, WireRouteType wrt, int numHorizTracks, int numVertTracks)
{
	DmfbArch *a = new DmfbArch();
	a->numCellsX = numCellsX;
	a->numCellsY = numCellsY;
	PinMapper *pm = new GrissomFppcPinMapper(a);
	pm->setType(ORIGINAL_FPPC_PM);
	pm->setArch(a);
	pm->setResAllocType(PC_INHERENT_RA);
	a->setPinMapper(pm);
	pm->setMapPreSched();

	stringstream fileName("");
	fileName << "Output/DacOriginalFppc_" << a->numCellsX << "x" << a->numCellsY << ".txt";

	FileOut::WriteArchForWireRouting(a, fileName.str());
	Synthesis::WireRoute(fileName.str(), wrt, numHorizTracks, numVertTracks);
}

///////////////////////////////////////////////////////////////////////////
// This test performs wire-routing on the enhanced FPPC topology (described
// in TCAD 2014), given several parameters.
///////////////////////////////////////////////////////////////////////////
void Test::WireRouteTcadEnhancedFppc(bool isPinOptimized, int numCellsX, int numCellsY, WireRouteType wrt, int numHorizTracks, int numVertTracks)
{
	DmfbArch *a = new DmfbArch();
	a->numCellsX = numCellsX;
	a->numCellsY = numCellsY;
	PinMapper *pm = new GrissomFppcPinMapper(a);
	if (isPinOptimized)
		pm->setType(ENHANCED_FPPC_PIN_OPT_PM);
	else
		pm->setType(ENHANCED_FPPC_ROUTE_OPT_PM);
	pm->setArch(a);
	pm->setResAllocType(PC_INHERENT_RA);
	a->setPinMapper(pm);
	pm->setMapPreSched();

	stringstream fileName("");
	fileName << "Output/DacEnhancedFppc_" << a->numCellsX << "x" << a->numCellsY << ".txt";

	FileOut::WriteArchForWireRouting(a, fileName.str());
	Synthesis::WireRoute(fileName.str(), wrt, numHorizTracks, numVertTracks);
}

//struct MinCostMaxFlow;

void Test::MinCostMaxFlow()
{
	// Create graph with nodes
	int numNodes = 7;
	MinCostMaxFlowGraph mcmf(numNodes);

	//mcmf.Init();

	// Add edges
	mcmf.AddEdge(0, 1, 1, 0);
	mcmf.AddEdge(1, 2, 1, 1);
	mcmf.AddEdge(2, 4, 4, 1);
	mcmf.AddEdge(0, 3, 2, 0);
	mcmf.AddEdge(3, 4, 3, 1);

	mcmf.AddEdge(0, 5, 2, 0);
	mcmf.AddEdge(5, 6, 2, 0);
	mcmf.AddEdge(3, 6, 3, 0);
	mcmf.AddEdge(6, 4, 10, 0);

	mcmf.OutputGraphFile("Output/MCMF_Pre");
	// Perform MCMF computation
	pair<long long, long long> flowCost = mcmf.GetMaxFlow(0, 4);

	mcmf.OutputGraphFile("Output/MCMF_Post");
	// Print results
	mcmf.PrintResults();

}

///////////////////////////////////////////////////////////////////////////
// This benchmark details the DAG for the PCR mixing stage detailed
// in Chakrabarty's benchmarks.
///////////////////////////////////////////////////////////////////////////
DAG *Test::Create_B1_PCRMix(double mult, int repeat)
{
	DAG *dag = new DAG();
	dag->setName("PCR");

	for (int i=0; i < repeat; i++)
	{
		AssayNode *D1 = dag->AddDispenseNode("tris-hcl", 10, "D_tris-hcl");
		AssayNode *D2 = dag->AddDispenseNode("kcl", 10, "D_kcl");
		AssayNode *D3 = dag->AddDispenseNode("bovine", 10, "D_bovine");
		AssayNode *D4 = dag->AddDispenseNode("gelatin", 10, "D_gelatin");
		AssayNode *D5 = dag->AddDispenseNode("primer", 10, "D_primer");
		AssayNode *D6 = dag->AddDispenseNode("beosynucleotide", 10, "D_beosynucleotide");
		AssayNode *D7 = dag->AddDispenseNode("amplitag", 10, "D_amplitag");
		AssayNode *D8 = dag->AddDispenseNode("lambda", 10, "D_lambda");

		//AssayNode *M1 = dag->AddMixNode(2, 3*mult, "M1");
		AssayNode *M1 = dag->AddMixNode(2, 10*mult, "M1");
		AssayNode *M2 = dag->AddMixNode(2, 5*mult, "M2");
		AssayNode *M3 = dag->AddMixNode(2, 6*mult, "M3");
		AssayNode *M4 = dag->AddMixNode(2, 5*mult, "M4");
		AssayNode *M5 = dag->AddMixNode(2, 5*mult, "M5");
		AssayNode *M6 = dag->AddMixNode(2, 10*mult, "M6");
		AssayNode *M7 = dag->AddMixNode(2, 3*mult, "M7");

		AssayNode *O1 = dag->AddOutputNode("output", "O_output");

		dag->ParentChild(D1, M1);
		dag->ParentChild(D2, M1);
		dag->ParentChild(D3, M2);
		dag->ParentChild(D4, M2);
		dag->ParentChild(D5, M4);
		dag->ParentChild(D6, M4);
		dag->ParentChild(D7, M5);
		dag->ParentChild(D8, M5);

		dag->ParentChild(M1, M3);
		dag->ParentChild(M2, M3);
		dag->ParentChild(M4, M6);
		dag->ParentChild(M5, M6);
		dag->ParentChild(M3, M7);
		dag->ParentChild(M6, M7);

		dag->ParentChild(M7, O1);
	}

    cout << "Bench1_PCRMix CREATED" << endl;
    return dag;

}

///////////////////////////////////////////////////////////////////////////
// This benchmark details the DAG for the multiplexed in-vitro diagnostics
// assay detailed in Chakrabarty's benchmarks.  Plasma/Serum are assayed
// for glucose/lactate measurements
///////////////////////////////////////////////////////////////////////////
DAG *Test::Create_B2_InVitroDiag_1(double mult, int repeat)
{
	DAG *dag = new DAG();
	dag->setName("In_Vitro_1");

	for (int i=0; i < repeat; i++)
	{
		AssayNode *D1 = dag->AddDispenseNode("S1", 10, "S1_1");
		AssayNode *D2 = dag->AddDispenseNode("S1", 10, "S1_2");
		AssayNode *D3 = dag->AddDispenseNode("S2", 10, "S2_1");
		AssayNode *D4 = dag->AddDispenseNode("S2", 10, "S2_2");
		AssayNode *D5 = dag->AddDispenseNode("R1", 10, "R1_1");
		AssayNode *D6 = dag->AddDispenseNode("R2", 10, "R2_1");
		AssayNode *D7 = dag->AddDispenseNode("R1", 10, "R1_2");
		AssayNode *D8 = dag->AddDispenseNode("R2", 10, "R2_2");

		AssayNode *M1 = dag->AddMixNode(2, 5*mult, "M1");
		AssayNode *M2 = dag->AddMixNode(2, 5*mult, "M2");
		AssayNode *M3 = dag->AddMixNode(2, 3*mult, "M3");
		AssayNode *M4 = dag->AddMixNode(2, 3*mult, "M4");

		AssayNode *d1 = dag->AddDetectNode(1, 5*mult, "dt1");
		AssayNode *d2 = dag->AddDetectNode(1, 4*mult, "dt2");
		AssayNode *d3 = dag->AddDetectNode(1, 5*mult, "dt3");
		AssayNode *d4 = dag->AddDetectNode(1, 4*mult, "dt4");

		AssayNode *O1 = dag->AddOutputNode("output", "O1");
		AssayNode *O2 = dag->AddOutputNode("output", "O2");
		AssayNode *O3 = dag->AddOutputNode("output", "O3");
		AssayNode *O4 = dag->AddOutputNode("output", "O4");

		dag->ParentChild(D1, M1);
		dag->ParentChild(D2, M2);
		dag->ParentChild(D3, M3);
		dag->ParentChild(D4, M4);
		dag->ParentChild(D5, M1);
		dag->ParentChild(D6, M2);
		dag->ParentChild(D7, M3);
		dag->ParentChild(D8, M4);


		dag->ParentChild(M1, d1);
		dag->ParentChild(M2, d2);
		dag->ParentChild(M3, d3);
		dag->ParentChild(M4, d4);

		dag->ParentChild(d1, O1);
		dag->ParentChild(d2, O2);
		dag->ParentChild(d3, O3);
		dag->ParentChild(d4, O4);
	}

	cout << "Bench2_InVitroDiag_1 CREATED" << endl;
	return dag;
}

///////////////////////////////////////////////////////////////////////////
// This benchmark details the DAG for the multiplexed in-vitro diagnostics
// assay detailed in Chakrabarty's benchmarks.  Plasma/Serum are assayed
// for glucose/lactate/pyruvate measurements
///////////////////////////////////////////////////////////////////////////
DAG *Test::Create_B2_InVitroDiag_2(double mult, int repeat)
{
	DAG *dag = new DAG();
	dag->setName("InVitro_2");

	for (int i=0; i < repeat; i++)
	{
		AssayNode *D1 = dag->AddDispenseNode("S1", 10);
		AssayNode *D2 = dag->AddDispenseNode("S1", 10);
		AssayNode *D3 = dag->AddDispenseNode("S1", 10);
		AssayNode *D4 = dag->AddDispenseNode("S2", 10);
		AssayNode *D5 = dag->AddDispenseNode("S2", 10);
		AssayNode *D6 = dag->AddDispenseNode("S2", 10);
		AssayNode *D7 = dag->AddDispenseNode("R1", 10);
		AssayNode *D8 = dag->AddDispenseNode("R2", 10);
		AssayNode *D9 = dag->AddDispenseNode("R3", 10);
		AssayNode *D10 = dag->AddDispenseNode("R1", 10);
		AssayNode *D11 = dag->AddDispenseNode("R2", 10);
		AssayNode *D12 = dag->AddDispenseNode("R3", 10);

		AssayNode *M1 = dag->AddMixNode(2, 5*mult, "M1");
		AssayNode *M2 = dag->AddMixNode(2, 5*mult, "M2");
		AssayNode *M3 = dag->AddMixNode(2, 5*mult, "M3");
		AssayNode *M4 = dag->AddMixNode(2, 3*mult, "M4");
		AssayNode *M5 = dag->AddMixNode(2, 3*mult, "M5");
		AssayNode *M6 = dag->AddMixNode(2, 3*mult, "M6");

		AssayNode *d1 = dag->AddDetectNode(1, 5*mult, "dt1");
		AssayNode *d2 = dag->AddDetectNode(1, 4*mult, "dt2");
		AssayNode *d3 = dag->AddDetectNode(1, 6*mult, "dt3");
		AssayNode *d4 = dag->AddDetectNode(1, 5*mult, "dt4");
		AssayNode *d5 = dag->AddDetectNode(1, 4*mult, "dt5");
		AssayNode *d6 = dag->AddDetectNode(1, 6*mult, "dt6");

		AssayNode *O1 = dag->AddOutputNode("output");
		AssayNode *O2 = dag->AddOutputNode("output");
		AssayNode *O3 = dag->AddOutputNode("output");
		AssayNode *O4 = dag->AddOutputNode("output");
		AssayNode *O5 = dag->AddOutputNode("output");
		AssayNode *O6 = dag->AddOutputNode("output");

		dag->ParentChild(D1, M1);
		dag->ParentChild(D2, M2);
		dag->ParentChild(D3, M3);
		dag->ParentChild(D4, M4);
		dag->ParentChild(D5, M5);
		dag->ParentChild(D6, M6);
		dag->ParentChild(D7, M1);
		dag->ParentChild(D8, M2);
		dag->ParentChild(D9, M3);
		dag->ParentChild(D10, M4);
		dag->ParentChild(D11, M5);
		dag->ParentChild(D12, M6);

		dag->ParentChild(M1, d1);
		dag->ParentChild(M2, d2);
		dag->ParentChild(M3, d3);
		dag->ParentChild(M4, d4);
		dag->ParentChild(M5, d5);
		dag->ParentChild(M6, d6);

		dag->ParentChild(d1, O1);
		dag->ParentChild(d2, O2);
		dag->ParentChild(d3, O3);
		dag->ParentChild(d4, O4);
		dag->ParentChild(d5, O5);
		dag->ParentChild(d6, O6);
	}

	cout << "Bench2_InVitroDiag_2 CREATED" << endl;
	return dag;
}

///////////////////////////////////////////////////////////////////////////
// This benchmark details the DAG for the multiplexed in-vitro diagnostics
// assay detailed in Chakrabarty's benchmarks.  Plasma/Serum/Saliva are
// assayed for glucose/lactate/pyruvate measurements
///////////////////////////////////////////////////////////////////////////
DAG *Test::Create_B2_InVitroDiag_3(double mult, int repeat)
{
	DAG *dag = new DAG();
	dag->setName("InVitro_3");

	for (int i=0; i < repeat; i++)
	{
		AssayNode *D1 = dag->AddDispenseNode("S1", 10);
		AssayNode *D2 = dag->AddDispenseNode("S1", 10);
		AssayNode *D3 = dag->AddDispenseNode("S1", 10);
		AssayNode *D4 = dag->AddDispenseNode("S2", 10);
		AssayNode *D5 = dag->AddDispenseNode("S2", 10);
		AssayNode *D6 = dag->AddDispenseNode("S2", 10);
		AssayNode *D7 = dag->AddDispenseNode("S3", 10);
		AssayNode *D8 = dag->AddDispenseNode("S3", 10);
		AssayNode *D9 = dag->AddDispenseNode("S3", 10);
		AssayNode *D10 = dag->AddDispenseNode("R1", 10);
		AssayNode *D11 = dag->AddDispenseNode("R2", 10);
		AssayNode *D12 = dag->AddDispenseNode("R3", 10);
		AssayNode *D13 = dag->AddDispenseNode("R1", 10);
		AssayNode *D14 = dag->AddDispenseNode("R2", 10);
		AssayNode *D15 = dag->AddDispenseNode("R3", 10);
		AssayNode *D16 = dag->AddDispenseNode("R1", 10);
		AssayNode *D17 = dag->AddDispenseNode("R2", 10);
		AssayNode *D18 = dag->AddDispenseNode("R3", 10);

		AssayNode *M1 = dag->AddMixNode(2, 5*mult, "M1");
		AssayNode *M2 = dag->AddMixNode(2, 5*mult, "M2");
		AssayNode *M3 = dag->AddMixNode(2, 5*mult, "M3");
		AssayNode *M4 = dag->AddMixNode(2, 3*mult, "M4");
		AssayNode *M5 = dag->AddMixNode(2, 3*mult, "M5");
		AssayNode *M6 = dag->AddMixNode(2, 3*mult, "M6");
		AssayNode *M7 = dag->AddMixNode(2, 4*mult, "M7");
		AssayNode *M8 = dag->AddMixNode(2, 4*mult, "M8");
		AssayNode *M9 = dag->AddMixNode(2, 4*mult, "M9");

		AssayNode *d1 = dag->AddDetectNode(1, 5*mult, "dt1");
		AssayNode *d2 = dag->AddDetectNode(1, 4*mult, "dt2");
		AssayNode *d3 = dag->AddDetectNode(1, 6*mult, "dt3");
		AssayNode *d4 = dag->AddDetectNode(1, 5*mult, "dt4");
		AssayNode *d5 = dag->AddDetectNode(1, 4*mult, "dt5");
		AssayNode *d6 = dag->AddDetectNode(1, 6*mult, "dt6");
		AssayNode *d7 = dag->AddDetectNode(1, 5*mult, "dt7");
		AssayNode *d8 = dag->AddDetectNode(1, 4*mult, "dt8");
		AssayNode *d9 = dag->AddDetectNode(1, 6*mult, "dt9");

		AssayNode *O1 = dag->AddOutputNode("output");
		AssayNode *O2 = dag->AddOutputNode("output");
		AssayNode *O3 = dag->AddOutputNode("output");
		AssayNode *O4 = dag->AddOutputNode("output");
		AssayNode *O5 = dag->AddOutputNode("output");
		AssayNode *O6 = dag->AddOutputNode("output");
		AssayNode *O7 = dag->AddOutputNode("output");
		AssayNode *O8 = dag->AddOutputNode("output");
		AssayNode *O9 = dag->AddOutputNode("output");

		dag->ParentChild(D1, M1);
		dag->ParentChild(D2, M2);
		dag->ParentChild(D3, M3);
		dag->ParentChild(D4, M4);
		dag->ParentChild(D5, M5);
		dag->ParentChild(D6, M6);
		dag->ParentChild(D7, M7);
		dag->ParentChild(D8, M8);
		dag->ParentChild(D9, M9);
		dag->ParentChild(D10, M1);
		dag->ParentChild(D11, M2);
		dag->ParentChild(D12, M3);
		dag->ParentChild(D13, M4);
		dag->ParentChild(D14, M5);
		dag->ParentChild(D15, M6);
		dag->ParentChild(D16, M7);
		dag->ParentChild(D17, M8);
		dag->ParentChild(D18, M9);


		dag->ParentChild(M1, d1);
		dag->ParentChild(M2, d2);
		dag->ParentChild(M3, d3);
		dag->ParentChild(M4, d4);
		dag->ParentChild(M5, d5);
		dag->ParentChild(M6, d6);
		dag->ParentChild(M7, d7);
		dag->ParentChild(M8, d8);
		dag->ParentChild(M9, d9);

		dag->ParentChild(d1, O1);
		dag->ParentChild(d2, O2);
		dag->ParentChild(d3, O3);
		dag->ParentChild(d4, O4);
		dag->ParentChild(d5, O5);
		dag->ParentChild(d6, O6);
		dag->ParentChild(d7, O7);
		dag->ParentChild(d8, O8);
		dag->ParentChild(d9, O9);
	}

	cout << "Bench2_InVitroDiag_3 CREATED" << endl;
	return dag;
}

///////////////////////////////////////////////////////////////////////////
// This benchmark details the DAG for the multiplexed in-vitro diagnostics
// assay detailed in Chakrabarty's benchmarks.  Plasma/Serum/Saliva are
// assayed for glucose/lactate/pyruvate/glutamate measurements
///////////////////////////////////////////////////////////////////////////
DAG *Test::Create_B2_InVitroDiag_4(double mult, int repeat)
{
	DAG *dag = new DAG();
	dag->setName("InVitro_4");

	for (int i=0; i < repeat; i++)
	{
		AssayNode *D1 = dag->AddDispenseNode("S1", 10);
		AssayNode *D2 = dag->AddDispenseNode("S1", 10);
		AssayNode *D3 = dag->AddDispenseNode("S1", 10);
		AssayNode *D4 = dag->AddDispenseNode("S1", 10);
		AssayNode *D5 = dag->AddDispenseNode("S2", 10);
		AssayNode *D6 = dag->AddDispenseNode("S2", 10);
		AssayNode *D7 = dag->AddDispenseNode("S2", 10);
		AssayNode *D8 = dag->AddDispenseNode("S2", 10);
		AssayNode *D9 = dag->AddDispenseNode("S3", 10);
		AssayNode *D10 = dag->AddDispenseNode("S3", 10);
		AssayNode *D11 = dag->AddDispenseNode("S3", 10);
		AssayNode *D12 = dag->AddDispenseNode("S3", 10);
		AssayNode *D13 = dag->AddDispenseNode("R1", 10);
		AssayNode *D14 = dag->AddDispenseNode("R2", 10);
		AssayNode *D15 = dag->AddDispenseNode("R3", 10);
		AssayNode *D16 = dag->AddDispenseNode("R4", 10);
		AssayNode *D17 = dag->AddDispenseNode("R1", 10);
		AssayNode *D18 = dag->AddDispenseNode("R2", 10);
		AssayNode *D19 = dag->AddDispenseNode("R3", 10);
		AssayNode *D20 = dag->AddDispenseNode("R4", 10);
		AssayNode *D21 = dag->AddDispenseNode("R1", 10);
		AssayNode *D22 = dag->AddDispenseNode("R2", 10);
		AssayNode *D23 = dag->AddDispenseNode("R3", 10);
		AssayNode *D24 = dag->AddDispenseNode("R4", 10);

		AssayNode *M1 = dag->AddMixNode(2, 5*mult, "M1");
		AssayNode *M2 = dag->AddMixNode(2, 5*mult, "M2");
		AssayNode *M3 = dag->AddMixNode(2, 5*mult, "M3");
		AssayNode *M4 = dag->AddMixNode(2, 5*mult, "M4");
		AssayNode *M5 = dag->AddMixNode(2, 3*mult, "M5");
		AssayNode *M6 = dag->AddMixNode(2, 3*mult, "M6");
		AssayNode *M7 = dag->AddMixNode(2, 3*mult, "M7");
		AssayNode *M8 = dag->AddMixNode(2, 3*mult, "M8");
		AssayNode *M9 = dag->AddMixNode(2, 4*mult, "M9");
		AssayNode *M10 = dag->AddMixNode(2, 4*mult, "M10");
		AssayNode *M11 = dag->AddMixNode(2, 4*mult, "M11");
		AssayNode *M12 = dag->AddMixNode(2, 4*mult, "M12");

		AssayNode *d1 = dag->AddDetectNode(1, 5*mult, "dt1");
		AssayNode *d2 = dag->AddDetectNode(1, 4*mult, "dt2");
		AssayNode *d3 = dag->AddDetectNode(1, 6*mult, "dt3");
		AssayNode *d4 = dag->AddDetectNode(1, 5*mult, "dt4");
		AssayNode *d5 = dag->AddDetectNode(1, 5*mult, "dt5");
		AssayNode *d6 = dag->AddDetectNode(1, 4*mult, "dt6");
		AssayNode *d7 = dag->AddDetectNode(1, 6*mult, "dt7");
		AssayNode *d8 = dag->AddDetectNode(1, 5*mult, "dt8");
		AssayNode *d9 = dag->AddDetectNode(1, 5*mult, "dt9");
		AssayNode *d10 = dag->AddDetectNode(1, 4*mult, "dt10");
		AssayNode *d11 = dag->AddDetectNode(1, 6*mult, "dt11");
		AssayNode *d12 = dag->AddDetectNode(1, 5*mult, "dt12");

		AssayNode *O1 = dag->AddOutputNode("output");
		AssayNode *O2 = dag->AddOutputNode("output");
		AssayNode *O3 = dag->AddOutputNode("output");
		AssayNode *O4 = dag->AddOutputNode("output");
		AssayNode *O5 = dag->AddOutputNode("output");
		AssayNode *O6 = dag->AddOutputNode("output");
		AssayNode *O7 = dag->AddOutputNode("output");
		AssayNode *O8 = dag->AddOutputNode("output");
		AssayNode *O9 = dag->AddOutputNode("output");
		AssayNode *O10 = dag->AddOutputNode("output");
		AssayNode *O11 = dag->AddOutputNode("output");
		AssayNode *O12 = dag->AddOutputNode("output");

		dag->ParentChild(D1, M1);
		dag->ParentChild(D2, M2);
		dag->ParentChild(D3, M3);
		dag->ParentChild(D4, M4);
		dag->ParentChild(D5, M5);
		dag->ParentChild(D6, M6);
		dag->ParentChild(D7, M7);
		dag->ParentChild(D8, M8);
		dag->ParentChild(D9, M9);
		dag->ParentChild(D10, M10);
		dag->ParentChild(D11, M11);
		dag->ParentChild(D12, M12);
		dag->ParentChild(D13, M1);
		dag->ParentChild(D14, M2);
		dag->ParentChild(D15, M3);
		dag->ParentChild(D16, M4);
		dag->ParentChild(D17, M5);
		dag->ParentChild(D18, M6);
		dag->ParentChild(D19, M7);
		dag->ParentChild(D20, M8);
		dag->ParentChild(D21, M9);
		dag->ParentChild(D22, M10);
		dag->ParentChild(D23, M11);
		dag->ParentChild(D24, M12);


		dag->ParentChild(M1, d1);
		dag->ParentChild(M2, d2);
		dag->ParentChild(M3, d3);
		dag->ParentChild(M4, d4);
		dag->ParentChild(M5, d5);
		dag->ParentChild(M6, d6);
		dag->ParentChild(M7, d7);
		dag->ParentChild(M8, d8);
		dag->ParentChild(M9, d9);
		dag->ParentChild(M10, d10);
		dag->ParentChild(M11, d11);
		dag->ParentChild(M12, d12);

		dag->ParentChild(d1, O1);
		dag->ParentChild(d2, O2);
		dag->ParentChild(d3, O3);
		dag->ParentChild(d4, O4);
		dag->ParentChild(d5, O5);
		dag->ParentChild(d6, O6);
		dag->ParentChild(d7, O7);
		dag->ParentChild(d8, O8);
		dag->ParentChild(d9, O9);
		dag->ParentChild(d10, O10);
		dag->ParentChild(d11, O11);
		dag->ParentChild(d12, O12);
	}

	cout << "Bench2_InVitroDiag_4 CREATED" << endl;
	return dag;
}

///////////////////////////////////////////////////////////////////////////
// This benchmark details the DAG for the multiplexed in-vitro diagnostics
// assay detailed in Chakrabarty's benchmarks.  Plasma/Serum/Saliva/Urine
// are assayed for glucose/lactate/pyruvate/glutamate measurements
///////////////////////////////////////////////////////////////////////////
DAG *Test::Create_B2_InVitroDiag_5(double mult, int repeat)
{
	DAG *dag = new DAG();
	dag->setName("InVitro_5");

	for (int i=0; i < repeat; i++)
	{
		AssayNode *D1 = dag->AddDispenseNode("S1", 10);
		AssayNode *D2 = dag->AddDispenseNode("S1", 10);
		AssayNode *D3 = dag->AddDispenseNode("S1", 10);
		AssayNode *D4 = dag->AddDispenseNode("S1", 10);
		AssayNode *D5 = dag->AddDispenseNode("S2", 10);
		AssayNode *D6 = dag->AddDispenseNode("S2", 10);
		AssayNode *D7 = dag->AddDispenseNode("S2", 10);
		AssayNode *D8 = dag->AddDispenseNode("S2", 10);
		AssayNode *D9 = dag->AddDispenseNode("S3", 10);
		AssayNode *D10 = dag->AddDispenseNode("S3", 10);
		AssayNode *D11 = dag->AddDispenseNode("S3", 10);
		AssayNode *D12 = dag->AddDispenseNode("S3", 10);
		AssayNode *D13 = dag->AddDispenseNode("S4", 10);
		AssayNode *D14 = dag->AddDispenseNode("S4", 10);
		AssayNode *D15 = dag->AddDispenseNode("S4", 10);
		AssayNode *D16 = dag->AddDispenseNode("S4", 10);
		AssayNode *D17 = dag->AddDispenseNode("R1", 10);
		AssayNode *D18 = dag->AddDispenseNode("R2", 10);
		AssayNode *D19 = dag->AddDispenseNode("R3", 10);
		AssayNode *D20 = dag->AddDispenseNode("R4", 10);
		AssayNode *D21 = dag->AddDispenseNode("R1", 10);
		AssayNode *D22 = dag->AddDispenseNode("R2", 10);
		AssayNode *D23 = dag->AddDispenseNode("R3", 10);
		AssayNode *D24 = dag->AddDispenseNode("R4", 10);
		AssayNode *D25 = dag->AddDispenseNode("R1", 10);
		AssayNode *D26 = dag->AddDispenseNode("R2", 10);
		AssayNode *D27 = dag->AddDispenseNode("R3", 10);
		AssayNode *D28 = dag->AddDispenseNode("R4", 10);
		AssayNode *D29 = dag->AddDispenseNode("R1", 10);
		AssayNode *D30 = dag->AddDispenseNode("R2", 10);
		AssayNode *D31 = dag->AddDispenseNode("R3", 10);
		AssayNode *D32 = dag->AddDispenseNode("R4", 10);

		AssayNode *M1 = dag->AddMixNode(2, 5*mult, "M1");
		AssayNode *M2 = dag->AddMixNode(2, 5*mult, "M2");
		AssayNode *M3 = dag->AddMixNode(2, 5*mult, "M3");
		AssayNode *M4 = dag->AddMixNode(2, 5*mult, "M4");
		AssayNode *M5 = dag->AddMixNode(2, 3*mult, "M5");
		AssayNode *M6 = dag->AddMixNode(2, 3*mult, "M6");
		AssayNode *M7 = dag->AddMixNode(2, 3*mult, "M7");
		AssayNode *M8 = dag->AddMixNode(2, 3*mult, "M8");
		AssayNode *M9 = dag->AddMixNode(2, 4*mult, "M9");
		AssayNode *M10 = dag->AddMixNode(2, 4*mult, "M10");
		AssayNode *M11 = dag->AddMixNode(2, 4*mult, "M11");
		AssayNode *M12 = dag->AddMixNode(2, 4*mult, "M12");
		AssayNode *M13 = dag->AddMixNode(2, 6*mult, "M13");
		AssayNode *M14 = dag->AddMixNode(2, 6*mult, "M14");
		AssayNode *M15 = dag->AddMixNode(2, 6*mult, "M15");
		AssayNode *M16 = dag->AddMixNode(2, 6*mult, "M16");

		AssayNode *d1 = dag->AddDetectNode(1, 5*mult, "dt1");
		AssayNode *d2 = dag->AddDetectNode(1, 4*mult, "dt2");
		AssayNode *d3 = dag->AddDetectNode(1, 6*mult, "dt3");
		AssayNode *d4 = dag->AddDetectNode(1, 5*mult, "dt4");
		AssayNode *d5 = dag->AddDetectNode(1, 5*mult, "dt5");
		AssayNode *d6 = dag->AddDetectNode(1, 4*mult, "dt6");
		AssayNode *d7 = dag->AddDetectNode(1, 6*mult, "dt7");
		AssayNode *d8 = dag->AddDetectNode(1, 5*mult, "dt8");
		AssayNode *d9 = dag->AddDetectNode(1, 5*mult, "dt9");
		AssayNode *d10 = dag->AddDetectNode(1, 4*mult, "dt10");
		AssayNode *d11 = dag->AddDetectNode(1, 6*mult, "dt11");
		AssayNode *d12 = dag->AddDetectNode(1, 5*mult, "dt12");
		AssayNode *d13 = dag->AddDetectNode(1, 5*mult, "dt13");
		AssayNode *d14 = dag->AddDetectNode(1, 4*mult, "dt14");
		AssayNode *d15 = dag->AddDetectNode(1, 6*mult, "dt15");
		AssayNode *d16 = dag->AddDetectNode(1, 5*mult, "dt16");

		AssayNode *O1 = dag->AddOutputNode("output");
		AssayNode *O2 = dag->AddOutputNode("output");
		AssayNode *O3 = dag->AddOutputNode("output");
		AssayNode *O4 = dag->AddOutputNode("output");
		AssayNode *O5 = dag->AddOutputNode("output");
		AssayNode *O6 = dag->AddOutputNode("output");
		AssayNode *O7 = dag->AddOutputNode("output");
		AssayNode *O8 = dag->AddOutputNode("output");
		AssayNode *O9 = dag->AddOutputNode("output");
		AssayNode *O10 = dag->AddOutputNode("output");
		AssayNode *O11 = dag->AddOutputNode("output");
		AssayNode *O12 = dag->AddOutputNode("output");
		AssayNode *O13 = dag->AddOutputNode("output");
		AssayNode *O14 = dag->AddOutputNode("output");
		AssayNode *O15 = dag->AddOutputNode("output");
		AssayNode *O16 = dag->AddOutputNode("output");

		dag->ParentChild(D1, M1);
		dag->ParentChild(D2, M2);
		dag->ParentChild(D3, M3);
		dag->ParentChild(D4, M4);
		dag->ParentChild(D5, M5);
		dag->ParentChild(D6, M6);
		dag->ParentChild(D7, M7);
		dag->ParentChild(D8, M8);
		dag->ParentChild(D9, M9);
		dag->ParentChild(D10, M10);
		dag->ParentChild(D11, M11);
		dag->ParentChild(D12, M12);
		dag->ParentChild(D13, M13);
		dag->ParentChild(D14, M14);
		dag->ParentChild(D15, M15);
		dag->ParentChild(D16, M16);
		dag->ParentChild(D17, M1);
		dag->ParentChild(D18, M2);
		dag->ParentChild(D19, M3);
		dag->ParentChild(D20, M4);
		dag->ParentChild(D21, M5);
		dag->ParentChild(D22, M6);
		dag->ParentChild(D23, M7);
		dag->ParentChild(D24, M8);
		dag->ParentChild(D25, M9);
		dag->ParentChild(D26, M10);
		dag->ParentChild(D27, M11);
		dag->ParentChild(D28, M12);
		dag->ParentChild(D29, M13);
		dag->ParentChild(D30, M14);
		dag->ParentChild(D31, M15);
		dag->ParentChild(D32, M16);

		dag->ParentChild(M1, d1);
		dag->ParentChild(M2, d2);
		dag->ParentChild(M3, d3);
		dag->ParentChild(M4, d4);
		dag->ParentChild(M5, d5);
		dag->ParentChild(M6, d6);
		dag->ParentChild(M7, d7);
		dag->ParentChild(M8, d8);
		dag->ParentChild(M9, d9);
		dag->ParentChild(M10, d10);
		dag->ParentChild(M11, d11);
		dag->ParentChild(M12, d12);
		dag->ParentChild(M13, d13);
		dag->ParentChild(M14, d14);
		dag->ParentChild(M15, d15);
		dag->ParentChild(M16, d16);

		dag->ParentChild(d1, O1);
		dag->ParentChild(d2, O2);
		dag->ParentChild(d3, O3);
		dag->ParentChild(d4, O4);
		dag->ParentChild(d5, O5);
		dag->ParentChild(d6, O6);
		dag->ParentChild(d7, O7);
		dag->ParentChild(d8, O8);
		dag->ParentChild(d9, O9);
		dag->ParentChild(d10, O10);
		dag->ParentChild(d11, O11);
		dag->ParentChild(d12, O12);
		dag->ParentChild(d13, O13);
		dag->ParentChild(d14, O14);
		dag->ParentChild(d15, O15);
		dag->ParentChild(d16, O16);
	}

	cout << "Bench2_InVitroDiag_5 CREATED" << endl;
	return dag;
}

///////////////////////////////////////////////////////////////////////////
// This benchmark details the DAG for the protein assay (based on Bradford
// reaction)detailed in Chakrabarty's benchmarks.
///////////////////////////////////////////////////////////////////////////
DAG *Test::Create_B3_Protein(double mult, int repeat)
{
	DAG *dag = new DAG();
	dag->setName("Protein");

	for (int i=0; i < repeat; i++)
	{
		vector<AssayNode*> db;
		vector<AssayNode*> dr;
		vector<AssayNode*> dlt;
		vector<AssayNode*> split;
		vector<AssayNode*> mix;
		vector<AssayNode*> det;
		vector<AssayNode*> out;

		// Create Protein Sample Dispense Node
		AssayNode * ds = dag->AddDispenseNode("DsS", 10, "ds");

		// Create Buffer Dispense Nodes
		db.push_back(NULL);
		for (int i = 1; i <= 39; i++)
		{
			char buff[12];
			sprintf(buff,"db%d", i);
			db.push_back(dag->AddDispenseNode("DsB", 10, buff));
		}

		// Create Reagent Dispense Nodes
		dr.push_back(NULL);
		for (int i = 1; i <= 8; i++)
		{
			char buff[12];
			sprintf(buff,"dr%d", i);
			dr.push_back(dag->AddDispenseNode("DsR", 10, buff));
		}

		// Create Dilute Nodes -- A dilute is a mixture followed by a split
		dlt.push_back(NULL);
		for (int i = 1; i <= 39; i++)
		{
			char buff[12];
			sprintf(buff,"dlt%d", i);
			dlt.push_back(dag->AddMixNode(2, 3*mult, buff));
		}
		split.push_back(NULL);
		for (int i = 1; i <= 7; i++)
		{
			char buff[12];
			sprintf(buff,"split%d", i);
			split.push_back(dag->AddSplitNode(false, 2, 2*mult, buff));
		}

		// Create Mix Nodes
		mix.push_back(NULL);
		for (int i = 1; i <= 8; i++)
		{
			char buff[12];
			sprintf(buff,"mix%d", i);
			mix.push_back(dag->AddMixNode(2, 3*mult, buff));
		}

		// Create Optical Detection Nodes
		det.push_back(NULL);
		for (int i = 1; i <= 8; i++)
		{
			char buff[12];
			sprintf(buff,"det%d", i);
			det.push_back(dag->AddDetectNode(1, 30*mult, buff));
		}

		// Create Output Nodes
		out.push_back(NULL);
		for (int i = 1; i <= 8; i++)
		{
			char buff[12];
			sprintf(buff,"out%d", i);
			out.push_back(dag->AddOutputNode("output", buff));
		}

		// Level 0
		dag->ParentChild(ds, dlt[1]);
		dag->ParentChild(db[1], dlt[1]);
		dag->ParentChild(dlt[1],split[1]);//Split
		//Level 1
		dag->ParentChild(db[2], dlt[2]);
		dag->ParentChild(split[1], dlt[2]);
		dag->ParentChild(split[1], dlt[3]);
		dag->ParentChild(db[3], dlt[3]);
		dag->ParentChild(dlt[2],split[2]);//Split
		dag->ParentChild(dlt[3],split[3]);//Split
		//Level 2
		dag->ParentChild(split[2],dlt[4]);
		dag->ParentChild(split[2],dlt[5]);
		dag->ParentChild(split[3],dlt[6]);
		dag->ParentChild(split[3],dlt[7]);
		//Level 3
		for (int i=4; i<=7; i++)
			dag->ParentChild(db[i], dlt[i]);
		//Level 4
		for (int i=4; i<=7; i++)
		{
			dag->ParentChild(dlt[i],split[i]);//Split
			dag->ParentChild(split[i],dlt[2*i]);
			dag->ParentChild(split[i],dlt[(2*i)+1]);
		}
		//Level 5-11
		for (int i=8; i<=39; i++)
			dag->ParentChild(db[i], dlt[i]);
		for (int i=8; i<=31; i++)
			dag->ParentChild(dlt[i], dlt[i+8]);
		//Level 12-15
		for (int i=1; i <= 8; i++)
		{
			dag->ParentChild(dlt[i+31],mix[i]);//12
			dag->ParentChild(dr[i],mix[i]);//13
			dag->ParentChild(mix[i],det[i]);//14
			dag->ParentChild(det[i],out[i]);//15
		}
	}

	cout << "Bench3_Protein CREATED" << endl;
	return dag;
}

void Test::JETC2014_Tests()
{
	////////////////////////////////////////////////////////////////////////////
	// JETC STATIC ONLINE TESTS
	////////////////////////////////////////////////////////////////////////////
	//DAG * dag = BiocodeTest::Create_2LevelProtein_With_SplitVolumeDetects();
	//DAG * dag = BiocodeTest::Create_2LevelProtein_Lev1FailureSubGraph();
	//DAG * dag = BiocodeTest::Create_2LevelProtein_Lev2SingleFailureSubGraph();
	//DAG * dag = BiocodeTest::Create_2LevelProtein_Lev2DoubleFailureSubGraph();
	//Util::WriteDagToFile(dag, "Assays/B3_ProteinSplit2FT.txt");
	//Synthesis::Schedule("Assays/B3_ProteinSplit2FT.txt", "DmfbArchs/Arch_15_19_B3.txt", "Output/1_SCHED_to_PLACE.txt", LIST_S, GRISSOM_FIX_RA, INDIVADDR_PM, maxDropsPerStorageMod);
	//Synthesis::Place("Output/1_SCHED_to_PLACE.txt", "Output/2_PLACE_to_ROUTE.txt", GRISSOM_FIX_P, INDIVADDR_PM, minCellsBetweenIrMods);
	//Synthesis::Route("Output/2_PLACE_to_ROUTE.txt", ROY_MAZE_R, INDIVADDR_PM, BEG_COMP, FIXED_FULL_PE);
}

///////////////////////////////////////////////////////////////////////////
// This function schedules a benchmark, and then inserts 10 random errors,
// evenly distributed through the list of mixes (to provide somewhat even
// distribution through the assay). Then, it goes through and generates
// recovery sub-graphs needed for each of the errors. It will start the new
// recovery graph at the point where the error occurred (all executed operations
// will be allowed to finish, while no new operations will be allowed to begin
// after the error).
///////////////////////////////////////////////////////////////////////////
void Test::DAC2014RecoveryDagGenerator(string inputDagFile, string inputArchFile, string outputFile, SchedulerType st, ResourceAllocationType rat, PinMapType pmt, int maxStorageDropsPerMod, int numFaults)
{
	///////////////////////////////////////////////////////////////////////////////////////
	// Set random seed based on current time
	unsigned int seed = time(0);
	cout << "Random seed for DAC 2014 error selection: " << seed << endl;
	//unsigned int seed = 1386625523;
	//cout << "Hard-coded seed for DAC 2014 error selection: " << seed << endl;
	srand(seed);

	///////////////////////////////////////////////////////////////////////////////////////
	// String constants/settings
	string outputFolder = "DAC2014/";
	string onBoardPort = "OnDMFB";
	string redundantWastePort = "RedundantWaste";
	string errorOutPort = "ErrorOut";
	string saveOnDmfbPort = "SaveOnDMFB";
	string recoveryPrefix = "_RecoverFromError_";
	string dynamicPrefix = "_DynamicTillError_";

	///////////////////////////////////////////////////////////////////////////////////////
	// Get initial schedule and make sure graph is big enough
	DAG *errorDag = ScheduleAndReturnDag(inputDagFile, inputArchFile, outputFile, st, rat, pmt, maxStorageDropsPerMod);
	cout << errorDag->mixes.size() << " mix operations" << endl;
	{
		stringstream ss;
		ss << "Assay must have at least " << numFaults << " faults, but only has " << errorDag->mixes.size() << endl;
		claim (errorDag->mixes.size() >= numFaults, &ss);
	}

	///////////////////////////////////////////////////////////////////////////////////////
	// Now, select 1 random operations for every quantileSize of mix operations.
	int quantileSize = errorDag->mixes.size() / numFaults; // Compute quantile size for errors
	cout << "Quantile size " << quantileSize << endl;
	cout << "Attemping to select 10 non-overlapping errors." << endl;
	cout << "If no more output is displayed within a few seconds, increase benchmark size or decrease number of errors (or try to run again)" << endl;

	// Make sure we don't get overlapping errors in time, so sort
	Sort::sortNodesByStartThenEndTS(&errorDag->mixes);
	int lastEnd = -1;
	vector<AssayNode *> errorNodes;
	for (unsigned i = 0; i < errorDag->mixes.size(); i = i + quantileSize)
	{
		// Compute the range for this quantile
		bool breakLoop = false;
		int maxIndex = i + quantileSize-1;
		if (maxIndex > errorDag->mixes.size()-1)
			maxIndex = errorDag->mixes.size()-1;
		else if ((errorDag->mixes.size())-(i+quantileSize) < quantileSize)
		{
			maxIndex = errorDag->mixes.size() - 1;
			breakLoop = true;
		}
		//cout << "Pick a random operation with index " << i << " to " << maxIndex << endl;

		// Now, select a random error within the quantile
		int selectedErrorIndex;
		bool foundNonOverlapping = false;
		while(!foundNonOverlapping)
		{
			selectedErrorIndex = (rand() % (maxIndex-i+1)) + i;
			AssayNode *candidate = errorDag->mixes.at(selectedErrorIndex);
			if ((int)candidate->startTimeStep >= lastEnd)
			{
				lastEnd = candidate->endTimeStep;
				foundNonOverlapping = true;
			}
		}
		{
			stringstream ss;
			ss << "Selected error index (" << selectedErrorIndex << " ) must be in the range [" << i << ", " << maxIndex << "]"<< endl;
			claim (selectedErrorIndex >= i && selectedErrorIndex <= maxIndex, &ss);
		}
		//cout << "Selecting index " << selectedErrorIndex << endl;

		// Add node to list of erroneous nodes
		AssayNode *e = errorDag->mixes.at(selectedErrorIndex);
		errorNodes.push_back(e);
		//e->Print();

		if (breakLoop)
			break;
	}
	cout << "Selected " << numFaults << " overlapping errors." << endl;

	///////////////////////////////////////////////////////////////////////////////////////
	// Save sorted results to file
	string errorSummaryFileName = outputFolder + "ErrorSummary.txt";
	ofstream os;
	os.open(errorSummaryFileName.c_str(), os.trunc);
	{
		stringstream str;
		str << "Failed to properly write DAG to file: " << errorSummaryFileName << endl;
		claim (os.good(), &str);
	}
	os << "Seed for DAC 2014 error selection: " << seed << endl;
	os << numFaults << " errors were selected randomly from " << errorDag->name << ":" << endl;
	Sort::sortNodesByStartThenEndTS(&errorNodes);
	for (unsigned i = 0; i < errorNodes.size(); i++)
		os << "Error " << i+1 << ": " << errorNodes.at(i)->name << endl;
	os << endl << endl << "The following represents the above errors with schedules as encountered in their individual recovery graphs." << endl;
	os.close();

	///////////////////////////////////////////////////////////////////////////////////////
	// Output the initial base graph
 	string dagBaseName = errorDag->name;
	DAG *schedRecoveryDag = FileIn::ReadDagFromFile(inputDagFile);
	string recoveryFileName = outputFolder + dagBaseName + recoveryPrefix + "0.txt";
	FileOut::WriteDagToFile(schedRecoveryDag, recoveryFileName);
	FileOut::WriteDagToFile(schedRecoveryDag, outputFolder + dagBaseName + "_INIT.txt");
	schedRecoveryDag->OutputGraphFile(outputFolder+dagBaseName + "_INIT", true, true);
	delete schedRecoveryDag;

	///////////////////////////////////////////////////////////////////////////////////////
	// For each error, create a new recovery graph to run.  The basic flow is:
	// 1.) Read in the recovery graph (just initial DAG for first pass)
	// 2.) Schedule the recovery graph
	// 3.) Identify the error for this pass in the scheduled recovery graph
	// 4.) Create new recovery graph by cutting out any prior operations to the
	//     scheduled error time (excluding nodes needed to reproduce operations to
	//     recover from the error)
	// 5.) Finally, create a dynamic execution graph that essentially gives all
	//     the nodes from the scheduled DAG that occur before the error
	//     a.) Any node that starts during the erroneous node is allowed to continue
	//         to its finish
	//     b.) Any node that ends before the erroneous node ends is allowed to continue
	//         to its finish
	for (unsigned en = 0; en < errorNodes.size(); en++)
	{
		AssayNode *e = errorNodes.at(en);

		///////////////////////////////////////////////////////////////////////////////////
		// 1.) Read in recovery graph to "process" (i.e., simulate execution till next error)
		stringstream in;
		in << outputFolder << dagBaseName << recoveryPrefix << en;
		schedRecoveryDag = FileIn::ReadDagFromFile(in.str() + ".txt");
		schedRecoveryDag->OutputGraphFile(in.str(), true, true);

		///////////////////////////////////////////////////////////////////////////////////
		// 2.) Schedule the recovery graph
		schedRecoveryDag = ScheduleAndReturnDag(in.str() + ".txt", inputArchFile, outputFile, st, rat, pmt, maxStorageDropsPerMod);
		schedRecoveryDag->OutputGraphFile(in.str()+"_Sched", true, true);

		///////////////////////////////////////////////////////////////////////////////////
		// Read in a new DAG to manipulate
		DAG *newRecoveryDag = FileIn::ReadDagFromFile(inputDagFile);

		///////////////////////////////////////////////////////////////////////////////////
		// Now do a mapping from all the old nodes to the new ones
		bool foundMatchingError = false;
		AssayNode *unschedError;
		map<AssayNode*, AssayNode*> schedToUnschedErrorNodeMap;
		map<AssayNode*, AssayNode*> unschedToSchedErrorNodeMap;
		for (unsigned i = 0; i < schedRecoveryDag->allNodes.size(); i++)
		{
			bool foundNodeMatch = false;
			AssayNode *s = schedRecoveryDag->allNodes.at(i);
			for (unsigned j = 0; j < newRecoveryDag->allNodes.size(); j++)
			{
				AssayNode *u = newRecoveryDag->allNodes.at(j);

				// Found a match
				if (s->GetName() == u->GetName())
				{
					//cout << "From unscheduled: " << endl; u->Print();
					//cout << "From scheduled: " << endl; s->Print();
					//cout << endl << endl;
					foundNodeMatch = true;
					schedToUnschedErrorNodeMap[s] = u;
					unschedToSchedErrorNodeMap[u] = s;
				}

				// 3.) Identify the error for this pass in the scheduled recovery graph.
				// This unscheduled node in new DAG corresponds to the original error in question
				if (e->GetName() == u->GetName() && !foundMatchingError)
				{
					foundMatchingError = true;
					unschedError = u;
					//cout << "Original Error: " << endl; u->Print();
					//cout << "Matched Error: " << endl; unschedError->Print();
					//cout << endl << endl;
				}

				if (foundMatchingError && foundNodeMatch)
					break;
			}
		}

		///////////////////////////////////////////////////////////////////////////////////
		// Now, grab all of the recovery nodes (the ancestor nodes of unschedError)
		set<string> *errorAncestors = new set<string>();
		errorAncestors->insert(unschedError->name);
		RecursiveAncestorFind(errorAncestors, unschedError);
		// Debug print
		/*cout << "The ancestors of the failed node " << unschedError->GetName() << ":" << endl;
		set<string>::iterator setIt = errorAncestors->begin();
		for (; setIt != errorAncestors->end(); setIt++)
			cout << (*setIt) << endl;*/

		///////////////////////////////////////////////////////////////////////////////////
		// Sets/variables for keeping track of decisions to keep/cut nodes
		set<AssayNode *> unschedNodesToCut;
		set<string> justExecuted;
		set<string> executeLater;
		int executedTill = -1;

		///////////////////////////////////////////////////////////////////////////////////
		// First, get the scheduled error information
		AssayNode *schedError = unschedToSchedErrorNodeMap[unschedError];
		cout << "Trimming above error #" << en+1 << ": " << schedError->GetSummary() << endl;

		///////////////////////////////////////////////////////////////////////////////////
		// 4.) Create new recovery graph by cutting out any prior operations to the scheduled
		// error time (excluding nodes needed to reproduce operations to recover from the error)

		/**************************************************************************************************
		 * It is important to understand the general structure here. This was implemented quickly so it may
		 * not be very clean. But, essentially, the recovery graph is scheduled (meaning that storages may
		 * now be in the scheduled graph. We want to create a new recovery graph, which only contains non-
		 * scheduled, non-storage nodes. So, in stead of doing the extra work to eliminate storage nodes
		 * and then reconnect all the original connections, we simply start by reading in the fresh recovery
		 * graph as it was before being scheduled. Thus, we made a mapping between the scheduled nodes (b/c
		 * we need to know rough scheduling info before making the recovery graph) and unscheduled nodes
		 * to know what operations were executed before the error occurred. With the map, we can remove
		 * the appropriate nodes from the new, unscheduled, recovery graph.
		 **************************************************************************************************/
		for (unsigned i = 0; i < newRecoveryDag->allNodes.size(); i++)
		{
			AssayNode *unschedNode = newRecoveryDag->allNodes.at(i);
			AssayNode *schedNode = unschedToSchedErrorNodeMap[unschedNode];

			if (schedNode && unschedNode->type != OUTPUT)
			{
				//cout << "Examining: "; schedNode->Print();

				// Checks if already executed based off when error occured
				if ( (schedNode->GetType() != DISPENSE  && ((schedNode->GetEndTS() <= schedError->GetEndTS())  ||  (schedNode->GetStartTS() < schedError->GetEndTS() && schedNode->GetEndTS() >= schedError->GetEndTS()))) ||
					(schedNode->GetType() == DISPENSE && (schedNode->GetEndTS() < schedError->GetEndTS()))   )

				{
					//cout << "\tExecuted: " << schedNode->GetName() << endl;
					justExecuted.insert(schedNode->name);
					if ((int)schedNode->endTimeStep > executedTill)
						executedTill = schedNode->endTimeStep;

					// If the node is not in the ancestor list
					if (!IsInSet(unschedNode->name, errorAncestors))
					{
						for (unsigned j = 0; j < schedNode->GetChildren().size(); j++)
						{
							// Make sure we find the first non-storage node child
							AssayNode *schedNodeChild = schedNode->GetChildren().at(j);
							while (schedNodeChild->type == STORAGE)
								schedNodeChild = schedNodeChild->GetChildren().front();

							// If parent was "executed" but child was not, then need to "restore" the droplet
							// from on the board (simulates the droplet remaining in a chamber by causing it
							// to be input from a dispense port)
							if (schedNodeChild->GetStartTS() >= schedError->GetEndTS())
							{
								//cout << "\tNeeds new \"Reproduce\" parent: " << schedNodeChild->GetName() << endl;
								AssayNode *unschedNodeChild = schedToUnschedErrorNodeMap[schedNodeChild];
								if (unschedNodeChild && unschedNodeChild->type != OUTPUT)
								{
									for (unsigned k = 0; k < unschedNodeChild->GetParents().size(); k++)
									{
										// Add the new dispense parent which simulates droplets remaining on board
										if (unschedNodeChild->GetParents().at(k) == unschedNode)
										{
											(&unschedNodeChild->parents)->erase((&unschedNodeChild->parents)->begin()+k);
											AssayNode *newDispense = newRecoveryDag->AddDispenseNode(onBoardPort, 10);
											(&unschedNodeChild->parents)->push_back(newDispense);
											(&newDispense->children)->push_back(unschedNodeChild);
											break;
										}
									}
								}
							}
						}
						// Cut the node from the recovery graph b/c it was executed
						//cout << "\tCut: " << unschedNode->GetName() << endl;
						unschedNodesToCut.insert(unschedNode);
					}
					//else // Was in the error's ancestors, keep for recovery graph
					//	cout << "\tKeep for recovery: " << unschedNode->GetName() << endl;
				}
				else
				{
					// If did not execute (i.e., it was after the error in the scheduled graph)
					executeLater.insert(schedNode->name);
					//cout << "\tLeave untouched: " << schedNode->GetName() << endl;

					// Need to check parents here to add onBoardPort if necessary.
					// Did not execute this node, but its parents may have been stored since last recovery
					// graph, so need to check parents of scheduled node
					for (int j = schedNode->parents.size()-1; j >= 0; j--)
					{
						AssayNode *schedNodeParent = schedNode->parents.at(j);
						while (schedNodeParent->type == STORAGE)
							schedNodeParent = schedNodeParent->parents.front();

						// Need to copy over dispense parent info (just for continuity between graphs)
						if (schedNodeParent->type == DISPENSE && schedNodeParent->portName == onBoardPort)
						{
							AssayNode *newDispense = newRecoveryDag->AddDispenseNode(onBoardPort, 10);
							newDispense->id = schedNodeParent->id;
							newDispense->name = schedNodeParent->name;
							(&newDispense->children)->push_back(unschedNode);
							(&unschedNode->parents)->push_back(newDispense);
						}
					}
				}
			}
			else if (unschedNode->type != OUTPUT)
			{
				// If no scheduled equivalent was found, that means that
				// it was probably already scheduled in a previous iteration
				//cout << "\tExecuted in past: " << schedNode->GetName() << endl;

				// Cut it unless it is an OnBoard reproduction, or is in the recovery path
				if (unschedNode->portName != onBoardPort && !IsInSet(unschedNode->name, errorAncestors))
				{
					unschedNodesToCut.insert(unschedNode);
					justExecuted.insert(unschedNode->name);
					//cout << "\tCut: " << unschedNode->GetName() << endl;
				}
			}

			// Now, process any child outputs; if this node was executed, execute child output node too
			for (unsigned j = 0; j < unschedNode->children.size(); j++)
			{
				AssayNode *c = unschedNode->children.at(j);
				if (c->type == OUTPUT)
				{
					if (IsInSet(unschedNode->name, &justExecuted))
					{
						justExecuted.insert(c->name);
						unschedNodesToCut.insert(c);
					}
					else if (IsInSet(unschedNode->name, &executeLater))
						executeLater.insert(c->name);
				}
			}
		}

		///////////////////////////////////////////////////////////////////////////////////
		// Output info to file about effects of error on assay (when it occurred and how
		// long the assay was allowed to execute)
		os.open(errorSummaryFileName.c_str(), os.app);
		os << "Error " << en+1 << ": " << schedError->GetSummary() << "." << endl;
		os << "\tOccurred while executing " << in.str() << ".dot" << endl;
		os << "\tLast TS executed: " <<  executedTill << endl;
		os.close();

		///////////////////////////////////////////////////////////////////////////////////
		// Now, that we've identified the nodes to cut, remove them from new recovery DAG
		for (int i = newRecoveryDag->allNodes.size()-1; i >= 0; i--)
		{
			AssayNode *cutNode = newRecoveryDag->allNodes.at(i);

			if (IsInSet(cutNode, &unschedNodesToCut))
			{
				// First check parents, if any are in recovery path, need
				// to change their child to an output
				for (unsigned j = 0; j < cutNode->GetParents().size(); j++)
				{
					AssayNode *cutParent = cutNode->GetParents().at(j);

					// If the cut node's parent is not being cut, need to replace that parent's
					// child with an output (for erroneous output (ErrorOut) or redundant recovery
					// waste ("RedundantWaste")
					if (!IsInSet(cutParent, &unschedNodesToCut))
					{
						for (unsigned k = 0; k < cutParent->GetChildren().size(); k++)
						{
							AssayNode *cutParentChild = cutParent->GetChildren().at(k);
							if (cutParentChild == cutNode && cutNode->type != OUTPUT)
							{
								//cout << "Needs new \"Output\" child: " << cutParent->GetName() << endl;
								(&cutParent->children)->erase((&cutParent->children)->begin()+k);
								AssayNode *newOutput;
								if (cutParent->name == e->name)
									newOutput = newRecoveryDag->AddOutputNode(errorOutPort);
								else
									newOutput = newRecoveryDag->AddOutputNode(redundantWastePort);

								newOutput->status = SCHEDULED;
								newOutput->startTimeStep = cutParent->endTimeStep;
								newOutput->endTimeStep = newOutput->startTimeStep + 1;
								(&cutParent->children)->push_back(newOutput);
								break;
							}
						}
					}
				}

				// Remove the node
				(&cutNode->parents)->clear();
				(&cutNode->children)->clear();
				//cout << "Cutting: " << cutNode->name << endl;
				newRecoveryDag->RemoveNodeFromDAG(cutNode);
				delete cutNode;
				cutNode = NULL;
			}
		}

		///////////////////////////////////////////////////////////////////////////////////
		// If any dispense nodes followed by outputs, remove from graph; assume performed
		for (int i = newRecoveryDag->heads.size()-1; i >= 0; i--)
		{
			AssayNode *dispense = newRecoveryDag->heads.at(i);
			if (dispense->GetChildren().front()->type == OUTPUT)
			{
				newRecoveryDag->RemoveNodeFromDAG(dispense);

				// If output only has one parent (the dispense), remove the output too
				if (dispense->GetChildren().front()->GetParents().size() == 1)
					newRecoveryDag->RemoveNodeFromDAG(dispense->GetChildren().front());
			}
		}

		///////////////////////////////////////////////////////////////////////////////////
		// Output the recovery graph
		stringstream ss;
		ss << outputFolder << dagBaseName << recoveryPrefix << en+1;
		newRecoveryDag->OutputGraphFile(ss.str(), true, true);
		ss << ".txt";
		FileOut::WriteDagToFile(newRecoveryDag, ss.str());

		///////////////////////////////////////////////////////////////////////////////////
		// 5.) Finally, create a dynamic execution graph that essentially gives all
		//     the nodes from the scheduled DAG that occur before the error
		///////////////////////////////////////////////////////////////////////////////////
		// First, if a parent is executed and a child is not, give the parent a new
		// output: "ErrorOut" if the droplet is from an error; "SaveOnDMFB" to
		// simulate the droplet being saved on the DMFB for the next recovery
		// graph to use.
		DAG *dynamicDag = FileIn::ReadDagFromFile(in.str() + ".txt"); // Start with fresh graph
		for (unsigned i = 0; i < dynamicDag->allNodes.size(); i++)
		{
			AssayNode *n = dynamicDag->allNodes.at(i);

			//cout << "Trimming Dynamic Node:" << n->name << endl;
			for (int j = n->children.size()-1; j >= 0; j--)
			{
				AssayNode *c = n->children.at(j);

				// If n was executed and its child was not
				if (IsInSet(n->name, &justExecuted) && IsInSet(c->name, &executeLater) && c->type != OUTPUT)
				{
					(&n->children)->erase((&n->children)->begin()+j);
					AssayNode *newOutput;
					if (n->name == e->name)
						newOutput = dynamicDag->AddOutputNode(errorOutPort);
					else
						newOutput= dynamicDag->AddOutputNode(saveOnDmfbPort);
					(&newOutput->parents)->push_back(n);
					(&n->children)->push_back(newOutput);
				}
			}
		}

		///////////////////////////////////////////////////////////////////////////////////
		// Now, actually delete all non-executed nodes
		for (int i = dynamicDag->allNodes.size()-1; i >= 0; i--)
		{
			AssayNode *n = dynamicDag->allNodes.at(i);

			// If not exectued, remove from dynamic graph
			if (IsInSet(n->name, &executeLater))
			{
				(&n->children)->clear();
				(&n->parents)->clear();
				dynamicDag->RemoveNodeFromDAG(n);
			}
			else if (!IsInSet(n->name, &justExecuted))
			{	// Also, remove if not found in the executed graph and occurred before error
				for (unsigned j = 0; j < schedRecoveryDag->allNodes.size(); j++)
				{
					AssayNode *schedN = schedRecoveryDag->allNodes.at(j);
					if (n->name == schedN->name)
					{
						// If output and parent wasn't executed, then remove
						if (n->type == OUTPUT)
						{
							if (IsInSet(n->parents.front()->name, &executeLater))
							{
								(&n->children)->clear();
								(&n->parents)->clear();
								dynamicDag->RemoveNodeFromDAG(n);
							}
						}
						else if (schedN->startTimeStep >= executedTill)
						{
							(&n->children)->clear();
							(&n->parents)->clear();
							dynamicDag->RemoveNodeFromDAG(n);
						}
					}
				}
			}
		}

		///////////////////////////////////////////////////////////////////////////////////
		// Output the dynamic execution graph
		ss.str("");
		ss << outputFolder << dagBaseName << dynamicPrefix << en+1;
		dynamicDag->OutputGraphFile(ss.str(), true, true);
		ss << ".txt";
		FileOut::WriteDagToFile(dynamicDag, ss.str());

		///////////////////////////////////////////////////////////////////////////////////
		// Cleanup
		delete newRecoveryDag;
		delete dynamicDag;
		delete schedRecoveryDag;
		errorAncestors->clear();
		delete errorAncestors;
	}

	///////////////////////////////////////////////////////////////////////////////////
	// Cleanup and exit
	delete errorDag;
	cout << "Recovery Graph Generator Complete. Exiting." << endl;
}

///////////////////////////////////////////////////////////////////////////
// Recursively searches parents of a node to find all ancestors. Returns
// them in the ancestors set.
///////////////////////////////////////////////////////////////////////////
void Test::RecursiveAncestorFind(set<string> *ancestors, AssayNode *n)
{
	for (unsigned i = 0; i < n->GetParents().size(); i++)
	{
		AssayNode *p = n->GetParents().at(i);
		ancestors->insert(p->name);
		//cout << "Added to error ancestors: "; p->Print();
		//if (ancestors->find(p) == ancestors->end())
		//	cout << "DID NOT ADD!!!" << endl; exit(0);
		RecursiveAncestorFind(ancestors, p);
	}
}

///////////////////////////////////////////////////////////////////////////
// Schedules the input file and returns the scheduled DAG.
///////////////////////////////////////////////////////////////////////////
DAG *Test::ScheduleAndReturnDag(string inputDagFile, string inputArchFile, string outputFile, SchedulerType st, ResourceAllocationType rat, PinMapType pmt, int maxStorageDropsPerMod)
{
	/////////////////////////////////////////////////////////
	// First, read in the inputDagFile and perform an initial scheduling
	/////////////////////////////////////////////////////////
	DAG *schedDag = FileIn::ReadDagFromFile(inputDagFile);

	DmfbArch *arch = FileIn::ReadDmfbArchFromFile(inputArchFile);
	PinMapper *pm;
	if (pmt == INDIVADDR_PM)
		pm = new IndivAddrPinMapper(arch);
	else
		pm = new PinMapper(arch);

	pm->setType(pmt);
	pm->setArch(arch); // Shouldn't be necessary, but 'arch' is not "sticking" in the constructor
	pm->setResAllocType(rat);
	arch->setPinMapper(pm);

	// Set the frequency for the DAG based on the architecture frequency
	for (unsigned i = 0; i < schedDag->getAllNodes().size(); i++)
		schedDag->getAllNodes().at(i)->SetNumCycles( (unsigned)ceil((double)arch->getFreqInHz() * schedDag->getAllNodes().at(i)->GetNumSeconds()) );

	string dir = outputFile.substr(0, outputFile.find_last_of("/")+1);
	FileOut::WriteDagToFile(schedDag, dir + "0_DAG_to_SCHED.txt");
	stringstream fName;
	fName.str("0_" + schedDag->getName());
	schedDag->OutputGraphFile(dir + fName.str(), true, true);
	cout << schedDag->getAllNodes().size() << " total nodes; " << schedDag->getNumNonIoNodes() << " non-I/O nodes." << endl;

	/////////////////////////////////////////////////////////
	// Set Pin-mapping (if implemented)
	/////////////////////////////////////////////////////////
	arch->getPinMapper()->setMapPreSched();

	/////////////////////////////////////////////////////////
	// Set parameters and do compatability checks
	/////////////////////////////////////////////////////////
	Scheduler *scheduler;
	if (st == LIST_S)
		scheduler = new ListScheduler();
	else if (st == PATH_S)
		scheduler = new PathScheduler();
	else if (st == GENET_S)
		scheduler = new GenetScheduler();
	else if (st == RICKETT_S)
		scheduler = new RickettScheduler();
	else if (st == FD_LIST_S)
		scheduler = new FDLScheduler();
	else
		scheduler = new Scheduler();

	scheduler->setType(st);
	scheduler->setMaxStoragePerModule(maxStorageDropsPerMod);
	CompatChk::PreScheduleChk(scheduler, arch, false);

	/////////////////////////////////////////////////////////
	// Scheduling ///////////////////////////////////////////
	/////////////////////////////////////////////////////////
	//ElapsedTimer sTime("Scheduling Time");
	//sTime.startTimer();
	scheduler->schedule(arch, schedDag);
	//sTime.endTimer();
	//sTime.printElapsedTime();

	/////////////////////////////////////////////////////////
	// Scheduling --> Placement Interface
	/////////////////////////////////////////////////////////
	FileOut::WriteScheduledDagAndArchToFile(schedDag, arch, scheduler, outputFile);
	fName.str("1_" + schedDag->getName() + "_Sched");
	schedDag->OutputGraphFile(dir + fName.str(), true, true);

	/////////////////////////////////////////////////////////
	// Schedule analysis
	/////////////////////////////////////////////////////////
	//FileOut::WriteStringToFile(Analyze::AnalyzeSchedule(schedDag), "Output/4_ANALYSIS_SCHEDULE.txt");

	/////////////////////////////////////////////////////////
	// Cleanup
	/////////////////////////////////////////////////////////
	delete scheduler;
	delete arch;

	return schedDag;
}

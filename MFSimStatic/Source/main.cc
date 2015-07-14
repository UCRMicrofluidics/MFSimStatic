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
 * Source: main.cc																*
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
#ifndef _MAIN
#define _MAIN

using namespace std;
#include "../Headers/Testing/biocode_test.h"
#include "../Headers/Testing/test.h"
#include "../Headers/command_line.h"
#include "../Headers/synthesis.h"
#include "../Headers/Resources/enums.h"
#include "../Headers/compatibility_check.h"

#include <math.h>

///////////////////////////////////////////////////////////////////////////////////
// Main function called when starting program.
///////////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
	//cout << "Debug Min-Cost-Max-Flow" << endl;
	//Test::MinCostMaxFlow();
	//cout << "Exiting." << endl;
	//exit(0);

	CmdLine cmdLine;

	if (cmdLine.IsUsingCommandLine(argc))
		cmdLine.ExecuteCommand(argc, argv);
	else // Hard-coded Sandbox
	{
		cmdLine.ForceCorrectUsage(); // Comment out to be able to run following code
		cout << "Executing Hard-coded Sandbox MF Simulation" << endl << "-----------------------------------------------"<< endl;

		BiocodeTest::Create_B1_PCRMix(1);
		//exit(0);

		////////////////////////////////////////////////////////////////////////////
		// Synthesis Variables
		////////////////////////////////////////////////////////////////////////////
		int maxDropsPerStorageMod = 1;
		int minCellsBetweenIrMods = 1;
		int numHorizTracks = 3;
		int numVertTracks = 3;

		////////////////////////////////////////////////////////////////////////////
		////////////////////////ENTIRE SYNTHESIS FLOW///////////////////////////////
		////////////////////////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////////////////
		// Entire Standard Synthesis Flow Tests
		////////////////////////////////////////////////////////////////////////////
		//Synthesis::EntireFlow("Assays/B1/Dilute/PCRDilute.txt", "DmfbArchs/IndividuallyAddressable/B1/Arch_16_16_B1.txt", LIST_S, GRISSOM_LE_B, ROY_MAZE_R, GRISSOM_FIX_1_RA, INDIVADDR_PM, NONE_WR, BEG_COMP, FIXED_FULL_PE, ALL_EX, maxDropsPerStorageMod, minCellsBetweenIrMods, 3, 3);
		////////////////////////////////////////////////////////////////////////////
		// Entire SkyCal Synthesis Flow Tests
		////////////////////////////////////////////////////////////////////////////
		//Synthesis::EntireFlow("Assays/B1/MixSplit/PCR.txt", "DmfbArchs/IndividuallyAddressable/B1/Arch_15_19_B1.txt", SKYCAL_S, SKYCAL_P, SKYCAL_R, GRISSOM_FIX_1_RA, INDIVADDR_PM, PATH_FINDER_WR, INHERENT_COMP, FIXED_PE, ALL_EX, maxDropsPerStorageMod, minCellsBetweenIrMods, numHorizTracks, numVertTracks);
		//Synthesis::EntireFlow("Assays/B2/InVitro_Ex1_2s_2r.txt", "DmfbArchs/IndividuallyAddressable/B2/Arch_15_19_B2.txt", SKYCAL_S, SKYCAL_P, SKYCAL_R, GRISSOM_FIX_1_RA, INDIVADDR_PM, NONE_WR, INHERENT_COMP, FIXED_PE, ALL_EX, maxDropsPerStorageMod, minCellsBetweenIrMods, numHorizTracks, numVertTracks);
		//Synthesis::EntireFlow("Assays/B2/InVitro_Ex5_4s_4r.txt", "DmfbArchs/IndividuallyAddressable/B2/Arch_15_19_B2.txt", SKYCAL_S, SKYCAL_P, SKYCAL_R, GRISSOM_FIX_1_RA, INDIVADDR_PM, NONE_WR, INHERENT_COMP, FIXED_PE, ALL_EX, maxDropsPerStorageMod, minCellsBetweenIrMods, numHorizTracks, numVertTracks);
		//Synthesis::EntireFlow("Assays/B2/InVitro_Ex5_4s_4r.txt", "DmfbArchs/IndividuallyAddressable/B2/Arch_15_19_B2.txt", SKYCAL_S, SKYCAL_P, SKYCAL_R, GRISSOM_FIX_1_RA, INDIVADDR_PM, NONE_WR, INHERENT_COMP, FIXED_PE, ALL_EX, maxDropsPerStorageMod, minCellsBetweenIrMods, numHorizTracks, numVertTracks);
		//Synthesis::EntireFlow("Assays/B3/MixSplit/Protein.txt", "DmfbArchs/IndividuallyAddressable/B3and4/Arch_15_19_B3.txt", SKYCAL_S, SKYCAL_P, SKYCAL_R, GRISSOM_FIX_1_RA, INDIVADDR_PM, NONE_WR, INHERENT_COMP, FIXED_PE, ALL_EX, maxDropsPerStorageMod, minCellsBetweenIrMods, numHorizTracks, numVertTracks);
		//Synthesis::EntireFlow("Assays/B4/MixSplit/ProteinSplit_05_Eq.txt", "DmfbArchs/IndividuallyAddressable/B3and4/Arch_15_19_B3.txt", SKYCAL_S, SKYCAL_P, SKYCAL_R, GRISSOM_FIX_1_RA, INDIVADDR_PM, NONE_WR, INHERENT_COMP, FIXED_PE, ALL_EX, maxDropsPerStorageMod, minCellsBetweenIrMods, numHorizTracks, numVertTracks);
		//Synthesis::EntireFlow("Assays/B4/MixSplit/ProteinSplit_01_Eq.txt", "DmfbArchs/IndividuallyAddressable/B3and4/Arch_15_19_B3.txt", SKYCAL_S, SKYCAL_P, SKYCAL_R, GRISSOM_FIX_1_RA, INDIVADDR_PM, NONE_WR, INHERENT_COMP, FIXED_PE, ALL_EX, maxDropsPerStorageMod, minCellsBetweenIrMods, numHorizTracks, numVertTracks);

		////////////////////////////////////////////////////////////////////////////
		///////////////////INDIVIDUAL SYNTHESIS STEPS///////////////////////////////
		////////////////////////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////////////////
		// Individual Synthesis Steps Tests - Field-Programmable Pin-Constrained
		////////////////////////////////////////////////////////////////////////////
		bool isParallel = true;
		bool performWash = true;
		PinMapType fppcPinMapType;
		RouterType fppcRouterType;
		if (isParallel)
		{
			fppcPinMapType = ENHANCED_FPPC_ROUTE_OPT_PM;
			fppcRouterType = FPPC_PARALLEL_R;
		}
		else
		{
			fppcPinMapType = ENHANCED_FPPC_PIN_OPT_PM;
			fppcRouterType = FPPC_SEQUENTIAL_R;
		}
		//Synthesis::Schedule("Assays/B1/MixSplit/PCR.txt", "DmfbArchs/FPPC/Original_DAC_13/Arch_PC1_B1.txt", "Output/1_SCHED_to_PLACE.txt", FPPC_S, PC_INHERENT_RA, fppcPinMapType, maxDropsPerStorageMod);
		Synthesis::Schedule("Assays/B1/MixSplit/PCR.txt", "DmfbArchs/FPPC/Enhanced_TCAD/Arch_FPPC2_B1.txt", "Output/1_SCHED_to_PLACE.txt", FPPC_S, PC_INHERENT_RA, fppcPinMapType, maxDropsPerStorageMod);
		//Synthesis::Schedule("Assays/B2/InVitro_Ex1_2s_2r.txt", "DmfbArchs/FPPC/Enhanced_TCAD/Arch_FPPC2_B2.txt", "Output/1_SCHED_to_PLACE.txt", FPPC_S, PC_INHERENT_RA, fppcPinMapType, maxDropsPerStorageMod);
		//Synthesis::Schedule("Assays/B2/InVitro_Ex2_2s_3r.txt", "DmfbArchs/FPPC/Enhanced_TCAD/Arch_FPPC2_B2.txt", "Output/1_SCHED_to_PLACE.txt", FPPC_S, PC_INHERENT_RA, fppcPinMapType, maxDropsPerStorageMod);
		//Synthesis::Schedule("Assays/B2/InVitro_Ex3_3s_3r.txt", "DmfbArchs/FPPC/Enhanced_TCAD/Arch_FPPC2_B2.txt", "Output/1_SCHED_to_PLACE.txt", FPPC_S, PC_INHERENT_RA, fppcPinMapType, maxDropsPerStorageMod);
		//Synthesis::Schedule("Assays/B2/InVitro_Ex4_3s_4r.txt", "DmfbArchs/FPPC/Enhanced_TCAD/Arch_FPPC2_B2.txt", "Output/1_SCHED_to_PLACE.txt", FPPC_S, PC_INHERENT_RA, fppcPinMapType, maxDropsPerStorageMod);
		//Synthesis::Schedule("Assays/B2/InVitro_Ex5_4s_4r.txt", "DmfbArchs/FPPC/Enhanced_TCAD/Arch_FPPC2_B2.txt", "Output/1_SCHED_to_PLACE.txt", FPPC_S, PC_INHERENT_RA, fppcPinMapType, maxDropsPerStorageMod);
		//Synthesis::Schedule("Assays/B4/MixSplit/ProteinSplit_02_Eq.txt", "DmfbArchs/FPPC/Enhanced_TCAD/Arch_FPPC2_B3.txt", "Output/1_SCHED_to_PLACE.txt", FPPC_PATH_S, PC_INHERENT_RA, fppcPinMapType, maxDropsPerStorageMod);
		//Synthesis::Schedule("Assays/B4/DiluteProteinSplitDilute_02_Eq.txt", "DmfbArchs/FPPC/Enhanced_TCAD/Arch_FPPC2_B3.txt", "Output/1_SCHED_to_PLACE.txt", FPPC_S, PC_INHERENT_RA, fppcPinMapType, maxDropsPerStorageMod);
		Synthesis::Place("Output/1_SCHED_to_PLACE.txt", "Output/2_PLACE_to_ROUTE.txt", FPPC_LE_B, minCellsBetweenIrMods);
		Synthesis::Route("Output/2_PLACE_to_ROUTE.txt", fppcRouterType, performWash, INHERENT_COMP, FPPC_PE, ALL_EX);
		Synthesis::WireRoute("Output/3_ROUTE_to_SIM.txt", ENHANCED_FPPC_WR, 3, 3);
		////////////////////////////////////////////////////////////////////////////
		// Individual Synthesis Steps Tests - Independently Addressed Electrodes
		////////////////////////////////////////////////////////////////////////////
		//Synthesis::Schedule("Assays/B3_ProteinSplit2FT.txt", "DmfbArchs/IndividuallyAddressable/B3/Arch_15_19_B3.txt", "Output/1_SCHED_to_PLACE.txt", LIST_S, GRISSOM_FIX_0_RA, INDIVADDR_PM, maxDropsPerStorageMod);
		//Synthesis::Schedule("Assays/Testing/Single_2_Input_Mix.txt", "DmfbArchs/IndividuallyAddressable/Testing/Arch_8_7_Simple.txt", "Output/1_SCHED_to_PLACE.txt", LIST_S, GRISSOM_FIX_0_RA, INDIVADDR_PM, maxDropsPerStorageMod); // Good for debugging pin-assignment and wire-routing
		//Synthesis::Schedule("Assays/Testing/Two_Dilutes.txt", "DmfbArchs/IndividuallyAddressable/B1/Arch_15_19_B1.txt", "Output/1_SCHED_to_PLACE.txt", PATH_S, GRISSOM_FIX_0_RA, INDIVADDR_PM, maxDropsPerStorageMod); // Good for debugging new dilution operator
		//Synthesis::Schedule("Assays/B1/MixSplit/PCR.txt", "DmfbArchs/IndividuallyAddressable/B1/Arch_15_19_B1.txt", "Output/1_SCHED_to_PLACE.txt", LIST_S, GRISSOM_FIX_0_RA, INDIVADDR_PM, maxDropsPerStorageMod);
		//Synthesis::Schedule("Assays/B2/InVitro_Ex1_2s_2r.txt", "DmfbArchs/IndividuallyAddressable/B2/Arch_15_19_B2.txt", "Output/1_SCHED_to_PLACE.txt", LIST_S, GRISSOM_FIX_0_RA, INDIVADDR_PM, maxDropsPerStorageMod);
		//Synthesis::Schedule("Assays/B2/InVitro_Ex2_2s_3r.txt", "DmfbArchs/IndividuallyAddressable/B2/Arch_15_19_B2.txt", "Output/1_SCHED_to_PLACE.txt", LIST_S, GRISSOM_FIX_0_RA, INDIVADDR_PM, maxDropsPerStorageMod);
		//Synthesis::Schedule("Assays/B2/InVitro_Ex3_3s_3r.txt", "DmfbArchs/IndividuallyAddressable/B2/Arch_15_19_B2.txt", "Output/1_SCHED_to_PLACE.txt", LIST_S, GRISSOM_FIX_0_RA, INDIVADDR_PM, maxDropsPerStorageMod);
		//Synthesis::Schedule("Assays/B2/InVitro_Ex4_3s_4r.txt", "DmfbArchs/IndividuallyAddressable/B2/Arch_15_19_B2.txt", "Output/1_SCHED_to_PLACE.txt", LIST_S, GRISSOM_FIX_0_RA, INDIVADDR_PM, maxDropsPerStorageMod);
		//Synthesis::Schedule("Assays/B2/InVitro_Ex5_4s_4r.txt", "DmfbArchs/IndividuallyAddressable/B2/Arch_15_19_B2.txt", "Output/1_SCHED_to_PLACE.txt", LIST_S, GRISSOM_FIX_0_RA, INDIVADDR_PM, maxDropsPerStorageMod);
		//Synthesis::Schedule("Assays/B4/MixSplit/ProteinSplit_03_Eq.txt", "DmfbArchs/IndividuallyAddressable/B3and4/Arch_15_19_B3.txt", "Output/1_SCHED_to_PLACE.txt", PATH_S, GRISSOM_FIX_0_RA, INDIVADDR_PM, maxDropsPerStorageMod);
		//Synthesis::Schedule("Assays/B4/Dilute/ProteinSplitDilute_03_Eq.txt", "DmfbArchs/IndividuallyAddressable/B3and4/Arch_15_19_B3.txt", "Output/1_SCHED_to_PLACE.txt", PATH_S, GRISSOM_FIX_0_RA, INDIVADDR_PM, maxDropsPerStorageMod);
		//Synthesis::Place("Output/1_SCHED_to_PLACE.txt", "Output/2_PLACE_to_ROUTE.txt", GRISSOM_PATH_B, minCellsBetweenIrMods);
		//Synthesis::Route("Output/2_PLACE_to_ROUTE.txt", ROY_MAZE_R, false, BEG_COMP, FIXED_PE, SIM_EX);
		//Synthesis::WireRoute("Output/3_ROUTE_to_SIM.txt", PATH_FINDER_WR, 3, 3);


		// SHARP TEST
		//Synthesis::Schedule("Assays/B4/MixSplit/ProteinSplit_12_Eq.txt", "DmfbArchs/Sharp/Demo.txt", "Output/1_SCHED_to_PLACE.txt", PATH_S, GRISSOM_FIX_0_RA, INDIVADDR_PM, maxDropsPerStorageMod);
		//Synthesis::Schedule("Assays/B4/Dilute/ProteinSplitDilute_07_Eq.txt", "DmfbArchs/Sharp/Demo.txt", "Output/1_SCHED_to_PLACE.txt", PATH_S, GRISSOM_FIX_0_RA, INDIVADDR_PM, maxDropsPerStorageMod);
		//Synthesis::Place("Output/1_SCHED_to_PLACE.txt", "Output/2_PLACE_to_ROUTE.txt", GRISSOM_PATH_B, minCellsBetweenIrMods);
		//Synthesis::Route("Output/2_PLACE_to_ROUTE.txt", ROY_MAZE_R, false, BEG_COMP, FIXED_PE, ALL_EX);
		//Synthesis::WireRoute("Output/3_ROUTE_to_SIM.txt", PATH_FINDER_WR, 3, 3);


		////////////////////////////////////////////////////////////////////////////
		///////////////////////////WIRE ROUTING TESTS///////////////////////////////
		////////////////////////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////////////////
		// Pin-constrained wire-routing Tests
		////////////////////////////////////////////////////////////////////////////
		//int oCap = 3;
		//Synthesis::WireRoute("Output/B1_PCR_08.txt", PATH_FINDER_WR, oCap, oCap);
		//Synthesis::WireRoute("Output/B2_InVitro_08.txt", PATH_FINDER_WR, oCap, oCap);
		//Synthesis::WireRoute("Output/B3_Protein_08.txt", PATH_FINDER_WR, oCap, oCap);
		//Synthesis::WireRoute("Output/B123_MultiFunctional_08.txt", PATH_FINDER_WR, oCap, oCap);
		//Synthesis::WireRoute("Output/B1_PCR_12.txt", PATH_FINDER_WR, oCap, oCap);
		//Synthesis::WireRoute("Output/B2_InVitro_12.txt", PATH_FINDER_WR, oCap, oCap);
		//Synthesis::WireRoute("Output/B3_Protein_12.txt", PATH_FINDER_WR, oCap, oCap);
		//Synthesis::WireRoute("Output/B123_MultiFunctional_12.txt", PATH_FINDER_WR, oCap, oCap);
		//Synthesis::WireRoute("Output/B1_PCR_IA.txt", PATH_FINDER_WR, oCap, oCap);
		//Synthesis::WireRoute("Output/B2_InVitro_IA.txt", PATH_FINDER_WR, oCap, oCap);
		//Synthesis::WireRoute("Output/B3_Protein_IA.txt", PATH_FINDER_WR, oCap, oCap);
		//Synthesis::WireRoute("Output/B123_MultiFunctional_IA.txt", PATH_FINDER_WR, oCap, oCap);
		////////////////////////////////////////////////////////////////////////////
		// FPPC Wire-Routing Tests
		////////////////////////////////////////////////////////////////////////////
		//Synthesis::WireRoute("Output/EnhancedFPPC_PinOpt_4.txt", FPPC2_WR, oCap, oCap);
		//Synthesis::WireRoute("Output/EnhancedFPPC_PinOpt_8.txt", FPPC2_WR, oCap, oCap);
		//Synthesis::WireRoute("Output/EnhancedFPPC_RouteOpt_4.txt", PATH_FINDER_WR, oCap, oCap);
		//Synthesis::WireRoute("Output/EnhancedFPPC_RouteOpt_8.txt", PATH_FINDER_WR, oCap, oCap);
		//Synthesis::WireRoute("Output/EnhancedFPPC_IA_4.txt", PATH_FINDER_WR, oCap, oCap);
		//Synthesis::WireRoute("Output/EnhancedFPPC_IA_8.txt", PATH_FINDER_WR, oCap, oCap);
		//Synthesis::WireRoute("Output/EnhancedFPPC_IA.txt", PATH_FINDER_WR, oCap, oCap);
		////////////////////////////////////////////////////////////////////////////
		// Variable Wire Routing Tests
		////////////////////////////////////////////////////////////////////////////
		//Test::WireRouteIndivAddr(41, 33, PATH_FINDER_WR, 3, 3);
		//Test::WireRouteJetcVirtTop(5, 5, PATH_FINDER_WR, 3, 3);
		//Test::WireRouteJetc2VirtTop(2, 2, PATH_FINDER_WR, 3, 3);
		//Test::WireRouteDacOrigFppc(15, 15, PATH_FINDER_WR, 3, 3);
		//Test::WireRouteTcadEnhancedFppc(true, 15, 15, FPPC2_WR, 3, 3);

		////////////////////////////////////////////////////////////////////////////
		///////////////////////RECOVERY DAG GENERATOR///////////////////////////////
		////////////////////////////////////////////////////////////////////////////
		//Test::DAC2014RecoveryDagGenerator("Assays/B4/MixSplit/ProteinSplit_05_Eq.txt", "DmfbArchs/IndividuallyAddressable/B3and4/Arch_15_19_B3.txt", "Output/1_SCHED_to_PLACE.txt", PATH_S, GRISSOM_FIX_1_RA, INDIVADDR_PM, maxDropsPerStorageMod, 4);
		//Synthesis::Schedule("DAC2014/B4_Protein_Mix_Levels_5_RecoverFromError_4.txt", "DmfbArchs/IndividuallyAddressable/B3and4/Arch_15_19_B3.txt", "Output/1_SCHED_to_PLACE.txt", PATH_S, GRISSOM_FIX_1_RA, INDIVADDR_PM, maxDropsPerStorageMod);
		//Synthesis::Schedule("DAC2014/B4_Protein_Mix_Levels_5_DynamicTillError_3.txt", "DmfbArchs/IndividuallyAddressable/B3and4/Arch_15_19_B3.txt", "Output/1_SCHED_to_PLACE.txt", PATH_S, GRISSOM_FIX_1_RA, INDIVADDR_PM, maxDropsPerStorageMod);
		//Synthesis::Place("Output/1_SCHED_to_PLACE.txt", "Output/2_PLACE_to_ROUTE.txt", GRISSOM_PATH_B, minCellsBetweenIrMods);
		//Synthesis::Route("Output/2_PLACE_to_ROUTE.txt", ROY_MAZE_R, BEG_COMP, FIXED_PE, SIM_EX);

		cout << "-------------------------" << endl << "Exiting MF Simulator" << endl;
	}

	return 0;
}

#endif

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
 * Source: compatibility_check.cc												*
 * Original Code Author(s): Dan Grissom											*
 * Original Completion/Release Date: May 29, 2013								*
 *																				*
 * Details: N/A																	*
 *																				*
 * Revision History:															*
 * WHO		WHEN		WHAT													*
 * ---		----		----													*
 * FML		MM/DD/YY	One-line description									*
 *-----------------------------------------------------------------------------*/
#include "../Headers/compatibility_check.h"
#include "../Headers/Testing/claim.h"
#include "../Headers/Models/dmfb_arch.h"
#include "../Headers/Placer/placer.h"
#include "../Headers/Scheduler/scheduler.h"
#include "../Headers/Router/router.h"
#include <set>

//////////////////////////////////////////////////////////////////////////////
// Checks to be done to check parameters just before scheduling.
// NOTE: Developers should add more checks as they create new synthesis methods.
//////////////////////////////////////////////////////////////////////////////
void CompatChk::PreScheduleChk(Scheduler *s, DmfbArch *arch, DAG *dagToSchedule, bool runAsEntireFlow)
{
	// Using the field-programmable pin-constrained scheduler...
	if (s->getType() == FPPC_S)
	{
		claim(s->getMaxStoragePerModule() == 1, "The field-programmable pin-constrained scheduler can only handle 1 droplet per storage module.");
		PinMapType pmt = arch->getPinMapper()->getType();
		claim(pmt == ORIGINAL_FPPC_PM || pmt == ENHANCED_FPPC_PIN_OPT_PM || pmt == ENHANCED_FPPC_ROUTE_OPT_PM, "The field-programmable pin-constrained (FPPC) scheduler must be paired with the FPPC pin-mapper.");
	}

	// General checks
	claim(s->getMaxStoragePerModule() >= 1, "Must set the max storage per module to at least 1 droplet.");

	// Ensure there is at least one external resource for each type of special operation
	if (dagToSchedule->requiresDetector())
	{
		stringstream ss("");
		ss << "The assay requires detectors, but the supplied architecure file (" << arch->getName() << ") has none.";
		claim(arch->hasDetectors(), ss.str());
	}
	if (dagToSchedule->requiresHeater())
	{
		stringstream ss("");
		ss << "The assay requires heaters, but the supplied architecure file (" << arch->getName() << ") has none.";
		claim(arch->hasHeaters(), ss.str());
	}

	// Ensure that the DMFB architecture has an Input/Output for every fluid required
	for (int i = 0; i < dagToSchedule->getAllInputNodes().size(); i++)
	{
		AssayNode *node = dagToSchedule->getAllInputNodes().at(i);
		if (node->GetType() == DISPENSE)
		{
			stringstream ss("");
			ss << "The assay requires an input reservoir of fluid type \"" << node->GetPortName() << "\", but the supplied architecure file (" << arch->getName() << ") has none.";
			claim(arch->getIoPort(node->GetPortName()) != NULL, ss.str());
		}
	}
	for (int i = 0; i < dagToSchedule->getAllOutputNodes().size(); i++)
	{
		AssayNode *node = dagToSchedule->getAllOutputNodes().at(i);
		if (node->GetType() == OUTPUT)
		{
			stringstream ss("");
			ss << "The assay requires an output reservoir named \"" << node->GetPortName() << "\", but the supplied architecure file (" << arch->getName() << ") has none.";
			claim(arch->getIoPort(node->GetPortName()) != NULL, ss.str());
		}
	}
}

//////////////////////////////////////////////////////////////////////////////
// Checks to be done to check parameters just before placement.
// NOTE: Developers should add more checks as they create new synthesis methods.
//////////////////////////////////////////////////////////////////////////////
void CompatChk::PrePlaceChk(Placer *p, DmfbArch *arch, bool runAsEntireFlow)
{
	// If using the field-programmable pin-constrained placer...
	if (p->getType() == FPPC_LE_B)
	{
		claim(p->getMaxStoragePerModule() == 1, "The field-programmable pin-constrained scheduler can only handle 1 droplet per storage module.");
		PinMapType pmt = arch->getPinMapper()->getType();
		claim(pmt == ORIGINAL_FPPC_PM || pmt == ENHANCED_FPPC_PIN_OPT_PM || pmt == ENHANCED_FPPC_ROUTE_OPT_PM, "The field-programmable pin-constrained (FPPC) placer must be paired with the FPPC pin-mapper.");
		claim(p->getPastSchedType() == FPPC_S || p->getPastSchedType() == FPPC_PATH_S, "The field-programmable pin-constrained (FPPC) placer must be paired with the FPPC scheduler.");
	}

	// If using a binder (non-pin-constrained)...
	if (p->getType() == GRISSOM_LE_B || p->getType() == GRISSOM_PATH_B)
		claim(p->getMaxStoragePerModule() <= 4, "The modules in the binders do not support  more than 2 droplets per module.");
	else
		claim(p->getMaxStoragePerModule() <= 4, "The modules in the free-placers only support up to 4 droplets per module.");

	// General checks
	claim(p->getHCellsBetweenModIR() >= 0 && p->getVCellsBetweenModIR() >= 0, "Cannot have a negative distance between module interference regions (IRs).");
}

//////////////////////////////////////////////////////////////////////////////
// Checks to be done to check parameters just before routing.
// NOTE: Developers should add more checks as they create new synthesis methods.
//////////////////////////////////////////////////////////////////////////////
void CompatChk::PreRouteChk(Router *r, DmfbArch *arch, bool runAsEntireFlow)
{
	// Using the field-programmable pin-constrained router...
	if (r->getType() == FPPC_SEQUENTIAL_R)
	{
		PinMapType pmt = arch->getPinMapper()->getType();
		claim(pmt == ORIGINAL_FPPC_PM || pmt == ENHANCED_FPPC_PIN_OPT_PM, "The field-programmable pin-constrained (FPPC) router must be paired with the FPPC or enhanced FPPC Pin-optimized pin-mapper.");
		claim(r->getPastSchedType() == FPPC_S || r->getPastSchedType() == FPPC_PATH_S, "The field-programmable pin-constrained (FPPC) router must be paired with the FPPC scheduler.");
		claim(r->getPastPlacerType() == FPPC_LE_B, "The field-programmable pin-constrained (FPPC) router must be paired with the FPPC placer.");
		claim(r->getProcEngineType() == FPPC_PE, "The field-programmable pin-constrained (FPPC) router must be paired with the FPPC processing engine.");
		claim(r->getCompactionType() == INHERENT_COMP, "The field-programmable pin-constrained (FPPC) router must be paired with the inherent compaction type.");
	}
	else if (r->getType() == FPPC_PARALLEL_R)
	{
		PinMapType pmt = arch->getPinMapper()->getType();
		claim(pmt == ENHANCED_FPPC_ROUTE_OPT_PM, "The field-programmable pin-constrained (FPPC) router must be paired with the route optimized FPPC pin-mapper.");
		claim(r->getPastSchedType() == FPPC_S || r->getPastSchedType() == FPPC_PATH_S, "The field-programmable pin-constrained (FPPC) router must be paired with the FPPC scheduler.");
		claim(r->getPastPlacerType() == FPPC_LE_B, "The field-programmable pin-constrained (FPPC) router must be paired with the FPPC placer.");
		claim(r->getProcEngineType() == FPPC_PE, "The field-programmable pin-constrained (FPPC) router must be paired with the FPPC processing engine.");
		claim(r->getCompactionType() == INHERENT_COMP, "The field-programmable pin-constrained (FPPC) router must be paired with the inherent compaction type.");
	}
	else if (r->getType() == GRISSOM_FIX_R || r->getType() == GRISSOM_FIX_MAP_R)
		claim(arch->getPinMapper()->getResAllocType() == GRISSOM_FIX_1_RA, "The 'GRISSOM_FIX_R' and 'GRISSOM_FIX_MAP_R' routers are only compatible with the 'GRISSOM_FIX_1_RA' resource allocation type.");
	else if (r->getType() == CDMA_FULL_R)
		claim(r->getCompactionType() == INHERENT_COMP, "The CDMA router must be paried with the inherent compaction type.");
	else
	{
		claim(r->getCompactionType() != INHERENT_COMP, "You must select the 'Beginning', 'Middle', or 'No' compaction type.");
	}

	// Only certain algorithms can perform wash droplet routing; ensure we have selected one
	if (r->performWash())
	{
		set<RouterType> washableTypes;
		washableTypes.insert(FPPC_SEQUENTIAL_R);
		washableTypes.insert(FPPC_PARALLEL_R);

		claim(washableTypes.count(r->getType()) > 0, "Router type does not implement wash droplet handling. Please select another router.");

	}

	// If used a binder for placement (with fixed placement), use proper processing engine(s)...
	if (r->getPastPlacerType() == GRISSOM_LE_B || r->getPastPlacerType() == GRISSOM_PATH_B)
		claim(r->getProcEngineType() == FIXED_PE, "A binder was used for placing, so a 'Fixed (-module)' processing engine must be used.");
	else if (r->getPastPlacerType() == KAMER_LL_P)
		claim(r->getProcEngineType() == FREE_PE, "A free-placer was used for placing, so a 'Free (-module)' processing engine must be used.");

	if (r->getCompactionType() == CHO_COMP)
		claim(r->getType() == CHO_R, "The Cho Compaction type MUST be used with the Cho router.");
}

//////////////////////////////////////////////////////////////////////////////
// Checks to be done to check parameters just before wire routing.
// NOTE: Developers should add more checks as they create new synthesis methods.
//////////////////////////////////////////////////////////////////////////////
void CompatChk::PreWireRouteChk(DmfbArch *arch, bool runAsEntireFlow)
{
	if (arch->getWireRouter()->getType() == ENHANCED_FPPC_WR)
		claim(arch->getPinMapper()->getType() == ENHANCED_FPPC_PIN_OPT_PM || arch->getPinMapper()->getType() == ENHANCED_FPPC_ROUTE_OPT_PM, "Must be using one of the enhanced FPPC pin-mapping types if using the enhanced FPPC wire router.");

	if (arch->getPinMapper()->getType() == SWITCH_PM)
		claim(arch->getWireRouter()->getType() == PIN_MAPPER_INHERENT_WR, "Must select the Pin-mapper Inherent WireRouter (or forgo the independent wire routing stage) in the wire routing stage when using a joint pin-mapper/wire-router such as the Switch-Aware pin-mapper/wire-router.");

	// These checks are made in the wire_router
	//claim(arch->getWireRouter()->getNumHorizTracks() > 1, "Number of horizontal wire routing tracks must be greater than 1.");
	//claim(arch->getWireRouter()->getNumVertTracks() > 1, "Number of vertical wire routing tracks must be greater than 1.");
}

//////////////////////////////////////////////////////////////////////////////
// Checks if this routing method is compatible with the route analyzer methods.
// NOTE: Developers should add more checks as they create new synthesis methods.
//////////////////////////////////////////////////////////////////////////////
bool CompatChk::CanPerformRouteAnalysis(Router *r)
{
	if(r->getType() == FPPC_SEQUENTIAL_R || r->getType() == FPPC_PARALLEL_R)
		return false;
	else
		return true;
}

//////////////////////////////////////////////////////////////////////////////
// Checks if this routing method is compatible with the compact simulation
// visualization.
// NOTE: Developers should add more checks as they create new synthesis methods.
//////////////////////////////////////////////////////////////////////////////
bool CompatChk::CanPerformCompactSimulation(Router *r)
{
	return true;
}

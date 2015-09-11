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
 * Source: synthesis.cc															*
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
#include "../Headers/synthesis.h"

#include "../Headers/Util/file_out.h"
#include "../Headers/Util/file_in.h"
#include "../Headers/Util/analyze.h"

#include "../Headers/Scheduler/force_directed_list_scheduler.h"
#include "../Headers/Scheduler/grissom_fppc_path_scheduler.h"
#include "../Headers/Scheduler/list_scheduler_rt_eval.h"
#include "../Headers/Scheduler/grissom_fppc_scheduler.h"
#include "../Headers/Scheduler/genet_path_scheduler.h"
#include "../Headers/Scheduler/rickett_scheduler.h"
#include "../Headers/Scheduler/genet_scheduler.h"
#include "../Headers/Scheduler/list_scheduler.h"
#include "../Headers/Scheduler/path_scheduler.h"
#include "../Headers/Scheduler/scheduler.h"

#include "../Headers/Placer/grissom_fppc_left_edge_binder.h"
#include "../Headers/Placer/grissom_left_edge_binder.h"
#include "../Headers/Placer/grissom_path_binder.h"
#include "../Headers/Placer/kamer_ll_placer.h"
#include "../Headers/Placer/placer.h"

#include "../Headers/Router/grissom_fixed_place_map_router.h"
#include "../Headers/Router/grissom_fppc_parallel_router.h"
#include "../Headers/Router/grissom_fixed_place_router.h"
#include "../Headers/Router/grissom_fppc_sequential_router.h"
#include "../Headers/Router/roy_maze_router.h"
#include "../Headers/Router/bioroute_router.h"
#include "../Headers/Router/lee_router.h"
#include "../Headers/Router/cho_router.h"
#include "../Headers/Router/router.h"
#include "../Headers/Router/cdma_full_router.h"

#include "../Headers/WireRouter/grissom_enhanced_fppc_wire_router.h"
#include "../Headers/WireRouter/path_finder_wire_router.h"
#include "../Headers/WireRouter/yeh_wire_router.h"
#include "../Headers/WireRouter/wire_router.h"

#include "../Headers/PinMapper/indiv_addr_pin_mapper.h"
#include "../Headers/PinMapper/clique_pin_mapper.h"
#include "../Headers/PinMapper/power_clique_pin_mapper.h"
#include "../Headers/PinMapper/reliability_aware_pin_mapper.h"
#include "../Headers/PinMapper/switching_aware_combined_wire_router_pin_mapper.h"

///////////////////////////////////////////////////////////////
// Synthesis constructor. Initializes synthesis methods
///////////////////////////////////////////////////////////////
Synthesis::Synthesis(SchedulerType st, PlacerType pt, RouterType rt, bool performWash, ResourceAllocationType rat, PinMapType pmt, WireRouteType wrt, CompactionType ct, ProcessEngineType pet, ExecutionType et, DAG *assay, DmfbArch *dmfbArch)
{
	arch = dmfbArch;
	dag = assay;
	executionType = et;
	rModules = new vector<ReconfigModule *>();
	routes = new map<Droplet *, vector<RoutePoint *> *>();
	dirtyCells = new vector<vector<RoutePoint*> *>();
	tsBeginningCycle = new vector<unsigned long long>();
	pinActivations = new vector<vector<int> *>();

	scheduler = getNewSchedulingMethod(st);
	placer = getNewPlacementMethod(pt);
	router = getNewRoutingMethod(rt, arch);
	router->setWash(performWash);

	scheduler->setType(st);
	placer->setType(pt);
	placer->setPastSchedType(st);
	router->setType(rt);
	router->setPastSchedType(st);
	router->setPastPlacerType(pt);

	setWireRoutingMethod(wrt, arch);
	setPinMappingMethod(pmt, rat, arch);
	arch->getPinMapper()->setType(pmt);
	arch->getPinMapper()->setMapPreSched(); // Set Pin-mapping (if implemented)
	router->setRoutingParams(ct, pet);
}

///////////////////////////////////////////////////////////////////////////////////
// Deconstructor
///////////////////////////////////////////////////////////////////////////////////
Synthesis::~Synthesis()
{
	delete scheduler;
	delete placer;
	delete router;
	delete tsBeginningCycle;

	for (unsigned i = 0; i < rModules->size(); i++)
	{
		ReconfigModule *rm = rModules->back();
		rModules->pop_back();
		delete rm;
	}
	delete rModules;

	while (!routes->empty())
	{
		Droplet *d = routes->begin()->first;
		vector<RoutePoint *> *route = routes->begin()->second;
		routes->erase(d);
		delete d;
		while (!route->empty())
		{
			RoutePoint *rp = route->back();
			route->pop_back();
			delete rp;
		}
		delete route;
	}
	delete routes;

	while (!dirtyCells->empty())
	{
		vector<RoutePoint *> *v = dirtyCells->back();
		dirtyCells->pop_back();
		v->clear(); // Not new RoutePoints; deleted elsewhere
		delete v;
	}
	delete dirtyCells;

	while (!pinActivations->empty())
	{
		vector<int> *pins = pinActivations->back();
		pinActivations->pop_back();
		pins->clear();
		delete pins;
	}
	delete pinActivations;
}

///////////////////////////////////////////////////////////////////////////////////////////////
// Simply deletes and re-news the variables in synthesis.  This function is called in-between
// scheduling, placement and routing so that the interfaces (file input and output) can be
// properly used.  This is used to ensure that the scheduling, placement and routing stages
// are properly separated from each other.
///////////////////////////////////////////////////////////////////////////////////////////////
void Synthesis::deleteAndRenewVariables()
{
	delete arch;
	delete dag;
	delete tsBeginningCycle;
	for (unsigned i = 0; i < rModules->size(); i++)
	{
		ReconfigModule *rm = rModules->back();
		rModules->pop_back();
		delete rm;
	}
	delete rModules;
	while (!routes->empty())
	{
		Droplet *d = routes->begin()->first;
		vector<RoutePoint *> *route = routes->begin()->second;
		routes->erase(d);
		delete d;
		while (!route->empty())
		{
			RoutePoint *rp = route->back();
			route->pop_back();
			delete rp;
		}
		delete route;
	}
	delete routes;
	while (!pinActivations->empty())
	{
		vector<int> *pins = pinActivations->back();
		pinActivations->pop_back();
		pins->clear();
		delete pins;
	}


	arch = new DmfbArch();
	dag = new DAG();
	rModules = new vector<ReconfigModule *>();
	routes = new map<Droplet *, vector<RoutePoint *> *>();
	pinActivations = new vector<vector<int> *>();
	tsBeginningCycle = new vector<unsigned long long>();
}

///////////////////////////////////////////////////////////////////////////////////////////////
// This function calls the scheduilng, placement and routing functions to compute a design
// for the given assay and DMFB architecture.  Also performs timing calculations.
///////////////////////////////////////////////////////////////////////////////////////////////
void Synthesis::synthesizeDesign()
{
	stringstream fName;
	string dir = "Output/";
	// Set the frequency for the DAG based on the architecture frequency
	for (unsigned i = 0; i < dag->getAllNodes().size(); i++)
		dag->getAllNodes().at(i)->SetNumCycles( (unsigned)ceil((double)arch->getFreqInHz() * dag->getAllNodes().at(i)->GetNumSeconds()) );

	/////////////////////////////////////////////////////////
	// Pre-synthesis
	/////////////////////////////////////////////////////////
	if (executionType == SIM_EX || executionType == ALL_EX)
	{
		FileOut::WriteDagToFile(dag, "Output/0_DAG_to_SCHED.txt");
		fName.str("0_" + dag->getName());
		dag->OutputGraphFile(dir + fName.str(), true, true);
		cout << dag->getAllNodes().size() << " total nodes; " << dag->getNumNonIoNodes() << " non-I/O nodes." << endl;
	}

	/////////////////////////////////////////////////////////
	// Set Pin-mapping (if implemented)
	/////////////////////////////////////////////////////////
	//arch->getPinMapper()->setMapPreSched();

	/////////////////////////////////////////////////////////
	// Do compatability checks
	/////////////////////////////////////////////////////////
	CompatChk::PreScheduleChk(scheduler, arch, dag, true);

	/////////////////////////////////////////////////////////
	// Scheduling ///////////////////////////////////////////
	/////////////////////////////////////////////////////////
	if (scheduler->hasExecutableSynthMethod())
	{
		ElapsedTimer sTime("Scheduling Time");
		sTime.startTimer();
		scheduler->schedule(arch, dag);
		sTime.endTimer();
		sTime.printElapsedTime();

		/////////////////////////////////////////////////////////
		// Scheduling --> Placement Interface
		// NOTE: This is unnecessary, but is used to keep the
		// stages of synthesis properly separated.
		/////////////////////////////////////////////////////////
		if (executionType == SIM_EX || executionType == ALL_EX)
		{
			FileOut::WriteScheduledDagAndArchToFile(dag, arch, scheduler, "Output/1_SCHED_to_PLACE.txt");
			//deleteAndRenewVariables();
			//Util::ReadScheduledDagAndArchFromFile(dag, arch, placer, "Output/1_SCHED_to_PLACE.txt");
			fName.str("1_" + dag->getName() + "_Sched");
			dag->OutputGraphFile(dir + fName.str(), true, true);
		}

		/////////////////////////////////////////////////////////
		// Schedule analysis
		/////////////////////////////////////////////////////////
		FileOut::WriteStringToFile(Analyze::AnalyzeSchedule(dag, arch, scheduler), "Output/4_ANALYSIS_SCHEDULE.txt");
	}
	else
		cout << "Basic scheduling bypassed according to synthesis flow." << endl;

	/////////////////////////////////////////////////////////
	// Do compatability checks
	/////////////////////////////////////////////////////////
	CompatChk::PrePlaceChk(placer, arch, true);

	/////////////////////////////////////////////////////////
	// Placement ////////////////////////////////////////////
	/////////////////////////////////////////////////////////
	if (placer->hasExecutableSynthMethod())
	{
		ElapsedTimer pTime("Placement Time");
		pTime.startTimer();
		placer->place(arch, dag, rModules);
		pTime.endTimer();
		pTime.printElapsedTime();

		/////////////////////////////////////////////////////////
		// Placement --> Routing Interface
		// NOTE: This is unnecessary, but is used to keep the
		// stages of synthesis properly separated.
		/////////////////////////////////////////////////////////
		if (executionType == SIM_EX || executionType == ALL_EX)
		{
			FileOut::WritePlacedDagAndArchToFile(dag, arch, placer, rModules, "Output/2_PLACE_to_ROUTE.txt");
			//deleteAndRenewVariables();
			//Util::ReadPlacedDagAndArchFromFile(dag, arch, rModules, "Output/2_PLACE_to_ROUTE.txt");
			fName.str("2_" + dag->getName() + "_Placed");
			dag->OutputGraphFile(dir + fName.str(), true, true);
		}

		/////////////////////////////////////////////////////////
		// Placement analysis
		/////////////////////////////////////////////////////////
		FileOut::WriteStringToFile(Analyze::AnalyzePlacement(arch, rModules), "Output/4_ANALYSIS_PLACEMENT_WARNINGS_AND_FAILURES.txt");
	}
	else
		cout << "Basic placement bypassed according to synthesis flow." << endl;

	/////////////////////////////////////////////////////////
	// Do compatability checks
	/////////////////////////////////////////////////////////
	CompatChk::PreRouteChk(router, arch, true);

	/////////////////////////////////////////////////////////
	// Routing //////////////////////////////////////////////
	/////////////////////////////////////////////////////////
	if (router->hasExecutableSynthMethod())
	{
		ElapsedTimer rTime("Routing Time");
		rTime.startTimer();
		router->route(dag, arch, rModules, routes, pinActivations, tsBeginningCycle);
		rTime.endTimer();
		rTime.printElapsedTime();

		/////////////////////////////////////////////////////////
		// Compute droplet locations from pin-activations
		// (if required)
		/////////////////////////////////////////////////////////
		if (router->getDropLocationsFromSim())
		{
			ElapsedTimer simTime("Simulation Time");
			simTime.startTimer();
			router->simulateDropletMotion(routes, pinActivations);
			simTime.endTimer();
			simTime.printElapsedTime();
		}
		else
		{
			ElapsedTimer simTime("Pin-Compute From Droplet-Motion Time");
			simTime.startTimer();
			router->setPinActivationsFromDropletMotion(routes, pinActivations);
			simTime.endTimer();
			simTime.printElapsedTime();
		}
		router->computeDirtyCells(routes, dirtyCells);
		printRoutingStats(tsBeginningCycle, arch);

		/////////////////////////////////////////////////////////
		// Droplet concentration and route interference analysis
		/////////////////////////////////////////////////////////
		if (CompatChk::CanPerformRouteAnalysis(router))
		{
			FileOut::WriteStringToFile(Analyze::AnalyzeDropletConcentrationAndIO(dag, arch, routes), "Output/4_ANALYSIS_DROPLET_IO_AND_CONCENTRATIONS.txt");
			FileOut::WriteStringToFile(Analyze::AnalyzeRoutes(arch, routes), "Output/4_ANALYSIS_ROUTING_FAILURES.txt");
		}
	}
	else
		cout << "Basic droplet routing bypassed according to synthesis flow." << endl;

	/////////////////////////////////////////////////////////
	// Set Pin-mapping (if implemented)
	/////////////////////////////////////////////////////////
	ElapsedTimer pmTime("Pin-Mapping (Post Route) Time");
	pmTime.startTimer();
	arch->getPinMapper()->setMapPostRoute(pinActivations, routes);
	pmTime.endTimer();
	pmTime.printElapsedTime();

	/////////////////////////////////////////////////////////
	// Do compatibility checks
	/////////////////////////////////////////////////////////
	CompatChk::PreWireRouteChk(arch, true);

	/////////////////////////////////////////////////////////
	// Compute wire-routes from pin-mapping and pin-activations
	// (if required)
	/////////////////////////////////////////////////////////
	if (arch->getWireRouter()->hasExecutableSynthMethod())
	{
		ElapsedTimer wrTime("Wire-Routing Time");
		wrTime.startTimer();
		arch->getWireRouter()->computeWireRoutes(pinActivations, false);
		wrTime.endTimer();
		wrTime.printElapsedTime();
	}
	else
		cout << "Basic wire-routing bypassed according to synthesis flow." << endl;

	printWireRoutingStats(arch);

	/////////////////////////////////////////////////////////
	// Routing --> Output Interface
	// NOTE: Must run the WriteCompacted() function last b/c
	// it deletes the routes as it goes.
	/////////////////////////////////////////////////////////
	if (executionType == PROG_EX || executionType == ALL_EX)
	{
		FileOut::WriteDmfbProgramToFile(routes, "Output/3_ELEC_ACTIVATIONS_COORDS.mfprog");
		FileOut::WriteDmfbBinaryProgramToFile(arch, pinActivations, "Output/3_PIN_ACTIVATIONS_BINARY.mfprog");
	}
	if (executionType == SIM_EX || executionType == ALL_EX)
	{
		FileOut::WriteRoutedDagAndArchToFile(dag, arch, router, rModules, routes, dirtyCells, pinActivations, tsBeginningCycle, "Output/3_ROUTE_to_SIM.txt");
		if (CompatChk::CanPerformCompactSimulation(router))
			FileOut::WriteCompactedRoutesToFile(dag, arch, rModules, routes, tsBeginningCycle, "Output/3_COMPACT_ROUTE_to_SIM.txt");
	}
	FileOut::WriteHardwareFileWithWireRoutes(arch, dir + "4_HARDWARE_DESCRIPTION.txt", true);
}

///////////////////////////////////////////////////////////////////////////////////////
// Runs through the entire synthesis process of scheduling, placement and routing.
// Outputs intermediate files all along the way, as well as the final droplet output.
///////////////////////////////////////////////////////////////////////////////////////
void Synthesis::EntireFlow(string assayFile, string archFile, SchedulerType st, PlacerType pt, RouterType rt, bool performWash, ResourceAllocationType rat, PinMapType pmt, WireRouteType wrt, CompactionType ct, ProcessEngineType pet, ExecutionType et, int maxStorageDropsPerMod, int cellsBetweenModIR, int numHorizTracks, int numVertTracks)
{
	// Read Sequencing Graph DAG from file
	DAG *dag = FileIn::ReadDagFromFile(assayFile);

	// Read Architectural Description File
	DmfbArch * arch = FileIn::ReadDmfbArchFromFile(archFile);

	// Select Synthesis Methods and synthesize/create design
	Synthesis *syn = new Synthesis(st, pt, rt, performWash, rat, pmt, wrt, ct, pet, et, dag, arch);
	syn->getScheduler()->setMaxStoragePerModule(maxStorageDropsPerMod);
	syn->getPlacer()->setMaxStoragePerModule(maxStorageDropsPerMod);
	syn->getPlacer()->setHCellsBetweenModIR(cellsBetweenModIR);
	syn->getPlacer()->setVCellsBetweenModIR(cellsBetweenModIR);
	syn->getRouter()->setHCellsBetweenModIR(cellsBetweenModIR);
	syn->getRouter()->setVCellsBetweenModIR(cellsBetweenModIR);
	syn->getArch()->getWireRouter()->setNumTracksAndCreateModel(numHorizTracks, numVertTracks);
	syn->synthesizeDesign();

	delete syn;
	delete dag;
	delete arch;
}

///////////////////////////////////////////////////////////////////////////////////////
// Takes in a assay (in the form of a DAG) and an architecture file, schedules it via
// the specified scheduler, and outputs a scheduled DAG.
///////////////////////////////////////////////////////////////////////////////////////
void Synthesis::Schedule(string inputDagFile, string inputArchFile, string outputFile, SchedulerType st, ResourceAllocationType rat, PinMapType pmt, int maxStorageDropsPerMod)
{
	/////////////////////////////////////////////////////////
	// Pre-synthesis
	/////////////////////////////////////////////////////////
	DAG *dag = FileIn::ReadDagFromFile(inputDagFile);

	// Utility code for DAG conversions
	/*dag->ConvertMixSplitsToDilutes();
	dag->ConvertMixesToDiluteWithSecondDropletOutputting();
	dag->OutputGraphFile("Output/B3_ProteinDilute", true, true);
	FileOut::WriteDagToFile(dag, "Output/B3_ProteinDilute.txt");
	exit(0);*/

	DmfbArch *arch = FileIn::ReadDmfbArchFromFile(inputArchFile);
	setPinMappingMethod(pmt, rat, arch);

	// Set the frequency for the DAG based on the architecture frequency
	for (unsigned i = 0; i < dag->getAllNodes().size(); i++)
		dag->getAllNodes().at(i)->SetNumCycles( (unsigned)ceil((double)arch->getFreqInHz() * dag->getAllNodes().at(i)->GetNumSeconds()) );

	string dir = outputFile.substr(0, outputFile.find_last_of("/")+1);
	FileOut::WriteDagToFile(dag, dir + "0_DAG_to_SCHED.txt");
	stringstream fName;
	fName.str("0_" + dag->getName());
	dag->OutputGraphFile(dir + fName.str(), true, true);
	cout << dag->getAllNodes().size() << " total nodes; " << dag->getNumNonIoNodes() << " non-I/O nodes." << endl;

	/////////////////////////////////////////////////////////
	// Set Pin-mapping (if implemented)
	/////////////////////////////////////////////////////////
	arch->getPinMapper()->setMapPreSched();

	/////////////////////////////////////////////////////////
	// Set parameters and do compatability checks
	/////////////////////////////////////////////////////////
	Scheduler *scheduler = getNewSchedulingMethod(st);
	scheduler->setType(st);
	scheduler->setMaxStoragePerModule(maxStorageDropsPerMod);
	CompatChk::PreScheduleChk(scheduler, arch, dag, false);

	/////////////////////////////////////////////////////////
	// Scheduling ///////////////////////////////////////////
	/////////////////////////////////////////////////////////
	ElapsedTimer sTime("Scheduling Time");
	sTime.startTimer();
	scheduler->schedule(arch, dag);
	sTime.endTimer();
	sTime.printElapsedTime();

	/////////////////////////////////////////////////////////
	// Scheduling --> Placement Interface
	/////////////////////////////////////////////////////////
	FileOut::WriteScheduledDagAndArchToFile(dag, arch, scheduler, outputFile);
	fName.str("1_" + dag->getName() + "_Sched");
	dag->OutputGraphFile(dir + fName.str(), true, true);

	/////////////////////////////////////////////////////////
	// Schedule analysis
	/////////////////////////////////////////////////////////
	FileOut::WriteStringToFile(Analyze::AnalyzeSchedule(dag, arch, scheduler), "Output/4_ANALYSIS_SCHEDULE.txt");

	/////////////////////////////////////////////////////////
	// Cleanup
	/////////////////////////////////////////////////////////
	delete scheduler;
	delete dag;
	delete arch;
}

///////////////////////////////////////////////////////////////////////////////////////
// Takes in a scheduled DAG file, places it via the specified placer, and outputs a
// placed DAG file.
///////////////////////////////////////////////////////////////////////////////////////
void Synthesis::Place(string inputFile, string outputFile, PlacerType pt, int cellsBetweenModIR)
{
	/////////////////////////////////////////////////////////
	// Create Placer
	/////////////////////////////////////////////////////////
	Placer *placer = getNewPlacementMethod(pt);
	placer->setType(pt);
	placer->setHCellsBetweenModIR(cellsBetweenModIR);
	placer->setVCellsBetweenModIR(cellsBetweenModIR);

	/////////////////////////////////////////////////////////
	// Scheduling --> Placement Interface
	/////////////////////////////////////////////////////////
	DAG *dag = new DAG();
	DmfbArch *arch = new DmfbArch();
	//setPinMappingMethod(pmt, INHERIT_RA, arch);
	vector<ReconfigModule *> *rModules = new vector<ReconfigModule *>();
	FileIn::ReadScheduledDagAndArchFromFile(dag, arch, placer, inputFile);

	string dir = outputFile.substr(0, outputFile.find_last_of("/")+1);
	stringstream fName;
	fName.str("1_" + dag->getName() + "_Sched");
	dag->OutputGraphFile(dir + fName.str(), true, true);

	/////////////////////////////////////////////////////////
	// Do compatability checks
	/////////////////////////////////////////////////////////
	CompatChk::PrePlaceChk(placer, arch, false);

	/////////////////////////////////////////////////////////
	// Placement ////////////////////////////////////////////
	/////////////////////////////////////////////////////////
	ElapsedTimer pTime("Placement Time");
	pTime.startTimer();
	placer->place(arch, dag, rModules);
	pTime.endTimer();
	pTime.printElapsedTime();

	/////////////////////////////////////////////////////////
	// Placement --> Routing Interface
	/////////////////////////////////////////////////////////
	FileOut::WritePlacedDagAndArchToFile(dag, arch, placer, rModules, outputFile);
	fName.str("2_" + dag->getName() + "_Placed");
	dag->OutputGraphFile(dir + fName.str(), true, true);

	/////////////////////////////////////////////////////////
	// Placement analysis
	/////////////////////////////////////////////////////////
	FileOut::WriteStringToFile(Analyze::AnalyzePlacement(arch, rModules), "Output/4_ANALYSIS_PLACEMENT_WARNINGS_AND_FAILURES.txt");

	/////////////////////////////////////////////////////////
	// Cleanup
	/////////////////////////////////////////////////////////
	delete placer;
	delete dag;
	delete arch;
	for (unsigned i = 0; i < rModules->size(); i++)
	{
		ReconfigModule *rm = rModules->back();
		rModules->pop_back();
		delete rm;
	}
	delete rModules;
}

///////////////////////////////////////////////////////////////////////////////////////
// Takes in a placed DAG file, routes via the specified router, and then outputs a
// simulation
///////////////////////////////////////////////////////////////////////////////////////
void Synthesis::Route(string inputFile, RouterType rt, bool performWash, CompactionType ct, ProcessEngineType pet, ExecutionType et)
{
	/////////////////////////////////////////////////////////
	// Create Router
	/////////////////////////////////////////////////////////
	DAG *dag = new DAG();
	DmfbArch *arch = new DmfbArch();
	Router *router	= getNewRoutingMethod(rt, arch);
	router->setType(rt);
	router->setWash(performWash);
	router->setRoutingParams(ct, pet);

	/////////////////////////////////////////////////////////
	// Placement --> Routing Interface
	/////////////////////////////////////////////////////////
	vector<ReconfigModule *> *rModules = new vector<ReconfigModule *>();
	map<Droplet *, vector<RoutePoint *> *> *routes = new map<Droplet *, vector<RoutePoint *> *>();
	vector<vector<RoutePoint*> *> *dirtyCells = new vector<vector<RoutePoint*> *>();
	vector<vector<int> *> *pinActivations = new vector<vector<int> *>();
	vector<unsigned long long> *tsBeginningCycle = new vector<unsigned long long>();
	FileIn::ReadPlacedDagAndArchFromFile(dag, arch, router, rModules, inputFile);
	string dir = inputFile.substr(0, inputFile.find_last_of("/")+1);
	stringstream fName;
	fName.str("2_" + dag->getName() + "_Placed");
	dag->OutputGraphFile(dir + fName.str(), true, true);

	/////////////////////////////////////////////////////////
	// Do compatability checks
	/////////////////////////////////////////////////////////
	CompatChk::PreRouteChk(router, arch, false);

	/////////////////////////////////////////////////////////
	// Routing //////////////////////////////////////////////
	/////////////////////////////////////////////////////////
	router->preRoute(arch);
	ElapsedTimer rTime("Routing Time");
	rTime.startTimer();
	router->route(dag, arch, rModules, routes, pinActivations, tsBeginningCycle);
	rTime.endTimer();
	rTime.printElapsedTime();

	/////////////////////////////////////////////////////////
	// Compute droplet locations from pin-activations
	// (if required)
	/////////////////////////////////////////////////////////
	if (router->getDropLocationsFromSim())
	{
		ElapsedTimer simTime("Simulation Time");
		simTime.startTimer();
		router->simulateDropletMotion(routes, pinActivations);
		simTime.endTimer();
		simTime.printElapsedTime();
	}
	else
	{
		ElapsedTimer simTime("Pin-Compute From Droplet-Motion Time");
		simTime.startTimer();
		router->setPinActivationsFromDropletMotion(routes, pinActivations);
		simTime.endTimer();
		simTime.printElapsedTime();
	}
	router->computeDirtyCells(routes, dirtyCells);
	printRoutingStats(tsBeginningCycle, arch);

	/////////////////////////////////////////////////////////
	// Droplet concentration and interference analysis
	/////////////////////////////////////////////////////////
	if (CompatChk::CanPerformRouteAnalysis(router))
	{
		FileOut::WriteStringToFile(Analyze::AnalyzeDropletConcentrationAndIO(dag, arch, routes), "Output/4_ANALYSIS_DROPLET_IO_AND_CONCENTRATIONS.txt");
		FileOut::WriteStringToFile(Analyze::AnalyzeRoutes(arch, routes), "Output/4_ANALYSIS_ROUTING_FAILURES.txt");
	}

	/////////////////////////////////////////////////////////
	// Set Pin-mapping (if implemented)
	/////////////////////////////////////////////////////////
	ElapsedTimer pmTime("Pin-Mapping (Post Route) Time");
	pmTime.startTimer();
	arch->getPinMapper()->setMapPostRoute(pinActivations, routes);
	pmTime.endTimer();
	pmTime.printElapsedTime();

	/////////////////////////////////////////////////////////
	// Routing --> Output Interface
	// NOTE: Must run the WriteCompacted() function last b/c
	// it deletes the routes as it goes.
	/////////////////////////////////////////////////////////
	FileOut::WriteHardwareFileWithWireRoutes(arch, dir + "4_HARDWARE_DESCRIPTION.txt", arch->getPinMapper()->getIsCombinedWireRouter()); // May be overwritten w/ better version later if wire-router called
	if (et == PROG_EX || et == ALL_EX)
	{
		FileOut::WriteDmfbProgramToFile(routes, "Output/3_ELEC_ACTIVATIONS_COORDS.mfprog");
		FileOut::WriteDmfbBinaryProgramToFile(arch, pinActivations, "Output/3_PIN_ACTIVATIONS_BINARY.mfprog");
	}
	if (et == SIM_EX || et == ALL_EX)
	{
		FileOut::WriteRoutedDagAndArchToFile(dag, arch, router, rModules, routes, dirtyCells, pinActivations, tsBeginningCycle, "Output/3_ROUTE_to_SIM.txt");
		FileOut::WriteCompactedRoutesToFile(dag, arch, rModules, routes, tsBeginningCycle, "Output/3_COMPACT_ROUTE_to_SIM.txt");
	}

	/////////////////////////////////////////////////////////
	// Cleanup
	/////////////////////////////////////////////////////////
	delete router;
	delete arch;
	delete dag;
	delete tsBeginningCycle;
	for (unsigned i = 0; i < rModules->size(); i++)
	{
		ReconfigModule *rm = rModules->back();
		rModules->pop_back();
		delete rm;
	}
	delete rModules;
	while (!routes->empty())
	{
		Droplet *d = routes->begin()->first;
		vector<RoutePoint *> *route = routes->begin()->second;
		routes->erase(d);
		delete d;
		while (!route->empty())
		{
			RoutePoint *rp = route->back();
			route->pop_back();
			delete rp;
		}
		delete route;
	}
	delete routes;

	while (!dirtyCells->empty())
	{
		vector<RoutePoint *> *v = dirtyCells->back();
		dirtyCells->pop_back();
		v->clear();
		delete v;
	}
	delete dirtyCells;

	while (!pinActivations->empty())
	{
		vector<int> *pins = pinActivations->back();
		pinActivations->pop_back();
		pins->clear();
		delete pins;
	}
	delete pinActivations;
}

void Synthesis::WireRoute(string inputFile, WireRouteType wrt, int numHorizTracks, int numVertTracks)
{
	/////////////////////////////////////////////////////////
	// Synthesis --> Wire-Routing Interface
	/////////////////////////////////////////////////////////
	DmfbArch *arch = new DmfbArch();
	// At this point, it doesn't really matter what pin-mapping type we have b/c we will
	// just read in the pin-assignment from prior sched/place/routing. Just create a
	// new pin-mapper so we can read in the pin-mapping.
	setPinMappingMethod(INDIVADDR_PM, INHERIT_RA, arch);
	vector<vector<int> *> *pinActivations = new vector<vector<int> *>();
	FileIn::ReadRoutedSimPinsAndArchFromFile(arch, pinActivations, inputFile);
	string dir = inputFile.substr(0, inputFile.find_last_of("/")+1);


	/////////////////////////////////////////////////////////
	// Set parameters and do compatability checks
	/////////////////////////////////////////////////////////
	setWireRoutingMethod(wrt, arch);
	arch->getWireRouter()->setNumTracksAndCreateModel(numHorizTracks, numVertTracks);
	CompatChk::PreWireRouteChk(arch, false);

	/////////////////////////////////////////////////////////
	// Compute wire-routes from pin-mapping and pin-activations
	// (if required)
	/////////////////////////////////////////////////////////
	if (arch->getWireRouter()->hasExecutableSynthMethod())
	{
		ElapsedTimer wrTime("Wire-Routing Time");
		wrTime.startTimer();
		arch->getWireRouter()->computeWireRoutes(pinActivations, false);
		wrTime.endTimer();
		wrTime.printElapsedTime();

		printWireRoutingStats(arch);

		/////////////////////////////////////////////////////////
		// Hardware Description file (with wire-router)
		/////////////////////////////////////////////////////////
		FileOut::WriteHardwareFileWithWireRoutes(arch, dir + "4_HARDWARE_DESCRIPTION.txt", true);
	}
	else if (wrt == NONE_WR)
		cout << "Basic wire-routing bypassed according to synthesis flow." << endl;

	/////////////////////////////////////////////////////////
	// Cleanup
	/////////////////////////////////////////////////////////
	delete arch;
	while (!pinActivations->empty())
	{
		vector<int> *pins = pinActivations->back();
		pinActivations->pop_back();
		pins->clear();
		delete pins;
	}
	delete pinActivations;
}

///////////////////////////////////////////////////////////////////////////////////////
// Initializes a new scheduler
///////////////////////////////////////////////////////////////////////////////////////
Scheduler * Synthesis::getNewSchedulingMethod(SchedulerType st)
{
	if (st == LIST_S)
		return new ListScheduler();
	else if (st == PATH_S)
		return new PathScheduler();
	else if (st == GENET_S)
		return new GenetScheduler();
	else if (st == GENET_PATH_S)
		return new GenetPathScheduler();
	else if (st == RICKETT_S)
		return new RickettScheduler();
	else if (st == FD_LIST_S)
		return new FDLScheduler();
	else if (st == FPPC_S)
		return new GrissomFppcScheduler();
	else if (st == FPPC_PATH_S)
		return new GrissomFppcPathScheduler();
	else if (st == RT_EVAL_LIST_S)
		return new RealTimeEvalListScheduler();
	else
		claim(false, "No valid scheduler type was specified.");
}
///////////////////////////////////////////////////////////////////////////////////////
// Initializes a new placer
///////////////////////////////////////////////////////////////////////////////////////
Placer * Synthesis::getNewPlacementMethod(PlacerType pt)
{
	if (pt == GRISSOM_LE_B)
		return new GrissomLEBinder();
	else if (pt == GRISSOM_PATH_B)
		return new GrissomPathBinder();
	else if (pt == KAMER_LL_P)
		return new KamerLlPlacer();
	else if (pt == FPPC_LE_B)
		return new GrissomFppcLEBinder();
	else
		claim(false, "No valid placement type was specified.");
}
///////////////////////////////////////////////////////////////////////////////////////
// Initializes a new router
///////////////////////////////////////////////////////////////////////////////////////
Router * Synthesis::getNewRoutingMethod(RouterType rt, DmfbArch *arch)
{
	if (rt == GRISSOM_FIX_R)
		return new GrissomFixedPlaceRouter(arch);
	else if (rt == GRISSOM_FIX_MAP_R)
		return new GrissomFixedPlaceMapRouter(arch);
	else if (rt == ROY_MAZE_R)
		return new RoyMazeRouter(arch);
	else if (rt == BIOROUTE_R)
		return new BioRouter(arch);
	else if (rt == FPPC_SEQUENTIAL_R)
		return new GrissomFppcSequentialRouter(arch);
	else if (rt == FPPC_PARALLEL_R)
		return new GrissomFppcParallelRouter(arch);
	else if (rt == CHO_R)
		return new ChoRouter(arch);
	else if (rt == LEE_R)
		return new LeeRouter(arch);
	else if (rt == CDMA_FULL_R)
		return new CDMAFullRouter(arch);
	else
		claim(false, "No valid router type was specified.");
}

///////////////////////////////////////////////////////////////////////////////////////
// Initializes a new router
///////////////////////////////////////////////////////////////////////////////////////
void Synthesis::setWireRoutingMethod(WireRouteType wrt, DmfbArch *arch)
{
	WireRouter *wr;
	if (wrt == PATH_FINDER_WR)
		wr = new PathFinderWireRouter(arch);
	else if (wrt == YEH_WR)
		wr = new YehWireRouter(arch);
	else if (wrt == ENHANCED_FPPC_WR)
		wr = new EnhancedFPPCWireRouter(arch);
	else if (wrt == NONE_WR || PIN_MAPPER_INHERENT_WR)
	{
		wr = new WireRouter(arch);
		wr->setType(wrt);
		wr->setHasExecutableSynthMethod(false);
	}
	else
		claim(false, "No valid wire-router type was specified.");

	wr->setArch(arch); // Shouldn't be necessary, but 'arch' is not "sticking" in the constructor
	arch->setWireRouter(wr);
}
///////////////////////////////////////////////////////////////////////////////////////
// Initializes a new pin-mapper
///////////////////////////////////////////////////////////////////////////////////////
void Synthesis::setPinMappingMethod(PinMapType pmt, ResourceAllocationType rat, DmfbArch *arch)
{
	PinMapper *pm;
	if (pmt == INDIVADDR_PM)
		pm = new IndivAddrPinMapper(arch);
	else if (pmt == CLIQUE_PM)
		pm = new CliquePinMapper(arch);
	else if (pmt == POWER_PM)
		pm = new PowerAwarePinMapper(arch);
	else if (pmt == RELY_PM)
		pm = new ReliabilityAwarePinMapper(arch);
	else if (pmt == SWITCH_PM)
		pm = new SwitchingAwarePMWR(arch);
	else if (pmt == ORIGINAL_FPPC_PM || pmt == ENHANCED_FPPC_PIN_OPT_PM || pmt == ENHANCED_FPPC_ROUTE_OPT_PM)
		pm = new GrissomFppcPinMapper(arch);
	else
		claim(false, "No valid pin-mapper type was specified.");

	pm->setType(pmt);
	pm->setArch(arch); // Shouldn't be necessary, but 'arch' is not "sticking" in the constructor
	pm->setResAllocType(rat);
	arch->setPinMapper(pm);
}

///////////////////////////////////////////////////////////////////////////////////////
// Prints number of cycles spent routing
///////////////////////////////////////////////////////////////////////////////////////
void Synthesis::printRoutingStats(vector<unsigned long long> *tsBeginningCycle, DmfbArch *arch)
{
	unsigned long long routingCycles = 0;
	for (unsigned i = 0; i < tsBeginningCycle->size(); i++)
	{
		unsigned long long lastTSEnd = 0;
		if (i > 0)
			lastTSEnd = tsBeginningCycle->at(i-1) + (int)(arch->getFreqInHz()*arch->getSecPerTS());
		routingCycles += (tsBeginningCycle->at(i)-lastTSEnd);
	}
	cout << "Number of cycles spent routing: " << routingCycles << " (" << (double)((double)routingCycles/arch->getFreqInHz()) << "s @" << arch->getFreqInHz() << "Hz)" << endl;
}

///////////////////////////////////////////////////////////////////////////////////////
// Prints the total length of wiring on each layer in wire-routing grid units
///////////////////////////////////////////////////////////////////////////////////////
void Synthesis::printWireRoutingStats(DmfbArch *arch)
{
	// Print the usage of pins and electrodes
	int elecCount = 0;
	set<int> pins;
	for (int x = 0; x < arch->getPinMapper()->getPinMapping()->size(); x++)
	{
		for (int y = 0; y < arch->getPinMapper()->getPinMapping()->at(x)->size(); y++)
		{

			if (arch->getPinMapper()->getPinMapping()->at(x)->at(y) >= 0)
			{
				pins.insert(arch->getPinMapper()->getPinMapping()->at(x)->at(y));
				elecCount++;
			}
		}
	}
	cout << "DMFB consumes area of " << arch->getNumCellsX() << "x" << arch->getNumCellsY() << endl;
	cout << "Total number of electrodes wired: " << elecCount << endl;
	cout << "Total number of external pins: " << pins.size() << endl;

	// Wire-length Statistics (prints #of electrode lengths of total wire per layer and total DMFB)
	/*vector<vector<WireSegment *> *> * wires = arch->getWireRouter()->getWireRoutesPerPin();
	map<int, double> layerWireMap;

	// Compute lengths per layer
	int maxLayer = -1;
	for (int i = 0; i < wires->size(); i++)
	{
		vector<WireSegment *> *wire = wires->at(i);

		for (int j = 0; j < wire->size(); j++)
		{
			WireSegment *ws = wire->at(j);

			if (ws->getLayerNum() > maxLayer)
				maxLayer = ws->getLayerNum();

			if (layerWireMap.find(ws->getLayerNum()) == layerWireMap.end())
				layerWireMap[ws->getLayerNum()] = 0;

			layerWireMap[ws->getLayerNum()] += ws->getLengthInWireCells();
		}
	}

	// Output results
	double sum = 0;
	cout << "Route Lengths per Layer:" << endl;
	for (int i = 0; i <= maxLayer; i++)
	{
		double gridWidths = (double)arch->getWireRouter()->getModel()->getTileGridSize(); // Number of wire-lengths in a tile
		cout << "Layer " << i+1 << ": " << layerWireMap[i] / gridWidths << " electrodes" << endl;
		sum += layerWireMap[i] / gridWidths;
	}
	cout << "Total Length: " << sum << " electrodes" << endl;*/

}

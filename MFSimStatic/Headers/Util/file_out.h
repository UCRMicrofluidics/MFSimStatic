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
 * Name: File Output															*
 *																				*
 * Details: This class handles all file output for synthesis interface files.	*
 * Simply pass it the necessary classes that were modified by the synthesis		*
 * methods and it will write the file to output to be saved or used later by	*
 * the simulator or the Java visualizer. Also creates other output files.		*
 *																				*
 * Changing the output of any file will cause issues if not done properly. For	*
 * instance, if you change the information in the routing output file, you will	*
 * also need to change the way the Java Visualizer reads that file or the 		*
 * visualizer may break and/or the way files are read in in file_in.h.			*
 *-----------------------------------------------------------------------------*/
#ifndef _FILE_OUT_H
#define _FILE_OUT_H

using namespace std;
#include "../Models/dmfb_arch.h"
#include "../Scheduler/scheduler.h"
#include "../Placer/placer.h"
#include "../Router/router.h"
#include "../command_line.h"
#include "../synthesis.h"
#include <fstream>
#include <string>
#include "sort.h"
#include "../Models/dag.h"

class Scheduler;
class Placer;
class Router;
class DmfbArch;

class FileOut
{
	public:
		// Constructors
		FileOut();
		virtual ~FileOut();

		// String Methods
		static void TokenizeString(string str, string token, vector<string> *dest);
		static void ParseLine(string line, string *tag, vector<string> *parameters);

		// File Output Methods
		static void WriteDagToFile(DAG *dag, string fileName);
		static void WriteScheduledDagAndArchToFile(DAG *dag, DmfbArch *arch,  Scheduler *scheduler, string fileName);
		static void WritePlacedDagAndArchToFile(DAG *dag, DmfbArch *arch, Placer *placer, vector<ReconfigModule *> *rModules, string fileName);
		static void WriteRoutedDagAndArchToFile(DAG *dag, DmfbArch *arch, Router *router, vector<ReconfigModule *> *rModules, map<Droplet *, vector<RoutePoint *> *> *routes, vector<vector<RoutePoint*> *> *dirtyCells, vector<vector<int> *> *pinActivations, vector<unsigned long long> *tsBeginningCycle, string fileName);
		static void WriteCompactedRoutesToFile(DAG *dag, DmfbArch *arch, vector<ReconfigModule *> *rModules, map<Droplet *, vector<RoutePoint *> *> *routes, vector<unsigned long long> *tsBeginningCycle, string fileName);
		static void WriteDmfbProgramToFile(map<Droplet *, vector<RoutePoint *> *> *routes, string fileName);
		static void WriteDmfbBinaryProgramToFile(DmfbArch *arch, vector<vector<int> *> *pinActivations, string fileName);
		static void WriteHardwareFileWithWireRoutes(DmfbArch *arch, string fileName, bool includeWireRouting);
		static void WriteInputtableDmfbArchToFile(DmfbArch *arch, string fileName);
		static void WriteDropletConcentrationStudyToFile(DAG *dag, DmfbArch *arch, map<Droplet *, vector<RoutePoint *> *> *routes, string fileName);
		static void WriteStringToFile(string s, string fileName);
		static void WriteArchForWireRouting(DmfbArch *arch, string fileName);

		// Enumeration/Key/Description Conversion Methods
		static string GetOperationString(OperationType ot);
		static string GetKeyFromSchedType(SchedulerType sType);
		static string GetKeyFromPlaceType(PlacerType pType);
		static string GetKeyFromRouteType(RouterType rType);
		static string GetKeyFromResourceAllocationType(ResourceAllocationType raType);
		static string GetKeyFromPinMapType(PinMapType pmType);
		static string GetKeyFromWrType(WireRouteType wrType);
		static string GetKeyFromCompType(CompactionType cType);
		static string GetKeyFromPeType(ProcessEngineType peType);
		static string GetKeyFromEtType(ExecutionType etType);
};
#endif

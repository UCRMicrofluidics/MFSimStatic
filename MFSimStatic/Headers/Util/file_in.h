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
 * Name: File Input																*
 *																				*
 * Details: This class handles all file input for the synthesis interface files *
 * and some other files. It reads interface files into the necessary data		*
 * structures to be used directly by the synthesis methods.						*
 *-----------------------------------------------------------------------------*/
#ifndef _FILE_IN_H
#define _FILE_IN_H

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

class FileIn
{
	public:
		// Constructors
		FileIn();
		virtual ~FileIn();

		// String Methods
		static void TokenizeString(string str, string token, vector<string> *dest);
		static void ParseLine(string line, string *tag, vector<string> *parameters);
		static string GetLine(ifstream *ifs);



		// File Input Methods
		static DAG *ReadDagFromFile(string fileName);
		static DmfbArch *ReadDmfbArchFromFile(string fileName);
		static void ReadScheduledDagAndArchFromFile(DAG *dag, DmfbArch *arch, Placer *placer, string fileName);
		static void ReadPlacedDagAndArchFromFile(DAG *dag, DmfbArch *arch, Router *router, vector<ReconfigModule *> *rModules, string fileName);
		static void ReadRoutedSimPinsAndArchFromFile(DmfbArch *arch, vector<vector<int> *> *pinActivations, string fileName);

		// Enumeration/Key/Description Conversion Methods
		static OperationType GetOpTypeFromString(string ot);
		static SchedulerType GetTypeFromSchedKey(string sKey);
		static PlacerType GetTypeFromPlaceKey(string pKey);
		static RouterType GetTypeFromRouteKey(string rKey);
		static ResourceAllocationType GetTypeFromResourceAllocationKey(string raKey);
		static PinMapType GetTypeFromPinMapKey(string pmKey);
		static WireRouteType GetTypeFromWrKey(string wrKey);
		static CompactionType GetTypeFromCompKey(string cKey);
		static ProcessEngineType GetTypeFromPeKey(string peKey);
		static ExecutionType GetTypeFromEtKey(string etKey);
};
#endif

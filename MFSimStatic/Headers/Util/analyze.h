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
 * Name: Analyze																*
 *																				*
 * Details: This class performs analysis and does "correctness" tests on the	*
 * results of synthesis to ensure that valid simulations have been generated	*
 * and that all the internal rules are being followed to produce valid output.	*
 *-----------------------------------------------------------------------------*/
#ifndef _ANALYZE_H
#define _ANALYZE_H

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

class Analyze
{
	public:
		// Constructors
		Analyze();
		virtual ~Analyze();

		// String Methods
		static string GetFormattedIOCell(IoPort *port, DmfbArch *arch);
		static string GetFormattedCell(RoutePoint *rp);
		static string GetFormattedCell(int x, int y);

		// Analysis
		static string AnalyzeSchedule(DAG *dag, DmfbArch *arch, Scheduler *scheduler);
		static string AnalyzePlacement(DmfbArch *arch, vector<ReconfigModule *> *rModules);
		static string AnalyzeRoutes(DmfbArch *arch, map<Droplet *, vector<RoutePoint *> *> *routes);
		static string AnalyzeDropletConcentrationAndIO(DAG *dag, DmfbArch *arch, map<Droplet *, vector<RoutePoint *> *> *routes);

		static bool ValidRoutingMove(RoutePoint *rpStart, RoutePoint *rpEnd);
		static bool DoesInterfere(RoutePoint *rp1, RoutePoint *rp2);
};
#endif

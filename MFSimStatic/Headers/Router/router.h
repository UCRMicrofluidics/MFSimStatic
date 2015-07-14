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
 * Type: Router																	*
 * Name: General Base Router													*
 *																				*
 * Not inferred or detailed in any publications									*
 * 																				*
 * Details: This is a base router class which contains some basic members		*
 * and functions useful to all routers.  It must be inherited from when			*
 * creating a new router in order to keep the structure of the simulator		*
 * and to keep the code clean and modular.										*
 *-----------------------------------------------------------------------------*/
#ifndef ROUTER_H_
#define ROUTER_H_

#include "../Models/dmfb_arch.h"
#include "../Models/droplet.h"
#include "../Resources/structs.h"
#include "../Testing/claim.h"
#include "../Util/sort.h"
#include "../Util/analyze.h"
#include <vector>
#include <map>
#include <algorithm>

class ReconfigModule;
class DAG;

class Router
{
	public:
		// Constructors
		Router();
		Router(DmfbArch *dmfbArch);
		virtual ~Router();

		// Methods
		virtual void preRoute(DmfbArch *arch);
		virtual void route(DAG *dag, DmfbArch *arch, vector<ReconfigModule *> *rModules, map<Droplet *, vector<RoutePoint *> *> *routes, vector<vector<int> *> *pinActivations, vector<unsigned long long> *tsBeginningCycle);
		virtual void postRoute();
		void setRoutingParams(CompactionType ct, ProcessEngineType pet);
		void simulateDropletMotion(map<Droplet *, vector<RoutePoint *> *> *routes, vector<vector<int> *> *pinActivations);
		void setPinActivationsFromDropletMotion(map<Droplet *, vector<RoutePoint *> *> *routes, vector<vector<int> *> *pinActivations);
		void computeDirtyCells(map<Droplet *, vector<RoutePoint *> *> *routes, vector<vector<RoutePoint*> *> *dirtyCells);

		// Getters/Setters
		int getHCellsBetweenModIR() { return hCellsBetweenModIR; }
		void setHCellsBetweenModIR(int cells) { hCellsBetweenModIR = cells; }
		int getVCellsBetweenModIR() { return vCellsBetweenModIR; }
		void setVCellsBetweenModIR(int cells) { vCellsBetweenModIR = cells; }
		virtual bool getDropLocationsFromSim() { return false; } // True if droplet routes/locations are to be calculated after the routing stage; False if the locations have already been computed by the router
		SchedulerType getPastSchedType() { return pastSchedType; }
		PlacerType getPastPlacerType() { return pastPlacerType; }
		RouterType getType() { return type; }
		//void setCompactionType(CompactionType ct) { compactionType = ct; }
		CompactionType getCompactionType() { return compactionType; }
		//void setProcEngineType(ProcessEngineType pet) { processEngineType = pet; }
		ProcessEngineType getProcEngineType() { return processEngineType; }
		bool hasExecutableSynthMethod() { return hasExecutableSyntesisMethod; }
		void setHasExecutableSynthMethod(bool hasMethod) { hasExecutableSyntesisMethod = hasMethod; }
		bool performWash() { return performDmfbWash; }


		void setPastSchedType(SchedulerType st) { pastSchedType = st; }
		void setPastPlacerType(PlacerType pt) { pastPlacerType = pt; }
		void setType(RouterType rt) { type = rt; }
		void setWash(bool wash) { performDmfbWash = wash; }

	protected:
		// Methods
		int dropletOccupyingCell(int x, int y, map<Droplet *, vector<RoutePoint *> *> *routes, vector<vector<RoutePoint *> *> *subRoutes, vector<Droplet *> *subDrops);
		void compactRoutes(vector<Droplet *> *subDrops, vector<vector<RoutePoint *> *> *subRoutes);
		void compactRoutesWithBegStalls(vector<vector<RoutePoint *> *> *subRoutes);
		void compactRoutesWithMidStalls(vector<vector<RoutePoint *> *> *subRoutes);
		void compactRoutesWithDynamicProgramming(vector<Droplet *> *subDrops, vector<vector<RoutePoint *> *> *subRoutes);
		void decompact(vector<vector<RoutePoint *> *> *subRoutes);
		void printRoutes(vector<vector<RoutePoint *> *> *routes);
		bool doesInterfere(RoutePoint *r1, RoutePoint *r2);
		bool hasExecutableSyntesisMethod; // Tells if method contains code to execute, or if it is just a shell

		// Members
		ExecutionType executionType;
		CompactionType compactionType;
		ProcessEngineType processEngineType;
		DmfbArch *arch;
		unsigned long long startingTS; // The TS that is getting ready to begin (droplets are being routed to)
		unsigned long long cyclesPerTS;
		unsigned long long routeCycle;
		unsigned long long cycle;
		int hCellsBetweenModIR; // # of cells, on x-axis, between module interference regions; should be non-negative
		int vCellsBetweenModIR; // # of cells, on y-axis, between module interference regions; should be non-negative
		SchedulerType pastSchedType;
		PlacerType pastPlacerType;
		RouterType type;
		bool performDmfbWash;
};
#endif /* ROUTER_H_ */

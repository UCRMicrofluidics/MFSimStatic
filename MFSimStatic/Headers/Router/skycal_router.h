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
 * Type: Scheduler-Placer-Router												*
 * Name: Routing Based Synthesis												*
 *																				*
 * Inferred from the following paper:											*
 * Authors: 																	*
 * Title: s																		*
 * Publication Details: 														*
 * 																				*
 * Detailed in the following paper:												*
 * Authors: 																	*
 * Title: 																		*
 * Publication Details: 														*
 * 																				*
 * Details: Performs routing-based synthesis by performing mixes and splits		*
 * while routing; there are no fixed modules (except for detecting, heating and *
 * other operations that may require a specific fixed resource on the DMFB.		*
 *-----------------------------------------------------------------------------*/

#ifndef SKYCAL_ROUTER_H_
#define SKYCAL_ROUTER_H_

#include "router.h"

#include "../Scheduler/scheduler.h"
#include "../Scheduler/priority.h"
#include "../Testing/elapsed_timer.h"
#include "../../Headers/Models/euler_graph.h"

enum Turning { NO_FACE, FORWARD1, FORWARD2, FORWARD3, TURN_LEFT, TURN_RIGHT, FULL_TURN };
Direction leftTurn(Direction dir);
Direction rightTurn(Direction dir);
Direction fullTurn(Direction dir);
string toString(Direction dir);
string toString(Turning t);
bool compareStartTS(ReconfigModule *rm1, ReconfigModule *rm2);

struct Move
{
	Direction dir;
	float value;
	RoutePoint *result;

	Move(Direction d) : dir(d), value(0.0), result(NULL) {}
	Move(Direction d, RoutePoint *r) : dir(d), value(0.0), result(r) {}

	bool operator<(const Move & rhs) const
	{
		return value < rhs.value;
	}
};

struct CellInfo
{
	ResourceType type;
	int depth;
	vector<FixedModule *> modules;
	vector<IoResource *> outs;
	string contaminator;
};

struct Progress
{
	int beginCycle;
	int expectedEndCycle;
	bool inProgress;

	int numCycles() const
	{
		return expectedEndCycle - beginCycle + 1;
	}

	bool isOnTime(int cycle) const
	{
		return cycle <= expectedEndCycle + numCycles();
	}

	bool isLate(int cycle) const
	{
		return !isOnTime(cycle) && cycle <= expectedEndCycle + numCycles() * 2;
	}

	bool isVeryLate(int cycle) const
	{
		return !isOnTime(cycle) && !isLate(cycle);
	}

	Progress() : beginCycle(0), expectedEndCycle(0), inProgress(false) {}
};

class SkyCalRouter: public Router, public Scheduler {
public:
	SkyCalRouter();
	SkyCalRouter(DmfbArch *dmfbArch);

	// Pre-defined constant weights for random movement
	static const int TWO_WEIGHT_BEST1A = 50;
	static const int TWO_WEIGHT_BEST2A = 100;
	static const int THREE_WEIGHT_BEST1A = 34;
	static const int THREE_WEIGHT_BEST2A = 67;
	static const int THREE_WEIGHT_BEST3A = 100;

	static const int TWO_WEIGHT_BEST1B = 75;
	static const int TWO_WEIGHT_BEST2B = 100;
	static const int THREE_WEIGHT_BEST1B = 50;
	static const int THREE_WEIGHT_BEST2B = 83;
	static const int THREE_WEIGHT_BEST3B = 100;

	static const int TWO_WEIGHT_BEST1C = 90;
	static const int TWO_WEIGHT_BEST2C = 100;
	static const int THREE_WEIGHT_BEST1C = 85;
	static const int THREE_WEIGHT_BEST2C = 95;
	static const int THREE_WEIGHT_BEST3C = 100;

	static const int TWO_WEIGHT_BEST1 = TWO_WEIGHT_BEST1C;
	static const int TWO_WEIGHT_BEST2 = TWO_WEIGHT_BEST1C;
	static const int THREE_WEIGHT_BEST1 = THREE_WEIGHT_BEST1C;
	static const int THREE_WEIGHT_BEST2 = THREE_WEIGHT_BEST1C;
	static const int THREE_WEIGHT_BEST3 = THREE_WEIGHT_BEST1C;

	static const int MIN_PARTITION_WIDTH = 5;
	static const int MIN_PARTITION_HEIGHT = 6;
protected:
	// Initializing Functions
	void computeCellDepth();
	void initCellTypeArray();
	void setGrowthFactors(DAG *dag);
	void initWashAssay();

	// Helper Functions
	void setBestWeights();
	void setImprovedWeights();
	void setNeutralWeights();
	int getPartitionIndex(RoutePoint *p) const;
	void getPartitionRect(int part, int & lx, int & rx, int & ty, int & by) const;
	bool areaIsClean(AssayNode *n, int lx, int rx, int ty, int by) const;
	void removeContaminator(RoutePoint *p, Droplet *d);
	void updateContaminationAt(RoutePoint *p, Droplet *d);
	RoutePoint *getNextWashDropletDest(AssayNode *n, Droplet *d, map<Droplet *, vector<RoutePoint *> *> *routes);
	RoutePoint *step(RoutePoint *p, Direction dir);
	Direction determineDirection(RoutePoint *from, RoutePoint *to);
	Turning determineDropletTurn(Droplet *d, map<Droplet *, vector<RoutePoint *> *> *routes, RoutePoint *next);
	float computeMixingDegree(Droplet *d, map<Droplet *, vector<RoutePoint *> *> *routes, RoutePoint *next);
	FixedModule *findModule(Droplet *d, AssayNode *n, map<Droplet *, vector<RoutePoint *> *> *routes);
	IoResource *findOutputPort(Droplet *d, AssayNode *n, map<Droplet *, vector<RoutePoint *> *> *routes, bool waste = false);
	bool moduleIsClear(FixedModule *fm, AssayNode *n, map<Droplet *, vector<RoutePoint *> *> *routes);
	bool cellIsAvailable(int x, int y, AssayNode *n);
	bool willContaminate(RoutePoint *p, AssayNode *n);
	bool checkFeasibility(RoutePoint *p, Droplet *d, AssayNode *n, map<Droplet *, vector<RoutePoint *> *> *routes, bool strict);
	int manhattanDist(RoutePoint *p1, RoutePoint *p2);
	int selectRandomMove(vector<Move> *moves);
	bool boardIsCongested();
	bool boardIsVeryCongested();
	bool delayDispense(AssayNode *n);
	void computeAvailableArea();
	void insertFeasibleMoves(vector<Move> *moves, Droplet *d, AssayNode *n, map<Droplet *, vector<RoutePoint *> *> *routes);

	// These functions look at a specific droplet and determines all possible moves and updates routes for the current cycle
	bool performDispense(Droplet *d, AssayNode* n, map<Droplet *, vector<RoutePoint *> *> *routes);
	void performGoalMove(Droplet *d, RoutePoint *goal, AssayNode* n, map<Droplet *, vector<RoutePoint *> *> *routes);
	void performMergeMove(Droplet *d1, Droplet *d2, AssayNode* n, map<Droplet *, vector<RoutePoint *> *> *routes);
	bool performMixMove(Droplet *d, AssayNode* n, map<Droplet *, vector<RoutePoint *> *> *routes);
	bool performSplitMove(Droplet *d, AssayNode* n, map<Droplet *, vector<RoutePoint *> *> *routes);
	void performUnweightedMove(Droplet *d, AssayNode* n, map<Droplet *, vector<RoutePoint *> *> *routes);
	void performNoMove(Droplet *d, AssayNode* n, map<Droplet *, vector<RoutePoint *> *> *routes);
	void performNoMoveUnforced(Droplet *d, AssayNode* n, map<Droplet *, vector<RoutePoint *> *> *routes);

public:
	// This function process an individual Assay based on its status and operation
	AssayNodeStatus processAssay(AssayNode* n, bool allowProcessing,
			DAG *dag, DmfbArch *arch,
			vector<ReconfigModule *> *rModules,
			map<Droplet *, vector<RoutePoint *> *> *routes,
			vector<vector<int> *> *pinActivations,
			vector<unsigned long long> *tsBeginningCycle);

	// The main functions of this Router
	virtual void preRoute(DmfbArch *arch);
	virtual void route(DAG *dag, DmfbArch *arch, vector<ReconfigModule *> *rModules, map<Droplet *, vector<RoutePoint *> *> *routes, vector<vector<int> *> *pinActivations, vector<unsigned long long> *tsBeginningCycle);
	virtual void postRoute();

	virtual ~SkyCalRouter();

private:
	// Members
	vector<vector<CellInfo > > cells; // Provides information of cells
	bool washEnabled;

	map<AssayNode *, bool> executionTable; // Keeps track of which node have been executed in a cycle
	map<AssayNode *, int > growthFactors; // Provides information for each node about their potential droplet growth

	int numDroplets;
	bool recomputationNecessary;
	int availableArea;
	int noDelta; // used to check for deadlocks
	static const double deadlockThreshold = 0.10;
	int underDeadlockThresholdCount;
	bool outputNecessary;
	IoPort *washPort;

	map<Droplet *, Progress> dropProgress;
	int twoWeightBest1;
	int twoWeightBest2;
	int threeWeightBest1;
	int threeWeightBest2;
	int threeWeightBest3;

	map<string, int> dropReference;
	DAG *washOps;
	vector<ReconfigModule *> partitions;
	int partitionWidth;
	int partitionHeight;

	vector< map<int, list<RoutePoint *> > > contaminationMap;
	vector< list<int> > contaminationQueue;

	bool assayComplete;
};

#endif /* SKYCALROUTER_H_ */

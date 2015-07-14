/*------------------------------------------------------------------------------*
 *                       (c)2012, All Rights Reserved.     						*
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
 * Name: Bypass Router															*
 *																				*
 * Inferred from the following paper:											*
 * Authors: Minsik Cho and David Z. Pan											*
 * Title: A high performance droplet routing algorithm for digital microfluidic	*
 * biochips																		*
 *------------------------------------------------------------------------------*/

#ifndef CHO_ROUTER_H_
#define CHO_ROUTER_H_

#include "../Testing/elapsed_timer.h"
#include "post_subprob_compact_router.h"
#include "../Resources/enums.h"
#include <stack>
#include <vector>
#include <set>

struct RotuingPoint;

enum cellType {Empty = 0, Blocked = 1, Concession = 2};

struct BoardCell1
{
	int x;
	int y;
	cellType type;
	int distance;
	bool visited;
	BoardCell1 * prev;
};

struct BoardCell2
{
	int x;
	int y;
	cellType type;
	int occupation;
	int distance;
	bool visited;
	BoardCell2 * prev;
};

class ChoRouter : public PostSubproblemCompactionRouter
{
	public:
		// Constructors
		ChoRouter();
		ChoRouter(DmfbArch *dmfbArch);
		virtual ~ChoRouter();

		friend class Router;

	protected:
		void computeIndivSupProbRoutes(vector<vector<RoutePoint *> *> *subRoutes, vector<Droplet *> *subDrops, map<Droplet *, vector<RoutePoint *> *> *routes);
		void routerSpecificInits();
		void sortDropletsInDecBypassability(vector<Droplet *> *routingThisTS, map<Droplet *, BoardCell1 *> *sourceCells, map<Droplet *, BoardCell1 *> *targetCells, int numCellsX, int numCellsY);
		void compactRoutesByTimingAndFaultTolerance(vector<Droplet *> *subDrops, vector<vector<RoutePoint *> *> *subRoutes);
		void compactorGetSourcesTargets(vector<Droplet *> *subDrops, vector<vector<RoutePoint *> *> *subRoutes, map<Droplet *, BoardCell1 *> *sourceCells, map<Droplet *, BoardCell1 *> *targetCells);


	private:
		// Members
		vector<vector<BoardCell1 *> *> *board;

		int getInfinityVal() { return 99999; }
};


#endif /* BYPASS_ROUTER_H_ */

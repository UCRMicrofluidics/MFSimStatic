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
 * Name: Roy's Maze Router														*
 *																				*
 * Inferred from the following paper:											*
 * Authors: Pranab Roy, Hafizur Rahaman and Parthasarathi Dasgupta				*
 * Title: A novel droplet routing algorithm for digital microfluidic biochips	*
 * Publication Details: In Proc. GLSVLSI, Providence, RI, 2010					*
 * 																				*
 * Detailed in the following paper:												*
 * Authors: Dan Grissom and Philip Brisk										*
 * Title: Fast online synthesis of generally programmable digital microfluidic	*
 * 			biochips															*
 * Publication Details: In Proc. ESWEEK (CODES+ISSS), Tampere, Finland, 2012	*
 * 																				*
 * Details: Uses Soukup's fast maze rotuer to generate individual routes. The	*
 * router was originally inferred from Roy's paper in GLSVLSI, but was slightly *
 * modified. The modified version is contained in this code and is detailed in	*
 * Grissom's CODES+ISSS paper.													*
 *-----------------------------------------------------------------------------*/

#ifndef ROY_MAZE_ROUTER_H_
#define ROY_MAZE_ROUTER_H_

#include "../Testing/elapsed_timer.h"
#include "post_subprob_compact_router.h"
#include "../Resources/enums.h"
#include <stack>
#include <set>
//#include <vector>

//enum SoukupC { NotReached = 0, LeeReached = 1, LineReached = 2 };
//enum SoukupS { OtherLayer = 0, LeftT = 1, RightT = 2, DownT = 3, UpT = 4, StartPoint = 5, TargetPoint = 6, Blockage = 7, UnknownS = 100};

struct RotuingPoint;

class RoyMazeRouter : public PostSubproblemCompactionRouter
{
	public:
		// Constructors
		RoyMazeRouter();
		RoyMazeRouter(DmfbArch *dmfbArch);
		virtual ~RoyMazeRouter();

	protected:
		void computeIndivSupProbRoutes(vector<vector<RoutePoint *> *> *subRoutes, vector<Droplet *> *subDrops, map<Droplet *, vector<RoutePoint *> *> *routes);
		void routerSpecificInits();

	private:
		// Methods
		bool towardTarget(SoukupCell *curr, SoukupCell *next, SoukupCell *target);
		int getTraceBackDir( SoukupCell* curr, SoukupCell* next );
		void debugPrintSoukupBoard();
		void debugPrintSourceTargetPair(Droplet *d, map<Droplet *, SoukupCell *> *sourceCells, map<Droplet *, SoukupCell *> *targetCells);

		// Members
		vector<vector<SoukupCell *> *> *board;
};


#endif /* ROY_MAZE_ROUTER_H_ */

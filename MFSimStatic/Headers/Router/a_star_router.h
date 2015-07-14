/*
 * A_Star_Router.h
 *
 *  Created on: Feb 26, 2013
 *      Author: Chris Jaress
 */

#ifndef A_STAR_ROUTER_H
#define A_STAR_ROUTER_H


#include "../Testing/elapsed_timer.h"
#include "post_subprob_compact_router.h"
//#include "../enums.h"
#include "router.h"

struct StarCell
{
	int x;
	int y;
	int score;
	StarCell* came_from;
	bool block;
};


//struct RotuingPoint;

class AStarRouter : public PostSubproblemCompactionRouter
{
	public:
		// Constructors
		AStarRouter();
		AStarRouter(DmfbArch *dmfbArch);
		virtual ~AStarRouter();

	protected:
		void computeIndivSupProbRoutes(vector<vector<RoutePoint *> *> *subRoutes, vector<Droplet *> *subDrops, map<Droplet *, vector<RoutePoint *> *> *routes);
		void routerSpecificInits();

	private:
		vector<RoutePoint*> * find_path(StarCell* previous_node, StarCell* current_node);
		StarCell* score_min(vector<StarCell*> *curr_set);
		bool is_in_evaluated_set(vector<StarCell*> *open_set, StarCell* in_question);

		// Members
		vector<vector<StarCell *> *> *board;
		void remove_from_open_set(vector<StarCell*> *open_set, StarCell* current_node);
		void printBlockages();


};

#endif




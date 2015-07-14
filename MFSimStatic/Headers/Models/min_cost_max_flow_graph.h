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
 * Name: MinCostMaxFlowGraph (Min-Cost-Max-Flow Graph)							*
 *																				*
 * Details: This class represents a simple graph structure represented by		*
 * vectors of integers and longs. The graph can perform min-cost-max-flow		*
 * computations on itself using a algorithm similar to the Ford-Fulkerson		*
 * algorithm.																	*
 * 																				*
 * Note: This code was taken and modified to its current state from 			*
 * http://www.stanford.edu/~liszt90/acm/notebook.html#file2						*
 *-----------------------------------------------------------------------------*/
#ifndef MIN_COST_MAX_FLOW_GRAPH_H_
#define MIN_COST_MAX_FLOW_GRAPH_H_

using namespace std;
#include "entity.h"
#include <cmath>
#include <vector>
#include <iostream>
#include <limits>

const long long INF = std::numeric_limits<long long>::max() / 4;

class MinCostMaxFlowGraph : public Entity
{
	protected:
		// Variables
		static int next_id;
		int N; // Number of nodes
		int S; // Source node
		int T; // Sink node
		vector<vector<long long> > cap, flow, cost;
		vector<int> found;
		vector<long long> dist, pi, width;
		vector<pair<int, int> > dad;
		pair<int, int> maxFlowCostResult;



		// Methods
		long long Dijkstra(int s, int t);
		void Relax(int s, int k, long long cap, long long cost, int dir);

	public:
		// Constructors
		MinCostMaxFlowGraph();
		MinCostMaxFlowGraph(int numNodes);
		virtual ~MinCostMaxFlowGraph();

		// Methods
		void AddEdge(int from, int to, long long cap, long long cost);


		// Getters/Setters
		pair<long long, long long> GetMaxFlow(int s, int t);
		long long getCap(int from, int to) { return cap[from][to]; }
		long long getFlow(int from, int to) { return flow[from][to]; }
		long long getCost(int from, int to) { return cost[from][to]; }

		// Print/Debug
		void PrintResults();
		void OutputGraphFile(string filename);

};
#endif

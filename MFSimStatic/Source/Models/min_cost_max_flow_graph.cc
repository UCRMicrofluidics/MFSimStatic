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
 * Source: min_cost_max_flow_graph.cc											*
 * Original Code Author(s): Stanford ACM										*
 * (http://www.stanford.edu/~liszt90/acm/notebook.html#file2); reformatted and	*
 * modified by Dan Grissom														*
 * Original Completion/Release Date: November 14, 2012							*
 *																				*
 * Details: N/A																	*
 *																				*
 * Revision History:															*
 * WHO		WHEN		WHAT													*
 * ---		----		----													*
 * FML		MM/DD/YY	One-line description									*
 *-----------------------------------------------------------------------------*/
#include "../../Headers/Models/min_cost_max_flow_graph.h"
#include "../../Headers/Testing/claim.h"
#include <fstream>


int MinCostMaxFlowGraph::next_id = 1;

///////////////////////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////////////////////
MinCostMaxFlowGraph::MinCostMaxFlowGraph()
{
	id = next_id++;
}
MinCostMaxFlowGraph::MinCostMaxFlowGraph(int numNodes) :
	N(numNodes), cap(numNodes, vector<long long>(numNodes)), flow(numNodes, vector<long long>(numNodes)), cost(numNodes, vector<long long>(numNodes)),
	found(numNodes), dist(numNodes), pi(numNodes), width(numNodes), dad(numNodes)
{
}


///////////////////////////////////////////////////////////////////////////////////
// Deconstructor
///////////////////////////////////////////////////////////////////////////////////
MinCostMaxFlowGraph::~MinCostMaxFlowGraph()
{
}


void MinCostMaxFlowGraph::PrintResults()
{
	cout << "Max flow: " << maxFlowCostResult.first << endl;
	cout << "Min cost: " << maxFlowCostResult.second << endl << endl;

	cout << "Edges with flow: SourceNode --(flowAmount)--> DestinationNode:" << endl;
	for (int i = 0; i < N; i++)
		for (int j = 0; j < N; j++)
			if (flow.at(i).at(j) > 0)
				cout << i << "--(" << flow.at(i).at(j) << ")-->" << j << endl;
}

void MinCostMaxFlowGraph::AddEdge(int from, int to, long long cap, long long cost)
{
	this->cap[from][to] = cap;
	this->cost[from][to] = cost;
}

void MinCostMaxFlowGraph::Relax(int s, int k, long long cap, long long cost, int dir)
{
	long long val = dist[s] + pi[s] - pi[k] + cost;
	if (cap && val < dist[k]) {
		dist[k] = val;
		dad[k] = make_pair(s, dir);
		width[k] = min(cap, width[s]);
	}
}

long long MinCostMaxFlowGraph::Dijkstra(int s, int t)
{
	fill(found.begin(), found.end(), false);
	fill(dist.begin(), dist.end(), INF);
	fill(width.begin(), width.end(), 0);
	dist[s] = 0;
	width[s] = INF;

	while (s != -1)
	{
		int best = -1;
		found[s] = true;
		for (int k = 0; k < N; k++)
		{
			if (found[k]) continue;
			Relax(s, k, cap[s][k] - flow[s][k], cost[s][k], 1);
			Relax(s, k, flow[k][s], -cost[k][s], -1);
			if (best == -1 || dist[k] < dist[best]) best = k;
		}
		s = best;
	}

	for (int k = 0; k < N; k++)
		pi[k] = min(pi[k] + dist[k], INF);
	return width[t];
}

pair<long long, long long> MinCostMaxFlowGraph::GetMaxFlow(int s, int t)
{
	S = s;
	T = t;
	long long totflow = 0, totcost = 0;
	while (long long amt = Dijkstra(s, t))
	{
		totflow += amt;
		for (int x = t; x != s; x = dad[x].first)
		{
			if (dad[x].second == 1)
			{
				flow[dad[x].first][x] += amt;
				totcost += amt * cost[dad[x].first][x];
			}
			else
			{
				flow[x][dad[x].first] -= amt;
				totcost -= amt * cost[x][dad[x].first];
			}
		}
	}
	maxFlowCostResult = make_pair(totflow, totcost);
	return maxFlowCostResult;
}

///////////////////////////////////////////////////////////////
//Creates a graph of the Dag in .dot format
///////////////////////////////////////////////////////////////
void MinCostMaxFlowGraph::OutputGraphFile(string filename)
{

	ofstream out;
	//filename = "Output/" + filename + ".dot";
	filename = filename + ".dot";

	out.open(filename.c_str());

	{
		stringstream str;
		str << "Failed to properly write DAG Graph file: " << filename << endl;
		claim (out.good(), &str);
	}

	// Opening bracket
	out<<"digraph G {\n";
	//out<<"  size=\"8.5,10.25\";\n";
	//out << "label=\"" << filename << "\"";

	// Print nodes
	for (int i = 0; i < N; i++)
	{
		string labelName;
		string colorName;
		stringstream ss;

		ss << i;
		labelName = ss.str();
		colorName = "tan";

		if (i == S)
		{
			labelName = "S" + labelName;
			colorName = "lightsteelblue";
		}
		else if (i == T)
		{
			labelName = "T" + labelName;
			colorName = "olivedrab";
		}

		out << i << " [label = \"" << labelName;
		//out << n->getId() << " [label = \"" << "";
		out << "\"";
		out << " fillcolor=" << colorName << ", style=filled];\n";
	}

	// Print edges
	for (int i = 0; i < N; i++)
	{
		for (int j = 0; j < N; j++)
		{
			if (cap.at(i).at(j) > 0)
			{
				out << i << " -> " << j;
				out << " [";
				string color;
				if (flow.at(i).at(j) > 0)
					color= "008000";
				else
					color= "000000";

				//out << "fontcolor=\"#" << color << "\", label=\"" << flow.at(i).at(j) << "/" << cap.at(i).at(j) << "\\n$" << cost.at(i).at(j) << "\", ";
				if (cap.at(i).at(j) >= 9999999)
					out << "fontcolor=\"#" << color << "\", label=\"" << flow.at(i).at(j) << "/" << "&#8734;" << "\\n$" << cost.at(i).at(j) << "\", ";
				else
					out << "fontcolor=\"#" << color << "\", label=\"" << flow.at(i).at(j) << "/" << cap.at(i).at(j) << "\\n$" << cost.at(i).at(j) << "\", ";
				out << "color=\"#" << color << "\"]";
				out << ";\n";
			}
		}
	}

	// Closing bracket
	out<<"}\n";
}


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
 * Name: EulerGraph (Euler Graph)												*
 *																				*
 * Details: Represents a Euler graph.											*
 *-----------------------------------------------------------------------------*/

#ifndef EULERGRAPH_H_
#define EULERGRAPH_H_
#include <iostream>
#include <vector>
#include <list>
#include <deque>
using namespace std;

struct eulerEdge{
	int node1;
	int node2;

	eulerEdge(int n1, int n2) : node1(n1), node2(n2){};

	void print()
	{
		cout << "(" << node1 <<")----("<<node2<<")" << endl;
	}

	int getPartner(int x)
	{
		return node1 == x ? node2 : node1;
	}
};

class EulerGraph {
private:
	bool isOdd(int x) const;
	int width;
	int height;


	vector<eulerEdge> edges;
	vector< list<int> > nodeEdgeLists;
	list<int> eulerPath;

public:
	EulerGraph();
	EulerGraph(int w, int h);

	//******
	//******** Accessors
	//*************************
	inline int posToIndex(int x, int y);

	vector<int> findOddNodes() const;
	list<int> getEulerPath() const;
	void printNodeEdgeLists();

	//******
	//******** Mutators
	//*************************

	void initGraph();
	void semiEulerize();
	void hierholzersAlgorithm();
};

#endif /* EULERGRAPH_H_ */

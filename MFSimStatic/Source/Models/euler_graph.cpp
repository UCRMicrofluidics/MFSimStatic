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
 * Source: euler_graph.cc														*
 * Original Code Author(s): Calvin Phung, Skyler Windh							*
 * Original Completion/Release Date: April 23, 2014								*
 *																				*
 * Details: N/A																	*
 *																				*
 * Revision History:															*
 * WHO		WHEN		WHAT													*
 * ---		----		----													*
 * FML		MM/DD/YY	One-line description									*
 *-----------------------------------------------------------------------------*/

#include "../../Headers/Models/euler_graph.h"

bool EulerGraph::isOdd(int x) const
{
	return (x % 2);
}

EulerGraph::EulerGraph(): width(0), height(0), edges(), nodeEdgeLists(), eulerPath() {}
EulerGraph::EulerGraph(int w, int h): width(w), height(h), nodeEdgeLists(w*h)
{
	this->initGraph();
	this->semiEulerize();
	this->hierholzersAlgorithm();
}

//******
//******** Accessors
//*************************
inline int EulerGraph::posToIndex(int x, int y) {return y*width + x;}
vector<int> EulerGraph::findOddNodes() const
{
	vector<int> oddNodes;
	for(unsigned i = 0; i < nodeEdgeLists.size(); ++i)
	{
		if( isOdd( nodeEdgeLists[i].size() ) )
		{
			oddNodes.push_back(i);
		}
	}
	return oddNodes;
}

list<int> EulerGraph::getEulerPath() const
{
	return eulerPath;
}

void EulerGraph::printNodeEdgeLists()
{
	for( unsigned i = 0; i < nodeEdgeLists.size(); ++i)
	{
		cout << "Node " << i << ":" << endl;
		for(list<int>::iterator it = nodeEdgeLists[i].begin(); it != nodeEdgeLists[i].end(); ++it)
			edges[(*it)].print();
	}
}

//******
//******** Mutators
//*************************

void EulerGraph::initGraph()
{
	for ( int y = 0; y < height; ++y )
	{
		for (int x = 0; x < width; ++x)
		{
			int nodePos = posToIndex(x,y);
			if( x < width-1 ){ //we can add right edge
				edges.push_back( eulerEdge( nodePos, nodePos+1) );
				nodeEdgeLists[nodePos].push_back(edges.size()-1);
				nodeEdgeLists[nodePos+1].push_back(edges.size()-1);
			}
			int secondNodePos = posToIndex(x, y+1);
			if( y < height -1 ){ //we can add bottom edge
				edges.push_back( eulerEdge( nodePos, secondNodePos) );
				nodeEdgeLists[nodePos].push_back(edges.size()-1);
				nodeEdgeLists[secondNodePos].push_back(edges.size()-1);
			}
		}
	}
}

void EulerGraph::semiEulerize()
{
	//Make graph semi-Eulerian (only 2 vertices of odd degree)
	//
	// Since graph is a connected rectangle, only odd vertices are
	// interior side nodes
	//
	//		E - O - O - E
	//		'   '   '   '       O - odd degree
	//		O - E - E - O       E - even degree
	//		'   '   '   '
	//		E - O - O - E
	//
	// lower left corner is dispense, lower right is waste output
	// these 2 corners need to be odd, so start in lower left corner
	// and make all edges even by adding duplicate edges where necessary
	//
	//		E = E - E = E
	//		"   '   '   "       O - odd degree
	//		E - E - E - E       E - even degree
	//		'   '   '   '
	//		O = E - E = O
	//
	//***************************************************************

	//Calculate corner indices
	int lowerLeft  = posToIndex(0, height-1);//(height-1)*width;
	int lowerRight = posToIndex(width-1, height-1);
	int upperRight = posToIndex(width-1, 0);
	int upperLeft  = posToIndex(0, 0);

	//modify bottom edge, left to right
	for(int i = lowerLeft; i < lowerRight; i+=2  ) //advance by two so we are only
	{
		edges.push_back( eulerEdge( i, i+1) );
		nodeEdgeLists[i].push_back(edges.size()-1);
		nodeEdgeLists[i+1].push_back(edges.size()-1);
	}

	//modify right edge, bottom to top
	//only start at the bottom corner if it is still even, otherwise move up a row
	if( isOdd(nodeEdgeLists[lowerRight].size()) )
		lowerRight =  (lowerRight)-width;

	for(int i = lowerRight; i > upperRight; i -= (2*width)  ) //decrement 2 rows at a time
	{
		edges.push_back( eulerEdge( i, i-width ) );
		nodeEdgeLists[i].push_back(edges.size()-1);
		nodeEdgeLists[i-width].push_back(edges.size()-1);
	}

	//modify top edge, right to left
	//If corner is even, skip it and start at the left neighbor
	if( !isOdd(nodeEdgeLists[upperRight].size()) )
		upperRight -= 1;

	for(int i = upperRight; i > 0; i -= 2  ) //decrement 2 columns at a time
	{
		edges.push_back( eulerEdge( i, i-1) );
		nodeEdgeLists[i].push_back(edges.size()-1);
		nodeEdgeLists[i-1].push_back(edges.size()-1);
	}

	//modify left edge, top to bottom
	//If corner is even, skip it and start at the bottom neighbor
	if( !isOdd(nodeEdgeLists[upperLeft].size()) )
		upperLeft = posToIndex(0, 1);
	for(int i = upperLeft; i < posToIndex(0, height-2); i += (2*width)  ) //increment 2 rows at a time
	{
		edges.push_back( eulerEdge( i, i+width) );
		nodeEdgeLists[i].push_back(edges.size()-1);
		nodeEdgeLists[i+width].push_back(edges.size()-1);
	}

}

void EulerGraph::hierholzersAlgorithm()
{
	deque<int> unfinishedNodes;
	list<int> subPath;

	//start at dispense node
	unfinishedNodes.push_back(posToIndex(0, height-1));


	int goalNode = posToIndex(width-1, height-1);
	while( !unfinishedNodes.empty() )
	{
		int currentNode = unfinishedNodes.front();
		unfinishedNodes.pop_front();

		if( nodeEdgeLists[currentNode].size() == 0 )//remove finished nodes from the list
			continue;
		subPath.push_back(currentNode);
		if(!eulerPath.empty())
		{
			//not first pass, look for Euler circuit instead of path
			goalNode = currentNode;
		}

		int oldCurrentNode = currentNode;
		do
		{
			int edge = nodeEdgeLists[currentNode].front();
			//remove edge from current node
			nodeEdgeLists[currentNode].pop_front();

			int to = edges[edge].getPartner(currentNode);
			//remove edge from dest. node
			for(list<int>::iterator it = nodeEdgeLists[to].begin(); it != nodeEdgeLists[to].end(); ++it)
			{
				if( *it == edge )
				{
					nodeEdgeLists[to].erase(it);
					break; //break since we have duplicate edges
				}
			}
			subPath.push_back(to);
			unfinishedNodes.push_back(to);

			currentNode = to;
		}while( currentNode != goalNode );

		//merge subPath into eulerPath
		if(eulerPath.empty()){
			eulerPath.splice(eulerPath.begin(), subPath);
		}else{
			for(list<int>::iterator it = eulerPath.begin(); it != eulerPath.end(); ++it)
			{
				if(*it == oldCurrentNode)
				{
					eulerPath.splice(it, subPath);
					eulerPath.erase(it);
					break;
				}
			}
		}
	}
}

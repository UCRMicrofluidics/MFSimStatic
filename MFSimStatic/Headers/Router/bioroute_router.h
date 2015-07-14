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
 * Name: Yuh's BioRoute Router													*
 *																				*
 * Inferred from the following paper:											*
 * Authors: Ping-Hung Yuh, Chia-Lin Yang and Yao-Wen Chang						*
 * Title: BioRoute: A Network-Flow Based Routing Algorithm for Digital 			*
 * Microfluidic Biochips														*
 * Publication Details: In Proc. ICCAD, San Jose, CA, 2007						*
 * 																				*
 * Details: Uses network flow computations to compute an initial global routing	*
 * stage, followed by a detailed routing stage.									*
 *-----------------------------------------------------------------------------*/

#ifndef BIO_ROUTER_H
#define BIO_ROUTER_H

#include "../Testing/elapsed_timer.h"
#include "post_subprob_compact_router.h"

		enum inclusion { INCLUDED, DISCARDED };
		enum action { CREATIVE, DISMISSIVE, POSSESSIVE };
		enum progress { UNROUTED, ROUTED_BC };
		enum update { UP, DOWN };

struct timeInterval {
	//variables
	int t1; int t2;
	//functions
	timeInterval(int a, int b) { t1 = a; t2 = b; }
	bool sensible() { if (t1 <= t2) return true; else return false; }
	int span() { return t2 - t1; }
	void display() {cout << "[" << t1 << "," << t2 << "]"; }
};

struct Cell {
	//variables
	int x;
	int y;
	int injunction;
	double cost;
	int arrival;
	int departure;
	int history;
	//functions
	Cell() { x = 0; y = 0; }
	Cell(int a, int b) { x = a; y = b; }
	bool operator==(const Cell &other) const
	{
	    if (x == other.x && y == other.y) return true;
	    else return false;
	}
	bool operator!=(const Cell &other) const
	{
		if (x != other.x || y != other.y) return true;
		else return false;
	}

	void display() { cout << "(" << x << "," << y << ")"; }
};

class compareCell {
public:
	bool operator()(Cell& c1, Cell& c2)
	{
		if (c1.cost > c2.cost) return true;
		if (c1.cost == c2.cost)
			if (c1.arrival > c2.arrival) return true;
		return false;
	}
};

struct Net {
	//variables
	progress global; // loop-focus: fundamental nets
	int netsInjunction; // loop-focus: local selecting independent nets
	inclusion independence; // loop-focus: local selecting independent nets
	Cell source;
	Cell target;
	//functions
	Net () { source = Cell(0,0); target = Cell(0,0); global = UNROUTED; independence = INCLUDED; }
	Net (Cell s, Cell t) { source = s; target = t; global = UNROUTED; independence = INCLUDED; }
	bool operator==(const Net &other) const
	{
	    if (source == other.source && target == other.target) return true;
	    else return false;
	}
	void display() { cout << "*"; source.display(); cout << "/"; target.display(); cout << "*"; }
};

struct Box {
	//variables
	int LX, RX, TY, BY;
	//functions
	Box() { LX = 0; RX = 0; TY = 0; BY = 0; }
	Box(int a, int b, int c, int d) { LX = a; RX = b; TY = c; BY = d; }
	void display() { cout << "#" << LX << "," << RX << "," << TY << "," << BY << "#"; }
};

struct globalCell {
	//variables
	int LX, RX, TY, BY;
	double capacity;
	double previous;
	double cost;
	//functions
	globalCell() { capacity = 0; previous = 0; cost = 0; }
	globalCell(int L, int R, int T, int B) { LX = L; RX = R; TY = T; BY = B; }
	void display() { cout << "^" << LX << "," << RX << "," << TY << "," << BY << "^"; }
};

struct GlobalArray
{
	//variables
	vector< vector<globalCell> > globalCells;
	vector< vector<bool> > inclusionMatrix;
};

class Node {
    private:
    int cost;
    int sum;
    bool complete;
    bool source;
    bool sink;
    vector< vector<int> > last;
    vector< vector<int> > history;
    int GCx, GCy;

    public:
    Node(int c) { cost = c; sum = 0; complete = false; source = false; sink = false; GCx = 0; GCy = 0; }
    Node(int c, int X, int Y) { cost = c; sum = 0; complete = false; source = false; sink = false; GCx = X; GCy = Y; }
    Node() { cost = 0; sum = 0; complete = false; source = false; sink = false; GCx = 0; GCy = 0; }
    int getCost() { return cost; }
    void setCost(int c) { cost = c; }
    int getSum() { return sum; }
    void setSum(int s) { sum = s; }
    bool isComplete() { return complete; }
    void setComplete(bool c) { complete = c; }
    void declareSource() { source = true; }
    void declareSink() { sink = true; }
    bool isSource() { return source; }
    bool isSink() { return sink; }
    int getGCx() { return GCx; }
    int getGCy() { return GCy; }
    vector< vector<int> > getLast() { return last; }
    void pushLast(vector< vector<int> > H);
    void clearLast();

    vector< vector<int> > getHistory() { return history; }
    void setHistory(vector< vector<int> > H) { history = H; }
    void updateHistory(int h);
    void clearHistory();
};

class Network {
    private:
    vector<Node> vertices;
    map< int, vector<int> > neighborMap;
    vector< vector<int> > pathInjuncts;

    public:
    void pushNode(Node A) { vertices.push_back(A); }
    void pushEdge(int in1, int in2) { neighborMap[in1].push_back(in2); }
    void removeNode(int GCx, int GCy);
    bool detonate();
    void reset();
    vector< vector <int> > getPaths() { return pathInjuncts; }
    vector<Node> getVertices() { return vertices; }
};

struct cellNode {
    //variables
	//vector<int> arrival;
	//vector<int> departure;
	vector<double> s;
	vector<double> currentHFP;
	vector<double> previousHFP;
	//functions
	cellNode(int nets, int tMax) {
		for (int i = 0; i <= tMax; i++) { currentHFP.push_back(1); previousHFP.push_back(1); s.push_back(0); }}
};

class BioRouter : public PostSubproblemCompactionRouter
{
	public:
		// Constructors
		BioRouter();
		BioRouter(DmfbArch *dmfbArch);
		virtual ~BioRouter();

	protected:
		//Variables
		vector<Cell> sources;
		vector<Cell> targets;
		vector<Net> nets;
		vector<Network> networks;
		vector<bool> routeStatus;
		vector< vector<int> > attributes;
		vector< vector<bool> > cellIsBlocked;

		int tMax;
		//Functions
		void computeIndivSupProbRoutes(vector<vector<RoutePoint *> *> *subRoutes, vector<Droplet *> *subDrops, map<Droplet *, vector<RoutePoint *> *> *routes);
		bool cellIsBoundedByArray(Cell * c);
		Box boundingBoxOfNet(Net * c); //, int &LX, int &RX, int &TY, int &BY);
		Box boundingBoxOfGlobalCell(globalCell * gc);
		bool boxesTouch(Box * b1, Box * b2);
		bool boxContainsCell(Box * b, Cell * c);
		vector<Cell> touchingCells(Box * b1, Box * b2);
		bool independent(Net * n1, Net * n2);
		timeInterval idleInterval(Cell * c, Net * n);
		timeInterval violationInterval(Cell * c, Net * n);
		bool intervalViolation(timeInterval dT1, timeInterval dT2);
		double criticality(Net * n);
		int manhattanDistance(Cell * C1, Cell * C2);
		bool cellIsFree(Cell * c, Net * n);
		int available(Cell * c, Net * n, int t);
		bool violationFree(Net * n1, Net * n2, Cell * c);
		bool unidirectionCheck(Cell * c, Box * b, Net * n1, Net * n2);
		bool compareNets(Net A, Net B);
		void mergesort(vector<Net> *a, int low, int high, update DIR);
		void merge(vector<Net> *a, int low, int pivot, int high, update DIR);
		void sort(vector<Net> * a, update DIR);
		void establishBlocks();
		void establishResources(vector<vector<RoutePoint *> *> *subRoutes, vector<Droplet *> *subDrops, map<Droplet *, vector<RoutePoint *> *> *routes);
		vector<Net> independentNets();
		void constructGlobalArray(GlobalArray * GC);
		void calculateCapacities(GlobalArray * GC);
		void calculateCosts(GlobalArray * GC, vector<Net> * N);
		void networkize(Network * Nw, GlobalArray * GC, Net * N);
		void updateGlobalArray(GlobalArray * GC, Network * N, update dir);
		bool cellIsInPath(Cell * c, Network * Nw, GlobalArray * GC);
		double costToMove(vector< vector< cellNode> > * graph, Cell * next, int nArrival, Cell * previous, int pArrival);
		void gathering(vector<Cell> * candidates, vector<Cell> * discarded, vector<Cell> * happened, GlobalArray * GC, Net N, Cell current, int iterationCount, bool specialIndividual);
		void cleanUp();
};


#endif /* BIO_ROUTER_H */

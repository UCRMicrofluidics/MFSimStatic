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
 * Source: diagonal_wire_routing_model.cc										*
 * Original Code Author(s): Dan Grissom											*
 * Original Completion/Release Date: November 18, 2013							*
 *																				*
 * Details: N/A																	*
 *																				*
 * Revision History:															*
 * WHO		WHEN		WHAT													*
 * ---		----		----													*
 * FML		MM/DD/YY	One-line description									*
 *-----------------------------------------------------------------------------*/

#include "../../Headers/Models/diagonal_wire_routing_model.h"
#include <sys/time.h>
#include <math.h>

int DiagonalWireRoutingModel::next_id = 0;
///////////////////////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////////////////////
DiagonalWireRoutingModel::DiagonalWireRoutingModel()
{
	allNodes = NULL;
	arch = NULL;
	wireGridSizeX = 0;
	wireGridSizeY = 0;
	claim(false, "DiagonalWireRoutingModel constructor must be called with a DmfbArch argument.");
}
///////////////////////////////////////////////////////////////////////////////////
// Constructor - Given the architecture, creates all the nodes and edges to
// represent the model
///////////////////////////////////////////////////////////////////////////////////
DiagonalWireRoutingModel::DiagonalWireRoutingModel(DmfbArch *dmfbArch)
{
	// Init
	arch = dmfbArch;
	int xDim = arch->getNumCellsX();
	int yDim = arch->getNumCellsY();
	vector<vector<int> *> *pinMapping = arch->getPinMapper()->getPinMapping();
	pinGroups = new map<int, vector<WireRouteNode *> *>();
	allNodes = new vector<WireRouteNode *>();

	// Ensure vertical/horizontal cap are equivalent
	claim (arch->getWireRouter()->getNumHorizTracks() == arch->getWireRouter()->getNumVertTracks(), "Number of horizontal and vertical tracks must be equivalent for diagonal wire-routing model.");
	//claim (xDim == pinMapping->size() && yDim == pinMapping->at(0)->size(), "Pin-mapping array must match dimensions of DMFB.");

	// Compute/gather key metrics
	oCap = arch->getWireRouter()->getNumHorizTracks();
	tileGridSize = ((oCap+2)*2 - 1);
	wireGridSizeX = ((xDim+1)*(tileGridSize-1))+1;
	wireGridSizeY = ((yDim+1)*(tileGridSize-1))+1;

	// Create the array of pins
	pins = new vector<vector<WireRouteNode *> *>();
	for (int x = -1; x <= xDim; x++)
	{
		vector<WireRouteNode *> *col = new vector<WireRouteNode *>();
		for (int y = -1; y <= yDim; y++)
		{
			bool createdPin = false;
			// If is on array
			if (x >= 0 && y >= 0 && x < xDim && y < yDim)
			{
				int pinNum = arch->getPinMapper()->getPinMapping()->at(x)->at(y);
				if (pinNum >= 0 && x >= 0 && y >= 0 && x < xDim && y < yDim)
				{
					WireRouteNode *n = new WireRouteNode();
					n->id = next_id++;
					n->nodeType = PIN_WRN;
					n->originalPinNum = pinNum;
					n->arch = dmfbArch;
					stringstream ss;
					ss << "P(" << x << ", " << y << ")";
					n->name = ss.str();
					col->push_back(n);
					allNodes->push_back(n);

					// Get the grouping (create new one if no group exists) and add pin
					vector<WireRouteNode* > *group = NULL;
					if (pinGroups->find(pinNum) == pinGroups->end())
						pinGroups->insert(make_pair(pinNum, new vector<WireRouteNode*>()));
					group = pinGroups->find(pinNum)->second;
					group->push_back(n);
					createdPin = true;
				}
			}

			// If haven't created pin yet, create empty pin
			if (!createdPin)
			{
				//col->push_back(NULL);
				WireRouteNode *n = new WireRouteNode();
				n->id = next_id++;
				n->nodeType = EMPTY_PIN_WRN;
				n->originalPinNum = -1;
				n->arch = dmfbArch;
				stringstream ss;
				ss << "EP(" << x << ", " << y << ")";
				n->name = ss.str();
				col->push_back(n);
				allNodes->push_back(n);
			}
		}
		pins->push_back(col);
	}
	// Debug Print Pin-Array
	/*for (int y = 0; y < yDim+2; y++)
	{
		for (int x = 0; x < xDim+2; x++)
		{
			cout << pins->at(x)->at(y)->name << "\t\t";
		}
		cout << endl;
	}*/

	// Debug Print for Pin Grouping
	/*map<int, vector<WireRouteNode *> *>::iterator it = pinGroups->begin();
	for (; it != pinGroups->end(); it++)
	{
		cout << "Pin Group " << it->first << ": ";
		vector<WireRouteNode *> *v = it->second;
		for (int i = 0; i < v->size(); i++)
			cout << v->at(i)->name << "  ";
		cout << endl;
	}*/

	// Create the array of wire routing tiles
	model = new vector<vector<WireRouteTile *> *>();
	for (int x = 0; x <= xDim; x++)
	{
		vector<WireRouteTile *> *col = new vector<WireRouteTile *>();
		for (int y = 0; y <= yDim; y++)
		{
			WireRouteTile *t = new WireRouteTile(oCap, isUsingSparseModel());
			col->push_back(t);
		}
		model->push_back(col);
	}

	// Now that the nodes are created, consolidate some of the tile-edge nodes
	// in the inner tiles (that is a tile's east edge nodes should be the same
	// nodes as it's east neighbor's west edge nodes, and so on)
	for (int x = 0; x <= xDim; x++)
	{
		for (int y = 0; y <= yDim; y++)
		{
			WireRouteTile* t = model->at(x)->at(y);

			// Has tile to right, merge its west nodes with this one's east nodes
			if (x < xDim)
			{
				WireRouteTile* tEast = model->at(x+1)->at(y);

				vector<WireRouteNode *> *rightCol = t->nodCols->back();

				// Delete existing nodes at east edge of tile
				while (!rightCol->empty())
				{
					WireRouteNode *n = rightCol->back();
					rightCol->pop_back();
					delete n;
				}

				// Now copy east tile's west edge nodes into this tile
				for (unsigned i = 0; i < tEast->nodCols->front()->size(); i++)
					rightCol->push_back(tEast->nodCols->front()->at(i));
			}
			if (y < yDim)
			{	// Has tile below, merge its north nodes with this one's south nodes

				WireRouteTile* tSouth = model->at(x)->at(y+1);

				for (unsigned r = 0; r < t->nodCols->size(); r++)
				{
					// Even columns in the middle have edge nodes on their borders
					if (r > 1 && r < t->nodCols->size()-2 && r % 2 == 0)
					{
						// Remove tile's south nodes and add neighbors north
						WireRouteNode *n = t->nodCols->at(r)->back();
						t->nodCols->at(r)->pop_back();
						delete n;
						t->nodCols->at(r)->push_back(tSouth->nodCols->at(r)->front());
					}
				}
			}
		}
	}

	// Go through and set id's and recreate grid-structure by adding NULL padding
	for (int x = 0; x <= xDim; x++)
	{
		for (int y = 0; y <= yDim; y++)
		{
			WireRouteTile *t = model->at(x)->at(y);
			for (unsigned i = 0; i < t->nodCols->size(); i++)
			{
				// Give all nodes an ID and add to master list of all nodes
				vector<WireRouteNode *> *col = t->nodCols->at(i);
				for (unsigned j = 0; j < col->size(); j++)
				{
					WireRouteNode* n = col->at(j);
					if (n->id < 0)
					{
						n->id = next_id++;
						allNodes->push_back(n);
					}
				}

				// Add the pins to the cols
				if (i == 0)
				{
					col->insert(col->begin(), pins->at(x)->at(y));
					col->push_back(pins->at(x)->at(y+1));
				}
				else if (i == t->nodCols->size()-1)
				{
					col->insert(col->begin(), pins->at(x+1)->at(y));
					col->push_back(pins->at(x+1)->at(y+1));
				}

				// Okay, now insert spaces (NULLs) in between all nodes
				if (col->size() > 1)
				{
					for (int j = col->size()-1; j >= 1; j--)
						col->insert(col->begin()+j, NULL);
				}

				// Now, pad the edge with spaces (NULLs)
				int spacesOnEachSide = (tileGridSize - col->size()) / 2;
				for (int j = 0; j < spacesOnEachSide; j++)
				{
					col->insert(col->begin(), NULL);
					col->push_back(NULL);
				}
			}
			// Debug print
			/*cout << "Tile " << x << ", " << y << endl;
			for (int wy = 0; wy < tileGridSize; wy++)
			{
				for (int wx = 0; wx < tileGridSize; wx++)
				{
					WireRouteNode *n = t->nodCols->at(wx)->at(wy);

					if (!n)
						cout << " \t";
					else if (n->nodeType == PIN_WRN)
						cout << n->name << "\t";
					else if (n->nodeType == EMPTY_PIN_WRN)
						cout << n->name << "\t";
					else if (n->nodeType == ESCAPE_WRN)
						cout << "E\t";
					else if (n->nodeType == INTERNAL_WRN)
						cout << "I\t";
					else if (n->nodeType == SUPER_ESCAPE_WRN)
						cout << "S\t";
				}
				cout << endl;
			}*/
		}
	}

	// Create the super escape edge to connect all peripheral escape edges to one node
	superEscape = new WireRouteNode();
	superEscape->id = next_id++;
	superEscape->nodeType = SUPER_ESCAPE_WRN;
	allNodes->push_back(superEscape);

	// Now, give the nodes their wire-routing coordinates and create edges
	for (int x = 0; x <= xDim; x++)
	{
		for (int y = 0; y <= yDim; y++)
		{
			WireRouteTile *t = model->at(x)->at(y);
			vector<vector<WireRouteNode *> *> *grid = t->nodCols;
			for (int wy = 0; wy < tileGridSize; wy++)
			{
				for (int wx = 0; wx < tileGridSize; wx++)
				{
					WireRouteNode *n = grid->at(wx)->at(wy);
					if (n)
					{
						// Compute wire-grid coordinates
						n->wgX = wx + (x*(tileGridSize-1));
						n->wgY = wy + (y*(tileGridSize-1));

						int diagPinConnDist; // The diagonal distance (x and y) to connect the pin to the closest NW/SW/NE/SE node
						if (isUsingSparseModel())
							diagPinConnDist = 2;
						else
							diagPinConnDist = 1;

						// Connect pins to nodes
						if (n->nodeType == PIN_WRN || n->nodeType == EMPTY_PIN_WRN)
						{
							if (wx == 0 && wy == 0)
							{	// Top-Left
								n->AddUniqueConnection(grid->at(wx+2)->at(wy)); // E
								n->AddUniqueConnection(grid->at(wx)->at(wy+2)); // S
								n->AddUniqueConnection(grid->at(wx+diagPinConnDist)->at(wy+diagPinConnDist)); // SE
							}
							else if (wx == tileGridSize-1 && wy == 0)
							{	// Top-Right
								n->AddUniqueConnection(grid->at(wx-2)->at(wy)); // W
								n->AddUniqueConnection(grid->at(wx)->at(wy+2)); // S
								n->AddUniqueConnection(grid->at(wx-diagPinConnDist)->at(wy+diagPinConnDist)); // SW
							}

							else if (wx == 0 && wy == tileGridSize-1)
							{	// Bottom-Left
								n->AddUniqueConnection(grid->at(wx+2)->at(wy)); // E
								n->AddUniqueConnection(grid->at(wx)->at(wy-2)); // N
								n->AddUniqueConnection(grid->at(wx+diagPinConnDist)->at(wy-diagPinConnDist)); // NE
							}
							else if (wx == tileGridSize-1 && wy == tileGridSize-1)
							{	// Bottom-Right
								n->AddUniqueConnection(grid->at(wx-2)->at(wy)); // W
								n->AddUniqueConnection(grid->at(wx)->at(wy-2)); // N
								n->AddUniqueConnection(grid->at(wx-diagPinConnDist)->at(wy-diagPinConnDist)); // NW
							}
						}

						// Connect internal nodes
						if (wy != 0 && wy != tileGridSize-1)
						{
							if (n->nodeType == INTERNAL_WRN)
							{
								// Even rows: Connect all orthogonal nodes 2 away
								if (wy % 2 == 0)
								{
									n->AddUniqueConnection(grid->at(wx-2)->at(wy));
									n->AddUniqueConnection(grid->at(wx+2)->at(wy));
									n->AddUniqueConnection(grid->at(wx)->at(wy+2));
									n->AddUniqueConnection(grid->at(wx)->at(wy-2));
								}
								else
								{	// Odd rows: Connect all diagonals x=1 and y=1 away
									n->AddUniqueConnection(grid->at(wx-1)->at(wy-1));
									n->AddUniqueConnection(grid->at(wx-1)->at(wy+1));
									n->AddUniqueConnection(grid->at(wx+1)->at(wy-1));
									n->AddUniqueConnection(grid->at(wx+1)->at(wy+1));
								}
							}
						}

						// DTG Debugging - Added to connect escape nodes to each other b/c of new model
						if ((n->wgX >= 2 && n->wgX <= (oCap*2)) || (n->wgX >= wireGridSizeX-(oCap*2)-1 && n->wgX <= wireGridSizeX-3))
						{
							// Connect to all orthogonal nodes 2 to left/right
							if (n->nodeType == ESCAPE_WRN)
							{
								n->AddUniqueConnection(grid->at(wx-2)->at(wy));
								n->AddUniqueConnection(grid->at(wx+2)->at(wy));
							}
						}
						if ((n->wgY >= 2 && n->wgY <= (oCap*2)) || (n->wgY >= wireGridSizeY-(oCap*2)-1 && n->wgY <= wireGridSizeY-3))
						{
							// Connect to all orthogonal nodes 2 to top/bottom
							if (n->nodeType == ESCAPE_WRN)
							{
								n->AddUniqueConnection(grid->at(wx)->at(wy-2));
								n->AddUniqueConnection(grid->at(wx)->at(wy+2));
							}
						}
						// END DTG Debugging.

						// Connect escape nodes to super-escape
						if (n->nodeType == ESCAPE_WRN || n->nodeType == PIN_WRN || n->nodeType == EMPTY_PIN_WRN)
						{
							// ...but only if the node is on the peripheral of the graph
							if (n->wgX == 0 || n->wgX == wireGridSizeX-1 || n->wgY == 0 || n->wgY == wireGridSizeY-1)
								n->AddUniqueConnection(superEscape);
						}
					}
				}
			}
		}
	}



	// Debug print
	/*cout << "Tile " << 0 << ", " << 4 << endl;
	WireRouteTile *t = model->at(0)->at(4);
	for (int wy = 0; wy < tileGridSize; wy++)
	{
		for (int wx = 0; wx < tileGridSize; wx++)
		{
			WireRouteNode *n = t->nodCols->at(wx)->at(wy);

			if (!n)
				cout << " \t";
			else if (n->nodeType == PIN_WRN)
				cout << n->name << "\t";
			else if (n->nodeType == EMPTY_PIN_WRN)
				cout << n->name << "\t";
			else if (n->nodeType == ESCAPE_WRN)
				cout << "E\t";
			else if (n->nodeType == INTERNAL_WRN)
				cout << "I\t";
			else if (n->nodeType == SUPER_ESCAPE_WRN)
				cout << "S\t";
		}
		cout << endl;
	}*/
	/*for (int wy = 0; wy < tileGridSize; wy++)
	{
		for (int wx = 0; wx < tileGridSize; wx++)
		{
			WireRouteNode *n = t->nodCols->at(wx)->at(wy);
			if (n)
			{
				if (n->nodeType == PIN_WRN)
					cout << n->name << endl;
				if (n->nodeType == EMPTY_PIN_WRN)
					cout << n->name << endl;
				if (n->nodeType == ESCAPE_WRN)
					cout << "E" << "-W(" << wx << "," << wy << ")" << endl;
				if (n->nodeType == INTERNAL_WRN)
					cout << "I" << "-W(" << wx << "," << wy << ")" << endl;
				if (n->nodeType == SUPER_ESCAPE_WRN)
					cout << "S" << "-W(" << wx << "," << wy << ")" << endl;

				for (int i = 0; i < n->neighbors.size(); i++)
				{
					WireRouteNode *nn = n->neighbors.at(i);

					if (nn->nodeType == PIN_WRN)
						cout << "\t" << nn->name << endl;
					if (nn->nodeType == EMPTY_PIN_WRN)
						cout << "\t" << nn->name << endl;
					if (nn->nodeType == ESCAPE_WRN)
						cout << "\t" << "E" << "-W(" << nn->wgX << "," << nn->wgY << ")" << endl;
					if (nn->nodeType == INTERNAL_WRN)
						cout << "\t" << "I" << "-W(" << nn->wgX << "," << nn->wgY << ")" << endl;
					if (nn->nodeType == SUPER_ESCAPE_WRN)
						cout << "\t" << "S" << "-W(" << nn->wgX << "," << nn->wgY << ")" << endl;
				}
			}
		}
		cout << endl;
	}*/


	/*cout << "Tile " << 15 << ", " << 4 << endl;
	t = model->at(15)->at(4);
	for (int wy = 0; wy < tileGridSize; wy++)
	{
		for (int wx = 0; wx < tileGridSize; wx++)
		{
			WireRouteNode *n = t->nodCols->at(wx)->at(wy);

			if (!n)
				cout << " \t";
			else if (n->nodeType == PIN_WRN)
				cout << n->name << "\t";
			else if (n->nodeType == EMPTY_PIN_WRN)
				cout << n->name << "\t";
			else if (n->nodeType == ESCAPE_WRN)
				cout << "E\t";
			else if (n->nodeType == INTERNAL_WRN)
				cout << "I\t";
			else if (n->nodeType == SUPER_ESCAPE_WRN)
				cout << "S\t";
		}
		cout << endl;
	}*/
	/*for (int wy = 0; wy < tileGridSize; wy++)
	{
		for (int wx = 0; wx < tileGridSize; wx++)
		{
			WireRouteNode *n = t->nodCols->at(wx)->at(wy);
			if (n)
			{
				if (n->nodeType == PIN_WRN)
					cout << n->name << endl;
				if (n->nodeType == EMPTY_PIN_WRN)
					cout << n->name << endl;
				if (n->nodeType == ESCAPE_WRN)
					cout << "E" << "-W(" << wx << "," << wy << ")" << endl;
				if (n->nodeType == INTERNAL_WRN)
					cout << "I" << "-W(" << wx << "," << wy << ")" << endl;
				if (n->nodeType == SUPER_ESCAPE_WRN)
					cout << "S" << "-W(" << wx << "," << wy << ")" << endl;

				for (int i = 0; i < n->neighbors.size(); i++)
				{
					WireRouteNode *nn = n->neighbors.at(i);

					if (nn->nodeType == PIN_WRN)
						cout << "\t" << nn->name << endl;
					if (nn->nodeType == EMPTY_PIN_WRN)
						cout << "\t" << nn->name << endl;
					if (nn->nodeType == ESCAPE_WRN)
						cout << "\t" << "E" << "-W(" << nn->wgX << "," << nn->wgY << ")" << endl;
					if (nn->nodeType == INTERNAL_WRN)
						cout << "\t" << "I" << "-W(" << nn->wgX << "," << nn->wgY << ")" << endl;
					if (nn->nodeType == SUPER_ESCAPE_WRN)
						cout << "\t" << "S" << "-W(" << nn->wgX << "," << nn->wgY << ")" << endl;
				}
			}
		}
		cout << endl;
	}*/
	//exit(0);










	// Now, remove the connections where there are I/O ports
	if (isAvoidingIOPorts())
	{
		for (unsigned i = 0; i < arch->getIoPorts()->size(); i++)
		{
			IoPort *p = arch->getIoPorts()->at(i);
			int pos = p->getPosXY();

			WireRouteTile *t = NULL;
			if (p->getSide() == NORTH)
			{
				//if (pos > 0)
				//{
					t = model->at(pos)->at(0);
					for (int j = 0; j < t->nodCols->size(); j++)
						if (t->nodCols->at(j)->at(0) != NULL && t->nodCols->at(j)->at(0)->nodeType == ESCAPE_WRN)
							t->nodCols->at(j)->at(0)->RemoveUniqueConnection(superEscape);
				//}
				//if (pos < xDim-1)
				//{
					t = model->at(pos+1)->at(0);
					for (int j = 0; j < t->nodCols->size(); j++)
						if (t->nodCols->at(j)->at(0) != NULL && t->nodCols->at(j)->at(0)->nodeType == ESCAPE_WRN)
							t->nodCols->at(j)->at(0)->RemoveUniqueConnection(superEscape);
				//}
				pins->at(pos)->at(0)->RemoveUniqueConnection(superEscape);
			}
			else if (p->getSide() == SOUTH)
			{
				//if (pos > 0)
				//{
					t = model->at(pos)->at(yDim-2);
					for (int j = 0; j < t->nodCols->size(); j++)
						if (t->nodCols->at(j)->at(tileGridSize-1) != NULL && t->nodCols->at(j)->at(tileGridSize-1)->nodeType == ESCAPE_WRN)
							t->nodCols->at(j)->at(tileGridSize-1)->RemoveUniqueConnection(superEscape);
				//}
				//if (pos < xDim-1)
				//{
					t = model->at(pos+1)->at(yDim-2);
					for (int j = 0; j < t->nodCols->size(); j++)
						if (t->nodCols->at(j)->at(tileGridSize-1) != NULL && t->nodCols->at(j)->at(tileGridSize-1)->nodeType == ESCAPE_WRN)
							t->nodCols->at(j)->at(tileGridSize-1)->RemoveUniqueConnection(superEscape);
				//}
				pins->at(pos)->at(yDim-1)->RemoveUniqueConnection(superEscape);
			}
			else if (p->getSide() == WEST)
			{
				//if (pos > 0)
				//{
					t = model->at(0)->at(pos);
					for (int j = 0; j < t->nodCols->at(0)->size(); j++)
						if (t->nodCols->at(0)->at(j) != NULL && t->nodCols->at(0)->at(j)->nodeType == ESCAPE_WRN)
							t->nodCols->at(0)->at(j)->RemoveUniqueConnection(superEscape);
				//}
				//if (pos < xDim-1)
				//{
					t = model->at(0)->at(pos+1);
					for (int j = 0; j < t->nodCols->at(0)->size(); j++)
						if (t->nodCols->at(0)->at(j) != NULL && t->nodCols->at(0)->at(j)->nodeType == ESCAPE_WRN)
							t->nodCols->at(0)->at(j)->RemoveUniqueConnection(superEscape);
				//}
				pins->at(0)->at(pos)->RemoveUniqueConnection(superEscape);
			}
			else if (p->getSide() == EAST)
			{
				//if (pos > 0)
				//{
					t = model->at(xDim-2)->at(pos);
					for (int j = 0; j < t->nodCols->at(0)->size(); j++)
						if (t->nodCols->at(tileGridSize-1)->at(j) != NULL && t->nodCols->at(tileGridSize-1)->at(j)->nodeType == ESCAPE_WRN)
							t->nodCols->at(tileGridSize-1)->at(j)->RemoveUniqueConnection(superEscape);
				//}
				//if (pos < xDim-1)
				//{
					t = model->at(xDim-2)->at(pos+1);
					for (int j = 0; j < t->nodCols->at(0)->size(); j++)
						if (t->nodCols->at(tileGridSize-1)->at(j) != NULL && t->nodCols->at(tileGridSize-1)->at(j)->nodeType == ESCAPE_WRN)
							t->nodCols->at(tileGridSize-1)->at(j)->RemoveUniqueConnection(superEscape);
				//}
				pins->at(xDim-1)->at(pos)->RemoveUniqueConnection(superEscape);
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////
// Destructor - Deletes all the nodes and edges created by constructor
///////////////////////////////////////////////////////////////////////////////////
DiagonalWireRoutingModel::~DiagonalWireRoutingModel()
{
	// Delete pins
	if (pins)
	{
		while (!pins->empty())
		{
			vector<WireRouteNode*> *v = pins->back();
			pins->pop_back();
			v->clear();
			delete v;
		}
		delete pins;
	}

	// Delete tiles
	if (model)
	{
		while (!model->empty())
		{
			vector<WireRouteTile*> *v = model->back();
			model->pop_back();
			while (!v->empty())
			{
				WireRouteTile *t = v->back();
				v->pop_back();
				delete t;
			}
			delete v;
		}
		delete model;
	}

	// Delete pinGroups structure
	map<int, vector<WireRouteNode *> *>::iterator it = pinGroups->begin();
	for (; it != pinGroups->end(); it++)
	{
		vector<WireRouteNode *> *v = it->second;
		v->clear();
		delete v;
	}
	pinGroups->clear();
	delete pinGroups;

	//delete superEscape; // This is deleted in the following for loop code and will cause a segfault if uncommented

	for (int i = 0; i < allNodes->size(); i++)
		delete allNodes->at(i);

	allNodes->clear();

	delete allNodes;
}

///////////////////////////////////////////////////////////////
//Creates a graph of the Network flow model in .dot format
///////////////////////////////////////////////////////////////
void DiagonalWireRoutingModel::OutputGraphFile(string filename, bool showSuperEscape)
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
	out<<"graph G {\n";

	// Print nodes & edges
	for (unsigned i = 0; i < allNodes->size(); i++)
	{
		WireRouteNode *n = allNodes->at(i);
		string colorName;
		string label = "";
		stringstream ss("");

		// Select color based on node type
		if (n->nodeType == ESCAPE_WRN)
		{
			colorName = "lightsteelblue";
			ss << "Ew(" << n->wgX << "," << n->wgY << ")";
			label = ss.str();
		}
		else if (n->nodeType == SUPER_ESCAPE_WRN)
		{
			colorName = "olivedrab";
			label = "SE";
		}
		else if (n->nodeType == PIN_WRN)
		{
			colorName = "goldenrod";
			ss << n->name << "w(" << n->wgX << "," << n->wgY << ")";
			label = ss.str();
		}
		else if (n->nodeType == EMPTY_PIN_WRN)
		{
			colorName = "chartreuse3";
			ss << n->name << "w(" << n->wgX << "," << n->wgY << ")";
			label = ss.str();
		}
		//else if (n->nodeType == PERIPH_DIR_WRN)
		//	colorName = "tan";
		else if (n->nodeType == INTERNAL_WRN)
		{
			colorName = "darkorange";
			ss << "Iw(" << n->wgX << "," << n->wgY << ")";
			label = ss.str();
		}
		else
			cout << "WHO AM I???" << endl;

		if (n->nodeType != SUPER_ESCAPE_WRN || (n->nodeType == SUPER_ESCAPE_WRN && showSuperEscape))
			out << n->id << " [label = \"" << label << "\" fillcolor=" << colorName << ", style=filled];\n";

		int seId = superEscape->id;

		// Print edges
		for (unsigned j = 0; j < n->neighbors.size(); j++)
		{
			WireRouteNode *to = n->neighbors.at(j);

			// If haven't already added this edge
			if (n->id < to->id)
				if ( !((n->id == seId || to->id == seId) && !showSuperEscape) )
					out << n->id << " -- " << to->id << ";\n";
		}
	}

	// Closing bracket
	out<<"}\n";
	out.close();
}

///////////////////////////////////////////////////////////////
// Returns the pin at thge requested XY-coordinate. The pin array
// has a ring of non-pins around the perimiter, so the XY values
// must be incremented to get the proper pin.
///////////////////////////////////////////////////////////////
WireRouteNode *DiagonalWireRoutingModel::getPin(int x, int y)
{
	claim(x >= -1, "The X-value for the requested pin must be >= -1");
	claim(y >= -1, "The Y-value for the requested pin must be >= -1");
	claim(x <= arch->getNumCellsX(), "The X-value for the requested pin must be <= the # of pins in the x-dimension");
	claim(y <= arch->getNumCellsY(), "The Y-value for the requested pin must be <= the # of pins in the y-dimension");

	x++;
	y++;
	return pins->at(x)->at(y);
}



/* CODE REPRESENTS THE MODEL WHERE THE PERIPHERAL EDGES HAVE CAPACITY (NOT SURE
 * THIS IS CORRECT AS IT DOESN'T SAY THIS IN THE PAPER.
 *
///////////////////////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////////////////////
DiagonalWireRoutingModel::DiagonalWireRoutingModel()
{
	claim(false, "DiagonalWireRoutingModel constructor must be called with a DmfbArch argument.");
}
///////////////////////////////////////////////////////////////////////////////////
// Constructor - Given the architecture, creates all the nodes and edges to
// represent the model
///////////////////////////////////////////////////////////////////////////////////
DiagonalWireRoutingModel::DiagonalWireRoutingModel(DmfbArch *dmfbArch)
{
	arch = dmfbArch;
	int xDim = arch->getNumCellsX();
	int yDim = arch->getNumCellsY();
	vector<vector<int> *> *pinMapping = arch->getPinMapper()->getPinMapping();
	allNodes = new vector<WireRouteNode *>();

	//claim (xDim == pinMapping->size() && yDim == pinMapping->at(0)->size(), "Pin-mapping array must match dimensions of DMFB.");

	claim (arch->getWireRouter()->getNumHorizTracks() == arch->getWireRouter()->getNumVertTracks(), "Number of horizontal and vertical tracks must be equivalent for diagonal wire-routing model.");
	oCap = arch->getWireRouter()->getNumHorizTracks();
	dCap = oCap * 2; // For now, just multiply times 2

	// Create source
	source = new WireRouteNode();
	source->id = next_id++;
	source->name = "SOURCE";
	source->nodeType = SOURCE_WRN;
	allNodes->push_back(source);

	// Create the array of pins
	pins = new vector<vector<WireRouteNode *> *>();
	for (int x = 0; x < xDim; x++)
	{
		vector<WireRouteNode *> *col = new vector<WireRouteNode *>();
		for (int y = 0; y < yDim; y++)
		{
			if (arch->getPinMapper()->getPinMapping()->at(x)->at(y) >= 0)
			{
				WireRouteNode *n = new WireRouteNode();
				n->id = next_id++;
				n->nodeType = PIN_WRN;
				stringstream ss;
				ss << "P(" << x << ", " << y << ")";
				n->name = ss.str();
				col->push_back(n);
				allNodes->push_back(n);
			}
			else
				col->push_back(NULL);
		}
		pins->push_back(col);
	}

	// Create the array of wire routing tiles
	model = new vector<vector<WireRouteTile *> *>();
	for (int x = 0; x < xDim-1; x++)
	{
		vector<WireRouteTile *> *col = new vector<WireRouteTile *>();
		for (int y = 0; y < yDim-1; y++)
		{
			stringstream ss;
			WireRouteTile *t = new WireRouteTile();
			// Add newly created nodes to allNodes list
			allNodes->push_back(t->periphInN);
			allNodes->push_back(t->periphOutN);
			allNodes->push_back(t->periphInE);
			allNodes->push_back(t->periphOutE);
			allNodes->push_back(t->periphInS);
			allNodes->push_back(t->periphOutS);
			allNodes->push_back(t->periphInW);
			allNodes->push_back(t->periphOutW);
			allNodes->push_back(t->centerIn);
			allNodes->push_back(t->centerOut);
			initTileNodes(t, x, y);
			col->push_back(t);
		}
		model->push_back(col);
	}

	// Create sink (s)
	sinkN = new WireRouteNode();
	sinkN->id = next_id++;
	sinkN->name = "N SINK";
	sinkN->nodeType = SINK_WRN;
	allNodes->push_back(sinkN);

	sinkE = new WireRouteNode();
	sinkE->id = next_id++;
	sinkE->name = "E SINK";
	sinkE->nodeType = SINK_WRN;
	allNodes->push_back(sinkE);

	sinkS = new WireRouteNode();
	sinkS->id = next_id++;
	sinkS->name = "S SINK";
	sinkS->nodeType = SINK_WRN;
	allNodes->push_back(sinkS);

	sinkW = new WireRouteNode();
	sinkW->id = next_id++;
	sinkW->name = "W SINK";
	sinkW->nodeType = SINK_WRN;
	allNodes->push_back(sinkW);

	sink = new WireRouteNode();
	sink->id = next_id++;
	sink->name = "SINK";
	sink->nodeType = SINK_WRN;
	allNodes->push_back(sink);

	// Now that the nodes are all properly created and placed, insert all the edges
	for (int x = 0; x < xDim-1; x++)
		for (int y = 0; y < yDim-1; y++)
			initIntraTileEdges(model->at(x)->at(y));
	initInterTileEdges();
	initSourceEdges();
}

///////////////////////////////////////////////////////////////////////////////////
// Destructor - Deletes all the nodes and edges created by constructor
///////////////////////////////////////////////////////////////////////////////////
DiagonalWireRoutingModel::~DiagonalWireRoutingModel()
{
	// Delete pins
	if (pins)
	{
		while (!pins->empty())
		{
			vector<WireRouteNode*> *v = pins->back();
			pins->pop_back();
			while (!v->empty())
			{
				WireRouteNode *n = v->back();
				v->pop_back();
				delete n;
			}
			delete v;
		}
		delete pins;
	}

	// Delete tiles
	if (model)
	{
		while (!model->empty())
		{
			vector<WireRouteTile*> *v = model->back();
			model->pop_back();
			while (!v->empty())
			{
				WireRouteTile *t = v->back();
				v->pop_back();
				delete t;
			}
			delete v;
		}
		delete model;
	}

	delete source;
	delete sink;
	delete sinkN;
	delete sinkE;
	delete sinkS;
	delete sinkW;
	allNodes->clear();
	delete allNodes;
}

///////////////////////////////////////////////////////////////////////////////////
// Initializes the nodes within a tile by giving them a contiguous id, a type and
// name.  Also associates the appropriate pin nodes with the tile.
///////////////////////////////////////////////////////////////////////////////////
void DiagonalWireRoutingModel::initTileNodes(WireRouteTile *t, int tileX, int tileY)
{
	// Set IDs
	t->periphInN->id = next_id++;
	t->periphOutN->id = next_id++;
	t->periphInE->id = next_id++;
	t->periphOutE->id = next_id++;
	t->periphInS->id = next_id++;
	t->periphOutS->id = next_id++;
	t->periphInW->id = next_id++;
	t->periphOutW->id = next_id++;
	t->centerIn->id = next_id++;
	t->centerOut->id = next_id++;

	// Set Node Types
	t->periphInN->nodeType = PERIPH_DIR_WRN;
	t->periphOutN->nodeType = PERIPH_DIR_WRN;
	t->periphInE->nodeType = PERIPH_DIR_WRN;
	t->periphOutE->nodeType = PERIPH_DIR_WRN;
	t->periphInS->nodeType = PERIPH_DIR_WRN;
	t->periphOutS->nodeType = PERIPH_DIR_WRN;
	t->periphInW->nodeType = PERIPH_DIR_WRN;
	t->periphOutW->nodeType = PERIPH_DIR_WRN;
	t->centerIn->nodeType = CENTER_WRN;
	t->centerOut->nodeType = CENTER_WRN;

	// Set Node names
	stringstream nodeName;
	nodeName << "(" << tileX << "-" << tileX+1 << ", " << tileY << "-" << tileY+1 << ")";
	t->periphInN->name = "Nin" + nodeName.str();
	t->periphOutN->name = "Nout" + nodeName.str();
	t->periphInE->name = "Ein" + nodeName.str();
	t->periphOutE->name = "Eout" + nodeName.str();
	t->periphInS->name = "Sin" + nodeName.str();
	t->periphOutS->name = "Sout" + nodeName.str();
	t->periphInW->name = "Win" + nodeName.str();
	t->periphOutW->name = "Wout" + nodeName.str();
	t->centerIn->name = "Cin" + nodeName.str();
	t->centerOut->name = "Cout" + nodeName.str();

	// Now set the pins from the pin array
	t->pinNW = pins->at(tileX)->at(tileY);
	t->pinNE = pins->at(tileX+1)->at(tileY);
	t->pinSW = pins->at(tileX)->at(tileY+1);
	t->pinSE = pins->at(tileX+1)->at(tileY+1);
}

///////////////////////////////////////////////////////////////////////////////////
// Creates edges to connect the internal node of a given WireRouteTile t
///////////////////////////////////////////////////////////////////////////////////
void DiagonalWireRoutingModel::initIntraTileEdges(WireRouteTile *t)
{
	// Add peripheral-to-peripheral edges
	int periphCap = (int)floor((double)oCap / 2.0);
	t->periphOutN->AddEdge(periphCap, 0, t->periphInE);
	t->periphOutE->AddEdge(periphCap, 0, t->periphInN);
	t->periphOutE->AddEdge(periphCap, 0, t->periphInS);
	t->periphOutS->AddEdge(periphCap, 0, t->periphInE);
	t->periphOutS->AddEdge(periphCap, 0, t->periphInW);
	t->periphOutW->AddEdge(periphCap, 0, t->periphInS);
	t->periphOutW->AddEdge(periphCap, 0, t->periphInN);
	t->periphOutN->AddEdge(periphCap, 0, t->periphInW);

	// Add intra-peripheral edges
	t->periphInN->AddEdge(oCap, 0, t->periphOutN);
	t->periphInE->AddEdge(oCap, 0, t->periphOutE);
	t->periphInS->AddEdge(oCap, 0, t->periphOutS);
	t->periphInW->AddEdge(oCap, 0, t->periphOutW);

	// Add intra-center edge
	int centerCap = dCap - ((int)(2.0 * floor((double)oCap / 2.0) ));
	t->centerIn->AddEdge(centerCap, 0, t->centerOut);

	// Add peripheral-to-center edges
	t->periphOutN->AddEdge(GetInfiniteCapacity(), 0, t->centerIn);
	t->periphOutE->AddEdge(GetInfiniteCapacity(), 0, t->centerIn);
	t->periphOutS->AddEdge(GetInfiniteCapacity(), 0, t->centerIn);
	t->periphOutW->AddEdge(GetInfiniteCapacity(), 0, t->centerIn);

	// Add center-to-peripheral edges
	t->centerOut->AddEdge(GetInfiniteCapacity(), 0, t->periphInN);
	t->centerOut->AddEdge(GetInfiniteCapacity(), 0, t->periphInE);
	t->centerOut->AddEdge(GetInfiniteCapacity(), 0, t->periphInS);
	t->centerOut->AddEdge(GetInfiniteCapacity(), 0, t->periphInW);

	// Add pin-to-peripheral edges
	if (t->pinNW)
		t->pinNW->AddEdge(1, 0, t->periphInN);
	if (t->pinNE)
		t->pinNE->AddEdge(1, 0, t->periphInE);
	if (t->pinSE)
		t->pinSE->AddEdge(1, 0, t->periphInS);
	if (t->pinSW)
		t->pinSW->AddEdge(1, 0, t->periphInW);
}

///////////////////////////////////////////////////////////////////////////////////
// Creates edges that link the nodes of a tile to its neighboring tile; also adds
// edges to link border tiles to the sink.
///////////////////////////////////////////////////////////////////////////////////
void DiagonalWireRoutingModel::initInterTileEdges()
{
	int txDim = arch->getNumCellsX() - 1; // Tile x-dim
	int tyDim = arch->getNumCellsY() - 1; // Tile y-dim

	// Inter-tile and tile-to-sink edges
	for (int x = 0; x < txDim; x++)
	{
		for (int y = 0; y < tyDim; y++)
		{
			WireRouteTile *t = model->at(x)->at(y);

			// North connection
			if (y == 0)
				t->periphOutN->AddEdge(oCap, 0, sinkN);
			else
				t->periphOutN->AddEdge(oCap, 0, model->at(x)->at(y-1)->periphInS);

			// East connection
			if (x == txDim-1)
				t->periphOutE->AddEdge(oCap, 0, sinkE);
			else
				t->periphOutE->AddEdge(oCap, 0, model->at(x+1)->at(y)->periphInW);

			// South connection
			if (y == tyDim-1)
				t->periphOutS->AddEdge(oCap, 0, sinkS);
			else
				t->periphOutS->AddEdge(oCap, 0, model->at(x)->at(y+1)->periphInN);

			// West connection
			if (x == 0)
				t->periphOutW->AddEdge(oCap, 0, sinkW);
			else
				t->periphOutW->AddEdge(oCap, 0, model->at(x-1)->at(y)->periphInE);
		}
	}

	// Directional pre-sinks to super-sink
	sinkN->AddEdge(GetInfiniteCapacity(), 0, sink);
	sinkE->AddEdge(GetInfiniteCapacity(), 0, sink);
	sinkS->AddEdge(GetInfiniteCapacity(), 0, sink);
	sinkW->AddEdge(GetInfiniteCapacity(), 0, sink);
}

///////////////////////////////////////////////////////////////////////////////////
// Adds edges from the source to the desired escape pins.
///////////////////////////////////////////////////////////////////////////////////
void DiagonalWireRoutingModel::initSourceEdges()
{
	// For now, just assume each each pin will be escaped to
	// as an independent pin
	for (int x = 0; x < arch->getNumCellsX(); x++)
		for (int y = 0; y < arch->getNumCellsY(); y++)
			if (pins->at(x)->at(y))
				source->AddEdge(1, 0, pins->at(x)->at(y));
}
*/

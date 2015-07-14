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
 * Source: kamer_ll_placer.cc													*
 * Original Code Author(s): Dan Grissom											*
 * Original Completion/Release Date: April 19, 2013								*
 *																				*
 * Details: N/A																	*
 *																				*
 * Revision History:															*
 * WHO		WHEN		WHAT													*
 * ---		----		----													*
 * FML		MM/DD/YY	One-line description									*
 *-----------------------------------------------------------------------------*/

#include "../../Headers/Placer/kamer_ll_placer.h"
#include "../../Headers/Util/sort.h"
#include <sys/time.h>

///////////////////////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////////////////////
KamerLlPlacer::KamerLlPlacer()
{
	//nodes = new set<KamerLlNode *>();
	board = NULL;
	oell = NULL;
	iell = NULL;
	rwll = NULL;
	debugPrint = false;
}

///////////////////////////////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////////////////////////////
KamerLlPlacer::~KamerLlPlacer()
{
	/*set<KamerLlNode *>::iterator it = nodes->begin();
	for (; it != nodes->end(); it++)
	{
		KamerLlNode *n = *it;
		nodes->erase(n);
		delete n;
	}*/

	// Delete 2D board of KamerCells
	if (board)
	{
		while (!board->empty())
		{
			vector<KamerCell*> *v = board->back();
			board->pop_back();
			while (!v->empty())
			{
				KamerCell *c = v->back();
				v->pop_back();
				delete c;
			}
			delete v;
		}
		delete board;
	}

	// Clear all linked lists
	vector<KamerLlNode *> llLeaders;
	llLeaders.push_back(iell);
	llLeaders.push_back(oell);
	llLeaders.push_back(rwll);
	for (unsigned i = 0; i < llLeaders.size(); i++)
	{
		KamerLlNode *remove = llLeaders.at(i);
		while (remove != NULL)
		{
			KamerLlNode *d = remove;
			d->next = NULL;
			d->last = NULL;
			remove = remove->next;
			delete d;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////
// Inserts node into the linked list at the appropriate place. Highest heights
// first b/c highest y-values are at bottom.  Returns the new In/Out edge.
///////////////////////////////////////////////////////////////////////////////////
KamerLlNode * KamerLlPlacer::insertInOutNode(KamerNodeType type, string modName, int height, int left, int right, unsigned long long endTS)
{
	KamerLlNode *n = new KamerLlNode();
	n->nodeType = type;
	n->modName = modName;
	n->height = height;
	n->left = left;
	n->right = right;
	n->endTS = endTS;

	bool inserted = false;
	KamerLlNode *search = NULL;

	if (n->nodeType == OE)
	{
		if (oell == NULL)
		{
			oell = n;
			n->last = NULL;
			n->next = NULL;
			inserted = true;
		}
		else
			search = oell;
	}
	else if (n->nodeType == IE)
	{
		if (iell == NULL)
		{
			iell = n;
			n->last = NULL;
			n->next = NULL;
			inserted = true;
		}
		else
			search = iell;
	}


	// Search through and insert new node into the
	// appropriate list in the appropriate place
	KamerLlNode *lastNode = NULL;
	while (search != NULL)
	{
		if (search->height <= n->height)
		{
			// Insert here
			KamerLlNode *pre = search->last;
			KamerLlNode *post = search;

			if (pre == NULL && n->nodeType == IE)
				iell = n;
			else if (pre == NULL && n->nodeType == OE)
				oell = n;
			else if (pre != NULL)
				pre->next = n;
			n->last = pre;
			n->next = post;
			post->last = n;

			inserted = true;
			break;
		}
		else
		{
			lastNode = search;
			search = search->next;
		}

	}
	if (!inserted)
	{
		lastNode->next = n;
		n->last = lastNode;
		n->next = NULL;
	}
	return n;
}

///////////////////////////////////////////////////////////////////////////////////
// Inserts node into the linked list at the appropriate place. Highest heights
// first b/c highest y-values are at bottom.
///////////////////////////////////////////////////////////////////////////////////
void KamerLlPlacer::insertRwNode(int height, int left, int right)
{
	KamerLlNode *n = new KamerLlNode();
	n->nodeType = RW;
	n->height = height;
	n->left = left;
	n->right = right;

	KamerLlNode *search = NULL;

	if (rwll == NULL)
	{
		rwll = n;
		n->last = NULL;
		n->next = NULL;
		return;
	}
	else
		search = rwll;

	// Search through and insert new node into the
	// appropriate list in the appropriate place
	bool inserted = false;
	KamerLlNode *lastNode = NULL;
	while (search != NULL)
	{
		if (search->height <= n->height)
		{
			// Insert here
			KamerLlNode *pre = search->last;
			KamerLlNode *post = search;

			if (pre == NULL)
				rwll = n;
			else
				pre->next = n;
			n->last = pre;
			n->next = post;
			post->last = n;

			inserted = true;
			break;
		}
		else
		{
			lastNode = search;
			search = search->next;
		}

	}
	if (!inserted)
	{
		lastNode->next = n;
		n->last = lastNode;
		n->next = NULL;
	}
}

///////////////////////////////////////////////////////////////////////////////////
// Given an in-edge, finds and returns the FRW/RWs below the in-edge that overlap
// any of the given in-edge (if one exists)...this is used to create new max
// empty rectangles. Cycles through RWs and finds all RWs that are lower than the
// given in-edge and overlap.
///////////////////////////////////////////////////////////////////////////////////
vector<KamerLlNode *> KamerLlPlacer::getRwsBelowIE(KamerLlNode *ie)
{
	vector<KamerLlNode *> rwsBelowIe;
	KamerLlNode *search = NULL;

	if (rwll == NULL)
		return rwsBelowIe; // No RWs
	else
		search = rwll;

	// Search for all RWs that are below the given IE
	while (search != NULL && search->height > ie->height)
	{
		// If overlapping, then we found a RW for a maximum empty rectangle
		if ( (ie->left >= search->left && ie->left <= search->right) || (ie->right >= search->left && ie->right <= search->right) ||
				(search->left >= ie->left && search->left <= ie->right) || (search->right >= ie->left && search->right <= ie->right)
				|| (ie->left == search->left && ie->right == search->right) )
			rwsBelowIe.push_back(search);
		search = search->next;
	}
	return rwsBelowIe;
}

///////////////////////////////////////////////////////////////////////////////////
// Given a MFR, width and height (with interference regions (IR) in dimensions) and
// an external resource being targeted/searched for, returns true if can place the
// module in the MFR in such a way that at least 1 cell of the module (assuming
// a 1-cell IR surrounding module) overlaps with the targeted external resource.
// If returns true, modPlacedTop/Left will hold the top/left coordinates of the
// module (not the IR, but the actual module).
//
// This algorithm works in a brute-force manner by placing the bottom-left corner
// of the module's IR in the bottom right corner of the mfr and them moving it one
// cell up or right until it finds a suitable location.
///////////////////////////////////////////////////////////////////////////////////
bool KamerLlPlacer::canPlaceSpecialModuleInMFR(MaxFreeRect *mfr, OperationType targetType, vector<vector<ResourceType>*> *cellType, int wIr, int hIr, int *modPlacedTop, int *modPlacedLeft)
{
	int vrc = vCellsBetweenModIR; // vert routing cells
	int hrc = hCellsBetweenModIR; // horiz. routing cells

	// If operation does not require external resources, return true
	if (!(targetType == HEAT || targetType == DETECT))
		return true;

	// Search all valid module locations in the MFR
	for (int x = mfr->left; x <= mfr->right-wIr+1; x++)
	{
		for (int y = mfr->lower-hIr+1; y >= mfr->upper; y--)
		{
			// Just search each module location, even though will cause some cells
			// to be examined multiple times...code is much cleaner and problem is
			// small enough
			for (int xMod = x+1+hrc; xMod <= x+wIr-2-hrc; xMod++) // -2 for IR
			{
				for (int yMod = y+1+vrc; yMod <= y+hIr-2-vrc; yMod++) // -2 for IR
				{
					if ((cellType->at(xMod)->at(yMod) == H_RES && targetType == HEAT) || (cellType->at(xMod)->at(yMod) == D_RES && targetType == DETECT))
					{
						*modPlacedLeft = x+1+hrc;
						*modPlacedTop = y+1+vrc;
						return true;
					}
				}
			}
		}
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////
// Removes node from the linked list at the appropriate place. Highest heights
// first b/c highest y-values are at bottom.
///////////////////////////////////////////////////////////////////////////////////
void KamerLlPlacer::removeInOutNode(KamerNodeType type, int height, int left, int right)
{
	KamerLlNode *search = NULL;
	if (type == OE)
		search = oell;
	else if (type == IE)
		search = iell;

	// Search through and remove node from the appropriate list
	bool removed = false;
	KamerLlNode *ge = NULL;
	while (search != NULL)
	{
		if (search->height == height && search->left == left && search->right == right)
		{
			ge = search->ref;
			if (search->next == NULL && search->last == NULL)
			{	// Only one node in list
				if (type == OE)
					oell = NULL;
				else if (type == IE)
					iell = NULL;
				delete search;
				removed = true;
				break;
			}
			else if (search->next == NULL)
			{	// Node is at end of list
				KamerLlNode *pre = search->last;
				pre->next = NULL;
				delete search;
				removed = true;
				break;
			}
			else if (search->last == NULL)
			{	// Node is at beginning of list
				KamerLlNode *post = search->next;
				post->last = NULL;

				if (type == OE)
					oell = post;
				else if (type == IE)
					iell = post;
				delete search;
				removed = true;
				break;
			}
			else
			{	// Node is in middle of list
				KamerLlNode *pre = search->last;
				KamerLlNode *post = search->next;
				pre->next = post;
				post->last = pre;

				delete search;
				removed = true;
				break;
			}
		}
		else
			search = search->next;

	}
	if (!removed)
	{
		claim(false, "Requested node to remove not found in linked list.");
	}
}

///////////////////////////////////////////////////////////////////////////////////
// Removes node from the linked list at the appropriate place. Highest heights
// first b/c highest y-values are at bottom.
///////////////////////////////////////////////////////////////////////////////////
void KamerLlPlacer::removeNode(KamerLlNode *remove)
{
	KamerNodeType type = remove->nodeType;

	// Remove node from the appropriate list
	if (remove->next == NULL && remove->last == NULL)
	{	// Only one node in list
		if (type == OE)
			oell = NULL;
		else if (type == IE)
			iell = NULL;
		else if (type == RW)
			rwll = NULL;
		delete remove;
	}
	else if (remove->next == NULL)
	{	// Node is at end of list
		KamerLlNode *pre = remove->last;
		pre->next = NULL;
		delete remove;
	}
	else if (remove->last == NULL)
	{	// Node is at beginning of list
		KamerLlNode *post = remove->next;
		post->last = NULL;

		if (type == OE)
			oell = post;
		else if (type == IE)
			iell = post;
		else if (type == RW)
			rwll = post;
		delete remove;
	}
	else
	{	// Node is in middle of list
		KamerLlNode *pre = remove->last;
		KamerLlNode *post = remove->next;
		pre->next = post;
		post->last = pre;
		delete remove;
	}
}

///////////////////////////////////////////////////////////////////////////////////
// Removes all in/out-edges expiring before the startTS and updates the KamerBoard
// as necessary.
// This function will NOT be efficient as it searches for expired edges and then
// calls the remove function which searches through the same list to delete it.
// Should be optimized later.
///////////////////////////////////////////////////////////////////////////////////
void KamerLlPlacer::reinitDataStructsToTS(unsigned long long startTS)
{
	vector<KamerLlNode *> ins;
	vector<KamerLlNode *> outs;

	// Start by finding the pairs of in/out edges that are expiring
	KamerLlNode *searchIn = iell;
	while (searchIn != NULL)
	{
		if (searchIn->endTS <= startTS)
		{
			ins.push_back(searchIn);
			KamerLlNode *searchOut = oell;
			while (searchOut != NULL)
			{
				if (searchIn->endTS == searchOut->endTS && searchIn->left == searchOut->left &&
						searchIn->right == searchOut->right && searchOut->height < searchIn->height &&
						searchIn->modName == searchOut->modName)
				{
					outs.push_back(searchOut);
					break;
				}
				searchOut = searchOut->next;
			}
		}
		searchIn = searchIn->next;
	}

	claim(ins.size() == outs.size(), "Did not find an equal number of in- and out-edges");

	// Now remove them from the board and the lists
	for (unsigned i = 0; i < ins.size(); i++)
	{
		addRemoveModuleFromKamerBoard(ins.at(i), outs.at(i));
		removeNode(ins.at(i));
		removeNode(outs.at(i));
	}
}

///////////////////////////////////////////////////////////////////////////////////
// Prints out the In/Out edges for debugging purposes. Gives the information of the
// node just placed, if given (passing in NULL will skip this output).
///////////////////////////////////////////////////////////////////////////////////
void KamerLlPlacer::debugPrintLinkedLists(AssayNode *justPlaced)
{
	if (!debugPrintEnabled())
		return;

	if (justPlaced)
	{
		cout << "After placing " << justPlaced->name << " at (";
		cout << justPlaced->reconfigMod->getLX() << "LX, " <<  justPlaced->reconfigMod->getRX() << "RX, ";
		cout << justPlaced->reconfigMod->getBY() << "BY, " << justPlaced->reconfigMod->getTY() << "TY) ";
		cout << "at TS " << justPlaced->startTimeStep << endl;
		cout << "In/Out Edges are:" << endl;
	}

	cout << "In Edges: ";
	KamerLlNode *it = iell;
	while (it != NULL)
	{
		cout << "(" << it->left << "L, " << it->right << "R, " << it->height << "B, " << it->endTS << "END), ";
		it = it->next;
	}

	cout << endl << "Out Edges: ";
	it = oell;
	while (it != NULL)
	{
		cout << "(" << it->left << "L, " << it->right << "R, " << it->height << "B, " << it->endTS << "END), ";
		it = it->next;
	}

	cout << endl << "Rectangular Wells: ";
	it = rwll;
	while (it != NULL)
	{
		cout << "(" << it->height << "H, " << it->left << "L, " << it->right << "R), ";
		it = it->next;
	}
	cout << endl << endl;
}

///////////////////////////////////////////////////////////////////////////////////
// Prints out the in/out edges, as well as module placements and forbidden Kamer cells
///////////////////////////////////////////////////////////////////////////////////
void KamerLlPlacer::debugPrintKamerBoard(DmfbArch *arch)
{
	if (!debugPrintEnabled())
		return;

	cout << "Kamer Board Status:" << endl;

	// Print X-coordinates
	cout << "\\ ";
	for (int x = 0; x < arch->getNumCellsX(); x++)
		cout << x % 10 << " ";
	cout << endl;

	for (int y = 0; y < arch->getNumCellsY(); y++)
	{
		for (int x = 0; x < arch->getNumCellsX(); x++)
		{
			// Print Y-coordinates
			if (x == 0)
				cout << y % 10;
			cout << " " << board->at(x)->at(y)->status;
		}
		cout << endl;
	}
	cout << endl;
}

///////////////////////////////////////////////////////////////////////////////////
// Given In- and Out-edges, adds the edges and module to the board for record
// keeping.  If the In- and out-edges already exist, will remove them (will remove
// the module too).
///////////////////////////////////////////////////////////////////////////////////
void KamerLlPlacer::addRemoveModuleFromKamerBoard(KamerLlNode * in, KamerLlNode * out)
{
	// Check bounds of in- and out-edges to see if exist on board
	bool inExists = board->at(in->left)->at(in->height)->status == 'i' && board->at(in->right)->at(in->height)->status == 'i';
	bool outExists = board->at(out->left)->at(out->height)->status == 'o' && board->at(out->right)->at(out->height)->status == 'o';

	claim(in->left == out->left && in->right == out->right, "Left & Right boundaries of In/Out edges must be identical.");
	claim(in->height > out->height, "In-edge must be lower (higher y dimension) than the out-edge.");

	claim(!(inExists ^ outExists), "Given In- and Out-edges don't represent a module b/c both do not exist on board.");

	// If both exist, remove from board
	if (inExists && outExists)
	{
		for (int x = in->left; x <= in->right; x++)
			for (int y = out->height; y <= in->height; y++)
				board->at(x)->at(y)->status = '-';
	}
	else // Edges/module does not exist, add to board
	{
		for (int x = in->left; x <= in->right; x++)
		{
			for (int y = out->height; y <= in->height; y++)
			{
				if (y == in->height)
					board->at(x)->at(y)->status = 'i';
				else if (y == out->height)
					board->at(x)->at(y)->status = 'o';
				else
					board->at(x)->at(y)->status = 'm';
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////
// Given and In- or Out-edge, adds the edge to the board for record keeping. If the
// edge already exists on the board, removes it.
///////////////////////////////////////////////////////////////////////////////////
void KamerLlPlacer::addRemoveEdgeFromKamerBoard(KamerLlNode * edge)
{
	// If is in-edge with height -1, then off the board; in-edge is intrinsic with DMFB top border and not represented on Kamer board
	if (edge->nodeType == IE && edge->height == -1)
		return;

	// Check bounds of in- and out-edges to see if exist on board
	char type;
	if (edge->nodeType == OE)
		type = 'o';
	else if (edge->nodeType == IE)
		type = 'i';
	else
		claim(false, "Unsupported KamerLlNode type passed into addRemoveEdgeFromKamerBoard()");

	bool exists = board->at(edge->left)->at(edge->height)->status == type && board->at(edge->right)->at(edge->height)->status == type;

	if (exists)
		for (int x = edge->left; x <= edge->right; x++)
			board->at(x)->at(edge->height)->status = '-';
	else // Edges/module does not exist, add to board
		for (int x = edge->left; x <= edge->right; x++)
			board->at(x)->at(edge->height)->status = type;

}
///////////////////////////////////////////////////////////////////////////////////
// Places each operation in the scheduled DAG onto the DMFB at the appropriate
// time.
///////////////////////////////////////////////////////////////////////////////////
void KamerLlPlacer::place(DmfbArch *arch, DAG *schedDag, vector<ReconfigModule *> *rModules)
{
	// Hard-coded (for now) MFR selection type; either picks a random valid MFR, or the smallest valid MFR
	enum MfrSelectionType { RANDOM_MFR, SMALLEST_MFR };
	MfrSelectionType mfrSelectionType = SMALLEST_MFR;

	unsigned int seed = time(0);
	cout << "Random seed for KAMER placer: " << seed << endl;
	//unsigned int seed = 1383951530;
	//cout << "Hard-coded seed for KAMER placer: " << seed << endl;
	srand(seed);

	// If true, creates a ring around the perimeter of the DMFB that is kept free for routing
	bool createRoutingPerimeter = true;

	///////////////////////////////////////////////////////////////////////////////////////
	// Initializations
	getAvailResources(arch);
	resetIoResources(arch);
	int INPUT_RES = 0;
	int OUTPUT_RES = 1;

	///////////////////////////////////////////////////////////////////////////////////////
	// Create a map of the fixed-module layout (heaters/detectors)
	int numX = arch->getNumCellsX();
	int numY = arch->getNumCellsY();
	vector<vector<ResourceType> *> *cellType = new vector<vector<ResourceType> *>();
	for (int x = 0; x < numX; x++)
	{
		vector<ResourceType> *col = new vector<ResourceType>();
		for (int y = 0; y < numY; y++)
			col->push_back(BASIC_RES);
		cellType->push_back(col);
	}
	for (unsigned i = 0; i < arch->getExternalResources()->size(); i++)
	{
		FixedModule *fm = arch->getExternalResources()->at(i);
		for (int x = fm->getLX(); x <= fm->getRX(); x++)
		{
			for (int y = fm->getTY(); y <= fm->getBY(); y++)
			{
				claim(x < numX && y < numY, "A fixed module has coordinates that do not fit on the array.");
				claim(cellType->at(x)->at(y) == BASIC_RES, "A cell has already been augmented with a fixed module; cannot augment with two fixed modules.");
				if (fm->getResourceType() == H_RES)
					cellType->at(x)->at(y) = H_RES;
				else if (fm->getResourceType() == D_RES)
					cellType->at(x)->at(y) = D_RES;
				else
					claim(false, "Unknown type of fixed resource.");
			}
		}
	}

	///////////////////////////////////////////////////////////////////////////////////////
	// Initialize 2D Kamer board
	board = new vector<vector<KamerCell *> *>();
	for (int x = 0; x < arch->getNumCellsX(); x++)
	{
		vector<KamerCell *> *col = new vector<KamerCell *>();
		for (int y = 0; y < arch->getNumCellsY(); y++)
		{
			KamerCell *c = new KamerCell();
			c->x = x;
			c->y = y;
			if ((x == 0 || y == 0 || x == arch->getNumCellsX()-1 || y == arch->getNumCellsY()-1) && createRoutingPerimeter)
				c->status = 'f'; // If on border, set to forbidden to leave room for I/O's to route droplets
			else
				c->status = '-';
			col->push_back(c);
		}
		board->push_back(col);
	}

	///////////////////////////////////////////////////////////////////////////////////////
	// Get initial nodes for placement and I/O
	list<AssayNode *> opsToPlace;
	vector<list<AssayNode *> *> ioOps;
	for (int i = 0; i < 2; i++) // 2 for inputs and outputs
		ioOps.push_back(new list<AssayNode*>());
	for (unsigned i = 0; i < schedDag->allNodes.size(); i++)
	{
		AssayNode *n = schedDag->allNodes.at(i);
		if (n->type == DISPENSE)
			ioOps.at(INPUT_RES)->push_back(n);
		else if (n->type == OUTPUT)
			ioOps.at(OUTPUT_RES)->push_back(n);
		else
			opsToPlace.push_back(n);
	}

	///////////////////////////////////////////////////////////////////////////////////////
	// Populate initial data-structure by adding top In-edge to linked lists and KamerBoard.
	if(createRoutingPerimeter)
		addRemoveEdgeFromKamerBoard(insertInOutNode(IE, "TOP_EDGE", 0, 1, arch->getNumCellsX()-2, 1000000000)); // Top is first in-edge (never expires) (leave 1 cell boundary for I/Os)
	else
		addRemoveEdgeFromKamerBoard(insertInOutNode(IE, "TOP_EDGE", -1, 0, arch->getNumCellsX()-1, 1000000000)); // Leaves not boundary for I/Os

	///////////////////////////////////////////////////////////////////////////////////////
	// Place each operation
	unsigned long long startingTS = 0;
	Sort::sortNodesByStartTSThenStorageFirst(&opsToPlace);
	while (!opsToPlace.empty())
	{
		AssayNode *n = opsToPlace.front();
		opsToPlace.pop_front();

		// If changing time-step, then need to clear the board of old modules
		if (n->startTimeStep > startingTS)
		{
			startingTS = n->startTimeStep;
			reinitDataStructsToTS(startingTS);
			debugPrintLinkedLists(NULL);
			debugPrintKamerBoard(arch);
		}

		///////////////////////////////////////////////////////////////////////////////////////
		// Clear all RWs so we can start from scratch and then add initial RWs
		KamerLlNode *remove = rwll;
		while (remove != NULL)
		{
			KamerLlNode *d = remove;
			remove = remove->next;
			delete d;
		}
		rwll = NULL;

		// Now add initial RW(s) at bottom where there are no other edges/modules
		int y;
		int xBeg;
		if (createRoutingPerimeter)
		{
			xBeg = 1;
			y = arch->getNumCellsY()-2;
		}
		else
		{
			xBeg = 0;
			y = arch->getNumCellsY()-1;
		}

		//int xEnd;
		bool isOpen = false;
		for (int x = xBeg; x <= arch->getNumCellsX()-1; x++)
		{
			if (board->at(x)->at(y)->status != '-') // Blocked
			{
				if (isOpen)
					insertRwNode(y, xBeg, x-1); // Draw RW from xBeg to x-1
				isOpen = false;
			}
			else // Open
			{
				if (!isOpen)
					xBeg = x;
				isOpen = true;
			}

			if (x == arch->getNumCellsX()-1 && isOpen) // If on border and open, create RW
				insertRwNode(y, xBeg, x);
		}
		//insertRwNode(arch->getNumCellsY()-2, 1, arch->getNumCellsX()-2); // First RW, at bottom (leave 1 cell boundary for I/Os)
		//debugPrintLinkedLists(NULL);
		//debugPrintKamerBoard(arch);



		// Create bins for MFRs of various types (e.g., basic, mix, detect, etc.)
		vector<vector<MaxFreeRect *> *> mfrs;
		for (int i = 0; i <= RES_TYPE_MAX; i++)
			mfrs.push_back(new vector<MaxFreeRect *>());


		///////////////////////////////////////////////////////////////////////////////////////
		// Generate maximum empty rectangles - looking at in/out edges
		KamerLlNode *ie = iell;
		KamerLlNode *oe = oell;
		while (ie != NULL || oe != NULL)
		{
			bool doInEdgeProcessing;

			// Determine if doing in or out -edge processing
			if (ie != NULL && oe != NULL && ie->height >= oe->height)
				doInEdgeProcessing = true;
			else if (ie != NULL && oe == NULL)
				doInEdgeProcessing = true;
			else
				doInEdgeProcessing = false;

			// Do in-edge processing
			if (doInEdgeProcessing)
			{
				// Search through FWLL and create a MFR for first FW that overlaps IE in X-direction; add top line at in-edge?
				vector<KamerLlNode *> rwsBelow = getRwsBelowIE(ie);
				for (unsigned i = 0; i < rwsBelow.size(); i++)
				{
					KamerLlNode *rw = rwsBelow.at(i);
					MaxFreeRect *mfr = new MaxFreeRect();
					mfr->left = rw->left;
					mfr->right = rw->right;
					mfr->lower = rw->height;
					mfr->upper = ie->height+1;

					// Determine which type of resource this MFR is
					bool canHeat = false;
					bool canDetect = false;
					for (int xMod = mfr->left; xMod <= mfr->right; xMod++)
					{
						for (int yMod = mfr->upper; yMod <= mfr->lower; yMod++)
						{
							if (cellType->at(xMod)->at(yMod) == D_RES)
								canDetect = true;
							else if (cellType->at(xMod)->at(yMod) == H_RES)
								canHeat = true;
						}
					}

					// Add MFR to appropriate resource-type bin
					if (canDetect && canHeat)
					{
						mfr->resType = DH_RES;
						mfrs.at(DH_RES)->push_back(mfr);
					}
					else if (canDetect)
					{
						mfr->resType = D_RES;
						mfrs.at(D_RES)->push_back(mfr);
					}
					else if (canHeat)
					{
						mfr->resType = H_RES;
						mfrs.at(H_RES)->push_back(mfr);
					}
					else
					{
						mfr->resType = BASIC_RES;
						mfrs.at(BASIC_RES)->push_back(mfr);
					}

					// Now that we've created the MFR, delete the RW
					removeNode(rw);

					// Finally, make 1 or two new RWs from the one we just deleted
					// One from rw.left to ie.left, and one from ie.right to rw.right
					if (mfr->left < ie->left)
						insertRwNode(mfr->lower, mfr->left, ie->left-1);
					if (mfr->right > ie->right)
						insertRwNode(mfr->lower, ie->right+1, mfr->right);

				}

				ie = ie->next;
			}
			else // Do out-edge processing
			{
				// Create RW whose bottom has the same height as 1 above the out-edge's height (so not overlapping with module edge)
				// Now add initial RW(s) at bottom where there are no other edges/modules
				y = oe->height-1;
				//int xleft = 1;

				if (y >= 0)
				{
					// First, check if any obstacles right above edge in the x-dimension
					vector<int> obstructionsAboveOE;
					for (int x = oe->left; x <= oe->right; x++)
						if (board->at(x)->at(y)->status != '-')
							obstructionsAboveOE.push_back(x);

					// If no obstructions, create a RW and stretch as far left/right as possible
					if (obstructionsAboveOE.size() == 0)
					{
						int leftX;
						int x = oe->left-1;
						while (x >= 0 && board->at(x)->at(y)->status == '-')
							x--;
						leftX = x+1;

						int rightX;
						x = oe->right+1;
						while (x <= arch->getNumCellsX()-1 && board->at(x)->at(y)->status == '-')
							x++;
						rightX = x-1;
						insertRwNode(y, leftX, rightX);
					}
					else
					{	// There are obstructions, so add 1 or 2 RWs that overlap
						// First to left of left-most obstruction
						int rightX = obstructionsAboveOE.front() - 1;
						if (rightX >= 0 && rightX >= oe->left)
						{	// Must be on board and overlap out-edge
							int leftX;
							int x = rightX-1;
							while (x >= 0 && board->at(x)->at(y)->status == '-')
								x--;
							leftX = x+1;
							insertRwNode(y, leftX, rightX);
						}

						// Then to right of right-most obstruction
						int leftX = obstructionsAboveOE.back() + 1;
						if (leftX <= arch->getNumCellsX()-1 && leftX <= oe->right)
						{	// Must be on board and overlap out-edge
							int x = leftX+1;
							while (x <= arch->getNumCellsX()-1 && board->at(x)->at(y)->status == '-')
								x++;
							rightX = x-1;
							insertRwNode(y, leftX, rightX);
						}
					}
				}
				oe = oe->next;
			}
		}

		///////////////////////////////////////////////////////////////////////////////////////
		// Hard-code dimensions (w/o interference region) for now
		int w;
		int h;
		if (n->type == MIX || n->type == DILUTE)
		{
			w = 4;
			h = 2;
		}
		else if (n->type == SPLIT)
		{
			w = 3;
			h = 1;
		}
		else if (n->type == STORAGE || n->type == DETECT || n->type == HEAT)
		{
			w = 1;
			h = 1;
		}
		else
			claim(false, "Unsupported operation in hard-coded module library for KamerLlPlacer.");

		///////////////////////////////////////////////////////////////////////////////////////
		// Now choose a method and select from one of the valid MFRs
		int wIr;
		int hIr;
		int modTop; // Used for placing operations with external resources
		int modLeft; // Used for placeing ops. w/ external resources
		MaxFreeRect *bestMfr = NULL;

		bool rotateModule; // True if w is horizontal and h is vertical; false otherwise
		bool mfrFound = false;
		int origVcells = vCellsBetweenModIR;
		int origHcells = hCellsBetweenModIR;

		if (mfrSelectionType == SMALLEST_MFR)
		{
			///////////////////////////////////////////////////////////////////////////////////////
			// Now that we have maximum free rectangles (MFR), SELECT one of these rectangles to
			// place n. Finds the smallest MFR that will fit the module
			int area = arch->getNumCellsX() * arch->getNumCellsY() + 1; // Area bigger than array size

			// Attempt to find an MFR; if cannot find one, reduce the routing space to reduce the space a module needs
			while (!mfrFound)
			{
				wIr = w+2 + (hCellsBetweenModIR*2);
				hIr = h+2 + (vCellsBetweenModIR*2);
				// First check basic resources
				if (n->type == MIX || n->type == DILUTE || n->type == SPLIT || n->type == STORAGE)
				{
					vector<MaxFreeRect *> *mfrRt = mfrs.at(BASIC_RES);

					for (unsigned i = 0; i < mfrRt->size(); i++)
					{
						MaxFreeRect *mfr = mfrRt->at(i);
						int mfrHeight = mfr->lower-mfr->upper+1;
						int mfrWidth = mfr->right-mfr->left+1;
						int mfrArea = mfrWidth * mfrHeight;

						if (mfrArea < area)
						{
							if (wIr <= mfrWidth && hIr <= mfrHeight)
							{
								rotateModule = false;
								bestMfr = mfr;
								area = mfrArea;
							}
							else if (wIr <= mfrHeight && hIr <= mfrWidth)
							{
								rotateModule = true;
								bestMfr = mfr;
								area = mfrArea;
							}
						}
					}
				}
				if (((n->type == MIX || n->type == DILUTE || n->type == SPLIT || n->type == STORAGE) && bestMfr == NULL) ||
						n->type == HEAT)
				{	// If didn't find basic resource for basic operation OR is a heat operation
					vector<MaxFreeRect *> *mfrRt = mfrs.at(H_RES);

					for (unsigned i = 0; i < mfrRt->size(); i++)
					{
						MaxFreeRect *mfr = mfrRt->at(i);
						int mfrHeight = mfr->lower-mfr->upper+1;
						int mfrWidth = mfr->right-mfr->left+1;
						int mfrArea = mfrWidth * mfrHeight;

						if (mfrArea < area)
						{
							if (wIr <= mfrWidth && hIr <= mfrHeight && canPlaceSpecialModuleInMFR(mfr, n->type, cellType, wIr, hIr, &modTop, &modLeft))
							{
								rotateModule = false;
								bestMfr = mfr;
								area = mfrArea;
							}
							else if (wIr <= mfrHeight && hIr <= mfrWidth && canPlaceSpecialModuleInMFR(mfr, n->type, cellType, hIr, wIr, &modTop, &modLeft))
							{
								rotateModule = true;
								bestMfr = mfr;
								area = mfrArea;
							}
						}
					}
				}
				if (((n->type == MIX || n->type == DILUTE || n->type == SPLIT || n->type == STORAGE) && bestMfr == NULL) ||
						n->type == DETECT)
				{	// If didn't find basic resource for basic operation OR is a detect operation
					vector<MaxFreeRect *> *mfrRt = mfrs.at(D_RES);

					for (unsigned i = 0; i < mfrRt->size(); i++)
					{
						MaxFreeRect *mfr = mfrRt->at(i);
						int mfrHeight = mfr->lower-mfr->upper+1;
						int mfrWidth = mfr->right-mfr->left+1;
						int mfrArea = mfrWidth * mfrHeight;

						if (mfrArea < area)
						{
							if (wIr <= mfrWidth && hIr <= mfrHeight && canPlaceSpecialModuleInMFR(mfr, n->type, cellType, wIr, hIr, &modTop, &modLeft))
							{
								rotateModule = false;
								bestMfr = mfr;
								area = mfrArea;
							}
							else if (wIr <= mfrHeight && hIr <= mfrWidth && canPlaceSpecialModuleInMFR(mfr, n->type, cellType, hIr, wIr, &modTop, &modLeft))
							{
								rotateModule = true;
								bestMfr = mfr;
								area = mfrArea;
							}
						}
					}
				}
				if (bestMfr == NULL)
				{	// If didn't find basic/heat/detect resource for basic/heat/detect operation, then use DH_RES
					vector<MaxFreeRect *> *mfrRt = mfrs.at(DH_RES);

					for (unsigned i = 0; i < mfrRt->size(); i++)
					{
						MaxFreeRect *mfr = mfrRt->at(i);
						int mfrHeight = mfr->lower-mfr->upper+1;
						int mfrWidth = mfr->right-mfr->left+1;
						int mfrArea = mfrWidth * mfrHeight;

						if (mfrArea < area)
						{
							if (wIr <= mfrWidth && hIr <= mfrHeight && canPlaceSpecialModuleInMFR(mfr, n->type, cellType, wIr, hIr, &modTop, &modLeft))
							{
								rotateModule = false;
								bestMfr = mfr;
								area = mfrArea;
							}
							else if (wIr <= mfrHeight && hIr <= mfrWidth && canPlaceSpecialModuleInMFR(mfr, n->type, cellType, hIr, wIr, &modTop, &modLeft))
							{
								rotateModule = true;
								bestMfr = mfr;
								area = mfrArea;
							}
						}
					}
				}

				// Check to see if we found a solution
				if (bestMfr != NULL)
					mfrFound = true;
				else
				{	// Temporarily reduce the space a module needs to try to place module
					vCellsBetweenModIR--;
					hCellsBetweenModIR--;

					// Try all the way down to having no routing cells on any side
					if (vCellsBetweenModIR < 0 && hCellsBetweenModIR < 0)
						break;
					else if (vCellsBetweenModIR < 0)
						vCellsBetweenModIR = 0;
					else if (hCellsBetweenModIR < 0)
						hCellsBetweenModIR = 0;

					if (debugPrintEnabled())
					{
						cout << "Cannot place module " << n->GetName();
						cout << " with current spacing constraints. Temporarily reducing horizontal/vertical spacing requirements to ";
						cout << hCellsBetweenModIR << "/" << hCellsBetweenModIR << endl;
					}
				}
			}
		}
		else if (mfrSelectionType == RANDOM_MFR)
		{
			///////////////////////////////////////////////////////////////////////////////////////
			// Now that we have maximum free rectangles (MFR), SELECT one of these rectangles to
			// place n. This part selects a RANDOM valid MFR.
			vector<MaxFreeRect *> validMfrs;

			// Attempt to find an MFR; if cannot find one, reduce the routing space to reduce the space a module needs
			while (!mfrFound)
			{
				wIr = w+2 + (hCellsBetweenModIR*2);
				hIr = h+2 + (vCellsBetweenModIR*2);
				// First check basic resources
				if (n->type == MIX || n->type == DILUTE || n->type == SPLIT || n->type == STORAGE)
				{
					vector<MaxFreeRect *> *mfrRt = mfrs.at(BASIC_RES);

					for (unsigned i = 0; i < mfrRt->size(); i++)
					{
						MaxFreeRect *mfr = mfrRt->at(i);
						int mfrHeight = mfr->lower-mfr->upper+1;
						int mfrWidth = mfr->right-mfr->left+1;
						if ((wIr <= mfrWidth && hIr <= mfrHeight) || (wIr <= mfrHeight && hIr <= mfrWidth))
							validMfrs.push_back(mfr);
					}
				}
				if (((n->type == MIX || n->type == DILUTE || n->type == SPLIT || n->type == STORAGE) && bestMfr == NULL) ||
						n->type == HEAT)
				{	// If didn't find basic resource for basic operation OR is a heat operation
					vector<MaxFreeRect *> *mfrRt = mfrs.at(H_RES);

					for (unsigned i = 0; i < mfrRt->size(); i++)
					{
						MaxFreeRect *mfr = mfrRt->at(i);
						int mfrHeight = mfr->lower-mfr->upper+1;
						int mfrWidth = mfr->right-mfr->left+1;
						if ((wIr <= mfrWidth && hIr <= mfrHeight && canPlaceSpecialModuleInMFR(mfr, n->type, cellType, wIr, hIr, &modTop, &modLeft)) ||
								(wIr <= mfrHeight && hIr <= mfrWidth && canPlaceSpecialModuleInMFR(mfr, n->type, cellType, hIr, wIr, &modTop, &modLeft)))
							validMfrs.push_back(mfr);
					}
				}
				if (((n->type == MIX || n->type == DILUTE || n->type == SPLIT || n->type == STORAGE) && bestMfr == NULL) ||
						n->type == DETECT)
				{	// If didn't find basic resource for basic operation OR is a detect operation
					vector<MaxFreeRect *> *mfrRt = mfrs.at(D_RES);

					for (unsigned i = 0; i < mfrRt->size(); i++)
					{
						MaxFreeRect *mfr = mfrRt->at(i);
						int mfrHeight = mfr->lower-mfr->upper+1;
						int mfrWidth = mfr->right-mfr->left+1;
						if ((wIr <= mfrWidth && hIr <= mfrHeight && canPlaceSpecialModuleInMFR(mfr, n->type, cellType, wIr, hIr, &modTop, &modLeft)) ||
								(wIr <= mfrHeight && hIr <= mfrWidth && canPlaceSpecialModuleInMFR(mfr, n->type, cellType, hIr, wIr, &modTop, &modLeft)))
							validMfrs.push_back(mfr);
					}
				}
				if (bestMfr == NULL)
				{	// If didn't find basic/heat/detect resource for basic/heat/detect operation, then use DH_RES
					vector<MaxFreeRect *> *mfrRt = mfrs.at(DH_RES);

					for (unsigned i = 0; i < mfrRt->size(); i++)
					{
						MaxFreeRect *mfr = mfrRt->at(i);
						int mfrHeight = mfr->lower-mfr->upper+1;
						int mfrWidth = mfr->right-mfr->left+1;
						if ((wIr <= mfrWidth && hIr <= mfrHeight && canPlaceSpecialModuleInMFR(mfr, n->type, cellType, wIr, hIr, &modTop, &modLeft)) ||
								(wIr <= mfrHeight && hIr <= mfrWidth && canPlaceSpecialModuleInMFR(mfr, n->type, cellType, hIr, wIr, &modTop, &modLeft)))
							validMfrs.push_back(mfr);
					}
				}

				if (validMfrs.size() > 0)
				{
					bestMfr = validMfrs.at(rand() % validMfrs.size());

					// Now determine if the selected module needs to be rotated or not
					int mfrHeight = bestMfr->lower-bestMfr->upper+1;
					int mfrWidth = bestMfr->right-bestMfr->left+1;
					if (bestMfr->resType == BASIC_RES)
					{
						if (wIr <= mfrWidth && hIr <= mfrHeight)
							rotateModule = false;
						else if (wIr <= mfrHeight && hIr <= mfrWidth)
							rotateModule = true;
					}
					else
					{
						if (wIr <= mfrWidth && hIr <= mfrHeight && canPlaceSpecialModuleInMFR(bestMfr, n->type, cellType, wIr, hIr, &modTop, &modLeft))
							rotateModule = false;
						else if (wIr <= mfrHeight && hIr <= mfrWidth && canPlaceSpecialModuleInMFR(bestMfr, n->type, cellType, hIr, wIr, &modTop, &modLeft))
							rotateModule = true;
					}

				}

				// Check to see if we found a solution
				if (bestMfr != NULL)
					mfrFound = true;
				else
				{	// Temporarily reduce the space a module needs to try to place module
					vCellsBetweenModIR--;
					hCellsBetweenModIR--;

					// Try all the way down to having no routing cells on any side
					if (vCellsBetweenModIR < 0 && hCellsBetweenModIR < 0)
						break;
					else if (vCellsBetweenModIR < 0)
						vCellsBetweenModIR = 0;
					else if (hCellsBetweenModIR < 0)
						hCellsBetweenModIR = 0;

					if (debugPrintEnabled())
					{
						cout << "Cannot place module " << n->GetName();
						cout << " with current spacing constraints. Temporarily reducing horizontal/vertical spacing requirements to ";
						cout << hCellsBetweenModIR << "/" << hCellsBetweenModIR << endl;
					}
				}
			}
		}
		else
			claim(false, "Unknown MFR selection policy in the KAMER LL Free Placer.");


		///////////////////////////////////////////////////////////////////////////////////////
		// If no MFR, then placement fails.
		if (bestMfr == NULL)
		{
			stringstream errMsg;
			errMsg << "KAMER (linked-list) Placement Failure: Cannot place operation (" << n->GetName() << ") because there are no Maximum Free Rectangles that can fit the operation (" << wIr << " x " << hIr << " cells, including IR)." << endl;
			errMsg << "Potential problem with scheduler or resource-allocator being too aggresive.";
			debugPrint = true;
			debugPrintLinkedLists(NULL);
			debugPrintKamerBoard(arch);
			claim(bestMfr != NULL, &errMsg);
		}
		///////////////////////////////////////////////////////////////////////////////////////
		// PLACE operation into the best MFR.
		// If placing over special resource (e.g. detector, heater)
		int vrc = vCellsBetweenModIR; // vert routing cells
		int hrc = hCellsBetweenModIR; // horiz. routing cells
		if (n->type == HEAT || n->type == DETECT)
		{
			// Now do placement
			if (rotateModule)
				n->reconfigMod = new ReconfigModule(bestMfr->resType, modLeft, modTop, modLeft+h-1, modTop+w-1);
			else
				n->reconfigMod = new ReconfigModule(bestMfr->resType, modLeft, modTop, modLeft+w-1, modTop+h-1);

			n->status = BOUND;
			n->reconfigMod->startTimeStep = n->startTimeStep;
			n->reconfigMod->endTimeStep = n->endTimeStep;
			n->reconfigMod->boundNode = n;
			rModules->push_back(n->reconfigMod);
		}
		else
		{	// Else, place operation in the lower-Left corner of the bestMfr
			if (rotateModule)
				n->reconfigMod = new ReconfigModule(bestMfr->resType, bestMfr->left+1+hrc, bestMfr->lower-vrc-w, bestMfr->left+hrc+h, bestMfr->lower-1-vrc);
			else
				n->reconfigMod = new ReconfigModule(bestMfr->resType, bestMfr->left+1+hrc, bestMfr->lower-vrc-h, bestMfr->left+hrc+w, bestMfr->lower-1-vrc);

			n->status = BOUND;
			n->reconfigMod->startTimeStep = n->startTimeStep;
			n->reconfigMod->endTimeStep = n->endTimeStep;
			n->reconfigMod->boundNode = n;
			rModules->push_back(n->reconfigMod);
		}

		///////////////////////////////////////////////////////////////////////////////////////
		// Now that the operation has been placed, add in/out-edges to linked-list and KamerBoard
		KamerLlNode * modIn = insertInOutNode(IE, n->GetName(), n->reconfigMod->bottomY+1, n->reconfigMod->leftX-1, n->reconfigMod->rightX+1, n->reconfigMod->endTimeStep);
		KamerLlNode * modOut = insertInOutNode(OE, n->GetName(), n->reconfigMod->topY-1, n->reconfigMod->leftX-1, n->reconfigMod->rightX+1, n->reconfigMod->endTimeStep);
		addRemoveModuleFromKamerBoard(modIn, modOut);

		// Restore original spacing values
		vCellsBetweenModIR = origVcells;
		hCellsBetweenModIR = origHcells;

		///////////////////////////////////////////////////////////////////////////////////////
		// Debug Print
		debugPrintLinkedLists(n);
		debugPrintKamerBoard(arch);

		///////////////////////////////////////////////////////////////////////////////////////
		// Cleanup
		while (!mfrs.empty())
		{
			vector<MaxFreeRect *> *mfrsResType = mfrs.back();
			mfrs.pop_back();
			while (!mfrsResType->empty())
			{
				MaxFreeRect *mfr = mfrsResType->back();
				mfrsResType->pop_back();
				delete mfr;
			}
			delete mfrsResType;
		}
	}

	/////////////////////////////////////////////////////////////
	// Now do simple Left-Edge binding for inputs/outputs
	bindInputsLE(ioOps.at(INPUT_RES));
	bindOutputsLE(ioOps.at(OUTPUT_RES));

	{	// Sanity check: All nodes should be bound by now
		stringstream msg;
		msg << "ERROR. All nodes were not bound during Kamer-Linked-List Placement." << endl;
		msg << "There is probably a problem with the schedule." << endl;
		msg << "Try increasing the number of resources and re-scheduling." << endl;
		bool allBound = true;
		for (unsigned i = 0; i < schedDag->allNodes.size(); i++)
		{
			if (schedDag->allNodes.at(i)->GetStatus() != BOUND)
			{
				cout << "Unbound Node: ";
				schedDag->allNodes.at(i)->Print(); // Debugging
				cout << endl;
				allBound = false;
			}

			/*for (int j = 0; j < schedDag->allNodes.at(i)->children.size(); j++)
			{
				if (schedDag->allNodes.at(i)->endTimeStep != schedDag->allNodes.at(i)->children.at(j)->startTimeStep)
				{
					cout << "P->C pair does not have contiguous time-steps: ";
					schedDag->allNodes.at(i)->Print();
					schedDag->allNodes.at(i)->children.at(j)->Print();
					allBound = false;
				}
			}*/
		}
		claim(allBound, &msg);
	}

	/////////////////////////////////////////////////////////////
	// Cleanup
	/////////////////////////////////////////////////////////////
	while (!ioOps.empty())
	{
		list<AssayNode*> *l = ioOps.back();
		l->clear();
		ioOps.pop_back();
		delete l;
	}
	while (!cellType->empty())
	{
		vector<ResourceType> *v = cellType->back();
		cellType->pop_back();
		cellType->clear();
		delete v;
	}
	delete cellType;
}


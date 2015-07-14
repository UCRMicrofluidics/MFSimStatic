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
 * Source: sa_placer.cc															*
 * Original Code Author(s): Benjamin Preciado									*
 * Original Completion/Release Date: October 7, 2012							*
 *																				*
 * Details: N/A																	*
 *																				*
 * Revision History:															*
 * WHO		WHEN		WHAT													*
 * ---		----		----													*
 * FML		MM/DD/YY	One-line description									*
 *-----------------------------------------------------------------------------*/
#include "../../Headers/Placer/sa_placer.h"
#include "../../Headers/Models/reconfig_module.h"
#include "../../Headers/Models/op_module.h"
#include "../../Headers/Util/sort.h"
#include <cstdlib>
#include <sys/time.h>
#include <map>
#include <math.h>

///////////////////////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////////////////////
SAPlacer::SAPlacer()
{
}

///////////////////////////////////////////////////////////////////////////////////
// Deconstructor
///////////////////////////////////////////////////////////////////////////////////
SAPlacer::~SAPlacer()
{
}

///////////////////////////////////////////////////////////////////////////////////
// Determines the ResourceType that corresponds to an OperationType 'OT'.
// Not intended to handle arguments of OperationType DISPENSE or OUTPUT.
// No OperationType can map to ResourceType DH_RES.
///////////////////////////////////////////////////////////////////////////////////
ResourceType SAPlacer::convert(OperationType OT)
{
	stringstream msg;
	msg << "Function Placer::convert must have non i/o arguments" << ends;
	claim(((OT != DISPENSE) && (OT != OUTPUT)),  &msg);

	if (OT == HEAT) return H_RES;
	else if (OT == DETECT) return D_RES;
	else return BASIC_RES;
}

///////////////////////////////////////////////////////////////////////////////////
// Retrieves one hard-coded dimension for a module of OperationType 'OT',
// where the state of 'coord' as zero or non-zero determines which of the two dimensions are returned.
// In this placer, the dimensions are not variable as they are in some assays.
///////////////////////////////////////////////////////////////////////////////////
int SAPlacer::modCoordinate(OperationType OT, int coord)
{
	if (OT == STORAGE)
	{
		if (coord == 0) return 1; else return 1;
	}
	else if (OT == MIX || OT == DILUTE)
	{
		if (coord == 0) return 2; else return 4;
	}
	else if (OT == HEAT || OT == DETECT)
	{
		if (coord == 0) return 2; else return 2;
	}
    else if (OT == SPLIT)
    {
        if (coord == 0) return 3; else return 3;
    }
	else
	{
		if (coord == 0) return 2; else return 2;
	}
}

///////////////////////////////////////////////////////////////////////////////////
// If 'coord' in the previous function is randomly generated beforehand as a means of retrieving one dimension,
// here is a clean way to generate the needed argument for retrieving the second dimension.
///////////////////////////////////////////////////////////////////////////////////
int SAPlacer::invert(int x)
{
	if (x != 0)
		return 0;
	else return 1;
}

///////////////////////////////////////////////////////////////////////////////////
// Randomly and exhaustively generates a unique pair of numbers which are returned in the form of a vector.
// horizontal and vertical must be initialized as follows:
// horizontal contains two integer boundary points in order a, b where a < b
// vertical maps every element of [a,b] to a vector that contains two integer boundary points in order c, d where c < d
// As long as horizontal are vertical are the same arguments passed, the returned vector contains random integers in order x, y
// such that a < x < b and c < y < d and the pair (x,y) is distinct from every previous one generated.
// The returned vector will be empty if all unique combinations have been exhausted.
// Argument 'seeder' improves randomization.
///////////////////////////////////////////////////////////////////////////////////
vector<int> SAPlacer::uniquePair(vector<int> * horizontal, map<int, vector<int> > * vertical, unsigned int * seeder)
{
	srand(*seeder);
	*seeder = rand();

	vector<int> values;

	bool finished = true;
	for (unsigned i = 0; i < horizontal->size()-1; i++)
		if (horizontal->at(i) != horizontal->at(i+1)-1)
			finished = false;
	if (finished)
		return values; //values will be empty

	int low = 0;
	int high = horizontal->size()-2;
	int injunct = low + rand() % ((high-low) + 1);
	int xlow, xhigh;
	while (true)
	{
		xlow = horizontal->at(injunct);
		if (horizontal->at(injunct) != horizontal->at(injunct+1)-1)
		{
			xhigh = horizontal->at(injunct+1);
			break;
		}
		if (injunct == horizontal->size()-2)
			injunct = 0;
		else injunct++;
	}
	int xc = (xlow+1) + rand() % (((xhigh-1) - (xlow+1)) + 1);

	low = 0;
	high = vertical->at(xc).size()-2;
	int injunctor = low + rand() % ((high-low) + 1);
	int ylow, yhigh;
	while (true)
	{
		ylow = vertical->at(xc).at(injunctor);
		if (vertical->at(xc).at(injunctor) != vertical->at(xc).at(injunctor+1)-1)
		{
			yhigh = vertical->at(xc).at(injunctor+1);
			break;
		}
		if (injunctor == vertical->at(xc).size()-2)
			injunctor = 0;
		else injunctor++;
	}
	int yc = (ylow+1) + rand() % (((yhigh-1) - (ylow+1)) + 1);

	vertical->at(xc).push_back(yc);
	int ypos = vertical->at(xc).size()-1;
	while (true)
	{
		if (ypos == 0) break;
		else if (vertical->at(xc).at(ypos) < vertical->at(xc).at(ypos-1))
		{
			int temp = vertical->at(xc).at(ypos);
			int temper = vertical->at(xc).at(ypos-1);
			vertical->at(xc).at(ypos-1) = temp;
			vertical->at(xc).at(ypos) = temper;
			ypos--;
		}
		else break;
	}

	bool end = true;
	for (unsigned i = 0; i < vertical->at(xc).size()-1; i++)
		if (vertical->at(xc).at(i) != vertical->at(xc).at(i+1)-1)
			end = false;

	if (end)
	{
		horizontal->push_back(xc);
		int xpos = horizontal->size()-1;
			while (true)
			{
				if (xpos == 0) break;
				if (horizontal->at(xpos) < horizontal->at(xpos-1))
				{
					int temp = horizontal->at(xpos);
					int temper = horizontal->at(xpos-1);
					horizontal->at(xpos-1) = temp;
					horizontal->at(xpos) = temper;
					xpos--;
				}
				else break;
			}
	}

	values.push_back(xc);
	values.push_back(yc);
	return values;
}

///////////////////////////////////////////////////////////////////////////////////
// Randomly and exhaustively generates a unique number which is returned in the form of a vector.
// horizontal must be initialized as follows:
// horizontal contains two integer boundary points in order a, b where a < b
// As long as horizontal is the same argument passed, the returned vector contains random integer x
// such that a < x < b and x is distinct from every previous one generated.
// The returned vector will be empty if all integers in the interval have been exhausted.
// Argument 'seeder' improves randomization.
///////////////////////////////////////////////////////////////////////////////////
vector<int> SAPlacer::uniqueInteger(vector<int> * horizontal, unsigned int * seeder)
{
	srand(*seeder);
	*seeder = rand();

	vector<int> value;

	bool finished = true;
	for (unsigned i = 0; i < horizontal->size()-1; i++)
		if (horizontal->at(i) != horizontal->at(i+1)-1)
			finished = false;
	if (finished)
		return value; //value will be empty

	int low = 0;
	int high = horizontal->size()-2;
	int injunct = low + rand() % ((high-low) + 1);
	int xlow, xhigh;
	while (true)
	{
		xlow = horizontal->at(injunct);
		if (horizontal->at(injunct) != horizontal->at(injunct+1)-1)
		{
			xhigh = horizontal->at(injunct+1);
			break;
		}
		if (injunct == horizontal->size()-2)
			injunct = 0;
		else injunct++;
	}
	int xvalue = (xlow+1) + rand() % (((xhigh-1) - (xlow+1)) + 1);

	horizontal->push_back(xvalue);
	int xpos = horizontal->size()-1;
	while (true)
	{
		if (xpos == 0) break;
		if (horizontal->at(xpos) < horizontal->at(xpos-1))
		{
			int temp = horizontal->at(xpos);
			int temper = horizontal->at(xpos-1);
			horizontal->at(xpos-1) = temp;
			horizontal->at(xpos) = temper;
			xpos--;
		}
		else break;
	}

	value.push_back(xvalue);
	return value;
}

///////////////////////////////////////////////////////////////////////////////////
//Copies the contents of one vector of OpModules to another.
///////////////////////////////////////////////////////////////////////////////////
void SAPlacer::copyModules(const vector<OpModule> * ZModules, vector<OpModule> * XModules)
{
	while (!XModules->empty())
		XModules->pop_back();

	for (unsigned i = 0; i < ZModules->size(); i++)
		XModules->push_back(ZModules->at(i));
}

///////////////////////////////////////////////////////////////////////////////////
// Determines the time step of the next set of nodes that require annealing provided opsByResType in the 'placer' function
// (displayed here as listOps), and the time step of the last set of nodes that underwent annealing.  Returns -1 (an invalid time step)
// if no further nodes require annealing.
///////////////////////////////////////////////////////////////////////////////////
int SAPlacer::calcNextTS(vector< list<AssayNode*> * > listOps, int annealTS)
{
	int nextTS = -1;
	for (int i = 0; i <= RES_TYPE_MAX; i++) //inputs and outputs are not of interest
	{
		ResourceType rt = (ResourceType)i;
		list<AssayNode*>::iterator it = listOps.at(rt)->begin();
		for (; it != listOps.at(rt)->end(); it++)
		{
			AssayNode *n = *it;
			int TS = n->GetStartTS();
			if (TS > annealTS && (TS < nextTS || nextTS == -1))
			{
				nextTS = TS;
				break;
			}
		}
	}
	return nextTS;
}

///////////////////////////////////////////////////////////////////////////////////
// Updates activeModules provided the annealingModules and the nextTS.
// Previous activeModules are removed while new ones are added from annealingModules.
///////////////////////////////////////////////////////////////////////////////////
void SAPlacer::updateActiveModules(vector<OpModule> * activeModules, vector<OpModule> annealingModules, int nextTS)
{
	vector<OpModule> active;
	for (unsigned i = 0; i < activeModules->size(); i++)
	{
		unsigned long long sTS = activeModules->at(i).getStartTS();
		unsigned long long eTS = activeModules->at(i).getEndTS();

		if (sTS < nextTS && nextTS < eTS)
			active.push_back(activeModules->at(i));
	}

	for (unsigned i = 0; i < annealingModules.size(); i++)
	{
		unsigned long long sTS = annealingModules.at(i).getStartTS();
		unsigned long long eTS = annealingModules.at(i).getEndTS();

		if (sTS < nextTS && nextTS < eTS)
			active.push_back(annealingModules.at(i));
	}
	activeModules->swap(active);
}

///////////////////////////////////////////////////////////////////////////////////
// Performs the actions that establish placement.
///////////////////////////////////////////////////////////////////////////////////
void SAPlacer::placement(vector<AssayNode*> annealingNodes, vector<OpModule> annealingModules, vector<ReconfigModule *> * rModules)
{
	for (unsigned i = 0; i < annealingNodes.size(); i++)
	{
		OpModule om = annealingModules.at(i);
		AssayNode * n = annealingNodes.at(i);

		n->status = BOUND;
		//n->reconfigMod = new ReconfigModule(om.getResourceType(), om.getLX(), om.getTY(), om.getRX(), om.getBY()); //DTG
		n->reconfigMod = new ReconfigModule(om.getBoundedResType(), om.getLX(), om.getTY(), om.getRX(), om.getBY());
		n->reconfigMod->startTimeStep = n->startTimeStep;
		n->reconfigMod->endTimeStep = n->endTimeStep;
		n->reconfigMod->boundNode = n;
		rModules->push_back(n->reconfigMod);
	}
}

///////////////////////////////////////////////////////////////////////////////////
// Determines whether or not an OpModule lies outside of the dimensions of the array.  Assumes that the dimensions
// of the object are sensible (i.e. member leftX is always less than or equal to member rightX).  Assumes
// an interference region (IR) and droplet routing path (DR) surrounding the object, which must also be contained
// by the array.
///////////////////////////////////////////////////////////////////////////////////
bool SAPlacer::outsideArray(DmfbArch *arch, OpModule om)
{
	int maxX = arch->getNumCellsX();
	int maxY = arch->getNumCellsY();
	int LX = om.getLX();
	int TY = om.getTY();
	int RX = om.getRX();
	int BY = om.getBY();

	bool violation = false;
	if (LX < 2 || BY >= maxY - 2 || RX >= maxX - 2 || TY < 2) violation = true; //2 for IR and DR
	return violation;
}

///////////////////////////////////////////////////////////////////////////////////
// Determines whether two OpModules intersect each other.  Assumes that the dimensions of the object are sensible.
// Assumes an IR and DR path surrounding each object, but allows routing paths to intersect.
///////////////////////////////////////////////////////////////////////////////////
bool SAPlacer::modIntersect(OpModule om1, OpModule om2)
{
	bool potential_violation = false;
	bool violation = false;

	//compare the horizontal intervals - if these overlap then the modules may intersect if they aren't displaced enough vertically
	//if the horizontal intervals don't overlap we can be guaranteed the modules don't intersect
	int hCells = getHCellsBetweenModIR() + 1;
	int vCells = getVCellsBetweenModIR() + 1;
	for (int i = om1.getLX() - hCells; i <= om1.getRX() + hCells; i++) // 2 for IR and DR
	{
		if ((i >= om2.getLX() - 1) && (i <= om2.getRX() + 1)) // 1 to allow routing paths to intersect
		{
			potential_violation = true;
			break;
		}
	}
	//compare the vertical intervals if the horizontal ones overlapped
	if (potential_violation)
	{
		for (int i = om1.getTY() - vCells; i <= om1.getBY() + vCells; i++) // 2 for IR and DR
		{
			if ((i >= om2.getTY() - 1) && (i <= om2.getBY() + 1)) // 1 to allow routing paths to intersect
			{
				violation = true;
				break;
			}
		}
	}
	return violation;
}

///////////////////////////////////////////////////////////////////////////////////
// Determines whether or not two OpModules occupy the exact same dimensions on the array.
///////////////////////////////////////////////////////////////////////////////////
bool SAPlacer::identicalPosition(OpModule om1, OpModule om2)
{
	if ((om1.getLX() == om2.getLX()) && (om1.getRX() == om2.getRX()) && (om1.getTY() == om2.getTY()) && (om1.getBY() == om2.getBY()))
		return true;
	else return false;
}

///////////////////////////////////////////////////////////////////////////////////
// Acquires the next set of nodes that require annealing by copying them from opsByResType in 'placer' (displayed here as listOps),
// to annealingNodes (displayed here as annealing) based on nextTS.  The nodes that require fixed resources on the array will appear
// earlier in the vector.  Other functions rely on this order.
///////////////////////////////////////////////////////////////////////////////////
void SAPlacer::calcAnnealingNodes(vector<AssayNode*> * annealing, vector< list<AssayNode*> * > listOps, int nextTS)
{
	//discard nodes corresponding to a previous time step
	while (!annealing->empty())
		annealing->pop_back();

	//update the vector
	for (int i = RES_TYPE_MAX; i >= 0; i--) //order established
	{
		ResourceType rt = (ResourceType)i;
		list<AssayNode*>::iterator it = listOps.at(rt)->begin();
		for (; it != listOps.at(rt)->end(); it++)
		{
			AssayNode *n = *it;
			int TS = n->GetStartTS();
			if (TS == nextTS)
				annealing->push_back(n);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////
// Create modules for each of the annealingNodes.
///////////////////////////////////////////////////////////////////////////////////
void SAPlacer::calcAnnealingModules(vector<OpModule> * annealingModules, vector<AssayNode*> annealingNodes)
{
	//discard modules belonging to a previous time step
	while(!annealingModules->empty())
		annealingModules->pop_back();

	//update the vector
	for (unsigned i = 0; i < annealingNodes.size(); i++)
	{
		OpModule om = OpModule(annealingNodes.at(i)->GetType(), -3, -3, -3, -3);
			//-3 is *somewhat* arbitrary - the setAnnealingModules function relies on the default position being outside the array
		om.setStartTS(annealingNodes.at(i)->GetStartTS());
		om.setEndTS(annealingNodes.at(i)->GetEndTS());
		om.setBoundedResType(annealingNodes.at(i)->boundedResType);
		annealingModules->push_back(om);
	}
}

///////////////////////////////////////////////////////////////////////////////////
// Populates fixedResources with OpModules that describe fixed resource positions on the array.
///////////////////////////////////////////////////////////////////////////////////
void SAPlacer::calcFixedResources(vector<OpModule> * fixedResources, DmfbArch * arch)
{
	map<int, bool> skipper;
	for (unsigned i = 0; i < arch->getExternalResources()->size(); i++)
	{
		if (skipper[i] == true) continue;
		bool special = false;

		FixedModule *fm = arch->getExternalResources()->at(i);
		ResourceType RT = fm->getResourceType();
		int LX = fm->getLX(); int RX = fm->getRX(); int TY = fm->getTY(); int BY = fm->getBY();

		OpModule om1 = OpModule(RT, LX, TY, RX, BY);
		for (unsigned j = i+1; j < arch->getExternalResources()->size(); j++)
		{
			fm = arch->getExternalResources()->at(j);
			RT = fm->getResourceType();
			int LX = fm->getLX(); int RX = fm->getRX(); int TY = fm->getTY(); int BY = fm->getBY();
			OpModule om2 = OpModule(RT, LX, TY, RX, BY);

			if (identicalPosition(om1, om2) && (om1.getBoundedResType() != om2.getBoundedResType()))
			{
				OpModule om3 = OpModule(GENERAL, DH_RES, LX, TY, RX, BY, 0, 0);
				fixedResources->push_back(om3);
				skipper[j] = true;
				special = true;
			}
		}
		if (!special)
		{
			fm = arch->getExternalResources()->at(i);
			RT = fm->getResourceType();
			int LX = fm->getLX(); int RX = fm->getRX(); int TY = fm->getTY(); int BY = fm->getBY();
			OpModule om3 = OpModule(GENERAL, RT, LX, TY, RX, BY, 0, 0);
			fixedResources->push_back(om3);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////
// Examines opsByResType and populates futureNodes with the set of nodes that are scheduled to begin along the time stretched by activeNodes
// and annealingNodes combined.  Nodes that require fixed resources appear earlier in the vector.
///////////////////////////////////////////////////////////////////////////////////
void SAPlacer::calcFutureNodes(vector<AssayNode*> * futureNodes, vector<OpModule> activeModules, vector<OpModule> annealingModules, vector<list<AssayNode*> *> opsByResType)
{
	unsigned long long earlyTS = annealingModules.at(0).getStartTS(), lateTS = 0, temp = 0;

	for (unsigned i = 0; i < activeModules.size(); i++)
	{
		temp = activeModules.at(i).getEndTS();
		if (temp > lateTS) lateTS = temp;
	}
	for (unsigned i = 0; i < annealingModules.size(); i++)
	{
		temp = annealingModules.at(i).getEndTS();
		if (temp > lateTS) lateTS = temp;
	}

	while (!futureNodes->empty())
	{
		futureNodes->pop_back();
	}
	for (int i = RES_TYPE_MAX; i >= 1; i--) //only D_RES, H_RES and DH_RES
	{
		ResourceType rt = (ResourceType)i;
		list<AssayNode*>::iterator it = opsByResType.at(rt)->begin();
		for (; it != opsByResType.at(rt)->end(); it++)
		{
			AssayNode *n = *it;
			int TS = n->GetStartTS();
			if (TS < lateTS && TS > earlyTS)
				futureNodes->push_back(n);
			else if (TS >= lateTS) break;
		}
	}

	//push BASIC_RES last
	ResourceType rt = BASIC_RES;
	list <AssayNode*>::iterator it = opsByResType.at(rt)->begin();
	for (; it != opsByResType.at(rt)->end(); it++)
	{
		AssayNode *n = *it;
		int TS = n->GetStartTS();
		if (TS < lateTS && TS > earlyTS)
			futureNodes->push_back(n);
		else if (TS >= lateTS) break;
	}
}

///////////////////////////////////////////////////////////////////////////////////
// Establishes the initial placement for simulated annealing.
///////////////////////////////////////////////////////////////////////////////////
bool SAPlacer::setAnnealingModules(vector<OpModule> * annealingModules, int select, vector<OpModule> * activeModules, vector<AssayNode*> * futureNodes, vector<OpModule> * fixedResources, DmfbArch *arch)
{
	if (annealingModules->size() == 0 || select >= annealingModules->size())
		return true;

	OpModule aM = annealingModules->at(select);
	ResourceType aM_rt = aM.getBoundedResType();
	OperationType aM_ot = aM.getOperationType();
	vector<OpModule> PosArea;

	if (aM_rt != BASIC_RES)
	{
		for (unsigned i = 0; i < fixedResources->size(); i++)
		{
			OpModule F = fixedResources->at(i);
			ResourceType F_rt = F.getBoundedResType();
			if ((F_rt == aM_rt) || (F_rt == DH_RES))
				PosArea.push_back(F);
		}
	}
	else
	{
		int maxX = arch->getNumCellsX();
		int maxY = arch->getNumCellsY();
		OpModule F = OpModule(GENERAL, 2, 2, maxX-3, maxY-3);
		PosArea.push_back(F);
	}

	vector< vector<int> > horizontals;
	vector< map<int, vector<int> > > verticals;
	vector< vector<int> > differentials;
	for (unsigned pos = 0; pos < PosArea.size(); pos++)
	{
		OpModule PBox = PosArea.at(pos);
		int LX = PBox.getLX(); int RX = PBox.getRX(); int TY = PBox.getTY(); int BY = PBox.getBY();
		int A = modCoordinate(aM_ot, 0);
		int B = modCoordinate(aM_ot, 1);
		vector<int> horizontal;
		horizontal.push_back(TY-1);
		horizontal.push_back(BY+A);
		horizontals.push_back(horizontal);
		map<int, vector<int> > vertical;
		for (int i = TY; i <= (BY+A-1); i++)
		{
			vertical[i].push_back(LX-1);
			vertical[i].push_back(RX+B);
		}
		verticals.push_back(vertical);
		vector<int> differential;
		differential.push_back(A);
		differential.push_back(B);
		differentials.push_back(differential);
		if (A != B)
		{
			vector<int> horizontal;
			horizontal.push_back(TY-1);
			horizontal.push_back(BY+B);
			horizontals.push_back(horizontal);
			map<int, vector<int> > vertical;
			for (int i = TY; i <= (BY+B-1); i++)
			{
				vertical[i].push_back(LX-1);
				vertical[i].push_back(RX+A);
			}
			verticals.push_back(vertical);
			vector<int> differential;
			differential.push_back(B);
			differential.push_back(A);
			differentials.push_back(differential);
		}
	}

	unsigned int seeder = rand();
	vector<int> exclude;
	int frontier = horizontals.size();
	while (exclude.size() != frontier)
	{
		int pos = rand() % frontier;
		bool resume = false;
		for (unsigned i = 0; i < exclude.size(); i++)
		{
			if (exclude.at(i) == pos)
				resume = true;
		}
		if (resume) continue;

		vector<int> pair = uniquePair(& horizontals.at(pos), & verticals.at(pos), & seeder);
		if (pair.size() == 0)
		{
			exclude.push_back(pos);
			continue;
		}

		//generate the span now

		int dy = differentials.at(pos).at(0);
		int dx = differentials.at(pos).at(1);
		int BY = pair.at(0);
		int RX = pair.at(1);
		int LX = RX - dx + 1;
		int TY = BY - dy + 1;

		OpModule span = OpModule(GENERAL, LX, TY, RX, BY);

		//perform validity testing

		bool valid = true;
		if (outsideArray(arch, span))
			valid = false;
		if (!valid) continue;
		for (unsigned m = 0; m < activeModules->size(); m++)
			if (modIntersect(activeModules->at(m), span))
			{
				valid = false;
				break;
			}
		if (!valid) continue;
		for (int m = 0; m < select; m++)
			if (modIntersect(annealingModules->at(m), span))
			{
				valid = false;
				break;
			}
		if (!valid) continue;

		annealingModules->at(select).setLX(span.getLX());
		annealingModules->at(select).setRX(span.getRX());
		annealingModules->at(select).setBY(span.getBY());
		annealingModules->at(select).setTY(span.getTY());

		bool compatible = fixedResourceCompatible(arch, fixedResources, activeModules, annealingModules, futureNodes);
		if (!compatible)
		{
			annealingModules->at(select).setLX(-3);
			annealingModules->at(select).setRX(-3);
			annealingModules->at(select).setBY(-3);
			annealingModules->at(select).setTY(-3);
			valid = false;
		}
		if (!valid) continue;

		int advanced = select + 1;
		bool future = setAnnealingModules(annealingModules, advanced, activeModules, futureNodes, fixedResources, arch);

		if (future)
			return true;
		else
		{
			annealingModules->at(select).setLX(-3);
			annealingModules->at(select).setRX(-3);
			annealingModules->at(select).setBY(-3);
			annealingModules->at(select).setTY(-3);
			continue;
		}
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////
// Determines whether or not a proposed placement meets the FRC requirement discussed in the program design.
///////////////////////////////////////////////////////////////////////////////////
bool SAPlacer::fixedResourceCompatible(DmfbArch *arch, vector<OpModule> * fixedResources, vector<OpModule> * activeModules, vector<OpModule> * annealingModules, vector<AssayNode*> * futureNodes)
{
	//Determine the earliest and latest time steps for the proposed placement
	unsigned long long earlyTS = annealingModules->at(0).getStartTS(), lateTS = 0, temp = 0;
	for (unsigned i = 0; i < activeModules->size(); i++)
	{
		temp = activeModules->at(i).getEndTS();
		if (temp > lateTS) lateTS = temp;
	}
	for (unsigned i = 0; i < annealingModules->size(); i++)
	{
		temp = annealingModules->at(i).getEndTS();
		if (temp > lateTS) lateTS = temp;
	}

	//gather the time steps when new modules are introduced
	vector<int> times;
	for (unsigned TS = earlyTS; TS < lateTS; TS++)
	{
		for (unsigned k = 0; k < futureNodes->size(); k++)
		{
			AssayNode * fn = futureNodes->at(k);
			unsigned long long sTS = fn->startTimeStep;
			if (sTS == TS)
			{
				times.push_back(TS);
				break;
			}
		}
	}
	//for every such time step
	for (unsigned t = 0; t < times.size(); t++)
	{
		int TS = times.at(t);
		unsigned long long sTS, eTS;
		vector<AssayNode*> FNodes;
		vector< vector<OpModule> > PMods;
		for (unsigned k = 0; k < futureNodes->size(); k++)
		{
			AssayNode * fn = futureNodes->at(k);
			sTS = fn->startTimeStep;
			eTS = fn->endTimeStep;
			if (sTS <= TS && TS < eTS)
			{
				FNodes.push_back(fn);
				//added
				vector<OpModule> boundary;
				PMods.push_back(boundary);
				int FSize = FNodes.size()-1;
				unsigned long long sTS0, eTS0;
				for (unsigned i = 0; i < activeModules->size(); i++)
				{
					sTS0 = activeModules->at(i).getStartTS();
					eTS0 = activeModules->at(i).getEndTS();
					if (sTS0 <= sTS && sTS < eTS0)
						PMods.at(FSize).push_back(activeModules->at(i));
				}
				for (unsigned i = 0; i < annealingModules->size(); i++)
				{
					sTS0 = annealingModules->at(i).getStartTS();
					eTS0 = annealingModules->at(i).getEndTS();
					if (sTS0 <= sTS && sTS < eTS0)
						PMods.at(FSize).push_back(annealingModules->at(i));
				}
			}
		}
		int select = 0;
		bool compatible = compatibilityDive(arch, fixedResources, & PMods, & FNodes, select);
		if (!compatible) return false;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////
// The recursive component of fixedResourceCompatible.
///////////////////////////////////////////////////////////////////////////////////
bool SAPlacer::compatibilityDive(DmfbArch *arch, vector<OpModule> * fixedResources, vector< vector<OpModule> > * PMods, vector<AssayNode*> * FNodes, int select)
{
	if (FNodes->size() == 0 || select == FNodes->size())
		return true;

	vector<OpModule> PModules = PMods->at(select);
	vector<OpModule> PosArea;
	AssayNode * fnode = FNodes->at(select);
	ResourceType fnode_rt = fnode->boundedResType;
	OperationType fnode_ot = fnode->GetType();

	if (fnode_rt != BASIC_RES)
	{
		for (unsigned i = 0; i < fixedResources->size(); i++)
		{
			OpModule F = fixedResources->at(i);
			ResourceType F_rt = F.getBoundedResType();
			if ((F_rt == fnode_rt) || (F_rt == DH_RES))
				PosArea.push_back(F);
		}
	}
	else
	{
		int maxX = arch->getNumCellsX();
		int maxY = arch->getNumCellsY();
		OpModule F = OpModule(GENERAL, 2, 2, maxX-3, maxY-3);
		PosArea.push_back(F);
	}

	vector< vector<int> > horizontals;
	vector< map<int, vector<int> > > verticals;
	vector< vector<int> > differentials;
	for (unsigned pos = 0; pos < PosArea.size(); pos++)
	{
		OpModule PBox = PosArea.at(pos);
		int LX = PBox.getLX(); int RX = PBox.getRX(); int TY = PBox.getTY(); int BY = PBox.getBY();
		int A = modCoordinate(fnode_ot, 0);
		int B = modCoordinate(fnode_ot, 1);
		vector<int> horizontal;
		horizontal.push_back(TY-1);
		horizontal.push_back(BY+A);
		horizontals.push_back(horizontal);
		map<int, vector<int> > vertical;
		for (int i = TY; i <= (BY+A-1); i++)
		{
			vertical[i].push_back(LX-1);
			vertical[i].push_back(RX+B);
		}
		verticals.push_back(vertical);
		vector<int> differential;
		differential.push_back(A);
		differential.push_back(B);
		differentials.push_back(differential);
		if (A != B)
		{
			vector<int> horizontal;
			horizontal.push_back(TY-1);
			horizontal.push_back(BY+B);
			horizontals.push_back(horizontal);
			map<int, vector<int> > vertical;
			for (int i = TY; i <= (BY+B-1); i++)
			{
				vertical[i].push_back(LX-1);
				vertical[i].push_back(RX+A);
			}
			verticals.push_back(vertical);
			vector<int> differential;
			differential.push_back(B);
			differential.push_back(A);
			differentials.push_back(differential);
		}
	}

	unsigned int seeder = rand();
	vector<int> exclude;
	int frontier = horizontals.size();
	while (exclude.size() != frontier)
	{
		int pos = rand() % frontier;
		bool resume = false;
		for (unsigned i = 0; i < exclude.size(); i++)
		{
			if (exclude.at(i) == pos)
				resume = true;
		}
		if (resume) continue;

		vector<int> pair = uniquePair(& horizontals.at(pos), & verticals.at(pos), & seeder);
		if (pair.size() == 0)
		{
			exclude.push_back(pos);
			continue;
		}

		//generate the span now

		int dy = differentials.at(pos).at(0);
		int dx = differentials.at(pos).at(1);
		int BY = pair.at(0);
		int RX = pair.at(1);
		int LX = RX - dx + 1;
		int TY = BY - dy + 1;

		OpModule span = OpModule(GENERAL, LX, TY, RX, BY);

		//perform validity testing

		bool valid = true;
		if (outsideArray(arch, span))
			valid = false;
		if (!valid) continue;
		for (unsigned m = 0; m < PModules.size(); m++)
			if (modIntersect(PModules.at(m), span))
				valid = false;
		if (!valid) continue;

		int advanced = select + 1;
		if (advanced < FNodes->size())
			for (unsigned m = advanced; m < PMods->size(); m++)
				PMods->at(m).push_back(span);
		bool deep = compatibilityDive(arch, fixedResources, PMods, FNodes, advanced);
		if (deep)
			return true;
		else
		{
			if (advanced < FNodes->size())
				for (unsigned m = advanced; m < PMods->size(); m++)
					PMods->at(m).pop_back();
		}
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////
// Generates a neighbor.
///////////////////////////////////////////////////////////////////////////////////
void SAPlacer::generateNeighbor(DmfbArch *arch, vector<OpModule> * fixedResources, vector<OpModule> * activeModules, vector<OpModule> * annealingModules, vector<AssayNode*> * futureNodes)
{
	int select = rand() % annealingModules->size();
	OpModule annealingModule = annealingModules->at(select);

	vector<OpModule> PModules;
	vector<OpModule> AModules;
	for (unsigned i = 0; i < activeModules->size(); i++)
		PModules.push_back(activeModules->at(i));
	for (unsigned i = 0; i < annealingModules->size(); i++)
	{
		if (i == select) continue;
		PModules.push_back(annealingModules->at(i));
	}
	for (unsigned i = 0; i < annealingModules->size(); i++)
	{
		if (i == select) continue;
		AModules.push_back(annealingModules->at(i));
	}

	vector<OpModule> PosArea;
	ResourceType AM_rt = annealingModule.getBoundedResType();
	OperationType AM_ot = annealingModule.getOperationType();

	if (AM_rt != BASIC_RES)
	{
		for (unsigned i = 0; i < fixedResources->size(); i++)
		{
			OpModule F = fixedResources->at(i);
			ResourceType F_rt = F.getBoundedResType();
			if ((F_rt == AM_rt) || (F_rt == DH_RES))
				PosArea.push_back(F);
		}
	}
	else
	{
		int maxX = arch->getNumCellsX();
		int maxY = arch->getNumCellsY();
		OpModule F = OpModule(GENERAL, 2, 2, maxX-3, maxY-3);
		PosArea.push_back(F);
	}

	vector< vector<int> > horizontals;
	vector< map<int, vector<int> > > verticals;
	vector< vector<int> > differentials;
	for (unsigned pos = 0; pos < PosArea.size(); pos++)
	{
		OpModule PBox = PosArea.at(pos);
		int LX = PBox.getLX(); int RX = PBox.getRX(); int TY = PBox.getTY(); int BY = PBox.getBY();
		int A = modCoordinate(AM_ot, 0);
		int B = modCoordinate(AM_ot, 1);
		vector<int> horizontal;
		horizontal.push_back(TY-1);
		horizontal.push_back(BY+A);
		horizontals.push_back(horizontal);
		map<int, vector<int> > vertical;
		for (int i = TY; i <= (BY+A-1); i++)
		{
			vertical[i].push_back(LX-1);
			vertical[i].push_back(RX+B);
		}
		verticals.push_back(vertical);
		vector<int> differential;
		differential.push_back(A);
		differential.push_back(B);
		differentials.push_back(differential);
		if (A != B)
		{
			vector<int> horizontal;
			horizontal.push_back(TY-1);
			horizontal.push_back(BY+B);
			horizontals.push_back(horizontal);
			map<int, vector<int> > vertical;
			for (int i = TY; i <= (BY+B-1); i++)
			{
				vertical[i].push_back(LX-1);
				vertical[i].push_back(RX+A);
			}
			verticals.push_back(vertical);
			vector<int> differential;
			differential.push_back(B);
			differential.push_back(A);
			differentials.push_back(differential);
		}
	}

	unsigned int seeder = rand();
	vector<int> exclude;
	int frontier = horizontals.size();
	while (exclude.size() != frontier)
	{
		int pos = rand() % frontier;
		bool resume = false;
		for (unsigned i = 0; i < exclude.size(); i++)
		{
			if (exclude.at(i) == pos)
				resume = true;
		}
		if (resume) continue;
		vector<int> pair = uniquePair(& horizontals.at(pos), & verticals.at(pos), & seeder);
		if (pair.size() == 0)
		{
			exclude.push_back(pos);
			continue;
		}

		//generate the span now

		int dy = differentials.at(pos).at(0);
		int dx = differentials.at(pos).at(1);
		int BY = pair.at(0);
		int RX = pair.at(1);
		int LX = RX - dx + 1;
		int TY = BY - dy + 1;

		OpModule span = OpModule(GENERAL, LX, TY, RX, BY);

		//perform validity testing

		bool valid = true;
		if (outsideArray(arch, span))
			valid = false;
		if (!valid) continue;
		for (unsigned m = 0; m < PModules.size(); m++)
			if (modIntersect(PModules.at(m), span))
				valid = false;
		if (!valid) continue;

		OpModule testMod = annealingModule;
		testMod.setLX(LX); testMod.setRX(RX); testMod.setTY(TY); testMod.setBY(BY);
		AModules.push_back(testMod);
		if (!fixedResourceCompatible(arch, fixedResources, activeModules, & AModules, futureNodes))
		{
			valid = false;
			AModules.pop_back();
		}
		if (!valid) continue;

		annealingModules->at(select).setLX(LX);
		annealingModules->at(select).setRX(RX);
		annealingModules->at(select).setTY(TY);
		annealingModules->at(select).setBY(BY);

		break;
	}
}

///////////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////////
int SAPlacer::areaPair(OpModule A, OpModule B)
{
	int ATY, ABY, ALX, ARX;
	int BTY, BBY, BLX, BRX;
	int mTY, mBY, mLX, mRX;
	ATY = A.getTY(); ABY = A.getBY(); ALX = A.getLX(); ARX = A.getRX();
	BTY = B.getTY(); BBY = B.getBY(); BLX = B.getLX(); BRX = B.getRX();

	if (ATY < BTY) mTY = ATY; else mTY = BTY;
	if (ABY > BBY) mBY = ABY; else mBY = BBY;
	if (ALX < BLX) mLX = ALX; else mLX = BLX;
	if (ARX > BRX) mRX = ARX; else mRX = BRX;

	int dy = mBY - mTY; int dx = mRX - mLX;
	int product = dx * dy;
	return product;
}

///////////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////////
int SAPlacer::Cost(const vector<OpModule> * activeModules, const vector<OpModule> * annealingModules)
{
	vector<OpModule> PModules;

	int TY, BY, LX, RX;
	int mTY = -1, mBY = -1, mLX = -1, mRX = -1;
	for (unsigned i = 0; i < annealingModules->size(); i++)
	{
		OpModule OM = annealingModules->at(i);
		TY = OM.getTY(); BY = OM.getBY(); LX = OM.getLX(); RX = OM.getRX();
		if (TY < mTY || mTY == -1) mTY = TY;
		if (BY > mBY || mBY == -1) mBY = BY;
		if (LX < mLX || mLX == -1) mLX = LX;
		if (RX > mRX || mRX == -1) mRX = RX;

		PModules.push_back(OM);
	}
	for (unsigned i = 0; i < activeModules->size(); i++)
	{
		OpModule OM = activeModules->at(i);
		TY = OM.getTY(); BY = OM.getBY(); LX = OM.getLX(); RX = OM.getRX();
		if (TY < mTY || mTY == -1) mTY = TY;
		if (BY > mBY || mBY == -1) mBY = BY;
		if (LX < mLX || mLX == -1) mLX = LX;
		if (RX > mRX || mRX == -1) mRX = RX;

		PModules.push_back(OM);
	}
	int dy = mBY - mTY; int dx = mRX - mLX;
	int product = dx * dy;
	int area = product;
	for (unsigned i = 0; i < PModules.size(); i++)
	{
		for (unsigned j = i; j < PModules.size(); j++)
		{
			if (j == PModules.size()-1)
				break;
			OpModule OM1 = PModules.at(i);
			OpModule OM2 = PModules.at(j+1);
			int pArea = areaPair(OM1, OM2);
			area += pArea;
		}
	}

	return area;
}

///////////////////////////////////////////////////////////////////////////////////
// Main place funciton that is called to start the simulated annealing placement
///////////////////////////////////////////////////////////////////////////////////
void SAPlacer::place(DmfbArch *arch, DAG *schedDag, vector<ReconfigModule *> *rModules)
{
	unsigned int seed = time(0);
	cout << "Random seed for simulated annealing: " << seed;
	//unsigned int seed = 1351274550;
	//cout << "Hard-coded seed for simulated annealing: " << seed;
	srand(seed);


	int INPUT_RES = RES_TYPE_MAX + 1;
	int OUTPUT_RES = RES_TYPE_MAX + 2;

	vector< list<AssayNode*> * > opsByResType; // operations distinguished by required resource type
	list<AssayNode*> storeList;
	for (int i = 0; i <= RES_TYPE_MAX + 2; i++) // +2 for inputs and outputs
		opsByResType.push_back(new list<AssayNode*>());
	for (unsigned i = 0; i < schedDag->getAllNodes().size(); i++)
	{
		AssayNode *n = schedDag->getAllNodes().at(i);
		OperationType ot = n->GetType();
		if (ot == STORAGE_HOLDER) cout << "HIT" << endl;
		if (n->GetType() == DISPENSE)
			opsByResType.at(INPUT_RES)->push_back(n);
		else if (n->GetType() == OUTPUT)
			opsByResType.at(OUTPUT_RES)->push_back(n);
		else if (n->GetType() != STORAGE)
			opsByResType.at(n->boundedResType)->push_back(n);
		else if (n->GetType() == STORAGE)
			storeList.push_back(n);
	}
	for (unsigned i = 0; i < schedDag->getAllStorageHolders().size(); i++)
	{
		AssayNode *n = schedDag->getAllStorageHolders().at(i);
		opsByResType.at(n->boundedResType)->push_back(n);
	}
	for (int i = 0; i <= RES_TYPE_MAX; i++)
		Sort::sortNodesByStartTS(opsByResType.at(i));

	vector<OpModule> fixedResources;
	vector<OpModule> activeModules;
	vector<OpModule> annealingModules;
	int previousTS = 0, nextTS = 0;
	//vector<AssayNode*> activeNodes;
	vector<AssayNode*> annealingNodes;
	vector<AssayNode*> futureNodes;
	calcFixedResources(& fixedResources, arch);

	while (true)
	{
		previousTS = nextTS;
		nextTS = calcNextTS(opsByResType, previousTS);

		//cout << "_______________________________" << endl;
		//cout << endl << "the time step is now " << nextTS << endl << "_______________________________" << endl;
		cout << endl << "TS " << nextTS << ": ";

		if (nextTS == -1)
			break;
		updateActiveModules(& activeModules, annealingModules, nextTS);
		for (unsigned i = 0; i < activeModules.size(); i++)
			OpModule OM = activeModules.at(i);
		calcAnnealingNodes(& annealingNodes, opsByResType, nextTS);
		calcAnnealingModules(& annealingModules, annealingNodes);
		calcFutureNodes(& futureNodes, activeModules, annealingModules, opsByResType);

		//THE SIMULATED ANNEALING PROCEDURE
		vector<OpModule> AModules;
		copyModules( & annealingModules, & AModules );

		//set up controlling parameters
		double T = 500;
		int Na = 5;
		int Nb = annealingModules.size();
		int L = Na * Nb;
		double cool = 0.7;
		int seeder = rand();

		//execute

		bool future = setAnnealingModules(& annealingModules, 0, & activeModules, & futureNodes, & fixedResources, arch);
		while (T > 10)
		{
			//cout << "the temperature is now " << T << endl;
			cout << T << "--" << endl;
			for (int i = 0; i < L; i++)
			{
				generateNeighbor(arch, & fixedResources, & activeModules, & AModules, & futureNodes);
				int cost1 = Cost(& activeModules, & AModules);
				int cost2 = Cost(& activeModules, & annealingModules);
				int diff = cost1 - cost2;

				if (diff <= 0)
				{

						copyModules( & AModules , & annealingModules);
						copyModules(& annealingModules, & AModules);
						continue;
				}

				else
				{
					double e = 2.71828;
					double exponent = (diff/(T/2)) * -1;
					double prob = pow(e, exponent);
					prob *= 1000;
					srand(seeder);
					seeder = rand();

					double roll = rand() % 1000;
					if (roll <= prob)
					{

						copyModules( & AModules , & annealingModules);
						copyModules(& annealingModules, & AModules);
						continue;
					}
				}
				copyModules(& annealingModules, & AModules);
			}
			T *= cool;
		}
		placement(annealingNodes, annealingModules, rModules);
	}

	//This is a section of the fixed placer that deals with input / output.
	//Our only interest is in optimizing the positions of operations that take place on the array itself.
	//So, his mechanisms for positioning i/o suffice and are copied here.

	int maxStoragePerModule = getMaxStoragePerModule();
	resetIoResources(arch);

	// sort the lists and copy
	Sort::sortNodesByStartTS(&storeList);
	list<AssayNode*> holdList;
	for (unsigned i = 0; i < schedDag->storageHolders.size(); i++)
		holdList.push_back(schedDag->storageHolders.at(i));

	// do left-edge binding of individual storage nodes into storage-holder scheduled nodes
	// assumes scheduler will always produce a list of storage holders of 1 TS in length
	while (!storeList.empty())
	{
		AssayNode *sNode = storeList.front();
		storeList.pop_front();

		unsigned long long runningEnd = 0;
		bool scheduled = false;
		bool split = false;
		list<AssayNode *> holdersFull;
		list<AssayNode *>::iterator it = holdList.begin();
		for(; it != holdList.end(); it++)
		{
			AssayNode *sHolder = *it;
			if (sNode->GetStatus() != BOUND)
			{
				if (sHolder->GetStartTS() == sNode->GetStartTS()
						//&& sHolder->GetEndTS() <= sNode->GetEndTS()
						&& sHolder->storageOps.size() < maxStoragePerModule)
				{
					sNode->status = BOUND;
					sNode->reconfigMod = sHolder->reconfigMod;
					sNode->boundedResType = sHolder->boundedResType;
					sHolder->storageOps.push_back(sNode);
					sHolder->numDrops++;
					runningEnd = sHolder->endTimeStep;
					scheduled = true;
					if (sHolder->storageOps.size() >= maxStoragePerModule)
						holdersFull.push_back(sHolder);
				}
			}
			else if (scheduled)
			{
				if (sHolder->GetStartTS() == runningEnd
						&& sHolder->storageOps.size() < maxStoragePerModule
						&& sHolder->reconfigMod == sNode->reconfigMod)
				{
					sHolder->storageOps.push_back(sNode);
					sHolder->numDrops++;
					runningEnd = sHolder->endTimeStep;
					if (sHolder->storageOps.size() >= maxStoragePerModule)
						holdersFull.push_back(sHolder);
				}
				else
					split = true;
			}

			// If we're at the end of the list and node hasn't been fully sched, split
			it++;
			if (sNode->endTimeStep != runningEnd && it == holdList.end())
				split = true;
			it--;

			if (split)
			{
				AssayNode *storeSecPart = schedDag->AddStorageNode();
				storeSecPart->startTimeStep = runningEnd;
				storeSecPart->endTimeStep = sNode->endTimeStep;
				sNode->endTimeStep = runningEnd;
				storeSecPart->status = SCHEDULED;
				schedDag->InsertNode(sNode, sNode->children.front(), storeSecPart);
				storeList.remove(sNode);
				storeList.push_front(storeSecPart);
				break;
			}
			if (runningEnd == sNode->endTimeStep)
			{
				storeList.remove(sNode);
				break;
			}
		}
		while (!holdersFull.empty())
		{
			holdList.remove(holdersFull.front());
			holdersFull.pop_front();
		}
	}

	/////////////////////////////////////////////////////////////
	// Now do simple Left-Edge binding for inputs/outputs
	bindInputsLE(opsByResType.at(INPUT_RES));
	bindOutputsLE(opsByResType.at(OUTPUT_RES));
}

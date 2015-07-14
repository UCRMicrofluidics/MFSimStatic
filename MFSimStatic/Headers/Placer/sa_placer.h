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
 * Type: Placer																	*
 * Name: Simulated Annealing Based Placer										*
 *																				*
 * Inferred from the following paper:											*
 * Authors: Fei Su and Krishnendu Chakrabarty									*
 * Title: Module placement for fault-tolerant microfluidics-based biochips		*
 * Publication Details: In TODAES, Vol. 11, No. 3, Article 16, Jul 2006			*
 * 																				*
 * Details: A simulated-annealing placement algorithm.  Does not implement the	*
 * fault-tolerant portions of the above paper, but just the basic SA concepts	*
 * for placement.																*
 *-----------------------------------------------------------------------------*/

#ifndef SA_PLACER_H_
#define SA_PLACER_H_


#include "../Models/dmfb_arch.h"
#include "../Models/op_module.h"
#include "../Testing/claim.h"
#include "../Resources/enums.h"
#include "placer.h"
#include <vector>

class DAG;

class SAPlacer : public Placer
{
	public:

	// Constructors
	SAPlacer();
	virtual ~SAPlacer();

	// Methods
	void place(DmfbArch *arch, DAG *dag, vector<ReconfigModule *> *rModules);

	protected:
		// Methods
		int modCoordinate(OperationType OT, int coord);
		int calcNextTS(vector< list<AssayNode*> * > listOps, int annealTS);
		void calcAnnealingNodes(vector<AssayNode*> * annealing, vector< list<AssayNode*> * > listOps, int nextTS);

		vector<int> uniquePair(vector<int> * horizontal, map<int, vector<int> > * vertical, unsigned int * seeder);
		vector<int> uniqueInteger(vector<int> * horizontal, unsigned int * seeder);

		void generateNeighbor(DmfbArch *arch, vector<OpModule> * fixedResources, vector<OpModule> * activeModules, vector<OpModule> * annealingModules, vector<AssayNode*> * futureNodes);
		int Cost(const vector<OpModule> * activeModules, const vector<OpModule> * annealingModules);
		void copyModules(const vector<OpModule> * ZModules, vector<OpModule> * XModules);
		int areaPair(OpModule A, OpModule B);

		bool outsideArray(DmfbArch *arch, OpModule om);
		bool modIntersect(OpModule om1, OpModule om2);
		void calcAnnealingModules(vector<OpModule> * annealingModules, vector<AssayNode*> annealingNodes);
		bool setAnnealingModules(vector<OpModule> * annealingModules, int select, vector<OpModule> * activeModules, vector<AssayNode*> * futureNodes, vector<OpModule> * fixedResources, DmfbArch *arch);
		void calcFixedResources(vector<OpModule> * fixedResources, DmfbArch * arch);
		void calcFutureNodes(vector<AssayNode*> * futureNodes, vector<OpModule> activeModules, vector<OpModule> annealingModules, vector<list<AssayNode*> *> opsByResType);
		bool fixedResourceCompatible(DmfbArch *arch, vector<OpModule> * fixedResources, vector<OpModule> * activeModules, vector<OpModule> * annealingModules, vector<AssayNode*> * futureNodes);
		bool compatibilityDive(DmfbArch *arch, vector<OpModule> * fixedResources, vector<vector<OpModule> > * PModules, vector<AssayNode*> * futureNodes, int select);
		void updateActiveModules(vector<OpModule> * activeModules, vector<OpModule> annealingModules, int nextTS);
		void placement(vector<AssayNode*> annealingNodes, vector<OpModule> annealingModules, vector<ReconfigModule *> * rModules);
		int invert(int x);
		ResourceType convert(OperationType OT);
		bool identicalPosition(OpModule om1, OpModule om2);

};
#endif /* PLACER_H_ */

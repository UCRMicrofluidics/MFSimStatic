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
 * Name: KAMER (Keep All Maximum Empty Rectangles) Linked-List Placer			*
 * 																				*
 * Detailed in the following paper:												*
 * Authors: Yi Lu, Thomas Marconi, Georgi Gaydadjiev and Koen Bertels			*
 * Title: An Efficient Algorithm for Free Resources Management on the FPGA		*
 * Publication Details: In Proc. DATE, Munich, Germany, 2008					*
 * 																				*
 * Details: Uses the linked-list implementation of KAMER to "keep (find) all	*
 * maximum empty rectangles" for placement. Then, this placer selects an 		*
 * available maximum empty rectangle to place each operation.					*
 *-----------------------------------------------------------------------------*/
#ifndef KAMER_LL_PLACER_H_
#define KAMER_LL_PLACER_H_

#include "../Models/reconfig_module.h"
#include "../Resources/structs.h"
#include "../Resources/enums.h"
#include "../Testing/claim.h"
#include "placer.h"
#include <vector>
#include <set>


class KamerLlPlacer : public Placer
{
	protected:
		// Methods
		KamerLlNode * insertInOutNode(KamerNodeType type, string modName, int height, int left, int right, unsigned long long endTS);
		void removeInOutNode(KamerNodeType type, int hieght, int left, int right);
		void insertRwNode(int height, int left, int right);
		vector<KamerLlNode *> getRwsBelowIE(KamerLlNode *ie);
		void reinitDataStructsToTS(unsigned long long startTS);
		void removeNode(KamerLlNode *remove);
		void debugPrintLinkedLists(AssayNode *justPlaced);
		void debugPrintKamerBoard(DmfbArch *arch);
		void addRemoveModuleFromKamerBoard(KamerLlNode * in, KamerLlNode * out);
		void addRemoveEdgeFromKamerBoard(KamerLlNode * edge);
		bool canPlaceSpecialModuleInMFR(MaxFreeRect *mfr, OperationType targetType, vector<vector<ResourceType>*> *cellType, int wIr, int hIr, int *modPlacedTop, int *modPlacedLeft);
		bool debugPrintEnabled() { return debugPrint; }

		// Members
		vector<vector<KamerCell *> *> *board;
		KamerLlNode *oell;
		KamerLlNode *iell;
		KamerLlNode *rwll;
		bool debugPrint;

	public:
		// Constructors
		KamerLlPlacer();
		virtual ~KamerLlPlacer();

		// Methods
		void place(DmfbArch *arch, DAG *schedDag, vector<ReconfigModule *> *rModules);
};



#endif /* KAMER_LL_PLACER_H_ */

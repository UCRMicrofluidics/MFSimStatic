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
 * Name: Grissom's Path Binder													*
 * 																				*
 * Detailed in the following paper:												*
 * Authors: Dan Grissom and Philip Brisk										*
 * Title: Fast Online Synthesis of Digital Microfluidic Biochips				*
 * Publication Details: IEEE Transactions on Computer-Aided Design (TCAD)		*
 * of Integrated Circuits and SystemsTCAD, Accepted on October 12, 2013.		*
 * 																				*
 * Details: Provided a set of fixed module locations, binds scheduled			*
 * operations to particular location; attempts to do this by examining the		*
 * "path" through a sequencing graph and causing as many consecutive nodes		*
 * to be bound to the same fixed module location as possible.					*
 *-----------------------------------------------------------------------------*/

#ifndef GRISSOM_PATH_BINDER_H_
#define GRISSOM_PATH_BINDER_H_

#include "../Resources/structs.h"
#include "../Resources/enums.h"
#include "placer.h"
#include <vector>
#include <set>

class FixedModule;

class GrissomPathBinder : public Placer
{
	protected:
		// Methods
		int computeManDist(FixedModule *m1, ReconfigModule *m2);
		AssayNode *addAndBindNewSHNode(DmfbArch *arch, DAG *schedDag, unsigned long long startTS, unsigned long long endTS, FixedModule *fm);
		AssayNode *addAndBindNewSnodeToSHnode(DAG *schedDag, AssayNode *sh);
		void addAndBindSnodeToSHnode(AssayNode *s, AssayNode *sh);

		// Members

	public:
		// Constructors
		GrissomPathBinder();
		virtual ~GrissomPathBinder();

		// Methods
		void place(DmfbArch *arch, DAG *schedDag, vector<ReconfigModule *> *rModules);
};



#endif /* GRISSOM_PATH_BINDER_H_ */

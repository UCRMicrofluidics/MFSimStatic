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
 * Name: ReconfigModule (Reconfigurable Module)									*
 *																				*
 * Details: Represents a reconfigurable module that can be placed anywhere on 	*
 * the DMFB at any time.  These are the objects that are "placed" onto the DMFB	*
 * and correspond directly to the operations/nodes.								*
 *-----------------------------------------------------------------------------*/
#ifndef _RECONFIG_MODULE_H
#define _RECONFIG_MODULE_H

#include "../Models/entity.h"
#include "../Resources/enums.h"
#include "../Models/assay_node.h"

class AssayNode;

class ReconfigModule : public Entity
{
	protected:
		// Variables
		static int next_id;
		int leftX;
		int topY;
		int rightX;
		int bottomY;
		int numDrops;
		ResourceType resourceType;
		unsigned long long startTimeStep;
		unsigned long long endTimeStep;// exclusive, so if it starts on 1 and the endTimeStep is 3, the operation is 2 cycles
		AssayNode *boundNode;
		ReconfigModule *linkedModule;
		int tiledNum; // If in a tileable design, tells which module for easy indexing

	public:
		// Constructors
		ReconfigModule(ResourceType rType, int lx, int ty, int rx, int by);
		virtual ~ReconfigModule();

		// Methods
		int getNumCellsInModule();
		int getNumCellsInCirc();

		// Getters/Setters
		int getLX() { return leftX; }
		int getTY() { return topY; }
		int getRX() { return rightX; }
		int getBY() { return bottomY; }
		ResourceType getResourceType() { return resourceType; }
		int getNumDrops() { return numDrops; }
		void incNumDrops() { numDrops = numDrops+1; }
		void resetNumDrops() { numDrops = 0; }
		void setTileNum(int tn) { tiledNum = tn; }
		int getTileNum() { return tiledNum; }
		int getStartTS() { return startTimeStep; }
		int getEndTS() { return endTimeStep; }
		AssayNode *getBoundNode() { return boundNode; }

		// Friend Classes
		friend class GrissomFppcLEBinder;
		friend class GrissomPathBinder;
		friend class GrissomLEBinder;
		friend class KamerLlPlacer;
		friend class FileOut;
		friend class FileIn;
};
#endif /* _RECONFIG_MODULE_H */

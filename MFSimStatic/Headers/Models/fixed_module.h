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
 * Name: FixedModule (Fixed Module)												*
 *																				*
 * Details: Represents a fixed, rectangular/square location on the DMFB that	*
 * represents the location of a fixed resources such as a heater, detecter, etc.*
 *-----------------------------------------------------------------------------*/
#ifndef _FIXED_MODULE_H
#define _FIXED_MODULE_H

#include "../Models/entity.h"
#include "../Resources/enums.h"

class FixedModule : public Entity
{
	protected:
		// Variables
		static int next_id;
		int leftX;
		int topY;
		int rightX;
		int bottomY;
		ResourceType resourceType;
		int tiledNum;
		bool busy;
		bool ready;

	public:
		// Constructors
		FixedModule(ResourceType rType, int lx, int ty, int rx, int by);
		virtual ~FixedModule();

		// Getters/Setters
		int getLX() { return leftX; }
		int getTY() { return topY; }
		int getRX() { return rightX; }
		int getBY() { return bottomY; }
		void setTileNum(int tn) { tiledNum = tn; }
		int getTileNum() { return tiledNum; }
		ResourceType getResourceType() { return resourceType; }
		void setBusy(bool b) { busy = b; }
		bool getBusy() { return busy; }
		void setReady(bool r) { ready = r; }
		bool getReady() { return ready; }

		// Friend Classes
		friend class FileOut;
		friend class FileIn;
};
#endif /* _FIXED_MODULE_H */

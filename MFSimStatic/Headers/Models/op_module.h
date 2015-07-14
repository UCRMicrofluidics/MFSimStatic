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
 * Name: OpModule (Opearation Module)											*
 *																				*
 * Details: A module used specifically by the simulated annealing placer.		*
 * It is similar to the ReconfigModule, but varies slightly.
 *-----------------------------------------------------------------------------*/
#ifndef FREE_MODULE_H_
#define FREE_MODULE_H_

#include "../Models/entity.h"
#include "../Resources/enums.h"

class OpModule : public Entity
{
	protected:
		// Variables
		static int next_id;
		int leftX;
		int topY;
		int rightX;
		int bottomY;
		OperationType operationType;
		ResourceType boundedResType;

		unsigned long long startTS;
		unsigned long long endTS;

	public:
		// Constructors
		OpModule(OperationType oType, int lx, int ty, int rx, int by);
		OpModule(ResourceType rType, int lx, int ty, int rx, int by);
		OpModule(OperationType oType, ResourceType rType, int lx, int ty, int rx, int by, unsigned long long start, unsigned long long end);
		virtual ~OpModule();

		// Getters/Setters
		int getLX() { return leftX; }
		int getTY() { return topY; }
		int getRX() { return rightX; }
		int getBY() { return bottomY; }
		OperationType getOperationType() { return operationType; }
		ResourceType getBoundedResType() { return boundedResType; }
		unsigned long long getStartTS() { return startTS; }
		unsigned long long getEndTS() { return endTS; }
		void setLX(int LX) { leftX = LX; }
		void setTY(int TY) { topY = TY; }
		void setRX(int RX) { rightX = RX; }
		void setBY(int BY) { bottomY = BY; }
		void setOperationType(OperationType OT) { operationType = OT; }
		void setStartTS(unsigned long long TS) { startTS = TS; }
		void setEndTS(unsigned long long TS) { endTS = TS; }
		void setBoundedResType(ResourceType Rt) { boundedResType = Rt; }


		OpModule operator=(const OpModule& opM) const;
		void display();

};
#endif /* FREE_MODULE_H_ */

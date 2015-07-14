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
 * Name: Wire Segment															*
 *																				*
 * Details: A basic structure to represent a wire segment for connecting the	*
 * electrodes and electrode-groups to an external pin via the PCB layers.		*
 *-----------------------------------------------------------------------------*/
#ifndef WIRE_SEGMENT_H_
#define WIRE_SEGMENT_H_

#include "../Models/entity.h"
#include "../Resources/enums.h"


class WireSegment : Entity
{
	private:
		// General identifiers and status variables
		static int next_id;


		int pinNo;
		int layer;
		WireSegType segmentType;

		// Start/stop relative locations
		int sourceWireCellX;
		int sourceWireCellY;
		int destWireCellX;
		int destWireCellY;

		// Arc parameters
		int startAngle;
		int arcAngle;


	public:
		// Constructors
		WireSegment();
		WireSegment(int pin, int lay, int beginX, int beginY, int endX, int endY);
		virtual ~WireSegment();

		// Getters/Setters
		int getId() { return id; }
		int getPinNum() { return pinNo; }
		int getLayerNum() { return layer; }
		int getSourceWireCellX(int x) { return sourceWireCellX; }
		int getSourceWireCellY(int y) { return sourceWireCellY; }
		int getDestWireCellX(int x) { return destWireCellX; }
		int getDestWireCellY(int y) { return destWireCellY; }
		double getLengthInWireCells();


		void setSourceWireCellX(int x) { sourceWireCellX = x; }
		void setSourceWireCellY(int y) { sourceWireCellY = y; }
		void setDestWireCellX(int x) { destWireCellX = x; }
		void setDestWireCellY(int y) { destWireCellY = y; }
		void setSegmentType(WireSegType type) { segmentType = type; }
		void setPinNo(int num) { pinNo = num; }
		void setLayer(int l) { layer = l; }

		friend class PathFinderWireRouter;
		friend class YehWireRouter;
		friend class WireRouter;
		friend class FileOut;
		friend class FileIn;
};
#endif /* WIRE_SEGMENT_H_ */

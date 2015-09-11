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
 * Name: Droplet																*
 *																				*
 * Details: A basic structure to represent a droplet.							*
 *-----------------------------------------------------------------------------*/
#ifndef DROPLET_H_
#define DROPLET_H_

using namespace std;
#include "entity.h"
#include <string>

class Droplet : Entity
{
	private:
		// General identifiers and status variables
		static int next_id;
		string uniqueFluidName; // Describes the chemical makeup of the droplet
		float volume; // Volume in micro-liters (uL)

		// SkyCal
		static const int HP_INIT = 1000;
		float completion; // SkyCal Router for operation completion percentage
		int hp; //SkyCal router wash droplet health points
		bool isWashDroplet;

	public:
		// Constructors
		Droplet();
		virtual ~Droplet();

		// Getters/Setters
		int getId() { return id; }
		float getVolume() { return volume; }
		string getComposition() { return uniqueFluidName; }
		bool isWashing() { return isWashDroplet; }

		// Friend Classes
		friend class PostSubproblemCompactionRouter;
		friend class GrissomFixedPlaceMapRouter;
		friend class GrissomFixedPlaceRouter;
		friend class SinghaRouter;
		friend class RoyRouter;
		friend class Router;
		friend class GrissomFppcRouter;
};
#endif /* DROPLET_H_ */

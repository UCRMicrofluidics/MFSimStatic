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
 * Name: Entity 																*
 *																				*
 * Details: A basic structure that contains an Id.  Other classes inherit 		*
 * from this simply so the 'id' field doesn't have to be added to every class.	*
 *-----------------------------------------------------------------------------*/
#ifndef _ENTITY_H
#define _ENTITY_H

class Entity
{
	protected:
		// Variables
		int id;

	public:
		// Constructors
		Entity();
		virtual ~Entity();

		// Getters/Setters
		int getId() { return id; }
};
#endif /* _ENTITY_H */

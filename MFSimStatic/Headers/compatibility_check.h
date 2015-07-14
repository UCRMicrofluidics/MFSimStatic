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
 * Name: CompatChk (Compatibility Check)										*
 *																				*
 * Details: This class is a static class that is meant to be added to and 		*
 * modified to add custom compatibility checks as new synthesis methods are		*
 * created and known to conflict with each other.  This will be especially 		*
 * helpful as the framework is used more by casual users so that meaningful		*
 * errors will be displayed instead of the program just crashing if 			*
 * incompatible parameters and settings are used.								*
 *-----------------------------------------------------------------------------*/
#ifndef COMPATIBILITY_CHECK_H_
#define COMPATIBILITY_CHECK_H_

class DmfbArch;
class Scheduler;
class Placer;
class Router;

class CompatChk
{
	public:
		// Constructors
		CompatChk();
		virtual ~CompatChk();

		// Methods
		static void PreScheduleChk(Scheduler *s, DmfbArch *arch, bool runAsEntireFlow);
		static void PrePlaceChk(Placer *p, DmfbArch *arch, bool runAsEntireFlow);
		static void PreRouteChk(Router *r, DmfbArch *arch, bool runAsEntireFlow);
		static void PreWireRouteChk(DmfbArch *arch, bool runAsEntireFlow);
		static bool CanPerformRouteAnalysis(Router *r);
		static bool CanPerformCompactSimulation(Router *r);
};

#endif /* COMPATIBILITY_CHECK_H_ */

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
 * Name: ElapsedTimer (Elapsed Timer)											*
 *																				*
 * Details: A timer than measures accuracy down to ns.  Is compatible in both	*
 * Windows and Linux.  For windows, add "winmm" to your libraries(-l). For		*
 * Linux, add "rt" to your libraries in Eclipse.								*
 *-----------------------------------------------------------------------------*/
#ifndef _ELAPSED_TIMER_H
#define _ELAPSED_TIMER_H

using namespace std;
#include <iostream>

#ifdef _WIN32
	#include <Windows.h>
#else
	#include <time.h>
#endif

class ElapsedTimer
{
	protected:
		// Variables
	#ifdef _WIN32
		LARGE_INTEGER timerFreq_;
		LARGE_INTEGER counterAtStart_;
		unsigned int diffTime;
		unsigned int lastTime;
		unsigned int newTime;
	#else
		timespec lastTime;
		timespec newTime;
		timespec diffTime;
	#endif

		string operationName;

		// Methods
		unsigned int calculateElapsedTime();


	public:
		// Constructors
		ElapsedTimer(string opName);
		virtual ~ElapsedTimer();

		// Methods
		void startTimer();
		void endTimer();
		double getElapsedTimeNS();
		void printElapsedTime();
};
#endif

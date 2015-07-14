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
/*---------------------------Implementation Details-----------------------------*
 * Source: elapsed_timer.cc														*
 * Original Code Author(s): Dan Grissom											*
 * Original Completion/Release Date: October 7, 2012							*
 *																				*
 * Details: 																	*
 * This class is tested and should work on Windows and Linux/Posix.				*
 * Small adjustments for linking must be made if using windows/linux:			*
 *																				*
 * Windows: Also, the project must be linked with Winmm.lib by adding			*
 * "winmm" to "Project->Properties->C/C++ Build->Settings->						*
 * MinGW (or other) C++ Linker->Libraries"										*
 *																				*
 * Linux/Posix: Pass '-lrt' to gcc by adding "rc" to							*
 * "Project->Properties->C/C++ Build->Settings->GCC C++ Linker->				*
 * Libraries->Libraries(-l)"													*
 *																				*
 * Revision History:															*
 * WHO		WHEN		WHAT													*
 * ---		----		----													*
 * FML		MM/DD/YY	One-line description									*
 *-----------------------------------------------------------------------------*/

#include "../../Headers/Testing/elapsed_timer.h"

///////////////////////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////////////////////
ElapsedTimer::ElapsedTimer(string opName)
{
	operationName = opName;
#ifdef _WIN32
	diffTime = 0;
	lastTime = 0;
	newTime = 0;
#endif
};

///////////////////////////////////////////////////////////////////////////////////
// Deconstructor
///////////////////////////////////////////////////////////////////////////////////
ElapsedTimer::~ElapsedTimer() {};

////////////////////////////////////////////////////////////////////
// Starts the timer.
////////////////////////////////////////////////////////////////////
void ElapsedTimer::startTimer()
{
#ifdef _WIN32
	timeBeginPeriod(1); //Add Winmm.lib in Project
	QueryPerformanceFrequency(&timerFreq_);
	QueryPerformanceCounter(&counterAtStart_);
	//cout<<"timerFreq_ = "<<timerFreq_.QuadPart<<endl;
	//cout<<"counterAtStart_ = "<<counterAtStart_.QuadPart<<endl;
	TIMECAPS ptc;
	UINT cbtc = 8;
	MMRESULT result = timeGetDevCaps(&ptc, cbtc);
	if (result != TIMERR_NOERROR)
	{
		cout << "result = TIMER ERROR - MUST BE USING WINDOWS FOR THIS FUNCTION." << endl;
		exit(1);
	}
	/*else
	 {
	 cout<<"Minimum resolution = "<<ptc.wPeriodMin<<endl;
	 cout<<"Maximum resolution = "<<ptc.wPeriodMax<<endl;
	 }*/
	lastTime = calculateElapsedTime();
#else
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &lastTime);
#endif
}

////////////////////////////////////////////////////////////////////
// Ends the timer.
////////////////////////////////////////////////////////////////////
void ElapsedTimer::endTimer()
{
#ifdef _WIN32
	newTime = calculateElapsedTime();
#else
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &newTime);
	calculateElapsedTime();
#endif

}

////////////////////////////////////////////////////////////////////
// Returns the elapsed time, based on the start and stop time, in
// nano-seconds
////////////////////////////////////////////////////////////////////
double ElapsedTimer::getElapsedTimeNS()
{
#ifdef _WIN32
	return (double)(((double)(newTime-lastTime))*(double)1000000);
#else
	return (double)((double)(diffTime.tv_sec*1000000000) + (double)(diffTime.tv_nsec));
#endif
}

////////////////////////////////////////////////////////////////////
// Prints the elapsed time, based on the start and stop time.
////////////////////////////////////////////////////////////////////
void ElapsedTimer::printElapsedTime()
{
#ifdef _WIN32
	cout << "--------------------TIMER----------------------" << endl;
	cout << "Elapsed time for \"" << operationName << "\" = " << newTime-lastTime << "ms" << endl;
	cout << "-----------------------------------------------" << endl;
#else
	cout << "--------------------TIMER----------------------" << endl;
	cout << "Elapsed time for \"" << operationName << "\" = ";
	cout << diffTime.tv_sec << "s:" << diffTime.tv_nsec << "ns" << " (" <<  diffTime.tv_sec*1000 + (int)(diffTime.tv_nsec/1000000) <<"ms)" << endl;
	cout << "-----------------------------------------------" << endl;
#endif

}

////////////////////////////////////////////////////////////////////
// Calculates the time.
////////////////////////////////////////////////////////////////////
unsigned int ElapsedTimer::calculateElapsedTime()
{
#ifdef _WIN32
	if (timerFreq_.QuadPart == 0)
	{
		return -1;
	}
	else
	{
		LARGE_INTEGER c;
		QueryPerformanceCounter(&c);
		return static_cast<unsigned int> ((c.QuadPart - counterAtStart_.QuadPart) * 1000 / timerFreq_.QuadPart);
	}
#else
	if ((newTime.tv_nsec-lastTime.tv_nsec)<0) {
		diffTime.tv_sec = newTime.tv_sec-lastTime.tv_sec-1;
		diffTime.tv_nsec = 1000000000+newTime.tv_nsec-lastTime.tv_nsec;
	} else {
		diffTime.tv_sec = newTime.tv_sec-lastTime.tv_sec;
		diffTime.tv_nsec = newTime.tv_nsec-lastTime.tv_nsec;
	}
#endif
}

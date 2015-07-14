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
 * Name: CmdLine (Command Line)													*
 *																				*
 * Details: This class handles the command line interface for the simulator.	*
 * It is designed to be easy to add new methods (schedulers, placers, etc.).	*
 * Simply add new entries in the constructor in command_line.cc, as shown in	*
 * that file.																	*
 *-----------------------------------------------------------------------------*/
#ifndef COMMAND_LINE_H_
#define COMMAND_LINE_H_

#include "Util/file_in.h"
#include <string>

class CmdLine
{
	public:
		// Constructors
		CmdLine();
		virtual ~CmdLine();

		void ForceCorrectUsage();
		bool IsUsingCommandLine(int argc);
		void ExecuteCommand(int argc, char **argv);

		friend class FileOut;
		friend class FileIn;

	private:
		string GetUsageString();
		string GetSchedulerLibrary();
		int GetLibIndexFromSchedKey(string sKey);
		int GetLibIndexFromPlaceKey(string pKey);
		int GetLibIndexFromRouteKey(string rKey);
		int GetLibIndexFromResourceAllocationKey(string raKey);
		int GetLibIndexFromPinMapKey(string pmKey);
		int GetLibIndexFromCompKey(string cKey);
		int GetLibIndexFromPeKey(string peKey);
		int GetLibIndexFromEtKey(string etKey);
		int GetLibIndexFromWrKey(string wrKey);

		vector<string> sKeys; 		// The command-line key for a scheduler
		vector<string> sDescrips;	// Description/name of scheduler
		vector<SchedulerType> sEnums;// Enum-type of scheduler

		vector<string> pKeys; 		// The command-line key for a placer
		vector<string> pDescrips;	// Description/name of placer
		vector<PlacerType> pEnums;	// Enum-type of placer

		vector<string> rKeys; 		// The command-line key for a router
		vector<string> rDescrips;	// Description/name of router
		vector<RouterType> rEnums;	// Enum-type of router

		vector<string> raKeys; 		// The command-line key for a resource-allocator
		vector<string> raDescrips;	// Description/name of resource-allocator
		vector<ResourceAllocationType> raEnums;	// Enum-type of resource-allocator

		vector<string> pmKeys; 		// The command-line key for a pin-mapper
		vector<string> pmDescrips;	// Description/name of pin-mapper
		vector<PinMapType> pmEnums;	// Enum-type of pin-mapper

		vector<string> wrKeys; 		// The command-line key for a wire-router
		vector<string> wrDescrips;	// Description/name of wire-router
		vector<WireRouteType> wrEnums;	// Enum-type of wire-router

		vector<string> cKeys; 		// The command-line key for a compaction
		vector<string> cDescrips;	// Description/name of compaction type
		vector<CompactionType> cEnums;	// Enum-type of compaction

		vector<string> peKeys; 		// The command-line key for the time-step processing engine
		vector<string> peDescrips;	// Description/name of time-step processing engine
		vector<ProcessEngineType> peEnums;	// Enum-type of time-step processing

		vector<string> etKeys; 		// The command-line key for the execution type
		vector<string> etDescrips;	// Description/name of execution type
		vector<ExecutionType> etEnums;	// Enum-type of execution type
};


#endif /* COMMAND_LINE_H_ */

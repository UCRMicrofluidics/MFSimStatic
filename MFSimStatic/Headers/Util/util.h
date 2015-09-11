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
 * Name: Util (Utility)															*
 *																				*
 * Details: This class contains the "random" general purpose methods, such as	*
 * string manipulation methods, that can be used by many classes.				*
  *-----------------------------------------------------------------------------*/
#ifndef _UTIL_H
#define _UTIL_H

using namespace std;
#include <fstream>
#include <string>
#include <cstdio>

class Util
{
	public:
		// Constructors
		Util();
		virtual ~Util();

		// String Methods
		static string StringToUpper(string strToConvert);
		static string StringToLower(string strToConvert);
		static string TrimString(string str);
		static int CountSubstring(const string& str, const string& sub);

		// Parsing Methods
		static char* itoa(int value, char* str, int base); // Only works in base 10
};
#endif

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
 * Source: util.cc																*
 * Original Code Author(s): Dan Grissom											*
 * Original Completion/Release Date: October 7, 2012							*
 *																				*
 * Details: N/A																	*
 *																				*
 * Revision History:															*
 * WHO		WHEN		WHAT													*
 * ---		----		----													*
 * FML		MM/DD/YY	One-line description									*
 *-----------------------------------------------------------------------------*/
#include "../../Headers/Util/util.h"

///////////////////////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////////////////////
Util::Util() {}

///////////////////////////////////////////////////////////////////////////////////
// Deconstructor
///////////////////////////////////////////////////////////////////////////////////
Util::~Util(){}

///////////////////////////////////////////////////////////////
// Converts to string with all upper-case characters
///////////////////////////////////////////////////////////////
string Util::StringToUpper(string strToConvert)
{
	for(unsigned int i=0;i<strToConvert.length();i++)
	{
		strToConvert[i] = toupper(strToConvert[i]);
	}
	return strToConvert;//return the converted string
}

///////////////////////////////////////////////////////////////
// Converts to string with all lower-case characters
///////////////////////////////////////////////////////////////
string Util::StringToLower(string strToConvert)
{
	for(unsigned int i=0;i<strToConvert.length();i++)
	{
		strToConvert[i] = tolower(strToConvert[i]);
	}
	return strToConvert;//return the converted string
}

///////////////////////////////////////////////////////////////
// Counts the number of times a sub-string (sub) occurs in
// another string (str)
///////////////////////////////////////////////////////////////
int Util::CountSubstring(const string& str, const string& sub)
{
	if (sub.length() == 0)
		return 0;
	int count = 0;
	for (size_t offset = str.find(sub); offset != std::string::npos; offset = str.find(sub, offset + sub.length()))
		count++;
	return count;
}


////////////////////////////////////////////////////////////////////////////
// Remove all white space characters from beginning and end
////////////////////////////////////////////////////////////////////////////
string Util::TrimString(string str)
{
	while (str.find_first_of(" \n\r\t") == 0)
		str = str.substr(1);
	while (str.find_last_of(" \n\r\t") == str.length()-1)
		str = str.substr(0, str.length()-1);
	return str;
}

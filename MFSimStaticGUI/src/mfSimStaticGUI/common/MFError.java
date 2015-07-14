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
/*------------------------Class/Implementation Details--------------------------*
 * Source: MFError.java (MicroFluidic Error)									*
 * Original Code Author(s): Dan Grissom											*
 * Original Completion/Release Date: October 7, 2012							*
 *																				*
 * Details: Provides a basic function to output an error via a pop-up dialog.	*
 *																				*
 * Revision History:															*
 * WHO		WHEN		WHAT													*
 * ---		----		----													*
 * FML		MM/DD/YY	One-line description									*
 *-----------------------------------------------------------------------------*/

package mfSimStaticGUI.common;

import javax.swing.JOptionPane;


public class MFError {
	/*public static void Print(String error)
	{
		System.err.println(error);
	}
	public static void PrintAndHalt(String error)
	{
		System.err.println("JAVA Error: " + error);
		System.exit(1);
	}*/
	
	//////////////////////////////////////////////////////////////////////////////////////
	// Displays the given error message in a pop-up window
	//////////////////////////////////////////////////////////////////////////////////////
	public static void DisplayError(String error)
	{
		JOptionPane.showMessageDialog(null, error, "DMFB Controller Error", JOptionPane.ERROR_MESSAGE);
	}
}

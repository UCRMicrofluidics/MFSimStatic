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
 * Source: DmfbArch.java (DMFB Architecture)									*
 * Original Code Author(s): Dan Grissom											*
 * Original Completion/Release Date: October 7, 2012							*
 *																				*
 * Details: Provides a basic structure for a DMFB architecture.					*
 *																				*
 * Revision History:															*
 * WHO		WHEN		WHAT													*
 * ---		----		----													*
 * FML		MM/DD/YY	One-line description									*
 *-----------------------------------------------------------------------------*/

package dmfbSimVisualizer.common;

import java.util.ArrayList;
import java.util.Random;

public class DmfbArch {
	String name;
	public ArrayList<FixedArea> ExternalResources;
	public ArrayList<FixedArea> ResourceLocations;
	public ArrayList<IoPort> IoPorts;
	public int numXcells;
	public int numYcells;
	public int numHTracks;
	public int numVTracks;
	public int numXwireCells;
	public int numYwireCells;


	//////////////////////////////////////////////////////////////////////////////////////
	// Constructor
	//////////////////////////////////////////////////////////////////////////////////////
	public DmfbArch()
	{
		name = "Un-named Architecture";
		ExternalResources = new ArrayList<FixedArea>();
		ResourceLocations = new ArrayList<FixedArea>();
		IoPorts = new ArrayList<IoPort>();
		numXcells = 0;
		numYcells = 0;
		numHTracks = 0;
		numVTracks = 0;
		numXwireCells = 0;
		numYwireCells = 0;
	}
}

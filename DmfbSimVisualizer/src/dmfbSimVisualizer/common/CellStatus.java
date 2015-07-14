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
 * Source: CellStatus.java (CellStatus)											*
 * Original Code Author(s): Dan Grissom											*
 * Original Completion/Release Date: October 7, 2012							*
 *																				*
 * Details: Provides a basic structure for the status of a cell. This class		*
 * can hold information for whether a droplet is occupying a cell or other		*
 * info, such as whether the cell is dirty and by what droplet.
 *																				*
 * Revision History:															*
 * WHO		WHEN		WHAT													*
 * ---		----		----													*
 * DTG		04/18/14	Changed from Droplet.java to make more generic/usable	*
 *-----------------------------------------------------------------------------*/

package dmfbSimVisualizer.common;

import java.awt.Color;



public class CellStatus {
	//public enum COLOR { LIGHT_GREEN, GREEN, YELLOW, ORANGE, RED, LIGHT_BLUE, CYAN, LIGHT_GRAY, WHITE, MAGENTA } 
	public int Cell;
	public int Global_ID;
	public String Status;
	public Color DrawColor;
	public int xCell;
	public int yCell;
	
	//////////////////////////////////////////////////////////////////////////////////////
	// Constructor
	//////////////////////////////////////////////////////////////////////////////////////
	public CellStatus(int cell, int gid, String status)
	{
		Cell = cell;
		Global_ID = gid;
		Status = status;
		DrawColor = GetColor(Status);		
	}
	
	//////////////////////////////////////////////////////////////////////////////////////
	// Constructor
	//////////////////////////////////////////////////////////////////////////////////////
	public CellStatus(int x, int y, int gid, String status)
	{
		xCell = x;
		yCell = y;
		Global_ID = gid;
		Status = status;
		DrawColor = GetColor(Status);
	}	
	
	//////////////////////////////////////////////////////////////////////////////////////
	// Gets the appropriate color given the status of the droplet
	//////////////////////////////////////////////////////////////////////////////////////
	static public Color GetColor(String status)
	{
		if (status.contains("D_INT_WAIT"))
			return Color.YELLOW;
		else if (status.contains("D_PROC"))
			return Color.WHITE;
		else if (status.contains("D_MERGE"))
			return Color.ORANGE;
		else if (status.contains("D_SPLIT"))
			return Color.MAGENTA;
		else if (status.contains("D_OUT"))
			return Color.GREEN;
		else if (status.contains("D_PROC_WAIT"))
			return Color.RED;
		else if (status.contains("D_WASH"))
			return new Color(149,179,215);
		else if (status.contains("D_WASTE"))
			return Color.CYAN;
		else if (status.contains("D_NORM"))
			return new Color(195,214,155);
		else if (status.contains("C_DIRTY"))
			return new Color(0.580f, 0.463f, 0.290f, 0.5f);
		else // D_NORM & C_DIRTY
			return Color.BLACK;
	}
}


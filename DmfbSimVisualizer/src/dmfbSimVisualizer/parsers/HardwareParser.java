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
 * Source: HardwareParser.java (HardwareParser)									*
 * Original Code Author(s): Dan Grissom											*
 * Original Completion/Release Date: March 30, 2013								*
 *																				*
 * Details: Parses a hardware input file.										*
 *																				*
 * Revision History:															*
 * WHO		WHEN		WHAT													*
 * ---		----		----													*
 * FML		MM/DD/YY	One-line description									*
 *-----------------------------------------------------------------------------*/

package dmfbSimVisualizer.parsers;

import java.io.BufferedReader;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;

import dmfbSimVisualizer.common.*;
import dmfbSimVisualizer.common.WireSegment.SEGTYPE;

public class HardwareParser {
	private ArrayList<FixedArea> ExternalResources;
	private ArrayList<FixedArea> ResourceLocations;
	private ArrayList<IoPort> IoPorts;
	private Map<Integer, ArrayList<String>> CoordsAtPin;
	private Map<Integer, ArrayList<WireSegment>> WireSegsToPin;
	private int numXcells;
	private int numYcells;
	private int numLayers;
	private int numHTracks;
	private int numVTracks;
	private int numXwireCells;
	private int numYwireCells;
	
	//////////////////////////////////////////////////////////////////////////////////////
	// Constructor
	//////////////////////////////////////////////////////////////////////////////////////
	public HardwareParser(String fileName)
	{

		ExternalResources = new ArrayList<FixedArea>();
		ResourceLocations = new ArrayList<FixedArea>();
		IoPorts = new ArrayList<IoPort>();
		CoordsAtPin = new HashMap<Integer, ArrayList<String>>();
		WireSegsToPin = new HashMap<Integer, ArrayList<WireSegment>>();
		numXcells = 0;
		numYcells = 0;
		numLayers = 0;
		numHTracks = 0;
		numVTracks = 0;
		ReadFile(fileName);
	}
	
	//////////////////////////////////////////////////////////////////////////////////////
	//Open and read from MF output file
	//////////////////////////////////////////////////////////////////////////////////////
	private void ReadFile(String fileName) {
		BufferedReader bf = null;
		try {
			bf = new BufferedReader(new FileReader(fileName));
			String line = null;
			int cycleNum = 0;
			
			while ((line = bf.readLine()) != null)
			{
				line = line.toUpperCase();

				if (line.startsWith(("DIM (")))
				{
					String params = line.substring(line.indexOf("(")+1, line.indexOf(")"));
					String[] tokens = params.split(",");
					for (int i = 0; i < tokens.length; i++)
						tokens[i] = tokens[i].trim();
					if (tokens.length != 2)
					{
						MFError.DisplayError(line + "\n\n" + "Dim must have 2 parameters: ([X_Cells], [Y_Cells])");
						return;
					}
					numXcells = Integer.parseInt(tokens[0]);
					numYcells = Integer.parseInt(tokens[1]);
				}
				else if (line.startsWith("PINMAP ("))
				{
					String params = line.substring(line.indexOf("(")+1, line.indexOf(")"));
					String[] tokens = params.split(",");
					for (int i = 0; i < tokens.length; i++)
						tokens[i] = tokens[i].trim();
					if (tokens.length != numXcells*numYcells)
					{
						MFError.DisplayError(line + "\n\n" + "Dim must have " + numXcells*numYcells + " parameters: ([X_Cells], [Y_Cells])");
						return;
					}
					
					int i = 0;
					
					for (int y = 0; y < numYcells; y++)
					{
						for (int x = 0; x < numXcells; x++)
						{
							String coord = "(" + x + ", " + y + ")";
							
							int pinNo = Integer.parseInt(tokens[i++]);
							
							ArrayList<String> al = CoordsAtPin.get(pinNo);
							if (al == null)
							{
								al = new ArrayList<String>();
								CoordsAtPin.put(pinNo, al);
							}									
							CoordsAtPin.get(pinNo).add(coord);
						}
					}
				}
				else if (line.startsWith("EXTERNALHEATER (") || line.startsWith("EXTERNALDETECTOR ("))
				{
					String params = line.substring(line.indexOf("(")+1, line.indexOf(")"));
					String[] tokens = params.split(",");
					for (int i = 0; i < tokens.length; i++)
						tokens[i] = tokens[i].trim();
					if (tokens.length != 5)
					{
						MFError.DisplayError(line + "\n\n" + "ExternalHeater/ExternalDetector must have 5 parameters: ([Heater/Detector_Id], [TL_X], [TL_Y], [BR_X], [BR_Y]) ");
						return;
					}
					FixedArea la = new FixedArea();
					la.id = Integer.parseInt(tokens[0]);
					la.tl_x = Integer.parseInt(tokens[1]);
					la.tl_y = Integer.parseInt(tokens[2]);
					la.br_x = Integer.parseInt(tokens[3]);
					la.br_y = Integer.parseInt(tokens[4]);
					
					if (line.startsWith("EXTERNALHEATER"))
					{
						la.name = "FH" + String.valueOf(la.id);
						la.opType = OperationType.HEAT;
					}
					else
					{
						la.name = "FD" + String.valueOf(la.id);
						la.opType = OperationType.DETECT;
					}

					ExternalResources.add(la);
				}
				else if (line.startsWith("RESOURCELOCATION ("))
				{
					String params = line.substring(line.indexOf("(")+1, line.indexOf(")"));
					String[] tokens = params.split(",");
					for (int i = 0; i < tokens.length; i++)
						tokens[i] = tokens[i].trim();
					if (tokens.length != 6)
					{
						MFError.DisplayError(line + "\n\n" + "Resource Location must have 6 parameters: ([HEAT/DETECT], [TL_X], [TL_Y], [BR_X], [BR_Y], tileNum) ");
						return;
					}
					FixedArea fa = new FixedArea();
					fa.tl_x = Integer.parseInt(tokens[1]);
					fa.tl_y = Integer.parseInt(tokens[2]);
					fa.br_x = Integer.parseInt(tokens[3]);
					fa.br_y = Integer.parseInt(tokens[4]);
					// We don't care about the resource type (tokens[0]); just using to draw box

					ResourceLocations.add(fa);
				}
				else if (line.startsWith("INPUT (") || line.startsWith("OUTPUT ("))
				{
					String params = line.substring(line.indexOf("(")+1, line.indexOf(")"));
					String[] tokens = params.split(",");
					for (int i = 0; i < tokens.length; i++)
						tokens[i] = tokens[i].trim();
					if (tokens.length != 5)
					{
						MFError.DisplayError(line + "\n\n" + "Input/Output must have 5 parameters: ([Input/Output_Id], [Side=Top/Bottom/Left/Right], [PosXorY], [FluidName], [WashPort=True/False])");
						return;
					}
					IoPort ioPort = new IoPort();
					ioPort.id = Integer.parseInt(tokens[0]);
					ioPort.side = tokens[1].toUpperCase();
					ioPort.pos_xy = Integer.parseInt(tokens[2]);
					ioPort.portName = tokens[3].toUpperCase();
					if (tokens[4].toUpperCase().equals("TRUE"))
							ioPort.containsWashFluid = true;
					else
						ioPort.containsWashFluid = false;
						
					
					if (line.startsWith("INPUT ("))
					{
						ioPort.opType = OperationType.INPUT;
						ioPort.name = "I" + String.valueOf(ioPort.id);
					}
					else
					{
						ioPort.opType = OperationType.OUTPUT;
						ioPort.name = "O" + String.valueOf(ioPort.id);
					}
					IoPorts.add(ioPort);					
				}
				else if (line.startsWith(("NUMVTRACKS (")) || line.startsWith(("NUMHTRACKS (")))
				{
					String params = line.substring(line.indexOf("(")+1, line.indexOf(")"));
					String[] tokens = params.split(",");
					for (int i = 0; i < tokens.length; i++)
						tokens[i] = tokens[i].trim();
					if (tokens.length != 1)
					{
						MFError.DisplayError(line + "\n\n" + "NumVTracks/NumHTracks must have 1 parameters: ([numTracks])");
						return;
					}
					if (line.startsWith(("NUMVTRACKS (")))
						numVTracks = Integer.parseInt(tokens[0]);
					else
						numHTracks = Integer.parseInt(tokens[0]);
				}
				else if (line.startsWith(("WIREGRIDDIM (")))
				{
					String params = line.substring(line.indexOf("(")+1, line.indexOf(")"));
					String[] tokens = params.split(",");
					for (int i = 0; i < tokens.length; i++)
						tokens[i] = tokens[i].trim();
					if (tokens.length != 2)
					{
						MFError.DisplayError(line + "\n\n" + "WireGridDim must have 2 parameters: ([X_Cells], [Y_Cells])");
						return;
					}
					numXwireCells = Integer.parseInt(tokens[0]);
					numYwireCells = Integer.parseInt(tokens[1]);
				}
				else if (line.startsWith(("RELLINE (")))
				{
					String params = line.substring(line.indexOf("(")+1, line.indexOf(")"));
					String[] tokens = params.split(",");
					for (int i = 0; i < tokens.length; i++)
						tokens[i] = tokens[i].trim();
					if (tokens.length != 6)
					{
						MFError.DisplayError(line + "\n\n" + "RelLine must have 8 or 10 parameters: ([pinNum], [layerNum], [sourceGridCellX], [sourceGridCellY], [destGridCellX], [destGridCellY])");
						return;
					}
					
					// Create new linear-line wire segment
					WireSegment ws = new WireSegment();
					ws.segmentType = SEGTYPE.LINE;
					ws.pinNo = Integer.parseInt(tokens[0]);
					ws.layer = Integer.parseInt(tokens[1]);
					ws.sourceGridCellX = Integer.parseInt(tokens[2]);
					ws.sourceGridCellY = Integer.parseInt(tokens[3]);
					ws.destGridCellX = Integer.parseInt(tokens[4]);
					ws.destGridCellY = Integer.parseInt(tokens[5]);

					// Get number of layers
					if ((ws.layer + 1) > numLayers)
						numLayers = ws.layer + 1;
					
					// Check that track numbers are within range
					if (    (ws.sourceGridCellX < 0 || ws.sourceGridCellX >= numXwireCells) ||
							(ws.sourceGridCellY < 0 || ws.sourceGridCellY >= numYwireCells) ||
							(ws.destGridCellX < 0 || ws.destGridCellX >= numXwireCells) ||
							(ws.destGridCellY < 0 || ws.destGridCellY >= numYwireCells) 	)
					{
						MFError.DisplayError(line + "\n\n" + "Track number must be in range 0-" + (numXwireCells-1) + " for horizonatal wires and 0-" + (numYwireCells-1) + " for vertical wires.");
						return;
					}
					
					// Now add to list of segments for appropriate pin					
					ArrayList<WireSegment> net = WireSegsToPin.get(ws.pinNo);
					if (net == null)
					{
						net = new ArrayList<WireSegment>();
						WireSegsToPin.put(ws.pinNo, net);
					}
					WireSegsToPin.get(ws.pinNo).add(ws);
				}
				else if (!(line.isEmpty() || line.startsWith("//")))
				{
					MFError.DisplayError(line + "\n\n" + "Unspecified line type for Initialization.");
					return;
				}
				
			}			
		} catch (FileNotFoundException ex) {
			MFError.DisplayError("FileNotFoundException: " + ex.getMessage());
		} catch (IOException ex) {
			MFError.DisplayError("IOException: " + ex.getMessage());
		} finally {
			// Close the BufferedReader
			try {
				if (bf != null)
					bf.close();
			} catch (IOException ex) {
				ex.printStackTrace();
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////////////////
	// Getters/Setters
	//////////////////////////////////////////////////////////////////////////////////////
	public int getNumXcells() {
		return numXcells;
	}
	public int getNumYcells() {
		return numYcells;
	}
	public int getNumLayers() {
		return numLayers;
	}
	public int getNumXwireCells() {
		return numXwireCells;
	}
	public int getNumYwireCells() {
		return numYwireCells;
	}
	public int getNumHTracks() {
		return numHTracks;
	}
	public int getNumVTracks() {
		return numVTracks;
	}
	public ArrayList<FixedArea> getExternalResources() {
		return ExternalResources;
	}
	public ArrayList<FixedArea> getResourceLocations() {
		return ResourceLocations;
	}
	public ArrayList<IoPort> getIoPorts() {
		return IoPorts;
	}
	public Map<Integer, ArrayList<String>> getCoordsAtPin() {
		return CoordsAtPin;
	}
	public Map<Integer, ArrayList<WireSegment>> getWireSegsToPin() {
		return WireSegsToPin;
	}
}


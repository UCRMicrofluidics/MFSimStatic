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
 * Source: PlacedParser.java (Placed Parser)									*
 * Original Code Author(s): Dan Grissom											*
 * Original Completion/Release Date: October 7, 2012							*
 *																				*
 * Details: Parses an input file based on the placed format.					*
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

public class PlacedParser {
	private Map<Integer, ArrayList<String>> NodesAtMod;
	private ArrayList<AssayNode> IoNodes;
	private ArrayList<FixedArea> ExternalResources;
	private ArrayList<FixedArea> ResourceLocations;
	private ArrayList<ReconfigArea> ReconfigAreas;
	private ArrayList<IoPort> IoPorts;
	private int numXcells;
	private int numYcells;

	//////////////////////////////////////////////////////////////////////////////////////
	// Constructor
	//////////////////////////////////////////////////////////////////////////////////////
	public PlacedParser(String fileName)
	{
		NodesAtMod = new HashMap<Integer, ArrayList<String>>();
		IoNodes = new ArrayList<AssayNode>();
		ExternalResources = new ArrayList<FixedArea>();
		ResourceLocations = new ArrayList<FixedArea>();
		ReconfigAreas = new ArrayList<ReconfigArea>();
		IoPorts = new ArrayList<IoPort>();
		numXcells = 0;
		numYcells = 0;
		ReadFile(fileName);
	}

	//////////////////////////////////////////////////////////////////////////////////////
	// Open and read from MF output file
	//////////////////////////////////////////////////////////////////////////////////////
	private void ReadFile(String fileName) {
		BufferedReader bf = null;
		try {
			bf = new BufferedReader(new FileReader(fileName));
			String line = null;

			// Read each line
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
				else if (line.startsWith("EXTERNAL ("))
				{
					String params = line.substring(line.indexOf("(")+1, line.indexOf(")"));
					String[] tokens = params.split(",");
					for (int i = 0; i < tokens.length; i++)
						tokens[i] = tokens[i].trim();
					if (tokens.length != 5)
					{
						MFError.DisplayError(line + "\n\n" + "External Module must have 5 parameters: ([HEAT/DETECT], [TL_X], [TL_Y], [BR_X], [BR_Y]) ");
						return;
					}
					FixedArea fa = new FixedArea();
					fa.tl_x = Integer.parseInt(tokens[1]);
					fa.tl_y = Integer.parseInt(tokens[2]);
					fa.br_x = Integer.parseInt(tokens[3]);
					fa.br_y = Integer.parseInt(tokens[4]);

					// Must correspond to ResourceType in enums.h
					if (Integer.parseInt(tokens[0]) == 2)
						fa.opType = OperationType.HEAT;
					else if (Integer.parseInt(tokens[0]) == 1)
						fa.opType = OperationType.DETECT;
					else
					{
						MFError.DisplayError(line + "\n\n" + "Unsupported fixed (external) type.");
						return;
					}

					ExternalResources.add(fa);
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
				else if (line.startsWith("RECONFIG ("))
				{
					String params = line.substring(line.indexOf("(")+1, line.indexOf(")"));
					String[] tokens = params.split(",");
					for (int i = 0; i < tokens.length; i++)
						tokens[i] = tokens[i].trim();
					if (tokens.length != 10)
					{
						MFError.DisplayError(line + "\n\n" + "ReconfigMixer/Diluter/Splitter/Heater/Detector must have 10 parameters: ([Id], [OpType], [ResType], [TL_X], [TL_Y], [BR_X], [BR_Y], [Start_TS_Inclusive], [Stop_TS_Exclusive], [tileNum])");
						return;
					}
					ReconfigArea ra = new ReconfigArea();
					ra.id = Integer.parseInt(tokens[0]);
					ra.tl_x = Integer.parseInt(tokens[3]);
					ra.tl_y = Integer.parseInt(tokens[4]);
					ra.br_x = Integer.parseInt(tokens[5]);
					ra.br_y = Integer.parseInt(tokens[6]);
					ra.start_TS = Long.parseLong(tokens[7]);
					ra.stop_TS = Long.parseLong(tokens[8]);

					if (tokens[1].toUpperCase().equals("MIX"))
					{
						ra.opType = OperationType.MIX;
						ra.name = "M" + String.valueOf(ra.id);
					}
					else if (tokens[1].toUpperCase().equals("DILUTE"))
					{
						ra.opType = OperationType.DILUTE;
						ra.name = "Dil" + String.valueOf(ra.id);
					}
					else if (tokens[1].toUpperCase().equals("SPLIT"))
					{
						ra.opType = OperationType.SPLIT;
						ra.name = "S" + String.valueOf(ra.id);
					}
					else if (tokens[1].toUpperCase().equals("HEAT"))
					{
						ra.opType = OperationType.HEAT;
						ra.name = "H" + String.valueOf(ra.id);
					}
					else if (tokens[1].toUpperCase().equals("DETECT"))
					{
						ra.opType = OperationType.DETECT;
						ra.name = "Det" + String.valueOf(ra.id);
					}
					else if (tokens[1].toUpperCase().equals("STORAGE") || tokens[1].toUpperCase().equals("STORAGE_HOLDER"))
					{
						ra.opType = OperationType.STORAGE;
						ra.name = "St" + String.valueOf(ra.id);
					}

					ReconfigAreas.add(ra);
				}
				else if (line.startsWith("INPUT (") || line.startsWith("OUTPUT ("))
				{
					String params = line.substring(line.indexOf("(")+1, line.indexOf(")"));
					String[] tokens = params.split(",");
					for (int i = 0; i < tokens.length; i++)
						tokens[i] = tokens[i].trim();
					if (tokens.length != 7)
					{
						MFError.DisplayError(line + "\n\n" + "Input/Output must have 7 parameters: ([Input/Output_Id], [Side=NORTH,SOUTH,EAST,WEST], [PosXorY], [Time (s)], [pinNo], [FluidName], [WashPort=True/False])");
						return;
					}
					IoPort ioPort = new IoPort();
					ioPort.id = Integer.parseInt(tokens[0]);
					ioPort.side = tokens[1].toUpperCase();
					ioPort.pos_xy = Integer.parseInt(tokens[2]);
					ioPort.portName = tokens[5].toUpperCase();
					if (tokens[6].toUpperCase().equals("TRUE"))
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
				
				else if (line.startsWith("NODE ("))
				{
					String params = line.substring(line.indexOf("(")+1, line.indexOf(")"));
					String[] tokens = params.split(",");
					for (int i = 0; i < tokens.length; i++)
						tokens[i] = tokens[i].trim();
					//if (tokens.length != 4)
					//	MFError.DisplayError(line + "\n\n" + "Nodes must have 4 parameters: ([NodeId], [NodeType], [Name], [Mod/Io-Id])");

					String descrip = "";
					String type = tokens[1].toUpperCase();
					if (type.equals("MIX"))
						descrip += "MIX_" + tokens[0];
					else if (type.equals("DILUTE"))
						descrip += "DIL_" + tokens[0];
					else if (type.equals("SPLIT"))
						descrip += "SPLT_" + tokens[0];
					else if (type.equals("HEAT"))
						descrip += "HEAT_" + tokens[0];
					else if (type.equals("DETECT"))
						descrip += "DET_" + tokens[0];
					else if (type.equals("STORAGE"))
						descrip += "STOR_" + tokens[0];
					else if (type.equals("GENERAL"))
						descrip += "GEN_" + tokens[0];

					if (!tokens[4].isEmpty() && (type.equals("MIX") || type.equals("DILUTE") || type.equals("SPLIT") || type.equals("DETECT")))
						descrip += ("\n(" + tokens[4] + ")");					
					else if (!tokens[3].isEmpty() && type.equals("HEAT"))
						descrip += ("\n(" + tokens[3] + ")");
					else if (!tokens[2].isEmpty() && (type.equals("STORAGE") || type.equals("GENERAL")))
						descrip += ("\n(" + tokens[2] + ")");

					// Add the node description to the appropriate module
					int locId = Integer.parseInt(tokens[tokens.length-1]);
					if (type.equals("DISPENSE") || type.equals("OUTPUT"))
					{
						AssayNode n = new AssayNode();
						if (type.equals("DISPENSE"))
						{
							n.type = OperationType.INPUT;
							n.id = Integer.parseInt(tokens[0]);
							n.name = tokens[4];
							n.startTS = Integer.parseInt(tokens[5]);
							n.stopTS = Integer.parseInt(tokens[6]);
							n.boundResourceId = Integer.parseInt(tokens[7]);
						}
						else
						{
							n.type = OperationType.OUTPUT;
							n.id = Integer.parseInt(tokens[0]);
							n.name = tokens[3];
							n.startTS = Integer.parseInt(tokens[4]);
							n.stopTS = Integer.parseInt(tokens[5]);
							n.boundResourceId = Integer.parseInt(tokens[6]);
						}
						IoNodes.add(n);
					}
					else
					{									
						ArrayList<String> al = NodesAtMod.get(locId);
						if (al == null)
						{
							al = new ArrayList<String>();
							NodesAtMod.put(locId, al);
						}									
						NodesAtMod.get(locId).add(descrip);
					}	
				}
				else if (line.startsWith("FREQ (") || line.startsWith("TIMESTEP (") || line.startsWith("EDGE (") || line.startsWith("ARCHNAME (") || line.startsWith("DAGNAME (")
						|| line.startsWith("PINMAP (") || line.startsWith("SPECIALPINS (") || line.startsWith("RESOURCECOUNT (") || line.startsWith("RESOURCELOCATION (")
						|| line.startsWith("SCHEDTYPE (") || line.startsWith("PINMAPTYPE (") || line.startsWith("PLACERTYPE (") || line.startsWith("HCELLSBETWEENMODIR (") || line.startsWith("VCELLSBETWEENMODIR (")
						|| line.startsWith("RESOURCEALLOCATIONTYPE ("))
				{
					// Do nothing, just ignore
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
	public ArrayList<FixedArea> getExternalResources() {
		return ExternalResources;
	}
	public ArrayList<FixedArea> getResourceLocations() {
		return ResourceLocations;
	}
	public ArrayList<ReconfigArea> getReconfigAreas() {
		return ReconfigAreas;
	}
	public ArrayList<IoPort> getIoPorts() {
		return IoPorts;
	}
	public Map<Integer, ArrayList<String>> getNodesAtMod() {
		return NodesAtMod;
	}
	public ArrayList<AssayNode> getIoNodes() {
		return IoNodes;
	}
}

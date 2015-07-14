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
 * Source: CompactRouteParser.java (Compact Route Parser)						*
 * Original Code Author(s): Dan Grissom											*
 * Original Completion/Release Date: October 7, 2012							*
 *																				*
 * Details: Parses an input file based on the compact routing format.			*
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

public class CompactRouteParser {

	private ArrayList<Map<Integer, ArrayList<Integer>>> RoutesAtTS;
	private Map<Integer, ArrayList<String>> NodesAtMod;
	private Map<Integer, ArrayList<String>> NodesAtIo;
	private ArrayList<FixedArea> ExternalResources;
	private ArrayList<FixedArea> ResourceLocations;
	private ArrayList<ReconfigArea> ReconfigAreas;
	private ArrayList<AssayNode> IoNodes;
	private ArrayList<IoPort> IoPorts;
	private int numXcells;
	private int numYcells;

	//////////////////////////////////////////////////////////////////////////////////////
	// Constructor
	//////////////////////////////////////////////////////////////////////////////////////
	public CompactRouteParser(String fileName)
	{
		RoutesAtTS = new ArrayList<Map<Integer, ArrayList<Integer>>>();
		NodesAtMod = new HashMap<Integer, ArrayList<String>>();
		NodesAtIo = new HashMap<Integer, ArrayList<String>>();
		ExternalResources = new ArrayList<FixedArea>();
		ResourceLocations = new ArrayList<FixedArea>();
		ReconfigAreas = new ArrayList<ReconfigArea>();
		IoNodes = new ArrayList<AssayNode>();
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
				if (line.contains("=======================INITIALIZATION======================="))
				{
					while (((line = bf.readLine()) != null))
					{
						line = line.toUpperCase();
						if (line.contains("=======================INIT DONE======================="))
							break;
						else if (line.startsWith(("DIM (")))
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
						else if (line.startsWith("RECONFIGMIXER (") || line.startsWith("RECONFIGDILUTER (") || line.startsWith("RECONFIGSPLITTER (") || line.startsWith("RECONFIGHEATER (") || line.startsWith("RECONFIGDETECTOR (") || line.startsWith("RECONFIGSTORAGE ("))
						{
							String params = line.substring(line.indexOf("(")+1, line.indexOf(")"));
							String[] tokens = params.split(",");
							for (int i = 0; i < tokens.length; i++)
								tokens[i] = tokens[i].trim();
							if (!(tokens.length == 7 || tokens.length == 8))
							{
								MFError.DisplayError(line + "\n\n" + "ReconfigMixer/Diluter/Splitter/Heater/Detector must have 7 or 8 parameters: ([Mixer/Diluter/Splitter/Heater/Detector_Id], [TL_X], [TL_Y], [BR_X], [BR_Y], [Start_Cycle_Inclusive], [Stop_Cycle_Exclusive], [Reconfig_Name](optional))");
								return;
							}
							ReconfigArea la = new ReconfigArea();
							la.id = Integer.parseInt(tokens[0]);
							la.tl_x = Integer.parseInt(tokens[1]);
							la.tl_y = Integer.parseInt(tokens[2]);
							la.br_x = Integer.parseInt(tokens[3]);
							la.br_y = Integer.parseInt(tokens[4]);
							la.start_TS = Long.parseLong(tokens[5]);
							la.stop_TS = Long.parseLong(tokens[6]);

							if (line.startsWith("RECONFIGMIXER ("))
							{
								la.opType = OperationType.MIX;
								la.name = "M" + String.valueOf(la.id);
							}
							else if (line.startsWith("RECONFIGDILUTER ("))
							{
								la.opType = OperationType.DILUTE;
								la.name = "Dil" + String.valueOf(la.id);
							}
							else if (line.startsWith("RECONFIGSPLITTER ("))
							{
								la.opType = OperationType.SPLIT;
								la.name = "S" + String.valueOf(la.id);
							}
							else if (line.startsWith("RECONFIGHEATER ("))
							{
								la.opType = OperationType.HEAT;
								la.name = "H" + String.valueOf(la.id);
							}
							else if (line.startsWith("RECONFIGDETECTOR ("))
							{
								la.opType = OperationType.DETECT;
								la.name = "Det" + String.valueOf(la.id);
							}
							else if (line.startsWith("RECONFIGSTORAGE ("))
							{
								la.opType = OperationType.STORAGE;
								la.name = "St" + String.valueOf(la.id);
							}
							if (tokens.length == 8)
								la.name = tokens[7];

							ReconfigAreas.add(la);
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
								descrip += (" (" + tokens[4] + ")");					
							else if (!tokens[3].isEmpty() && type.equals("HEAT"))
								descrip += (" (" + tokens[3] + ")");
							else if (!tokens[2].isEmpty() && (type.equals("STORAGE") || type.equals("GENERAL")))
								descrip += (" (" + tokens[2] + ")");

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
						else if (line.startsWith("EDGE (") || line.startsWith("DAGNAME ("))
						{
							// Do nothing, just ignore
						}
						else if (!(line.isEmpty() || line.startsWith("//")))
						{
							MFError.DisplayError(line + "\n\n" + "Unspecified line type for Initialization.");
							return;
						}
					}					
				}
				if (line.contains("ROUTING TO TIMESTEP "))
				{
					HashMap<Integer, ArrayList<Integer>> routesThisTS = new HashMap<Integer, ArrayList<Integer> >();
					int dropletNum = -1;
					while (((line = bf.readLine()) != null))
					{
						line = line.toUpperCase();
						if (line.contains("NUMBER OF DROPLETS ROUTED"))
						{
							RoutesAtTS.add(routesThisTS);
							break;
						}							
						else if (line.contains("DROPLET"))
						{								
							dropletNum = Integer.parseInt(line.substring(line.indexOf(" ")).replaceAll("[^\\d]", ""));
							routesThisTS.put(dropletNum, new ArrayList<Integer>());							
						}
						else if (!line.contains("END ROUTE"))
						{
							int x;
							int y;
							x = Integer.parseInt(line.substring(line.indexOf("("), line.indexOf(",")).replaceAll("[^\\d]", ""));
							y = Integer.parseInt(line.substring(line.indexOf(","), line.indexOf(")")).replaceAll("[^\\d]", ""));
							routesThisTS.get(dropletNum).add(x);
							routesThisTS.get(dropletNum).add(y);
						}
						//MFError.DisplayError("INVALID DROPLET IN CYCLE " + cycleNum);
					}					
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
	public Map<Integer, ArrayList<String>> getNodesAtIo() {
		return NodesAtIo;
	}
	public ArrayList<Map<Integer, ArrayList<Integer>>> getRoutesAtTS() {
		return RoutesAtTS;
	}
	public ArrayList<AssayNode> getIoNodes() {
		return IoNodes;
	}
}

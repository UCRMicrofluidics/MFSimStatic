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
 * Source: DotGraphParser.java (Dotty Graph Parser)								*
 * Original Code Author(s): Dan Grissom											*
 * Original Completion/Release Date: October 21, 2013							*
 *																				*
 * Details: Parses a dotty graph file.											*
 *																				*
 * Revision History:															*
 * WHO		WHEN		WHAT													*
 * ---		----		----													*
 * FML		MM/DD/YY	One-line description									*
 *-----------------------------------------------------------------------------*/
package dmfbSimVisualizer.parsers;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;
import dmfbSimVisualizer.common.AssayNode;
import dmfbSimVisualizer.common.DrawOptions;
import dmfbSimVisualizer.common.Edge;
import dmfbSimVisualizer.common.FixedArea;
import dmfbSimVisualizer.common.IoPort;
import dmfbSimVisualizer.common.MFError;
import dmfbSimVisualizer.common.OperationType;
import dmfbSimVisualizer.common.WireSegment;
import dmfbSimVisualizer.common.WireSegment.SEGTYPE;


public class DotGraphParser {

	private ArrayList<AssayNode> Nodes;
	private ArrayList<Edge> Edges;	
	private ArrayList<String> NodeLines;
	private ArrayList<String> EdgeLines;
	
	
	//////////////////////////////////////////////////////////////////////////////////////
	// Constructor
	//////////////////////////////////////////////////////////////////////////////////////
	public DotGraphParser(String fileName)
	{

		Nodes = new ArrayList<AssayNode>();
		Edges = new ArrayList<Edge>();
		NodeLines = new ArrayList<String>();
		EdgeLines = new ArrayList<String>();
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
				String lineOriginal = line;
				line = line.toUpperCase();

				// Assay Node
				if (line.contains("[LABEL ="))
				{
					AssayNode node = new AssayNode();
					String temp = line.substring(0, line.indexOf("[LABEL =")).trim();
					node.id = Integer.parseInt(temp);
					String timeStep = line.substring(line.indexOf("TS[") + 3);
					timeStep = timeStep.substring(0, timeStep.indexOf(")"));
					
					String[] tokens = timeStep.split(",");
					for (int i = 0; i < tokens.length; i++)
						tokens[i] = tokens[i].trim();
					if (tokens.length != 2)
					{
						MFError.DisplayError(line + "\n\n" + "Time-step info must have 2 parameters: (TS[start, stop)");
						return;
					}
					node.startTS = Integer.parseInt(tokens[0]);
					node.stopTS = Integer.parseInt(tokens[1]);
					
					Nodes.add(node);
					NodeLines.add(lineOriginal);					
				}
				else if (line.contains("->"))
				{
					Edge edge = new Edge();
					
					String[] tokens = line.split("->");
					for (int i = 0; i < tokens.length; i++)
						tokens[i] = tokens[i].replaceAll(";", " ").trim();
					if (tokens.length != 2)
					{
						MFError.DisplayError(line + "\n\n" + "Edge info must have 2 parameters: ParentID -> ChildId");
						return;
					}
					edge.parentNodeId = Integer.parseInt(tokens[0]);
					edge.childNodeId = Integer.parseInt(tokens[1]);
					
					Edges.add(edge);
					EdgeLines.add(lineOriginal);
				}				
				//else if (!(line.isEmpty() || line.startsWith("//")))
				//{
				//	MFError.DisplayError(line + "\n\n" + "Unspecified line type for Initialization.");
				//	return;
				//}
				
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
	// Takes in the draw options and rewrites the dot graph file.  The file is rewritten
	// in such a way that gives focus to the time steps in the specified range (it makes
	// all other nodes white (removes the color)).
	//
	// Returns the modifed file name
	//////////////////////////////////////////////////////////////////////////////////////
	public String RewriteDotGraphWithRangeFocus(DrawOptions options, String outDir, String fileName)
	{
		if (options.outputRangeType.equals("Time Step"))
		{
			int startTS = options.minOutputRange;
			int endTS = options.maxOutputRange;		
			int maxTS = 0;
			
			// Get Max TS and update file name
			for (int i = 0; i < Nodes.size(); i++)
		    	if (Nodes.get(i).stopTS > maxTS)
		    		maxTS = Nodes.get(i).stopTS;	
			if (endTS > maxTS)
				endTS = maxTS;
			fileName = fileName + "_" + Integer.toString(startTS) + "_" + Integer.toString(endTS);	
			
			// Create new file Copy input file into simulation directory for reference
		    try {
			    File outputFile = new File(outDir + "/" + fileName + ".dot");				
			    FileWriter out = new FileWriter(outputFile);
			    out.write("digraph G {\n");
			    
			    for (int i = 0; i < Nodes.size(); i++)
			    {
			    	AssayNode node = Nodes.get(i);
			    	String line = NodeLines.get(i);
			    	
			    	if ( (startTS <= node.startTS && endTS >= node.startTS) || (startTS < node.stopTS && endTS > node.stopTS) ||
			    			(node.startTS <= startTS && node.stopTS > startTS) || (node.startTS <= endTS && node.stopTS > endTS) )
			    		out.write(line + "\n");
			    	else
			    	{
			    		String color = line.substring(line.indexOf("fillcolor=") + 10);
			    		color = color.substring(0, color.indexOf(","));
			    		line = line.replaceFirst(color, "white");
			    		out.write(line + "\n");			    		
			    	}		    	
			    }		
			    
			    for (String e : EdgeLines)
			    	out.write(e + "\n");
			    
			    out.write("}");			    
			    out.close();
			} catch (IOException e1) { MFError.DisplayError(e1.getMessage());}
		    
		    
		}
		
		return fileName;		
	}
	
}

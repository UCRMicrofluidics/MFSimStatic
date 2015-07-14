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
 * Source: OutputThread.java (Output Thread)									*
 * Original Code Author(s): Dan Grissom											*
 * Original Completion/Release Date: October 7, 2012							*
 *																				*
 * Details: This thread class calls the appropriate visualization functions		*
 * based on the input. It is a thread class so that the GUI will remain			*
 * responsive and can be used to halt this output thread if desired by the		*
 * user.																		*
 *																				*
 * Revision History:															*
 * WHO		WHEN		WHAT													*
 * ---		----		----													*
 * FML		MM/DD/YY	One-line description									*
 *-----------------------------------------------------------------------------*/

package dmfbSimVisualizer.common;

import java.awt.Frame;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;

import javax.swing.ImageIcon;

import com.sun.j3d.utils.applet.MainFrame;

import dmfbSimVisualizer.parsers.*;
import dmfbSimVisualizer.views.ImagePanel;
import dmfbSimVisualizer.views.Main;
import dmfbSimVisualizer.views.Placed3dViewerApp;

public class OutputThread extends Thread {
	Main m;
	DrawOptions drawOptions;

	String outDir = "Sim";
	String imgExt = "png";

	//////////////////////////////////////////////////////////////////////////////////////
	// Constructor
	//////////////////////////////////////////////////////////////////////////////////////
	public OutputThread(String name, Main main, DrawOptions dOpts) {
		super(name);
		m = main;
		drawOptions = dOpts;	
		
	}
	
	//////////////////////////////////////////////////////////////////////////////////////
	// This is the method that is executed when the thread is started.
	// This will figure out which simulation type you selected in the GUI and call the
	// appropriate visualization function to create the proper output images.
	//////////////////////////////////////////////////////////////////////////////////////
	public void run()
	{
		String inFile = drawOptions.inFile;
		
		// Parse file name
		String fileName;
		if (inFile.lastIndexOf('\\') > 0)
			fileName = inFile.substring(inFile.lastIndexOf('\\'));
		else if (inFile.lastIndexOf('/') > 0)
			fileName = inFile.substring(inFile.lastIndexOf('/'));
		else
			fileName = inFile;
		
		//Delete all the old simulation images/files				
		m.updateProgress(0, "Deleting old simulation files...");
		File dir = new File(outDir);
	    String[] list = dir.list();
	    for (int i = 0; i < list.length; i++)
	    {
	      File file = new File(outDir, list[i]);
	      file.delete();
	    }	   				

		 //Copy input file into simulation directory for reference
	    try {
	    	m.updateProgress(0, "Copying Interface File To Output Directory...");
		    File inputFile = new File(inFile);
		    File outputFile = new File(outDir + "/" + fileName);
		    FileReader in;					
			in = new FileReader(inputFile);
			
		    FileWriter out = new FileWriter(outputFile);
		    int c;
		    while ((c = in.read()) != -1)
		      out.write(c);
		    in.close();
		    out.close();
		} catch (IOException e1) { MFError.DisplayError(e1.getMessage());}
	    
	    // Run the proper parser and visualizer
	    if (drawOptions.simType == "Hardware Description")
	    {
			try {
				m.updateProgress(0, "Parsing Interface File...");
				HardwareParser hp = new HardwareParser(inFile);
	    		DmfbDrawer.DrawHardware(hp, drawOptions, m);
			    m.setOutputting(false);
			} catch (IOException e1) { MFError.DisplayError(e1.getMessage());}
	    }
	    else if (drawOptions.simType == "Unscheduled DAG")
		{
			String name = fileName;
			if (fileName.lastIndexOf(".") > 0)
				name = fileName.substring(0, fileName.lastIndexOf("."));
			
			m.updateProgress(0, "Outputting DAG");
			Runtime rt = Runtime.getRuntime();
			Process p;
			try {
				p = rt.exec("../Shared/Graphviz/bin/dot.exe -T" + imgExt + " " + inFile + " -o " + outDir + "/" + name + "." + imgExt);
				p.waitFor();
				m.updateProgress(100, "DAG Output");
			} 
			catch (IOException e) {MFError.DisplayError(e.getMessage());}      
			catch (InterruptedException e1) { MFError.DisplayError(e1.getMessage());}
			m.setOutputting(false);
		}
	    else if (drawOptions.simType == "Scheduled DAG" || drawOptions.simType == "Placed DAG")
		{
			String name = fileName;
			if (fileName.lastIndexOf(".") > 0)
				name = fileName.substring(0, fileName.lastIndexOf("."));
			
			// Parse and re-write dotty graph file
			DotGraphParser gp = new DotGraphParser(inFile);
			name = gp.RewriteDotGraphWithRangeFocus(drawOptions, outDir, name);			
			
			// Call dotty to draw graph
			m.updateProgress(0, "Outputting DAG");
			Runtime rt = Runtime.getRuntime();
			Process p;
			try {
				p = rt.exec("../Shared/Graphviz/bin/dot.exe -T" + imgExt + " " + outDir + "/" + name + ".dot" + " -o " + outDir + "/" + name + "." + imgExt);
				p.waitFor();
				m.updateProgress(100, "DAG Output");
			} 
			catch (IOException e) {MFError.DisplayError(e.getMessage());}      
			catch (InterruptedException e1) { MFError.DisplayError(e1.getMessage());}
			m.setOutputting(false);
		}
		else if (drawOptions.simType == "Cyclic Simulation" || drawOptions.simType == "Cyclic Routes")
		{	
			try {
				m.updateProgress(0, "Parsing Interface File...");
				RouteParser p = new RouteParser(inFile, drawOptions, m);
		    	if (drawOptions.simType == "Cyclic Simulation")
		    		DmfbDrawer.DrawCycles(p, true, drawOptions, m);
		    	else
		    		DmfbDrawer.DrawCycles(p, false, drawOptions, m);
			    m.setOutputting(false);
			} catch (IOException e1) { MFError.DisplayError(e1.getMessage());}

		}
		else if (drawOptions.simType == "Compact Simulation")
		{
			try {
				m.updateProgress(0, "Parsing Interface File...");
				CompactRouteParser cp = new CompactRouteParser(inFile);
		    	DmfbDrawer.DrawAllRoutes(cp, drawOptions, m);
		    	cp = new CompactRouteParser(inFile); // Just read it in again b/c DrawAllRoutes() deletes some of the info
		    	DmfbDrawer.DrawAllTimeSteps(null, cp, drawOptions, m);
		    	m.setOutputting(false);
			} catch (IOException e1) { MFError.DisplayError(e1.getMessage());}
		}
		else if (drawOptions.simType == "Compact Routes")
		{
			try {
				m.updateProgress(0, "Parsing Interface File...");
				CompactRouteParser cp = new CompactRouteParser(inFile);
		    	DmfbDrawer.DrawAllRoutes(cp, drawOptions, m);
		    	m.setOutputting(false);
			} catch (IOException e1) { MFError.DisplayError(e1.getMessage());}
		}
		else if (drawOptions.simType == "2D Placement")
		{
	    	try {
	    		m.updateProgress(0, "Parsing Interface File...");
	    		PlacedParser pp = new PlacedParser(inFile);
				DmfbDrawer.DrawAllTimeSteps(pp, null, drawOptions, m);
				m.setOutputting(false);
			} catch (IOException e1) { MFError.DisplayError(e1.getMessage());}
		}
		else if (drawOptions.simType == "3D Placement")
		{	    	
			Frame frame = new MainFrame(new Placed3dViewerApp(inFile), 256, 256);
		}
		else
			MFError.DisplayError("Unknown/unhandled visualization type selected.");

	}
}

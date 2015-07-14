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
 * Source: SimulationThread.java (Simulation Thread)							*
 * Original Code Author(s): Dan Grissom											*
 * Original Completion/Release Date: October 7, 2012							*
 *																				*
 * Details: This thread class calls the C++ simulation via the Java 			*
 * Runtime.exec() command, including all arguments specified by the user.		*
 * It is a thread class so that the GUI will remain	responsive and can be used 	*
 * to halt this output thread if desired by the	user.							*
 *																				*
 * Revision History:															*
 * WHO		WHEN		WHAT													*
 * ---		----		----													*
 * FML		MM/DD/YY	One-line description									*
 *-----------------------------------------------------------------------------*/

package mfSimStaticGUI.common;

import java.awt.Frame;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStreamReader;

import mfSimStaticGUI.views.Main;



public class SimulationThread extends Thread {
	Main m;
	String command = "";

	//////////////////////////////////////////////////////////////////////////////////////
	// Constructor
	//////////////////////////////////////////////////////////////////////////////////////
	public SimulationThread(String name, Main main, String cmd) {
		super(name);
		m = main;
		command = cmd;		
	}
	
	//////////////////////////////////////////////////////////////////////////////////////
	// This is the method that is executed when the thread is started.
	// This will call the external C++ program with the command specified in the 
	// constructor.
	//////////////////////////////////////////////////////////////////////////////////////
	public void run()
	{
		// Now run the actual simulation
		try
		{
			String stdOut = "Command: " + command + "\n";
			String stdErr = "";
			
			Process p = Runtime.getRuntime().exec(command);
			m.setSimProcess(p);

			// Get the std output
			BufferedReader in = new BufferedReader(new InputStreamReader(p.getInputStream()));
			String line = "";
			while ((line = in.readLine()) != null)
			{
				stdOut = stdOut.concat(line + "\n");
				m.txtOutput.setText(stdOut);
				m.repaint();
			}
			in.close();
			
			// Get the std errors
			in = new BufferedReader(new InputStreamReader(p.getErrorStream()));
			line = "";
			while ((line = in.readLine()) != null)
			{
				stdErr = stdErr.concat("\n***ERROR: " + line);
				m.txtOutput.setText(stdOut + stdErr);
				m.repaint();
			}
			
			if (stdErr.length() > 0)
				MFError.DisplayError(stdErr);
			
			m.setSimulating(false);
			m.setButtonAsSynthesize(true);
			
			String outDir = "Output";
			File outputFile = new File(outDir + "/SimulationOutput.txt");
		    FileWriter out = new FileWriter(outputFile);
		    out.write("STD_OUTPUT:\n\n" + stdOut);
		    if (stdErr.length() > 0)
		    	out.write("\n\nSTD_ERROR:\n\n" + stdErr);
		    out.close();
			
		}
		catch (IOException e) { MFError.DisplayError(e.getMessage()); }

	}
}

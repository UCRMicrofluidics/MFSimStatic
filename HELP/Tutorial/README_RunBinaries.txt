/*------------------------------------------------------------------------------*
 *                       (c)2014, All Rights Reserved.     			*
 *       ___           ___           ___     					*
 *      /__/\         /  /\         /  /\    					*
 *      \  \:\       /  /:/        /  /::\   					*
 *       \  \:\     /  /:/        /  /:/\:\  					*
 *   ___  \  \:\   /  /:/  ___   /  /:/~/:/        				*
 *  /__/\  \__\:\ /__/:/  /  /\ /__/:/ /:/___     UCR DMFB Synthesis Framework  *
 *  \  \:\ /  /:/ \  \:\ /  /:/ \  \:\/:::::/     www.microfluidics.cs.ucr.edu	*
 *   \  \:\  /:/   \  \:\  /:/   \  \::/~~~~ 					*
 *    \  \:\/:/     \  \:\/:/     \  \:\     					*
 *     \  \::/       \  \::/       \  \:\    					*
 *      \__\/         \__\/         \__\/    					*
 *-----------------------------------------------------------------------------*/

For video tutorials, please see the following YouTube links:

Using Binaries (starting at 7:32) - 		http://www.youtube.com/watch?v=WIQjnyUvTvU#t=7m32s


NOTE: [VIDEO_HELP] tags have been inserted into the instructions below which link to a specific portion of the
IDE setup video which contains instruction/information for that particular step if more detail is needed.
---------------------------------------------------------------------------------------------------------------------
---------------------------------------------------------------------------------------------------------------------
---------------------------------------------------------------------------------------------------------------------

To Run Windows Binaries (was compiled for 64-bit Windows 7, not tested on other versions of Windows)
---------------------------------------------------------------------------------------------------------------------
1.) Ensure Java is setup on your machine:
	- Check your version of the Java Runtime Environment (JRE) at "java.com/en/download/installed.jsp?detect=jre&try=1"
		- If needed download the latest JRE at "http://www.oracle.com/technetwork/java/javase/downloads/index.html"

2.) Add Java 3D files to your JRE folder (for 3D placement visualization):
	- [VIDEO_HELP]: http://www.youtube.com/watch?v=BbBI65fyvu4#t=4m29s
	- Files to copy and destination are based on which version of the JRE you are using
	- Copy Java3D dlls and jars to the appropriate "\bin" and "\lib\ext" sub-folders in your JRE directory
		- [64-Bit JRE] Copy from "MFSimStatic_WindowsBinaries\Shared\Java3D\64" to JRE directory  (probably "C:\Program Files\Java\jreX")
		- [32-Bit JRE] Copy from "MFSimStatic_WindowsBinaries\Shared\Java3D\32" to JRE directory  (probably "C:\Program Files (x86)\Java\jreX")

3.) Double click the "MFSimStatic_WindowsBinaries\MFSimStaticGUI.jar" file to load the simulator GUI
	- [VIDEO_HELP]: http://www.youtube.com/watch?v=WIQjnyUvTvU#t=10m25s
	- Select a valid executable file (should be "MFSimStatic_WindowsBinaries\MFSimStatic.exe", shown as "MFSimStatic.exe" in the binary location)
	- To test the application, click "Select Binary" and then select:
		- On the "General" Tab
			- Synthesis Type: "Entire Flow"
			- Pin Mapping: "Pin-Mapper for Individually-Addressable Designs"
			- Drops Per Storage Mod: 2
			- Cells Between Modules: 1
		- On the "Scheduling" Tab
			- Scheduler: "List Scheduling"
			- Resource Allocation Type: "Resource Allocator for Grissom's Fixed Placement - Horiz. Route Every 1 Mods"
			- Assay: "Assays\B1_PCR.txt"
			- Dmfb Arch: "DmfbArchs\Arch_15_19_B1.txt"
		- On the "Placement" Tab
			- Placer: "Grissom's Left Edge Binder"
		- On the "Router" Tab
			- Router: "Grissom's Simplified Roy Router"
			- Compaction Type: "Add stalls to Beginning of routes"
			- Processing Engine Type: "Fixed Full PE"
			- Execution Type: "All Execution"
		- On the "Wire Routing" Tab
			- Wire Routing Type: "No Wire Routing"
			- Horizontal Tracks: 3
			- Vertical Tracks: 3
	- Then, click the "Synthesis" Tab
		- Click "Synthesize"
			- When a simulation has been properly run, you will see timing information on the output window and, at the bottom, you should read "Simulation Complete. Exiting Static MF Simulator"
			- All output files from the simulator will be found in "MFSimStatic_WindowsBinaries\Output"

4.) Double click the "MFSimStatic_WindowsBinaries\DmfbSimVisualizer.jar" file to load the visualizer GUI
	- [VIDEO_HELP]: http://www.youtube.com/watch?v=WIQjnyUvTvU#t=15m36s
	- Select the desired "Visualization Type"
	- Modify any relevant parameters
	- Click "Visualize"
		- Graphical output/visualizations will be found in the "MFSimStatic_WindowsBinaries\Sim" folder
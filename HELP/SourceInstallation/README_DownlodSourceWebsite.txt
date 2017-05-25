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

IDE Setup - 				http://www.youtube.com/watch?v=BbBI65fyvu4	(covers material in this doc and MORE!)
Source Code Overview - 			http://www.youtube.com/watch?v=_JbafP624Rs
Adding new scheduler/placer/router - 	http://www.youtube.com/watch?v=rHw3r0RO3UM
Creating and Using Binaries - 		http://www.youtube.com/watch?v=WIQjnyUvTvU


NOTE: [VIDEO_HELP] tags have been inserted into the instructions below which link to a specific portion of the
IDE setup video which contains instruction/information for that particular step if more detail is needed.
---------------------------------------------------------------------------------------------------------------------
---------------------------------------------------------------------------------------------------------------------
---------------------------------------------------------------------------------------------------------------------

To Set Up Development Evnironment (Windows/Linux):
---------------------------------------------------------------------------------------------------------------------
Download Source:
1.) Download the latest source code release (www.microfluidics.cs.ucr.edu)


Setting up Windows Environment (gcc & gdb):
2.) [WINDOWS] Download and install MinGW: 
	- [VIDEO_HELP]: http://www.youtube.com/watch?v=BbBI65fyvu4#t=1m22s
	- Download latest repository (http://sourceforge.net/projects/mingw/files/latest/download?source=files)
	- Ensure that the C++ compiler is installed (other options are optional)
2b.) [MACOS] Make sure Developer Tools are installed: enter either 'gcc' or 'make' into the Terminal application and install when prompted or install through Xcode.
3.) [WINDOWS] Adjust system environmental variables:
	- Note: Following steps may be slightly different on various Windows versions
		- Open your Windows Control Panel
		- Open the "System" page
		- Click on "Advanced system settings"
		- Click on the "Advanced" tab
		- Click on the "Environment Variables..." button
		- Under the "System variables" box, find the "PATH" Variable and Edit it
			- Add ";C:\MinGW\bin" to the end of the "Variable value" field
			- NOTE: Be careful when editing thise field, DO NOT delete anything there, only add to it the prescribed string
		-RESTART YOUR COMPUTER


Setting up General Eclipse Environment:
4.) Download Eclipse Standard
	- [VIDEO_HELP]: http://www.youtube.com/watch?v=BbBI65fyvu4#t=2m03s
5.) Setup Java
	- Download and install the latest JDK, at "http://www.oracle.com/technetwork/java/javase/downloads/index.html"
		- You must install the same version of the JDK (32bit or 64bit) as you did for eclipse (32bit or 64bit)
6.) Install the Eclipse CDT (C++ Development Tools):
	- [VIDEO_HELP]: http://www.youtube.com/watch?v=BbBI65fyvu4#t=10m24s
	- Go to the Eclipse CDT download page (http://www.eclipse.org/cdt/downloads.php)
	- Copy the address for the "p2 software repository" for your version of Eclipse
	- In Eclipse, click on "Help->Install New Software..." and paste the repository address in the "Work with:" field and
          hit the 'Enter' key on the keyboard; options should appear in the box below 
		- NOTE: This step can be finicky; sometimes you have to try a few times, restart Eclipse, or try using the "Add..." button
	- Select "CDT Main Features"; DO NOT select "CDT Optional Features" as it will cause conflicts on the installation
	- Click "Next" 
	- Select all options (C/C++ Development Tools AND SDK) and let the CDT install
	- If the install does not complete, try suspending any virus protection (e.g., AVG)
7.) Install the Eclipse WindowBuilder Plugin (For GUI development):
	- [VIDEO_HELP]: http://www.youtube.com/watch?v=BbBI65fyvu4#t=14m13s
	- Go to the Eclipse Windowbuilder download page (http://www.eclipse.org/windowbuilder/download.php)
	- Copy the "link" address for the "Release Version" "Update Site" for your version of Eclipse
	- In Eclipse, click on "Help->Install New Software..." and paste the repository address in the "Work with:" field and
          hit the 'Enter' key on your keyboard; options should appear in the box below 
		- NOTE: This step can be finicky; sometimes you have to try a few times, restart Eclipse, or try using the "Add..." button
	- Select all options
	- Click "Next" and continue through the prompts to let windowbuilder install
	- If the install does not complete, try suspending any virus protection (e.g., AVG)


Adding Java3D to your project/system:
8.) Add Java 3D files to your JRE folder (for 3D placement visualization):
	- [VIDEO_HELP]: http://www.youtube.com/watch?v=BbBI65fyvu4#t=4m29s
	- Files to copy and destination are based on which version of the JDK/JRE you are using
	- Copy Java3D dlls and jars to the appropriate "\bin" and "\lib\ext" sub-folders in your JRE directory
		- [64-Bit JRE] Copy from "MFSimStatic_Source\Shared\Java3D\64" to JRE directory  (probably "C:\Program Files\Java\jreX")
		- [32-Bit JRE] Copy from "MFSimStatic_Source\Shared\Java3D\32" to JRE directory  (probably "C:\Program Files (x86)\Java\jreX")


Setting up Eclipse Projects:
9.) Open Eclipse and click "File->Switch Workspace->Other..."; choose the directory that contains the folders for the Java and C++ projects ("MFSimStatic_Source/")
10.) Create the first Java project:
	- [VIDEO_HELP]: http://www.youtube.com/watch?v=BbBI65fyvu4#t=8m40s
	- Click "File->New->Java Project"
	- Give it a "Project name" of "DmfbSimVisualizer" (this will cause it to automatically import the existing files)
	- Click "Finish"
11.) Create the second Java project:
	- [VIDEO_HELP]: http://www.youtube.com/watch?v=BbBI65fyvu4#t=9m26s
	- Click "File->New->Java Project"
	- Give it a "Project name" of "MFSimStaticGUI" (this will cause it to automatically import the existing files)
	- Click "Finish"
12.) Create a C++ project:
	- [VIDEO_HELP]: http://www.youtube.com/watch?v=BbBI65fyvu4#t=12m20s
	- Click "File->New->Project..."
	- Expland the "C/C++" folder, select "C++ Project" and click "Next"
	- Give it a "Project name" of "MFSimStatic" (ignore warning, this will cause it to automatically import the existing files)
	- For "Project type", select "Executable->Empty Project"
	- For "Toolchains", select "MinGW GCC" (windows), "MAC OS GCC" (macos), or "Linux GCC" (linux)
	- Click "Finish"
	- When it asks to open the associated perspective, click "Yes"
13.) Add references for timer library:
	- [VIDEO_HELP]: http://www.youtube.com/watch?v=BbBI65fyvu4#t=20m50s
	- Right click on the "MFSimStatic" C++ project in the Project Explorer
	- On the menu that pops up, click "Properties->C/C++ Build->Settings
    - Click on the C++ Linker option (MinGW for windows, etc)
		- Make sure the "[All configurations]" is selected for "Configurations"
		- [WINDOWS] Add "winmm" to the "Libraries(-l) input box
		- [LINUX] Add "rt" and "dl" to the "Libraries(-l) input box)
        - [MACOS] Add "-stdlib=libstdc++" to the "Miscellaneous" input box 
	- This will remove undefined reference errors to 'timeBeginPeriod...' (Windows) or 'clock_gettime' (Linux)
    - [MACOS] - Click on the C++ Compiler section and add "-stdlib=libstdc++" to the "other flags" entry box.
14.) Add references for lpsolve library:
	- [VIDEO_HELP]: NA
	- Right click on the "MFSimStatic" C++ project in the Project Explorer
	- On the menu that pops up, click "Properties->C/C++ Build->Settings->MinGW C++ Linker->Miscellaneous"
		- Make sure the "[All configurations]" is selected for "Configurations"
		- [WINDOWS/LINUX] Add path the lpsolve55/lib file (under MFSmStatic root) to other objects
15.) Add stdlib and gcc linker flags:
	- [VIDEO_HELP]: NA
	- Right click on the "MFSimStatic" C++ project in the Project Explorer
	- On the menu that pops up, click "Properties->C/C++ Build->Settings->MinGW C++ Linker->Miscellaneous"
		- Make sure the "[All configurations]" is selected for "Configurations"
		- [WINDOWS/LINUX] Modify the linker flags field to include the following: 
		  "static-libgcc -static-libstdc++"


Running your first Simulation:
14.) Compile simple PCR example
	- [VIDEO_HELP]: http://www.youtube.com/watch?v=BbBI65fyvu4#t=21m58s
	- In Eclipse, using the "Project Explorer", open "MFSimStatic->Source->main.cc"
	- Ensure that the "C/C++" perspective is selected in the top right
		- If you don't see this perspective, click the little box with the yellow "+" to add the "C/C++" perspective
	- Click on the hammer in the toolbar to compile the Debug version of the simulator
15.) Create run configuration:
	- [VIDEO_HELP]: http://www.youtube.com/watch?v=BbBI65fyvu4#t=22m52s
	- Click "Run->Run Configurations..."
	- Double click "C/C++ Application" to create a new run configuration
	- Name it "MFSimStatic Debug" and ensure that "Debug\MFSimStatic.exe" is entered in the "C/C++ Application" field
	- NOTE: If need to test/use command-line arguments within Eclipse later, add to the "Arguments" tab in the "Program arguments" box; we will not need this for our PCR example in these instructions
	- Click "Close"
16.) Run simple simulation:
	- [VIDEO_HELP]: http://www.youtube.com/watch?v=BbBI65fyvu4#t=22m52s
	- Click "Run->Run As->Local C/C++ Application" to run the Debug configuration you just made
	- If there is no output on the Eclipse console (usually happens with the 64bit version of Eclipse):
		- Go to your 'Run Configuration'
		- Select the 'Environment' tab, and add a new variable such that variable Name="PATH", Value="C:\MinGW\bin".
	- You should see output listing the runtimes for each stage of synthesis and some basic statistics or the usage string
	- NOTE: When gathering official timing information, make sure to create a Release version of the executable as it is much faster than the Debug version	


Running the visualizations for your simulation:
17.) In your OS's folder explorer, navigate to the "\MFSimStatic_Source\MFSimStatic\Output" to view the output
	- You should see several .txt files and .dot files, possibly a .mfprog file
18.) Double click the "MFSimStatic_Source\MFSimStatic\DmfbSimVisualizer.jar" file to load the visualizer GUI
	- Select the desired "Visualization Type"
	- Modify any relevant parameters
	- Click "Visualize"
		- Graphical output/visualizations will be found in the "MFSimStatic_Source\MFSimStatic\Sim" folder

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
Now that you have run through the initial binary demonstration (README_RunBinaries.txt),
this tutorial will provide configurations to test and then give quick pointers about
what to look for in the visualization output to demonstrate the effects of changing
certain parameters.

NOTE: A triple asterisk (***) is used to denote that a particular parameter has
been changed from the previous configuration to ease the tutorial user in
identifying changes from configuration to configuration.

--------------------------------------------------------------------------------
- Configuration #1 - Initial Setup and Output ----------------------------------
--------------------------------------------------------------------------------
- "General" Tab
	- Synthesis Type: "Entire Flow"
	- Pin Mapping: "Pin-Mapper for Individually-Addressable Designs"
	- Drops Per Storage Mod: 2
	- Cells Between Modules: 1
- "Scheduling" Tab
	- Scheduler: "List Scheduling"
	- Resource Allocation Type: "Resource Allocator for Grissom's Fixed Placement
	Horiz. Route Every 1 Mods"
	- Assay: "Assays\B1_PCR.txt"
	- Dmfb Arch: "DmfbArchs\Arch_15_19_B1.txt"
- "Placement" Tab
	- Placer: "Grissom's Left Edge Binder"
- "Router" Tab
	- Router: "Grissom's Simplified Roy Router"
	- Compaction Type: "Add stalls to Beginning of routes"
	- Processing Engine Type: "Fixed Full PE"
	- Execution Type: "All Execution"
- "Wire Routing" Tab
	- Wire Routing Type: "No Wire Routing"
	- Horizontal Tracks: 3
	- Vertical Tracks: 3
--------------------------------------------------------------------------------
- Visualization Test #1 --------------------------------------------------------
--------------------------------------------------------------------------------
- Run "Unscheduled DAG"
	- Notice a graph is created with the operation dependencies
- Run "Scheduled DAG"
	- Notice the graph is now annotated with start/stop times
- Run "2D Placement"
	- Notice an image is created for each time-step (1s by default)
- Run "Cyclic Simulation"
	- Notice output draws an image for each droplet location (cycle), including
	times when droplets are mixing
- Run "Cyclic Routes"
	- Notice output draws an image for each droplet location (cycle), but only
	during the routing phases
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
- Configuration #2 - Basic Scheduling Info & Hardware Description --------------
--------------------------------------------------------------------------------
- "General" Tab
	- Synthesis Type: "Entire Flow"
	- Pin Mapping: "Pin-Mapper for Individually-Addressable Designs"
	- Drops Per Storage Mod: 2
	- Cells Between Modules: 1
- "Scheduling" Tab
	- Scheduler: "List Scheduling"
	- Resource Allocation Type: "Resource Allocator for Grissom's Fixed Placement
	Horiz. Route Every 1 Mods"
***	- Assay: "Assays\B3_Protein.txt"
***	- Dmfb Arch: "DmfbArchs\Arch_15_19_B3.txt"
- "Placement" Tab
	- Placer: "Grissom's Left Edge Binder"
- "Router" Tab
	- Router: "Grissom's Simplified Roy Router"
	- Compaction Type: "Add stalls to Beginning of routes"
	- Processing Engine Type: "Fixed Full PE"
	- Execution Type: "All Execution"
- "Wire Routing" Tab
	- Wire Routing Type: "No Wire Routing"
	- Horizontal Tracks: 3
	- Vertical Tracks: 3
--------------------------------------------------------------------------------
- Visualization Test #2 --------------------------------------------------------
--------------------------------------------------------------------------------
- Run "Scheduled DAG"
	- Notice a much larger graph. Take note of the "LS Time" in the 'DMFB
	Static Simulator GUI' output window and that it should match the end
	time of one of the output nodes in the scheduled DAG image; this is
	the schedule length
- Run "Hardware Description"
	- Ensure "Resource Locations" is selected
	- Open the "HW.png" and examine the DMFB. Take note of the thick black
	rectangles...these are the locations where modules can take place when
	using a binder during the placement stage
		- Count the number of modules and notice the spacing
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
- Configuration #3 - Resource Allocation ---------------------------------------
--------------------------------------------------------------------------------
- "General" Tab
	- Synthesis Type: "Entire Flow"
	- Pin Mapping: "Pin-Mapper for Individually-Addressable Designs"
	- Drops Per Storage Mod: 2
	- Cells Between Modules: 1
- "Scheduling" Tab
	- Scheduler: "List Scheduling"
***	- Resource Allocation Type: "Resource Allocator for Grissom's Fixed Placement
	Horiz. Route Every 0 Mods"
	- Assay: "Assays\B3_Protein.txt"
	- Dmfb Arch: "DmfbArchs\Arch_15_19_B3.txt"
- "Placement" Tab
	- Placer: "Grissom's Left Edge Binder"
- "Router" Tab
	- Router: "Grissom's Simplified Roy Router"
	- Compaction Type: "Add stalls to Beginning of routes"
	- Processing Engine Type: "Fixed Full PE"
	- Execution Type: "All Execution"
- "Wire Routing" Tab
	- Wire Routing Type: "No Wire Routing"
	- Horizontal Tracks: 3
	- Vertical Tracks: 3
--------------------------------------------------------------------------------
- Visualization Test #3 --------------------------------------------------------
--------------------------------------------------------------------------------
- Run "Scheduled DAG"
	- Notice the "LS Time" in the 'DMFB Static Simulator GUI' output window
	is much less than the last run.
- Run "Hardware Description"
	- Ensure "Resource Locations" is selected
	- Open the "HW.png" and examine the DMFB. Take note that there are now
	more thick black rectangles than the last run. This means there are 
	more places to perform operations, meaning the assay can complete
	in less time (hence the smaller schedule length)
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
- Configuration #4 - Changing Assays -------------------------------------------
--------------------------------------------------------------------------------
- "General" Tab
	- Synthesis Type: "Entire Flow"
	- Pin Mapping: "Pin-Mapper for Individually-Addressable Designs"
	- Drops Per Storage Mod: 2
	- Cells Between Modules: 1
- "Scheduling" Tab
	- Scheduler: "List Scheduling"
***	- Resource Allocation Type: "Resource Allocator for Grissom's Fixed Placement
	Horiz. Route Every 1 Mods"
***	- Assay: "Assays\B4_ProteinSplit_5.txt"
	- Dmfb Arch: "DmfbArchs\Arch_15_19_B3.txt"
- "Placement" Tab
	- Placer: "Grissom's Left Edge Binder"
- "Router" Tab
	- Router: "Grissom's Simplified Roy Router"
	- Compaction Type: "Add stalls to Beginning of routes"
	- Processing Engine Type: "Fixed Full PE"
	- Execution Type: "All Execution"
- "Wire Routing" Tab
	- Wire Routing Type: "No Wire Routing"
	- Horizontal Tracks: 3
	- Vertical Tracks: 3
--------------------------------------------------------------------------------
- Visualization Test #4 --------------------------------------------------------
--------------------------------------------------------------------------------
- Run "Scheduled DAG"
	- Notice the "LS Time" in the 'DMFB Static Simulator GUI' output window
	and DAG graph are very large (output may take a few seconds)
- Change the scheduler to "Path Scheduler" and "Synthesize" again
- Run "Scheduled DAG" again
	- Notice the "LS Time" in the 'DMFB Static Simulator GUI' output window
	and DAG graph are very large (output may take a few seconds)
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
- Configuration #5 - Changing Schedulers ---------------------------------------
--------------------------------------------------------------------------------
- "General" Tab
	- Synthesis Type: "Entire Flow"
	- Pin Mapping: "Pin-Mapper for Individually-Addressable Designs"
	- Drops Per Storage Mod: 2
	- Cells Between Modules: 1
- "Scheduling" Tab
***	- Scheduler: "Path Scheduling"
	- Resource Allocation Type: "Resource Allocator for Grissom's Fixed Placement
	Horiz. Route Every 1 Mods"
	- Assay: "Assays\B4_ProteinSplit_5.txt"
	- Dmfb Arch: "DmfbArchs\Arch_15_19_B3.txt"
- "Placement" Tab
	- Placer: "Grissom's Left Edge Binder"
- "Router" Tab
	- Router: "Grissom's Simplified Roy Router"
	- Compaction Type: "Add stalls to Beginning of routes"
	- Processing Engine Type: "Fixed Full PE"
	- Execution Type: "All Execution"
- "Wire Routing" Tab
	- Wire Routing Type: "No Wire Routing"
	- Horizontal Tracks: 3
	- Vertical Tracks: 3
--------------------------------------------------------------------------------
- Visualization Test #5 --------------------------------------------------------
--------------------------------------------------------------------------------
- Run "Scheduled DAG"
	- Notice the "PS Time" in the 'DMFB Static Simulator GUI' output window
	and DAG graph are much shorter than Configuration #4 when list-scheduling
	was used. This shows that certain schedulers have better/worse results
	on different assays
- Run "2D Placement"
	- Step through the time-steps and notice that the storage modules typically
	have more than one node bound to them (in this case, the black ovals have
	two red lines [instead of one] indicating the nodes/droplets being stored
	at that module.
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
- Configuration #6 - Storage Per Module ----------------------------------------
--------------------------------------------------------------------------------
- "General" Tab
	- Synthesis Type: "Entire Flow"
	- Pin Mapping: "Pin-Mapper for Individually-Addressable Designs"
***	- Drops Per Storage Mod: 1
	- Cells Between Modules: 1
- "Scheduling" Tab
	- Scheduler: "Path Scheduling"
	- Resource Allocation Type: "Resource Allocator for Grissom's Fixed Placement
	Horiz. Route Every 1 Mods"
	- Assay: "Assays\B4_ProteinSplit_5.txt"
	- Dmfb Arch: "DmfbArchs\Arch_15_19_B3.txt"
- "Placement" Tab
	- Placer: "Grissom's Left Edge Binder"
- "Router" Tab
	- Router: "Grissom's Simplified Roy Router"
	- Compaction Type: "Add stalls to Beginning of routes"
	- Processing Engine Type: "Fixed Full PE"
	- Execution Type: "All Execution"
- "Wire Routing" Tab
	- Wire Routing Type: "No Wire Routing"
	- Horizontal Tracks: 3
	- Vertical Tracks: 3
--------------------------------------------------------------------------------
- Visualization Test #6 --------------------------------------------------------
--------------------------------------------------------------------------------
- Run "Scheduled DAG"
	- Notice the "PS Time" in the 'DMFB Static Simulator GUI' output window
	and DAG graph has gone up since Configuration #5 b/c of the storage 
	change.
- Run "2D Placement"
	- Step through the time-steps and notice that the storage modules will
	no longer have more than one droplet being stored in a single module. 
	This shows that resources are being under-utilized, which impacts the
	schedule/assay length.
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
- Configuration #7 - Free Module Placement -------------------------------------
--------------------------------------------------------------------------------
- "General" Tab
	- Synthesis Type: "Entire Flow"
	- Pin Mapping: "Pin-Mapper for Individually-Addressable Designs"
	- Drops Per Storage Mod: 1
	- Cells Between Modules: 1
- "Scheduling" Tab
	- Scheduler: "Path Scheduling"
	- Resource Allocation Type: "Resource Allocator for Grissom's Fixed Placement
	Horiz. Route Every 1 Mods"
	- Assay: "Assays\B4_ProteinSplit_5.txt"
	- Dmfb Arch: "DmfbArchs\Arch_15_19_B3.txt"
- "Placement" Tab
***	- Placer: "KAMER Linked-List Placement"
- "Router" Tab
	- Router: "Grissom's Simplified Roy Router"
	- Compaction Type: "Add stalls to Beginning of routes"
***	- Processing Engine Type: "Free Full PE"
	- Execution Type: "All Execution"
- "Wire Routing" Tab
	- Wire Routing Type: "No Wire Routing"
	- Horizontal Tracks: 3
	- Vertical Tracks: 3
--------------------------------------------------------------------------------
- Visualization Test #7 --------------------------------------------------------
--------------------------------------------------------------------------------
- Run "Scheduled DAG"
	- Notice the "PS Time" does not change.
- Run "2D Placement"
	- Step through the time-steps and notice that the module locations are 
	no longer restricted to the black rectangles, but are now placed "freely"
	all over the DMFB
	- Notice that there is generally 1 cell in between each of the module's
	interference regions
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
- Configuration #8 - Module Spacing --------------------------------------------
--------------------------------------------------------------------------------
- "General" Tab
	- Synthesis Type: "Entire Flow"
	- Pin Mapping: "Pin-Mapper for Individually-Addressable Designs"
	- Drops Per Storage Mod: 1
***	- Cells Between Modules: 0
- "Scheduling" Tab
	- Scheduler: "Path Scheduling"
	- Resource Allocation Type: "Resource Allocator for Grissom's Fixed Placement
	Horiz. Route Every 1 Mods"
	- Assay: "Assays\B4_ProteinSplit_5.txt"
	- Dmfb Arch: "DmfbArchs\Arch_15_19_B3.txt"
- "Placement" Tab
	- Placer: "KAMER Linked-List Placement"
- "Router" Tab
	- Router: "Grissom's Simplified Roy Router"
	- Compaction Type: "Add stalls to Beginning of routes"
	- Processing Engine Type: "Free Full PE"
	- Execution Type: "All Execution"
- "Wire Routing" Tab
	- Wire Routing Type: "No Wire Routing"
	- Horizontal Tracks: 3
	- Vertical Tracks: 3
--------------------------------------------------------------------------------
- Visualization Test #8 --------------------------------------------------------
--------------------------------------------------------------------------------
- Run "2D Placement"
	- Notice that the placements are much more compact and there are generally
	0 cells in between each of the module's	interference regions. This uses
	less space, but can cause problems with routing (in fact, it may yield an
	error during routing while you try Configuration #8; if it does, just keep
	synthesizing until the simulation completes).
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
- Configuration #9 - Sequential Routes -----------------------------------------
--------------------------------------------------------------------------------
- "General" Tab
	- Synthesis Type: "Entire Flow"
	- Pin Mapping: "Pin-Mapper for Individually-Addressable Designs"
	- Drops Per Storage Mod: 1
***	- Cells Between Modules: 1
- "Scheduling" Tab
	- Scheduler: "Path Scheduling"
	- Resource Allocation Type: "Resource Allocator for Grissom's Fixed Placement
	Horiz. Route Every 1 Mods"
***	- Assay: "Assays\B1_PCR.txt"
***	- Dmfb Arch: "DmfbArchs\Arch_15_19_B1.txt"
- "Placement" Tab
	- Placer: "KAMER Linked-List Placement"
- "Router" Tab
	- Router: "Grissom's Simplified Roy Router"
***	- Compaction Type: "No Compaction"
	- Processing Engine Type: "Free Full PE"
	- Execution Type: "All Execution"
- "Wire Routing" Tab
	- Wire Routing Type: "No Wire Routing"
	- Horizontal Tracks: 3
	- Vertical Tracks: 3
--------------------------------------------------------------------------------
- Visualization Test #9 --------------------------------------------------------
--------------------------------------------------------------------------------
- Run "Cyclic Routes"
	- Notice that each droplet is routed sequentially, one after the other
	- Notice the Number of cycles spent routing in the 'DMFB Static Simulator
	GUI' output window
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
- Configuration #10 - Route Compaction -----------------------------------------
--------------------------------------------------------------------------------
- "General" Tab
	- Synthesis Type: "Entire Flow"
	- Pin Mapping: "Pin-Mapper for Individually-Addressable Designs"
	- Drops Per Storage Mod: 1
	- Cells Between Modules: 1
- "Scheduling" Tab
	- Scheduler: "Path Scheduling"
	- Resource Allocation Type: "Resource Allocator for Grissom's Fixed Placement
	Horiz. Route Every 1 Mods"
	- Assay: "Assays\B1_PCR.txt"
	- Dmfb Arch: "DmfbArchs\Arch_15_19_B1.txt"
- "Placement" Tab
	- Placer: "KAMER Linked-List Placement"
- "Router" Tab
	- Router: "Grissom's Simplified Roy Router"
***	- Compaction Type: "Add stalls to Beginning of routes"
	- Processing Engine Type: "Free Full PE"
	- Execution Type: "All Execution"
- "Wire Routing" Tab
	- Wire Routing Type: "No Wire Routing"
	- Horizontal Tracks: 3
	- Vertical Tracks: 3
--------------------------------------------------------------------------------
- Visualization Test #10 --------------------------------------------------------
--------------------------------------------------------------------------------
- Run "Cyclic Routes"
	- Notice that droplets are now routed in parallel, as possible and that 
	droplet routes will sometimes stall at the beginning of their path to 
	allow other drolets to cross their own path
	- Notice the Number of cycles spent routing in the 'DMFB Static Simulator
	GUI' output window is much less than in Configuration #9
This document contains basic rules for synthesis. It is not meant to be an authoratative document and
our software may enforce/expect additional rules, but this document describes some of the general rules
and requirements that need to be adhered to when implementing DMFB algorithms:

//////////////////////////////////////////////////////////////////////////////////////////////////////
// SCHEDULING RULES
//////////////////////////////////////////////////////////////////////////////////////////////////////
1.) All input nodes must have 0 parents
2.) All output nodes must have 0 children
3.) All mix nodes must have 1 child.
4.) All split nodes must have 1 parent.
5.) All dilute nodes much have an equal number of parents and children.
6.) #Inputs - #MixParents + #SplitChildren - #Outputs should EQUAL 0.
7.) There should be no cycles in the DAG.
8.) There should be no gaps in the DAG (i.e. each parent should end at the same time as its children)


//////////////////////////////////////////////////////////////////////////////////////////////////////
// PLACEMENT RULES
//////////////////////////////////////////////////////////////////////////////////////////////////////
1.) No two modules can use the same cells at the same time.

2.) No two modules can use cells directly adjacent to one another at the same time (i.e., there must
    be an interference region between modules)

*3.) In general, a module *should* not be placed in such a way that its cells or IR covers the cell
    directly adjacent to an I/O port.  This is treated as a WARNING, as there are cases where this
    could be okay (e.g. a module can block an output port when no droplet is being output for the entire
    duration of the module's existence).


//////////////////////////////////////////////////////////////////////////////////////////////////////
// ROUTING RULES 
//////////////////////////////////////////////////////////////////////////////////////////////////////
1.) The last droplet in a route must end with either a DROP_OUTPUT or DROP_MERGING
    status because these are the two statuses that indicate a droplet is "disappearing"
    from the DMFB
	- When outputting a droplet, the droplet's last location must be adjacent to an output port

2.) Each droplet's routing points from cycle to cycle can show either:
	- No movement (xy coordinates do not change)
	- An orthogonal movement (droplet can move 1 cell to north, south, east or west,
	  but CANNOT move diagonally

3.) There can be no gaps in the route:
	- There may be NO NULL routing points anywhere in the final route
	- Each consecutive routing point must be once cycle greater than the previous routing point

4.) There can be no droplet interference:
	- Droplets must obey the satic and dynamic interferenc rules 
	- No droplet can interfere with any other droplet on the DMFB at any time
		- Only exception is for droplets that are splitting or merging
			- In this case, the statuses of interfering droplets must BOTH be marked 
			  as DROP_MERGING or DROP_SPLITTING to ignore the interference rules



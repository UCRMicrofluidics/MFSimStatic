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

***********************************************************************************************
*                                        Bugs List                                            *
***********************************************************************************************
Jul 17, 2015
| KNOWN ISSUES |
- Number of wire-routing tracks in switching_aware_combined_wire_router_pin_mapper are hardcoded to 3
  b/c framework does not currently support setting those values before the Synthesis::WireRoute() 
  method is called (as standalone method); will work properly if run via Synthesis::EntireFlow(), but
  not via Synthesis::WireRoute(). 

| FIXED |
- Memory leaks in path_finder_wire_rotuer.cc and yeh_wire_router.cc (thanks Zach Zimmerman)

Oct 22, 2013
| KNOWN ISSUES |
- Nodes with 2+ dispense parents of the same type may cause the scheduler to crash if there are
  not enough free dispense ports (getReadyDispenseWell() returns NULL when it is expected to
  return a valid IoResource
  - Basically, the first check for getReadyDispenseWell() doesn't expect a node to have two
    parents of the same dispensing fluid type
  - TEMPORARY WORKAROUND => Try adding 1+ additional ports of the duplicate fluid OR change the
    fluid type of the duplicate fluid types so they are different; this will obviously change the
    assay, but may be a suitable workaround in some cases
- compactRoutesWithMidStalls() doesn't seem to work properly as it always defaults to compactRoutesWithBegStalls()

| FIXED |
- Putting spaces in the Assay file's DAGNAME attribute will no longer cause output problems for dotty
-----------------------------------------------------------------------------------------------
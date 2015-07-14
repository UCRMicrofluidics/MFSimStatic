NOTE: This same process can be followed to add a new scheduler, placer, wire-router and pin-mapper.
Simply replace the desired synthesis method ("scheduler", "placer", "wire-router", "pin-mapper") with "router" below.

Adding new router:
------------------
1.) Add .h file to Headers/Routers
2.) Add .cc file to Source/Routers
3.) Add enum type to RouterType in Headers/Resources/enums.h
4.) Add "else if" statement with new router name and enum to getNewRoutingMethod() (in Source/synthesis.cc)
5.) Add #include statement with new router's header to top of Source/synthesis.cc
6.) Add reference to CmdLine() constructor in Source/commandline.cc by adding values to rKeys, rDescrips, and rEnums
7.) (Optional) May need to add "friend class" references to headers of classes that give compile-time errors
    claiming a certain member "is protected" "within this context".
	- For example, when adding a copy of RoyMazeRouter, will need to add friend class references to:
		- ~/Headers/Models/assay_node.h		
8.) (Optional) Add any new restrictions to Source/compatability_check.cc to ensure the router or method you just added
    is only called with other compatible synthesis methods


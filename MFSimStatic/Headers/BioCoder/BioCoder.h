
/*------------------------------------------------------------------------------*
 *                   2012 by University of California, Riverside                *
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
/*-------------------------------Class Details----------------------------------*
 * Type: BioCoder Class															*
 * Name: BioCoder 																*
 *																				*
 * Original source code from the following paper:								*
 * Authors: Vaishnavi Ananthanarayanan and William Thies						*
 * Title: Biocoder: A programming language for standardizing and automating		*
 * 			biology protocols													*
 * Publication Details: In J. Biological Engineering, Vol. 4, No. 13, Nov 2010	*
 * 																				*
 * Details: UCR has modified and added to the original biocoder so that it 		*
 * could actually be used for synthesis on a DMFB.								*
 * 																				*
 * This class contains most of the unmodified code from the original BioCoder	*
 * with several modifications to allow it to be used to create a DAG to pass	*
 *
 *-----------------------------------------------------------------------------*/
#ifndef _BIOCODER_H
#define _BIOCODER_H

#include "TimeVolumeSpeed.h"

#include "assayProtocol.h"
#include "MCFlow.h"
#include "BioEnums.h"
#include "../Models/dag.h"
#include <iostream>
#include <cstdio>
#include <fstream>
#include <list>

//forward declarations
struct Fluid;
struct Container;
struct graphNodelist;
struct assayProtocol;

class BioCoder
{
private:
	int total_time_required;
	FILE * fp;
	int options_flag;
	int option_no;
	//initiallization of flags
	int print_reagents;
	int print_parameters;
	int first;

	int sub_step_flag;
	int prev_cont;

	//initiallization of number of containers, equipment, etc.
	int equip_no;
	int microfuge_no;
	int centrifuge_no;
	int incubator_no;
	int electrophoresis_no;
	int mortar_no;
	int thermocycler_no;
	int electro_no;
	int shaker_no;
	int spectrophotometer_no;
	int cont_no;
	int* container_count;//[24];
	char** containers;//[20];
	char** equipments;//[20];
	string filename;

	//variables required for keeping track of usage of fluids and containers
	int list_fluid_no;
	int list_container_no;
	char* prev_container;



	list<Time*> delTimePtrs;
	list<Speed*> delSpeedPtrs;
	list<Volume*> delVolumePtrs;
	graphNodelist* all_nodes;
	Fluid * usage_list_fluids;
	Container * usage_list_containers;
	assayProtocol* proSteps;
	//! Creates an html file with the given name and opens it for writing the protocol.
	/*!
	\param name the name of the protocol.
	\par Example:
	\code start_protocol("DNA Miniprep");\endcode
	\par Output:
	Internally, opens a new html file named "DNA Miniprep" for writing. The title of the protocol "DNA Miniprep" is also printed on the top left corner of the file.
	\sa end_protocol()
	*/
	void start_protocol(string name);
	/*
	exports the current runtime graph to the given filename, in .dot format
	*/
	void export_graph(const char* filename);
	void exportDagGraph(assayProtocol pro);
	void combine_helper (int count, Container * container1);
	void mix (Container &container1, enum mixing type, Time time1);
	void mix (Container &container1, enum mixing type, int times);
	void mix (Container &container1, enum mixing type, int min_times, int max_times);
	void mix (Container &container1, enum mixing type);
	void mix (Container &container1, enum until event1);
	std::string getFile();

	int get_ttr();

public:
	BioCoder(string name);
	~BioCoder();

	//! Marks the first step of a protocol.
	/*!
	\par Example:
	\code first_step(); \endcode
	\par Output:
	\htmlonly 1. \endhtmlonly
	\sa next_step(), optional_step() and first_sub_step()
	*/
	string getName();
	assayProtocol * getProtocolData();
	string save_fluid(Container & con);
	string reuse_fluid(Container & con);

	void first_step();
	//! Marks the first step of a protocol. Also specifies the name of the step.
	/*!
	\param step_name the name of the step.
	\par Example:
	\code first_step(DNA extraction); \endcode
	\par Output:
	\htmlonly <h2>Steps:</h2>
	1. <b>DNA extraction</b> \endhtmlonly
	\sa next_step(), optional_step() and first_sub_step()
	*/
	void first_step(char* step_name);
	//! Marks the ending of a step and the beginning of the next step of a protocol.
	/*!
	\par Example:
	\code next_step();\endcode
	\par Output:
	\htmlonly 2. \endhtmlonly
	\sa first_step(), optional_step() and next_sub_step()
	 */
	void next_step();
	//! Marks the ending of a step and the beginning of the next step of a protocol. Also specifies the name of the step.
	/*!
	\param step_name the name of the step.
	\par Example:
	\code next_step("DNA lysis");\endcode
	\par Output:
	\htmlonly 2.<b>DNA lysis</b> \endhtmlonly
	\sa first_step(), optional_step() and next_sub_step()
	 */
	void next_step(char* step_name);
	//! Marks a parallel step in the protocol.
	/*!
	\par Example:
	\code parallel_step(); \endcode
	\par Output:
	\htmlonly 3. <b> Meanwhile: </b> \endhtmlonly
	\sa first_step(), next_step(), first_sub_step() and next_sub_step()
	 */
	void parallel_step();
	//! Marks the first sub-step of a step in the protocol.
	/*!
	\par Example:
	\code first_sub_step(); \endcode
	\par Output:
	\htmlonly a. \endhtmlonly
	\sa next_step(), optional_step() and next_sub_step()
	 */
	void first_sub_step();
	//! Marks the end of a sub-step and the start of the subsequent sub-step of a step in the protocol.
	/*!
	\par Example:
	\code next_sub_step();\endcode
	\par Output:
	\htmlonly b. \endhtmlonly
	\sa next_step(), optional_step() and next_sub_step()
	 */
	void next_sub_step();
	//! Used to prompt the user to repeat a given step of a protocol.
	/*!
	\param step_no the step number of the protocol that should be repeated.
	\par Example:
	\code repeat(3);\endcode
	\par Output:
	\htmlonly Repeat Step 3. \endhtmlonly
	 */
//	void repeat(int step_no);
	/*
	displays the equipment used during the protocol on the html document
	*/
	void display_equip();
	void usage_details();
	void timing_details();
	//! Marks the end of the protocol (calls display_equipment, closes the file and resets all the variables).
	/*!
	\par Example:
	\code end_protocol; \endcode
	\par Output:
	Internally, closes the html file.
	\sa start_protocol()
	 */
	void end_protocol();
	void check_container(Container& container1);
	//! Marks the steps in the protocol that need further clarification for coding with BioCoder.
	/*!
	\par Example:
	\code to_do("Add the samples to appropriate wells."); \endcode
	\par Output:
	\htmlonly <font color=red><i>Add the samples to appropriate wells.</i></font> \endhtmlonly
	\sa comment()
	 */
	void to_do(char* clarify);
	//! Used to specify a hint/ caution type message that supplements a step in a protocol.
	/*!
	\par Example:
	\code comment("Take care with this step as the pellet may not stick to the tube completely.");\endcode
	\par Output:
	\htmlonly <font color=#800517><i>Take care with this step as the pellet may not stick to the tube completely.</i></font> \endhtmlonly
	\sa to_do()
	 */
	void comment(char* comm);



	//! Used to declare a new fluid with the given name.
	/*!
	\param name the name of the fluid.
	\return Fluid
	\par Example:
	\code Fluid solution1 = new_fluid("Solution 1"); \endcode
	\par Output:
	\htmlonly <ul type="circle"><li>Solution 1</li></ul> \endhtmlonly
	\sa new_solid(), new_container() and new_plate()
	*/
	Fluid new_fluid(char* name);
	//! Used to declare a new fluid with the given name.
/*!
\param name the name of the fluid.
\param volume1 the initial volume of the fluid.
\return Fluid
\par Example:
\code Fluid solution1 = new_fluid("Solution 1", vol(10, ML)); \endcode
\par Output:
\htmlonly <ul type="circle"><li>10 ml Solution 1</li></ul> \endhtmlonly
\sa new_solid(), new_container() and new_plate()
 */
	Fluid new_fluid(char* name, Volume volume1);
	//! Used to declare a new fluid with the given name. The solid has been stored at the specified temperature.
/*!
\param name the name of the fluid.
\param temp the temperature of storage of the fluid.
\return Fluid
\par Example:
\code Fluid solution1 = new_fluid("Solution 1", ICE_COLD, vol(10, ML)); \endcode
\par Output:
\htmlonly <ul type="circle"><li>10 ml ice-cold Solution 1</li></ul> \endhtmlonly
\sa new_solid(), new_container() and new_plate()
 */
	Fluid new_fluid(char*name, float temp);
	//! Used to declare a new fluid with the given name. The solid has been stored at the specified temperature.
/*!
\param name the name of the fluid.
\param temp the temperature of storage of the fluid.
\param volume1 the initial volume of the fluid.
\return Fluid
\par Example:
\code Fluid solution1 = new_fluid("Solution 1", ICE_COLD); \endcode
\par Output:
\htmlonly <ul type="circle"><li>ice-cold Solution 1</li></ul> \endhtmlonly
\sa new_solid(), new_container() and new_plate()
 */
	Fluid new_fluid(char*name, float temp, Volume volume1);
	//! Used to declare a new fluid with the given name and characteristics.
/*!
\param name the name of the fluid.
\param state the characteristics of the fluid (Eg: "freshly prepared" or composition of a solution).
\return Fluid
\par Example:
\code Fluid solution1 = new_fluid("Solution 1", "50mM SDS, 20mM TE"); \endcode
\par Output:
\htmlonly <ul type="circle"><li>Solution 1<br><i>(50mM SDS, 20mM TE)</i></li></ul> \endhtmlonly
\sa new_solid(), new_container() and new_plate()
 */
	Fluid new_fluid(char*name, char* state);
	//! Used to declare a new fluid with the given name and characteristics.
/*!
\param name the name of the fluid.
\param state the characteristics of the fluid (Eg: "freshly prepared" or composition of a solution).
\param volume1 the initial volume of the fluid.
\return Fluid
\par Example:
\code Fluid solution1 = new_fluid("Solution 1", "50mM SDS, 20mM TE", vol(10, ML)); \endcode
\par Output:
\htmlonly <ul type="circle"><li>10 ml Solution 1<br><i>(50mM SDS, 20mM TE)</i></li></ul> \endhtmlonly
\sa new_solid(), new_container() and new_plate()
 */
	Fluid new_fluid(char*name, char* state, Volume volume1);
	//! Used to declare a new fluid with the given name and characteristics. The fluid has been stored at the specified temperature.
/*!
\param name the name of the fluid.
\param state the characteristics of the fluid (Eg: "freshly prepared" or composition of a solution).
\param temp the temperature of storage of the fluid.
\return Fluid
\par Example:
\code Fluid solution1 = new_fluid("Solution 1", "50mM SDS, 20mM TE", RT); \endcode
\par Output:
\htmlonly <ul type="circle"><li>Solution 1 at room temperature<br><i>(50mM SDS, 20mM TE)</i></li></ul> \endhtmlonly
\sa new_solid(), new_container() and new_plate()
 */
	Fluid new_fluid (char* name, char*state, float temp);
	//! Used to declare a new fluid with the given name and characteristics. The fluid has been stored at the specified temperature.
/*!
\param name the name of the fluid.
\param state the characteristics of the fluid (Eg: "freshly prepared" or composition of a solution).
\param temp the temperature of storage of the fluid.
\param volume1 the initial volume of the fluid.
\return Fluid
\par Example:
\code Fluid solution1 = new_fluid("Solution 1", "50mM SDS, 20mM TE", RT, vol(10, ML)); \endcode
\par Output:
\htmlonly <ul type="circle"><li>10 ml Solution 1 at room temperature<br><i>(50mM SDS, 20mM TE)</i></li></ul> \endhtmlonly
\sa new_solid(), new_container() and new_plate()
 */
	Fluid new_fluid (char* name, char*state, float temp, Volume volume1);
	Fluid new_operation(char* name);


	void store_container_names(int i,char* name);

	//! Used to declare a new container of the type specified by \c cont_id.
	/*!
	\param cont_id the type of container.
	\return Container
	\par Example:
	\code Container microfuge_tube1 = new_container(STERILE_MICROFUGE_TUBE);\endcode
	\par Output:
	Internally, sets aside a sterile microfuge tube with the name "a sterile microfuge tube".
	\sa enum container_type, new_fluid(), new_solid() and new_plate()
	 */
	Container new_container(enum container_type cont_id);
	//! Used to declare a new container of the type specified by \c cont_id that contains a fluid at the start of the protocol.
	/*!
	\param cont_id the type of container.
	\param fluid1 the contents of the container.
	\return Container
	\par Example:
	\code Container microfuge_tube1 = new_container(STERILE_MICROFUGE_TUBE, comp_cells);\endcode
	\par Output:
	Internally, sets aside a sterile microfuge tube with the name "a sterile microfuge tube" and \c comp_cells as contents.
	\sa enum container_type, new_fluid(), new_solid() and new_plate()
	 */
	Container new_container(enum container_type cont_id, Fluid& fluid1);
	//! Sets the specified container as the working container for the specified sample.
	/*!
	\param fluid1 the sample whose container is being set.
	\param container1 the container which contains \c sample1.
	\par Example:
	\code set_container(culture, flask);\endcode
	\par Output:
	Internally, set the contents of \c flask to \c culture.
	\sa new_container(), enum container_type
	 */

	void set_container(Fluid& fluid1, Container& container1);
	//! Measures out \c container's into another specified container.
	/*!
	\param container the container whose contents have to be measured out.
	\param container1 the container into which \c fluid1 has to be measured.
	\par Example:
	\code measure_fluid(rxn_tube, eppendorf); \endcode
	\par Output:
	\htmlonly Measure out <font color=#357EC7>(rxn_tube.contents.name)</font> into (eppendorf.name).<br>\endhtmlonly
	\sa measure_solid() and measure_prop()
	 */

	void measure_fluid(Container& container, Container& container1);
	//! Measures out \c fluid1 into the specified container.
	/*!
	\param fluid1 the fluid that has to be measured out.
	\param container1 the container into which \c fluid1 has to be measured.
	\par Example:
	\code measure_fluid(solution1, eppendorf);
	 measure_fluid(solution2, eppendorf);\endcode
	\par Output:
	\htmlonly Measure out <font color=#357EC7>(solution1.name)</font> into (eppendorf.name).<br>
	Add <font color=#357EC7>(solution2.name)</font> to (solution1.name).\endhtmlonly
	\sa measure_solid() and measure_prop()
	 */
	void measure_fluid(Fluid& fluid1, Container& container1);
	//! Measures out a specified volume of \c fluid1.
	/*!
	\param fluid1 the fluid that has to be measured out.
	\param volume1 the volume of \c fluid1 required.
	\par Example:
	\code measure_fluid(solution1, vol(1, ML));\endcode
	\par Output:
	\htmlonly Measure out <b><font color=#357EC7>1 ml</font></b> of <font color=#357EC7>(solution1.name)</font>.\endhtmlonly
	\sa measure_solid() and measure_prop()
	 */
//	void measure_fluid(Fluid& fluid1, Volume volume1);


	//! Measures out a specified volume of \c fluid1 into the given container.
	/*!
	\param fluid1 the fluid that has to be measured out.
	\param volume1 the volume of \c fluid1 required.
	\param container1 the container into which \c fluid1 has to be measured.
	\par Example:
	\code measure_fluid(solution1, vol(1, ML), tube1);
	 measure_fluid(solution2, vol(1, ML), tube1);\endcode
	\par Output:
	\htmlonly Measure out <b><font color=#357EC7>1 ml</font></b> of <font color=#357EC7>(solution1.name)</font> into (tube1.name).<br>
	Add <b><font color=#357EC7>1 ml</font></b> of <font color=#357EC7>(solution2.name)</font>.\endhtmlonly
	\sa measure_solid() and measure_prop
	 */
	void measure_fluid(Fluid& fluid1, Volume volume1, Container& container1);
	//! Measures out a specified volume of contents of container \c con into another container, \c container1.
	/*!
	\param con the container whose contents have to be measured out.
	\param volume1 the volume of \c con 's contents required.
	\param container1 the destination container of the measured contents of \c con.
	\par Example:
	\code measure_fluid(eppendorf1, vol(1, ML), tube1);\endcode
	\par Output:
	\htmlonly Measure out <b><font color=#357EC7>1 ml</font></b> of <font color=#357EC7>(eppendorf1.contents.name)</font> into (tube1.name).\endhtmlonly
	\sa measure_solid() and measure_prop
	 */
	void measure_fluid(Container& con, Volume volume1, Container& container1, bool ensureMeasurement);
	void measure_fluid(Container& source, int NumSplit, int piecesToDest, Container& Dest, bool ensureMeasurement);
	//! Sets the temperature of the contents of the given container as specified.
	/*!
	\param container1 the container whose contents have to be set to a particular temperature.
	\param temp the temperature setting.
	\par Example:
	\code set_temp(eppendorf1, 37); \endcode
	\par Output:
	\htmlonly Set the temperature of (eppendorf1.contents.name) to <b><font color=#357EC7>37 C</font></b>. \endhtmlonly
	\sa store(), store_for(), store_until(), incubate()
	 */
	void set_temp(Container& container1, float temp);
	//			//! Measures a specified proportion of fluid \c source and adds it to the \c dest container. Combination of measure_fluid() and combine() instructions.
	//			/*!
	//			\param dest the container to which \c source is added.
	//			\param source the fluid that has to be measured out.
	//			\param prop the proportion of \c dest 's contents that has to be measured out of \c source.
	//			\par Example:
	//			\code measure_prop(tube1, phenol, 2); \endcode
	//			\par Output:
	//			\htmlonly Add <b><font color=#357EC7>2</font></b> volumes <font color=#357EC7>(phenol.name)</font>. \endhtmlonly
	//			\sa measure_fluid and measure_fluid()
	//			 */
	//			void measure_prop(Container& dest, Fluid& source, float prop);
	//			//! Measures a specified proportion of \c source container's contents and adds it to the \c dest container. Combination of measure_fluid() and combine() instructions.
	//			/*!
	//			\param dest the container to which \c source 's contents are added.
	//			\param source the container whose contents have to be measured out.
	//			\param prop the proportion of \c dest 's contents that has to be measured out of \c source.
	//			\par Example:
	//			\code measure_prop(tube1, tube2, 2); \endcode
	//			\par Output:
	//			\htmlonly Add <b><font color=#357EC7>2</font></b> volumes <font color=#357EC7>(tube2.contents.name)</font>. \endhtmlonly
	//			\sa measure_fluid and measure_fluid()
	//			 */
	//			void measure_prop(Container& dest, Container& source, float prop);
	//! Combines the contents of the given containers.
	/*!
	\param count the number of containers whose contents have to be mixed.
	\param container1 the first container of \c count number of containers.
	\par Example:
	\code combine(3, tube1, tube2, tube3); \endcode
	\par Output:
	\htmlonly Combine the (tube1.contents.name), (tube2.contents.name), (tube3.contents.name). \endhtmlonly
	\sa combine_and_mix()
	 */
	void combine (int count, Container* container1 );
	//! Transfers the contents of a container \c container1 to another specified container \c con2.
	/*!
	\param container1 the container whose contents have to be transferred.
	\param con2 the container to which \c container1 's contents have to be transferred.
	\par Example:
	\code transfer(tube1, tube2);\endcode
	\par Output:
	\htmlonly Transfer (tube1.contents.name) into (tube2.name). \endhtmlonly
	\sa combine_and_mix()
	 */
	void transfer(Container& container1, Container& con2);
	//! Discards the contents of the specified container.
	/*!
	\par Example:
	\code discard(epepndorf1);\endcode
	\par Output:
	\htmlonly Discard (eppendorf1.contents.name). \endhtmlonly
	\param container1 the container whose contents have to be discarded.
	 */
	void discard(Container& container1, string outputSink);
	//! Mixes the contents of the given container by tapping the container for a few seconds.
	/*!
	\param container1 the container whose contents have to be mixed.
	\par Example:
	\code tap(eppendorf);\endcode
	\par Output:
	\htmlonly Gently tap the mixture for a few secs. \endhtmlonly
	\sa stir(), invert(), vortex(), dissolve(), resuspend(), pipet() and combine_and_mix()
	 */
	void tap(Container& container1);
	//! Mixes the contents of the given container by tapping until the specified result is obtained.
	/*!
	\param container1 the container whose contents have to be mixed.
	\param event1 the event until which the container has to be mixed. (See enum until)
	\par Example:
	\code tap(eppendorf, PELLET_DISLODGES); \endcode
	\par Output:
	\html only Close the tube tightly and gently tap the mixture for a few secs until the pellet dislodges. \endhtmlonly
	\sa stir(), invert(), vortex(), dissolve(), resuspend(), pipet(), combine_and_mix() and enum until
	 */
	void tap (Container& container1, enum until event1);
	//! Mixes the contents of the given container by tapping the container for the specified duration of time.
	/*!
	\param container1 the container whose contents have to be mixed.
	\param time1 the duration of tapping.
	\par Example:
	\code tap(eppendorf, time(10, SECS));\endcode
	\par Output:
	\htmlonly Gently tap the mixture for <b><font color=#357EC7>10 secs</font></b>. \endhtmlonly
	\sa invert(), vortex(), dissolve(), resuspend(), pipet() and combine_and_mix()
	 */
	 void tap(Container& container1, Time time1);
	 //! Mixes the contents of the given container by stirring the container for a few seconds.
	/*!
	\param container1 the container whose contents have to be mixed.
	\par Example:
	\code stir(eppendorf);\endcode
	\par Output:
	\htmlonly Gently stir the mixture for a few secs. \endhtmlonly
	\sa tap(), invert(), vortex(), dissolve(), resuspend(), pipet() and combine_and_mix()
	 */
	void stir(Container& container1);
	//! Mixes the contents of the given container by stirring the container for the specified duration of time.
	/*!
	\param container1 the container whose contents have to be mixed.
	\param time1 the duration of tapping.
	\par Example:
	\code stir(eppendorf, time(10, SECS));\endcode
	\par Output:
	\htmlonly Gently stir the mixture for <b><font color=#357EC7>10 secs</font></b>. \endhtmlonly
	\sa tap(), invert(), vortex(), dissolve(), resuspend(), pipet() and combine_and_mix()
	 */
	void stir(Container& container1, Time time1);
	//! Mixes the contents of the given container by inverting the container for a few seconds.
	/*!
	\param container1 the container whose contents have to be mixed.
	\par Example:
	\code invert(eppendorf);\endcode
	\par Output:
	\htmlonly Close the tube tightly and gently mix the contents by inverting the tube. \endhtmlonly
	\sa tap(), stir(), vortex(), dissolve(), resuspend(), pipet() and combine_and_mix()
	 */
	void invert(Container& container1);
	//! Mixes the contents of the given container by inverting it for the specified number of times.
	/*!
	\param container1 the container whose contents have to be mixed.
	\param times the number of times that the container has to be inverted.
	\par Example:
	\code invert(eppendorf, 5);\endcode
	\par Output:
	\htmlonly Close the tube tightly and gently mix the contents by inverting the tube <b><font color=#357EC7>5 times</font></b>. \endhtmlonly
	\sa tap(), stir(), vortex(), dissolve(), resuspend(), pipet() and combine_and_mix()
	 */
	void invert(Container& container1, int times);
	//! Mixes the contents of the given container by inverting it for the specified number of times.
	/*!
	\param container1 the container whose contents have to be mixed.
	\param min_times the minimum number of times that the container has to be inverted.
	\param max_times the maximum number of times that the container has to be inverted.
	\par Example:
	\code invert(eppendorf, 3, 5);\endcode
	\par Output:
	\htmlonly Close the tube tightly and gently mix the contents by inverting the tube <b><font color=#357EC7>3 - 5 times</font></b>. \endhtmlonly
	\sa tap(), stir(), vortex(), dissolve(), resuspend(), pipet() and combine_and_mix()
	 */
	void invert(Container& container1, int min_times, int max_times);
	//! Mixes the contents of the given container by inverting it until the specified result is obtained.
	/*!
	\param container1 the container whose contents have to be mixed.
	\param event1 the event until which the container has to be mixed. (See enum until)
	\par Example:
	\code invert(eppendorf, PPT_STOPS_STICKING);\endcode
	\par Output:
	\htmlonly Close the tube tightly and gently mix the contents by inverting the tube until the precipitate stops sticking to the walls of the tube. \endhtmlonly
	\sa tap(), stir(), vortex(), dissolve(), resuspend(), pipet(), combine_and_mix() and enum until
	 */
	void invert(Container& container1, enum until event1);
	//! Mixes the contents of the given container by vortexing the container for a few seconds.
	/*!
	\param container1 the container whose contents have to be mixed.
	\par Example:
	\code vortex(eppendorf);\endcode
	\par Output:
	\htmlonly Vortex the mixture for a few seconds. \endhtmlonly
	\sa tap(), stir(), invert(), dissolve(), resuspend(), pipet() and combine_and_mix()
	 */
	void vortex(Container& container1);
	//! Mixes the contents of the given container by vortexing the container for the specified duration of time.
	/*!
	\param container1 the container whose contents have to be mixed.
	\param time1 the duration of vortexing.
	\par Example:
	\code vortex(eppendorf, time(1, MINS));\endcode
	\par Output:
	\htmlonly Vortex the mixture for <b><font color=#357EC7>1 min</font></b>. \endhtmlonly
	\sa tap(), stir(), invert(), dissolve(), resuspend(), pipet() and combine_and_mix()
	 */
	void vortex(Container& container1, Time time1);
	//! Resuspends the contents of the solution in tube.
	/*!
	\param container1 the container whose contents have to be resuspended.
	\par Example:
	\code resuspend(eppendorf);\endcode
	\par Output:
	\htmlonly Resuspend the (eppendorf.contents.name) by vortexing/shaking vigorously. \endhtmlonly
	\sa tap(), stir(), invert(), vortex(), dissolve(), pipet() and combine_and_mix()
	 */
	void resuspend(Container& container1);
	//! Dissolves the contents of the solution in tube.
	/*!
	\param container1 the container whose contents have to be dissolved.
	\par Example:
	\code dissolve(eppendorf1);\endcode
	\par Output:
	\htmlonly Dissolve the pellet in the solution. \endhtmlonly
	\sa tap(), stir(), invert(), vortex(), resuspend(), pipet() and combine_and_mix()
	 */
	void dissolve (Container& container1);
	//! Resuspends the contents of the solution in tube by pipetting up and down for a few seconds.
	/*!
	\param container1 the container whose contents have to be resuspended.
	\par Example:
	\code pipet(tube1);\endcode
	\par Output:
	\htmlonly Mix solution by pipetting up and down several times. \endhtmlonly
	\sa tap(), stir(), invert(), vortex(), dissolve(), resuspend() and combine_and_mix()
	 */
	void pipet (Container& container1);
	//! Holds the given container for the specified unit of time.
	/*!
	\param container1 the container whose contents have to be held.
	\param time1 the duration for which \c container1 has to be held.
	\par Example:
	\code wait(tube1, time(5, MINS)); \endcode
	\par Output:
	\htmlonly Keep (tube1.contents.name) aside for <b><font color=#357EC7>5 mins</font></b>. \endhtmlonly
	\sa store(), store_for(), store_until(), incubate()
	 */
	void wait (Container& container1, Time time1);
	//! Combines the given list of containers' contents and mixes them with the specified setting for a few seconds.
	/*!
	\param type the type of mixing. (See enum mixing)
	\param count the number of containers whose contents have to be mixed.
	\param container1 the first of the list of containers whose contents have to be mixed.
	\par Example:
	\code combine_and_mix(VORTEXING, 2, tube1, tube2); \endcode
	\par Output:
	\htmlonly Combine the (tube1.contents.name), (tube2.contents.name).
	Vortex the mixture for a few secs. \endhtmlonly
	\sa combine(), tap(), stir(), invert(), vortex(), resuspend(), dissolve(), pipet() and enum mixing
	 */
	void combine_and_mix (enum mixing type, int count, Container* container1);
	//! Combines the given list of containers' contents and mixes them for the specified duration of time and setting.
	/*!
	\param type the type of mixing. (See enum mixing)
	\param time1 the duration of mixing.
	\param count the number of containers whose contents have to be mixed.
	\param container1 the first of the list of containers whose contents have to be mixed.
	\par Example:
	\code combine_and_mix(VORTEXING, time_range(2, 3, MINS), 2, tube1, tube2); \endcode
	\par Output:
	\htmlonly Combine the (tube1.contents.name), (tube2.contents.name).
	Vortex the mixture for <b><font color=#357EC7>2 - 3 mins</font></b>. \endhtmlonly
	\sa combine(), tap(), stir(), invert(), vortex(), resuspend(), dissolve(), pipet() and enum mixing
	 */
	void combine_and_mix (enum mixing type, Time time1, int count, Container* container1 );
	//! Stores the specified container at a given temperature.
	/*!
	\param container1 the container whose contents have to be stored.
	\param temp the temperature of storage.
	\par Example:
	\code store(tube1, ON_ICE);\endcode
	\par Output:
	\htmlonly Store the tube <b><font color=#357EC7>on ice</font></b>. \endhtmlonly
	\sa  store_for(), store_until(), incubate()
	 */
	void store(Container& container1, float temp);
	//! Stores the specified container at a given temperature and given duration of time. Combination of set_temp() and wait().
	/*!
	\param container1 the container whose contents have to be stored.
	\param temp the temperature of storage.
	\param time1 the duration of storage.
	\par Example:
	\code store_for(tube1, 37, min_time(5, MINS));\endcode
	\par Output:
	\htmlonly Store at <b><font color=#357EC7>37 c</font></b> for at most <b><font color=#357EC7>5 mins</font></b>. \endhtmlonly
	\sa  store(), store_until() and incubate()
	 */
	void store_for(Container& container1, float temp, Time time1);
	//! Stores the specified container at a given temperature and given duration of time. Combination of set_temp() and wait().
	/*!
	\param container1 the container whose contents have to be stored.
	\param temp the temperature of storage.
	\param time1 the duration of storage.
	\param function the function being performed as a result of the store_for().
	\par Example:
	\code store_for(eppendorf, 80, time(30, SECS), ENZYME_INAC);\endcode
	\par Output:
	\htmlonly Perform enzyme inactivation by storing at <b><font color=#357EC7>80 C</font></b> for <b><font color=#357EC7>30 secs</font></b>. \endhtmlonly
	\sa store(), store_until() and incubate()
	 */
	void store_for(Container& container1, float temp, Time time1, enum func function);
	//! Stores the specified container at a given temperature until the occurence of a specified event.
	/*!
	\param container1 the container whose contents have to be stored.
	\param temp the temperature of storage.
	\param type the event until whose occurence \c container1 has to be stored.
	\par Example:
	\code store_until(tube1, RT, ETHANOL_EVAP);\endcode
	\par Output:
	\htmlonly Store (tube1.contents.name) at <b><font color=#357EC7>room temperature</font></b> until the ethanol has evaporated and no fluid is visible in the tube.\endhtmlonly
	\sa store(), store_for(), incubate(), and enum time_unit
	 */
	void store_until(Container& container1, float temp, enum until type);
	//! Stores the specified container at a given temperature until the occurence of a specified event. The approximate time taken for the occurence of the event is also specified.
	/*!
	\param container1 the container whose contents have to be stored.
	\param temp the temperature of storage.
	\param type the event until whose occurence \c container1 has to be stored.
	\param time1 the approximate time taken for the occurence of the event.
	\par Example:
	\code store_until(tube1, RT, ETHANOL_EVAP, time_range(10, 15, MINS));\endcode
	\par Output:
	\htmlonly Store (tube1.contents.name) at <b><font color=#357EC7>room temperature</font></b> until the ethanol has evaporated and no fluid is visible in the tube(~ <b><font color=#357EC7>10-15 mins</font></b>).\endhtmlonly
	\sa store(), store_for() and incubate()
	 */
	void store_until(Container& container1, float temp, enum until type, Time time1);
	//! Incubates the given container at the specified temperature for the specified duration of time, without shaking.
	/*!
	\param container1 the container whose contents need to be incubated.
	\param temp the temperature of incubation.
	\param time1 the duration of incubation.
	\par Example:
	\code incubate(eppendorf, ON_ICE, time_range(3, 5, MINS)); \endcode
	\par Output:
	\htmlonly Incubate <b><font color=#357EC7>on ice</font></b> for <b><font color=#357EC7>3 - 5 mins</font></b>. \endhtmlonly
	\sa store(), store_for(), store_until() and inoculation()
	 */
	void incubate(Container& container1, float temp, Time time1);
	//! Incubates the given container at the specified temperature for the specified duration of time, with shaking at the given rpm.
	/*!
	\param container1 the container whose contents need to be incubated.
	\param temp the temperature of incubation.
	\param time1 the duration of incubation.
	\param rpm the speed of shaking while incubating (in rpm).
	\par Example:
	\code incubate(flask, RT, time(3, HRS), 200); \endcode
	\par Output:
	\htmlonly Incubate at <b><font color=#357EC7>room temperature</font></b> for <b><font color=#357EC7>3 hrs</font></b> with shaking at <font color=#357EC7>200</font> rpm. \endhtmlonly
	\sa store(), store_for(), store_until() and inoculation()
	*/
	void incubate(Container& container1, float temp, Time time1, int rpm);
	//! Incubates the given container at the specified temperature for the specified duration of time, with shaking at the given rpm and stirring during the incubation as specified.
	/*!
	\param container1 the container whose contents need to be incubated.
	\param temp the temperature of incubation.
	\param time1 the duration of incubation.
	\param time_mix the time intervals at which the contents of the container have to be mixed during incubation.
	\param type the type of mixing.
	\par Example:
	\code incubate_and_mix(flask, RT, time(3, HRS), 200, time(10, MINS)); \endcode
	\par Output:
	\htmlonly Incubate at <b><font color=#357EC7>room temperature</font></b> for <b><font color=#357EC7>3 hrs</font></b> with shaking at <font color=#357EC7>200 rpm</font>, mixing gently by stirring the container every <b><font color=#357EC7>10 mins</font></b>. \endhtmlonly
	\sa store(), store_for(), store_until(), incubate(), inoculation() and enum ranges
	 */
	void incubate_and_mix(Container& container1, float temp, Time time1, Time time_mix, enum mixing type);
	//! Names the contents of the specified container with the \c new_name.
	/*!
	\param container1 the container whose contents need to be named.
	\param new_name the desired name of the contents of \c container1.
	\par Example:
	\code name_sample(tube1, "lysate"); \endcode
	\par Output:
	Internally, the name of \c tube1 's contents is set to "lysate".
	\sa name_container()
	 */
	void name_sample(Container& container1, char* new_name);
	//! Labels a container with the given name.
	/*!
	\param container1 the container that has to be labeled.
	\param name the name with which \c container1 has to be labeled.
	\par Example:
	\code name_container(eppendorf1, "Reaction Tube"); \endcode
	\par Output:
	\htmlonly Set aside a fresh (eppendorf1.name). Call it Reaction Tube. \endhtmlonly
	\sa name_sample()
	 */
	void name_container(Container& container1, char* name);
	//! Sends the contents of the given container to a capillary electrophoresis(CE) unit for separation/detection at the given settings.
	/*!
	\param container1 the container whose contents need to be detected/separated with CE.
	\param length length of the CE column.
	\param volt_per_cm voltage setting.
	\param fluid1 the separation buffer.
	\par Example:
	\code ce_detect(eppendorf1, 23, 200, sep_buffer); \endcode
	\par Output:
	\htmlonly Detect/separate (eppendorf1.contents.name) by capillary electrophoresis with the following settings - <b><font color=#357EC7>23</font></b> cm at <b><font color=#357EC7>200</font></b> V/cm using (sep_buffer.name).\endhtmlonly.
	\sa nanodrop()
	 */
	string ce_detect (Container& container1, float length, float volt_per_cm, Fluid& fluid1);
	//! Sends the contents of the given container to a capillary electrophoresis(CE) unit for separation/detection at the given settings.
	/*!
	\param container1 the container whose contents need to be detected/separated with CE.
	\param length length of the CE column.
	\param volt_per_cm voltage setting.
	\param fluid1 the separation buffer.
	\param time1 the duration of capillary electrophoresis at the given settings.
	\par Example:
	\code ce_detect(eppendorf1, 23, 200, sep_buffer, time(3, MINS)); \endcode
	\par Output:
	\htmlonly Detect/separate (eppendorf1.contents.name) by capillary electrophoresis with the following settings - <b><font color=#357EC7>23</font></b> cm at <b><font color=#357EC7>200</font></b> V/cm using (sep_buffer.name) for <b><font color=#357EC7>3 mins</font></b>.\endhtmlonly.
	\sa nanodrop()
	 */
	string ce_detect (Container& container1, float length, float volt_per_cm, Fluid& fluid1, Time time1);
	//! Measures the fluorescence of the contents of the specified container.
	/*!
	\param container1 the container whose contents need to be measured for fluorescence.
	\par Example:
	\code measure_fluorescence(eppendorf1);\endcode
	\par Output:
	\htmlonly Measure the fluorescence of (eppendorf1.contents.name).\endhtmlonly
	 */
	string measure_fluorescence (Container& container1, Time time1);
	//! Drains the specified container.
	/*!
	\param container1 the container that has to be drained.
	\par Example:
	\code drain(cult_chamber); \endcode
	\par Output:
	\htmlonly Drain (cult_chamber.name). \endhtmlonly
	\sa discard()
	 */
	void drain(Container& container1, string outputSink);
	//! Performs agarose gel elecrophoresis of the contents of \c container1.
	/*!
	\param container1 the container whose contents have to be subjected to agarose gel electrophoresis.
	\par Example:
	\code electrophoresis(eppendorf1);\endcode
	\par Output:
	\htmlonly Perform agarose gel electrophoresis of appropriate quantity of (eppendorf1.contents.name)mixed with ethidium bromide
	and visualize with UV transilluminator to confirm the presence of required product. \endhtmlonly
	\sa nanodrop()
	 */
	string electrophoresis(Container& container1);
	//! Performs agarose gel elecrophoresis of the contents of \c container1.
	/*!
	\param container1 the container whose contents have to be subjected to agarose gel electrophoresis.
	\param agar_conc the concentration of agarose in the gel.
	\par Example:
	\code electrophoresis(eppendorf1, 0.8);\endcode
	\par Output:
	\htmlonly Perform 0.8% agarose gel electrophoresis of appropriate quantity of (eppendorf1.contents.name)mixed with ethidium bromide
	and visualize with UV transilluminator to confirm the presence of required product. \endhtmlonly
	\sa nanodrop()
	 */
	string electrophoresis(Container& container1, float agar_conc);
	//! Performs agarose gel elecrophoresis of the contents of \c container1.
	/*!
	\param no_samples the number of samples loaded on to the gel.
	\param container_array the containers whose contents have to be subjected to agarose gel electrophoresis.
	\param agar_conc the concentration of agarose in the gel.
	\param pdt_vol the volume of sample used for electrophoresis.
	\param loading_buff_vol the volume of loading buffer used.
	\par Example:
	\code electrophoresis(2, array1, 0.8, vol(5, UL), vol(1, UL));\endcode
	\par Output:
	\htmlonly Perform 0.8% agarose gel electrophoresis of (array1[0].contents.name) and (array1[1].contents.name)(5µl sample and 1µl of loading buffer), mixed with ethidium bromide
	and visualize with UV transilluminator to confirm the presence of required product. \endhtmlonly
	\sa nanodrop()
	 */
	string electrophoresis(int no_samples, Container container_array[], float agar_conc, Volume pdt_vol, Volume loading_buff_vol);
	//! Prompts the user to send the sample for sequencing after diluting to appropriate concentration.
	/*!
	\param container1 the container whose contents have to be sent for sequencing.
	\par Example:
	\code sequencing(eppendorf); \endcode
	\par Output:
	\htmlonly Dilute to <font color=#357EC7>100ng/ µL</font> and send <font color=#357EC7>1 µg (10 µL)</font> for sequencing. \endhtmlonly
	\sa electrophoresis(), nanodrop()
	 */
	string sequencing(Container& container1);
	//! Prompts the user to weigh the amount of solid present in the given container.
	/*!
	\param container1 the container with the solid whose weight needs to be determined.
	\par Example:
	\code weigh(eppendorf);\endcode
	\par Output:
	\htmlonly Weigh the amount of (eppendorf.contents.name) present. \endhtmlonly
	 */
	string weigh(Container& container1);
	//! Peforms fluorescence activated cell sorting (FACS) of the contents of \c container1.
	/*!
	\param container1 the container whose contents have to be sorted using FACS.
	\par Example:
	\code facs(eppendorf1); \endcode
	\par Output:
	\htmlonly FACS: Sort (eppendor1.contents.name) based on fluorescence. \endhtmlonly
	 */
	string facs(Container& container1);
	//! Performs cell culture of the cells present in the given container with the specified parameters.
	/*!
	\param cells the container with the cells that need to be cultured.
	\param medium the cell culture medium.
	\param centri_speed the speed of centrifugation to retrieve the cells. (in rpm)
	\param temp the temperature of cell culture.
	\param time the duration of cell culture.
	\param percent_CO2 the percent of carbon dioxide in the cell culture chamber.
	\param for_wash_valves the fluid used to wash the valves in the cell culture chamber.
	\param for_wash_chambers the fluid used to wash the chamber.
	\param for_trypsinization the fluid used for trypsinization.
	\param for_feeding the time duration between two consecutive feeds.
	\par Example:
	\code cell_culture(cult_chamber, medium, 200, RT, 48, 30, fluid1, fluid2, fluid3, 20); \endcode
	\par Output:
	\htmlonly Perform cell culture with the specified parameters. \endhtmlonly
	 */
	string cell_culture(Container& cells, Fluid& medium, int centri_speed, float temp, float time, float percent_CO2, Fluid& for_wash_valves, Fluid& for_wash_chambers, Fluid& for_trypsinization, float for_feeding);
	//! Transfects the cells contained in \c container1 with DNA.
	/*!
	\param container1 the container with the cells to be transfected.
	\param medium the medium for transfection.
	\param dna the fluid with the DNA that will transfect the cells in \c container1.
	\par Example:
	\code transfection(flask, growth_medium, dna); \endcode
	\par Output:
	\htmlonly Transfect (flask.contents.name) with (dna.name). \endhtmlonly
	 */
	string transfection(Container& container1, Fluid& medium, Fluid& dna);
	//! Electroporates the contents of the specified container with the given settings.
	/*!
	\param container1 the container whose contents have to be subjected to electroporation.
	\param voltage the electroporation voltage.
	\param no_pulses the number of voltage pulses.
	\par Example:
	\code electroporate(tube1, 220, 5); \endcode
	\par Output:
	\htmlonly Set the electroporator to deliver <b><font color=#357EC7>220 V</font></b>, and then press the PULSE button <b><font color=#357EC7>5 times</font></b>.\endhtmlonly
	 */
	string electroporate (Container& container1, float voltage, int no_pulses);
	//! Used to declare a new parameter (symbolic volume) with the given name and definition.
	/*!
	\param symbol the symbol used to denote the parameter(symbolic volume).
	\param def the definition of the parameter.
	\return Plate
	\par Example:
	\code Symbol x = new_symbol("X", "concentration of vector DNA");\endcode
	\par Output:
	\htmlonly X - concentration of vector DNA \endhtmlonly
	\sa new_fluid(), new_solid(), new_plate() and new_container()
	 */
	Symbol new_symbol(char* symbol, char* def);
	//! Used to declare a new parameter (symbolic time) with the given name and definition.
	/*!
	\param symbol the symbol used to denote the parameter(symbolic volume).
	\param def the definition of the parameter.
	\return Plate
	\par Example:
	\code Symbol_t x = new_symbol_t("X", "time taken for elongation");\endcode
	\par Output:
	\htmlonly X - time taken for elongation \endhtmlonly
	\sa new_fluid(), new_solid(), new_plate() and new_container()
	 */
	Symbol_t new_symbol_t(char* symbol, char* def);
	//! Sets the value of the specified symbolic volume.
	/*!
	\param symbol1 the symbol whose value is being set.
	\param value the value of the symbol.
	\param unit the unit of \c value.
	\par Example:
	\code set_value(x, 4, UL); \endcode
	\par Output:
	Internally, \c x 's value is set to '4' and \c x 's unit is set to 'UL'.
	\sa new_container(), enum container_type
	 */
	void set_value(Symbol& symbol1, float value, enum vol_unit unit);
	//! Sets the value of the specified symbolic volume.
	/*!
	\param symbol1 the symbol whose value is being set.
	\param value the value of the symbol.
	\param unit the unit of \c value.
	\par Example:
	\code set_value(x, 1, MINS); \endcode
	\par Output:
	Internally, \c x 's value is set to '4' and \c x 's unit is set to 'MINS'.
	\sa new_container(), enum container_type
	 */
	void set_value(Symbol_t& symbol1, float value, enum time_unit unit);
	//! Sets symbolic volume \c s1 to be equal to symbolic volume \c s2..
	/*!
	\param s1 the symbolic volume that is assigned a value.
	\param s2 the symbolic volume that \c s1 is equal to.
	\par Example:
	\code assign(s_vol(x), s_vol(y)); \endcode
	\par Output:
	\htmlonly Let (x.name) = (y.name). \endhtmlonly
	\sa new_symbol() and set_value()
	 */
	void assign(Symbol& s1, Symbol& s2);
	//! Sets symbolic volume \c s1 to be equal to symbolic volume \c s2..
	/*!
	\param s1 the symbolic time that is assigned a value.
	\param s2 the symbolic time that \c s1 is equal to.
	\par Example:
	\code assign(s_time(x), s_time(y)); \endcode
	\par Output:
	\htmlonly Let (x.name) = (y.name). \endhtmlonly
	\sa new_symbol() and set_value()
	 */
	void assign(Symbol_t& s1, Symbol_t& s2);
	//! Subtracts an actual volume from a symbolic volume.
	/*!
	\param vol1 the symbolic volume.
	\param vol2 the actual volume.
	\par Example:
	\code y = subtract(s_vol(x), vol(10, UL));\endcode
	\par Output:
	Internally, \c y is assigned the value \c x-10.
	\sa add(), multiply() and divide()
	 */
	Symbol subtract(Symbolic_vol* vol1, Volume vol2);
	//! Subtracts a symbolic volume from an actual volume.
	/*!
	\param vol1 the actual volume.
	\param vol2 the symbolic volume.
	\par Example:
	\code y = subtract(vol(10, UL), s_vol(x));\endcode
	\par Output:
	Internally, \c y is assigned the value \c 10-x.
	\sa add(), multiply() and divide()
	 */
	Symbol subtract(Volume vol1,Symbolic_vol*  vol2);
	//! Subtracts a symbolic volume from another symbolic volume.
	/*!
	\param vol1 the first symbolic volume.
	\param vol2 the other symbolic volume.
	\par Example:
	\code z = subtract(s_vol(x), s_vol(y)));\endcode
	\par Output:
	Internally, \c z is assigned the value \c x-y.
	\sa add(), multiply() and divide()
	 */
	Symbol subtract(Symbolic_vol* vol1, Symbolic_vol* vol2);
	//! Adds a symbolic volume with another symbolic volume.
	/*!
	\param vol1 the symbolic volume.
	\param vol2 the other symbolic volume.
	\par Example:
	\code Symbol z = add(s_vol(x), s_vol(y)); \endcode
	\par Output:
	Internally, \c z is assigned the value \c x+y.
	\sa subtract(), multiply() and divide()
	 */
	Symbol add(Symbolic_vol* vol1, Symbolic_vol* vol2);
	//! Adds a symbolic volume with an actual volume.
	/*!
	\param vol1 the symbolic volume.
	\param vol2 the actual volume.
	\par Example:
	\code Symbol y = add(s_vol(x), vol(10, UL)); \endcode
	\par Output:
	Internally, \c y is assigned the value \c x+10.
	\sa subtract(), multiply() and divide()
	 */
	Symbol add(Symbolic_vol* vol1, Volume vol2);
	//! Multiplies a symbolic volume with an actual volume.
	/*!
	\param vol1 the symbolic volume.
	\param vol2 the actual volume.
	\par Example:
	\code y = multiply(s_vol(x), vol(10, UL)); \endcode
	\par Output:
	Internally, \c y is assigned the value \c x*10.
	\sa add(), subtract() and divide()
	 */
	Symbol multiply(Symbolic_vol* vol1, Volume vol2);
	//! Multiplies a symbolic volume with another symbolic volume.
	/*!
	\param vol1 the symbolic volume.
	\param vol2 the other symbolic volume.
	\par Example:
	\code z = multiply(s_vol(x), s_vol(y)); \endcode
	\par Output:
	Internally, \c z is assigned the value \c x*y.
	\sa add(), subtract() and divide()
	 */
	Symbol multiply(Symbolic_vol* vol1, Symbolic_vol* vol2);
	//! Divides a symbolic volume by an actual volume.
	/*!
	\param vol1 the symbolic volume (dividend).
	\param vol2 the actual volume (divisor).
	\par Example:
	\code y = divide(s_vol(x), vol(10, UL)); \endcode
	\par Output:
	Internally, \c y is assigned the value \c x/10.
	\sa add(), multiply() and subtract()
	 */
	Symbol divide(Symbolic_vol* vol1, Volume vol2);
	//! Divides an actual volume by a symbolic volume.
	/*!
	\param vol1 the actual volume (dividend).
	\param vol2 the symbolic volume (divisor).
	\par Example:
	\code y = divide(vol(10, UL), s_vol(x)); \endcode
	\par Output:
	 Internally, \c y is assigned the value \c 10/x.
	\sa add(), multiply() and subtract()
	 */
	Symbol divide(Volume vol1, Symbolic_vol*  vol2);
	//! Divides a symboic volume by another symbolic volume.
	/*!
	\param vol1 the dividend.
	\param vol2 the divisor.
	\par Example:
	\code z = divide(s_vol(x), s_vol(y)); \endcode
	\par Output:
	 Internally, \c z is assigned the value \c x/y.
	\sa add(), multiply() and subtract()
	 */
	Symbol divide(Symbolic_vol* vol1, Symbolic_vol*  vol2);

	void drainAndFillWith(Container & drainCon, string outputSink, Container & filler);
	Time time (float x, enum time_unit unit);
	Time_range time_range (float x, float y, enum time_unit unit);
	Minimum_time min_time(float x, enum time_unit unit);
	Maximum_time max_time(float x, enum time_unit unit);
	Symbolic_time s_time(Symbol_t s);
	Approx_time approx_time (float x, enum time_unit unit);
	Speed speed (float x, enum speed_type unit);
	Speed_range speed_range (float x, float y, enum speed_type unit);
	Minimum_speed min_speed(float x, enum speed_type unit);
	Maximum_speed max_speed(float x, enum speed_type unit);
	Approx_speed approx_speed(float x, enum speed_type unit);
	Volume vol(float x, enum vol_unit unit);
	Symbolic_vol s_vol(Symbol s);
	Volume_range vol_range(float x, float y, enum vol_unit unit);
	Approx_volume approx_vol(float x, enum vol_unit unit);
//	Time* time (float x, enum time_unit unit);
//	Time_range* time_range (float x, float y, enum time_unit unit);
//	Minimum_time* min_time(float x, enum time_unit unit);
//	Maximum_time* max_time(float x, enum time_unit unit);
//	Symbolic_time* s_time(Symbol_t s);
//	Approx_time* approx_time (float x, enum time_unit unit);
//	Speed* speed (float x, enum speed_type unit);
//	Speed_range* speed_range (float x, float y, enum speed_type unit);
//	Minimum_speed* min_speed(float x, enum speed_type unit);
//	Maximum_speed* max_speed(float x, enum speed_type unit);
//	Approx_speed* approx_speed(float x, enum speed_type unit);
//	Volume vol(float x, enum vol_unit unit);
//	Symbolic_vol* s_vol(Symbol s);
//	Volume_range* vol_range(float x, float y, enum vol_unit unit);
//	Approx_volume* approx_vol(float x, enum vol_unit unit);


	int getDropRestriction();
	void setDropRestriction(int n);
	//shell functions for assayProtocol
	void printAssaySteps();
	void printLinks();
	void printMixtures();
	//=============================================================================
	//Translator from the BioCoder to Dan Grissom Scheduler
	//
	//int x and y are coordinate points that signify the size of the microfluidic
	//chip they are used to establish the dispense locations of the chip
	//=============================================================================
	void Translator(DAG *dag);
	void TransWebApp ();// takes in xml file and translates them into biocoder steps and feeds them into the Sim
	//=============================================================================
	//Translator from the BioCoder to Control Flow System(Jeff McDaniel)
	//=============================================================================
	void CFTranslator(string s);


};


#endif
